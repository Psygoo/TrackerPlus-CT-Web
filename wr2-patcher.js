// MIT. S12C direct-stock composition from project-owned payloads.
import payloads from './wr2/generated/payloads.js';
import guards from './wr2/generated/stock-guards.js';
import { patchFirmware, parsePTF, encodePTF, STOCK_SHA256 } from './patcher.js';
const EXPECTED='06adca54ff28ae704e92c4464a975f3c0e34f16d67a177db53157895be199445';
const CT2_HASH='baa93d46208c0f6ab2ca0c7507fb3c430703a3ceada582a780779844a0d03b20';
const bytes=h=>Uint8Array.from(h.match(/../g)||[],x=>parseInt(x,16));
const sha=async b=>Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',b)),x=>x.toString(16).padStart(2,'0')).join('');
function branch(at,to,link=false){
 const d=(to&~1)-(at+4);if((at&1)||d<-(1<<24)||d>=(1<<24)||(d&1))throw Error('Thumb branch out of range');
 const s=(d>>>24)&1,j1=(~(d>>>23)^s)&1,j2=(~(d>>>22)^s)&1;
 return Uint8Array.of((0xf000|(s<<10)|((d>>>12)&1023))&255,(0xf000|(s<<10)|((d>>>12)&1023))>>>8,(link?0xd000:0x9000)|(j1<<13)|(j2<<11)|((d>>>1)&2047)&255,((link?0xd000:0x9000)|(j1<<13)|(j2<<11)|((d>>>1)&2047))>>>8);
}
function joined(a,b){const x=new Uint8Array(a.length+b.length);x.set(a);x.set(b,a.length);return x;}
function emitRecords(existing,at,data,alignmentOrigin=at){
 const out=[];let upper=[...existing].reverse().find(r=>r.kind===4);let hi=upper?upper.data[0]*256+upper.data[1]:null,offset=0;
 while(offset<data.length){const addr=at+offset,next=addr>>>16;if(next!==hi){hi=next;out.push({kind:4,at:0,data:Uint8Array.of(hi>>>8,hi&255)});}
  const n=Math.min(16-((addr-alignmentOrigin)%16),data.length-offset,65536-(addr&65535));out.push({kind:0,at:addr,data:data.slice(offset,offset+n)});offset+=n;}
 return out;
}
export async function verifyWR2StockGuards(stock){
 const base=stock.base;
 for(const [key,digest] of Object.entries(guards)){const [at,n]=key.split(':').map(Number);const slice=stock.image.slice(at-base,at-base+n);if(slice.length!==n||await sha(slice)!==digest)throw Error(`Stock patch window mismatch at 0x${at.toString(16)}.`);}
 return true;
}
export async function patchCombinedFirmware(input,onState=()=>{}){
 onState('validating');const original=new Uint8Array(input).slice();
 if(await sha(original)!==STOCK_SHA256)throw Error('Unsupported original Tracker+ 1.2.0 firmware.');
 const stock=parsePTF(original);await verifyWR2StockGuards(stock);const ct2Data=await patchFirmware(original,onState),ct2=parsePTF(ct2Data);
 if(await sha(ct2Data)!==CT2_HASH)throw Error('CT2 baseline verification failed.');
 let current=ct2.image.slice();const records=ct2.records.filter(r=>r.kind!==1&&r.kind!==5),ending=ct2.records.filter(r=>r.kind===1||r.kind===5),base=ct2.base;
 const stockSlice=(at,n)=>stock.image.slice(at-base,at-base+n);
 const put=(at,data)=>{const off=at-base;if(off<0||off+data.length>current.length)throw Error(`Patch outside current image at 0x${at.toString(16)}.`);current.set(data,off);};
 const add=(at,data,origin=at)=>{if(at!==base+current.length)throw Error(`Payload gap at 0x${at.toString(16)}.`);records.push(...emitRecords(records,at,data,origin));current=joined(current,data);};
 const sym=(group,name)=>{const n=payloads[group].symbols[name];if(!n)throw Error(`Missing compiled symbol ${group}.${name}`);return n;};
 const ui=bytes(payloads.ui.hex);if(await sha(ui)!==payloads.ui.sha256)throw Error('WR2 UI payload hash mismatch.');
 const wr1=bytes(payloads.wr1.hex);if(await sha(wr1)!==payloads.wr1.sha256)throw Error('WR1 compatibility payload hash mismatch.');
 const hook=(at,n,name)=>{const data=branch(at,sym('ui',name),false);const out=new Uint8Array(n);out.set(data);for(let i=4;i<n;i+=2){out[i]=0;out[i+1]=0xbf;}put(at,out);};
 onState('patching');
 add(0x601bc63a,joined(Uint8Array.of(0,0xbf),wr1),0x601bab78);put(0x601bc135,Uint8Array.of(0x57,0x52,0x32));
 add(0x601bc6f0,ui);
 hook(0x600f3e34,24,'wr2_more_handler');hook(0x60107d60,6,'wr2_group_marker_color');hook(0x60107cb2,4,'wr2_group_marker_selected_palette2');hook(0x60107db6,4,'wr2_group_marker_selected_palette1');hook(0x600f7c54,4,'wr2_refresh_entry');hook(0x600f3f2a,8,'wr2_page3_pad_callback_gate');hook(0x600f8002,4,'wr2_refresh_epilogue');hook(0x600f5124,4,'wr2_after_stock_control_init');put(0x60032b3a,branch(0x60032b3a,sym('ui','wr2_mode5_gateway'),true));
 for(const [at,h] of [[0x601bdb94,'07f1180025b0bde8f08f'],[0x601bdc78,'07f1180200bf'],[0x601bdd0c,'00000000']])put(at,bytes(h));put(0x601bdb3e,bytes(payloads.address.hex));
 for(const name of ['s11b','s12a','s12b','s12c']){const p=payloads[name],cfg=p.settings,code=bytes(p.hex);if(await sha(code)!==cfg.payload_sha256)throw Error(`${name} payload hash mismatch`);const start=base+current.length,aligned=(start+3)&~3;if(aligned!==cfg.CODE)throw Error(`${name} payload address mismatch`);add(start,joined(new Uint8Array(cfg.CODE-start),code));for(const h of cfg.hooks){if(h.kind==='heap-origin'){put(h.address,Uint8Array.of(0x70,0xfc,0x02,0x20));continue;}const op=branch(h.address,p.symbols[h.symbol],h.kind==='BL');put(h.address,op);}}
 const final=encodePTF({...ct2,image:current,records:[...records,...ending]});if(await sha(final)!==EXPECTED){const out=parsePTF(final);const refHash='931acd25fe374c678456e590346447311047678d6d934055cd49d5e1d6df8659';throw Error(`Combined output hash mismatch: ${await sha(final)}; decoded ${await sha(out.image)}; expected decoded ${refHash}`);}return final;
}
