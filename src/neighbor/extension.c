/* H10: independent last evaluated conditional-note results for neighbours. */
#include <stdint.h>
#define TRACKS 16u
#define STATE ((volatile uint8_t *)0x20205030u)
#define EVENT(t) STATE[20u+(t)]
#define NE_PATTERN (*(volatile uint8_t *)0x20205055u)
#define NE_BITS (*(volatile uint16_t *)0x20205056u)
#define NE_CONTEXT (*(volatile uintptr_t *)0x20205058u)
#define NE_COOKIE (*(volatile uint32_t *)0x2020505Cu)
#define COOKIE 0x31454E48u
#define PATTERN (*(const volatile uint8_t *)0x202046C8u)
#define EVALUATED 8u
#define VALID_NOTE 16u
typedef int32_t (*chance_fn)(int32_t,int32_t,const uint8_t *,uint32_t);
typedef uint32_t (*process_fn)(void *,uint32_t);
typedef uint32_t (*advance_fn)(void *,uint32_t);

static void initialize(void)
{
    if (NE_COOKIE!=COOKIE) {
        NE_BITS=0;
        NE_CONTEXT=0;
        NE_PATTERN=PATTERN;
        NE_COOKIE=COOKIE;
    }
}

int32_t h10_chance(int32_t lo,int32_t hi,const uint8_t *column,uint32_t track)
{
    initialize();
    if (track<TRACKS && column[0x73]==4u) {
        uint32_t value=column[0x74];
        if (value<=141u && (EVENT(track)&1u)) {
            uint32_t flags=EVALUATED;
            const volatile uint8_t *b=(const volatile uint8_t *)NE_CONTEXT;
            if (b && ((uintptr_t)column==(uintptr_t)b || (uintptr_t)column+2u==(uintptr_t)b)) {
                uint32_t instrument=b[0x70];
                if (b[0x6F]<128u && b[0x81]==0u &&
                    ((instrument<48u && track<8u) || (instrument>=48u && instrument<=66u)))
                    flags|=VALID_NOTE;
            }
            EVENT(track)|=flags;
        }
        if (value==140u || value==141u) {
            if (EVENT(track)&1u)
                EVENT(track)|=4u;
            uint32_t source=track ? (NE_BITS>>(track-1u))&1u : 0u;
            return (source==(value==140u)) ? 0 : 0x7FFFFFFF;
        }
    }
    return ((chance_fn)0x601BC141u)(lo,hi,column,track);
}

uint32_t h10_process(void *seq,uint32_t track)
{
    if (track>=TRACKS)
        return ((process_fn)0x601BC1A1u)(seq,track);
    uint8_t *b=(uint8_t *)seq+0x68u*track;
    if (*(volatile uint16_t *)(b+0x86)!=1u)
        return ((process_fn)0x601BC1A1u)(seq,track);
    initialize();
    if (NE_PATTERN!=PATTERN) {
        NE_BITS=0;
        NE_PATTERN=PATTERN;
    }
    uintptr_t previous=NE_CONTEXT;
    NE_CONTEXT=(uintptr_t)b;
    /* Retained H07 synchronization applies resets before stock; its extended
     * reset-consumption store clears both histories at the exact same boundary. */
    uint32_t result=((process_fn)0x601BC1A1u)(seq,track);
    uint32_t flags=EVENT(track);
    if (flags&EVALUATED) {
        uint32_t mask=1u<<track;
        uint32_t bits=NE_BITS & ~mask;
        if ((flags&VALID_NOTE) && *(volatile uint8_t *)(b+0x98)==0u)
            bits|=mask;
        NE_BITS=(uint16_t)bits;
    }
    /* Restore the exact H08/H09 flags before its PRE event==3 commit gate. */
    EVENT(track)=(uint8_t)(flags&7u);
    NE_CONTEXT=previous;
    return result;
}

uint32_t h10_advance(void *seq,uint32_t track)
{
    if (track>=TRACKS)
        return ((advance_fn)0x600AD011u)(seq,track);
    volatile int32_t *queued=(volatile int32_t *)((uint8_t *)seq+0x68u*track+0xBCu);
    int32_t before=*queued;
    uint32_t result=((advance_fn)0x600AD011u)(seq,track);
    if (before>=0 && *queued<0) {
        initialize();
        NE_BITS=(uint16_t)(NE_BITS & ~(1u<<track));
    }
    return result;
}
