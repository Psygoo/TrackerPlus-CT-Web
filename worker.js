// MIT. All imports finish before ready; firmware processing requires no requests.
import { patchFirmware } from './patcher.js';
self.onmessage = async ({ data }) => {
  try {
    const output = await patchFirmware(data, state => self.postMessage({ state }));
    self.postMessage({ output }, [output.buffer]);
  } catch (error) {
    self.postMessage({ error: error instanceof Error ? error.message : 'Firmware processing failed.' });
  }
};
self.postMessage({ ready: true });
