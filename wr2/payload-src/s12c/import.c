/* A single foreground Import cohort. Saved root remains sole positive source
 * authority. Native descriptor -> native PCM -> Workspace -> independent row.
 * No task/worker, PCM pointer cache, new fixed RAM or source-project writes. */
#include "import.h"
#define FN S12_FN
#define ANC S12B_ANC
#define INST ((volatile uint8_t *)(uintptr_t)0x20200000u)
typedef struct {
    s12b_work work;
    uint64_t replaced,header_bad,published;
    unsigned destination,source,scope,scope_slot,native_bad,candidate,pcm_ok,workspace_ok,header_ok,verify,verified_bytes;
    unsigned type,frames,channels,pcm_bytes,identity,count;
    uint8_t leaf[64],row[104],root_sha[32],bank_sha[2][32],actual_sha[32];
    uint32_t native_fil[2];
    char workspace[256];
} import_job;
_Static_assert(offsetof(import_job,work.io.fil)==5288,"inherited FIL address/alignment");
_Static_assert(sizeof(import_job)<8192,"one bounded transient Import cohort");
static import_job *job(void){return (ANC->flags&S12C_IMPORT)?ANC->owned:0;}
static void zero(void *p,unsigned n){uint8_t *b=p;while(n--)*b++=0;}
static void cp(void *d,const void *s,unsigned n){uint8_t *a=d;const uint8_t *b=s;while(n--)*a++=*b++;}
static int eq(const void *a,const void *b,unsigned n){const uint8_t *x=a,*y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static unsigned len(const char *s,unsigned n){unsigned k=0;while(k<n&&s[k])++k;return k;}
static uint32_t g32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void p32(uint8_t *p,uint32_t v){for(unsigned n=0;n<4;++n)p[n]=(uint8_t)(v>>(n*8));}
static int text(char *d,const char *s,unsigned cap){unsigned n=len(s,cap);if(n==cap)return 0;cp(d,s,n+1);return 1;}
static int pti(const char *s){unsigned n=len(s,256);return n>=4&&n<256&&s[n-4]=='.'&&(s[n-3]|32)=='p'&&(s[n-2]|32)=='t'&&(s[n-1]|32)=='i';}
static int importing(void){return *(volatile uint8_t *)(uintptr_t)(S12_FM+0x13f)==10;}
static int path(import_job *j,const char *tail){
    unsigned a=len(j->work.base,256),b=len(tail,256);if(a+b>=256)return 0;
    cp(j->work.finish.path,j->work.base,a);cp(j->work.finish.path+a,tail,b+1);return 1;
}
static int quiet(import_job *j){return !j->work.io.file[0]&&!j->native_fil[0]&&!j->native_fil[1];}
static void release(import_job *j){
    if(!quiet(j)){ANC->flags=(ANC->flags&1u)|S12C_IMPORT|S12B_POISON;return;}
    ANC->owned=0;ANC->flags&=1u;FN(0x60166364u,void(*)(void *))(j);
}
void s12c_poll(void){
    import_job *j=job();if(!j||!(ANC->flags&S12B_POISON))return;
    (void)s12_close(&j->work.io);
    for(unsigned n=0;n<2;++n)if(j->native_fil[n]&&FN(0x6009c9a0u,int(*)(void *))((void *)(uintptr_t)j->native_fil[n])==0){
        FN(0x6016a084u,void(*)(void *,unsigned))((void *)(uintptr_t)j->native_fil[n],0x250);j->native_fil[n]=0;
    }
    release(j);
}
void s12c_cancel(void){import_job *j=job();if(j){ANC->flags&=~1u;release(j);}}
static void stop(void){volatile uint8_t *f=(void *)(uintptr_t)S12_FM;f[0x13f]=0;f[0x13d]=0x2e;}
int s12c_pre(unsigned stage){
    if(stage!=0)return 1;
    volatile uint8_t *f=(void *)(uintptr_t)S12_FM;
    unsigned first=f[0x1a],last=f[0x1b],dest=f[0x1c];uint64_t mask=0;
    if(first>last||last>=48||dest>=48)return 1; /* Inactive invalid native request. */
    const uint32_t *leaves=(const uint32_t *)(uintptr_t)(S12_FM+0xab8);
    for(unsigned n=first;n<=last;++n)if(leaves[n]&&pti((const char *)(uintptr_t)leaves[n])){
        if(dest+n-first>=48){stop();return 0;}mask|=UINT64_C(1)<<(dest+n-first);
    }
    if(!mask)return 1; /* Native Import Sample: no WR2 allocation, fence or clear. */
    uint32_t *native=*(uint32_t **)(uintptr_t)0x2001f7a8u;
    if(ANC->owned||(native&&native[0])){stop();return 0;}
    uint32_t err=0;import_job *j=FN(0x60166550u,void *(*)(void *,unsigned))(&err,sizeof(*j));
    if(!j){stop();return 0;}zero(j,sizeof(*j));j->destination=48;j->scope_slot=48;j->replaced=mask;
    ANC->owned=j;ANC->flags=(ANC->flags&1u)|S12C_IMPORT;
    if(!s12_fence_mask(&j->work.io,j->work.finish.packet,mask)){stop();release(j);return 0;}
    for(unsigned n=0;n<48;++n)if(mask&(UINT64_C(1)<<n))wr2_slot_clear(n);
    return 1;
}
void s12c_post(unsigned op,unsigned stage){
    (void)stage;import_job *j=job();if(!j)return;
    if(op!=10){s12c_cancel();return;}
    /* A native failed close aborts before another foreground stage can open
     * more native FILs. Header-loop reader calls also refuse once latched. */
    if(j->native_bad)stop();
    if(!importing())release(j);
}
extern int s12_close_native(uint32_t *);
int s12c_native_close(uint32_t *file,uintptr_t caller){
    import_job *j=job();if(!j||!j->scope||!importing())return s12_close_native(file);
    if(!file||!file[0])return 0;uint32_t fil=file[0];
    int r=FN(0x6009c9a0u,int(*)(void *))((void *)(uintptr_t)fil);
    if(r==0||(r==9&&caller==0x6009e79du)){
        FN(0x6016a084u,void(*)(void *,unsigned))((void *)(uintptr_t)fil,0x250);zero(file,259);return r==0;
    }
    j->native_bad=1;j->candidate=0;
    for(unsigned n=0;n<2;++n)if(!j->native_fil[n]){j->native_fil[n]=fil;file[0]=0;return 0;}
    return 0; /* At most two close calls in the admitted native helper. */
}
void s12c_header_begin(const uint32_t *desc,unsigned slot){
    (void)desc;import_job *j=job();if(j)j->scope_slot=slot;
}
unsigned s12c_header_read(uint32_t *ctx,const char *source,void *out,unsigned bytes,unsigned param){
    import_job *j=job();if(j&&j->native_bad)return ~0u;
    if(j)j->scope=1;
    unsigned r=FN(0x600c40d4u,unsigned(*)(uint32_t *,const char *,void *,unsigned,unsigned))(ctx,source,out,bytes,param);
    if(j){j->scope=0;if(j->scope_slot<48&&(r!=2||ctx[3]!=392||bytes!=392||j->native_bad))j->header_bad|=UINT64_C(1)<<j->scope_slot;if(j->native_bad)return ~0u;}
    return r;
}
/* Typed absence is essential: an exists() boolean can hide an IO error. */
static int absent(import_job *j,const char *tail){
    if(!path(j,tail))return 0;int r=s12_open(&j->work.io,j->work.finish.path,0);
    if(r==4||r==5)return 1;if(!r)(void)s12_close(&j->work.io);return 0;
}
static int guards(import_job *j){return absent(j,"/wr2_groups.wr2.pending")&&absent(j,"/wr2_groups.wr2.tmp");}
static int read_exact(import_job *j,const char *p,void *data,unsigned n){
    if(s12_open(&j->work.io,p,0))return 0;
    int ok=s12_size(&j->work.io)==n&&s12_read(&j->work.io,data,n);int closed=s12_close(&j->work.io);return ok&&closed;
}
static void root_hash(import_job *j,uint8_t out[32]){
    wr2_sha256_t *s=(wr2_sha256_t *)j->work.finish.chunk;wr2_sha256_init(s);wr2_sha256_update(s,&j->work.blob,WR2_BLOB_BYTES);wr2_sha256_final(s,out);
}
static int source_root(import_job *j){
    return guards(j)&&path(j,"/wr2_groups.wr2")&&read_exact(j,j->work.finish.path,&j->work.blob,WR2_BLOB_BYTES)&&wr2_blob_valid(&j->work.blob)&&guards(j);
}
static int canonical_dir(const char *p,unsigned n){
    if(!n||p[n-1]=='/')return 0;unsigned first=0;
    if(p[0]=='/')first=1;
    else if(n>=3&&p[0]>='0'&&p[0]<='9'&&p[1]==':'&&p[2]=='/')first=3;
    if(first==n)return 0;unsigned start=first;
    for(unsigned k=first;k<=n;++k){if(k<n&&(p[k]=='\\'||p[k]==':'))return 0;
        if(k==n||p[k]=='/'){unsigned size=k-start;if(!size||(size==1&&p[start]=='.')||(size==2&&p[start]=='.'&&p[start+1]=='.'))return 0;start=k+1;}}
    if(n-first==9){static const char ws[]="workspace";unsigned same=1;for(unsigned k=0;k<9;++k)if((p[first+k]|32)!=ws[k])same=0;if(same)return 0;}
    return 1;
}

static int descriptor(import_job *j,const uint32_t *d,int fresh){
    if(((const uint8_t *)d)[8]!=1||d[3]!=392)return 0;
    const char *dir=(const char *)(uintptr_t)d[0],*leaf=(const char *)(uintptr_t)d[1];
    unsigned a=len(dir,256),b=len(leaf,64);if(a<=12||a==256||!eq(dir+a-12,"/instruments",12)||!canonical_dir(dir,a-12)||b>=64||!pti(leaf))return 0;
    unsigned k=0,slot=0;while(k<b&&leaf[k]>='0'&&leaf[k]<='9'&&k<2){slot=slot*10+(unsigned)(leaf[k]-'0');++k;}
    if(!k||leaf[0]=='0'||leaf[k]!=' '||slot<1||slot>48)return 0;
    for(unsigned n=0;n<b;++n)if(leaf[n]=='/'||leaf[n]=='\\')return 0;
    if(fresh){cp(j->work.base,dir,a-12);j->work.base[a-12]=0;zero(j->leaf,64);cp(j->leaf,leaf,b);j->source=slot-1;}
    else if(j->source!=slot-1||len(j->work.base,256)!=a-12||!eq(j->work.base,dir,a-12)||!eq(j->leaf,leaf,b+1))return 0;
    return 1;
}
static int metadata(import_job *j){
    volatile uint8_t *i=INST+j->destination*0x174;
    unsigned ch=FN(0x600bce20u,unsigned(*)(void *,unsigned))((void *)(uintptr_t)S12_FM,j->destination)?2:1;
    return i[0]==1&&i[4]==j->type&&ch==j->channels&&*(volatile uint32_t *)(i+0x2c)==j->frames&&i[0x168]==j->count&&wr2_identity(i)==j->identity;
}
extern int s12b_packet_valid(uint8_t *);
static void candidate(import_job *j,const uint32_t *d,unsigned slot){
    j->destination=slot;j->candidate=0;j->pcm_ok=0;j->workspace_ok=0;j->header_ok=0;j->verify=0;j->verified_bytes=0;j->workspace[0]=0;
    if(j->native_bad||j->work.io.file[0]||j->header_bad&(UINT64_C(1)<<slot)||!descriptor(j,d,1)||!source_root(j))return;
    const uint8_t *r=j->work.blob.record[j->source];if(!r[0])return;cp(j->row,r,104);root_hash(j,j->root_sha);
    volatile uint8_t *i=INST+slot*0x174;j->type=i[4];j->frames=*(volatile uint32_t *)(i+0x2c);j->channels=FN(0x600bce20u,unsigned(*)(void *,unsigned))((void *)(uintptr_t)S12_FM,slot)?2:1;j->count=i[0x168];j->identity=g32(r+4);
    if(j->type>1||!j->frames||j->frames>0x3fffffff/j->channels||!metadata(j)||j->count!=r[1])return;
    j->pcm_bytes=j->frames*j->channels*2;
    for(unsigned bank=0;bank<2;++bank){
        char tail[]="/wr2_companions/instrument_01.a.w2i";tail[27]=(char)('0'+(j->source+1)/10);tail[28]=(char)('0'+(j->source+1)%10);tail[30]=(char)('a'+bank);
        if(!path(j,tail))continue;uint8_t *p=j->work.finish.packet;
        int ok=read_exact(j,j->work.finish.path,p,288);
        if(j->work.io.file[0]){j->candidate=0;return;}
        if(!ok||!s12b_packet_valid(p)||g32(p+20)!=j->source||g32(p+24)!=j->type||g32(p+28)!=j->frames||g32(p+32)!=j->channels||g32(p+36)!=j->pcm_bytes||!eq(p+48,j->root_sha,32)||!eq(p+112,j->leaf,64)||!eq(p+176,j->row,104))continue;
        cp(j->bank_sha[bank],p+80,32);j->candidate|=1u<<bank;
    }
}
unsigned s12c_pcm_read(uint32_t *ctx,const char *source,unsigned offset,void *out,unsigned bytes,unsigned *frames){
    import_job *j=job();if(j&&j->native_bad)return 3;
    if(j)j->scope=2;
    unsigned r=FN(0x600c4c68u,unsigned(*)(uint32_t *,const char *,unsigned,void *,unsigned,unsigned *))(ctx,source,offset,out,bytes,frames);
    if(j){j->scope=0;if(r==2)j->pcm_ok=!j->native_bad&&offset==392&&ctx[3]==ctx[4]&&ctx[3]==bytes&&bytes==j->pcm_bytes;else if(r>=3)j->pcm_ok=0;}
    return r;
}
unsigned s12c_import_pcm(void *ctx,unsigned slot,const uint32_t *d){
    import_job *j=job();int active=j&&slot<48&&j->replaced&(UINT64_C(1)<<slot)&&((const uint8_t *)d)[8]==1;
    if(active){if(j->destination!=slot)candidate(j,d,slot);else if(!descriptor(j,d,0))j->candidate=0;}
    return FN(0x601156ecu,unsigned(*)(void *,unsigned,const uint32_t *))(ctx,slot,d);
}
unsigned s12c_workspace_header(void *ctx,unsigned slot){
    import_job *j=job();int active=j&&slot<48&&(j->replaced&(UINT64_C(1)<<slot));
    if(active)j->scope=3;
    unsigned r=FN(0x600c1a28u,unsigned(*)(void *,unsigned))(ctx,slot);
    if(active){j->scope=0;j->header_ok=r==1&&!j->native_bad;}return r;
}
unsigned s12c_pcm_write(uint32_t *ctx,const void *data,unsigned bytes,const char *p,unsigned offset,unsigned flag){
    import_job *j=job();int active=j&&j->scope==4;
    if(active&&((uint8_t *)ctx)[4]==0&&!text(j->workspace,p,256))j->candidate=0;
    unsigned r=FN(0x600c4d68u,unsigned(*)(uint32_t *,const void *,unsigned,const char *,unsigned,unsigned))(ctx,data,bytes,p,offset,flag);
    if(active){if(r==2)j->workspace_ok=!j->native_bad&&offset==392&&ctx[3]==ctx[4]&&ctx[3]==bytes&&bytes==j->pcm_bytes;else if(r>=3)j->workspace_ok=0;}
    return r;
}
/* Import-only foreground continuation. At most 32,640 PCM bytes are read
 * and hashed per service invocation. Ordinary Open never enters this path. */
static unsigned verify_step(import_job *j){
    s12_io *io=&j->work.io;uint8_t *buf=(uint8_t *)&j->work.blob;wr2_sha256_t *s=(wr2_sha256_t *)j->work.finish.chunk;
    if(!j->verify){
        if(!j->candidate||!j->pcm_ok||!j->header_ok||!j->workspace_ok||j->native_bad||io->file[0]||!j->workspace[0]||!metadata(j)||s12_open(io,j->workspace,0))return 1;
        int ok=s12_size(io)==392+j->pcm_bytes&&s12_read(io,buf,392);
        if(ok)ok=g32(buf)==0x14954u&&buf[12]==0x74&&buf[13]==1&&buf[16]==1&&buf[20]==j->type&&g32(buf+60)==j->frames&&buf[376]==j->count&&wr2_identity(buf+16)==j->identity;
        if(!ok){(void)s12_close(io);return 1;}
        p32(j->work.finish.packet,j->type);p32(j->work.finish.packet+4,j->frames);p32(j->work.finish.packet+8,j->channels);
        wr2_sha256_init(s);wr2_sha256_update(s,j->work.finish.packet,12);j->verified_bytes=0;j->verify=1;
    }
    unsigned budget=32640;int ok=1;
    while(j->verified_bytes<j->pcm_bytes&&budget&&ok){unsigned n=j->pcm_bytes-j->verified_bytes;if(n>512)n=512;if(n>budget)n=budget;
        ok=s12_read(io,buf,n);if(ok)wr2_sha256_update(s,buf,n);j->verified_bytes+=n;budget-=n;}
    if(ok&&j->verified_bytes<j->pcm_bytes)return 0;
    j->verify=0;int closed=s12_close(io);if(!ok||!closed)return 1;
    wr2_sha256_final(s,j->actual_sha);
    if(!metadata(j)||!(((j->candidate&1u)&&eq(j->actual_sha,j->bank_sha[0],32))||((j->candidate&2u)&&eq(j->actual_sha,j->bank_sha[1],32)))||!source_root(j))return 1;
    uint8_t hash[32];root_hash(j,hash);
    if(eq(hash,j->root_sha,32)&&eq(j->work.blob.record[j->source],j->row,104)&&metadata(j)&&wr2_slot_publish(j->row,j->destination))j->published|=UINT64_C(1)<<j->destination;
    return 1;
}
unsigned s12c_workspace_write(void *ctx,unsigned slot){
    import_job *j=job();int active=j&&slot<48&&j->destination==slot&&(j->replaced&(UINT64_C(1)<<slot));
    if(active&&j->verify)return verify_step(j);
    if(active)j->scope=4;
    unsigned r=FN(0x600c3478u,unsigned(*)(void *,unsigned))(ctx,slot);
    if(active){j->scope=0;if(r&&!j->native_bad)return verify_step(j);}
    return r;
}

unsigned s12c_layout(unsigned q){switch(q){case 0:return sizeof(import_job);case 1:return offsetof(import_job,work.io);case 2:return offsetof(import_job,native_fil);case 3:return offsetof(import_job,destination);case 4:return offsetof(import_job,scope);case 5:return sizeof(s12b_anchor);default:return 0;}}
