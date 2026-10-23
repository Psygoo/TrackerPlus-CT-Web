.syntax unified
.cpu cortex-m7
.thumb

/* ct_reset is internal to one compiled blob. Its callers rely on r0 and other
 * registers that AAPCS would permit clobbering. Use only the original reset's
 * r1-r3 scratch registers, then replay the stolen prologue and original loop.
 */
.section .text.h07_reset_gateway,"ax",%progbits
.global h07_reset_gateway
.type h07_reset_gateway,%function
.thumb_func
h07_reset_gateway:
    ldr r2,=0x20205054
    movs r3,#1
    strb r3,[r2]
    movs r3,#0
    ldr r2,=0x20205002
    b.w ct_reset_resume
    .ltorg
.size h07_reset_gateway,.-h07_reset_gateway

/* Exact displaced stock formatter prologue: no PC-relative instruction.
 * Original +4 performs the remaining stack allocation/argument reads.
 */
.section .text.h07_stock_value_gateway,"ax",%progbits
.global h07_stock_value_gateway
.type h07_stock_value_gateway,%function
.thumb_func
h07_stock_value_gateway:
    .space 4  /* supplied from the user input by the browser copy recipe */
    b.w stock_value_resume
.size h07_stock_value_gateway,.-h07_stock_value_gateway
