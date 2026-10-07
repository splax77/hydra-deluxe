# Plan: ship the 2026-10-06 speedups (2026-10-06)

**Status: planned, not yet approved for dispatch.** Base: main at bc58282 (Hydra Deluxe 2.1.0). Task briefs are written at dispatch time as `tasks/perf-<id>.md`, each starting from `docs/agents/brief-preamble.md`, the way phases 6 and 7 did. No code listings live in this plan or in the briefs: each task names the patch hunks to lift, by file and function, and the implementer writes the code.

**User decisions (D86, `docs/audit/2026-10-03-fix-decisions.md`, 2026-10-06).** Quoted in full, because every number below comes from them:

1. "A failed group COMMIT retries each chart alone. The batch writer saves up to 16 charts in one transaction, each in its own savepoint. If the group's COMMIT fails, the writer rolls the group back and saves each of its charts again in its own transaction. Only a chart whose own save fails is marked failed, so D71 item 6 holds exactly as before."
2. "A group holds at most 16 charts. 16 and 64 measured the same (11.5 s against 11.7 s for a full run), because a group also closes whenever the writer catches up with the workers; groups average 10 to 13 charts. At about 0.5 ms per chart, 16 keeps the store lock and the progress lag near 8 ms, under one screen refresh."
3. "The WAL checkpoint moves to 10,000 pages only while a batch runs. At batch end one checkpoint forces every result to disk and truncates `hydra.db-wal` to zero, and the setting goes back to SQLite's 1,000 pages. 10,000 pages of 4,096 bytes plus a 24-byte frame header each is 41.2 MB of log at most. The installed Hydra's log already stood at 45.6 MB (11,074 pages) that day, because SQLite reuses the log file without shrinking it."
4. "mimalloc gets a measured trial later, not a ship decision. After the other speed changes land, an agent may download the mimalloc source release from github.com/microsoft/mimalloc into a throwaway worktree and measure it. Shipping it, a new third-party dependency, comes back to the user with numbers. The Windows segment heap (one manifest line) ships in the meantime."
5. "`build_cpp.ps1` builds with `--parallel`, uncapped. Two cold builds took 83 s without it and 60 s with it, one run each. It changes build speed only."
6. "The user times a first-ever scan after their next reboot. Another session read every song.ini in 44 s the first time and 0.9 s the second. The user runs one timed scan into a scratch database after a reboot, before opening Hydra, so the scan work knows whether first opens dominate."

