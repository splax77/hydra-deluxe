# One column-width rule, and report windows that fit the screen (plan, 2026-10-10)

**Status:** approved by the user on 2026-10-10, with their answers at the end. The build runs in the two waves of Part 4.

**Where this came from:** `docs/handoffs/2026-10-10-report-column-widths-handoff.md`. The user saw the path report window hang past the screen's edges, and saw the Hardest ms column wide while the Timing chips were cut to "Beyond 170···". Their verdict: "why are different columns deciding their widths in different ways? this is complete nonsense." The user then answered the handoff's open questions:

1. One width rule covers every table in the app.
2. A report opens maximized when the main window is maximized. Otherwise its whole outer window, frame included, fits the monitor's usable area. A saved size that no longer fits is corrected the same way.
3. In a narrow window the text columns give way first. Numbers and chips always show in full. Text columns shrink and cut with "…" down to a minimum, then the table scrolls sideways.
4. A one-time reset of saved column widths is fine.

Every claim below was checked against the code on `main` at 75ea6722, not taken from the handoff. Three of the handoff's claims, and one point in the question reviewer's note, needed correcting or filling in; they are in the last section.

---

## Part 1: the one width rule

### What happens today

No code in Hydra says how wide a column should be. Each table does something different, and in the report tables nothing decides at all.

The report tables (`report_frame::table`, `src/ui/report_window.h` lines 304-320) set up every data column as `WidthStretch` with no weight under `ImGuiTableFlags_SizingStretchProp` (`base_table_flags`, `src/ui/widgets.h` line 150). Under that policy ImGui gives each column a share equal to how far its cells drew during the first three frames (`third_party/imgui/imgui_tables.cpp` line 956 "Fit for three frames", line 1058 for the share). So how a cell draws decides the column's width. A number cell right-aligns by moving the cursor to the column's right edge (`move_to_right_edge`, `widgets.h` line 143, called from `cell` in `report_window.cpp` line 353), so it always seems to fill its column and a wide column stays wide. A chip shrinks to the room it has and cuts its text (`chip`, `report_window.cpp` lines 359-375: `w = min(text + 2 * FramePadding.x, avail)`), so it never asks for more. The user's saved weights in `hydra_ui.ini` (from the handoff: Hardest ms 1.4556, Timing 0.6201) are that accident, frozen.

The Library table (`src/ui/library_table.cpp` lines 235-249) gives its five stretch columns hand-picked weights: Title 1.0, Artist 0.75, Charter 0.5, Folder 0.75, Best path 1.0.

The Paths tab's backend table (`src/ui/paths_tab.cpp` lines 352-372) measures the widest Timing, Chord and Points text itself, rounds up, and bakes the three widths into the table's ID (`app::backend_table_id`, `src/app/path_view.cpp` line 70) so a new width makes a new table. Rating stretches. This is the rule the user wants, written once for one table.

The Dynamics tab's two tables and the Stars tab's table (`src/ui/dynamics_tab.cpp` lines 124-130 and 155-160, `src/ui/stars_tab.cpp` lines 29-34) pass no width at all under `ImGuiTableFlags_SizingFixedFit`, so ImGui auto-fits each column to the widest cell it drew. That is the right answer, found by ImGui's own rule, over three frames.

That is four width schemes. The user wants one.

### The rule, in words

A column is as wide as the widest thing it will show: its header, or its widest cell text, whichever is wider, measured from the actual rows in the font the cell draws with, with a chip's padding included. Columns that may cut their text (Song, Artist, Charter, Mode, Path in the reports; Title, Artist, Charter, Folder, Best path in the Library; Pad in Dynamics; Rating in the backend table) share whatever room is left after the others take theirs, in proportion to their widest text. When there is no leftover room, those columns shrink, each down to its header's width, and then the table scrolls sideways so the number and chip columns still show in full. How a cell draws (right-aligned number, outlined chip, "…" cut) only places text inside a width that is already decided.

The minimum for a cut column is its header's width. That is not a new number: a header is never cut, so a column can never usefully be narrower than it. If the user wants another floor, that is a number for them to name.

### Where it lives and what it looks like

