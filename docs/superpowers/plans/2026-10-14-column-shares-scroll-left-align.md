# Column widths by fixed shares, clicks that never scroll, and all text left-aligned (plan, 2026-10-14)

**Status:** every question is answered (2026-10-14, see "The user's answers" below). The build waits for the user's yes. Nothing is built. Planner a764d5771db875e84 (task PLAN-CW, session 7dbd22aa) read every file named below on main at a7ff29c1. The D115 build started while this plan was written, so "Overlap with D115" below was rewritten by the main session at 18:45 from the D115 branches as they stood then.

This plan covers three changes to the GUI. Each one has the same shape: today several places answer one question, so the answer drifts. After the change one function answers it, every table or screen asks that function, and a scan rule stops a second answer from coming back.

The three questions are "how wide is a table column", "does a selection change scroll the list", and "which way is text aligned".

---

## The user's answers (2026-10-14)

The user answered all eleven questions in chat on 2026-10-14. On ten of them they took the recommendation. On question 6, which had none, they chose to keep button labels centred. On question 11 they approved the left-aligned breakdown after seeing it drawn from a real song. Where a section further down still offers a choice, the answer here wins.

1. **dmleaderboards shares:** yes, Song 48%, Artist 27%, Charter 25%.
2. **The two settings forms and the placeholder bars:** leave them where they are. The column-width rule's header says so.
3. **Mode leaves the path report (D115 A2-2):** whichever change lands second scales the four remaining shares to Song 31%, Artist 17%, Charter 16%, Path 36%.
4. **Previous, Next, Up and Down:** scroll only when the row is not fully on screen, and only as far as the nearest edge.
5. **A report row click that selects a Library song:** treated like Previous/Next (`Elsewhere` answers `NearestEdge`).
6. **Button labels:** stay centred. `apply_theme` does not set `ButtonTextAlign`, and ImGui's default stands. The Analysis settings button keeps its own push to (0, 0.5) (`settings_bar.cpp` line 276), which starts its cut label at the left. It is the one named exception in the alignment rule's owner lines.
7. **Controls at a window's right edge:** stay where they are. `move_to_right_edge` becomes `place_control_at_right_edge`, for buttons only. `right_align` and `align_right` merge into it.
8. **Text at a window's right edge** ("123 of 456 paths", "Built 13:42", "1:23 elapsed · 4 min left", the timeline's end label): moves left and starts right after the item before it on the line. The timeline's start and end labels sit side by side at the left. If that reads badly in the real-app check, the main session shows the user a screenshot before the merge; the fallback is the end label in the hover tooltip only.
9. **Text centred by hand:** the number in an activation's circle and the Preview's key-cap letters stay centred. Both are owner-line exceptions in the alignment rule, each with a comment naming this answer. The timeline's activation numbers and the "SP" gauge label start at their mark's x or the gauge's left edge. The startup box stays centred in the window, because it is a box and its text is already left-aligned.
10. **Progress bar overlays:** left to ImGui.
11. **Score breakdown:** left-aligned exactly as the user approved it. Every number starts in one column, right after the longest label and two spaces, and the "Avg. Multiplier" line's number starts in that same column. With today's labels the numbers start at column 19:

    ```
    Avg. Multiplier:  5.015x

    Notes:            102,145
    Combo Bonus:      318,060
    Star Power:       111,900
    Solo Bonus:       0
    Accent Notes:     3,850
    Ghost Notes:      1,100

    Total Score:      537,055
    ```

    The numbers come from the user's video of "Never Kill A Mouse; Let It Kill Itself (Demo)" on 2026-10-13. D115 B1-6 and B1-7 rename these labels (for example A2-5's `kAverageMultiplierLabel`, "Average Multiplier"). The start column then follows the longest label, so the rule is "longest label plus two spaces", not a fixed 19.

The question texts below are kept as they were asked, for the record.

## Questions for the user (answered above)

**1. The dmleaderboards report's shares (Change 1).** The comparison window (`src/app/dm_report_view.cpp` lines 52 to 54) has three text columns: Song, Artist and Charter. The same measurement that gave the Library and path report shares gives Song 48%, Artist 27%, Charter 25%. The user has not seen these. Proposed: use them.

**2. Two small forms that are not in the rule (Change 1, decision 7).** Two tables draw text and never go through `column_widths`. The batch confirm box's settings table (`src/ui/library_dialogs.cpp` line 363, `##batchsettings`, two columns: a setting's name and its value) and the Analysis settings panel's form (`src/ui/settings_bar.cpp` line 57, `##settingsform`, two columns: a setting's name and its control). Both open with `ImGuiTableFlags_SizingFixedFit` and no column setup, so ImGui makes each column as wide as its widest cell. Neither ever cuts text, so there is no share to give. The settings form also keeps its three groups' name columns one width through ImGui's synced-tables feature (three `BeginTable` calls with one ID). A third table, the grey placeholder bars a building report draws (`src/ui/report_window.cpp` line 142, `##placeholder`), holds no text. Recommended: leave all three where they are and say so in the rule's header. Routing the settings form through the rule would lose the synced widths unless the caller measured all three groups together, which is more code for the same picture. If the user wants "every table" to mean these too, each becomes a `begin_small_table` call with two columns that never cut, and the settings form's three groups hand the rule the names of all three.

**3. The path report's Mode column (Change 1 meets D115).** The D115 plan's answer 2 drops the Mode column from the path report (`docs/superpowers/plans/2026-10-12-one-record-per-chart.md`, "Questions for the user", item 2). The approved shares give Mode 12%. Once Mode goes, the other four text columns' shares sum to 88. Recommended: whichever change lands second scales the four up to 100 and keeps their proportions: Song 31, Artist 17, Charter 16, Path 36 (27, 15, 14 and 32 each divided by 88, times 100, rounded to a whole percent). The alternative is to measure again without Mode.

**4. Previous/Next and the arrow keys (Change 2).** Today any change of the selected song centres the row in the Library (`src/ui/library_table.cpp` lines 339 to 347 and 390: `if (selected_path != app.library_ui.scrolled_to)` sets `scroll_to`, and the row then calls `ImGui::SetScrollHereY(0.5f)`). A report window centres the row on every Up or Down press (`src/ui/report_window.h` lines 314 to 331 and 406). The design below: a row the user reaches with Previous, Next, Up or Down scrolls only when it is not fully on screen, and only as far as needed to bring it in at the nearest edge. Example from each end: the row below the last visible one, reached with Down, ends up at the bottom edge of the list, and everything above it moves up by one row; a row already on screen, reached with Up, moves nothing. The main session recommended this and the user did not object. Please confirm.

