"""MIT. Render the bundled documentation offline without third-party tooling."""
from pathlib import Path
import html,re
ROOT=Path(__file__).resolve().parents[1]
DOCS=[('README.md','overview'),('CONDITIONS.md','conditions'),('INSTALL.md','install'),
      ('COMPATIBILITY.md','compatibility'),('WR2.md','wr2'),('RECOVERY.md','recovery'),
      ('DISCLAIMER.md','disclaimer'),('NOTICE.md','credits'),('VALIDATION.md','validation'),
      ('RELEASE_NOTES.md','release-notes'),('PUBLICATION.md','publication')]
LINKS={name:'#'+key for name,key in DOCS}
LINKS['WR2_QUICK_START.md']='quick-start.html#ct2-quick-start'
LINKS['WR2_USER_GUIDE.md']='#wr2-details'
NAV_LABELS={'wr2':'WR2 details'}

def inline(text):
    text=html.escape(text)
    def link(m):
        dest=html.unescape(m[2])
        if dest in LINKS:
            target=LINKS[dest]
            dest=target if target.startswith('#') else './'+target
        assert not re.match(r'(?:javascript|data):',dest,re.I)
        return '<a href="'+html.escape(dest,quote=True)+'">'+m[1]+'</a>'
    text=re.sub(r'\[([^\]]+)\]\(([^)]+)\)',link,text)
    text=re.sub(r'`([^`]+)`',r'<code>\1</code>',text)
    text=re.sub(r'\*\*(.+?)\*\*',r'<strong>\1</strong>',text)
    return text

def markdown(text,heading_offset=1,heading_ids=None):
    out=[];para=[];listing=None;table=False
    def flush():
        nonlocal listing,table
        if para:out.append('<p>'+inline(' '.join(para))+'</p>');para.clear()
        if listing:out.append('</'+listing+'>');listing=None
        if table:out.append('</tbody></table>');table=False
    for line in text.splitlines():
        if not line.strip():flush();continue
        if line.startswith('#'):
            flush();level=min(6,len(line)-len(line.lstrip('#'))+heading_offset)
            title=line.lstrip('# ');anchor=(heading_ids or {}).get(title)
            attr=' id="'+html.escape(anchor,quote=True)+'"' if anchor else ''
            out.append(f'<h{level}{attr}>'+inline(title)+f'</h{level}>')
        elif line.startswith('|'):
            if re.fullmatch(r'[|\s:-]+',line):continue
            if para:flush()
            cells=line.strip('|').split('|')
            if not table:
                out.append('<table><thead><tr>'+''.join('<th>'+inline(c.strip())+'</th>' for c in cells)+'</tr></thead><tbody>');table=True
            else:out.append('<tr>'+''.join('<td>'+inline(c.strip())+'</td>' for c in cells)+'</tr>')
        elif re.match(r'^(?:- |\d+\. )',line):
            if para or table:flush()
            tag='ol' if re.match(r'^\d+\. ',line) else 'ul'
            if listing and listing!=tag:flush()
            if not listing:out.append('<'+tag+'>');listing=tag
            out.append('<li>'+inline(re.sub(r'^(?:- |\d+\. )','',line))+'</li>')
        elif listing and line.startswith('  '):
            out[-1]=out[-1][:-5]+' '+inline(line.strip())+'</li>'
        else:
            if listing or table:flush()
            para.append(line)
    flush();return '\n'.join(out)

def page_head(title):
    index=(ROOT/'index.html').read_text()
    head=index.split('<body')[0].replace('<script type="module" src="./app.js"></script>','')
    head='\n'.join(line.rstrip() for line in head.splitlines())+'\n'
    return re.sub(r'<title>.*?</title>','<title>'+html.escape(title)+'</title>',head,count=1)

def render_quick_start():
    body=markdown((ROOT/'WR2_QUICK_START.md').read_text(),heading_offset=0,
                  heading_ids={'CT2 — Chance':'ct2-quick-start','WR2 — Slice Weighted Random':'wr2-quick-start'})
    links='<nav aria-label="Page navigation"><a href="./">← Web Patcher</a> <a href="./documentation.html">Full Documentation</a></nav>'
    return page_head('CT2 + WR2 Quick Start')+'<body><main class="docs">'+links+body+'</main></body></html>\n'

def render():
    head=page_head('Tracker+ CT2 + WR2 — Full Documentation')
    nav='<nav aria-label="Documentation">'+''.join('<a href="#'+key+'">'+NAV_LABELS.get(key,key.replace('-',' ').title())+'</a>' for _,key in DOCS)+'</nav>'
    legacy=('<section class="legacy-links" aria-label="Older documentation links">'
            '<p id="ct2-quick-start">The Quick Start is now a standalone page: <a href="./quick-start.html#ct2-quick-start">CT2 + WR2 Quick Start</a>.</p>'
            '<p id="wr2-quick-start">For WR2 controls, open the <a href="./quick-start.html#wr2-quick-start">WR2 Quick Start section</a>.</p>'
            '<p id="wr2-user-guide">Detailed WR2 instructions are in <a href="#wr2-details">Full Documentation — WR2 details</a>.</p>'
            '</section>\n')
    sections=[]
    for name,key in DOCS:
        body=markdown((ROOT/name).read_text())
        if key=='wr2':
            detail=(ROOT/'WR2_USER_GUIDE.md').read_text().splitlines()
            detail='\n'.join(detail[1:]) if detail and detail[0].startswith('# ') else '\n'.join(detail)
            body+='<h2 id="wr2-details">Group operation and project data</h2>'+markdown(detail)
        sections.append('<section id="'+key+'">'+body+'</section>\n')
    return head+'<body><main class="docs"><a href="./">← Web Patcher</a><h1>Full Documentation</h1>'+nav+'\n'+legacy+''.join(sections)+'</main></body></html>\n'

if __name__=='__main__':
    import sys
    result=render();target=ROOT/'documentation.html'
    quick=render_quick_start();quick_target=ROOT/'quick-start.html'
    if '--check' in sys.argv:
        assert target.read_text()==result,'Rendered docs are stale'
        assert quick_target.read_text()==quick,'Rendered Quick Start is stale'
    else:
        target.write_text(result);quick_target.write_text(quick)
