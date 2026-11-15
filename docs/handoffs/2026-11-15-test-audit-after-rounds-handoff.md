# Test-suite audit: the find and verify rounds are done, the checks and exits are next (2026-11-15)

Session 16892967-2cf8-4437-a7cd-e3dad250f621. It continued the handoff `docs/handoffs/2026-11-15-test-audit-find-rounds-handoff.md` (removed; `git show 482e1b04f82b6ecbf8f12b80fa4cba71906ea427:docs/handoffs/2026-11-15-test-audit-find-rounds-handoff.md`).

## Goal

Finish the audit of every test in Hydra's suite, by the approved plan `docs/superpowers/plans/2026-11-14-test-suite-audit.md`. The audit is read-only. It writes records, verifies them, chooses each finding's exit, checks every closure and asks the user. It fixes nothing; every fix waits for the user's yes and gets its own plan. All of its work is on branch `claude/test-audit-records`, in `docs/audit/2026-11-14-test-audit/`, and this handoff sits on that branch.

## What was done this session

**T5 is complete.** All 92 slices have a records file in `records/`, from A1 to E4, plus `records/C14-C15-fuzz.md` for six fuzz files and `engine.cpp`, which the C14 and C15 auditors left out. Every slice has a verify file in `verify/`. A verifier that ran out of budget got one re-send, and those are the `-resend` files. No record is left unreached. The last two rounds covered the hook tests. Round 23 had four auditors on E1 to E4. Round 24 had eight verifiers, two per E slice, because those slices hold 158 to 307 records each.

**Reading rules set during T5.** Each one is in `log.md`, at the round where it was set, and every brief from then on carries it. In short:

- Question 3 for a claim that is not about a chart needs the real thing. A files, store or settings claim is checked on real input against an independent owner. A claim about what a screen shows needs a capture. A hand-built fixture alone is "claim not checked".
- Threads, timing and transports count as checked on the real mechanism: the real pool, the real clock, real WinHTTP against a loopback server.
- A hook test counts as checked when it feeds the hook a recorded tool call or a real git state.
- A test whose only fault is an unchecked claim is "Investigate, claim not checked only", not Rewrite.
- A verifier reads the code behind every record. Agreeing because a verdict looks right is not a check.

**What went wrong, and how it was handled.** At first the auditors read question 3 in different ways, and that inflated the Keeps. A fresh recheck of A4's Keeps left 11 of 25 standing (`verify/A4-q3.md`). The rule above came from that. The E auditors stated many answers once, in shared blocks, and read many hooks only in outline. Their verifiers read the hooks in full, and they disputed between 6 and 27 verdicts a part. The E1 auditor's commit `c09246235da66296cffd4f97db666325d1185885` also swept in E4's staged records file, so E4's first records sit in that commit.

**The capture defect** (from A29): `records-screen/moments.py` places the "SP end" screenshots just before the end. For example, `believer_path4.cmd.txt` sets 109.255572 s while the engine's end is 109255.57244 ms. So those frames still draw Star Power as running, and no capture sits at or after a window's end.

## The user's words this session

"cont worktrees\test-audit-records\docs\handoffs\2026-11-15-test-audit-find-rounds-handoff.md"

"31 keep tests in a4? give me an example" (answered with test 48 in `tests/test_analysis.cpp`, checked against `src/app/analysis.cpp`)

These are chat words, not filed rulings. The words from the session before, which still bind this audit, are quoted in the Claims table.

## What is next

