Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task T6: wire the windows into Hydra

Task id: RW-T6. Base: `1a0bdd8` (main, the wave-2 tip plus the wave-2 handoff). Branch `claude/rw-t6-wire`, worktree `.claude\worktrees\rw-t6-wire`; make it as the preamble says.

Your spec is the plan's "Task 6" section, plus the spec's "What the windows show" (states and keyboard) and "Testing" ("The windows"). Mock boards 1, 2 and 4. D103 items 7 to 11 and 19 to 22 bear on you. The wave-2 handoff, `docs/handoffs/2026-10-08-report-windows-wave2-handoff.md`, lists three extra jobs for T6; this brief carries them.

## What this brief adds

**What waves 1 and 2 left you.**
- AppState (T4) holds both reports as `ReportSlot`s, `path_report` and `dm_report`, with `path_report_build()`, `dm_report_build()`, `request_path_report`, `cancel_path_report`, `show_path_report`, `start_dm_report`, `request_dm_report`, `reopen_dm_picker`, `cancel_dm_report` and `select_chart`. Each slot has `window_open`, which T4's toolbar, strip and picker already set.
- The windows (T5) draw from `ReportWindowInput` in `src/ui/report_window.h`, with `ReportCallbacks`. `path_report_window.cpp` and `dm_report_window.cpp` build each window's input and view. A test-only hook opens them on sample rows.
- Nothing draws the windows from AppState yet. That's your job.

**1. Draw both windows from AppState.** `library_view.cpp` draws each window every frame while its slot's `window_open` is set. Build the `ReportWindowInput` from the slot and the job. The callbacks:
- row click: `select_chart(hyhash, chartmode)`. A "not in library" row has no click (T5 already leaves its callback empty).
- Refresh and Try again: `request_path_report` or `request_dm_report`.
- Cancel: `cancel_path_report`. The comparison's Cancel calls `cancel_dm_report` and then `reopen_dm_picker` (D103 item 10).
- "Compare another player...": `reopen_dm_picker`.
- close (Esc, Ctrl+W, the OS close box): clears `window_open`. The result stays in memory, so reopening is instant.

If the input-building is more than a few lines per report, put it in a function in `path_report_window.cpp` / `dm_report_window.cpp` that takes the slot and the job state, so `library_view.cpp` only calls it. Those files may include `app_state.h`; `report_window.{h,cpp}` still must not.

**2. One enum per idea.** AppState has `ReportStale { None, Library, Settings }` and `ReportBuild { None, Building, Ready, Cancelled, Failed }`. `report_window.h` has `ReportOutOfDate { None, Library, Settings }` and `ReportState { Building, Cancelled, Failed, Ready }`. These are two copies of each idea, and wiring them must not add a mapping between them. Keep one enum of each, in a small header both sides include (for example a new `src/ui/report_state.h`), and delete the other. Pick the names that read best at the call sites and say why in your report. If the window can be asked to draw "None" (never built), say what it draws and where that rule lives.

**3. The building progress (handoff item).** The path report's building state reads "Analyzing n of N charts". Today it can't, because `generate_report` and `ReportOptions` report no progress.
- Add a progress callback to `report::ReportOptions` that `generate_report` calls with charts done and charts total, the way its existing `cancel` flag is checked between charts. Test first in `tests/test_report.cpp`: a new case that runs `generate_report` on a small input and pins the calls (starts at 0 of N, ends at N of N, never goes back).
- `ReportJob` (in `library_jobs.{h,cpp}`) keeps the latest pair in atomics, the way `AnalyzeJob` keeps its `progress_`. AppState hands it to the window input's `progress_done` and `progress_total`.
- If a report built from a batch's seed reads no charts (so N is 0), say what the window shows then; don't invent a new sentence.

**4. The strip colours (handoff item).** `kDoneBg` and `kProblemBg` are private in `library_dialogs.cpp`. Move them into the theme, next to `chip_color`, keeping their exact values. The batch strip in `library_dialogs.cpp` reads them from there. The report windows' out-of-date strip uses the done colour and the left-out strip uses the problem colour: mock board 2b draws the out-of-date strip with the `done` class and board 2d draws the left-out strip with the `warn` class. If `report_window.cpp` already draws those strips with some other colour, replace it. T7 owns `tests/test_single_owner.cpp` this wave, so don't edit it. If a scan row should guard the move (for example, flag an `ImVec4` strip colour written outside the theme), write the row's text in your report and the main session adds it at the merge.

**5. The batch time (handoff item).** `ReportWindowInput::batch_finished` is drawn as "(a batch finished at HH:MM)" when the out-of-date reason is Library (T5, D103 item 21). Record the finish time where `update_background_jobs` marks the reports out of date after a batch, store it on the slot, and fill the input from it. One time formatter: T5 already writes "Built HH:MM" and the batch time with one function; reuse it.

**6. The end-to-end uitests.** Add these scripts to `tests/ui/uitest_report_windows.cpp`, named exactly so (T8 cites them in `docs/agents/ui-testing.md`):
- `report-window-open-path`: the path window opens from the toolbar's "Open path report", from the finished strip's "Open report", and by itself after a batch when "Open automatically" is on. Check the building state's "Analyzing n of N charts" while it builds if the harness can catch it; otherwise say so.
- `report-window-dm-handover`: picking a player closes the picker and opens the comparison window; its Cancel reopens the player list; "Compare another player..." reopens the box. Use the uitest's existing dm fakes; never reach the real server.
- `report-window-row-click`: a row click selects that chart in the library; a row of the other chart mode switches the settings bar first; a "not in library" row click does nothing.
- `report-window-reopen`: Esc closes the window; opening it again shows the same rows at once, with no new build.
- `report-window-out-of-date`: after a batch finishes, the strip shows "Your library changed since this report was built." with the batch time; changing a setting the report reads shows the settings sentence; Refresh rebuilds and clears the strip.

