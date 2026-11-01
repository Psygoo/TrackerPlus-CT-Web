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

typedef void (*stock_callback_t)(uint32_t);
typedef void (*refresh_t)(void *);

extern void wr2_group_edit_exit(void);

static volatile uint8_t *const screen = (volatile uint8_t *)SCREEN_ADDR;
static volatile uint8_t *const edit = (volatile uint8_t *)EDIT_ADDR;

static void stock_add(uint32_t event) {
    ((stock_callback_t)0x600f4c89u)(event);
}

static void redraw(void) {
    ((refresh_t)0x600f7c55u)((void *)SCREEN_ADDR);
}

/*
 * Proportionally scale all current group weights to 90%, then assign the new
 * member 10%. Hamilton largest-remainder rounding is deterministic: remainder
 * ties go to the lower slice index. Refuse an addition if any old slice would
 * receive zero. Existing zero-probability members remain valid and zero.
 */
static int add_selected_to_locked_group(void) {
    uint8_t scaled[MAX_SLICES];
    uint8_t remainder[MAX_SLICES];
    uint8_t awarded[MAX_SLICES];
    uint8_t *inst;
    volatile uint8_t *row;
    uint32_t slot, selected, master, count, i, sum = 0, floor_sum = 0;
    uint32_t remaining, k, best, best_remainder;
    volatile uint8_t *owners, *weights;

    for (i = 0; i < MAX_SLICES; ++i) {
        scaled[i] = 0;
        remainder[i] = 0;
        awarded[i] = 0;
    }
    if (*(volatile uint32_t *)EDIT_ADDR != EDIT_MAGIC)
        return 0;
    if (screen[0xf0] != 2)
        return 0;
    inst = (uint8_t *)(uintptr_t)(*(volatile uint32_t *)(SCREEN_ADDR + 0xf4));
    if (inst == 0 || inst[0x3c] != 5)
        return 0;
    slot = *(volatile uint8_t *)ACTIVE_SLOT_ADDR;
    if (slot >= MAX_INSTRUMENTS ||
        (uintptr_t)inst != INST_BASE + slot * INST_STRIDE ||
        edit[5] != slot)
        return 0;

    count = inst[0x168];
    selected = inst[0x169];
    master = edit[6];
    if (count < 2 || count > MAX_SLICES || selected >= count || master >= count)
        return 0;

    row = (volatile uint8_t *)(GROUP_BASE + slot * ROW_STRIDE);
    if (*(volatile uint32_t *)row != GROUP_MAGIC || row[4] != count || row[5] != 1)
        return 0;
    owners = row + 8;
    weights = row + 56;
    if (owners[master] != master || weights[master] > 100)
        return 0;

    if (owners[selected] == master)
        return 0; /* already this group, including its master: idempotent */
    if (owners[selected] != 0xffu)
        return 0; /* owned by a different group or an invalid owner */

    for (i = 0; i < count; ++i) {
        if (owners[i] == master) {
            uint32_t w = weights[i];
            uint32_t product;
            if (w > 100)
                return 0;
            sum += w;
            product = w * 90u;
            scaled[i] = (uint8_t)(product / 100u);
            remainder[i] = (uint8_t)(product % 100u);
            if (w != 0 && scaled[i] == 0)
                return 0; /* do not silently remove a member's effective weight */
            floor_sum += scaled[i];
        }
    }
    if (sum != 100u || floor_sum > 90u)
        return 0;
    remaining = 90u - floor_sum;

    for (k = 0; k < remaining; ++k) {
        best = MAX_SLICES;
        best_remainder = 0;
        for (i = 0; i < count; ++i) {
            if (owners[i] == master && !awarded[i] &&
                (best == MAX_SLICES || remainder[i] > best_remainder)) {
                best = i;
                best_remainder = remainder[i];
            }
        }
        if (best == MAX_SLICES)
            return 0;
        ++scaled[best];
        awarded[best] = 1;
    }

    /* Commit only after all validation and apportionment has succeeded. */
    for (i = 0; i < count; ++i)
        if (owners[i] == master)
            weights[i] = scaled[i];
    owners[selected] = (uint8_t)master;
    weights[selected] = 10;
    return 1;
}

void wr2_page3_add_callback_c(uint32_t event) {
    uint8_t *inst = (uint8_t *)(uintptr_t)(*(volatile uint32_t *)(SCREEN_ADDR + 0xf4));
    int page3 = inst != 0 && inst[0x3c] == 5 && screen[0xf0] == 2;

    if (!page3) {
        stock_add(event);
        return;
    }
    if (event == 0) {
        /* Page 3 Add is our own action; never enter the stock Add path here. */
        return;
    }
    if (event != 1 && event != 0xffu)
        return;
    if (*(volatile uint32_t *)EDIT_ADDR != EDIT_MAGIC)
        return;
    if (add_selected_to_locked_group())
        redraw(); /* stock marker renderer sees the committed owner/weight row */
}

/*
 * Remove a member from the locked group. Re-normalize the remaining weights
 * to 100 using Hamilton largest-remainder apportionment, with ties resolved
 * by slice index. The owner/weight table is only written after the whole
 * record has passed validation and a complete result has been calculated.
 * Return 1 if a mutation occurred, 2 if deleting the master ended edit mode.
 */
