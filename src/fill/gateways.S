.syntax unified
.cpu cortex-m7
.thumb

.section .text.h12_fill_max,"ax",%progbits
.global h12_fill_max
.type h12_fill_max,%function
.thumb_func
h12_fill_max:
    cmp r1,#4
    bne .Loriginal_max
    movs r0,#141
    bx lr
.Loriginal_max:
    b.w h12_stock_max
.size h12_fill_max,.-h12_fill_max

.section .text.h12_generator_max,"ax",%progbits
.global h12_generator_max
.type h12_generator_max,%function
.thumb_func
h12_generator_max:
    cmp r1,#4
    bne .Lgenerator_stock
    ldr r12,[sp,#0x18]          /* original decoded Fill type, fixed Chance only */
    cmp r12,#4
    bne .Lgenerator_stock
    movs r0,#141
    bx lr
.Lgenerator_stock:
    b.w h12_stock_max
.size h12_generator_max,.-h12_generator_max

.section .text.h12_fromto_store,"ax",%progbits
.global h12_fromto_store
.type h12_fromto_store,%function
.thumb_func
h12_fromto_store:
    ldr r12,[sp,#0x18]
    cmp r12,#4
    bne .Lstore
    push {r1,r2,r3,lr}
    bl h11_chance_value
    pop {r1,r2,r3,lr}
.Lstore:
    .space 4  /* supplied from the user input by the browser copy recipe */
    bx lr
.size h12_fromto_store,.-h12_fromto_store

.section .text.h12_random_gateway,"ax",%progbits
.global h12_random_gateway
.type h12_random_gateway,%function
.thumb_func
h12_random_gateway:
    ldr r2,[sp,#0x18]
    b.w h12_random_value
.size h12_random_gateway,.-h12_random_gateway

.thumb_set h12_stock_max,0x600AF2C9
.thumb_set h11_chance_value,0x601BC4A9
