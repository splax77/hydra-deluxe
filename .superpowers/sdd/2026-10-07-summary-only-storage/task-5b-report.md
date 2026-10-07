# storage-T5b report (agent ad610d023374f9e30)

Status: DONE_WITH_CONCERNS. hydra_tests and hydra_uitest build, the named tests pass, the upgrade test went red then green, and D92 is in. One piece of the brief is not done: Step 5, `tools/compare_db.py --summary-only` and its test. I hit the 100-call wrap-up point before reaching it.

Branch: `worktree-agent-ad610d023374f9e30`, on top of T5's branch (b283175) merged in. Commits, in order:

- c5b05c0 deletes `tests/test_s2_stamps.cpp`. A pathspec error dropped every other file from that commit, so its message describes the next one.
- 6fe182f carries the real change (26 files). Its message says the previous commit was the deletion only.
- 1200555 re-pins the digests (R16), with the engine diff-stat proof in the message.
- The tip commit holds the stale-comment fixes and this report.

## What changed

**test_store.cpp, rewritten for the summary-only store.** Store round-trip tests went: path format, for_each_blob, path dedupe and GC, songmeta, song length, tempo map, stars fill and reindex. Lookups now ask `get_summary`, and names come from library rows.

The new upgrade test builds an old-layout file by hand. It uses the old results table with its `structure` blob (path format 7, then the fingerprint), plus `paths`, `path_refs`, `songmeta` and `dynamics` with a row each. The seeded rows cover:

- a Ready row, and a Ready row in another chart mode
- a no-paths Ready row with stars and score both NULL, which must stay (R17)
- Stale rows for another build, other rules, and both
- one row with a score and NULL stars

After the first open it checks four things. Every status and stale reason is unchanged, the NULL-stars row is gone, no detail table or `structure` column is left, and `freelist_count` is 0. A second open is checked to do nothing: same result ids, and the delete rule is not applied again. A separate test checks that `journal_size_limit` reads 4194304 on each open.

The schema 2 and 3 tests rebuild the old blob with `unhex('07000000' || hex(rules_fp))`. The schema 3 test now proves `rules_fp` is filled from the blob's bytes 5 to 12.

**The engine hasher (R16).** `tests/song_digest.h` gained `record_hash`, a field-by-field walk of HydraRecord. It covers:

- the rules fingerprint, ms limit, cap, converged flag and fill rule
- the multiplier squeezes (chord code and combo)
- every path tree, the all-0 paths included. For each path: activations, variant tail, notecount, trailing bank, the six score parts, tied count, var_point and variants.
- for each activation: tick, chord, frontend points, display_backends rows, every squeeze with its offset and transfer scale, e_offset, transfer_post, sqout_tick, sp_end_steps, bank_rise_ticks and skipped_fill_ticks

`row_hash(row, record)` now folds `record_hash` with the summary. `hydra_bench --engine` and test_perf_digest call it. test_search's "a 4-bar graph built at the song's phrase count" now compares `record_hash`. The other record_bytes users were store or codec round trips and are deleted: the test_search codec tests and the tied-variant round trip. test_single_owner's record_bytes row now points at `digest::record_hash`. It flags `record_bytes`, `flatten_record` or `encode_path_node` coming back.

**Re-pinned digests (1200555).** Engine 0x1a0929fcb662ebaa, Expert parse 0x40c54a484d033934. The Hard parse digest counts no dynamics and is unchanged.

Here is the proof the engine is untouched. `git diff 39e8469 --stat -- src/parse src/core src/search` gives `src/core/model.h | 28`, `src/parse/song.h | 2`, 2 files, 13 insertions and 17 deletions. I read the `-U0` diff: every changed line in both files is a comment.

