ENTRY(wr01_mode5_gateway)
MEMORY { EXT (rx) : ORIGIN = 0x601bc63a, LENGTH = 0x9c6 }
PHDRS { flash PT_LOAD FLAGS(5); }
SECTIONS {
    .text : { *(.text.wr01_mode5_gateway) *(.text*) *(.rodata*) } > EXT :flash
    .data : { *(.data*) } > EXT :flash
    .bss (NOLOAD) : { *(.bss*) *(COMMON) } > EXT :NONE
    ASSERT(SIZEOF(.data)==0, "No initialized RAM in flash extension")
    ASSERT(SIZEOF(.bss)==0, "RNG state is in separately mapped OCRAM")
    ASSERT(SIZEOF(.text) <= LENGTH(EXT), "WR01 extension exceeds free flash window")
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
