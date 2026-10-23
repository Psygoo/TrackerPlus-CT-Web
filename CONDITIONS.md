# Chance conditions

Enter Chance (`C`) in FX1 or FX2. The value field uses the labels below, rather
than internal IDs. Both columns and Fill use the same logical encoder order:

`0–100 → 1:2, 2:2, 1:3 … 8:8 → PR → !PR → NE → !NE → 1S → !1S`

Ratios are ordered by denominator 2–8, then numerator 1–denominator.

| Family | Meaning and example |
|---|---|
| Percentage, 0–100 | Stock probability behavior is preserved. Try 50 on a hat for variation. |
| Ratio A:B | Pass on position A of a B-pass track cycle. `1:4` starts a four-pass phrase; `4:4` adds its final fill. |
| FIRST, 1S / !1S | 1S passes the first track pass after reset; !1S passes later passes. Use 1S for an opening crash. |
| PRE, PR / !PR | Consult the last eligible Chance event's result on this same track; !PR inverts it. |
| NEI, NE / !NE | Consult the last evaluated conditional-note result on the immediately preceding track; !NE inverts it. |

## PRE history

Percentage values 0–100 and ratio/FIRST values 101–137 retain their existing
PRE producer behavior: final combined condition acceptance is recorded after
evaluation. **NE/!NE now also produce PRE**, using the final logical conditional-
note result: accepted conditions AND a valid note trigger. A rejected NE, or
one evaluated without a valid note, records false. A later PR follows it and
!PR inverts it. This corrects the H12 bug and is hardware-confirmed in H13.

**PR/!PR never produce PRE.** Any event containing PR or !PR leaves PRE
unchanged, including when another producer or NE is in the other column.
An NE+NE or NE+base event without PR records its final combined logical note
result once. Both columns see the same prior history before that commit;
stock evaluates FX2 then FX1. PR on the same row as NE sees the previous PRE,
not a partial current-row result.

Plain notes, empty rows and skipped evaluations retain history. PR always reads
its own track. Reset clears PRE to false, making PR reject and !PR pass until a
producer. Base producers keep their previous handling of FX-only conditions;
NE specifically uses logical note validity, as described below.

Example: put a 50% Chance hit on row 01, PR on row 03 and !PR on row 05 of one
track. The PR hit follows an accepted percentage event; !PR follows a rejected
one. The consumer rows do not replace the percentage event's PRE history.

Example after NE: track 1 row 01 has note + 1S. Track 2 has note + NE on row 03,
PR on row 05, !PR on row 07. First pass: NE and PR play; !PR rejects. Later
passes: NE and PR reject; !PR plays. Replace NE with !NE and the source outcome
reverses while PR still follows the evaluated !NE result. Micro-move and ordinary
notes between those rows do not change which PRE result the consumers read.

## Neighbour results, across rows

NE reads the **last evaluated conditional-note result on the immediately
preceding track**, whether on an earlier row or the same row. It uses logical
acceptance/scheduling, not the exact audible dispatch time. Micro-move changes
sound timing without changing that logical result. PR/!PR, NE/!NE, percentages,
FIRST and ratios can all produce neighbour results, enabling chains.

An evaluated conditional event records true only if its combined conditions
pass and it has a valid note trigger. Rejection records false. A condition on
an FX-only/invalid-note event cannot record a successfully played note. Empty
rows and ordinary notes do not replace the last conditional result. A muted or
disabled track that stock processing skips does not evaluate a condition and
retains its prior result; NE does not measure mixer output, volume or audibility.
These edge cases were checked offline, not individually hardware-certified.
Track 1 has no neighbour: NE rejects and !NE accepts. There is no wraparound.

Example on different rows: track 1 row 01 has note + 1S; track 2 row 03 has note
+ NE; track 3 row 05 has note + !NE. On the first pass tracks 1 and 2 play;
on later passes track 3 plays. The same logical result holds on the same row
and with Micro-move. Tracks are evaluated in their established order; a later
track can observe the preceding track's newly evaluated conditional result.

Loops retain results. Transport restart, preview/count-in reset and global
pattern changes clear histories; queued per-track pattern changes clear that
track's result when consumed. Song/performance modes, complex routing and every
transport combination have not all been separately hardware-tested.

## Fill FX1 and FX2

Use the stock three modes and endpoint fields. Select Chance explicitly.

- **Constant:** any percentage or condition, for example C50, C1:4, CPR, CNE,
  C!1S. Every eligible row receives that stored condition.
- **From-To:** interpolate across logical positions, then encode to existing
  condition IDs. Stock binary32 interpolation and integer truncation remain.
  Six selected rows from 0 to 59 produce `0, 11, 23, 35, 47, 59`.
  Reversing endpoints reverses direction; one row uses the first endpoint.
  Skipped rows retain stock interpolation positions. Try `1:2 → 4:4`,
  `PR → !NE`, or mixed `50 → 1S`.
- **Random:** the existing min/max controls select an inclusive logical interval.
  Reversed bounds are sorted. Stock PRNG/modulo and draw order remain; equal
  bounds still consume a draw. `PR → !NE` chooses PR, !PR, NE or !NE;
  `0 → !1S` includes all 142 values. Percentage-only ranges preserve stock
  generation exactly. No condition is automatically excluded.

Other FX and the stock Random-FX-type option keep their previous behavior.
There is no new Fill mode. Stored IDs: percentages 0–100; 1S/!1S 101/102;
ratios 103–137; PR/!PR 138/139; NE/!NE 140/141.
