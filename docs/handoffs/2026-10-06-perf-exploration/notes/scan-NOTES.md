# perf-scan notes (agent aa1f0a157a9a76668, key scan)

Worktree: C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\perf-scan (detached at bc58282, uncommitted edits).
Baseline exes: baseline\ ; experimental exes: exp\ (copied after build). No settings files next to either.
Machine: Ryzen 7 9800X3D (16 logical), NVMe SSD. Library: 23,160 folders, 19,436 charts.
All numbers are WARM OS file cache (cannot flush without admin).

Switches (env vars, read by the exp build):
- HYDRA_SCAN_FINDEX=1   FindFirstFileExW(FindExInfoBasic, LARGE_FETCH) in list_dir (core/winstr.cpp)
- HYDRA_SCAN_PWALK=N    parallel walk on N threads, then replay of the serial stack order (analysis.cpp parallel_walk)
- HYDRA_SCAN_HASHIO=1   CreateFileW+ReadFile SEQUENTIAL_SCAN, per-thread reused buffer, HYDRA_SCAN_HASHBUF_KB (default 1024)
- HYDRA_SCAN_REUSEBUF=1 stock fread path but a per-thread reused 1 MB buffer (stock allocates+zero-fills 1 MB per file)
- HYDRA_SCAN_WORKERS=N  read/hash pool size (stock: min(hw-1, 8) = 8)
- HYDRA_SCAN_LEAFSIM=1  rescan estimate: folders holding cached charts are not listed (treated as leaves)

Scripts: ab_lib.ps1 (runner), t_*.ps1 (timing), correct1.ps1 + compare_charts.py (charts table by rowid, raw bytes).

## Correctness 1 (19:44)
Fresh dbs, no cache. A=baseline; B=findex+pwalk8+hashio+workers16; C=reusebuf; D=exp all off.
19,436 rows each; B, C, D differ from A in 0 rows (rowid order, all columns).

## Walk timing (19:45, 5 reps interleaved, compilers busy 0), medians, no cache
baseline enum 0.87 s, total 4.41 s
findex   enum 0.85 s (no gain)
pwalk4   enum 0.29 s; pwalk8 0.19 s; pwalk16 0.16 s
findex+pwalk8 enum 0.19 s
=> parallel walk saves ~0.7 s; FindFirstFileEx basic/large-fetch saves nothing measurable.

## Hash timing (19:50, 5 reps, compilers 0), medians, no cache
baseline read 3.55 s | reusebuf 2.68 | hashio 1MB 2.63 | hashio 256KB 2.66 | workers12 3.16 | workers16 3.19 | hashio+w16 2.29
=> the win is from NOT allocating+zero-filling a fresh 1 MB vector per file (19,436 x 1 MB), not from CreateFile/SEQUENTIAL_SCAN.

## Size profile (sizes.py on A.db)
7.47 GB of chart files in total; 124 files over 1 MB hold 5.56 GB; median 87 KB.
The three Endless Setlist .sng (931 MB, 1009 MB, 1044 MB) are rowids 19386-19388, i.e. near the END of walk order,
so they start last and hash alone: the read stage's tail is single-thread MD5 of a 1 GB file.
New switch HYDRA_SCAN_BIGFIRST=1: work order sorted by listed size, descending (results still stored by walk index).

## Combo timing (20:01, 5 reps, compilers 0), medians, no cache
baseline total 4.22 (enum 0.81, read 3.40)
reusebuf 3.58 | bigfirst 3.57 | reusebuf+bigfirst 2.76 (read 1.88) | +w16 2.93
pwalk8+reusebuf+bigfirst 2.06 (enum 0.19, read 1.89)  <- best, about half of baseline
pwalk8+hashio+bigfirst+w16 2.39

## Rescan timing, cache full (20:05, 5 reps, compilers 0), medians
baseline enum 0.77, read 0.03, total 0.80 (wall 0.99 incl. db open + library write)
findex 0.78 | pwalk8 0.19 | pwalk8+findex 0.19 | leafsim (serial, estimate) 0.22 but found 19,319 charts not 19,436
=> on rescan the walk is ~96% of discover time. pwalk8 cuts it 4x. Leaf skipping adds nothing on top and lost
117 charts that live in song folders nested inside other song folders (row diffs vs A: 5,307 rows shifted/missing).
Rescan dbs R_A, R_P, R_F, R_C: 19,436 rows, 0 differing from A.

## Correctness 2 (20:12)
E=pwalk8+reusebuf+bigfirst (total 2.03 s), F=pwalk8+hashio+bigfirst+w16, G=findex+pwalk16+w12: 19,436 rows each, 0 differing from A
(rowid order, all columns). The library has 542 md5s with more than one copy, so D63's first-copy order is exercised.

## Leaf-skip safety (idea 4)
Not safe on its own. Editing song.ini or notes.mid in place changes the file's mtime but not its folder's, so the
skip would keep a stale md5 and stale names. It also assumes song folders are leaves; 117 charts here live in song
folders nested inside other song folders. And with pwalk8 the whole rescan walk is already 0.19 s, so the
remaining prize is ~0.15 s at most.

Patch: scan.patch (git diff of the worktree).
