#include "wr2_project.h"

_Static_assert(sizeof(wr2_blob)==WR2_BLOB_BYTES,"disk layout");
static const uint8_t magic[8]={'W','2','P','R','O','J','1',0};
static uint32_t get32(const uint8_t *p) {
    return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}
static void put32(uint8_t *p,uint32_t v) {
    for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(v>>(8*i));
}
static uint32_t crc_more(uint32_t crc,const uint8_t *p,size_t n) {
    while(n--){crc^=*p++;for(unsigned i=0;i<8;++i)crc=(crc>>1)^((0u-(crc&1u))&0x82f63b78u);}
    return crc;
}
uint32_t wr2_crc32c(const uint8_t *p,size_t n) {return ~crc_more(~0u,p,n);}
static uint32_t blob_crc(const wr2_blob *b) {
    return ~crc_more(crc_more(~0u,b->header,28),(const uint8_t *)b->record,sizeof(b->record));
}
uint32_t wr2_identity(const volatile uint8_t *p) {
    /* Deliberately excludes geometry, selected slice, PCM addresses and mode.
     * This is a slot/record sanity binding, not a PCM-content digest. */
    uint8_t key[37]={0};
    key[0]=p[4];
    unsigned n=0;
    while(n<32&&p[5+n])++n;
    /* Exact stock 600C1CBC normalization on load: strip a final .wav in
     * either case. Ignore bytes after NUL, which are not part of the name. */
    if(n>=4&&p[5+n-4]=='.'&&(p[5+n-3]|32u)=='w'&&
       (p[5+n-2]|32u)=='a'&&(p[5+n-1]|32u)=='v')n-=4;
    for(unsigned i=0;i<n;++i)key[1+i]=p[5+i];
    for(unsigned i=0;i<4;++i)key[33+i]=p[0x2c+i];
    return wr2_crc32c(key,sizeof(key));
}
int wr2_record_valid(const uint8_t *r) {
    unsigned n=r[1],active=0;
    uint16_t total[48]={0};
    if(r[0]>1||r[2]>=48||r[3])return 0;
    if(!r[0]){
        if(n||get32(r+4))return 0;
        for(unsigned i=0;i<48;++i)if(r[8+i]!=255||r[56+i])return 0;
        return 1;
    }
    if(!n||n>48)return 0;
    for(unsigned i=0;i<48;++i){
        unsigned owner=r[8+i],weight=r[56+i];
        if(i>=n){if(owner!=255||weight)return 0;continue;}
        if(owner==255){if(weight)return 0;continue;}
        if(owner>=n||r[8+owner]!=owner||weight>100)return 0;
        total[owner]=(uint16_t)(total[owner]+weight);
        if(total[owner]>100)return 0;
        if(owner==i)active=1;
    }
    for(unsigned i=0;i<n;++i)if(r[8+i]==i&&total[i]!=100)return 0;
    return active!=0;
}
void wr2_blob_seal(wr2_blob *b) {
    for(unsigned i=0;i<32;++i)b->header[i]=0;
    for(unsigned i=0;i<8;++i)b->header[i]=magic[i];
    b->header[8]=1;b->header[10]=48;
    put32(b->header+12,WR2_BLOB_BYTES);put32(b->header+16,WR2_RECORD_BYTES);
    put32(b->header+28,blob_crc(b));
}
int wr2_blob_valid(const wr2_blob *b) {
    for(unsigned i=0;i<8;++i)if(b->header[i]!=magic[i])return 0;
    if(b->header[8]!=1||b->header[9]||b->header[10]!=48||b->header[11]||
       get32(b->header+12)!=WR2_BLOB_BYTES||get32(b->header+16)!=WR2_RECORD_BYTES)return 0;
    for(unsigned i=20;i<28;++i)if(b->header[i])return 0;
    if(get32(b->header+28)!=blob_crc(b))return 0;
    for(unsigned i=0;i<48;++i)if(b->record[i][2]!=i||!wr2_record_valid(b->record[i]))return 0;
    return 1;
}
