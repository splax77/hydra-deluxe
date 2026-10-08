Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task T5: the window frame and both windows

Task id: RW-T5. Base: the wave-1 tip your prompt gives. Branch `claude/rw-t5-windows`, worktree `.claude\worktrees\rw-t5-windows`; make it as the preamble says.

Your spec is the plan's "Task 5" section, plus the spec's "What the windows show" (all of it) and mock boards 1, 2, 4 and 5. D103 items 1, 4, 12 and 14 to 18 bear on you. This brief adds only what the plan leaves to dispatch time.

## What this brief adds

**What wave 1 left you.** All in `src/app`, plain C++:
- `report_view.h`: `TableView<Row>` (search, keep-rule, two-key sort, `visible()`, `count_line`), `first_direction`, `Column`, `CellLook` and `Tone`. Draw from these; never sort, filter, count or word a cell yourself.
- `path_report_view.h`: `path_columns(hit_window_ms)`, `path_first_sort`, `timing_choices`, `path_keep`, `path_search_text`, the noun.
- `dm_report_view.h`: `dm_columns`, `dm_first_sort`, `status_choices`, `dm_keep`, `dm_search_text`, the noun.
- `report.h`: `path_tiles(rows, shown, hit_window_ms)`, `ChipToken`, `kDash`, `beyond_edge_text`. `dm_report.h`: `dm_tiles(rows, shown)`, `status_token`.
- The result structs carry the rows, subtitle and footer.
Read the J1 merge's report (in your prompt) for any name that changed at the join.

**What T1 left you.** `report_window_class()` and `place_report_window(name)` in `src/ui/app_shell.h`. A first open takes the main window's rectangle (D103 item 14). Also the "Report spike" window, its `--report-spike` switch in `main.cpp`, its setter in `app_shell.h` and `tests/ui/uitest_report_spike.cpp`. Delete all four, with the spike's lines in `uitest_tests.cpp` and `CMakeLists.txt`.

**Minimum size (D103 item 14).** Each window's minimum size is where the controls row and the first five columns still fit, measured from the content each frame (`SetNextWindowSizeConstraints`). No fixed pixel numbers.

**One title bar.** In its own OS window a report window shows Windows' title bar and ImGui's own. Hide ImGui's (`ImGuiWindowFlags_NoTitleBar`). The OS title is the spec's "Path report — Hydra" or "dmleaderboards: <player> — Hydra". Check how the Win32 backend names the OS window (from the ImGui window name before `###`) and use a `###` id so the title can change without losing the saved placement.

**Popups stay inside (D103 item 15).** T1b's marked block in `third_party/imgui/imgui.cpp` already keeps them inside; add nothing for it.

**Chip colours.** `chip_color(ChipToken)` goes in `theme.{h,cpp}`. It holds the seven tier colours from `src/app/html_page.cpp:146-147` (`--t0` to `--t5`, `--tn`) and the dim colour, written once there. The path row's tier needs a `ChipToken`; add that one mapping beside `ChipToken` in `report.h`, reading the tier, not re-deriving it. The contrast test checks each colour at 3:1 against Hydra's window background and the chip text at 4.5:1. Find the background in `theme.cpp`; the spec says #252526, so confirm it. If a colour fails, stop and report it; don't pick a colour.

**The table** uses the library table's flags (`src/ui/library_table.cpp:232-237`), its imgui#9519 workaround (line 259) and its list clipper (line 320). Call shared code where `library_table.cpp` already has it; if a piece you need is private there and you'd have to copy it, stop and report which.

**Sample rows for the uitest.** `sample_path_rows()` and `sample_dm_rows()` live in `tests/test_report.cpp` (T2 put them there). The uitest needs the same rows. Move them to a new `tests/report_samples.{h,cpp}`, linked into both `hydra_tests` and `hydra_uitest_harness`, and make `test_report.cpp` call them. Don't copy them.

