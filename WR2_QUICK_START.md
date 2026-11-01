# WR2 Quick Start Guide

## Make a slice group

1. Open **Sample Playback**, choose **Beat Slice** mode, then press **MORE** until Page 3 appears.
2. Select an unassigned slice and press **Trig** to create a group. That slice is the group master.
3. Select another slice and press **Add** to add it to the group. Repeat for more slices.
4. To change a member's chance, select it, press **Weight**, and turn the jog encoder. The group weights are adjusted to total 100%.
5. Press **Trig** again to leave group editing.
6. To remove a member, select it and press **Remove**. Removing the master deletes the whole group.

## Play and see groups

Triggering a group's master makes a weighted random choice among that group's slices. The weights describe chances over repeated triggers; playback is not round-robin and does not promise a fixed order. Triggering a member directly plays that slice.

Physical pad LEDs are **white only**. At maximum brightness the group master is brightest; active-group members are medium-high, slices in other groups are medium-low while editing, and unassigned slices are low. Pads without a slice are off. The perceived brightness also depends on the device's global pad-brightness setting.

Red/orange markers on the screen's waveform and the white pad LEDs show the same note-to-slice and group relationships in different ways. Red/orange describes the on-screen markers only; physical pad LEDs remain white.

## Save and protect your work

Normal project Save and Save As store group data in `project/wr2_groups.wr2`. Instrument Copy transfers groups when the source has WR2 data. Cross-project Instrument Import can carry groups when the source project has compatible WR2 metadata; without valid metadata, native Import continues without inferred groups. Import Sample into an existing instrument retains its WR2 groups.

Use compatible CT2+WR2 firmware to edit and preserve WR2 data. Stock or CT2-only firmware does not support WR2 playback or group editing. Before updating firmware or moving projects, keep an official recovery file and back up the **entire SD card**, including projects and Workspace. Recovery after a failed save or update is not guaranteed.
