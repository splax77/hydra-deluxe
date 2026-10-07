# Handoff: storage audit of hydra.db (2026-10-06)

Written 2026-10-06, late evening. No code changed. Nothing was committed. Main was at `f3882e7` when this was written. Another session moved main forward while this one ran, so check `git log` before building on anything here.

## Read this first

The user asked three things, in order:

1. How much disk does the database use? What is stored, and how much of it is needed? Can it be stored more compactly? Is SQLite the right database?
2. A full audit of the storage format, taking nothing for granted. What really has to be stored, and what can be calculated when a song is opened?
3. Why does `hydra_replay` reinvent analysis when the engine already knows how?

The answers are below. The full agent reports and every measurement script sit in [2026-10-06-storage-audit/](2026-10-06-storage-audit/). The open questions at the end are waiting on the user. Nothing should be built until they answer.

## 1. Disk use and what's in it

All measurements were taken on a copy of the installed database at `C:\Program Files\Hydra\hydra.db`, never the live file. It takes 338 MB on disk: 292 MB of database plus a 46 MB write-ahead log (WAL), which is never shrunk back. Python's SQLite has no `dbstat`, so I walked the file's page tree myself (`scripts/main/walk.py`). Every page was accounted for, and only 35 were free.

| What | Size | Share |
|---|---|---|
| Path nodes (`paths` + its index) | 202 MB | 69% |
| Tempo maps, names, lengths (`songmeta`) | 38 MB | 13% |
| Path links (`path_refs` + 2 indexes) | 25 MB | 9% |
| Results summary rows (`results`) | 14 MB | 5% |
| Chart library list (`charts`) | 9 MB | 3% |
| Kick dynamics (`dynamics`) | 4 MB | 1.5% |

The path nodes hold 143 MB of raw bytes. I decoded every one (`scripts/main/a3.py`). Backend squeeze rows are 63% of it: 2.6 million rows, about six per activation. The rest is the activation head (14%), the SP-end history (9%), bank arrivals (7%), skipped fills (4%) and SqIns (2%).

The share-once scheme from ADR 0009 has never shared anything. Each of the 91,196 path nodes is referenced exactly once, and each chart has exactly one results row. The scheme still costs about 37 MB, mostly hex-text keys that average 89 bytes and are copied into four structures.

Rows nothing can read come to about 4.5 MB: 295 results rows from old versions (212 of them from 1.6.2), and 288 chart hashes that are no longer in the library. Nothing in the code ever deletes them. See `notes/other-tables-and-scheme.md`.

## 2. Compact storage, measured

These numbers come from a random sample of 3,000 rows per column, scaled up to the full column (`scripts/main/a6.py`). The dictionary was trained on different rows than it was tested on. A full-library run of the same test would take over 20 minutes; the sample took 66 seconds.

| Change | Path nodes (145 MB today) | Tempo maps (30 MB) | Structure blobs (8 MB) |
|---|---|---|---|
| zlib level 6 (miniz is already linked) | ~49 MB | ~13 MB | ~3.4 MB |
| zstd level 3 | ~52 MB | ~13 MB | ~3.6 MB |
| zstd level 19 plus a trained 112 KB dictionary | ~37 MB | ~10 MB | ~2.6 MB |
| Tighter number encoding, no compression (`a5.py`) | ~72 MB | ~17 MB | not measured |

The tighter encoding is lossless. It uses variable-length integers, where small numbers take one byte, and stores the gaps between ticks instead of full ticks. Python's dictionary speeds look slow only because it reloads the dictionary for every row; C++ would load it once. Combining tight encoding with compression was not tested.

Done together, these changes plus raw 16-byte hashes and a WAL size limit would take 338 MB to roughly 100 MB. That total is an estimate.

## 3. Is SQLite the right database?

Yes. Two web-research agents covered this (`notes/web-research-database-choice.md`, `notes/web-research-compact-storage.md`). The only side-by-side size test found was redb's own benchmark. On size, SQLite came in at 557 MB; LMDB was 1.26 GB and redb 1.69 GB. Only RocksDB was smaller, at 455 MB, because it compresses by default. Compressing our own blobs gets the same win.

The rest were ruled out. DuckDB is the wrong shape for blobs. LMDB and libmdbx have no compression and no sorting. Berkeley DB is AGPL, ObjectBox has a proprietary core, Realm is being wound down, sled is beta, and Turso isn't at 1.0. sqlite_zstd_vfs has no WAL and no Windows support. ZIPVFS is paid.

The batch's single writer thread is a known bottleneck. That's about how the thread is used (see the perf exploration), not the engine.