T4's scripts in `uitest_batch_reports.cpp` (`batch-done-strip`, `report-buttons`, `settings-and-reports`, `batch-open-failure`) check the open flags. Where a check would now be clearer as "the window shows", update it; don't duplicate a script.

**The theme contrast test.** If you move the two strip colours, add them to the theme test's contrast check only if D103 or the spec names a contrast rule for strips. They don't today, so don't add a threshold; list it as a question if you think one is needed.

**7. Comments that name `hydra_report`.** T7 deletes the `hydra_report` CLI this wave. Three comments in your files name it: `src/ui/library_jobs.cpp` lines 581 and 606, and `tests/test_library_jobs.cpp` line 345 (on the base). Reword each so it names `generate_report` or the GUI's report job instead.

## Shared files with T7 (same wave)

T7 deletes the HTML pages at the same time, so a few files are shared. Each of you touches a separate region; the main session merges T7 first and then yours, and resolves the overlap.
- `src/app/report.{h,cpp}`: you own `ReportOptions` and the chart loop in `generate_report` (the progress calls). T7 owns the page functions and `GeneratedReport::html`. Don't touch T7's region.
- `tests/test_report.cpp`: you add one new case at the end. T7 deletes the page cases.
- `tests/ui/uitest_batch_reports.cpp`: you edit scripts' checks. T7 removes the browser and folder recorder calls.
- `tests/ui/uitest_tests.cpp`: you add your registration lines. Add them as one block after T5's report-window lines.

## Owned files

`src/ui/library_view.cpp`, `src/ui/app_state.{h,cpp}`, `src/ui/library_jobs.{h,cpp}`, `src/ui/report_window.{h,cpp}`, `src/ui/path_report_window.cpp`, `src/ui/dm_report_window.cpp`, a new `src/ui/report_state.h` (or the name you choose for the shared enums), `src/ui/theme.{h,cpp}`, `src/ui/library_dialogs.cpp` (the two colours only), the regions of `src/app/report.{h,cpp}` named above, `tests/test_report.cpp` (one new case), `tests/test_app_state.cpp`, `tests/test_library_jobs.cpp`, `tests/ui/uitest_report_windows.cpp`, `tests/ui/uitest_batch_reports.cpp`, `tests/ui/uitest_tests.cpp`, and a `CMakeLists.txt` line only if a new source file needs one (a header-only file doesn't).

This is about 18 files. Plan for the 100-call wrap-up: commit after each numbered job above, in order, so a finisher can pick up at the next one.

## Preflight

Run by the orchestrator on main at `1a0bdd8` on 2026-10-08:
- `Grep "report|Report" src/ui/library_view.cpp`: only line 169-175, the dm job reaping. Nothing draws a report window.
- `ReportSlot` is at `app_state.h:170`, with `ReportStale` at 159 and `ReportBuild` at 163. `ReportState` and `ReportOutOfDate` are at `report_window.h:35` and 38.
- `ReportOptions` is at `report.h:186`, with `cancel` at 196 and no progress member. `ReportJob` is at `library_jobs.h:422`.
- `kDoneBg` and `kProblemBg` are at `library_dialogs.cpp:43` and 45; the batch strip reads them at 444.
- No batch finish time is stored anywhere in AppState; `built_at` is set at `app_state.cpp:727`.
- Full suite on `1a0bdd8`: 1,223 cases pass (3 skipped), every `hydra_uitest --all` script passes.

## Tests

`-sf=*test_report.cpp*` (your new case, then the file once), `-sf=*test_app_state*`, `-sf=*test_library_jobs*`, the theme test if you touch it, and `hydra_uitest` on every script in `uitest_report_windows.cpp` and `uitest_batch_reports.cpp` by name. Never `--all`.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.

## T6c: the user's answers to T6's three questions (2026-10-08)

A follow-up agent (task id RW-T6c) applies these in T6's worktree, on top of T6's signed-off tip. They are D103 items 24 to 26.

1. **Analysis off (item 24).** With a bad `hydra_rules.ini` and no report in memory, the path window shows the toolbar's sentence, "hydra_rules.ini has an error, so analysis is off until the file is fixed and Hydra is restarted.", and `app.rules_error` under it, where today it draws the header alone. The sentence is a literal in `library_toolbar.cpp:142-144`; give it one owner (a named constant beside `analysis_blocked` in AppState, or in the toolbar's header) that both the toolbar and the window read. The button stays (item 20). Owned for this: `src/ui/library_toolbar.cpp` (that sentence only), plus the T6 files.
2. **The bar's word (item 25).** "Analyzing n of N charts" becomes "Analyzing n of N records". The count stays one per chart and mode still to analyze. Update every place the old string is pinned (report-window-states, report-window-open-path, the docs' ui-testing line if it quotes it; the docs line is the main session's).
3. **Nothing to analyze (item 26).** When the progress total is 0, the bar moves with no count, the way the comparison's building bar does, under the building subtitle. Reuse the comparison's moving-bar code; don't write a second one. Pin it in a uitest (a seed that covers every chart) or a window-input test.

Tests: the touched uitest scripts by name, and `-sf=*test_report.cpp*` if the progress case changes.