**5. A report row click that selects a Library song (Change 2).** Clicking a row in a report window selects that chart in the Library (`src/ui/path_report_window.cpp` line 192 and `src/ui/dm_report_window.cpp` line 143 call `AppState::select_chart`). Today the Library then centres that row, because its selection changed. The user's rule is "a click on a row never scrolls, in any table", but here the click was in another window and the Library row may be off screen. Recommended: treat it like Previous/Next, so the Library brings the row to its nearest edge when it is off screen and moves nothing when it is on screen. The alternative is no scroll at all, which can leave the selected song out of sight.

**6. Button labels (Change 3).** ImGui centres a button's label inside the button: `ImGuiStyle::ButtonTextAlign` defaults to (0.5, 0.5), and every `ImGui::Button` reads it. Hydra changes it in one place: the Analysis settings button pushes (0, 0.5) so its cut label starts at the left (`src/ui/settings_bar.cpp` line 276). On a button sized to its own label the two look nearly the same, since the label fills the button. The difference shows on a button wider than its label: "Pause" sits in a slot as wide as "Resume" (`src/ui/library_dialogs.cpp` line 434), "Hide library" in a slot as wide as "Show library" (`src/ui/details_panel.cpp` line 38), "Open path report" in a slot as wide as "Building path report..." (`src/ui/library_toolbar.cpp` line 130), and "<", ">" and "X" in square buttons. Centred: "Pause" sits in the middle of its slot with equal space each side. Left: "Pause" starts just inside the slot's left edge and the spare room is all on its right. Does "all text" include button labels? If yes, the owner sets `style.ButtonTextAlign` to (0, 0.5) once in `apply_theme` (`src/ui/theme.cpp` line 39 onward) and the settings button's push goes.

**7. Controls placed at the right edge (Change 3).** These are positions of buttons, not alignment of text. The song panel's header puts "Hide library", "<", ">" and "X" at the right edge (`details_panel.cpp` line 46); the toolbar puts "Open path report" there (`library_toolbar.cpp` line 140); the batch confirm puts "Cancel" and "Start analyzing" there (`library_dialogs.cpp` line 397); the running-batch strip puts "Pause" and "Stop" there (line 473); the finished-batch strip puts "Open report" and "X" there (line 523); the Paths tab puts "Show in Preview >" (`paths_tab.cpp` line 430) and "Expand all" / "Collapse all" (line 488) there; a report's header puts "Compare another player..." and "Refresh" there (`report_window.cpp` line 70) and "Cancel" while building (line 194). Recommended: they stay where they are. The rule is about where text starts inside the space it is given, and these buttons' labels follow question 6. If the user wants them moved, each one becomes the next item on its line, after the text, and the helper that places them (`move_to_right_edge` in `src/ui/widgets.h` line 155) is deleted instead of kept for controls.

**8. Text placed at the right edge (Change 3).** Four pieces of plain text are moved to the right edge the same way the buttons are. The report's count line "123 of 456 paths" sits at the right end of the controls row (`report_window.cpp` line 330, `right_align`). "Built 13:42" sits beside the header's buttons at the right (line 70). The running batch's time line "1:23 elapsed · 4 min left" sits at the right end of its line, after the counts (`library_dialogs.cpp` line 466). The Paths tab timeline's end label (the last measure's label) sits at the right end under the strip, opposite the start label (`paths_tab.cpp` line 268). Recommended: these follow the text rule and start at the left, right after whatever precedes them on the line. For the timeline that puts the start and end labels side by side at the left; if that reads badly the end label can go on the hover tooltip only. Please say which.

**9. Text centred by hand (Change 3).** Five places draw text at a position worked out from the text's own width so it sits centred. The activation number in its circle on an activation row (`paths_tab.cpp` line 298: `dot.x - CalcTextSize(num).x * 0.5f`). The activation numbers over the timeline's marks (lines 231 to 242 through `spaced_labels` in `src/ui/label_layout.h`, which centres each label on its mark and nudges it to avoid its neighbour). The letters on the Preview's key caps ("[", "]", "Space", `src/ui/preview_tab.cpp` lines 108 to 111). The "SP" label and the bar readout over the Star Power gauge on the highway image (lines 755 to 764). The startup progress box, which is a box centred in the window (`src/ui/library_view.cpp` line 146) whose text is left-aligned inside it. Recommended: the number in a circle and the key-cap letters stay centred, since the circle and the cap are drawn around the text and a left-aligned "7" in a circle looks off-centre; the timeline numbers and the SP label follow the text rule (each starts at its mark's x, or at the gauge's left edge); the startup box stays where it is because it is a box, not text. Please say which of the five move.

**10. Progress bar overlays (Change 3).** ImGui draws a progress bar's overlay text just past the filled part, clamped inside the bar (`third_party/imgui/imgui_widgets.cpp` line 1494), and centres it only on an indeterminate bar (one with no fraction, the report's "building" bar at `report_window.cpp` line 191). The startup box's "12,345 / 38,009" and the report's "Analyzing 1 of 2 records" therefore move with the fill. Recommended: leave these to ImGui; forcing them left would mean drawing the bar's text by hand. Say if they should start at the bar's left edge instead.

**11. The score breakdown's numbers (Change 3).** `build_score_breakdown` (`src/app/path_view.cpp` lines 418 to 429) pads each number to ten characters with `%10s`, so in the mono font the numbers line up at the right: "Notes:            102,145" over "Combo Bonus:        4,500". Left-aligned, the labels keep their padding and the numbers start at one column but end ragged: "Notes:            102,145" over "Combo Bonus:      4,500". The literals in `tests/test_path_list.cpp` lines 32 to 34 change with it. Confirm this is what "left-aligned" means here.

---

## Change 1: column widths by fixed shares

### The cause, in plain words

The Library's text columns are as wide as their text. `library_column_specs` (`src/ui/library_table.cpp` line 232) marks every column `may_cut`, and the rule in `src/ui/column_widths.cpp` then gives each cut column its "usual width", the width that fits 99 of every 100 cells (`kUsualShare`, line 21, read by `usual_of`, line 32). So the width depends on what the cells say right now.

