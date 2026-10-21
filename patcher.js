// MIT. Fixed CT6 port of tools/ptf.py, compile_inline and the verified CLI.
import { buildProgram, symbols, windows } from './ct6-program.js';

export const STOCK_SHA256 = 'd4616304f33d6d0b56516c658f34fa22517c1dd1630021c1ca89afca7b618ccd';
export const IMAGE_SHA256 = '0d924ba9352ea0ce24672ee252b44b760bb949a8d8faf3bac2d39594962e1f3b';
export const OUTPUT_SHA256 = 'dc7fe730fdac0dda92401ef623463e83b30624f886895129e882a045f61ed24e';
export const PROGRAM_SHA256 = '54f5c8cdec14a0293a58caac6a2ea6ec30cdc7be3a9910b2e27540f11d99e97f';
export const BASE = 0x60030000, CODE = 0x601bab80, PHASE = 0x20205000;
export const OUTPUT_NAME = 'TrackerPlus_1.2.0_CT.ptf';
export const MAX_FILE_SIZE = 8 * 1024 * 1024;
const encoder = new TextEncoder();
export function require(condition, message) { if (!condition) throw new Error(message); }
export const equal = (a, b) => a.length === b.length && a.every((value, i) => value === b[i]);
export async function sha256(bytes) {
  require(globalThis.crypto?.subtle, 'Web Crypto is unavailable. Open this page on localhost or HTTPS.');
  return Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256', bytes)), n => n.toString(16).padStart(2, '0')).join('');
}
function view(bytes) { return new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength); }
function offset(image, base, at, length) {
  require(Number.isInteger(at) && Number.isInteger(length) && length > 0 && at >= base && at + length <= base + image.length, 'Stock interval outside image');
  return at - base;
}
function address(value) {
  require(Number.isInteger(value) && value >= 0 && value <= 0xffffffff, 'Invalid 32-bit address');
  return value;
}

