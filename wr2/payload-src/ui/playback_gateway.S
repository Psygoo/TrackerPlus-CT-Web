.syntax unified
.cpu cortex-m7
.thumb

.section .text.wr2_mode5_gateway,"ax",%progbits
.global wr2_mode5_gateway
.type wr2_mode5_gateway,%function
.thumb_func
wr2_mode5_gateway:
    /* ABI verified at 0x60032b3a: r0=player, r1=slot, r2=note.
     * Preserve the 16-byte frame used by the hardware-tested WR1 gateway. */
    push    {r0, r1, r2, lr}
    bl      wr2_select_note
    mov     r12, r0
    ldmia.w sp!, {r0, r1, r3, lr}
    mov     r2, r12
    ldr     r3, =0x60032b51
    bx      r3
    .ltorg
.size wr2_mode5_gateway,.-wr2_mode5_gateway