1. Check every claim below against the code (agent rule 9).
2. **Main has moved.** The audit branch last took main at `c10c7f5485cb34ab77e15c7298403223771b5ccc`. Main is now at `e60ab3c3f60d34cbd9a0135e18a991d3313fcc15`, 68 commits later, from the batch-rows work. Those commits changed 26 test files: the C++ files `tests/test_activation_rows.cpp`, `test_app_click.cpp`, `test_app_library.cpp`, `test_app_reports.cpp`, `test_batch_text.cpp`, `test_cli.cpp`, `test_config.cpp`, `test_docs_match_code.cpp`, `test_engine_sweeps.cpp`, `test_fills_and_skips.cpp`, `test_library_jobs.cpp`, `test_library_model.cpp`, `test_model.cpp`, `test_preview_controller.cpp`, `test_store_library.cpp`, `test_store_open.cpp`, `test_store_records.cpp` and `test_store_results.cpp`; the UI tests `tests/ui/uitest_details.cpp`, `uitest_library.cpp` and `uitest_live.cpp`; the helpers `tests/app_state_fixtures.h`, `corpus_util.h` and `song_digest.h`; one rule file, `tests/single_owner/rules/tests/concept-G5.json`; and `tools/test_compare_db.py`. They also added two helpers, `tests/store_file_fixtures.h` and `tests/store_lock_holder.h`. Merge main into the audit branch. Then audit and verify only what changed in those files (`git diff c10c7f54 main -- <file>`), marking each new or changed test's record as from the delta round. Do this before T8, so the exits are chosen for the tests main really has.
3. **The sweep.** Records from the rounds before the "claim not checked only" rule may still give Rewrite where the only fault is question 3. A9 and A14 are known cases. One fresh agent reads every Rewrite in `records/` and its verify file, and lists each one whose only fault is question 3. The main session moves those to Investigate in T8.
4. **The capture pass** (plan T1's rule, after the last find round). First fix the rounding in `moments.py`, so that each end frame lands at or after the end. Then collect every capture and real input the records name under "claim not checked", across `records/` and `verify/`, and capture them. Screens go in `records-screen/` as command files, the way its index says. Charts go in `records-engine/` through `hydra_replay`. Real hook inputs come from recorded calls (`gates\tests\replay_extract.py`) and from the real state files the E1a verifier found (`gates\state\agents\*\*.handback` and `.json`). Builds and runs are the main session's, through `hydra_build_pass`. Rebuild `hydra_uitest` and `hydra_replay` first, because `src/` has changed since c10c7f54. After the pass, each "claim not checked only" record whose claim holds may go to Keep.
5. **T6 and T7** are the main session's. T6 changes the corpus in a throwaway worktree and runs the full suite twice (once with one chart added, once with one unnamed chart removed), writing `mechanical/corpus.md`. T7 plants one auditor's named bug per auditor and runs the tests it names, writing `mechanical/planted.md`. The plan's T6 and T7 sections give the steps and say which rule allows each run.
6. **P5**, the plan's unchecked count of doctest cases: run `hydra_tests.exe --list-test-cases`, `hydra_uitest.exe --list` and `hydra_pathbuilder_uitest.exe --list`, and compare the totals with the records.
7. **Count check per slice.** For every slice, count Keep, Rewrite, Remove, Investigate and "not reached" from the records file after its verify file's disputes. Write the table into `log.md`. Every test case in `coverage.md` must have exactly one record.
8. **T8 to T11**, as the plan says: choose each finding's exit (only the main session picks one), have a fresh agent check every closure, ask the user the questions, and record. Each round's findings and splits are in `log.md`, and in each records file's and verify file's findings section.
9. Open points from earlier sessions that still stand. The early-fill setting plan is on `claude/early-fill-setting-plan`. When its code lands, re-dump and recapture any record whose E mark could move. Beg's E0 cannot move. `claude/early-fill-reach-slack-handoff` was overtaken by a filed ruling, and the user decides whether to drop it. The throwaway calibration worktree `worktrees/test-audit-cal2` (branch `claude/test-audit-cal2`) can go once the user agrees.

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| branch | `claude/test-audit-records` | the audit's branch: coverage, engine and screen records, every slice's records and verify files, briefs, log, this handoff; not merged | `git log main..claude/test-audit-records` |
| commit | `482e1b04f82b6ecbf8f12b80fa4cba71906ea427` | round 24's results logged; the find and verify rounds are done; on the audit branch, not on main | `git show --stat` |
| commit | `e60ab3c3f60d34cbd9a0135e18a991d3313fcc15` | `main` when this handoff was written; not in the audit branch; local only, not pushed | `git rev-parse main`; `git merge-base --is-ancestor e60ab3c3 claude/test-audit-records` fails |
| commit | `c10c7f5485cb34ab77e15c7298403223771b5ccc` | the last main commit the audit branch holds, and the commit the screen records were built at; on main, not pushed | `git merge-base main claude/test-audit-records` |
| commit | `326cd4e0c24933cedc7bdcc5a0cd47e856adbc29` | the main commit the engine records were made from; on main, not pushed | `git show --stat` |
| commit | `c09246235da66296cffd4f97db666325d1185885` | the E1 auditor's commit that also swept in E4's staged records file; not on main | `git show --stat` |
| commit | `4c6272e22896f4b824dfab67b71801e142de9b47` | the rubric change for question 3; on main and pushed | `git show --stat` |
| path | `docs/superpowers/plans/2026-11-14-test-suite-audit.md` | the approved plan, T0 to T11, with P5 still unchecked | read it |
| path | `docs/audit/2026-11-14-test-audit/coverage.md` | every test file and its slice: 263 files, 92 slices | read it |
| path | `docs/audit/2026-11-14-test-audit/log.md` | every round in order, with its commits, counts, findings, splits and the reading rules set | read it |
| path | `docs/audit/2026-11-14-test-audit/records/E4.md` | the last slice's records, 307 headed records | read it |
| path | `docs/audit/2026-11-14-test-audit/records/C14-C15-fuzz.md` | the 22 records for the six fuzz files and `engine.cpp` that C14 and C15 left out | read it |
| path | `docs/audit/2026-11-14-test-audit/verify/E4b.md` | the last verify file of round 24 | read it |
| path | `docs/audit/2026-11-14-test-audit/verify/A4-q3.md` | the fresh recheck of A4's Keeps: 11 of 25 stand | read it |
| path | `docs/audit/2026-11-14-test-audit/records-screen/moments.py` | writes the playhead and screenshot lines; its SP-end rounding lands just before the end | read it |
| path | `docs/audit/2026-11-14-test-audit/records-screen/index.md` | the screen records, how to re-run them, what is not captured | read it |
| path | `docs/audit/2026-11-14-test-audit/records-engine/index.md` | the seven charts' engine records and which case each shows | read it |
| path | `tests/test_store_records.cpp` | one of the 26 test files main changed after c10c7f54 | `git diff --stat c10c7f54 main -- tests/test_store_records.cpp` |
| path | `tests/ui/uitest_live.cpp` | one of the 26 test files main changed after c10c7f54 | `git diff --stat c10c7f54 main -- tests/ui/uitest_live.cpp` |
| path | `C:\Users\Patrick\.claude\gates\tests\replay_extract.py` | writes recorded real hook calls, a real input for the hook tests | read it |
| state | 26 test files changed on main since the audit's base | the list in step 2 of "What is next" | `git diff --name-only --diff-filter=M c10c7f54 main -- tests tools` lists 33; the 7 not named are five single-owner rule files, `tools/bench.cpp` and `tools/replay.cpp` |
| state | the hook repo `C:\Users\Patrick\.claude` at `0586e15` | the commit the E slices were read at | `git -C C:\Users\Patrick\.claude rev-parse --short HEAD` |
| branch | `claude/early-fill-setting-plan` | another session's plan for the adjustable early-fill setting | `git log -1 claude/early-fill-setting-plan` |
| branch | `claude/early-fill-reach-slack-handoff` | an early-fill handoff overtaken by a filed ruling; not merged; the user decides | `git log main..claude/early-fill-reach-slack-handoff` |
| branch | `claude/test-audit-cal2` | the throwaway tree for calibration run 2; not merged, not to be merged | `git log claude/test-audit-records..claude/test-audit-cal2` |
| ruling | `16892967-2cf8-4437-a7cd-e3dad250f621` | "cont worktrees\test-audit-records\docs\handoffs\2026-11-15-test-audit-find-rounds-handoff.md" | this session's card, once written |
| ruling | `16892967-2cf8-4437-a7cd-e3dad250f621` | "31 keep tests in a4? give me an example" | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "No, audit them too" | no card for that session yet; quoted in the removed find-rounds handoff |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "yes use a workflow" | no card for that session yet; quoted in the removed find-rounds handoff |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "well the display doesnt exist in a vacuum, does it? its displaying things that are tied to real charts. and we can measure those." | no card for that session yet; quoted in the removed find-rounds handoff |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "no. you dont get to stop and refuse to do the work. figure out how to get what you need and do it." | no card for that session yet; quoted in the removed find-rounds handoff |
| ruling | `a3ba79d1` | "so we should use the hit window. and stop inventing our own numbers." | `docs/agents/rulings/code-and-engine.md`, under `## Confirmed` |
| ruling | `a3ba79d1` | "make the early fill threshold an adjustable setting pinned to one number in both directions" | `docs/agents/rulings/code-and-engine.md`, under `## Confirmed` |

## Checker output

```
ok | branch | claude/test-audit-records | the audit's branch: coverage, engine and screen records, every slice's records and verify files, briefs, log, this handoff; not merged
not-on-main | commit | 482e1b04f82b6ecbf8f12b80fa4cba71906ea427 | round 24's results logged; the find and verify rounds are done; on the audit branch, not on main
not-pushed | commit | e60ab3c3f60d34cbd9a0135e18a991d3313fcc15 | `main` when this handoff was written; not in the audit branch; local only, not pushed
not-pushed | commit | c10c7f5485cb34ab77e15c7298403223771b5ccc | the last main commit the audit branch holds, and the commit the screen records were built at; on main, not pushed
not-pushed | commit | 326cd4e0c24933cedc7bdcc5a0cd47e856adbc29 | the main commit the engine records were made from; on main, not pushed
not-on-main | commit | c09246235da66296cffd4f97db666325d1185885 | the E1 auditor's commit that also swept in E4's staged records file; not on main
ok | commit | 4c6272e22896f4b824dfab67b71801e142de9b47 | the rubric change for question 3; on main and pushed
ok | path | docs/superpowers/plans/2026-11-14-test-suite-audit.md | the approved plan, T0 to T11, with P5 still unchecked
ok | path | docs/audit/2026-11-14-test-audit/coverage.md | every test file and its slice: 263 files, 92 slices
ok | path | docs/audit/2026-11-14-test-audit/log.md | every round in order, with its commits, counts, findings, splits and the reading rules set
ok | path | docs/audit/2026-11-14-test-audit/records/E4.md | the last slice's records, 307 headed records
ok | path | docs/audit/2026-11-14-test-audit/records/C14-C15-fuzz.md | the 22 records for the six fuzz files and `engine.cpp` that C14 and C15 left out
ok | path | docs/audit/2026-11-14-test-audit/verify/E4b.md | the last verify file of round 24
ok | path | docs/audit/2026-11-14-test-audit/verify/A4-q3.md | the fresh recheck of A4's Keeps: 11 of 25 stand
ok | path | docs/audit/2026-11-14-test-audit/records-screen/moments.py | writes the playhead and screenshot lines; its SP-end rounding lands just before the end
ok | path | docs/audit/2026-11-14-test-audit/records-screen/index.md | the screen records, how to re-run them, what is not captured
ok | path | docs/audit/2026-11-14-test-audit/records-engine/index.md | the seven charts' engine records and which case each shows
ok | path | tests/test_store_records.cpp | one of the 26 test files main changed after c10c7f54
ok | path | tests/ui/uitest_live.cpp | one of the 26 test files main changed after c10c7f54
ok | path | C:\Users\Patrick\.claude\gates\tests\replay_extract.py | writes recorded real hook calls, a real input for the hook tests
manual | state | 26 test files changed on main since the audit's base | the list in step 2 of "What is next"
manual | state | C:\Users\Patrick\.claude | the commit the E slices were read at
ok | branch | claude/early-fill-setting-plan | another session's plan for the adjustable early-fill setting
ok | branch | claude/early-fill-reach-slack-handoff | an early-fill handoff overtaken by a filed ruling; not merged; the user decides
ok | branch | claude/test-audit-cal2 | the throwaway tree for calibration run 2; not merged, not to be merged
manual | ruling | 16892967-2cf8-4437-a7cd-e3dad250f621 | "cont worktrees\test-audit-records\docs\handoffs\2026-11-15-test-audit-find-rounds-handoff.md"
manual | ruling | 16892967-2cf8-4437-a7cd-e3dad250f621 | "31 keep tests in a4? give me an example"
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "No, audit them too"
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "yes use a workflow"
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "well the display doesnt exist in a vacuum, does it? its displaying things that are tied to real charts. and we can measure those."
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "no. you dont get to stop and refuse to do the work. figure out how to get what you need and do it."
manual | ruling | a3ba79d1 | "so we should use the hit window. and stop inventing our own numbers."
manual | ruling | a3ba79d1 | "make the early fill threshold an adjustable setting pinned to one number in both directions"
checked 33 rows: 18 ok, 0 missing, 2 not-on-main, 3 not-pushed, 10 manual
```
