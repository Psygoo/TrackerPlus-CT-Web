.syntax unified
.cpu cortex-m7
.thumb
.section .text.hooks,"ax",%progbits
.global wr2_boot_hook
.thumb_func
wr2_boot_hook:
    push {r0-r4,lr}
    bl wr2_boot
    pop {r0-r4,lr}
    ldr r12,=0x60166295
    bx r12

/* Stock New/default entry originally starts PUSH then MOV r4,r0.
 * Execute the metadata clear before entering that original operation. */
.global wr2_new_hook
.thumb_func
wr2_new_hook:
    push {r0-r4,lr}
    bl wr2_new_clear
    pop {r0-r4,lr}
    push {r3-r7,lr}
    mov r4,r0
    ldr r12,=0x60117c91
    bx r12
