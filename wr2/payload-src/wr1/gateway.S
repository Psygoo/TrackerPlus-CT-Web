.syntax unified
.cpu cortex-m7
.thumb
.section .text.wr01_mode5_gateway,"ax",%progbits
.global wr01_mode5_gateway
.type wr01_mode5_gateway,%function
.thumb_func
wr01_mode5_gateway:
    /* Public ABI from the mode-5 dispatch call: r0=player, r1=slot, r2=note.
     * Save arguments and LR on a 16-byte frame before calling C. */
    push {r0,r1,r2,lr}
    bl wr01_select_note
    mov r12,r0
    ldmia.w sp!,{r0,r1,r3,lr}
    mov r2,r12
    ldr r3,=0x60032b51
    bx r3
    .ltorg
.size wr01_mode5_gateway,.-wr01_mode5_gateway
