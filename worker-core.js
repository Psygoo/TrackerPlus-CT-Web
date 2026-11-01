// MIT. Pure selection dispatch shared by the module worker and Node tests.
import { patchFirmware } from './patcher.js';
import { patchCombinedFirmware } from './wr2-patcher.js';
export async function runSelectedFirmware({ input, selection, onState = () => {} }) {
  if (selection?.ct2 !== true) throw new Error('CT2 is required. WR2-only output is unsupported.');
  if (selection.wr2 && selection.devWR2 !== true) throw new Error('WR2 generation is disabled in this release.');
  return selection.wr2 ? patchCombinedFirmware(input, onState) : patchFirmware(input, onState);
}