static int remove_selected_from_locked_group(void) {
    uint8_t scaled[MAX_SLICES];
    uint8_t remainder[MAX_SLICES];
    uint8_t awarded[MAX_SLICES];
    uint8_t *inst;
    volatile uint8_t *row;
    volatile uint8_t *owners, *weights;
    uint32_t slot, selected, master, count, i, sum = 0;
    uint32_t remaining_sum, floor_sum = 0, remaining, k;
    uint32_t best, best_remainder;

    for (i = 0; i < MAX_SLICES; ++i) {
        scaled[i] = 0;
        remainder[i] = 0;
        awarded[i] = 0;
    }
    if (*(volatile uint32_t *)EDIT_ADDR != EDIT_MAGIC || screen[0xf0] != 2)
        return 0;
    inst = (uint8_t *)(uintptr_t)(*(volatile uint32_t *)(SCREEN_ADDR + 0xf4));
    if (inst == 0 || inst[0x3c] != 5)
        return 0;
    slot = *(volatile uint8_t *)ACTIVE_SLOT_ADDR;
    if (slot >= MAX_INSTRUMENTS ||
        (uintptr_t)inst != INST_BASE + slot * INST_STRIDE || edit[5] != slot)
        return 0;

    count = inst[0x168];
    selected = inst[0x169];
    master = edit[6];
    if (count == 0 || count > MAX_SLICES || selected >= count || master >= count)
        return 0;

    row = (volatile uint8_t *)(GROUP_BASE + slot * ROW_STRIDE);
    if (*(volatile uint32_t *)row != GROUP_MAGIC || row[4] != count || row[5] != 1)
        return 0;
    owners = row + 8;
    weights = row + 56;
    if (owners[master] != master || weights[master] > 100)
        return 0;
    if (owners[selected] != master)
        return 0; /* unassigned and foreign-owned selections are strictly inert */

    /* Validate only this group's records; unrelated groups remain untouched. */
    for (i = 0; i < count; ++i) {
        if (owners[i] == master) {
            if (weights[i] > 100)
                return 0;
            sum += weights[i];
        }
    }
    if (sum != 100u)
        return 0;

    if (selected == master) {
        /* Delete the complete group, retaining the initialized row header. */
        for (i = 0; i < count; ++i) {
            if (owners[i] == master) {
                owners[i] = 0xffu;
                weights[i] = 0;
            }
        }
        wr2_group_edit_exit();
        return 2;
    }

    remaining_sum = sum - weights[selected];
    if (remaining_sum == 0u) {
        uint32_t peers = 0, each, extra;
        for (i = 0; i < count; ++i)
            if (owners[i] == master && i != selected)
                ++peers;
        if (peers == 0)
            return 0;
        each = 100u / peers;
        extra = 100u % peers;
        for (i = 0; i < count; ++i) {
            if (owners[i] == master && i != selected) {
                scaled[i] = (uint8_t)(each + (extra != 0));
                if (extra)
                    --extra;
            }
        }
        remaining_sum = 1u; /* marks the deterministic equal-share result */
        floor_sum = 100u;
    }
    for (i = 0; floor_sum != 100u && i < count; ++i) {
        if (owners[i] == master && i != selected) {
            uint32_t product = (uint32_t)weights[i] * 100u;
            scaled[i] = (uint8_t)(product / remaining_sum);
            remainder[i] = (uint8_t)(product % remaining_sum);
            floor_sum += scaled[i];
        }
    }
    if (floor_sum > 100u)
        return 0;
    remaining = 100u - floor_sum;
    for (k = 0; k < remaining; ++k) {
        best = MAX_SLICES;
        best_remainder = 0;
        for (i = 0; i < count; ++i) {
            if (owners[i] == master && i != selected && !awarded[i] &&
                (best == MAX_SLICES || remainder[i] > best_remainder)) {
                best = i;
                best_remainder = remainder[i];
            }
        }
        if (best == MAX_SLICES)
            return 0;
        ++scaled[best];
        awarded[best] = 1;
    }

    /* Commit the validated result atomically with respect to all checks. */
    for (i = 0; i < count; ++i) {
        if (owners[i] == master) {
            if (i == selected) {
                owners[i] = 0xffu;
                weights[i] = 0;
            } else {
                weights[i] = scaled[i];
            }
        }
    }
    return 1;
}

void wr2_page3_remove_callback_c(uint32_t event) {
    uint8_t *inst = (uint8_t *)(uintptr_t)(*(volatile uint32_t *)(SCREEN_ADDR + 0xf4));
    int page3 = inst != 0 && inst[0x3c] == 5 && screen[0xf0] == 2;
    int result;

    if (!page3) {
        ((stock_callback_t)0x600f4b55u)(event);
        return;
    }
    if (event != 1 && event != 0xffu)
        return; /* physical release never repeats the operation */
    if (*(volatile uint32_t *)EDIT_ADDR != EDIT_MAGIC)
        return;
    result = remove_selected_from_locked_group();
    if (result != 0)
        redraw(); /* stock waveform renderer reflects the committed table */
}
