ENTRY(s13b_refresh_tail)
SECTIONS {
  . = 0x601C53D0;
  .text : { *(.text*) *(.rodata*) }
  .data : { *(.data*) }
  .bss (NOLOAD) : { *(.bss*) *(COMMON) }
  /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) *(.note*) }
  ASSERT(SIZEOF(.data)==0,"no initialized RAM")
  ASSERT(SIZEOF(.bss)==0,"no fixed RAM")
  ASSERT(. < 0x60200000,"flash limit")
}
