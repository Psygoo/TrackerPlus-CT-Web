# Tracker+ Conditional Trigs CT2 — v0.2.1-CT2

Corrected CT2 release, authorized for publication by the project owner.
Firmware identity: **1.2.0-CT2 / Build 1504**. Output: **TrackerPlus_CT2.ptf**.

**Fix PRE after NE/!NE:** an evaluated NE/!NE now updates its track's PRE with
the logical conditional-note result. If it plays, later PR plays and !PR rejects;
if rejected, later PR rejects and !PR plays. Works across rows and with Micro-move.
Neighbour-readable results and NE chains remain functional. PR/!PR do not produce
PRE, and PR-containing mixed rows retain their previous PRE in either FX order.

CT2 includes the exact hardware-validated H13 implementation. The only firmware
change from H13 is the displayed version label, from 1.2.0-h13 to 1.2.0-CT2.
Build 1504, saved condition IDs, labels, history/reset rules, encoder order and
Fill FX1/FX2 Constant, From-To and Random remain unchanged.

Encoder order: `0–100 → 1:2 … 8:8 → PR → !PR → NE → !NE → 1S → !1S`.

CT2 supersedes [v0.2.0-h12](https://github.com/Psygoo/TrackerPlus-CT-Web/releases/tag/v0.2.0-h12),
which incorrectly excluded NE/!NE from PRE updates. H12's release/tag/history
remain available; CT6 is preserved at
[archive/ct6-before-h12](https://github.com/Psygoo/TrackerPlus-CT-Web/tree/archive/ct6-before-h12).
H12/H13 saved IDs need no migration. CT6 projects need
[manual condition-ID conversion](COMPATIBILITY.md).

Bring your own original official Tracker+ 1.2.0 PTF. The browser patcher processes
it locally and enables download only for this verified CT2 SHA-256:

`baa93d46208c0f6ab2ca0c7507fb3c430703a3ceada582a780779844a0d03b20`

No Polyend firmware binaries are supplied or uploaded. Distribution contains
MIT project source and the static patcher only. Read [installation](INSTALL.md),
[recovery](RECOVERY.md), [disclaimer](DISCLAIMER.md) and [verification](VALIDATION.md).

Hardware: the project owner confirmed the final CT2 firmware works correctly
on physical Tracker+ hardware. H13's PRE fix was also hardware-confirmed.
Offline and Chromium/Firefox checks are reported separately; not every hardware
mode or browser platform is claimed verified.

Origins and credits: **Psygoo** — original concept, roadmap and CT6.
**FatRodzianko / Modular-Trackular** — subsequent reimplementation, expansion
and technical infrastructure. **CT2** continues Psygoo's original roadmap using
and extending that implementation. Original MIT notices and historical
[NOTICE](NOTICE.md) remain preserved; MIT grants no Polyend firmware rights.
