# Speed exploration of chart scanning and analysis (2026-10-06)

The user asked whether Hydra's chart scanning and chart analysis could go faster, and asked us to explore every idea in a scratchpad without touching the real code. This is the full write-up.

Nothing in the real code changed. Main is still at `bc58282` (Hydra Deluxe 2.1.0). Every experiment ran in a detached throwaway worktree with no branch and no commit. Those worktrees are now removed. Each experiment's changes are saved as a patch in `patches/`, and each patch applies cleanly to `bc58282` (checked with `git apply --check`). Each agent's working notes are in `notes/`.

## The short answer

The path search was never the slow part. It is about 7% of the work in a library run. Three other things are where the time goes, and all three have changes that kept every stored result byte-identical across the whole library:

1. **The database writer.** One thread saves every result, and the eight analysis workers spend more time waiting for it than analyzing. Grouping saves cut a full library run by about a third.
2. **Building the score graph.** It cost more than the search. Three small changes cut it by about 80%.
3. **Parsing chart files.** This was the biggest single piece of each worker's job. Lean readers roughly halved it, and the kick dynamics count got about 8x faster.

The library scan also halved, but it was already only about 4 seconds.

Dominance pruning, the idea the user remembered dismissing, was tested on the whole library and is unsound. It loses the true best score on 3 charts. The old reason it was dismissed still holds, and now it is measured.

Two changes need the user's decision before they could ship. Both are in the writer fix, covered in "Decisions needed" below.

## How the experiments were run

Six experiment agents ran in parallel, each in its own worktree under `.claude\worktrees\perf-<key>`. They covered profiling (`prof`), the scan (`scan`), the writer (`write`), the engine and build settings (`engine`), the search algorithm (`algo`) and parsing (`parse`). Four web scouts searched GitHub, papers and vendor docs first. A separate session researched compiler settings, and its findings are folded in below. Its own write-up is `docs/handoffs/2026-10-06-cpp-optimization-research-handoff.md`.

**Timing.** Every timing that matters ran through one machine-wide lock, so only one benchmark ran at a time. The lock script also waited for compilers and other Hydra processes to finish before starting the clock. Each log entry records how many compiler processes were running at the start; in nearly every reported run it was 0. A and B runs were interleaved (A, B, A, B...) and the medians compared. The full log of every locked run is `notes/bench_log.md`.

**Data.** The data was the user's library at `C:\Clone Hero`: 23,160 folders, 19,436 charts, 18,811 distinct charts analyzed. Analysis was timed on a copy of the real GUI database with `--redo`, because a fresh database once hid a 69-second slowdown. Correctness was checked on fresh databases.

**Correctness.** A speedup only counted if the output stayed identical. Scan changes were checked by comparing the charts table row by row, in insert order. Analysis changes were checked two ways:

- A whole-library run of the baseline and the experiment into two fresh databases, with every content column compared across results, paths, path_refs, songmeta, dynamics and meta.
- A per-chart hash of every row that would be stored, printed by a scratch harness mode.

The only column that ever differed was `result_id`. It is the row number SQLite hands out in whichever order worker threads finish, so it varies between any two runs. It is not part of a result.

**Limits of the method.** All numbers are warm-cache. The OS file cache cannot be flushed without admin rights, so a cold disk was only estimated, with a separate test. The machine was sometimes noisy. The baseline full-run time drifted between 12.6 s and 20.0 s across sessions, so each comparison uses the baseline measured in the same session. The machine is a Ryzen 7 9800X3D (8 cores, 16 threads), 31 GB RAM, and a Crucial CT2000E100SSD8 NVMe SSD.

## Where the time went

This is the first per-phase split of a whole-library run anyone has recorded. It comes from the `prof` agent's timers. The timers cost nothing measurable: 16.81 s with them and 16.81 s without, with identical results.

| Phase | Real db, `--redo` | Fresh db |
|---|---|---|
| Whole process (wall) | 15.2 s | 16.8 s |
| Scan | 0.95 s (all cache hits) | 4.23 s |
| Batch analysis (wall) | 13.6 s | 12.2 s |

Inside the batch, summed over all threads on the real-db run:

