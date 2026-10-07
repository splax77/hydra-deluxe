# Handoff: speedups wave 2, the mimalloc trial and the follow-ups (2026-10-06)

Written 2026-10-06, late evening. Nothing is running. Wave 0 and wave 1 of the speedups plan are merged to main. Main is at 92bd322, 48 commits ahead of origin/main, and nothing is pushed. The plan is [2026-10-06-perf-speedups.md](../superpowers/plans/2026-10-06-perf-speedups.md). This handoff says what is done, what is left, and how the user wants the rest run.

## Read this first: how the user wants checks run

The user stopped two rounds of extra testing in this session, and both rules now bind the rest of the plan.

First, parallel tasks don't each run whole-library checks. The experiments had already proved each speedup alone, so per-task library runs only repeated that work. There is one correctness check, on the final joined code, and one timing run at the end.

Second, that one check is one run. A fresh-database `hydra_batch` run, compared table by table with `tools/compare_db.py` against the baseline's fresh database, goes through the scan, the parse, the dynamics count, the engine and the writer. So it covers every stored table. Extra engine and parse hashes, other-difficulty parses, rescans and real-database re-analyses only re-check the same output. The user called them "completely unnecessary".

Both rules are in memory (`one-library-check-at-the-join.md`) and in `docs/superpowers/plans/tasks/_perf-preamble.md`. Where the plan's wave 2 section asks for more than that, this handoff overrides it.

## What happened this session

H1 (the proof harness) merged as M0, 7dc350a. It adds `tools/compare_db.py`, the `--engine` and `--parse` modes in `hydra_bench`, the benchmark lock `tools/bench_run.ps1`, and a test that pins the corpus digests.