Flipping a setting such as 2x Bass changes what every Best path cell says: the summary lookup key changes, so every cell reads "Not analyzed" until the chart is analyzed again. `update_column_widths` (line 248) measures those cells again through `update_cells`, the Best path column's usual width shrinks to the width of "Not analyzed", and `place_columns` hands the room it gave up to Title and Artist. Flip the setting back and the old widths return. A batch does the same once per finished chart, because `take_summary_changes` reports each landed row and the column grows one cell at a time.

The 2026-10-10 plan (`docs/superpowers/plans/2026-10-10-one-column-width-rule.md`) designed this on purpose, so that no room is wasted. The user has now decided the opposite: a text column's width must never follow its text.

### The design

A text column, one that may cut its text with "…", gets a whole-percent share written once in its table's column list. A number column, one that never cuts, stays exactly as wide as the widest thing it shows (its header or its widest cell), as today. Text columns split whatever room is left after the number columns, in proportion to their shares. Text never feeds a text column's width again.

The rule, as `place_columns` will work it out each frame:

1. Every column's floor is its header's width (the header text, or the width ImGui drew it at with the sort arrow, whichever is wider). This is unchanged.
2. Number columns take their widest width. Hidden columns take nothing.
3. The leftover is the room minus ImGui's spacing minus the number columns. The shown text columns split it in proportion to their shares, in whole pixels, each rounded down. The pixels lost to rounding go to the last shown text column, so the columns exactly fill the room and no stray horizontal scrollbar appears.
4. A text column whose split falls below its header's width is pinned at its header's width, and the others split what remains. This repeats until nothing more pins (the loop at `column_widths.cpp` lines 161 to 182 already does this; it changes only in what it splits by).
5. When even the headers do not fit, every text column is at its header's width and the table scrolls sideways, as today.
6. A column the user dragged is pinned at the dragged width until the rows reload. The other text columns split the rest by their shares. A dragged number column can only be widened: its floor stays its widest value, so a number is never cut (decision 2).

A worked example for the Library with 1,000 px of room and no spacing. Every Library column is text, so the number columns take nothing and the shares apply to the whole 1,000:

| Column | Share | Width (panel closed) |
|---|---|---|
| Title | 17 | 170 |
| Artist | 10 | 100 |
| Charter | 9 | 90 |
| Folder | 39 | 390 |
| Best path | 25 | 250 |

With the song panel open, Charter and Folder are hidden, so the three shown shares, 17, 10 and 25, split the same 1,000 (decision 3). 17/52 of 1,000 is 326.9, rounded down 326. 10/52 gives 192. 25/52 gives 480. Those sum to 998, so the last shown text column, Best path, takes the 2 lost pixels and ends at 482. These numbers are the literals the unit test pins.

With the panel open and only 300 px of room, and a fake font of 10 px a character so the headers are 50 ("Title"), 60 ("Artist") and 90 ("Best path"): Artist's split, 300 × 10/52 = 57, is below its 60 floor, so it pins at 60; Title and Best path split the remaining 240 by 17 and 25: 97 and 142, with the lost pixel going to Best path, so 143. At 150 px every split is below its floor, every column is at its header (50, 60 and 90), the inner width is 200 and the table scrolls sideways.

For the path report the number columns ("#", Score, Acts, Max skip, Hardest ms, the Timing chip, Early fill, Avg multiplier, SqIn, SqOut, Notes and the rest) keep their widest widths, and the five text columns split what is left: Song 27, Artist 15, Charter 14, Mode 12, Path 32. The Paths tab's backend table and the Dynamics tab each have one text column (Rating, Pad), which gets share 100 and so takes all the leftover room. The Stars tab has no text column, so nothing changes there.

The shares come from the main session's measurement of the user's real library: 19,512 charts in a copy of the installed `hydra.db`, measured in the app's fonts at 18 px, taking for each text column the width that fits 99 of every 100 cells and turning those into whole percents (`mock_measure.py` and `mock_render.py` in `docs/superpowers/plans/2026-10-14-column-shares/`, with their output `measure.json` and `results.json` and the three mocks the user approved the shares from, `mock_library_closed.png`, `mock_library_open.png` and `mock_path_report.png`). To run them again: `py -I mock_measure.py <folder>` and then `py -I mock_render.py <folder>`, where the folder holds a copy of `hydra.db` and the two fonts from `C:\Program Files\Hydra\resource`. The Library's five sum to 100, the path report's five sum to 100, and the proposed dmleaderboards three sum to 100.

### Where the shares live

`ColumnSpec` (`src/ui/column_widths.h` line 57) gets one more field, `int share`, after `width_of`, so the existing positional initialisers keep compiling and a number column's share stays 0. Each table's column list sets it: `library_column_specs` (`library_table.cpp` lines 236 to 240), `backend_column_specs` (`src/ui/paths_tab.cpp` line 346, Rating 100), `dynamics_column_specs` (`src/ui/dynamics_tab.cpp` line 77, Pad 100). The report tables build their specs from the app's column descriptions, so `app::report_view::CellLook` (`src/app/report_view.h` line 48, which already holds `truncate`) gets `int share`, the view files set it (`src/app/path_report_view.cpp` lines 68 to 77, `src/app/dm_report_view.cpp` lines 52 to 54), and `report_frame::column_spec` (`report_window.cpp` line 391) copies it across as it copies `truncate` today. That keeps each table's shares next to its own column list, the one place per table the user asked for.

`place_columns` splits in proportion to whatever the shown shares sum to, so hiding a column needs no second list. It asserts in a debug build that a table's text columns sum to 100, so a typo fails the first GUI test instead of quietly skewing a table.

### What goes away

Measuring text columns stops. `measure_widths` measures a `may_cut` column's header and nothing else. `usual_widths`, `kUsualShare`, `usual_of` and `update_cells` go, and with them the Library's per-cell refresh: `update_column_widths` no longer calls `take_summary_changes`, so `LibraryModel::take_summary_changes` and `summary_changes_` (`src/ui/library_model.h` lines 135 and 170) go too, with their test (`tests/test_library_model.cpp` line 423). The Library has only text columns, so it measures nothing but five headers; the 60 to 120 ms load-time measure the 2026-10-10 plan accepted disappears as a side effect. The report tables still measure their number columns over every row, as now.

One GUI test's premise goes with the old rule. `library-text-fits` (`tests/ui/uitest_library.cpp` lines 813 to 887) checks that no cell is cut while room sits unused at the table's right. Under fixed shares a long Best path is cut at 25% of the room even when Title has room to spare. That is the user's decision 1, so the test is deleted, not rewritten. `library-column-widths` (line 626) keeps its shape: it compares each column's `WidthGiven` with the production `place_columns` on the table's own inputs.

