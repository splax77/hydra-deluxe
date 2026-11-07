# The stale comment audit's code tasks

Written 2026-11-07 by the planning agent PL-CODE (session 28b91640). Nothing here is built until the user says yes. The code was read on main at commit 77750db8 that day.

## The plain version

The stale comment audit fixed comments. Ten of its rulings also asked for small code changes, which a comment fix could not do. This plan turns those ten into ten executor tasks (one each) and adds one live run with the game, which checks CT-8. Four of the changes are things a player can see: a sentence in four report screens, a chip's outline in two report windows, what a click on a comparison row does to the settings bar, and one sentence in the README. The rest are test code, probe tools and rule files.

The rulings live in `docs/audit/2026-10-03-fix-decisions.md` as records D305 to D313, and in `docs/audit/2026-10-31-stale-comments/questions/q1.md` to `q9.md`. Those are on the audit branch `claude/stale-comment-scouts` (tip b9251f68), not on main. Main's decision log ends at D304. Question 1 below is about what to do with that.

Two targets had moved since the rulings were written. `src/audio/mixer.cpp`'s comment (D308 item 1) already says a stem is copied in, so that part is done and has no task. Two rule rows that D309 item 5 treats as "nothing left to catch" still catch live code, so they go to the user as questions 3 and 4 instead of into a task.

## Rulings that bind this plan

Each ruling is quoted from its record, and each record's question file is named, so the executor reads the ruling and not this summary.

- D305 item 5 (`q1.md` question 5): the "no chart library" sentence. After: "This database has no chart library. Run hydra_batch --whole-library, or scan in Hydra, to build one." The user chose "All recommended".
- D306 item 5 (`q2.md` question 5): `hit_offset_ms` "stops reading the field and always uses the send time; the lines `watch_window.py` prints say "+0x2e0" with no "hit time"; and the test that pins them changes with them."
- D308 item 1 (`q4.md` question 1): six small promises, listed in the record.
- D308 item 2 (`q4.md` question 2): unused code goes, "each after the executor confirms it has no caller"; `song_digest` digests the constant `true` in place of `sp_cap_converged`.
- D308 item 3 (`q4.md` question 3): `DmWindow` and `PathWindow` move into `AppState`.
- D308 item 5 (`q4.md` question 5): "the window draws the outline, so the two match, and the comment goes. It gets a real-app check before merge. The word on the chip does not change."
- D309 item 5 (`q5.md` question 5): rule rows with nothing left to catch "go, as D308 item 1 removed a check of the same dead sentence."
- D310 item 2 (`q6.md` question 2): README's Stale sentence "drops its version list and reads "A release that changes how charts are analyzed marks them `Stale` until each song is analyzed again.""
- D310 item 3 (`q6.md` question 3): a DM comparison row click "stops switching, as the path report's does under D115 item 5, and the song opens in the bar's mode. The test that pins the switch changes."
- D310 item 5 (`q6.md` question 5): `menu_tools.py`'s `same_folder` "uses `realpath` as `tools/hydra_ui_mcp.py` does. It needs a check on a live run."

## How the plan handles records main lacks

A player-visible fix cites the D-record that asked for it. Main's decision log has no D305 to D313, so a comment on main cannot point at them yet. The plan's answer is in question 1: merge the audit branch's decision-log entries before these tasks merge. If the user says no, every task writes "the stale comment audit's ruling of 2026-11-06" in comments and puts the D-number only in its commit message and this plan.

No other session has taken these items on main. Main's log ends at D304, and every target below was found as the ruling describes it, except the two moved targets named in the plain version.

## One rule for the executor of CT-1

The new sentence holds the flag `--whole-library`. A hook refuses any shell command whose text holds that flag unless the user typed `allow library run` (`C:\Users\Patrick\.claude\gates\percall.py`, `_library_refusal`, which reads the `command` of every shell tool call). So the CT-1 executor edits `src/app/report.h` and its test with the Edit tool only, searches for the sentence with the Grep tool only, and writes a commit message that does not hold the flag. The same holds for the reviewer.

## What a player sees

Agent rules 11 and 12 ask for every screen that shows a change, with its text before and after. Here they are.

**The "no chart library" sentence (CT-1, D305 item 5).** It shows when the database holds results but no chart library, which is what `hydra_batch <folder>` leaves behind. Four screens read the one constant `report::kNoChartLibrary` in `src/app/report.h` line 209: the path report window's empty text (`src/ui/path_report_window.cpp` line 234), the library job strip (`src/ui/library_jobs.cpp` line 615, which throws `report.why_empty`), the DM comparison (`src/app/dm_report.cpp` line 87) and the fill comparison (`src/app/fill_report.cpp` line 395). Before: "This database has no chart library. Run hydra_batch without folder arguments, or scan in Hydra, to build one." After: "This database has no chart library. Run hydra_batch --whole-library, or scan in Hydra, to build one." The text is the same for every chart, because it depends only on the database. GUI check: case 1 of `docs/agents/ui-testing.md` (text), covered by the four doctests that compare each screen with the constant.

**The tn chip's outline (CT-9, D308 item 5).** `report_window.cpp` line 390 skips the rounded outline for `ChipToken::tn`; the HTML pages give every `.chip` a `border: 1px solid currentColor` (`src/app/html_page.cpp` line 260) and `.s-same`, the tn colour, keeps it. Two windows draw tn chips: the Path report's Squeeze column, where the tier named "None" carries token "tn" (`src/core/squeeze_rating.cpp` line 194), and the DM comparison's status column for a score the library does not list (`src/app/dm_report.cpp` line 175, `kStatusNotInLibrary`). Before: those chips have coloured text and no outline. After: a one-line rounded outline in the chip's own colour, as every other chip has. The word on the chip does not change. GUI check: case 2 (what the user sees). Before merge the main session (the only one that builds, see "Who builds") runs the real app with the user's `hydra_ui.ini` and a scratch database built from `testdata/input` (a render check never starts a whole-library analysis, ui-testing.md case 3), opens both windows and shows the captures to the user.