| Where | Time | Share of worker time |
|---|---|---|
| Analysis workers, busy | 52.4 s | 100% |
| Parse | 22.4 s | 43% |
| Graph build | 17.7 s | 34% |
| Kick dynamics count | 4.8 s | 9% |
| Main path search | 3.5 s | 7% |
| Enumerate (main and all-0) | 1.6 s | 3% |
| Everything else (prepare_row, rebuild, all-0 search) | 1.5 s | 3% |
| **Workers blocked waiting for the writer** | **54.9 s** | — |
| **Writer thread busy saving** | **12.5 s of 13.6 s** | — |

The writer thread was busy 92% of the batch. Of its time, 58% was the per-chart COMMIT. If saving cost nothing, the batch would take about 6.5 s, which is 52.4 s spread over 8 workers.

Per chart, the median takes 2.0 ms to analyze and the slowest takes 402 ms. The 20 slowest charts are only 5.7% of worker time. Text `.chart` files are 22% of charts but 30% of worker time, because their parse was about three times slower than a `.mid`'s.

## What worked

Every change in this section left stored results identical on the whole library.

### 1. The database writer (`write` agent, `patches/write.patch`)

**What was wrong.** Each chart was saved in its own transaction. About ten SQL statements were compiled from text for every chart. Each commit rewrote every database page it touched, including shared index pages, into SQLite's write-ahead log (WAL, a journal file next to `hydra.db`). SQLite also folded that log back into the database every 1,000 pages, which is a full flush to disk.

**What was tried.** Each idea sat behind its own switch, so one exe could test any combination.

*Statement cache.* Each statement is compiled once and reused (reset and rebind per chart), including BEGIN, COMMIT and SAVEPOINT. Compiling had cost about 2.4 s of the writer's time per run. Pooled wall time fell from 17.1 s to 14.6 s. This is code-only and low risk.

*Group commit.* One transaction holds up to 16 charts, and each chart is its own SAVEPOINT inside it. So a chart whose save fails still fails alone, as decision D71 requires. A group commits when it reaches 16 charts, or as soon as no results are waiting, so a transaction is never left open while the thread waits. Groups of 64 did no better, because groups rarely fill: there are about 1,400–1,900 commits per run either way.

*Less frequent checkpoints.* `PRAGMA wal_autocheckpoint` was raised from 1,000 pages to 10,000. This was the biggest single step.

**Result.** All three together brought a full run to a pooled 11.6 s. Against each session's own baseline: 11.5 vs 16.3, 12.1 vs 20.0, 12.6 vs 17.8 and 11.7 vs 16.3 s. That is about a third faster. The batch phase alone went from about 15 s to about 10.4 s. Writer time per chart fell from 0.79 ms to about 0.50 ms.

**Correctness.** On real-db copies after `--redo`: 0 rows differed across 19,106 results, 91,196 paths and path_refs, 19,099 songmeta rows, 18,820 dynamics rows and 19,436 charts. Fresh databases: 18,811 charts compared, 0 differing.

**What doesn't help yet.** More workers made runs slower: 12.6 s at 8, 13.9 s at 12, 18.1 s at 15. The extra workers take CPU time from the writer. Memory was not the limit; peak commit was 640, 660 and 685 MB. Moving tempo-map encoding and freeing onto the workers made no difference (11.5 vs 11.7 s). Turning off disk sync entirely (`synchronous=OFF`) cut one run to 10.2 s, but it is not recommended. It only shows what the remaining syncs cost.

**What's left.** The writer is still the bottleneck at about 0.5 ms per chart. Its next costs are the replace-purge (1.2 s per run), the path-node inserts (0.8 s) and the dynamics write (0.8 s).

### 2. Building the score graph (`engine` agent, `patches/engine.patch`)

Temporary timers inside the graph build showed three costs, measured on the library baseline:

- About a third went to `set_head_time`, which rebuilt two vectors at every note.
- About a quarter went to `category_scores`, which allocated a vector of notes for every chord.
- About a tenth went to copying backend rows onto every SP deactivation edge.

Those timers are saved separately in `patches/engine-probe-timers.patch`. On the search side, the main loop took about 2.8 s, enumerate about 1 s, rebuild 0.35 s and the all-0 pass about 0.8 s.

