#include <stdint.h>
#include "levels.h"

#define SCREEN_ADDR       0x200254b4u
#define ACTIVE_SLOT_ADDR  0x202046c5u
#define INST_BASE         0x20200000u
#define INST_STRIDE       0x174u
#define GROUP_BASE        0x20205068u
#define ROW_STRIDE        104u
#define GROUP_MAGIC       0x31524757u
#define EDIT_ADDR         0x202063e8u
#define EDIT_MAGIC        0x31454757u
#define PAD_MGR_ADDR      0x200171d0u
#define SETTINGS_ADDR     0x2002061cu
#define GAMMA_ADDR        0x60037390u
#define STOCK_SET_PAD     0x600a1bf1u
#define PAD_COUNT         48u
#define PAD_COLS          12u

typedef void (*set_pad_fn)(void *, uint32_t, uint32_t, uint32_t);

struct led_context {
    volatile uint8_t *owners;
    uint32_t count;
    uint32_t editing;
    uint32_t active_master;
};

static volatile uint8_t *const screen = (volatile uint8_t *)(uintptr_t)SCREEN_ADDR;
static volatile uint8_t *const manager = (volatile uint8_t *)(uintptr_t)PAD_MGR_ADDR;
static volatile uint8_t *const edit = (volatile uint8_t *)(uintptr_t)EDIT_ADDR;
static const uint8_t *const gamma = (const uint8_t *)(uintptr_t)GAMMA_ADDR;

/* The active slot is checked before the screen's instrument pointer is ever
 * dereferenced.  A zero row magic is the normal empty/no-group state.  Any
 * partially published or malformed nonzero row falls back to stock output. */
static int context_get(struct led_context *c) {
    volatile uint8_t *inst;
    volatile uint8_t *row;
    volatile uint8_t *owners;
    volatile uint8_t *weights;
    uint16_t totals[PAD_COUNT];
    uint32_t slot, count, i, owner, magic;

    if (!screen[4] || screen[0xc2] != 2)
        return 0;
    slot = *(volatile uint8_t *)(uintptr_t)ACTIVE_SLOT_ADDR;
    if (slot >= 48u)
        return 0;
    inst = (volatile uint8_t *)(uintptr_t)(INST_BASE + slot * INST_STRIDE);
    if (*(volatile uint32_t *)(uintptr_t)(SCREEN_ADDR + 0xf4u) != (uintptr_t)inst)
        return 0;
    if (inst[0] != 1u || inst[0x3c] != 5u)
        return 0;
    count = inst[0x168];
    if (!count || count > PAD_COUNT)
        return 0;

    row = (volatile uint8_t *)(uintptr_t)(GROUP_BASE + slot * ROW_STRIDE);
    magic = *(volatile uint32_t *)row;
    c->owners = 0;
    if (magic != 0u) {
        if (magic != GROUP_MAGIC || row[4] != count || row[5] != 1u ||
            row[6] != 0u || row[7] != 0u)
            return 0;
        owners = row + 8;
        weights = row + 56;
        for (i = 0; i < PAD_COUNT; ++i)
            totals[i] = 0;
        for (i = 0; i < count; ++i) {
            owner = owners[i];
            if (owner == 0xffu) {
                if (weights[i] != 0u)
                    return 0;
                continue;
            }
            if (owner >= count || owners[owner] != owner || weights[i] > 100u)
                return 0;
            totals[owner] = (uint16_t)(totals[owner] + weights[i]);
        }
        for (i = 0; i < count; ++i)
            if (owners[i] == i && totals[i] != 100u)
                return 0;
        c->owners = owners;
    }

    c->count = count;
    c->editing = 0;
    c->active_master = 0xffu;
    magic = *(volatile uint32_t *)(uintptr_t)EDIT_ADDR;
    if (magic != 0u) {
        uint32_t master;
        if (magic != EDIT_MAGIC || edit[5] != slot || !c->owners)
            return 0;
        master = edit[6];
        if (master >= count || c->owners[master] != master)
            return 0;
        c->editing = 1;
        c->active_master = master;
    }
    return 1;
}

static void *led_object(void) {
    return (void *)(uintptr_t)(*(volatile uint32_t *)(uintptr_t)(PAD_MGR_ADDR + 0x98u));
}

static int valid_led_object(void *leds) {
    uintptr_t p = (uintptr_t)leds;
    return p && !(p & 3u) && leds == led_object();
}

