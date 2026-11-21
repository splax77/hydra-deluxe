# Brief: T5, the window round, groups WN1 to WN5 (auditors) and their verifiers

Read `docs/agents/brief-preamble.md` first, in the worktree named below. Its rules bind you. Where this brief and that page disagree, stop and report it.

## Why this round exists

The find rounds read the tests at main commit `c10c7f5485cb34ab77e15c7298403223771b5ccc`. There, the early-fill rule was fixed: `fill_refuses(e_offset)` in `src/core/model.h` was `e_offset < -kHitWindowMs`, with `kHitWindowMs = 85.0`. At `5ec682c7` the rule takes a window, `fill_refuses(e_offset, window_ms)` and `is_e0(e_offset, skips, window_ms)` (`src/core/model.h:331` and `335`), and the search's default is `kDefaultEarlyFillWindowMs = 60.0` (`src/core/model.h:112`, read through `SearchSettings::early_fill_window_ms` in `src/search/pather.h:45`). The commits are `abb6b45d`, `e066919d` and `0339e2a2`. So every test that runs the search on the default window now runs it at 60 ms, not 85, even where the test file did not change. The delta round (`records/delta-DL*.md`) read only the test cases whose own lines changed. This round reads the rest.

The engine records in `records-engine/` were dumped at `326cd4e0`, under the old fixed 85 ms rule. They are evidence of what the engine did at 85, not at 60.

## Who and where

