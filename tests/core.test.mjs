// MIT. Private firmware fixtures are opt-in; no binaries belong in this tree.
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { patchFirmware, parsePTF, encodePTF, sha256, STOCK_SHA256, OUTPUT_SHA256, MAX_FILE_SIZE } from '../patcher.js';
import { buildProgram, recipe } from '../ct2-program.js';
import { FirmwareSession } from '../session.js';

test('project template and input-only replay slots', async () => {
  const bytes = buildProgram();
  assert.equal(bytes.length, recipe.end - recipe.start);
  assert.equal(await sha256(bytes), recipe.template_sha256);
  assert.equal(recipe.plans.length, 29);
  assert.equal(recipe.copies.reduce((n, x) => n + x.length, 0), 14);
  for (const c of recipe.copies) assert(bytes.slice(c.to - recipe.start, c.to - recipe.start + c.length).every(n => n === 0));
});
test('strict synthetic PTF parser, checksum and round trip', () => {
  // Synthetic data only, deliberately unrelated to the supported firmware.
  const bytes = new TextEncoder().encode(':0200000460009A\r\n:0400000001020304F2\r\n:040000056000000196\r\n:00000001FF\r\n');
  const ptf = parsePTF(bytes);
  assert.equal(ptf.base, 0x60000000); assert.equal(ptf.start, 0x60000001);
  assert.deepEqual(encodePTF(ptf), bytes);
  for (const bad of [new Uint8Array(), bytes.slice(0, -1), new TextEncoder().encode(new TextDecoder().decode(bytes).replace('F2', 'F3'))]) assert.throws(() => parsePTF(bad));
});
test('unsupported types, empty, oversized, synthetic and damaged inputs reject', async () => {
  for (const bad of [null, 'firmware.ptf', new Uint8Array(), new Uint8Array(MAX_FILE_SIZE + 1), new Uint8Array([1, 2, 3])]) await assert.rejects(patchFirmware(bad));
});
test('UI rejects invalid selections without running patcher', async () => {
  let calls = 0;
  const session = new FirmwareSession(() => { calls++; }, () => {});
  const file = (name, size) => ({ name, size, arrayBuffer: async () => new ArrayBuffer(1) });
  for (const files of [[], [file('x.bin', 1)], [file('x.ptf', 0)], [file('x.ptf', MAX_FILE_SIZE + 1)], [file('a.ptf', 1), file('b.ptf', 1)]]) {
    await session.select(files); assert.equal(session.state, 'error'); assert.equal(session.output, null);
  }
  assert.equal(calls, 0);
});
test('UI fails closed on worker faults or unverified output and blocks concurrent jobs', async () => {
  let release;
  const session = new FirmwareSession(() => new Promise(resolve => { release = resolve; }), () => {});
  const file = { name: 'original.ptf', size: 1, arrayBuffer: async () => new ArrayBuffer(1) };
  const job = session.select([file]);
  await new Promise(resolve => setImmediate(resolve));
  await session.select([file]); assert.equal(session.busy, true);
  release(new Uint8Array([1])); await job;
  assert.equal(session.state, 'error'); assert.equal(session.output, null); assert.equal(session.busy, false);
  session.run = async () => { throw Error('worker failed'); };
  await session.select([file]); assert.equal(session.state, 'error');
});
test('private stock → patcher → exact CT2 reference from hardware-tested H13, repeatability, rejection and stale output', { skip: !(process.env.STOCK_PTF && process.env.CT2_PTF) }, async () => {
  const stock = new Uint8Array(await readFile(process.env.STOCK_PTF));
  const reference = new Uint8Array(await readFile(process.env.CT2_PTF));
  assert.equal(await sha256(stock), STOCK_SHA256); assert.equal(await sha256(reference), OUTPUT_SHA256);
  const original = stock.slice(), states = [];
  const output = await patchFirmware(stock, state => states.push(state));
  assert.deepEqual(output, reference); assert.deepEqual(stock, original);
  assert.deepEqual(states, ['validating', 'patching']);
  assert.deepEqual(await patchFirmware(stock.buffer), reference);
  const damaged = stock.slice(); damaged[0] ^= 1;
  const bad = [damaged, stock.slice(0, -2), reference, new TextEncoder().encode(new TextDecoder().decode(stock).replaceAll('\r\n', '\n'))];
  for (const path of (process.env.UNSUPPORTED_PTFS ?? '').split(':').filter(Boolean)) bad.push(new Uint8Array(await readFile(path)));
  for (const bytes of bad) await assert.rejects(patchFirmware(bytes), /Unsupported firmware/);
  // Selection snapshot: caller mutation after invocation must not change the job.
  const mutable = stock.slice(), pending = patchFirmware(mutable); mutable.fill(0);
  assert.deepEqual(await pending, reference);
  const session = new FirmwareSession((input, selection, job, state) => patchFirmware(input, state), () => {});
  const file = bytes => ({ name: 'input.ptf', size: bytes.length, arrayBuffer: async () => bytes.slice().buffer });
  await session.select([file(stock)]); assert.equal(session.state, 'success'); assert.deepEqual(session.output, reference);
  await session.select([file(damaged)]); assert.equal(session.state, 'error'); assert.equal(session.output, null);
});