### Dragging

Today `apply_column_widths` (`column_widths.cpp` line 304) remembers a drag in the table's window state and skips the dragged column until the rule's width for it changes. Under shares the rule's width for a text column changes on every window resize, so that memory would lose the drag at the first resize. Decision 5 says a drag holds until the rows reload.

So the drag becomes an input to the rule rather than an exception to the apply step. `TableRoom` (`column_widths.h` line 114) gets `pinned`, one optional width per column. `table_room` reads it from the same window-state memory `apply_column_widths` writes when ImGui reports a resize (`table->ResizedColumn`, `LastResizedColumn`, lines 325 to 329 today). `place_columns` treats a pinned text column as fixed at that width and splits the rest among the other text columns by their shares; a pinned number column is fixed at the larger of the drag and its widest value. `apply_column_widths` then just sets every width the layout gives, with no skip.

The pins clear when the rows reload. Each table hands `table_room` a reset key, a number that changes when its rows do: the Library's is `LibraryModel::rows_version()` (bumped only in `set_charts`, `library_model.cpp` line 159) combined with a new `query_version()` counter bumped on each search change, so a new search, a scan and a restart all clear it (the window state is not saved to `hydra_ui.ini`, so a restart clears it by itself). A report's key is a counter in `report_frame::Memory` bumped by `reset()` (a new result) and when `search_box` reports a changed search. ImGui still writes dragged widths to `hydra_ui.ini`, and as today they never win after a restart, because the rule applies its widths on the first settled frame.

### Why the rule keeps its own arithmetic instead of ImGui's stretch weights

ImGui can do proportional columns itself: `ImGuiTableColumnFlags_WidthStretch` with an initial weight, and the enabled stretch columns split the room by weight (`third_party/imgui/imgui_tables.cpp` lines 1053 to 1060 set the weight from `InitStretchWeightOrWidth`, lines 1085 to 1095 give each enabled stretch column `width_avail × weight / sum`). Hidden columns are skipped there, so a re-split on hide comes free. The vendored version is 1.93.0 WIP (`third_party/imgui/imgui.h` line 32). The plan does not use it, for four reasons, each read in the vendored source.

First, ImGui's stretch has no header floor. A stretch column shrinks to `MinColumnWidth` (line 1093), which is `FramePadding.x`, and it never makes the table scroll instead. ocornut/imgui issue #5685, "Table: set minimum width for stretch column", asks for exactly that floor and is still open. Decision 4 needs the floor and the scroll.

Second, ImGui turns a drag into a new weight and saves it. `TableUpdateColumnsWeightFromWidth` (lines 2585 to 2611) rewrites every visible stretch column's weight from its dragged width, and the table settings save weights to `hydra_ui.ini`. A drag would change the shares for good and survive a restart, against decision 5. Issue #4046 records the related trap: an initial width passed to `TableSetupColumn` under `ScrollX` "ended up being the permanent width".

Third, ImGui says mixing stretch columns with `ScrollX` only makes sense with an explicit `inner_width` (lines 107 to 113 and 139 to 140). That width is the sum of every column's width plus spacing, which is what `place_columns` computes anyway. Once the rule knows every width, handing ImGui weights adds a second owner of the same answer.

Fourth, the rule already exists and is tested. `place_columns` and its unit tests (`tests/test_column_widths.cpp`) only change in what the text columns split by. The ImGui side (`setup_column`, `apply_column_widths`, `table_room`) keeps every column `WidthFixed` as it is today.

