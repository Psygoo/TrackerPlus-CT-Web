/* Small project-only persistence. No Stage 10 manager/sidecar/hash code is
 * linked. All storage work executes in the original foreground file service. */
#include "wr2_project.h"
#define FILEMAN 0x2001e488u
#define FN(addr,type) ((type)(uintptr_t)((addr)|1u))
typedef struct {
    wr2_blob blob;
    uint32_t file[65];
    _Alignas(8) uint32_t fil[0x250/4];
    char base[256],path[256],temporary[256],guard[256];
} work;
_Static_assert(offsetof(work,fil)%8==0,"preserve stock FIL alignment");
typedef void *(*alloc_fn)(void *,uint32_t);
typedef void (*free_fn)(void *);
typedef uint32_t (*path_fn)(void *,const char *);
typedef uint32_t (*rename_fn)(void *,const char *,const char *);
typedef int (*fat_open_fn)(void *,const char *,uint32_t);
typedef uint32_t (*size_fn)(void *,void *);
typedef int32_t (*transfer_fn)(void *,void *,uint32_t,void *);
typedef int (*close_fn)(void *);
static void *fs(void) {return *(void **)(uintptr_t)FILEMAN;}
static uintptr_t method(unsigned offset) {
    void *f=fs();return f?((const uint32_t *)(uintptr_t)*(uint32_t *)f)[offset/4]:0;
}
static work *acquire(void) {
    /* Bypass stock malloc-or-halt. Reviewed nano allocator uses reent only for
     * errno on failure; this non-null local supplies that word. */
    uint32_t error=0;
    return FN(0x60166550u,alloc_fn)(&error,(uint32_t)sizeof(work));
}
static void release(work *w) {if(w)FN(0x60166364u,free_fn)(w);}
static size_t bounded(const char *p,size_t n) {size_t i=0;while(i<n&&p[i])++i;return i;}
static void copy(char *d,const char *s,size_t n) {for(size_t i=0;i<n;++i)d[i]=s[i];}
static int project_base(char *out) {
    const char *dir=(const char *)(uintptr_t)(FILEMAN+0x241u);
    const char *name=(const char *)(uintptr_t)(FILEMAN+0x340u);
    size_t a=bounded(dir,255),b=bounded(name,33);
    if(!a||a==255||!b||b==33||a+1+b>=256)return 0;
    copy(out,dir,a);out[a++]='/';copy(out+a,name,b);out[a+b]=0;return 1;
}
static int paths(work *w,const char *base) {
    static const char leaf[]="/wr2_groups.wr2";
    size_t n=bounded(base,256),k=sizeof(leaf)-1;
    if(n==256||n+k+sizeof(".pending")>256)return 0;
    copy(w->path,base,n);copy(w->path+n,leaf,k+1);
    copy(w->temporary,w->path,n+k);copy(w->temporary+n+k,".tmp",5);
    copy(w->guard,w->path,n+k);copy(w->guard+n+k,".pending",9);
    return 1;
}
static int exists(const char *p) {
    uintptr_t fn=method(8);return fn?((path_fn)fn)(fs(),p)!=0:0;
}
static int remove_file(const char *p) {
    uintptr_t fn=method(0x14);return fn&&((path_fn)fn)(fs(),p)!=0;
}
static void *open_file(work *w,const char *p,unsigned mode) {
    for(unsigned i=0;i<65;++i)w->file[i]=0;
    for(unsigned i=0;i<0x250/4;++i)w->fil[i]=0;
    /* Same backend/path/flags as 6009E74C and its mode table, but caller-owned
     * FIL storage avoids that wrapper's malloc-or-halt allocation. */
    if(FN(0x6009bfe4u,fat_open_fn)(w->fil,p,mode==3?0x0bu:1u)!=0)return 0;
    w->file[0]=(uint32_t)(uintptr_t)w->fil;
    return w->file;
}
static int close_file(void *h) {
    /* Backend FRESULT: zero succeeds. This is NOT the boolean 6009E704 API.
     * Do not call the latter: it would free the embedded FIL buffer. */
    uint32_t *file=h;
    int rc=FN(0x6009c9a0u,close_fn)((void *)(uintptr_t)file[0]);
    file[0]=0;return rc==0;
}
static int write_bytes(work *w,const char *p,const void *data,unsigned bytes) {
    void *h=open_file(w,p,3);uintptr_t fn=method(0x34);int ok=1;
    if(!h)return 0;
    if(!fn)ok=0;
    for(unsigned done=0;ok&&done<bytes;){
        unsigned n=bytes-done;if(n>512)n=512;
        int32_t rc=((transfer_fn)fn)(fs(),(uint8_t *)(uintptr_t)data+done,n,h);
        if(rc!=(int32_t)n)ok=0;else done+=n;
    }
    int closed=close_file(h);return ok&&closed;
}
static int read_blob(work *w,const char *p) {
    void *h=open_file(w,p,0);uintptr_t sz=method(0x24),rd=method(0x30);int ok=0;
    if(!h)return 0;
    if(sz&&rd&&((size_fn)sz)(fs(),h)==WR2_BLOB_BYTES){
        ok=1;
        for(unsigned done=0;ok&&done<WR2_BLOB_BYTES;){
            unsigned n=WR2_BLOB_BYTES-done;if(n>512)n=512;
            int32_t rc=((transfer_fn)rd)(fs(),(uint8_t *)&w->blob+done,n,h);
            if(rc!=(int32_t)n)ok=0;else done+=n;
        }
    }
    int closed=close_file(h);
    return ok&&closed&&wr2_blob_valid(&w->blob);
}
static int guarded_read(work *w) {return !exists(w->guard)&&read_blob(w,w->path);}
static int guard(work *w) {
    static const uint8_t tag[8]={'W','2','P','E','N','D','1',0};
    return write_bytes(w,w->guard,tag,sizeof(tag));
}
static int publish_file(work *w) {
    uintptr_t rename=method(0x0c);
    uint32_t expected=(uint32_t)w->blob.header[28]|(uint32_t)w->blob.header[29]<<8|
                      (uint32_t)w->blob.header[30]<<16|(uint32_t)w->blob.header[31]<<24;
    if((!exists(w->guard)&&!guard(w))||!write_bytes(w,w->temporary,&w->blob,sizeof(w->blob))||
       !read_blob(w,w->temporary)||
       ((uint32_t)w->blob.header[28]|(uint32_t)w->blob.header[29]<<8|
        (uint32_t)w->blob.header[30]<<16|(uint32_t)w->blob.header[31]<<24)!=expected)return 0;
    if(exists(w->path)&&!remove_file(w->path))return 0;
    if(!rename||!((rename_fn)rename)(fs(),w->temporary,w->path))return 0;
    return remove_file(w->guard);
}
static void failed(void) {
    /* Leave the existing project-dirty indication set. After stock Save has
     * completed, use its existing Save-error status instead of advertising
     * complete success when WR2 failed. Never interrupt stock progress. */
    *(volatile uint8_t *)(uintptr_t)0x202046f4u|=1u;
    volatile uint8_t *c=(volatile uint8_t *)(uintptr_t)FILEMAN;
    if(c[0x13f]==0u&&c[0x13d]==0x1bu)c[0x13d]=0x2du;
}
int wr2_save_begin(void) {
    work *w=acquire();int ok=0;
    if(w){if(project_base(w->base)&&paths(w,w->base))ok=guard(w);release(w);}
    if(!ok)failed();return ok;
}
int wr2_save_finish(void) {
    work *w=acquire();int ok=0;
    if(w){
        wr2_snapshot(&w->blob);
        if(project_base(w->base)&&paths(w,w->base)&&publish_file(w))ok=1;
        release(w);
    }
    if(!ok)failed();return ok;
}
int wr2_restore(void) {
    work *w=acquire();int ok=0;
    wr2_clear();
    if(w){if(project_base(w->base)&&paths(w,w->base)&&guarded_read(w)){wr2_publish(&w->blob);ok=1;}release(w);}
    return ok;
}
void *wr2_fileop_step(void *ctx) {
    volatile uint8_t *c=(volatile uint8_t *)ctx;
    unsigned op=0,stage=255;
    int known=(uintptr_t)ctx==FILEMAN;
    if(known){
        op=c[0x13f];
        if(op==1)stage=c[0x38];
        if(op==3)stage=c[0x68];
        if(op==4)stage=c[0x90];
        if((op==1||op==3)&&stage==0)wr2_clear();
        if(op==4&&stage==0)(void)wr2_save_begin();
    }
    void *result=FN(0x600bca88u,void *(*)(void *))(ctx);
    if(known){
        if(op==1&&stage==5&&c[0x13f]!=1)(void)wr2_restore();
        if(op==4&&stage==4&&c[0x13f]==0)(void)wr2_save_finish();
    }
    return result;
}
