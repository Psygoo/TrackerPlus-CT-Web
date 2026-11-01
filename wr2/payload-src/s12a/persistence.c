/* Small project-only persistence. No Stage 10 manager/sidecar/hash code is
 * linked. All storage work executes in the original foreground file service. */
#include "s12.h"
#define FILEMAN 0x2001e488u
#define FN(addr,type) ((type)(uintptr_t)((addr)|1u))
typedef struct {
    wr2_blob blob;
    s12_io io;
    char base[256],path[256],temporary[256],guard[256];
} work;
_Static_assert(offsetof(work,io.fil)%8==0,"preserve stock FIL alignment");
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
    if(s12_blocked())return 0;
    work *w=FN(0x60166550u,alloc_fn)(&error,(uint32_t)sizeof(work));
    if(w)w->io.file[0]=0;return w;
}
static void release(work *w) {if(w)s12_release(w,&w->io);}
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
static int write_bytes(work *w,const char *p,const void *data,unsigned bytes) {
    if(s12_open(&w->io,p,1))return 0;
    int ok=s12_write(&w->io,data,bytes);int closed=s12_close(&w->io);return ok&&closed;
}
static int read_blob(work *w,const char *p) {
    if(s12_open(&w->io,p,0))return 0;
    int ok=s12_size(&w->io)==WR2_BLOB_BYTES&&s12_read(&w->io,&w->blob,WR2_BLOB_BYTES);
    int closed=s12_close(&w->io);return ok&&closed&&wr2_blob_valid(&w->blob);
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
    if(!ok)failed();else s12_saved();return ok;
}
int wr2_restore(void) {
    work *w=acquire();int ok=0;
    wr2_clear();
    if(w){if(project_base(w->base)&&paths(w,w->base)&&guarded_read(w)){s12_filter(&w->blob,&w->io,(uint8_t *)w->temporary);
            if(!w->io.file[0]){wr2_publish(&w->blob);ok=1;}}release(w);}
    return ok;
}
void *wr2_fileop_step(void *ctx) {
    volatile uint8_t *c=(volatile uint8_t *)ctx;
    unsigned op=0,stage=255;
    int known=(uintptr_t)ctx==FILEMAN;
    if(known){
        s12_poll();
        if(s12_blocked()){
            /* A retained FIL must never turn a newly admitted operation into
             * an endless Loading wait. Refuse before its next native step. */
            unsigned pending=c[0x13f];
            if(pending){c[0x13f]=0;c[0x13d]=pending==4?0x2d:0x2e;}
            return ctx;
        }
        op=c[0x13f];
        if(op==1)stage=c[0x38];
        if(op==3)stage=c[0x68];
        if(op==4)stage=c[0x90];
        if(op==11)stage=c[0xac];
        if(!s12_pre(op,stage))return ctx;
        if((op==1||op==3)&&stage==0)wr2_clear();
        if(op==4&&stage==0)(void)wr2_save_begin();
    }
    void *result=FN(0x600bca88u,void *(*)(void *))(ctx);
    if(known){
        if(op==1&&stage==5&&c[0x13f]!=1)(void)wr2_restore();
        if(op==4&&stage==4&&c[0x13f]==0)(void)wr2_save_finish();
        s12_post(op,stage);
    }
    return result;
}

unsigned s12_work_bytes(void){return sizeof(work);}