## 4. The audit: what has to be stored

**The key fact: the engine re-analyzes a chart faster than most people would notice, and gets the same answer every time.** Almost everything in the database is a saved copy of the engine's output.

The timing agent ran the engine on 400 random charts plus the slowest-parsing ones (`scripts/timing/engine.json`, `engine_stats.py`):

| | Parse | Whole analysis (parse + graph + search) |
|---|---|---|
| Median | 1.0 ms | 1.8 ms |
| 99th percentile | 7 ms | 12.8 ms |
| Slowest | 229 ms | 294 ms |

The slowest charts were Nirvana's "Endless, Nameless Setlist" (.sng and .chart copies, 294 and 291 ms) and blink-182's Discography (202 ms). Parsing the whole library typically took 0.49 ms per chart, slowest 220 ms (`parse_stats.py`).

My own 4-chart test (`scripts/main/t2.py`) timed a full `hydra_batch --redo` against loading the stored record with `hydra_replay dump --no-analyze`. Both timings include starting the program. The load timings also include writing JSON, up to 2 MB.

| Chart | Full analysis + save | Load stored record |
|---|---|---|
| Wings of a Butterfly (median) | 14 ms | 21 ms |
| Hail The Sun – Discography | 158 ms | 74 ms |
| Rise Against – Discography | 175 ms | 90 ms |
| blink-182 – Discography (biggest record) | 273 ms | 103 ms |

The timing agent re-analyzed 74 charts, including the biggest, into a scratch copy. The result matched the stored rows exactly, in every table. All library chart files exist. In a sample of 1,000, none had changed since the scan.

**So what must be stored is only what the chart file can't give at the moment a song opens.** That's about 15–20 MB:

- **The `charts` library list.** It holds names, file paths, and the size and date pairs (`sig`) a rescan uses to skip unchanged files. The library shows and sorts these without opening anything.
- **One summary row per chart and settings.** That's the `results` row minus its blob: score, stars, counts, hardest timing, best path text, the settings key, and the version and rules stamps. The library sorts and filters on these. Getting them any other way means analyzing the whole library.
- **`meta`**: three rows.

**Everything else can come from the engine's normal analysis (`analyze_chart`) when the song opens, with no second derivation:**

- All path nodes, path links and structure blobs (227 MB).
- `songmeta` (38 MB). The tempo map comes from the parse. Today it exists only so a click shows ms and measures before any parse, and so a record still displays if its chart file is gone. Its names are byte-identical copies of the `charts` names: 0 of 19,099 rows differ.
- `dynamics` (4 MB). On a miss the Dynamics tab already parses and counts through the same owner.

The database would go from 292 MB to roughly 20 MB.

**What changes for the user. These are the pending decisions:**

1. Opening a song runs the analysis: about 2 ms typical, about 0.3 s for the slowest chart. Today a click reads the database, 20–100 ms.
2. A chart whose file is missing or edited can't show its details. Its row and summary stay.
3. Reports and DMBot read every chart's paths today. They'd analyze the whole library each run, about 10 seconds on all cores (see the perf exploration).
4. What happens when a freshly computed summary disagrees with the stored row: mark it Stale, or replace it.

## 5. If the user would rather keep storing details: the trim list

From `notes/record-content-inventory.md` and `notes/other-tables-and-scheme.md`. Each item names its owner function so nothing becomes a second derivation.

- **Dead fields.** `frontend_points` (1.7 MB) has no reader. `sp_cap_converged` is always true.
- **Stored twice.**
  - `ms_limit` and `sp_cap` sit in both the blob and the key columns.
  - `notecount` is stored on every root (79,933 times) and again as a column.
  - The `songmeta` names duplicate `charts`.
  - `charts.folder` can be derived from `path` plus the scan root.
- **A pure cache of one owner function.** The transfer scales (7.9 MB) come from `frontend_transfer_scales(act, timing)`, and `get_record` already has the timing.
- **Backend rows** (90 MB). Their chord and offset (47 MB) follow from the row tick, the Song and the stored deact tick. Their points need the combo at that chord, which means a graph walk.
- **The scheme.** Either put nodes inline in the structure blob (removes 37 MB, the garbage collection and a query per load), or keep sharing with integer ids and raw 16-byte hashes in WITHOUT ROWID tables (about 6 MB). `path_refs_by_node` serves only the garbage collection, which could read by `result_id` instead.
- **Housekeeping.**
  - Set `journal_size_limit`, and truncate the WAL at batch end. The perf plan's D86 already has the checkpoint.
  - Decide whether anything should delete rows for charts that left the library. Today nothing does.

## 6. hydra_replay