**A DM comparison row click (CT-4, D310 item 3).** Today `src/ui/dm_report_window.cpp` line 155 passes the comparison's mode to `select_chart`, so the settings bar switches to it. The path report passes the bar's own mode (`src/ui/path_report_window.cpp` line 265). Say you built a comparison on Pro Drums and then set the bar to Drums. Before: a row click flips the bar's "Pro Drums" row back on and the song opens on Pro Drums. After: the bar stays on Drums and the song opens on Drums. Screens that read the mode: the settings bar's Difficulty, "Pro Drums" and "2x Bass" rows (`src/ui/settings_bar.cpp` lines 98 to 105), the song panel, whose result is the bar's mode, and the Library's result columns. GUI check: case 1 (behaviour), covered by the doctest CT-4 adds.

**README's Stale sentence (CT-10, D310 item 2).** `README.md` line 57. Before: "A release that changes how charts are analyzed marks them `Stale` until each song is analyzed again, as 2.1.0, 2.4.0 and 2.8.0 did." After: "A release that changes how charts are analyzed marks them `Stale` until each song is analyzed again." The rest of the paragraph stays, including "Upgrading keeps your library and settings.", which D310 item 2 left on the open list. The README is a public page, so it follows `docs/agents/public-doc-style.md` and goes through the doc review gate in `docs/agents/doc-review.md`.

No other task changes anything a player sees.

## Questions for the user

**Answered.** The user said "Yes, all recommended" on 2026-11-07, and D315 records it. That approves the plan, and option (a) of all five questions. So the decision log reaches main first, CT-4 removes `select_chart`'s mode parameter, CT-6 keeps the two rows and removes only their dead samples, SN-216 and I-0623 close with no edit, and CT-7 also owns `watch_checks.py` and `test_watch_checks.py`. Where a task below says "until answered", this paragraph is the answer. LS-1 still waits for its own go.

Each question names which side owns it.

1. **Which comes first, the audit branch or these tasks?** (Owner: the main session, git.) The D-records live only on `claude/stale-comment-scouts`. Options: (a) merge that branch, or at least its `docs/audit/2026-10-03-fix-decisions.md`, before any task here merges, so comments and commits can cite D305 to D313; (b) start the tasks now and cite "the stale comment audit's ruling of 2026-11-06" in comments, with D-numbers only in commit messages. Recommended: (a). Cost of (a): no task merges until that branch merges. Cost of (b): comments carry a wordier cite and the D-numbers stay out of the code. The audit branch's merge itself is out of this plan's scope; this question only asks for its order.
2. **Does `select_chart` keep its mode parameter?** (Owner: C++, `AppState::select_chart` in `src/ui/app_state.cpp` line 632.) After CT-4, both callers pass the bar's own mode, so the branch that switches the mode and commits the settings (lines 633 to 646) can no longer run. D310 item 3 says "The test that pins the switch changes", and the test that pins it is `tests/test_app_reports.cpp` line 366, which calls `select_chart` directly with another mode. Options: (a) drop the parameter and the branch, and rewrite that test to check selection only; (b) keep both, change only the DM click, and leave the test as a unit test of a branch nothing reaches. Recommended: (a); it is what "the test changes" means, and an unreachable branch is D308 item 2's kind of code. Cost of (a): the function loses its two refusals for a mode switch (the batch-running status and "this build cannot analyze that mode"), so a later row that wants to switch the bar must put them back. Cost of (b): dead code stays, with a test that exercises it. CT-4 waits for this answer.
3. **Two rule rows D309 item 5 names still catch live code.** (Owner: the rule files under `tests/single_owner/rules/`.) `parse.json`'s "Which file is the folder's song.ini?" row (line 200) looks for `is_song_ini(`; that function exists and is called in `src/parse/chart_files.cpp` line 50, so the row still guards against a second caller. Only its must_match sample at line 214 names a form no code has. `app/concept-H3.json`'s row (line 31) looks for "Nothing matches", "No charts match" and a typed search hint; today those match only the owner, `src/app/html_page.h` line 47, but the row guards one sentence and one hint shape across three screens. Its sample at line 45 quotes the sentence D308 item 1 removed from `smoke.txt`. Options: (a) both rows stay and only the dead sample lines go; (b) both rows go, by the letter of D309 item 5. Recommended: (a). Cost of (a): two rows D309 item 5 may have meant to remove stay, because they still catch live code. Cost of (b): the guard against a second `is_song_ini` caller, and against a second spelling of the empty-search sentence and hint, is gone. CT-6 does (a) unless told otherwise; the five rows whose patterns match nothing go either way.
4. **May a rule sample name a dead symbol?** (Owner: the rule files.) SN-216 is `app.json` lines 168 and 170, must_match samples naming `chart_files_unchanged` and `chart_files_sig` in forms no code has; the row's `hash_chart_file` alternative is live (`src/pathbuilder/pb_state.cpp` line 33 is exempt, `tools/replay.cpp` lines 472 and 525 are outside its `src` scope). I-0623 is `store.json` line 235, a must_not_match sample naming `kSummaryColumnCount`, which is gone; the row's pattern still matches the owner's VALUES line. A sample is a self-test of the regex, not a claim about the tree. Options: (a) the samples stay and both rows close with no edit; (b) each dead sample is replaced with a line the tree has. Recommended: (a). Cost of (a): two sample lines keep naming symbols the tree no longer has, which a reader may take for live code. Cost of (b): two rows are edited for samples that test nothing about the tree. No task does this until answered.
5. **`watch_checks.py` calls +0x2E0 the hit time too.** (Owner: Python, `tools/ch_probe/experiments/watch_checks.py`.) Its `hit_time_check` prints "PASS hit time (+0x2E0): ..." (lines 110, 138, 199) and `tools/ch_probe/tests/test_watch_checks.py` pins those lines. D306 item 5 named only `watch_window.py`. Options: (a) CT-7 rewords those lines the same way; (b) they stay. Recommended: (a). Cost of (a): that tool's new check lines no longer match its old runs' text, as D306 item 5 already accepts for `watch_window.py`. Cost of (b): the same field is called a hit time in one tool and not in the other. CT-7 does only what the ruling names until answered.