// Preserve record boundaries and CRLF serialization. Structural checks are stricter
// than the generic Python reader; supported stock must pass the full file hash first.
export function parsePTF(bytes) {
  const text = new TextDecoder('utf-8', { fatal: true }).decode(bytes);
  require(text.endsWith('\r\n') && !text.replaceAll('\r\n', '').includes('\n'), 'Invalid PTF line endings');
  let upper = 0, start = null, ended = false, top = null, low = null;
  const records = [];
  for (const line of text.slice(0, -2).split('\r\n')) {
    require(!ended && /^:(?:[0-9a-fA-F]{2}){5,}$/.test(line), 'Invalid PTF record');
    const raw = Uint8Array.from(line.slice(1).match(/../g), n => parseInt(n, 16));
    require(raw.length === raw[0] + 5 && raw.reduce((sum, n) => sum + n, 0) % 256 === 0, 'PTF checksum or length mismatch');
    const kind = raw[3], at = raw[1] * 256 + raw[2], data = raw.slice(4, -1);
    let recordAt = at;
    if (kind === 4) {
      require(at === 0 && data.length === 2, 'Invalid upper address record');
      upper = (data[0] * 256 + data[1]) * 65536;
    } else if (kind === 0) {
      require(data.length > 0 && at + data.length <= 65536, 'Invalid data record');
      recordAt = upper + at;
      require(top === null || recordAt === top, 'PTF data is not contiguous or overlaps');
      if (low === null) low = recordAt;
      top = recordAt + data.length;
    } else if (kind === 5) {
      require(at === 0 && data.length === 4 && start === null, 'Invalid or duplicate entry record');
      start = new DataView(data.buffer).getUint32(0);
    } else if (kind === 1) {
      require(at === 0 && data.length === 0, 'Invalid EOF');
      ended = true;
    } else throw new Error('Unsupported PTF record type');
    records.push({ kind, at: recordAt, data });
  }
  require(ended && low !== null && start !== null && top - low <= MAX_FILE_SIZE, 'Incomplete PTF');
  const image = new Uint8Array(top - low);
  for (const r of records) if (r.kind === 0) image.set(r.data, r.at - low);
  return { records, base: low, image, start };
}
function line(kind, at, data) {
  const raw = [data.length, (at >> 8) & 255, at & 255, kind, ...data];
  raw.push((-raw.reduce((sum, n) => sum + n, 0)) & 255);
  return ':' + raw.map(n => n.toString(16).padStart(2, '0').toUpperCase()).join('');
}
export function encodePTF(ptf) {
  return encoder.encode(ptf.records.map(r => line(r.kind, r.at, r.kind === 0 ? ptf.image.subarray(r.at - ptf.base, r.at - ptf.base + r.data.length) : r.data)).join('\r\n') + '\r\n');
}
export function copiesFor(image, base) {
  const at = offset(image, base, 0x600302a0, 48), copies = [], v = view(image);
  for (let i = at; i < at + 48; i += 12) {
    const load = v.getUint32(i, true), exe = v.getUint32(i + 4, true), size = v.getUint32(i + 8, true);
    if (size && exe < 0x20000) {
      require(((load | exe | size) & 3) === 0 && exe + size <= 0x20000, 'Invalid ITCM copy descriptor');
      offset(image, base, load, size);
      for (const c of copies) require(!(exe < c.exe + c.size && c.exe < exe + size) && !(load < c.load + c.size && c.load < load + size), 'Ambiguous ITCM copy descriptors');
      copies.push({ load, exe, size });
    }
  }
  require(copies.length > 0, 'No stock ITCM copy descriptor');
  return copies;
}
function mapExecution(image, base, exe, size, copies) {
  if (exe < 0x20000) {
    const hits = copies.filter(c => c.exe <= exe && exe + size <= c.exe + c.size);
    require(hits.length === 1, 'Execution window is not wholly stock-copied ITCM');
    return [hits[0].load + exe - hits[0].exe, hits[0].exe];
  }
  offset(image, base, exe, size);
  require(!copies.some(c => exe < c.load + c.size && c.load < exe + size), 'Copy-source alias must use its ITCM execution address');
  return [exe, base];
}
function thumbWidth(data, at) {
  require(at >= 0 && at + 2 <= data.length, 'Partial Thumb halfword');
  const prefix = view(data).getUint16(at, true) & 0xf800;
  const width = [0xe800, 0xf000, 0xf800].includes(prefix) ? 4 : 2;
  require(at + width <= data.length, 'Window cuts a 32-bit Thumb instruction');
  return width;
}
function itCount(hw) {
  let mask = hw & 15;
  if ((hw & 0xff00) !== 0xbf00 || mask === 0) return 0;
  require(((hw >> 4) & 15) < 14, 'Unsupported IT condition');
  let trailing = 0;
  while (!(mask & 1)) { trailing++; mask >>= 1; }
  return 4 - trailing;
}
export function validateThumbWindow(data) {
  let remaining = 0;
  for (let at = 0; at < data.length;) {
    const width = thumbWidth(data, at), count = width === 2 ? itCount(view(data).getUint16(at, true)) : 0;
    if (count) { require(!remaining, 'Nested IT block'); remaining = count; }
    else if (remaining) remaining--;
    at += width;
  }
  require(!remaining, 'Window cuts the original IT block');
}
function rejectITEntry(image, base, exe, regionStart, copies) {
  for (let candidate = Math.max(regionStart, exe - 18); candidate < exe; candidate += 2) {
    const [at] = mapExecution(image, base, candidate, 2, copies);
    const count = itCount(view(image).getUint16(at - base, true));
    let cursor = candidate + 2;
    for (let i = 0; i < count; i++) {
      const [write] = mapExecution(image, base, cursor, 2, copies), width = thumbWidth(image, write - base);
      require(!(cursor <= exe && exe < cursor + width), 'Inline entry may be inside an original IT block');
      cursor += width;
      if (cursor > exe) break;
    }
  }
}
export function thumbBW(at, target) {
  address(at); address(target);
  require(!(at & 1) && (target & 1), 'B.W needs an even site and Thumb target');
  const off = (target - 1) - (at + 4);
  require(off >= -(2 ** 24) && off < 2 ** 24, 'B.W target out of range');
  const s = (off >> 24) & 1, i1 = (off >> 23) & 1, i2 = (off >> 22) & 1;
  const data = new Uint8Array(4), v = view(data);
  v.setUint16(0, 0xf000 | (s << 10) | ((off >> 12) & 1023), true);
  v.setUint16(2, 0x9000 | (((~(i1 ^ s)) & 1) << 13) | (((~(i2 ^ s)) & 1) << 11) | ((off >> 1) & 2047), true);
  return data;
}
export async function compileHook(image, base, hook, program, syms, copies) {
  const { write_at: at, exec_at: exe, resume_at: resume, length, mode, target: name } = hook;
  [at, exe, resume].forEach(address);
  require(Number.isInteger(length) && length >= 4 && !(length & 1) && !((at | exe | resume) & 1), 'Invalid inline length or alignment');
  require(exe + length === resume && resume <= 0xffffffff, 'Invalid resume address');
  require(mode === 'thumb_bw' || (mode === 'thumb_abs8' && !(exe & 3) && [8, 10].includes(length)), 'Unsupported inline mode or length');
  const off = offset(image, base, at, length), original = image.subarray(off, off + length);
  require(await sha256(original) === hook.expected_sha256, 'Stock hook fingerprint mismatch: ' + name);
  const [mapped, regionStart] = mapExecution(image, base, exe, length, copies);
  require(mapped === at, 'Write address does not match stock execution mapping');
  mapExecution(image, base, resume, 2, copies);
  validateThumbWindow(original);
  rejectITEntry(image, base, exe, regionStart, copies);
  require(Object.hasOwn(syms, name), 'Missing inline target symbol');
  const target = address(syms[name]);
  require((target & 1) && CODE <= target - 1 && target - 1 <= CODE + program.length - 2, 'Inline target outside placed patch code');
  const data = new Uint8Array(length);
  for (let i = 0; i < length; i += 2) view(data).setUint16(i, 0xbf00, true);
  if (mode === 'thumb_abs8') {
    view(data).setUint16(0, 0xf8df, true); view(data).setUint16(2, 0xf000, true); view(data).setUint32(4, target, true);
  } else data.set(thumbBW(exe, target));
  return { at, data };
}

