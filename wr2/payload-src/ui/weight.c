#include <stdint.h>

#define SCREEN_ADDR       0x200254b4u
#define INST_BASE         0x20200000u
#define ACTIVE_SLOT_ADDR  0x202046c5u
#define GROUP_BASE        0x20205068u
#define EDIT_ADDR         0x202063e8u
#define GROUP_MAGIC       0x31524757u
#define EDIT_MAGIC        0x31454757u
#define INST_STRIDE       0x174u
#define ROW_STRIDE        104u
#define MAX_SLICES        48u
#define MAX_INSTRUMENTS   48u

typedef void (*set_ptr38_t)(void *, int32_t, const char *);
typedef void (*refresh_t)(void *, int32_t);

static volatile uint8_t *const screen = (volatile uint8_t *)SCREEN_ADDR;
static volatile uint8_t *const edit = (volatile uint8_t *)EDIT_ADDR;

struct weight_context {
    volatile uint8_t *inst;
    volatile uint8_t *row;
    volatile uint8_t *owners;
    volatile uint8_t *weights;
    uint32_t slot, count, selected, master, members;
};

/* Validate every active row/group before allowing either a display or edit.
 * Unassigned entries must have no weight; every owner must point to a real
 * self-owned master; and each group must total exactly 100. */
static int weight_context_get(struct weight_context *c) {
    volatile uint8_t *inst;
    uint32_t i, j, sum;
    uintptr_t expected;
    volatile uint8_t *row;

    /* c2 is the logical page. Stock Slice formatting temporarily sets f0=0
     * while Page 3 refresh is being rendered. */
    if (*(volatile uint32_t *)EDIT_ADDR != EDIT_MAGIC || screen[0xc2] != 2)
        return 0;
    inst = (volatile uint8_t *)(uintptr_t)(*(volatile uint32_t *)(SCREEN_ADDR + 0xf4));
    if (!inst || inst[0x3c] != 5)
        return 0;
    c->slot = *(volatile uint8_t *)ACTIVE_SLOT_ADDR;
    if (c->slot >= MAX_INSTRUMENTS || edit[5] != c->slot)
        return 0;
    expected = (uintptr_t)INST_BASE + c->slot * INST_STRIDE;
    if ((uintptr_t)inst != expected)
        return 0;
    c->count = inst[0x168];
    c->selected = inst[0x169];
    c->master = edit[6];
    if (c->count == 0 || c->count > MAX_SLICES ||
        c->selected >= c->count || c->master >= c->count)
        return 0;

    row = (volatile uint8_t *)(GROUP_BASE + c->slot * ROW_STRIDE);
    if (*(volatile uint32_t *)row != GROUP_MAGIC || row[4] != c->count || row[5] != 1)
        return 0;
    c->row = row;
    c->owners = row + 8;
    c->weights = row + 56;
    if (c->owners[c->master] != c->master)
        return 0;

    for (i = 0; i < c->count; ++i) {
        uint32_t owner = c->owners[i];
        if (owner == 0xffu) {
            if (c->weights[i] != 0)
                return 0;
            continue;
        }
        if (owner >= c->count || c->owners[owner] != owner || c->weights[i] > 100)
            return 0;
    }
    for (i = 0; i < c->count; ++i) {
        if (c->owners[i] != i)
            continue;
        sum = 0;
        for (j = 0; j < c->count; ++j)
            if (c->owners[j] == i)
                sum += c->weights[j];
        if (sum != 100)
            return 0;
    }
    c->members = 0;
    for (i = 0; i < c->count; ++i)
        if (c->owners[i] == c->master)
            ++c->members;
    return c->members != 0;
}

static void weight_blank(void) {
    volatile char *text = (volatile char *)(SCREEN_ADDR + 0x126);
    int32_t id = (int8_t)screen[0xc8];
    text[0] = 0;
    ((set_ptr38_t)0x60101d75u)((void *)0x20027d64u, id,
                               (const char *)(uintptr_t)(SCREEN_ADDR + 0x126));
    ((refresh_t)0x60101e85u)((void *)0x20027d64u, id);
}

