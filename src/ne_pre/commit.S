.syntax unified
.cpu cortex-m7
.thumb
.section .text.h13_pre_commit,"ax",%progbits
.global h13_pre_commit
.type h13_pre_commit,%function
.thumb_func
h13_pre_commit:
    /* Internal H10 tail ABI, after the neighbour mask is committed:
     * r0=stock return, r1=new NE mask, r2=EVENT, lr=track+20,
     * r12=history-state base. NE bit 5 implies EVALUATED bit 3.
     * Do not change r0, lr, r12, saved registers, or the existing frame.
     */
    lsls r3,r2,#26              /* evaluated NE/!NE marker, bit 5 */
    bpl .Lfinish
    lsls r3,r2,#29              /* PR/!PR keeps whole-event PRE exclusion */
    bmi .Lexclude_outer
    sub.w r3,lr,#20             /* original track index, 0..15 */
    lsrs r1,r3
    and r1,r1,#1                /* exact H12 logical conditional-note result */
    add r3,r12
    strb r1,[r3,#4]             /* HISTORY[track], within existing allocation */
.Lexclude_outer:
    orr r2,r2,#4                /* restore H12 low flags; no H06 double commit */
.Lfinish:
    and r2,r2,#7                /* displaced original instruction */
    b.w h13_resume              /* retain lr=track+20 for original EVENT store */
.size h13_pre_commit,.-h13_pre_commit
.thumb_set h13_resume,0x601BC3D3