The changes, each timed alone on the giants. "Giants" means blink-182 Discography, Rise Against Discography (2024) and Endless Setlist I, analyzed 5 times each, as the median of 3 interleaved runs; the baseline was 1.63 s.

| Change | Giants | Verdict |
|---|---|---|
| 6a: `set_head_time` trims its vectors in place | 1.29 s | Good, but 6c supersedes it |
| 6b: `category_scores` sorts notes in a fixed array (stable insertion sort, same order as `Chord::notes(true)`) | 1.51 s | Worth doing |
| 6c: each deactivation edge stores a start and end index into one shared list of rows, instead of a copy | 1.33 s | Worth doing; biggest single win |
| Segment heap: one line in `src/app/long_paths.manifest` opts the exe into Windows' newer allocator | 1.43 s | Worth doing; one line |
| 6b + 6c + segment heap together | 1.09 s (about 33% less) | Worth doing |

For 6c, a self-check build kept the old copies alongside the new ranges and compared them at every graph build. It found no mismatch across the whole library.

**Result.** On one thread over the whole library, total analysis went from 17.6 s to 6.6 s. Graph build alone went from 13.6 s to 2.7 s.

**Correctness.** All 18,811 per-chart row hashes were identical. The fresh-db comparison matched every content column; only the write-order `result_id` differed.

**Why a full run didn't move.** `hydra_batch --redo` took 12.6 s baseline against 12.9 s with the changes, because the writer sets the pace. Even the baseline gets only about 2.3x from 8 workers. The engine gains reach the batch only after the writer fix.

**Not tried.** The agent ran out of tool calls before trying the rest:

- Profile-guided optimization (PGO).
- Reusing enumerate for the all-0 pass. Worth about 0.5 s of single-thread CPU.
- Smaller search-loop tweaks. The whole loop is about 2.8 s.
- clang-cl. It is not installed.
- mimalloc. It needs a download.

### 3. Parsing (`parse` agent, `patches/parse.patch`)

**Where parse time went** (one thread, whole library, about 15 s):

- **.chart files (6.1 s).** 4.96 s went on reading sections. Every line of every section (guitar, vocals, lyrics) became a heavy entry stored under its tick. The drum notes themselves took only 0.78 s.
- **.mid files (about 7 s).** Decoding took 2.77 s, because every track became 64-byte messages before the drum track was even picked. The drum pass took 3.0 s, from sheer volume: all four difficulties plus every note-off. Freeing the decoded file cost about 1 s more.
- **The kick dynamics count (3.85 s).** `Chord::notes()` allocated a small vector at every timestamp.
- **.sng/.srb archives.** Only 0.65 s.

**The three changes.** Each can be switched off with `HYDRA_PERF_OFF`, so A/B runs used one exe.

*Dynamics count.* It reads the chord's five lanes in place. The whole library went from 3.8 s to 0.48 s (8x); the slowest 50 charts from 0.17 s to 0.019 s. It is a five-line change.

*Lean .mid decode.* Each track is walked once to learn its name, applying the same length checks, so it throws the same errors. Then only what the parser reads gets decoded: track 0's tempo and meter, the first PART DRUMS track's text events, the notes that can act at this difficulty, and EVENTS text events. Dropped messages pass their time gap to the next message kept, so every note keeps its tick. On its own it saves about 2.3 s. The drum pass still converts messages one at a time, so there is more to gain.

*Lean .chart reader.* Only [Song] and [SyncTrack] keep full entries. The drum section becomes a flat list of 24-byte lines, sorted by tick once; the sort is stable, so lines at one tick keep file order. [Events] keeps only practice sections. Every other section is still read line by line, only so it throws wherever the old reader threw. Well-formed numbers are read directly, and anything unusual falls back to the original `std::stoi`/`stoll`, so those rules still decide. On its own it saves about 3.3 s. This is the biggest single parse win.

**Result.**

| Measurement | Before | After |
|---|---|---|
| Whole-library parse, quiet runs | about 15.0 s | 8.9 / 9.1 / 9.9 s |
| Parse plus dynamics count | about 18.9 s | about 9.4 s |
| Slowest 50 charts (median of 3 interleaved pairs) | 1.380 s | 0.611 s (2.26x) |
| Nirvana "Endless, Nameless Setlist" .chart (9 MB) | 288 ms | about 50 ms |
| Full batch `--redo`, median of 6 pairs | 12.9 s | 11.3 s |

