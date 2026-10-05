Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task ST2: song facts in the store (findings 62, 63, 345, 54 owner, 88 flag)

Task id: ST2. Base: main after M_D and M7-1 (the main session names the hash at launch). Branch: claude/p7-st2 (worktree `.claude\worktrees\p7-st2`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 2 table, "ST2 Song facts in the store". Decisions: D51 calls 9, 10, 11 and 12 in `docs/audit/2026-10-03-fix-decisions.md` (questions 9, 10, 11 and 12 of `docs/audit/2026-10-04-phase-7-questions.md`); finding 54 is a code-only call. Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 62.`, `#### 63.`, `#### 345.`, `#### 54.` and `#### 88.`.

## Goal

Four questions, all answered in `src/store/record_store.cpp`: how long is this difficulty, which copy names a chart, is the scan cache current, and which fill rule does a file hold. After this task each difficulty's timeline ends at its own last note (D51 call 9), the first copy the scan listed names a duplicate chart and a batch analyzes it once (call 10), the scan cache carries a reader stamp (call 12), the store says which fill rule a file is stamped with, and a record's summary says once whether it has a scored best path, for wave 3 to read (call 11). Scores, paths and result blobs are byte-identical. The results stamp stays "2.1.0".

## What the code does today

Line numbers are from `claude/p7-w1` for the store and from main for `analysis.cpp`; the merged base moves them a little.

**62.** `store::song_length_ms` (record_store.cpp line 522) is the one formula. `songmeta` has one `length_ms` per chart (hyhash), not per difficulty. `upsert_song` (line 946) writes it on every analysis through `COALESCE(excluded.length_ms, songmeta.length_ms)`, so the last analyzed difficulty wins. `set_song_length` (line 936) fills it only when missing; its one caller is `AppState::update_song_length` in `src/ui/app_state.cpp`, fed by `SongLengthJob::run` in `src/ui/song_length_job.cpp`, which already calls `store::song_length_ms`. `get_record` (line 1240) hands that one length to every difficulty's record through `RecordLookup::song_length_ms`. Analyze Expert, then Easy, then open Expert: the Expert timeline ends at Easy's last note.

**63.** `rebuild_chart_library` (line 1695) names a chart from the first copy the scan listed (the `MIN(rowid)` update). `upsert_song` on conflict overwrites the names with whichever copy was saved last. `run_batch` in `src/app/analysis.cpp` (line 509) skips charts already analyzed but never skips a second copy of the same md5 in one run, so it analyzes the chart once per folder.

**345.** `sig_of` (analysis.cpp line 240) keys the rescan cache on file size and mtime only. `chart_library_cache` (record_store.cpp line 1736) hands every cached row back with no version check. A change to the hash rule or the song.ini, .sng or .srb name readers never reaches an unchanged file. ADR 0018 says every kind of stored computed data carries a stamp; this one has none.

**54.** `engine_mode_stamp` in `src/search/graph.h` owns the "ch10"/"ch11" text. The schema upgrade `upgrade_results_key` (record_store.cpp line 706) compares the stored stamp with the literal "ch10". The three CLI readers (`src/cli/batch.cpp`, `report.cpp`, `fillcompare.cpp`) each map the stamp to a rule their own way; they are task RP's in wave 3. Nothing in the store answers "which fill rule does this file hold".

**88.** `summarize_record` (line 557) returns an empty `PathSummary` for a Ready record with no paths, so its `score` and `stars` are unset. Five readers in the library, the query and the two report pages then disagree on whether such a chart is analyzed. Nobody has seen a real analysis produce one.

## What changes

Owner: `RecordStore` in `src/store/record_store.cpp`, with `record_store.h` for declarations.

**62, the stored layout (D51 call 9).** A new table `songlength` with `hyhash`, `chartmode` and `length_ms`, primary key (hyhash, chartmode), created with `CREATE TABLE IF NOT EXISTS` beside the others in the constructor; no upgrade step, no `user_version` change (phase 6's J4-2 owns that slot). `save_analysis` writes this difficulty's length into it on every analysis, replacing the row (it has the row's `chartmode`). A new overload `set_song_length(hyhash, chartmode, length_ms)` fills a difficulty's row only when it has none. The old `set_song_length(hyhash, length_ms)` stays exactly as it is, because `app_state.cpp` is phase 6 J2-4's this wave; task LB switches `update_song_length` to the new overload in wave 3. `get_record` reads the key's difficulty from `songlength` first and falls back to `songmeta.length_ms`. `songmeta.length_ms` keeps its write rule; `add_song` stays the names-and-map write. The comment on `RecordLookup::song_length_ms` says where the number comes from and what the fallback is. Which stored bytes change: one new table; every existing table, row and blob is untouched. How old databases read: a database from before this build has no `songlength` rows, so every record reads the fallback, which is exactly what it shows today, until that difficulty is analyzed again or the backfill (after LB) fills it. An older Hydra opening the file ignores the new table. `song_length_job.cpp` is expected to stay as it is; it already calls the owner.

**63 (call 10).** One private rule in the store, "which copy names an md5": the first copy the scan listed (the `charts` row with the smallest rowid for that md5). `rebuild_chart_library`'s update and `upsert_song`'s conflict branch both use it: when the `charts` table lists the chart, the names come from that copy, whatever names the save carried; when it does not (tests, `hydra_bench`, a chart analyzed before any scan), the save's names are taken, as today. `run_batch` skips a `ScanItem` whose md5 is already in its todo list, so `progress.total` counts charts, not copies. The batch confirm dialog's count is LB's (wave 3).

**345 (call 12).** `kChartMetaStamp` in `src/store/stored_versions.h`, a `StampRule<int, 1>` written as 1 and accepting 1, with a BUMP comment naming what it covers: `hash_chart_file` and the 1 MB .sng head rule (D51 call 13), `sig_of`, and the song.ini, .sng and .srb name readers. The stamp is stored once per file as a `meta` row named `chart_meta_version`, written by `rebuild_chart_library` inside its transaction. `chart_library_cache` returns an empty cache when the row is missing or `is_current` says no, so the next scan re-reads every file once and writes a stamped table. `sig_of` stays size plus mtime; its comment says the stamp, not the sig, answers for the readers.

**54.** `RecordStore::stamped_fill_rule()` returns `std::optional<FillDeadlineRule>`: a "ch10" stamp is `Ch10`, "ch11" is `Ch11`, an unstamped file with results rows is `Ch11` (the migration's and hydra_batch's rule today), an unstamped empty file is empty. It builds its text from `engine_mode_stamp`; `record_store.cpp` may include `search/graph.h`, since both live in `hydra_core`. `upgrade_results_key` calls it instead of spelling "ch10". The CLI readers switch over in RP.

**88 (call 11).** `PathSummary` gets one answer to "does this record have a scored best path", decided once in record_store: recommended as a member function that reads `score.has_value()`, so a summary read back from the columns answers the same as a fresh one. The comment on `summarize_record` says a Ready record with no paths has no facts (D51 call 11) and that LB's `facts_of` and `query_matches` and RP's leaderboard and fill pages read this flag in wave 3.

## Owned files (only these may change)

- `src/store/record_store.cpp`, `src/store/record_store.h`
- `src/store/stored_versions.h` (`kChartMetaStamp` and its comment; no other stamp moves)
- `src/app/analysis.cpp`, `src/app/analysis.h` (`sig_of`'s comment, `run_batch`'s dedupe)
- `src/ui/song_length_job.cpp` (expected unchanged)
- `tests/test_store.cpp`, `tests/test_analysis.cpp`
- `tests/test_single_owner.cpp`: your own scan rows at the end of the file only; the main session joins them at M7-2.

`tests/test_cli.cpp` is phase 6 J2-3's this wave: run its case, never edit it. `src/ui/app_state.cpp` is J2-4's: untouched. `src/cli/*.cpp` are RP's.

## Test cases to add or extend

Write each red first, then green. Use the files' own helpers: `fixture()`, `at_cap`, `chart_entry`, `temp_db`, `exec_on_file`, `beat_song` from `tests/record_fixtures.h`, `hash_chart_file`, `corpus::first_chart_with_notes()`. No new helper.

In `tests/test_store.cpp`:
1. `a song's length is stored per difficulty` (new). `save_analysis` the same hyhash under chartmode "a" with `fixture().song` and under "b" with `beat_song({}, {}, 1920)` (the song the tempo-map case already uses), each with `prepare_row` of `at_cap(4)`. Pin: `get_record` at "a" reads `song_length_ms(fixture().song)` and at "b" reads `song_length_ms(second)`, both computed by calling the owner. Red line: today both read the second song's length.
2. `an old database's one length shows until that difficulty is analyzed again` (new). `add_song` plus `add_record` under "a" and "b" (no per-difficulty rows), then pin both lookups read the `songmeta` length; then `save_analysis` under "a" with the other song and pin "a" reads the new length while "b" still reads the old one.
3. `the scan's first copy names a chart whatever copy was analyzed` (new). `rebuild_chart_library` with `chart_entry("h", "Scanned Title")` and a second copy at another `notespath`, then `add_song("h", "Second Copy", ...)` as analyzing copy B does. Pin: `list_records` shows "Scanned Title". Red line: "Second Copy".
4. `the scan cache is dropped when its reader stamp is not current` (new, file database). Rebuild the library, reopen, pin the cache has the row; `exec_on_file` deletes the `chart_meta_version` meta row, reopen, pin the cache is empty; rebuild again and pin it is back. Red line: the cache is full today.
5. `the store names the fill rule a file holds` (new). Four stores: stamped with `engine_mode_stamp(Ch10)`, with `engine_mode_stamp(Ch11)`, unstamped with one `add_record`, unstamped and empty. Pin Ch10, Ch11, Ch11, empty.
6. Extend `a current-version record with no paths is Ready, not Stale`: its summary says no scored best path; `at_cap(4)`'s says yes; the `list_records` row for each says the same.

In `tests/test_analysis.cpp`:
7. `run_batch analyzes a chart found in two folders once` (new). Two `ScanItem`s with one md5 from `hash_chart_file(corpus::first_chart_with_notes())` and different `notespath`s, a counting analyzer through `BatchCallbacks::analyze` as the cancel cases do, `on_progress` capturing `total`. Pin: `total` is 1 and the analyzer ran once. Red line: 2 and 2.

Existing cases that must pass unchanged: `a stored song keeps its length, and an old songmeta row reads none`, `a saved song's tempo map follows the latest analysis`, `a song's stored names follow the latest analysis and the latest scan`, `a schema 2 database hydra_batch --legacy-fills filled is filed under 1.0`, `a failed library rebuild keeps the previous scan`, `a charts table from before the sig column still rebuilds`, `rescan cache reproduces the scan without reading chart files`, `run_batch files results under the lens it is given`.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*test_store*`
- `build-cpp\Release\hydra_tests.exe -sf=*test_analysis*`
- `build-cpp\Release\hydra_tests.exe -tc="hydra_batch reuses the GUI's scan cache"` (runs the hydra_batch exe: build `hydra_batch` with a warm `.\build_cpp.ps1 -Target hydra_batch` first)

Nothing else. Never the full suite.

## Stored results

No result row or blob changes, and the results stamp stays "2.1.0". Two things are added to the file: the `songlength` table and the `chart_meta_version` meta row. Say in your report how a database from the shipped 2.0.0 reads after this build opens it: lengths fall back, the first scan re-reads every chart once.

## Not in this task

- `AppState::update_song_length` switching to the per-difficulty call, and the batch confirm's count (LB, wave 3).
- The CLI readers of the stamp (RP) and the five readers of the no-paths flag (LB and RP).
- `summarize_path`, `decode_record`, the purge rules and the UNIQUE key (ST1, merged in M7-1); the `user_version` slot (phase 6 J4-2).
- `tests/test_cli.cpp`.

## Done when

- Each difficulty reads its own stored length and old files read the fallback; one "which copy names an md5" rule; `run_batch` dedupes by md5; `kChartMetaStamp` guards the cache; `stamped_fill_rule` exists and the migration calls it; the summary answers "has a scored best path" in one place.
- The seven cases above pass and every other case in the three filters passes.
- `git diff --stat <base>..HEAD` lists only the owned files.

## Open questions (each with a recommended answer)

1. **Old scan caches** (needs the user: one slower scan). A file from before the stamp has no `chart_meta_version` row. Recommended: it reads as not current, so the first scan after upgrading re-reads every chart file once, the way dynamics rows from before their stamp are recounted (ADR 0018). The alternative, stamping the old rows current on open, saves that one scan but trusts a cache no stamp vouches for.
2. **What old records show** (needs the user: what old records display). Recommended: a record with no per-difficulty row reads the old per-chart length, so nothing on screen changes until the difficulty is analyzed again. The alternative, showing no length, would drop the timeline's end on every old record at once.
3. **The flag's shape** (code-only). Recommended: a `PathSummary` member function over `score`; a stored bool would need its own column or a second place that sets it.
4. **The store includes `search/graph.h`** (code-only). Recommended: yes, both are `hydra_core`; the alternative is a store-level copy of the stamp text, which is the copy finding 54 is about.
5. **Names when the scan has not listed the chart** (code-only). Recommended: the save's names win, as today, so `hydra_bench` and the tests keep working.

Notes on the refs: `record_store.cpp/.h` and `test_store.cpp` differ between `claude/p7-w1` (ST1: schema 4 `upgrade_results_key`, `decode_record`, the narrowed purge, stale reasons) and `claude/p3-d2` (`SummaryLookup` gains `stale_build`/`stale_rules`, `summarize_path` reworked); the merged base holds both. `analysis.cpp/.h` and `test_analysis.cpp` on main already carry phase 6 M6-J1a's `to_library_entry`; `claude/p7-w1` is older there. Build against the base, not either branch.

## Commits

One commit for the store, one for `run_batch` if you prefer, trailers `Task: ST2` plus the preamble's others. Report as the preamble says.
