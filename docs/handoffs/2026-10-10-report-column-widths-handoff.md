# Handoff: report windows' sizes and column widths (2026-10-10)

**For the next session: plan this, don't build it.** Both fixes change how the
report windows look, so the design goes to the user first. Then the build waits
for their yes (repo rule 3). Nothing was built or launched in the session that
wrote this.

## What the user saw

The user sent a screenshot of the path report (the "Path Index" window) and
asked two things:

1. Why is the window trying to escape the screen's edges?
2. Why does the Hardest ms column waste so much space while the Timing column
   is too narrow? In the screenshot, the Timing chips are cut to "Beyond 170···".

When they heard the cause of the second one, their reply was: "why are
different columns deciding their widths in different ways? this is complete
nonsense". That reply sets the goal. Every column's width comes from one rule,
decided in one place.

## Cause 1: the window is bigger than the screen

The user's main Hydra window is maximized on a 2,560 × 1,440 monitor. The first
time a report window opens, `place_report_window` in `src/ui/app_shell.cpp`
(about line 369) copies the main window's inside area (its client area) for the
report's position and size. Windows then adds the report's own frame around that
area: the resize borders on the left, right and bottom. So the report's outer
edge sits a few pixels past the screen on three sides. Because the report is
a normal window and not a maximized one, Windows leaves it like that.

The bad size then sticks. ImGui saves it to the user's
`C:\Program Files\Hydra\hydra_ui.ini`. The `[Window][pathreport]` entry there
reads `ViewportPos=0,23` and `Size=2560,1417`, which is exactly the maximized
main window's inside area. On the next open, `placement_on_screen` (same file,
about line 120) only checks that a strip along the title bar is visible. An
oversized window passes that check, so it comes back at the same size every
time. The `[Window][dmreport]` entry has the same `ViewportPos=0,23`, so the
comparison report most likely has the same problem. Nobody has checked its
saved size yet.

This was traced from the code and the ini file. It has not been reproduced in
`hydra_uitest`.

## Cause 2: columns get their widths in two different ways

No code in Hydra says how wide a report column should be. `report_frame::table`
in `src/ui/report_window.h` (about line 304) sets up every data column as
`WidthStretch` with no weight, under `ImGuiTableFlags_SizingStretchProp`
(`base_table_flags` in `src/ui/widgets.h`). Under that policy, ImGui sets each
column's share from how far its cells drew while the table first laid itself
out (`third_party/imgui/imgui_tables.cpp`, about line 1058). So the way a cell
is drawn ends up deciding the column's width, and the two kinds of cell are
drawn differently:

- **Number cells** right-align by moving the cursor to the column's right edge
  (`move_to_right_edge` in `src/ui/widgets.h`, used in
  `src/ui/report_window.cpp` about line 353 and `report_window.h` about line
  367). The text always ends at the edge, so the column always seems to need
  its whole current width. A wide number column stays wide.
- **Chip cells** (the Timing column) shrink to the room they are given and cut
  the text (`chip` in `src/ui/report_window.cpp`, about line 359). So the column
  only seems to need the narrow width it already had.

The user's saved weights for the path table show the result. They are in
`hydra_ui.ini` under `[Table][0x69899948,16]`; a weight is a column's share
of the stretch. Hardest ms (column 9) is 1.4556, Timing (column 10) is 0.6201,
Early fill (column 11) is 1.6123 and Avg multiplier (column 12) is 1.6775.
These are saved, so the same layout comes back on every open.

## Other tables use other width rules

A search of `TableSetupColumn` under `src/ui` turned up more width schemes:

- The Library table (`src/ui/library_table.cpp`, about line 235) gives
  stretch columns hand-picked weights such as 0.75 and 0.5.
- The Paths tab (`src/ui/paths_tab.cpp`, about line 369) gives Timing, Chord
  and Points fixed widths that it measures, and lets Rating stretch.
- The Dynamics and Stars tabs (`dynamics_tab.cpp`, `stars_tab.cpp`) pass no
  width at all.

The planner should ask the user whether "one rule" covers only the two report
tables or every table in the app. The comparison report
(`src/ui/dm_report_window.cpp`) shares `report_frame::table`, so whatever rule
the path report gets, it gets too.

## The design proposed to the user (not yet approved)

**Columns.** One rule: a column is as wide as the widest thing it will show.
That is its header or its widest cell text, measured from the actual rows, with
chip padding included. Any leftover space goes to the text columns that
truncate (Song, Artist, Charter, Mode, Path). How a cell draws (right-aligned
number, chip) then only places text inside a width that's already decided. The
table needs a new ID so the old saved weights stop applying, which resets any
widths the user dragged in it once. Settle with the user what happens to a
column whose widest text would crowd out the text columns on a small window.

**Window.** Size the report so its whole outer window, frame included, fits the
monitor's usable area. Apply that on the first open and to a saved size that no
longer fits. If the main window is maximized, open the report maximized too.

## Open questions for the user

1. Does the one-width rule cover only the report tables, or every table?
2. When the main window is maximized, should the report open maximized, or
   open as a normal window that fits the screen?
3. Is the one-time reset of dragged column widths acceptable?

## Checks the plan should include

- Reproduce both problems in `hydra_uitest` before the fix, then show them
  gone after it. Follow `docs/agents/ui-testing.md`. Use the chart corpus in
  `testdata/input`, not the full library (rule 8).
- Check that the Timing chip shows its full label ("Beyond 170 ms").
- Test a saved oversized window size, not only a first open.
