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