`dump` uses the real engine: it reads the stored record or calls `analyze_chart`.

`target` uses the real engine in a special mode. `search_target` (src/search/pather.cpp:220–250) pins only *where* activations start (engine.cpp:1771–1786). It sets the score band to keep everything (`kKeepEveryPathBand`, pather.cpp:236–238), so every way of *ending* each activation survives. The frontier grows exponentially (`scripts/main/t3.py`): pinning the first 5, 10, 20, 40 and 60 of blink's 522 activations returned 2, 3, 6, 48 and 192 paths. The full list ran over 3 minutes of CPU without finishing. Hail The Sun and Rise Against timed out past 20 s. The comment at tools/replay.cpp:648 ("Each run is milliseconds") is wrong for long charts. Anything using `target` on a long chart, including the FC video workflow, will hang the same way.

`score` is a real second derivation. src/core/replay.cpp walks the chart chord by chord and restates the window rules (replay.h:7–17). It only borrows `category_scores`, `backend_value` and `sqout_chord`, and it is checked only by `selfcheck` comparing the two copies.

A proposal card, "Make hydra_replay use the engine, not a copy" (task_d8fbee82), is waiting for the user to start. It plans two fixes. First, have the graph hand out the per-chord values it already computes (graph.cpp:126, 139). Second, let `target` pin the whole path, not just where activations start.

## 7. Lessons from this session (the user corrected these)

- **Don't re-price a stored path to answer "can it be computed at open".** I chased `search_target` as a way to regenerate a stored path's details. The user: "why are you reinventing analysis? the engine already knows how to analyze charts." The answer is the engine's normal analysis. Recorded in memory `storage-audit-2026-10-06`.
- **Test small first.** Twice I started whole-library runs (compression, then timing) that would have taken 20+ minutes. A sample answers the same question in about a minute.
- **When something hangs, find out why before re-running it.** I re-ran `target` with timeouts before reading the code. The cause was in pather.cpp and engine.cpp all along.
- **blink-182 Discography is the biggest stored record, not the slowest chart.** The Nirvana "Endless, Nameless Setlist" charts are slower because of their parse.

## Where things are

`docs/handoffs/2026-10-06-storage-audit/` (untracked, not committed):

- `notes/record-content-inventory.md`: every record field, with its writer, readers, class and how to regenerate it, all cited with file:line.
- `notes/other-tables-and-scheme.md`: songmeta, charts, dynamics, meta, the share-once scheme and dead rows.
- `notes/web-research-database-choice.md` and `notes/web-research-compact-storage.md`: the cited web research. Some pages there were summarized by a smaller model.
- `notes/timing-agent-status.md`: the timing agent's progress lines. It was interrupted before its final report.
- `scripts/main`, `scripts/inventory`, `scripts/timing`, `scripts/other-tables`: every measurement script, with the timing agent's saved results (`engine.json`, `big20.txt`, `slow_parse40.txt`, `det_folders.txt`). Its 2.9 MB `parse.tsv` was left out.

The scripts have hard-coded paths into this session's scratchpad, which will be deleted. To re-run one, copy the installed DB first (`hydra.db` plus `-wal` and `-shm`), point the script at the copy, and run it with `py`.

Two build traps:

- `build-cpp\Release\hydra_replay.exe` is newer than `build-cpp\Release\hydra_batch.exe`. The batch writes 2.1.0 rows that this replay calls Stale.
- `build-ship\Release` holds the 2.1.0 ship build and matches the installed database.

Agent journal at the time of writing, as the hook reported it:

```
agent-a669086124d691f43: unfinished, last tool call SubagentHandback, touched 23:20
agent-a88499ba1b00cfecd: unfinished, last tool call SubagentHandback, touched 23:18
agent-ac8cf7224c2bcdfab: unfinished, last tool call PowerShell, touched 23:32
```

The first two did finish. Their reports are in `notes/`; the journal lists them only because a handback leaves no finish mark. The third was interrupted and is not running. TaskStop found no task for it.

## Open questions for the user

1. **Rerun the analysis when a song opens, and store only the library list and summaries?** That brings the four changes in section 4: opens of about 0.3 s worst, no details for a missing or edited chart, reports that analyze the whole library, and a rule for a summary that disagrees.
2. If not: which items on the section 5 trim list, and should blobs be compressed (zlib now, or zstd with a dictionary)?
3. Should anything ever delete the rows of a chart that left the library, or of a chart mode no longer used? Today nothing does.
4. Will you ever keep two settings' results side by side for one chart? If not, the share-once scheme can go.
5. Start the hydra_replay card (task_d8fbee82)?
