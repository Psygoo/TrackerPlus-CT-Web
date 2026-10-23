# Verification — H12 release preparation

**2026-10-23: public browser distribution path PASS; publication pending explicit approval.**
No firmware modification, device flashing, push, deployment or live Pages
change was performed. The existing CT6 branch still points to
`dfb5cd4bdfe3f8bd1c68c4c998c2d44365a596ad`.

## Exact reference and source rebuild

The existing official-stock fixture has SHA-256
`d4616304f33d6d0b56516c658f34fa22517c1dd1630021c1ca89afca7b618ccd`.
Browser output is **4,606,124 bytes**, compared byte-for-byte to the exact
hardware-tested H12 reference and independently hashed:

`40079d13a100c8c73953496c9bdebec034c6b86a1e99c2706e717fc8e0b18c79`

The complete output and decoded image match, including Intel HEX checksums,
entry address, record boundaries and serialization. There are no unexplained
differences. Arm GNU 15.2.Rel1 independently rebuilds the public project
source template without reading firmware; `--check` passes. The template is
6,816 bytes; 29 fingerprinted prefix edits and 14 input-only replay bytes
reproduce the preserved H12 placement. No firmware or original replay bytes
are embedded in the public assets. See [BROWSER.md](BROWSER.md).

## Browser and rejection checks performed

| Check | Observed result |
|---|---|
| Chromium 156.0.8078.4, Linux headless | PASS: real chooser and drag/drop, module worker, Web Crypto, verified Blob download, twice byte-exact H12. |
| Firefox 157.0, Linux headless | PASS: same complete path, twice byte-exact H12. |
| Privacy | Zero requests while processing; tests abort all requests after worker ready. Only static GETs reach the local server. A deliberate POST/fetch probe is blocked by `connect-src 'none'` before transmission. Runtime source contains no network client API. |
| Wrong input | Invalid type/extension, multiple files, empty/oversized, synthetic, damaged, truncated, line-ending-altered, H11 and already-patched H12 inputs reject; no download. |
| UI lifecycle | Invalid input after success clears prior output; concurrent jobs blocked; unverified worker output/faults fail closed. |
| Stability | Repeated Node and browser outputs match the reference. Original input stays unchanged; caller mutation after invocation does not affect the job snapshot. |
| Layout and links | Home tested at 320/375/1280 px; documentation at 320/1280 px without horizontal overflow. Local links/anchors and both pages' CSP pass. |
| WebKit / Safari | NOT VERIFIED: WebKit test engine downloaded but cannot launch because host GTK/GStreamer and other shared libraries are absent. No Safari/iOS support claim. |

The fixture-free unit checks and the private-file integration check all pass
(six test groups). Private fixture files, screenshots, browser caches and
raw test evidence remain outside the public tree.

## Focused ARM rechecks of the actual browser download

These checks execute the downloaded Firefox PTF with existing bounded ARM
fixtures, using synthetic peripherals/MIDI and an in-memory filesystem:

- 2,480 Constant/From-To/Random generation cases, both columns, every constant,
  percentage-only/condition-only/mixed ranges, reversed/equal bounds and row
  eligibility. Full Random range observes all 142 stored values.
- 224 stock/H11/H12 percentage generator comparisons preserve bytes, RNG state
  and draw order. 172 other-FX generation and 492 other-FX editor checks match H11.
- Twelve actual Fill confirmation/undo cases pass. All 142 generated IDs in
  both columns save/reload through the stock writer/checksum/reader and produce
  identical H11/H12 playback, including generated From-To/Random rows.
- 610 ABI/stack/register/memory-guard cases pass. No new sequencer stack;
  maximum adapter frames remain 0/16/8/24 bytes for max/From-To/Random/text.
- H11 encoder mapping: 1,846 pure edits, 568 detents, 252 larger deltas,
  564 continuous steps, 42 unassigned-ID checks and real dispatcher boundaries.
- Different-row and same-row 1S → NE → !NE examples play tracks 1+2 first,
  track 3 on later passes; a 16-track chain passes. 384 Micro-move/retention
  events, PRE's 76-note/48-commit matrix, shared-column snapshots and eleven
  reset/preview scenarios pass.

Earlier H12 offline evidence additionally covers 2,840 Fill editor navigation
cases, 410 explicit field labels, raster/font bounds, all 37 older conditions,
branch/relocation targets, ITCM stubs, RAM allocation and PTF integrity. Those
are preserved earlier results, not a claim of additional hardware testing.
Exact browser/reference identity preserves that tested firmware and its hooks.
No unrelated extensive emulation suite was run for release preparation.

## Hardware evidence and limits

The user installed the exact H12 PTF on physical Polyend Tracker+ hardware and
confirmed it works as intended. Earlier H10/H11 reports confirmed NE chains,
Micro-move and encoder/label behavior. There is no exhaustive H12 physical test
matrix. Offline checks do not certify every Song/performance/preview path,
external-clock combination, audio routing, installed cache behavior or worst-case
interrupt timing. No additional device installation was attempted here.

The official downloads page and manual are linked in [INSTALL.md](INSTALL.md).
Automated direct access to the download page returned HTTP 403; a fresh account-
authenticated firmware download was not performed. The exact locally held stock
fixture was used and its full hash verified. Users must obtain their own input;
a differing official file is rejected rather than silently accepted.

## Export and recovery audit

The source/static allowlist is [PUBLIC_FILES.json](PUBLIC_FILES.json), excluding
itself from hash recursion. The export contains no PTFs, stock dumps, proprietary
instruction windows, test firmware fixtures, node_modules, local paths or detected
credential forms. The project patch template is rebuilt from the included MIT
source with the three stock replay slots zeroed. Applicable MIT license text
and historical CT6 NOTICE are preserved verbatim, with an explicit origins addendum.
This is a bounded source/package review, not a general legal/security certification.

All 26 pre-existing PTFs and 171 protected source/candidate artifacts remain
hash-identical. From the earlier 306-file preservation manifest, only its stage
README changed to record hardware validation; every other file is unchanged.
The original private CT6 development tracked-file snapshot is also unchanged.
Public CT6 remains recoverable by its pinned commit and a private exact source
snapshot. [CT6 project-ID migration](COMPATIBILITY.md) is a documented limitation.

Remaining release conditions: explicit owner publication approval; acceptance
of the stated browser/hardware coverage limits. No output-reproduction blocker
remains for the tested Chromium and Firefox paths.
