# prof agent notes (key `prof`)

Worktree: C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\perf-prof (detached, uncommitted edits)
Baseline exes: baseline\ ; instrumented copy: inst\ (both with the installed hydra_settings.ini next to them,
so both scan C:\Clone Hero and save the library, depth scores 4, SP cap 4, 10 ms cap).
Real db copy: realdb\ (db+wal+shm). Runs, logs, CSVs: runs\.

Instrumentation: src/core/phase_prof.{h,cpp} (QPC, global relaxed atomics, per-thread per-chart array),
timers in work_pool.h (queue wait / consumer idle), analysis.cpp (scan + run_batch), pather.cpp (graph,
main vs all-0 ctx), engine.cpp run_search (enumerate / engine.run / rebuild), record_store.cpp
(save_analysis statements), batch.cpp (top level + report()). CSV via env HYDRA_PROF_CSV.
SaveUpsertSong INCLUDES SaveNaming (nested). WorkerTotal includes everything on the worker for that chart.

## 19:47 first fresh-db run (lock, 0 compilers)
A 17.1 s, B 16.8 s wall. Found 19,436, analyzed 19,378 rows (18,811 distinct md5), 58 failed.
Scan 4.2 s (walk 0.9 serial, read stage 3.3; worker sums MD5 10.7 s, read 1.9, open 0.8, ini 1.8).
BatchRun 12.2 s wall: workers busy 45.1 s total, blocked on full queue 50.9 s; consumer SaveTotal 11.3 s
(COMMIT 7.7 s). => run is bound by the single consumer's saves, not by analysis.
Worker split: Parse 19.1, GraphBuild 15.4, Dynamics 4.2, MainEngine 3.0, enumerates 1.3, rebuilds 0.4, prepare_row 0.6.
Correctness fresh: 18,811 charts compared, 0 differ (all tables identical, charts in order).

## 19:49 real-db --redo (lock, 0 compilers)
A 15.5 s, B 15.2 s wall. Scan cached: 0.95 s (walk 0.9). BatchRun 13.6 s; workers 52.4 s busy, 54.9 s queue wait;
SaveTotal 12.5 s (COMMIT 7.2). Correctness: 19,099 charts compared, 0 differ.

## 19:52 overhead A/B fresh, A,B x3 interleaved (lock, 0 compilers): A median 16.81 s, B median 16.81 s.

## CSV (B_real.csv, csv_analysis.py, sched_sim.py)
p50 1.97 ms/chart, p99 16.5, max 402 ms. 20 slowest = 5.7% of worker time; top 100 = 11.1%; top 500 = 20.7%.
.mid 66% of worker time, .chart 30% (parse-heavy: 2.3 ms mean parse vs 0.8 for .mid), archives 4%.
Idle tail 0.16 s (writer-bound run throttles workers so the tail barely shows).
No-writer-limit sim at 8 workers: scan order 6.75 s vs longest-first 6.55 s (= lower bound); raw file size
order already reaches 6.55. 16 workers: 3.57 vs 3.27. => start-biggest-first worth ~0.2-0.3 s.
Size predictor: per kind good (.chart spearman 0.93, .mid 0.60, archives 0.79), mixed 0.56.

## 19:55 cold-ish hash pass (hashcold.py, lock, 0 compilers), 7.83 GB, 19,436 files, md5 all match
rounds 1-2: buf8 3.0/3.2 s, nobuf8 4.1/3.9 s, nobuf16 8.3/7.9 s, buf16 3.6/3.4, nobuf-nohash8 2.9/3.1,
nobuf-nohash16 7.0/8.0. Round 3 everything ~2x slower (machine/SSD noise). 16 threads slower than 8 with
NO_BUFFERING, probably the Python GIL with ctypes calls; not trusted as a disk number.
Disk: Crucial CT2000E100SSD8, NVMe SSD. CPU 9800X3D 8C/16T.

## Profiler
wpr -start CPU: "Failed to enable the policy to profile system performance" (needs admin). xperf same family.
VSDiagnostics.exe present (VS 18 Community, Team Tools\DiagnosticsHub\Collector). /loadConfig crashes
(Newtonsoft.Json 13.0.0.0 manifest mismatch). Retrying with /loadAgent CLSID. Symbols build: CMakeLists
in worktree adds /Zi + /DEBUG /OPT:REF /OPT:ICF (perf-only).

## 20:15 CPU profiles (VSDiagnostics /loadAgent:4EA90761-...;DiagnosticsHub.CpuAgent.dll, 1 kHz; xperf -a profile -detail; local pdb only, no symbol server)
blink x20 copies (4.1 s CPU): ntdll 38.8% + ntoskrnl 13.6% + vcruntime 5.5% unsymbolized (likely heap alloc/free, page faults, memcpy); top own functions each ~2%: parse_track, reduce_iteration_paths, advance, reduce_group, optype, Timecode ctor, Engine::run, StampMap::get_or_insert, enumerate.
es3 (1.7 s CPU): bcryptprimitives 83.7% = hydra_bench's own scan MD5 of the 1 GB .sng; analysis is a sliver.
Inclusive: xperf butterfly on these traces only resolves ntdll frames (98.6%), so no usable inclusive table. vsdiag\*.diagsession open in VS for that.
Patch: prof.patch (includes CMake /Zi change).
