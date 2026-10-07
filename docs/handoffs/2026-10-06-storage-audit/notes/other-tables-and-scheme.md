# Storage audit: everything outside the record content (songmeta, charts, dynamics, meta, the paths scheme, dead rows)

Read-only. Nothing in the repo was touched. Scratch scripts are in `...\scratchpad\audit3\` (q1.py, q2.py, q3.py). All numbers come from the checkpointed copy at `...\scratchpad\db\hydra.db` (292 MB, 71,380 pages of 4 KB, 35 free). Agent id a669086124d691f43; status lines written.

## The short version

Nothing in the paths/path_refs scheme is shared today. Every one of the 91,196 nodes has exactly one reference, and every chart+mode has exactly one results row. The scheme costs about 37 MB of hex-text keys and indexes to make possible something that has never happened in this database.

The 29.7 MB tempomap is a cache of the parsed chart. Its only production reader is `get_record`, which uses it so the Paths tab can show ms and measures the moment you click a song, before any file is parsed. Reports and DMBot never touch it. Dropping it means a parse on click (about 1 ms average, hundreds of ms for the worst text .chart) and losing the display when a chart file is gone or changed.

songmeta's names are a byte-for-byte copy of the charts table's naming copy: 0 of 19,099 rows differ. They only earn their place for charts that are no longer in the library, which D77 says reports should still count.

Nothing ever deletes a songmeta, dynamics, or orphaned results row. 288 md5s have songmeta and results rows but no charts row; all of them are old-version results. The old `records` table is not in the installed DB.

## Table

| table.column | MB | writer | readers | class | computed at open? how |
|---|---|---|---|---|---|
| songmeta.hyhash | 0.6 (+0.6 PK index) | `upsert_song` record_store.cpp:1108-1119 | every join | B (key) | no (identity) |
| songmeta.ref_name / ref_artist / ref_charter | 0.9 | `upsert_song` :1097-1117 (names from `kNamingCopiesSql`); `rebuild_chart_library` :1904-1907 copies charts' names on every scan | `list_records` :1802 (report.cpp:246, dm_report.cpp:209 then :235-238); `for_each_blob` :1560 (report.cpp:269, :292-294) | D today (0 rows differ from charts); C only for charts gone from the library | yes: JOIN `kNamingCopiesSql` (record_store.h:344) at read time |
| songmeta.tempomap | 29.7 | `encode_tempomap` :130-144 via `upsert_song` :1113, rewritten on every analysis | `get_record` :1447, :1456 only (app_state.cpp:233). `get_timing` :1476 has no production caller: grep over src/tools/tests hit only tests/test_store.cpp:1872, 2321, 2326 | A (cache of `Song::tpm_changes`/`bpm_changes`) | yes: `load_songpath(notespath)` then `Song::timing()`; the ledger says `decode_tempomap` and `Song::build_timing` use the same `SongTiming` constructor (derivation-ledger.md:4930). Cost: parse, avg ~1.2 ms pooled (REPORT.md:53 22.4 s over 18.8k charts), worst 402 ms (REPORT.md:64) |
| songmeta.length_ms / length_version | 0.3 | `write_song_length` :1072-1083 via `save_analysis` :938 and `fill_song_length` :1060 (app_state.cpp:343) | `get_record` :1460-1461 then `RecordLookup::song_length_ms` to paths_tab.cpp:555, path_view.cpp:288-290, :446-447 | A (owner `song_length_ms` song_length.cpp:28) | yes: `chart_song_length_ms` song_length.cpp:42, which needs a parse only when no stated length (the backfill `SongLengthJob` does exactly this today, song_length_job.cpp:19-27) |
| charts.md5 | 0.6 (+0.6 charts_by_md5) | `rebuild_chart_library` :1888 from `hash_chart_file` analysis.cpp:330 | everything | B | no |
| charts.name / artist / charter | 0.9 (+0.35 charts_by_name) | :1889-1891 from the ini/.sng/.srb readers analysis.cpp:150-205 | `list_chart_library` :1952 to app_state.cpp:83; naming copy :1099, :1906, :1516 | B (list, sort, search) though an A cache of song.ini | yes but pointless: the library needs them without opening |
| charts.path | 2.6 | :1892 | every open, every job | B (the only record of where the file is) | no |
| charts.folder | 1.4 | :1893 from `relpath(parent_folder(dir), origin)` analysis.cpp:438 | library_table.cpp:222, :397; library_model.cpp:115 (search Folder field) | D-ish | yes: `relpath` of the chart's grandparent against the `settings.chartfolders` root that prefixes the path. Not derivable from `path` alone: the scan root is not on the row |
| charts.sig | 0.9 | :1894 from `sig_of` analysis.cpp:237-241 (`notes size:mtime[:ini size:mtime]`) | `chart_library_cache` :1930 (scan skip, analysis.cpp:486); preview_load_job.cpp:136 (changed-chart check) | B (scan-skip) | no |
| charts.stated_length_ms / delay_ms | 0.3 | :1896-1897 from `read_chart_timing_meta` analysis.cpp:381-391 | `chart_timing_meta(scanned, path)` analysis.cpp:393 then song_length_job.cpp:19, preview_load_job.cpp:166, library_jobs.cpp:329, analysis.cpp:708 | A (cache of song.ini / .sng / .srb metadata) | yes: `read_chart_timing_meta` re-reads the ini (it already does for rows with none) |
| dynamics.* | 4.4 (2.2 blob + keys x2) | `insert_dynamics` :913-924 via `save_analysis` :945 (analysis.cpp:705, app_state.cpp:626) and `put_dynamics` :907 via `save_dynamics` dynamics_breakdown.cpp:194 (app_state.cpp:471) | `get_dynamics` :959-971 via `load_stored_dynamics` dynamics_breakdown.cpp:187 from app_state.cpp:447 | A | yes, and it already does on a miss: `DynamicsLoadJob` parses and calls `count_dynamics` dynamics_load_job.cpp:23-26, the same owner (dynamics_breakdown.cpp:111). Cost = one parse; the count itself is 0.48 s for the whole library (REPORT.md:137), about 0.03 ms a chart |
| meta (3 rows) | ~0 | `meta_set` :871; `set_engine_mode` :892 (batch.cpp:186); :1305; :1909 | batch.cpp:166-167, fillcompare.cpp:96, `stamped_fill_rule` :897, :1274, :1940 | C | no |
| paths + path_refs keys and 3 indexes | ~37 | `write_row` :1225-1244 | `load_nodes` :1011 (`kLoadNodesSql` :1002); GC :1254 | scheme overhead; D in practice (1 ref per node) | see scheme section |
| dead rows (295 old results, 288 orphan songmeta, 6 orphan dynamics) | ~4.5 | legacy | none | D | n/a |

## songmeta

Who writes it. `save_analysis` (record_store.cpp:926-957) calls `upsert_song` (:1085-1120) on every analysis, GUI (app_state.cpp:624) or batch (analysis.cpp:727). It inserts or updates the names and rewrites the tempomap every time (the comment at :1092-1095 says why: a reader fix reaches the stored map on the next analysis). `rebuild_chart_library` also rewrites the names from the charts table on every scan (:1904-1907). The length is written only by `write_song_length` (:1072), from an analysis (:938) or the open-song backfill (`fill_song_length` :1060, app_state.cpp:343).

Why store the tempomap when the chart is parsed anyway. Because on click the chart is not parsed. `refresh_viewed_record` (app_state.cpp:227-242) calls `get_record`, which decodes the stored tempomap (:1456), restores every stored tick to ms (:1457) and hands `timing` to the Paths tab (paths_tab.cpp:553). The Preview tab parses the file (preview_load_job.cpp:157), the Dynamics tab parses on a miss, the length backfill parses when needed; the Paths tab never does. So the tempomap exists to make the first click instant and to keep working when the chart file is gone or changed. No reader needs timing without the chart for any other reason: `for_each_blob` (:1559-1562) reads names only and its header says timecodes are not restored (record_store.h:481); `list_records` (:1801-1808) reads names and summary columns; DMBot goes through `list_records` (dm_report.cpp:209). Reports and DMBot never read timing.

Where the 29.7 MB goes. 1,667,981 bpm changes at 16 bytes each (i64 tick + f64 bpm) is 26.7 MB of it. Half the songs (10,025) have 32 changes or fewer and together use 1.5 MB; 1,662 songs with more than 256 changes use 14.7 MB; 39 songs over 2,048 changes use 2.5 MB alone (max 13,869 changes, 226 KB). tpm changes are a median of 1. A delta-varint encoding would cut this roughly 3x without changing who owns anything. Dropping it entirely means a parse on click.

Names. `ref_name/artist/charter` equal the charts naming copy in every one of 19,099 rows (query joining on `MIN(rowid)` per md5: 0 differences). They are a denormalized copy, kept in sync by two writers (:1097-1106 and :1904-1907). They matter only for a result whose chart has left the library; D77 (record_store.h:453-458, report.cpp:276-278) says such a chart still counts once in reports, so today those names let the report print it. The 288 orphans in this DB are all stale and never listed, so the case has not actually arisen here.

Length. Class A; the owner is `song_length_ms` (song_length.cpp:28-40), fed by charts' stated length and delay plus the parsed `chart_offset_s`, or the last Expert note when no length is stated. Stored so the Paths tab can draw song fractions on the first click; the backfill job works it out again when the stamp is not current. 18,811 rows carry stamp 2; the 288 orphans carry 0.

## charts

Writer: one, `rebuild_chart_library` (:1873-1915), which deletes every row and reinserts the scan (:1882-1898), from `save_scan_as_library` (analysis.cpp:368-379) in the GUI scan job and hydra_batch (batch.cpp:222-224). It is a full replace, so a chart that vanished from disk vanishes from this table on the next scan; nothing else follows.

folder. It is `relpath(parent_folder(dir), origin)` (analysis.cpp:438): the chart folder's parent, relative to the configured scan root. Example row: path `C:\Clone Hero\songs\synchotic\Sync Charts\Rock Band\Rock Band Network\Zoo Seven - Painted\notes.mid`, folder `songs\synchotic\Sync Charts\Rock Band\Rock Band Network`. All 19,436 rows follow that shape; 3,166 distinct folders. It is derivable from `path` plus the scan root, and the root is not on the row, so a pure `path`-only derivation is not possible. With `settings.chartfolders` at hand it is a cheap string operation at list time. It is 1.4 MB. It feeds the Folder column (library_table.cpp:222), its highlight (library_table.cpp:397-399) and the `folder:` search field (library_model.cpp:115, library_query.h:48).

sig. `notes.size:notes.mtime[:ini.size:ini.mtime]` (analysis.cpp:237-241). It is the rescan cache key (`chart_library_cache` :1926-1935 skips rows with an empty sig; `sig_unchanged` analysis.cpp:246) and the Preview's "has the chart changed" shortcut (preview_load_job.cpp:136, through `chart_files_unchanged` analysis.cpp:340-351). It is what makes a rescan 0.95 s instead of 4.2 s (REPORT.md:45). Needed.

The 542 repeated md5s. 519 md5s appear twice, 21 three times, 2 four times. 515 of the 542 groups span two different root folders: the same file copied into two places on disk, typically `...\Drummer's Monthly Drive\<charter>\...` and `...\<charter>\...`. All 542 groups have different sigs (separate files, separate mtimes), and 46 have different names in their song.ini. These are real duplicate files, not a scan bug. The library lists every copy on purpose (D76/D77), and `kNamingCopiesSql` (record_store.h:344-354) picks the first-listed copy's names for the one songmeta row. 58 md5s in the library have no songmeta, results or dynamics row: the "no notes" failures (REPORT.md:155).