The web search (GitHub, ocornut/imgui issues #5685, #4046, #3935, #7934, #5920) found no issue describing a share-based layout with a header floor and a scroll fallback, which is what the user wants; the closest, #5685, asks for the floor and has not been built.

### Who owns the rule after the change

`place_columns` in `src/ui/column_widths.cpp` owns how wide every column is, for every table. The shares live in each table's column list. Nothing else sets or computes a width: `setup_column` is the only `TableSetupColumn` call in `src/ui`, and `apply_column_widths` the only writer of a column's `WidthRequest`.

A new single-owner row (ADR 0029, `tests/single_owner/rules/ui/column-widths.json`) keeps it that way: question "How wide is a table column?", owner `place_columns` in `src/ui/column_widths.cpp`, pattern `TableSetupColumn\s*\(|TableSetColumnWidth\s*\(|\.WidthRequest\s*=|ImGuiTableColumnFlags_Width(Fixed|Stretch)\b|\.StretchWeight\b` over `src`, with the owner lines in `column_widths.cpp` (`setup_column` and `apply_column_widths`). Its `must_match` examples are the old Library weights line and a `TableSetupColumn` call; its `must_not_match` is a `setup_column(...)` call. A second row, "What share of the room does a text column get?", owner `ColumnSpec::share` read in `place_columns`, pattern `\.share\b` over `src/ui`, with the reads inside `place_columns` as owner lines, so no other code can read a share and work out a width of its own.

### How a test proves it

Unit tests in `tests/test_column_widths.cpp` (doctest, fake font of 10 px a character), each expected number a literal worked by hand in the test's comment: the three Library examples above (1,000 px closed; 1,000 px with the panel open, including the 2 lost pixels; 300 px and 150 px with the floors). A number column beside text columns takes its widest and the text columns split the rest. A pinned (dragged) text column is fixed and the others re-split. A pinned number column never goes below its widest. A changed reset key clears the pins. Text in a cut column does not change the layout: two measures with different cell text give the same widths. The tests that encoded the old rule go: "cut columns share the leftover in proportion", "a cut column stops at its header's width and the rest share", "with room to spare, cut columns stop at their measured width", "one outlier row does not take the room", "the usual width is the widest below 100 rows", "when the usual widths don't fit", "a column that reaches its widest hands the rest on", "spare room grows the smallest gap first", "the outlier row shrinking is an update", "a one-cell update equals a full measure", "a dragged column keeps the drag until the rule's width changes" (replaced by the pin tests) and the 40,000-row timing case (lines 140, 154, 233, 267, 333, 349, 368, 383, 400, 421, 519 and 539).

GUI tests (`hydra_uitest`, `docs/agents/ui-testing.md`), written red first against the old code with the red line kept for the reviewer:

- `library-widths-hold` (new, `tests/ui/uitest_library.cpp`): scan the 97 test charts, note every column's `WidthGiven`, open the settings panel (`open_settings_panel`), click `##bass2x`, close the panel, wait for `view_settled`, and check every `WidthGiven` is unchanged. Then run a batch over the corpus (`Analyze library...`, `Start analyzing`, `wait-idle`) and check again. Red today: the Best path cells change from "Not analyzed" to "<score>  <path>", the measured width moves, and Title and Artist move with it.
- `library-column-widths` (existing, line 626): keeps comparing `WidthGiven` with the production `place_columns`; its narrow-library half still expects Title at its header's width and a horizontal scrollbar.
- `report-window-column-widths` (existing, `tests/ui/uitest_report_windows.cpp` line 1076): unchanged in shape; the report's number columns are still read from the production function.
- The Dynamics, Stars and backend-table lines in `tests/ui/uitest_details.cpp` and `uitest_paths.cpp` keep their one-line "width equals the rule's output" checks.

Tests pin literals or call the production function; none recomputes a share.

---

## Change 2: clicking a row never scrolls

### The cause, in plain words

The Library table treats every change of the selected song the same way. At `library_table.cpp` lines 339 to 347, when `selected_path` differs from `app.library_ui.scrolled_to`, it looks the row up and sets `scroll_to`; at line 390 that row calls `ImGui::SetScrollHereY(0.5f)`, which puts the row in the middle of the list whether it was on screen or not. A click changes the selection, so every click centres the clicked row. The list creeps down by the distance from the clicked row to the middle, and the next click at the same spot lands on a different song.

The report windows have the same centring on the arrow keys (`report_window.h` line 327 sets `scroll_to` in the Up/Down branch, line 406 calls `SetScrollHereY(0.5f)`). Their row clicks do not scroll today (`scroll_to` is set only in the key branch).

### The design

One function answers "does this selection change scroll the list, and how far". It takes why the selection changed and returns one of two answers: no scroll, or bring the row into view at its nearest edge. A click never scrolls. Previous, Next, Up and Down bring the row in at the nearest edge, which moves nothing when the row is already fully on screen (question 4). A selection made from somewhere else (a report row click, `AppState::select_chart`) is question 5; the recommendation is the nearest-edge answer.

The new pair of files `src/ui/list_scroll.h` and `.cpp` hold it (the `.cpp` joins the `hydra_ui` source list in `CMakeLists.txt`):

```
enum class SelectionSource { Click, Step, Elsewhere };
enum class ScrollTo { None, NearestEdge };
ScrollTo selection_scroll(SelectionSource source);        // the decision
void scroll_last_row(ScrollTo how);                        // applies it to the row just drawn
```

`scroll_last_row` is the one place that calls ImGui. For `NearestEdge` it calls `ImGui::ScrollToItem(ImGuiScrollFlags_KeepVisibleEdgeY | ImGuiScrollFlags_NoScrollParent)` right after the row's `Selectable`, where `SetScrollHereY` is called today. `ScrollToItem` is in the vendored `imgui_internal.h` at line 3694 and the flags at lines 1755 to 1766; only `list_scroll.cpp` includes that header for it, so the tables' files need no new include (`library_table.cpp` already has it at line 21; `report_window.h` does not and does not need it). `ScrollToRectEx` (`third_party/imgui/imgui.cpp` lines 12755 to 12808) does what the user asked for: it checks whether the item is fully visible (line 12776); if not, it scrolls so the item's top meets the top edge when the item is above (line 12798) or its bottom meets the bottom edge when below (line 12800), and it moves nothing when the item is already in view. `NoScrollParent` (line 12814) keeps the library pane or the report window from scrolling too. Both tables already force the target row through the clipper with `clipper.IncludeItemByIndex`, which is the condition ocornut/imgui issue #5995 ("Is ImGui::ScrollToItem supposed to work with ImGuiListClipper?") found necessary: the row has to be submitted for `ScrollToItem` to see it. Issue #2812 records that `ScrollToItem` was added in 2021 as the replacement for `ScrollToBringRectIntoView`; it is still marked work-in-progress in the header, so `scroll_last_row` is the one line to change if ImGui renames it.

The tables need to know why the selection changed. `AppState::select` (both overloads, `src/ui/app_state.h` lines 331 and 334) gains a `SelectionSource` argument and records it in a new `last_selection_source` field. The Library row click passes `Click` (`library_table.cpp` line 385). `select_relative` (`app_state.cpp` line 241, the panel's Previous and Next) passes `Step`. `select_chart` (line 501, the report windows' row clicks) passes `Elsewhere`. The Library table's change detector (`scrolled_to`) stays as it is; when it fires, the table asks `selection_scroll(app.last_selection_source)` and hands the answer to `scroll_last_row` at the row. The report table passes `Click` from its `pick` on a click and `Step` from the arrow branch.

### Who owns the rule after the change

`selection_scroll` in `src/ui/list_scroll.cpp` owns the decision and `scroll_last_row` the one ImGui call. A new single-owner row (`tests/single_owner/rules/ui/list-scroll.json`): question "Does a selection change scroll the list, and how far?", owner `selection_scroll` in `src/ui/list_scroll.cpp`, pattern `SetScrollHereY\s*\(|SetScrollFromPosY\s*\(|ScrollToItem\s*\(|ScrollToBringRectIntoView\s*\(|SetScrollY\s*\(` over `src`, with the one call in `scroll_last_row` as the owner line. `must_match` holds today's two `SetScrollHereY(0.5f)` lines; `must_not_match` holds `scroll_last_row(how);`. The test at `uitest_library.cpp` line 879 calls `SetScrollY` to set up a case, which is why the scope is `src`.

### How a test proves it

A unit test (`tests/test_list_scroll.cpp`, doctest) pins `selection_scroll(Click) == None`, `selection_scroll(Step) == NearestEdge` and `selection_scroll(Elsewhere) == NearestEdge` (answer 5), plus the literal flag value `scroll_last_row` passes.

GUI tests, red first:

- `library-click-keeps-scroll` (new, `uitest_library.cpp`): scan, scroll the table to a middle position with `ImGui::SetScrollY(table->InnerWindow, y)`, pick a row whose item rect sits in the lower half of the visible rect (the `read_drawn_table` helper at line 778 already finds the rows on screen), click it by title, yield, and check `table->InnerWindow->Scroll.y` is unchanged. Red today: `SetScrollHereY(0.5f)` moves the clicked row to the middle.
- `library-next-scrolls-just-enough` (new): with the song panel open on a row near the bottom of the view, click `>##nextsong` until the selected row is the first one below the visible rect, then check the selected row's item rect ends at the inner rect's bottom edge (within `ItemSpacing.y`), not in the middle, and that a further `<##prevsong` to a row already on screen leaves `Scroll.y` unchanged.
- `report-window-keys` (existing, `uitest_report_windows.cpp` line 376) gains the same two checks on the path window after `ctx->WindowResize` makes it short enough that the six sample rows do not all fit.

---

## Change 3: all text left-aligned

### The cause, in plain words

There is no rule for text alignment. Each place that wanted a number on the right, or a label centred, worked the position out itself from the text's width. Three helpers do the right-edge move: `move_to_right_edge` (`src/ui/widgets.h` line 155, which adds the leftover room to the cursor), `right_align` (`report_window.cpp` line 35) and `align_right` (`paths_tab.cpp` line 85), both of which call it after checking the item fits on the line. Centring is done by subtracting half the text's width from a centre point. One block of mono text pads numbers with spaces so they line up on the right.

### What the user sees today, and what left-aligned looks like

Text aligned somewhere other than left:

1. Number cells in both report tables. `report_frame::cell` (`report_window.cpp` line 357) calls `move_to_right_edge(CalcTextSize(text).x)` when the column is `numeric`: Score, Acts, Max skip, Hardest ms, Early fill, Avg multiplier, SqIn, SqOut and Notes in the path report; Actual, Hydra opt, Points left, % of opt and FC in the comparison. Today "1,234,567" ends at the column's right edge. Left: it starts at the column's left edge like Song does, and shorter numbers end ragged.
2. The "#" column in both report tables (`report_window.h` line 397): the dim row number is right-aligned. Left: it starts at the cell's left edge.
3. The score breakdown in the Paths tab (`src/app/path_view.cpp` lines 418 to 429, `%10s`): see question 11.
4. Button labels, centred by ImGui's default `ButtonTextAlign`: question 6.
5. Controls at the right edge: question 7. Positions, not alignment.
6. Text at the right edge: question 8.
7. Text centred by hand: question 9.
8. Progress bar overlays: question 10.

Text that is already left-aligned and stays so: every `TextUnformatted`, `TextColored`, `TextWrapped`, `text_ellipsized` and tooltip; table headers (ImGui draws the label at the left and the sort arrow at the right); the Library's cells; the Dynamics and Stars tables' cells; the Preview's overlay boxes (`src/render/overlay_layout.h` line 114 says every line is left-aligned at one inset); `SeparatorText` headings (ImGui's `SeparatorTextAlign` defaults to the left); combo previews and checkbox labels. The settings panel's rows keep names at the left and controls at the right, the user's own row rule from 2026-10-09; that is control placement, like question 7.

