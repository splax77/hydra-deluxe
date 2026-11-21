# Audit every test in the suite by the test rubric (2026-11-14)

## The short version

Agents read every test Hydra has and ask it the twelve questions on `docs/agents/test-rubric.md`. They write one record per test. Fresh agents check those records. The main session then picks how each finding leaves, and a fresh agent checks every closure. Nothing is fixed here. Every rewrite or removal of a test waits for the user's yes.

The audit changes no test and no code. Its only output is a folder of records, `docs/audit/2026-11-14-test-audit/`, on a branch of its own.

The work is 86 slices of test files, read by 86 auditor agents over 22 rounds of at most four. That count is provisional: the calibration run (T3) measures how much one agent can finish, and T4 re-cuts the slices once if the measure says so.

## Scope

The scope is the whole suite, as the user asked: "a full audit of the entire test suite based on these principles". Counted on this branch at 003d76a3, that is 238 files.

| Where | Files | Lines |
|---|---|---|
| `tests/*.cpp` | 142 | 74,116 with `tests/ui` |
| `tests/ui/*.cpp` | 13 | (counted in the row above) |
| `tests/pb_ui/*.cpp`, `tests/single_owner/*.cpp`, `tests/fuzz/*.cpp` | 6, 4 and 6 | 3,483 |
| `tools/**/test_*.py` | 62 | 30,146 |
| `tools/test_*.ps1` | 5 | 1,168 |

The first request for this plan counted only the first two rows. The other 83 files are tests too (the five PowerShell tests are the ones `docs/development.md` lines 292 to 299 and `.github/workflows/ci.yml` line 128 name), so they are in. Leaving them out would narrow the user's words.