A new pair of files, `src/ui/column_widths.h` and `src/ui/column_widths.cpp`, in `hydra::ui`. The `.cpp` joins the `hydra_ui` library's source list in `CMakeLists.txt` (lines 355-381). The rule is plain C++ with no ImGui in its arithmetic, like `render::ellipsize` (`src/render/overlay_layout.h` line 101), so a unit test can feed it a pretend font. It has three parts.

**Measuring.** A table hands over one `ColumnSpec` per column: the header text, whether the column may cut, and any extra padding each cell adds (a chip adds `2 * FramePadding.x`; everything else adds nothing). It also hands over the row count and a function that returns column `c`'s text for row `r`, plus a `width_of(text)` function. In the app that is `ImGui::CalcTextSize` with the column's font pushed (the reports' mono columns and the Library's Best path cell draw in `g_mono_font`); in a unit test it is a fake that counts characters. The result, `MeasuredWidths`, is one number per column: the widest of header and cells, padding included, before ImGui's own `CellPadding`. This is the expensive part and it is cached (next section).

**Placing.** `place_columns(measured, specs, available_width)` returns one width per column for this frame, and the table's inner width. Non-cut columns get their measured width. Cut columns share `available - sum(non-cut) - ImGui's spacing` in proportion to their measured widths, each clamped between its header's width and its measured width. If the sum of everyone's minimum is more than the available width, the inner width is that sum and the table scrolls. This is pure arithmetic, called every frame; it costs a few dozen operations. Only the enabled columns are handed in. The Library hides Charter and Folder while the song panel is open (`library_table.cpp` lines 258-263), and the header's right-click menu can hide any column but Title, so a hidden column takes no width and no share. The table already reads which columns are enabled with `TableGetColumnFlags(c) & ImGuiTableColumnFlags_IsEnabled` (lines 277-280).

**Applying.** `apply_column_widths(layout)` is the only place that touches ImGui. Called right after a table's `TableSetupColumn` calls, it sets every column `WidthFixed` and, once the table has settled, calls `ImGui::TableSetColumnWidth(c, width)` for each column whose width differs from what the rule last applied. "Once settled" matters: on a table's first frame ImGui loads its saved `.ini` widths inside `TableUpdateLayout` (`imgui_tables.cpp` lines 864-865), after anything we set in `BeginTable`'s frame, so a width set on the first frame is overwritten by the saved one. `report_frame::table` already waits one frame for the same reason when it pushes the sort (`report_window.h` lines 327-334, `table_settled()`), and the rule uses the same check.

So the whole table is Fixed columns and the "stretch" is ours. That is deliberate. ImGui's own comments say a stretch column with `ScrollX` only makes sense with an explicit inner width, and that mixing fixed and stretch columns with `ScrollX` "does not make much sense" (`imgui_tables.cpp` lines 111-112 and 139-140). Doing the share in our own code keeps the rule in one place and never asks ImGui to do the thing it says not to do. The one thing ImGui still owns is dragging.

### Dragging

With every column Fixed, dragging a column's edge changes that column's width and shifts the columns after it (`TableSetColumnWidth`, `imgui_tables.cpp` lines 2535-2541, the "offsetting resize" path). The drag holds until the width the rule wants for that column changes. The apply step sets a column only when the rule's width for it differs from what the rule last applied, so a re-measure that leaves a column's widest text the same leaves a drag on it alone. The cut columns re-share the leftover as the window is resized, so a drag on a cut column lasts until the next window resize. A drag on a number or chip column lasts until a re-measure changes that column's widest text. Dragged widths are still written to `hydra_ui.ini` by ImGui, but they never win over the rule after a restart, because the rule applies its widths on the table's first settled frame of every launch. That is the one-time reset the user accepted, and it needs no new table IDs: the rule overwrites the old saved widths on the first settled frame and ImGui saves the new ones over them. The `[Table][0x69899948,16]` entry the handoff names simply gets rewritten.

If the user wants dragged widths to survive a launch, that is a different design (the rule would have to record that a column was dragged and leave it alone) and should be said now.

### When widths are measured, and where they are cached

Measuring every row every frame is out: the Library can hold tens of thousands of rows. The rule measures once per trigger and keeps the `MeasuredWidths` next to the rows it measured.

The triggers are: the rows changed, or the UI scale changed. Everything else (window resize, scroll, sort, search) only re-runs the cheap placing step. Sort and search do not change the widest text; the rule measures all rows, not the visible ones, so a filtered table keeps its widths and the columns never jump as the user types.

How each table knows its rows changed:

- **Library.** `LibraryModel` gets a `rows_version()` counter, bumped only in `set_charts` (`src/ui/library_model.cpp` line 120), which replaces the rows; that triggers a full measure. Summaries landing do not. They come through `set_summaries` (line 147) and `set_summary_for` (line 156): on every analyze-on-click, after a settings change, and about once a second while a batch runs (`tick_library` calls `refresh_library_summaries` at most once per `kBatchRefreshSeconds = 1.0`, `app_state.cpp` lines 172-181 and `app_state.h` line 386; that refresh calls `set_summaries`, `library_jobs.cpp` line 67). A full measure there would hitch the UI once a second during a batch. A summary changes only its row's Best path text, and both functions already know which rows changed (`apply_summary` returns true for those, lines 151 and 159). So the model records those rows, and the Library measures just their Best path cells: a new text wider than the column's widest becomes the widest. If the row that held the widest got narrower, only the Best path column is measured again. The result is the same as a full measure, at the cost of one cell per landing in the usual case. The cached widths, with the row that holds each column's widest, live in `LibraryViewState` (`src/ui/app_state.h` line 100) with the version they were measured at.
- **Reports.** Each window's `rebuild` makes a fresh `TableView` (`src/ui/path_report_window.cpp` line 82, `src/ui/dm_report_window.cpp` line 59), so the cache lives beside the view in `PathWindow` and `DmWindow` and is measured right after the view is made. A closed window drops its rows (`drop_rows`) and the widths with them.
- **Paths tab backend tables, Dynamics, Stars.** The rows are a handful per table and come from the open song's record. They are measured when the record changes, cached in the tab's UI state, in the same shape.

The UI scale is a trigger because every width is in pixels of the current font. `set_ui_scale` (`src/ui/app_shell.cpp` line 217) runs at startup, when the window lands on a monitor with another DPI (`src/ui/main.cpp` line 262), and on `WM_DPICHANGED` (line 373). Each cache records the `g_ui_scale` (`src/ui/fonts.h`) it was measured at, and a cache whose scale differs from the current one is measured again. `set_ui_scale` needs no change. The measure is then redone at the next frame, on the main thread. It cannot move off the main thread: ImGui 1.92's font loader rasterizes glyphs on demand while measuring (`setup_imgui` relies on that, `app_shell.cpp` line 300), and the atlas is not shared across threads.

**Cost.** One measure of the Library is `rows × 5 cells` calls to `CalcTextSize`. ImGui's `CalcTextSizeA` is a per-character loop with one glyph lookup per character. At 40,000 rows and a guess of 30 characters a cell, that is 6 million characters. I have not timed it; a reasonable guess is 10 to 20 ns a character, so 60 to 120 ms once per library load or scan, and once per DPI change. That is one hitch, not a freeze, but it is near the line the user drew for the UI ("long work never freezes the UI"). It happens only on a full measure: a library load, a scan, a chart whose hash changed (all through `set_charts`), or a UI scale change. A click or a batch refresh measures only the Best path cells of the rows that changed (see the Library bullet above), so a batch does not repeat the full measure. A settings change (chart mode, SP cap, lens) can change most rows' summaries at once (`app_state.cpp` lines 859-863), and the "widest row got narrower" case re-measures the whole Best path column; either is one column of five, about a fifth of a full measure (12 to 24 ms on the same guess). Task 1 below times the full measure, a one-column measure and the one-cell update on a synthetic 40,000-row table and reports all three. The timing uses a real ImGui font and `CalcTextSize`, not the unit tests' fake `width_of`, because the fake only counts characters and says nothing about the 10 to 20 ns guess. (`tests/test_theme.cpp` already makes an ImGui context inside `hydra_tests`.) If a font cannot be made there, Task 1 stops and says so, and the number comes from a GUI test in wave 2 instead. If it is too long, the fallback is to measure only the strings that can be widest: sort each column's cells by byte length and measure the longest few dozen. That is cheaper and nearly always exact, but a short string of wide letters can beat a longer one, so it is a change to the rule's promise and needs the user's yes. The plan does not assume it.

### How it meets ImGui's table sizing

Every table gets `ImGuiTableFlags_ScrollX`. (The Library and the report tables already have `ScrollY`; the Paths backend, Dynamics and Stars tables have neither.) With `ScrollX` and the inner width the rule computes, ImGui lays the columns out in that width (`imgui_tables.cpp` line 1079 uses `InnerWidth` when it is set) and shows a horizontal scrollbar only when the inner width is more than the table's outer width. ImGui's header comment says `inner_width > 0` is exactly the way to get a scrolling table with a known total width (lines 105-108). `inner_width` is an argument of `BeginTable` (`imgui_tables.cpp` lines 315 and 385), so the table's inner width has to be known before `BeginTable` is called: `place_columns` runs first each frame, with the room the table will have, and its inner width goes in as that argument. Without it, "Stretch columns become Fixed columns" (line 107), which is why no half measure with stretch columns works.

Two ImGui details the executor must handle:

- `TableSetupScrollFreeze(1, 1)` in the report tables (`report_window.h` line 307) freezes the "#" column. With `ScrollX` a frozen column may not grow past the visible width (`TableCalcMaxColumnWidth`, `imgui_tables.cpp` lines 2436-2444). The "#" column is a few digits wide, so this never bites, but the plan names it so no one wonders why that column has a cap.
- `TableSetColumnWidth` clamps to `table->MinColumnWidth`, which is `FramePadding.x` (line 905). The rule's floor (the header's width) is always above that.

The "#" column goes through the rule too. Today `row_number_width` (`report_window.cpp` line 382) measures the widest row number, which is the same rule written a second time. The rule gets it as a column whose header is "#" and whose cells are `group_thousands(k)`, and `row_number_width` goes.

### The schemes this replaces

Each of these goes away, so only the one rule remains:

- `report_frame::table`: the `WidthStretch` flags, `row_number_width`, and `measure_columns` (`report_window.h` line 350, `report_window.cpp` line 421), which sums the first five headers' widths for the window's minimum size (D103 item 14). The window's minimum width keeps D103 item 14's meaning, the first five columns: it becomes those five columns at the widths the rule gives them at its minimum (see "Narrow windows").
- `chip` in `report_window.cpp`: the `min(…, avail)` shrink. The chip draws its text at full width inside a cell the rule sized for it.
- `cell` in `report_window.cpp`: `move_to_right_edge` stays (it places a number inside the decided width), but it no longer influences the width.
- The Library's weights 1.0 / 0.75 / 0.5 / 0.75 / 1.0.
- The Paths tab's own measuring loop and `app::backend_table_id` with its declaration in `src/app/path_view.h` line 111. Three tests pin it: `tests/test_path_view.cpp` line 907, `tests/ui/uitest_paths.cpp` line 476, and a derive-once scan in `tests/test_single_owner.cpp` (lines 4037-4053) that names the function and the Paths call.
- The Dynamics and Stars tables' bare `TableSetupColumn("…")` calls under `SizingFixedFit`: they become specs for the rule, with explicit widths.
- `base_table_flags` in `widgets.h`: replaced by one `table_flags()` in `column_widths.h` that the Library and the report tables start from, with `ScrollX` in it. The reports still add `SortMulti`. The Paths backend, Dynamics and Stars tables do not use `base_table_flags` today (they open with `Borders | Resizable` or `RowBg`, plus `SizingFixedFit`), so they keep their own look flags. They take from `column_widths.h` only what the rule needs: `ScrollX` and the fixed sizing. Task 1 writes the header so those two pieces can be taken without the Library's `Sortable | Hideable | ScrollY`.

`keep_table_column_order` (`widgets.h` line 164, imgui#9519) stays as it is; it is about order, not width.

`tests/test_single_owner.cpp` also pins `base_table_flags` as the owner of "which flags does a Library-style table open with" (lines 5600-5614, which name `report_frame::table_flags` and the Dynamics and Stars `table_flags` lines as the allowed callers). Its `move_to_right_edge` entry (lines 5661-5679) stays true. No task owns that file. The main session edits the `backend_table_id` and `base_table_flags` entries at the merge after wave 2, together with the `widgets.h` deletion (Part 4). Until then a wave-2 executor leaves the file alone, does not run it, and lists in its report which entries its change makes stale.

### Narrow windows

The sequence as the window shrinks: cut columns give up their leftover share; then each shrinks toward its header's width, cutting with "…" and offering the full text on hover as `text_ellipsized` already does; then, once every cut column is at its floor, the inner width stops shrinking and the horizontal scrollbar appears. Numbers and chips never lose a character. The Timing chip reads "Beyond 170 ms" at every window width. (The 170 is `beyond_edge_text(hit_window_ms)`, `src/app/report.cpp` line 151, for the user's hit window.)

The report window's own minimum width (D103 item 14, `report_window.cpp` lines 230-238) keeps D103 item 14's meaning: "the controls row, or the first five columns, whichever is wider" (`docs/audit/2026-10-03-fix-decisions.md` line 464). Only the source of the five widths changes: they come from the rule's output at its minimum instead of from `measure_columns`. It must not become the whole table's minimum inner width. The table has 16 columns in the user's saved layout, and a window that could never be narrower than all of them would never scroll sideways, which is the behavior the user chose.

---

## Part 2: the report window's size

### What happens today

`place_report_window` (`src/ui/app_shell.cpp` lines 369-380) runs every frame before a report's `Begin`. When the window is not already open, it takes the saved rectangle (`saved_rect`, from the live ImGui window if there was one this session, else the `[Window]` entry in `hydra_ui.ini`), and keeps it if `placement_on_screen` passes. Otherwise it copies the main viewport's position and size.

Both halves let an oversized window through. The first-open copy is the main window's *client* area; when the main window is maximized that is most of the screen (the user's saved 2,560 × 1,417 below), and Windows then adds the report's own frame around it (the Win32 backend calls `AdjustWindowRectEx` on the client rect, `third_party/imgui/backends/imgui_impl_win32.cpp` lines 1211-1212), so the outer window is a frame wider and taller than the screen. And `placement_on_screen` (lines 120-129) only asks whether a 64-pixel band of the title bar is reachable (`kMinVisiblePx`, `app_shell.h` line 70), so a saved rectangle that is too big passes. The user's `hydra_ui.ini` holds `ViewportPos=0,23 Size=2560,1417` for the path report, which is the 2,560 × 1,440 monitor minus a 23-pixel title band, so the saved size comes back every time.

Nothing maximizes a report today. The only maximize code is the main window's: `note_window_placement` reads `IsZoomed` (`main.cpp` line 112) and `ShowWindow(SW_SHOWMAXIMIZED)` restores it (line 254). ImGui's own `[Window]` entry keeps a report's position and size only. That matches the question reviewer's note.

### The design

**A pure placement function** in `app_shell`, unit-tested with no window:

```
struct FrameInsets { int left, top, right, bottom; };  // the OS frame around a client area
struct ReportPlacement { ScreenRect client; bool maximized; };
ReportPlacement report_placement(const std::optional<ScreenRect>& saved_client,
                                 bool saved_maximized,
                                 bool main_maximized,
                                 const ScreenRect& main_client,
                                 const std::vector<ScreenRect>& work_areas,
                                 const FrameInsets& frame);
