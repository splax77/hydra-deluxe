Read docs/superpowers/plans/tasks/_perf-preamble.md first; it holds the rules.

# Task G1: the score graph, the chord's note order, and the dynamics count

Task id: G1. Base: H1's tip, which your prompt gives. Branch: `claude/perf-g1`, worktree `.claude\worktrees\perf-g1`; make it as the preamble says.

Your spec is the plan's "Task G1" section, word for word. This brief adds only what the plan leaves to dispatch time.

## What this brief adds

The plan says `Chord::notes(true)` sorts at model.cpp line 183. On the base the `std::stable_sort` is at line 188; it is the same sort.

Correctness compares against the baseline's saved `engine_rows.txt` and its hash, and `fresh.db`; you only run your own side. The graph-time criterion needs the same-session baseline, so run the baseline's `hydra_bench --engine` and yours as one interleaved pair through the lock and quote both graph times.

The segment-heap manifest line changes how the process allocates, not what it computes. Say in the report whether the `--engine` hash and `fresh.db` compare were taken with the manifest in place (they must be).

## Decided by the main session (2026-10-06, 22:35): the segment-heap line

The implementer found that the manifest's segment-heap line never reaches the linked exes. The graph speedup measured 13.6 s to 2.7 s without it, so it is not part of what G1 proves. A finisher spends at most about 20 tool calls on it, inside `src/app/long_paths.manifest` only. If the fix needs `CMakeLists.txt` (B1's file) or a new build step, restore `long_paths.manifest` to its base content in one commit, say why in the commit body and the report, and return `complete: true`. The main session makes the segment heap a follow-up after the join. Whole-library runs are already done for G1 (0 differ, same engine hash); do not repeat them.

## Owned files

The plan's G1 list: `src/search/graph.h`, `src/search/graph.cpp`, `src/search/engine.cpp`, `src/core/model.h`, `src/core/model.cpp`, `src/core/scoring.cpp`, `src/app/dynamics_breakdown.cpp`, `src/app/long_paths.manifest`, `tests/test_model.cpp`, `tests/test_search.cpp`, `tests/test_dynamics_breakdown.cpp`, `tests/test_single_owner.cpp` (your own rows at the end only).

Owned-file check: every G1 acceptance criterion is met inside the files above.

## Preflight

Command: `grep -n "stable_sort" src/core/model.cpp`, `grep -n "category_scores" src/core/scoring.cpp` and `grep -n "count_dynamics" src/app/dynamics_breakdown.cpp`, run by the orchestrator on the base's code on 2026-10-06.
Output: `std::stable_sort` at model.cpp 188; `category_scores` defined at scoring.cpp 26; `count_dynamics` defined at dynamics_breakdown.cpp 111. `git apply --check` of `patches/engine.patch` and `patches/parse.patch` passes cleanly on this code.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
