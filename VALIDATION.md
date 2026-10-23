# CT2 verification — 2026-10-23

**PASS offline and local-browser distribution checks. Final CT2 hardware
validation has been confirmed by the project owner; publication is authorized.**

## Reference identity and exact differences

Hardware-tested parent H13 SHA-256:
`076628c6cbec2a110ae1be1c5014a9f586dad9042b8a5725b671cf9ab361fd76`.

CT2 reference and required browser output SHA-256:
`baa93d46208c0f6ab2ca0c7507fb3c430703a3ceada582a780779844a0d03b20`.

Only three decoded bytes change: `0x601BC135: 68→43`, `0x601BC136: 31→54`,
`0x601BC137: 33→32`, changing the same-size format suffix `h13` to `CT2`.
One PTF data record/checksum changes. No other byte differs, no records move,
and no code/RAM/stack/hook/ITCM/Fill/order/history layout changes. PTF remains
4,606,231 bytes; decoded image 1,623,610 bytes; entry 0x60030319.
Actual stock Config selector/formatter reports **1.2.0-CT2 / 1504** in both
selector states.

The compiler-only source export rebuilds with Arm GNU 15.2.Rel1, reading no
firmware. Its 6,850-byte project template SHA-256 is
`bd12857664cf6b7617aedf0a2911257a9557b7e42cae1042de22ca4a9ab88ee0`.
There are 29 fingerprinted original-prefix edits and 14 zero replay bytes,
supplied exclusively from the original user input. Template regeneration
`--check` passes. No stock firmware/function dump is embedded in the source.

## Complete browser path

| Check | Result |
|---|---|
| Chromium 156.0.8078.4, Linux headless | PASS: stock file picker and drag/drop, actual module worker, hash gate and Blob download. Two outputs match the CT2 reference byte-for-byte and by SHA-256. |
| Firefox 157.0, Linux headless | PASS: the same full flow, two exact downloads. |
| Correct identity | Title/release v0.2.1-CT2, firmware 1.2.0-CT2 / Build 1504, visible/download filename TrackerPlus_CT2.ptf, PRE fix notice and contributor credits checked. |
| No transmission | Zero requests during processing; all post-load requests blocked by the test. Deliberate synthetic POST is rejected by CSP before reaching the server. Runtime code has no network API, external assets or service worker. |
| Input rejection | Invalid extension/type, empty, multiple, oversized, synthetic, modified, truncated, line-ending-altered, H12/H13 and already-patched CT2 files reject. Only the exact supported original stock SHA is accepted. |
| Download controls | No download until worker and UI hashes pass. Bad input after success clears stale output; concurrent jobs blocked; worker faults and unverified output fail closed. |
| Stability | Repeated Node/browser outputs match; input is unmodified; caller mutation after invocation cannot alter the snapshotted job. |
| Layout/docs | Home at 320/375/1280 px, docs at 320/1280 px without horizontal overflow; local links/anchors, compatibility and PRE semantics checked. |

The six core/integration test groups pass. Original stock fixture SHA-256:
`d4616304f33d6d0b56516c658f34fa22517c1dd1630021c1ca89afca7b618ccd`.
The preparation browser tests used localhost. Firmware test inputs, outputs,
caches and screenshots stay outside the public tree. Publication additionally
checks the actual live Pages site and output before claiming deployment success.

## Focused firmware retests

The **actual Chromium-generated CT2 download** executes in the established
bounded ARM fixtures. Hardware H13 is the independent comparison baseline.

- Eight parallel source/NE/FX-column pairs over three loops cover NE/!NE passing
  and failing, replacement of old true/false PRE, PR/!PR inversion, ordinary and
  empty retention, and Micro-move 0/50/97/100. H13 and CT2 match exactly in MIDI
  events/ticks, PRE and NE state, commit sequence and RNG state.
- Sixteen mixed Chance pairs retain PR-containing-event exclusion and shared
  snapshots, including both column orders. Same/different-row and 16-track NE
  chains, 384 Micro-move/retention events and invalid-note/mute cases pass.
- Percentage/ratio/FIRST PRE, actual C0 draw-zero acceptance, 1,184 ct1 predicate
  cases and 116 stock-percentage MIDI notes pass. Eleven reset/preview/count-in
  cases, pattern changes, queued track copies and deferred reset snapshots pass.
- H13 PRE adapter: 2,560 register/stack/bounds cases pass. No new frame/state.
- Fill: 2,480 Constant/From-To/Random generator cases cover every constant,
  condition/percentage/mixed ranges, reversed/equal endpoints and both columns.
  Full Random observes all 142 values. Stock percentage generation matches in
  224 cases, including RNG state/draw order. Other FX: 172 generation and
  492 editor cases; twelve confirmation/undo cases pass.
- Fill UI/ABI: 2,840 encoder cases, endpoint/pad boundaries, 410 labels,
  533 bounded-text cases and all 610 existing adapter/stack cases pass.
- All 142 generated IDs in both columns round-trip stock save/checksum/load.
  A separately saved NE → PR/!PR chain loads unchanged in older/new firmware
  and plays correctly for three loops under CT2.
- All 142 rank/value mappings and twenty actual both-column encoder boundary
  cases pass. Exact code identity preserves the remaining H13 navigation tests.
- Independent strict contiguous Intel HEX decoding, all checksums, record layout,
  complete binary diff and byte-exact re-encoding pass. Only version text differs.

## Hardware evidence, remaining limits and publication gate

The project owner reports successful **final CT2 hardware validation** on a
physical Polyend Tracker+. H13 was previously hardware-confirmed, including
NE/!NE → PRE. CT2 changes only H13's version label. No automatic device flashing
is performed by the patcher or release tooling.
No exhaustive Song/performance/external-clock/routing/interrupt-timing hardware
matrix is claimed. Offline fixtures use synthetic peripherals/MIDI and an
in-memory SD overlay; they do not prove installed flash/cache behavior.

Only the stated Linux Chromium/Firefox engines were tested. Safari/WebKit/iOS
and other browser/platform combinations are not claimed verified. Users must
obtain their own official supported firmware; a fresh authenticated official
firmware download was not performed here.

Read-only remote checks found existing H12 at commit
`ca853c107ef3ddfdfa3251ea967f89da60d7816d`, with v0.2.0-h12 and the CT6 archive
preserved. CT2 supersedes H12 and becomes the default patcher; H12 remains
available through its unchanged release/tag. CT6 projects still need manual ID
conversion. No output-reproduction or firmware-integrity blocker remains.
Publication is explicitly authorized; live deployment/output verification follows.

Public-source review checks the explicit file allowlist, source-only packaging,
local links, path/credential patterns, MIT license and historical NOTICE. This
is a bounded review, not a general legal/security certification. No original or
modified Polyend PTF, stock dump, private fixture or development cache is included.
