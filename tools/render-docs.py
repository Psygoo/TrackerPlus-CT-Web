"""MIT. Render the bundled documentation offline without third-party tooling."""
from pathlib import Path
import html,re
ROOT=Path(__file__).resolve().parents[1]
DOCS=[('README.md','overview'),('CONDITIONS.md','conditions'),('INSTALL.md','install'),
      ('COMPATIBILITY.md','compatibility'),('RECOVERY.md','recovery'),
      ('DISCLAIMER.md','disclaimer'),('NOTICE.md','credits'),('VALIDATION.md','validation'),
      ('RELEASE_NOTES.md','release-notes'),('PUBLICATION.md','publication')]
LINKS=dict(DOCS)
def inline(text):
    text=html.escape(text)
    def link(m):
        dest=html.unescape(m[2]);dest='#'+LINKS[dest] if dest in LINKS else dest
        assert not re.match(r'(?:javascript|data):',dest,re.I)
        return '<a href="'+html.escape(dest,quote=True)+'">'+m[1]+'</a>'
    text=re.sub(r'\[([^\]]+)\]\(([^)]+)\)',link,text)
    text=re.sub(r'`([^`]+)`',r'<code>\1</code>',text)
    text=re.sub(r'\*\*(.+?)\*\*',r'<strong>\1</strong>',text)
    return text
def markdown(text):
    out=[];para=[];listing=False;table=False
    def flush():
        nonlocal listing,table
        if para:out.append('<p>'+inline(' '.join(para))+'</p>');para.clear()
        if listing:out.append('</ul>');listing=False
        if table:out.append('</tbody></table>');table=False
    for line in text.splitlines():
        if not line.strip():flush();continue
        if line.startswith('#'):
            flush();level=min(6,len(line)-len(line.lstrip('#'))+1)
            out.append(f'<h{level}>'+inline(line.lstrip('# '))+f'</h{level}>')
        elif line.startswith('|'):
            if re.fullmatch(r'[|\s:-]+',line):continue
            if para:flush()
            cells=line.strip('|').split('|')
            if not table:
                out.append('<table><thead><tr>'+''.join('<th>'+inline(c.strip())+'</th>' for c in cells)+'</tr></thead><tbody>');table=True
            else:out.append('<tr>'+''.join('<td>'+inline(c.strip())+'</td>' for c in cells)+'</tr>')
        elif re.match(r'^(?:- |\d+\. )',line):
            if para or table:flush()
            if not listing:out.append('<ul>');listing=True
            out.append('<li>'+inline(re.sub(r'^(?:- |\d+\. )','',line))+'</li>')
        elif listing and line.startswith('  '):
            out[-1]=out[-1][:-5]+' '+inline(line.strip())+'</li>'
        else:
            if listing or table:flush()
            para.append(line)
    flush();return '\n'.join(out)
def render():
    index=(ROOT/'index.html').read_text()
    head=index.split('<body')[0].replace('<script type="module" src="./app.js"></script>','').replace('<title>Tracker+ Conditional Trigs — CT2 · v0.2.1-CT2</title>','<title>CT2 documentation — Tracker+ Conditional Trigs</title>')
    nav='<nav aria-label="Documentation">'+''.join('<a href="#'+key+'">'+key.replace('-',' ').title()+'</a>' for _,key in DOCS)+'</nav>'
    return head+'<body><main class="docs"><a href="./">← Local firmware patcher</a><h1>CT2 documentation</h1>'+nav+'\n'+''.join('<section id="'+key+'">'+markdown((ROOT/name).read_text())+'</section>\n' for name,key in DOCS)+'</main></body></html>\n'
if __name__=='__main__':
    import sys
    result=render();target=ROOT/'documentation.html'
    if '--check' in sys.argv:assert target.read_text()==result,'Rendered docs are stale'
    else:target.write_text(result)
