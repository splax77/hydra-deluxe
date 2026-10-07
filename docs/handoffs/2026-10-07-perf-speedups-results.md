# Speedups: the combined timing (2026-10-07)

This is the one timing wave 2 owed: all five wave 1 changes (the batch writer, the score graph, the lean readers, the parallel scan and the build settings) measured together against the old build. It replaces the plan's task T1, as the wave 2 handoff said.

## The answer

Re-analysing your whole library on a copy of the real database now takes 9.7 s, down from 14.2 s. A full scan into a fresh database now takes 2.2 s, down from 4.4 s. The new build also uses less memory for the re-analysis: 412 MB at its peak, against 489 MB.

| Run | Old build | New build |
|---|---|---|
| `hydra_batch --redo` on a fresh copy of the real database | 14.2 s, peak 489 MB | 9.7 s, peak 412 MB |
| Full scan into a fresh database (`hydra_bench --scan`) | 4.4 s, peak 61 MB | 2.2 s, peak 75 MB |

Every run started with 0 compiler processes running, which the benchmark lock logged each time.

## How it was measured

Each run went through `tools/bench_run.ps1`, one at a time, on a quiet machine. The old build is the baseline set in `~\.claude\hooks\state\bench\baseline-c251abd\` (bc58282's engine). The new build is main at c2fdb43, whose code is the same as M1 (92bd322); its exes are staged in `~\.claude\hooks\state\bench\new-c2fdb43\`. Both used the same settings file. Each `--redo` run worked on its own fresh copy of the baseline's `real.db`. Peak memory is the process's own peak working set, read by the timing script every 100 ms.

One thing went wrong and was redone. The first pass ran the old build first in each pair, and it read the song files from disk while the new build found them already in memory. That made the old build look far worse than it is: 28.8 s for the re-analysis and 14.5 s for the scan (13.5 s of it reading and hashing files). Both old-build runs were done again straight after, with the files already in memory, and the table uses those. So every number in the table is a warm run.

The cold numbers are still worth knowing. A first run after the files fall out of memory spends most of its time reading the disk, not working. That is what your own first-scan timing after a reboot (D86.6) will measure.

## Against the earlier per-task numbers

Wave 1's own measurement of the batch writer alone was 24.1 s to 16.4 s. Those runs shared the machine with other agents' builds (15 to 51 compiler processes running at the start). Today's runs had none, which most likely explains why both of today's numbers are lower; that was not tested. So compare proportions, not seconds. The writer alone saved 32%, and all five changes together save 32% of the re-analysis (14.2 s to 9.7 s). So in a whole-library re-analysis, the graph and parse gains add little on top of the writer, even though each was large in its own single-thread timing. Why was not measured here. If it matters, the next step is a `--redo` run with the per-stage timings, not more pairs.

## Stored results

This run did not re-check stored results; the join check already did that (0 rows differ in every table). The mimalloc trial's fresh database, built from the same code with a different allocator, also matched the baseline in every table today. See [2026-10-07-mimalloc-trial.md](2026-10-07-mimalloc-trial.md).