uint32_t s13b_level_fraction(uint32_t top, uint32_t numerator, uint32_t denominator) {
    uint32_t target, best = 0, best_delta = 0xffffffffu, i;
    if (!denominator)
        return 0;
    if (top > 31u)
        top = 31u;
    target = ((uint32_t)gamma[top] * numerator + denominator / 2u) / denominator;
    for (i = 0; i <= top; ++i) {
        uint32_t pwm = gamma[i];
        uint32_t delta = pwm > target ? pwm - target : target - pwm;
        if (delta < best_delta) {
            best = i;
            best_delta = delta;
        }
    }
    return best;
}

static uint32_t bright_ceiling(void) {
    uint32_t top = *(volatile uint8_t *)(uintptr_t)(SETTINGS_ADDR + 0x35u);
    return top > 31u ? 31u : top;
}

static uint32_t desired_level(const struct led_context *c, uint32_t pad) {
    uint32_t top, owner;
    if (pad >= c->count)
        return 0;
    top = bright_ceiling();
    if (!c->owners)
        return s13b_level_fraction(top, S13B_UNASSIGNED_NUM, S13B_UNASSIGNED_DEN);
    owner = c->owners[pad];
    if (owner == 0xffu)
        return s13b_level_fraction(top, S13B_UNASSIGNED_NUM, S13B_UNASSIGNED_DEN);
    if (!c->editing)
        return owner == pad ? top :
            s13b_level_fraction(top, S13B_ACTIVE_MEMBER_NUM, S13B_ACTIVE_MEMBER_DEN);
    if (owner == c->active_master)
        return owner == pad ? top :
            s13b_level_fraction(top, S13B_ACTIVE_MEMBER_NUM, S13B_ACTIVE_MEMBER_DEN);
    return s13b_level_fraction(top, S13B_OTHER_GROUP_NUM, S13B_OTHER_GROUP_DEN);
}

static uint32_t frame_slot(uint32_t pad) {
    return pad + (pad / PAD_COLS) * 4u;
}

static int output_matches(void *leds, uint32_t pad, uint32_t on, uint32_t level) {
    volatile uint8_t *driver = (volatile uint8_t *)leds + 4;
    uint32_t slot = frame_slot(pad);
    uint32_t enabled = (driver[0x14eu + slot / 8u] >> (slot & 7u)) & 1u;
    return enabled == (on != 0u) && driver[8u + slot] == gamma[level];
}

static void write_if_changed(void *leds, uint32_t pad, uint32_t on, uint32_t level) {
    if (!valid_led_object(leds) || pad >= PAD_COUNT)
        return;
    if (level > 31u)
        level = 31u;
    if (!on)
        level = 0;
    if (!output_matches(leds, pad, on, level))
        ((set_pad_fn)(uintptr_t)STOCK_SET_PAD)(leds, pad, on, level);
}

static uint32_t stock_level(uint32_t pad) {
    uint32_t a = manager[0x04u + pad];
    uint32_t b = manager[0x34u + pad];
    uint32_t level = a > b ? a : b;
    return level > 31u ? 31u : level;
}

static void restore_live_stock(void *leds) {
    uint32_t pad;
    for (pad = 0; pad < PAD_COUNT; ++pad) {
        uint32_t level = stock_level(pad);
        write_if_changed(leds, pad, level != 0u, level);
    }
}

void s13b_led_force_restore(void) {
    void *leds = led_object();
    if (valid_led_object(leds))
        restore_live_stock(leds);
}

void s13b_led_sync(void) {
    struct led_context c;
    void *leds = led_object();
    uint32_t pad;
    if (!valid_led_object(leds))
        return;
    if (!context_get(&c)) {
        restore_live_stock(leds);
        return;
    }
    for (pad = 0; pad < PAD_COUNT; ++pad) {
        uint32_t level = desired_level(&c, pad);
        write_if_changed(leds, pad, level != 0u, level);
    }
}

/* Shared output boundary only.  Native manager A/B arrays continue to change
 * underneath this presentation and are never written by S13B. */
void s13b_pad_set_hook(void *leds, uint32_t pad, uint32_t on, uint32_t level) {
    struct led_context c;
    if (!leds || ((uintptr_t)leds & 3u))
        return;
    if (!valid_led_object(leds) || pad >= PAD_COUNT) {
        ((set_pad_fn)(uintptr_t)STOCK_SET_PAD)(leds, pad, on, level);
        return;
    }
    if (context_get(&c)) {
        uint32_t desired = desired_level(&c, pad);
        write_if_changed(leds, pad, desired != 0u, desired);
        return;
    }
    ((set_pad_fn)(uintptr_t)STOCK_SET_PAD)(leds, pad, on, level);
}
