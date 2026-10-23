MEMORY { EXT (rx) : ORIGIN = 0x601BC140, LENGTH = 0xEC0 }
ct_reset_resume = 0x601BADD5;
stock_value_resume = 0x600AFB4D;
SECTIONS {
    .text : { *(.text.h07_*) *(.text*) *(.rodata*) } > EXT
    .data : { *(.data*) } > EXT
    .bss (NOLOAD) : { *(.bss*) *(COMMON) } > EXT
    ASSERT(SIZEOF(.data)==0, "H07 requires no initialized RAM")
    ASSERT(SIZEOF(.bss)==0, "H07 requires no new RAM")
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