### The design

One owner, in `src/ui/theme.cpp`'s `apply_theme`, states the rule: text starts at the left edge of the space it is given, and a button's label stays centred in its button (answer 6). It is the one place a text-alignment style value is written. It writes `ButtonTextAlign` and `SelectableTextAlign` explicitly at ImGui's defaults, (0.5, 0.5) and (0, 0), with a comment naming answer 6, so the choice is visible in one place instead of left implicit. The header comment in `src/ui/widgets.h` points at it.

Then every right or centre site either goes or is routed through it:

- `cell` drops its `numeric` branch; `numeric` stays on `Column` because it also picks the first sort direction (`report_view.h` line 64). The "#" column drops its `move_to_right_edge`.
- `build_score_breakdown` pads every label, the "Avg. Multiplier" line's included, to the longest label plus two spaces, and writes the numbers unpadded (answer 11). `tests/test_path_list.cpp` lines 32 to 34 pin the new literals. The `right10` lambda goes. The single-owner row in `tests/single_owner/rules/core.json` whose `must_match` quotes a `right10` line (line 1910) is updated to the new line.
- `move_to_right_edge`, `right_align` and `align_right`: the controls stay at the right edge (answer 7). `move_to_right_edge` is renamed `place_control_at_right_edge`, and its comment says it is for buttons only. `right_align` and `align_right` are the same function written twice, so they become that one helper in `widgets.h`, which takes a button width. No call hands it a text width. The count line, "Built HH:MM", the batch time line and the timeline end label become ordinary text after the item before them (answer 8).
- The settings button's `PushStyleVar(ButtonTextAlign)` (`settings_bar.cpp` line 276) stays. It is the one exception answer 6 leaves, and it is an owner line in the alignment rule.
- The hand-centred draws follow answer 9. The circle number (`paths_tab.cpp` line 298) and the key caps (`preview_tab.cpp` lines 108 to 111) stay centred and become owner lines in the alignment rule. The timeline's activation numbers and the "SP" gauge label move: the x becomes the mark's or the gauge's left edge, and `spaced_labels` (`label_layout.h`) takes left edges instead of centres, with its unit test's literals updated.

### Who owns the rule after the change

`apply_theme` in `src/ui/theme.cpp` owns text alignment. A new single-owner row (`tests/single_owner/rules/ui/text-alignment.json`): question "Which way is text aligned?", owner `apply_theme` in `src/ui/theme.cpp`, pattern `ButtonTextAlign|SelectableTextAlign|SeparatorTextAlign|(move_to_right_edge|right_align|align_right|place_control_at_right_edge)\s*\(\s*ImGui::CalcTextSize|"%-?\d+s"|CalcTextSize\([^;]*\)\.x\s*\*\s*0\.5f` over `src`. The owner lines are the style lines in `apply_theme` and the three exceptions the user's answers keep: the settings button's push (answer 6), the circle number and the key caps (answer 9). `must_match` quotes today's `report_window.cpp` line 357 and `path_view.cpp` line 420. `must_not_match` quotes a `place_control_at_right_edge(button_slot_width("Cancel"))` call. The pattern cannot catch every hand-placed draw, so the reviewer's checklist for this change is the inventory above.

### How a test proves it

- `test_path_list.cpp`: the breakdown literals.
- `tests/test_label_layout.cpp` (if the timeline numbers move): literals for left edges.
- `report-window-cells-left` (new, `uitest_report_windows.cpp`): on the sample rows, the Score cell's item rect starts at its column's `WorkMinX` (within `CellPadding.x`), and so does the "#" cell. Red today: both start at `WorkMaxX` minus the text's width.
- `test_theme.cpp` (existing, it already makes an ImGui context): after `apply_theme`, `style.ButtonTextAlign` is (0.5, 0.5) and `style.SelectableTextAlign` is (0, 0), as literals (answer 6).

