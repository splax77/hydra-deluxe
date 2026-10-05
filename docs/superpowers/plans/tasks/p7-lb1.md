Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task LB1: the batch's own counts, skip set and settings (findings 142, 125 owner, 124 owner, 140 Settings type and ReportJob, R7.2 owner)

Task id: LB1. Base: main after M7-2 and M6-J2 (the main session names the hash at launch). **Forks from RP's commit**, not from main, because finding 140's report half (ReportOptions becomes a double) is RP's. Branch: claude/p7-lb1 (worktree `.claude\worktrees\p7-lb1`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 3 table, "LB Library and batch". The plan's one LB task is split in two because it would change about 620 lines (D59 item 4): LB1 is the app, job, CLI and settings half; LB2 (`p7-lb2.md`) is the app-state, library screen and toolbar half and forks from LB1's commit. No file is in both. Decisions: D51 calls 10, 15, 23 and 26 in `docs/audit/2026-10-03-fix-decisions.md` (questions 10, 15, 23 and 26 of `docs/audit/2026-10-04-phase-7-questions.md`); the D51 addendum's SE1 note on `depth_mode`. Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 142.`, `#### 125.`, `#### 124.`, `#### 140.` and `#### R7.2`.

## Goal

Four questions get one owner each in the batch pipeline: how many charts a batch analyzed, skipped and failed (D51 call 26); which charts a batch skips and how many distinct charts it runs (call 10); which settings a finished batch filed its results under; and what the hit window is, with its decimal kept (call 15). The owners are `run_batch` and its helpers in `src/app/analysis.cpp`, `BatchJob` in `src/ui/library_jobs.cpp`, and `Settings` in `src/app/config.cpp`. LB2 points the confirm dialog, the strip and the report start at them. No score, path or stored record changes; the results stamp stays "2.1.0".

## What the code does today

Line numbers are from main; `claude/p7-st2` and `claude/p6-j2-4` move them a little.

**142.** `BatchProgress` (`analysis.h` line 136) carries only `completed`, `total` and `current_title`. Every caller rebuilds the other numbers. `BatchJob::run` (`library_jobs.cpp` line 281) sets `skipped` as `items_.size() - p.total`; `src/cli/batch.cpp` (line 228) does the same with `scanitems.size()`, and counts `analyzed` and `failed` in its own `on_result` and `on_error`. `batch_counts` and the Stop tooltip in `library_dialogs.cpp` (lines 56 and 420) print `completed - failed`. The job bumps `snap_.failed` in `on_error` and `snap_.completed` in `on_progress`, so a frame between the two reads "analyzed" one too low. On the base, ST2's `run_batch` also runs a chart found in two folders once, so both `size() - total` lines now count a second copy as "skipped (already had a result)", which it is not.

**125.** `run_batch` (`analysis.cpp`, on `claude/p7-st2` line 544) asks `store.analyzed_hashes` and builds its to-do list by hand (dedupe by md5, skip the analyzed). `AppState::open_batch_confirm` counts from the cached library chips instead; that caller is LB2's. Nothing in `analysis.h` can be asked "which charts will this batch skip, and how many will run".

**124.** `BatchJob` holds its `BatchRun` as a private `run_` (`library_jobs.h` line 166) with no accessor, so `update_background_jobs` (LB2) has to read the live settings for the report's cap and lens.

**140.** `Settings::hit_window_ms` (`config.h` line 81) is an `int` set to `static_cast<int>(kDefaultHitWindowMs)`. SE1's key table (`config.cpp` line 125) handles it through the `int Settings::*` kind, and `read_value` parses it with `atoi`, so `hit_window_ms=85.5` in the file loads as 85. `ReportJob` (`library_jobs.h` line 242) takes an `int` with the same cast as its default and stores `int hit_window_ms_`. `paths_tab.cpp` (J3-3's, not yours) casts the setting to a double before use; that cast is harmless on a double and stays. `report.h`'s copy is RP's.

**R7.2.** `Settings::auto_open_report` (`config.h` line 101) has no comment saying what it covers. Both `ReportJob` and `DmReportJob` read it. The hint in `render_batch_done` and the guide line are LB2's and DOC's.

## What changes

