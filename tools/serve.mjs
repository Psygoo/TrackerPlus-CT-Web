// MIT. Local review server: static GET/HEAD only, no firmware endpoint.
import http from 'node:http';
import { readFile, stat } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { resolve, extname, sep } from 'node:path';
const root = fileURLToPath(new URL('../', import.meta.url));
const types = { '.html': 'text/html; charset=utf-8', '.js': 'text/javascript', '.mjs': 'text/javascript', '.css': 'text/css', '.json': 'application/json', '.md': 'text/plain; charset=utf-8' };
export function createServer(onRequest = () => {}) {
  return http.createServer(async (req, res) => {
    onRequest({ method: req.method, url: req.url, contentLength: req.headers['content-length'] ?? '0' });
    try {
      if (!['GET', 'HEAD'].includes(req.method)) { res.writeHead(405); res.end(); return; }
      // Test under the real repository subpath as well as localhost root.
      let name = decodeURIComponent(new URL(req.url, 'http://localhost').pathname);
      if (name.startsWith('/TrackerPlus-CT-Web/')) name = name.slice('/TrackerPlus-CT-Web'.length);
      if (name.endsWith('/')) name += 'index.html';
      if (name.split('/').some(x => x.startsWith('.') && x !== '.nojekyll')) throw Error();
      const target = resolve(root, '.' + name);
      if (!target.startsWith(root.endsWith(sep) ? root : root + sep) || !(await stat(target)).isFile()) throw Error();
      const data = await readFile(target);
      res.writeHead(200, { 'content-type': types[extname(target)] ?? 'text/plain', 'cache-control': 'no-store', 'x-content-type-options': 'nosniff' });
      res.end(req.method === 'HEAD' ? undefined : data);
    } catch { res.writeHead(404); res.end('Not found'); }
  });
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  createServer().listen(8080, '127.0.0.1', () => console.log('Local review only: http://127.0.0.1:8080/TrackerPlus-CT-Web/'));
}
