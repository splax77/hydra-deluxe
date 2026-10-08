Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task T4: app state and jobs

Task id: RW-T4. Base: the wave-1 tip your prompt gives. Branch `claude/rw-t4-state`, worktree `.claude\worktrees\rw-t4-state`; make it as the preamble says.

Your spec is the plan's "Task 4" section, plus the spec's "How the windows work" (entry points) and "How the code is arranged" (row click, the jobs). D103 items 3, 8 to 11 and 13 are yours. This brief adds only what the plan leaves to dispatch time.

## What this brief adds

**Wave 1 is in.** `GeneratedReport` now carries `paths` (the rows), `subtitle`, `footer` and `hit_window_ms`. `GeneratedDmReport` carries `rows`, `subtitle`, `footer`, `username` and `chartmode` (T2). Keep each finished result as a `shared_ptr` to a const result in AppState. Don't copy rows out of it.

**Where the old plumbing is.** `ReportJob` calls `publish_report` at `src/ui/library_jobs.cpp:609` (class at `library_jobs.h:425`, `open_when_done` at 433 and 461). `DmReportJob` calls it at `src/ui/dm_jobs.cpp:58` (class at `dm_jobs.h:48`, `open_when_done` at 53 and 75). "Open path report" is gated on `app::report_file_exists` at `src/ui/app_state.cpp:467`; the button is at `library_toolbar.cpp:128-137`. The batch-done strip's "Open report", "Show in folder" and "Open automatically" are at `library_dialogs.cpp:453-505`. The dm box's last step ("Open report again", "Compare another") is at `library_dialogs.cpp:574-582`.

**What stays for T7.** Stop calling `publish_report`, `show_in_folder`, `report_file_exists` and the two `open_*_in_browser` functions, but don't delete them or `report_outcome.h`; T7 does that once `hydra_report` goes. If removing a call leaves an include unused in a file you own, drop the include.

**The out-of-date reason.** It's "none", "library" or "settings". It's set to "library" when a batch finishes after the report was built, and to "settings" when the committed settings change after the build. The two sentences the window shows for them are in the spec's "New words" list; T5 owns the text. You own only the enum and when it changes. If both happen, keep whichever happened last.

**Request functions.** Each report gets one function that starts its job when none is running, and does nothing when one is. A window's Refresh, Try again, and open-with-nothing-in-memory all call it. The comparison's needs the player's name: Refresh reuses the last player.

**`select_chart(hyhash, chartmode)`** (D103 item 3). Switch the settings bar to `chartmode` first when it differs, through the same path a user's change takes. Then find the chart's library entry by hash and call the existing `select`. With several copies, pick the first in the library's current order. Use the existing hash normaliser (`normalize_chart_hash`); don't write a second.

**Words.** The new strings T4 shows are "The path report is ready." and the hint "Open the path report as soon as it's built.", both from the spec. Nothing else new.

**Tests that read `opened_urls`.** `uitest_batch_reports.cpp`'s `batch-done-strip`, `report-buttons`, `settings-and-reports` and `batch-open-failure` check the browser recorder today. Make them check the open flags instead. Leave the recorders themselves in `uitest_harness.cpp`; T7 removes them. If a script only makes sense with a browser (for example a test of a failed open), say in your report what you did with it.

## Owned files

`src/ui/app_state.h`, `src/ui/app_state.cpp`, `src/ui/library_jobs.h`, `src/ui/library_jobs.cpp`, `src/ui/dm_jobs.h`, `src/ui/dm_jobs.cpp`, `src/ui/library_toolbar.cpp`, `src/ui/library_dialogs.cpp`, `tests/test_app_state.cpp`, `tests/test_library_jobs.cpp`, `tests/ui/uitest_batch_reports.cpp`.

Owned-file check: every T4 step changes app state, the two jobs, the toolbar, the strip and the dm box, and their tests; no window is drawn (T5) and nothing is deleted from the app layer (T7).

## Review fix round (added by the orchestrator, 2026-10-08)

`select_chart` searches the Difficulty, Pro Drums and 2x Bass choices for the one whose `chartmode_key` spells a mode. `settings_for_mode` in `src/app/report.cpp` already answers that question, privately. For the fix round only, you may edit `src/app/report.h` and `src/app/report.cpp` to make `settings_for_mode` public (or move it beside `chartmode_key` if that's its natural owner), and have `select_chart` call it. Add the scan row the fix-round rules ask for. T5 also adds one function to `report.h` this wave; touch only `settings_for_mode`'s lines so the merge stays mechanical.

The review's finding 3 is answered: D103 item 22 keeps T4's rule (a report goes out of date only when a setting it reads changes). Give that rule one owner beside the report builders: one function per report, in `report.h` and `dm_report.h`, saying whether a settings change touches what it reads. `apply_settings` calls them and its comment names them instead of restating the rule. For this, the fix round may also edit `src/app/dm_report.h` and `src/app/dm_report.cpp`.

## Preflight

Command: `Grep "publish_report|open_when_done|report_file_exists|class ReportJob|class DmReportJob|show_in_folder|Open path report|Show in folder|Open automatically|Open report|Compare another" src/ui`, run by the orchestrator on main at e0507e1 on 2026-10-08.
Output: `publish_report` at `library_jobs.cpp:609` and `dm_jobs.cpp:58`; `class ReportJob` at `library_jobs.h:425`; `class DmReportJob` at `dm_jobs.h:48`; `report_file_exists` at `app_state.cpp:467`; "Open path report" at `library_toolbar.cpp:128,137`; "Open report" and "Show in folder" at `library_dialogs.cpp:453-467`; "Open automatically" at 505; the dm box's "Open report again" and "Compare another" at 574 and 582; `report_outcome_shown` at `app_state.h:422`. Wave 1's join (J1) touched only `src/app` and tests, so these lines hold on your base.

## Tests

The cases you touch in `-sf=*test_app_state*` and `-sf=*test_library_jobs*`, and the uitest scripts `batch-done-strip`, `report-buttons`, `settings-and-reports`, `batch-open-failure`, `dm-compare-flow` and any other dm picker script (grep `tests/ui` for "Compare dmleaderboards user"), by name.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
