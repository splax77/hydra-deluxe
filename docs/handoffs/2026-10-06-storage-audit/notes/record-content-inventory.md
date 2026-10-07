# Record-content storage audit (agent a88499ba1b00cfecd, read-only)

Scope: the path-node payload, the structure blob and the `results` row. I read the codec, the engine's copy-out, every reader I could find by grepping field names and accessor names across src/, tools/ and tests/, and decoded the installed DB copy with a scratch script (`...\scratchpad\audit1\measure_structure.py`). Nothing in the repo was touched.

## The short version

The record stores three kinds of thing. First, the player's choices: where to activate, which phrase to squeeze in or out, and which tied variants exist. Those have to stay. Second, engine facts about a chosen path: SP-end history, bank arrivals, passed fills, early-fill offset, score totals, backend rows. The engine can regenerate every one of them, but only by re-running the search pinned to the path's activation ticks (`search_target`, src/search/pather.h:86-88), which builds the whole graph again. Third, caches and duplicates: the summary columns (needed for the library list), and a handful of fields stored twice or never read.

The one clean win under the project's derive-once rule is the transfer scales (about 7.9 MB). They are already computed by one owner function, `frontend_transfer_scales(act, timing)` (src/core/squeeze_rating.h:65-66), from the stored SP-end steps plus the song's timing, and `get_record` already hands the display that timing (src/store/record_store.h:220). Storing them is a pure cache of a call the display could make.

The big block, backend rows (90 MB, 63%), is also regenerable, but not without the graph. Each row is a chart chord within 500 ms of the SP end D, its SP value, and its ms offset from D (src/search/graph.cpp:249-267). D is stored. The chord at a tick is a chart fact. The value is `category_scores` (graph.cpp:126,139). The offset is `offset_from_sp_end(note_ms, D_ms)` (graph.cpp:264-265; src/core/model.h:114-116). Today no entry point rebuilds those rows for a stored path except a full `search_target` run.

Two fields are dead: `frontend_points` (written on every activation, read by nothing outside the engine and codec; 1.7 MB) and `sp_cap_converged` (always true; src/core/model.h:772-775). Three facts are stored twice: `ms_limit` and `sp_cap` sit in the structure blob and in the key columns (`prepare_row` checks they agree, record_store.cpp:633-653); `notecount` is stored once per root (79,933 times, 320 KB) and once as a summary column, yet it is one chart fact (engine.cpp:1846-1848); `rules_fp` is copied out of the structure head so the UNIQUE key can hold it (record_store.cpp:229-233).

## DB facts I measured (installed copy, 19,106 results)

18,811 rows are current (`2.1.0+allzero`, format 7). 295 rows are stale in formats 1, 2, 3 and 6 (hyversion 1.6.2 to 1.8.2); their blobs are 0.9 MB and are never decoded (`rank_row`, record_store.cpp:409-421). All rows are cap 4; all 18,811 current rows have an ms limit set. Structure blobs total 8.3 MB (avg 437 B): root score totals + notecount 4.16 MB, node hashes 1.5 MB, variant counts 0.38 MB, root trailing banks 0.38 MB, ms_limit/sp_cap/converged 0.28 MB, multsqueezes 0.24 MB, heads 0.23 MB, variant var_point + trailing 0.13 MB. The nodes hold 428,710 activations, 2,604,998 backend rows, 36,494 SqIns, 16,183 squeeze-outs, 633,810 SP-end steps (428,710 Activation, 160,554 Collected, 8,052 Clamped, 36,494 SqIn), 1,098,001 bank arrivals, 527,431 passed fills. 243,778 activations (57%) have only the Activation step. 205,688 (48%) passed no fill. 411,043 e_offsets (96%) are above the 60 ms window, where the only thing read off them is "not E". No stored transfer scale is unknown (D4 holds); 194,541 post scales (45%) and 19,952 SqIn scales (55%) are exactly x1.00/x1.00. 14,006 variants hang off 67,923 roots plus 12,010 all-0 roots. Results columns: bestpath 232 KB, hyhash 611 KB, chartmode 478 KB, hyversion 246 KB, rules_fp 153 KB, avgmult 303 KB, score 116 KB; hardest_ms is non-null on only 3,103 rows.

## Table

Bytes are 428,710 activations x field width unless stated. Writer W, readers R. Classes: A choice/identity, B engine fact regenerable by the engine's own code, C needed by the library list without opening a song, D unused or duplicated.