Two things are out, and both are listed under "Out of scope" for the user to approve: the hook tests in `C:\Users\Patrick\.claude\gates\`, which live in another repo and test the hooks, not Hydra; and the two scripts `tools/ch_probe/experiments/key_delivery_test.py` and `tools/ch_probe/experiments/pad_flash_test.py`, which `docs/development.md` lines 307 to 309 say drive the real game and are not run as tests.

Test helpers (`tests/*.h`) are not sliced on their own. An auditor opens a helper when a test it is judging uses it, and says so under question 12.

## What the audit looks for

Each kind below is a "no" to one of the rubric's questions. The examples page shows a real one of each (B1 to B14).

| Kind | Rubric question | Example |
|---|---|---|
| The claim is a rule a display invented | 2, 3, 6 | B1, B2, B4 |
| A check that cannot fail | 8 | B3 |
| A snapshot number | 4, 5 | B6, B7, B12 |
| A floor with no reason | 5, 9 | B8 |
| A silent skip | 9 | B9 |
| A value checked against the function that made it, or made by production code | 5, 9 | B10, B11 |
| A raw result whose meaning lives in a comment | 4 | B13, B14 |
| A scenario no real chart can reach | 11 | none yet |

A finding against production code (a display working out a game fact) is written on its own line, as the rubric's verdict section says. Rewriting the test does not close it.

## What the audit reads first

Audit-method stage 1 asks for the decisions and plans the audit must read. Every auditor brief names these.

- `docs/audit/2026-10-03-fix-decisions.md`. Tests cite D-numbers from it, such as D43 at line 101, which approves `acts > 1000` "as test limits". The rubric says an approval is not a reason, so a cited D-number is opened and quoted, never accepted.
- `docs/adr/`, all 31 records. `0008-preview-is-a-faithful-port-of-onyx.md` names `tests/test_preview_golden.cpp`, a golden-image test, so the auditor of that file reads the ADR and the ruling on golden files together and writes the clash as a question, not a verdict.
- `docs/agents/rulings/`, lines under `## Confirmed` only. `code-and-engine.md` line 50 reads "we don't need golden anymore, remove it all". No auditor proposes a saved expected-output file.
- `docs/clone-hero/`, for every rule number a test types.
- `docs/superpowers/plans/`. The 16 other plans there are about hooks, the build limit, the Clone Hero player and Clone Hero facts. A search of all 16 for "rubric", "test suite" and "test audit" finds only this plan (P8). `2026-11-08-player-audit.md` is the model this plan's shape follows, and is about the playing code, not tests.
- The session cards in `docs/sessions/`, searched by keyword before anyone writes "no ruling found".
- The Preview fix handoff, `git show claude/preview-fix-handoff:docs/handoffs/2026-11-14-preview-draws-engine-sp-handoff.md`. The audit does not fix that bug. Its tests are B1 to B5, and the audit judges them like any other test.

## Who does what

| Stage (audit-method) | Who | Model |
|---|---|---|
| 1 Plan | this plan, then one fresh reviewer, then the user's yes | Fable, Sonnet |
| 2 Calibrate | main session writes the key; one calibration agent | Sonnet scout |
| 3 Find | 86 auditors, one slice each | Sonnet scouts |
| 4 Verify | fresh verifiers, never on a slice they audited | Sonnet scouts |
| 5 Choose exits | main session only | |
| 6 Check closures | fresh closure checkers | Sonnet scouts |
| 7 Ask | main session, to the user | |
| 8 Record | main session | |
| Builds, red runs, planted bugs, record dumps | main session only (agent rule 14) | |

No agent builds or runs `hydra_tests`, `hydra_uitest` or `hydra_replay`. Agents are read-only on tests and code. Each owns exactly one file: its own records file under `docs/audit/2026-11-14-test-audit/`, in the audit's worktree, which it commits by name.

## The tasks

### T0: Set up, and prove the slice table covers the suite

The main session cuts the branch `claude/test-audit-records` and a worktree for it, and makes `docs/audit/2026-11-14-test-audit/`. It then lists every test file the build and the tool folders know and compares the list with the slice table below. The sources are: the root `CMakeLists.txt` (line 739 globs `tests/test_*.cpp` into `hydra_tests`, which also takes `tests/report_samples.cpp` and the `tests/single_owner` files by name; lines 604 to 630 list the `tests/ui` files; lines 712 to 717 list the `tests/pb_ui` files; the fuzz targets are added near line 966); and every `test_*.py`, `*_test.py` and `test_*.ps1` under `tools/` and `tests/`. There is no `tests/CMakeLists.txt`. `tools/precheck/test_pick.cpp` is read by name only: T0 opens it and says whether it is a test or the precheck's own code.

Acceptance: a file `coverage.md` in the audit folder lists every test file with its slice id. Zero files have no slice. A file the table lacks gets a new slice, appended to the last round. The calibration file is the one file with "T3" in place of a slice id. The two experiment scripts are listed with "out of scope, pending the user's yes" (question 2).

### T1: Dump the engine's record for real charts

Question 3 needs the engine's record, and agents cannot run `hydra_replay`. So the main session runs `hydra_replay dump` then `hydra_replay schedule` on a few charts from `testdata/input` and saves the text under `docs/audit/2026-11-14-test-audit/records-engine/`. The two command names are in `tools/replay.cpp` lines 868 and 870. The main session reads the tool's usage text for their arguments (P9).

The starting charts are the five the corpus keeps in folders of their own. The reasons below are what was read, and the last two are weak.

| Chart | Why |
|---|---|
| `testdata/input/test_pitch_dark_ratio` | The rubric's own question 3 example: a late squeeze-in, step on chord 243. Its folder holds `notes.mid` and `song.ini`; `tests/test_replay_schedule.cpp` uses it. |
| `testdata/input/test_folded_sqin` | Its files are named for squeeze-in cases (`folded_sqin.chart`, `clamped_sqin_a.chart`, `twin_end_nodes.chart` and two more), and `tests/test_squeeze_rating.cpp` and `tests/test_tied_variants.cpp` use it. |
| `testdata/input/test_fast_tempo` | Seven short charts such as `early_sqin_twice.chart` and `late_sqin_twice.chart`. `tests/test_fast_tempo.cpp` lines 1 to 6 say they are charts where one SP bar lasts under the 500 ms squeeze window, and its lines 68 to 74 check one step per squeeze-in. It is also the chart for question 11 (can the scenario happen). |
| `testdata/input/test_flammarker` | Four `.mid` files named for flam markers. Only `tests/test_dynamics_breakdown.cpp` line 208 uses it, for a dynamics case. Nothing read ties it to Star Power, so it is a candidate only. |
| `testdata/input/test_path_reduction` | One chart, `test0.chart`, titled "Path Comparison SP Test". No test names it (the corpus loops read it with every other chart); a grep of `tests/`, `src/` and `tools/` finds it only in old session cards. A candidate only. |

What each record actually holds is not checked yet (P10). So T1 ends with an index: for each dumped chart, which of these cases its record shows, with the activation and chord. The cases are a squeeze-in step, a squeeze-out, a skipped fill, and an early fill at the SP-ready tick. A case no starting chart shows gets one chart from `testdata/input/common` that does. The cap is ten charts in all. A case still missing at the cap is written in the index as "no corpus chart found", and tests that rest on it read "claim not checked".

Blight is not in the corpus. Its record for the squeeze-out case is already quoted in the Preview fix handoff, and auditors use that text. No whole-library run happens (agent rule 8).

An auditor who needs another chart names it in its report and writes "claim not checked". The main session dumps the asked-for charts once, after the last find round, and T5's verifiers finish those records. The cap is that one extra dump pass.

Acceptance: one text file per chart, and `index.md` naming the case, activation and chord for each. Every path in the index exists.

### T2: The calibration answer key

The main session writes the key before any calibration agent exists. The file is `tests/test_preview_scene.cpp` (1,004 lines, 33 test cases). Neither rubric page quotes it: every `tests/` path both pages name was listed (P17), and it is not among them. It builds the Preview's scene, which is a display, and it loops over the corpus three times. So it is the kind of file where a test can state a display's own account of a game fact.

Whether it holds such a test is not checked (P11). The main session checks by doing questions 1 to 3 itself on the file, with T1's records. The key must hold at least one test whose claim is the fault, and at least one whose checks are the fault. If the file has no test of the first kind, the main session tries `tests/test_preview_boxes.cpp`, which is also unquoted. If neither has one, it stops and tells the user. It does not write a key without one.

The key stays out of the repo and out of every brief until T3 is scored. This plan says nothing about what is wrong in the file, and the calibration brief must not either, not even as an example.

Acceptance: a key file in the main session's scratch folder, written before T3 starts, listing each known-bad test, the question it fails and the engine record line that shows it.

### T3: Calibrate

One Sonnet scout gets the calibration file, both rubric pages, T1's index and the standard audit brief (below). It gets no hint. It returns one record per test case.

The main session scores it against the key. It passes if it reports every test in the key, and reports the claim-is-the-fault tests as failing question 2 or 3, not only question 4. If it misses one, the main session stops, reports the miss to the user and fixes the brief before any auditor starts (audit-method stage 2). The cap is two calibration runs, each with a fresh agent. After a second miss the audit stops and the user decides.

T3 also measures size: how many test cases and lines the agent finished before the hook's 20-minute warning.

Acceptance: a line in the audit folder's `log.md` with the key's count, the found count, the verdict (pass or miss) and the measured size.

### T4: Re-cut the slices once, if the measure says so

The slice table below packs files by lines, 1,500 for C++ and 2,500 for Python, at most eight files to a slice. Those caps are an estimate, not a measure (P12). If T3's agent finished at least its whole file, the table stands. If it finished less, the main session lowers the caps to what it finished, re-packs by the rule under the table and tells the user the new slice and round counts before the find rounds start. This happens once. It is not repeated per round.

Acceptance: `log.md` says "table stands" or gives the new caps and counts.

### T5: Find and verify, round by round

Each round launches four auditors together, one slice each, in table order: A slices, then B, C and D. Display tests come first, as the rubric asks. From round two on, each round also launches two verifiers for the round before, so no stage queues behind another (agent rule 6).

**An auditor** judges every test case in its slice and writes one record per test, in the shape under "The record". It grades nothing as closed and picks no exit. It writes side notes with file and line.

**A verifier** takes the records of two slices it did not audit. It checks every record that is not Keep against the code, and every Keep as well, because a wrong Keep is the miss this audit exists to stop. It marks each claim "by reading" or "proven by" a named run. It lists every split a record's tables show, one line each. It finishes any "claim not checked" record whose chart has since been dumped.

Caps. Every agent runs under the progress hook: a warning at 100 calls or 20 minutes, a stop at 150 calls or 30 minutes. At the warning an agent commits its records file and lists the test names it did not reach. The main session sends those exact names to one fresh agent. A slice gets one such re-send. Tests still unjudged after it are listed in the final report as "not audited", by name.

Stop rule. The find stage ends when every slice in `coverage.md` has a committed records file or a "not audited" list. There are no completeness rounds. The verify stage ends when every records file has one verifier file beside it.

Acceptance: for every slice id, `records/<id>.md` and `verify/<id>.md` exist. The number of records plus the number of "not audited" names equals the number of tests in the slice's files. A test is one unit by file kind, and the main session counts each kind from its own source:

- `tests/*.cpp` and `tests/single_owner/*.cpp` (the doctest files): one `TEST_CASE`. The count is the lines that start `TEST_CASE` in the file (2,108 in `tests/*.cpp` and 19 in `tests/single_owner` today), and the main session checks the total against `hydra_tests.exe --list-test-cases` (P5).
- `tests/ui/*.cpp`: one entry registered with `IM_REGISTER_TEST` (`tests/ui/uitest_tests.cpp` line 22, `uitest_paths.cpp` line 1051, `uitest_report_windows.cpp` line 1652, `uitest_engine.cpp` line 111). These files hold no `TEST_CASE`. The count is `hydra_uitest.exe --list` (`tests/ui/uitest_main.cpp` line 9), and each name is matched to its file by the entry tables.
- `tests/pb_ui/*.cpp`: the same, from `hydra_pathbuilder_uitest.exe --list` (`tests/pb_ui/pbtest_main.cpp` line 7).
- `tests/fuzz/*.cpp`: each file is one libFuzzer target (`LLVMFuzzerTestOneInput`), so one record per file.
- `tools/**/test_*.py`: one pytest test function. `docs/development.md` line 292 names pytest as the runner. The count is the `def test_` lines in the file, and a parametrised function counts once.
- `tools/test_*.ps1`: one `Check` call, because each script prints one line per `Check` (for example `tools/test_release_sign.ps1` lines 13 and 39). The count is the `Check` lines in the file; T0 confirms the helper's name in each of the five.

### T6: Change the corpus and watch what turns red

This is the rubric's first mechanical check. It needs builds and test runs, so it is the main session's. It runs alongside the find rounds, in a throwaway worktree that is never merged.

Step one adds one chart to `testdata/input` (a copy of an existing chart under a new folder name) and runs the tests. Step two restores it, removes one chart that no test names, and runs them again. The removed chart is picked by grepping `tests/` for each corpus folder and file name and taking the first with no hit. Both runs are the full suite, run by the main session and by no agent. Which rule allows it: rule 1 forbids only agents from running the full suite, and gives the main session the tests that need a build; rule 14 makes every build and every test that needs one the main session's, built one at a time through a pass from `hydra_build_pass`, so each build here takes such a pass; rule 8 covers runs over the user's song library, and these runs use the checked-in corpus only, so it does not apply. Rule 1's line about re-running untouched parts is about hook tests and does not decide this: the point of this step is to change the corpus and see what moves.

Acceptance: `mechanical/corpus.md` lists every test that changed colour in each step, with its failure line. Each one is a finding ("leans on which charts are in the corpus") and is matched to its record in T7. The file also says that a test which stayed green is not proven independent.

### T7: Plant a sample of the auditors' bugs

This is the rubric's second mechanical check. Each record's question 8 names a bug as a small change in a named function. After each letter group (A, B, C, D) finishes, the main session plants bugs from that group in a throwaway worktree that is never merged (so the audit branch stays free of code changes), builds once per batch of bugs that do not touch the same test (each build through a `hydra_build_pass`, rule 14), runs the tests each record names, and restores the tree from git.

The sample is one named bug from each auditor, so every auditor's reasoning is tested once. If an auditor's claim is wrong (the test stayed green, or a different line went red), the claim is reported, and a verifier re-reads every question 8 answer in that auditor's slice. That closer read is the rubric's own rule.

`tools/mutation_probe.ps1` (removed; `git show ed0aa857:tools/mutation_probe.ps1`) exists and plants flipped comparisons on its own. It is not used here, because this check tests the bugs the auditors named, not bugs a tool picks.

Acceptance: `mechanical/planted.md` has one row per auditor: the bug, the tests run, red or green, and the line that failed. Records whose claim was confirmed change "reasoned" to "confirmed".

### T8: Choose each finding's exit

The main session picks one exit for every finding, and for every split inside one, and writes its evidence (audit-method stage 5). Agents never pick an exit.

- Exit 1, fixed, cannot happen inside this audit, because the audit fixes nothing. It is used only where a commit made by someone else during the audit already removed the problem; the evidence is that commit.
- Exit 2, a question to the user, is the default. Every Rewrite and every Remove is a change to a test, so it goes to the user. Every Investigate goes to the user. Every production-code finding goes to the user.
- Exit 3, covered by a ruling, needs the verbatim quote from a line under `## Confirmed` or a D-record, with file and line, naming both sides.
- Exit 4, not a problem, is a Keep verdict that the verifier agreed with. Its evidence is the record itself.

