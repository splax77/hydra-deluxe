# Handoff: CI and test tooling (2026-10-10)

**For the next session: build this plan.** On 2026-10-10 the user read a summary of this plan and replied "approve all recommendations". That yes covers the design under repo rule 3. Every decision below is settled. Nothing has been built yet.

The plan is `docs/superpowers/plans/2026-10-10-ci-test-tooling.md`. Read it first; this note only records what was decided and how it fits with the other work running now.

## Why this exists

A read-only audit on 2026-10-10 (`docs/audit/2026-10-10-test-suite-audit.md`) found that CI runs only `hydra_tests` and `hydra_uitest`. Nothing runs the 12 `ch_probe` pytest files, `tools/test_compare_db.py` or the two `.ps1` self-tests. The C++ suite runs as one serial exe. One full run on 2026-10-10 had 1,250 cases and took 42 seconds, 18.2 of them in one test; the per-test timings are in `docs/handoffs/2026-10-10-test-audit/suite-durations.txt`. There is no sanitizer, coverage or fuzzing. The GUI suite can disagree between its serial `--all` mode and CI's `--jobs 4`. The tools scout's survey is `docs/handoffs/2026-10-10-test-audit/W2-tools.md`.

## What the user decided

A new `tool-tests` CI job runs the Python and PowerShell tool tests. It uses `fetch-depth: 0` so the precheck self-test's real-range part runs. Measure the checkout time on the first run, and drop back to a shallow clone if it's over a minute.

`hydra_tests` is sharded into 4 slices by source file under CTest, plus a cover-all check that fails if any test case falls outside every slice. Agents keep running only their own tests (rule 1). The main session's one run at merge becomes `ctest -j 4`.

The GUI suite gets one run mode, a fresh process per test everywhere, with a default of 4 jobs for `--all`. `kRunOrder` goes. `docs/agents/ui-testing.md` drops its standing test count (it says 85; `hydra_uitest --list` prints 88) and its two stale timings. It gets one fresh timing with its date and run count instead.

An AddressSanitizer job runs on every push and PR, beside the existing job. New CMake options `HYDRA_ASAN` and `HYDRA_MIMALLOC` keep the two from ever being built into one exe, because both hook the heap. Fall back to nightly only if the job proves flaky.

Coverage comes from `Microsoft.CodeCoverage.Console`, after a probe step proves it handles native C++. The report goes to a workflow artifact and the job summary only, on pushes to main only, with no threshold and no Codecov for now.

Fuzzing runs weekly: 10 minutes per target, six parser entry points in parallel. Crash files are checked into `testdata/fuzz/`, and a table-driven doctest replays them.

RapidCheck is vendored for one trial on the replay's "window order does not matter" property, with a fixed seed. A short note afterwards says whether its shrinking earned its keep.

The user also said yes to a one-line follow-up outside the plan's tasks. CI pins `runs-on: windows-2025-vs2026`, which GitHub's changelog called a pre-migration test label. Try `windows-2025` on a branch, and switch if it builds.

## How to run it

Wave 1 holds tasks 1, 2 and 7, with task 3 beside them. Then task 4. Then tasks 5 and 6, which need task 4's no-mimalloc build. The plan estimates four to five executor-days in all. Executors run on Opus, and each merge gets one fresh Sonnet reviewer under D107. The doc change in task 2 also goes through the doc-review gate. The repo `splax77/hydra-deluxe` is public, so Actions minutes are free.

## Fits with other work

Task 2 edits `main` in `tests/ui/uitest_main.cpp`, where the "Cap every uncapped wait in the test suites" session is replacing an `INFINITE` wait. Task 2 starts after that session merges. Task 3's slice list should be balanced after the "Trim the slow test and dead tests" session merges, because removing its 18-second case changes the balance. The plan estimates the longest slice at about 7.4 seconds once that lands. Any `ctest` registration change must keep the cover-all check passing.