```

`main_maximized` comes from `window_placement().maximized` (`app_shell.cpp` line 153), which `main.cpp`'s `note_window_placement` keeps current (line 107). Its rules, in order. If the main window is maximized, or the report was saved maximized, the answer is "maximized" and the client rectangle is the work area of the monitor the main window is on, shrunk by the frame (Windows sizes a maximized window itself; this rectangle is only the ImGui window's size for the first frame). Otherwise the candidate is the saved client rectangle when `placement_on_screen` passes, else the main window's client rectangle. The candidate grows by the frame insets to its outer rectangle, which is then fitted into the work area that holds its title bar: width and height capped to the area's, then the position moved so the outer rectangle is inside. The client rectangle inside that is the answer.

The frame insets come from `AdjustWindowRectExForDpi(WS_OVERLAPPEDWINDOW, WS_EX_APPWINDOW, dpi)`, the same style the backend gives a decorated viewport (`imgui_impl_win32.cpp` lines 1166-1176, 1189-1199). `main.cpp` measures them once per DPI and hands them to `app_shell` through `set_report_frame_insets`; the GUI test runner has no OS window and leaves them at zero. Every number in this function is in the units `ScreenRect` already uses.

**Maximizing.** ImGui makes the OS window in `UpdatePlatformWindows` after the frame is rendered (`third_party/imgui/imgui.cpp` lines 17762-17774), so on the frame the report opens there is no window yet. `place_report_window` therefore remembers "maximize when the window exists" per report name, and on a later frame, when `ImGui::FindWindowByName(name)->Viewport->PlatformHandleRaw` is set, calls `ShowWindow(hwnd, SW_MAXIMIZE)` once and clears the note. Windows then sends `WM_SIZE`, the backend sets `PlatformRequestResize` (`imgui_impl_win32.cpp` line 1449), and ImGui copies the new size into the window (`WindowSyncOwnedViewport`, `imgui.cpp` lines 17650-17654). No ImGui change is needed. In the test runner there are no platform windows (ImGui turns viewports off without a backend, `imgui.cpp` line 11842), so the handle is null and the step does nothing; the placement math is still tested.

**Saving.** A new `[Hydra][Window:pathreport]` and `[Hydra][Window:dmreport]` section in `hydra_ui.ini`, holding the same three lines the main window's `[Hydra][Window]` does (`Pos`, `Size`, `Maximized`, `format_window_placement`, `app_shell.cpp` line 131). `placement_read_open` (line 94) learns the new names. Each frame a report's OS window exists, `place_report_window` reads `IsZoomed` and, while the window is neither maximized nor minimized, `GetWindowRect(hwnd)`, as `note_window_placement` does for the main window (`main.cpp` lines 107-120). That keeps the last un-maximized rectangle while the window is maximized, and it is in screen coordinates. (`GetWindowPlacement(hwnd).rcNormalPosition` is the other way to get it. Win32 documents that rectangle as workspace coordinates for a window without `WS_EX_TOOLWINDOW`, which differ from screen coordinates when the taskbar is on the top or left. That is from memory of the Win32 documentation and was not checked here, which is why the plan follows the main window's code.) It calls `remember_report_placement(name, …)`, which marks the ini dirty only on a change, as `remember_window_placement` does (line 155). The saved rectangle is the OS window's outer one, as the main window's is. `report_placement` takes a client rectangle, so the saved outer rectangle has the frame insets taken off before the call. The `name` that keys the section is the part of the window title after `###` (`pathreport`, `dmreport`), because the title `place_report_window` receives is the full text ("Path report — Hydra###pathreport", `path_report_window.cpp` line 148). The un-maximized rectangle is what the next un-maximized open uses, through `report_placement`. ImGui keeps writing its own `[Window]` entry; `saved_rect` reads the `[Hydra]` one first and falls back to ImGui's for an ini from before this change. That fallback is the "saved size that no longer fits" case, and `report_placement` corrects it.

