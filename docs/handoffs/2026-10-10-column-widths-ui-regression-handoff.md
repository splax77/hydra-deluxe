# Handoff: the one column-width rule broke the Library on a real library (2026-10-10)

**For the next session: diagnose and fix, starting from the findings below.** Nothing was changed while investigating. The dev settings file was backed up, used once, and restored byte for byte.

## Where things stand

The plan `docs/superpowers/plans/2026-10-10-one-column-width-rule.md` was built and merged to `main` in three merges:

- wave 1 (the rule in `src/ui/column_widths.*` and the report window fit) at 37adcab4 and 013f4c66;
- wave 2 (report tables, Library, Paths/Dynamics/Stars tabs) at cf494106;
- the "maximized report reopens on its own monitor" change plus the docs at 1f61e768.

The user then rebuilt `build-cpp\Release\Hydra.exe` (from `main` at 18f65cdb) and opened it on their real library (19,499 charts, `build-cpp\Release\hydra.db`). The Library came up broken: only Title, Artist and Best path showed, each squeezed to about its header's width (roughly 40, 45 and 60 px), with the rest of a 2,560 px wide window empty. Every title, artist and score was cut to three or four characters. The user's screenshot is in this session's scratchpad as `images\1.webp`.

## Why the tests missed it

Every GUI test passed, and every reviewer signed off. The tests checked that each column's width equals what the rule computes, so a wrong rule passes as long as the table obeys it. No test checked that the text actually fits, and nobody opened the real app on the real library before merging, even though the memory note on UI changes asks for screenshots before merge. The next fix needs a test that fails when visible text is cut while the window has spare room.

## Finding 1: Charter and Folder are hidden, and nobody asked for that (cause found)

The Library hides Charter and Folder when the song panel opens and shows them when it closes (`columns_for_panel` in `src/ui/app_state.h`, used in `render_table` in `src/ui/library_table.cpp`). This code dates from a50b3c04 on 2026-09-27.

At startup `columns_for_panel` is unset, so the table calls `TableSetColumnEnabled(true)` for both columns on its first frame. But ImGui restores the saved column state from `hydra_ui.ini` inside its first layout, after that call. So a saved "hidden" wins. This was seen directly: the real app run with the user's settings file showed Charter and Folder hidden with the song panel closed (screenshot `real1.png` in the scratchpad).

