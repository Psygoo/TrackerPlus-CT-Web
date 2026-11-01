# Slice Weighted Random (WR2)

WR2 adds weighted selection for groups of Beat Slice samples. Each master-slice trigger makes a weighted probability draw across its group; this is not round-robin playback and does not guarantee a fixed sequence. The Web Patcher keeps CT2 mandatory and checked and makes WR2 optional and unchecked. With WR2 selected, it generates the owner-confirmed, hardware-tested S13D CT2+WR2 image directly from your original firmware 1.2.0. Start with the [WR2 Quick Start Guide](WR2_QUICK_START.md).

## Groups and playback

A group has a master slice and one or more member slices. Triggering the master note makes a weighted random choice among that group's slices. Triggering a member directly plays that slice. Unassigned slices continue to use stock Beat Slice playback. Weights are chances across repeated triggers, not a guaranteed short sequence.

In Beat Slice mode, use **MORE** to open Page 3. Select an unassigned slice and press **Trig** to create or enter a group. Select another unassigned slice and press **Add** to make it a member. Press **Trig** again to exit editing. Selecting another slice does not change which group is being edited. Leaving Page 3, changing instrument or leaving Beat Slice ends editing without deleting groups. The [quick-start guide](WR2_QUICK_START.md) explains the on-screen markers and white physical pad LEDs.

While editing, select a group slice, press **Weight**, and turn the jog encoder. The display shows its percentage; the other members are adjusted so the group totals 100%. Add assigns the new member 10% and redistributes the rest. Press **Remove** on a member to remove it and redistribute; removing the master deletes that group.

Example: assign three drum slices weights 60%, 30%, and 10%. Trigger the master note for weighted variation. Trigger an individual member note when a specific slice is needed.

## Saving, Copy and Import

Normal project Save and Save As persist groups in `<project>/wr2_groups.wr2`. WR2 also creates optional files under `<project>/wr2_companions/` for cross-project Instrument Import. Back up the full project directory and Workspace, including these files. They are not backups of unsaved changes.

Instrument Copy transfers a source instrument's groups independently. A source without WR2 groups replaces the destination's eligibility. Cross-project Instrument Import can carry groups when the source was saved with compatible WR2 firmware and its metadata is available; missing or invalid metadata leaves native Import working without inferred groups. Save the destination afterward. Import Sample retains existing WR2 groups under behavior verified in S12C and inherited by S13D.

## Compatibility and recovery

CT2 supplies conditional triggers; WR2 supplies weighted slice playback. CT2-only and stock firmware do not implement WR2. Do not expect WR2 metadata edits to be preserved after saving with stock or CT2-only firmware. Slice-count changes can make groups ineligible. Direct-slice FX S2 bypasses weighted selection; invalid groups fall back to direct playback. Physical pad LEDs are white; the screen's red/orange waveform markers show the same master/member relationships separately. See the [WR2 Quick Start Guide](WR2_QUICK_START.md).

Keep an official recovery file and a complete SD/project backup. WR2 sidecar publication is not an atomic transaction over every project file. An interrupted Save can leave the previous group authority unavailable. Recovery is not guaranteed; follow [the recovery guide](RECOVERY.md).

In the Web Patcher, leave Slice Weighted Random (WR2) unchecked to generate CT2, or check it to generate CT2+WR2. Then choose your verified original firmware 1.2.0 file. The output name and verification hash match the selected combination.

Combined output identity: `TrackerPlus_CT2_WR2.ptf`, SHA-256
`0bdc7dfc32f5c98ebf8a3146811a87dc9fbf8a60a324ad9edc1a64cea7609c1a`.
The exact S12C recovery image remains locally preserved at SHA-256
`06adca54ff28ae704e92c4464a975f3c0e34f16d67a177db53157895be199445`.