Both report windows share all of this; the comparison window's `[Window][dmreport]` entry has the same `ViewportPos=0,23`, so it has the same problem.

**Prior art checked.** imgui#9384 ("Maximizing/Restoring primary viewport causes other viewports to move") is about the *main* viewport's maximize moving secondary windows, through `TranslateWindowsInViewport`; it does not affect a report window, whose viewport is its own and never merges (`report_window_class` sets `NoAutoMerge`, `app_shell.cpp` line 364). imgui#9442 (GLFW, X11) is a window manager clamping a viewport's position while ImGui keeps the requested one; on Windows the `WM_MOVE`/`WM_SIZE` path above keeps them in step, and the plan relies on that path rather than on ImGui accepting our rectangle. No ImGui issue or discussion describes maximizing a secondary viewport's OS window from the app; the `ShowWindow` on the viewport's handle is the plain Win32 way and is what `main.cpp` already does for the main window.

---

## Part 3: tests

Executors run only their own tests (rule 1), by `-tc=` or `--test`, on the checked-in corpus in `testdata/input` (97 charts in the runner, `docs/agents/ui-testing.md`). Never the library (rule 8).

**Unit tests, the rule** (`tests/test_column_widths.cpp`, doctest, new): with a fake `width_of` of 10 pixels a character. A header wider than every cell wins. A chip's padding is added. A cut column shares leftover in proportion and stops at its header's width. When the minimums exceed the available width, the inner width is their sum. A table with no cut columns has inner width equal to the sum of its columns. Every expected number is a literal worked from the inputs by hand and written in the test's comment, never computed by the test.