**142 and 125, the owner.** `BatchProgress` gains `analyzed`, `skipped` and `failed`; `completed` stays `analyzed + failed`, `total` stays the number of charts to run. `analysis.h` gets two functions beside `run_batch`: `charts_with_result(store, run, redo)` returns the md5 set the run skips (empty when `redo`; otherwise `analyzed_hashes` under the run's chartmode, cap and lens), and `plan_batch(items, already)` turns a `ScanItem` list into the to-do list of distinct charts (first copy wins, D51 call 10) plus the skipped count (charts in `already`, counted once each). `run_batch` calls both and fills every `BatchProgress` field itself, so the counts are made once and arrive together. `BatchRun` gains `cap_query()`, the one spelling of the cap filter the run files under; `run_batch`'s `RecordKey` uses it. Comments name the owner (D59 item 2); they do not restate the rule.

**142, the readers here.** `BatchJob::Snapshot` gains `analyzed`, and `on_progress` copies `total`, `completed`, `analyzed`, `skipped` and `failed` from the progress in one place; `on_error` only appends the two failure strings. The `items_.size() - p.total` line goes. In `cli/batch.cpp` the closing "Analyzed %d, skipped %d already stored, %d failed" line reads the last progress's three numbers, and the `scanitems.size() - p.total` line goes. The per-line `[n/total]` counter stays as it is: it numbers output lines as they print, and the callbacks that print them fire before the progress does (see open question 4).

**124.** `BatchJob` exposes `batch_run()` (a `const app::BatchRun&`; `run()` is already its private thread body). `ReportJob` exposes `cap()` and `lens()` so LB2's test can read what a report was built from.

**140.** `Settings::hit_window_ms` becomes a `double` equal to `kDefaultHitWindowMs`. The key table gains a double kind: `Key::member` takes `double Settings::*`, `number` gets an overload with double edges, `pull_into_range` works for both, `read_value` parses the double (junk reads 0 and then takes the range, as `atoi` made it), and `save_file` writes it through the stream, so 85 still saves as `hit_window_ms=85` and 85.5 as `hit_window_ms=85.5`. `Settings::clamp(int Settings::*, int)` is unchanged; no box edits the hit window. `ReportJob` takes and stores a `double`, default `kDefaultHitWindowMs` with no cast. `static_cast<int>(kDefaultHitWindowMs)` then appears nowhere in `src` (RP removed `report.h`'s; if it is still there at your fork, stop and report).

**R7.2.** The comment on `auto_open_report` says it covers every report Hydra builds, the batch's path report and the leaderboard comparison alike (D51 call 23), and that `ReportJob` and `DmReportJob` both read it. Behaviour is unchanged.

## Owned files (only these may change)

- `src/app/analysis.h`, `src/app/analysis.cpp` (`BatchProgress`, `BatchRun::cap_query`, `charts_with_result`, `plan_batch`, `run_batch`)
- `src/ui/library_jobs.h`, `src/ui/library_jobs.cpp`
- `src/cli/batch.cpp`
- `src/app/config.h`, `src/app/config.cpp`
- `tests/test_analysis.cpp`, `tests/test_library_jobs.cpp`, `tests/test_config.cpp`
- `tests/test_single_owner.cpp`: your own scan rows at the end of the file only; the main session joins them at M7-3.

Not yours: `src/ui/app_state.*`, `library_dialogs.cpp`, `library_toolbar.cpp` and the rest of `src/ui/library_*` (LB2); `src/app/report.*`, `src/cli/report.cpp` (RP); `src/ui/paths_tab.cpp` (phase 6 J3-3); `src/store/*` (J3-6); `tests/test_cli.cpp` (run only).

Notes on the base. `analysis.h/.cpp` and `test_analysis.cpp` changed on `claude/p7-st2` (the md5 dedupe in `run_batch`, the case `run_batch analyzes a chart found in two folders once`). `library_jobs.h/.cpp` and `test_library_jobs.cpp` changed on `claude/p6-j2-4` at 1019d98 (`scan_item_of`, `fail(e)`, the search-string constructor deleted, `BatchJob::run` now converts entries through `scan_item_of`). `cli/batch.cpp`, `config.h/.cpp` and `test_config.cpp` changed on `claude/p6-j2-1` (`describe_settings`, `resource_dir`; the hit-window key line is main's). `describe_settings` takes an `AnalysisSettings`, which has no hit window, so hydra_batch's header bytes do not move. Build against the merged base, not any one branch.

## Test cases to add or re-pin

Write each red first, then green. Use the files' own helpers (`test_run`, `fake_charts`, the counting analyzers the cancel cases use, `temp_db`, the INI round-trip helpers in `test_config.cpp`); no new helper.

In `tests/test_analysis.cpp`:
1. `plan_batch: a second copy is one chart and a stored chart is skipped` (new). Four `ScanItem`s: md5 A twice under two notespaths, md5 B once, md5 C once, with `already` holding C. Pin: the to-do list holds A then B, in input order, and `skipped` is 1. Red line: the name does not exist.
2. `run_batch reports analyzed, skipped and failed itself` (new). The same four items against a store, a counting analyzer that throws for B's notespath, `on_progress` keeping the last progress. Pin on the last progress: `total` 2, `analyzed` 1, `failed` 1, `skipped` 1, `completed` 2. Red line: the fields do not exist.

In `tests/test_library_jobs.cpp`:
3. `jobs: the snapshot's counts come from the batch in one piece` (new). Three fake charts, an analyzer that fails one; after the job finishes pin `analyzed` 2, `failed` 1, `completed` 3, `skipped` 0, `total` 3, and `job.batch_run().lens == test_run().lens`. Red line: `analyzed` and `batch_run` do not exist. The one-frame gap itself cannot be pinned; one callback writing all five numbers is the proof, and your report says so.
4. `jobs: a report job carries the cap and lens it was built from` (new). Pin `cap()` and `lens()` equal the arguments, and that the constructor takes `85.5` as a double. Red line: the accessors do not exist.

In `tests/test_config.cpp`:
5. `settings: the hit window keeps a decimal, and 0 reads the default (D51 Q15)` (new). A file line `hit_window_ms=85.5` loads 85.5 and saves back as `hit_window_ms=85.5`; `hit_window_ms=0` loads `kDefaultHitWindowMs`; the default saves as `hit_window_ms=85`, the line today's files hold. Red line: 85 today. The line `Settings::clamp(&Settings::hit_window_ms, 0) == 85` in `Settings::clamp is the one range every caller asks` moves here as the file-load pin, because `clamp` is the int boxes' range and the hit window is no longer an int field.

Existing cases that must pass unchanged: `run_batch analyzes a chart found in two folders once`, `run_batch files results under the lens it is given`, both `run_batch: cancel` cases, every `jobs:` case, `settings round-trip through an INI file`, `settings: load and save name the same keys`, `a missing INI yields defaults`, `batch_run bundles one Settings' chartmode, lens and search settings`.

Scan rows (end of `tests/test_single_owner.cpp`, each an `OwnerRule` with `must_match` examples taken from the old lines): batch counts, owner `run_batch` in `analysis.cpp`, flagging `size()) - p.total` and `completed - failed` spellings in `src`; the hit window as an int, owner `kDefaultHitWindowMs` in `model.h`, flagging `static_cast<int>(kDefaultHitWindowMs)` in `src` and `tests`.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*test_analysis*`, `-sf=*test_library_jobs*`, `-sf=*test_config*`
- `build-cpp\Release\hydra_tests.exe -tc="hydra_batch*"` (run only; build `hydra_batch` with a warm `.\build_cpp.ps1 -Target hydra_batch` first)
- `build-cpp\Release\hydra_tests.exe -tc="single-owner*"`

Nothing else. Never the full suite. Run one build with no `-Target` before committing so `hydra_ui` and the app link.

## Stored results

None change. The INI file gains a decimal in `hit_window_ms` only when the user types one; a file written today loads to the same values. hydra_batch prints the same words; on a library with a chart in two folders its "skipped" number drops by the copies, which D51 call 10 approved for the confirm and open question 1 extends to this line.

## Not in this task

- The confirm's counts, the strip's and tooltip's readers, `batch_settings_summary`, the "Open automatically" hint and `update_background_jobs` (LB2).
- `ReportOptions` and `report.cpp` (RP); `paths_tab.cpp`'s cast (J3-3); the guide's line on "Open automatically" (DOC).
- `DmReportJob`'s read of `auto_open_report`: it stays, because call 23 keeps the behaviour and fixes the words.

## Done when

- `BatchProgress` carries the five counts and `run_batch` fills them; `plan_batch`, `charts_with_result` and `BatchRun::cap_query` exist; no `size()) - p.total` or `completed - failed` spelling remains in `src`.
- `BatchJob::batch_run`, `Snapshot::analyzed`, `ReportJob::cap`/`lens` exist; `hit_window_ms` is a double from the file to the report job, and `static_cast<int>(kDefaultHitWindowMs)` is gone from `src` and `tests`.
- The five cases pass with their red lines recorded; the named existing cases pass unchanged; `-tc="hydra_batch*"` passes.
- `git diff --stat <base>..HEAD` lists only the owned files. The stamp still reads "2.1.0".

## Open questions (each with a recommended answer)

**Decided (D62):** the user took the recommended answer to every question below. Treat each recommendation as the decision.

1. **A second copy counts nowhere (needs the user: a displayed count).** After D51 call 10 a chart in two folders runs once. Its second copy is then neither "analyzed" nor "skipped (already had a result)" in the strip and in hydra_batch's closing line. Recommended: count it nowhere; one chart is one chart. The alternative, a new "copies" number, is new text no decision names.
2. **Where the counts live (code-only).** Recommended: fields on `BatchProgress` set only by `run_batch`, copied by the job in its one `on_progress`. A `BatchJob` that recounts from its callbacks would be the copy finding 142 is about.
3. **How a decimal hit window is written (code-only).** Recommended: the stream's default, so today's files stay byte-identical (85 writes "85") and 85.5 writes "85.5".
4. **hydra_batch's per-line counter (code-only).** Recommended: it stays a local line number; it equals `completed` and is not a total the user reads. A scan row does not flag it.
5. **The confirm's store read runs on the UI thread (code-only, LB2 acts on it).** `charts_with_result` returns md5s only, one query. Recommended: fine to call from `open_batch_confirm`; `list_chart_library` was the slow query, and it is not involved.

## Commits

One commit for `analysis` and the CLI, one for the jobs and `config` if you prefer, trailers `Task: LB1` plus the preamble's others. Report as the preamble says, and name the commit LB2 forks from.
