# Proposed release: v0.2.0-h12

**Prepared for review; not published.** Firmware identity: 1.2.0-h12, Build 1504.

H12 adds the full conditional Chance range to Constant, From-To and Random in
both Fill FX columns, continuing Psygoo's original conditional trigger roadmap
on FatRodzianko's Modular-Trackular implementation. Ratios 1:2 through 8:8,
FIRST 1S/!1S, PRE PR/!PR and NEI NE/!NE share the encoder order:

`0–100 → 1:2 … 8:8 → PR → !PR → NE → !NE → 1S → !1S`

PRE keeps each track's last eligible Chance producer result. NE follows the
immediately preceding track's last evaluated conditional-note result across
rows, with chains and logical Micro-move handling. Fill endpoints use that same
logical order, preserving stored IDs and stock percentage generation behavior.

Distribution contains MIT project source and a local browser patcher only.
Bring your own official Tracker+ 1.2.0 PTF; no firmware is uploaded, supplied or
installed by the website. Required output SHA-256:
`40079d13a100c8c73953496c9bdebec034c6b86a1e99c2706e717fc8e0b18c79`.

**Upgrade caution:** original CT6 condition IDs differ. Read
[compatibility](COMPATIBILITY.md) and convert backed-up project copies manually.
H10/H11 saved condition IDs are preserved. Retain official firmware and backups.

Hardware: the user installed H12 and confirmed it works as intended.
Offline/browser evidence and limits: [VALIDATION.md](VALIDATION.md).
Not every playback mode or hardware scenario has been individually validated.
Independent project; not affiliated with Polyend. Read [installation](INSTALL.md),
[recovery](RECOVERY.md) and [disclaimer](DISCLAIMER.md).

Credits: Psygoo — original concept, roadmap and CT6 initial six conditions;
FatRodzianko / Modular-Trackular — subsequent reimplementation, extensions and
technical infrastructure. [License/provenance](NOTICE.md).
