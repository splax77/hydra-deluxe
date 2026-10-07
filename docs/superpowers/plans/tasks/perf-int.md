Read docs/superpowers/plans/tasks/_perf-preamble.md first; it holds the rules. Then follow `docs/agents/integrate.md`; it is your procedure.

# Task INT: join wave 1

Task id: INT. Your prompt gives: H1's signed-off tip (the wave branch starts there), the fork point (H1's tip that wave 1 forked from; it may equal the signed-off tip), and the five task branches with their signed-off tips.

Wave branch: `claude/perf-w1`, worktree `.claude\worktrees\perf-w1`. Make it with `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test worktree add C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\perf-w1 -b claude/perf-w1 <H1 signed-off tip>`.

## Join order

`claude/perf-b1`, `claude/perf-g1`, `claude/perf-p1`, `claude/perf-w1-writer`, `claude/perf-s1`, then `main` last. B1 goes first because it alone owns `CMakeLists.txt`. W1 and S1 both edit `src/app/analysis.cpp`, in disjoint functions (S1 above line 593, W1 from 593 down); git should merge them cleanly, and if it does not, keep both sides' functions whole. `tests/test_analysis.cpp` and `tests/test_store.cpp` take appended cases from W1, S1 and B1, and `tests/test_single_owner.cpp` takes appended rows from W1, G1, P1 and S1: rebuild each as integrate.md step 3 says. Merging `main` last brings in M0 (H1's merge) if the main session has made it, so the review range `main...<head>` holds only wave 1.

## What this brief adds to integrate.md

Step 1's fork is the fork point your prompt gives. Step 4's build: `pwsh -NoProfile -File <worktree>\tools\build_slot.ps1 -Repo <worktree> -Target hydra_tests`, then `.\build_cpp.ps1 -Target hydra_bench` and `-Target hydra_batch`. Step 4's named tests, besides the scan and docs tests: `build-cpp\Release\hydra_tests.exe -sf=*test_perf_digest*` (the pinned corpus digests must still pass with all five changes together; if they fail, stop and report, because that means two byte-identical changes are not identical together), and `-sf=*test_analysis.cpp*,*test_store.cpp*` (the three tasks' appended cases side by side).

Then the grep the plan names for after wave 1: `grep -rn "HYDRA_PERF\|HYDRA_SCAN_\|HYDRA_TXN\|HYDRA_STMT\|GetEnvironmentVariable\|_dupenv_s" src` must find nothing. Put its output in the report.

## Owned files

Only the merge commits on `claude/perf-w1` and the join fixes integrate.md allows.

Owned-file check: the join changes no file beyond what the five task branches and `main` already change, plus join fixes integrate.md names.

## Preflight

Command: `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test worktree list`, run by the orchestrator on 2026-10-06 before dispatch.
Output: only the main checkout at 7403685 [main]; no `perf-*` worktree existed yet, so the wave branch name `claude/perf-w1` was free.

## Return

`complete`, `branch`, `worktree`, `tip` (full hash of the head after step 4), `precheck` (its full output), `report`, `questions`, `handoff`.
