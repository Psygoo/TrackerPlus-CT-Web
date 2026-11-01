#include "s12.h"
#define INST ((volatile uint8_t *)(uintptr_t)0x20200000u)
#define ROWS ((volatile uint8_t *)(uintptr_t)0x20205068u)
#define EDIT ((volatile uint32_t *)(uintptr_t)0x202063e8u)
#define MAGIC 0x31524757u
static void barrier(void) {__asm__ volatile("dmb sy":::"memory");}
void wr2_clear(void) {
    /* Validity only. No CT2, RNG, geometry, instrument or UI-pointer access. */
    for(unsigned i=0;i<48;++i)*(volatile uint32_t *)(ROWS+i*104)=0;
    *EDIT=0;barrier();
}
void wr2_boot(void) {s12_init();wr2_clear();}
void wr2_new_clear(void) {s12_new();wr2_clear();}
static void empty(uint8_t *r,unsigned slot) {
    for(unsigned i=0;i<104;++i)r[i]=0;
    r[2]=(uint8_t)slot;
    for(unsigned i=0;i<48;++i)r[8+i]=255;
}
void wr2_slot_clear(unsigned s) {
    if(s>=48)return;
    *(volatile uint32_t *)(ROWS+s*104)=0;
    volatile uint8_t *edit=(volatile uint8_t *)EDIT;
    if(edit[5]==s)*EDIT=0;
    barrier();
}
void wr2_slot_snapshot(uint8_t *r,unsigned s) {
    empty(r,s);if(s>=48)return;
    volatile uint8_t *inst=INST+s*0x174,*row=ROWS+s*104;
    if(inst[0]!=1||*(volatile uint32_t *)row!=MAGIC||row[5]!=1||
       row[4]!=inst[0x168]||!row[4]||row[4]>48)return;
    r[0]=1;r[1]=row[4];uint32_t key=wr2_identity(inst);
    for(unsigned i=0;i<4;++i)r[4+i]=(uint8_t)(key>>(8*i));
    for(unsigned i=0;i<r[1];++i){r[8+i]=row[8+i];r[56+i]=row[56+i];}
    if(!wr2_record_valid(r))empty(r,s);
}
int wr2_slot_publish(const uint8_t *r,unsigned s) {
    if(s>=48||!r[0]||!wr2_record_valid(r))return 0;
    volatile uint8_t *inst=INST+s*0x174,*row=ROWS+s*104;
    uint32_t key=(uint32_t)r[4]|(uint32_t)r[5]<<8|(uint32_t)r[6]<<16|(uint32_t)r[7]<<24;
    if(inst[0]!=1||inst[0x168]!=r[1]||wr2_identity(inst)!=key)return 0;
    wr2_slot_clear(s);row[4]=r[1];row[5]=1;row[6]=0;row[7]=0;
    for(unsigned i=0;i<48;++i){row[8+i]=r[8+i];row[56+i]=r[56+i];}
    barrier();*(volatile uint32_t *)row=MAGIC;barrier();return 1;
}
void wr2_snapshot(wr2_blob *b) {
    for(unsigned s=0;s<48;++s)wr2_slot_snapshot(b->record[s],s);
    wr2_blob_seal(b);
}
void wr2_publish(const wr2_blob *b) {
    /* Caller validates the whole file/close before any publication. Foreground
     * sole writer; audio may interrupt. Publish magic last for each row. */
    wr2_clear();
    for(unsigned s=0;s<48;++s){
        const uint8_t *r=b->record[s];
        volatile uint8_t *inst=INST+s*0x174,*row=ROWS+s*104;
        uint32_t key=(uint32_t)r[4]|(uint32_t)r[5]<<8|(uint32_t)r[6]<<16|(uint32_t)r[7]<<24;
        if(!r[0]||inst[0]!=1||inst[0x168]!=r[1]||wr2_identity(inst)!=key)continue;
        row[4]=r[1];row[5]=1;row[6]=0;row[7]=0;
        for(unsigned i=0;i<48;++i){row[8+i]=r[8+i];row[56+i]=r[56+i];}
        barrier();*(volatile uint32_t *)row=MAGIC;
    }
    barrier();
}