**Unit tests, the placement** (added to `tests/test_app_shell.cpp`): using the user's numbers from the handoff as the worked example. Monitor 2,560 × 1,440 with a work area of `{0, 0, 2560, 1392}` (a 48-pixel taskbar, a stand-in), frame insets `{8, 31, 8, 8}` (what `AdjustWindowRectEx` gives `WS_OVERLAPPEDWINDOW` at 96 DPI, to be confirmed by the executor with one call and written into the test as a literal). Cases: the saved client `{0, 23, 2560, 1440}` is corrected to fit; a first open from a maximized main window comes back maximized; a first open from a normal main window copies it and fits; a saved client on a monitor that is gone falls back to the main window; a saved maximized report reopens maximized. The `[Hydra][Window:pathreport]` text round-trips through the existing `format_window_placement`/`parse_window_placement_line`.

**GUI tests, before and after.** Each is written and run red against the old code first, and the exact red line goes in the executor's report (preamble, "Tests").

- `report-window-chip-label` (`tests/ui/uitest_report_windows.cpp`, sample-row group): resize the path window to 900 × 600 with `ctx->WindowResize`, then check the frame text holds the Beyond tier's full label from `app::path_report_view::tier_label` (`src/app/path_report_view.cpp` line 58; the window code aliases the namespace as `view_rules`) for the samples' hit window (85 ms, `tests/report_samples.h` line 20). This is red today because a cut chip draws through `DrawList->AddText` (`text_ellipsized`, `widgets.h` line 217), which ImGui's text log never sees, while a full label goes through `TextUnformatted`, which it does. It is green after, when the chip has its full width and the table scrolls instead. The same test checks every numeric cell of the sample rows is in the log in full, and that `ImGui::TableFindByID` for `##pathtable` reports `InnerWindow->ScrollbarX` once the window is narrower than the inner width.
- `report-window-column-widths` (same file): with the window wide, each column's `WidthGiven` equals the rule's output for the sample rows, read by calling the production `place_columns` on the production `MeasuredWidths`, not recomputed.
- `report-window-fits-screen` (same file): load `[Window][###pathreport]\nViewportPos=0,23\nSize=2560,1417` with `ImGui::LoadIniSettingsFromMemory` before opening the report (`uitest_details.cpp` line 564 does the same for the library). The runner's work area is its 1,280 × 800 display (`uitest_harness.h` lines 37-38; `imgui_work_areas` falls back to the main viewport, `app_shell.cpp` line 337). Red today: the window's `SizeFull` is 2,560 × 1,417. Green after: it is at most 1,280 × 800 and inside it. A second case sets `remember_window_placement` to maximized and checks the window's size is the work area (zero insets in the runner).
- `library-column-widths` (`tests/ui/uitest_library.cpp`): after `scan_library`, each column's `WidthGiven` equals the rule's output for `app.library.rows()`; narrowing the library pane (drag the split, `library_share`) stops the Title column at its header's width and shows the horizontal scrollbar.
- The Dynamics and Stars tabs and the backend table each get one line in their existing tests: a column's width equals the rule's output for that table's rows.

