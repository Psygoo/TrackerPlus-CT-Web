MEMORY
{
    PATCH (rx) : ORIGIN = 0x601BAF60, LENGTH = 8K
    RAM (rw) : ORIGIN = 0x20205030, LENGTH = 64K
}
SECTIONS
{
    .text : { KEEP(*(.text.h03_*)) *(.text*) *(.rodata*) } > PATCH
    .data : { *(.data*) } > PATCH
    .bss : { *(.bss*) *(COMMON) } > PATCH
    ASSERT(SIZEOF(.data) == 0, "H03 must not use initialized data")
    ASSERT(SIZEOF(.bss) == 0, "H03 must not use BSS")
    .mt_ram (NOLOAD) : ALIGN(16) {
        __mt_ram_start = .;
        *(.mt_ram*)
        . = ALIGN(16);
        __mt_ram_end = .;
    } > RAM
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