---

## Tasks, waves and files

Every task is sized for one Opus executor inside 100 tool calls and 20 minutes, in its own worktree. Each merge gets one fresh Sonnet reviewer who fixes its own findings and signs off (D107), plus the derive-once review the hook asks for. Executors run only their own tests; the main session runs the full suite once per merge.

The waves below are the planner's. "Overlap with D115" further down changes when each one starts, now that D115 is running. It also splits T5 into T5a (everything but the score breakdown) and T5b (the breakdown, after D115's B1-7).

**Wave 1**, three tasks together. No two own a file; the one shared file is named.

- **T1, the shares rule.** Owns `src/ui/column_widths.h`, `src/ui/column_widths.cpp`, `tests/test_column_widths.cpp`, the new rule file `tests/single_owner/rules/ui/column-widths.json`, and one line in each column list: `src/ui/library_table.cpp` lines 236 to 240, `src/ui/paths_tab.cpp` line 346, `src/ui/dynamics_tab.cpp` line 77, `src/app/report_view.h` line 48 (`share` on `CellLook`), `src/app/path_report_view.cpp` lines 68 to 77, `src/app/dm_report_view.cpp` lines 52 to 54 (the dm shares from question 1), and `src/ui/report_window.cpp` line 391 (`column_spec` copies `share`). It adds `share` and `pinned`, drops the usual-width code and `update_cells`, and leaves the Library's `update_column_widths` calling a now-empty `take_summary_changes` for T3 to remove (so main builds between merges). Shared region: `paths_tab.cpp` line 346 only; T5 owns the rest of that file. Runs `hydra_tests.exe -sf=test_column_widths.cpp`, `-tc="single-owner*"`, and `hydra_uitest --test library-column-widths --test report-window-column-widths` plus the Dynamics, Stars and backend-table width tests by name.
- **T2, the scroll owner.** Owns the new `src/ui/list_scroll.h` and `.cpp`, the `hydra_ui` source list in `CMakeLists.txt`, `src/ui/app_state.h` and `app_state.cpp` (the `SelectionSource` argument and `last_selection_source` only), `tests/test_list_scroll.cpp` and the rule file `tests/single_owner/rules/ui/list-scroll.json`. It does not touch the two tables; until T3 and T4 land, the rule row lists the two old `SetScrollHereY` lines as baseline entries in `tests/single_owner/baseline.json` dated inside the 14-day limit, and T3 and T4 remove them. Runs `-sf=test_list_scroll.cpp`, `-sf=test_app_click.cpp`, `-tc="single-owner*"`.
- **T5, the alignment owner and the non-table sites.** Owns `src/ui/theme.cpp`, `src/ui/widgets.h`, `src/ui/settings_bar.cpp` (line 276), `src/ui/library_dialogs.cpp`, `src/ui/library_toolbar.cpp`, `src/ui/details_panel.cpp`, `src/ui/paths_tab.cpp` (all but line 346), `src/ui/preview_tab.cpp`, `src/ui/label_layout.h`, `src/app/path_view.cpp` (the breakdown), `tests/test_path_list.cpp`, `tests/test_label_layout.cpp`, `tests/test_theme.cpp`, the `right10` example line in `tests/single_owner/rules/core.json`, and the rule file `tests/single_owner/rules/ui/text-alignment.json`. It leaves `report_window.*` to T4 and lists those lines as baseline entries the same way. Runs `-sf=test_path_list.cpp`, `-sf=test_label_layout.cpp`, `-sf=test_theme.cpp`, `-tc="single-owner*"`, and `hydra_uitest --test paths-rows --test paths-row-layout --test paths-folds-copy` (the Paths tab tests that read the activation rows, the timeline and the breakdown; the names are the table at `tests/ui/uitest_paths.cpp` lines 595 to 604).

**Wave 2**, two tasks together, each forked from wave 1's merge tip. They wait for wave 1 because both edit the files T1, T2 and T5 changed the interfaces of.

