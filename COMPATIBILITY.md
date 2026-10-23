# Project compatibility

H12 preserves H10/H11's stored condition IDs and trigger semantics. H11 changed
labels and encoder order, not saved IDs. H12 extends Fill without changing IDs.
Stock percentage-only patterns retain their meanings. No automatic migration
of other modified firmware formats is provided.

## CT6 projects need manual conversion

The original public CT6 used different stored values. Opening those projects
unchanged in H12 can play different conditions. **Back up first, work on copies,
and re-enter the intended labels under H12 before playback.** Keep a record of
the original values while still using CT6. Do not mass-convert unrelated FX.

| Original CT6 stored value / label | H12 stored value / label |
|---|---|
| 102 / 1:2 | 103 / 1:2 |
| 103 / 2:2 | 104 / 2:2 |
| 104 / 1:4 | 108 / 1:4 |
| 105 / 2:4 | 109 / 2:4 |
| 106 / 3:4 | 110 / 3:4 |
| 107 / 4:4 | 111 / 4:4 |

CT6's reserved value 101 becomes 1S in H12; review any unsupported stored values
manually. The public patcher takes official stock firmware only; it does not
patch CT6 firmware, convert projects or read your SD card.

Returning to stock or CT6 does not preserve H12 condition meanings. Keep
separate project copies for each firmware. Preserve CT6 source at repository
commit `dfb5cd4bdfe3f8bd1c68c4c998c2d44365a596ad`; its original website must remain
recoverable. This preparation has not changed the live CT6 site.