- Task id: T5-WN1 to T5-WN5 for an auditor, T5-V-WN1 to T5-V-WN5 for a verifier, as your launch prompt names. Session id: e633f7f6-f313-4787-b622-8d901b577b64.
- Your worktree: `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records`, branch `claude/test-audit-records`. Read code there, at its HEAD (which holds `5ec682c7`'s code). Do every git command there.
- An auditor owns and writes one file, `docs/audit/2026-11-14-test-audit/records/window-<group>.md`. A verifier owns and writes one file, `docs/audit/2026-11-14-test-audit/verify/window-<group>.md`, with its agent id on the first line. Commit it by name, with the trailers the preamble gives (`--trailer "Task: ..." --trailer "Agent: ..." --trailer "Session: ..."`). Other agents commit on this branch at the same time: stage only your file by name, check `git diff --cached --name-only` shows only your file before you commit, and if git reports an `index.lock` or a moved HEAD, wait a few seconds and retry. Commit after every few test files.
- Tests you may run: none. You build nothing and run no `hydra_*` program (agent rule 14).
- Caps: the progress hook's (a warning at 100 calls or 20 minutes, a stop at 150 calls or 30 minutes). At the warning, commit and list every test file and test case you did not reach, by exact name.
- Status: one line every 5 minutes through `status_append.ps1`, as the preamble says.

## The groups

| Group | Files |
|---|---|
| WN1 | `tests/test_library_query.cpp`, `test_search_target.cpp`, `test_rules.cpp`, `test_library_jobs.cpp`, `test_store_results.cpp`, `test_store_records.cpp`, and the helpers `tests/store_fixtures.h`, `tests/record_fixtures.h` |
| WN2 | `tests/test_sp_end.cpp`, `test_sp_end_clamp.cpp`, `test_sp_meter_curve.cpp`, `test_best_finish.cpp`, `test_stars.cpp`, `test_track_state.cpp`, `test_fast_tempo.cpp`, `test_squeeze_rating.cpp`, and the helper `tests/fast_tempo_charts.h` |
| WN3 | `tests/test_engine_sweeps.cpp`, `test_fills_and_skips.cpp`, `test_tied_variants.cpp`, `test_model.cpp`, `test_path_costs.cpp`, `test_helpers.cpp`, and the helpers `tests/corpus_util.h`, `tests/analyzed_chart.h` |
| WN4 | `tests/test_replay.cpp`, `test_pb_coach.cpp`, `test_perf_digest.cpp`, `test_pb_text.cpp`, `test_analysis.cpp`, `test_app_reports.cpp`, `test_preview_scene.cpp`, `test_pathstring_read.cpp`, `test_report_rows.cpp`, `test_app_click.cpp`, `test_saved_figure_fill.cpp`, `test_replay_schedule.cpp`, `test_replay_windows.cpp`, `test_cli.cpp`, `test_activation_rows.cpp`, `test_preview_golden.cpp`, `test_song_parse.cpp`, `test_preview_controller.cpp`, and the helpers `tests/app_state_fixtures.h`, `song_digest.h`, `song_details_fixtures.h`, `dm_fixture.h` |
| WN5 | `tests/ui/uitest_details.cpp`, `uitest_engine.cpp`, `uitest_engine.h`, `uitest_preview.cpp`, `uitest_paths.cpp`, `uitest_harness.cpp`, `uitest_harness.h`, `uitest_live.cpp`, `uitest_main.cpp`, `uitest_library.cpp`, `uitest_commands.cpp`, `uitest_report_windows.cpp`, `uitest_script.cpp`, and `tests/pb_ui/pbtest_harness.cpp`, `pbtest_harness.h`, `pbtest_main.cpp` |

`coverage.md` names each file's old records file. Read the old record of every test case you judge.

## The auditor's job

For each file in your group, find every test case that reaches the search: directly (`analyze_chart`, `analyze_chart_file`, `search_target`, `search_allzero`, an `Engine` or `ScoreGraph`), or through a helper (`corpus::analyzed`, `corpus::analyzed_with_paths`, the fixtures in your group, the GUI harness's analyze path). Follow helpers until you know which window the search gets. Then sort each test case into exactly one of three kinds, in a table at the head of the file's section, one row per test case:

- **A, the window is set.** The test or its helper sets `early_fill_window_ms` (or the settings field it comes from) to a named value. Write the value and where it is set.
- **B, the window cannot move the result.** The test runs on the default window, but nothing it asserts can depend on the early-fill rule. Name the reason in the code: for example, the chart has no fill the search can activate from, or the assertion reads only parse output that `fill_refuses` and `is_e0` never touch. "Not about fills" alone is not a reason; say which fact makes the window unreachable from the asserted values.
- **C, the window can move the result.** The test runs on the default window and an asserted value can depend on it: a path, a score, a count, an E mark, an activation, a typed number taken from a corpus run, a Keep that rested on an engine record dumped at 85. Write a full record for it.

Every C gets a full record, in the shape of the rubric's "A filled-in record" (`docs/agents/test-rubric.md`; read it and `docs/agents/test-rubric-examples.md` first). It starts with the line `Delta round: window, replaces <old records file> record <number or heading>`, and says whether the old verdict still holds at 60 ms and why. Its question 3 names the engine record the claim now needs: a dump at 60 ms of a named chart (`hydra_replay` at `--efill 60`), since the existing ones are at 85. A typed number the test took from a run at 85 is a finding on its own line: name the number, the line, and why it can move. A test case that already has a record in `records/delta-DL*.md` is out of scope here: list it in the table as "delta round" and do not judge it again.

The question 3 rule from the earlier rounds holds: a claim about a chart is checked against an engine record of a real chart; a hand-built fixture alone is "claim not checked"; a test whose only fault is question 3 gets Investigate with the words "claim not checked only". Rewrite is for a fault in the test itself.

## The verifier's job

Your part is one auditor's file, `records/window-<group>.md`. Another agent wrote it.

1. **Scope.** Do the auditor's search yourself for every file in the group. A test case that reaches the search and has no row in the tables is a finding. Check every A row's value and place, and every B row's reason, against the code. A B whose reason does not hold is a dispute: it should be C.
2. **Each C record.** Check its claims against the code, the Keeps as well as the rest. Mark each claim "by reading" or "proven by" a named run in the records folders. Say whether you agree with the verdict, and where you do not, what it should be, with file and line. Check question 8's named bug at a line. List every split, one line each.

## Rules every audit agent keeps

- A finding leaves the audit by one of four exits: fixed, a question to the user, covered by a ruling, not a problem (`docs/agents/audit-method.md`). Only the main session picks an exit. You pick none, and you grade nothing as closed.
- Every split, where two parts of Hydra answer one question differently, is its own line.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own. Widen it, and say where you looked.
- No whole-library run happens without the user's `allow library run` in chat. You run nothing anyway.
- No rewrite may propose a saved expected-output file. The filed ruling at `docs/agents/rulings/code-and-engine.md` line 50 reads "we don't need golden anymore, remove it all".
- The filed rulings about the window are under `## Confirmed` in `docs/agents/rulings/code-and-engine.md`, lines 55 to 57. Quote them; do not paraphrase them.
- The user's words can be in the decision log (`docs/audit/2026-10-03-fix-decisions.md`), the question files under `docs/audit/`, the plans under `docs/superpowers/plans/`, and the session cards in `docs/sessions/`. Only a line under `## Confirmed` in `docs/agents/rulings/` is a filed ruling.

## Your final report

An auditor returns: the commit hash; per file, the count of A, B, C and "delta round" rows; the verdict count for the C records; every typed number found that can move; the exact names of anything not reached; and anything you stopped on. A verifier returns: the commit hash; the count of rows checked and disputed per kind; C verdicts agreed and disputed; every test case missing from the tables; anything not reached; and anything you stopped on. The file is the answer; do not restate it.
