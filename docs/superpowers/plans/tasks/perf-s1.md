Read docs/superpowers/plans/tasks/_perf-preamble.md first; it holds the rules.

# Task S1: the library scan

Task id: S1. Base: H1's tip, which your prompt gives. Branch: `claude/perf-s1`, worktree `.claude\worktrees\perf-s1`; make it as the preamble says.

Your spec is the plan's "Task S1" section, word for word. This brief adds only what the plan leaves to dispatch time.

## What this brief adds

On the base, `stream_md5` is at line 89 of `src/app/analysis.cpp` and `discover_charts` at lines 398 and 559 (the second is the overload that forwards). You own those, `PendingChart` and `pending_chart_of`. W1 owns `run_batch` and the anonymous namespace at lines 593 to 622 of the same file, in parallel, so touch nothing from line 593 down. Any include you need goes at the end of the include block.

Correctness compares against the baseline's saved `scan_first.db` (fresh) and `scan_rescan.db` (the rescan): make your own the same way, a fresh scan then a second scan on the same database, and `compare_db.py` each against the baseline's. Timing is five interleaved pairs for full scans and five for rescans, as the plan says.

## Owned files

The plan's S1 list: `src/app/analysis.cpp` (`stream_md5`, `discover_charts`, `PendingChart`, `pending_chart_of` and new helpers for them, above line 593 only), `tests/test_analysis.cpp` (appended cases), `tests/test_single_owner.cpp` (your own rows at the end only).

Owned-file check: every S1 acceptance criterion is met inside the files above.

## Added by the main session (2026-10-06, after the review)

The review's kind D finding is the comment at `src/app/analysis.h` lines 77 to 78, which still calls the walk serial. The fix round may change that comment, and only that comment, in `analysis.h`; nothing else in that file is S1's.

## Preflight

Command: `grep -n "stream_md5(\|discover_charts(" src/app/analysis.cpp` and `grep -n "batch_worker_count" src/app/analysis.h`, run by the orchestrator on the base's code on 2026-10-06.
Output: `stream_md5` defined at 89 and called at 333, 508 and 514; `discover_charts` at 398 and 559; `int batch_worker_count();` declared at analysis.h 196. `git apply --check` of `patches/scan.patch` passes cleanly on this code.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
