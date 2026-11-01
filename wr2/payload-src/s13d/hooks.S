.syntax unified
.thumb
.cpu cortex-m7

.section .text.s13b_refresh_tail,"ax",%progbits
.global s13b_refresh_tail
.type s13b_refresh_tail,%function
.thumb_func
s13b_refresh_tail:
    @ Entered from the established S12C WR2 refresh epilogue with its original
    @ {r4-r8,lr} frame still on the stack.  Synchronize in the foreground,
    @ then reproduce the displaced S12C tail exactly.
    bl      s13b_led_sync
    ldr     r0, =0x20027d64
    pop.w   {r4, r5, r6, r7, r8, lr}
    ldr     r3, =0x60102015
    bx      r3
.size s13b_refresh_tail,.-s13b_refresh_tail

.section .text.s13b_cleanup_hook,"ax",%progbits
.global s13b_cleanup_hook
.type s13b_cleanup_hook,%function
.thumb_func
s13b_cleanup_hook:
    @ Sample Playback vtable +8 owns normal screen deactivation.  Restore the
    @ current native A/B composition before stock clears screen+4.
    push    {r4, lr}
    mov     r4, r0
    bl      s13b_led_force_restore
    mov     r0, r4
    ldr     r12, =0x600f3e51
    blx     r12
    pop     {r4, pc}
.size s13b_cleanup_hook,.-s13b_cleanup_hook