The maximized OS window itself cannot be seen in the runner. The main session checks it in the app once, with `Hydra.exe` on the user's maximized window, and takes one screenshot for the review.

---

## Part 4: the build, split for parallel Opus executors

Two waves. No two tasks in a wave touch the same file. Each task's brief names its files, its tests, the status-line rule (`hooks\state\status`) and the derive-once reviewer that follows it.

**Wave 1** (two tasks, together):

- **Task 1, the rule.** Creates `src/ui/column_widths.h`, `src/ui/column_widths.cpp`, `tests/test_column_widths.cpp`, and adds them to `CMakeLists.txt`: the `.cpp` to the `hydra_ui` source list (lines 355-381), the test next to `tests/test_app_shell.cpp` (line 517). The interface is the one in Part 1, written out in the header so wave 2 codes against it. The cached measure records the scale it was taken at, so no other file changes for the scale trigger. Times the three measures of 40,000 synthetic rows with a real ImGui font (Part 1, "Cost") and reports the numbers. Runs `hydra_tests.exe -sf=test_column_widths.cpp`.
- **Task 2, the window.** `src/ui/app_shell.h`, `src/ui/app_shell.cpp`, `src/ui/main.cpp`, `tests/test_app_shell.cpp`. `report_placement`, the frame-insets seam, the per-report `[Hydra]` sections, the deferred `ShowWindow(SW_MAXIMIZE)`, the per-frame `IsZoomed` note. Runs `hydra_tests.exe -sf=test_app_shell.cpp`. Hands the GUI fit test to Task 3, since that test's file is Task 3's.

