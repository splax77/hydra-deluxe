# storage-T5 report (agent a9226fe941c878fb0)

Status: BLOCKED (call budget). The production side is done and builds; the
tests are not updated yet, so `hydra_tests` does not build. One part of the
brief needs a decision before it can be finished (Step 4, below).

Branch: `worktree-agent-a9226fe941c878fb0`. Base: a450499 plus main merged
twice (f059e60, then f5592ac = main 39e8469 with T2 merged). Work commits:
5a5c8fa (WIP: store, batch, click save, report, bench) and the comment-only
commit after it (tip in the hand-back message).

## What is done

The store keeps summary rows only. `record_store.{h,cpp}` was rewritten. The
results table no longer has a `structure` column, and no path, song or
dynamics table is created. `rank_row` now reads `hyversion` and the `rules_fp`
column; a row is Ready when its stamp is current and its fingerprint is this
process's. `PreparedRow` lost `structure` and `nodes` and gained `rules_fp`
(from `HydraRecord::rules_fingerprint`). `save_analysis(row)` takes only the
row. Deleted: `get_record`, `get_timing`, `for_each_blob`/`BlobRow`,
`load_nodes`, `reload_row`, `collect_orphan_paths`, `upsert_song`, `add_song`,
`encode/decode_tempomap`, `write_song_length`, `fill_song_length`,
`insert/put/get_dynamics`, `fill_missing_stars`, `reindex`, `RecordLookup`,
`DynamicsKey`, `DynamicsEntry`, `naming_copy_of_one_sql`, `read_tempomap`,
`decode_record`. `list_records` names charts through `kNamingCopiesSql` with
a LEFT JOIN. `counts()` now returns {charts with a result, results}.
`delete_auto_results` and the scan purge work on `results` alone.

The upgrade is `drop_stored_details`, the last step of `set_up_schema`. It runs
only when a `paths` table exists. In one transaction it deletes rows with a
score and no stars, drops the four detail tables and drops the `structure`
column. Then it runs VACUUM and `PRAGMA wal_checkpoint(TRUNCATE)`. Every open
sets `PRAGMA journal_size_limit=4194304` (`kJournalSizeLimitBytes`), and a
test seam `journal_size_limit_for_test()` reads it back. Any failure throws
through the constructor's DatabaseOpen wrapper. Older schema 2 and 3 files
still upgrade: `upgrade_results_key` fills `rules_fp` from the old blob's head
(two local constants name the old offset), and `kSchema2ResultsColumns` no
longer lists `structure`.

Also: `path_codec.{h,cpp}`, `test_path_codec.cpp`, `test_dynamics_store.cpp`
and `record_bytes.h` are deleted and out of CMake. `stored_versions.h` lost
`kPathFormatStamp`, `kDynamicsCountStamp`, `kDynamicsBlobStamp` and
`kSongLengthStamp`. `dynamics_breakdown` lost `encode/decode_dynamics`,
`save_dynamics`, `load_stored_dynamics`, `dynamics_store_key` and
`dynamics_entry_from_analysis`. `run_batch` builds only the summary row (no
dynamics count, no length, no Song kept). The click's `save_view_summary`
takes (key, result) and calls `save_analysis(row)`. `collect_stored_rows` is
gone. path_view's `build_record_status`, `RecordStatusView` and
`PathsTabCache::status` were dead (no UI caller after T2) and took a
`RecordLookup`, so they are deleted. `tests/song_digest.h` and `tools/bench.cpp`
were moved off the deleted pieces (see concerns).

Builds: Hydra, hydra_batch, hydra_report, hydra_replay, hydra_bench and
hydra_fillcompare all build. `hydra_tests` and `hydra_uitest` were not built.

The brief's grep over `src` and `tools` C++ files finds one hit: the upgrade's
own drop of the old `songmeta` table in record_store.cpp. The upgrade has to
name it.

## Decision needed (Step 4)

The brief says to replace `record_bytes` with the `replay_json` writer only if
it covers every field `record_bytes` covered, and to stop otherwise. It does
not. `paths_json` writes each path's activations (ticks, SP meter, skips,
chord, squeezes, sqin ticks, skipped fills) and the score split. It leaves out
the record's multiplier squeezes, the SP-end history and bank lists, the
per-SqIn transfer scales, the fill lists, the root/variant tree and each
variant's bank, and the record-level fields (`rules_fingerprint`, caps,
`notecount`). So the record_bytes equality checks (tests/test_store.cpp
round-trip tests, the "a song with ..." check named in test_single_owner's
record_bytes entry, and any other `record_bytes(` caller) cannot be switched
without either a new comparison or dropping those checks. Most of them were
store round-trip tests, which have nothing left to round-trip, so deleting
them may be the right answer; that is the coordinator's call.

