.syntax unified
.cpu cortex-m7
.thumb

/* Replaces exactly 14 bytes at 0x601bdb3e. At entry:
 * r4 = loaded instrument-table base, already checked nonzero
 * r3 = slot * 0x174, with slot already checked <= 47
 * r1 = slot, r2 = incoming note.
 * The following comparison still rejects every unexpected table base.
 * Reuse the existing mode-rejection BNE at 0x601bdb52 to fit in place.
 */
.section .text,"ax",%progbits
.global wr2_resolve_selected_instrument
.type wr2_resolve_selected_instrument,%function
.thumb_func
wr2_resolve_selected_instrument:
    add     r4, r3
    add.w   r3, r3, #0x20000000
    add.w   r3, r3, #0x00200000
    cmp     r4, r3
    bne.n   wr2_mode_reject
.size wr2_resolve_selected_instrument, .-wr2_resolve_selected_instrument