stated_length_ms / delay_ms. Only 19,428 and 11,258 rows carry a value; 407 delays are non-zero. They are a cache of the metadata readers so the length owner need not reopen song.ini; `chart_timing_meta` (analysis.cpp:393) already falls back to re-reading when a row has none.

## dynamics

The blob is 118 bytes (dynamics_breakdown.cpp:135-139): a layout stamp, `dynamics_enabled`, nine rows of (ghost, accent, normal) u32 counts, `late_tag_ms` and `marks_before_tag`. It is a pure function of the parsed Song (`count_dynamics` :111-129). Keyed by (md5, difficulty, pro): 18,811 rows are (Expert, pro, stamp 2); 9 rows carry stamps 0 or 1 and read as missing (:969); 6 rows belong to md5s no longer in the library; 3 md5s have both a pro and a non-pro row.

The only reader is the Dynamics tab (app_state.cpp:445-452). On a miss it starts `DynamicsLoadJob`, which parses the chart with `load_songpath` and calls the same `count_dynamics` (dynamics_load_job.cpp:23-26), then saves the answer (app_state.cpp:469-471). So "compute at open" is already the fallback and already uses the one owner. The cost of a recount is the parse, not the count: the count is 0.48 s across the whole library (REPORT.md:137), the parse 22.4 s pooled (REPORT.md:53), about 1.2 ms a chart on average and three times that for text .chart files (REPORT.md:64). The cache saves one parse per tab open. Its 4.4 MB is 2.2 MB of blobs plus the text key (`md5` 32 chars + `"Expert"`) stored twice (table and PRIMARY KEY autoindex). Rows are only ever `INSERT OR REPLACE`d (:916); nothing deletes one.