Wave 1 merged as M1, 92bd322: W1 (the batch writer), G1 (the score graph and one owner for a chord's note order), P1 (the lean chart and MIDI readers), S1 (the parallel scan) and B1 (parallel builds, link-time optimization, Release symbols that never ship, two SQLite options). Each task had its own derive-once review. W1, G1, P1 and S1 each needed one fix round, B1 none, and none of the fixes changed a result. S1's reviewer found a real race over which root folder a chart is credited to; it is fixed and pinned by a test. The join's review found one leftover copy of the lane count and it was fixed. The full suite passed on the join: hydra_tests 1,172 of 1,172 cases, hydra_uitest 65 of 65. The joined build's fresh database matches the baseline in every table, 0 rows differ (results 18,811, paths 90,674, charts 19,436). No stamp in `stored_versions.h` moved.

What each task measured on the user's library, against the same baseline:

| Step | Before | After |
|---|---|---|
| Re-analysing the real database (`--redo`) | 24.1 s | 16.4 s |
| Full scan | 4.22 s | 1.99 s |
| Rescan | 0.84 s | 0.23 s |
| Parse, single thread | 17.3 s | 10.4 s |
| Graph build, single thread | 13.6 s | 2.7 s |

The plan's question 1 is answered without the user. The batch-end `wal_checkpoint(TRUNCATE)` took 433 ms, under the one-second line, so the progress strip needs no change. D86 in `docs/audit/2026-10-03-fix-decisions.md` does not record that number yet; add one line there saying so.

G1's first report said the segment-heap manifest line never reached the exes. Its finisher proved that wrong: the line is embedded, and a running process uses the segment heap.

The workflow was stopped by the main session during the join check, after the check's fresh-database run had finished, because that run alone was enough. The stop is why the journal shows one agent started without a result:

```
wf_62493e7d-998: started=30 result=29
  result: {"complete":true,"branch":"claude/perf-w1","worktree":"C:\\Users\\Patrick\\Downloads\\Hydra\\hydra-test\\.claude\\worktrees\\perf-w1","tip":"a89de3fd847e7c4ccff4ebd876d73bf3b71b47e8","report":"I fixed
  result: {"verdict":"CLEAN","key":"a89de3fd847e7c4ccff4ebd876d73bf3b71b47e8","review_file":"C:\\Users\\Patrick\\AppData\\Local\\Temp\\claude\\C--Users-Patrick-Downloads-Hydra-hydra-test\\e26f30dd-44f6-44ea-9d1
wf_62493e7d-998: status=killed agents=30 name=perf-speedups-wave01
```

## What is left

### 1. One combined timing (replaces the plan's task T1)

Nobody has timed all five changes together yet; that is the one new number wave 2 owes the user. Run one before-and-after pair of each, through the lock, one at a time. First, `hydra_batch --redo` on a fresh copy of the real database, once with the old build and once with the new. Second, one full scan into a fresh database with each build. The old build is the baseline set in `C:\Users\Patrick\.claude\hooks\state\bench\baseline-c251abd\`. It was built from H1's first commit, which has bc58282's engine (its README says so), and it holds a pristine backup-API copy of the real database, `real.db`, to copy for each run. The new build is main at 92bd322. Report the two wall times, the peak memory of each `--redo` run, and the compilers-busy count, in plain sentences, in `docs/handoffs/<date>-perf-speedups-results.md`. The plan's repeated pairs, its `--engine` and `--parse` split and its extra compare are dropped (see "Read this first").

Before timing, clear the idle MSBuild helpers (follow-up A below), or each lock run waits 90 seconds first.

### 2. The mimalloc trial (D86.4)

This measures Microsoft's mimalloc allocator against the segment heap that already ships, so the user can decide with numbers whether to add it. Nothing from the trial merges. It ends in a report.

**Ask the user before any download.** Name the exact release file: its name, version, the github.com/microsoft/mimalloc releases URL and the size the page shows. Wait for a plain yes in chat. A download needs the user's own yes, every time.

Then work in a detached throwaway worktree of main, under `.claude\worktrees\perf-mimalloc`. Our build uses the dynamic C runtime (`/MD`), so the route is mimalloc's override DLL: `mimalloc-override.dll` and `mimalloc-redirect.dll` beside the exe, with the exe linked against the override's import library so the DLL loads first. The static route needs `/MT`, which we don't use, so don't try it. Build mimalloc with its own CMake (shared, override on). Confirm the redirect is active with `MIMALLOC_VERBOSE=1`, and quote that output in the report.

Measure the same way as item 1: one `hydra_batch --redo` pair on a real-database copy, with and without the override, through the lock. Report both wall times and the peak memory. One fresh-database `compare_db.py` against the baseline's `fresh.db` confirms the override changes nothing stored.

The report, `docs/handoffs/<date>-mimalloc-trial.md`, also says what shipping would take. That means a `third_party/mimalloc` source tree with its MIT licence in the third-party list, and a CMake target and `install(FILES)` so `cmake --install` stages the two DLLs. The installer would pick them up from the stage, and its "no user data leaked" and "no repo path" guards must still pass. End with "shipping is the user's call", and recommend no more than the numbers support. Remove the worktree afterwards.

Items 1 and 2 both time whole-library runs, so they share the lock and run one after the other. Item 2 can be built while item 1 times.

### 3. The user's first-scan timing (D86.6)

This one is the user's step, not an agent's. Before their next reboot, rebuild `hydra_bench` in the main checkout from main (`.\build_cpp.ps1 -Target hydra_bench`). The plan's "The user's step" section has the command and how to read the result. The exe built at the start of this session predates S1's scan change, so it must be rebuilt first. The warm full scan is now about 2 s.

## Follow-ups

**A. Idle MSBuild helpers stall the benchmark lock.** MSBuild leaves its reuse nodes running after a build: fifteen `MSBuild.exe` processes were idle tonight. `tools/bench_run.ps1` counts `MSBuild` as a busy compiler, so every lock run waited its full 90-second quiet limit. The main session cleared them by hand once (it stopped MSBuild only after checking no `cl` or `link` was running), but every build brings them back. The fix is one line. Either `build_cpp.ps1` passes `-nodeReuse:false` to the build, or `bench_run.ps1` stops counting MSBuild (a real compile always shows `cl` or `link`). Pick one and say why in the commit. It is a tooling change only, so it needs no user decision.

**B. `hydra_bench` and the fills setting.** H1's fix agent noticed this. `hydra_batch` forces Clone Hero 1.0 fills off from its own flag, but `hydra_bench --engine` and `--parse` keep whatever the ini says. So a bench run beside an ini with 1.0 fills on would print a different digest from the pinned test. No comparison this session was affected, because both sides of every comparison used the same ini. The fix is to make the bench take fills the way `hydra_batch` does, through the same owner. Grep for where `hydra_batch` sets it; don't copy the rule.

**C. Worktrees and branches.** These are left from wave 0 and wave 1, all merged into main:

- worktrees `perf-b1`, `perf-g1`, `perf-h1`, `perf-p1`, `perf-s1`, `perf-w1` and `perf-w1-writer` under `.claude\worktrees\`, plus the detached `perf-jc-base` at c8bf007 that the join check made;
- the seven `claude/perf-*` branches.

Remove the worktrees with `git worktree remove --force` and delete the branches with `git branch -d`, which refuses an unmerged branch. Keep `baseline-c251abd` until items 1 and 2 are done; it holds the old build and the clean database copy. The join check's scratch is under this session's scratchpad (`scratchpad\jc\`) and goes with it.

**D. Push.** Main is 48 commits ahead of origin/main and nothing is pushed. Pushing is the user's call.

**E. A plan row to correct.** B1's plan row says `/fp:fast`, `/fp:contract`, `/arch:AVX2` and `/GS-` appear nowhere in the repo. The vendored Opus library's own CMake uses some of them. Opus only decodes Preview audio and never touches scores or stored results. The main session read the rule as "not in Hydra's own build settings", and B1's reviewer passed it that way. If the user wants Opus's flags looked at too, that is a separate small task.

## Files

The task briefs from this session are in `docs/superpowers/plans/tasks/` (`_perf-preamble.md`, `perf-*.md`). The review files are in this session's scratchpad. The bench log, `C:\Users\Patrick\.claude\hooks\state\bench\bench_log.md`, holds every lock run with its time and compilers-busy count.

Two untracked files in `docs/handoffs/` came from other sessions and were left alone: `release-2.1.0-notes.md` and `2026-09-29-public-timing-scripts/blink/results/blink-both.json`.