**Uitest helpers.** `window_named` is in `uitest_paths.cpp:126`, and the spike test copied it as `window_shown`. Give the harness one helper (in `uitest_harness.h`) and use it in your file; `uitest_paths.cpp` may switch to it.

**The input struct is the seam.** `ReportWindowInput` carries everything AppState will supply in T6: result pointer, state, built time, out-of-date reason, progress, failure text, and the callbacks (row click, Refresh, Cancel, Try again, Compare another player, close). Your test hook builds it from the sample rows. Don't include `app_state.h`.

**Words.** Every string is the spec's or today's page's. Nothing else new.

## Owned files

New `src/ui/report_window.h`, `src/ui/report_window.cpp`, `src/ui/path_report_window.cpp`, `src/ui/dm_report_window.cpp`, `tests/ui/uitest_report_windows.cpp`, `tests/report_samples.h`, `tests/report_samples.cpp`. Also `src/ui/theme.h`, `src/ui/theme.cpp`, the tier-to-`ChipToken` mapping in `src/app/report.h` (and `report.cpp` if it needs a body), `tests/test_report.cpp` (only to call the moved samples), `tests/ui/uitest_harness.h` (and `.cpp`, if needed, for the one window helper only), `tests/ui/uitest_paths.cpp` (only to use that helper), the spike's lines in `src/ui/app_shell.{h,cpp}` and `src/ui/main.cpp`, deleting `tests/ui/uitest_report_spike.cpp` and its lines in `tests/ui/uitest_tests.cpp`, a theme test file if one exists (`tests/test_theme*.cpp`) or a new one, and your lines in `CMakeLists.txt`.

Owned-file check: every T5 step draws from a plain struct and wave 1's views; nothing reads or changes AppState (T4 owns it this wave), and `library_view.cpp` (T6) is untouched.

## Orchestrator answers to the first agent's questions (2026-10-08)

1. **The imgui#9519 workaround** moves into one shared helper in `src/ui/widgets.h` (for example `keep_table_column_order()`). `library_table.cpp`'s `render_table` calls it in place of its own lines 258-270, and `report_frame::table` calls it right after its `TableSetupColumn` loop. For this, the finisher also owns `src/ui/widgets.h` and those lines of `src/ui/library_table.cpp`. Add a scan row that would flag the old inline reset if it came back. Rerun the five report-window uitests and `library-column-order` by name.
2. **The strip background colours** (`kDoneBg`, `kProblemBg` in `library_dialogs.cpp`) wait for wave 3: T4 owns `library_dialogs.cpp` this wave. T6 moves them into the theme and uses them in both places. Leave the strips as they are now.
3. **The two mock sentences** not in the spec's word list wait for the user; don't add them.

## Preflight

Command: `Grep "9519|ImGuiTableFlags_|ScrollFreeze|ListClipper" src/ui/library_table.cpp`, `Grep "window_named|window_shown" tests/ui`, `Grep "sample_path_rows|sample_dm_rows" tests`, `Grep "report_window_class|place_report_window|report_spike" src/ui`, run by the orchestrator on main at e0507e1 on 2026-10-08.
Output: table flags at `library_table.cpp:232-234`, `TableSetupScrollFreeze(0, 1)` at 237, the #9519 note at 259, `ImGuiListClipper` at 320; `window_named` at `uitest_paths.cpp:126`, `window_shown` at `uitest_report_spike.cpp:17`; `sample_path_rows` at `test_report.cpp:1059`, `sample_dm_rows` at 1100 (J1 may shift these lines); `report_window_class` and `place_report_window` declared in `app_shell.h`, defined in `app_shell.cpp`, with the spike window drawn from `run_frame`.

## Tests

`hydra_uitest` on `uitest_report_windows.cpp`'s scripts by name, the contrast test, `-sf=*test_report.cpp*` once (for the moved samples), and the `uitest_paths.cpp` scripts by name if you change that file.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