## Tasks

Every task runs on Opus in its own worktree cut from main by hand (not `isolation: worktree`), finishes inside 100 tool calls and 20 minutes (the caps in `docs/agents/brief-preamble.md`, "Committing"), and stops at its stop condition. Each cites its D-record as question 1 decides. No two parallel tasks own the same file.

**Who builds.** Agent rule 14 and D314 say agents never build, and that a main session builds one at a time through a pass from `hydra_build_pass`. So an executor writes its change and commits. The main session then builds each C++ branch in turn in the one built worktree, `fx-w2`, a warm build each, and runs the tests the task names. A failed build or test goes to a fresh executor on the same branch, by D314 item 1. D314 item 3 says a test runs red only where a brief asks. This plan asks for that in two places: CT-4's row click test and CT-2's `hydra_replay` flag test. Each commits its test alone, then the fix, and the main session runs both commits. Every other new test the reviewer reads, saying what would make it fail. So every "Tests" line below is what the main session runs; an executor's stop condition is only its edits committed and the searches it names done. CT-7, CT-8 and the Python parts of CT-2 are Python: their executors run those pytest commands themselves, because they need no build. CT-6 changes only rule files, but the single-owner scan reads the source tree its exe was built from, so an exe from another tree would scan main and not the branch. The main session therefore runs CT-6's scan in `fx-w2` with CT-6's branch checked out (rebuilding first if the rules are compiled in), and again on main after the merge. CT-9's real-app check and its captures are also the main session's: it builds CT-9's branch in `fx-w2`, runs the real app, and shows the captures to the user. The executor never starts the app.

**Run in parallel, wave A:** CT-1, CT-2, CT-3, CT-6, CT-7, CT-8, CT-9, CT-10. None shares a file with another.

**Wait:** CT-4 waits for question 2's answer. CT-5 waits for the merges of CT-4 and CT-9, because CT-4 shares `src/ui/dm_report_window.cpp` and `src/ui/app_state.h` with it and CT-9 shares `tests/ui/uitest_report_windows.cpp`. LS-1 waits for CT-8's merge and the user's go.

### CT-1: the "no chart library" sentence (D305 item 5)