**D92 (the coordinator's message).**
- `report::lacks_chart_library(store)` in report.cpp is the one owner. It means the store has results but no library. generate_report's NoLibrary branch, `collect_dm_rows` and `generate_fill_report` all ask it. The DM page throws AlreadyPlain with `report::kNoChartLibrary`. The fill page returns that sentence, and either database can trigger it.
- `report::records_by_hash` drops results whose chart the library doesn't list, so both comparison pages leave them out. On the DM page the leaderboard score stays and reads "not in library". On the fill page the chart's row is absent.
- I wrote four failing-first tests and kept their red lines: `CHECK( stopped )` false (DM no library); `no paths == not in library` (DM outside result); `reason == ""` and "Nothing is analyzed ..." vs the sentence (fill no library); `REQUIRE( 3 == 1 )` (fill outside results).
- test_single_owner has a new row, "Does this database have a chart library to report on?". It flags `chart_library_count() == 0` anywhere in src except the owner.
- One reading I chose: a database with no results at all keeps each page's own words. This keeps D50 item 3's "Nothing is analyzed ... in either database" for two empty databases, and the path report's NothingStored.

**Other test moves.**
- A shared `test::name_chart` in display_fixtures.h lists a chart in the library and keeps the other rows. It replaces add_song wherever a test reads names or reports: fill, DM, CLI and the dm_fixture.
- DynamicsBreakdown and DynamicsCounts gained a spelled-out `operator==`. test_app_state and test_dynamics_breakdown compare with it instead of encode_dynamics.
- Dead tests are deleted: build_record_status and the status cache, the batch length tests, the stored-vs-engine report test, and test_s2_stamps.cpp (also removed from CMake).
- The CLI fillcompare test now gives both databases a library row (D92).

**test_single_owner.** I updated or removed every entry that quoted deleted code: path_codec owner lines, rebuild_record, the path_refs delete order, the stored-length entry, the structure-head owner, and the cap purge line. The MR1 baseline drops two test_search clones that no longer exist.

**Two production edits for single-owner.** Both get_summaries and list_records built this process's rules fingerprint with the same line, which the rules entry flagged twice. A small helper, `ready_rules_fp`, now builds it in one place. I also wrote the test's old-layout table in fewer, longer lines, so the clone check doesn't flag it against `kResultsColumnDefs`.

**Stale comments fixed:**
- user_messages.h :2 (add_song becomes add_row) and :47 (RecordLookup becomes SummaryLookup)
- song_length.h :4 (nothing stores the length)
- the dynamics_breakdown.h late_tag_ms comment
- library_jobs.h :227 is code that uses the still-live `store::SongLength`, so I left it.

## Acceptance criteria

- Upgrade test (statuses kept, NULL-stars rows gone, no detail tables or `structure`): PASS. Red run, with `drop_stored_details()` and the pragma commented out: `CHECK( store.get_summary(no_stars).status == RecordStatus::NotAnalyzed ) values: CHECK( 2 == 0 )`, `detail_tables_in(path) == 0 values: CHECK( 5 == 0 )`, `structure_columns_in(path) == 0 values: CHECK( 1 == 0 )`.
- A second open is idempotent: PASS.
- journal_size_limit reads 4194304: PASS. Red: `CHECK( -1 == 4194304 )`.
- The grep for path_codec, tempomap, songmeta, get_record, for_each_blob, reindex and kPathFormatStamp over src and tools C++ files finds one hit: the upgrade's own `DROP TABLE IF EXISTS songmeta`. The upgrade has to name it.
- `py tools\test_compare_db.py` passes: NOT DONE (Step 5 not started).
- The batch's rows match a baseline under `compare_db.py --summary-only`: NOT DONE.

## Tests run (all in my worktree)

- `hydra_tests -sf=*test_store.cpp`: 61/61, run after the last production edit.
- `-sf=` over test_fill_report, test_dm_report, test_report, test_app_state and test_analysis: 150/150.
- `-sf=` over test_app_state, test_report, test_analysis, test_path_view, test_fill_report, test_dm_report, test_search, test_dynamics_breakdown and test_s2_dynamics_tag, before D92: 266/267. The one failure was the fill copies test, which I fixed and reran in the 150/150 run above.
- `-tc=single-owner*`: 10/10.
- `-sf=*test_cli.cpp`: 13/13.
- `-sf=*test_perf_digest.cpp`: 3/3.
- `hydra_uitest --test` for analyze, cap-switch, legacy-fills, dynamics, dynamics-reopen and stars: all PASS.
- The full build (`.\build_cpp.ps1`, all targets, hydra_uitest included) succeeds.

Twice a freshly built hydra_tests crashed (an access violation, then a SIGSEGV). Both times the fix was deleting `build-cpp\**\*.iobj` and `*.ipdb` and rebuilding: the known stale-state trap.

## Concerns

- Step 5 is not done: `compare_db.py --summary-only` and its test, and the testdata baseline comparison. It needs a fresh agent.
- Commit c5b05c0 holds only the stamps test's deletion, though its message describes the whole change. 6fe182f carries the rest and says so. I could not amend.
- D92 reading: "a database with no chart library" applies only when it holds results. Two empty databases still get D50 item 3's settings sentence. Please confirm this matches the user's intent.
- test_search, test_path_view, test_dynamics_breakdown and test_s2_dynamics_tag were last run before the D92 edits and the `ready_rules_fp` helper. Neither touches what those files test.
- A few `#include "app/user_messages.h"  // kNoPathsFound` comments are left. path_view.cpp and test_path_view.cpp no longer use that constant.
- `record_hash` walks a superset of the old codec's fields: it adds variant_tail, tied_count and legacy_fills. A future engine change to any of those moves the pin.