Also binding: D71 item 6 (a failed save during a batch fails that chart alone and the batch goes on), D72 (a database that fails at startup or at a batch's start says so) and D73 (every store read throws instead of answering empty).

## What this plan does, in plain terms

The perf exploration (`docs/handoffs/2026-10-06-perf-exploration/REPORT.md`) found that a whole-library analysis is held back by the one thread that saves results, and inside each worker by parsing and graph building. It left seven experiment patches in `docs/handoffs/2026-10-06-perf-exploration/patches/`, each behind an environment switch, with timers. This plan turns the ones worth shipping into real code: the switches, the timers and the old code paths go, and every change proves it stores exactly the same bytes as before over the whole library (`C:\Clone Hero`, 18,811 distinct charts).

Nothing here moves a stored result, a score, a path or a displayed text. That is the whole point of the proof harness in wave 0: no stamp in `src/store/stored_versions.h` moves, and the byte-identical comparison is the evidence (see "Stored versions" below).

The order is: one small harness task first, then five independent code tasks in one parallel wave of worktrees, then one combined timing and the mimalloc trial. The user's own reboot-time scan measurement runs whenever the user is ready and needs no agent.

## How it runs

The rules are the ones CLAUDE.md and the memory notes set:

- **The sources are committed.** Worktrees only see committed files, so the perf report folder (`docs/handoffs/2026-10-06-perf-exploration/`, with its patches and notes), both 2026-10-06 handoffs, D86 and this plan were committed to main together before any dispatch.

- **Who runs what.** This plan was written on Fable. Each code task is one Opus implementer in its own worktree. Each derive-once reviewer runs on Sonnet (rule 2).
- **Waves, not a queue.** Wave 1's five tasks fork from H1's branch tip the moment H1's code exists, before H1's review, because the harness is tools and tests only (memory `parallel-by-default.md`: "after merge X" means fork from X's tip once its code exists). Each task merges main back in before its own review. Wave 2 forks the same way from wave 1's join tip.
- **Tests.** Agents run only the tests their brief names, with `-tc=` or `-sf=` filters, and only the `hydra_uitest --test <name>` scripts named. The main session runs the full suite once per merge (rule 1).
- **Timing.** Every whole-library run, even a correctness-only one, goes through the benchmark lock `tools/bench_run.ps1` (H1 commits it), one at a time. A and B are interleaved and the median reported with the run count and the compilers-busy count.
- **Review.** Every code task ends with the derive-once review from `docs/agents/derive-once-review.md`, one round: the reviewer reports, the author fixes once, the reviewer fixes anything left and signs off (rule 5). Wave 1 joins on one branch through `docs/agents/integrate.md`, then one review of the join and one full-suite run by the main session.
- **Status and budget.** Every agent writes a status line every 10 tool calls or 5 minutes, starts wrapping up at 100 calls and stops at 150 (rule 7). Every brief names `C:\Users\Patrick\.claude\hooks\state\status\<agent id>.md` and the `status_append.ps1` call.
- **Nothing silent.** Anything a task finds that would change a display or a stored record beyond D86 stops and comes to the user (rule 3). The two such items this plan already sees are in "Questions for the user" at the end.

## Proving identical results (the recipe every task follows)

A speedup counts only if the output is identical. The comparison scripts the experiments used were deleted with their scratchpad, so H1 rebuilds them as committed tools, and every later task runs this recipe in its own worktree before asking for review.

**The exes.** Two sets: the baseline and the task's. The main session builds the baseline once at dispatch, from the exact commit the wave forks from, in a detached throwaway worktree through `tools/build_slot.ps1`, and copies `hydra_batch.exe` and `hydra_bench.exe` to `C:\Users\Patrick\.claude\hooks\state\bench\baseline-<short hash>\`, so five agents do not each cold-build a baseline. The task builds its own `hydra_batch` and `hydra_bench` by `-Target` in its worktree (`hydra_bench` is EXCLUDE_FROM_ALL, so a plain build never rebuilds it).

**The settings.** `hydra_batch` and `hydra_bench` read `hydra_settings.ini` from the folder they run in. Both exe folders get a copy of the installed `C:\Program Files\Hydra\hydra_settings.ini` with its `dm_last_user` line removed, and the brief has the agent confirm, by reading the ini against the key table in `src/app/config.cpp`, that it says Expert, Pro Drums on, 2x Bass on, depth 4 scores, Path limit 10 ms, SP cap 4 and the folder `C:\Clone Hero`. Never guess a key name.

**Fresh-database run.** Through the lock, `hydra_batch.exe --db <scratch>\A.db` with no folder argument (D79 item 1: with no folder arguments the tool scans the ini's folders and saves the library, so the `charts` table is written too), once per exe set. Then `py tools\compare_db.py A.db B.db`. It compares every content column of `results`, `paths`, `path_refs`, `songmeta`, `dynamics`, `charts` and `meta`, leaving out `result_id` because that is only the order worker threads finished in; `charts` is compared row by row in insert order, because a scan's order is part of its answer (D63: the first-listed copy names a duplicate). The expected line for each table is "N rows compared, 0 differ". Any difference is a finding to report, never something to fix by loosening the comparison.

**Per-chart hashes.** `hydra_bench --engine "C:\Clone Hero" --out rows.txt` prints one hash over every prepared row (structure, nodes, best path, summary) and writes a per-chart line; `hydra_bench --parse "C:\Clone Hero" --out parse.tsv` does the same for the whole parsed song plus its dynamics blob, and records a failure's exception type and message. Both are single-threaded. Run baseline and task through the lock and compare the printed hash and the per-chart files. Never compare against the numbers in the notes (`8d17f958172bd6f4`, `937d9aed20f59dff`): those came from scratch harness code and H1's committed harness may hash differently. Always A against B from the same harness build.

**Scan comparison.** `hydra_bench --scan "C:\Clone Hero" --db <scratch>\scanA.db` for each exe set, then `py tools\compare_db.py scanA.db scanB.db`, which already compares `charts` in insert order. A second run on the same database exercises the rescan cache; compare that too.

**Real database with `--redo`.** Timing of analysis runs on a copy of the user's real GUI database, never only on a fresh one, because a fresh database once hid a 69-second slowdown (memory `naming-copies-query-slowdown-2026-10.md`). Hydra is usually running, so the copy is taken through SQLite's backup API, which reads a consistent snapshot while the live connection keeps writing, and leaves a single-file copy with no `-wal`: `py -c "import sqlite3; s = sqlite3.connect(r'file:C:/Program Files/Hydra/hydra.db?mode=ro', uri=True); d = sqlite3.connect(r'<scratch>\real.db'); s.backup(d); d.close(); s.close()"`. Keep that copy pristine and copy it again for each run. Then `hydra_batch.exe --db <copy> --redo` through the lock.

**The lock.** `pwsh -NoProfile -File tools\bench_run.ps1 -Label "<task>-<what>" -Script <your timing .ps1>`. It waits for the lock and for compilers and other Hydra processes to quiet down, runs the script, and logs the run with the compilers-busy count. Call it with the PowerShell tool's timeout at 600000 ms and keep each timing script under five minutes. Exit code 3 means the lock stayed busy: do other work and retry.

## Stored versions: no stamp moves

`src/store/stored_versions.h` (ADR 0018) holds every stamp: `kResultsStamp` ("2.1.0+allzero"), `kPathFormatStamp` (7), `kDynamicsCountStamp` (2), `kDynamicsBlobStamp` (2), `kChartMetaStamp` (2) and `kSongLengthStamp` (2). Each bumps only when an unchanged input would read or compute differently. Every change in this plan keeps stored bytes identical, so no stamp moves, and no task may touch that file. The evidence is the recipe above: `compare_db.py` reporting 0 differing rows in every table over the whole library, and identical `--engine`, `--parse` and `--scan` hashes. The parse change is the one that could look like a stamp case (the chart readers change), but `kDynamicsCountStamp`'s and `kResultsStamp`'s rule is "when a chart reads differently", and the proof is that no chart does, including the 41 crafted error files.

## What in the patches must not ship

The patches are experiment code. The list of what stays behind, so no brief has to rediscover it:

Every environment switch and its reader: `HYDRA_TXN_GROUP`, `HYDRA_STMT_CACHE`, `HYDRA_WAL_CKPT`, `HYDRA_SYNC`, `HYDRA_CACHE_KB`, `HYDRA_ORDER`, `HYDRA_LEAN`, `HYDRA_WORKERS` (`write.patch`); `HYDRA_PERF_OFF` and `perf_exp_on` (`parse.patch`); `HYDRA_SCAN_FINDEX`, `HYDRA_SCAN_PWALK`, `HYDRA_SCAN_HASHIO`, `HYDRA_SCAN_HASHBUF_KB`, `HYDRA_SCAN_REUSEBUF`, `HYDRA_SCAN_WORKERS`, `HYDRA_SCAN_BIGFIRST`, `HYDRA_SCAN_LEAFSIM` and `env_int` (`scan.patch`); `perf_env_str` and `perf_env_int`. After wave 1, `grep -rn "HYDRA_PERF\|HYDRA_SCAN_\|HYDRA_TXN\|HYDRA_STMT\|GetEnvironmentVariable\|_dupenv_s" src` must find nothing (today it finds nothing).

Every timer and report: `PerfLap`, `g_perf_parts`, `perf_save_parts`, `chart_times`, the `PERF`/`PERFSAVE` stderr lines and `HYDRA_PERF_DUMP` (`write.patch`); the `graph destroy` timer line is fine to keep inside the harness only. All of `prof.patch` (`phase_prof.{h,cpp}`, every timer, and its unconditional `/Zi` in CMakeLists.txt; B1 adds symbols scoped to Release instead). All of `engine-probe-timers.patch`. All of `algo.patch` (dominance pruning was measured unsound).

Experiment-only code: `perf_exec`, `save_analysis_encoded`, `perf_encode_tempomap` and the `HYDRA_LEAN` worker-side tempo map (measured no gain); the `HYDRA_ORDER` sort of the batch (measured slower); 6a (`set_head_time` trimming in place, superseded by 6c); every `#ifdef PERF_VERIFY_6C` block and the `backends_check`/`recent_check_` members; the `FindFirstFileExW` switch and the leaf-skip simulation (not worth it, unsafe); the `CreateFileW`/`ReadFile` hash path (the gain was the buffer, not the API); `HYDRA_SCAN_WORKERS`. Every comment reading "PERF EXPERIMENT" or "PERF 6b/6c".

## Wave 0: the proof harness (one task, starts now)

### Task H1: committed comparison tools and the bench harness modes

**Goal:** The whole-library correctness checks the experiments ran by hand become committed tools: a database comparison script, the `--engine` and `--parse` modes in `hydra_bench`, the benchmark lock script, and a test that pins the 97-chart corpus's engine and parse digests as literals, so every wave 1 task can prove "byte-identical" the same way and the reviewer can re-run it.

**Files:**
- Create: `tools/compare_db.py` (the table comparison; `py` is system Python 3.14)
- Create: `tools/test_compare_db.py` (pytest: two tiny databases that differ only in `result_id` order compare clean; one changed payload byte is reported)
- Create: `tools/bench_run.ps1` (lifted from `docs/handoffs/2026-10-06-perf-exploration/notes/bench_run.ps1`, with the lock file and `bench_log.md` moved to `C:\Users\Patrick\.claude\hooks\state\bench\`, beside the build-slot locks, so the repo never collects logs)
- Create: `tests/song_digest.h` (header-only: the FNV hash, `row_hash` over a `store::PreparedRow`, `song_digest` over a `Song`; one owner, included by both `tools/bench.cpp` and the new test, which works because `hydra_bench` already has `tests` on its include path in CMakeLists.txt)
- Create: `tests/test_perf_digest.cpp` (pins the corpus digests)
- Modify: `tools/bench.cpp` (the `--engine` and `--parse` modes, and the `bench_main` dispatch for both)
- Modify: `CMakeLists.txt` (one line: the new test source in `hydra_tests`)
- Modify: `docs/development.md` (a short "Proving identical results" section that names the three tools and points at this plan's recipe)

**Acceptance Criteria:**
- `py tools\compare_db.py A.db B.db` prints one line per table in the order results, paths, path_refs, songmeta, dynamics, charts, meta, each "N rows compared, M differ", lists the first few differing keys, and exits 1 when any M is not 0. Columns are read with `PRAGMA table_info`, so the script needs no copy of the schema; `result_id` is left out of `results` and replaced in `path_refs` by its result's other columns; `charts` is compared row by row in `rowid` order.
- `py -m pytest tools\test_compare_db.py -q` passes: the result_id-only difference reports 0 differ; the planted payload byte reports 1 differ and exit 1.
- `hydra_bench --engine <folder> [--cache <db>] [--reps N] [--out <file>]` and `hydra_bench --parse <folder or list.txt> [--reps N] [--out <file>] [--nodyn]` work as the `engine_mode` and `parse_mode` hunks of `patches/engine.patch` and `patches/parse.patch` (`tools/bench.cpp`) describe, with the hash helpers moved into `tests/song_digest.h`. Running either twice on the corpus prints the same hash.
- `tests/test_perf_digest.cpp` holds two cases, "the corpus's prepared-row digest is pinned" and "the corpus's parse digest is pinned", each at Expert, Pro Drums, 2x Bass, cap 4, depth 4 scores, Path limit 10 ms (built through `app::Settings::to_analysis_settings`, the scratch_settings.h pattern), and a third at Hard, Pro off, 2x off for the parse digest. Each pins the digest literal from one run on main and prints the digest it got on failure, so a later change that legitimately moves results (with its stamp bump) can repin it on purpose.
- `tools/bench_run.ps1` takes `-Label` and `-Script`, logs to the hooks state folder, and refuses to run two timings at once (two calls from two PowerShell windows: the second waits or exits 3).
- `grep -n "PERF EXPERIMENT" tools tests` finds nothing.

**Verify:** `.\build_cpp.ps1 -Target hydra_bench; .\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -sf=*test_perf_digest*; py -m pytest tools\test_compare_db.py -q` all pass, and `.\build-cpp\Release\hydra_bench.exe --engine testdata\input` prints a hash equal to the one the test pins.

**Steps.** Start with the test for `compare_db.py`, red, then the script. Then lift the two harness hunks from the patches: `engine.patch`'s `tools/bench.cpp` block (`fnv`, `fnv_str`, `fnv_opt`, `row_hash`, `engine_mode`, the `--engine` dispatch) and `parse.patch`'s (`song_digest`, `parse_mode`, the `--parse` dispatch; its `--engine` block is the same code and is lifted once). Move the hash functions into `tests/song_digest.h`, so the test and the tool share them, and drop the "PERF EXPERIMENT (scratch only)" wording for a plain comment saying what each mode prints and that the test pins the corpus digest. Then write the pinned test: run the two modes on `testdata/input` once on main, pin what they print, and have the test compute the digest through the same header. Copy `bench_run.ps1` in with its paths changed; nothing else in it changes. Last, the development.md section. Commit after each step.

**Merge M0.** Precheck, one Sonnet derive-once review (`tests/song_digest.h` is the thing to check: one hash owner, no second copy in the tool), full suite by the main session, merge to main. Wave 1 forks from H1's tip as soon as the harness commit exists; it does not wait for this merge.

## Wave 1: the speedups, five worktrees at once

All five fork from H1's tip. The main session builds the baseline exe set from that same commit first (see the recipe). No two tasks own the same function. Two files are touched by two tasks each in disjoint places, and the join handles them as `docs/agents/integrate.md` says:

- `src/app/analysis.cpp`: W1 owns `run_batch` and its anonymous-namespace helpers (lines 580 to 752 on bc58282); S1 owns `stream_md5` and `discover_charts` (lines 89 to 124 and 398 to 557). Neither adds an include the other adds: W1 adds none, S1 adds any it needs at the end of the include block. Git merges disjoint hunks in one file cleanly.
- `tests/test_analysis.cpp` and `tests/test_store.cpp`: W1, S1 and B1 each append their own cases at the end. Two appended blocks merge badly in git, so the integrator rebuilds each file as the wave branch's version plus the other task's added cases (integrate.md step 3).

Everything else is one writer per file. `CMakeLists.txt` belongs to B1 alone in this wave (P1 adds no new test file, so it needs no CMake line). `src/core/model.h` and `.cpp` belong to G1, which is why the dynamics count lives in G1 and not P1 (see G1).

### Task W1: the batch writer

**Goal:** A whole-library batch saves its results about a third faster, with no change to what is stored. The save path compiles each SQL statement once and reuses it. The batch consumer saves up to 16 charts in one transaction, each in its own SAVEPOINT, closing the group when it reaches 16 or whenever no result is waiting, so a transaction is never open while the thread waits. A group whose COMMIT fails is rolled back and each of its charts is saved again alone, so only a chart whose own save fails is marked failed (D86.1, D71 item 6). While a batch runs the WAL checkpoint threshold is 10,000 pages; at batch end one `wal_checkpoint(TRUNCATE)` runs and the threshold returns to 1,000 (D86.3). Progress and result callbacks for a group go out after its COMMIT, so a reported chart is always on disk.

**Files:**
- Modify: `src/store/record_store.h`, `src/store/record_store.cpp` (the statement cache; `begin_save_group`, `commit_save_group`, `save_group_open`; the per-chart SAVEPOINT in `save_analysis`; a batch-writes guard that owns the two PRAGMA strings and the end checkpoint; the two constants `kSaveGroupSize = 16` and `kBatchWalAutocheckpointPages = 10000` with a comment naming D86; one test seam that makes the next group COMMIT fail the way a real one does)
- Modify: `src/app/analysis.cpp` (`run_batch` only: the deferred reports, the flush, the retry-alone path, the guard)
- Modify: `src/app/work_pool.h` (the idle hook: `consume`'s caller runs it when the line is empty before a wait and once at the end)
- Modify: `tests/test_store.cpp`, `tests/test_analysis.cpp` (appended cases), `tests/test_single_owner.cpp` (its own rows only)
- Read only, run as named: `tests/ui/uitest_batch_reports.cpp` scripts `batch-confirm`, `batch-pause-stop`, `batch-strip-workers`, `batch-done-strip`, `batch-open-failure`, `analyze-db-fails`

**Acceptance Criteria:**
- Fresh-database whole-library run, baseline against W1: `compare_db.py` reports 0 differ in every table (the experiment saw results 18,811, paths and path_refs 90,674, songmeta 18,811, dynamics 18,811, charts 19,436). Real-database `--redo` copies: 0 differ as well.
- `hydra_batch --redo` on a real-database copy, three interleaved pairs through the lock: W1's median is at least 25% under the baseline's (the experiment measured 11.6 s against 17.1 s pooled; the baseline drifts between sessions, so the comparison is against the same-session baseline, never a number from the notes).
- Test "a group commits before any of its charts is reported": a file store, a second `RecordStore` on the same file opened inside `on_result`, and that second connection already sees the chart's row when the callback fires.
- Test "cancel during a group writes nothing more and leaves no transaction open": a fake analyzer, cancel set after the consumer has seen some results; after `run_batch` returns, `store.counts()` equals the number of `on_result` calls, and a plain `save_analysis` on the same store then succeeds. The existing "run_batch: cancel stops running searches within seconds" still passes under 5 s.
- Test "a failed group COMMIT saves each chart alone, and only a chart whose own save fails is failed" (D86.1): three charts, the seam arms one group-commit failure, a trigger refuses `hyhash = 'boom'` (the `refuse_boom` pattern already in test_store.cpp); the run ends with two analyzed, one failed, the failure naming boom with the database sentence, and the store holding exactly the two rows.
- Test "the batch guard sets the checkpoint threshold and truncates the log at the end": on a file store, after the guard ends, `hydra.db-wal` is 0 bytes and the store's threshold reads 1,000 through a test-only getter (the threshold is per connection, so a second connection cannot read it).
- The `wal_checkpoint(TRUNCATE)` at the end of a whole-library run is timed once and the number is in the report. If it is over about a second, the task stops short of any display change and raises question 1 below.
- Statement handles are finalized before `sqlite3_close`; the cache is on for every store (no switch), and `close()` clears it. A write that fails inside a chart's SAVEPOINT where SQLite has already rolled the whole transaction back (`sqlite3_get_autocommit` true: SQLITE_FULL, IOERR, NOMEM) is treated as a lost group and takes the same retry-alone path.
- The store lock is held for a group. The brief lists every other `RecordStore` user and what it waits on: the UI thread's `AppState::reload_library`, `refresh_library_summaries`, `refresh_library_row`, the details panel's `get_record`, `load_stored_dynamics`, `charts_with_result`, the song-length job's `fill_song_length`, the single-song `save_analysis` in `AppState`, and `ReportJob`/`DmReportJob`'s `for_each_blob` (which already releases the lock between rows). Each waits at most one group: about 16 saves at 0.5 ms plus one COMMIT, the "near 8 ms" D86.2 accepted. The report says so with the measured per-chart save time.
- The six named `hydra_uitest` scripts pass. If none of them checks that the finished strip's analyzed count equals the library's Analyzed chip after a whole-corpus batch, one new script in `uitest_batch_reports.cpp` does.
- `git diff --stat <base>..HEAD` lists only the owned files.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -sf=*test_store.cpp*,*test_analysis.cpp* -tc="single-owner*"`; then `.\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --test batch-confirm --test batch-pause-stop --test batch-strip-workers --test batch-done-strip --test batch-open-failure --test analyze-db-fails`; then the recipe's fresh-database compare and the `--redo` A/B through the lock.

**Steps.** Red tests first for the four new cases. Then lift, in this order, from `patches/write.patch`: in `record_store.cpp` the `UseStmt` struct and `use_stmt`, the `ctl` method, the `close()` finalize loop, `begin_save_group` and `commit_save_group`, and every `prepare_write`-to-`use_stmt` change in `insert_dynamics`, `write_song_length`, `upsert_song`, `write_row` and `collect_orphan_paths`; in `record_store.h` the three group methods and the `stmt_cache_`, `group_open_`, `group_lost_` members; in `work_pool.h` the `run_work_pool_idle` body (keep the name `run_work_pool` with an idle parameter defaulting to nothing, rather than two templates); in `analysis.cpp` the `Deferred` struct, the `report` and `flush_group` lambdas and the consume-side grouping. Make the cache always on (drop `stmt_cache_on_` and `set_statement_cache`). Then change `flush_group` to D86.1: on a COMMIT failure, every deferred chart that was not already failed is saved again through `save_analysis` with no group open, and only a chart whose own save throws is recorded failed through `record_failure`; the deferred copies must keep what that save needs (the `Song` or its encoded tempo map, the prepared row, the dynamics entry, the length), so `Deferred` holds the whole `WorkResult`, not a trimmed copy. Then the guard: an RAII type in the store that runs `PRAGMA wal_autocheckpoint=10000` on construction and `PRAGMA wal_checkpoint(TRUNCATE)` then `PRAGMA wal_autocheckpoint=1000` on destruction, created at the top of `run_batch` after the empty-todo return, so an exception out of the pool still restores the setting. Delete every timer, every switch and the ordering code. Run the recipe. Commit after each step.

### Task G1: the score graph, the chord's note order, and the dynamics count

**Goal:** Graph building drops from about 13.6 s to about 2.7 s single-threaded over the library, and the kick dynamics count from about 3.8 s to about 0.5 s, with identical stored rows. Three changes: each SP deactivation edge keeps a start and end index into one shared list of backend rows instead of its own copy (6c); the chord gives its notes in base-score order without touching the heap, through one owner that `category_scores`, `Chord::notes` and `count_dynamics` all read (6b and the dynamics count); and one manifest line opts every exe into Windows' segment heap.

**Files:**
- Modify: `src/search/graph.h`, `src/search/graph.cpp` (6c), `src/search/engine.cpp` (6c: `Enum`, `create_deactivated_path`, `rebuild`)
- Modify: `src/core/model.h`, `src/core/model.cpp` (the one note-order owner: an array-backed `Chord::note_list(bool basesorted)`; `Chord::notes(bool)` becomes a copy of it)
- Modify: `src/core/scoring.cpp` (`category_scores` reads `note_list(true)`)
- Modify: `src/app/dynamics_breakdown.cpp` (`count_dynamics` reads `note_list(false)`)
- Modify: `src/app/long_paths.manifest` (the segment-heap `windowsSettings` block)
- Modify: `tests/test_model.cpp`, `tests/test_search.cpp`, `tests/test_dynamics_breakdown.cpp`, `tests/test_single_owner.cpp` (its own rows only)

**Acceptance Criteria:**
- `hydra_bench --engine "C:\Clone Hero"` prints the same hash for baseline and G1, and the per-chart files are identical (18,811 charts). Fresh-database `compare_db.py`: 0 differ in every table. The pinned corpus digests in `tests/test_perf_digest.cpp` still pass unchanged.
- `--engine` single-thread graph time over the library is at most a third of the baseline's in the same session (the experiment: 13.6 s to 2.7 s), and the harness's parse time is unchanged within noise.
- Test in test_model.cpp, "note_list(true) orders a tied chord as notes(true) always has": a chord of kick (50), yellow cymbal (65), blue accent (100), green (50) lists Kick, Green, Yellow, Blue, pinned as literal lanes; `notes(true)` on the same chord gives the same lanes.
- Test in test_dynamics_breakdown.cpp: the existing count cases pass, and one new case pins the counts of one corpus chart as literals from a run on main (the WGFA fixture already has "36 ghost kicks" pinned; reuse that case's chart if it covers every row).
- Test in test_search.cpp, "a deactivation edge's rows are a range of the graph's rows": for `corpus::first_chart_with_notes()`, pin from one run on main the total of `all_backends().size()` and the number of deactivation edges, and check every edge's `backend_begin <= backend_end <= all_backends().size()`.
- No `#ifdef PERF_VERIFY_6C`, no `recent_backends_`, no `backends` vector on `ScoreGraphEdge`, and no comment reading "PERF" remains. `grep -n "PERF" src/search src/core/scoring.cpp src/core/model.cpp` finds nothing.
- `tests/test_single_owner.cpp` gains one row: "In what order does a chord list its notes?" with owner `Chord::note_list` in `src/core/model.cpp`, a pattern that would flag a second `stable_sort` or insertion sort on `basescore()` in `src/`, two must-match lines (the removed `scoring.cpp` sort and the old `Chord::notes` sort) and one must-not-match line.
- `git diff --stat <base>..HEAD` lists only the owned files.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -sf=*test_model.cpp*,*test_search.cpp*,*test_dynamics_breakdown*,*test_perf_digest* -tc="single-owner*"`; then `.\build_cpp.ps1 -Target hydra_bench` and the recipe's `--engine` A/B and fresh-database compare through the lock.

**Steps.** Red tests first. For 6c lift, from `patches/engine.patch`: in `graph.h` the `backend_begin`/`backend_end` fields on `ScoreGraphEdge`, `all_backends()`, `edge_backends()`, `tail_backends_`, `all_backends_` and `recent_lo_`; in `graph.cpp` the end of `build()` (closing open ranges and filling `tail_backends_`), `store_new_backend`, `edge_backends`, `set_head_time` and `add_deact_edge`; in `engine.cpp` the `all_backends` pointer on `Enum` set in `enumerate`, the loop in `create_deactivated_path` that reads a range and measures each row against the edge's dest with `offset_from_sp_end`, and the extra `graph` parameter of `rebuild`. Leave every `PERF_VERIFY_6C` block out and do not lift 6a. For 6b do not lift `sorted_notes` into scoring.cpp as the patch has it: that would be a second copy of the order rule `Chord::notes(true)` owns (`std::stable_sort` on `basescore()`, model.cpp line 183). Instead the owner moves into `Chord`: an array-backed `note_list(bool basesorted)` with the stable insertion sort from the patch's `sorted_notes` (lane order first, then a stable sort on `basescore()`), and `notes(bool)` copies it into a vector. Then `category_scores` takes `note_list(true)` and `count_dynamics` iterates `note_list(false)` (order does not matter for a count, and the lane list stays in `Chord`, so the patch's `kLanes` array is not lifted either). Last the manifest hunk. Run the recipe. Commit after each step.

### Task P1: lean chart and MIDI readers, with the old readers deleted

**Goal:** Parsing drops from about 15 s to about 9 s single-threaded over the library, and every chart, including 41 crafted error files, parses to exactly what the old readers produced, with the same exception type and message where they threw. The old code paths go: `.chart` sections other than [Song] and [SyncTrack] are no longer stored as heavy per-tick entries, and a `.mid` is no longer decoded in full before the drum track is picked.

**Files:**
- Modify: `src/parse/song.cpp` (the `.chart` reader: `load_sections` rewritten as the lean reader; one line classification `classify_chart_line`; one `optype`; `word_stoi`, `word_stoll` and `try_parse_int` gain the fast path inside themselves; `load_songbytes_mid` builds the note filter from the existing pitch owners and always uses the lean decode)
- Modify: `src/parse/midi.h`, `src/parse/midi.cpp` (`walk_track` as the one track walker; `parse_track` reimplemented over it keeping everything; `MidiFile::lean` keeping only what the filter names; `MidiLeanFilter`)
- Create: `testdata/parse_edge/gen_edge.py` (a deterministic generator for the 41 crafted files), `testdata/parse_edge/list.txt`, `testdata/parse_edge/expected_expert.tsv`, `testdata/parse_edge/expected_hard.tsv`, and the generated files
- Modify: `tests/test_song.cpp`, `tests/test_midi.cpp` (appended cases), `tests/test_single_owner.cpp` (its own rows only)
- Read, not changed: `tests/song_digest.h` (H1's digest, the one the new test reads)

**Acceptance Criteria:**
- `hydra_bench --parse "C:\Clone Hero"` prints the same hash for baseline and P1 and the per-chart lines are identical (the experiment compared 18,869 charts with 58 failures, all "no notes"), at the GUI's settings and again at Hard, Pro Drums off, 2x Bass off (5,744 failures, same text). `--engine` row hash identical. Fresh-database `compare_db.py`: 0 differ. The pinned corpus digests still pass.
- `--parse` single-thread time over the library is at most two thirds of the baseline's in the same session (the experiment: 15.0 s to about 9 s).
- The 41 crafted files exist under `testdata/parse_edge/`, generated by `gen_edge.py` so the generator is the record of what each file tests, covering the notes' list: bad and overflowing numbers in skipped, drum, events and song sections; odd keys; duplicate and unterminated sections; unsorted ticks; CRLF; disco markers; missing [Song] and Resolution; a bad header; a MIDI with two drum tracks, late names, an oversize meta in another track and in the drum track, format 0, SMPTE, truncation, running status after a meta, an unknown chunk, a bad status byte and a zero tempo.
- `expected_expert.tsv` and `expected_hard.tsv` were captured from the baseline exe (`hydra_bench --parse testdata\parse_edge\list.txt --out`) before any parser change, and the brief keeps that command and the baseline's commit hash in the report. Each line holds the file, the digest, and for a failure the exception type and message text.
- Test in test_song.cpp, "the crafted edge files parse as the old readers did": for every line of each expected file, parse with `load_songpath_with_notes` at that file's settings and compare the digest through `tests/song_digest.h`, or the thrown exception's type name and `what()` against the pinned text. The test prints the offending file on failure.
- The existing `.chart: malformed lines keep their handling`, `.chart: a timing line that can't measure time is refused`, `.mid: a timing line that can't measure time`, every `midi:` case in test_midi.cpp (running status, metas, track names, SMPTE, the 1,000,000-byte cap) and `analyze_chart_file reads a drum track whose first name is unrecognized` pass unchanged.
- One `optype` in `ChartParser`, one line classifier, one track walker in midi.cpp, and one integer fast path. `grep -n "load_sections_lean\|perf_exp_on\|lean_stoi\|lean_stoll\|lean_try_parse_int\|HYDRA_PERF" src` finds nothing. The MIDI note filter is built from `is_handled_note` and `is_midi_marker_pitch` (song.cpp), never from a second pitch list.
- `git diff --stat <base>..HEAD` lists only the owned files.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -sf=*test_song.cpp*,*test_midi.cpp*,*test_perf_digest* -tc="single-owner*"`; then `.\build_cpp.ps1 -Target hydra_bench` and the recipe's `--parse` A/B at both settings sets, the `--engine` A/B and the fresh-database compare through the lock.

**Steps.** First, before touching a parser, write `gen_edge.py` and `list.txt`, generate the files, and capture both expected files with the baseline exe; commit them. Red: the new test fails to compile (no digest include yet) or passes trivially; make it read the expected files and pass on the unchanged parser, then it is the guard for every later step. Then lift from `patches/parse.patch`: in `midi.cpp` the anonymous-namespace `walk_track` and `track_name_of` and the `MidiFile::lean` body; in `midi.h` `MidiLeanFilter`, the `lean` declaration and the private default constructor; then rewrite `parse_track` as a `walk_track` caller that keeps every message (so the two readers share one walk and one set of length checks). In `song.cpp` lift `fast_leading_int`, `classify_chart_line`, `LineKind`, `ChartLine`, the `ChartLine` `optype`, the templated `push_timestamp`, `load_sections_lean` and the lean branch at the end of `ChartParser::parse`; then delete the old `load_sections` body in favour of the lean one, delete the `ChartDataEntry` `optype` and the per-tick `tick_data`/`tick_order` map for the drum section, keep `ChartDataEntry` only as what [Song] and [SyncTrack] store and build it from `classify_chart_line` so the line rule has one owner, and fold the fast paths into `word_stoi`, `word_stoll` and `try_parse_int` themselves (each becomes "fast path, else the original std call") instead of adding `lean_` twins. In `load_songbytes_mid`, build `MidiLeanFilter` from `difficulty_base_pitch`, the difficulty's `kick2x_pitch`, `is_handled_note` and `is_midi_marker_pitch`, and delete the full-decode line. Run the edge test after every step, then the recipe. Commit after each step.

### Task S1: the library scan

**Goal:** A full scan of the library halves (about 4.2 s to about 2.1 s) and a rescan with a warm cache drops from about 0.8 s to about 0.2 s, with the `charts` table byte-identical and in the same order. Three changes: folders are listed on several threads and then replayed in the serial walk's exact order, with the folder count still reported live from the calling thread; each hashing thread reuses one 1 MB read buffer instead of allocating and zero-filling a fresh one per file; and the biggest files are hashed first so a 1 GB archive found last no longer finishes alone, while results still land by walk position.

**Files:**
- Modify: `src/app/analysis.cpp` (`stream_md5` and `discover_charts` only, plus `PendingChart.size` and `pending_chart_of`; W1 owns `run_batch` in the same file)
- Modify: `tests/test_analysis.cpp` (appended cases), `tests/test_single_owner.cpp` (its own rows only)

**Acceptance Criteria:**
- `hydra_bench --scan "C:\Clone Hero" --db` for baseline and S1, then `compare_db.py`: `charts` 19,436 rows compared in insert order, 0 differ, on a fresh database and again on a rescan of the same database. The library has 542 md5s with more than one copy, so D63's first-listed-copy rule is exercised.
- Full-scan time, five interleaved pairs through the lock: S1's median at most 60% of the baseline's in the same session; rescan median at most 40%.
- The parallel walk's thread count is `batch_worker_count()` (the existing owner, 8 on this machine), not a new literal. The calling thread is one of the walkers and reports `on_folders(visited.size())` each time it finishes a folder, so progress keeps coming from the calling thread, with no timer and no new number. The serial walk is gone; the replay reproduces its stack order (roots in order, each popped folder's charts, then its subfolders pushed in listing order), and `errors` come out in that same order.
- Test "the scan's folder count is reported while the walk runs, from the calling thread": on `testdata/input`, `on_folders` is called more than once, each call comes from the thread that called `discover_charts` (compare `std::this_thread::get_id()`), and the last value equals the folder total.
- Test "the scan's items come out in walk order whatever order they were hashed in": the existing "discover_charts output matches the checked-in scan snapshot" case already pins the order; keep it passing and add one case that plants a large file so it is hashed first and checks the snapshot order still holds.
- `stream_md5` reads through one `thread_local` 1 MB buffer; no per-file `std::vector<uint8_t> buf(1 << 20)` remains.
- No `env_int`, no `FindFirstFileExW`, no `HYDRA_SCAN_*`, no leaf-skip code, no `stream_md5_win32`.
- `git diff --stat <base>..HEAD` lists only the owned files.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -sf=*test_analysis.cpp* -tc="single-owner*"`; then `.\build_cpp.ps1 -Target hydra_bench` and the recipe's `--scan` A/B (fresh and rescan) through the lock.

**Steps.** Red tests first. Lift from `patches/scan.patch` (`src/app/analysis.cpp`): `scan_folder`, `WalkNode` and `parallel_walk`, then make `parallel_walk` the only enumerate stage (delete the serial `while (!unexplored.empty())` loop and the `leafsim` block), give it the calling thread as worker 0 and the `on_folders` callback, and take its thread count from `batch_worker_count()`. Lift the `thread_local` reused buffer from the `HYDRA_SCAN_REUSEBUF` branch of `stream_md5` and delete the `fresh` vector and the `hashio` branch. Lift `PendingChart.size`, its assignment in `pending_chart_of`, and the `order` permutation in stage 2 (the `stable_sort` by size, descending), dropping its switch. Keep `batch_worker_count()` as the read pool size. Run the recipe. Commit after each step.

### Task B1: build settings

**Goal:** The build runs projects in parallel (D86.5), Release binaries get link-time optimization when the toolchain supports it, Release builds write symbol files beside the exes without shipping them, and SQLite is built with two options that cut its own CPU slightly. Stored results stay byte-identical, proven, or the optimization does not ship.

**Files:**
- Modify: `build_cpp.ps1` (`--parallel` with no count on the `cmake --build` line)
- Modify: `CMakeLists.txt` (`include(CheckIPOSupported)` and `check_ipo_supported` after `project()` and before the first target, setting `CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE` only when supported; `/Zi` for C and C++ and `/DEBUG /OPT:REF /OPT:ICF` for the linker, both scoped to the Release configuration; `target_compile_definitions(sqlite3 PRIVATE SQLITE_DEFAULT_MEMSTATUS=0 SQLITE_OMIT_SHARED_CACHE)`)
- Modify: `installer/build_installer.ps1` (one more guard after the stage: a `*.pdb` under the stage folder throws)
- Modify: `docs/development.md` (what the Release build now does and why `/fp:fast`, `/fp:contract`, `/arch:AVX2` and turning off `/GS` stay out)
- Modify: `tests/test_store.cpp` (one appended case: `sqlite3_compileoption_used` answers true for `DEFAULT_MEMSTATUS=0` and `OMIT_SHARED_CACHE`, beside the existing "the vendored SQLite is built without FTS5" case)

**Acceptance Criteria:**
- `cmake --build` in `build_cpp.ps1` carries `--parallel` with no number (D86.5). A cold build of `hydra_tests` still passes.
- Configure prints that IPO is supported and on for Release; `hydra_batch.exe` built with it, baseline against B1, passes the recipe: fresh-database `compare_db.py` 0 differ in every table; `--engine` and `--parse` hashes identical. If any difference appears, IPO comes out of the change and the task reports the difference; nothing else in this task is conditional on it.
- `build-cpp\Release\` holds `Hydra.pdb`, `hydra_batch.pdb` and friends after a Release build; `cmake --install` into a scratch prefix stages no `.pdb` (the `install(TARGETS ... RUNTIME)` rules do not include them), and the new installer guard proves it on a `-SkipBuild` run against the dev build. CPack's zip, built from the same install rules, has none either (list the zip).
- The SQLite test case passes. `SQLITE_DEFAULT_WAL_SYNCHRONOUS` is not added: `record_store.cpp` sets `PRAGMA synchronous=NORMAL` on every open, so the compile default never applies, and a define that changes nothing would be a second copy of the rule the store owns.
- None of `/fp:fast`, `/fp:contract`, `/arch:AVX2`, `/GS-` appears anywhere in the repo.
- Warm and cold build times before and after, one run each, are in the report, with the link time of `Hydra.exe` named (IPO makes links slower).
- `git diff --stat <base>..HEAD` lists only the owned files.

**Verify:** `.\build_cpp.ps1 -Configure -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -sf=*test_store.cpp* -tc="*SQLite*"`; then `.\build_cpp.ps1 -Target hydra_batch; .\build_cpp.ps1 -Target hydra_bench` and the recipe's fresh-database compare and `--engine`/`--parse` A/B through the lock; then `.\installer\build_installer.ps1 -SkipBuild` far enough to pass the stage guards (Inno Setup may be absent; the guards run before `ISCC`).

**Steps.** `build_cpp.ps1` first (one line). Then CMake: the IPO check and the Release-scoped `/Zi` and link options (the unconditional lines in `patches/prof.patch`'s `CMakeLists.txt` hunk are the model, with `$<$<CONFIG:Release>:...>` generator expressions added), then the two SQLite defines. Note that the vendored ogg and opus subprojects sit under `CMAKE_POLICY_VERSION_MINIMUM 3.5`; if configure warns that IPO is ignored for them, that is fine and goes in the report. Build, then run the recipe before anything else, because the result decides whether IPO stays. Then the installer guard and the docs. Commit after each step.

**Merge M1.** The five branches join on `claude/perf-w1` through `docs/agents/integrate.md` (W1 and S1 share `analysis.cpp` in disjoint functions; three tasks append to `test_analysis.cpp` and `test_store.cpp`). The integrator builds the join once and runs the scan and docs tests. Then the precheck, one Sonnet derive-once review of the join, one sweep fix round if needed, the full suite by the main session (`hydra_tests` and `hydra_uitest --all --jobs 4`), the recipe once more on the joined build against the bc58282 baseline, and merge to main.

## Wave 2: the combined measurement and the mimalloc trial (fork from M1's join tip)

### Task T1: one combined whole-library timing

**Goal:** One set of numbers for everything together, which no experiment measured: a real-database `--redo` run, a full and a cached scan, and the single-thread parse, graph and search split, each as an interleaved A/B against bc58282 through the lock, plus the identical-results check on the joined build. The result goes to the user as a plain report with per-phase numbers.

**Files:**
- Create: `docs/handoffs/2026-10-0X-perf-speedups-results.md` (dated the day it runs; the only file this task writes)

**Acceptance Criteria:**
- Baseline exe set built from bc58282 by the main session (`baseline-bc58282\`), task set from the M1 join; both with the same settings ini.
- A real-database copy taken with the backup API as the recipe says, copied fresh for every run; `hydra_batch --redo` A, B, A, B, A, B through the lock; medians and all six walls reported with the compilers-busy counts.
- `hydra_bench --scan "C:\Clone Hero" --db` A/B, fresh and rescan, three pairs; the walk and read-stage split reported.
- `hydra_bench --engine` and `--parse` A/B on the whole library, one pair each (single-threaded, several minutes): parse, graph, analyze and dynamics times reported. The baseline for these two is the M0 build (main after H1), because bc58282 has no harness; the report says so.
- Fresh-database `compare_db.py` between the bc58282 baseline and the joined build: 0 differ in every table, stated with the row counts.
- Peak memory of the `--redo` run (peak working set, read from the process as the experiments' runner did) reported for A and B.
- The report says what the user will notice: the batch wall time before and after on their library, in seconds, and the scan time before and after.

**Verify:** the report file exists, every number in it names its run count and compilers-busy count, and the compare line reads 0 differ.

**Steps.** No code. Write the timing scripts in the scratchpad, run them through the lock one at a time, and write the report. If the lock stays busy, do other sections first.

### Task M1: the mimalloc trial (D86.4; a download the main session must approve first)

**Goal:** Measure Microsoft's mimalloc allocator against the shipped segment heap on the real workload, so the user can decide with numbers whether to add it as a third-party dependency. Nothing from this task merges; it ends in a report.

**Files:**
- A detached throwaway worktree of the M1 join tip under `.claude\worktrees\perf-mimalloc`, never committed, removed by the main session afterwards
- Create: `docs/handoffs/2026-10-0X-mimalloc-trial.md` (the report; the only file that lands in the repo, via the main session)

**Acceptance Criteria:**
- Before downloading anything, the agent reports to the main session the exact release file (name, version, the `github.com/microsoft/mimalloc/releases` URL and the size the page shows) and waits for a yes. The main session takes it to the user. Nothing is downloaded without that.
- The route is named up front: our build is `/MD`, and mimalloc's documented Windows route for a dynamic CRT is the override DLL (`mimalloc-override.dll` plus `mimalloc-redirect.dll` beside the exe, with the exe linked against the override import library so the DLL loads first). The static route needs `/MT`, which we do not use, so it is not tried.
- `hydra_batch` and `hydra_bench` built in the worktree with the override linked and the two DLLs beside them; `MIMALLOC_VERBOSE=1` output confirms the redirect is active (quoted in the report).
- `hydra_batch --redo` on a real-database copy, three interleaved pairs against the same worktree's build without mimalloc, through the lock; `--engine` one pair. Medians, peak working set, and `compare_db.py` 0 differ.
- The report names what shipping would change: a `third_party/mimalloc` source tree and its MIT licence in the third-party list, a CMake target and `install(FILES)` for the two DLLs so `cmake --install` stages them, the installer picking them up from the stage, and the installer's "no user data leaked" and "no repo path" guards still passing. It ends with "shipping is the user's call" and no recommendation stronger than the numbers support.

**Verify:** the report exists with the approval exchange quoted, the redirect confirmation, the A/B medians and the compare line.

**Steps.** Ask first. Then download into the worktree only, build mimalloc with its own CMake (shared library, override on), link, confirm the redirect, measure through the lock, write the report, and leave the worktree for the main session to remove.

## The user's step: time a first-ever scan (D86.6)

This needs no agent and can happen any time after the next reboot, before Hydra is opened. It answers whether a user's very first scan is held up by opening files (a cold disk or Defender on first open), which none of the warm-cache numbers show. The one command, run in PowerShell, times a scan into a scratch database and keeps the walk/read split that `hydra_bench --scan` prints:

```
Measure-Command { & "C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_bench.exe" --scan "C:\Clone Hero" --db "$env:TEMP\hydra-firstscan.db" | Tee-Object -FilePath "$env:TEMP\hydra-firstscan.txt" } | Select-Object TotalSeconds
```

Before the reboot the main session builds the exe with `.\build_cpp.ps1 -Target hydra_bench` from main, so the command runs the current scan code. Run it once; a second run would be warm. Then paste `$env:TEMP\hydra-firstscan.txt` and the seconds into chat. Reading the result: the warm full scan on this machine is about 4 s (about 2 s after S1). If the first-ever scan is within a few seconds of that, first opens do not dominate and the scan plan stands with no more scan work planned. If it is far above it (the other session's 44 s for the song.ini reads alone would put it well over 20 s) with the read stage carrying the time, then the next scan work should go after per-file first-open cost (fewer opens per chart, or guidance on a Defender exclusion for the library folder) rather than walk or hash speed, and that is a new plan.

## Rough time

From earlier phases, an implementer takes 12 to 20 minutes and a merge 30 to 40. H1 is about 20 minutes plus its review. Wave 1 runs five implementers at once, about 40 minutes to a joined branch, because W1 and P1 are the heavy ones (each carries a whole-library correctness run of a few minutes through the lock, and the five tasks' library runs serialize behind that lock). M1's join, review, suite and recipe: about 45 minutes. T1 and the mimalloc trial run in parallel after that, about 40 minutes, their timings serialized by the lock. About three hours of wall clock in all, plus whatever the lock queue adds.

## What the sources disagreed on, and how this plan resolves it

The compiler research handoff recommends `SQLITE_DEFAULT_WAL_SYNCHRONOUS=1`, but `record_store.cpp` already sets `PRAGMA synchronous=NORMAL` on every open, so the define changes nothing; B1 leaves it out and says why.

The report's "decisions needed" item 1 described a failed group COMMIT as failing every chart in the group, and `write.patch` implements exactly that. D86.1 overrides it: the group rolls back and each chart is saved again alone. W1 changes the patch's `flush_group` accordingly, and the fault-injection test pins the decided behaviour.

The report measured the 10,000-page checkpoint as a setting left on for the whole process; D86.3 narrows it to the batch and adds the end `wal_checkpoint(TRUNCATE)`, whose cost the experiments never measured (a 100,000-page run once paid 3.6 s at close). W1 measures it and stops for question 1 if it is large.

The scan experiment's parallel walk used a thread count from a switch (8 measured best); the plan takes it from the existing owner `batch_worker_count()`, which gives 8 on this machine, so no new number needs a decision.

The task brief for this plan lists the dynamics count under "Parse". `parse.patch` makes it read the chord's five lanes in a lane list of its own, which would be a second copy of the order `Chord::notes` owns in `src/core/model.cpp`; the one-owner form lives in `Chord`, whose files G1 holds for 6b, so the dynamics count moves to G1. P1's `--parse` digest still covers the dynamics blob, and the join proves the two together.

The notes' hashes (`8d17f958172bd6f4`, `937d9aed20f59dff`) came from scratch harness code that no longer exists; the committed harness is compared A against B from one build, never against those numbers.

The handoff notes that the preamble's "three to six minute" cold build did not reproduce on an idle machine (83 s, 60 s with `--parallel`); the preamble's figure stands for a busy machine and nothing here changes it.

## Questions for the user

Nothing below is decided here; each is a display change beyond D86, so it waits for an answer (rule 3).

1. **If the batch-end checkpoint takes long.** D86.3's `wal_checkpoint(TRUNCATE)` at batch end forces every result to disk. W1 measures it on the whole library. If it takes more than about a second, the progress strip would sit at its last count with no sign anything is happening until the checkpoint ends. Options: accept the pause; or show a short "Saving..." state in the strip (new text, so it needs your words); or run the checkpoint after the strip reports finished. The task stops short of any of these and reports the number; the recommendation depends on it.

2. **Answered 2026-10-06 (D86 item 7): accept the bursts.** The command-line tool prints one line per chart as results are reported. Under group commit a line prints when its group commits, so up to 16 lines appear at once, about every 8 ms (the same lag D86.2 accepted for the progress strip). The text of each line does not change. Recommendation: accept it, since a report must follow its commit (a line before the commit could name a chart whose group then fails). The alternative is to print a provisional line per chart and a correction on a failed group, which is more text and a new rule.
