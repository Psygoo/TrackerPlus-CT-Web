// MIT. Each request is tagged so superseded jobs cannot publish a result.
import { runSelectedFirmware } from './worker-core.js';
let current = 0;
self.onmessage = async ({ data }) => {
  if (data?.type === 'cancel') { current = Math.max(current, data.jobId); return; }
  const { jobId, selection, input } = data || {};
  current = jobId;
  try {
    const output = await runSelectedFirmware({ input, selection, onState: state => {
      if (jobId === current) self.postMessage({ jobId, state });
    }});
    if (jobId === current) self.postMessage({ jobId, output }, [output.buffer]);
  } catch (error) {
    if (jobId === current) self.postMessage({ jobId, error: error instanceof Error ? error.message : 'Firmware processing failed.' });
  }
};
self.postMessage({ ready: true });
