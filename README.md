# Tracker+ Conditional Trigs — CT2

An independent, unofficial modification for **Polyend Tracker+ (2024 plus model)**,
using original firmware **1.2.0**. Not affiliated with, endorsed by or supported
by Polyend. Not for Tracker Mini or the original Tracker.

Public firmware identifier: **1.2.0-CT2 / Build 1504**. Release: **v0.2.1-CT2**.
Use the [public web patcher](https://psygoo.github.io/TrackerPlus-CT-Web/) to
generate your firmware locally from the supported original input.

CT2 uses the exact **hardware-validated H13** implementation, including the
NE/!NE → PR/!PR correction. Only its displayed version label changes. The user
confirmed H13 works on physical Tracker+ hardware, specifically including that
fix. The project owner also installed and confirmed the final CT2 firmware
works correctly on physical Tracker+ hardware. These reports do not establish
that every hardware mode or configuration has been tested.

## Corrected PRE after NE

**NE and !NE now update their own track's PRE history** with the evaluated
logical conditional-note result, including both acceptance and rejection.
A later PR follows that result; !PR inverts it. Micro-move changes dispatch
timing without changing the recorded logical result. Neighbour chains continue
to work, using the immediately preceding track's last conditional-note result.
PR/!PR remain PRE non-producers. An event containing PR/!PR retains previous
PRE even when NE is in the other FX column; both columns read before commit.
[Full semantics and examples](CONDITIONS.md).

CT2 is the corrected successor to the **superseded H12 public release**,
[v0.2.0-h12](https://github.com/Psygoo/TrackerPlus-CT-Web/releases/tag/v0.2.0-h12).
H12 did not update PRE after NE/!NE. Its release, source and tag remain
available for reference/recovery; the default web patcher now targets CT2.
CT6 is preserved at [archive/ct6-before-h12](https://github.com/Psygoo/TrackerPlus-CT-Web/tree/archive/ct6-before-h12).

## Make firmware locally

Supply your own unmodified PTF from the [official Tracker+ downloads](https://polyend.com/downloads/tracker-plus-downloads/).
The patcher runs in browser memory and never uploads firmware. It supplies no
Polyend firmware and installs nothing. Only an exact verified output enables
Download, named **TrackerPlus_CT2.ptf**.

| File | SHA-256 |
|---|---|
| Supported original Tracker+ 1.2.0 PTF | `d4616304f33d6d0b56516c658f34fa22517c1dd1630021c1ca89afca7b618ccd` |
| Required CT2 output/reference | `baa93d46208c0f6ab2ca0c7507fb3c430703a3ceada582a780779844a0d03b20` |

Read [installation](INSTALL.md), [recovery](RECOVERY.md),
[compatibility](COMPATIBILITY.md), [verification](VALIDATION.md) and the
[disclaimer](DISCLAIMER.md) before installing. **CT6 projects require manual
condition-ID conversion.** H10–H13 saved IDs remain compatible with CT2;
NE → PRE playback intentionally changes from H12's incorrect behavior.

## Chance and Fill

Percentages **0–100**; ratios **1:2 through 8:8**; FIRST **1S / !1S**;
PRE **PR / !PR**; NEI **NE / !NE**. Exact encoder order in both directions:

`0–100 → 1:2, 2:2, 1:3, 2:3, 3:3 … 1:8 … 8:8 → PR → !PR → NE → !NE → 1S → !1S`

Ratios are ordered by denominator 2–8, then numerator 1–denominator. Existing
stored IDs are unchanged. Fill FX1 and FX2 retain **Constant, From-To and Random**
with all percentages and conditions, using the same logical order and endpoint
controls. Try `1:4` for a four-pass phrase or Random `PR → !NE` to choose among
PR, !PR, NE and !NE. [Condition guide](CONDITIONS.md).

## Origins and credits

**Psygoo originated the concept, full roadmap and CT6 implementation**, including
the longer-term plan for ratios up to 8:8, FIRST, PRE and NEI. CT6 implemented the
initial six conditions. **FatRodzianko / Modular-Trackular** subsequently
reimplemented and expanded conditional triggers and developed the technical
infrastructure. **CT2 continues Psygoo's original roadmap, using and extending
the Modular-Trackular implementation.** The original concept did not originate
in Modular-Trackular.

Original MIT copyright and license text, CT6 NOTICE and H12 provenance are
preserved in [LICENSE](LICENSE) and [NOTICE.md](NOTICE.md). MIT covers project
code, not Polyend firmware or rights to distribute it. No original/modified
firmware binaries, dumps or extracted proprietary functions are included.

## Development and issue reports

Serve locally with `node tools/serve.mjs`; use HTTPS for remote hosting.
Direct file URLs are unsupported because module workers and Web Crypto need
an appropriate origin. Website visitors need no runtime dependencies.

`node --test tests/core.test.mjs` runs fixture-free checks. Full verification
sets private `STOCK_PTF` and `CT2_PTF` paths, then runs that command and
`node tests/browser.mjs`. Browser tests use Playwright 1.64.1 (`npm ci` and
install the desired browsers in your development environment).
`BROWSER_ENGINES=chromium,firefox` selects the verified test engines.

`python3 tools/build-program.py --compiler /path/to/arm-gnu-toolchain/bin --check`
rebuilds the MIT patch template with Arm GNU **15.2.Rel1**, reading no firmware.
[Architecture](BROWSER.md), [release notes](RELEASE_NOTES.md),
[publication plan and allowlist](PUBLICATION.md).

[Report issues](https://github.com/Psygoo/TrackerPlus-CT-Web/issues) with model,
firmware version, playback mode, tracks/rows/FX, expected/observed results and
minimal steps. Do not attach firmware, dumps, private projects or credentials.