"On purpose", "by design", "deliberately" and "kept" close nothing. A finding with no exit that fits becomes a question.

Acceptance: `exits.md` has one row per finding and per split: the record, the exit, and the evidence. No row has an empty exit. The count of rows equals the count of verified findings plus splits.

### T9: Check every closure

Fresh Sonnet scouts check every finding that left by exit 1, 3 or 4. None of them audited or verified the finding. Each returns PASS, REOPEN or GONE. A REOPEN becomes a question. Checkers take closures in groups of about eight, three or four agents to a round.

The main session then greps every exit 3 quote in its cited file. A quote that is not there turns PASS into REOPEN.

A checker searches the session cards before it writes "no ruling found".

Acceptance: `closures.md` has a verdict for every exit 1, 3 and 4 row in `exits.md`. Every exit 3 PASS has a grep line under it showing the quote found.

### T10: Ask the user

Questions go out in small groups; five or fewer per message is the main session's own reading of "small" (audit-method stage 7). They go out group by group as T9 clears them, display findings first.

Tests that share one rule share one question (rubric question 10). Each question shows the test's name, the line that fails, what the line checks today, and what the rewrite would check, in plain words. Where the fix would change what Hydra shows, the question lists every screen that reads the value, with its text before and after from a real chart in T1's records (agent rule 11). The main session reads the code again on the day each question goes out.

