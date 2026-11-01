/* S11B extension: one independent row, one negative eligibility fence.
 * No positive Workspace checkpoint, per-slot manager, companion or import. */
#include "s12.h"
#include "companion.h"
#define ALL48 UINT64_C(0x0000ffffffffffff)
#define SELECTED 1u
#define POISON 2u
#define WORK 4u
#define COPYING 8u
#define ANC ((volatile anchor *)(uintptr_t)0x2002fc68u)
typedef struct {void *owned;uint32_t flags;} anchor;
typedef struct {
    s12_io io;
    uint8_t row[104],packet[32];
    uint32_t source,destination,complete;
    uint32_t native_fil[2];
} job;
_Static_assert(sizeof(anchor)==8,"heap origin reservation");
_Static_assert(sizeof(job)<=1024,"one small transient Copy job");
static const char fence_path[]="Workspace/wr2_replaced.mask";
static void free_mem(void *p){S12_FN(0x60166364u,void(*)(void *))(p);}
static void zero(void *p,unsigned n){uint8_t *b=p;for(unsigned k=0;k<n;++k)b[k]=0;}
static uint32_t get32(const uint8_t *b){return (uint32_t)b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;}
static void put32(uint8_t *b,uint32_t v){for(unsigned n=0;n<4;++n)b[n]=(uint8_t)(v>>(n*8));}
static uint64_t getmask(const uint8_t *b){return (uint64_t)get32(b+16)|(uint64_t)get32(b+20)<<32;}
static void packet(uint8_t *b,uint64_t mask){
    static const uint8_t magic[8]={'W','2','F','E','N','C','1',0};
    zero(b,32);for(unsigned n=0;n<8;++n)b[n]=magic[n];b[8]=1;b[10]=32;
    put32(b+16,(uint32_t)mask);put32(b+20,(uint32_t)(mask>>32));put32(b+12,wr2_crc32c(b,32));
}
static int valid(uint8_t *b){
    static const uint8_t magic[12]={'W','2','F','E','N','C','1',0,1,0,32,0};
    for(unsigned n=0;n<12;++n)if(b[n]!=magic[n])return 0;
    if(getmask(b)&~ALL48)return 0;
    for(unsigned n=24;n<32;++n)if(b[n])return 0;
    uint32_t crc=get32(b+12);put32(b+12,0);int ok=wr2_crc32c(b,32)==crc;put32(b+12,crc);return ok;
}
/* 0 valid, 1 positively absent, -1 uncertain/corrupt. All uncertainty masks all.
 * An unsuccessful f_open owns no live FIL; unsuccessful close retains it. */
