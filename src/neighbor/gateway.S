.syntax unified
.cpu cortex-m7
.thumb
.section .text.h10_reset_store,"ax",%progbits
.global h10_reset_store
.type h10_reset_store,%function
.thumb_func
h10_reset_store:
    /* Internal H07 ABI: r2 is zero; r5 is 0x20205000. Preserve every register
     * and flags, replay the displaced store, and clear only the new result bits.
     * H07 saved its original LR; the inserted BL requires no additional frame. */
    strb.w r2,[r5,#0x54]
    strh.w r2,[r5,#0x56]
    bx lr
.size h10_reset_store,.-h10_reset_store