The same question hits `digest::row_hash` (tests/song_digest.h), which
hydra_bench --engine and test_perf_digest pin. It hashed the structure blob
and nodes; it now hashes bestpath and the summary only, so the pinned engine
digest literal changes and the digest no longer proves the paths identical.
`with_dynamics` now hashes `count_dynamics`' fields instead of the old blob,
so the pinned parse digests change too. Both literals need recomputing once
the approach is agreed.

## Handoff: what is left, in order

1. Make `hydra_tests` build. Expect errors in: test_store.cpp (about 280
   references: get_record, for_each_blob, structure, paths tables, add_song,
   save_analysis's old signature, songmeta tests, length tests, dynamics),
   test_path_view.cpp (build_record_status tests at about :58-106 and the
   status cache check at about :747-752; delete them), test_analysis.cpp
   (`batch_saved` uses RecordLookup at about :700-770; read `get_summary`
   instead, and drop the length/dynamics checks), test_report.cpp (the
   collect_stored_rows equivalence test at about :786-810; delete it),
   test_app_state.cpp (encode_dynamics equality at about :508; give
   DynamicsBreakdown an equality or compare fields), test_s2_dynamics_tag.cpp
   (:128-141, encode/decode round trip; delete), test_s2_stamps.cpp (:26-29;
   delete), test_dynamics_breakdown.cpp (:421 encode_dynamics equality),
   display_fixtures.h (:138 get_record -> get_summary), dm_fixture.h (:31
   add_song; names now come from a charts row, so the fixture must seed a
   library row if its test reads names), test_fill_report.cpp,
   test_library_model.cpp, test_cli.cpp, test_dm_report.cpp,
   test_long_paths.cpp, test_search.cpp. tests/ui: uitest_details,
   uitest_paths, uitest_library may use add_song or get_record.
2. Write the upgrade test in test_store.cpp: build an old-layout file by hand
   (results with a `structure` column whose bytes 5-12 hold the fingerprint,
   plus `paths`, `path_refs`, `songmeta`, `dynamics` tables) with Ready,
   Stale-version, Stale-rules, NULL-stars and other-chart-mode rows. Check no
   detail tables and no `structure` column after open, statuses unchanged,
   NULL-stars rows gone; open twice (idempotent); check
   `journal_size_limit_for_test() == 4194304`. For the red line, comment out
   the `drop_stored_details()` call and the pragma, run, restore.
3. The schema 2/3 tests (test_store about :1284-1480) build an old table from
   a current row with `kSchema2ResultsColumns`; they now need a hand-made
   `structure` value (path format then fingerprint) since the current table
   has none.
4. test_single_owner.cpp: update or delete the entries that quote deleted
   code (search it for path_codec, structure, songmeta, kPathFormatStamp,
   reindex, write_song_length, SongLength::found, record_bytes,
   RecordStatusView, dynamics_store_key, rules_fp_of, naming_copy_of_one_sql,
   counts()). Consider an entry pinning journal_size_limit to set_up_schema.
5. Step 5: `tools/compare_db.py --summary-only` and its test.
6. Run the brief's verify list plus -sf=*test_app_state*, *test_report*,
   *test_cli*, every edited test file, and the uitests that click and save
   (uitest_details).

## Concerns

- NULL-stars rule: the upgrade deletes rows with `stars IS NULL AND score IS
  NOT NULL`, not every NULL-stars row. A Ready result with no paths has NULL
  stars and NULL score by design; deleting it would turn Ready into Not
  analyzed, which the acceptance criteria forbid.
- Stale reasons: without the path format, `stale_rules` is now "fingerprint
  differs" on any row. A pre-format-7 row (always an old stamp, so Stale
  anyway) may now also say "other rules" in its tooltip. Status is unchanged.
- list_records uses a LEFT JOIN: a result whose chart the library doesn't list
  (hydra_batch with folder arguments) still appears in the DM and fill reports,
  but with blank names, since songmeta no longer names it. Before, songmeta
  named it. This is a display change on folder-argument databases.
- ADR 0014 names: `rules_fp_of` and `row_readable_sql` survive with new
  bodies (rules_fp_of reads only an old blob now; row_readable_sql checks the
  stamp only). `upgrade_results_key` survives unchanged in purpose.
- Files touched beyond the brief's list, all forced by the deletions:
  CMakeLists.txt, src/app/path_view.{h,cpp}, src/app/report.{h,cpp},
  src/core/model.h (comments), src/parse/song.h (comment), src/app/analysis.h
  (comments), src/store/serialize.h (comments), tools/bench.cpp,
  tests/song_digest.h.
- Comments still naming deleted things, outside the brief's grep list:
  src/app/user_messages.h (:2 "add_song failed" example, :47 RecordLookup),
  src/app/song_length.h (:4 save_analysis, fill_song_length),
  src/ui/library_jobs.h (:227 SongLength comment, if any).
- serialize.cpp's restore_timecodes and BinaryReader may now be used only by
  tests; not checked.
- No test was run: hydra_tests does not build yet.
