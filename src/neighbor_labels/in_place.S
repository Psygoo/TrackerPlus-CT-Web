.syntax unified
.cpu cortex-m7
.thumb

/* Only the old result-selection suffix changes. Entry, range/type checks,
 * ACTIVE/consumer marking, saved registers, fallback and literals stay put.
 * r4=value, r3=current track, r2=state; current-event consumer bit is already set.
 */
.section .consumer,"ax",%progbits
.global h09_consumer_result
.type h09_consumer_result,%function
.thumb_func
h09_consumer_result:
    movs r0,#0                 /* track 1 NE source is explicitly false */
    cmp r4,#140
    blo .Lown
    cbz r3,.Lresult
    subs r3,#1
.Lown:
    adds r3,#4
    ldrb r0,[r3,r2]
.Lresult:
    clz r0,r0
    lsrs r0,r0,#5              /* history == 0 */
    ands r4,r4,#1              /* odd values invert: !PR and !NE */
    eors r0,r4                 /* rejection boolean */
    rsbs r0,r0,#0
    lsrs r0,r0,#1              /* 0 or existing CT_FAIL 0x7fffffff */
    pop {r4,r5}
    bx lr
.size h09_consumer_result,.-h09_consumer_result

/* Shared editor/grid entry, original 12-byte saved-register frame.
 * Preserve compact in its original caller-stack slot without loading/storing it.
 * Fallback skips H07's now-unnecessary compact restore at 0x601bc242, rejoining
 * the original value restore/pop/gateway at 0x601bc244.
 */
.section .formatter,"ax",%progbits
.global h09_formatter_head
.type h09_formatter_head,%function
.thumb_func
h09_formatter_head:
    cmp r3,#4
    push {r4,r5,r6}
    ldrb.w r4,[sp,#12]
    bne.n h09_format_fallback
    sub.w r6,r4,#138
    cmp r6,#3
    bhi.n h09_format_fallback
    ldr r3,[pc,#48]            /* existing word at 0x601bc24c -> !PR */
    lsrs r6,r6,#1              /* r6=family; carry=negated */
    add.w r3,r3,r6,lsl #2      /* !NE immediately follows existing !PR */
    it cc
    addcc r3,#1                /* positive labels skip the leading ! */
    /* Falls into the unchanged bounded copy loop at 0x601bc226. */
.size h09_formatter_head,.-h09_formatter_head