The batch medians differ by 1.6 s, but the quiet pairs differ by only 0.3–0.8 s. Again, the writer sets the pace.

**Correctness.** A new harness mode hashed the whole parsed song: every note, timecode, map, section and dynamics field, plus the stored dynamics blob and any failure's exception type and message. Over the whole library, for each change alone and all combined, 18,869 charts were compared and 0 differed. The 58 failures gave the same text; all are "no notes" failures, so the library never exercises the error paths.

To cover those paths, the agent wrote 41 crafted files, all identical to baseline including exception types and messages. They include:

- bad and overflowing numbers in every kind of section;
- duplicate and unfinished sections, odd keys and unsorted ticks;
- two drum tracks, and an oversize text event in another track;
- SMPTE timing and truncated files.

A whole-library rerun at Hard, with Pro Drums off and 2x Bass off, also matched: 18,869 compared, 0 differing, all 5,744 failures the same. The engine row hash, `8d17f958172bd6f4` over 18,811 charts, matched the engine agent's baseline. The fresh-db batch comparison matched every content column.

**Caveat.** The lean readers sit next to the old code behind the switch, so the patch duplicates code on purpose. A real change would delete the old paths. A MIDI file with zero tracks was undefined behaviour in the old parser and still is; nothing in the library has one.

### 4. The library scan (`scan` agent, `patches/scan.patch`)

**Correctness first.** `hydra_bench --scan --db` writes the same charts table the GUI writes. It was compared against baseline row by row in insert order, which also checks scan order. The library has 542 md5s with more than one copy, so decision D63 (the first-listed copy names a duplicate) was really exercised. Every combination: 19,436 charts, 0 differing.

| Change | Before → after | Verdict |
|---|---|---|
| Parallel folder walk. N threads list folders into a tree, which is then replayed in the serial walk's exact order. | Walk 0.87 → 0.29 s (4 threads), 0.19 s (8), 0.16 s (16) | Worth doing |
| Reuse one read buffer per thread. Today every file gets a fresh, zero-filled 1 MB buffer: about 19 GB of zeroing per scan. | Read stage 3.55 → 2.68 s | Worth doing |
| Hash the biggest files first. The three 1 GB Endless Setlist .sng files sit near the end of walk order and used to finish alone. Results are still stored by walk position. | Read stage 3.40 → 2.69 s; with the reused buffer, 1.88 s | Worth doing; tiny change |
| `FindFirstFileExW` with FindExInfoBasic and FIND_FIRST_EX_LARGE_FETCH | 0.87 → 0.85 s | Not worth it (noise) |
| CreateFileW + ReadFile with FILE_FLAG_SEQUENTIAL_SCAN | Same as the reused buffer alone | Not worth it; the gain was the buffer |
| More hashing threads (12, 16) | Slower: 3.16 s and 3.19 s against 3.55 s | Not worth it with a warm cache |
| Skip a song folder whose own date is unchanged on rescan | Walk 0.77 → 0.22 s, but found 19,319 charts, not 19,436 | Unsafe |

**Best combination** (parallel walk on 8 threads, reused buffer, biggest first): a full scan went from 4.22 s to 2.06 s. A rescan with a full cache went from 0.80 s to 0.19 s. The new floor is about one thread's MD5 of the largest 1 GB file.

**Why folder skipping is unsafe.** The 117 missing charts live in song folders nested inside other song folders, so "a song folder is a leaf" is false in this library. Editing song.ini or notes.mid in place changes the file's date but not its folder's, so a skip would keep a stale md5. The parallel walk already brings a rescan to 0.19 s, so a safe skip could save at most about 0.15 s more.

**One gap.** In the experiment the folder-progress callback fires only once, at the end. A real change needs the calling thread to report the count while the walk runs.

**Cold disk.** The `prof` agent hashed every chart file (7.83 GB) with reads that bypass the OS cache. At 8 threads that took 3.9–4.1 s cold against 3.0–3.2 s warm, so a cold scan adds about 1 s on this drive. MD5 on the CPU is the limit, not the disk. The 16-thread cold number (about 8 s) is not trustworthy, because Python's global lock likely throttled it. Every MD5 matched.

