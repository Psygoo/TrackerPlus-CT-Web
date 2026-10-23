import { FirmwareSession } from './session.js';
import { OUTPUT_NAME, OUTPUT_SHA256 } from './patcher.js';

const choose = document.querySelector('#choose');
const input = document.querySelector('#firmware');
const drop = document.querySelector('#drop');
const download = document.querySelector('#download');
const status = document.querySelector('#status');
const stepLabel = document.querySelector('#step-label');
let worker, pending = null, downloadURL = null, ready = false;
function clearDownload() {
  download.disabled = true;
  if (downloadURL) URL.revokeObjectURL(downloadURL);
  downloadURL = null;
}
function update({ state, message, canDownload }) {
  clearDownload();
  document.body.dataset.state = state;
  status.textContent = message;
  const stage = { waiting: 1, validating: 2, patching: 3, success: 4, error: 2 }[state] ?? 1;
  stepLabel.textContent = { waiting: '01 / SELECT FIRMWARE', validating: '02 / VERIFY INPUT', patching: '03 / PATCH AND VERIFY OUTPUT', success: '04 / DOWNLOAD VERIFIED FILE', error: 'CHECK YOUR FILE' }[state];
  for (const item of document.querySelectorAll('#workflow li')) {
    const number = Number(item.dataset.step);
    item.classList.toggle('done', number < stage || (state === 'success' && number < 4));
    item.classList.toggle('current', number === stage);
    if (number === stage) item.setAttribute('aria-current', 'step');
    else item.removeAttribute('aria-current');
  }
  const busy = state === 'validating' || state === 'patching';
  choose.disabled = !ready || busy;
  input.disabled = !ready || busy;
  drop.setAttribute('aria-busy', String(busy));
  download.disabled = !canDownload;
  document.querySelector('#hash').hidden = state !== 'success';
}
const session = new FirmwareSession((bytes, onState) => new Promise((resolve, reject) => {
  if (!ready) { reject(new Error('The local patcher is not ready. Reload this page.')); return; }
  pending = { resolve, reject, onState };
  worker.postMessage(bytes, [bytes]);
}), update);
document.querySelector('#output-hash').textContent = OUTPUT_SHA256;
if (!globalThis.crypto?.subtle || !globalThis.Worker) {
  session.emit('error', 'This browser needs Web Crypto and Web Workers. Open the site on localhost or HTTPS.');
} else {
  try {
    worker = new Worker(new URL('./worker.js', import.meta.url), { type: 'module' });
    worker.onmessage = ({ data }) => {
      if (data.ready) { ready = true; update({ state: session.state, message: status.textContent, canDownload: false }); return; }
      if (!pending) return;
      if (data.state) pending.onState(data.state);
      else {
        const job = pending; pending = null;
        if (data.error) job.reject(new Error(data.error));
        else job.resolve(data.output);
      }
    };
    worker.onerror = event => {
      event.preventDefault(); ready = false; worker.terminate();
      const error = new Error('The local patcher could not run. Reload this page to try again.');
      if (pending) { pending.reject(error); pending = null; }
      else session.emit('error', error.message);
    };
  } catch { session.emit('error', 'The local patcher could not start. Reload this page on localhost or HTTPS.'); }
}
choose.addEventListener('click', () => input.click());
input.addEventListener('change', () => { const files = [...input.files]; input.value = ''; session.select(files); });
for (const event of ['dragenter', 'dragover']) drop.addEventListener(event, e => {
  e.preventDefault(); drop.classList.add('dragging');
});
for (const event of ['dragleave', 'drop']) drop.addEventListener(event, e => {
  e.preventDefault(); drop.classList.remove('dragging');
});
drop.addEventListener('drop', e => { if (ready) session.select([...e.dataTransfer.files]); });
// Prevent the browser from navigating to a file dropped outside the target.
window.addEventListener('dragover', e => e.preventDefault());
window.addEventListener('drop', e => e.preventDefault());
download.addEventListener('click', () => {
  if (session.state !== 'success' || !session.output || download.disabled) return;
  if (!downloadURL) downloadURL = URL.createObjectURL(new Blob([session.output], { type: 'application/octet-stream' }));
  const anchor = document.createElement('a');
  anchor.href = downloadURL; anchor.download = OUTPUT_NAME;
  anchor.click();
});
window.addEventListener('pagehide', () => { clearDownload(); worker?.terminate(); });
