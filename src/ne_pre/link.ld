ENTRY(h13_pre_commit)
MEMORY { EXT (rx) : ORIGIN = 0x601BC618, LENGTH = 0x9E8 }
SECTIONS {
    .text : { *(.text.h13_pre_commit) *(.text*) *(.rodata*) } > EXT
    .data : { *(.data*) } > EXT
    .bss (NOLOAD) : { *(.bss*) *(COMMON) } > EXT
    ASSERT(SIZEOF(.data)==0, "No initialized RAM")
    ASSERT(SIZEOF(.bss)==0, "No new RAM allocation")
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