A question never asks the user to pick a rule the plan or an auditor made up (agent rule 15). Before each group goes out, one fresh Sonnet scout, not an auditor or verifier of those findings, reads the group's questions in `questions.md` and, for each one, names the engine value, decision or ruling it rests on, with file and line. A question with none is dropped as a question and goes back to the main session as a finding about a display or an invented rule. Where the engine's record and a decision disagree, both are shown. Where a display works a game fact out for itself, the finding says so and names the engine's value; it is not put as a Clone Hero question.

Acceptance: `questions.md` holds every question sent, each with its finding ids. Every exit 2 row in `exits.md` points at one.

### T11: Record

Each answer becomes a D-record in `docs/audit/2026-10-03-fix-decisions.md`, quoting the question's text and naming both sides of any split. `exits.md` gains the D-number. The audit branch then goes to the user for merge.

The fixes are not in this plan. After the answers, fixes get their own plan: executors on Opus, one worktree each, no two owning one file, one fresh reviewer per merge.

Acceptance: every answered question in `questions.md` names its D-record.

## The record every auditor returns

One record per test case, in the shape of "A filled-in record" on the rubric page. It holds:

- the test's name, file and line;
- **Read:** everything opened, by file and line. Anything not opened is written "not read";
- answers 1 to 11, each a sentence or two naming files, lines and functions. Question 2 lists every caller found in `src/`. Question 3 is the two lines "Engine:" and "Claim:" from a T1 record, or "claim not checked";
- for questions 4 and 8, one line per assertion;
- the verdict: Keep, Rewrite, Remove or Investigate. No "Keep, with a flag";
- for Rewrite and Remove, what the test catches today;
- any production-code finding, on its own line;
- any split a table shows, each on its own line;
- side notes, with file and line.

A record whose question 1 cannot be written stops there with Investigate.

## What every brief carries

Every auditor, verifier and checker brief starts with `docs/agents/brief-preamble.md`, then gives: the task id, session id, slice id and file list (with line ranges for a split file), the caps and the status rule; the four exits, and that only the main session picks one; that every split is its own line; that "on purpose", "by design", "deliberately" and "kept" are claims, not evidence; that a search that finds nothing proves nothing; that no whole-library run happens without the user's `allow library run`; where the user's words can be (the decision log, the question files, the plans, the session cards); the reading list above; the path of T1's index; and that no rewrite may propose a saved expected-output file. A second-wave agent gets exact test names.

## The slices

A slices are display tests: every C++ file that matches `PreviewScene|build_preview_scene|sp_meter|gauge|[Rr]eport`, plus every file under `tests/ui` and `tests/pb_ui`. With the `tests/ui` and `tests/pb_ui` files that is 79 files, 78 in the A slices and the calibration file (`tests/test_preview_scene.cpp`); the handoff gave 30. The pattern is wide, and a file it sweeps in by the word "report" alone still gets every question. B slices are the rest of the files that call `corpus::chart_paths` (28 files call it in all; 17 of them are already in A or are the calibration file). C slices are every other C++ test file. D slices are the Python and PowerShell tool tests.

The packing rule: take the files of a group in name order, the files directly under `tests/` first, then `tests/ui`, `tests/pb_ui`, `tests/single_owner` and `tests/fuzz`, and under `tools/` the folders `ch_mcp/tests`, `ch_probe/tests` and `tools/` itself, Python files before PowerShell ones; start a new slice when the next file would pass the line cap or would be the ninth file; a file over the cap is cut into equal parts at the nearest test-case boundary, and each part is its own slice with its line range in the brief.

Rounds run four slices at a time in table order: 22 rounds for 86 slices, the last with two. A script that repeats the rule over the files' line counts reproduces this table, so the table and the rule agree; T0 reruns it.

