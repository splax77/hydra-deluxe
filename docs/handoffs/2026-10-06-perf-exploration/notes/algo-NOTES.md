# algo agent notes (search-algorithm ideas)

Worktree: C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\perf-algo (detached, uncommitted)
Baseline exes: baseline\hydra_batch.exe, baseline\hydra_bench.exe (no settings file next to either; defaults).
Default analysis: depth Scores 4, ms filter on. All-0 pass: Scores 0, no_skips, hard 0 ms.

## History (task 1)
- Dominance "dismissed" = archive/phase4-activation-dp-cpp.md (2026-08-14). Context was the
  backward activation DP (hy_dp_search), not the BFS. DP state space ~candidates^2 on blink; BFS avoids
  it with per-group pruning; DP had "no sound equivalent" because activation spends the whole meter,
  so more SP is not dominated by less SP. Never tested empirically in the BFS (no git commit mentions it).
- tight-bound-prune-does-not-pay.md: bound pruning (max_suffix + tight SP bound) was built, exact,
  pruned 0.0036% on blink, cost 7%. Deleted 2026-08-18 (repo-cleanup).
- ms-filter-frontier-fix.md: filtered paths now also limited by depth band.
- Group key today: implicit timestamp (level-synchronous; base and SP tracks advance together),
  track flag; base: SP meter (bars) + ready_class (count of upcoming fills refused / over-limit for
  this SP-ready time); SP: SP end tick + banked_phrase_ordinal, second word pending_sqout_at.
  Paths holding spent/banked-ahead phrases are not grouped at all.

## Experiment code (engine.cpp, marked PERF EXPERIMENT)
Env HYDRA_XDOM=2: result uses cross-group dominance. HYDRA_EXP_LOG=<file>: per search, runs shadow passes
(xdom, oracle bound, live bound, depth0) and compares their output byte-for-byte with the plain run.
HYDRA_EXP_DUMP=1: lists paths + drops for differing passes.

## Results
- Library A (baseline) vs X (HYDRA_XDOM=2), fresh dbs, under lock: 18,811 results compared, 58 differ.
  Wall 16.6 s vs 18.3 s (one run each, 0 compilers).
- Smallest differing: Clairo - Juna (887 notes). Main list identical; the all-0 section changes
  273,000 -> 271,780. Drop at tick 47040: B (score 142,620, 0 bars) dropped by A (143,040, 1 bar).
  All-0 forces activation at first chance, so A's extra bar forces earlier activations (50112 E0, 65472,
  80832 with 2 bars) while B ends with a 3-bar final activation at 80832.
- blink smoke: main search 84 ms; xdom same output, frontier 571,885 -> 555,908 (-2.8%);
  oracle bound drops 36 paths; depth0 frontier 115,977 (5x smaller). nodes 15,841 vs notes 252,936.

## Library shadow run (lib_exp.log, 46.9 s wall under lock, 0 compilers)
Instrumented build with switches off == baseline: 18,811 results compared, 0 differ.
main (18,811 searches): plain frontier sum 31.6M, engine search 5.6 s CPU total.
- xdom: differs on 13 main + 45 all-0 searches (= the 58 db diffs). On identical ones frontier -6.1%.
  3 main searches lose the best score: BTBAM Informal Gluttony x2 (932,440 -> 932,280), Colors (8,791,335 -> 8,791,175).
  Smallest main: 1,202 notes (#5 near-best path replaced).
- oracle bound (floor = lowest score in final list): exact on all, frontier -8.75%.
- live bound (floor from never-activate-again finishes): exact on all, frontier -6.76%.
- depth0: frontier -73% (8.4M vs 31.4M), pass time 1.08 s vs 5.6 s.
- graph: notes 45.3M, nodes 2.48M (1.24M timestamps), fills 623k, sp-gain edges 186k, deact nodes 849k.
  43.7% of path-steps sit on nodes with no branch (no decision).
Search per chart: ordinary (<5k notes) 0.23 ms main + 0.03 ms all-0; giants (>50k) 39 ms main.

## Grounding (task 4)
- Baseline library run, fresh db, under lock, 0 compilers, 2 runs: wall 19.8 / 22.5 s, process CPU 81.5 / 73.4 s.
  Engine search (main + all-0) summed over the library: ~6.3 s CPU => roughly 8% of analysis CPU.
- hydra_bench baseline, 2 runs each, identical: blink 0.18 s (parse 0.04, graph+search 0.14), Rise Against 0.14-0.16,
  Hail The Sun 0.11, Endless Setlist I .sng 0.10-0.11. Ordinary charts (BrianTD9192\Blink-182, 8 charts): all 0.00 s
  at 10 ms resolution except one 0.02 s. Engine BFS alone on blink: 84 ms main + 4 ms all-0.
- Giants (12 charts > 50k notes) are 0.49 s of the 6.3 s search total.
Patch: algo.patch. Verdicts: xdom unsound (do not ship); bounds exact but ~7-9% frontier of an ~8% share; lazy k-best not worth a rewrite.
