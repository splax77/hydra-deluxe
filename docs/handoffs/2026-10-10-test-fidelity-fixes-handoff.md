# Handoff: test fidelity fixes (2026-10-10)

**For the next session: build this plan, but bring the golden thresholds back to the user.** On 2026-10-10 the user read a summary of this plan and replied "approve all recommendations". That yes covers the design under repo rule 3. Three questions in it have no single recommendation, so they stay open (see the last section). Nothing has been built yet.

The plan is `docs/superpowers/plans/2026-10-10-test-fidelity-fixes.md`. Read it first; this note only records what was decided and how it fits with the other work running now.

## Why this exists

A read-only audit of the test suite on 2026-10-10 (`docs/audit/2026-10-10-test-suite-audit.md`) found tests whose expected answer shares production's mistakes, or is too loose to catch a real bug. The old-database upgrade tests build their "old" files from column lists production also uses. The golden-image test passes on a whole-frame average error under 10 out of 255, and passes silently when its fixture is missing. The stem-skipping rule is tested against a copy of the loop. Two tests recompute their expected values. The engine digest is pinned only at default settings. Three tests fail or pass on wall-clock time. The one audio-device test proves only that the constructor doesn't throw. The scouts' evidence is in `docs/handoffs/2026-10-10-test-audit/` (A-engine, B1-store-app and B2-preview-report).

## What the user decided

The old-database fixtures come from four real releases: v1.8.4, v1.8.4 run with `--legacy-fills`, v2.0.0 and v2.1.0. Each is built from its tag in a worktree, run on three corpus charts, and saved with `VACUUM INTO`. The cap is 256 KB per file and 1 MB for the set. If a file comes out bigger, it drops to two charts. `kSchema2ResultsTableSql` then leaves production, because its only reader was the synthetic fixture.

The golden test gets a per-pixel delta, a worst-tile budget and a whole-frame budget. A missing fixture fails. The fallback tolerance in the code goes, and a README in `testdata/preview/` explains how to recapture. The thresholds themselves are still open; see below.

The engine digest gets pinned at three more settings: Hard with no Pro and no 2x kick; CH 1.0 fills; and Note Shuffle with the ms limit off. It also checks the chart count. Each pin adds about 0.1 seconds.

The two "under 20 ms" checks become pinned row counts. The wall-clock ratio test in `test_path_view.cpp` is deleted, because a build-count test already covers it.

The audio-device test is kept and strengthened. It asserts a new `started()` and that the first callback ran within a 2-second cap. The 2 seconds is a hang detector, not a timing test.

## How to run it

Tasks 2 to 7 run in parallel. Task 1 has a build half that runs alone first: building old tags under VS 2026 may fail, so it has a 10-minute stop per tag, and the known workaround is in memory `vs2026-incomplete-instance`. The old `hydra_batch` must run from its own build folder so it finds mimalloc's DLLs. Task 1's tests then join the wave. The plan estimates about two agent-days. Executors run on Opus, and each merge gets one fresh Sonnet reviewer under D107. Task 1 removes a production constant that a single-owner row may name; the executor updates that row.

One extra item came from the duplicates planner. `tests/test_sng.cpp` around line 110 recomputes the format's XOR formula. That planner judged it a spec check, and this plan's executor should confirm or replace it with a pinned table like the other recomputed oracles.

## Fits with other work

The "Trim the slow test and dead tests" session owns several nearby fixes and must not be duplicated. These are the no-assertion "upgrade timing" test, `leak_checked`, "every corpus chord", the one-chart loop in `test_rules`, the 300 MB Opus cancel test and `delete_results_without_chart`. The "Cap every uncapped wait" session owns all uncapped waits. The new 2-second callback cap in Task 7 should use that session's shared wait helper if it has merged.

## Still open: ask the user

The golden thresholds are decided after Task 2's measurement step. That step reports the current worst tile and per-pixel delta percentiles, and the user picks the per-pixel delta, the worst-tile budget and the whole-frame budget from that evidence. The plan's starting suggestion is to tighten the mean from 10 to 7, with the option of correcting the capture's gamma first and then setting a tighter mean near 2. Bring the numbers and options to the user before building the check.

The plan also asks whether the user remembers the Onyx window size, and how the pause landed on 0:36.913. If not, the recapture README says "confirm at the next recapture".

Finally, `start()` swallows a failed `ma_device_start` today. Should that failure be shown to the user? That would be a follow-up outside this plan, and a visible change, so it needs the user's call.

## Outcome (2026-10-10, session 88f58f4f)

Built and merged. All three open questions were answered; the answers are in the plan's "User decisions" items 3, 4a and 8a. The golden test uses D 32, a 9.4 percent worst-tile budget, a 0.08 percent whole-frame budget and a mean tolerance of 7. The recapture README marks the window size and pause method "to confirm". The visible `ma_device_start` failure is a proposed follow-up session, not built here.

Merges on main: tf-t3, tf-t4, tf-t5, tf-t6 and tf-t7 (25b4c8de to 42084836), then tf-t2, tf-t7b and tf-t1 (16096abf to e20b09e7). Each had one clean Sonnet review, and the full suite passed on each joined tree.

Three things differed from the plan. Both v1.8.4 fixtures keep no results rows after today's upgrade, because the copy leaves out rows with a score and no stars (ADR 0026). The test pins that instead of "3 rows, Stale". The v2.1.0 release writes `user_version` 0, not 4. The `charts` table is empty in every fixture (hydra_batch never fills it), so rows are matched by hyhash. The engine digest's analysed count is checked against the charts that load at each pin's settings, because 30 corpus charts have no Hard part.

Small things left, none blocking. The "stars:7" library query pins 0 rows, because the synthetic data never gives 7 stars. `results_rows_in` in test_store.cpp and `results_rows` in test_app_state.cpp wrap the same one-line count. `tools/build_slot.ps1` runs `build_cpp.ps1` from the caller's folder, which built the wrong tree for old tags. Old-tag builds also need a short path, because the scratchpad path breaks MSBuild's 260-character limit.