| field | bytes | writer | readers | class | how to regenerate |
|---|---|---|---|---|---|
| act timecode (tick) | 3.4 MB | engine.cpp:1857; codec path_codec.cpp:122 | every display, pathstring, path_identity model.cpp:620-630, preview_view.cpp:340-380, replay.cpp:231-250 | A | none: it is the choice |
| act chord (code str) | ~3.9 MB | engine.cpp:1860 (node->chord) | path_view.cpp:287 (rowstr) | B (chart fact) | Song chord at the tick; no engine rule involved |
| frontend_points | 1.7 MB | engine.cpp:1865; codec :124 | none outside engine/codec (grep src, tools, tests: only engine.cpp:67,168,927, graph.h:158, model.h:480) | D | drop; engine recomputes from graph edge |
| backend rows: tick | 20.8 MB | graph.cpp:252 via engine.cpp:1868/1880 | path_view.cpp:363-430, squeeze_rating.cpp:108-124, replay.cpp:247-248 (sqout row), model.cpp:774-821 | B | graph add_deact_edge window rule (graph.cpp:259-267; ADR 0011 finding 48) from stored D + Song |
| backend rows: chord | 23.4 MB | graph.cpp:253 | path_view.cpp:369-430 (row label) | B (chart fact) | Song chord at the row's tick |
| backend rows: points, sqout_points | 20.8 MB | graph.cpp:139,254-255 (category_scores) | path_view.cpp:149,406,422 via core/backend_value.h:47-67 | B | category_scores(chord, combo, rules) needs the combo at that chord: a graph walk |
| backend rows: offset_ms (opt f64) | 23.4 MB | graph.cpp:264-265; tail rows engine.cpp:1874-1882 | model.cpp:817 (display_backends), path_view.cpp:363-430, backend_value.h | B/D | offset_from_sp_end(timing.ms(row tick), timing.ms(deact_tick)): same function, both ticks stored. Exact to the bit if the same SongTiming is used |
| SqIn offset_ms (f64) | 0.3 MB | engine.cpp:1898 from OutSq (graph squeeze offer) | hardest/difficulty model.cpp:473-530, notationstr, path_view.cpp:128-129, summary sqin_count | A+B | which phrase: choice (A). Offset: search_target only |
| SqIn transfer (opt scale) | 0.6 MB | engine.cpp:1945-1950 via frontend_transfer_scales | squeeze_rating.cpp:50-56 (stored_transfer_scales) -> path_view.cpp:300-345 | B | frontend_transfer_scales(act, timing): same owner, timing already in RecordLookup |
| e_offset (f64) | 3.4 MB | engine.cpp:1866 from fill_e_offset(deadline, sp_ready_ms) engine.cpp:764,891,1579-1581 | is_e0/is_E0/e_difficulty model.cpp:460-467, hardest model.cpp:487-493, report.cpp:306, path_view.cpp:292-297 | B | search_target only: needs the search's SP-ready time |
| transfer_post (opt scale) | 7.3 MB | engine.cpp:1946 | squeeze_rating.cpp:50-52 -> path_view.cpp:300-345 | B | same as SqIn transfer |
| sqout_tick (opt i64) | 0.56 MB | engine.cpp:1912 set_sqout(oa.sqout_phrase) | model.cpp:774-805, replay.cpp:247-248, replay_json.cpp:122, backend_value.h:21 | A | choice of the SqOut branch (ADR 0014) |
| sp_end_steps | 12.5 MB | engine.cpp:1930-1931 (emit_ends engine.cpp:628-637) | every SP-end accessor model.cpp:692-770, squeeze_rating.cpp:29-40, preview_view.cpp:362-364, replay.cpp:231-245, replay_json.cpp:99-123 | A (SqIn steps) + B (Collected/Clamped: extend_deacts graph.cpp:279-301) | search_target; ADR 0011/0021 forbid any other re-derivation |
| bank_rise_ticks | 10.5 MB | engine.cpp:1861 (emit_ticks banks_) | sp_meter() model.h:520 -> path_view.cpp:284-285, notationstr; preview_view.cpp:137,361 | B | search_target only; replay.h:292-301 records that a simulation got 37/1486 meters wrong |
| skipped_fill_ticks | 5.9 MB | engine.cpp:1863 | skips() -> notationstr model.cpp:541, is_allzero :823, summary maxskip; preview_view.cpp:360,407 | B | search_target only; same note: 224/1486 skip counts wrong by simulation |
| structure head: format u32 | 75 KB | path_codec.cpp:395 | rank_row record_store.cpp:409-421 (SQL substr too, :440-451) | C (Ready check) | none |
| rules_fingerprint u64 | 150 KB | pather stamps; path_codec.cpp:396 | rank_row; rules_fp_of SQL | C | none (ADR 0014) |
| ms_limit, sp_cap, sp_cap_converged | 0.28 MB | path_codec.cpp:397-399 | path_view.cpp:201-207 (Details header); converged: nobody | D | key columns ms_enabled/ms_value/sp_cap hold the same values (record_store.cpp:633-653); converged is always true |
| multsqueezes | 0.24 MB | pather.cpp:19 from graph.cpp:128-129,245-247 | path_view.cpp:218-241, pathstring_verbose model.cpp:584-595, paths_tab.cpp:517 | B (chart fact) | ScoreGraph build (MultSqueeze::applies per chord, graph.cpp:128) |
| node hash x refs | 1.5 MB | path_codec.cpp:291-298 | read_tree_entry :312-319 | A (identity) | none |
| variant var_point (opt i32) | ~0.1 MB | engine.cpp:1206,1647,1965; codec :302 | prepare_variants model.cpp:641-657 | A | none (ADR 0022) |
| variant trailing bank | in 0.13 MB | engine.cpp:1849-1850; codec :306-307 | leftover_sp path_view.cpp:441-443, preview_view.cpp:377 | B | search_target (ADR 0022 lone_pricing guard) |
| root score totals (6 x i64) | 3.8 MB | engine.cpp:1840-1845; codec :256-261 | totalscore everywhere; path_view.cpp:467-473; stars.cpp:17,33; avg_mult model.cpp:835-840 | B | search_target (engine), or core/replay.h replay_path: a second derivation by design (replay.h:19-22), flagged |
| root notecount (i32) | 0.32 MB | engine.cpp:1848 (ScoreGraph::note_count); codec :262 | avg_mult via chart_base_score? no: path_view.cpp? only summary notecount (report.cpp:313, fill_report.cpp:221) | D (stored 79,933x for one chart fact) | Song note count |
| root trailing bank | 0.38 MB | engine.cpp:1849; codec :265-266 | path_view.cpp:441-443, preview_view.cpp:377 | B | search_target |
| allzero_paths | in structure | pather.cpp:54 search_allzero | path_view.cpp:496-507 (dedupe by path_identity) | A (second search's answer) | search_allzero re-run (cheap, pather.h:58-66) |
| tied_count | 0 | not stored; recounted path_codec.cpp:468-473 | tied_pathcount | — | already derived at load |
| results: hyhash, chartmode, sp_cap, ms_*, depth_*, legacy_fills | 1.1 MB | write_row record_store.cpp:1198-1216 | every lookup (lens_match :350-354) | C (key) | none |
| hyversion | 0.25 MB | prepare_row :665 | rank_row; replay.cpp:569 | C | none |
| rules_fp | 0.15 MB | write_row :1206 (substr of structure) | UNIQUE key only; rank_row reads the blob head | D (duplicate by design) | substr(structure) |
| bestpath | 0.23 MB | prepare_row :668 best_path_text :584-586; reindex :1724 | library_model.cpp:60-117, batch.cpp:260, get_summaries | C | best_path().pathstring() |
| score, actcount, maxskip, hardest_ms, avgmult, notecount, sqin_count, sqout_count, pathcount, stars | ~0.6 MB | summarize_record :590-628; reindex; fill_missing_stars | library_model.cpp:41,54-55,93,209-211 (sort/filter via library_query.h:89-98), fill_report.cpp:212-221, dm_report.cpp:245, details_panel.cpp:159, batch.cpp:258 | C | summarize_record(decoded record) |

## Class A: must stay

The activation ticks, the SqOut tick, which phrases were squeezed in (the SqIn steps and SqIn count), the variant tree with its var_points, the node hashes, and the all-0 roots. `search_target` takes only the activation ticks and returns every squeeze variant of that set (pather.h:73-79), so the squeeze choices are what pick the stored variant out of its answer. They cannot be dropped.

## Class B: regenerable by the engine's own code

Two sub-kinds. The first needs nothing but stored ticks and the song's timing: the transfer scales (`frontend_transfer_scales`, squeeze_rating.h:65-66, stamped through that very function at engine.cpp:1945-1950) and the backend `offset_ms` (`offset_from_sp_end` on two stored ticks, model.h:114-116). These reproduce bit-for-bit given the same `SongTiming`, which `get_record` already builds from songmeta (record_store.cpp:1447-1452; serialize.cpp:102-111). About 31 MB together. Caveat for the scales: model.h:555-559 says they are stored "so the details view never needs a SongTiming", and the D4 "unknown is a bug" guard (ADR 0021, path_view.cpp:314-318) would move from write time to load time.

The second needs the search graph: backend row lists and values, score totals, SP-end Collected/Clamped steps, bank arrivals, passed fills, e_offset, trailing banks, multsqueezes, notecount. The only engine entry point is `search_target(song, settings, act_ticks)` (pather.h:86-88; EngineOptions::target_act_ticks engine.h:47), used by `hydra_replay target` (tools/replay.cpp:603-668) and the tied-variant guard `lone_pricing` (tests/record_fixtures.h:336-365). It builds the full graph, so it costs about an analysis per open (graph + parse dominate per chart; memory note perf-exploration-2026-10-06). Reproduction caveats: it returns several roots and tied variants; `keep_target_paths` promotes folded variants (pather.h:90-100, D45); and the ADR 0022 guard found stored variants that no lone root reproduces ("tied_under_root", record_fixtures.h:302-313, 351-358). So a stored path is not always re-found as a root; tie-merge pruning changes which path leads. `core/replay.h` can re-price the six score totals from windows, but it is a parallel derivation ("display and tooling only", replay.h:19-22) and cannot give meter or skips (replay.h:292-301).

## Class C: the library needs them without a decode

The ten summary columns, bestpath, and the key columns. `get_summaries` reads only these plus the 12-byte blob head (record_store.cpp:1337-1363). `list_records` sorts in SQL on them (:1805-1820). The library filter reads stars and hardest_ms (library_model.cpp:41, library_query.h:89-98). The report does not use them: it re-summarizes every path from the decoded record (report.cpp:287-313).

## Class D: dead or duplicated

`frontend_points` (1.7 MB) has no reader. `sp_cap_converged` is always true. `ms_limit` and `sp_cap` are in both the blob and the key. `notecount` is one chart fact stored per root plus as a column. `rules_fp` is a copy of 8 bytes of the blob. 295 stale rows (0.9 MB) in formats 1-6 are never decoded; `write_row` only purges them when that chart is re-saved (record_store.cpp:1160-1174). Backend `chord` and `offset_ms` (47 MB) duplicate what the tick plus the Song and D already say.

## Surprises

`write_activation` stores `display_backends()` (path_codec.cpp:128), which filters to the 500 ms window plus the sqout row, so the stored rows are already the display's rows; the engine's fuller edge list is not kept. The squeeze-out is stored once as a tick and its SqOut entry is rebuilt from its row at read (path_codec.cpp:217-225). `e_offset` is a full f64 but 96% of values only ever answer "not E-critical" (is_e0 model.h:258-260). `hardest_ms` is null on 84% of rows (paths with nothing to time). The engine's `OutPath::tied_count` is checked against the recount and then not stored (engine.cpp:1986-1989; path_codec.cpp:468-473).

## Open questions for the user

1. Drop `frontend_points` and `sp_cap_converged` from the format (dead; 1.7 MB; one format bump, every row Stale once)?
2. Compute the transfer scales at load with `frontend_transfer_scales` and the record's timing instead of storing them (7.9 MB)? This keeps one owner but moves the D4 guard to load time.
3. Backend rows: (a) keep as is, (b) store tick + points only and look the chord and offset up from the Song and D at open (47 MB, no engine re-run, chord lookup is a chart fact), or (c) rebuild the whole row list via the graph (needs a graph build per open)?
4. Is a `search_target` run per song open acceptable to regenerate the graph-dependent facts (score totals, bank arrivals, passed fills, e_offset)? Given the tie-merge caveat above I would say no, but it is the user's call.
5. Remove the duplicated `ms_limit`/`sp_cap` from the blob and store `notecount` once per record?
6. Purge the 295 old-format rows on open?

Status file: C:\Users\Patrick\.claude\hooks\state\status\a88499ba1b00cfecd.md. Scratch: ...\scratchpad\audit1\measure_structure.py (the decoder, re-runnable with `py`). About 55 tool calls used; nothing broke.
