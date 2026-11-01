ENTRY(wr2_more_handler)
MEMORY { EXT (rx) : ORIGIN = 0x601bc6f0, LENGTH = 0x2000 }
PHDRS { flash PT_LOAD FLAGS(5); }
SECTIONS {
    .text : { *(.text.wr2_more_handler) *(.text*) *(.rodata*) *(.aftertext*) } > EXT :flash
    .data : { *(.data*) } > EXT :flash
    .bss (NOLOAD) : { *(.bss*) *(COMMON) } > EXT :NONE
    ASSERT(SIZEOF(.data)==0, "No initialized RAM in the UI prototype")
    ASSERT(SIZEOF(.bss)==0, "No BSS allocation in the UI prototype")
    ASSERT(SIZEOF(.text) <= LENGTH(EXT), "WR2 UI extension exceeds validated XIP gap")
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