## meta

Three rows: `engine_mode` = `ch11` (a label hydra_batch writes, batch.cpp:186, and reads back to refuse a run under the other fill rule, batch.cpp:166-183; hydra_fillcompare reads it at fillcompare.cpp:96; the schema-2 upgrade reads it at :815), `auto_results_deleted` = 1 (one-time flag, :1266-1305) and `chart_meta_version` = 2 (`kChartMetaStamp`, :885, :1909, :1940). Class C, negligible size, all needed.

## The paths / path_refs scheme

What the data says. 91,196 nodes, 91,196 refs, every node referenced exactly once, 0 orphan nodes, 0 dangling refs. 19,106 results, exactly one per (hyhash, chartmode). 7 hyhashes carry two rows, but those are two chartmodes, and nodes are scoped per chartmode (:976-985), so even there nothing can be shared. Sharing has never happened in this database.

When sharing could happen. A second row for the same chart+mode needs a different UNIQUE key (:258-259): another SP cap, another Lens (ms on/off and value, depth mode and value, legacy fills) or another rules fingerprint. The installed DB holds one cap (4), one lens (ms on at 10, scores depth 4, 1.1 fills) and one current rules_fp. The user has never kept two settings' results side by side. Even if they did, a node is shared only when its bytes are identical under both settings; ADR 0009:20-26 says the limit changes pruning in both directions, so the overlap is unknowable without data, and there is no data. 588 groups of byte-identical payloads do exist across different charts, but the scheme refuses to share across charts by design (:976-978).

