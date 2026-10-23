#include <stdint.h>

#define TRACKS 16u
#define TRACK_STRIDE 0x68u
#define TRK_MICRO 0x86u
#define TRK_FAILED 0x98u
#define SEQ_TRACK(t) (TRACK_STRIDE * (t))
#define FX_VALUE 0x74u
#define FX_CHANCE 4u
#define CT_FIRST 101u
#define CT_LAST 137u
#define CT_ACTIVE 1u
#define CT_SEEN_CONDITION 2u
#define H03_MAGIC 0x33485250u /* "PRH3" */

typedef void (*process_track_fn)(void *seq, uint32_t track);
typedef int32_t (*chance_fn)(int32_t lo, int32_t hi, const uint8_t *column, uint32_t track);

const volatile uint32_t h03_next_process_track __attribute__((used)) = 0;
const volatile uint32_t h03_next_chance __attribute__((used)) = 0;

/* NOLOAD state is deliberately separate from ct1's 48-byte counter state. */
struct h03_state {
    uint32_t magic;
    uint8_t history[TRACKS];
    uint8_t event[TRACKS];
} __attribute__((aligned(16)));
static struct h03_state h03_state_ram __attribute__((section(".mt_ram")));
#define H03 (&h03_state_ram)

static void h03_init(void)
{
    if (H03->magic == H03_MAGIC)
        return;
    for (uint32_t t = 0; t < TRACKS; t++) {
        H03->history[t] = 0;
        H03->event[t] = 0;
    }
    H03->magic = H03_MAGIC;
}

/* Preserve the existing ct1 condition result, then record that this event used
 * at least one of ct1's conditional Chance values. No new value is accepted. */
int32_t h03_chance(int32_t lo, int32_t hi, const uint8_t *column, uint32_t track)
{
    h03_init();
    if (track < TRACKS && H03->event[track] == CT_ACTIVE &&
        column[0x73] == FX_CHANCE && column[FX_VALUE] >= CT_FIRST &&
        column[FX_VALUE] <= CT_LAST)
        H03->event[track] |= CT_SEEN_CONDITION;
    return ((chance_fn)h03_next_chance)(lo, hi, column, track);
}

/* Commit once after stock processing, using its accumulated rejection byte. */
void h03_process_track(void *seq, uint32_t track)
{
    volatile uint32_t retained[2];
    uint8_t *trk = (uint8_t *)seq + SEQ_TRACK(track < TRACKS ? track : 0);
    retained[0] = (uint32_t)(uintptr_t)seq;
    retained[1] = track;
    if (track >= TRACKS || *(volatile uint16_t *)(trk + TRK_MICRO) != 1) {
        ((process_track_fn)h03_next_process_track)(seq, track);
        (void)retained[0];
        return;
    }
    h03_init();
    H03->event[track] = CT_ACTIVE;
    ((process_track_fn)h03_next_process_track)(seq, track);
    if (H03->event[track] == (CT_ACTIVE | CT_SEEN_CONDITION))
        H03->history[track] = *(volatile uint8_t *)(trk + TRK_FAILED) == 0;
    H03->event[track] = 0;
    (void)retained[0];
}
