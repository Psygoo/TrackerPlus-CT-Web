# Slice Weighted Random (WR2)

WR2 adds weighted selection by grouping slices within a Beat Slice sample. The Web Patcher keeps CT2 mandatory and checked and makes WR2 optional and unchecked. With WR2 selected, it generates the owner-confirmed, hardware-tested S13D CT2+WR2 image directly from your original firmware 1.2.0. Start with the [CT2 + WR2 Quick Start](WR2_QUICK_START.md).

## Runtime notes

Direct-slice FX S2 bypasses weighted selection, and invalid groups fall back to direct playback. Changing a sample's slice count can make its groups ineligible.

## Project-data safety

WR2 metadata is not preserved by stock or CT2-only firmware. Group sidecar writes are not atomic with every native project operation; an interrupted Save can leave the previous group data unavailable. Keep a complete SD-card backup and an official recovery file. Recovery is not guaranteed; follow [the recovery guide](RECOVERY.md).

In the Web Patcher, leave Slice Weighted Random (WR2) unchecked to generate CT2, or check it to generate CT2+WR2. Then choose your verified original firmware 1.2.0 file. The output name and verification hash match the selected combination.

Combined output identity: `TrackerPlus_CT2_WR2.ptf`, SHA-256
`0bdc7dfc32f5c98ebf8a3146811a87dc9fbf8a60a324ad9edc1a64cea7609c1a`.
The exact S12C recovery image remains locally preserved at SHA-256
`06adca54ff28ae704e92c4464a975f3c0e34f16d67a177db53157895be199445`.
