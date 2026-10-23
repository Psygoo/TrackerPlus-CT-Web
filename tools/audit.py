"""MIT. Audit the explicit source/static export, not the private development tree."""
from pathlib import Path
from html.parser import HTMLParser
from urllib.parse import urlsplit,unquote
import hashlib,json,re,sys
ROOT=Path(__file__).resolve().parents[1]
TOP={'.gitignore','.nojekyll','LICENSE','NOTICE.md','README.md','BROWSER.md','CONDITIONS.md','INSTALL.md',
     'COMPATIBILITY.md','RECOVERY.md','DISCLAIMER.md','RELEASE_NOTES.md','PUBLICATION.md','VALIDATION.md',
     'package.json','package-lock.json','index.html','documentation.html','style.css','app.js','worker.js',
     'session.js','patcher.js','ct2-program.js','PUBLIC_FILES.json'}
SOURCES={'ct1':{'condtrig.c','link.ld'},'history':{'hook.c','link.ld'},
         'pre':{'extension.c','gateways.S','link.ld'},'neighbor_labels':{'in_place.S','in_place.ld'},
         'neighbor':{'extension.c','gateway.S','link.ld'},'order':{'order.c','order.h','gateway.S','link.ld'},
         'fill':{'extension.c','gateways.S','link.ld'},'ne_pre':{'commit.S','link.ld'}}
ALLOWED=TOP|{'src/stock-fingerprints.json'}|{f'src/{folder}/{n}' for folder,names in SOURCES.items() for n in names}|{
    'tools/build-program.py','tools/serve.mjs','tools/render-docs.py','tools/audit.py','tests/core.test.mjs','tests/browser.mjs'}
def digest(data):return hashlib.sha256(data).hexdigest()
class Links(HTMLParser):
    def __init__(self,text):
        super().__init__();self.ids=set();self.links=[];self.feed(text)
    def handle_starttag(self,tag,attrs):
        attrs=dict(attrs)
        if 'id' in attrs:
            assert attrs['id'] not in self.ids,'Duplicate HTML id';self.ids.add(attrs['id'])
        for key in ('href','src'):
            if key in attrs:self.links.append(attrs[key])
def audit():
    files={str(p.relative_to(ROOT)):p for p in ROOT.rglob('*') if p.is_file()}
    assert set(files)<=ALLOWED,sorted(set(files)-ALLOWED)
    assert ALLOWED-{'PUBLIC_FILES.json'}<=set(files),sorted(ALLOWED-set(files))
    assert not any(p.is_symlink() for p in ROOT.rglob('*')),'Symlinks forbidden in export'
    parsed={n:Links(p.read_text()) for n,p in files.items() if n.endswith('.html')}
    for name,p in files.items():
        data=p.read_bytes();text=data.decode('utf-8')
        assert b'\x00' not in data and len(data)<160_000,(name,'binary/oversized asset')
        private_path=r'/'+'home/'+r'[^\s]+|/'+'Users/'+r'[^\s]+|[A-Z]:\\(?:Users|work)\\'
        assert not re.search(private_path,text),(name,'private local path')
        assert not re.search(r'-----BEGIN (?:RSA |OPENSSH |EC )?PRIVATE KEY-----|gh[pousr]_[A-Za-z0-9]{30,}|AKIA[0-9A-Z]{16}',text),(name,'possible credential')
        if name.endswith('.html'):
            for href in parsed[name].links:
                link=urlsplit(html_unescape(href))
                if link.scheme or link.netloc:
                    assert link.scheme=='https',(name,href);continue
                target=(ROOT/Path(name).parent/unquote(link.path or Path(name).name)).resolve()
                if link.path=='./':target=ROOT/'index.html'
                assert target.is_relative_to(ROOT) and target.is_file(),(name,href)
                if link.fragment:assert link.fragment in parsed[str(target.relative_to(ROOT))].ids,(name,href)
        if name.endswith('.md'):
            for href in re.findall(r'\[[^\]]+\]\(([^)]+)\)',text):
                link=urlsplit(href)
                if link.scheme or link.netloc:
                    assert link.scheme=='https',(name,href);continue
                target=(ROOT/Path(name).parent/unquote(link.path)).resolve()
                assert target.is_relative_to(ROOT) and target.is_file(),(name,href)
        if name in {'app.js','worker.js','session.js','patcher.js','ct2-program.js'}:
            assert not re.search(r'\bfetch\s*\(|XMLHttpRequest|WebSocket|sendBeacon|importScripts|https?://',text),(name,'runtime network API')
    assert digest(files['LICENSE'].read_bytes())=='080a0c2c2f4481f060f9ee3486137d9e7512a78cfbabecb31300cc275b837f97','MIT notice changed'
    historical=files['NOTICE.md'].read_text().split('## Original CT6 NOTICE.md (verbatim historical record)\n\n',1)[1]
    assert digest(historical.encode())=='80f17ae788c3397d66511fe0557f34771f23b4f246b076746ea285229ace50d8','Historical CT6 notice changed'
    for name in ('index.html','documentation.html'):
        assert "connect-src 'none'" in files[name].read_text()
    manifest={name:digest(p.read_bytes()) for name,p in sorted(files.items()) if name!='PUBLIC_FILES.json'}
    path=ROOT/'PUBLIC_FILES.json'
    if '--write-manifest' in sys.argv:path.write_text(json.dumps(manifest,indent=2)+'\n')
    else:assert json.loads(path.read_text())==manifest,'Publication manifest is stale'
    print(json.dumps({'pass':True,'public_files':len(files)+int('PUBLIC_FILES.json' not in files),'firmware_files':0,'local_links':'pass','license_and_historical_notice':'verbatim','runtime_network_apis':0}))
def html_unescape(value):
    import html
    return html.unescape(value)
if __name__=='__main__':audit()
