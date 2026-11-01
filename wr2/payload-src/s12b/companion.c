/* Foreground Save-only derived caches. No reader is connected to restore or
 * Import. Native PCM is observed once, while the copier owns its buffer. */
#include "companion.h"
#define FN S12_FN
static void zero(void *p,unsigned n){uint8_t *b=p;while(n--)*b++=0;}
static void cp(void *d,const void *s,unsigned n){uint8_t *a=d;const uint8_t *b=s;while(n--)*a++=*b++;}
static int eq(const void *a,const void *b,unsigned n){const uint8_t *x=a,*y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static unsigned length(const char *p,unsigned max){unsigned n=0;while(n<max&&p[n])++n;return n;}
static uint32_t g32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void p32(uint8_t *p,uint32_t v){for(unsigned n=0;n<4;++n)p[n]=(uint8_t)(v>>(n*8));}
static uint64_t seq(const uint8_t *p){return (uint64_t)g32(p+40)|(uint64_t)g32(p+44)<<32;}
static void free_mem(void *p){FN(0x60166364u,void(*)(void *))(p);}
static void *fs(void){return *(void **)(uintptr_t)S12_FM;}
static uintptr_t method(unsigned off){void *f=fs();return f?((uint32_t *)(uintptr_t)*(uint32_t *)f)[off/4]:0;}
static int join(char *out,const char *base,const char *tail){unsigned a=length(base,256),b=length(tail,256);if(a+b>=256)return 0;cp(out,base,a);cp(out+a,tail,b+1);return 1;}
static int cachepath(char *out,const char *base,unsigned slot,char bank){
    char leaf[]="/wr2_companions/instrument_01.a.w2i";
    if(slot>=48)return 0;leaf[27]=(char)('0'+(slot+1)/10);leaf[28]=(char)('0'+(slot+1)%10);leaf[30]=bank;
    return join(out,base,leaf);
}
static void seal(uint8_t *p){p32(p+16,0);p32(p+16,wr2_crc32c(p,288));}
static int envelope(uint8_t *p,int draft){
    static const uint8_t head[16]={'W','2','I','N','S','T','1',0,1,0,176,0,32,1,0,0};
    static const uint8_t dmagic[8]={'W','2','D','R','A','F','1',0};
    if(!eq(p,draft?dmagic:head,8)||!eq(p+8,head+8,8))return 0;
    uint32_t crc=g32(p+16);p32(p+16,0);int ok=crc==wr2_crc32c(p,288);p32(p+16,crc);
    unsigned slot=g32(p+20),type=g32(p+24),frames=g32(p+28),ch=g32(p+32);
    if(!ok||slot>=48||type>1||ch!=type+1||!frames||frames>0x3fffffff/ch||g32(p+36)!=frames*ch*2)return 0;
    unsigned n=length((char *)p+112,64);if(!n||n==64)return 0;
    for(unsigned k=112+n;k<176;++k)if(p[k])return 0;
    for(unsigned k=280;k<288;++k)if(p[k])return 0;
    if(!draft&&(!seq(p)||!p[176]||p[178]!=slot||!wr2_record_valid(p+176)))return 0;
    return 1;
}
/* Exported for executable packet tests; no runtime restoration caller. */
int s12b_packet_valid(uint8_t *p){return envelope(p,0);}
static s12b_capture *capture(s12b_work *w){return &w->capture;}
s12b_work *s12b_work_get(void){return (S12B_ANC->flags&S12B_SAVE)?S12B_ANC->owned:0;}
int s12b_begin(s12b_work *w){
    zero(&w->blob,sizeof(w->blob));s12b_capture *c=capture(w);
    uint32_t err=0;c->lease=FN(0x60166550u,void *(*)(void *,unsigned))(&err,sizeof(s12b_lease));
    if(c->lease)zero(c->lease,sizeof(*c->lease));
    S12B_ANC->owned=w;S12B_ANC->flags=(S12B_ANC->flags&1u)|S12B_SAVE;
    /* Optional mkdir failure disables capture, never primary persistence. */
    uintptr_t mk=method(0x10);
    if(c->lease&&(!join(w->path,w->base,"/wr2_companions")||!mk||
       !((unsigned(*)(void *,unsigned,const char *,unsigned))mk)(fs(),0,w->path,1)))c->lease->bad=1;
    return 1;
}
static int lease_poll(s12b_lease *l){
    if(!l)return 1;(void)s12_close(&l->io);
    for(unsigned n=0;n<2;++n)if(l->native_fil[n]&&FN(0x6009c9a0u,int(*)(void *))((void *)(uintptr_t)l->native_fil[n])==0){
        FN(0x6016a084u,void(*)(void *,unsigned))((void *)(uintptr_t)l->native_fil[n],0x250);l->native_fil[n]=0;
    }
    return !l->io.file[0]&&!l->native_fil[0]&&!l->native_fil[1];
}
void s12b_release(s12b_work *w,s12b_lease *l){
    /* Every uncertain FIL stays at its original address. Path scratch is dead
     * after finish/cancel, so it may retain this pointer during quarantine. */
    int clean=(!l||(!l->io.file[0]&&!l->native_fil[0]&&!l->native_fil[1]))&&!w->io.file[0];
    if(!clean){cp(w->base,&l,sizeof(l));S12B_ANC->owned=w;S12B_ANC->flags=(S12B_ANC->flags&1u)|S12B_SAVE|S12B_POISON;return;}
    if(l)free_mem(l);S12B_ANC->owned=0;S12B_ANC->flags&=1u;free_mem(w);
}
void s12b_poll(void){
    if(!(S12B_ANC->flags&S12B_POISON))return;
    s12b_work *w=s12b_work_get();if(!w)return;s12b_lease *l;cp(&l,w->base,sizeof(l));
    int closed=s12_close(&w->io);int clean=lease_poll(l);if(closed&&clean)s12b_release(w,l);
}
void s12b_cancel(void){
    s12b_work *w=s12b_work_get();if(!w)return;
    S12B_ANC->flags&=~1u;if(S12B_ANC->flags&S12B_POISON)return;
    s12b_release(w,capture(w)->lease);
}
static int exporting(void){return *(volatile uint8_t *)(uintptr_t)(S12_FM+0x13f)==4&&*(volatile uint8_t *)(uintptr_t)(S12_FM+0x90)==3;}
void s12b_export_begin(uint32_t *ctx,const char *destination){
    s12b_work *w=s12b_work_get();if(!w||!exporting())return;
    s12b_capture *c=capture(w);if(!c->lease||c->lease->bad)return;
    if(c->active){if(c->copier!=(uint32_t)(uintptr_t)ctx)c->lease->bad=1;return;}
    if(((uint8_t *)ctx)[4]!=0)return;
    for(unsigned n=0;n<2;++n){uint32_t *f=*(uint32_t **)(uintptr_t)(0x2001f7a8u+4*n);if(f&&f[0]){c->lease->bad=1;return;}}
    unsigned slot=*(volatile uint32_t *)(uintptr_t)(S12_FM+0x94),n=length(w->base,256);
    static const char infix[]="/instruments/";
    if(slot>=48||length(destination,256)==256||!eq(destination,w->base,n)||!eq(destination+n,infix,sizeof(infix)-1))return;
    const char *leaf=destination+n+sizeof(infix)-1;unsigned len=length(leaf,64),value=0,k=0;
    while(k<len&&leaf[k]>='0'&&leaf[k]<='9'&&k<2){value=value*10+(unsigned)(leaf[k]-'0');++k;}
    if(!k||leaf[k]!=' '||value!=slot+1||len<6||len>=64||!eq(leaf+len-4,".pti",4))return;
    for(unsigned i=0;i<len;++i)if(leaf[i]=='/'||leaf[i]=='\\')return;
    c->seen&=~(UINT64_C(1)<<slot);
    zero(c->packet,288);cp(c->packet,"W2DRAF1",8);c->packet[8]=1;c->packet[10]=176;c->packet[12]=32;c->packet[13]=1;
    p32(c->packet+20,slot);cp(c->packet+112,leaf,len);c->slot=slot;c->offset=0;c->total=0;c->active=1;c->copier=(uint32_t)(uintptr_t)ctx;
}
void s12b_tap(unsigned bytes,uint32_t *ctx){
    s12b_work *w=s12b_work_get();if(!w||!exporting())return;s12b_capture *c=capture(w);
    if(!c->lease||c->lease->bad||!c->active||c->copier!=(uint32_t)(uintptr_t)ctx)return;
    if(!bytes)return;
    if(bytes>32640||ctx[3]!=c->offset){c->active=0;return;}
    const uint8_t *p=(const uint8_t *)(uintptr_t)0x83f45c84u;
    if(!c->offset){
        unsigned type=p[20],frames=g32(p+60),ch=type+1;
        if(bytes<392||g32(p)!=0x14954u||p[12]!=0x74||p[13]!=1||p[16]!=1||type>1||!frames||frames>0x3fffffff/ch){c->active=0;return;}
        c->total=392+frames*ch*2;if(c->total<392||ctx[4]!=c->total){c->active=0;return;}
        c->identity=wr2_identity(p+16);c->count=p[376];
        p32(c->packet+24,type);p32(c->packet+28,frames);p32(c->packet+32,ch);p32(c->packet+36,c->total-392);
        p32(c->packet+176,c->identity);p32(c->packet+180,c->count);
        wr2_sha256_init(&c->sha);wr2_sha256_update(&c->sha,c->packet+24,12);
    }
    if(bytes>c->total-c->offset){c->active=0;return;}
    unsigned skip=c->offset?0:392;wr2_sha256_update(&c->sha,p+skip,bytes-skip);c->offset+=bytes;
}
static int write_packet(s12_io *io,const char *path,uint8_t *p){
    if(s12_open(io,path,1))return 0;int ok=s12_write(io,p,288);int closed=s12_close(io);return ok&&closed;
}
static int read_packet(s12_io *io,const char *path,uint8_t *p){
    int r=s12_open(io,path,0);if(r==4||r==5)return 1;if(r)return -1;
    unsigned size=s12_size(io);int ok=size==288&&s12_read(io,p,288);int closed=s12_close(io);return !closed?-1:size!=288?2:ok?0:-1;
}
void s12b_export_end(uint32_t *ctx,unsigned result){
    s12b_work *w=s12b_work_get();if(!w||!exporting())return;s12b_capture *c=capture(w);s12b_lease *l=c->lease;
    if(!l||!c->active)return;
    if(result==1)return;
    if(l->bad){c->active=0;return;}
    c->active=0;
    if(result!=2||ctx[3]!=ctx[4]||c->offset!=c->total||c->offset!=ctx[4]||!c->total)return;
    wr2_sha256_final(&c->sha,c->packet+80);seal(c->packet);
    if(!cachepath(w->temporary,w->base,c->slot,'d')||!write_packet(&l->io,w->temporary,c->packet)||
       read_packet(&l->io,w->temporary,c->check)!=0||!eq(c->packet,c->check,288)||!envelope(c->check,1)){
        if(l->io.file[0])l->bad=1;return;
    }
    c->seen|=UINT64_C(1)<<c->slot;
}
extern int s12_close_native(uint32_t *);
int s12b_native_close(uint32_t *file,uintptr_t caller){
    if(!exporting())return s12_close_native(file);
    s12b_work *w=s12b_work_get();s12b_capture *c=w?capture(w):0;s12b_lease *l=c?c->lease:0;
    if(!c||!c->active||!l)return s12_close_native(file);
    if(!file||!file[0])return 0;uint32_t fil=file[0];
    int r=FN(0x6009c9a0u,int(*)(void *))((void *)(uintptr_t)fil);
    if(r==0||(r==9&&caller==0x6009e79du)){
        FN(0x6016a084u,void(*)(void *,unsigned))((void *)(uintptr_t)fil,0x250);zero(file,259);return r==0;
    }
    for(unsigned n=0;n<2;++n)if(!l->native_fil[n]){l->native_fil[n]=fil;file[0]=0;l->bad=1;return 0;}
    /* Before capture starts, admission below excludes foreign live wrappers;
     * no third native FIL can enter this operation. */
    l->bad=1;return 0;
}
/* After root publication, path/tmp/guard are dead. Exactly 768 bytes reused. */
typedef s12b_finish_scratch finish_scratch;
_Static_assert(sizeof(finish_scratch)==768,"dead path union");
static int bank(s12_io *io,finish_scratch *s,const char *base,unsigned slot,char b,uint64_t *generation){
    if(!cachepath(s->path,base,slot,b))return -1;
    int r=read_packet(io,s->path,s->packet);if(r)return r==2?1:r;
    if(!envelope(s->packet,0)||g32(s->packet+20)!=slot){*generation=0;return 1;}
    *generation=seq(s->packet);return 0;
}
static int verify(s12_io *io,finish_scratch *s){
    if(s12_open(io,s->path,0))return 0;int ok=s12_size(io)==288;
    for(unsigned n=0;n<288&&ok;){unsigned k=288-n;if(k>224)k=224;
        ok=s12_read(io,s->chunk,k)&&eq(s->chunk,s->packet+n,k);n+=k;}
    int closed=s12_close(io);return ok&&closed;
}
void s12b_finish(s12b_work *w,s12b_lease *l,uint64_t seen){
    if(!l||l->bad||l->io.file[0])return;
    finish_scratch *s=&w->finish;uint8_t root[32];
    wr2_sha256_t *sha=(wr2_sha256_t *)s->chunk;wr2_sha256_init(sha);wr2_sha256_update(sha,&w->blob,sizeof(w->blob));wr2_sha256_final(sha,root);
    for(unsigned slot=0;slot<48;++slot){
        const uint8_t *r=w->blob.record[slot];if(!r[0]||!(seen&(UINT64_C(1)<<slot)))continue;
        uint64_t a=0,b=0;int ar=bank(&l->io,s,w->base,slot,'a',&a);if(ar<0)break;
        int br=bank(&l->io,s,w->base,slot,'b',&b);if(br<0)break;
        uint64_t next=(a>b?a:b);if(next==UINT64_MAX)continue;++next;
        char target=ar?'a':br?'b':a<=b?'a':'b';
        if(!cachepath(s->path,w->base,slot,'d')||read_packet(&l->io,s->path,s->packet)!=0||!envelope(s->packet,1)||
           g32(s->packet+20)!=slot||g32(s->packet+176)!=g32(r+4)||g32(s->packet+180)!=r[1]){if(l->io.file[0])break;continue;}
        cp(s->packet,"W2INST1",8);p32(s->packet+40,(uint32_t)next);p32(s->packet+44,(uint32_t)(next>>32));
        cp(s->packet+48,root,32);cp(s->packet+176,r,104);seal(s->packet);
        if(!envelope(s->packet,0)||!cachepath(s->path,w->base,slot,target))continue;
        if(!write_packet(&l->io,s->path,s->packet)||!verify(&l->io,s)){if(l->io.file[0])break;continue;}
        /* Staging is never eligible. Cleanup is best effort after bank verify. */
        uintptr_t rm=method(0x14);if(rm&&cachepath(s->path,w->base,slot,'d'))((unsigned(*)(void *,const char *))rm)(fs(),s->path);
    }
}
unsigned s12b_layout(unsigned q){switch(q){case 0:return sizeof(s12b_work);case 1:return sizeof(s12b_lease);case 2:return sizeof(s12b_capture);case 3:return sizeof(finish_scratch);default:return 0;}}
