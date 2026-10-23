"""MIT. Rebuild the H12 patch template from project source; reads no firmware.

Use the Arm GNU Toolchain 15.2.Rel1. Three stock replay windows remain zero:
the browser obtains them exclusively from the user's original firmware.
"""
from pathlib import Path
import argparse,hashlib,json,os,struct,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[1]
START,END=0x601bab78,0x601bc618
CPU=['-mcpu=cortex-m7','-mthumb','-mfloat-abi=hard','-mfpu=fpv5-d16']
FLAGS=['-Os','-std=c11','-mno-unaligned-access','-ffreestanding','-fno-builtin',
       '-fno-tree-loop-distribute-patterns','-fno-common','-ffunction-sections','-Wall','-Wextra','-Werror']
def branch(at,target,link=True):
    d=(target&~1)-(at+4)
    if at&1 or not -(1<<24)<=d<(1<<24):raise ValueError('Thumb branch bounds')
    s=(d>>24)&1;j1=(~(d>>23)^s)&1;j2=(~(d>>22)^s)&1
    return struct.pack('<HH',0xf000|(s<<10)|((d>>12)&1023),(0xd000 if link else 0x9000)|(j1<<13)|(j2<<11)|((d>>1)&2047))

def build(compiler):
    compiler=Path(compiler)
    def run(name,*args):return subprocess.run([str(compiler/('arm-none-eabi-'+name)),*map(str,args)],check=True,capture_output=True).stdout
    with tempfile.TemporaryDirectory(prefix='ct-h12-build-') as temp:
        temp=Path(temp)
        def component(folder,sources,roots=()):
            work=temp/folder;work.mkdir();objects=[]
            for name in sources:
                obj=work/(Path(name).stem+'.o');args=CPU+([] if name.endswith('.S') else FLAGS)
                run('gcc',*args,'-c',ROOT/'src'/folder/name,'-o',obj);objects.append(obj)
            elf=work/'program.elf';ld=ROOT/'src'/folder/'link.ld'
            options=['-Wl,--gc-sections',*[f'-Wl,--undefined={r}' for r in roots]] if roots else []
            run('gcc',*CPU,'-nostdlib','-nostartfiles','-T',ld,*options,*objects,'-o',elf)
            raw=work/'program.bin';run('objcopy','-O','binary','-j','.text',elf,raw)
            syms={r.split()[2]:int(r.split()[0],16) for r in run('nm','-n',elf).decode().splitlines() if len(r.split())==3}
            return raw.read_bytes(),syms
        ct,cs=component('ct1',['condtrig.c'],[
            'mt_ct_chance','mt_ct_count_in','mt_ct_play_from_a','mt_ct_play_from_b','mt_ct_play_from_c','mt_ct_play_from_d',
            'mt_ct_advance_step','mt_ct_seq_advance','mt_ct_render_advance','mt_ct_cell_max','mt_ct_fx1_text','mt_ct_fx2_text','mt_version_fmt'])
        hist,hs=component('history',['hook.c'],['h03_chance','h03_process_track'])
        pre,ps=component('pre',['extension.c','gateways.S'])
        ne,ns=component('neighbor',['extension.c','gateway.S'])
        order,osym=component('order',['gateway.S','order.c'])
        fill,fs=component('fill',['gateways.S','extension.c'])
        asm=temp/'labels.o';elf=temp/'labels.elf'
        run('as','-mcpu=cortex-m7','-mthumb',ROOT/'src/neighbor_labels/in_place.S','-o',asm)
        run('ld','-T',ROOT/'src/neighbor_labels/in_place.ld',asm,'-o',elf)
        sections={}
        for section in ('consumer','formatter'):
            raw=temp/(section+'.bin');run('objcopy','-O','binary','-j','.'+section,elf,raw);sections[section]=raw.read_bytes()
        assert [len(x) for x in (ct,hist,pre,ne,order,fill)]==[989,264,312,502,124,296]
        image=bytearray(b'\xff'*(END-START))
        def put(at,data):
            assert START<=at and at+len(data)<=END
            image[at-START:at-START+len(data)]=data
        def word(at,value):put(at,struct.pack('<I',value))
        def edit(at,before,after):
            before=bytes.fromhex(before);assert image[at-START:at-START+len(before)]==before,(hex(at),image[at-START:at-START+len(before)].hex())
            assert len(before)==len(after);put(at,after)
        put(0x601bab80,ct)
        chain={'chance':0x3af5,'advance_step':ns['h10_advance']|1,'seq_advance':0x600aa859,'render_advance':0x600bc2d9,
               'count_in':0x1251,'play_from_a':0x1251,'play_from_b':0x1251,'play_from_c':0x1251,'play_from_d':0x1251,
               'cell_max':0x600af2c9,'fx1_text':0x600afdc9,'fx2_text':0x600afdc9}
        for name,value in chain.items():word(cs['mt_ct_next_'+name],value)
        put(0x601baf60,hist);word(hs['h03_next_chance'],cs['mt_ct_chance']|1);word(hs['h03_next_process_track'],0x203d)
        # Retain the inert H04 image exactly: this is our history code, not stock.
        edit(0x601baf94,'22688846',bytes.fromhex('884615e0'));edit(0x601bb000,'05d8',bytes.fromhex('05e0'))
        def stub(target,args=False):
            return (struct.pack('<HH',0x463a,0x4643) if args else b'')+struct.pack('<HHI',0xf8df,0xf000,target|1)
        put(0x601bb070,stub(hs['h03_chance'],True)+stub(cs['mt_ct_count_in'])+stub(hs['h03_process_track']))
        put(0x601bb08c,b'%d.%d.%d-h04\0')
        bundle=bytearray(image[0x601baf60-START:0x601bb099-START]);delta=0x601bc000-0x601baf60
        for off in (0x80,0xfc,0x118,0x128):struct.pack_into('<I',bundle,off,struct.unpack_from('<I',bundle,off)[0]+delta)
        put(0x601bc000,bundle)
        edit(0x601bc034,'884615e0',bytes.fromhex('22688846'));edit(0x601bc0a0,'05e0',bytes.fromhex('05d8'))
        edit(0x601bc058,'653d',bytes.fromhex('003d'));edit(0x601bc05c,'242d',bytes.fromhex('892d'))
        put(0x601bc12c,b'%d.%d.%d-h12\0')
        put(0x601bc140,pre);put(0x601bc278,b'!NE\0');put(0x601bc27c,ne);put(0x601bc474,order);put(0x601bc4f0,fill)
        word(0x601bc100,ns['h10_chance']|1);word(0x601bc104,ns['h10_process']|1)
        edit(0x601bad4e,'8920',bytes.fromhex('8d20'))
        put(0x601badd0,branch(0x601badd0,ps['h07_reset_gateway'],False))
        edit(0x601bc156,'012d',bytes.fromhex('032d'));put(0x601bc16e,sections['consumer']);put(0x601bc208,sections['formatter'])
        put(0x601bc1d8,branch(0x601bc1d8,ns['h10_reset_store']))
        edit(0x601bae5c,'73207423',bytes.fromhex('53200023'));edit(0x601bae66,'7323',bytes.fromhex('5323'))
        plans=[]
        def plan(at,data):plans.append({'at':at,'hex':data.hex()})
        for at,name in [(0x600aa7f8,'mt_ct_seq_advance'),(0x600bc27a,'mt_ct_render_advance'),(0x600af1b2,'mt_ct_cell_max'),
                        (0x600df748,'mt_ct_fx1_text'),(0x600df77c,'mt_ct_fx2_text')]:plan(at,branch(at,cs[name]))
        load,exe=0x6018d6ec,8
        for at,target in [(0x30ce,0x1fb40),(0x3752,0x1fb4c),(0x372a,0x1fb54)]:plan(load+at-exe,branch(at,target))
        plan(0x601acf00,struct.pack('<I',cs['mt_ct_advance_step']|1))
        for at,name in [(0x600ac604,'a'),(0x600ac694,'b'),(0x600acd34,'c'),(0x600ad544,'d')]:plan(at,struct.pack('<I',cs['mt_ct_play_from_'+name]|1))
        plan(0x600cbf9c,struct.pack('<I',0x601bc12c));plan(0x600302c4,struct.pack('<III',0x601bc110,0x1fb40,28))
        plan(0x600afb48,branch(0x600afb48,ps['h07_value_text'],False))
        plan(0x600af178,branch(0x600af178,osym['h11_editor_gateway'],False)+bytes.fromhex('00bf'))
        for at,name in [(0x600e03ba,'h12_fill_max'),(0x600e040a,'h12_fill_max'),(0x600e0736,'h12_fill_max'),(0x600e07c2,'h12_fill_max'),
                        (0x600e3f72,'h12_fill_max'),(0x600e41fe,'h12_fill_max'),(0x600e3fd0,'h12_fill_text'),(0x600e425c,'h12_fill_text'),
                        (0x600af82e,'h12_generator_max'),(0x600af064,'h12_generator_max'),(0x600af846,'h12_fromto_store'),(0x600aeff8,'h12_random_gateway')]:plan(at,branch(at,fs[name]))
        copies=[{'from':0x600afb48,'to':0x601bc26c,'length':4},
                {'from':0x600af178,'to':0x601bc474,'length':6},
                {'from':0x600af846,'to':0x601bc526,'length':4}]
        for c in copies:assert image[c['to']-START:c['to']-START+c['length']]==bytes(c['length'])
        guards=json.loads((ROOT/'src/stock-fingerprints.json').read_text())
        for p in plans:p['sha256']=guards[f"{p['at']:x}:{len(bytes.fromhex(p['hex']))}"]
        for c in copies:c['sha256']=guards[f"{c['from']:x}:{c['length']}"]
        data=dict(base=0x60030000,start=START,end=END,template_sha256=hashlib.sha256(image).hexdigest(),
                  plans=sorted(plans,key=lambda p:p['at']),copies=copies,
                  record_ends=[0x601bb099,0x601bc139,0x601bc278,0x601bc27c,0x601bc472,0x601bc4f0,END])
        return bytes(image),data

def render(image,data):
    rows=[image[i:i+32].hex() for i in range(0,len(image),32)]
    return ('// MIT project patch code, rebuilt from src/; no firmware is read by the builder.\n'
            '// Zero replay slots are populated from the user input only.\n'
            'const hex =\n'+ ' +\n'.join('  '+json.dumps(r) for r in rows)+';\n'
            'export const recipe = Object.freeze('+json.dumps(data,indent=2)+');\n'
            'export function buildProgram() { return Uint8Array.from(hex.match(/../g), s => parseInt(s, 16)); }\n')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--compiler',default=os.environ.get('ARM_GCC_BIN',''))
    p.add_argument('--check',action='store_true');a=p.parse_args()
    image,data=build(a.compiler);source=render(image,data);target=ROOT/'h12-program.js'
    if a.check:
        assert target.read_text()==source,'Source-generated H12 program changed'
        print('H12_SOURCE_REBUILD_PASS',data['template_sha256'])
    else:target.write_text(source);print('H12 project template exported; no firmware read',data['template_sha256'])
