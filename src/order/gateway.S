.syntax unified
.cpu cortex-m7
.thumb
.section .text.h11_editor_gateway,"ax",%progbits
.global h11_editor_gateway
.type h11_editor_gateway,%function
.thumb_func
h11_editor_gateway:
    /* Replay both displaced loads before choosing the Chance-only route. */
    .space 6  /* supplied from the user input by the browser copy recipe */
    cmp r1,#4
    bne .Lfallback
    mov r0,r2
    ldr r1,[sp,#4]                 /* stock selection frame's signed delta */
    push {r2,r3,r12,lr}            /* keep stock stack 8-byte aligned */
    bl h11_chance_edit
    pop {r2,r3,r12,lr}
    movs r1,#4
    b.w h11_stock_store
.Lfallback:
    b.w h11_stock_arithmetic
.size h11_editor_gateway,.-h11_editor_gateway
.thumb_set h11_stock_arithmetic,0x600AF17F
.thumb_set h11_stock_store,0x600AF1BD
