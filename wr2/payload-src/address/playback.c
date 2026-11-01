#include <stdint.h>

#define INST_TABLE       0x60032c94u
#define INST_BASE        0x20200000u
#define ACTIVE_SLOT      0x202046c5u
#define GROUP_BASE       0x20205068u
#define RNG_MAGIC_ADDR   0x20205060u
#define RNG_STATE_ADDR   0x20205064u
#define RNG_MAGIC        0x57525231u
#define RNG_SEED_XOR     0x9e3779b9u
#define RNG_SEED_ALT     0x6d2b79f5u
#define GROUP_MAGIC      0x31524757u
#define INST_STRIDE      0x174u
#define ROW_STRIDE       104u
#define MAX_SLICES       48u
#define MAX_SLOTS        48u
#define NOTE_BASE        0x18u

static uint32_t next_random(void *voice, uint32_t slice_count) {
    volatile uint32_t *magic = (volatile uint32_t *)RNG_MAGIC_ADDR;
    volatile uint32_t *state = (volatile uint32_t *)RNG_STATE_ADDR;
    uint32_t x;

    if (*magic != RNG_MAGIC || *state == 0) {
        x = *(volatile uint32_t *)0xe000e018u;
        x ^= slice_count << 24;
        x ^= (uint32_t)(uintptr_t)voice;
        if (x == RNG_SEED_XOR)
            x = RNG_SEED_ALT;
        else
            x ^= RNG_SEED_XOR;
        *state = x;
        *magic = RNG_MAGIC;
    }

    x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return (uint32_t)(((uint64_t)x * 100u) >> 32);
}

/* Snapshot and validate one instrument row with bounded reads only.
 * This runs in the audio path: no locks, waits, allocation, or retry loops.
 * A UI edit can be observed between byte stores; an invalid snapshot simply
 * falls back to the incoming note. */
uint32_t wr2_select_note(void *voice, uint32_t slot, uint32_t note) {
    uint8_t owners[MAX_SLICES], weights[MAX_SLICES];
    uint8_t totals[MAX_SLICES];
    volatile uint8_t *inst, *row;
    uint32_t count, i, active = 0, master, cumulative;
    uint32_t rng;

    if (!voice || slot >= MAX_SLOTS)
        return note;
    /* Match the stock FX S2 direct-slice bypass before reading or advancing RNG. */
    if (*(volatile uint8_t *)((uintptr_t)voice + 0xaeu) != 0)
        return note;

    /* Resolve the selected record, not the table base. Integer arithmetic
     * matches patch.S and retains null and expected-address validation before
     * dereferencing any instrument fields. slot is already bounded to 0..47. */
    uintptr_t table = *(volatile uint32_t *)INST_TABLE;
    if (!table)
        return note;
    inst = (volatile uint8_t *)(table + slot * INST_STRIDE);
    if ((uintptr_t)inst != INST_BASE + slot * INST_STRIDE || inst[0x3c] != 5)
        return note;
    count = inst[0x168];
    if (count == 0 || count > MAX_SLICES || note < NOTE_BASE ||
        note - NOTE_BASE >= count)
        return note;

    row = (volatile uint8_t *)(GROUP_BASE + slot * ROW_STRIDE);
    if (*(volatile uint32_t *)row != GROUP_MAGIC) {
        return note; /* empty or malformed: normal stock Beat Slice playback */
    }
    if (row[4] != count || row[5] != 1)
        return note;

    for (i = 0; i < MAX_SLICES; ++i)
        totals[i] = 0;
    for (i = 0; i < count; ++i) {
        owners[i] = row[8 + i];
        weights[i] = row[56 + i];
        if (owners[i] == 0xffu) {
            if (weights[i] != 0)
                return note;
        }
    }

    /* Validate ownership after the full bounded owner snapshot is available. */
    for (i = 0; i < count; ++i) {
        uint32_t owner = owners[i];
        if (owner == 0xffu)
            continue;
        if (owner >= count || owners[owner] != owner || weights[i] > 100)
            return note;
        if ((uint32_t)totals[owner] + weights[i] > 100)
            return note;
        totals[owner] = (uint8_t)(totals[owner] + weights[i]);
        if (owners[i] == i)
            active = 1;
    }
    for (i = 0; i < count; ++i)
        if (owners[i] == i && totals[i] != 100)
            return note;

    if (!active)
        return note;

    master = note - NOTE_BASE;
    if (owners[master] != master)
        return note; /* members and unassigned slices always play directly */

    /* The selected master must have a validated complete 100% distribution. */
    if (totals[master] != 100)
        return note;
    rng = next_random(voice, count); /* one independent draw for each group trigger */
    cumulative = 0;
    for (i = 0; i < count; ++i) {
        if (owners[i] != master || weights[i] == 0)
            continue;
        cumulative += weights[i];
        if (rng < cumulative)
            return NOTE_BASE + i;
    }
    /* Defensive fail-safe; valid totals make this unreachable. */
    return note;
}
