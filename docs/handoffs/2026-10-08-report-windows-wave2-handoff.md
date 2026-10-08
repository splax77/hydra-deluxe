# Report windows: waves 1 and 2 merged, wave 3 not started (handoff, 2026-10-08)

The path report and the dmleaderboards comparison are moving out of the browser into their own Hydra windows. Waves 1 and 2 of the plan are on `main` at `bf270da`. The windows exist and are tested on sample rows, but nothing in the app opens them yet: that wiring is wave 3. The user asked to stop here and hand off.

## First thing the next session does

Build `main` and run the full suite once, before anything else. T4 and T5 were developed in parallel and merged with no text conflicts, but the combined tree hasn't been built. Two same-wave tasks can merge cleanly and still fail to compile together (it happened in the 2026-09-26 waves). Run `tools\build_slot.ps1 -Target hydra_tests`, then `build_cpp.ps1 -Target hydra_uitest`, then `hydra_tests.exe` and `hydra_uitest.exe --all`. Wave 1's tip (`742ea1e`) passed: 1,212 cases and every uitest script. If the combined build breaks, fix the join in a small commit with a derive-once review, then go on.

## Where everything is

The spec is `docs/superpowers/specs/2026-10-08-path-report-window-design.md`. The plan is `docs/superpowers/plans/2026-10-08-path-report-window.md`. The user's decisions are D103 items 1 to 22 in `docs/audit/2026-10-03-fix-decisions.md`. Items 14 to 22 were added this session; they are summarised below. The task briefs are in `docs/superpowers/plans/tasks/`, each starting from `_rw-preamble.md`. Wave 3's briefs (T6, T7, T8) are not written yet.

## What's on main

Each piece below went through one derive-once review and was signed off CLEAN before merging.

- **T1 and T1b, the OS windows** (merged `e0507e1`). Multi-viewports are on, and only the report windows' shared class gets its own OS window, owned by the main window. The main loop keeps drawing while a report window shows. A report window first opens at the main window's rectangle (D103 item 14), and a saved spot that's off every monitor falls back to that. Tooltips, dropdowns and popups stay inside the window that opened them (D103 item 15). ImGui has no setting for this, so it's one marked `// Hydra:` block in `third_party/imgui/imgui.cpp`.
- **The hands-on check passed** before T1 merged. The screenshots are in `docs/handoffs/2026-10-08-spike-check/`. The check covered the second monitor, minimize and restore, drawing while Hydra was covered (5.7% CPU), placement kept across a restart, the off-screen fallback, and a dropdown and a tooltip in a small main window. One old quirk showed up: a long help tooltip in a very narrow window is cut off at the right edge. It did that before viewports too, and it was left alone.
- **T2, the data layer** (merged `22b250c`). The result structs carry the rows, subtitle and footer. `path_tiles` and `dm_tiles` take the window's visible row indices, and `status_token` maps a status to a `ChipToken`. `report::kDash` is the one em dash.
- **T3 and J1, the table view and columns** (merged `742ea1e`). This is `app/report_view.h` (`TableView`), plus `path_report_view` and `dm_report_view`. J1 fixed T3's review findings and applied D103 items 16 to 18. `beyond_edge_text`, `beyond_tier` and `none_tier` are public in `report.h`. The seven `kStatus*` words and `kPercentDecimals` are in `dm_report.h`. `count_line` takes a `CountNoun`.
- **T4 and T4c, app state and jobs** (merged `d3d5357`).
  - Both results live in AppState as `ReportSlot`s (`path_report`, `dm_report`). Nothing writes or opens a page any more.
  - New functions: `request_path_report`, `cancel_path_report`, `show_path_report`, `request_dm_report`, `reopen_dm_picker` and `select_chart`.
  - The out-of-date rule has one owner per report, `report::settings_change_touches` and `dm_report::settings_change_touches` (D103 item 22).
  - `Settings::with_chartmode` is the one search from a chart mode to the settings-bar choices.
  - `AppState::kBatchRunningStatus` owns "A batch is running.".
- **T5 and T5b, the window frame and both windows** (merged `bf270da`).
  - `ui/report_window.{h,cpp}` holds `ReportWindowInput` (the seam T6 fills), `report_frame` and the table. The thin per-report files are `path_report_window.cpp` and `dm_report_window.cpp`.
  - `chip_color` in the theme.
  - `keep_table_column_order`, `base_table_flags` and `move_to_right_edge` in `widgets.h`, shared with the library table.
  - D103 item 21: the building subtitle and "(a batch finished at HH:MM)".
  - The spike is deleted.
  - The sample rows live in `tests/report_samples.{h,cpp}`.
  - The uitest scripts are `report-window-sort`, `-filters`, `-states`, `-keys` and `report-windows-both`.

## Decisions the user made this session (D103 items 14 to 22)