What the keys cost. Each paths row and each path_refs row carries hyhash (32 hex) + chartmode (about 25 chars) + phash (32 hex), 89 bytes of text, 8.1 MB per table. paths stores it twice (row + PK autoindex), path_refs about three times (row + PK (result_id, phash) + `path_refs_by_node`). That is the 37 MB. Across all tables, hex hash text in rows alone is 14.1 MB and chartmode text 5.0 MB, before indexes.

Alternatives, measured. (a) Nodes inline in the structure blob: the sum of structure + its nodes' payloads is 153.1 MB (avg 8.0 KB, max 1.47 MB for one result), against 144.8 MB payload + 8.3 MB structure + 37 MB keys today. It removes two tables, three indexes, the refs-first purge order (:1139-1158), the GC (`collect_orphan_paths` :1251-1262), the second node query on every load (`load_nodes` :1011, :1673) and the reused-result_id hazard that `reload_row` (:1029) guards. The structure blob already names nodes by hash (path_codec); inline it would name them by position. (b) Integer ids plus 16-byte raw hashes in WITHOUT ROWID tables: paths PK (chart_id, mode_id, phash16) is about 20 bytes stored once (clustered), about 1.8 MB; path_refs (result_id, phash16) plus a by-node index about 4.4 MB; roughly 6 MB instead of 37, and sharing stays possible. (c) Independently, storing md5/hyhash/phash as 16-byte blobs instead of 32-char hex everywhere would halve the 14.1 MB of hash text and its index copies.

