# CT2 origins, credits and provenance

**Original concept, full roadmap and CT6: Psygoo.** FatRodzianko / Modular-Trackular
subsequently reimplemented, expanded and developed the technical infrastructure.
CT2 continues Psygoo's original roadmap using and extending that implementation.
The concept did not originate in Modular-Trackular.

CT2 contains the hardware-confirmed H13 NE/!NE → PRE correction and a version-only
rename to 1.2.0-CT2 / Build 1504. `src/ne_pre` contains the MIT project assembly
and linker layout for the 34-byte correction. The source-only builder compiles
it and reconstructs the final CT2 template without opening firmware. Three stock
replay slots totaling 14 bytes remain zero; only the user's verified original
input supplies those bytes locally. No Polyend firmware rights are granted.

The original MIT LICENSE and all earlier NOTICE provenance below are retained
verbatim. The following H12 record is historical; current semantics are in
CONDITIONS.md and release identity is CT2. Playwright remains a development-only
Apache-2.0 dependency; runtime assets have no external libraries.

## Preserved H12 NOTICE.md (verbatim historical record)

# Origins, credits and H12 provenance

Psygoo originated the Tracker+ conditional trigger concept, including the
long-term roadmap for ratios up to 8:8, FIRST, PRE and NEI. TrackerPlus-CT-Web
(CT6), by Psygoo, implemented the initial six conditions. FatRodzianko's
Modular-Trackular subsequently reimplemented and extended conditional triggers
and developed the technical infrastructure used here. H12 continues Psygoo's
original roadmap while building on that Modular-Trackular implementation.
This chronology does not transfer concept authorship to Modular-Trackular.

The MIT LICENSE is preserved verbatim, including copyright (c) 2026
Modular-Trackular contributors. The original public CT6 notice below is kept
verbatim as historical provenance; its old file references describe that source
snapshot, not the H12 public file inventory.

## H12 source additions

Upstream implementation reference:
[FatRodzianko/Modular-Trackular](https://github.com/FatRodzianko/Modular-Trackular),
pinned commit `a30b280653a93ff4eff9618685a3a476c0d9dfc6`.
`src/ct1` preserves the upstream conditional implementation. `src/history`,
`src/pre`, `src/neighbor_labels`, `src/neighbor`, `src/order` and `src/fill`
contain the incremental H03/H07/H09/H10/H11/H12 project C/assembly and fixed
linker layouts. `tools/build-program.py` reproduces the H12 project template
without opening any stock firmware or candidate binary.

The browser's PTF reader/encoder, worker/session UI and stylesheet build on the
MIT public CT6 patcher, pinned commit
`dfb5cd4bdfe3f8bd1c68c4c998c2d44365a596ad`. Runtime imports are self-contained.
The generated `h12-program.js` contains only rebuilt project patch code, padding,
metadata and **zero placeholders for 14 stock replay bytes**. Those bytes are
copied exclusively from the verified user-supplied original input in memory.
`src/stock-fingerprints.json` contains hashes, not stock instruction windows.
No original/modified firmware, stock dump or extracted proprietary function is
included. The MIT license does not grant rights to Polyend firmware or its
redistribution. Playwright/playwright-core are Apache-2.0 development dependencies
only and are not delivered to website visitors.

## Original CT6 NOTICE.md (verbatim historical record)

# Provenance and redistribution boundary

This project reuses Modular-Trackular, MIT License, copyright (c) 2026 Modular-Trackular contributors. The original LICENSE is preserved verbatim. Upstream: https://github.com/FatRodzianko/Modular-Trackular . Local source snapshot HEAD: `021d19249da8b1f183037c890ec6986c4797d717`, plus the original CT6 implementation and patcher extensions. This standalone repository deliberately does not copy the source repository's Git history, unrelated patches, projects or binary artifacts.

Reused components:

- `tools/ptf.py`: Intel HEX/PTF parsing and deterministic serialization.
- `tools/patch_ptf.py`: placement, RAM allocation, relocations and existing patch formats, plus the locally validated inline-hook extension.
- `tools/emu/machine.py`, `fakefs.py`, `seq_timing.py`: the existing Unicorn Machine, overlay filesystem and sequencer fixtures. seq_timing came from the companion Modular-Trackular upstream checkout; this is not a second emulator. Companion checkout commit: `ac584b93304672eaa78103b1a15db934c89dfcde`.
- `patch/community_beta/payload.py`: minimal extraction of the existing Conditional Trigs ARM emitters (original six-condition wrappers, UI, atomic transport and swap), preserving their emitted bytes.
- `tests/community_beta_arm.py`: preserved historical expanded-condition harness, excluded from CT6 validation; portable extraction of existing stock-gate, UI and Pattern/Song tests; fixture imports are rebound to this repository. No functional ARM changes are introduced.
- `tools/test_inline_hooks.py`: host-side patcher tests, using synthetic guards rather than embedded stock windows.

The firmware input, generated PTF, original instruction windows, dumps, SD contents and raw local test artifacts are excluded. Hook metadata contains addresses and SHA-256 fingerprints, not the original firmware windows; the patcher obtains those windows from the user's input. Payload source generates our patch code and replays the small displaced instruction sequences required by the hooks; it does not embed stock functions or firmware dumps. The MIT license is not a license to Polyend's firmware. No claim is made here about redistribution rights for generated firmware.

Browser Phase 1 additions port the existing MIT PTF/inline-hook logic and generate a source representation of the original MIT CT6 patch program without reading firmware. The fixed program includes only project patch code and the same small displaced-instruction replays already in the original emitter. All hook originals are still obtained from the user's input. No Polyend firmware license is granted. Playwright and playwright-core are Apache-2.0 development dependencies only; browser runtime assets have no third-party dependency. See [BROWSER.md](BROWSER.md) for the export and verification procedure.