Wave 2 exposed it. It gave the Library table a new ImGui ID (the user's ini now holds `[Table][0x6ACAF71F,5]` beside the old `[Table][0x5E899311,5]`). The new entry was first saved while the song panel was open, so it holds `Visible=0` for both columns, and that comes back on every start. The likely fix is to apply the panel's column visibility on a settled frame, the way `report_frame::table` already waits for `table_settled()` before pushing its sort. Not yet built or tested.

## Finding 2: leftover room is lost when a text column hits its widest text (cause found)

In `place_columns` (`src/ui/column_widths.cpp`, about lines 114-140), text columns share the spare room in proportion to their widest text. Each one is then capped at its widest text with `std::min(..., wide[c])`. But the room a capped column didn't use is never handed on to the other text columns. So one column can be cut while room sits empty at the right.

This was reproduced headless on the test charts: after Burnout's analysis landed, its Best path text ("378,315  3- 1 2…") was cut while about 200 px stayed empty at the right edge (`attached3.png` in the scratchpad). The fix is to treat a capped column like a pinned one, taking it out of the share and sharing again, as the loop already does for columns pinned at their floor. A unit test should pin a case where one text column caps and another is cut.

## Finding 3: the user's narrow widths are not explained yet

These are ruled out, each checked:

- The column numbering. `library_cell_text` casts a column index to `LibrarySort`, and the two numberings match (`library_model.h` lines 30-40).
- ImGui capping the width. With `ScrollX` on, `TableCalcMaxColumnWidth` gives a non-frozen column no cap, and the Library freezes no columns.
- Asserts. `TableSetColumnWidth` asserts the layout is unlocked, but `imconfig.h` leaves `IM_ASSERT` as the standard assert, so a Release build skips the check.
- A big library alone. The headless runner started from a copy of `build-cpp\Release\hydra.db` (19,499 charts) sized the columns properly at 1,280 px (`libwidths2.png`). In that run every chart read "Not analyzed", because the runner's analysis settings differ from the user's, so the long Best path texts were not part of it.
- The user's settings file alone. The real dev app with the user's `hydra_ui.ini`, on the 97 test charts, sized Title, Artist and Best path properly (`real1.png`).
- The startup load. `collect_store_open` (`app_state.cpp` line 86) moves in the store job's model, whose `rows_version` is already 1 from `set_charts`. The first measure therefore sees the rows.

What is left is the combination none of the runs had together: the real 19,499-row library, with its real Best path texts, plus the saved ini (widths `Column 0 Width=43`, `Column 1 Width=48`, `Column 4 Width=74`, `RefScale=18`, Charter and Folder `Visible=0`), plus the live app at the user's DPI. Two leads are worth checking first.

The first lead is that ImGui changes a width by itself. Re-enabling a column queues an auto-fit (`AutoFitQueue`), a `RefScale` change rescales saved widths, and the header menu and a double-click on a border can auto-fit too. `apply_column_widths` keeps a memory of the width it last set and only sets a column again when the rule's width changes, so a width ImGui changed behind it is never put back. ImGui's auto-fit of a column whose cells were drawn cut measures the cut text, so a small width stays small.

The second lead is a model swap that keeps the same `rows_version`. `rows_version` lives in each `LibraryModel`, so two models that each went through `set_charts` once both read 1. Any path that moves in a fresh model over one already measured would skip the re-measure. Startup looked safe, but other paths were not checked.

The fastest way to settle it is a C++ GUI test, or a temporary debug print. Either one loads the user's `[Table][0x6ACAF71F,5]` entry with `ImGui::LoadIniSettingsFromMemory` (as `uitest_library.cpp` near line 696 does) and runs with `--db` on a copy of the dev DB under the user's analysis settings. It then prints, per column, the measured width, the room, `place_columns`' output and the table's `WidthRequest` and `WidthGiven`, frame by frame. The headless runner has no option to load a settings file, and attached mode (`Hydra.exe --uitest`) always uses the test charts, so neither reproduces this on its own.

Opening a copy of the user's DB only loads rows; it analyzes nothing. Still, check rule 8 before running anything wider.

## Update, later on 2026-10-10 (session db2ab41e): root cause found, fix approved

The main cause is the rule itself. It shares the room in proportion to each column's single widest cell, and the real library has outliers. On a copy of the user's DB, the longest Best path text is 1,128 characters (522 activations), the median is 10 and the 99th percentile is 40. Title runs median 15, 99th percentile 56, longest 137. That one path makes Best path look about 12,000 px wide, so it swallows the room.

This was reproduced in the real Release app, run from a scratch copy with the user's `hydra_ui.ini` and a copy of `hydra.db`. At 2,560 px, Title and Artist were cut at 179 and 80 px while Best path took 2,251 px, mostly blank. With a song open, Title and Artist dropped to about their header widths. After closing it, all five columns were cut, with about 1,500 px of blank Best path on the right.

`apply_column_widths` also never puts back a width ImGui changed by itself. ImGui clamps a width set on a hidden column to its 4 px minimum, which is where the ini's `Width=4` lines come from. The user's exact screenshot, with all three columns at header width and the panel closed, was not reproduced, and the click sequence behind it is unknown.

The user chose the fix "usual width, then grow." Each column first gets the width that fits 99 of every 100 rows. Spare room then goes to the columns still cut, the one closest to fitting first. Two tasks build it in parallel. `cwf-rule` changes the rule, puts back widths ImGui changed, and updates the guide. `cwf-lib` fixes Finding 1 and adds `library-text-fits`, the outcome test. The briefs are in session db2ab41e's scratchpad. Before merge, the main session takes screenshots of the real app with the user's ini and a DB copy, using the PrintWindow/PostMessage scripts `shot.ps1` and `steps.ps1` in that scratchpad.

**Done:** merged to `main` at 912d43bd (not pushed), reviewed CLEAN, full suites green (1,296/1,296 unit, 94/94 GUI). The real-app screenshots on the user's ini and a DB copy are in `2026-10-10-maximized-report-stretch/j-*.png`. The next open item, a maximized report drawn too small and stretched, is in `2026-10-10-maximized-report-stretch-handoff.md`.

## Repro scripts and screenshots (this session's scratchpad)

The scratchpad is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\557b3904-50b5-4b52-9425-0e5acda90af8\scratchpad\`.

- `libwidths.txt` scans the test charts and takes `libwidths.png`. It looks right.
- `libwidths2.txt` with `--db dbcopy\hydra.db` takes `libwidths2.png`. With the big library, the columns size properly at 1,280 px.
- `attached.txt` opens Burnout and closes the panel, ending in `attached3.png`. It shows Finding 2.
- `attached_real.txt` was run as `Hydra.exe --uitest` with the user's ini and took `real0.png` and `real1.png`. They show Finding 1.
- `hydra_ui.ini.backup` is the user's dev settings file, as it was before the attached run. It was copied back afterwards and the hashes match.

## Other open items from this work

- The report window's maximize, restore and reopen behaviour has never been checked by hand in the real app. No test can see a real OS window.
- Worktrees from this work are still on disk under `.claude\worktrees\`: cw-t1 to cw-t7 and cw-w2. They can be cleaned up once the fix lands.
- Another session (Single-owner redesign, branch `claude/so-t1`) is replacing `tests/test_single_owner.cpp` with `tests/single_owner/rules/*.json`. Coordinate any new scan row with it.
- The plan's Part 2 lines about 132 and 152 still describe a saved-maximized report reopening on the main window's monitor. That was superseded by the user's decision and 1f61e768.