- **T3, the Library.** Owns `src/ui/library_table.cpp`, `src/ui/library_model.h`, `src/ui/library_model.cpp` (`query_version`, remove `take_summary_changes`), `src/ui/app_state.h` (the `ColumnWidths` struct's fields, lines 148 to 153), `tests/test_library_model.cpp`, `tests/ui/uitest_library.cpp`, and the two baseline entries T2 left for it. Writes `library-widths-hold`, `library-click-keeps-scroll` and `library-next-scrolls-just-enough` red then green, deletes `library-text-fits`. Runs `-sf=test_library_model.cpp` and `hydra_uitest --test` for each `library-*` test it touched.
- **T4, the report windows.** Owns `src/ui/report_window.h`, `src/ui/report_window.cpp`, `src/ui/path_report_window.cpp`, `src/ui/dm_report_window.cpp`, `tests/ui/uitest_report_windows.cpp`, and the baseline entries T2 and T5 left for it. The reset key for drags, the scroll owner on clicks and arrows, the cell and "#" alignment, the header's and count line's placement (questions 7 and 8), `report-window-cells-left`, and the two scroll checks in `report-window-keys`. Runs `hydra_uitest --test` for each `report-window-*` test it touched.

**After wave 2**, the main session runs the full suite once, then the real-app check below, then merges.

### Overlap with D115 (one record per chart and profile)

**The D115 build is running.** The planner wrote this section while D115 was still waiting. By 18:45 on 2026-10-14 another session ("One record per chart build") had started D115's wave 1 in eight worktrees under `.claude/worktrees/d115-*`. The main session read the branches then, and this is what it found:

- **A2-5** (`claude/d115-a2-5`, committed 8003b5d6, not merged). It rewrote `report_frame::cell` to cut text through a new `FittedTextCache` (`fitted_text` in `report_window.cpp`). The `numeric` branch with `move_to_right_edge` is still there, so T4 deletes it from the new `cell`. It also renamed three report labels: "Avg multiplier" is now "Average Multiplier" (`kAverageMultiplierLabel` in `src/app/display_format.h`), "Notes" is now "Note count", and the tier "No squeezes" is now "No timing needed". Number columns measure their headers, so those widths change by themselves. The shares are unaffected.
- **A2-1** (`claude/d115-a2-1`, uncommitted edits at 18:45). It is editing `src/ui/app_state.h` and `.cpp`, `library_model.h` and `.cpp`, `library_parts.h`, `library_toolbar.cpp`, `tests/ui/uitest_library.cpp`, `tests/test_app_click.cpp`, `tests/test_library_model.cpp` and `tests/single_owner/rules/ui.json`. T2, T3 and T5 all touch files on that list.
- **B1-1** (`claude/d115-b1-1`, committed). It touches `src/app/path_view.cpp` (`build_path_buttons`) and `tests/test_path_list.cpp` (a new all-0 case). Those are different functions and tests from the breakdown T5 rewrites, so they will only need a routine merge.
- A1-0, B1-2, B1-3, B1-5 and B2-1 touch the store, the engine, the chart parser, the preview view and timing. None of them touches a file in this plan.

The files both plans touch, read from D115's part files by the planner:

- `src/ui/report_window.h` and `.cpp`: D115's A2-5 (wave 1) adds `FittedTextCache` to `cell` and `table`; this plan's T1 (`column_spec`) and T4 (`cell`, `table`, the scroll call, the header) edit the same functions.
- `src/app/path_report_view.cpp` and `.h`: A2-5 changes labels, A2-2 (wave 6) drops the Mode column; T1 adds the shares (question 3).
- `src/ui/path_report_window.cpp`: A2-5 and A2-2; T4.
- `src/ui/stars_tab.cpp`: B1-4 changes the Multiplier header; this plan reads the file but changes nothing in it (no text column), so no conflict.
- `src/ui/paths_tab.cpp`: B1-8 (wave 8) renames `points` fields; T1 and T5.
- `src/ui/app_state.h` and `.cpp`: A2-1 (wave 1) and A2-3 (wave 4) edit other functions; T2 edits `select`, `select_relative` and `select_chart`.
- `src/app/path_view.cpp`: B1-6 and B1-7 (wave 8) rename `score_base` and its kin in `build_score_breakdown`; T5 rewrites the same lines.
- `src/ui/library_toolbar.cpp`: A2-1; T5 (one line, 140).
- `tests/ui/uitest_library.cpp`: A2-1 and A1-1; T3.
- `tests/ui/uitest_report_windows.cpp`: A2-5 and A2-2; T4.
- `tests/ui/uitest_details.cpp`: B1-4; T1 (one line).
- `tests/single_owner/rules/core.json`: B1-8; T5 (the `right10` line).

**Order, now that D115 is running.** The planner recommended running this plan before D115. D115 started first, so that no longer applies. Each task here forks from the D115 merge tip that already holds the D115 tasks touching its files. The rule is the one in the user's memory "Parallel by default": "after merge X" means fork from X's join tip once its code exists, never queue. A task never waits for D115 tasks that touch none of its files.

- **T1** (shares rule) forks once A2-5 is merged, because it edits `column_spec` in the `report_window.cpp` A2-5 changed. If A2-2 has merged by then, Mode is gone and T1 writes the scaled path report shares from answer 3 (31 / 17 / 16 / 36). Otherwise it writes 27 / 15 / 14 / 12 / 32, and A2-2's executor scales them when it drops Mode. The main session puts that line in A2-2's brief.
- **T2** (scroll owner) forks once A2-1 is merged, because both edit `app_state.h` and `.cpp`. A2-3 (D115 wave 4) edits other functions in the same files. It merges around T2 and T2 does not wait for it.
- **T5** (alignment) splits in two. **T5a** is everything except the score breakdown, and it forks once A2-1 is merged (both touch `library_toolbar.cpp`). **T5b** is the breakdown in `src/app/path_view.cpp` and its literals in `tests/test_path_list.cpp`, plus the `right10` example in `tests/single_owner/rules/core.json`. It forks once B1-7 is merged (D115 wave 8), because B1-6 and B1-7 rename the very fields and labels the breakdown lines print. Until T5b lands, the `%10s` line stays a dated baseline entry in the alignment rule, like T2's and T5's other baseline entries. Its 14-day limit (`kBaselineMaxDays`) has to cover D115's waves 2 to 8. If it would expire first, the main session re-dates the entry and says so in the merge note.
- **T3** (Library) and **T4** (report windows) fork from the tip holding T1, T2 and T5a, as before. T4 also needs A2-5, which T1 already waits for, and A2-2 if it has merged. If A2-2 is still waiting, T4 builds on the Mode column and A2-2's executor removes it later.
- `paths_tab.cpp` (T1's one line and T5a against B1-8 in D115 wave 8) and `tests/single_owner/rules/core.json` (T5b against B1-8) are only a routine merge. B1-8 renames `points` fields, and T1 and T5a edit other lines.

T1, T2 and T5a can run together as soon as A2-5 and A2-1 have both merged. On 2026-10-14 at 18:45, A2-5 was committed and A2-1 still had uncommitted edits.

### Stored data and the results stamp

Nothing here changes stored data. The files this plan touches are all under `src/ui`, `src/app` (the report views' column descriptions and the breakdown text) and `tests`. None of them writes to the store, and `src/store/stored_versions.h` is not touched: `kResultsStamp` stays "2.4.0" and `kChartMetaStamp` stays 4 (read at lines 62 and 81). `hydra_ui.ini` keeps being written by ImGui as today; the rule overwrites saved column widths on the first settled frame as it already does, so no ini migration is needed.

### The real-app check before merge

Tests checked against the rule's own output proved nothing last time (`column widths, 2026-10-10`, in the user's memory), so the main session runs the real `Hydra.exe` from `build-cpp` on a copy of the user's installed `hydra.db` (with its `-wal` and `-shm`) and the user's `hydra_ui.ini`, and:

1. Opens the Library with the song panel closed, flips 2x Bass in the Analysis settings and back, and watches that no column edge moves. Repeats with the panel open.
2. Clicks one row, then clicks the same spot five more times, and confirms the same song is selected each time and the list has not moved.
3. Selects a row near the bottom of the view and presses Next until the selection leaves the view: the list moves by one row at a time and the new row sits at the bottom edge.
4. Opens the path report, flips a setting, and confirms the columns hold; presses Down past the bottom edge and confirms the one-row scroll.
5. Takes screenshots of the Library with the panel open and closed and of the path report (and the comparison window if question 1 is answered yes), and shows them to the user before the merge.

---

## What this plan does not do

It does not change the stars, scores or stored records. It does not change what any cell says, only how wide its column is, where the list scrolls and where text starts. It does not route the two settings forms or the placeholder bars through the rule unless question 2 says to. It leaves `keep_table_column_order` (`widgets.h` line 168, imgui#9519) alone; that is about column order, not width.
