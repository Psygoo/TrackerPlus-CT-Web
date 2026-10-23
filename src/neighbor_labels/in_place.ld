ENTRY(h09_consumer_result)
h09_format_fallback = 0x601bc244;
SECTIONS {
    .consumer 0x601bc16e : { *(.consumer) }
    .formatter 0x601bc208 : { *(.formatter) }
    ASSERT(SIZEOF(.consumer)==34, "Consumer suffix must fit its exact H08 slot")
    ASSERT(SIZEOF(.formatter)==30, "Formatter must meet the unchanged copy loop")
    /DISCARD/ : { *(.ARM.attributes) *(.comment) }
}
