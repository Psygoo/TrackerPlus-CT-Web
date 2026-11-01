.syntax unified
.thumb
.cpu cortex-m7

.section .text.wr2_more_handler, "ax", %progbits
.global wr2_more_handler
.type wr2_more_handler, %function
.thumb_func
wr2_more_handler:
    push    {r4, r5, lr}
    sub     sp, #4                  @ keep AAPCS stack alignment
    mov     r4, r0                  @ Sample Playback screen object
    cmp     r3, #1                  @ caller arrives with mode - 4
    beq     .Lbeat_slice

    @ Preserve the stock mode-4 two-page toggle byte-for-byte in meaning.
    ldrb.w  r2, [r4, #0xf0]
    eor.w   r2, r2, #1
    strb.w  r2, [r4, #0xf0]
    cbnz    r2, .Lrefresh
    movs    r2, #0
    strb.w  r2, [r4, #0xd6]
    b       .Lrefresh

.Lbeat_slice:
    ldrb.w  r5, [r4, #0xf0]         @ old page state: 0, 1, or 2
    adds    r2, r5, #1
    cmp     r2, #3
    bne     .Lstore_page
    movs    r2, #0
.Lstore_page:
    strb.w  r2, [r4, #0xf0]
    strb.w  r2, [r4, #0xc2]         @ logical page survives stock Slice refresh
    strb.w  r2, [r4, #0xc1]

    @ Use the existing Slice focus/reset behavior on page 1 and page 3.
    cmp     r2, #0
    beq     .Lfocus_slice
    cmp     r2, #2
    bne     .Lbindings
.Lfocus_slice:
    movs    r3, #0
    strb.w  r3, [r4, #0xd6]

.Lbindings:
    cmp     r2, #2
    beq     .Lbind_inert
    cmp     r5, #2
    bne     .Lrefresh
    bl      wr2_restore_handlers
    b       .Lrefresh
.Lbind_inert:
    bl      wr2_inert_handlers

.Lrefresh:
    mov     r0, r4
    ldr     r3, =0x600f7c55
    blx     r3
    @ Page presentation and focus metadata are applied by stock refresh plus
    @ the Page 3 presentation epilogue.
.Lreturn:
    add     sp, #4
    pop     {r4, r5, pc}
.size wr2_more_handler, .-wr2_more_handler


.section .aftertext.wr2_after_stock_control_init, "ax", %progbits
.global wr2_after_stock_control_init
.type wr2_after_stock_control_init, %function
.thumb_func
wr2_after_stock_control_init:
    @ The stock Page 1 softkey initializer tail-calls 0x600cf7a8.
    @ Finish that registration first, then restore Page 3's private bindings
    @ if the preserved logical page is still Page 3. This shared tail covers
    @ every Sample Playback screen-entry route that uses the stock initializer.
    push    {r4, lr}
    ldr     r12, =0x600cf7a9
    blx     r12
    ldr     r4, =0x200254b4
    ldr.w   r0, [r4, #0xf4]
    cbz     r0, .Lpost_init_return
    ldrb.w  r0, [r0, #0x3c]
    cmp     r0, #5
    bne     .Lpost_init_return
    ldrb.w  r0, [r4, #0xc2]
    cmp     r0, #2
    bne     .Lpost_init_return
    mov     r0, r4
    bl      wr2_inert_handlers
.Lpost_init_return:
    pop     {r4, pc}
.size wr2_after_stock_control_init, .-wr2_after_stock_control_init


.section .text.wr2_refresh_epilogue, "ax", %progbits
.global wr2_refresh_epilogue
.type wr2_refresh_epilogue, %function
.thumb_func
wr2_refresh_epilogue:
    @ Hooked at 0x600f8002 before the stock ldr/pop/tail-call sequence.
    @ Resolve the singleton Sample Playback screen explicitly here. The
    @ refresh can arrive with r4 holding a stock control index on some paths.
    ldr     r4, =0x200254b4
    mov     r0, r4
    bl      wr2_group_edit_maintain
    ldr.w   r0, [r4, #0xf4]
    ldrb.w  r0, [r0, #0x3c]
    cmp     r0, #5
    bne     .Lrefresh_other_mode

    @ Page-count byte. c2 is maintained by More so stock temporary f0 changes
    @ during the Slice adjustment path do not hide the logical Page 3 state.
    movs    r1, #3
    strb.w  r1, [r4, #0xc0]
    ldrb.w  r1, [r4, #0xc2]
    strb.w  r1, [r4, #0xc1]
    cmp     r1, #2
    bne     .Lrefresh_tail

    @ Page 3: re-apply controls on every refresh, without resetting focus.
    mov     r0, r4
    bl      wr2_set_page3_labels
    @ The stock f0!=0 branch creates the Page 2 value for “No of Slices”.
    @ Reuse the complete Page 1 Slice formatter and its stock waveform update,
    @ then clear the four secondary values on Page 3 action controls.
    mov     r0, r4
    ldr     r3, =0x600f77f9
    blx     r3
    mov     r0, r4
    bl      wr2_page3_post_refresh
    movs    r1, #2
    strb.w  r1, [r4, #0xf0]
    b       .Lrefresh_tail

.Lrefresh_other_mode:
    @ Return to stock count if a mode switch left the WR2 count in place.
    movs    r1, #0
    strb.w  r1, [r4, #0xc2]
    strb.w  r1, [r4, #0xc1]
    ldrb.w  r1, [r4, #0xc0]
    cmp     r1, #3
    bne     .Lrefresh_tail
    movs    r1, #2
    strb.w  r1, [r4, #0xc0]

.Lrefresh_tail:
    @ Recreate the stock epilogue and tail-call exactly: display manager in r0,
    @ original saved registers restored, then 0x60102014.
    ldr     r0, =0x20027d64
    pop.w   {r4, r5, r6, r7, r8, lr}
    ldr     r3, =0x60102015
    bx      r3
.size wr2_refresh_epilogue, .-wr2_refresh_epilogue

.section .text.wr2_refresh_entry, "ax", %progbits
.type wr2_refresh_entry, %function
.thumb_func
wr2_refresh_entry:
    @ Run the stock Page 1 setup path while Page 3 is logically active. This
    @ gives Page 3 Slice the original selectable widget type, focus flags and
    @ button mapping before Page 3's labels and values are reapplied.
    ldr     r12, =0x200254b4
    ldr     r12, [r12, #0xf4]
    ldrb.w  r12, [r12, #0x3c]
    cmp     r12, #5
    bne     .Lrefresh_entry_stock
    ldr     r12, =0x200254b4
    ldrb.w  r12, [r12, #0xc2]
    cmp     r12, #2
    bne     .Lrefresh_entry_stock
    ldr     r12, =0x200254b4
    movs    r3, #0
    strb.w  r3, [r12, #0xf0]
.Lrefresh_entry_stock:
    @ Replicate the displaced stock push and continue at its first body op.
    push.w  {r4, r5, r6, r7, r8, lr}
    ldr     r3, =0x600f7c59
    bx      r3
.size wr2_refresh_entry, .-wr2_refresh_entry

.type wr2_set_page3_labels, %function
.thumb_func
wr2_set_page3_labels:
    push    {r4, lr}
    mov     r4, r0
    ldr     r0, =0x20027d64
    ldrsb.w r1, [r4, #0xc4]
    ldr     r2, =wr2_label_slice
    ldr     r3, =0x60101d5d
    blx     r3
    ldr     r0, =0x20027d64
    ldrsb.w r1, [r4, #0xc5]
    ldr     r2, =wr2_label_trig
    ldr     r3, =0x60101d5d
    blx     r3
    ldr     r0, =0x20027d64
    ldrsb.w r1, [r4, #0xc6]
    ldr     r2, =wr2_label_add
    ldr     r3, =0x60101d5d
    blx     r3
    ldr     r0, =0x20027d64
    ldrsb.w r1, [r4, #0xc7]
    ldr     r2, =wr2_label_remove
    ldr     r3, =0x60101d5d
    blx     r3
    ldr     r0, =0x20027d64
    ldrsb.w r1, [r4, #0xc8]
    ldr     r2, =wr2_label_weight
    ldr     r3, =0x60101d5d
    blx     r3
    ldr     r0, =0x20027d64
    ldrsb.w r1, [r4, #0xc9]
    ldr     r2, =wr2_label_more
    ldr     r3, =0x60101d5d
    blx     r3
    pop     {r4, pc}
.size wr2_set_page3_labels, .-wr2_set_page3_labels

.type wr2_clear_page3_values, %function
.thumb_func
wr2_clear_page3_values:
    push    {r4, lr}
    mov     r4, r0
    .irp off, 0xc5, 0xc6, 0xc7, 0xc8
    ldr     r0, =0x20027d64
    ldrsb.w r1, [r4, #\off]
    ldr     r2, =wr2_empty
    ldr     r12, =0x60101d75
    blx     r12
    ldr     r0, =0x20027d64
    ldrsb.w r1, [r4, #\off]
    ldr     r12, =0x60101cf5
    blx     r12
    ldr     r0, =0x20027d64
    ldrsb.w r1, [r4, #\off]
    ldr     r12, =0x60101e85
    blx     r12
    .endr
    pop     {r4, pc}
.size wr2_clear_page3_values, .-wr2_clear_page3_values

.type wr2_inert_handlers, %function
.thumb_func
wr2_inert_handlers:
    push    {r4, lr}
    ldr     r4, =0x200209e0
    @ Page 3 encoder/focus input uses the distinct manager +0x18 table.
    ldr     r0, [r4]
    movs    r1, #0
    ldr     r2, =wr2_page3_input_cb + 1
    movs    r3, #0
    ldr     r12, =0x600cf6ad
    blx     r12
    @ Index 0 is the encoder input path. Preserve its Page 1 Slice behavior
    @ through a Page 3-aware wrapper.
    ldr     r0, [r4]
    movs    r1, #0
    ldr     r2, =wr2_page3_slice_cb + 1
    ldr     r12, =0x600cf74d
    blx     r12
    .irp idx, 1, 2, 3, 4
    ldr     r0, [r4]
    movs    r1, #\idx
    .if \idx == 1
    @ Slot 1 is the Trig button on Page 3. Bind a dedicated callback while
    @ leaving the original Page 1/2 callback table untouched.
    ldr     r2, =wr2_page3_trig_cb + 1
    .elseif \idx == 2
    ldr     r2, =wr2_page3_cb2 + 1
    .elseif \idx == 3
    ldr     r2, =wr2_page3_cb3 + 1
    .else
    ldr     r2, =wr2_page3_cb4 + 1
    .endif
    ldr     r12, =0x600cf74d
    blx     r12
    .endr
    pop     {r4, pc}
.size wr2_inert_handlers, .-wr2_inert_handlers

.type wr2_restore_handlers, %function
.thumb_func
wr2_restore_handlers:
    push    {r4, lr}
    ldr     r4, =0x200209e0
    ldr     r0, [r4]
    movs    r1, #0
    ldr     r2, =0x600c92e1
    movs    r3, #0
    ldr     r12, =0x600cf6ad
    blx     r12
    ldr     r0, [r4]
    movs    r1, #0
    ldr     r2, =0x600f5761
    ldr     r12, =0x600cf74d
    blx     r12
    .irp idx, 1, 2, 3, 4
    ldr     r0, [r4]
    movs    r1, #\idx
    .if \idx == 1
    ldr     r2, =0x600f5751
    .elseif \idx == 2
    ldr     r2, =0x600f4c89
    .elseif \idx == 3
    ldr     r2, =0x600f4b55
    .else
    ldr     r2, =0x600f4e15
    .endif
    ldr     r12, =0x600cf74d
    blx     r12
    .endr
    pop     {r4, pc}
.size wr2_restore_handlers, .-wr2_restore_handlers

@ The manager's +0x18 callback dispatches encoder/focus adjustments. On Page
@ 3, visible controls 1..4 have no adjustment action; Slice remains stock.
.type wr2_page3_input_cb, %function
.thumb_func
wr2_page3_input_cb:
    @ Keep the stock callback's 8-byte stack alignment across C and refresh
    @ calls. The old three-register push misaligned the VFP waveform renderer.
    push    {r4, lr}
    mov     r4, r0
    ldr     r12, =0x200254b4
    ldr     r12, [r12, #0xf4]
    ldrb.w  r12, [r12, #0x3c]
    cmp     r12, #5
    bne     .Lpage3_input_stock
    ldr     r12, =0x200254b4
    ldrb.w  r12, [r12, #0xf0]
    cmp     r12, #2
    bne     .Lpage3_input_stock
    ldr     r12, =0x200254b4
    ldrb.w  r12, [r12, #0xd6]
    cmp     r12, #0
    beq     .Lpage3_input_slice
    cmp     r12, #4
    beq     .Lpage3_input_weight
    b       .Lpage3_input_inert
.Lpage3_input_slice:
    @ Slice focus keeps the complete stock jog/marker behavior, then forces
    @ the normal Page 3 presentation pass so the # of # string stays current.
    mov     r0, r4
    ldr     r12, =0x600c92e1
    blx     r12
    ldr     r0, =0x200254b4
    ldr     r12, =0x600f7c55
    blx     r12
    b       .Lpage3_input_return
.Lpage3_input_weight:
    @ Jog direction is supplied in r2 by the stock focus manager. The C
    @ helper validates the locked group and performs one 1% step.
    mov     r0, r2
    bl      wr2_weight_adjust_c
    ldr     r0, =0x200254b4
    ldr     r12, =0x600f7c55
    blx     r12
    b       .Lpage3_input_return
.Lpage3_input_stock:
    mov     r0, r4
    ldr     r12, =0x600c92e1
    blx     r12
    b       .Lpage3_input_return
.Lpage3_input_inert:
    @ Controls Trig/Add/Remove/Weight have no encoder adjustment path on P3.
.Lpage3_input_return:
    pop     {r4, pc}
.size wr2_page3_input_cb, .-wr2_page3_input_cb

@ Keep these table entries safe if the user changes playback mode while page 3
@ is open. The stock mode-change path clears +0xf0, so the controls immediately
@ resume their original handlers even before the next More press.
.macro PAGE3_WRAPPER name, target
.type \name, %function
.thumb_func
\name:
    ldr     r12, =\target
    b.w     wr2_page3_inert_dispatch
.size \name, .-\name
.endm

PAGE3_WRAPPER wr2_page3_cb2, wr2_page3_add_callback + 1
PAGE3_WRAPPER wr2_page3_cb3, wr2_page3_remove_callback + 1
PAGE3_WRAPPER wr2_page3_cb4, wr2_page3_weight_cb + 1

.type wr2_page3_inert_dispatch, %function
.thumb_func
wr2_page3_inert_dispatch:
    push    {r4, r5, lr}
    sub     sp, #4
    mov     r4, r0
    mov     r5, r12
    ldr     r12, =0x200254b4
    ldr     r12, [r12, #0xf4]
    cmp     r12, #0
    beq     .Linert_page1_2
    ldrb.w  r12, [r12, #0x3c]
    cmp     r12, #5
    bne     .Linert_page1_2
    ldr     r12, =0x200254b4
    ldrb.w  r12, [r12, #0xf0]
    cmp     r12, #2
    beq     .Linert_page3
.Linert_page1_2:
    mov     r0, r4
    blx     r5
    b       .Linert_done
.Linert_page3:
    @ Add and Remove are the implemented Page 3 action callbacks. Other
    @ visible action rows remain inert even if their wrappers are called.
    ldr     r12, =wr2_page3_add_callback + 1
    cmp     r5, r12
    beq     .Linert_call
    ldr     r12, =wr2_page3_remove_callback + 1
    cmp     r5, r12
    beq     .Linert_call
    ldr     r12, =wr2_page3_weight_cb + 1
    cmp     r5, r12
    bne     .Linert_done
.Linert_call:
    mov     r0, r4
    blx     r5
.Linert_done:
    add     sp, #4
    pop     {r4, r5, pc}
.size wr2_page3_inert_dispatch, .-wr2_page3_inert_dispatch

.type wr2_page3_slice_cb, %function
.thumb_func
wr2_page3_slice_cb:
    push    {r4, r5, lr}
    sub     sp, #4
    mov     r5, r0                  @ preserve encoder event
    ldr     r4, =0x200254b4
    ldr     r0, [r4, #0xf4]
    ldrb.w  r0, [r0, #0x3c]
    cmp     r0, #5
    bne     .Lslice_cb_passthrough
    ldrb.w  r0, [r4, #0xf0]
    cmp     r0, #2
    bne     .Lslice_cb_passthrough
    movs    r0, #0
    strb.w  r0, [r4, #0xf0]
    @ The type-3 softkey dispatcher supplies a control/filter value in r0,
    @ while the stock Slice focus handler expects event 1. Translate the
    @ Page 3 Slice press to the same stock focus event used on Page 1.
    movs    r0, #1
    ldr     r3, =0x600f5761
    blx     r3
    movs    r0, #2
    strb.w  r0, [r4, #0xf0]
    mov     r0, r4
    ldr     r3, =0x600f7c55
    blx     r3
    b       .Lslice_cb_done
.Lslice_cb_passthrough:
    mov     r0, r5
    ldr     r3, =0x600f5761
    blx     r3
.Lslice_cb_done:
    add     sp, #4
    pop     {r4, r5, pc}
.size wr2_page3_slice_cb, .-wr2_page3_slice_cb

.type wr2_page3_weight_cb, %function
.thumb_func
wr2_page3_weight_cb:
    push    {r4, lr}
    mov     r4, r0
    ldr     r12, =0x200254b4
    ldr     r12, [r12, #0xf4]
    cmp     r12, #0
    beq     .Lweight_softkey_stock
    ldrb.w  r12, [r12, #0x3c]
    cmp     r12, #5
    bne     .Lweight_softkey_stock
    ldr     r12, =0x200254b4
    ldrb.w  r12, [r12, #0xc2]
    cmp     r12, #2
    bne     .Lweight_softkey_stock
    cmp     r4, #1
    beq     .Lweight_softkey_select
    cmp     r4, #0xff
    bne     .Lweight_softkey_done
.Lweight_softkey_select:
    ldr     r12, =0x200254b4
    movs    r3, #4
    strb.w  r3, [r12, #0xd6]
    ldr     r0, =0x200254b4
    ldr     r12, =0x600f7c55
    blx     r12
    b       .Lweight_softkey_done
.Lweight_softkey_stock:
    mov     r0, r4
    ldr     r12, =0x600f4e15
    blx     r12
.Lweight_softkey_done:
    pop     {r4, pc}
.size wr2_page3_weight_cb, .-wr2_page3_weight_cb

.align 2
wr2_label_slice:  .asciz "Slice"
wr2_label_trig:   .asciz "Trig"
wr2_label_add:    .asciz "Add"
wr2_label_remove: .asciz "Remove"
wr2_label_weight: .asciz "Weight"
wr2_label_more:   .asciz "More"
wr2_empty:        .asciz ""

.ltorg

.section .text.wr2_zz_page3_action_values, "ax", %progbits
.align 2
.type wr2_page3_post_refresh, %function
.thumb_func
wr2_page3_post_refresh:
    push    {r4, lr}
    mov     r4, r0
    bl      wr2_clear_page3_values
    mov     r0, r4
    bl      wr2_set_page3_action_values
    bl      wr2_weight_update_display_c
    pop     {r4, pc}
.size wr2_page3_post_refresh, .-wr2_page3_post_refresh

.align 2
.type wr2_set_page3_action_values, %function
.thumb_func
wr2_set_page3_action_values:
    push    {r4, lr}
    mov     r4, r0
    .irp off, 0xc5, 0xc6, 0xc7, 0xc8
    ldr     r0, =0x20027d64
    ldrsb.w r1, [r4, #\off]
    .if \off == 0xc8
    movs    r2, #1
    .else
    movs    r2, #0
    .endif
    ldr     r3, =0x60101d8d
    blx     r3
    ldr     r0, =0x20027d64
    ldrsb.w r1, [r4, #\off]
    ldr     r3, =0x60101e85
    blx     r3
    .endr
    pop     {r4, pc}
.size wr2_set_page3_action_values, .-wr2_set_page3_action_values
.ltorg

@ The group table is deliberately outside WR1's RNG words at 0x20205060..68
@ and within the documented unused OCRAM tail. One row is allocated for each
@ stock instrument; ownership is per slice, so a slice cannot join two groups.
.equ WR2_GROUP_STATE, 0x20205068
.equ WR2_GROUP_MAGIC, 0x31524757 @ "WGR1"
.equ WR2_MAX_INSTRUMENTS, 48
.equ WR2_MAX_SLICES, 48
.equ WR2_EDIT_STATE, 0x202063e8 @ 12-byte volatile session record after group rows
.equ WR2_EDIT_MAGIC, 0x31454757 @ "WGE1"

.type wr2_page3_trig_cb, %function
.thumb_func
wr2_page3_trig_cb:
    push    {r4, lr}
    mov     r4, r0                  @ stock callback event: 0xff/1 press, 0 release
    @ This callback is installed only while Page 3 is active, but validate
    @ both mode and page again in case the UI state changed before dispatch.
    ldr     r12, =0x200254b4
    ldr     r12, [r12, #0xf4]
    cmp     r12, #0
    beq     .Ltrig_stock
    ldrb.w  r12, [r12, #0x3c]
    cmp     r12, #5
    bne     .Ltrig_stock
    ldr     r12, =0x200254b4
    ldrb.w  r12, [r12, #0xf0]
    cmp     r12, #2
    bne     .Ltrig_stock
    cmp     r4, #0
    beq     .Ltrig_release
    cmp     r4, #1
    beq     .Ltrig_press
    cmp     r4, #0xff
    beq     .Ltrig_press
    b       .Ltrig_stock
.Ltrig_press:
    bl      wr2_toggle_group_edit
    b       .Ltrig_cb_done
.Ltrig_release:
    @ Let the original control handler finish its momentary release path, but
    @ do not let a release become a second edit-session toggle.
    mov     r0, r4
    ldr     r12, =0x600f5751
    blx     r12
    ldr     r0, =WR2_EDIT_STATE
    ldr     r1, [r0]
    ldr     r2, =WR2_EDIT_MAGIC
    cmp     r1, r2
    bne     .Ltrig_cb_done
    bl      wr2_group_edit_apply
    b       .Ltrig_cb_done
.Ltrig_stock:
    mov     r0, r4
    ldr     r12, =0x600f5751
    blx     r12
.Ltrig_cb_done:
    pop     {r4, pc}
.size wr2_page3_trig_cb, .-wr2_page3_trig_cb

.type wr2_create_group, %function
.thumb_func
wr2_create_group:
    push    {r4-r7, lr}
    sub     sp, #4                  @ keep AAPCS stack alignment
    ldr     r4, =0x200254b4
    ldr     r5, [r4, #0xf4]        @ current instrument record
    cmp     r5, #0
    beq     .Ltrig_done
    ldrb.w  r0, [r5, #0x3c]
    cmp     r0, #5                 @ Beat Slice only
    bne     .Ltrig_done
    ldrb.w  r0, [r4, #0xf0]
    cmp     r0, #2                 @ Page 3 only
    bne     .Ltrig_done
    ldrb.w  r6, [r5, #0x168]       @ current slice count
    cmp     r6, #1
    blo     .Ltrig_done
    cmp     r6, #WR2_MAX_SLICES
    bhi     .Ltrig_done
    ldrb.w  r7, [r5, #0x169]       @ selected slice is the group master
    cmp     r7, r6
    bhs     .Ltrig_done

    @ Resolve this record against the stock 48-entry instrument table.
    movs    r3, #0
    ldr     r0, =0x20200000
    ldr     r1, =0x174
.Ltrig_find_instrument:
    mul     r2, r3, r1
    adds    r2, r0, r2
    cmp     r2, r5
    beq     .Ltrig_instrument_found
    adds    r3, #1
    cmp     r3, #WR2_MAX_INSTRUMENTS
    blo     .Ltrig_find_instrument
    b       .Ltrig_done

.Ltrig_instrument_found:
    @ 104-byte rows: magic +0, slice count +4, schema +5, owners +8,
    @ weights +56. Owner 0xff means unassigned; owner[i]==i is a master.
    mov     r0, r3
    lsls    r1, r0, #6             @ slot * 64
    lsls    r2, r0, #5             @ slot * 32
    adds    r1, r1, r2
    lsls    r2, r0, #3             @ slot * 8
    adds    r1, r1, r2             @ slot * 104
    ldr     r0, =WR2_GROUP_STATE
    adds    r3, r0, r1

    ldr     r0, =WR2_GROUP_MAGIC
    ldr     r1, [r3]
    cmp     r1, r0
    bne     .Ltrig_initialize_record
    ldrb.w  r1, [r3, #4]
    cmp     r1, r6
    bne     .Ltrig_initialize_record
    ldrb.w  r1, [r3, #5]
    cmp     r1, #1
    bne     .Ltrig_initialize_record
    b       .Ltrig_assign_master

.Ltrig_initialize_record:
    @ Initialize bounded member arrays before publishing the valid header.
    movs    r0, #0
    mvns    r0, r0
    adds    r1, r3, #8
    movs    r2, #12                @ 48 owner bytes, four at a time
.Ltrig_clear_owners:
    str     r0, [r1], #4
    subs    r2, #1
    bne     .Ltrig_clear_owners
    adds    r1, r3, #56
    movs    r0, #0
    movs    r2, #12                @ unassigned slices have no weight
.Ltrig_clear_weights:
    str     r0, [r1], #4
    subs    r2, #1
    bne     .Ltrig_clear_weights
    strb    r6, [r3, #4]
    movs    r0, #1
    strb    r0, [r3, #5]
    movs    r0, #0
    strh    r0, [r3, #6]
    ldr     r0, =WR2_GROUP_MAGIC
    str     r0, [r3]

.Ltrig_assign_master:
    adds    r1, r3, #8
    ldrb    r2, [r1, r7]
    cmp     r2, r7                 @ same master: idempotent
    beq     .Ltrig_done
    cmp     r2, #0xff
    bne     .Ltrig_done             @ a slice cannot belong to two groups
    strb    r7, [r1, r7]
    adds    r1, r3, #56
    movs    r2, #100
    strb    r2, [r1, r7]            @ master is first member at 100 percent

.Ltrig_done:
    add     sp, #4
    pop     {r4-r7, pc}
.size wr2_create_group, .-wr2_create_group
.ltorg

.type wr2_page3_pad_callback_gate, %function
.thumb_func
wr2_page3_pad_callback_gate:
    @ This is the pad-selection handler's stock formatter gate at 0x600f3f2a.
    @ Page 1 (f0=0) already enters the common value-format path. Allow only
    @ Beat Slice Page 3 (mode 5, f0=2) to enter that same stock block.
    ldr     r3, [r4, #0xf4]
    ldrb.w  r3, [r3, #0x3c]
    cmp     r3, #5
    bne     .Lpad_gate_stock
    ldrb.w  r3, [r4, #0xf0]
    cmp     r3, #2
    beq     .Lpad_gate_weight_refresh
.Lpad_gate_stock:
    ldrb.w  r3, [r4, #0xf0]
    cmp     r3, #0
    beq     .Lpad_gate_format
    ldr     r12, =0x600f3ed1         @ original non-Page-1 skip path
    bx      r12
.Lpad_gate_format:
    ldr     r12, =0x600f3ec5         @ stock calls to 77f8 and 7860 + d9 set
    bx      r12
.Lpad_gate_weight_refresh:
    @ Reproduce the stock format path, then synchronize Weight from the actual
    @ pad-selected slice before returning through the stock callback epilogue.
    push    {r4, lr}
    mov     r0, r4
    ldr     r12, =0x600f77f9
    blx     r12
    mov     r0, r4
    ldr     r12, =0x600f7861
    blx     r12
    movs    r3, #1
    strb.w  r3, [r4, #0xd9]
    bl      wr2_weight_update_display_c
    pop     {r4, lr}
    ldr     r12, =0x600f3ed5
    bx      r12
.size wr2_page3_pad_callback_gate, .-wr2_page3_pad_callback_gate
.ltorg

.type wr2_group_marker_color, %function
.thumb_func
wr2_group_marker_color:
    @ Role codes: 0 stock, 1 active master, 2 active member, 3 foreign master,
    @ 4 foreign member, 5 unassigned during edit. Only RGB is changed.
    sxth.w  r8, r8
    push    {r4, lr}
    bl      wr2_marker_group_role
    pop     {r4, lr}
    cmp     r0, #1
    beq     .Lgroup_marker_red
    cmp     r0, #2
    beq     .Lgroup_marker_orange
    cmp     r0, #3
    beq     .Lgroup_marker_red_dim
    cmp     r0, #4
    beq     .Lgroup_marker_orange_dim
.Lgroup_marker_stock:
    cmp     r0, #5
    beq     .Lgroup_marker_stock_dim
    ldr     r0, [r5, #0x28]
    ldr     r0, [r0]              @ stock palette entry 0
    b       .Lgroup_marker_rejoin
.Lgroup_marker_stock_dim:
    ldr     r0, [r5, #0x28]
    ldr     r0, [r0]
    bl      wr2_half_rgb
    b       .Lgroup_marker_rejoin
.Lgroup_marker_red:
    ldr     r0, =0x00ff0000       @ EVE RGB888 red
    b       .Lgroup_marker_rejoin
.Lgroup_marker_orange:
    ldr     r0, =0x00ff8000       @ EVE RGB888 orange for group members
    b       .Lgroup_marker_rejoin
.Lgroup_marker_red_dim:
    ldr     r0, =0x00ff0000
    bl      wr2_half_rgb
    b       .Lgroup_marker_rejoin
.Lgroup_marker_orange_dim:
    ldr     r0, =0x00ff8000
    bl      wr2_half_rgb
.Lgroup_marker_rejoin:
    ldr     r12, =0x60107d67      @ resume at stock eve_color_rgb call
    bx      r12
.size wr2_group_marker_color, .-wr2_group_marker_color
.ltorg

.type wr2_group_marker_selected_palette2, %function
.thumb_func
wr2_group_marker_selected_palette2:
    push    {r4, lr}
    bl      wr2_marker_group_role
    pop     {r4, lr}
    cmp     r0, #1
    beq     .Lselected_palette2_red
    cmp     r0, #2
    beq     .Lselected_palette2_orange
    cmp     r0, #3
    beq     .Lselected_palette2_red_dim
    cmp     r0, #4
    beq     .Lselected_palette2_orange_dim
.Lselected_palette2_stock:
    cmp     r0, #5
    beq     .Lselected_palette2_stock_dim
    ldr     r1, [r5, #0x28]
    ldr     r0, [r1, #8]
    b       .Lselected_palette2_stock_rejoin
.Lselected_palette2_stock_dim:
    ldr     r1, [r5, #0x28]
    ldr     r0, [r1, #8]
    bl      wr2_half_rgb
.Lselected_palette2_stock_rejoin:
    ldr     r12, =0x60107cb7
    bx      r12
.Lselected_palette2_red:
    ldr     r0, =0x00ff0000
    ldr     r12, =0x60107cb7      @ stock eve_color_rgb call
    bx      r12
.Lselected_palette2_orange:
    ldr     r0, =0x00ff8000
    ldr     r12, =0x60107cb7
    bx      r12
.Lselected_palette2_red_dim:
    ldr     r0, =0x00ff0000
    bl      wr2_half_rgb
    ldr     r12, =0x60107cb7
    bx      r12
.Lselected_palette2_orange_dim:
    ldr     r0, =0x00ff8000
    bl      wr2_half_rgb
    ldr     r12, =0x60107cb7
    bx      r12
.size wr2_group_marker_selected_palette2, .-wr2_group_marker_selected_palette2

.type wr2_group_marker_selected_palette1, %function
.thumb_func
wr2_group_marker_selected_palette1:
    push    {r4, lr}
    bl      wr2_marker_group_role
    pop     {r4, lr}
    cmp     r0, #1
    beq     .Lselected_palette1_red
    cmp     r0, #2
    beq     .Lselected_palette1_orange
    cmp     r0, #3
    beq     .Lselected_palette1_red_dim
    cmp     r0, #4
    beq     .Lselected_palette1_orange_dim
.Lselected_palette1_stock:
    cmp     r0, #5
    beq     .Lselected_palette1_stock_dim
    ldr     r1, [r5, #0x28]
    ldr     r0, [r1, #4]
    b       .Lselected_palette1_stock_rejoin
.Lselected_palette1_stock_dim:
    ldr     r1, [r5, #0x28]
    ldr     r0, [r1, #4]
    bl      wr2_half_rgb
.Lselected_palette1_stock_rejoin:
    b       .Lselected_palette1_rejoin
.Lselected_palette1_red:
    ldr     r0, =0x00ff0000
    b       .Lselected_palette1_rejoin
.Lselected_palette1_orange:
    ldr     r0, =0x00ff8000
    b       .Lselected_palette1_rejoin
.Lselected_palette1_red_dim:
    ldr     r0, =0x00ff0000
    bl      wr2_half_rgb
    b       .Lselected_palette1_rejoin
.Lselected_palette1_orange_dim:
    ldr     r0, =0x00ff8000
    bl      wr2_half_rgb
    b       .Lselected_palette1_rejoin
.Lselected_palette1_rejoin:
    ldr     r12, =0x60107dbb      @ continue at stock branch to color call
    bx      r12
.size wr2_group_marker_selected_palette1, .-wr2_group_marker_selected_palette1
.ltorg

.type wr2_marker_group_role, %function
.thumb_func
wr2_marker_group_role:
    @ r5=marker widget, r2=marker index. Return 0 stock, 1/2 active-group
    @ master/member, 3/4 foreign master/member, or 5 unassigned during edit.
    movs    r0, #0
    ldr     r1, =0x200254b4
    ldrb.w  r3, [r1, #0xf0]
    cmp     r3, #2
    bne     .Lmarker_master_return
    ldr     r3, [r1, #0xf4]
    cmp     r3, #0
    beq     .Lmarker_master_return
    ldrb.w  r12, [r3, #0x3c]
    cmp     r12, #5
    bne     .Lmarker_master_return
    ldr     r12, [r5, #0x2c]
    ldr     r0, =0x20025f68
    cmp     r12, r0
    bne     .Lmarker_master_return
    ldr     r0, =0x202046c5
    ldrb    r0, [r0]
    cmp     r0, #48
    bhs     .Lmarker_master_return
    movs    r1, #104
    muls    r0, r1
    ldr     r12, =WR2_GROUP_STATE
    adds    r0, r12
    ldr     r12, [r0]
    ldr     r1, =WR2_GROUP_MAGIC
    cmp     r12, r1
    bne     .Lmarker_master_return
    ldrb    r1, [r0, #4]
    cmp     r2, r1
    bhs     .Lmarker_master_return
    ldrb    r1, [r0, #5]
    cmp     r1, #1
    bne     .Lmarker_master_return
    adds    r0, #8
    mov     r3, r0                  @ owner array base
    ldrb    r0, [r0, r2]
    cmp     r0, #0xff
    beq     .Lmarker_unassigned
    cmp     r0, r2
    beq     .Lmarker_role_master
    ldr     r1, =0x200254b4
    ldr     r1, [r1, #0xf4]
    ldrb.w  r1, [r1, #0x168]
    cmp     r0, r1
    bhs     .Lmarker_master_return
    ldrb    r1, [r3, r0]
    cmp     r1, r0
    bne     .Lmarker_master_return
    movs    r1, #2
    b       .Lmarker_check_edit
.Lmarker_role_master:
    movs    r1, #1
.Lmarker_check_edit:
    mov     r4, r0                  @ preserve slice owner/master through checks
    ldr     r12, =WR2_EDIT_STATE
    ldr     r12, [r12]
    ldr     r0, =WR2_EDIT_MAGIC
    cmp     r12, r0
    bne     .Lmarker_role_base
    ldr     r0, =0x202046c5
    ldrb    r0, [r0]
    ldr     r12, =WR2_EDIT_STATE
    ldrb    r12, [r12, #5]
    cmp     r0, r12
    bne     .Lmarker_role_base
    ldr     r12, =WR2_EDIT_STATE
    ldrb    r2, [r12, #6]           @ locked master
    ldr     r12, =0x200254b4
    ldr     r12, [r12, #0xf4]
    ldrb.w  r12, [r12, #0x168]
    cmp     r2, r12
    bhs     .Lmarker_role_base
    ldrb    r12, [r3, r2]
    cmp     r12, r2
    bne     .Lmarker_role_base
    cmp     r1, #0
    beq     .Lmarker_role_unassigned_dim
    cmp     r4, r2
    beq     .Lmarker_role_base
    adds    r1, #2                 @ foreign master/member => role 3/4
    b       .Lmarker_role_base
.Lmarker_role_unassigned_dim:
    movs    r1, #5
.Lmarker_role_base:
    mov     r0, r1
    bx      lr
.Lmarker_unassigned:
    movs    r1, #0
    b       .Lmarker_check_edit
.Lmarker_master_return:
    movs    r0, #0
    bx      lr
.size wr2_marker_group_role, .-wr2_marker_group_role
.ltorg

.type wr2_half_rgb, %function
.thumb_func
wr2_half_rgb:
    @ floor each RGB888 channel independently: 0xff -> 0x7f.
    ubfx    r1, r0, #16, #8
    lsrs    r1, r1, #1
    lsls    r1, r1, #16
    ubfx    r2, r0, #8, #8
    lsrs    r2, r2, #1
    lsls    r2, r2, #8
    orrs    r1, r2
    ubfx    r2, r0, #0, #8
    lsrs    r2, r2, #1
    orrs    r0, r1, r2
    bx      lr
.size wr2_half_rgb, .-wr2_half_rgb
.ltorg

.type wr2_current_group_row, %function
.thumb_func
wr2_current_group_row:
    @ Input r5=instrument record, r6=slot. Return validated row or zero.
    mov     r0, r6
    movs    r1, #104
    muls    r0, r1
    ldr     r1, =WR2_GROUP_STATE
    adds    r0, r1
    ldr     r1, [r0]
    ldr     r2, =WR2_GROUP_MAGIC
    cmp     r1, r2
    bne     .Lgroup_row_invalid
    ldrb.w  r1, [r5, #0x168]
    cmp     r1, #WR2_MAX_SLICES
    bhi     .Lgroup_row_invalid
    ldrb    r2, [r0, #4]
    cmp     r1, r2
    bne     .Lgroup_row_invalid
    ldrb    r2, [r0, #5]
    cmp     r2, #1
    bne     .Lgroup_row_invalid
    bx      lr
.Lgroup_row_invalid:
    movs    r0, #0
    bx      lr
.size wr2_current_group_row, .-wr2_current_group_row
.ltorg

.type wr2_toggle_group_edit, %function
.thumb_func
wr2_toggle_group_edit:
    push    {r4-r7, lr}
    sub     sp, #4
    ldr     r4, =0x200254b4

    @ An active Trig press always exits; the current slice is ignored.
    ldr     r0, =WR2_EDIT_STATE
    ldr     r1, [r0]
    ldr     r2, =WR2_EDIT_MAGIC
    cmp     r1, r2
    bne     .Ledit_open
    bl      wr2_group_edit_exit
    @ Brightness depends on the locked edit master; redraw markers immediately
    @ after clearing the session so every marker returns to full brightness.
    mov     r0, r4
    ldr     r12, =0x600f7c55
    blx     r12
    b       .Ledit_toggle_done

.Ledit_open:
    ldr     r5, [r4, #0xf4]
    cmp     r5, #0
    beq     .Ledit_toggle_done
    ldrb.w  r0, [r5, #0x3c]
    cmp     r0, #5
    bne     .Ledit_toggle_done
    ldrb.w  r0, [r4, #0xf0]
    cmp     r0, #2
    bne     .Ledit_toggle_done
    ldrb.w  r7, [r5, #0x168]
    cmp     r7, #1
    blo     .Ledit_toggle_done
    cmp     r7, #WR2_MAX_SLICES
    bhi     .Ledit_toggle_done
    ldrb.w  r3, [r5, #0x169]
    cmp     r3, r7
    bhs     .Ledit_toggle_done

    @ Require the screen instrument pointer and stock active slot to agree.
    ldr     r0, =0x202046c5
    ldrb    r6, [r0]
    cmp     r6, #WR2_MAX_INSTRUMENTS
    bhs     .Ledit_toggle_done
    mov     r0, r6
    ldr     r1, =0x174
    muls    r0, r1
    ldr     r1, =0x20200000
    adds    r0, r1
    cmp     r0, r5
    bne     .Ledit_toggle_done

    @ Idempotently create an unassigned selection or leave an existing
    @ master/member untouched, then resolve the owning group.
    bl      wr2_create_group
    bl      wr2_current_group_row
    cmp     r0, #0
    beq     .Ledit_toggle_done
    ldrb.w  r3, [r5, #0x169]
    adds    r1, r0, #8
    ldrb    r2, [r1, r3]
    cmp     r2, r7
    bhs     .Ledit_toggle_done
    adds    r1, r0, #8
    ldrb    r3, [r1, r2]
    cmp     r3, r2                 @ master must own itself
    bne     .Ledit_toggle_done

    @ Save the exact normal Trig palette, then publish the separate session.
    ldrb    r1, [r4, #0xc5]
    adds    r1, #4
    lsls    r1, r1, #2
    ldr     r0, =0x20027d64
    ldr     r0, [r0, r1]
    cmp     r0, #0
    beq     .Ledit_toggle_done
    ldr     r3, [r0, #0x28]
    cmp     r3, #0
    beq     .Ledit_toggle_done
    ldr     r0, =WR2_EDIT_STATE
    str     r3, [r0, #8]
    strb    r6, [r0, #5]
    strb    r2, [r0, #6]
    ldr     r1, =WR2_EDIT_MAGIC
    str     r1, [r0]
    @ Reuse the stock Sample Playback redraw path. This redraws the waveform
    @ markers after group creation without changing slice selection.
    mov     r0, r4
    ldr     r12, =0x600f7c55
    blx     r12

.Ledit_toggle_done:
    add     sp, #4
    pop     {r4-r7, pc}
.size wr2_toggle_group_edit, .-wr2_toggle_group_edit
.ltorg

.type wr2_group_edit_apply, %function
.thumb_func
wr2_group_edit_apply:
    push    {r4, lr}
    ldr     r4, =0x200254b4
    ldrb    r1, [r4, #0xc5]
    ldr     r0, =0x20027d64
    ldr     r2, =wr2_edit_trig_palette
    ldr     r12, =0x60101da5      @ disp_ctrl_set_colors
    blx     r12
    ldrb    r1, [r4, #0xc5]
    ldr     r0, =0x20027d64
    movw    r2, #0x2000           @ label renderer's per-widget fill flag
    ldr     r12, =0x60101d25      @ stock disp_ctrl_set_flags
    blx     r12
    ldrb    r1, [r4, #0xc5]
    ldr     r0, =0x20027d64
    ldr     r12, =0x60101e85      @ disp_ctrl_refresh
    blx     r12
    pop     {r4, pc}
.size wr2_group_edit_apply, .-wr2_group_edit_apply

.type wr2_group_edit_exit, %function
.global wr2_group_edit_exit
.thumb_func
wr2_group_edit_exit:
    push    {r4, lr}
    ldr     r4, =0x200254b4
    ldr     r0, =WR2_EDIT_STATE
    ldr     r1, [r0]
    ldr     r2, =WR2_EDIT_MAGIC
    cmp     r1, r2
    bne     .Ledit_exit_clear
    ldr     r2, [r0, #8]
    ldrb    r1, [r4, #0xc5]
    ldr     r0, =0x20027d64
    ldr     r12, =0x60101da5
    blx     r12
    ldrb    r1, [r4, #0xc5]
    ldr     r0, =0x20027d64
    movw    r2, #0x2000
    ldr     r12, =0x60101d41      @ stock disp_ctrl_clear_flags
    blx     r12
    ldrb    r1, [r4, #0xc5]
    ldr     r0, =0x20027d64
    ldr     r12, =0x60101e85
    blx     r12
.Ledit_exit_clear:
    ldr     r0, =WR2_EDIT_STATE
    movs    r1, #0
    str     r1, [r0]
    pop     {r4, pc}
.size wr2_group_edit_exit, .-wr2_group_edit_exit

.type wr2_group_edit_maintain, %function
.thumb_func
wr2_group_edit_maintain:
    push    {r4-r7, lr}
    sub     sp, #4
    mov     r4, r0
    ldr     r0, =WR2_EDIT_STATE
    ldr     r1, [r0]
    ldr     r2, =WR2_EDIT_MAGIC
    cmp     r1, r2
    bne     .Ledit_maintain_done
    ldr     r5, [r4, #0xf4]
    cmp     r5, #0
    beq     .Ledit_maintain_exit
    ldrb.w  r1, [r5, #0x3c]
    cmp     r1, #5
    bne     .Ledit_maintain_exit
    ldrb    r1, [r4, #0xc2]       @ logical page survives refresh substitution
    cmp     r1, #2
    bne     .Ledit_maintain_exit
    ldr     r0, =0x202046c5
    ldrb    r6, [r0]
    ldr     r0, =WR2_EDIT_STATE
    ldrb    r1, [r0, #5]
    cmp     r1, r6
    bne     .Ledit_maintain_exit
    cmp     r6, #WR2_MAX_INSTRUMENTS
    bhs     .Ledit_maintain_exit
    mov     r0, r6
    ldr     r1, =0x174
    muls    r0, r1
    ldr     r1, =0x20200000
    adds    r0, r1
    cmp     r0, r5
    bne     .Ledit_maintain_exit
    bl      wr2_current_group_row
    cmp     r0, #0
    beq     .Ledit_maintain_exit
    ldr     r1, =WR2_EDIT_STATE
    ldrb    r7, [r1, #6]
    ldrb.w  r2, [r5, #0x168]
    cmp     r7, r2
    bhs     .Ledit_maintain_exit
    adds    r0, #8
    ldrb    r0, [r0, r7]
    cmp     r0, r7
    bne     .Ledit_maintain_exit
    @ Stock refresh rebuilt the Page 3 widget palette. Restore only Trig's
    @ private edit palette after validation, leaving all shared styles alone.
    bl      wr2_group_edit_apply
    b       .Ledit_maintain_done
.Ledit_maintain_exit:
    bl      wr2_group_edit_exit
.Ledit_maintain_done:
    add     sp, #4
    pop     {r4-r7, pc}
.size wr2_group_edit_maintain, .-wr2_group_edit_maintain
.ltorg

.align 2
wr2_edit_trig_palette:
    .word 0x00000000             @ normal label text is black/dark
    .word 0x00cd1818             @ stock selected-control red from the default UI palette
    .word 0x00cd1818             @ same red for the alternate/focused state
    .word 0x00000000             @ secondary text

.section .text.wr2_stage4_add, "ax", %progbits
.align 2
.global wr2_page3_add_callback
.type wr2_page3_add_callback, %function
.thumb_func
wr2_page3_add_callback:
    @ Implemented in C so table validation and deterministic integer
    @ apportionment remain auditable. The stock type-3 dispatcher calls this
    @ entry on both press and release.
    b.w     wr2_page3_add_callback_c
.size wr2_page3_add_callback, .-wr2_page3_add_callback
.ltorg

.section .text.wr2_stage6_remove, "ax", %progbits
.align 2
.global wr2_page3_remove_callback
.type wr2_page3_remove_callback, %function
.thumb_func
wr2_page3_remove_callback:
    @ Like Add, the stock type-3 dispatcher calls this on activation/release.
    b.w     wr2_page3_remove_callback_c
.size wr2_page3_remove_callback, .-wr2_page3_remove_callback
.ltorg