| Slice | Lines | Files |
|---|---|---|
| A1 | 105 | `tests/report_samples.cpp` |
| A2 | 847 | `tests/test_activation_rows.cpp` (part 1 of 2) |
| A3 | 847 | `tests/test_activation_rows.cpp` (part 2 of 2) |
| A4 | 1410 | `tests/test_analysis.cpp` |
| A5 | 771 | `tests/test_app_click.cpp` |
| A6 | 1010 | `tests/test_app_library.cpp` |
| A7 | 1326 | `tests/test_app_reports.cpp`, `tests/test_app_shell.cpp` |
| A8 | 843 | `tests/test_batch_text.cpp`, `tests/test_best_finish.cpp`, `tests/test_column_widths.cpp` |
| A9 | 1060 | `tests/test_config.cpp`, `tests/test_dm_other_speed.cpp` |
| A10 | 1059 | `tests/test_dm_report.cpp` |
| A11 | 1106 | `tests/test_docs_match_code.cpp` (removed; `git show 4ccd4b93:tests/test_docs_match_code.cpp`) (part 1 of 2) |
| A12 | 1106 | `tests/test_docs_match_code.cpp` (removed; `git show 4ccd4b93:tests/test_docs_match_code.cpp`) (part 2 of 2) |
| A13 | 994 | `tests/test_engine_sweeps.cpp`, `tests/test_fill_deadline.cpp` |
| A14 | 1275 | `tests/test_fill_report.cpp`, `tests/test_fills_and_skips.cpp`, `tests/test_fitted_text_cache.cpp`, `tests/test_helpers.cpp` |
| A15 | 1031 | `tests/test_highway_draw.cpp` |
| A16 | 1052 | `tests/test_library_jobs.cpp` |
| A17 | 991 | `tests/test_library_query.cpp`, `tests/test_long_paths.cpp`, `tests/test_main.cpp` |
| A18 | 1273 | `tests/test_model.cpp` |
| A19 | 1305 | `tests/test_path_list.cpp`, `tests/test_pb_text.cpp`, `tests/test_precheck.cpp` |
| A20 | 1097 | `tests/test_preview_boxes.cpp` |
| A21 | 1413 | `tests/test_preview_controller.cpp`, `tests/test_preview_golden.cpp` |
| A22 | 1417 | `tests/test_preview_load_progress.cpp`, `tests/test_preview_renderer.cpp`, `tests/test_preview_timeline.cpp` |
| A23 | 839 | `tests/test_replay.cpp` (part 1 of 2) |
| A24 | 839 | `tests/test_replay.cpp` (part 2 of 2) |
| A25 | 1359 | `tests/test_report_page.cpp`, `tests/test_report_rows.cpp` |
| A26 | 1187 | `tests/test_report_view.cpp`, `tests/test_rules.cpp`, `tests/test_run_guard.cpp` |
| A27 | 1462 | `tests/test_saved_figure_fill.cpp`, `tests/test_search_target.cpp`, `tests/test_song_audio.cpp`, `tests/test_song_input.cpp` |
| A28 | 862 | `tests/test_sp_end.cpp` |
| A29 | 729 | `tests/test_sp_meter_curve.cpp` |
| A30 | 993 | `tests/test_squeeze_rating.cpp` |
| A31 | 1327 | `tests/test_stem_reader.cpp`, `tests/test_store_library.cpp` |
| A32 | 1258 | `tests/test_store_open.cpp` |
| A33 | 793 | `tests/test_store_records.cpp` |
| A34 | 920 | `tests/test_store_results.cpp`, `tests/test_theme.cpp` |
| A35 | 619 | `tests/test_tied_variants.cpp` |
| A36 | 1114 | `tests/test_track_state.cpp` |
| A37 | 1500 | `tests/test_user_messages.cpp`, `tests/test_winstr.cpp`, `tests/ui/uitest_batch_reports.cpp` |
| A38 | 438 | `tests/ui/uitest_commands.cpp` |
| A39 | 1384 | `tests/ui/uitest_details.cpp` |
| A40 | 1147 | `tests/ui/uitest_engine.cpp`, `tests/ui/uitest_harness.cpp` |
| A41 | 1034 | `tests/ui/uitest_library.cpp` (part 1 of 2) |
| A42 | 1034 | `tests/ui/uitest_library.cpp` (part 2 of 2) |
| A43 | 1120 | `tests/ui/uitest_live.cpp`, `tests/ui/uitest_main.cpp` |
| A44 | 1057 | `tests/ui/uitest_paths.cpp` |
| A45 | 1110 | `tests/ui/uitest_preview.cpp` |
| A46 | 829 | `tests/ui/uitest_report_windows.cpp` (part 1 of 2) |
| A47 | 829 | `tests/ui/uitest_report_windows.cpp` (part 2 of 2) |
| A48 | 1455 | `tests/ui/uitest_script.cpp`, `tests/ui/uitest_tests.cpp`, `tests/pb_ui/pbtest_build.cpp`, `tests/pb_ui/pbtest_coach.cpp`, `tests/pb_ui/pbtest_files.cpp`, `tests/pb_ui/pbtest_harness.cpp` |
| A49 | 1341 | `tests/pb_ui/pbtest_main.cpp`, `tests/pb_ui/pbtest_window.cpp`, `tests/single_owner/scan.cpp` |
| B1 | 1372 | `tests/test_cli.cpp`, `tests/test_dynamics_breakdown.cpp`, `tests/test_fuzz_regressions.cpp` |
| B2 | 1425 | `tests/test_generated_fills.cpp`, `tests/test_midi.cpp`, `tests/test_pathstring_read.cpp` |
| B3 | 865 | `tests/test_replay_schedule.cpp` (part 1 of 2) |
| B4 | 865 | `tests/test_replay_schedule.cpp` (part 2 of 2) |
| B5 | 694 | `tests/test_replay_windows.cpp`, `tests/test_scoring.cpp` |
| B6 | 1392 | `tests/test_song_parse.cpp` |
| B7 | 460 | `tests/test_timing.cpp` |
| C1 | 1226 | `tests/test_activation_row_layout.cpp`, `tests/test_app_settings.cpp`, `tests/test_audio_decode.cpp`, `tests/test_audio_device.cpp`, `tests/test_audio_mixer.cpp`, `tests/test_audio_player.cpp`, `tests/test_badge_pill.cpp`, `tests/test_build_flags.cpp` |
| C2 | 1482 | `tests/test_chart_index.cpp`, `tests/test_chord_code.cpp`, `tests/test_difficulty_table.cpp`, `tests/test_disco_flip.cpp`, `tests/test_display_format.cpp`, `tests/test_dynamics_tag.cpp`, `tests/test_fast_tempo.cpp`, `tests/test_fill_landing.cpp` |
| C3 | 1348 | `tests/test_five_lane.cpp`, `tests/test_font_coverage.cpp`, `tests/test_http_download.cpp`, `tests/test_icons.cpp`, `tests/test_image_decode.cpp`, `tests/test_kick2x.cpp`, `tests/test_label_layout.cpp`, `tests/test_launch.cpp` |
| C4 | 653 | `tests/test_layering.cpp` |
| C5 | 1448 | `tests/test_library_model.cpp`, `tests/test_library_read_order.cpp`, `tests/test_list_scroll.cpp`, `tests/test_live_protocol.cpp`, `tests/test_note_shuffle.cpp` |
| C6 | 1349 | `tests/test_note_window.cpp`, `tests/test_obj_loader.cpp`, `tests/test_overlay_layout.cpp`, `tests/test_path_costs.cpp` |
| C7 | 1312 | `tests/test_pb_coach.cpp`, `tests/test_pb_files.cpp`, `tests/test_pb_state.cpp`, `tests/test_perf_digest.cpp`, `tests/test_phrase_end.cpp` |
| C8 | 1079 | `tests/test_precheck_check1.cpp`, `tests/test_precheck_check24.cpp`, `tests/test_precheck_check3.cpp`, `tests/test_precheck_check5.cpp`, `tests/test_precheck_git.cpp` |
| C9 | 1476 | `tests/test_precheck_pick.cpp`, `tests/test_precheck_pick_exe.cpp`, `tests/test_precheck_pick_lines.cpp`, `tests/test_precheck_pick_names.cpp`, `tests/test_precheck_rows.cpp`, `tests/test_preview_clock.cpp` |
| C10 | 1166 | `tests/test_preview_config.cpp`, `tests/test_preview_source.cpp`, `tests/test_preview_transport.cpp`, `tests/test_rules_file.cpp`, `tests/test_section_names.cpp` |
| C11 | 1154 | `tests/test_sng.cpp`, `tests/test_song_length.cpp`, `tests/test_song_panel_state.cpp`, `tests/test_sp_end_clamp.cpp` |
| C12 | 1440 | `tests/test_srb.cpp`, `tests/test_stars.cpp`, `tests/test_stream_mix.cpp`, `tests/test_strutil.cpp` |
| C13 | 1415 | `tests/test_text_decode.cpp`, `tests/test_text_role.cpp`, `tests/test_timeline_view.cpp`, `tests/test_update_release.cpp`, `tests/test_update_state.cpp`, `tests/test_update_verify.cpp` |
| C14 | 1031 | `tests/test_upgrade_files.cpp`, `tests/test_utf8_paths.cpp`, `tests/test_wait_util.cpp`, `tests/single_owner/clones.cpp`, `tests/single_owner/engine.cpp`, `tests/single_owner/special.cpp`, `tests/fuzz/fuzz_audio.cpp`, `tests/fuzz/fuzz_chart.cpp` |
| C15 | 24 | `tests/fuzz/fuzz_image.cpp`, `tests/fuzz/fuzz_mid.cpp`, `tests/fuzz/fuzz_sng.cpp`, `tests/fuzz/fuzz_srb.cpp` |
| D1 | 2306 | `tools/ch_mcp/tests/test_bot_run.py`, `tools/ch_mcp/tests/test_build_facts.py`, `tools/ch_mcp/tests/test_build_stamp.py`, `tools/ch_mcp/tests/test_chdis.py`, `tools/ch_mcp/tests/test_compare_dumps.py`, `tools/ch_mcp/tests/test_compare_scorestats.py` |
| D2 | 2471 | `tools/ch_mcp/tests/test_dump_facts.py`, `tools/ch_mcp/tests/test_foreground.py`, `tools/ch_mcp/tests/test_frida_attach.py` |
| D3 | 2296 | `tools/ch_mcp/tests/test_frida_tools.py`, `tools/ch_mcp/tests/test_front.py`, `tools/ch_mcp/tests/test_game_files.py` |
| D4 | 1778 | `tools/ch_mcp/tests/test_game_tools.py` (part 1 of 2) |
| D5 | 1778 | `tools/ch_mcp/tests/test_game_tools.py` (part 2 of 2) |
| D6 | 2413 | `tools/ch_mcp/tests/test_hydra_end.py`, `tools/ch_mcp/tests/test_kit_tools.py`, `tools/ch_mcp/tests/test_launch_core.py`, `tools/ch_mcp/tests/test_launch_patch.py` |
| D7 | 2217 | `tools/ch_mcp/tests/test_machine_recorder.py`, `tools/ch_mcp/tests/test_machine_report.py`, `tools/ch_mcp/tests/test_menu_tools.py` |
| D8 | 2186 | `tools/ch_mcp/tests/test_method_watch.py`, `tools/ch_mcp/tests/test_mono_facts.py`, `tools/ch_mcp/tests/test_press_schedule.py`, `tools/ch_mcp/tests/test_quiet.py`, `tools/ch_mcp/tests/test_recorder_link.py`, `tools/ch_mcp/tests/test_server.py` |
| D9 | 2329 | `tools/ch_mcp/tests/test_sp_tools.py`, `tools/ch_mcp/tests/test_trace_hits.py`, `tools/ch_probe/tests/test_cheap_checks.py`, `tools/ch_probe/tests/test_debugger.py`, `tools/ch_probe/tests/test_early_fill_walk.py`, `tools/ch_probe/tests/test_edge_probe.py` |
| D10 | 1505 | `tools/ch_probe/tests/test_engine.py`, `tools/ch_probe/tests/test_engine_finder.py`, `tools/ch_probe/tests/test_find_checksum.py`, `tools/ch_probe/tests/test_hit_window_scripts.py`, `tools/ch_probe/tests/test_input_driver.py`, `tools/ch_probe/tests/test_ocr.py`, `tools/ch_probe/tests/test_overlay_logs.py` |
| D11 | 2312 | `tools/ch_probe/tests/test_play_chart.py` |
| D12 | 1567 | `tools/ch_probe/tests/test_play_in_game.py`, `tools/ch_probe/tests/test_prep_inputs.py`, `tools/ch_probe/tests/test_probe_chart.py`, `tools/ch_probe/tests/test_probe_songs.py`, `tools/ch_probe/tests/test_process.py`, `tools/ch_probe/tests/test_process_readonly.py`, `tools/ch_probe/tests/test_s2_window_constants.py`, `tools/ch_probe/tests/test_score_compare.py` |
| D13 | 2236 | `tools/ch_probe/tests/test_sp_compare.py`, `tools/ch_probe/tests/test_sp_watch.py`, `tools/ch_probe/tests/test_stall_harness.py`, `tools/ch_probe/tests/test_tick_rate_vs_hydra.py`, `tools/ch_probe/tests/test_watch_checks.py`, `tools/test_agent_cost.py` |
| D14 | 1626 | `tools/test_check_handoff.py`, `tools/test_check_pointers.py`, `tools/test_compare_db.py`, `tools/test_effort_test_faults.py`, `tools/test_effort_test_scout.py`, `tools/test_effort_test_shuffle.py`, `tools/test_effort_test_taint.py` |
| D15 | 2293 | `tools/test_hydra_ui_mcp.py`, `tools/test_build_pass.ps1`, `tools/test_ci_build_folder.ps1`, `tools/test_effort_test_guard.ps1`, `tools/test_mutation_probe.ps1` (removed; `git show ed0aa857:tools/test_mutation_probe.ps1`), `tools/test_release_sign.ps1` |