**Wave 2** (three tasks, together, each forked from wave 1's merge):

- **Task 3, the report tables.** `src/ui/report_window.h`, `src/ui/report_window.cpp`, `src/ui/path_report_window.cpp`, `src/ui/dm_report_window.cpp`, `tests/ui/uitest_report_windows.cpp`. Replaces the stretch columns, `row_number_width`, `measure_columns` and the chip's shrink with the rule; caches widths beside each window's view; writes and runs `report-window-chip-label`, `report-window-column-widths` and `report-window-fits-screen` red then green. Runs `hydra_uitest --test` for those three plus the existing `report-window-*` tests (they are its own change's tests).
- **Task 4, the Library.** `src/ui/library_table.cpp`, `src/ui/library_model.h`, `src/ui/library_model.cpp`, `src/ui/app_state.h` (the cache field in `LibraryViewState`), `tests/ui/uitest_library.cpp`. Adds `rows_version()`, replaces the weights, writes and runs `library-column-widths` red then green, plus the existing `library-*` tests.
- **Task 5, the tabs.** `src/ui/paths_tab.cpp`, `src/ui/dynamics_tab.cpp`, `src/ui/stars_tab.cpp`, `src/app/path_view.h`, `src/app/path_view.cpp` (removing `backend_table_id`), their tests under `tests/` (`tests/test_path_view.cpp` line 907 for `backend_table_id`; not `tests/test_single_owner.cpp`, see Part 1), and `tests/ui/uitest_details.cpp` and `uitest_paths.cpp` for the one-line width checks. Runs those tests.

**After wave 2**, the main session removes `base_table_flags` from `src/ui/widgets.h` at the merge (one deletion, once nothing references it), updates the `base_table_flags` and `backend_table_id` entries in `tests/test_single_owner.cpp` (Part 1, "The schemes this replaces"), and runs the full suite once (rule 1).

Each wave-2 task's reviewer reads the task's code and the plan's Part 1 and checks the rule is called, not copied: a grep for `CalcTextSize` inside the table files should find only the `width_of` the table hands the rule.

---

## What the handoff and the reviewer's note got wrong

- The handoff says the comparison report "most likely has the same problem" and that "nobody has checked its saved size yet". It does: the user's ini holds `[Window][dmreport]` with `ViewportPos=0,23` and `Size=2560,1417`, the same as the path report. It also shares `report_frame::table` and `place_report_window`, so the plan treats both the same, and the fit test on the path window covers the shared code.
- The handoff speaks of "every data column" being stretched and never mentions the "#" column. That column is not stretch; it is a fixed column whose width `row_number_width` measures, a second copy of the rule, so it goes through the rule too.
- The handoff suggests "a new ID so the old saved weights stop applying". Not needed: a width set after the table's first frame overwrites the saved one, because ImGui applies saved widths inside the first `TableUpdateLayout`. The plan uses that instead of new IDs. The user's acceptance of a one-time reset still stands; it just happens through the rule.
- The reviewer's note is right that nothing maximizes a report today and that ImGui's ini keeps only position and size. It did not say where the OS window appears (after the frame, in `UpdatePlatformWindows`), which is why the maximize has to wait a frame; the plan names it.

## The user's answers (2026-10-10)

1. Dragging: drags are temporary, as designed above. A drag holds until a re-measure gives that column a different width (or, on a cut column, until the next window resize), and never survives a restart.
2. The floor for a cut column is its header's width.
3. A slow full measure is accepted: widths stay exact, and Hydra pauses for that measure on a library load, a scan or a UI scale change. The near-exact "longest strings only" measure is not used. Task 1 still times it and reports the numbers.
4. The build is approved, in the two waves of Part 4.
