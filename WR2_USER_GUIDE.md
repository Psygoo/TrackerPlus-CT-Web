# WR2 Details

## 1. Introduction

Slice Weighted Random (WR2) lets you group several slices within one Beat Slice
sample and give each slice its own playback probability. Trigger the group's
master slice and WR2 chooses one slice from that group using the assigned
weights.

This is **weighted probability, not round-robin playback**. A 60% slice is more
likely to play over many triggers, but WR2 does not play a fixed repeating
sequence or promise a particular short-term order.

## 2. Accessing WR2

1. Open **Sample Playback**.
2. Choose **Beat Slice** mode.
3. Press **MORE** until you reach **Page 3**. This is the WR2 group page.

## 3. Creating and Editing Groups

1. On Page 3, select an unassigned slice and press **Trig**. This creates a
   group and makes the selected slice its **master**.
2. To enter an existing group for editing, select its master or any member
   slice and press **Trig**.
3. Select another unassigned slice and press **Add** to make it a member of
   the group. Repeat for each slice you want to add.
4. Press **Trig** again to finish editing the group.

While editing, the selected group remains the group being edited even if you
select another slice. Leaving Page 3, changing instruments, or leaving
Beat Slice mode also ends group editing; it does not delete the saved group.

## 4. Adjusting Probabilities

1. Enter the group by selecting its master or any member slice and pressing
   **Trig**.
2. Select a member slice, press **Weight**, and turn the jog encoder to change
   its displayed percentage.
3. WR2 adjusts the other members so the group's weights continue to total
   100%.

A newly added member starts with a 10% weight, and WR2 adjusts the other
members to keep the group total at 100%.

## 5. Removing Slices and Groups

- To remove one member, select it and press **Remove**. WR2 redistributes the
  remaining weights so the group still totals 100%.
- To delete the whole group, select its master and press **Remove**.

## 6. Weighted Random Playback

Suppose a group has three slices with weights of 60%, 30%, and 10%. Each time
you trigger the master, WR2 makes a weighted choice among those slices. Across
many triggers, the first slice should play most often, the second less often,
and the third least often. You may still hear the same slice several times in
a row because this is random playback rather than a rotating sequence.

Triggering a **member** directly plays that particular slice. Slices that are
not assigned to a group keep their normal Beat Slice behavior.

Groups can be useful for varied drum hits, subtle instrument variations, or
octave variations. For example, divide one sample into slices containing
different kick hits or octave versions of a bass sound, then assign those
slices to one group.

## 7. Visual Indicators

The physical pad LEDs are **white only**. While editing a group, their relative
brightness helps show which slices belong to which group:

| Physical pad LED brightness | Slice relationship |
|---|---|
| Maximum brightness | Master of the group being edited |
| Medium-high brightness | Members of the active group |
| Medium-low brightness | Slices belonging to other groups |
| Low brightness | Unassigned slices that exist |
| Off | Pads with no corresponding slice |

The device's global LED brightness setting affects the perceived brightness of
all these levels.

The red/orange markers shown on the display and the white pad LEDs represent
the same note-to-slice and group relationships in different visual ways. The
red/orange colors apply to the on-screen markers; physical pad LEDs remain
white.

## 8. Saving and Reusing Groups

Normal project **Save** and **Save As** store WR2 group data in
`project/wr2_groups.wr2`. Keep this file with the project. WR2 also uses
companion files under `project/wr2_companions/` to support cross-project
Instrument Import. Keep the companion folder with the project when copying or
moving a project for that purpose.

- **Instrument Copy** transfers groups from a source instrument that has WR2
  data. Copying from a source with no WR2 groups removes any WR2 groups from
  the destination.
- **Instrument Import** from another project can bring groups when that project
  was saved with compatible WR2 firmware and its WR2 metadata and companions
  are available. If usable metadata is missing, the normal instrument import
  continues without adding inferred groups. Save the destination project
  afterward.
- **Import Sample** into an existing instrument retains that instrument's WR2
  groups and weights.

## 9. Compatibility and Safety

WR2 is for the **Polyend Tracker+ (plus model)**. It is not for Tracker Mini or
the original Tracker. The Web Patcher requires an unmodified official Polyend
Tracker+ firmware **1.2.0** file as input.

The Web Patcher processes the file locally in your browser. It does not upload
your firmware and does not provide Polyend firmware. Use compatible CT2+WR2
firmware to edit and preserve WR2 data; stock and CT2-only firmware do not
support WR2 playback or group editing. Do not expect them to preserve WR2
metadata when saving projects.

This is an unofficial experimental modification. It may cause instability,
data loss, or a failure to boot, and recovery is not guaranteed. Before
updating firmware or moving projects, back up the **entire SD card**, including
all projects and the Workspace folder. Keep an official firmware file available
so you can follow Polyend's recovery procedure if needed.