static int read_fence(s12_io *io,uint8_t *b,uint64_t *mask){
    *mask=ALL48;int r=s12_open(io,fence_path,0);
    if(r==4||r==5){*mask=0;return 1;}if(r)return -1;
    int ok=s12_size(io)==32&&s12_read(io,b,32);int closed=s12_close(io);
    if(!ok||!closed||!valid(b))return -1;*mask=getmask(b);return 0;
}
static int commit_fence(job *j){
    uint64_t mask,readback;read_fence(&j->io,j->packet,&mask);
    if(j->io.file[0])return 0;
    mask|=UINT64_C(1)<<j->destination;packet(j->packet,mask);
    if(s12_open(&j->io,fence_path,1))return 0;
    int ok=s12_write(&j->io,j->packet,32);int closed=s12_close(&j->io);
    if(!ok||!closed)return 0;
    return read_fence(&j->io,j->packet,&readback)==0&&readback==mask;
}
void s12_init(void){ANC->owned=0;ANC->flags=0;}
int s12_blocked(void){return (ANC->flags&POISON)!=0;}
static void cancel_native(void){volatile uint8_t *c=(void *)(uintptr_t)S12_FM;c[0x13f]=0;c[0x13d]=0x2e;}
static void drop_job(void){
    job *j=ANC->owned;if(!j)return;
    if(j->io.file[0]||j->native_fil[0]||j->native_fil[1]){ANC->flags|=POISON;return;}
    ANC->owned=0;ANC->flags&=SELECTED;free_mem(j);
}
void s12_poll(void){
    if(ANC->flags&S12B_SAVE){s12b_poll();return;}
    if(!(ANC->flags&POISON)||!ANC->owned)return;
    if(ANC->flags&WORK){s12_io *io=(s12_io *)((uint8_t *)ANC->owned+WR2_BLOB_BYTES);
        if(s12_close(io)){void *p=ANC->owned;ANC->owned=0;ANC->flags&=SELECTED;free_mem(p);}return;
    }
    job *j=ANC->owned;(void)s12_close(&j->io);
    for(unsigned n=0;n<2;++n)if(j->native_fil[n]&&
       S12_FN(0x6009c9a0u,int(*)(void *))((void *)(uintptr_t)j->native_fil[n])==0){
        S12_FN(0x6016a084u,void(*)(void *,unsigned))((void *)(uintptr_t)j->native_fil[n],0x250);j->native_fil[n]=0;
    }
    drop_job();
}
void s12_new(void){
    if(ANC->flags&S12B_SAVE){s12b_cancel();return;}
    ANC->flags&=~SELECTED;
    if(ANC->owned&&!(ANC->flags&WORK)){wr2_slot_clear(((job *)ANC->owned)->destination);drop_job();}
}
void s12_release(void *p,s12_io *io){
    if(!p)return;
    if(io->file[0]){ANC->owned=p;ANC->flags=(ANC->flags&SELECTED)|WORK|POISON;return;}
    free_mem(p);
}
void s12_filter(wr2_blob *b,s12_io *io,uint8_t *scratch){
    if(ANC->flags&SELECTED)return;
    uint64_t mask;read_fence(io,scratch,&mask);
    for(unsigned n=0;n<48;++n)if(mask&(UINT64_C(1)<<n))b->record[n][0]=0;
    /* Already validated cohort; the publisher consults active first. It does
     * not revalidate this transient negative-only view as a disk packet. */
}
void s12_saved(void){
    void *f=*(void **)(uintptr_t)S12_FM;if(!f||s12_blocked())return;
    uintptr_t fn=((const uint32_t *)(uintptr_t)*(uint32_t *)f)[0x14/4];
    if(fn)(void)((unsigned(*)(void *,const char *))fn)(f,fence_path);
}
int s12_pre(unsigned op,unsigned stage){
    if((ANC->flags&S12B_SAVE)&&op!=4)s12b_cancel();
    if(op==3&&stage==0)ANC->flags|=SELECTED;
    if(op!=11){
        if(ANC->owned&&(ANC->flags&COPYING)){job *j=ANC->owned;wr2_slot_clear(j->destination);drop_job();}
        return !s12_blocked();
    }
    if(stage!=0)return !s12_blocked();
    volatile uint8_t *c=(void *)(uintptr_t)S12_FM;
    unsigned src=c[0xae],dst=c[0xaf];
    if(ANC->owned||src>=48||dst>=48||src==dst){cancel_native();return 0;}
    /* The copier owns exactly its source/destination wrappers. Never enter
     * its ignored-preclose path with someone else's live FIL, or acquire a
     * third quarantined object. Native successful/empty closes leave word0=0. */
    for(unsigned n=0;n<2;++n){
        uint32_t *file=*(uint32_t **)(uintptr_t)(0x2001f7a8u+4u*n);
        if(file&&file[0]){cancel_native();return 0;}
    }
    uint32_t error=0;job *j=S12_FN(0x60166550u,void *(*)(void *,unsigned))(&error,sizeof(job));
    if(!j){cancel_native();return 0;}zero(j,sizeof(*j));j->source=src;j->destination=dst;
    ANC->owned=j;ANC->flags=(ANC->flags&SELECTED)|COPYING;
    wr2_slot_snapshot(j->row,src);
    if(!commit_fence(j)){cancel_native();drop_job();return 0;}
    wr2_slot_clear(dst);return 1;
}
void s12_post(unsigned op,unsigned stage){
    volatile uint8_t *c=(void *)(uintptr_t)S12_FM;
    if(op==1&&stage==5&&c[0x13f]!=1)ANC->flags&=~SELECTED;
    if(op==3&&c[0x13f]!=3&&c[0x13f]!=1)ANC->flags&=~SELECTED;
    if(op!=11||!ANC->owned||!(ANC->flags&COPYING))return;
    job *j=ANC->owned;
    if(ANC->flags&POISON){wr2_slot_clear(j->destination);cancel_native();return;}
    if(c[0x13f]!=11){
        if(c[0x13d]==0x20&&j->complete&&j->row[0])
            (void)wr2_slot_publish(j->row,j->destination);
        drop_job();
    }
}
/* The exact native copier remains authoritative. Length equality additionally
 * rejects its early-EOF "done" result; the native loaded/header check follows. */
extern unsigned s12_copier_native(uint32_t *,const char *,const char *);
unsigned s12_copier(uint32_t *ctx,const char *src,const char *dst){
    s12b_export_begin(ctx,dst);
    unsigned r=s12_copier_native(ctx,src,dst);
    s12b_export_end(ctx,r);
    if((ANC->flags&COPYING)&&ANC->owned&&*(volatile uint8_t *)(uintptr_t)(S12_FM+0x13f)==11){
        job *j=ANC->owned;
        if(r==2)j->complete=ctx[3]==ctx[4]&&ctx[4]>=0x188;
        if(r==3||r==4)j->complete=0;
    }return r;
}
/* Scoped native close: ordinary S11B Open/Save use the untouched trampoline.
 * During Copy a failed live close transfers FIL ownership into this one job.
 * A verified never-opened cleanup is the sole exception (original native site). */
extern int s12_close_native(uint32_t *);
int s12_native_close(uint32_t *file,uintptr_t caller){
    if(ANC->flags&S12B_SAVE)return s12b_native_close(file,caller);
    if(!(ANC->flags&COPYING)||!ANC->owned)return s12_close_native(file);
    if(!file||!file[0])return 0;
    job *j=ANC->owned;uint32_t fil=file[0];
    int r=S12_FN(0x6009c9a0u,int(*)(void *))((void *)(uintptr_t)fil);
    if(r==0||(r==9&&caller==0x6009e79du)){
        S12_FN(0x6016a084u,void(*)(void *,unsigned))((void *)(uintptr_t)fil,0x250);
        zero(file,259);return r==0;
    }
    for(unsigned n=0;n<2;++n)if(!j->native_fil[n]){j->native_fil[n]=fil;file[0]=0;break;}
    ANC->flags|=POISON;return 0;
}

/* Read-only ABI telemetry; no state substitution or budget model. */
unsigned s12_layout(unsigned q){
    switch(q){case 0:return sizeof(job);case 1:return sizeof(s12_io);
    case 2:return offsetof(job,row);case 3:return offsetof(job,packet);
    case 4:return offsetof(job,native_fil);case 5:return offsetof(s12_io,fil);
    case 6:return sizeof(anchor);default:return 0;}
}