export async function patchFirmware(input, onState = () => {}) {
  onState('validating');
  // Take a private snapshot so callers cannot mutate bytes during async hashing.
  const bytes = new Uint8Array(input).slice();
  require(bytes.length > 0 && bytes.length <= MAX_FILE_SIZE, 'Unsupported firmware file size');
  require(await sha256(bytes) === STOCK_SHA256, 'Unsupported firmware. Select the exact original Tracker+ 1.2.0 PTF.');
  const stock = parsePTF(bytes);
  require(stock.base === BASE && await sha256(stock.image) === IMAGE_SHA256, 'Unsupported stock application image');
  require(equal(encodePTF(stock), bytes), 'Stock PTF round trip mismatch');
  const program = buildProgram();
  require(program.length === 1100 && await sha256(program) === PROGRAM_SHA256, 'CT6 patch program verification failed');
  require(BASE + stock.image.length === CODE - 8 && CODE % 16 === 0, 'Fixed CT6 placement changed');
  // The original allocator selects the first 16-aligned four-byte startup block
  // in ocram_tail (0x20205000..0x20208000); no relocation is possible here.
  require(PHASE === 0x20205000 && PHASE % 16 === 0 && PHASE + 4 <= 0x20208000, 'CT6 RAM placement changed');
  const copies = copiesFor(stock.image, BASE), plans = [];
  require(windows.length === 21, 'CT6 hook count changed');
  for (const hook of windows) {
    const plan = await compileHook(stock.image, BASE, hook, program, symbols, copies);
    require(!plans.some(p => p.at < plan.at + plan.data.length && plan.at < p.at + p.data.length), 'Overlapping hook windows');
    plans.push(plan);
  }
  onState('patching');
  const image = new Uint8Array(stock.image.length + 8 + program.length);
  image.set(stock.image);
  for (const p of plans) image.set(p.data, p.at - BASE);
  image.fill(0xff, stock.image.length, stock.image.length + 8);
  image.set(program, CODE - BASE);
  const ending = stock.records.filter(r => r.kind === 5 || r.kind === 1);
  const records = stock.records.filter(r => r.kind !== 5 && r.kind !== 1);
  let upper = null;
  for (const r of records) if (r.kind === 4) upper = r.data[0] * 256 + r.data[1];
  // Python appends 16-byte records starting at stock_end, including the FF gap.
  for (let off = stock.image.length; off < image.length; off += 16) {
    const at = BASE + off, nextUpper = Math.floor(at / 65536);
    if (nextUpper !== upper) {
      upper = nextUpper;
      records.push({ kind: 4, at: 0, data: Uint8Array.of(upper >> 8, upper & 255) });
    }
    records.push({ kind: 0, at, data: image.slice(off, off + 16) });
  }
  const output = encodePTF({ ...stock, records: [...records, ...ending], image });
  require(await sha256(output) === OUTPUT_SHA256, 'CT6 output verification failed. No download is available.');
  const check = parsePTF(output);
  require(check.base === BASE && check.start === stock.start && equal(check.image, image) && equal(encodePTF(check), output), 'CT6 output structure verification failed');
  return output;
}
