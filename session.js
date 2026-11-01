// MIT. One job at a time; a selection change invalidates every prior result.
import { MAX_FILE_SIZE, OUTPUT_SHA256, require, sha256 } from './patcher.js';
export const CT2 = 'ct2';
export const WR2 = 'wr2';
export function outputFor(selection) {
  require(selection?.ct2 === true, 'Conditional Trigs (CT2) is required.');
  return selection.wr2 ? { name: 'TrackerPlus_CT2_WR2.ptf', sha256: '0bdc7dfc32f5c98ebf8a3146811a87dc9fbf8a60a324ad9edc1a64cea7609c1a' }
    : { name: 'TrackerPlus_CT2.ptf', sha256: OUTPUT_SHA256 };
}
const same = (a,b) => a?.ct2 === b?.ct2 && a?.wr2 === b?.wr2;
export class FirmwareSession {
  constructor(run, update, { allowWR2 = false } = {}) {
    this.run = run; this.update = update; this.allowWR2 = allowWR2;
    this.busy = false; this.output = null; this.job = 0; this.selection = { ct2: true, wr2: false };
    this.emit('waiting', 'Waiting for original firmware');
  }
  emit(state, message) {
    this.state = state;
    this.update({ state, message, canDownload: state === 'success' && this.output !== null, jobId: this.job, selection: this.selection });
  }
  setSelection(selection) {
    require(selection?.ct2 === true, 'Conditional Trigs (CT2) is required.');
    require(!selection.wr2 || this.allowWR2, 'WR2 generation is disabled in this release.');
    this.job++;
    this.cancel?.(this.job);
    this.selection = { ct2: true, wr2: Boolean(selection.wr2) };
    this.output = null; this.busy = false;
    this.emit('waiting', 'Selection changed. Choose the original firmware again.');
  }
  async select(files) {
    if (this.busy) return;
    const job = ++this.job, selection = { ...this.selection }, expected = outputFor(selection);
    this.busy = true; this.output = null;
    try {
      this.emit('validating', 'Checking your original firmware…');
      require(files.length === 1, 'Choose exactly one original .ptf firmware file.');
      const file = files[0];
      require(/\.ptf$/i.test(file.name), 'Choose an original .ptf firmware file.');
      require(file.size > 0 && file.size <= MAX_FILE_SIZE, 'Unsupported firmware file size.');
      const data = await file.arrayBuffer();
      if (job !== this.job || !same(selection, this.selection)) return;
      const output = await this.run(data, selection, job, state => {
        if (job !== this.job) return;
        require(['validating', 'patching'].includes(state), 'Unexpected patcher state');
        this.emit(state, state === 'patching' ? 'Applying selected features and verifying the result…' : 'Validating stock firmware and safety checks…');
      }, cancel => { this.cancel = cancel; });
      if (job !== this.job || !same(selection, this.selection)) return;
      require(output instanceof Uint8Array && await sha256(output) === expected.sha256, 'Output verification failed. No download is available.');
      if (job !== this.job) return;
      this.output = output; this.emit('success', 'Output SHA-256 verified.');
    } catch (error) {
      if (job === this.job) { this.output = null; this.emit('error', error instanceof Error ? error.message : 'Firmware processing failed. No download is available.'); }
    } finally { if (job === this.job) { this.busy = false; this.cancel = null; } }
  }
}
