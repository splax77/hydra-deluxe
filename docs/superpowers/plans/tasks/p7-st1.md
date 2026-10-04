Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task ST1: the store cache and other-rules rows (findings 65, 343, 116, 129, 130 store half, 132, 133)

Task id: ST1. Base: main at 81a2519. Branch: claude/p7-st1 (worktree `.claude\worktrees\p7-st1`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 1 table, "ST1 Store cache and other-rules rows". Decisions: D51 calls 8, 12 and 13 in `docs/audit/2026-10-03-fix-decisions.md` (questions 8, 12 and 13 of `docs/audit/2026-10-04-phase-7-questions.md`). Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 65.`, `#### 343.`, `#### 116.`, `#### 129.`, `#### 130.`, `#### 132.` and `#### 133.`.

## Goal

Two questions, both answered in `src/store/record_store.cpp`: which stored rows survive a write, and what the summary columns are. After this task a result made under other rules survives a write and reads Ready again when the rules match (D51 call 8), every analysis rewrites the tempo map (call 12), every reader decodes a row the same way, and the summary columns, `bestpath` included, are a cache of the blob that `reindex` rewrites in full (call 13). Existing records keep their bytes; the results stamp stays "2.1.0".

## What the code does today

**65.** `RecordStore::write_row` purges twice before it inserts. Purge (1) deletes every row of the chart and mode that fails `row_ready_sql()`. That predicate includes the rules fingerprint (bytes 5 to 12 of the structure blob), so a row analyzed under other rules dies with the old-version rows. Purge (2) deletes the row with the same key (chart, mode, cap, lens) whatever its rules, so a rules-A row and a rules-B row can never sit side by side. ADR 0014 and `docs/UserGuide.md` promise they do. One more fact the scouts missed: `kResultsColumnDefs` declares a UNIQUE constraint over (hyhash, chartmode, sp_cap, ms_enabled, ms_value, depth_mode, depth_value, legacy_fills). It has no fingerprint, so two rows for one key cannot both be inserted today even if purge (2) spared one. See Open questions.

**343.** `upsert_song` writes the tempo map on insert only. On conflict it refreshes the names and the length and leaves the map, with a comment saying it cannot have changed.

**116.** `get_record` rebuilds the record and sets `legacy_fills` from the key's lens. `for_each_blob` rebuilds and never sets it, so a walked 1.0 row says 1.1. `reindex` and `fill_missing_stars` rebuild too and never set it (they don't read it). Four copies of one decode.

**130 (store half).** `prepare_row` checks the cap both ways but the ms limit one way: a record with an ms limit filed under a limit-off lens passes. The test `prepare_row refuses a key whose ms limit isn't the record's` pins that pass today (its last check).

**132, 133, 129.** `prepare_row` writes `bestpath` and the summary columns (`kSummaryColumnList`: score, actcount, maxskip, hardest_ms, avgmult, notecount, sqin_count, sqout_count, pathcount, stars) from `summarize_record`. `reindex` rewrites the summary columns but not `bestpath`. Nothing says the columns are a cache, or that a rule change that alters them bumps `kResultsStamp`. `kSummaryColumnList` lives in `record_store.cpp` (the scouts said `.h`).

## What changes

Owner: `RecordStore` in `src/store/record_store.cpp`, with `src/store/record_store.h` for declarations.

