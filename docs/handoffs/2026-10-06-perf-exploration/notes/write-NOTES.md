# write agent notes (batch pipeline around the engine)

Worktree: C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\perf-write (detached, uncommitted edits).
Baseline exe: baseline\hydra_batch.exe; experimental: exp\hydra_batch.exe. Both folders hold the GUI's hydra_settings.ini (Expert, pro, 2x, depth 4, ms 10, cap 4, folder C:\Clone Hero).
Runner: run_one.ps1 (fresh copy of pristine\ real db each run, --redo, polls peak WS / peak commit). Compare: compare.py A.db B.db.

Switches in the experimental exe (env vars):
- HYDRA_STMT_CACHE=1: save path reuses compiled statements (record_store.cpp use_stmt), BEGIN/COMMIT/SAVEPOINT too.
- HYDRA_TXN_GROUP=N: consumer opens one transaction and saves up to N charts in it, each in SAVEPOINT chart; commits when N reached or the line is empty (work_pool idle hook). Store lock held for the group. Progress callbacks for a group go out after its commit; a failed commit fails every chart in the group.
- HYDRA_WORKERS=N: worker count (batch.cpp).
- HYDRA_ORDER=size | file:<csv md5,cost>: longest-first ordering of plan.todo (permutation only; item and rows unchanged).
- HYDRA_PERF_REPORT=1 (+ HYDRA_PERF_DUMP=csv): stderr PERF line with consumer/save/worker times.

## Measurements

19:54 write-first (compilers 0): A1 wall 16.9 s (analysis 15.4 s), peak WS 514 MB, commit 639 MB.
B0 (all off, instrumented) wall 17.6 s. PERF: workers=8, worker busy 50.0 s total => util 0.387.
Consumer busy 15.2 s of 16.1 s run_batch wall; save_analysis alone 12.8 s = 0.68 ms/chart.
=> The single consumer thread is the bottleneck, not the engine. Median chart analysis 1.9 ms.
Giants: Endless Nameless notes.chart 0.38 s, Endless Setlist .sng 0.35 s, blink-182 0.33 s, Rise Against 0.26 s.
The .sng giants start at 15.8 s (discovered last), so they form a ~0.3 s tail.

19:58 write-cache-group (compilers 0), 3 reps, analysis_s medians: A 15.6 (17.9/15.6/15.0); cache 13.9 (15.5/13.9/12.8); cache+G16 12.6 (14.9/12.6/12.2); cache+G64 14.2 (19.2/14.2/11.6). Noisy; runs drift faster over the session.
Groups commit far more often than N: the line is often empty (G16: ~1400 commits = ~13 charts/commit; G64: 500-800 commits).

20:04 write-breakdown (compilers 0), one run each, analysis_s: A 17.5; off 15.7; cache 13.1; cache+ckpt100000 15.0 (wall 18.6: 3.6 s final checkpoint at close); cache+sync OFF 10.2; cache+G16+ckpt100000 9.0; cache+64MB page cache 13.8.
Save-step totals (ms), off: tempomap 142 begin 48 upsert 1473 length 134 purge1 941 purge2 1243 insert 261 nodes 548 gc 302 dynamics 374 COMMIT 6949.
cache: upsert 719 purge1 535 purge2 863 insert 107 gc 169 commit 6525 => the cache saves ~2.4 s of statement compiling; commit dominates.
cache+G16+ckpt: savepoint release 10 ms, group commits 1944 ms over 1616 commits. Consumer busy 8.2 of 9.0 s; worker util 0.70.
Reading: per-chart COMMIT writes every touched page (index interior pages, roots) to the WAL each time; a group writes each shared page once. The default autocheckpoint (1000 pages) adds checkpoint copies + fsyncs on top.

20:12 write-group-ckpt (compilers 0), 3 reps, median wall / analysis s: A 16.3 / 15.0; cache+G16+ckpt10000 11.5 / 10.1; cache+G64+ckpt10000 11.7 / 10.3; same + HYDRA_LEAN 11.5 / 9.9.
G64 = G16 because groups rarely fill (1100-1700 commits either way). LEAN (tempo map + Song freed on the worker) gains nothing measurable.
Consumer still busy ~9.3 of ~10 s: save 4.1 s + group commits 3.3 s + ~1.6 s other (printing, bookkeeping).

20:15 write-matrix (compilers 0), 3 reps, median wall / analysis s: A 20.0 / 18.3 (20.0, 15.4, 22.9 wall: noisy); cache 14.1 / 12.6; cache+G16 13.4 / 12.1; cache+G16+ckpt10000 12.1 / 10.7.
Pooled walls over all sessions: A median 17.1 (n=11); cache 14.6 (n=7); cache+G16 13.65 (n=6); cache+G16+ckpt10000 11.6 (n=6).
Correctness (real-db copies after --redo, A_1 vs CG16k10_1): results 19,106, path_refs 91,196, paths 91,196, songmeta 19,099, dynamics 18,820, charts 19,436: 0 differ. IDENTICAL.

20:19 write-workers (compilers 0), cache+G16+ckpt10000, 3 reps, median wall / analysis s, peak WS / peak commit MB (max of 3):
A 17.8 / 15.7, 496 / 625; W8 12.6 / 11.1, 513 / 640; W12 13.9 / 12.3, 538 / 660; W15 18.1 / 15.5, 562 / 685.
More workers are slower: the one consumer thread is still the bottleneck, and extra workers take CPU from it (8 cores / 16 threads). Worker util drops 0.59 -> 0.37 -> 0.24.

20:23 write-order (compilers 0), all with cache+G16+ckpt10000, 3 reps, median wall / analysis s, peak commit MB range:
A 16.3 / 14.9, 634-648; unordered 11.7 / 10.4, 629-650; size-first 12.2 / 10.8, 661-737; previous-run-time-first (oracle from dump_B0; the db stores no analysis time) 12.6 / 11.2, 802-826.
Spearman(file size, analysis time) = 0.54; 12 of the 50 slowest charts are among the 50 largest files (archives carry audio).
Longest-first removes the ~0.2-0.3 s end tail but the consumer, not the tail, sets the pace; giants run together, so peak memory rises.

20:26 write-fresh-correctness (compilers 0): fresh dbs, baseline vs cache+G16+ckpt10000+size-order: results 18,811, path_refs 90,674, paths 90,674, songmeta 18,811, dynamics 18,811, charts 19,436: 0 differ. IDENTICAL.
(Fresh-db walls include an uncached scan: A 20.4 s, B 15.9 s; analysis 15.6 vs 11.3 s.)

Cancel: not tested (hydra_batch has no cancel). By construction the group never stays open across a wait (it commits whenever the line is empty or holds 16), each consume checks cancel before writing, so after a cancel at most the charts already saved in the open group (<=15, ~0.4 ms each) are committed, then nothing more is written.
Patch: write.patch. Kept run dbs remain under runs\ (CG16k10_1, GOS_1, FA, FB, B0); pristine\ is the real-db copy.