test('feature output names are CT2-mandatory and WR2-only is rejected', async () => {
  const { outputFor, FirmwareSession } = await import('../session.js');
  assert.deepEqual(outputFor({ ct2: true, wr2: false }), {
    name: 'TrackerPlus_CT2.ptf', sha256: OUTPUT_SHA256,
  });
  assert.deepEqual(outputFor({ ct2: true, wr2: true }), {
    name: 'TrackerPlus_CT2_WR2.ptf', sha256: '06adca54ff28ae704e92c4464a975f3c0e34f16d67a177db53157895be199445',
  });
  assert.throws(() => outputFor({ ct2: false, wr2: true }));
  const session = new FirmwareSession(async () => new Uint8Array(), () => {});
  assert.throws(() => session.setSelection({ ct2: false, wr2: true }));
  assert.throws(() => session.setSelection({ ct2: true, wr2: true }));
});
test('selection changes cancel and invalidate an in-flight worker job', async () => {
  const { FirmwareSession } = await import('../session.js');
  let resolve, cancelled = 0, updates = [];
  const session = new FirmwareSession((input, selection, job, state, registerCancel) => new Promise(r => {
    resolve = r; registerCancel(() => { cancelled++; });
  }), state => updates.push(state));
  const file = { name: 'stock.ptf', size: 1, arrayBuffer: async () => new ArrayBuffer(1) };
  const running = session.select([file]);
  await new Promise(r => setImmediate(r));
  session.setSelection({ ct2: true, wr2: false });
  resolve(new Uint8Array([1])); await running;
  assert.equal(cancelled, 1); assert.equal(session.output, null); assert.equal(session.busy, false);
  assert.equal(session.state, 'waiting');
});

test('direct stock JavaScript engines reproduce exact CT2 and S12C references', { skip: !(process.env.STOCK_PTF && process.env.CT2_PTF && process.env.CT2_WR2_PTF) }, async () => {
  const { patchCombinedFirmware } = await import('../wr2-patcher.js');
  const stock = new Uint8Array(await readFile(process.env.STOCK_PTF));
  const ct2 = new Uint8Array(await readFile(process.env.CT2_PTF));
  const combined = new Uint8Array(await readFile(process.env.CT2_WR2_PTF));
  const before = stock.slice();
  const ct2Output = await patchFirmware(stock);
  const wr2Output = await patchCombinedFirmware(stock);
  assert.deepEqual(ct2Output, ct2);
  assert.deepEqual(wr2Output, combined);
  assert.deepEqual(stock, before);
  assert.equal(await sha256(ct2Output), 'baa93d46208c0f6ab2ca0c7507fb3c430703a3ceada582a780779844a0d03b20');
  assert.equal(await sha256(wr2Output), '06adca54ff28ae704e92c4464a975f3c0e34f16d67a177db53157895be199445');
  assert.deepEqual(parsePTF(ct2Output).image, parsePTF(ct2).image);
  assert.deepEqual(encodePTF(parsePTF(wr2Output)), wr2Output);
  assert.deepEqual(parsePTF(wr2Output).image, parsePTF(combined).image);
  const { runSelectedFirmware } = await import('../worker-core.js');
  assert.deepEqual(await runSelectedFirmware({ input: stock, selection: { ct2: true, wr2: false } }), ct2);
  assert.deepEqual(await runSelectedFirmware({ input: stock, selection: { ct2: true, wr2: true, devWR2: true } }), combined);
});
test('combined engine rejects wrong stock before applying any WR2 operations', async () => {
  const { patchCombinedFirmware } = await import('../wr2-patcher.js');
  await assert.rejects(patchCombinedFirmware(new Uint8Array([1,2,3])), /Unsupported original/);
});

test('worker dispatch enforces release gate and rejects WR2-only selection', async () => {
  const { runSelectedFirmware } = await import('../worker-core.js');
  await assert.rejects(runSelectedFirmware({ input: new Uint8Array(), selection: { ct2: false, wr2: true, devWR2: true } }), /CT2 is required/);
  await assert.rejects(runSelectedFirmware({ input: new Uint8Array(), selection: { ct2: true, wr2: true } }), /disabled in this release/);
  await assert.rejects(runSelectedFirmware({ input: new Uint8Array(), selection: { ct2: true, wr2: true, devWR2: true } }), /Unsupported original/);
});

test('module worker suppresses a cancelled stale reply and reports the current job', async () => {
  const messages = [];
  globalThis.self = { postMessage: message => messages.push(message) };
  await import(`../worker.js?worker-message-test=${Date.now()}`);
  const handler = globalThis.self.onmessage;
  assert.deepEqual(messages.shift(), { ready: true });
  const stale = handler({ data: { jobId: 10, selection: { ct2: true, wr2: false }, input: new Uint8Array([1]) } });
  await handler({ data: { type: 'cancel', jobId: 11 } });
  await stale;
  assert(messages.some(message => message.jobId === 10 && message.state === 'validating'));
  assert.equal(messages.some(message => message.jobId === 10 && (message.output || message.error)), false);
  await handler({ data: { jobId: 12, selection: { ct2: true, wr2: false }, input: new Uint8Array([1]) } });
  assert(messages.some(message => message.jobId === 12 && /Unsupported firmware/.test(message.error ?? '')));
  delete globalThis.self;
});

test('WR2 guards detect a changed complete stock window', async () => {
  const { verifyWR2StockGuards } = await import('../wr2-patcher.js');
  const { parsePTF } = await import('../patcher.js');
  if (!process.env.STOCK_PTF) return;
  const input = new Uint8Array(await readFile(process.env.STOCK_PTF));
  const stock = parsePTF(input); const address = 0x600f3f2a;
  stock.image[address-stock.base] ^= 1;
  await assert.rejects(verifyWR2StockGuards(stock), /Stock patch window mismatch at 0x600f3f2a/);
});
