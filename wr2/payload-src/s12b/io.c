#include "s12.h"
static void *fs(void){return *(void **)(uintptr_t)S12_FM;}
static uintptr_t method(unsigned n){void *f=fs();return f?((const uint32_t *)(uintptr_t)*(uint32_t *)f)[n/4]:0;}
int s12_open(s12_io *i,const char *p,unsigned mode){
    if(i->file[0])return -1;
    for(unsigned n=0;n<65;++n)i->file[n]=0;
    for(unsigned n=0;n<0x250/4;++n)i->fil[n]=0;
    int r=S12_FN(0x6009bfe4u,int(*)(void *,const char *,uint32_t))(i->fil,p,mode?0x0bu:1u);
    if(!r)i->file[0]=(uint32_t)(uintptr_t)i->fil;
    return r;
}
int s12_close(s12_io *i){
    if(!i->file[0])return 1;
    int r=S12_FN(0x6009c9a0u,int(*)(void *))((void *)(uintptr_t)i->file[0]);
    if(!r)i->file[0]=0; /* Only positive close releases ownership. */
    return r==0;
}
unsigned s12_size(s12_io *i){uintptr_t f=method(0x24);return f?((unsigned(*)(void *,void *))f)(fs(),i->file):~0u;}
static int transfer(s12_io *i,void *p,unsigned n,unsigned off){
    uintptr_t f=method(off);if(!f||!i->file[0])return 0;
    for(unsigned done=0;done<n;){unsigned k=n-done;if(k>512)k=512;
        int r=((int(*)(void *,void *,unsigned,void *))f)(fs(),(uint8_t *)p+done,k,i->file);
        if(r!=(int)k)return 0;done+=k;
    }return 1;
}
int s12_read(s12_io *i,void *p,unsigned n){return transfer(i,p,n,0x30);}
int s12_write(s12_io *i,const void *p,unsigned n){return transfer(i,(void *)(uintptr_t)p,n,0x34);}
