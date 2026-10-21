// MIT. A single processing session; only verified results can enable download.
import { MAX_FILE_SIZE, OUTPUT_SHA256, require, sha256 } from './patcher.js';
export class FirmwareSession {
  constructor(run, update) {
    this.run = run; this.update = update; this.busy = false; this.output = null;
    this.emit('waiting', 'Waiting for original firmware');
  }
  emit(state, message) {
    this.state = state;
    this.update({ state, message, canDownload: state === 'success' && this.output !== null });
  }
  async select(files) {
    if (this.busy) return;
    this.busy = true; this.output = null;
    try {
      this.emit('validating', 'Checking your original firmware…');
      require(files.length === 1, 'Choose exactly one original .ptf firmware file.');
      const file = files[0];
      require(/\.ptf$/i.test(file.name), 'Choose an original .ptf firmware file.');
      require(file.size > 0 && file.size <= MAX_FILE_SIZE, 'Unsupported firmware file size.');
      const output = await this.run(await file.arrayBuffer(), state => {
        require(['validating', 'patching'].includes(state), 'Unexpected patcher state');
        this.emit(state, state === 'patching' ? 'Applying CT6 and verifying the result…' : 'Validating stock firmware and safety checks…');
      });
      // Independently verify worker output before the UI can create a download.
      require(output instanceof Uint8Array && await sha256(output) === OUTPUT_SHA256, 'Output verification failed. No download is available.');
      this.output = output;
      this.emit('success', 'CT6 is ready. Output SHA-256 verified.');
    } catch (error) {
      this.output = null;
      this.emit('error', error instanceof Error ? error.message : 'Firmware processing failed. No download is available.');
    } finally { this.busy = false; }
  }
}