- **14.** A report window first opens at the main window's rectangle. Its minimum size is measured from the content.
- **15.** Popups and tooltips stay inside the window that opened them.
- **16.** Text columns sort like the Library table.
- **17.** An empty Posted date sinks to the bottom.
- **18.** The count line uses `counted` ("1 of 1 path").
- **19.** A row click of another mode while a batch locks the bar does nothing, and says "A batch is running.".
- **20.** "Open path report" also shows while a report is loaded or building.
- **21.** The mock's building subtitle and batch time are drawn.
- **22.** A report goes out of date only when a setting it reads changes. The path report reads the SP cap and the lens. The comparison reads its mode and the lens.

One more decision was recorded in the spec, not D103. Report search keeps D56 item 1: it folds accents and matches words only, with no query language.

## What wave 3 has to do

The plan's T6, T7 and T8 still stand. These additions came up during waves 1 and 2, and the briefs should carry them.

**T6, wire the windows.** `library_view.cpp` draws both windows from AppState each frame and connects the callbacks, with the end-to-end uitests the plan lists. It also has three extra jobs:

- **The building progress.** The path report's building state should read "Analyzing n of N charts". Today it can't, because `generate_report` and `ReportOptions` report no progress. Add a progress callback in `report.{h,cpp}`, then pass it through `ReportJob` and AppState into `ReportWindowInput`.
- **The strip colours.** `kDoneBg` and `kProblemBg` are private in `library_dialogs.cpp`. Move them into the theme, and use them for both the batch strip and the report windows' out-of-date and left-out strips (mock boards 2b and 2d).
- **The batch time.** Fill `ReportWindowInput::batch_finished` from AppState with the finish time of the batch that marked the library out of date.

**T7, delete the old pages.** Delete the pages, the GUI file plumbing and `hydra_report`, per the spec's "What gets deleted". Before deleting from `html_page`, move two functions: `html::search_field`, which `path_search_text` and `dm_search_text` call, and `html::replace_all`, which `dm_report_view` calls. Every wave-1 and wave-2 report lists the JavaScript copies its C++ replaced, which is T7's deletion list. `report_outcome.h`, `publish_report`, `show_in_folder`, `report_file_exists`, the `open_*_in_browser` pair, the result structs' `html` field and the uitest browser recorders are all now unused by the GUI. The scan row for the comparison's status words skips HTML `<option value=...>` lines only because the dm page still exists; tighten it once the page is gone.

**T8, the docs.** Write ADR 0027 and the user-guide changes. The ADR should cover two things. The first is the `imgui.cpp` patch for D103 item 15, and why there's no setting for it: GitHub issue #4624 asked and was closed without an answer. The second is that `hydra_ui.ini` now carries a `ViewportPos` line per report window. The docs should name D103 items 14 to 22.

**Follow-ups outside wave 3.**

- Four older right-align copies are listed in `known_copies` in `tests/test_single_owner.cpp`: `paths_tab.cpp:87`, `library_dialogs.cpp:402`, `library_toolbar.cpp:132` and `settings_bar.cpp:252`. They should call `move_to_right_edge`.
- `kMaxSortKeys`' comment in `report_view.h` lists its readers, which can go stale.
- If a hit-window control is ever added, the path report's out-of-date rule needs the hit window too. It's one line in `report::settings_change_touches`.

## Worktrees left on disk

These are all merged into `main` and can be removed once the next session no longer needs them. They are `.claude\worktrees\rw-t1-viewport`, `rw-t2-data`, `rw-t3-view`, `rw-w1-join`, `rw-t4-state` and `rw-t5-windows`. The `rw-t1-viewport` build folder also has a test `hydra.db` and `hydra_ui.ini` from the hands-on check.

## The user's installed Hydra

The user's installed Hydra (`C:\Program Files\Hydra\Hydra.exe`) was running when the hands-on check started, and wasn't running afterwards. Every window action in the check went by the test process's own id, so the check shouldn't have touched it. When it closed is unknown.

## Agents from this session

Journal state at the handoff, as the handoff hook reported it (all workflows completed, nothing in flight):

```
wf_102907bf-31c: started=6 result=6   report-windows-wave2   completed
wf_69c99480-c86: started=2 result=2   report-windows-join1   completed
wf_739c5a01-5fd: started=2 result=2   report-windows-t1b     completed
wf_75c8b4ba-6e4: started=8 result=8   report-windows-wave1   completed
wf_e43f9137-37a: started=4 result=4   report-windows-t5b     completed (sign-off CLEAN on bcc7b19, merged as bf270da)
wf_e94b2652-899: started=2 result=2   report-windows-t4c     completed
```

The hook showed `wf_e43f9137-37a` at 3 of 4 results. Its fourth result, T5's CLEAN sign-off, arrived just after, and T5 was then merged.