## What did not pay

### Dominance pruning, and why it was dismissed before (`algo` agent, `patches/algo.patch`)

**The history.** The user remembered dismissing this but not why. It was dismissed in August, during the activation-DP experiment (memory note `archive/phase4-activation-dp-cpp.md`). That experiment built a backward "best score from here" table. It was score-correct but took about 140 s on blink-182 against the current search's 8 s. The note's reason: an activation always spends the whole meter, so holding more SP is not automatically better than holding less. So the DP had no safe way to throw away low-SP states.

It was never tried inside the current search. No commit, doc or other note mentions it, so it was a reasoned dismissal, not a measured one. A related idea was measured in August and deleted: dropping paths that can't catch up even in the best case (`archive/tight-bound-prune-does-not-pay.md`). It was exact, but pruned 0.0036% on blink and cost 7%.

**What the search already does.** Every step, all paths move forward one timestamp. Paths are then grouped by a key meaning "these paths have the same future":

- the track (base or SP);
- off SP: the bar count, plus a ready class (how many upcoming fills would refuse this path, or put it over the ms limit, given when its SP became ready);
- on SP: the SP end tick, the banked phrase in squeeze reach, and the SP end where the newest phrase can still be squeezed out.

Paths holding a spent or banked-ahead phrase are not grouped. Inside a group, equal scores fold into tied variants, and only the top depth+1 distinct scores survive (or everything within N points). That is already dominance pruning, within a group.

**The test.** The scout's rule drops a path when enough other paths have both more points and at least as many bars, with an equal ready class. "Enough" means past the user's depth band. It was added behind a switch and run over the whole library into fresh databases. Of 18,811 results, 58 changed:

- 45 differed only in the all-0 section.
- 13 differed in the main near-best list.
- 3 lost the true best score. Between the Buried and Me's "Informal Gluttony" (two charter versions) dropped from 932,440 to 932,280, and "Colors" dropped from 8,791,335 to 8,791,175.

**The counterexample, in game terms.** In "Informal Gluttony", at tick 236160, path A holds 1 bar and leads by 4,420 points. Path B has just finished a squeeze-in with 0 bars. A must later burn all 3 bars at once, at tick 270720. That pushes its next activation from tick 304800 to 316800. B plays two 2-bar activations, at 270720 and 304800, and finishes 160 points ahead. More SP hurt, because you can't spend part of the meter.

The smallest case is Clairo's "Juna" (887 notes), in the all-0 section. An extra bar forced earlier 2-bar activations. The pruned path would have ended on a 3-bar activation and won by 1,220 points.

**Verdict.** It is unsound, and it would save only 6.1% of the frontier on the charts where it happened to be right. The August reasoning was correct.

### The other search ideas

- **Upper-bound pruning (branch and bound).** The bound is the best score reachable with every branch free, computed backwards once. It kept every result identical, but shrank the frontier by only 6.8% with a realistic mid-search floor (8.8% with a perfect one). That is under 1% of analysis time. This matches the August finding.
- **CHOpt's skip-ahead.** CHOpt jumps straight from one SP-granting note to the next. Our graph mostly does this already: 2.48 million graph nodes against 45.3 million notes in the library. 43.7% of path-steps sit on nodes with no decision, so skipping them could save only part of each step's work.
- **Best path first, near-best list afterwards (lazy k-best).** Searching at depth 0 has a 73% smaller frontier and runs about 5x faster (1.1 s vs 5.6 s summed). But stored records also depend on details the search produces as side effects of its fold order: variant order, the tie cut-off and the two-tier ms-limit rule. Rebuilding those afterwards would be a risky rewrite for about a 6% saving.

**Grounding.** The engine search, summed over every chart, is about 6.3 s of CPU, roughly 8% of analysis. An ordinary chart searches in about 0.23 ms. The 12 giants add up to 0.49 s. blink-182 Discography takes 0.18 s in total (0.04 s parse, 0.14 s graph and search).

### Starting the slowest charts first

The `prof` agent replayed the measured per-chart times through an 8-worker pool with no save limit. Walk order finishes in 6.75 s and longest-first in 6.55 s, which is the best possible. File size is a usable predictor within one file type (rank correlation 0.93 for .chart, 0.60 for .mid), but weaker across types (0.56), because an archive's size is mostly audio.

