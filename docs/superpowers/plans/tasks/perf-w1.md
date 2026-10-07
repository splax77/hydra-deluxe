Read docs/superpowers/plans/tasks/_perf-preamble.md first; it holds the rules.

# Task W1: the batch writer

Task id: W1. Base: H1's tip, which your prompt gives. Branch: `claude/perf-w1-writer`, worktree `.claude\worktrees\perf-w1-writer`; make it as the preamble says, but with that name (the plain `perf-w1` name is kept for the wave's join branch).

Your spec is the plan's "Task W1" section, word for word. This brief adds only what the plan leaves to dispatch time.

## What this brief adds

The plan's line range for `run_batch` was off. On the base, `run_batch` starts at line 668 of `src/app/analysis.cpp` and its anonymous-namespace helpers sit at lines 593 to 622. You own `run_batch` and those helpers. S1 owns `stream_md5` and `discover_charts` in the same file in parallel, so touch nothing above line 593 and add no include.

Correctness compares against the baseline's saved `fresh.db`; you only run your own side. For the `--redo` check, run both baseline and yours on fresh copies of the baseline folder's `real.db` and compare those two. Timing is three interleaved pairs, as the plan says.

**The checkpoint question.** Time the batch-end `wal_checkpoint(TRUNCATE)` on a whole-library `--redo` run of a `real.db` copy, and put the number in `checkpoint_ms`. If it is over 1,000 ms, finish the code exactly as the plan says (the checkpoint runs where the guard ends) and change no display. Put the plan's question 1 in `questions` with the number. The user picks later; this does not hold the merge.

## Owned files

The plan's W1 list: `src/store/record_store.h`, `src/store/record_store.cpp`, `src/app/analysis.cpp` (`run_batch` and its helpers only), `src/app/work_pool.h`, `tests/test_store.cpp`, `tests/test_analysis.cpp`, `tests/test_single_owner.cpp` (your own rows at the end only), and one new script in `tests/ui/uitest_batch_reports.cpp` only if the plan's last-but-one criterion asks for it.

Owned-file check: every W1 acceptance criterion is met inside the files above; the timing scripts and databases live in your scratchpad.

## Preflight

Command: `grep -n "run_batch(\|^namespace {\|^}  // namespace" src/app/analysis.cpp` and `grep -n "synchronous" src/store/record_store.cpp`, run by the orchestrator on the base's code on 2026-10-06.
Output: `run_batch(` at 668; the anonymous namespace at 593 to 622; `PRAGMA synchronous=NORMAL` at record_store.cpp line 726. `git apply --check` of `patches/write.patch` passes cleanly on this code.

## Return

`complete`, `branch`, `worktree`, `tip`, `checkpoint_ms`, `report`, `questions`, `handoff`.