void wr2_weight_update_display_c(void) {
    struct weight_context c;
    volatile char *text;
    uint32_t value;
    int32_t id;

    if (!weight_context_get(&c) || c.owners[c.selected] != c.master) {
        weight_blank();
        return;
    }
    value = c.weights[c.selected];
    text = (volatile char *)(SCREEN_ADDR + 0x126);
    if (value >= 100) {
        text[0] = '1'; text[1] = '0'; text[2] = '0'; text[3] = '%'; text[4] = 0;
    } else if (value >= 10) {
        text[0] = (char)('0' + value / 10);
        text[1] = (char)('0' + value % 10);
        text[2] = '%'; text[3] = 0;
    } else {
        text[0] = (char)('0' + value);
        text[1] = '%'; text[2] = 0;
    }
    id = (int8_t)screen[0xc8];
    ((set_ptr38_t)0x60101d75u)((void *)0x20027d64u, id,
                               (const char *)(uintptr_t)(SCREEN_ADDR + 0x126));
    ((refresh_t)0x60101e85u)((void *)0x20027d64u, id);
}

/* Set one member's requested probability and apportion the remainder over
 * the other group members. Hamilton remainders tie by ascending slice index.
 * If all peers have zero weight, split equally, with spare points assigned
 * to the lowest slice indices. */
static int weight_set_selected(struct weight_context *c, uint32_t requested) {
    uint8_t next[MAX_SLICES], rem[MAX_SLICES], awarded[MAX_SLICES];
    uint32_t i, peer_sum = 0, peer_count = 0, floor_sum = 0;
    uint32_t remaining, left, k, best, best_rem;

    if (c->members <= 1)
        return 0; /* a one-slice group is always its master's 100% */
    if (requested > 100)
        requested = 100;
    for (i = 0; i < MAX_SLICES; ++i) {
        next[i] = 0;
        rem[i] = 0;
        awarded[i] = 0;
    }
    for (i = 0; i < c->count; ++i) {
        if (c->owners[i] == c->master && i != c->selected) {
            peer_sum += c->weights[i];
            ++peer_count;
        }
    }
    if (peer_count == 0)
        return 0;
    remaining = 100u - requested;
    if (peer_sum == 0) {
        uint32_t each = remaining / peer_count;
        left = remaining % peer_count;
        for (i = 0; i < c->count; ++i) {
            if (c->owners[i] == c->master && i != c->selected) {
                next[i] = (uint8_t)(each + (left != 0));
                if (left)
                    --left;
            }
        }
    } else {
        for (i = 0; i < c->count; ++i) {
            if (c->owners[i] == c->master && i != c->selected) {
                uint32_t product = (uint32_t)c->weights[i] * remaining;
                next[i] = (uint8_t)(product / peer_sum);
                rem[i] = (uint8_t)(product % peer_sum);
                floor_sum += next[i];
            }
        }
        if (floor_sum > remaining)
            return 0;
        left = remaining - floor_sum;
        for (k = 0; k < left; ++k) {
            best = MAX_SLICES;
            best_rem = 0;
            for (i = 0; i < c->count; ++i) {
                if (c->owners[i] == c->master && i != c->selected && !awarded[i] &&
                    (best == MAX_SLICES || rem[i] > best_rem)) {
                    best = i;
                    best_rem = rem[i];
                }
            }
            if (best == MAX_SLICES)
                return 0;
            ++next[best];
            awarded[best] = 1;
        }
    }
    next[c->selected] = (uint8_t)requested;
    /* All validation and arithmetic precede the first mutation. */
    for (i = 0; i < c->count; ++i)
        if (c->owners[i] == c->master)
            c->weights[i] = next[i];
    return 1;
}

void wr2_weight_adjust_c(uint32_t direction) {
    struct weight_context c;
    uint32_t current, requested;
    /* The stock jog-focus dispatcher supplies a signed detent delta in r2.
     * Keep its magnitude: Stage 7 previously reduced accelerated movement to
     * its sign, forcing every callback to change exactly one percentage point.
     * No local timing thresholds or acceleration curve are applied here. */
    int32_t delta = (int32_t)direction;
    if (delta == 0 || !weight_context_get(&c) || c.owners[c.selected] != c.master)
        return;
    if (c.members <= 1)
        return;
    current = c.weights[c.selected];
    if (delta > 0) {
        uint32_t room = 100u - current;
        requested = (uint32_t)delta >= room ? 100u : current + (uint32_t)delta;
    } else {
        /* Compare before adding so even INT32_MIN cannot overflow. */
        requested = delta <= -(int32_t)current ? 0u : (uint32_t)((int32_t)current + delta);
    }
    if (requested != current)
        (void)weight_set_selected(&c, requested);
}