The `write` agent tested it for real. Unordered took 11.7 s, size-first 12.2 s and previous-run-time first 12.6 s. Peak memory rose to 737 MB and 826 MB, because the giants ran together. Not worth it. The scan's version, hashing the 1 GB archives first, is the one that pays.

## Compiler and build settings

These come from the separate research session's handoff. They were not measured here.

Our Release build uses CMake's stock MSVC flags: `/O2 /Ob2 /DNDEBUG`, dynamic CRT, `/fp:precise`. It has no link-time optimization, no profile-guided optimization and no symbols.

That session recommends, in order:

1. **Link-time optimization.** Set `CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE` behind `check_ipo_supported()`. It lets the compiler inline across files. Risk to results is low, but it should be proven with the harnesses these experiments built.
2. **Release symbols.** Add `/Zi` with `/DEBUG /OPT:REF /OPT:ICF`. The program doesn't get slower, and VSDiagnostics could then name the 58% of profile time currently stuck in unnamed Windows code.
3. **mimalloc.** It is a new third-party dependency, and the clean static route needs `/MT` where we use `/MD`.
4. **SQLite compile options.** `SQLITE_DEFAULT_MEMSTATUS=0`, `SQLITE_OMIT_SHARED_CACHE` and `SQLITE_DEFAULT_WAL_SYNCHRONOUS=1` are worth about 5% of SQLite's own CPU, per sqlite.org.
5. **PGO**, later.

Leave alone:

- `/fp:fast` and `/fp:contract`. Stored records depend on bit-for-bit float math (`src/core/timing.h`).
- `/arch:AVX2`. It crashes on older CPUs and gains little on integer and graph code.
- Turning off `/GS`. The app reads chart files downloaded from the internet.

## Follow-up from a second research session

A second session ("C++ optimization research leads") checked the compiler handoff's three small leads and the build itself. Nothing in the repo was changed there.

**song.ini parsing is not slow.** It benchmarked a verbatim copy of `read_song_ini_keys` over all 19,352 song.ini files. Parsing all of them takes 0.11 s of CPU; a string_view version takes 0.06 s and gives identical maps. Across 8 scan workers that is about 6 ms of wall time. The 1.8 s the profile attributed to "song.ini" is opening and reading the files, not parsing them.

**`word_stoi` is already covered.** The parse patch's `lean_stoi`, `lean_stoll` and `lean_try_parse_int` handle it. The last one also skips the exception `try_parse_int` throws for every named key in a .chart [Song] section. Tick numbers fit in the small-string buffer, so the old temporary string never allocated anyway.

**Precompiled headers would save little.** The common Windows and standard headers cost about 0.75 s per file. `sqlite3.c` alone takes about 8 s, and precompiled headers can't help it.

**The build runs one project at a time.** `build_cpp.ps1` calls `cmake --build` without `--parallel`, and no preset sets a job count. So MSBuild builds one project at a time, with `/MP` only inside each project. Two cold builds of the full target list took 83 s as today and 60 s with `--parallel`, about 28% faster, with all 8 exes built both times. That is one run each, so the figure is rough. This speeds up building, not the app. The user has not decided whether to add it.

**Open question: the first-ever scan may be much slower than measured here.** That session's first read of all 19,352 song.ini files took 44 s; the second took 0.9 s. It could not tell whether the cause was a cold disk cache or Windows Defender scanning each file on first open.

This matters, because it disagrees with the cold-disk estimate in the scan section. That estimate (cold adds about 1 s) bypassed the OS cache for reads, but it ran on files that had already been opened. So it would not catch per-file costs on first open, such as cold folder metadata or an antivirus scan. If the 44 s is real, a user's very first scan is held up by opening files, and none of the warm-cache scan numbers here show it. Measuring it needs either a reboot or a library copy that has never been opened, timed once.

## Profiling notes

WPR, Windows' own tracer, refuses to run without admin. Visual Studio's collector (VSDiagnostics.exe, installed with VS 18 Community) works without admin when its CPU agent is loaded directly. Loading it by config file crashes on a library version mismatch. Nothing was downloaded.

