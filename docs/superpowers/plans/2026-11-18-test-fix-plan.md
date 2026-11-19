# Fixing the tests the audit found: the task plan

Written by the planner, task `test-fix-plan`, session 8e6a2908, on 2026-11-18. Its input is the work list, `docs/audit/2026-11-14-test-audit/worklist.md`. The folder `docs/audit/2026-11-14-test-audit` is called AUD below.

**This plan is not finished.** The planner ran out of its time budget. The section "Not done" near the end says exactly what is missing. Read it before you approve anything.

## The goal, in the user's words

"well i still want to fix all the garbage tests, i just want to stop wasting time with more reads and get to the point where we can actually act on our findings" (session 98fa2925, as the main session quoted it in the planner's brief).

So this plan turns the work list into executor tasks. It adds no reading round and re-judges no test. Each task gets one Opus executor, one fresh reviewer, and the main session's build and tests. Nothing more.

## The counts

The plan covers 3,558 faulty rows. That is every data line of `AUD/exits/labels/labels.tsv`, and every one of them has exit 2 in `AUD/exits.md`, so none drops out as "not a problem".

The planner joined each row to its test file with a scratch script (the rule is under "How rows were joined to files"). Each row then landed in exactly one of four piles. The piles add up to 3,558, so no row is in two piles and none is in none.

| Pile | Rows | What decides it |
|---|---|---|
| Ready to fix | 2,459 | the row carries at least one key that breaks a filed ruling, and no live-run key |
| Waits for question 1 (live runs) | 525 | the row carries O-gamemem, F-sample, F-presslog or F-watchlog |
| Waits for question 2 (rubric keys) | 476 | every key on the row is E-echo or a K key |
| Waits for question 3 (no key) | 98 | the row has no key at all |

A ready row that also carries a rubric key gets its ruling recipe now. Its rubric recipe is applied in the same task only if question 2 was answered yes before the task was sent.

The 20 open records in `AUD/exits/open.md` are separate from the 3,558 and wait for question 4.

The plan has 46 tasks: 31 test-file tasks (T01 to T31), 9 bug tasks (B1 to B9), 2 hooks-repo tasks (H1, H2), 1 check task for the unproven bugs (G1), and 3 for the visible group (V0 to V2).

## How rows were joined to files

For each row id in `labels.tsv`, the script found the ledger line with the same id (the ledger's `id` column, or `file:line` where `id` is empty). It took the first test file path named in that line's `mut_test`, `heading_or_section`, `record` or `test` column. It matched bare file names against `git ls-files` in this repo and in `C:\Users\Patrick\.claude`.

That named an exact test file for 2,377 rows. For 19 more it named a file that exists twice in the hooks repo (`gates/tests` and `hooks/tests`). For 1,162 rows the ledger names no test file: it gives only a short code such as `PG:140` or `TP:184`, or the record file such as `D2.md`. Those rows are grouped by the audit file they come from. This is premise P4, and it is unchecked. Task T00 settles it.

## T00: finish the row-to-file table (main session, a script, no agent)

The main session extends the join so every row names its test file and line. The short codes are spelled out at the top of each count file under `AUD/counts/`, and each record file under `AUD/records/` names its test file in its header. The result is one file, `AUD/exits/row-files.tsv`, with the row id, the test file, the line and the keys.

Acceptance: the file has 3,558 lines, no empty test file, and every test file exists in `git ls-files` of one of the two repos. This is a lookup, not a re-read of any test. Tasks marked `Needs: T00` below are not sent until it exists.

## The test-file tasks

Every task follows the same shape. The executor owns the listed test files whole. It opens `row-files.tsv` (or, for an exact-file task, `labels.tsv` joined as above), takes its rows, and applies the recipe from Part 2 of the work list for each key a row carries. It changes no production code unless the task says so. It hands the branch back unbuilt.

Acceptance for every test-file task, checked by the reviewer and the main session:

1. Every row the task owns is either changed by its recipe or listed in the executor's report with the one-line reason it could not be (a recipe that needs data the repo lacks). The reviewer checks the report's list against the task's row list. A row in neither fails the task.
2. No test in the owned files still builds by hand the thing its key names. The reviewer reads the diff for this.
3. The main session builds the branch and runs the tests `hydra_precheck.exe --tests` names for it. All pass.
4. No row waiting on a question was touched. The reviewer checks the diff against the wait piles.

Row counts below come from the planner's join. "Ready" is the ready pile only. A file with more than about 40 ready rows is split by test-case line ranges into parts a, b, c, each one executor; the split points are set when T00's table gives the lines.

### C++ tests (the main session builds and runs these; executors run nothing)

| Task | Owned files | Ready rows | Main keys | Needs |
|---|---|---|---|---|
| T01 | `tests/test_preview_boxes.cpp` (two parts) | 68 | P-builtchart, Z-cite, S-snapshot, F-result | none |
| T02 | `tests/test_replay.cpp`, `tests/test_replay_schedule.cpp`, `tests/test_replay_windows.cpp` | 60 | S-snapshot, P-builtchart, F-result, F-schedule | none |
| T03 | `tests/test_activation_rows.cpp`, `tests/test_activation_row_layout.cpp`, `tests/test_squeeze_rating.cpp` | 67 | P-other, P-setting, F-result, P-tiers | none |
| T04 | `tests/test_config.cpp`, `tests/test_app_shell.cpp`, `tests/test_helpers.cpp` | 47 | P-setting, P-name, P-other | T00 |
| T05 | `tests/test_dm_report.cpp`, `tests/test_dm_other_speed.cpp`, `tests/test_http_download.cpp`, `tests/test_update_state.cpp`, `tests/test_update_release.cpp` | 38 | O-reply, P-message | none |
| T06 | `tests/test_library_jobs.cpp`, `tests/test_app_library.cpp`, `tests/test_analysis.cpp` | 49 | C-analyzer, F-row, P-message | T00 |
| T07 | `tests/test_store_records.cpp`, `tests/test_store_open.cpp`, `tests/test_store_library.cpp`, `tests/test_store_results.cpp`, `tests/store_lock_holder.h`, `tests/test_chart_index.cpp` | 48 | F-db, F-record, F-row | T00 |
| T08 | `tests/test_library_query.cpp`, `tests/test_library_model.cpp`, `tests/test_report_rows.cpp`, `tests/test_report_view.cpp`, `tests/test_report_page.cpp`, `tests/report_samples.cpp` | 77 (two parts) | F-row, F-record, P-message | T00 |
| T09 | `tests/test_fill_report.cpp`, `tests/test_run_guard.cpp`, `tests/test_cli.cpp`, `tests/test_batch_text.cpp`, `tests/test_user_messages.cpp` | 68 (two parts) | P-message, F-record, P-tiers | T00 |
| T10 | `tests/test_preview_scene.cpp`, `tests/test_preview_controller.cpp`, `tests/test_preview_timeline.cpp`, `tests/test_sp_meter_curve.cpp`, `tests/test_track_state.cpp`, `tests/test_overlay_layout.cpp` | 74 (two parts) | Z-cite, S-snapshot, F-result | T00 |
| T11 | `tests/test_pb_coach.cpp`, `tests/test_pb_text.cpp`, `tests/test_pb_state.cpp`, `tests/test_pb_files.cpp`, `tests/pb_ui/pbtest_coach.cpp`, `tests/pb_ui/pbtest_window.cpp`, `tests/pb_ui/pbtest_build.cpp`, `tests/pb_ui/pbtest_files.cpp` | 83 (two parts) | P-burnout, P-pbticks, P-allister, S-snapshot, Z-cite | T00 |
| T12 | `tests/test_fills_and_skips.cpp`, `tests/test_generated_fills.cpp`, `tests/test_fill_deadline.cpp`, `tests/test_fill_landing.cpp`, `tests/test_fast_tempo.cpp`, `tests/test_sp_end.cpp`, `tests/test_sp_end_clamp.cpp`, `tests/test_tied_variants.cpp`, `tests/test_search_target.cpp`, `tests/test_path_costs.cpp`, `tests/test_rules.cpp` | 99 (three parts) | P-builtchart, S-snapshot, Z-cite | none |
| T13 | `tests/test_model.cpp`, `tests/test_path_list.cpp`, `tests/test_stars.cpp`, `tests/test_highway_draw.cpp`, `tests/test_note_window.cpp`, `tests/test_timeline_view.cpp`, `tests/test_badge_pill.cpp`, `tests/test_dynamics_breakdown.cpp`, `tests/test_saved_figure_fill.cpp` | 96 (three parts) | P-other, P-burnout, F-result | T00 |
| T14 | `tests/test_song_parse.cpp`, `tests/test_song_input.cpp`, `tests/test_song_length.cpp`, `tests/test_midi.cpp`, `tests/test_sng.cpp`, `tests/test_srb.cpp`, `tests/test_timing.cpp`, `tests/test_stem_reader.cpp`, `tests/test_engine_sweeps.cpp`, `tests/test_perf_digest.cpp` | 61 (two parts) | S-snapshot, Z-cite, P-placeholder, P-other | T00 |
| T15 | `tests/ui/uitest_paths.cpp`, `tests/ui/uitest_report_windows.cpp`, `tests/ui/uitest_library.cpp`, `tests/ui/uitest_details.cpp`, `tests/ui/uitest_preview.cpp`, `tests/ui/uitest_batch_reports.cpp`, `tests/ui/uitest_live.cpp` | 74 (two parts) | P-burnout, F-row, P-message | T00 |
| T16 | `tests/test_docs_match_code.cpp`, `tests/single_owner/scan.cpp`, `tests/single_owner/clones.cpp`, and the precheck tests `tests/test_precheck*.cpp` | 71 (two parts) | C-ownchecker, P-message, P-other | T00 |
| T17 | every other `tests/*.cpp` file with a ready row (each holds 1 to 7; the list comes from T00's table) | about 60 (two parts) | mixed | T00 |

### Clone Hero tool tests (Python; the executor may run its own file with `py -I -m pytest <file>`)

Most rows in these files wait for question 1. Each task below fixes only the ready rows.

| Task | Owned files | Ready rows | Main keys | Needs |
|---|---|---|---|---|
| T18 | `tools/ch_mcp/tests/test_game_tools.py` (five parts) | about 190 | C-rig, C-stub, P-message, F-schedule | T00 |
| T19 | `tools/ch_mcp/tests/test_bot_run.py` (two parts) | 55 | C-rig, C-stub, P-message | T00 |
| T20 | `tools/ch_mcp/tests/test_sp_tools.py`, `test_trace_hits.py`, `test_kit_tools.py` in the same folder | 60 (two parts) | C-session, C-stub, P-message | T00 |
| T21 | `tools/ch_mcp/tests/test_foreground.py`, `test_dump_facts.py`, `test_frida_attach.py`, `test_frida_tools.py`, `test_front.py`, `test_game_files.py` | about 110 (three parts) | C-session, P-message, C-ownchecker, C-worker | T00 |
| T22 | `tools/ch_mcp/tests/test_machine_recorder.py`, `test_machine_report.py`, `test_menu_tools.py`, `test_method_watch.py`, `test_mono_facts.py`, `test_quiet.py`, `test_press_schedule.py`, `test_server.py`, `test_launch_core.py`, `test_launch_patch.py`, `test_hydra_end.py` and the small files left in that folder | about 150 (four parts) | C-stub, C-reader, O-process, P-message | T00 |
| T23 | `tools/ch_probe/tests/test_play_chart.py`, `test_play_in_game.py`, `test_prep_inputs.py`, `test_probe_chart.py`, `test_probe_songs.py`, `test_process.py`, `test_process_readonly.py` | about 100 (three parts) | C-clock, C-stub, Z-cite, P-other | T00 |
| T24 | `tools/ch_probe/tests/test_sp_compare.py`, `test_sp_watch.py`, `test_engine.py`, `test_engine_finder.py`, `test_find_checksum.py`, `test_debugger.py`, `test_watch_checks.py`, `test_score_compare.py`, `test_hit_window_scripts.py` | 36 | F-result, F-schedule, P-other | none |
| T25 | `tools/ch_probe/tests/test_input_driver.py`, `test_cheap_checks.py`, `test_early_fill_walk.py`, `test_stall_harness.py`, `test_edge_probe.py`, `test_ocr.py`, `test_overlay_logs.py`, `test_s2_window_constants.py` | 73 (two parts) | P-other, C-stub, O-ocr | none |

### Hydra's other tool tests

| Task | Owned files | Ready rows | Main keys | Needs |
|---|---|---|---|---|
| T26 | `tools/test_effort_test_taint.py`, `test_effort_test_shuffle.py`, `test_effort_test_faults.py`, `test_effort_test_scout.py`, `test_agent_cost.py` under `tools/` | 44 | O-transcript, P-message | none |
| T27 | `tools/test_compare_db.py`, `test_check_handoff.py`, `test_check_pointers.py`, and the PowerShell tests `tools/test_build_pass.ps1`, `test_mutation_probe.ps1`, `test_ci_build_folder.ps1`, `test_release_sign.ps1`, `test_guard.ps1`, with `tools/build_slot.ps1` | about 35 | F-db, S-snapshot, P-message | T00 |

### Hooks-repo tests (in a worktree of `C:\Users\Patrick\.claude`, never in Hydra's)

| Task | Owned files | Ready rows | Main keys | Needs |
|---|---|---|---|---|
| T28 | `gates/tests/test_ref_judge.py`, `test_refusals.py`, `test_review_dispatch.py`, `test_review_rules.py` | about 100 (three parts) | F-verdict, C-gate, P-message, O-transcript | T00 |
| T29 | `gates/tests/test_progress.py`, `test_shell_output.py`, `test_stall.py`, `test_agent_stop.py`, `test_percall_rewrite.py`, `test_percall_key.py` | 55 (two parts) | P-other, S-snapshot, P-message | none |
| T30 | the other `gates/tests` files with a ready row, and `tools/session_cards/check.py`, `batch.py` | about 12 | P-other, P-message | none |
| T31 | `test_h52_cleanup_sweep.ps1` and `test_h53_session_cards_start.ps1` | 13 | P-message | question 5 |

The executor of a hooks task may run only its own test file, and never all the hook tests.

## The bug tasks from Part 1

Each of these changes production code and no number or word a player sees. If an executor finds that its change would alter one, it stops and reports, and the item joins the visible group.

| Task | Work list items | What it does | Goes with |
|---|---|---|---|
| B1 | 18 | Removes the test-only switches from the store and the jobs, once no test calls them. | after T06 and T07; `Needs: T06, T07` |
| B2 | 19 | Deletes the code only tests call, and the tests that pin only it. A test that reaches real code through a dead wrapper moves to the real function. `assign_score`'s removal changes words in `CONTEXT.md:518`; the executor does not edit that file and reports the line for the user. | own task; it edits tests in files other tasks own, so it runs after T02, T03, T13 and T14 merge |
| B3 | 21, 27, 29, 30, 35 | The Clone Hero tools: dead readers, plurals, the two route-word copies, the copied multiplier, the refuted +0x2E0 check with its D13 tests, and a source comment on each unnamed number. | after T22 to T25 merge |
| B4 | 23, 24, 28 | The precheck's flag name, its case-blind member count and its repeated mask; the quiet check's worktree folder; the guard script's home-folder default. | none |
| B5 | 22 | Corrects the wrong comments and stale lists. | none |
| B6 | 15, 16, 17 | Gives each ownerless screen sentence one owner, states the short-beat refusal once, and names the SQLite setting. No text changes. | none; T09 and T15 read these owners if B6 has merged |
| B7 | 7 | Moves the squeeze rating figures from display code to the engine. No number may change; a changed number stops the task. | waits for V0's map, because it changes where Hydra decides a figure |
| B8 | 10, 11, 12 | Bank arrivals, the shared path key, and one reader for the all-0 answer. | waits for V0's map |
| B9 | 3, 9 | The solo box sum and the fill page's double count. | waits for V0's map and for V1's run; joins the visible group if the run shows a difference |
| H1 | 25 | `status_append.ps1` reports a failed write and reads the status path from its owner. | hooks worktree |
| H2 | 26 | `bash_windows_rule.py` learns the other git option spellings. | hooks worktree |

Acceptance for B1 and B2: a search of `src/` and `tools/` for each removed name finds nothing, and the main session's build passes. Acceptance for B6, B7 and B8: the main session's `hydra_uitest` text for the screens that read the value is the same before and after. Acceptance for H1 and H2: each gets a test in its own test file that fails on the old code, which the reviewer reads.

### G1: check the four unproven bugs (items 31 to 34)

One read-only scout checks each of the four once. Item 31 is checked against the MIDI rule in the parser's own cited source. Item 33 is checked against `docs/clone-hero/` first. A fault that shows becomes a bug task the main session adds. A fault that does not show is dropped, and the scout's report says which. Cap: one scout, one pass, no second look.

## The visible group (waits; nothing else waits on it)

Items 1, 2, 4, 5, 6, 8 and 13, and the fills spelling in item 14, change what a player sees.

- **V0: the owner-map scout.** A scout writes the Owner map for these eight items, in the shape `docs/agents/owner-map.md` gives. The planner did not write it. A row with no owner stops the item and goes to the user.
- **V1: the before-and-after run.** The main session runs `hydra_uitest` on corpus charts from `testdata/input` and records each screen's text before the change. The after text comes from the same run on each V2 branch. No text in this plan is invented. `Waits for: V0`.
- **V2: one fix task per item.** Written by a planner from V0's map. `Waits for: V0, V1 and question 6`.

## Questions

Each question is in the user's hands only because no filed ruling decides it.

### Round 1

1. **May the Clone Hero tool tests be fed from real runs of the game?** 525 tests fake the game's memory, a recorder sample, a press log or a watcher log. The fix is to record those once from the real game and check the recordings in. That needs live Clone Hero runs on single songs, never the library. You would see the game start and play a song with nobody at the keyboard. Unblocks 525 rows, in T18 to T25.
2. **Do the tests that break only the test rubric get fixed too?** 476 tests break no rule you have filed. They echo a value back, cannot fail, skip quietly, repeat another test, or check something other than their name says. Nothing on any screen changes either way. Unblocks 476 rows across every task.
3. **The 98 tests no label fits.** The audit sorted them into kinds in `AUD/exits/groups.md`. About half are hand-written input files that the real code reads, which your earlier answer called input, not a fake. Should those stay as they are, and the rest (fixed sleeps, bundled tests, proxies) be fixed? Nothing on screen changes. Unblocks 98 rows.
4. **May the 20 unsettled records be settled by running them?** Twelve need one test run on the checked-in songs; one of those needs your library's timing file. Six need the game's real `dump.cs` and `facts.json`. Two are scripts with no check in them that need a person watching the game; are those two tests at all? Unblocks the 20 open records.
5. **Two hook tests exist twice.** `test_h52_cleanup_sweep.ps1` and `test_h53_session_cards_start.ps1` are in both `gates\tests` and `hooks\tests` of your hooks folder. Which copy is the live one? Unblocks T31, 19 rows.

### Round 2

6. **The eight screen changes.** Asked after V0 and V1, with each screen's real text before and after. Unblocks V2.
7. **Two things you said that are not filed.** "Yes, drop it" and "why are we relying on framerate at all? this is just bad design" (session 1ac0d6b4) sit under Unconfirmed, and a tool cites them as confirmed. Filing them is your click.

Work-list question 5 is partly not asked here. Whether a row is P-builtchart or P-other, and whether a fixed wait is K-floor, changes no fix: both sides get the same recipe. The hand-written-input part of it is question 3.

## Order and rounds

Round 1 needs no answer and no T00: T01, T02, T03, T05, T12, T24, T25, T26, T29, T30, B4, B5, B6, H1, H2, G1, and V0. The main session runs T00 at the same time. About four executors run at once, biggest first: T12, T01, T25, T03, then the rest.

Round 2 is every task marked `Needs: T00`. Round 3 is B1, B2 and B3, which follow the test tasks they name. The visible group runs beside all of this and blocks nothing.

The main session merges each task after its one reviewer signs off.

## Not done

1. **1,162 rows have no test file yet.** T00 closes this. Until it does, the row counts of every task marked `Needs: T00` are the planner's estimates from the audit slice each row came from, and the "about" figures are rough.
2. **No task has its test-case line ranges.** The parts of a split task are counted, not drawn.
3. **Item 20 of the work list needs no task** (the code stays). Item 14's name owner (not its fills spelling) is not placed in a task; it belongs in B6.
4. **The task tables were not summed back to 2,459.** The four piles are exact. The split of the ready pile across T01 to T31 is not proven to add up, and T17's file list is not written out.
5. **The premise rows marked unchecked were not tried further.**
6. The planner opened no production line the work list names.

## Out of scope, for the user to approve

Fixing the hooks named in work-list items 34 (the key lock and `prepass.py`) beyond G1's check.

## Promise map

| Promise | Plan section | Task | The check that proves it |
|---|---|---|---|
| Every faulty row is in one task or one wait | The counts | T00 | `row-files.tsv` has 3,558 lines, and the four piles sum to 3,558 |
| Every owned row is fixed or reported | The test-file tasks, acceptance 1 | T01 to T31 | reviewer compares the report's list with the row list |
| No hand-built stand-in is left for a fixed key | acceptance 2 | T01 to T31 | reviewer reads the diff |
| The branch builds and its tests pass | acceptance 3 | T01 to T31, B1 to B9 | main session's build and `hydra_precheck.exe --tests` list |
| No waiting row is touched | acceptance 4 | T01 to T31 | reviewer checks the diff against the wait piles |
| A bug task changes nothing a player sees | The bug tasks | B1 to B8 | `hydra_uitest` text the same before and after; a difference stops the task |
| Removed code has no caller left | The bug tasks | B1, B2 | search of `src/` and `tools/` finds nothing; the build passes |
| Hook fixes have a test that fails on the old code | The bug tasks | H1, H2 | reviewer reads the test |
| An unproven bug is checked once, then fixed or dropped | G1 | G1 | the scout's report names each of the four |
| Visible items wait for the map and the user | The visible group | V0, V1, V2 | V2 is not written until the map and answer 6 exist |
| No run touches the whole library | Questions 1 and 4 | every task | the whole-library hook refuses the command |
| Hooks work stays out of Hydra's worktrees | Hooks-repo tests | T28 to T31, H1, H2 | the branch is in `C:\Users\Patrick\.claude` |
| Nothing waits on the visible group except B7 to B9 | Order and rounds | all | the `Needs` column |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: `labels.tsv` holds 3,558 faulty rows, 1,877 count rows and 1,681 records | code | `docs/audit/2026-11-14-test-audit/exits/labels/labels.tsv` | checked 2026-11-18, counted by kind |
| P2: every one of those rows has exit 2 | code | `docs/audit/2026-11-14-test-audit/exits.md` | checked 2026-11-18, joined on row id |
| P3: the user's five test rulings are filed under Confirmed | ruling | `docs/agents/rulings/tests-and-runs.md` lines 9-13, in the main checkout only; the plan's branch lacks them | checked 2026-11-18 on main |
| P4: every faulty row can be matched to one test file and line | unchecked | `docs/audit/2026-11-14-test-audit/exits/ledger.tsv` names a file for 2,377 rows and none for 1,162 | T00 |
| P5: the recipes in Part 2 of the work list fit every row under their key | unchecked | `docs/audit/2026-11-14-test-audit/worklist.md` only reports this | T01 (acceptance 1 lists each row a recipe did not fit) |
| P6: each Part 1 item's source line says what the work list says | unchecked | `docs/audit/2026-11-14-test-audit/worklist.md`; the planner opened none | B5 (each bug task's executor opens its lines first and stops on a mismatch) |
| P7: 20 records are open, in groups of 12, 6 and 2 | code | `docs/audit/2026-11-14-test-audit/exits/open.md` | checked 2026-11-18 |
| P8: 98 rows carry no key | code | `docs/audit/2026-11-14-test-audit/exits/groups.md` line 88 | checked 2026-11-18; the join found the same 98 |
| P9: a rubric key breaks no filed ruling, so its group is its own question | code | `docs/audit/2026-11-14-test-audit/exits/keys.md` line 23 | checked 2026-11-18 |
| P10: the two h5x hook tests each exist in `gates/tests` and `hooks/tests` | code | `git ls-files` in `C:\Users\Patrick\.claude` | checked 2026-11-18 |
| P11: the Python tool tests need no build | unchecked | `docs/agents/brief-preamble.md` line 47 says so of "the Python tools' tests" in general | T24 (its executor reports any test file that will not run without a build) |
| P12: the user wants every bad test fixed with no further reading rounds | ruling | "well i still want to fix all the garbage tests, i just want to stop wasting time with more reads and get to the point where we can actually act on our findings", session 98fa2925, as the main session's brief quotes it | checked 2026-11-18 |

There are no live steps in this plan. Question 1's answer adds them, and whoever writes them follows the call-list rule on `docs/agents/premise-map.md`.
