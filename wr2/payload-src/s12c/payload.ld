ENTRY(wr2_fileop_step)
SECTIONS {
  . = 0x601C1E48;
  .text : { *(.text.hooks) *(.text*) *(.rodata*) }
  .data : { *(.data*) }
  .bss (NOLOAD) : { *(.bss*) *(COMMON) }
  /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) *(.note*) }
  ASSERT(SIZEOF(.data)==0,"no new initialized RAM")
  ASSERT(SIZEOF(.bss)==0,"no new fixed RAM reservation")
  ASSERT(. < 0x60200000,"flash limit")
}
