# Project compatibility and superseded releases

CT2 preserves H10–H13's stored condition IDs, labels, logical order and Fill
values. It matches hardware-tested H13 except for the version label. No saved-
pattern conversion is required from H12 or H13. Playback involving NE → PR/!PR
intentionally changes from H12: NE/!NE now replace PRE with their logical note
result, fixing H12's exclusion bug. PR-containing mixed events retain PRE.

The superseded [H12 release](https://github.com/Psygoo/TrackerPlus-CT-Web/releases/tag/v0.2.0-h12)
remains available for history/reference. Its tag points to
`ca853c107ef3ddfdfa3251ea967f89da60d7816d`. CT6 remains recoverable at
[archive/ct6-before-h12](https://github.com/Psygoo/TrackerPlus-CT-Web/tree/archive/ct6-before-h12),
commit `dfb5cd4bdfe3f8bd1c68c4c998c2d44365a596ad`. CT2 replaces the default
web patcher, while both prior versions remain recoverable through these references.

## CT6 projects need manual conversion

The original public CT6 used different stored values. Opening those projects
unchanged in CT2 can play different conditions. **Back up first, work on copies,
and re-enter the intended labels under CT2 before playback.** Keep a record of
the original values while still using CT6. Do not mass-convert unrelated FX.

| Original CT6 stored value / label | CT2 stored value / label |
|---|---|
| 102 / 1:2 | 103 / 1:2 |
| 103 / 2:2 | 104 / 2:2 |
| 104 / 1:4 | 108 / 1:4 |
| 105 / 2:4 | 109 / 2:4 |
| 106 / 3:4 | 110 / 3:4 |
| 107 / 4:4 | 111 / 4:4 |

CT6's reserved value 101 becomes 1S in CT2; review any unsupported stored values
manually. The public patcher takes official stock firmware only; it does not
patch CT6 firmware, convert projects or read your SD card.

Returning to stock or CT6 does not preserve CT2 condition meanings. Keep
separate project copies for each firmware. Preserve CT6 source at repository
commit `dfb5cd4bdfe3f8bd1c68c4c998c2d44365a596ad`; its original website must remain
recoverable. The default website targets CT2; older source snapshots stay available.
