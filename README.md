# Tracker+ Conditional Trigs — H12

An independent, unofficial modification for **Polyend Tracker+ (2024 plus model)**,
using original firmware **1.2.0**. Not affiliated with, endorsed by or supported
by Polyend. Not for Tracker Mini or the original Tracker.

**Release preparation, 2026-10-23:** suggested source release **v0.2.0-h12**;
firmware **1.2.0-h12, Build 1504**. The user installed H12 on physical Tracker+
hardware and confirmed it works as intended. This is an overall hardware report,
not a claim that every playback mode or hardware configuration was tested.
See [verification and limitations](VALIDATION.md).

This tree is staged for review. Publication requires explicit approval. The
existing CT6 website remains unchanged until that approval and a reviewed update.

## Make the firmware locally

Supply your own unmodified PTF from the [official Tracker+ downloads](https://polyend.com/downloads/tracker-plus-downloads/).
The patcher runs in your browser; firmware is never uploaded. It supplies no
Polyend firmware and installs nothing on your device. Only an exactly verified
result enables the download, named `TrackerPlus_H12.ptf`.

| File | SHA-256 |
|---|---|
| Supported original Tracker+ 1.2.0 PTF | `d4616304f33d6d0b56516c658f34fa22517c1dd1630021c1ca89afca7b618ccd` |
| Hardware-tested H12 / required output | `40079d13a100c8c73953496c9bdebec034c6b86a1e99c2706e717fc8e0b18c79` |

Read [installation](INSTALL.md), [recovery](RECOVERY.md),
[project compatibility](COMPATIBILITY.md) and the [disclaimer](DISCLAIMER.md)
first. **CT6 projects need manual conversion: their condition IDs differ.**
Use backed-up project copies. H10/H11 condition IDs remain compatible with H12.

## Conditions

Chance supports percentages **0–100**, every ratio from **1:2 through 8:8**,
FIRST **1S / !1S**, PRE **PR / !PR**, and NEI **NE / !NE**.
Both encoder directions use this exact logical order:

`0–100 → 1:2, 2:2, 1:3, 2:3, 3:3 … 1:8 … 8:8 → PR → !PR → NE → !NE → 1S → !1S`

Ratios run by denominator 2 through 8, then numerator 1 through that denominator.
Stored IDs are preserved; this is a selection order, not an ID renumbering.
Fill FX1 and FX2 retain **Constant, From-To and Random**, with all conditions
available in each mode using this same order and the existing endpoint controls.

For example, `1:4` makes a hit on the first of every four track passes. A `PR`
fill follows a preceding accepted Chance event on its own track. An `NE` hit
can follow another track's conditional result even several rows later.
[Condition guide and musical examples](CONDITIONS.md).

## Origins and credits

**Psygoo originated the Tracker+ conditional trigger concept and roadmap**,
including ratios up to 8:8, FIRST, PRE and NEI. Psygoo's **TrackerPlus-CT-Web
(CT6)** implemented the initial six conditions. **FatRodzianko's
[Modular-Trackular](https://github.com/FatRodzianko/Modular-Trackular)** subsequently
reimplemented and extended conditional triggers and developed the technical
infrastructure used here. This project continues Psygoo's original roadmap
while building on that implementation.

The MIT copyright/license and historical provenance are preserved in
[LICENSE](LICENSE) and [NOTICE.md](NOTICE.md). MIT covers project code, **not
Polyend firmware or rights to redistribute it**. No original or modified PTFs,
stock dumps or extracted proprietary code are included in this source release.

## Local development and review

Runtime assets need no dependencies or build service. Run `node tools/serve.mjs`
and open its localhost URL; HTTPS is required when hosting remotely. A direct
`file://` opening is unsupported because of module workers and Web Crypto.

`node --test tests/core.test.mjs` runs fixture-free unit checks. For complete
release checks, set `STOCK_PTF` and `H12_PTF` to your private local files, then
run the same command and `node tests/browser.mjs`. Browser checks require
Playwright 1.64.1 (`npm ci`, then install the browsers in your development
environment); no dependency is loaded by website visitors. `BROWSER_ENGINES`
selects a comma-separated subset of `chromium,firefox,webkit`.

`python3 tools/build-program.py --compiler /path/to/arm-gnu-toolchain/bin --check`
recompiles the project code with **Arm GNU 15.2.Rel1** without reading firmware.
See [source provenance](NOTICE.md), [release notes](RELEASE_NOTES.md),
[publication manifest](PUBLICATION.md) and [verification](VALIDATION.md).

Report issues at [TrackerPlus-CT-Web issues](https://github.com/Psygoo/TrackerPlus-CT-Web/issues)
with firmware/version, model, playback mode, row/track/FX details, expected and
observed behavior, and minimal reproduction steps. Do not attach firmware,
stock dumps, private projects or credentials.