Owns `src/app/report.h` and `tests/test_report_rows.cpp`. Change the constant at line 209 to the ruled sentence, with the Edit tool only (see the rule above). The comment above it stays. Add one check to the test at `tests/test_report_rows.cpp` line 930 that pins the new sentence as a literal, the way the first D89 test did. Tests the main session runs, each by name: `hydra_tests.exe "-tc=a report on results with no chart library*"`, `"-tc=collect_dm_rows: a database with no chart library*"`, `"-tc=generate_fill_report: a database with no chart library*"`, and `"-sf=*test_library_jobs.cpp"` (its check at line 482 compares the strip's message with the constant). Executor's stop condition: a Grep for "without folder arguments" finds nothing outside `docs/audit/` and `docs/sessions/`; the main session's: the four pass.

### CT-2: the small promises (D308 item 1)

Owns `tests/test_store_results.cpp`, `tools/precheck/range.cpp` (and `range.h` if a helper needs declaring), `tests/test_precheck_git.cpp`, `tools/replay.cpp`, `tests/test_cli.cpp`, `CMakeLists.txt` (only the lines that give `test_cli.cpp` the `hydra_replay` exe path, see below), `tests/ui/scripts/smoke.txt` and `tools/test_hydra_ui_mcp.py`. Five of the six fixes remain; the mixer comment is already done on main.

The self-comparison at `test_store_results.cpp` line 358 goes, and the comment at lines 356 and 357 says only that an off limit stores 0. `range.cpp` line 249 strips a quoted path's closing quote too, through one small named helper. `parse_numstat` cannot show the difference, because it reads only the path's top folder and a trailing quote never changes that. So the test calls the helper directly: a quoted path with a space comes back with neither quote. On the old code that test is red because the helper does not exist; that is the failing-first state, and the executor does not need to build to commit it. `replay.cpp` lines 187 and 761 read `--ms` and `--depth` with the strict reader `parse_index` uses (`strict_ll` at line 223), so `--ms abc` refuses with a line naming the flag instead of meaning 0. No test runs `hydra_replay` today (premise P19, now checked): `tests/test_cli.cpp` runs only `hydra_batch` and `hydra_fillcompare`, through `run_exe` and exe paths that `CMakeLists.txt` lines 817 and 818 hand it, and `hydra_replay` is built on demand (`CMakeLists.txt` line 875). So the task adds a `HYDRA_REPLAY_EXE` define beside those two and a test in `test_cli.cpp` that runs `hydra_replay --ms abc` (and `--depth 1x`) with `run_exe` and checks the refusal line names the flag. The cost, which the main session knows before it builds: the test target then needs `hydra_replay` built too, as the tests need the two batch tools built. `smoke.txt` line 27 goes. The "no --live yet" branch in `tools/test_hydra_ui_mcp.py` lines 619 and 620 goes; `why_no_live` keeps its "not built" answer.

Tests the main session runs: `hydra_tests.exe "-sf=*test_store_results.cpp"`, `"-tc=precheck: the range size*"`, the new helper test and the new `test_cli.cpp` replay test by their names (at both commits, red then green), and `hydra_uitest.exe --script tests/ui/scripts/smoke.txt`. The executor runs `py -I -m pytest tools/test_hydra_ui_mcp.py -q` itself (Python, no build). Executor's stop condition: the new tests committed alone before the fixes, and the pytest passes.

### CT-3: code nothing uses (D308 item 2)

Owns `src/core/model.h`, `tests/song_digest.h`, `src/ui/library_jobs.h`, `tests/report_samples.cpp`, `tests/ui/uitest_harness.h`, `tests/ui/uitest_harness.cpp`, `tests/ui/uitest_preview.cpp`. It owns no rule file; if the single-owner scan names a row that blocks it, it stops and reports.

Each removal happens only after the executor confirms no caller, widening the search past the first name (preamble, "The code is the source of truth"). What I confirmed on 2026-11-07: `sp_cap_converged` appears only at `model.h` line 919 and `song_digest.h` line 156 (and in a 2026-08 session card, which is a record). `BatchJob::cancel` (`library_jobs.h` line 268): every `->cancel()` and `.cancel()` in `src/` and `tests/` is on a ViewJob, ScanJob, ReportJob, DmFetchUsersJob, DmReportJob, UpdateDownloadJob, a Path Builder or Preview job, or a `ResultJobBase`; `batch_job` (`app_state.h` line 497) is never cancelled; one call, `held->cancel()` at `tests/test_library_jobs.cpp` line 626, I did not type-check (P17). `/*mode*/` at `report_samples.cpp` line 15 is unused; every `add_path` call drops that argument. `text_width` and `rule_width` (`uitest_harness.h` lines 217 and 229) have no caller outside `uitest_harness.cpp`; `text_measurer` stays. `play0` at `uitest_preview.cpp` line 652 is read once and discarded at line 667.

`song_digest.h` digests the constant `true` where it digested the field, so no pinned digest moves. Tests the main session runs: `hydra_tests.exe "-sf=*test_perf_digest.cpp"` (the pinned digests), `"-sf=*test_library_jobs.cpp"`, `"-sf=*test_report_rows.cpp"` (uses `report_samples`), `hydra_uitest.exe --test layout-drift` (registered in `tests/ui/uitest_preview.cpp` line 1101), and `"-tc=single-owner*"`. Executor's stop condition: no pinned digest edited and the report names every search that found no caller; the main session's: all pass.

### CT-4: a comparison row click keeps the bar's mode (D310 item 3)

Owns `src/ui/dm_report_window.cpp`, `src/ui/app_state.cpp`, `src/ui/app_state.h`, `tests/test_app_reports.cpp`. Waits for question 2.

Line 155 of `dm_report_window.cpp` passes `app.settings.chartmode_key()` as the path report does at `path_report_window.cpp` line 265, and the comment above it says a click never switches the mode. If question 2 chooses (a), `select_chart` loses its mode parameter and lines 633 to 646, the header at `app_state.h` line 401 follows, and the test at `test_app_reports.cpp` line 366 becomes "select_chart selects the first copy in the bar's mode" with its D103 comment gone. Either way the task adds a test: build a DM result whose `chartmode` is not the bar's, call `dm_report_input(*app).callbacks.row_click(0)`, and check the bar's mode and the ini are unchanged and the song is selected. Red first: today that test fails because the mode flips. The executor commits that test alone, then the fix.

Tests the main session runs: `hydra_tests.exe "-sf=*test_app_reports.cpp"` at both commits (red, then green), and `"-tc=single-owner*"` because the rule at `tests/single_owner/rules/app.json` line 1096 names the call this task may remove. Executor's stop condition: the test commit and the fix commit exist, and its report says whether `Settings::with_chartmode` keeps a caller (`config.cpp` line 396 does).

### CT-5: the report windows' state lives in AppState (D308 item 3)

Owns `src/ui/dm_report_window.cpp`, `src/ui/path_report_window.cpp`, `src/ui/report_window.h`, `src/ui/app_state.h`, `tests/ui/uitest_report_windows.cpp`. Waits for the merges of CT-4 and CT-9.

The `static DmWindow w` (`dm_report_window.cpp` line 41) and `static PathWindow w` (`path_report_window.cpp` line 52) go. The draw functions (`report_window.h` lines 110 and 111) take their state from the caller: `AppState` holds one of each next to `details_ui` (`app_state.h` line 441), and the GUI test fixture in `uitest_report_windows.cpp` holds its own, since it draws the windows with sample results and no AppState (lines 96 to 125). How the struct is exposed (a header type or an opaque pointer) is the executor's call; the rule is that nothing static remains. The task adds one GUI test: open the DM window, type a search, rebuild the fixture's state as the runner builds an AppState per test, and read the search box empty.

Tests the main session runs: `hydra_uitest.exe --test report-window-row-click`, the new test by name, and `hydra_tests.exe "-sf=*test_app_reports.cpp"`. Executor's stop condition: a Grep for `static DmWindow|static PathWindow` in `src/` finds nothing. This is the largest C++ edit in the plan and the executor cannot compile it, so a failed build is likeliest here; it goes to a fresh executor, as "Who builds" says.

### CT-6: rule rows with nothing left to catch (D309 item 5)

Owns `tests/single_owner/rules/core.json`, `tests.json`, `store.json`, `ui.json`, `app.json`, `parse.json`, `app/concept-H3.json`, `app/d115-profile.json`.

Five rows go, because on 2026-11-07 their patterns matched nothing in their scope: `core.json` "In what order does a record list its paths?" (line 1610, SN-252), `tests.json` "Are two engine results the same answer? (tests)" (line 928, CK-8/N1), `store.json` "Is this still the row the report walk listed?" (line 155, I-0620), `ui.json` "Which charts does a batch analyze?" (line 774, I-0643), and `app.json` "Where does the Beyond tier start on the report page?" (line 987). For `app/d115-profile.json`'s one row (line 4, also under CK-8/N1) the executor runs its pattern over `src` and `tools` first (P18): if it matches nothing, the row goes; if it matches live code, the executor stops and reports it for a question like question 3. `parse.json` and `concept-H3.json` follow question 3's answer; until then, only their dead samples (lines 214 and 45) go. SN-216 and I-0623 wait for question 4 and are not touched.

Tests the main session runs, with CT-6's branch checked out in `fx-w2` (see "Who builds"): `hydra_tests.exe "-tc=single-owner*"` (the scan's own self-test "single-owner rules match their own examples" fails if a removed sample broke a row). The executor runs no scan. Executor's stop condition: the report lists each row with the grep that showed its pattern matched nothing; the main session's: the scan passes.