## The handoff's open points

**The seven older bad examples (B2, B3, B5, B6, B8, B9, B11).** Settled, no question. Their files are in the slice table like any other, so each test is judged fresh from the code. The auditor of those slices is told that the examples page's quoted lines may have drifted and that a page example is a claim (agent rule 9). Where a fresh record disagrees with the page, the difference is a side note, and fixing the page is a doc change after the audit. B3's rewrite is judged by the same route.

**Is `bank_rise_ticks` engine truth or a drawing rule (B5)?** Settled as an audit job, not a question. Rubric question 2 says to judge by the callers, and question 3 by the record on a real chart. The callers were read on 2026-11-14. The search stamps the field at `src/search/engine.cpp` line 2015. The Preview reads it (`src/app/preview_view.cpp` lines 158 and 426). `Activation::sp_ready_tick` reads it (`src/core/model.cpp` lines 695 to 698), and that has callers the handoff did not list: `early_fill_window_end` in `src/core/note_window.cpp` line 133, and the replay tools at `tools/replay_json.cpp` line 140 and `tools/replay_schedule.cpp` line 419. So it is not a Preview-only value. Whether anything uses it to choose or score a path is still open (P13). The auditor of the slice holding `tests/test_sp_end.cpp` gets this caller list as a starting point to check, and answers question 3 from T1's records. If the verdict stays Investigate, it goes to the user through T10 with the record's own lines.

