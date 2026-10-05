Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task LB2: app state, library screen and toolbar read their owners (findings 62 caller, 143, R7.3, 109, 88 library half, 170 toolbar, 56 `batch_settings_summary`, 124 and 125 callers, 142 readers, R7.2 hint)

Task id: LB2. Base: main after M7-2 and M6-J2 (the main session names the hash at launch). **Forks from LB1's commit** (which forks from RP's), because it reads LB1's `plan_batch`, `charts_with_result`, `BatchJob::batch_run` and `Snapshot::analyzed`, and RP's Clone Hero rules gate. Branch: claude/p7-lb2 (worktree `.claude\worktrees\p7-lb2`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 3 table, "LB Library and batch", split into LB1 and LB2 (see `p7-lb1.md`; no file is in both). Decisions: D51 calls 9, 10, 11, 23, 24 and 26, the D51 addendum's SE1 note on `depth_mode`, and D58 item 5. Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 62.`, `#### 143.`, `#### R7.3`, `#### 109.`, `#### 88.`, `#### 170.`, `#### 56.`, `#### 124.`, `#### 125.`, `#### 142.` and `#### R7.2`.

Plan correction: `hydra_uitest` selects a test by its exact name (`selects` in `uitest_harness.cpp`), and no test is called `library` or `batch`. The real names are listed under "Test filters".

## Goal

The app state asks one owner for each fact it used to work out itself: the viewed difficulty's song length goes into ST2's per-difficulty row (D51 call 9); one helper says which library row is the selected one; one `can_scan` says whether a scan may start, and a scan asked for during a batch is refused with "A batch is running." (call 24); one `any_job_running` lists every background job, and `wait-idle` waits on it; the library's filters see no facts for a Ready chart with no paths (call 11); the Compare button asks RP's gate; the batch confirm, strip and report start read LB1's counts and run (calls 10 and 26); the "Open automatically" hint reads "Open each report in your browser as soon as it's built." (call 23). No score, path or stored record changes; the results stamp stays "2.1.0".

## What the code does today

Line numbers are from main; `claude/p6-j2-4` (1019d98) moves `app_state.cpp` and `library_toolbar.cpp` by a few lines.

**62.** `AppState::update_song_length` (`app_state.cpp` line 287) writes `store->set_song_length(chart.md5, *length)`, the per-chart row, and puts the length on `viewed` and on every parked lookup whatever its difficulty. `length_tried_md5_` is keyed by md5 alone, so switching difficulty on the same chart never retries. ST2 added `set_song_length(hyhash, chartmode, length_ms)` and the `songlength` table, and `get_record` reads that row first, so today the backfill's write lands only in the fallback.

**143.** `relative_row` (line 148) and two sites in `library_table.cpp` (lines 337 and 360) each compare `notespath` to decide which row is selected. `analyze_job_shown` (line 276) makes the same choice without naming it.

**R7.3.** The toolbar's `Scan library` button (`library_toolbar.cpp`, on `claude/p6-j2-4` line 69) is on when there are folders, no running batch and no `scan_job`. The `request_scan` handler in `library_view.cpp` (line 129) checks folders and `scan_job` but not the batch. `start_scan` (line 436) refuses only an unfinished scan. "Scan now" (`library_dialogs.cpp` line 163) and "Rescan library" (`details_panel.cpp`, not yours) both set `request_scan`, so a scan starts during a batch.

**109.** `jobs_busy` (`tests/ui/uitest_harness.cpp` line 336) keeps its own list of six jobs plus `preview->loading()`. It misses `dynamics_job`, `length_job` and the Preview's path-switch job. PV added `PreviewController::busy()` (on `claude/p7-pv`), which covers both Preview jobs. `docs/agents/ui-testing.md` line 66 lists the jobs again.

**88, library half.** `facts_of` (`library_model.cpp` line 36) hands facts to every Ready row. `query_matches` (`library_query.cpp` line 380) and the `RowFacts` comment (`library_query.h` line 82) say a row is "analyzed" when `stars` holds a value. ST2's `PathSummary::has_scored_best_path()` is the owner.

**170.** `render_actions_row` (`library_toolbar.cpp` line 97 on main) tests Expert, the 4-bar cap and 1.1 fills itself and picks one of three tooltip sentences. After RP, the gate in `src/app/dm_report.h` owns the three rules.

**56.** `batch_settings_summary` (`library_dialogs.cpp` line 85) tests `depth_mode == 0`; SE1's `Settings::search_depth_mode()` says anything but 1 is scores, so a hand-edited `depth_mode=2` reads "points" here and searches by scores.

**124, 125, 142, R7.2.** `open_batch_confirm` (line 445) counts from `library.counts().analyzed`, a cache, and `render_batch_confirm` (`library_dialogs.cpp` line 279) counts copies, not charts. `update_background_jobs` (line 480) builds the `ReportJob` from the live `settings.cap_query()` and `settings.lens()`. `batch_counts` (line 56) and the Stop tooltip (line 420) print `completed - failed`. The hint (line 506) reads "Open the report in the browser whenever a batch finishes".

## What changes

**62.** `update_song_length` calls ST2's per-difficulty `set_song_length` with the chartmode the job was started under. `AppState` records that chartmode beside the job when it starts it, and `length_tried_md5_` becomes a md5-plus-chartmode pair, so another difficulty of the same chart gets its own backfill. On finish the length goes to `viewed` only when `committed_chartmode_` is the job's chartmode, and to a parked lookup only when its `RecordKey` chartmode matches. `song_length_job.cpp` is untouched. The old per-chart overload keeps its one dead declaration in `record_store.h` (J3-6's file this wave); say so in your report so J4-2 removes it.

**143.** `AppState::is_selected_row(const store::ChartLibraryEntry&) const` answers by `notespath`. `relative_row`, the highlight and the scroll-to loop in `library_table.cpp`, and `analyze_job_shown` (for the job's song) all call it. Its comment says why notespath and not md5 in one line.

**R7.3.** `AppState::can_scan() const`: song folders present, no `scan_job`, no running batch (`batch_running()`). `start_scan` returns unless `can_scan()`, and when the thing in the way is a running batch it calls `set_status("A batch is running.")` (D51 call 24; see open question 1). The `request_scan` handler calls `start_scan` and opens "Scanning charts" only when a `scan_job` now exists. The toolbar's button reads `can_scan()`; its tooltip "Busy: a batch is running." stays byte for byte.

**109.** `AppState::any_job_running() const` lists `scan_job`, `batch_job`, `analyze_job`, `report_job`, `dm_fetch_job`, `dm_report_job`, `dynamics_job`, `length_job` and `preview && preview->busy()`. The parked DM jobs are left out; the comment names them and says nothing on screen waits on them. `jobs_busy` returns it. The `wait-idle` row in `docs/agents/ui-testing.md` reads: wait until every background job has finished; the list is `AppState::any_job_running`.

**88.** `facts_of` gives facts only to a Ready row whose `summary.has_scored_best_path()`; the `RowFacts` comment and `query_matches`'s comment say "a row with no facts matches no filter" and name `facts_of` as the owner of when a row has facts. The two `summary.score.has_value()` reads in the Best path sort (`library_model.cpp` lines 200 and 201) call the owner too. Behaviour is unchanged; the comments and calls are the fix.

**170.** The Compare button is disabled when RP's gate refuses. Read `p7-rp.md` for the gate's name. If the gate returns which rule failed, the toolbar keeps its three sentences byte for byte and picks by that answer; if it returns the sentence, the toolbar prints it and its copies go (open question 3).

**56.** `batch_settings_summary` asks `s.search_depth_mode()`; the words "scores" and "points" and their format stay.

**124 and 125.** `open_batch_confirm` builds the scope's `ScanItem`s through J2-4's `scan_item_of`, asks LB1's `charts_with_result(*store, settings.batch_run(), false)` once, and runs `plan_batch`: `batch_scope_with_result` is the plan's skipped count and a new `batch_scope_charts` is the distinct chart count. `render_batch_confirm` reads `batch_scope_charts` where it read `batch_scope.size()`. `BatchJob` is still given the scope rows; its `run_batch` makes the same plan from the same store, so the strip's `skipped` is the number the confirm showed. `update_background_jobs` builds the `ReportJob` from `batch_job->batch_run().cap_query()` and `.lens`; `auto_open_report` and `hit_window_ms` stay live, because they only shape the page.

**142 readers and R7.2.** `batch_counts` and the Stop tooltip read `s.analyzed`. The hint reads exactly "Open each report in your browser as soon as it's built."

## Owned files (only these may change)

- `src/ui/app_state.h`, `src/ui/app_state.cpp`
- `src/ui/library_toolbar.cpp`, `src/ui/library_view.cpp`, `src/ui/library_table.cpp`, `src/ui/library_model.cpp`, `src/ui/library_dialogs.cpp`
- `src/app/library_query.h`, `src/app/library_query.cpp` (comments and the `query_matches` comment only)
- `tests/ui/uitest_harness.cpp` (`jobs_busy` only), `docs/agents/ui-testing.md` (the `wait-idle` row)
- `tests/test_app_state.cpp`, `tests/test_batch_text.cpp`, `tests/test_library_model.cpp`, `tests/test_library_query.cpp`
- `tests/test_single_owner.cpp`: your own scan rows at the end of the file only.

Not yours: `src/ui/details_panel.cpp`, `library_jobs.*`, `analysis.*`, `config.*` (LB1), `src/app/dm_report.*` (RP), `src/store/*` and `src/ui/song_length_job.cpp`, `src/ui/preview_*` (PV, merged), `tests/test_song_panel_state.cpp` (run only).

Notes on the base. `app_state.h/.cpp`, `library_toolbar.cpp`, `uitest_harness.cpp` and `test_app_state.cpp` changed on `claude/p6-j2-4` at 1019d98 (`batch_running()` callers, `library_total` gone, `analyze_search_label`, `kStatusFadeSeconds`, `scan_item_of`); `test_app_state.cpp` also on `claude/p7-se2`. `library_query.h/.cpp` and `test_library_query.cpp` are phase 6 J2-5's in J2 (`term_applies_to`, the stars error text); its branch did not exist when this was written, so re-read both at your fork. `library_view.cpp`, `library_table.cpp`, `library_model.cpp`, `library_dialogs.cpp`, `test_library_model.cpp` and `test_batch_text.cpp` are main's text on every branch.

## Test cases to add or re-pin

Write each red first, then green. Build apps the way `refresh_library_row picks up one chart's new result` and the batch cases in `test_app_state.cpp` do (their store, library and `BatchGate`); no new helper.

In `tests/test_app_state.cpp`:
1. `update_song_length stores the length under the viewed difficulty (D51 Q9)` (new). A Ready record with no stored length for a corpus chart, selected with the panel open; tick until `length_job` finishes. Pin: `get_record` under the viewed chartmode reads `store::song_length_ms` of that chart (call the owner), and under another chartmode reads none. Red line: both read the length today, through the per-chart row.
2. `is_selected_row: the selected row is the one with its notespath, not its md5` (new). Two entries, one md5, two notespaths (as `library model: one chart in two folders gets both rows updated` builds them); select the first. Pin true for it, false for its twin. Red line: the name does not exist.
3. `a scan cannot start during a batch, and the status line says so (D51 Q24)` (new). With a gated batch running: `can_scan()` false, `start_scan()` leaves `scan_job` empty, `status_message == "A batch is running."`; after the batch finishes `can_scan()` is true. Red line: a scan job is created today.
4. `any_job_running lists every background job` (new). Idle: false. A running batch: true. The `length_job` from case 1 in flight: true. After both finish: false. Red line: the name does not exist.
5. `the post-batch report lists the batch's cap and lens, not the live settings` (new). A gated batch at cap 4 finishes; set `settings.sp_cap = 5` without committing, then run `update_background_jobs()`. Pin `report_job->cap() == store::CapQuery::at(4)` and `lens()` equals the batch's. Red line: `at(5)` today.
6. `the confirm counts charts with a result from the store, once per chart (D51 Q10)` (new). Three rows, two sharing an md5, one of them stored through `store->` directly with no library reload; `open_batch_confirm()`. Pin `batch_scope_charts` 2 and `batch_scope_with_result` 1. Red line: 3 and 0 today.

In `tests/test_batch_text.cpp`:
7. Extend `batch text: the confirm lists the settings a batch runs with`: `depth_mode = 2` reads "4 scores" (D51 addendum: anything but 1 is scores). Red line: "4 points".

In `tests/test_library_model.cpp`:
8. `library model: a Ready row with no paths is Analyzed but has no facts (D51 Q11)` (new). A Ready `SummaryLookup` with no score and no stars: `set_query("stars:7")` and `set_query("squeeze<=200")` leave it out, and the Analyzed chip counts it. This case is green on the base; it pins the decision and the owner call, and your report says so.

Existing cases that must pass unchanged: every case in `test_song_panel_state.cpp` (`select_relative` reads the new helper), `library query: stars:N keeps analyzed rows with exactly N stars`, `library query: squeeze<=N keeps analyzed rows whose hardest squeeze is at most N ms`, `library model: Best path sorts by score, with unscored rows last both ways`, `the chart-file check runs on open and then every two seconds`, `a batch result for the open chart turns the panel Ready`.

Scan rows (end of `tests/test_single_owner.cpp`, `OwnerRule`s with `must_match` examples from the old lines, scope `src` and `tests`): the selected row, owner `is_selected_row`, flagging `notespath` compared with `selected`; may a scan start, owner `can_scan`, flagging the toolbar's and the handler's old lines; every background job, owner `any_job_running`, flagging the harness's old `->finished()) return true` lines; has a scored best path, owner `has_scored_best_path` in `record_store.h`, flagging `summary.score.has_value()` and `summary.stars.has_value()` outside it.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*test_app_state*`, `-sf=*test_batch_text*`, `-sf=*test_library_model*`, `-sf=*test_library_query*`, `-sf=*song_panel_state*` (run only), `-tc="single-owner*"`
- `build-cpp\Release\hydra_uitest.exe --test <name>` for `scan`, `library-state-per-app`, `library-search`, `batch-confirm`, `batch-done-strip`, `batch-strip-drift`, `batch-pause-stop`, `settings-and-reports`, `compare-disabled`, `report-buttons` and `status-line` (build `hydra_uitest` with `.\build_cpp.ps1 -Target hydra_uitest` first)
- Finding 109's repro once, as a scratch script run with `hydra_uitest <script path>`: open an unanalyzed chart, click Dynamics, `wait-idle`, `text`. Report the output and delete the script before you commit.

Nothing else. Never the full suite, never `hydra_uitest --all`.

## Stored results

None change. The `songlength` row the backfill writes now carries the viewed difficulty, which ST2's `get_record` already reads first; a database from before ST2 behaves as D58 item 5 says.

## Not in this task

- `BatchProgress`, `plan_batch`, `charts_with_result`, `BatchJob::batch_run`, the hit window's type and hydra_batch's line (LB1).
- The gate itself and the leaderboard page's "SP cap 4" text (RP); `details_panel.cpp`'s "Rescan library" button (it sets `request_scan`, which the handler now refuses).
- The guide's "Open automatically" line and the "All-0 path" sentence (DOC); the old per-chart `set_song_length` (J4-2).

## Done when

- `is_selected_row`, `can_scan` and `any_job_running` exist and every former copy calls them; `update_song_length` writes the per-difficulty row; `facts_of` and the sort call `has_scored_best_path`; the toolbar calls RP's gate; the confirm, strip, tooltip and report start read LB1's owners; the hint and the status line carry D51's exact words.
- The eight cases pass with their red lines recorded; the named existing cases pass unchanged; the eleven GUI scripts pass; the 109 repro reads the Dynamics count, not "Reading chart...".
- `git diff --stat <base>..HEAD` lists only the owned files. The stamp still reads "2.1.0".

## Open questions (each with a recommended answer)

1. **How "A batch is running." shows (needs the user: a display call).** D51 call 24 names the status line but not whether it fades. Recommended: `set_status` (news, fades after `kStatusFadeSeconds`), because the batch strip above it already shows the batch. The alternative, `set_problem`, would hold the line until something replaced it.
2. **The confirm counts charts, not copies (needs the user: a displayed count).** "Analyze N charts that have no result yet?" and "re-analyzing N" count distinct charts; a second copy adds nothing. Recommended: yes, this is D51 call 10's "the confirm's count drops by the number of duplicates" made exact.
3. **Who holds the Compare button's three sentences (code-only).** Recommended: whatever RP's gate returns decides; the words stay byte-identical either way, and the toolbar never tests a rule itself.
4. **The confirm reads the store on the UI thread (code-only).** Recommended: yes; `charts_with_result` is one md5-only query (LB1 open question 5).
5. **Parked DM jobs stay out of `any_job_running` (code-only).** Recommended: out; they are cancelled and nothing on screen waits on them, as finding 109 says.
6. **The backfill retries per difficulty (code-only).** Recommended: key `length_tried_` by md5 and chartmode, so Easy gets its own length after Expert's.

## Commits

One commit, trailers `Task: LB2` plus the preamble's others. Report as the preamble says, plus the 109 repro's output and the dead-overload note for J4-2.