Is path_refs_by_node needed. Its only user is the GC subquery `SELECT phash FROM path_refs WHERE hyhash=? AND chartmode=?` (:1254-1255); no other statement filters path_refs by chart (grep across src, tests, docs). Even in the current scheme the GC could read the replaced row's refs by result_id (the PK) before the purge and delete only nodes that row named and the new row does not, so the index and the hyhash/chartmode columns on path_refs (5.2 MB of text plus their index copies) could both go.

## Rows nothing can read

Old versions: 295 results rows are not stamped `2.1.0+allzero` (1.6.2: 212 with ms_enabled = -1; 1.7.5: 30; 1.7.8: 43; 1.7.10: 5; 1.8.2: 5). The brief said 83; I measured 295 and have listed the breakdown so the two counts can be reconciled. 288 of the 295 belong to md5s no longer in charts, and every orphaned md5 is one of these. The other 7 are for charts still in the library but in chartmodes no longer run (Expert Drums 2x, Expert Pro Drums 1x). Their bytes: 0.89 MB structure, 1.86 MB nodes, 0.90 MB tempomap, plus their share of keys, around 4.5 MB.

Who deletes them. `write_row` step (1) (:1160-1174) purges unreadable rows for the same hyhash+chartmode when that chart+mode is written again, and `delete_auto_results` (:1269-1310) ran once. That is all. There is no `DELETE FROM songmeta` or `DELETE FROM dynamics` anywhere in src (grep :1148, :1154, :1254, :1296-1298, :1882 are the only DELETEs). A chart removed from disk keeps its results, nodes, songmeta and dynamics rows forever; a chartmode you stop using keeps its old rows forever.

The old `records` table is not in the installed DB: sqlite_master lists only the eight tables above and their indexes. Code never reads it (record_store.h:359, record_store.cpp:789-792, ADR 0009:83-90); tests/test_store.cpp:1226-1250 creates one only to prove it is left alone. The `songlength` table is dropped at every open (:783). `results_before_upgrade` is transient (:824-830).

## Open questions for the user

1. Should the Paths tab show ms and measures for a stored result when the chart file is gone or has changed? Yes keeps the tempomap (a compact encoding would cut its 29.7 MB about 3x). No lets it go: the click parses the file through `load_songpath`, the one owner, at about 1 ms typical.
2. Should reports and DMBot keep listing charts no longer in the library (D77 counts them)? Yes keeps songmeta's names. No makes them a JOIN to the charts naming copy, and the three columns go.
3. Should anything ever delete the rows of a chart that left the library, or of a chartmode no longer used? Today nothing does. On scan, or never?
4. Do you ever want two settings' results kept side by side for one chart? Never: nodes inline in the structure blob is the simpler scheme and removes 37 MB plus the GC. Yes: integer ids and 16-byte hashes keep sharing at about 6 MB.
5. charts.folder: keep the 1.4 MB, or derive it at list time from path plus the scan root in settings (a root removed from settings would lose its label)?
6. dynamics: keep the 4.4 MB cache, which saves one parse per tab open, or always count on open? And is the (difficulty, pro) key worth keeping when only Expert pro is ever filled?

## What I grepped (a miss is not proof of absence)

`get_timing|read_tempomap|decode_tempomap|tempomap` over src, tools, tests; `get_record\(|for_each_blob\(|list_records\(|get_summaries\(|analyzed_hashes\(` over src outside store; `\.ref_name|\.rootfolder|\.sig\b|get_dynamics\(|library_copies\(|song_length_ms` over src; `DELETE FROM (songmeta|dynamics|results|charts|paths|path_refs|meta)` over src; `FROM (songmeta|charts|...)` over tools (one hit, tools/test_compare_db.py:124, a test helper); `path_refs_by_node|phash NOT IN|WHERE hyhash=\? AND chartmode=\?` over src, tests, docs; `records` table references over the repo; CONTEXT.md for songmeta/tempomap/sqlite/RecordStore (no matches, so CONTEXT.md does not describe the store).