**65.** Purge (1) narrows to rows this build can never read: another results stamp or another path format, without the rules clause. Keep one SQL spelling: split `row_ready_sql()` into a build-readable part and the rules part, so the purge uses the first and `row_ready_sql()` is the two joined; `bind_ready_params` follows. Purge (2) adds the fingerprint (`substr(structure,5,8)` against the store's own fingerprint), so it replaces only this build's row under these rules. Resolve the UNIQUE constraint as the open question decides before you start.

**343.** `upsert_song` writes `tempomap = excluded.tempomap` on conflict and the comment says each analysis rewrites the map (D51 call 12). The length rule is untouched (finding 62 is ST2's, wave 2).

**116.** One private `decode_record` that takes the structure, the node lookup and the row's fill rule, rebuilds the record and sets `legacy_fills`. `get_record` passes the key's lens, `for_each_blob` the walk's lens, `reindex` and `fill_missing_stars` the row's `legacy_fills` column. All four call it; `rebuild_record` is called nowhere else in the file.

**130.** `prepare_row` refuses a record that carries an ms limit when the key's lens has the limit off, with a sentence in the style of the three refusals already there. Both real callers build the key and the search from one `Settings`, so no reachable input is refused.

**132/133/129.** `reindex` rewrites `bestpath` with every summary column. One comment block beside `kSummaryColumnList` in record_store.cpp: the columns, `bestpath`, `stars` and `hardest_ms` included, are a cache of the blob for sorting and filtering; `reindex` rewrites all of them; a rule change that alters any of them bumps `kResultsStamp`. One sentence in the `kResultsStamp` comment in `src/store/stored_versions.h` says the summary columns are covered by the bump rule.

`summarize_path` is phase 3 task K2's and must not change.

## Owned files (only these may change)

- `src/store/record_store.cpp` (everything except `summarize_path`)
- `src/store/record_store.h`
- `src/store/stored_versions.h` (the one cache sentence; no stamp value changes)
- `tests/test_store.cpp`

The plan row names the first three. The test file comes with the plan's filter. `tests/test_cli.cpp` is not yours; its reindex case is run, not edited.

## Test cases to add or re-pin (in `tests/test_store.cpp`)

Write each red first, then green. Use the file's own helpers: `fixture()`, `at_cap`, `at_cap_ms10`, `legacy_at_cap`, `kLensA`/`kLensB`, `temp_db`, `exec_on_file`, `scalar`. No new helper.

1. `a write under rules A keeps the rules-B row` (new). Write a row stamped with other rules (as the case `a row analyzed under other rules reads Stale until the rules match again` builds one, `max_tied_paths = 2`), then write the default-rules row for the same key. Pins: two results rows; the default store reads Ready; a store opened with the other rules reads Ready on the same key; the other-rules row's paths are still in `paths`. Red line: today the count is 1.

2. `a saved song's tempo map follows the latest analysis` (new). Save a song, then save it again with a map from a different fixture song (`beat_song` in `tests/record_fixtures.h` with another BPM), and read `get_timing` back: it is the second map. Pin one tick's ms under each map, computed by calling the timing object, not typed.

3. `a walked 1.0 row says 1.0` (new). Store `legacy_at_cap(4)` under the legacy lens, walk it with `for_each_blob` under that lens, and pin `legacy_fills == true` on the record handed to the callback. Red line: false today.

4. Re-pin `prepare_row refuses a key whose ms limit isn't the record's`: its last check (a 10 ms record under `kLensB`) becomes a throw, with a comment saying the limit is checked both ways (finding 130).

5. Extend `RecordStore maintenance: has_record, list_records, reindex`: on a file database, blank the `bestpath` column with `exec_on_file`, run `reindex`, and pin that `list_records` shows the best path's `pathstring()` again. Red line: the blanked text today.

Existing cases that must pass unchanged: `a current-version write purges the chart's old-version rows and their paths`, `a row analyzed under other rules reads Stale until the rules match again`, `1.0 and 1.1 results for one chart sit side by side`, `a stored song keeps its length, and an old songmeta row reads none`, `a saved result stores its best path's star count`.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*test_store*`
- `build-cpp\Release\hydra_tests.exe -tc="hydra_batch --reindex*"` (in tests/test_cli.cpp; it runs the hydra_batch exe, so build `hydra_batch` with a warm `.\build_cpp.ps1 -Target hydra_batch` first)

Nothing else. Never the full suite.

## Stored results

None. Every existing row keeps its bytes; only rows that would have been deleted now stay, and `bestpath` is rewritten to the same text it already holds. The stamp stays "2.1.0".

## Not in this task

- `summarize_path` (phase 3 K2) and the song length rule (ST2, wave 2).
- The display halves of 130 (`path_view`, `preview_tab`: wave 2) and the headline's stars (129 is recorded as it is).
- `delete_auto_results` and the stars backfill.

## Done when

- A rules-B row survives a rules-A write and reads Ready under rules B; `upsert_song` rewrites the map; one `decode_record`; `prepare_row` checks the ms limit both ways; `reindex` rewrites `bestpath`; the two comments are in place.
- The five cases above pass, every other case in `-sf=*test_store*` passes, and the reindex CLI case passes.
- `git diff --stat 81a2519..HEAD` lists only the four owned files.

## Open questions

1. **The UNIQUE constraint (blocking).** The results table's unique key has no rules fingerprint, so a rules-A row and a rules-B row for one key cannot coexist without a schema change. Recommended: a schema 4 rebuild in the style of `add_fill_rule_column`, adding a `rules_fp` column filled from `substr(structure,5,8)` and a UNIQUE key that includes it, with every row and `result_id` kept and blobs untouched; `write_row` then fills the column and purge (2) compares it. The alternative is dropping the UNIQUE key and trusting purge (2). The main session decides before launch; the implementer does not pick.
2. Case 1's rules-B store opened on the same file must find the row Ready; if `WinnerPicker::only_winner` refuses two candidate rows for one key when only one is Ready, report it rather than widening the picker.

## Commits

One commit, trailers `Task: ST1` plus the preamble's others. Report as the preamble says.
