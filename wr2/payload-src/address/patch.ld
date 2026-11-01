ENTRY(wr2_resolve_selected_instrument)
wr2_mode_reject = 0x601bdb52;
SECTIONS {
    .text 0x601bdb3e : { *(.text) }
    ASSERT(SIZEOF(.text) == 14, "Address correction must fit the original 14-byte span")
    /DISCARD/ : { *(.ARM.attributes) *(.comment) }
}