### CT-7: +0x2E0 is not a hit time (D306 item 5)

Owns `tools/ch_probe/experiments/live.py`, `watch_window.py`, `active_probe.py`, `walk_edges.py`, `tools/ch_probe/input_driver.py`, `tools/ch_probe/tests/test_hit_window_scripts.py`, `tools/ch_probe/tests/test_engine.py`, `tools/ch_probe/experiments/results/overlay-spike/il2cpp-fields.md`.

`hit_offset_ms` (`live.py` line 166) drops its two hit-time parameters and returns the send-time offset alone; its callers at `active_probe.py` line 152 and `walk_edges.py` line 143 follow, and `walk_edges.py` keeps its +0x2E0 column as a plain reading but loses the `from_engine` marker. `watch_window.py`'s printed lines at 207, 211 and 230 say "+0x2e0" with no "hit time"; its header (lines 8 to 12) says the field is the double-lane round's start, no more than TM-14 on `docs/clone-hero/03-timing.md` proves. The test at `test_hit_window_scripts.py` line 114 becomes a test of the send-time offset, and line 191 pins the new wording. The comments wave-followups.md lists change the same way: `input_driver.py` line 12, `test_engine.py` lines 163 to 166, `active_probe.py` line 19, `il2cpp-fields.md` line 115. `ocr.py` has no such line (its line 3 is about the on-screen accuracy text), so nothing changes there. `watch_checks.py` waits for question 5.

