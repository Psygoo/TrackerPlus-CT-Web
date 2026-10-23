ENTRY(h11_editor_gateway)
MEMORY { EXT (rx) : ORIGIN = 0x601BC474, LENGTH = 0xB8C }
SECTIONS {
    .text : { *(.text.h11_editor_gateway) *(.text*) *(.rodata*) } > EXT
    .data : { *(.data*) } > EXT
    .bss (NOLOAD) : { *(.bss*) *(COMMON) } > EXT
    ASSERT(SIZEOF(.data)==0, "No initialized RAM")
    ASSERT(SIZEOF(.bss)==0, "No added state")
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
