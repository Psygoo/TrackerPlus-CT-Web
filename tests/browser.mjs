// MIT. Real browser path; fixtures remain outside the served/public tree.
import assert from 'node:assert/strict';
import { readFile, mkdir } from 'node:fs/promises';
import { pathToFileURL } from 'node:url';
import { createHash } from 'node:crypto';
import { createServer } from '../tools/serve.mjs';
import { OUTPUT_SHA256, OUTPUT_NAME } from '../patcher.js';
assert(process.env.STOCK_PTF && process.env.H12_PTF, 'Set private STOCK_PTF and H12_PTF fixture paths');
const stock = await readFile(process.env.STOCK_PTF), expected = await readFile(process.env.H12_PTF);
const moduleName = process.env.PLAYWRIGHT_MODULE ? pathToFileURL(process.env.PLAYWRIGHT_MODULE).href : 'playwright';
const playwright = await import(moduleName);
const requests = [], server = createServer(req => requests.push(req));
await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
const url = `http://127.0.0.1:${server.address().port}/TrackerPlus-CT-Web/`;
const results = [];
try {
  for (const engine of (process.env.BROWSER_ENGINES ?? 'chromium,firefox,webkit').split(',')) {
    const browser = await playwright[engine].launch({ headless: true });
    try {
      const context = await browser.newContext({ acceptDownloads: true, serviceWorkers: 'block' });
      const network = [], errors = [], consoleErrors = [], violations = [];
      context.on('request', req => network.push({ url: req.url(), method: req.method(), body: req.postData() }));
      const page = await context.newPage();
      page.on('pageerror', e => errors.push(e.message));
      page.on('console', m => { if (m.type() === 'error') consoleErrors.push(m.text()); });
      await page.addInitScript(() => { window.cspViolations = []; document.addEventListener('securitypolicyviolation', e => window.cspViolations.push(e.violatedDirective)); });
      await page.goto(url);
      await page.waitForFunction(() => !document.querySelector('#choose').disabled);
      assert.equal(await page.locator('#download').isDisabled(), true);
      assert.match(await page.title(), /H12/);
      const loaded = network.length;
      // Abort all subsequent network requests. Successful patching must need none.
      const unexpected = [];
      await context.route('**/*', route => { unexpected.push(route.request().url()); return route.abort(); });
      const bad = stock.subarray(0, 100);
      await page.locator('#firmware').setInputFiles({ name: 'wrong.ptf', mimeType: 'application/octet-stream', buffer: bad });
      await page.waitForFunction(() => document.body.dataset.state === 'error');
      assert.equal(await page.locator('#download').isDisabled(), true);
      const verifyDownload = async () => {
        await page.waitForFunction(() => document.body.dataset.state === 'success', { }, { timeout: 90000 });
        assert.equal(await page.locator('#output-hash').textContent(), OUTPUT_SHA256);
        const event = page.waitForEvent('download'); await page.locator('#download').click();
        const download = await event;
        assert.equal(download.suggestedFilename(), OUTPUT_NAME);
        const bytes = await readFile(await download.path());
        assert.equal(createHash('sha256').update(bytes).digest('hex'), OUTPUT_SHA256);
        assert(bytes.equals(expected));
        if (process.env.EVIDENCE_DIR) {
          await mkdir(process.env.EVIDENCE_DIR, { recursive: true });
          await download.saveAs(`${process.env.EVIDENCE_DIR}/${engine}-generated-H12.ptf`);
        }
      };
      // Normal file chooser, actual module worker, output hash gate and Blob save.
      await page.locator('#firmware').setInputFiles({ name: 'TrackerPlus_1.2.0.ptf', mimeType: 'application/octet-stream', buffer: stock });
      await verifyDownload();
      // A bad selection after success must revoke stale output.
      await page.locator('#firmware').setInputFiles({ name: 'wrong.ptf', mimeType: 'application/octet-stream', buffer: bad });
      await page.waitForFunction(() => document.body.dataset.state === 'error');
      assert.equal(await page.locator('#download').isDisabled(), true);
      assert.equal(await page.locator('#hash').isHidden(), true);
      // Drag-and-drop path and repeated stable output, with no network enabled.
      const handle = await page.evaluateHandle(base64 => {
        const bytes = Uint8Array.from(atob(base64), c => c.charCodeAt(0));
        const data = new DataTransfer(); data.items.add(new File([bytes], 'original.ptf'));
        return data;
      }, stock.toString('base64'));
      await page.locator('#drop').dispatchEvent('drop', { dataTransfer: handle });
      await handle.dispose(); await verifyDownload();
      assert.deepEqual(unexpected, []); assert.equal(network.length, loaded);
      assert.deepEqual(await page.evaluate(() => window.cspViolations), []);
      // Deliberate fetch verifies CSP; no request must reach server or route.
      const before = requests.length;
      assert.equal(await page.evaluate(async () => { try { await fetch('./firmware-upload', { method: 'POST', body: 'synthetic privacy probe' }); return false; } catch { return true; } }), true);
      assert.equal(requests.length, before); assert.deepEqual(unexpected, []);
      assert((await page.evaluate(() => window.cspViolations)).includes('connect-src'));
      violations.push('deliberate connect-src probe blocked');
      await context.unroute('**/*');
      for (const width of [320, 375, 1280]) {
        await page.setViewportSize({ width, height: 900 });
        assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), `horizontal overflow at ${width}`);
        if (process.env.EVIDENCE_DIR) {
          await mkdir(process.env.EVIDENCE_DIR, { recursive: true });
          await page.screenshot({ path: `${process.env.EVIDENCE_DIR}/${engine}-${width}.png`, fullPage: true });
        }
      }
      await page.goto(url + 'documentation.html');
      assert(await page.locator('#compatibility').count());
      assert(await page.locator('#credits').count());
      for (const width of [320, 1280]) {
        await page.setViewportSize({ width, height: 900 });
        assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), `documentation overflow at ${width}`);
      }
      assert.deepEqual(errors, []);
      assert(consoleErrors.every(s => /connect-src|Content Security Policy|Content-Security-Policy|NetworkError|Load failed|Fetch API|Failed to fetch/i.test(s)), consoleErrors.join('\n'));
      assert(network.every(req => req.url.startsWith(url) && req.method === 'GET' && req.body === null));
      results.push({ engine, version: browser.version(), exact_ptf: true, repeated_downloads: 2, processing_network_requests: 0, csp_upload_blocked: true, invalid_input_and_stale_output: 'pass', widths: [320, 375, 1280], errors });
    } finally { await browser.close(); }
  }
  assert(requests.every(req => ['GET', 'HEAD'].includes(req.method) && req.contentLength === '0'));
  console.log(JSON.stringify({ pass: true, output_sha256: OUTPUT_SHA256, server_received_firmware: false, results }, null, 2));
} finally { await new Promise(resolve => server.close(resolve)); }
