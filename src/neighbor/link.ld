ENTRY(h10_chance)
MEMORY { EXT (rx) : ORIGIN = 0x601BC27C, LENGTH = 0xD84 }
SECTIONS {
    .text : { *(.text.h10_chance) *(.text.h10_process) *(.text.h10_advance) *(.text*) *(.rodata*) } > EXT
    .data : { *(.data*) } > EXT
    .bss (NOLOAD) : { *(.bss*) *(COMMON) } > EXT
    ASSERT(SIZEOF(.data)==0, "No initialized RAM")
    ASSERT(SIZEOF(.bss)==0, "Reuse only existing reserved padding")
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