**The reach rule behind 7285 (B13, FL-23).** Settled as an audit job. FL-23 on `docs/clone-hero/04-fills.md` (line 232) is labelled "Proven in the game's code". It says the game clears the fill at the deadline and Hydra keeps it until 85 ms past, through `fill_refuses` and `fill_e_offset` in `src/core/model.h`. Its Ruling line cites D250 and FL-24. The auditor of `tests/test_fills_and_skips.cpp` does questions 2 and 3 for it: the callers of `fill_refuses`, and a skipped fill in a T1 record. T1's case list includes a skipped fill for this reason.

**The 2026-10-25 report.** It sits untracked at `docs/handoffs/2026-10-25-test-audit/report.md` in the main checkout. It looked for duplicates, slow tests and false confidence. The audit reuses two things from it, both as leads and never as verdicts. Its section 5 names planted bugs that survived (the SMPTE refusal in `test_midi.cpp`, the http cancel poll, the button colour in `test_theme.cpp`); the auditors of those files are pointed at section 5 and check whether each still holds. Its section 4, the redundancy map, is a lead for question 10. Its counts (1,663 test cases) are stale and are not used. The file stays untracked; nobody in this work commits it (agent rule 13).

**Test names with commas.** Settled, no question. doctest's `-tc` filter splits on commas, so such a test cannot be run alone. That matters to this audit's own T7, which runs named tests. So an auditor writes a side note for every test name that holds a comma, and T7 picks a bug whose test has none. Renaming is a fix, and waits with the others.

## The last two mechanical checks

The rubric names two more checks: a test that compares every display with the engine's record, chord by chord, on every corpus chart; and a standing scan that fails a corpus total compared with a literal. Both are code changes. Each gets its own plan after this audit, and neither is planned or built here. The audit's findings feed them: T6's list says which totals exist today.

## Missing coverage

The rubric says missing coverage is a separate pass after the audit: list the rules on the `docs/clone-hero/` pages and the filed rulings, and flag each one no test checks. It is not part of this plan.

## Out of scope, for the user to approve

