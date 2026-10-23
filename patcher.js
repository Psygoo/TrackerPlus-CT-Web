// MIT. PTF reader/encoder reused from TrackerPlus-CT-Web CT6; H12 source recipe.
import { buildProgram, recipe } from './h12-program.js';
export const STOCK_SHA256 = 'd4616304f33d6d0b56516c658f34fa22517c1dd1630021c1ca89afca7b618ccd';
export const IMAGE_SHA256 = '0d924ba9352ea0ce24672ee252b44b760bb949a8d8faf3bac2d39594962e1f3b';
export const OUTPUT_SHA256 = '40079d13a100c8c73953496c9bdebec034c6b86a1e99c2706e717fc8e0b18c79';
export const OUTPUT_NAME = 'TrackerPlus_H12.ptf';
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
export async function patchFirmware(input, onState = () => {}) {
  onState('validating');
  require(input instanceof Uint8Array || input instanceof ArrayBuffer, 'Select an original PTF file.');
  require(input.byteLength > 0 && input.byteLength <= MAX_FILE_SIZE, 'Unsupported firmware file size.');
  const bytes = new Uint8Array(input).slice();
  require(await sha256(bytes) === STOCK_SHA256, 'Unsupported firmware. Select the exact original Tracker+ 1.2.0 PTF.');
  const stock = parsePTF(bytes);
  require(stock.base === recipe.base && stock.start === 0x60030319 && stock.base + stock.image.length === recipe.start, 'Unsupported stock layout.');
  require(await sha256(stock.image) === IMAGE_SHA256 && equal(encodePTF(stock), bytes), 'Stock application verification failed.');
  const program = buildProgram();
  require(program.length === recipe.end - recipe.start && await sha256(program) === recipe.template_sha256, 'H12 project code verification failed.');
  const plans = [];
  for (const p of recipe.plans) {
    const replacement = Uint8Array.from(p.hex.match(/../g), s => parseInt(s, 16));
    const i = offset(stock.image, stock.base, p.at, replacement.length);
    require(await sha256(stock.image.subarray(i, i + replacement.length)) === p.sha256, 'Stock hook verification failed.');
    require(!plans.some(old => old.at < p.at + replacement.length && p.at < old.at + old.data.length), 'Overlapping patch windows.');
    plans.push({ at: p.at, data: replacement });
  }
  for (const c of recipe.copies) {
    const i = offset(stock.image, stock.base, c.from, c.length);
    require(c.to >= recipe.start && c.to + c.length <= recipe.end, 'Replay destination outside patch code.');
    require(program.subarray(c.to - recipe.start, c.to - recipe.start + c.length).every(n => n === 0), 'Replay slot is not empty.');
    require(await sha256(stock.image.subarray(i, i + c.length)) === c.sha256, 'Input replay verification failed.');
  }
  onState('patching');
  const image = new Uint8Array(recipe.end - stock.base);
  image.set(stock.image); image.set(program, recipe.start - stock.base);
  // Original instruction bytes come only from the private, verified input.
  for (const c of recipe.copies) image.set(stock.image.subarray(c.from - stock.base, c.from - stock.base + c.length), c.to - stock.base);
  for (const p of plans) image.set(p.data, p.at - stock.base);
  const records = stock.records.filter(r => r.kind !== 1 && r.kind !== 5);
  const ending = stock.records.filter(r => r.kind === 1 || r.kind === 5);
  let upper = [...records].reverse().find(r => r.kind === 4).data;
  upper = upper[0] * 256 + upper[1];
  let at = recipe.start;
  // The H12 candidate retains incremental record boundaries, including short
  // records at earlier stage ends. They are part of its exact file identity.
  for (const end of recipe.record_ends) {
    require(end > at && end <= recipe.end, 'Invalid record boundary.');
    while (at < end) {
      const nextUpper = Math.floor(at / 65536);
      if (nextUpper !== upper) {
        upper = nextUpper; records.push({ kind: 4, at: 0, data: Uint8Array.of(upper >> 8, upper & 255) });
      }
      const n = Math.min(16 - ((at - recipe.start) % 16), end - at, 65536 - (at & 65535));
      records.push({ kind: 0, at, data: image.slice(at - stock.base, at - stock.base + n) }); at += n;
    }
  }
  require(at === recipe.end, 'Incomplete patch coverage.');
  const output = encodePTF({ ...stock, records: [...records, ...ending], image });
  require(await sha256(output) === OUTPUT_SHA256, 'H12 output verification failed. No download is available.');
  const check = parsePTF(output);
  require(check.start === stock.start && check.base === stock.base && equal(check.image, image) && equal(encodePTF(check), output), 'H12 PTF integrity verification failed.');
  return output;
}
