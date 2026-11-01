#ifndef S12B_COMPANION_H
#define S12B_COMPANION_H
#include "s12.h"
#include "sha256.h"
/* Exactly the inherited S11B allocation. Capture occupies blob only until
 * wr2_snapshot; finalization reuses dead path/temporary/guard storage. */
typedef struct {s12_io io;uint32_t native_fil[2];unsigned bad;} s12b_lease;
typedef struct {
    s12b_lease *lease;
    uint64_t seen;
    wr2_sha256_t sha;
    uint8_t packet[288],check[288];
    uint32_t slot,offset,total,identity,count,active,copier;
} s12b_capture;
typedef struct {uint8_t packet[288];char path[256];uint8_t chunk[224];} s12b_finish_scratch;
typedef struct {union {wr2_blob blob;s12b_capture capture;};s12_io io;char base[256];union {struct {char path[256],temporary[256],guard[256];};s12b_finish_scratch finish;};} s12b_work;
typedef struct {void *owned;uint32_t flags;} s12b_anchor;
#define S12B_ANC ((volatile s12b_anchor *)(uintptr_t)0x2002fc68u)
#define S12B_SAVE 16u
#define S12B_POISON 2u
#define S12B_WORK 4u
_Static_assert(sizeof(s12b_work)==6904,"inherited work size");
_Static_assert(sizeof(s12b_capture)<=WR2_BLOB_BYTES,"dead blob capture lifetime");
int s12b_begin(s12b_work *);
s12b_work *s12b_work_get(void);
void s12b_cancel(void);
void s12b_poll(void);
void s12b_release(s12b_work *,s12b_lease *);
void s12b_export_begin(uint32_t *,const char *);
void s12b_export_end(uint32_t *,unsigned);
void s12b_tap(unsigned,uint32_t *);
int s12b_native_close(uint32_t *,uintptr_t);
void s12b_finish(s12b_work *,s12b_lease *,uint64_t);
#endif
