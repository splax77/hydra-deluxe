# engine perf agent notes (agent a09ad612bccf6a1a6)

Worktree: C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\perf-engine (detached, uncommitted)
Scratch: this folder. baseline\ holds unmodified hydra_batch.exe, hydra_bench.exe, plus
hydra_bench_h.exe = baseline engine + my harness mode (tools/bench.cpp `--engine <folder> [--cache db] [--reps N] [--out file]`).
Both baseline\ and build-cpp\Release\ have the same hydra_settings.ini (installed one minus dm_last_user).

Harness: single thread, per chart: parse, a throwaway ScoreGraph build, analyze_chart, prepare_row;
prints sums + FNV hash over every PreparedRow (structure, nodes, bestpath, summary). Same hash = same stored bytes
(stronger check: whole-library hydra_batch dbs compared by py script).

## Baseline
- hydra_batch whole library fresh db: Found 19,436, analyzed 19,378, 58 failed, 15.8 s analysis (43.6 s wall incl scan). 0 compilers.
- giants (blink, rise, ES1) harness x5: graph 0.23 s per build of all 3, analyze x5 2.675 s.
- whole library single-thread harness (18,811 unique md5, 58 failed): parse 16.6 s | graph 14.3 s | analyze 18.8 s | prepare 0.56 s. hash 8d17f958172bd6f4. 0 compilers.
  => graph build is ~75% of analyze_chart on the library. Search+allzero ~4.5 s.
  => 8 workers only give ~2.3x over single thread sum (36 s -> 15.8 s): something serializes (writer? allocator?).

## Experiments (each in exp\<name>\ with its exes, ini and patch; combo patch = engine.patch)
- 6a set_head_time filters in place: giants analyze x5 median 1.286 vs base 1.627 (3 runs, 0 compilers). hash same.
- 6b category_scores heap-free note order: giants 1.505. hash same.
- 6c backend rows as ranges into one per-graph array (no per-edge copies; engine reads range; rebuild materializes): giants 1.333. hash same.
  PERF_VERIFY_6C build (old copies kept and compared to the ranges at every graph build) ran over whole library: no mismatch, 58 failed as base, hash same.
- seg segment heap manifest: giants 1.431. hash same.
- combo = 6b + 6c + seg (6c supersedes 6a): giants 1.085 (runs 0.879, 1.085, 2.083).
  Library single-thread harness: analyze 17.6 s -> 6.56 s, graph 13.6 -> 2.74 s, parse 15.1 -> 13.4 s. Hash 8d17f958172bd6f4 identical over 18,811 charts.
- hydra_batch --redo on real db copy, 3 runs interleaved, 0 compilers: base 12.6 s, combo 12.9 s (runs 12.5, 12.9, 22.9 outlier).
  => batch is bound by something else (single writer thread save_analysis, most likely): engine CPU is ~1/8 of wall now.
- Not tried (call budget): PGO, ideas 4 (rest), 5, 7. Probe bounds on library single-thread: rebuild 0.35 s, allzero pass 0.77 s total, enumerate ~1 s (x2 runs), BFS ~2.8 s.
- clang-cl not installed. mimalloc untested (download).
- Correctness, whole library hydra_batch fresh dbs (db_base.db vs db_combo.db, cmpdb.py): 18,811 charts in each; results, paths, path_refs, songmeta, dynamics, meta all identical in content.
  Only results.result_id / path_refs.result_id differ (12,573 charts): the row id the writer hands out in worker-completion order, which also varies run to run. combo batch run: 12.2 s analysis.
