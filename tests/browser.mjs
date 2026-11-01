// MIT. Real browser path; fixtures remain outside the served/public tree.
import assert from 'node:assert/strict';
import { readFile, mkdir, stat } from 'node:fs/promises';
import { pathToFileURL, fileURLToPath } from 'node:url';
import { resolve, sep, extname } from 'node:path';
import { createHash } from 'node:crypto';
import { createServer } from '../tools/serve.mjs';
import { OUTPUT_SHA256, OUTPUT_NAME } from '../patcher.js';
assert(process.env.STOCK_PTF && process.env.CT2_PTF, 'Set private STOCK_PTF and CT2_PTF fixture paths');
const stock = await readFile(process.env.STOCK_PTF), expected = await readFile(process.env.CT2_PTF);
const combinedExpected = process.env.CT2_WR2_PTF ? await readFile(process.env.CT2_WR2_PTF) : null;
const moduleName = process.env.PLAYWRIGHT_MODULE ? pathToFileURL(process.env.PLAYWRIGHT_MODULE).href : 'playwright';
const playwright = await import(moduleName);
const requests = [], virtualHttp = process.env.BROWSER_VIRTUAL_HTTP === '1';
const server = virtualHttp ? null : createServer(req => requests.push(req));
if (server) await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
const url = `http://127.0.0.1:${server?.address()?.port ?? 41739}/TrackerPlus-CT-Web/`;
const projectRoot = fileURLToPath(new URL('../', import.meta.url));
const contentTypes = { '.html': 'text/html; charset=utf-8', '.js': 'text/javascript', '.mjs': 'text/javascript', '.css': 'text/css', '.json': 'application/json', '.md': 'text/plain; charset=utf-8' };
async function routeLocalSite(route) {
  const request = route.request(), targetUrl = new URL(request.url());
  if (targetUrl.origin !== new URL(url).origin || !['GET', 'HEAD'].includes(request.method())) {
    await route.abort(); return;
  }
  requests.push({ method: request.method(), url: request.url(), contentLength: request.headers()['content-length'] ?? '0' });
  let name = decodeURIComponent(targetUrl.pathname).replace(/^\/TrackerPlus-CT-Web\/?/, '');
  if (!name || name.endsWith('/')) name += 'index.html';
  const target = resolve(projectRoot, '.' + (name.startsWith('/') ? name : `/${name}`));
  if (!target.startsWith(projectRoot.endsWith(sep) ? projectRoot : projectRoot + sep)) { await route.fulfill({ status: 404, body: 'Not found' }); return; }
  try {
    if (!(await stat(target)).isFile()) throw Error('not a file');
    const body = await readFile(target);
    await route.fulfill({ status: 200, contentType: contentTypes[extname(target)] ?? 'text/plain', body: request.method() === 'HEAD' ? undefined : body, headers: { 'cache-control': 'no-store', 'x-content-type-options': 'nosniff' } });
  } catch { await route.fulfill({ status: 404, body: 'Not found' }); }
}
const results = [];
try {
  for (const engine of (process.env.BROWSER_ENGINES ?? 'chromium,firefox,webkit').split(',')) {
    const args = engine === 'chromium' && process.env.CHROMIUM_LAUNCH_ARGS ? process.env.CHROMIUM_LAUNCH_ARGS.split(/\s+/).filter(Boolean) : [];
    const browser = await playwright[engine].launch({ headless: true, args });
    try {
      const context = await browser.newContext({ acceptDownloads: true, serviceWorkers: 'block' });
      if (virtualHttp) await context.route('**/*', routeLocalSite);
      const network = [], errors = [], consoleErrors = [], violations = [];
      context.on('request', req => network.push({ url: req.url(), method: req.method(), body: req.postData() }));
      const page = await context.newPage();
      page.on('pageerror', e => errors.push(e.message));
      page.on('console', m => { if (m.type() === 'error') consoleErrors.push(m.text()); });
      await page.addInitScript(() => { window.cspViolations = []; document.addEventListener('securitypolicyviolation', e => window.cspViolations.push(e.violatedDirective)); });
      await page.goto(url);
      await page.waitForFunction(() => !document.querySelector('#choose').disabled);
      assert.equal(await page.locator('#feature-ct2').isChecked(), true);
      assert.equal(await page.locator('#feature-ct2').isDisabled(), true, 'CT2 is mandatory');
      assert.equal(await page.locator('#feature-wr2').isChecked(), false);
      assert.equal(await page.locator('#feature-wr2').isDisabled(), false, 'WR2 is selectable in the production candidate');
      assert.match(await page.locator('label[for=feature-wr2]').textContent(), /Slice Weighted Random \(WR2\)/);
      const wr2Help = page.locator('#wr2-help');
      assert.equal(await wr2Help.textContent(), "In Beat Slice mode, group slices and assign each one a playback probability. Trigger the master slice to randomly play one of the group's slices according to its weight.");
      assert.equal(await wr2Help.evaluate(el => el.closest('label')), null, 'description stays outside the checkbox label');
      assert.equal(await page.locator('.feature-guide').first().textContent(), 'CT2 + WR2 Quick Start ↗');
      assert.equal(await page.locator('.feature-guide').first().getAttribute('href'), './quick-start.html');
      assert.equal(await page.locator('.feature-guide').nth(1).textContent(), 'Full Documentation ↗');
      assert.equal(await page.locator('.feature-guide').nth(1).getAttribute('href'), './documentation.html');
      await page.locator('.feature-guide').first().click();
      assert(page.url().endsWith('/quick-start.html'));
      assert.equal(await page.locator('main.docs > h1').textContent(), 'CT2 + WR2 Quick Start');
      assert(await page.locator('#ct2-quick-start').isVisible(), 'standalone quick start renders its CT2 section');
      assert(await page.locator('#wr2-quick-start').isVisible(), 'standalone quick start renders its WR2 section');
      assert.equal(await page.locator('main.docs nav a').first().getAttribute('href'), './');
      assert.equal(await page.locator('main.docs nav a').nth(1).getAttribute('href'), './documentation.html');
      assert.equal(await page.locator('main.docs > ol > li').count(), 9, 'WR2 instructions render as a numbered list');
      const quickStart = await page.locator('main.docs').innerText();
      for (const phrase of ['FX1', 'FX2', 'Chance', 'PR / !PR', 'NE / !NE', '1S / !1S', 'MORE', 'Page 3', 'Trig', 'Add', 'Weight', 'jog encoder', 'Remove', 'weighted random', 'global LED brightness', 'red/orange']) assert(quickStart.toLowerCase().includes(phrase.toLowerCase()), `missing quick-start content: ${phrase}`);
      for (const phrase of ['Playback probability', 'Play on pass A of a B-pass cycle', 'previous eligible Chance result on the same track', 'last conditional-note result on the previous track', 'First pass only / subsequent passes', 'Save: Save the project to preserve the groups', 'colours instead of brightness']) assert(quickStart.toLowerCase().includes(phrase.toLowerCase()), `missing owner-approved quick-start wording: ${phrase}`);
      assert.equal(await page.locator('main.docs table').count(), 2, 'quick start contains compact CT2 and LED tables');
      assert.doesNotMatch(quickStart, /wr2_groups\.wr2|entire SD card|for example|60%|history|PWM/);
      for (const width of [320, 375, 480, 600, 768, 1280]) {
        await page.setViewportSize({ width, height: 900 });
        assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), `quick-start overflow at ${width}`);
      }
      await page.setViewportSize({ width: 1280, height: 900 });
      await page.goto(url);
      await page.locator('.feature-guide').nth(1).click();
      assert(page.url().endsWith('/documentation.html'));
      assert.equal(await page.locator('main.docs > h1').textContent(), 'Full Documentation');
      assert.equal(await page.locator('#ct2-quick-start a').getAttribute('href'), './quick-start.html#ct2-quick-start');
      assert.equal(await page.locator('#wr2-quick-start a').getAttribute('href'), './quick-start.html#wr2-quick-start');
      assert.equal(await page.locator('#wr2-user-guide a').getAttribute('href'), '#wr2-details');
      assert.equal(await page.locator('#ct2-quick-start table').count(), 0, 'full documentation does not duplicate quick-start tables');
      const fullGuide = await page.locator('#wr2').innerText();
      for (const phrase of ['MORE', 'Page 3', 'Trig', 'Add', 'Weight', 'jog encoder', 'Remove', '60%', 'red/orange', 'companion', 'entire SD card', 'maximum brightness', 'medium-high brightness', 'medium-low brightness', 'low brightness', 'pads with no corresponding slice']) assert(fullGuide.toLowerCase().includes(phrase.toLowerCase()), `missing full-guide content: ${phrase}`);
      assert.equal(await page.locator('#wr2 table').count(), 1, 'full documentation contains the detailed LED brightness table once');
      assert.doesNotMatch(fullGuide, /PWM|255|31\s*\/\s*18/);
      const retiredLabel = ['Full', 'WR2', 'User', 'Guide'].join(' ');
      assert.equal((await page.locator('main.docs').innerText()).includes(retiredLabel), false);
      await page.goto(url);
      await wr2Help.click();
      assert.equal(await page.locator('#feature-wr2').isChecked(), false, 'description click does not toggle WR2');
      assert.equal(await page.locator('#download').isDisabled(), true);
      assert.equal(await page.title(), 'Tracker+ Conditional Trigs + Slice Weighted Random');
      assert.equal(await page.locator('#title').textContent(), 'Tracker+ Conditional Trigs + Slice Weighted Random');
      assert.equal(await page.locator('.tagline').textContent(), 'Less repetition. More natural variation.');
      assert.equal(await page.locator('.hero-description').textContent(), 'Bring your patterns to life by varying which notes are played and which sample slices are heard. Create more organic, evolving performances.');
      assert.doesNotMatch(await page.locator('.hero').innerText(), /Build 1504|1\.2\.0-CT2/);
      assert.match(await page.locator('.version').textContent(), /v0\.2\.1-CT2/);
      assert.equal(await page.locator('.firmware-line').textContent(), 'Unofficial firmware · CT2 / CT2+WR2');
      assert.match(await page.locator('.download-note').textContent(), /TrackerPlus_CT2\.ptf/);
      assert.match(await page.locator('.warning').textContent(), /NE → PRE correction/);
      assert.match((await page.locator('#workflow').innerText()).replace(/\s+/g, ' '), /Select.*Verify.*Patch.*Download/);
      assert.match(await page.locator('.privacy-line').textContent(), /100% local.*No uploads/);
      assert.equal(await page.locator('.workflow [aria-current="step"]').getAttribute('data-step'), '1');
      assert.equal(await page.locator('.condition-help').evaluate(el => el.open), false);
      await page.locator('.condition-help summary').focus();
      assert(await page.evaluate(() => parseFloat(getComputedStyle(document.activeElement).outlineWidth) >= 2), 'disclosure keyboard focus outline is visible');
      await page.keyboard.press('Enter');
      assert.equal(await page.locator('.condition-help').evaluate(el => el.open), true);
      const conditionHelp = await page.locator('.condition-help').innerText();
      for (const phrase of ['1:2–8:8', 'PR / !PR', 'NE / !NE', '1S / !1S', 'repeating cycle', 'same track', 'neighbouring track to the left', 'first pass', 'all later passes']) assert(conditionHelp.includes(phrase), `missing condition description: ${phrase}`);
      assert.equal(await page.locator('.condition-help a').getAttribute('href'), './documentation.html#conditions');
      await page.keyboard.press('Enter');
      assert.equal(await page.locator('.condition-help').evaluate(el => el.open), false);
      assert.equal((await page.locator('footer').innerText()).trim(), 'Psygoo · Independent firmware mod');
      assert.doesNotMatch(await page.locator('footer').innerText(), /Build 1504|1\.2\.0-CT2/);
      assert.equal(await page.locator('footer a').getAttribute('href'), 'https://github.com/Psygoo/TrackerPlus-CT-Web');
      for (let i = 0; i < 8; i++) {
        await page.keyboard.press('Tab');
        if (await page.evaluate(() => document.activeElement?.id === 'choose')) break;
      }
      assert.equal(await page.evaluate(() => document.activeElement?.id), 'choose');
      assert(await page.evaluate(() => parseFloat(getComputedStyle(document.activeElement).outlineWidth) >= 2), 'keyboard focus outline is visible');
      await page.keyboard.press('Tab');
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
          await download.saveAs(`${process.env.EVIDENCE_DIR}/${engine}-generated-CT2.ptf`);
        }
      };
      // Normal file chooser, actual module worker, output hash gate and Blob save.
      await page.locator('#firmware').setInputFiles({ name: 'TrackerPlus_1.2.0.ptf', mimeType: 'application/octet-stream', buffer: stock });
      await page.waitForFunction(() => document.body.dataset.state === 'patching');
      assert.match(await page.locator('.download-note').textContent(), /Filename: TrackerPlus_CT2\.ptf/, 'CT2 filename remains correct during processing');
      await page.waitForFunction(() => document.body.dataset.state === 'success', { }, { timeout: 90000 });
      assert.match(await page.locator('.download-note').textContent(), /Filename: TrackerPlus_CT2\.ptf/, 'CT2 filename remains correct after generation');
      assert.equal(await page.locator('.workflow [aria-current="step"]').getAttribute('data-step'), '4');
      assert.equal(await page.locator('#download').isDisabled(), false);
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
      if (virtualHttp) await context.route('**/*', routeLocalSite);
      for (const width of [320, 375, 480, 600, 768, 1280]) {
        await page.setViewportSize({ width, height: 900 });
        assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), `horizontal overflow at ${width}`);
        const headingBox = await page.locator('#title').boundingBox();
        assert(headingBox.x >= 0 && headingBox.x + headingBox.width <= width, `hero title fits viewport at ${width}`);
        if (process.env.EVIDENCE_DIR) {
          await mkdir(process.env.EVIDENCE_DIR, { recursive: true });
          await page.screenshot({ path: `${process.env.EVIDENCE_DIR}/${engine}-${width}.png`, fullPage: true });
        }
      }
      await page.goto(url + 'documentation.html');
      assert(await page.locator('#compatibility').count());
      assert(await page.locator('#ct2-quick-start').count());
      assert(await page.locator('#wr2-quick-start').count());
      assert.match(await page.locator('#ct2-quick-start').innerText(), /standalone page/);
      assert.match(await page.locator('#wr2-quick-start').innerText(), /Quick Start section/);
      assert(await page.locator('#wr2-user-guide').count(), 'old detailed-section anchor remains available');
      assert(await page.locator('#credits').count());
      assert.match(await page.locator('#credits').textContent(), /Psygoo/);
      assert.match(await page.locator('#credits').textContent(), /FatRodzianko/);
      assert.match(await page.locator('#credits').textContent(), /MIT/);
      assert.match(await page.locator('#conditions').textContent(), /NE\/!NE now also produce PRE/);
      assert.match(await page.locator('#compatibility').textContent(), /superseded/);
      for (const width of [320, 1280]) {
        await page.setViewportSize({ width, height: 900 });
        assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), `documentation overflow at ${width}`);
      }
      assert.deepEqual(errors, []);
      assert(consoleErrors.every(s => /connect-src|Content Security Policy|Content-Security-Policy|NetworkError|Load failed|Fetch API|Failed to fetch/i.test(s)), consoleErrors.join('\n'));
      assert(network.every(req => req.url.startsWith(url) && req.method === 'GET' && req.body === null));
      let combined = 'not run';
      if (combinedExpected) {
        await page.goto(url);
        await page.waitForFunction(() => !document.querySelector('#choose').disabled);
        assert.equal(await page.locator('#feature-wr2').isDisabled(), false, 'WR2 stays selectable without a development flag');
        await page.locator('#feature-wr2').check();
        assert.match(await page.locator('.download-note').textContent(), /Filename: TrackerPlus_CT2_WR2\.ptf/, 'WR2 filename updates before firmware selection');
        const combinedLoaded = network.length;
        await page.locator('#firmware').setInputFiles({ name: 'TrackerPlus_1.2.0.ptf', mimeType: 'application/octet-stream', buffer: stock });
        await page.waitForFunction(() => document.body.dataset.state === 'patching');
        assert.match(await page.locator('.download-note').textContent(), /Filename: TrackerPlus_CT2_WR2\.ptf/, 'WR2 filename remains correct during processing');
        await page.waitForFunction(() => document.body.dataset.state === 'success', { }, { timeout: 90000 });
        assert.match(await page.locator('.download-note').textContent(), /Filename: TrackerPlus_CT2_WR2\.ptf/, 'WR2 filename remains correct after generation');
        assert.equal(network.length, combinedLoaded, 'combined generation made no network requests');
        assert.equal(await page.locator('#output-hash').textContent(), '0bdc7dfc32f5c98ebf8a3146811a87dc9fbf8a60a324ad9edc1a64cea7609c1a');
        const event = page.waitForEvent('download'); await page.locator('#download').click();
        const download = await event; assert.equal(download.suggestedFilename(), 'TrackerPlus_CT2_WR2.ptf');
        const bytes = await readFile(await download.path());
        assert.equal(createHash('sha256').update(bytes).digest('hex'), '0bdc7dfc32f5c98ebf8a3146811a87dc9fbf8a60a324ad9edc1a64cea7609c1a');
        assert(bytes.equals(combinedExpected));
        await page.locator('#feature-wr2').uncheck();
        assert.match(await page.locator('.download-note').textContent(), /Filename: TrackerPlus_CT2\.ptf/, 'unchecking WR2 immediately restores the CT2 filename');
        assert.equal(await page.locator('#download').isDisabled(), true, 'selection change invalidates a previous result');
        assert.equal(await page.locator('#hash').isHidden(), true);
        combined = 'exact production-candidate output and selection invalidation pass';
      }
      results.push({ engine, version: browser.version(), exact_ptf: true, repeated_downloads: 2, combined_production_candidate_output: combined, processing_network_requests: 0, csp_upload_blocked: true, invalid_input_and_stale_output: 'pass', widths: [320, 375, 480, 600, 768, 1280], keyboard_focus: 'visible', errors });
    } finally { await browser.close(); }
  }
  assert(requests.every(req => ['GET', 'HEAD'].includes(req.method) && req.contentLength === '0'));
  console.log(JSON.stringify({ pass: true, output_sha256: OUTPUT_SHA256, server_received_firmware: false, results }, null, 2));
} finally { if (server) await new Promise(resolve => server.close(resolve)); }
