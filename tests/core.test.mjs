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
  const session = new FirmwareSession(patchFirmware, () => {});
  const file = bytes => ({ name: 'input.ptf', size: bytes.length, arrayBuffer: async () => bytes.slice().buffer });
  await session.select([file(stock)]); assert.equal(session.state, 'success'); assert.deepEqual(session.output, reference);
  await session.select([file(damaged)]); assert.equal(session.state, 'error'); assert.equal(session.output, null);
});
