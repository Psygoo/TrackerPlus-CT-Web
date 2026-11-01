# Weighted Random 2 (WR2)

WR2 adds weighted selection for groups of Beat Slice samples. This local production release candidate keeps CT2 mandatory and checked and makes WR2 optional and unchecked. It generates the verified CT2+WR2 image directly from original firmware 1.2.0. The candidate has not been deployed; the current public site remains CT2-only pending separate publication approval.

## Groups and playback

A group has a master slice and one or more member slices. Triggering the master note chooses among group members according to their weights. Triggering a member directly plays that slice. Unassigned slices continue to use stock Beat Slice playback. Weights are chances across repeated triggers, not a guaranteed short sequence.

In Beat Slice mode, use **More** to open Page 3. Select an unassigned slice and press **Trig** to create or enter a group. Select another unassigned slice and press **Add** to make it a member. The master marker is red; member markers are orange. Press **Trig** again to exit editing. Selecting another slice does not change which group is being edited. Leaving Page 3, changing instrument or leaving Beat Slice ends editing without deleting groups.

While editing, select a group slice, press **Weight**, and turn the jog encoder. The display shows its percentage; the other members are adjusted so the group totals 100%. Add assigns the new member 10% and redistributes the rest. Press **Remove** on a member to remove it and redistribute; removing the master deletes that group.

Example: assign three drum slices weights 60%, 30%, and 10%. Trigger the master note for weighted variation. Trigger an individual member note when a specific slice is needed.

## Saving, Copy and Import

Normal project Save and Save As persist groups in `<project>/wr2_groups.wr2`. WR2 also creates optional files under `<project>/wr2_companions/` for cross-project Instrument Import. Back up the full project directory and Workspace, including these files. They are not backups of unsaved changes.

Instrument Copy transfers a source instrument's groups independently. A source without WR2 groups replaces the destination's eligibility. Cross-project Instrument Import can carry groups when the source was saved with compatible WR2 firmware and its metadata is available; missing or invalid metadata leaves native Import working without inferred groups. Save the destination afterward. Import Sample retains existing WR2 groups under the documented S12C behavior.

## Compatibility and recovery

CT2 supplies conditional triggers; WR2 supplies weighted slice playback. CT2-only and stock firmware do not implement WR2. Do not expect WR2 metadata edits to be preserved after saving with stock or CT2-only firmware. Slice-count changes can make groups ineligible. Direct-slice FX S2 bypasses weighted selection; invalid groups fall back to direct playback.

Keep an official recovery file and a complete SD/project backup. WR2 sidecar publication is not an atomic transaction over every project file. An interrupted Save can leave the previous group authority unavailable. Recovery is not guaranteed; follow [the recovery guide](RECOVERY.md).

In this release candidate, select Weighted Random — WR2 before choosing the original firmware to generate the combined image. The public GitHub Pages site remains unchanged until this candidate receives separate publication approval.
