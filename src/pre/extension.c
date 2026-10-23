/* H07 adds history consumers and labels around the preserved H06 machinery.
 * No new allocation: one deferred-reset byte uses the original RAM padding.
 */
#include <stdint.h>
#define TRACKS 16u
#define H06 ((volatile uint8_t *)0x20205030u)
#define HISTORY(t) H06[4u+(t)]
#define EVENT(t) H06[20u+(t)]
#define RESET_PENDING H06[36u]
#define CT_MAGIC (*(volatile uint32_t *)0x20205000u)
#define CT_PENDING (*(volatile uint8_t *)0x20205024u)
#define CT_MAGIC_VALUE 0x54434D47u
#define ACTIVE 1u
#define SEEN_PRE 4u
#define PRE 138u
#define NOT_PRE 139u
#define CT_FAIL 0x7FFFFFFF

typedef int32_t (*chance_fn)(int32_t,int32_t,const uint8_t *,uint32_t);
typedef void (*process_fn)(void *,uint32_t);
typedef void (*sync_fn)(void);
extern void h07_stock_value_gateway(void *,char *,uint32_t,uint32_t,uint32_t,uint32_t);

/* Called by H06's observer tail, after its existing lazy initialization.
 * Mark consumers so H06's event==3 commit gate excludes the whole mixed event.
 * The history array itself is never changed here.
 */
int32_t h07_chance(int32_t lo,int32_t hi,const uint8_t *column,uint32_t track)
{
    if (track<TRACKS && column[0x73]==4u) {
        uint32_t value=column[0x74];
        if (value==PRE || value==NOT_PRE) {
            if (EVENT(track)&ACTIVE)
                EVENT(track)|=SEEN_PRE;
            return ((HISTORY(track)!=0)==(value==PRE)) ? 0 : CT_FAIL;
        }
    }
    return ((chance_fn)0x601BAB81u)(lo,hi,column,track);
}

/* H06 still starts/commits/clears each transaction. Synchronize a reset only
 * before its stock call, at a fresh event, then tail-call the original processor.
 * New ct1 pending resets can be consumed here even at count-in within one tick.
 * A reset that arrives between the two FX evaluations remains deferred.
 */
void h07_before_stock(void *seq,uint32_t track)
{
    if (track<TRACKS && *(volatile uint16_t *)((uint8_t *)seq+0x68u*track+0x86u)==1u) {
        if (CT_MAGIC!=CT_MAGIC_VALUE || CT_PENDING)
            ((sync_fn)0x601BAE75u)();
        if (RESET_PENDING) {
            RESET_PENDING=0;
            for (uint32_t t=0;t<TRACKS;t++)
                HISTORY(t)=0;
        }
    }
    ((process_fn)0x203Du)(seq,track);
}

/* The same bounded labels serve stock editor and grid callers, in either column.
 * All other formatting executes the original stock function through its gateway.
 */
void h07_value_text(void *seq,char *buf,uint32_t size,uint32_t type,uint32_t value,uint32_t compact)
{
    value=(uint8_t)value;
    if (type==4u && (value==PRE || value==NOT_PRE)) {
        const char *label=value==PRE ? "PR" : "!PR";
        uint32_t i=0;
        if (size) {
            while (label[i] && i+1<size) {
                buf[i]=label[i];
                i++;
            }
            buf[i]=0;
        }
        return;
    }
    h07_stock_value_gateway(seq,buf,size,type,value,compact);
}
