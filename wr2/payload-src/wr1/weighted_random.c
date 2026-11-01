#include <stdint.h>

#define TARGET_SLOT       0u       /* visible slot 01; table/pattern indexes are 0-based */
#define TRIGGER_NOTE      0x19u    /* C#; Beat Slice note base 0x18 */
#define BEAT_SLICE_MODE   5u
#define MIN_SLICE_COUNT   4u
#define MAX_SLICE_COUNT   48u
#define RNG_MAGIC         0x57525231u /* "WRR1" */
#define RNG_MAGIC_ADDR    ((volatile uint32_t *)0x20205060u)
#define RNG_STATE_ADDR    ((volatile uint32_t *)0x20205064u)
#define SYSTICK_VAL       ((volatile uint32_t *)0xE000E018u)
#define INSTR_TABLE_PTR   ((volatile uint32_t *)0x60032C94u)

/* Return a local mapper note. The incoming note remains stored in the track context. */
uint32_t wr01_select_note(uint32_t voice, uint32_t slot, uint32_t note)
{
    if (slot != TARGET_SLOT || note != TRIGGER_NOTE || voice == 0u)
        return note;

    uint32_t instrument_base = *INSTR_TABLE_PTR;
    if (instrument_base != 0x20200000u)
        return note;
    const volatile uint8_t *instrument =
        (const volatile uint8_t *)(instrument_base + slot * 0x174u);
    if (instrument[0x3c] != BEAT_SLICE_MODE)
        return note;
    uint32_t slice_count = instrument[0x168];
    if (slice_count < MIN_SLICE_COUNT || slice_count > MAX_SLICE_COUNT)
        return note;

    /* The stock S effect owns this override path; leave it untouched. */
    const volatile uint8_t *player = (const volatile uint8_t *)voice;
    if (player[0xae] != 0u)
        return note;

    uint32_t state;
    if (*RNG_MAGIC_ADDR != RNG_MAGIC || *RNG_STATE_ADDR == 0u) {
        /* A varying hardware timer and voice-pool address seed cold/uninitialized RAM.
         * If OCRAM survives a reset, the valid state continues instead of restarting. */
        state = *SYSTICK_VAL ^ voice ^ (slice_count << 24) ^ 0x9e3779b9u;
        if (state == 0u)
            state = 0x6d2b79f5u;
        *RNG_STATE_ADDR = state;
        *RNG_MAGIC_ADDR = RNG_MAGIC;
    }
    state = *RNG_STATE_ADDR;

    /* xorshift32: dedicated state, no call to the CT2/Chance RNG. */
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    *RNG_STATE_ADDR = state;

    /* Phase 6 multiply-high range mapping; total relative weight is 100. */
    uint32_t draw = (uint32_t)(((uint64_t)state * 100u) >> 32);
    if (draw < 50u)
        return 0x19u; /* slice 1 */
    if (draw < 80u)
        return 0x1au; /* slice 2 */
    return 0x1bu;     /* slice 3 */
}