Tests: `py -I -m pytest tools/ch_probe/tests -q` (the folder's runner, `tools/ch_probe/README.md` line 93). Stop condition: the folder passes, and a case-insensitive Grep for "hit time" in the files CT-7 owns finds no sentence that calls +0x2E0 a hit time (the executor lists each remaining hit with why it is about something else). A Grep over all of `tools/ch_probe` finds 126 hits in 22 files on 2026-11-07, most in files CT-7 does not own (`constants.py`, `interfaces.py`, `overlay_logs.py`, `test_sp_compare.py`, `watch_checks.py`, `test_watch_checks.py`, result CSVs and others). CT-7 does not touch them; its report lists the ones that call +0x2E0 a hit time, so the main session can decide on them with question 5.

### CT-8: `same_folder` resolves links (D310 item 5)

Owns `tools/ch_mcp/menu_tools.py` and `tools/ch_mcp/tests/test_menu_tools.py`. Line 71 uses `os.path.normcase(os.path.realpath(x))` on both sides, as `tools/hydra_ui_mcp.py` line 151 does, and the docstring at lines 67 and 68 names links and short names. The test at `test_menu_tools.py` line 343 gains a case: a junction made in the test's temp folder with `_winapi.CreateJunction` compares equal to its target. Tests: `py -I -m pytest tools/ch_mcp/tests/test_menu_tools.py -q`. Stop condition: the new case is red before and green after, and the file passes. The live check is LS-1.

### CT-9: the tn chip's outline (D308 item 5)

Owns `src/ui/report_window.cpp` and `tests/ui/uitest_report_windows.cpp`. The `if` at line 390 and the comment at lines 388 and 389 go, so `chip()` draws the outline for every token. The task adds one GUI test that draws a tn chip and a t0 chip with the same label text (a different text would change the glyph count) into the test window and checks the draw list grew by the same number of vertices for each; if the engine cannot read the draw list, the executor says so and the real-app capture is the check. Before merge: the real-app check described under "What a player sees", run by the main session, with captures shown to the user; if the user says to merge before it is done, the report says plainly that it is still owed (ui-testing.md case 2).

Tests the main session runs: `hydra_uitest.exe --test report-window-row-click` and the new test by name, then the real-app check and captures. Executor's stop condition: the edit and the new test are committed (the test alone first); the main session's: both tests pass and the captures are taken.

### CT-10: README's Stale sentence (D310 item 2)

Owns `README.md` and `tests/test_docs_match_code.cpp`. Line 57 of the README takes the ruled sentence. The docs test, which already loads `README.md` (line 127), gains a check that the README holds the ruled sentence word for word, the way `tests/single_owner/special.cpp` line 39 pins a User Guide line. The page then goes through the doc review gate. Tests the main session runs, in `fx-w2` with this branch checked out (the docs test reads the tree its exe was built from): `hydra_tests.exe "-tc=docs match code*"`. Executor's stop condition: the README line and the new check are committed on the branch; the main session then runs the test and requests the review.

## The live steps

### LS-1: the game's own folder string still matches after CT-8 (2 runs at most, 15 minutes each)

Waits for CT-8's merge and the user's go. The main session runs it (no executor, no build; the game is driven through the `ch` MCP server). The point: `same_folder` compares a folder string the game reports with the folder it was given (`menu_tools.py` lines 118 and 281), and `realpath` could change a match that works today. So this step reads that string from the game and hands it to the merged `same_folder`.

It uses the direct start, not a menu walk. The direct start is proven on this chart (P21). No saved run holds `ch_open_song` or `ch_menu` (P22), and `tools/ch_mcp/menu_steps.json` is unverified, so a walk would chain many untried presses where a step may hold one new call. Nothing is pressed, and the kit keeps the user's keys away from the game, so the user keeps the machine.

This step covers the loaded-song read at line 281. It does not exercise the Song Select read at line 118, which only a menu walk reaches. No folder on the Window Map path is a link (P34), so the run shows `realpath` does no harm to a plain path. CT-8's junction test shows it resolves a link. The caps are mine: two runs in case the first stops on a launch fault, and 15 minutes because the chart is 62 seconds long and the start takes seconds.

Calls: `ch_start("C:\Clone Hero\songs\Hydra Probe\Window Map", human=True)` (seen: LS-D1b run 1), `ch_attach` (seen: LS-D1b run 1), `ch_kit_setup(player="System")` (seen: LS-D1b run 1), `ch_menu` (never tried), `ch_stop` (seen: LS-D1b run 1)

Pass: `ch_menu` prints a folder on its "GlobalVariables currentSongFolder:" line (`tools/ch_mcp/frida_tools.py` line 307 prints it), and, after the run, the main session calls `same_folder` from the merged `tools/ch_mcp/menu_tools.py` with `py -I` (a Python call, not a game call, so it is not on the Calls line) on that string and the folder given to `ch_start`, and it returns True, every `ch_stop` check is "ok", every file and the registry key are restored or unchanged, and the foreground count is at most 1, at 0.0 s. That one event is the launch; LS-D1b run 1 shows the same (P35).

Fail: `same_folder` returns False. Then line 71 goes back to `normpath` in a new commit, the report gives the two strings, and the row goes back to the user. After two failed runs, stop and report the same.

If that line prints null, the run proves nothing either way (P33). Stop at once and bring the user that result, with no second run.

## Out of scope (needs the user's yes to leave out)

- The audit's 41 open rows, which need the game or an installer run.
- D307 item 5's removal of the eight finished plans and the lines that name them.
- The audit branch's merge itself (question 1 asks only about its order).
- The unchecked lines in `wave-followups.md` under "Stale lines no row covers".
- `watch_checks.py`'s hit-time lines (question 5), `select_chart`'s parameter (question 2) and the samples in question 4, until answered.
- `src/audio/mixer.cpp`'s comment: already as D308 item 1 asked.
- "Upgrading keeps your library and settings." in the README, which stays on the open list.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| The four screens say the ruled sentence | "What a player sees", first paragraph | CT-1 | the four named doctests, which compare each screen with the constant, plus the new literal check in `test_report_rows.cpp` |
| No code or doc still says "without folder arguments" | CT-1 stop condition | CT-1 | Grep finds nothing outside `docs/audit/` and `docs/sessions/` |
| The flag never reaches a shell command | "One rule for the executor of CT-1" | CT-1 | the hook `percall.py` `_library_refusal` refuses the call if it does; the reviewer confirms the commit message lacks it |
| The self-comparison at line 358 is gone | CT-2 | CT-2 | `-sf=*test_store_results.cpp` passes with the line removed; the reviewer reads the test |
| A quoted numstat path loses both quotes | CT-2 | CT-2 | the new direct test of the helper, red on the old code because the helper is absent, green after |
| `--ms abc` and `--depth 1x` refuse instead of meaning 0 | CT-2 | CT-2 | the new `test_cli.cpp` test that runs `hydra_replay`, red-then-green |
| `smoke.txt` has no check that cannot fail | CT-2 | CT-2 | `hydra_uitest --script` run passes without line 27 |
| `why_no_live` has no "no --live yet" branch | CT-2 | CT-2 | `pytest tools/test_hydra_ui_mcp.py` passes |
| Each removal in CT-3 has no caller | CT-3 | CT-3 | the build links; the report names each search; `-tc=single-owner*` passes |
| No pinned digest moves | CT-3 | CT-3 | `-sf=*test_perf_digest.cpp` passes unchanged |
| A DM row click leaves the bar's mode and the ini alone | "What a player sees", third paragraph | CT-4 | the new doctest, red before and green after |
| `select_chart` has no unreachable switch (if question 2 says a) | question 2 | CT-4 | the rewritten test at line 366 and `-sf=*test_app_reports.cpp` |
| No window state is static | CT-5 | CT-5 | Grep for `static DmWindow|static PathWindow` finds nothing; the new GUI test reads a fresh search box |
| Only rows whose patterns match nothing go | CT-6 | CT-6 | the report's grep per row; `-tc=single-owner*` passes |
| Rows that still catch live code wait for the user | questions 3 and 4 | CT-6 | the diff touches no row but the five named and the two sample lines |
| `hit_offset_ms` never reads +0x2E0 | CT-7 | CT-7 | the rewritten test at `test_hit_window_scripts.py` line 114 |
| `watch_window.py` prints "+0x2e0" with no "hit time" | CT-7 | CT-7 | the rewritten test at line 191; the Grep in CT-7's stop condition |
| `same_folder` resolves a junction | CT-8 | CT-8 | the new junction case, red-then-green |
| The live run takes no focus past the launch | LS-1 | LS-1 | `ch_stop` reports a foreground count of at most 1, at 0.0 s |
| Agents never build | "Who builds" | all | no brief lets an executor or reviewer build; the main session builds each branch in turn in `fx-w2`, with a pass |
| A failed live run reverts the line | LS-1 fail line | LS-1 | the report shows the revert diff and the two folder strings |
| tn chips draw the outline; the word does not change | "What a player sees", second paragraph | CT-9 | the new GUI test's equal vertex counts; the real-app captures shown to the user |
| The README's Stale sentence has no version list | "What a player sees", fourth paragraph | CT-10 | the new check in `test_docs_match_code.cpp` |
| Every task fits one executor's caps and shares no file with a parallel task | "Tasks" | all | the file lists above; the main session checks them before launch |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: Main's decision log ends at D304 and holds no D305 to D313 | code | `docs/audit/2026-10-03-fix-decisions.md` on main, Grep for `**D30[4-9]|**D31` finds line 1707 (D304) and nothing later | checked 2026-11-07 |
| P2: The audit folder is not on main; it sits on `claude/stale-comment-scouts` at b9251f68 | code | `Test-Path` of `docs/audit/2026-10-31-stale-comments` on main is False; `git log -1` in the scouts worktree | checked 2026-11-07 |
| P3: The user answered D305 to D310 "All recommended" / "accept all. create handoff." | ruling | `docs/audit/2026-10-03-fix-decisions.md` lines 1725, 1733, 1749, 1757, 1765 on the audit branch | checked 2026-11-07 |
| P4: `kNoChartLibrary` holds the old sentence and four screens read it | code | `src/app/report.h` lines 209 to 211; `src/ui/path_report_window.cpp` line 234; `src/ui/library_jobs.cpp` lines 613 to 615; `src/app/dm_report.cpp` line 87; `src/app/fill_report.cpp` line 395 | checked 2026-11-07 |
| P5: The four tests compare with the constant, none pins the old words | code | `tests/test_report_rows.cpp` line 943; `tests/test_dm_report.cpp` line 972; `tests/test_fill_report.cpp` line 694; `tests/test_library_jobs.cpp` line 482 | checked 2026-11-07 |
| P6: A hook refuses any shell command holding the flag without a typed allow | code | `C:\Users\Patrick\.claude\gates\percall.py` lines 12 and 41 to 50 | checked 2026-11-07 |
| P7: `hit_offset_ms` reads the field; `watch_window.py` prints "hit time"; the tests pin both | code | `tools/ch_probe/experiments/live.py` lines 166 to 176; `watch_window.py` lines 207, 211, 230; `tests/test_hit_window_scripts.py` lines 114 to 120 and 190 to 191 | checked 2026-11-07 |
| P8: TM-14 proves +0x2E0 is the double-lane round's start, not a hit time | data | `docs/clone-hero/03-timing.md` TM-14 (lines 161 to 169, label "Proven in the game's code"); `tools/ch_probe/engine.py` line 231 | checked 2026-11-07 (reviewer opened TM-14 on main) |
| P9: The six small promises' targets stand as D308 item 1 says, except the mixer comment | code | `tests/test_store_results.cpp` line 358; `tools/precheck/range.cpp` line 249; `tools/replay.cpp` lines 187 and 761; `tests/ui/scripts/smoke.txt` line 27; `tools/test_hydra_ui_mcp.py` lines 619 to 620; `src/audio/mixer.cpp` lines 26 to 28 already say "each stem is copied in" | checked 2026-11-07 |
| P10: `sp_cap_converged` is read only by `song_digest.h` | code | Grep over the repo outside `docs/audit/`: `src/core/model.h` line 919, `tests/song_digest.h` line 156, one 2026-08 session card | checked 2026-11-07 |
| P11: `BatchJob::cancel` has no caller in `src/` or `tests/` | code | `src/ui/library_jobs.h` line 268; Grep of `.cancel()`/`->cancel()`: `app_state.cpp` 373, 395, 432 (view_job), 800 (dm_fetch_job), 812, 835 (dm_report_job), 958, 973 (report_job), 1378 (update_download_job); `library_dialogs.cpp` 319 and `test_app_library.cpp` 838 (scan_job); `app_state.h` 468, 496 to 498, 631 to 632, 763 give the types | checked 2026-11-07 |
| P12: `/*mode*/`, `text_width`, `rule_width` and `play0` are as D308 item 2 says | code | `tests/report_samples.cpp` line 15; `tests/ui/uitest_harness.h` lines 217 and 229, `.cpp` lines 467 to 470, Grep finds no other caller in `tests/`; `tests/ui/uitest_preview.cpp` lines 652 and 667 | checked 2026-11-07 |
| P13: The two windows keep their state in statics; `AppState` already holds per-frame state | code | `src/ui/dm_report_window.cpp` lines 27 to 43; `src/ui/path_report_window.cpp` lines 33 to 54; `src/ui/app_state.h` lines 65 to 70 and 441 | checked 2026-11-07 |
| P14: The GUI tests draw the windows with sample results and callbacks, no AppState | code | `tests/ui/uitest_report_windows.cpp` lines 96 to 125; `src/ui/report_window.h` lines 110 to 111 | checked 2026-11-07 |
| P15: The window skips the tn outline and the pages draw it | code | `src/ui/report_window.cpp` lines 388 to 394; `src/app/html_page.cpp` lines 257 to 266 | checked 2026-11-07 |
| P16: tn chips are the Squeeze "None" tier and the DM not-in-library status | code | `src/core/squeeze_rating.cpp` line 194; `src/app/dm_report.cpp` lines 172 to 176 | checked 2026-11-07 (the status's exact text is in `kStatusNotInLibrary`, not read) |
| P17: `held->cancel()` at `tests/test_library_jobs.cpp` line 626 is not a `BatchJob` | unchecked | `tests/test_library_jobs.cpp` line 626, type not read | CT-3 |
| P18: `app/d115-profile.json`'s pattern matches nothing live in `src` and `tools` | unchecked | `tests/single_owner/rules/app/d115-profile.json` lines 4 to 15, pattern not run | CT-6 |
| P19: No test runs `hydra_replay`; `tests/test_cli.cpp` runs only `hydra_batch` and `hydra_fillcompare` through `run_exe`, and `hydra_replay` is built on demand | code | `tests/test_cli.cpp` lines 1 to 6, 44 and the `TEST_CASE` list; `CMakeLists.txt` lines 817, 818 and 875 to 876; Grep of `tests/` for `parse_index` and `--index` found nothing | checked 2026-11-07 |
| P20: The five rows' patterns match nothing in `src`, `tests`, `tools` | code | one Grep of the five patterns over `{src,tests,tools}/**/*.{cpp,h,js,py}` matched only `is_song_ini` (chart_files.cpp 46 and 50, test_song_parse.cpp 927 to 929); `core.json` 1617, `tests.json` 935, `store.json` 162, `ui.json` 781, `app.json` 994 hold the patterns | checked 2026-11-07 |
| P21: `ch_start` on Window Map with `human=True`, `ch_attach`, `ch_kit_setup(player="System")` and `ch_stop` all ran on this target, and the kit's setup passed | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d1b-run1\replies.txt` lines 7, 16, 21 to 32 and 83 | checked 2026-11-07 |
| P22: No saved run holds `ch_open_song` or `ch_menu` | run | Grep of `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs` for each name finds no file | checked 2026-11-07 |
| P23: `same_folder` uses `normpath`; `hydra_ui_mcp.py` uses `realpath`; two call sites | code | `tools/ch_mcp/menu_tools.py` lines 66 to 71, 118, 281; `tools/hydra_ui_mcp.py` lines 148 to 151 | checked 2026-11-07 |
| P24: The DM click passes the comparison's mode; the path report passes the bar's; `select_chart` switches and commits | code | `src/ui/dm_report_window.cpp` line 155; `src/ui/path_report_window.cpp` lines 262 to 265; `src/ui/app_state.cpp` lines 632 to 646 | checked 2026-11-07 |
| P25: `tests/test_app_reports.cpp` line 366 pins the switch and no test calls `dm_report_input` | code | lines 364 to 401; Grep of `tests/` for `dm_report_input(` finds nothing | checked 2026-11-07 |
| P26: The README's line 57 holds the version list | code | `README.md` line 57 | checked 2026-11-07 |
| P27: The docs test loads `README.md` | code | `tests/test_docs_match_code.cpp` lines 127 to 129 | checked 2026-11-07 |
| P28: The ch_probe tests run with pytest over their folder | code | `tools/ch_probe/README.md` line 93; `tools/ch_mcp/README.md` line 91 | checked 2026-11-07 |
| P29: The single-owner scan's test names start "single-owner" | code | `tests/single_owner/scan.cpp` lines 170, 202, 478 | checked 2026-11-07 |
| P30: The GUI check cases are as named | code | `docs/agents/ui-testing.md` lines 5 to 13 | checked 2026-11-07 |
| P31: A render check runs on `testdata/input`, never a whole-library analysis, and the path report analyzes on open | ruling | `docs/agents/ui-testing.md` line 11, the user's words "you dont need a full library run to check the rendering"; D249's analyze-on-open is in the user's memory note only, so CT-9 opens the report on a scratch database, not the user's copy | checked 2026-11-07 |
| P32: The hooks refuse `--whole-library` in a commit message too, because a commit is a shell command | code | `percall.py` line 44 reads the whole `command` string of any shell tool | checked 2026-11-07 |
| P33: After a direct start, `ch_menu`'s currentSongFolder line holds the song's folder, not null | unchecked | `tools/ch_mcp/agent/menu.ts` line 324 reads `currentSongEntry`; no run has printed it on a direct start | LS-1 |
| P34: No folder on the Window Map path is a link | code | `Get-Item` on `C:\Clone Hero`, `C:\Clone Hero\songs`, `C:\Clone Hero\songs\Hydra Probe` and `C:\Clone Hero\songs\Hydra Probe\Window Map` shows an empty LinkType for each | checked 2026-11-07 |
| P35: A healthy direct start reports one foreground event, at 0.0 s | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d1b-run1\replies.txt` lines 85 and 86 | checked 2026-11-07 |
| P36: Only one C++ build runs at a time, and a reviewer gets no build of its own | ruling | the user's words "i dont accept that each reviewer needs its own build", from the session the main session's memory note `one-cold-build-at-a-time` records; the main session quotes them in the reviewer's brief and confirms | checked 2026-11-07 |
