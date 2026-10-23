ENTRY(h12_fill_max)
MEMORY { EXT (rx) : ORIGIN = 0x601BC4F0, LENGTH = 0xB10 }
SECTIONS {
    .text : { *(.text.h12_fill_max) *(.text.h12_generator_max) *(.text.h12_fromto_store) *(.text.h12_random_gateway) *(.text*) *(.rodata*) } > EXT
    .data : { *(.data*) } > EXT
    .bss (NOLOAD) : { *(.bss*) *(COMMON) } > EXT
    ASSERT(SIZEOF(.data)==0, "No initialized RAM")
    ASSERT(SIZEOF(.bss)==0, "No new state")
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