A profile of 20 copies of blink-182 put 58% of CPU time in Windows system code: ntdll 38.8%, kernel 13.6%, C runtime 5.5%. Without OS symbols those functions can't be named. Heap allocation, page faults and memory copies are the likely causes, but that is a guess. Hydra's own top functions each took about 2%. The `prof` notes give the exact commands.

## Web research, in brief

Four scouts searched before the experiments. Their useful leads became the experiments above. A few of their ideas were ruled out for this app:

- **Reading the drive's file table directly**, as Everything and WizTree do. It needs admin rights or a background service.
- **Changing the hash.** The MD5 of every byte is the song's identity in Clone Hero, DMBot and every stored table.
- **Memory-mapping small files.** No gain for files this size.
- **Swapping thread-pool libraries.** One chart per thread is already the right grain.
- **CHOpt.** The other well-known Clone Hero optimizer is a longest-path search with skip-ahead edges. Its useful idea is already in our graph.

## Decisions needed before any of this ships

Most of the work is code-only, with identical results, so it can go ahead on a recommendation, with the usual derive-once review. Four items need the user first:

1. **Group commit failure scope.** With 16 charts per transaction, a failed COMMIT fails the whole group of up to 16 charts, not one. A failed commit is rare: a full disk or a locked file. A chart whose own save fails still fails alone, through its savepoint. Progress on screen would also update after each group, not after each chart.
2. **The checkpoint setting of 10,000 pages.** It is a new number. It lets the WAL file next to `hydra.db` grow to about 40 MB during a run, instead of about 4 MB.
3. **The group size of 16.** It is also a new number. 64 measured no better.
4. **mimalloc.** It would be a new third-party dependency and is untested.

Two gaps also need closing before the writer change ships. Cancel was never tested, because `hydra_batch` has no cancel; the design keeps it prompt, since a transaction is never open while the thread waits. And the GUI's Analyze library was not tested, though it runs the same `run_batch`.

## Suggested order, if the user wants to proceed

1. **The writer fix first,** because it unblocks everything else.
2. **In parallel, in separate worktrees:** the graph-build changes (6b, 6c and the segment heap), the parse changes, and the scan changes. The parse patch would first drop its duplicated old paths.
3. **Link-time optimization and Release symbols together,** measured with the same harnesses, then one named profile to decide on mimalloc.
4. **Measure a first-ever scan once** (see the open question above), before spending more effort on the scan.
5. **Then a combined whole-library timing.** No run here measured every change together. My estimate is that a full run would land around 10 s, set by the writer's remaining 0.5 ms per chart. Single-chart analysis in the GUI would be roughly twice as fast. Both figures are estimates, not measurements.

## Files in this folder

`patches/` holds one patch per experiment, each made with `git diff HEAD --binary` from its worktree before removal and each applying cleanly to `bc58282`. They are experiment code, not shippable: every idea sits behind an environment-variable switch next to the old code, and some carry temporary timers.

| Patch | What's in it |
|---|---|
| `write.patch` | Statement cache, group commit, WAL checkpoint, worker count, ordering, perf report |
| `engine.patch` | 6a/6b/6c, the segment-heap manifest line, and the `hydra_bench --engine` harness |
| `engine-probe-timers.patch` | The temporary graph-build timers |
| `parse.patch` | Lean .chart and .mid readers, the dynamics fix, and `hydra_bench --parse` |
| `scan.patch` | Parallel walk, buffer reuse, biggest-first, the FindFirstFileExW switch, the folder-skip simulation |
| `algo.patch` | Dominance switch, bound and depth-0 shadow passes, frontier logging |
| `prof.patch` | Phase timers, per-chart CSV, and a debug-info CMake setting that must not ship |

`notes/` holds each agent's working notes (`<key>-NOTES.md`, with exact commands and switch names), the shared experiment brief, the benchmark lock script (`bench_run.ps1`), and the log of every locked timing run.

The session scratchpad was deleted after this report was written. It held the copied databases, exe sets, CSVs, per-agent timing and comparison scripts, and the 1 GB file copies. Paths into it that appear in the notes no longer exist. The patches and notes here are enough to rebuild any experiment. The comparison scripts were not kept, so a rerun would need new ones.