- The hook tests in `C:\Users\Patrick\.claude\gates\`. They are another repo's tests.
- `tools/ch_probe/experiments/key_delivery_test.py` and `tools/ch_probe/experiments/pad_flash_test.py`. They are scripts that drive the real game, not tests (`docs/development.md` lines 307 to 309).
- Every fix. Rewrites, removals, renames and production-code fixes get their own plan after the answers.
- The two code-change mechanical checks, and the missing-coverage pass, as said above.
- Fixing the Preview bug. It has its own handoff.

## Questions

1. The audit as planned is 86 auditor agents, about 43 verifiers and a handful of closure checkers, plus two full-suite runs and about 86 planted bugs built by the main session. Is that size approved? This is a permission question (agent rule 3).
2. May the hook tests in `C:\Users\Patrick\.claude\gates\`, and the two experiment scripts under `tools/ch_probe/experiments/`, stay out of this audit? Both narrow the scope of "the entire test suite", so only the user can say yes (audit-method stage 1).

## Promise map

| Promise | Plan section | Task | The check that proves it |
|---|---|---|---|
| Every test file in the suite is in a slice | Scope, The slices | T0 | `coverage.md`: zero files with no slice |
| Every test case gets a record or is named "not audited" | T5 | T5 | records plus "not audited" names equal the test-case count per slice |
| The audit changes no test and no code | The short version | T11 | `git diff --stat main...claude/test-audit-records` shows only files under `docs/audit/2026-11-14-test-audit/` and the decision log |
| No agent builds or runs a built test | Who does what | T1, T6, T7 | every dump, run and planted bug is logged in `log.md` under the main session |
| Question 3 is answered from the engine's record or reads "claim not checked" | T1, The record | T5 | verifier checks each record's question 3 names a T1 file or says "claim not checked" |
| At most ten charts dumped; no whole-library run | T1 | T1 | `index.md` lists ten or fewer charts, all under `testdata/input` |
| The key is written before calibration and never shown | T2 | T2, T3 | key file's time is before T3's launch in `log.md`; the calibration brief is saved in the audit folder and holds no test name from the key |
| The key holds a test whose claim is the fault | T2 | T2 | the key names it with its engine record line; without one the main session stops |
| A calibration miss stops the audit; two runs at most | T3 | T3 | `log.md` line with the verdict |
| Slices are re-cut once at most | T4 | T4 | `log.md` line |
| Four auditors a round, display tests first | T5 | T5 | launch order in `log.md` matches the table |
| A verifier never checks a slice it audited, and checks every Keep | T5 | T5 | `verify/<id>.md` names its agent id, which differs from the records file's |
| An unfinished slice is re-sent once, by exact names | T5 | T5 | `log.md` re-send lines |
| The find stage has a stop rule that can be met | T5 | T5 | every slice has a records file or a "not audited" list |
| Every test that changes colour with the corpus is a finding | T6 | T6 | `mechanical/corpus.md` rows each matched in `exits.md` |
| One planted bug per auditor; a wrong claim triggers a closer read | T7 | T7 | `mechanical/planted.md` has one row per auditor |
| Every finding and split leaves by one of four exits, picked by the main session | T8 | T8 | `exits.md`: no empty exit |
| Every Rewrite, Remove and Investigate goes to the user | T8 | T8, T10 | every such row is exit 2 and points at a question |
| A fresh agent checks every closure; exit 3 quotes are grepped | T9 | T9 | `closures.md` verdicts and grep lines |
| Questions go in small groups with real before and after text | T10 | T10 | `questions.md` |
| No question offers a rule the plan made up | T10 | T10 | the scout named in T10 lists, per question, the engine value, decision or ruling it rests on; `questions.md` keeps that list beside the question |
| No rewrite proposes a saved expected-output file | What every brief carries | T5 | verifier flags any record that does |
| Every answer becomes a D-record | T11 | T11 | `questions.md` names each D-record |
| The seven older examples are judged fresh | The handoff's open points | T5 | their files' records exist |
| Comma names are flagged | The handoff's open points | T5 | side notes in records; T7 picks no comma-named test |

This plan has no live step. Nothing in it starts Clone Hero.

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The suite has 142 test files directly under `tests/`, 13 under `tests/ui/`, 6 under `tests/pb_ui/`, 4 under `tests/single_owner/` and 6 under `tests/fuzz/`. | code | `tests/` listed at commit `003d76a3` | checked 2026-11-14 |
| P2: 62 files under `tools/` are named `test_*.py`. | code | `tools/ch_mcp/tests/`, `tools/ch_probe/tests/`, `tools/` | checked 2026-11-14, by a walk for that name pattern only |
| P3: Test files outside the folders and name patterns of P1 and P2 are five `tools/test_*.ps1` scripts and two experiment scripts, and nothing else. | code | `CMakeLists.txt` lines 739 to 747 (the `hydra_tests` sources; there is no `tests/CMakeLists.txt`), `docs/development.md` lines 289 to 309, `.github/workflows/ci.yml` lines 128 and 131; a find over the repo, with `third_party` and the build folders left out, for the names `test_*`, `*_test.*` and `*test*.ps1` | checked 2026-11-14, by name patterns only; T0 lists again from the build files |
| P4: 28 of the 142 files call `corpus::chart_paths`. | code | grep of `tests/*.cpp` for `corpus::chart_paths` | checked 2026-11-14 |
| P5: The doctest files hold about 2,127 test cases. | unchecked | `tests/*.cpp` holds 2,108 lines that start `TEST_CASE` and `tests/single_owner/*.cpp` holds 19; the brief for this plan reported 2131 from `hydra_tests.exe --count`, which is 4 more, and nothing explains the gap | T5 (the main session's own `--list-test-cases` run settles it) |
| P6: The rubric, its examples page and the audit method are on main. | code | `docs/agents/test-rubric.md`, `docs/agents/test-rubric-examples.md`, `docs/agents/audit-method.md` | checked 2026-11-14 |
| P7: The ruling against saved expected-output files is filed. | ruling | `docs/agents/rulings/code-and-engine.md` line 50 | checked 2026-11-14 |
| P8: No plan under `docs/superpowers/plans/` is about the test suite. | code | `docs/superpowers/plans/` (16 titles read; a case-insensitive search of all 16 for "rubric", "test suite" and "test audit" finds only this plan) | checked 2026-11-14, by title and keyword, not by reading each plan whole |
| P9: `hydra_replay` has `dump` and `schedule` commands. | code | `tools/replay.cpp` lines 868 and 870 | checked 2026-11-14; their arguments were not read, and T1 reads the usage text |
| P10: The five named corpus charts between them show a squeeze-in, a squeeze-out, a skipped fill and an early fill. | unchecked | `tests/test_fast_tempo.cpp` lines 1 to 6 and 68 to 74 (squeeze-in cases on `test_fast_tempo` and `test_folded_sqin`); `test_flammarker` and `test_path_reduction` have no stated tie to Star Power; no engine record was read for any chart | T1 |
| P11: `tests/test_preview_scene.cpp` holds at least one test whose claim is the fault. | unchecked | `tests/test_preview_scene.cpp` (test names and a grep read, no test judged) | T2 |
| P12: One Sonnet scout can judge a 1,500-line slice inside the hook's limits. | unchecked | `docs/agents/brief-preamble.md` (the limits: 150 calls or 30 minutes) | T3, then T4 |
| P13: Nothing in the search uses `bank_rise_ticks` to choose or score a path. | unchecked | `src/search/engine.cpp` line 2015 (the stamp; the rest of the search was grepped by name only) | T5 |
| P14: `Activation::sp_ready_tick` is called from `early_fill_window_end` and the replay tools as well as read by the Preview's field. | code | `src/core/note_window.cpp` line 133, `tools/replay_json.cpp` line 140, `tools/replay_schedule.cpp` line 419 | checked 2026-11-14 |
| P15: FL-23 is labelled "Proven in the game's code" and names `fill_refuses`. | data | `docs/clone-hero/04-fills.md` line 232, block FL-23 | checked 2026-11-14 |
| P16: The 2026-10-25 report is untracked and its section 5 lists planted bugs that survived. | code | `docs/handoffs/2026-10-25-test-audit/report.md` lines 122 to 142, in the main checkout | checked 2026-11-14 |
| P17: Neither rubric page names `tests/test_preview_scene.cpp` or `tests/test_preview_boxes.cpp`. | code | `docs/agents/test-rubric.md`, `docs/agents/test-rubric-examples.md` (every `tests/` path in both listed) | checked 2026-11-14 |
| P18: The handoff's Claims all check out, apart from four commits not pushed. | code | `tools/check_handoff.py` run on the handoff: no `MISSING`, no `CHANGED SINCE` | checked 2026-11-14 |
| P19: Agents are stopped at 150 tool calls or 30 minutes. | code | `docs/agents/brief-preamble.md` line 69 | checked 2026-11-14 |
| P20: D43 approves `acts > 1000` "as test limits". | code | `docs/audit/2026-10-03-fix-decisions.md` line 101 | checked 2026-11-14, first 200 characters of the line |
