# Fixing the tests the audit found: the task plan

Written by two planners, tasks `test-fix-plan` and `test-fix-plan-2`, session 8e6a2908, on 2026-11-18. Its input is the work list, `docs/audit/2026-11-14-test-audit/worklist.md`. The folder `docs/audit/2026-11-14-test-audit` is called AUD below.

## The goal, in the user's words

"well i still want to fix all the garbage tests, i just want to stop wasting time with more reads and get to the point where we can actually act on our findings" (session 98fa2925, as the main session quoted it in the planner's brief).

So this plan turns the work list into executor tasks. It adds no reading round and re-judges no test. Each task gets one Opus executor, one fresh reviewer, and the main session's build and tests. Nothing more.

## The counts

The plan covers 3,558 faulty rows. That is every data line of `AUD/exits/labels/labels.tsv`, and every one of them has exit 2 in `AUD/exits.md`, so none drops out as "not a problem".

Every count below was made by a script over `AUD/exits/row-files.tsv`, the row-to-file table. Each row lands in exactly one of four piles, by its keys.

| Pile | Rows | What decides it |
|---|---|---|
| Fix now | 2,459 | the row carries at least one key that breaks a filed ruling, and no live-run key |
| Waits for question 1 (live runs) | 525 | the row carries O-gamemem, F-sample, F-presslog or F-watchlog |
| Waits for question 2 (rubric keys) | 476 | every key on the row is E-echo or a K key |
| Waits for question 3 (no key) | 98 | the row has no key at all |

The piles add up to 3,558, so no row is in two piles and none is in none.

Each row also lands in exactly one task. The test-file tasks T01 to T30 hold 3,356 rows. The mop-up tasks M1 to M6 and T31 hold the other 202. That is 3,558 again. The same check holds pile by pile:

| Pile | In T01 to T30 | In M1 to M6 and T31 | Sum |
|---|---|---|---|
| Fix now | 2,301 | 158 | 2,459 |
| Waits for question 1 | 524 | 1 | 525 |
| Waits for question 2 | 434 | 42 | 476 |
| Waits for question 3 | 97 | 1 | 98 |
| All | 3,356 | 202 | 3,558 |

The numbers in each task's heading below are the same script's output, and the script summed them to the first column.

A fix-now row that also carries a rubric key gets its ruling recipe now. Its rubric recipe is applied in the same pass only if question 2 was answered yes before the task was sent.

The 20 open records in `AUD/exits/open.md` are separate from the 3,558 and wait for question 4.

The plan has 29 test-file tasks (T01 to T26 and T28 to T30; the old T27 is folded into T26 and M5). Three files are too big for one executor and are cut by line range, and the bigger tasks are cut by file, which makes 65 executor parts. There are 7 mop-up tasks (M1 to M6 and T31), 9 bug tasks (B1 to B9), 2 hooks-repo bug tasks (H1, H2), 1 check task (G1), 1 task for the open records (O1) and 3 for the visible group (V0 to V2).

## The row-to-file table

The table `AUD/exits/row-files.tsv` exists (commit 1fc5e0b1). The main session made it with `AUD/exits/row_files.py`, which reads each row's audit line and takes the test file the audit named there, from a path, a file name, a test name, the audit file's legend of short codes, or the audit file's header.

Its `how` column says how sure each line is, and this plan treats the lines in four ways.

- **3,220 rows are mapped from what the audit wrote.** The plan takes the file as given.
- **204 rows are guesses.** The script gave each the file of the nearest mapped row above it in the same audit file (`how` is `inherited`). 183 of them sit in test-file tasks and 21 in mop-up tasks. The "Guessed" column below shows where they are. A task that holds any confirms each one as its first step (see "The first step").
- **134 rows have no usable file.** 53 are `unresolved`. 62 are `notinrepo`: an abbreviation such as `sl.cpp` or `lib.cpp`, or a name such as `schedule.js` that is no path in either repo. 19 are `ambiguous`: the two hook tests that exist twice. These go to the mop-up tasks.
- **68 mapped rows name a file that is not a test file.** The audit's line named production code such as `src/core/song.cpp` or `~/.claude/gates/review_rules.py`, and the script took that name. The test that row is about lives in some test file the table does not name. This plan puts these 68 in the mop-up tasks too, because their first step is the same: find the test file. This is why the mop-up tasks hold 202 rows and not 134.

A path that starts `~/.claude/` is in the hooks repo, `C:\Users\Patrick\.claude`.

## The test-file tasks

Every task follows the same shape. The executor owns the listed test files whole, and no file is in two tasks. It opens `row-files.tsv`, takes the rows whose `test_file` is one of its files, and applies the recipe from Part 2 of the work list for each key a row carries. It changes no production code unless the task says so. It hands the branch back unbuilt.

**Parts.** A task with more than about 45 fix-now rows is cut into parts a, b, c. Each part is one executor on its own branch. Most parts are whole files, so they can run side by side. Three files hold too many rows for one executor: `tests/test_preview_boxes.cpp`, `tools/ch_mcp/tests/test_frida_attach.py` and `tools/ch_mcp/tests/test_game_tools.py`. Each is cut by line range, with the cuts on test-case starts and the first test of each part named, because line numbers move once the part before it merges. Parts of one file never run at the same time: part b starts from main after part a has merged.

**Passes.** A task may hold rows that go now and rows that wait. The file is still owned by that one task. The first pass fixes the "Fix now" rows. When a question is answered yes, the same task runs again for that answer's rows, with a fresh executor. A waiting pass with more than about 45 rows is cut into parts the same way as the first pass.

**The first step.** Before it changes anything, the executor finds each of its rows' tests in its files, by the test name or line the audit row gives. A row whose test is not in the task's files is not fixed there. The executor lists it in its report with the file it is really in, and the main session moves it to that file's task or to a mop-up task. This step matters most for the guessed rows. It is a lookup of where a test is, not a second judgement of the test.

**What an executor may run.** An executor never builds, so it runs no C++ test: `hydra_tests.exe`, `hydra_uitest.exe` and `hydra_replay.exe` are the main session's. For a Python test file it owns, it may run that one file with `py -I -m pytest <file>`. A hooks-repo executor may run only its own test file, and never all the hook tests. If one of its Python tests will not run without a built exe, it does not run it and says so in its report (premise P11).

**F-schedule rows.** Their recipe makes a file with `hydra_replay`, which an executor may not run. The executor writes the test to read the checked-in file and puts the exact command and chart in its report. The main session runs the command and adds the file before it runs the tests.

Acceptance for every test-file task and every mop-up task, checked by the reviewer and the main session:

1. Every row of the pass is either changed by its recipe, or listed in the executor's report with a one-line reason: the recipe needs data the repo lacks, or the first step found the test in another file. The reviewer checks the report's list against the pass's rows in `row-files.tsv`. A row in neither fails the task.
2. No test in the owned files still builds by hand the thing its fixed key names. The reviewer reads the diff for this.
3. The main session builds the branch and runs the tests `hydra_precheck.exe --tests` names for it. All pass.
4. No row waiting on an unanswered question was changed. The reviewer checks the diff against the pass's row list.
5. The executor's report gives the result of the first step: how many rows it found in its files, and each one it did not.

In the tables below, "Fix now" is the first pass. "After q1", "After q2" and "After q3" are the rows that wait for questions 1, 2 and 3. "Guessed" counts the rows the table placed by guess. For a file cut by line range, the counts are for the whole file and sit on its first part; see "Not done", item 1.

**C++ tests, T01 to T17.** The main session builds and runs these, and executors run nothing.

T04, T09 and T15 hold P-setting, P-name and P-message rows whose recipe reads a text from its owner. Where B6 makes that owner, they read it if B6 has merged, and otherwise report the row as waiting for B6.

### T01: 89 rows in 1 file (now 68, after question 1 0, after question 2 12, after question 3 9; 0 placed by guess)

Rows per key: P-builtchart 48, K-comment 18, K-duplicate 18, Z-cite 10, P-other 9, S-snapshot 9, F-result 6, K-invented 5, P-setting 4, K-namebody 4, K-unreachable 2, K-cantfail 2, P-name 2, P-message 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T01a | `tests/test_preview_boxes.cpp` lines 1-612 (27 test cases, from `build_time_box: timestamp, measure, tempo`) | 68 | 0 | 12 | 9 | 0 |
| T01b | `tests/test_preview_boxes.cpp` lines 613-1098 (26 test cases, from `drain box: past the length it reads the moment the time box `) | same file | | | |  |

### T02: 80 rows in 3 files (now 55, after question 1 0, after question 2 23, after question 3 2; 1 placed by guess)

Rows per key: K-comment 18, F-result 17, P-builtchart 17, K-floor 16, K-selfmade 13, S-snapshot 12, K-skip 10, P-message 10, P-other 9, Z-cite 9, E-echo 5, K-cantfail 5, K-duplicate 5, F-schedule 3, K-partial 2, P-name 2, K-nocode 2, K-namebody 1, P-onefile 1, K-unreachable 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T02a | `tests/test_replay.cpp` | 28 | 0 | 18 | 2 | 0 |
| T02a | `tests/test_replay_windows.cpp` | 4 | 0 | 2 | 0 | 0 |
| T02b | `tests/test_replay_schedule.cpp` | 23 | 0 | 3 | 0 | 1 |

### T03: 72 rows in 2 files (now 62, after question 1 0, after question 2 10, after question 3 0; 1 placed by guess)

Rows per key: P-other 30, F-result 14, K-comment 12, K-selfmade 12, S-snapshot 10, K-duplicate 10, Z-cite 9, P-setting 8, P-message 7, K-cantfail 6, P-tiers 5, K-floor 4, P-burnout 3, P-builtchart 3, F-record 2, E-echo 2, K-namebody 2, K-skip 2, K-unreachable 2.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T03a | `tests/test_activation_rows.cpp` | 35 | 0 | 8 | 0 | 0 |
| T03b | `tests/test_squeeze_rating.cpp` | 27 | 0 | 2 | 0 | 1 |

### T04: 75 rows in 3 files (now 52, after question 1 0, after question 2 14, after question 3 9; 0 placed by guess)

Rows per key: P-other 22, P-setting 14, K-duplicate 14, P-name 11, K-selfmade 8, K-cantfail 6, K-comment 4, Z-cite 3, K-unreachable 3, E-echo 2, P-message 2, K-namebody 2, F-window 2, S-snapshot 1, K-partial 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T04a | `tests/test_config.cpp` | 27 | 0 | 13 | 9 | 0 |
| T04a | `tests/test_helpers.cpp` | 4 | 0 | 0 | 0 | 0 |
| T04b | `tests/test_app_shell.cpp` | 21 | 0 | 1 | 0 | 0 |

### T05: 41 rows in 5 files (now 38, after question 1 0, after question 2 3, after question 3 0; 0 placed by guess)

Rows per key: O-reply 18, P-message 16, P-other 13, P-setting 6, F-row 4, E-echo 3, K-cantfail 3, K-floor 3, K-comment 2, S-snapshot 1, K-duplicate 1, Z-cite 1, P-name 1, F-record 1, K-partial 1, C-stub 1, K-namebody 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T05 | `tests/test_dm_report.cpp` | 28 | 0 | 2 | 0 | 0 |
| T05 | `tests/test_dm_other_speed.cpp` | 4 | 0 | 0 | 0 | 0 |
| T05 | `tests/test_update_state.cpp` | 4 | 0 | 1 | 0 | 0 |
| T05 | `tests/test_http_download.cpp` | 1 | 0 | 0 | 0 | 0 |
| T05 | `tests/test_update_release.cpp` | 1 | 0 | 0 | 0 | 0 |

### T06: 100 rows in 5 files (now 83, after question 1 0, after question 2 14, after question 3 3; 18 placed by guess)

Rows per key: F-row 41, C-analyzer 37, K-comment 15, P-message 15, P-other 12, F-record 6, K-namebody 6, O-reply 4, K-floor 3, K-cantfail 3, Z-cite 2, K-duplicate 2, E-echo 2, P-placeholder 2, S-snapshot 2, K-partial 2, P-setting 1, C-worker 1, F-db 1, K-skip 1, K-selfmade 1, P-onefile 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T06a | `tests/test_app_library.cpp` | 32 | 0 | 4 | 0 | 13 |
| T06a | `tests/test_analysis.cpp` | 11 | 0 | 3 | 1 | 0 |
| T06b | `tests/test_library_jobs.cpp` | 24 | 0 | 6 | 0 | 5 |
| T06b | `tests/test_app_reports.cpp` | 11 | 0 | 1 | 2 | 0 |
| T06b | `tests/test_song_panel_state.cpp` | 5 | 0 | 0 | 0 | 0 |

### T07: 99 rows in 6 files (now 81, after question 1 0, after question 2 17, after question 3 1; 2 placed by guess)

Rows per key: F-record 38, K-cantfail 16, P-other 16, F-row 13, Z-cite 11, F-db 8, S-snapshot 6, E-echo 5, K-selfmade 5, F-result 4, P-message 4, K-duplicate 4, K-skip 4, K-floor 4, K-namebody 4, K-comment 3, K-partial 3, P-name 3, K-unreachable 1, K-nocode 1, F-summary 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T07a | `tests/test_store_records.cpp` | 36 | 0 | 8 | 0 | 1 |
| T07a | `tests/test_store_library.cpp` | 8 | 0 | 0 | 0 | 1 |
| T07a | `tests/store_lock_holder.h` | 0 | 0 | 0 | 1 | 0 |
| T07b | `tests/test_store_open.cpp` | 19 | 0 | 3 | 0 | 0 |
| T07b | `tests/test_store_results.cpp` | 16 | 0 | 4 | 0 | 0 |
| T07b | `tests/test_chart_index.cpp` | 2 | 0 | 2 | 0 | 0 |

### T08: 132 rows in 6 files (now 122, after question 1 0, after question 2 9, after question 3 1; 36 placed by guess)

Rows per key: F-row 86, P-other 25, P-message 18, F-record 13, P-setting 12, K-duplicate 8, F-result 8, Z-cite 7, K-comment 7, K-selfmade 5, P-placeholder 4, P-tiers 4, S-snapshot 3, K-unreachable 3, E-echo 2, K-cantfail 2, K-floor 2, P-onefile 2, K-nocode 2, F-summary 1, P-burnout 1, P-name 1, K-skip 1, K-partial 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T08a | `tests/test_library_model.cpp` | 47 | 0 | 2 | 0 | 35 |
| T08b | `tests/test_library_query.cpp` | 24 | 0 | 3 | 0 | 0 |
| T08b | `tests/test_report_view.cpp` | 13 | 0 | 1 | 0 | 0 |
| T08b | `tests/report_samples.cpp` | 4 | 0 | 0 | 0 | 0 |
| T08c | `tests/test_report_rows.cpp` | 23 | 0 | 3 | 1 | 1 |
| T08c | `tests/test_report_page.cpp` | 11 | 0 | 0 | 0 | 0 |

### T09: 64 rows in 6 files (now 60, after question 1 0, after question 2 4, after question 3 0; 0 placed by guess)

Rows per key: P-message 39, F-record 13, P-setting 9, K-duplicate 6, P-other 6, K-nocode 5, F-row 4, K-comment 4, K-unreachable 2, P-name 2, P-placeholder 1, K-namebody 1, P-builtchart 1, K-selfmade 1, K-cantfail 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T09a | `tests/test_cli.cpp` | 22 | 0 | 0 | 0 | 0 |
| T09a | `tests/test_fill_report.cpp` | 21 | 0 | 1 | 0 | 0 |
| T09b | `tests/test_live_protocol.cpp` | 7 | 0 | 0 | 0 | 0 |
| T09b | `tests/test_batch_text.cpp` | 4 | 0 | 3 | 0 | 0 |
| T09b | `tests/test_run_guard.cpp` | 3 | 0 | 0 | 0 | 0 |
| T09b | `tests/test_user_messages.cpp` | 3 | 0 | 0 | 0 | 0 |

### T10: 123 rows in 6 files (now 85, after question 1 0, after question 2 34, after question 3 4; 0 placed by guess)

Rows per key: K-comment 35, Z-cite 29, P-other 29, F-result 28, S-snapshot 27, K-duplicate 11, P-setting 10, K-selfmade 10, K-partial 9, K-skip 8, K-floor 7, P-builtchart 3, E-echo 3, K-unreachable 3, F-row 3, C-stub 3, P-message 3, K-cantfail 3, K-namebody 3, P-burnout 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T10a | `tests/test_sp_meter_curve.cpp` | 24 | 0 | 3 | 0 | 0 |
| T10a | `tests/test_preview_controller.cpp` | 16 | 0 | 8 | 2 | 0 |
| T10a | `tests/test_track_state.cpp` | 4 | 0 | 10 | 0 | 0 |
| T10b | `tests/test_preview_scene.cpp` | 15 | 0 | 12 | 0 | 0 |
| T10b | `tests/test_preview_timeline.cpp` | 15 | 0 | 0 | 0 | 0 |
| T10b | `tests/test_overlay_layout.cpp` | 11 | 0 | 1 | 2 | 0 |

### T11: 85 rows in 8 files (now 83, after question 1 0, after question 2 1, after question 3 1; 0 placed by guess)

Rows per key: Z-cite 40, S-snapshot 35, P-burnout 24, P-message 23, P-pbticks 20, P-other 9, P-builtchart 6, P-allister 4, F-result 3, K-selfmade 2, K-duplicate 1, K-namebody 1, P-setting 1, E-echo 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T11a | `tests/test_pb_coach.cpp` | 24 | 0 | 0 | 1 | 0 |
| T11a | `tests/test_pb_text.cpp` | 19 | 0 | 0 | 0 | 0 |
| T11a | `tests/test_pb_state.cpp` | 1 | 0 | 0 | 0 | 0 |
| T11a | `tests/test_pb_files.cpp` | 1 | 0 | 1 | 0 | 0 |
| T11b | `tests/pb_ui/pbtest_coach.cpp` | 19 | 0 | 0 | 0 | 0 |
| T11b | `tests/pb_ui/pbtest_window.cpp` | 13 | 0 | 0 | 0 | 0 |
| T11b | `tests/pb_ui/pbtest_build.cpp` | 5 | 0 | 0 | 0 | 0 |
| T11b | `tests/pb_ui/pbtest_files.cpp` | 1 | 0 | 0 | 0 | 0 |

### T12: 112 rows in 11 files (now 89, after question 1 0, after question 2 23, after question 3 0; 0 placed by guess)

Rows per key: P-builtchart 48, S-snapshot 23, K-comment 23, Z-cite 21, K-skip 16, K-selfmade 13, K-floor 12, P-other 8, F-result 6, P-tiers 6, K-duplicate 4, K-unreachable 4, K-cantfail 3, P-message 3, K-namebody 2, P-setting 1, K-partial 1, P-name 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T12a | `tests/test_rules.cpp` | 15 | 0 | 8 | 0 | 0 |
| T12a | `tests/test_search_target.cpp` | 13 | 0 | 3 | 0 | 0 |
| T12a | `tests/test_fills_and_skips.cpp` | 11 | 0 | 3 | 0 | 0 |
| T12a | `tests/test_fill_landing.cpp` | 6 | 0 | 0 | 0 | 0 |
| T12b | `tests/test_sp_end.cpp` | 9 | 0 | 5 | 0 | 0 |
| T12b | `tests/test_tied_variants.cpp` | 9 | 0 | 2 | 0 | 0 |
| T12b | `tests/test_fill_deadline.cpp` | 8 | 0 | 1 | 0 | 0 |
| T12b | `tests/test_fast_tempo.cpp` | 7 | 0 | 1 | 0 | 0 |
| T12b | `tests/test_generated_fills.cpp` | 5 | 0 | 0 | 0 | 0 |
| T12b | `tests/test_path_costs.cpp` | 5 | 0 | 0 | 0 | 0 |
| T12b | `tests/test_sp_end_clamp.cpp` | 1 | 0 | 0 | 0 | 0 |

### T13: 151 rows in 8 files (now 129, after question 1 0, after question 2 20, after question 3 2; 47 placed by guess)

Rows per key: P-other 53, F-result 33, K-comment 22, Z-cite 18, P-setting 15, S-snapshot 14, K-selfmade 12, K-duplicate 10, F-record 7, P-message 7, P-name 6, P-burnout 6, K-unreachable 4, K-floor 4, K-namebody 4, K-skip 4, E-echo 3, P-tiers 3, K-partial 3, F-row 1, P-placeholder 1, K-cantfail 1, P-builtchart 1, K-nocode 1, P-onefile 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T13a | `tests/test_model.cpp` | 44 | 0 | 8 | 1 | 36 |
| T13b | `tests/test_highway_draw.cpp` | 27 | 0 | 2 | 1 | 11 |
| T13b | `tests/test_path_list.cpp` | 15 | 0 | 1 | 0 | 0 |
| T13c | `tests/test_stars.cpp` | 12 | 0 | 2 | 0 | 0 |
| T13c | `tests/test_saved_figure_fill.cpp` | 11 | 0 | 3 | 0 | 0 |
| T13c | `tests/test_dynamics_breakdown.cpp` | 8 | 0 | 1 | 0 | 0 |
| T13c | `tests/test_note_window.cpp` | 6 | 0 | 3 | 0 | 0 |
| T13c | `tests/test_timeline_view.cpp` | 6 | 0 | 0 | 0 | 0 |

### T14: 120 rows in 10 files (now 84, after question 1 0, after question 2 34, after question 3 2; 4 placed by guess)

Rows per key: Z-cite 33, P-other 31, S-snapshot 25, K-floor 18, P-message 17, K-skip 14, K-selfmade 11, K-comment 10, P-placeholder 7, K-cantfail 5, K-duplicate 5, K-namebody 4, E-echo 3, K-partial 2, F-record 1, K-unreachable 1, P-name 1, F-result 1, C-reader 1, C-ownchecker 1, P-tiers 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T14a | `tests/test_song_length.cpp` | 14 | 0 | 0 | 0 | 0 |
| T14a | `tests/test_stem_reader.cpp` | 12 | 0 | 1 | 2 | 0 |
| T14a | `tests/test_midi.cpp` | 11 | 0 | 0 | 0 | 0 |
| T14a | `tests/test_sng.cpp` | 8 | 0 | 3 | 0 | 0 |
| T14b | `tests/test_song_input.cpp` | 10 | 0 | 2 | 0 | 0 |
| T14b | `tests/test_song_parse.cpp` | 9 | 0 | 8 | 0 | 4 |
| T14b | `tests/test_perf_digest.cpp` | 8 | 0 | 0 | 0 | 0 |
| T14b | `tests/test_engine_sweeps.cpp` | 5 | 0 | 13 | 0 | 0 |
| T14b | `tests/test_timing.cpp` | 4 | 0 | 3 | 0 | 0 |
| T14b | `tests/test_srb.cpp` | 3 | 0 | 4 | 0 | 0 |

### T15: 85 rows in 8 files (now 72, after question 1 0, after question 2 12, after question 3 1; 5 placed by guess)

Rows per key: P-burnout 19, P-other 17, P-message 16, F-row 13, Z-cite 9, K-selfmade 6, S-snapshot 6, P-setting 6, K-floor 5, O-reply 5, K-cantfail 4, C-analyzer 4, K-duplicate 4, E-echo 3, F-record 2, P-name 2, K-skip 2, K-nocode 2, C-stub 1, F-window 1, C-ownchecker 1, P-onefile 1, K-namebody 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T15a | `tests/ui/uitest_paths.cpp` | 20 | 0 | 2 | 0 | 0 |
| T15a | `tests/ui/uitest_report_windows.cpp` | 19 | 0 | 0 | 1 | 0 |
| T15a | `tests/ui/uitest_batch_reports.cpp` | 3 | 0 | 5 | 0 | 0 |
| T15a | `tests/ui/uitest_live.cpp` | 2 | 0 | 0 | 0 | 0 |
| T15a | `tests/ui/uitest_details.cpp` | 1 | 0 | 0 | 0 | 0 |
| T15a | `tests/ui/uitest_harness.cpp` | 0 | 0 | 2 | 0 | 0 |
| T15b | `tests/ui/uitest_library.cpp` | 15 | 0 | 2 | 0 | 0 |
| T15b | `tests/ui/uitest_preview.cpp` | 12 | 0 | 1 | 0 | 5 |

### T16: 88 rows in 14 files (now 74, after question 1 0, after question 2 13, after question 3 1; 0 placed by guess)

Rows per key: P-message 42, P-other 23, C-ownchecker 22, K-skip 10, K-floor 6, Z-cite 4, P-onefile 3, O-toolout 3, K-namebody 2, F-result 2, P-setting 1, K-selfmade 1, S-snapshot 1, F-row 1, E-echo 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T16a | `tests/single_owner/scan.cpp` | 15 | 0 | 3 | 1 | 0 |
| T16a | `tests/test_precheck_pick.cpp` | 15 | 0 | 1 | 0 | 0 |
| T16a | `tests/test_docs_match_code.cpp` | 12 | 0 | 6 | 0 | 0 |
| T16a | `tests/test_precheck_check3.cpp` | 3 | 0 | 0 | 0 | 0 |
| T16b | `tests/test_precheck_git.cpp` | 6 | 0 | 1 | 0 | 0 |
| T16b | `tests/test_precheck_pick_exe.cpp` | 5 | 0 | 0 | 0 | 0 |
| T16b | `tests/test_precheck.cpp` | 4 | 0 | 0 | 0 | 0 |
| T16b | `tests/test_precheck_check24.cpp` | 4 | 0 | 0 | 0 | 0 |
| T16b | `tests/test_precheck_rows.cpp` | 4 | 0 | 1 | 0 | 0 |
| T16b | `tests/test_precheck_check1.cpp` | 2 | 0 | 0 | 0 | 0 |
| T16b | `tests/single_owner/clones.cpp` | 1 | 0 | 1 | 0 | 0 |
| T16b | `tests/test_precheck_check5.cpp` | 1 | 0 | 0 | 0 | 0 |
| T16b | `tests/test_precheck_pick_lines.cpp` | 1 | 0 | 0 | 0 | 0 |
| T16b | `tests/test_precheck_pick_names.cpp` | 1 | 0 | 0 | 0 | 0 |

### T17: 115 rows in 31 files (now 47, after question 1 0, after question 2 60, after question 3 8; 7 placed by guess)

Rows per key: K-comment 27, P-other 24, Z-cite 20, K-floor 14, K-cantfail 13, S-snapshot 12, K-duplicate 11, K-skip 8, K-selfmade 5, K-nocode 4, P-setting 3, F-row 3, E-echo 3, C-clock 3, K-namebody 3, P-name 2, F-result 2, K-unreachable 2, P-builtchart 1, K-partial 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T17a | `tests/test_theme.cpp` | 9 | 0 | 1 | 0 | 0 |
| T17a | `tests/test_list_scroll.cpp` | 5 | 0 | 0 | 0 | 3 |
| T17a | `tests/test_app_click.cpp` | 3 | 0 | 4 | 0 | 0 |
| T17a | `tests/test_long_paths.cpp` | 3 | 0 | 3 | 0 | 0 |
| T17a | `tests/test_note_shuffle.cpp` | 3 | 0 | 0 | 0 | 0 |
| T17a | `tests/test_preview_clock.cpp` | 3 | 0 | 0 | 0 | 0 |
| T17a | `tests/audio_util.h` | 2 | 0 | 3 | 0 | 4 |
| T17a | `tests/test_five_lane.cpp` | 2 | 0 | 6 | 0 | 0 |
| T17a | `tests/test_kick2x.cpp` | 2 | 0 | 0 | 0 | 0 |
| T17a | `tests/test_label_layout.cpp` | 2 | 0 | 0 | 0 | 0 |
| T17a | `tests/test_launch.cpp` | 2 | 0 | 0 | 0 | 0 |
| T17a | `tests/test_winstr.cpp` | 2 | 0 | 2 | 0 | 0 |
| T17a | `tests/test_best_finish.cpp` | 1 | 0 | 2 | 0 | 0 |
| T17a | `tests/test_display_format.cpp` | 1 | 0 | 0 | 0 | 0 |
| T17a | `tests/test_dynamics_tag.cpp` | 1 | 0 | 1 | 0 | 0 |
| T17a | `tests/test_obj_loader.cpp` | 1 | 0 | 2 | 0 | 0 |
| T17a | `tests/test_preview_golden.cpp` | 1 | 0 | 0 | 1 | 0 |
| T17a | `tests/test_preview_renderer.cpp` | 1 | 0 | 3 | 0 | 0 |
| T17a | `tests/test_scoring.cpp` | 1 | 0 | 1 | 0 | 0 |
| T17a | `tests/test_column_widths.cpp` | 0 | 0 | 16 | 0 | 0 |
| T17a | `tests/test_fitted_text_cache.cpp` | 0 | 0 | 0 | 6 | 0 |
| T17a | `tests/test_font_coverage.cpp` | 0 | 0 | 1 | 0 | 0 |
| T17a | `tests/test_fuzz_regressions.cpp` | 0 | 0 | 2 | 0 | 0 |
| T17a | `tests/test_image_decode.cpp` | 0 | 0 | 1 | 0 | 0 |
| T17a | `tests/test_layering.cpp` | 0 | 0 | 3 | 0 | 0 |
| T17a | `tests/test_library_read_order.cpp` | 0 | 0 | 1 | 0 | 0 |
| T17a | `tests/test_preview_load_progress.cpp` | 0 | 0 | 1 | 0 | 0 |
| T17a | `tests/test_text_role.cpp` | 0 | 0 | 1 | 0 | 0 |
| T17a | `tests/test_upgrade_files.cpp` | 0 | 0 | 5 | 0 | 0 |
| T17b | `tests/test_song_audio.cpp` | 1 | 0 | 1 | 1 | 0 |
| T17b | `tests/test_text_decode.cpp` | 1 | 0 | 0 | 0 | 0 |

**Clone Hero tool tests, T18 to T25.** These are Python. Many of their rows wait for question 1; the first pass fixes only the fix-now rows.

### T18: 237 rows in 1 file (now 175, after question 1 42, after question 2 19, after question 3 1; 42 placed by guess)

Rows per key: C-rig 120, P-message 113, E-echo 46, C-stub 42, O-gamemem 28, F-schedule 20, Z-cite 17, P-other 15, F-presslog 13, C-reader 13, F-record 12, C-session 12, F-row 11, O-scorestats 6, K-namebody 6, K-nocode 5, K-selfmade 5, K-comment 5, F-result 4, P-name 4, C-clock 3, C-worker 3, K-duplicate 3, S-snapshot 2, F-sample 1, K-cantfail 1, K-floor 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T18a | `tools/ch_mcp/tests/test_game_tools.py` lines 1-893 (47 test cases, from `test_session_dir_is_the_scratch_folder_the_backups_go_in`) | 175 | 42 | 19 | 1 | 42 |
| T18b | `tools/ch_mcp/tests/test_game_tools.py` lines 894-1652 (47 test cases, from `test_the_status_names_the_first_chord_while_the_run_goes_on`) | same file | | | |  |
| T18c | `tools/ch_mcp/tests/test_game_tools.py` lines 1653-2299 (47 test cases, from `test_a_run_reusing_a_log_name_starts_the_file_over`) | same file | | | |  |
| T18d | `tools/ch_mcp/tests/test_game_tools.py` lines 2300-2947 (47 test cases, from `test_an_adopted_game_starts_no_watch`) | same file | | | |  |
| T18e | `tools/ch_mcp/tests/test_game_tools.py` lines 2948-3558 (47 test cases, from `test_a_run_with_no_song_gets_the_cap_with_no_folder`) | same file | | | |  |

### T19: 62 rows in 1 file (now 50, after question 1 12, after question 2 0, after question 3 0; 1 placed by guess)

Rows per key: C-rig 37, P-message 30, C-stub 24, P-other 20, O-gamemem 12, E-echo 8, C-clock 7, C-analyzer 7, F-result 6, P-name 6, Z-cite 3, O-scorestats 2, K-duplicate 2, K-selfmade 1, K-namebody 1, C-ownchecker 1, K-nocode 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T19 | `tools/ch_mcp/tests/test_bot_run.py` | 50 | 12 | 0 | 0 | 1 |

### T20: 129 rows in 3 files (now 61, after question 1 60, after question 2 8, after question 3 0; 0 placed by guess)

Rows per key: P-message 79, O-gamemem 47, C-session 41, P-other 28, C-stub 28, E-echo 21, F-sample 17, C-rig 15, Z-cite 6, K-nocode 4, F-watchlog 4, K-duplicate 4, F-summary 3, F-schedule 2, S-snapshot 2, C-reader 2, F-result 1, O-payload 1, O-process 1, K-selfmade 1, C-clock 1, K-comment 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T20a | `tools/ch_mcp/tests/test_trace_hits.py` | 24 | 13 | 3 | 0 | 0 |
| T20a | `tools/ch_mcp/tests/test_kit_tools.py` | 14 | 19 | 3 | 0 | 0 |
| T20b | `tools/ch_mcp/tests/test_sp_tools.py` | 23 | 28 | 2 | 0 | 0 |

### T21: 274 rows in 6 files (now 126, after question 1 82, after question 2 23, after question 3 43; 0 placed by guess)

Rows per key: P-message 108, O-gamemem 80, C-session 54, E-echo 34, C-clock 33, P-other 27, C-ownchecker 22, K-nocode 16, C-stub 14, C-reader 10, K-cantfail 8, K-duplicate 8, C-worker 8, K-comment 6, K-selfmade 5, K-namebody 5, P-name 5, Z-cite 3, S-snapshot 3, C-rig 3, F-sample 2, F-result 2, K-unreachable 2, P-setting 1, O-process 1, K-skip 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T21a | `tools/ch_mcp/tests/test_frida_attach.py` lines 1-851 (47 test cases, from `test_detaches_after_a_normal_end`) | 60 | 16 | 11 | 1 | 0 |
| T21b | `tools/ch_mcp/tests/test_frida_attach.py` lines 852-1499 (46 test cases, from `test_a_failed_log_write_stops_the_log_and_not_the_reply`) | same file | | | |  |
| T21c | `tools/ch_mcp/tests/test_dump_facts.py` | 18 | 0 | 2 | 26 | 0 |
| T21c | `tools/ch_mcp/tests/test_front.py` | 14 | 0 | 2 | 5 | 0 |
| T21c | `tools/ch_mcp/tests/test_foreground.py` | 12 | 0 | 0 | 0 | 0 |
| T21d | `tools/ch_mcp/tests/test_game_files.py` | 14 | 0 | 3 | 11 | 0 |
| T21d | `tools/ch_mcp/tests/test_frida_tools.py` | 8 | 66 | 5 | 0 | 0 |

### T22: 316 rows in 16 files (now 192, after question 1 109, after question 2 15, after question 3 0; 16 placed by guess)

Rows per key: P-message 115, F-sample 67, P-other 57, C-stub 43, O-gamemem 42, E-echo 41, O-process 36, C-clock 33, C-reader 31, C-rig 28, Z-cite 18, K-duplicate 16, P-name 14, S-snapshot 9, F-schedule 6, P-setting 5, F-result 5, K-nocode 4, O-toolout 4, O-scorestats 4, C-session 3, K-namebody 3, F-summary 3, K-cantfail 3, P-onefile 2, K-partial 2, O-reply 2, C-worker 2, K-selfmade 2, C-analyzer 2, K-skip 1, K-floor 1, O-payload 1, K-comment 1, K-unreachable 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T22a | `tools/ch_mcp/tests/test_machine_recorder.py` | 42 | 3 | 7 | 0 | 0 |
| T22a | `tools/ch_mcp/tests/test_build_facts.py` | 3 | 0 | 1 | 0 | 0 |
| T22b | `tools/ch_mcp/tests/test_server.py` | 26 | 1 | 1 | 0 | 0 |
| T22b | `tools/ch_mcp/tests/test_method_watch.py` | 18 | 10 | 0 | 0 | 0 |
| T22b | `tools/ch_mcp/tests/test_chdis.py` | 1 | 0 | 0 | 0 | 0 |
| T22c | `tools/ch_mcp/tests/test_quiet.py` | 18 | 0 | 0 | 0 | 0 |
| T22c | `tools/ch_mcp/tests/test_recorder_link.py` | 18 | 5 | 0 | 0 | 0 |
| T22c | `tools/ch_mcp/tests/test_launch_patch.py` | 9 | 0 | 1 | 0 | 0 |
| T22d | `tools/ch_mcp/tests/test_press_schedule.py` | 16 | 1 | 0 | 0 | 16 |
| T22d | `tools/ch_mcp/tests/test_mono_facts.py` | 13 | 0 | 0 | 0 | 0 |
| T22d | `tools/ch_mcp/tests/test_menu_tools.py` | 8 | 22 | 1 | 0 | 0 |
| T22d | `tools/ch_mcp/tests/test_hydra_end.py` | 7 | 0 | 0 | 0 | 0 |
| T22e | `tools/ch_mcp/tests/test_compare_scorestats.py` | 5 | 0 | 0 | 0 | 0 |
| T22e | `tools/ch_mcp/tests/test_launch_core.py` | 3 | 8 | 0 | 0 | 0 |
| T22e | `tools/ch_mcp/tests/test_machine_report.py` | 3 | 59 | 3 | 0 | 0 |
| T22e | `tools/ch_mcp/tests/test_build_stamp.py` | 2 | 0 | 1 | 0 | 0 |

### T23: 173 rows in 7 files (now 79, after question 1 68, after question 2 19, after question 3 7; 2 placed by guess)

Rows per key: O-gamemem 68, C-clock 46, P-other 36, C-stub 33, P-message 27, E-echo 21, C-reader 20, P-name 13, K-comment 12, K-selfmade 8, S-snapshot 7, K-duplicate 7, Z-cite 5, K-cantfail 5, F-schedule 4, K-nocode 3, C-ownchecker 2, K-namebody 2, P-setting 2, C-rig 2, K-partial 1, K-floor 1, K-unreachable 1, P-onefile 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T23a | `tools/ch_probe/tests/test_play_chart.py` | 49 | 43 | 10 | 7 | 0 |
| T23b | `tools/ch_probe/tests/test_prep_inputs.py` | 9 | 0 | 0 | 0 | 0 |
| T23b | `tools/ch_probe/tests/test_probe_songs.py` | 7 | 1 | 0 | 0 | 1 |
| T23b | `tools/ch_probe/tests/test_probe_chart.py` | 5 | 0 | 4 | 0 | 0 |
| T23b | `tools/ch_probe/tests/test_process.py` | 4 | 7 | 4 | 0 | 0 |
| T23b | `tools/ch_probe/tests/test_process_readonly.py` | 4 | 0 | 1 | 0 | 1 |
| T23b | `tools/ch_probe/tests/test_play_in_game.py` | 1 | 17 | 0 | 0 | 0 |

### T24: 211 rows in 9 files (now 42, after question 1 148, after question 2 21, after question 3 0; 1 placed by guess)

Rows per key: O-gamemem 60, F-watchlog 59, P-message 56, P-other 38, F-result 28, E-echo 24, F-presslog 23, F-schedule 22, C-clock 17, K-duplicate 13, Z-cite 11, K-comment 10, F-sample 8, K-nocode 6, F-summary 6, K-cantfail 5, O-scorestats 5, C-stub 5, K-unreachable 4, S-snapshot 3, C-rig 3, K-selfmade 3, O-process 3, P-name 2, C-reader 2, K-floor 2, K-namebody 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T24 | `tools/ch_probe/tests/test_sp_compare.py` | 8 | 40 | 4 | 0 | 0 |
| T24 | `tools/ch_probe/tests/test_engine.py` | 7 | 18 | 2 | 0 | 0 |
| T24 | `tools/ch_probe/tests/test_find_checksum.py` | 6 | 14 | 1 | 0 | 0 |
| T24 | `tools/ch_probe/tests/test_hit_window_scripts.py` | 6 | 5 | 5 | 0 | 0 |
| T24 | `tools/ch_probe/tests/test_sp_watch.py` | 5 | 15 | 2 | 0 | 0 |
| T24 | `tools/ch_probe/tests/test_engine_finder.py` | 5 | 5 | 0 | 0 | 0 |
| T24 | `tools/ch_probe/tests/test_score_compare.py` | 3 | 21 | 2 | 0 | 0 |
| T24 | `tools/ch_probe/tests/test_debugger.py` | 2 | 11 | 4 | 0 | 0 |
| T24 | `tools/ch_probe/tests/test_watch_checks.py` | 0 | 19 | 1 | 0 | 1 |

### T25: 85 rows in 8 files (now 71, after question 1 3, after question 2 10, after question 3 1; 0 placed by guess)

Rows per key: P-other 38, P-message 18, C-stub 17, Z-cite 15, E-echo 9, O-ocr 8, C-clock 5, F-schedule 4, S-snapshot 3, K-selfmade 2, K-cantfail 2, K-nocode 2, C-ownchecker 2, K-duplicate 2, K-unreachable 1, O-gamemem 1, K-namebody 1, C-session 1, F-presslog 1, F-sample 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T25a | `tools/ch_probe/tests/test_input_driver.py` | 12 | 0 | 3 | 1 | 0 |
| T25a | `tools/ch_probe/tests/test_cheap_checks.py` | 11 | 1 | 3 | 0 | 0 |
| T25a | `tools/ch_probe/tests/test_early_fill_walk.py` | 10 | 0 | 1 | 0 | 0 |
| T25a | `tools/ch_probe/tests/test_stall_harness.py` | 9 | 0 | 2 | 0 | 0 |
| T25b | `tools/ch_probe/tests/test_ocr.py` | 8 | 0 | 0 | 0 | 0 |
| T25b | `tools/ch_probe/tests/test_overlay_logs.py` | 8 | 0 | 0 | 0 | 0 |
| T25b | `tools/ch_probe/tests/test_edge_probe.py` | 7 | 1 | 1 | 0 | 0 |
| T25b | `tools/ch_probe/tests/test_s2_window_constants.py` | 6 | 1 | 0 | 0 | 0 |

**Hydra's other tool tests, T26.** Python, and every row is fix-now.

### T26: 69 rows in 9 files (now 69, after question 1 0, after question 2 0, after question 3 0; 0 placed by guess)

Rows per key: O-transcript 27, P-message 24, P-other 11, F-db 7, S-snapshot 6, P-name 4, Z-cite 2.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T26a | `tools/test_effort_test_taint.py` | 24 | 0 | 0 | 0 | 0 |
| T26a | `tools/test_hydra_ui_mcp.py` | 13 | 0 | 0 | 0 | 0 |
| T26a | `tools/test_compare_db.py` | 8 | 0 | 0 | 0 | 0 |
| T26b | `tools/test_effort_test_shuffle.py` | 9 | 0 | 0 | 0 | 0 |
| T26b | `tools/test_agent_cost.py` | 4 | 0 | 0 | 0 | 0 |
| T26b | `tools/test_effort_test_faults.py` | 4 | 0 | 0 | 0 | 0 |
| T26b | `tools/test_check_handoff.py` | 3 | 0 | 0 | 0 | 0 |
| T26b | `tools/test_check_pointers.py` | 2 | 0 | 0 | 0 | 0 |
| T26b | `tools/test_effort_test_scout.py` | 2 | 0 | 0 | 0 | 0 |

**Hooks-repo tests, T28 to T30.** These run in a worktree of `C:\Users\Patrick\.claude`, never in Hydra's.

### T28: 102 rows in 4 files (now 90, after question 1 0, after question 2 11, after question 3 1; 0 placed by guess)

Rows per key: P-message 45, F-verdict 33, C-gate 33, E-echo 17, O-transcript 11, C-stub 5, S-snapshot 3, K-cantfail 3, O-payload 3, Z-cite 2, K-duplicate 1, K-namebody 1, P-name 1, P-other 1, K-selfmade 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T28a | `~/.claude/gates/tests/test_review_rules.py` | 38 | 0 | 4 | 0 | 0 |
| T28a | `~/.claude/gates/tests/test_refusals.py` | 5 | 0 | 0 | 0 | 0 |
| T28b | `~/.claude/gates/tests/test_ref_judge.py` | 32 | 0 | 1 | 1 | 0 |
| T28c | `~/.claude/gates/tests/test_review_dispatch.py` | 15 | 0 | 6 | 0 | 0 |

### T29: 55 rows in 6 files (now 53, after question 1 0, after question 2 2, after question 3 0; 0 placed by guess)

Rows per key: P-other 28, P-message 20, S-snapshot 8, K-cantfail 2, K-duplicate 1, O-payload 1, K-namebody 1, O-transcript 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T29a | `~/.claude/gates/tests/test_progress.py` | 19 | 0 | 2 | 0 | 0 |
| T29a | `~/.claude/gates/tests/test_shell_output.py` | 13 | 0 | 0 | 0 | 0 |
| T29a | `~/.claude/gates/tests/test_stall.py` | 8 | 0 | 0 | 0 | 0 |
| T29a | `~/.claude/gates/tests/test_percall_rewrite.py` | 5 | 0 | 0 | 0 | 0 |
| T29b | `~/.claude/gates/tests/test_agent_stop.py` | 7 | 0 | 0 | 0 | 0 |
| T29b | `~/.claude/gates/tests/test_percall_key.py` | 1 | 0 | 0 | 0 | 0 |

### T30: 12 rows in 8 files (now 9, after question 1 0, after question 2 3, after question 3 0; 0 placed by guess)

Rows per key: P-other 5, P-message 3, K-duplicate 2, K-cantfail 2, P-name 1, K-namebody 1.

| Part | File | Fix now | After q1 | After q2 | After q3 | Guessed |
|---|---|---|---|---|---|---|
| T30 | `~/.claude/gates/tests/test_agent_files.py` | 2 | 0 | 0 | 0 | 0 |
| T30 | `~/.claude/gates/tests/test_docs_numbers.py` | 2 | 0 | 0 | 0 | 0 |
| T30 | `~/.claude/gates/tests/test_posture.py` | 2 | 0 | 0 | 0 | 0 |
| T30 | `~/.claude/gates/tests/test_publish.py` | 2 | 0 | 0 | 0 | 0 |
| T30 | `~/.claude/gates/tests/test_bash_windows_rule.py` | 1 | 0 | 0 | 0 | 0 |
| T30 | `~/.claude/gates/tests/test_common.py` | 0 | 0 | 1 | 0 | 0 |
| T30 | `~/.claude/gates/tests/test_delete_rule.py` | 0 | 0 | 1 | 0 | 0 |
| T30 | `~/.claude/gates/tests/test_ruling_rules.py` | 0 | 0 | 1 | 0 | 0 |

## The mop-up tasks (M1 to M6 and T31)

These hold the 202 rows whose test file the table does not give. The table below lists them by audit file, as the script counted them. Its last column is what `row-files.tsv` holds in place of a test file. The tasks under it group the audit files, and their row counts are sums of this table's lines.

| Kind | Audit file | Rows | Fix now | After q1 | After q2 | After q3 | What the table holds |
|---|---|---|---|---|---|---|---|
| twice in the hooks repo | `records/E4.md` | 19 | 13 | 0 | 6 | 0 | ~/.claude/gates/tests/test_h52_cleanup_sweep.ps1 or ~/.claude/h (11); ~/.claude/gates/tests/test_h53_session_cards_start.ps1 or ~/.cl (8) |
| names a file that is no test | `counts/T5-IR15.md` | 6 | 6 | 0 | 0 | 0 | docs/audit/note-shuffle/probes/vectors/gen_vectors.py (5); docs/audit/note-shuffle/verify/check_video.py (1) |
| names a file that is no test | `counts/T5-IR4.md` | 12 | 12 | 0 | 0 | 0 | ~/.claude/gates/review_rules.py (11); ~/.claude/gates/ruling_rules.py (1) |
| names a file that is no test | `counts/T5-KR1.md` | 1 | 1 | 0 | 0 | 0 | tools/ch_mcp/mono_reader.ps1 (1) |
| names a file that is no test | `records/B2.md` | 19 | 12 | 0 | 7 | 0 | src/core/song.cpp (7); src/parse/midi.cpp (7); src/search/pathstring_read.cpp (5) |
| names a file that is no test | `records/B4.md` | 9 | 4 | 0 | 5 | 0 | tools/replay_schedule.cpp (6); src/core/model.cpp (2); tools/replay_schedule.h (1) |
| names a file that is no test | `records/C1.md` | 10 | 5 | 0 | 5 | 0 | src/ui/badge_pill.h (7); src/ui/paths_tab.cpp (1); src/app/config.cpp (1); src/ui/app_state.cpp (1) |
| names a file that is no test | `records/C10.md` | 5 | 2 | 0 | 3 | 0 | src/ui/preview_transport.cpp (2); src/render/preview_config.cpp (1); src/app/rules_file.cpp (1); src/parse/section_names.h (1) |
| names a file that is no test | `records/D13.md` | 4 | 2 | 1 | 1 | 0 | tools/ch_probe/experiments/sp_compare.py (1); tools/ch_probe/engine.py (1); tools/ch_probe/experiments/sp_watch.py (1); tools/agent_cost.py (1) |
| names a file that is no test | `records/E4.md` | 2 | 2 | 0 | 0 | 0 | ~/.claude/tools/session_cards/check.py (1); ~/.claude/tools/session_cards/batch.py (1) |
| named by a file not in the repo | `counts/T5-IR1.md` | 1 | 1 | 0 | 0 | 0 | src/calc.py (1) |
| named by a file not in the repo | `counts/T5-IR13.md` | 1 | 1 | 0 | 0 | 0 | launch.js (1) |
| named by a file not in the repo | `counts/T5-IR19.md` | 54 | 53 | 0 | 1 | 0 | sl.cpp (15); so.cpp (15); lib.cpp (12); dyn.cpp (6); rep.cpp (5); click.cpp (1) |
| named by a file not in the repo | `counts/T5-IR2.md` | 1 | 1 | 0 | 0 | 0 | builds-N.js (1) |
| named by a file not in the repo | `counts/T5-IR7.md` | 1 | 1 | 0 | 0 | 0 | schedule.js (1) |
| named by a file not in the repo | `counts/T5-KR1.md` | 1 | 1 | 0 | 0 | 0 | s.js (1) |
| named by a file not in the repo | `records/B4.md` | 2 | 2 | 0 | 0 | 0 | pitch_dark_ratio.schedule.js (2) |
| named by a file not in the repo | `records/D13.md` | 1 | 1 | 0 | 0 | 0 | mouths_refine_schedule.js (1) |
| no file found | `counts/T5-KR4.md` | 3 | 3 | 0 | 0 | 0 | :188 (1); :215 (1); :430 (1) |
| no file found | `records/A39.md` | 11 | 8 | 0 | 2 | 1 | test_dynamics: the Dynamics tab on Acid Romance shows its ki (1); test_dynamics_reopen: the Dynamics count comes from the clic (1); test_dynamics_hard: at Hard, the Dynamics tab counts Hard's  (1); test_stars: the Stars tab shows nothing while the analysis r (1); test_details_close_teardown: hiding the details panel by any (1); test_preview_load_bar: the Preview's loading bar shows one o (1); test_panel_open_close: the panel docks beside the library an (1); test_layout_sweep: on four charts with long titles, at the n (1) |
| no file found | `records/B2.md` | 4 | 4 | 0 | 0 | 0 | Read: lines 62-73 and the helper song_with_chords (29-46); S (1); Read: lines 88-91; S-G.  (1); Read: lines 93-96; S-G; FL-16.  (1); Read: lines 98-103; S-G; FL-16 (D115 item 4, cited in the co (1) |
| no file found | `records/C1.md` | 10 | 6 | 0 | 4 | 0 | Read: lines 92-99; layout header lines 52-112. Adding commit (1); Read: lines 101-106; layout lines 69-77, 110. Adding commit  (1); Read: lines 108-121; layout lines 78-111; the plan named in  (1); Read: lines 123-133; layout lines 85-110. Adding commit 1694 (1); Read: lines 135-145; layout lines 84-86. Adding commit 3f46a (1); Read: lines 147-157; layout lines 70-72,85; D262 (opened, li (1); Read: lines 159-186; layout lines 66,103-108. Adding commit  (1); Read: lines 188-207; layout lines 81-100. Adding commit 1694 (1) |
| no file found | `records/C10.md` | 1 | 1 | 0 | 0 | 0 | Read: lines 1-98 (helpers check_color, check_light, check_is (1) |
| no file found | `records/D15.md` | 9 | 7 | 0 | 2 | 0 | BP-3 the reply names the pass's limit and the command to bui (1); BP-13 off the gate, builds as before (`:235-246`)  (1); BP-14 with a pass: claim before configure, low priority on t (1); BP-15 a Ninja preset has no low-priority switch (`:263-270`) (1); CF-3 an ancestor record, one changed file (`:86-97`)  (1); MP-2 `Select-Spread` (`:112-114`) 112 `Select-Spread 6 3` gi (1); MP-4 a normal run (`:213-222`) 215 exit 0. 218 each edit's r (1); MP-8 under the gate with a pass, one pass holds across all r (1) |
| no file found | `records/window-WN3.md` | 14 | 8 | 0 | 6 | 0 | Delta round: window, replaces A13.md record 2. The old verdi (1); Delta round: window, replaces A13.md record 3. The old verdi (1); Delta round: window, replaces A13.md record 9. The old verdi (1); Delta round: window, replaces A14.md record 28. The old verd (1); Delta round: window, replaces A14.md record 29. The old verd (1); Delta round: window, replaces A14.md record 30. The old verd (1); Delta round: window, replaces A14.md record 31. The old verd (1); Delta round: window, replaces A14.md record 32. The old verd (1) |
| no file found | `records/window-WN8.md` | 1 | 1 | 0 | 0 | 0 | Delta round: window, replaces records/A10.md record 1  (1) |

Each mop-up task's first step finds the test file for each of its rows, from the audit line the row names. A fix then goes into a file some test-file task owns. That is why the mop-up tasks run only after that file's first pass has merged: at that moment no other task is touching the file. A row whose test cannot be found is listed in the report, never dropped. The rest of the shape and the acceptance list are the same as for the test-file tasks.

### M1: the `counts/T5-IR19.md` rows named by abbreviation (54 rows)

The table holds abbreviations here: `sl.cpp`, `so.cpp`, `lib.cpp`, `dyn.cpp`, `rep.cpp` and `click.cpp`. The audit file's own text says which test file each stands for. 53 rows are fix-now and 1 waits for question 2. Two parts: M1a takes the 30 `sl.cpp` and `so.cpp` rows, M1b the other 24. The files will be C++ tests, so the executor runs nothing.

### M2: the other count-file rows (15 rows)

Five rows named by a file that is no path in the repo (`counts/T5-IR1.md`, `T5-IR13.md`, `T5-IR2.md`, `T5-IR7.md`, `T5-KR1.md`), three unresolved rows of `counts/T5-KR4.md`, six rows of `counts/T5-IR15.md` that name a probe script under `docs/audit/note-shuffle/`, and one row of `counts/T5-KR1.md` that names `tools/ch_mcp/mono_reader.ps1`. All 15 are fix-now.

### M3: the `records/B2.md` and `records/B4.md` rows (34 rows)

`records/B2.md` has 19 rows that name `src/core/song.cpp`, `src/parse/midi.cpp` or `src/search/pathstring_read.cpp`, and 4 unresolved rows. `records/B4.md` has 9 rows that name `tools/replay_schedule.cpp`, `tools/replay_schedule.h` or `src/core/model.cpp`, and 2 named by a `.js` file that is not in the repo. 22 rows are fix-now and 12 wait for question 2.

### M4: the `records/C1.md`, `C10.md`, `A39.md`, `window-WN3.md` and `window-WN8.md` rows (52 rows)

`records/C1.md` has 10 rows that name code under `src/` and 10 unresolved. `records/C10.md` has 5 and 1. `records/A39.md` has 11 unresolved rows whose text gives a UI test name such as `test_dynamics`. `records/window-WN3.md` has 14 and `records/window-WN8.md` has 1; each says which older record it replaces, and that record names the file. 31 rows are fix-now, 20 wait for question 2 and 1 waits for question 3. Two parts: M4a takes the 26 rows of C1 and C10, M4b the 26 rows of A39 and the two window files.

### M5: the `records/D13.md` and `records/D15.md` rows (14 rows)

`records/D15.md` has 9 unresolved rows with codes BP, CF, MP and RS; they are the PowerShell tool tests the earlier plan called T27. `records/D13.md` has 4 rows that name a tool under `tools/` and 1 named by a `.js` file. 10 rows are fix-now, 1 waits for question 1 and 3 wait for question 2. The executor may run a PowerShell test file it changed only if that file starts no build.

### M6: hooks-repo rows that name a gate, not its test (14 rows)

`counts/T5-IR4.md` has 12 rows that name `~/.claude/gates/review_rules.py` or `ruling_rules.py`. `records/E4.md` has 2 that name `~/.claude/tools/session_cards/check.py` and `batch.py`. All 14 are fix-now. This task runs in a worktree of the hooks repo, after T28 to T30.

### T31: the two hook tests that exist twice (19 rows)

`test_h52_cleanup_sweep.ps1` (11 rows) and `test_h53_session_cards_start.ps1` (8 rows) are each in both `gates\tests` and `hooks\tests` of the hooks repo. 13 rows are fix-now and 6 wait for question 2. `Waits for: question 5`, which says which copy is live.

### O1: the 20 open records

The 20 records in `AUD/exits/open.md` are not in the 3,558. After a yes to question 4 the main session settles them as that question describes, and each one that turns out faulty joins its file's task. `Waits for: question 4`.

## The bug tasks from Part 1

Each of these changes production code and no number or word a player sees. If an executor finds that its change would alter one, it stops and reports, and the item joins the visible group.

No planner opened the production lines the work list names for these items (premise P6). So every bug task starts the same way: the executor opens each line its items name, and stops and reports if a line does not say what the work list says.

### B1: remove the test-only switches (item 18)

Removes the test-only switches from the store and the jobs, once no test calls them. `Waits for: T06, T07` (their first passes).

### B2: delete the code only tests call (item 19)

Deletes that code and the tests that pin only it. A test that reaches real code through a dead wrapper moves to the real function. `assign_score`'s removal changes words in `CONTEXT.md:518`; the executor does not edit that file and reports the line for the user. It edits tests in files other tasks own. `Waits for: T02, T03, T13, T14` (their first passes), and it runs while no pass on those files is out.

### B3: the Clone Hero tools (items 21, 27, 29, 30, 35)

Dead readers, plurals, the two route-word copies, the copied multiplier, the refuted +0x2E0 check with its D13 tests, and a source comment on each unnamed number. `Waits for: T22, T23, T24, T25` (their first passes).

### B4: the precheck, the quiet check and the guard script (items 23, 24, 28)

The precheck's flag name, its case-blind member count and its repeated mask; the quiet check's worktree folder; the guard script's home-folder default.

### B5: wrong comments and stale lists (item 22)

Corrects them. Nothing else changes.

### B6: one owner for each repeated text (items 14, 15, 16, 17)

Gives each ownerless screen sentence one owner, states the short-beat refusal once, and names the SQLite setting. From item 14 it takes the settings names: "Pro Drums", "2x Bass" and "Note Shuffle" get one owner, which `batch_settings_summary` and `change_phrase` in `library_dialogs.cpp` both read. No text changes. The fills spelling in item 14 does change text, so it is not here; it is in the visible group.

### B7: the squeeze rating figures (item 7)

Moves them from display code to the engine. No number may change; a changed number stops the task. `Waits for: V0`. It keeps the earlier planner's call: it moves where Hydra decides a figure, so it is built from the scout's Owner map.

### B8: bank arrivals, the shared path key, the all-0 reader (items 10, 11, 12)

`Waits for: V0`, for the same reason as B7: each changes which code answers a game question.

### B9: the solo box sum and the fill page's double count (items 3, 9)

`Waits for: V0, V1`. It waits for the map for the same reason as B7, and for the run because the work list says these two are visible only if a run shows a difference. If the run shows one, the item joins the visible group.

### H1: the status script (item 25)

`status_append.ps1` reports a failed write and reads the status path from its owner. Hooks worktree.

### H2: the Bash rule (item 26)

`bash_windows_rule.py` learns the other git option spellings. Hooks worktree.

Item 20 of the work list needs no task: that code is used by a tool and stays.

Acceptance for B1 and B2: a search of `src/` and `tools/` for each removed name finds nothing, and the main session's build passes. Acceptance for B3 to B5: the main session's build passes and the tests `hydra_precheck.exe --tests` names pass. Acceptance for B6, B7 and B8: the main session's `hydra_uitest` text for the screens that read the value is the same before and after. Acceptance for H1 and H2: each gets a test in its own test file that fails on the old code, which the reviewer reads. Acceptance for every bug task: the report lists each source line opened and whether it matched the work list.

### G1: check the four unproven bugs (items 31 to 34)

One read-only scout checks each of the four once. Item 31 is checked against the MIDI rule in the parser's own cited source. Item 33 is checked against `docs/clone-hero/` first. A fault that shows becomes a bug task the main session adds. A fault that does not show is dropped, and the scout's report says which. Cap: one scout, one pass, no second look.

## The visible group (waits; nothing else waits on it)

Items 1, 2, 4, 5, 6, 8 and 13, and the fills spelling in item 14, change what a player sees.

### V0: the owner-map scout

A scout writes the Owner map for these eight items and for B7 to B9, in the shape `docs/agents/owner-map.md` gives. No planner wrote it. A row with no owner stops the item and goes to the user.

### V1: the before-and-after run

The main session runs `hydra_uitest` on corpus charts from `testdata/input` and records each screen's text before the change. The after text comes from the same run on each V2 branch. No text in this plan is invented. `Waits for: V0`.

### V2: one fix task per item

Written by a planner from V0's map. `Waits for: V0, V1, question 6`.

## Questions

Each question is in the user's hands only because no filed ruling decides it.

### Round 1

1. **May the Clone Hero tool tests be fed from real runs of the game?** 525 tests fake the game's memory, a recorder sample, a press log or a watcher log. The fix is to record those once from the real game and check the recordings in. That needs live Clone Hero runs on single songs, never the library. You would see the game start and play a song with nobody at the keyboard. Unblocks 525 rows: 524 in T18 to T25 and 1 in M5.
2. **Do the tests that break only the test rubric get fixed too?** 476 tests break no rule you have filed. They echo a value back, cannot fail, skip quietly, repeat another test, or check something other than their name says. Nothing on any screen changes either way. Unblocks 476 rows: 434 in the test-file tasks and 42 in the mop-up tasks.
3. **The 98 tests no label fits.** The audit sorted them into kinds in `AUD/exits/groups.md`. 46 of the 98 are hand-written input files that the real code reads, which your earlier answer called input, not a fake. Should those 46 stay as they are, and the other 52 (fixed sleeps, bundled tests, proxies and the rest) be fixed? Nothing on screen changes. Unblocks 98 rows: 97 in the test-file tasks and 1 in M4.
4. **May the 20 unsettled records be settled by running them?** Twelve need one test run on the checked-in songs; one of those needs your library's timing file. Six need the game's real `dump.cs` and `facts.json`. Two are scripts with no check in them that need a person watching the game; are those two tests at all? Unblocks O1.
5. **Two hook tests exist twice.** `test_h52_cleanup_sweep.ps1` and `test_h53_session_cards_start.ps1` are in both `gates\tests` and `hooks\tests` of your hooks folder. Which copy is the live one? Unblocks T31, 19 rows.

The 46 in question 3 is a count of the rows in the "Rows no key fits" table of `AUD/exits/groups.md` that carry no key and come from the audit files the work list names as hand-written input: `counts/T5-IR3.md` (22), `T5-IR17.md` (11), `T5-IR7.md` (7), `T5-IR19.md` (2), `T5-KR4.md` (1) and `T5-KR5.md` (3).

### Round 2

6. **The eight screen changes.** Asked after V0 and V1, with each screen's real text before and after. Unblocks V2.
7. **Two things you said that are not filed.** "Yes, drop it" and "why are we relying on framerate at all? this is just bad design" (session 1ac0d6b4) sit under Unconfirmed, and a tool cites them as confirmed. Filing them is your click.

Work-list question 5 is mostly not asked here, and this keeps the earlier planner's call. Whether a row is P-builtchart or P-other changes no fix, because both keys have the same kind of recipe: read the value from its owner or check the behaviour. Whether a fixed wait is K-floor changes no fix either, because the row is fixed by waiting on the event with a cap on both sides. The hand-written-input part of it does change the fix, and that part is question 3.

## Order and rounds

**Round 1 needs no answer.** It is the first pass of every test-file part, plus B4, B5, B6, H1, H2, G1 and V0. Three or four executors run at once. When one finishes, the next on this list starts. The list is biggest first, by fix-now rows:

T19 (50), T23a (49), T08a (47), then the parts with 45: T11a, T12a, T14a, T15a, T16a, T17a, T22a, T22b, T22c, T26a, T29a. Then T07a, T10a, T12b, T13a, T21c and T22d (44 each), T06a, T09a, T13c and T28a (43), T13b, T24 and T25a (42), T08b and T10b (41), T06b (40), T14b (39), T05, T11b and T20a (38), T07b (37), T03a and T18a (35), T01a and T08c (34), T02a and T28b (32), T04a (31), T21a and T23b (30), T16b and T25b (29), T03b and T15b (27), T26b (24), T02b and T20b (23), T21d (22), T04b (21), T09b (17), T28c (15), T22e (13), T30 (9), T29b (8), T17b (2).

The three files cut by line range are averaged in that list, at 35 rows a part for T18, 34 for T01 and 30 for T21's `test_frida_attach.py`. Their later parts go in order, each after the one before it has merged: T01b after T01a; T21b after T21a; T18b, T18c, T18d and T18e after T18a.

No two parts in the list share a file, so any three or four of them can run together. B4, B5, B6, H1, H2, G1 and V0 touch no test file a part owns and start in the first wave beside them. B6 goes first among them, because T04, T09 and T15 read its owners.

**Round 2 is the mop-up tasks M1 to M6**, after the first passes they depend on have merged, and B1, B2 and B3, each after the tasks its `Waits for:` line names.

**Then each answer's tasks.** A yes to question 1 first needs a short plan for the recording runs, with live steps written to the call-list rule on `docs/agents/premise-map.md`; the after-q1 passes of T18 to T25 and M5 wait for those recordings. A yes to question 2 sends the after-q2 pass of every task that has one. An answer to question 3 sends the after-q3 passes. Question 4 sends O1. Question 5 sends T31. A pass for an answer never runs while another pass on the same file is out.

The visible group runs beside all of this. Only B7, B8 and B9 wait on it.

The main session merges each part after its one reviewer signs off.

## Not done

1. **The three files cut by line range have no row count per part.** `row-files.tsv` names each row's test file but not the test's line in it. So T01, T18 and T21's `test_frida_attach.py` have a count for the whole file and drawn ranges, but not a count for each range. The cuts give each part the same number of test cases. Each part's executor learns its rows in the first step.
2. **The after-answer passes are counted but not cut into parts.** Some are large, such as 66 rows in `test_frida_tools.py` and 59 in `test_machine_report.py` after question 1. They are cut when the answer comes.
3. **Premises P5, P6 and P11 are still unchecked.** Settling P5 or P6 by reading would be another reading round over 3,558 rows or 35 bug items, which the user's words rule out. Each is settled by the first step of the tasks named in its row.

## Out of scope, for the user to approve

Fixing the hooks named in work-list item 34 (the key lock and `prepass.py`) beyond G1's check.

## Promise map

| Promise | Plan section | Task | The check that proves it |
|---|---|---|---|
| Every faulty row is in one pile and one task | The counts | T01 to T30, M1 to M6, T31 | the script's sums: 3,356 + 202 = 3,558, and each pile's two parts sum to its total |
| No test file is in two tasks | The test-file tasks | T01 to T30 | the script that made the tables stops if a file is named twice; the reviewer greps a file name in the plan |
| Parts of one file never run together | Parts; Order and rounds | T01, T18, T21 | part b's branch is cut from main after part a's merge |
| Each row's test is found before it is changed, and a guessed row that is elsewhere is moved | The first step | every test-file and mop-up task | acceptance 5: the report's found and not-found lists |
| Every row of a pass is fixed or reported | acceptance 1 | every test-file and mop-up task | reviewer compares the report's list with the pass's rows in `row-files.tsv` |
| No hand-built stand-in is left for a fixed key | acceptance 2 | every test-file and mop-up task | reviewer reads the diff |
| The branch builds and its tests pass | acceptance 3 | every code task | main session's build and the `hydra_precheck.exe --tests` list |
| No waiting row is changed before its answer | acceptance 4 | every test-file and mop-up task | reviewer checks the diff against the pass's row list |
| An executor builds nothing and runs only its own Python or hook test file | What an executor may run | every executor | the build hook refuses a build; the reviewer reads the report's test commands |
| An F-schedule file is made by the main session, not typed | F-schedule rows | tasks with F-schedule rows | the report gives the command; the main session runs it before the tests |
| A mop-up fix never meets another task in the same file | The mop-up tasks | M1 to M6 | each runs after that file's first pass merged |
| A bug task opens its source lines first and stops on a mismatch | The bug tasks | B1 to B9, H1, H2 | the report lists each line opened and whether it matched |
| A bug task changes nothing a player sees | The bug tasks | B1 to B8 | `hydra_uitest` text the same before and after; a difference stops the task |
| Removed code has no caller left | The bug tasks | B1, B2 | search of `src/` and `tools/` finds nothing; the build passes |
| The settings names get one owner and no text changes | B6 | B6 | `hydra_uitest` text of the batch confirm the same before and after |
| Hook fixes have a test that fails on the old code | The bug tasks | H1, H2 | reviewer reads the test |
| An unproven bug is checked once, then fixed or dropped | G1 | G1 | the scout's report names each of the four |
| Visible items wait for the map and the user | The visible group | V0, V1, V2 | V2 is not written until the map and answer 6 exist |
| No run touches the whole library | Questions 1 and 4 | every task | the whole-library hook refuses the command |
| Hooks work stays out of Hydra's worktrees | Hooks-repo tests | T28 to T31, M6, H1, H2 | the branch is in `C:\Users\Patrick\.claude` |
| Nothing waits on the visible group except B7 to B9 | Order and rounds | all | the `Waits for:` lines |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: `labels.tsv` holds 3,558 faulty rows | code | `docs/audit/2026-11-14-test-audit/exits/labels/labels.tsv` | checked 2026-11-18; `row-files.tsv` has one line for each |
| P2: every one of those rows has exit 2 | code | `docs/audit/2026-11-14-test-audit/exits.md` | checked 2026-11-18, joined on row id by the first planner |
| P3: the user's five test rulings are filed under Confirmed | ruling | `docs/agents/rulings/tests-and-runs.md` lines 9-13, on this branch | checked 2026-11-18 |
| P4: the table gives 3,220 rows a file from the audit's own words, 204 by guess, and 134 none | code | `docs/audit/2026-11-14-test-audit/exits/row-files.tsv`, column `how`; `docs/audit/2026-11-14-test-audit/exits/row_files.py` | checked 2026-11-18, counted by `how` |
| P4a: each mapped row's test really is in the file the table names | unchecked | `docs/audit/2026-11-14-test-audit/exits/row-files.tsv` only reports it; 68 mapped rows name a file that is not a test file | T01 (the first step of every test-file task, acceptance 5) |
| P5: the recipes in Part 2 of the work list fit every row under their key | unchecked | `docs/audit/2026-11-14-test-audit/worklist.md` only reports this | T01 (acceptance 1 of every task lists each row a recipe did not fit) |
| P6: each Part 1 item's source line says what the work list says | unchecked | `docs/audit/2026-11-14-test-audit/worklist.md`; no planner opened the lines | B5 (every bug task's executor opens its lines first and stops on a mismatch) |
| P7: 20 records are open, in groups of 12, 6 and 2 | code | `docs/audit/2026-11-14-test-audit/exits/open.md` | checked 2026-11-18 by the first planner |
| P8: 98 rows carry no key, and 46 of them come from the audit files the work list names as hand-written input | code | `docs/audit/2026-11-14-test-audit/exits/groups.md` line 88 and the table under it; `docs/audit/2026-11-14-test-audit/worklist.md` line 171 | checked 2026-11-18, counted by audit file |
| P9: a rubric key breaks no filed ruling, so its group is its own question | code | `docs/audit/2026-11-14-test-audit/exits/keys.md` line 23 | checked 2026-11-18 by the first planner |
| P10: the two h5x hook tests each exist in `gates/tests` and `hooks/tests` | code | `docs/audit/2026-11-14-test-audit/exits/row-files.tsv`, the 19 `ambiguous` lines, which the script wrote from `git ls-files` in `C:\Users\Patrick\.claude` | checked 2026-11-18 |
| P11: every Python test file an executor is allowed to run needs no built exe | unchecked | `docs/agents/brief-preamble.md` line 47 says so of the Python tools' tests in general; a search found the words `build-cpp` or an exe name in `tools/ch_mcp/tests/test_quiet.py`, `tools/ch_probe/tests/test_prep_inputs.py` and `tools/test_hydra_ui_mcp.py`, not opened | T22 (it owns `test_quiet.py`; T23 and T26 own the other two; each reports a test that needs a build and does not run it) |
| P12: the user wants every bad test fixed with no further reading rounds | ruling | "well i still want to fix all the garbage tests, i just want to stop wasting time with more reads and get to the point where we can actually act on our findings", session 98fa2925, as the main session's brief quotes it | checked 2026-11-18 |

There are no live steps in this plan. A yes to question 1 adds them, and whoever writes them follows the call-list rule on `docs/agents/premise-map.md`.
