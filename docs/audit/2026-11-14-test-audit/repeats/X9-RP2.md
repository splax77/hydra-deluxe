agent abea1aa76f54d2ce0 (reader T5-X9-RP2, K3 rows of RP2, rows 70 to 138; session 63c70014-4328-49dc-9205-742235732752)

How I worked: I printed the asserted line of every place of each K3 row from `repeats.tsv`, then opened the code around every place that could feed engine output into app, ui, store or report code, and read the production function it feeds in `src/`. I ran nothing. Verdicts: X9 (made-up stand-in), sound, snapshot (expected value pinned from one run of the engine; `docs/agents/test-rubric.md:23`), echo (typed in, same thing out), unclear. I found no echo and nothing unclear in these rows. A snapshot is marked only where the test's own comment says "from one run", "pinned" or "the engine's record"; I did not hunt for unmarked ones. Other numbers on built charts (ticks in rows 81, 105, 109, 127, 128) are called sound because the test states a rule or places the tick itself, not because I proved every expectation.

## Table

| Row | Value | Verdict | Reason | Function fed | Places opened |
|---|---|---|---|---|---|
| 70 | 5000 | X9 (app_library); rest sound | `save_scored(*app, 3, 5000)` stores a typed score and the test expects "5,000" on the library row. The other places are a lens depth the user types (config, store lens, path_list:303) or a percent computed by a rule (`dm_report:491`: optimal/2 gives 5000) | library row score text (`LibraryModel` best label) | test_app_library:398-406,550; test_config:641; test_dm_report:480-491; test_path_list:303; test_store_results:202 |
| 71 | 130 | X9 (overlay_layout:340); snapshot (test_replay:1536); rest sound | `wrap_words("  3- 1 2  (optimal)", 130)` is the known path label. `test_replay:1536` is in the case commented at :1516 "pinned from one run". `test_model:37` is a ChordNote rule; `test_stars:259` is the star table's own test; `test_replay:1436` states "(50 + 15) x 2" | `wrap_words` (overlay layout) | test_overlay_layout:340; test_replay:1436,1516,1536; test_model:37; test_stars:259; test_column_widths:342 |
| 72 | -10.0 | X9 (activation_rows:805); rest sound | A hand-built `Activation` with `e_offset = 10.0` goes to `badge_texts`. `test_model:519` tests the Activation's own rule on the field it sets. `test_pb_coach:196` passes -10.0 as the user's limit. `test_preview_boxes` and `test_replay:409` pass a ms to an engine function | `app::activation_badges` (`src/app/path_view.cpp:69`); engine owner `Activation::badge_timings` | test_activation_rows:790-806; test_model:510-519; test_pb_coach:190-196; test_preview_boxes:249; test_replay:409 |
| 74 | .chart | sound | A file suffix the test asks the corpus for, or a temp name | none | test_analysis:1053; test_app_click:1037; test_section_names:62; test_srb:370; test_winstr:196 |
| 76 | Bloodline | sound | Charter text is chart-file metadata handed to the library; the leaderboard names at `dm_report:386` come from DMBot, outside the engine; `library_query:163` and `song_input:353` strip tags from a typed title | `display_title`, `strip_rich_tags`, `collect_dm_rows` | test_dm_report:360-386; test_library_model:180,860; test_library_query:163; test_song_input:353 |
| 77 | \\notes.mid | sound | A file the test writes in its temp dir | none | test_analysis:1332; test_app_click:847; test_long_paths:107,127; test_pb_state:88; test_sng:255 |
| 78 | 6000.0 | sound | ms on a hand-built song and scene (`build_preview_scene` on a built song, `sp_meter_bars_at` on a built curve); expectations follow a stated tempo ("120 BPM until tick 5760") | `build_preview_scene`, `drain_box_at`, `sp_meter_bars_at` | test_preview_boxes:265,597-622; test_preview_scene:421; test_sp_meter_curve:82,323 |
| 79 | 8000.0 | sound | Same. `test_replay:789` reads a field of replay output on a built chart | same; `tools/replay.cpp` | test_preview_boxes:839,890; test_sp_meter_curve:113,298; test_replay:789 |
| 80 | 2880 | sound | Ticks of hand-built songs, engine run on them | `song_with_chords`, `measure_lines` | test_generated_fills:151; test_preview_boxes:184; test_timing:151 |
| 81 | 13440 | snapshot (test_sp_meter_curve:465,466); rest sound | `beat_song({}, {}, 13440)` is the test's own chart. `test_sp_meter_curve:461` says "The engine's record: the squeezed-out chord is 12960", so :465-466 copy the engine's answer. The `test_sp_end` places state the tick they placed | `Activation::deact_tick` (engine itself) | record_fixtures.h:509; test_sp_end:216,485,813; test_sp_meter_curve:461-466; test_model:1296 |
| 82 | 11520 | sound | End tick the test passes to `sp_act_at`; `test_tied_variants` uses a built song. `test_replay:529` reads a hand-typed path JSON (see row 129) | `sp_act_at`, `windows_from_json` | test_preview_scene:419; test_sp_meter_curve:92-104; test_tied_variants:108; test_replay:508-529 |
| 83 | 12.6 | X9 (activation_rows); rest sound | A hand-built `SPSqueeze{SqIn, 12.6}` on an `Activation`, given to the badge and path-button code. `test_model:132,385-407` test `Activation`'s own notation and hardest rule on the fields it sets; `test_display_format:60` is a rounding check | `app::activation_badges`, path button timing text | test_activation_rows:725-750,910-921; test_model:132,385-407,509; test_display_format:60; test_app_click:1048 |
| 84 | 0.01 | sound | Float tolerances. `test_report_rows:797` is a tier cutoff plus 0.01 | `report::tier_for` | test_audio_decode:114; test_highway_draw:84; test_obj_loader:173; test_report_rows:797 |
| 85 | 250.0 | sound | ms from a hand-built chart or an input | `build_preview_scene`, transport | test_preview_scene:53; test_preview_source:300; test_sp_end:243 |
| 86 | 30.0 | X9 (activation_rows:814-837); rest sound | Hand-set `e_offset = -30.0` and `SqOut -5.0` fed to the badge code. Timeouts and clocks are inputs. `test_model:1243` is the Activation's own tie rule on a typed list | `app::activation_badges` | test_activation_rows:810-837; test_library_jobs:142; test_live_protocol:15; test_model:1243 |
| 87 | 170 | X9 (overlay_layout:344,472,505; report_view:423); column widths sound | Overlay: the known path label. `report_view:423` builds `path_row("Halo","Beyonce","Someone","1",100,500.0,false)`, a hand-typed report row with a made-up squeeze, and checks "Beyond 170 ms". Column widths are layout input. `test_report_rows:468` is the K2 text part, not mine | `wrap_words`, `ellipsize`; the Beyond column cell of the report view | test_overlay_layout:344,472,505; test_report_view:410-423,447; test_report_rows:468; test_column_widths:146 |
| 88 | 12.0 | X9 (library_model); rest sound | `ready(500000, "2 2", 7, 12.0)` is a made-up stored summary (score, path text, stars, squeeze ms) given to the library. `test_library_jobs:190` reads a clock. `squeeze_rating:455` is a difficulty rule on a built Activation | `LibraryModel::set_summary_for` and `set_summaries` (`src/ui/library_model.cpp:273,282`); owner `store::RecordLookup` | test_library_model:566,589,608,618,816; test_library_jobs:190; test_squeeze_rating:455 |
| 89 (K3 part) | Expert Pro Drums, 2x Bass | X9 | `testdm::kMode` is typed in `dm_fixture.h:26` and stored with DM records; `test_report_rows:891,894` type it into `nothing_under_settings`. The real label is made by `app::chartmode_key` (`src/app/config.cpp:331`) | `report::nothing_under_settings` (`src/app/report.cpp:271`); the record store save | dm_fixture.h:20-28; test_report_rows:880-894 |
| 90 | 1100.0 | sound | Song length of a built chart | `song_length_ms` | test_app_click:1027; test_preview_controller:299; test_song_length:123 |
| 91 | 900 | sound | Screen sizes, ticks, an ini value the user types | `report_placement`, `load_ini_text` | test_app_shell:153; test_config:372; test_preview_timeline:112 |
| 92 | main...feature | sound | A git range of the test's own repo | `resolve_range` | test_precheck:384; test_precheck_git:91 |
| 94 | 100000 | sound | `test_model:688` is `pathstring_verbose` on a hand-built Path with the rule in a comment; `test_stars:222` is the star table's own test; `test_live_protocol` a timeout | `Path::pathstring_verbose` (engine itself) | test_model:676-697,1318; test_stars:222; test_live_protocol:24 |
| 95 | -50.0 | sound | Engine functions called on a typed ms (`backend_row_value`, `word`, `press_room_ms`) | `backend_row_value`, `squeeze_rating::word` (engine) | test_model:207,881; test_squeeze_rating:959; test_replay_schedule:1070 |
| 97 | -12.0 | X9 (activation_rows:760,877,881; library_query:353); rest sound | `user_row()` is a hand-built Activation with `SqIn -12.0` handed to the badge code. `analyzed(7, -12.0)` is a made-up row fact for the library search. `test_model:490` is the Activation's own rule | `app::activation_badges`; `query_matches` | test_activation_rows:696-704,760,877; test_library_query:30,353; test_model:485-490; test_highway_draw:45 |
| 98 | 12345 | sound | A count of charts handed to text functions, not a value computed from a chart | `counted`, `batch_cant_analyze_text` | test_batch_text:22,32; the other places |
| 99 | 1000000 | X9 (fill_report:128,248); rest sound | `put_ch10(store, kBoth, 1000000, 3, "old-path-A")` writes made-up scores and path strings into the store, which the fill comparison ranks. `test_midi:524` is a message limit, `test_stars:341` the star table, `precheck` numbers are text | fill comparison (`src/app/fill_report.cpp`) reading the record store | test_fill_report:120-131,248; test_midi:524; test_precheck_check3:106; test_stars:341 |
| 100 | 130.4 | snapshot | Real chart ("White Washed"); the comments say "From one run", "Pinned from one run". The number is the engine's squeeze-out typed into the expectation, and copies a real result. `test_model:1013` is sound (built SPSqueeze at :992) | `tight_window_note`, `coach_list` (`src/pathbuilder/pb_coach.cpp`), `step_short`, `end_choice_text` (`pb_text.cpp:98`) | test_pb_coach:140-163,258-273; test_pb_text:140-177; pb_ui/pbtest_coach:176-183; test_model:1013 |
| 102 | 12000 | X9 (preview_boxes:795); rest sound | A hand-set `PreviewSoloNotes` with `bonus = 12000` on an empty scene, not one made by `build_preview_scene`. The audio frame counts are inputs | `solo_box_at`; owner `build_preview_scene` | test_preview_boxes:785-795; test_preview_load_progress:345; test_song_audio:131; test_stream_mix:361 |
| 104 | 180 | snapshot (test_replay:1535); rest sound | `test_replay:1535` straddle.notes, case commented at :1516 "pinned from one run". Scene scores in `test_preview_boxes` come from `build_preview_scene` on a built song. Widths are layout | `build_preview_scene`, `score_box_at` | test_replay:1516,1535; test_preview_boxes:262-294,527; test_column_widths:286; test_preview_timeline:574 |
| 105 | 6912 | sound | Hand-set `sp_end_steps` on an Activation with the rule in a comment (C1/C2); the other places are the engine on a built song | `Activation::end_anchor_tick` (engine itself) | test_model:600-613; test_sp_end:134,715; test_sp_end_clamp:333; test_sp_meter_curve:250 |
| 107 | 1200.0 | sound | ms inputs of built charts | `song_length_ms`, transport | test_preview_transport:93; test_replay_schedule:529; test_song_length:159; test_sp_end_clamp:285 |
| 109 | 5760 | sound | Ticks of built songs; `kMaxFrame` | engine functions | test_sp_end:249; test_sp_meter_curve:315; test_stem_reader:105; test_tied_variants:419 |
| 110 | -315.8 | X9 (activation_rows:768,798; test_report_rows:360; test_store_results:641,645); test_model:1012 sound | `record_with_sqin(-315.8)` builds a hand-made record saved to the store and read back through the report and record rows. `free_in` is a hand-built squeeze handed to the badge code | `app::activation_badges`; `record_of` and `RecordStore::lookup`; report rows | test_activation_rows:768,798; test_model:1012; test_report_rows:352-360; test_store_results:636-645 |
| 111 | 111 | sound | A DM user id the test types as the setting and in the DMBot reply, both outside the engine | settings save and load, DM user parse | test_app_reports:491,509; test_dm_report:130; test_utf8_paths:45; uitest_batch_reports:261 |
| 112 | "111" | sound | Same as row 111 | same | same |
| 114 (K3 part) | 140 | sound | `test_song_parse:666` is a tick of a hand-written lane chart; the other places are K1 (X1) | chart parse | test_song_parse:666 |
| 115 | 1500 | sound | A screen rect, an elo in a DMBot reply, a tick, a string length; all input | `report_placement`, DM parse | test_app_shell:153; test_dm_report:132; test_song_parse:329; uitest_live:616 |
| 116 | sine220.opus | sound | The audio fixture file the tests ship | decoder | test_audio_decode:60; test_long_paths:88; test_preview_load_progress:130; test_stem_reader:333 |
| 120 (K3 part) | 163.0 ms | X9 (weak) | `format_ms(163.0)` is Burnout's real squeeze figure typed as a stand-in into the formatter of engine ms. The function takes any double, so the harm is a second copy of 163.0 more than a wrong test | `format_ms` (`src/core/model.cpp:937`) | test_activation_rows:570-579 |
| 121 | 0.05 | sound | Tolerances, a scroll fraction, the default highway config | scrollbar, config | test_audio_mixer:98; test_obj_loader:183; test_preview_config:62; test_timeline_view:135 |
| 123 | 50000 | X9 (fill_report:131); rest sound | `delta == 50000` is the difference of the made-up scores in row 99, the same instance. `test_model:1315` is `bank_arrivals` on a hand-built path with a stated rule; `test_stars:222` the star table | fill comparison | test_fill_report:120-131; test_model:1305-1315; test_stars:222; test_timing:307 |
| 125 | 30000.0 | sound | ms of built timelines | timeline functions | test_preview_clock:40; test_preview_timeline:282; test_timeline_view:61; uitest_preview:866 |
| 126 (K3 part) | Acid Romance | sound | `chart("acid", "Acid Romance", ...)` is a library entry built from chart metadata (a title), the input a scan gives the library. The made-up summaries in the same file are row 88 | `LibraryModel` sort and filter | test_library_model:60-100,174,316,482,543,617,824 |
| 127 | 5376 | sound | Ticks of hand-built songs; `test_sp_end_clamp` states the rule (3456 against 5376 is -250 ms at 2400 BPM) | `extension_of`, `Activation::squeeze_end_tick` (engine) | test_model:619; test_sp_end:94,509; test_sp_end_clamp:100-123 |
| 128 | 3456 | sound | `fill_length = 3456` is the fixture's own chart note (`record_fixtures.h:114`); `uitest_report_windows:272` reads "3,456" off the screen on that built chart | `extension_of`, report window | record_fixtures.h:114; test_sp_end_clamp:102,106; uitest_report_windows:262-272 |
| 129 | activations | sound for the key reads; X9 for test_replay:508-529 | The key is the replay tool's own output, read by tests of that tool (writers `tools/replay.cpp:484`, `tools/replay_json.cpp:187`). But `test_replay:508` types a path JSON by hand, shaped like `paths_json` output, to test `windows_from_json`. It also covers older dumps, which cannot be made from a chart now, so this one is a judgement call | `windows_from_json` (`tools/replay_json.cpp`); owner `paths_json` | test_cli:702; test_replay:504-529,670; test_replay_schedule:468 |
| 130 | 12000.0 | sound | Deact ms of a built chart and a playhead input; `test_cli:707` reads the tool's `deact_ms` on a built chart | `sp_meter_bars_at`, replay tool | test_cli:707; test_sp_meter_curve:105; uitest_preview:289 |
| 132 | notes.chart | sound | A file the test creates; `test_srb:259` passes through metadata it made | `srb_parse_metadata` | test_cli:586; test_five_lane:323; test_srb:259,298 |
| 133 | 1080 | sound | Screen sizes and ticks | `track_rect`, `report_placement` | test_app_shell:37; test_overlay_layout:33; test_preview_scene:369 |
| 135 | 160 | sound | Widths, image sizes, sample counts | layout, renderer | test_column_widths:199; test_preview_renderer:94; test_stream_mix:215 |
| 136 | 9000.0 | sound | ms on a scene built from a hand-built song and path | `drain_box_at`, `next_act_box_at` | test_preview_boxes:176,640-654,914,965; test_sp_meter_curve:403 |
| 137 | 7000.0 | sound | Same; "x2 combo 15" follows from the notes the test placed (its comments) | `score_box_at` | test_preview_boxes:355-380; test_preview_timeline:403; test_sp_meter_curve:324 |
| 138 | 120.0 | sound | BPM of built tempo maps | `SongTiming`, `build_preview_scene` | test_fill_deadline:130; test_preview_scene:398; test_replay:737 |

Rows not in the table were K1, K2 or K4 in RP2 (73, 75, 89 except its K3 part, 93, 96, 101, 103, 106, 108, 113, 117 to 119, 122, 124, 131, 134).

## X9 instances, grouped by the function they feed

`app::activation_badges` (`src/app/path_view.cpp:69`; engine owner `Activation::badge_timings`, and the search that fills `sqinouts`). The test builds an `Activation` by hand and types fill or squeeze ms.
- test_activation_rows.cpp:805 (e_offset 10.0 gives "early fill -10.0 ms")
- test_activation_rows.cpp:749,910,912,921 (squeeze 12.6)
- test_activation_rows.cpp:814-837 (30.0, -5.0)
- test_activation_rows.cpp:760,877,881 (`user_row`, -12.0)
- test_activation_rows.cpp:768,798 (-315.8; -163.0 at :745-800 is Burnout's real figure)

`LibraryModel::set_summary_for` and `set_summaries` (`src/ui/library_model.cpp:273,282`; engine owner the stored `RecordLookup`).
- test_library_model.cpp:566,589,608,618,816: `ready(500000,"2 2",7,12.0)` and `ready(378315,"3- 1 2",7,163.0)`. The 378315, "3- 1 2" and 163.0 copy Burnout's real result.

Library search `query_matches`.
- test_library_query.cpp:353 `analyzed(7, -12.0)`; :332-340 `analyzed(7, 163.0)` with the comment "Burnout's real 163.0 ms" (already in X9).

Library row score text.
- test_app_library.cpp:406,550: `save_scored(*app, 3, 5000)`, a typed score read back as "5,000".

Store and report rows (`RecordStore::lookup`, `record_of`, report rows).
- test_store_results.cpp:641,645 and test_report_rows.cpp:360: `record_with_sqin(-315.8)` saved and read back as `hardest_ms`.

`report::nothing_under_settings` (`src/app/report.cpp:271`) and the DM record save; owner `app::chartmode_key` (`src/app/config.cpp:331`).
- dm_fixture.h:26 (`kMode`); test_report_rows.cpp:891,894.

Fill comparison (`src/app/fill_report.cpp`).
- test_fill_report.cpp:128,131,248: made-up scores 1000000 and 1050000 and path strings "old-path-A", "new-path-A" put into the store.

`wrap_words` and `ellipsize` (overlay layout).
- test_overlay_layout.cpp:340,344,472,505: "3- 1 2  (optimal)" (already in X9; label rule at `preview_view.cpp:823`).

Beyond column cell of the report view.
- test_report_view.cpp:423: `path_row(..., 100, 500.0, false)`, a hand-typed report row.

`solo_box_at` (preview boxes); owner `build_preview_scene`.
- test_preview_boxes.cpp:785-795: a hand-set `PreviewSoloNotes` with `bonus = 12000`.

`format_ms` (`src/core/model.cpp:937`).
- test_activation_rows.cpp:579: `format_ms(163.0)`, Burnout's real figure (weak).

`windows_from_json` (`tools/replay_json.cpp`); owner `paths_json` in the same file.
- test_replay.cpp:508-529: a hand-typed path JSON (judgement call, see row 129).

## Snapshot instances (not X9)

- test_pb_coach.cpp:163,174,273; test_pb_text.cpp:159,177; pb_ui/pbtest_coach.cpp:182: 130.4 ms, "From one run" or "Pinned from one run".
- test_replay.cpp:1535,1536: 180 notes and combo 130, case commented at :1516 "pinned from one run".
- test_sp_meter_curve.cpp:465,466: 13440 and 14400, comment at :461 "The engine's record".

## Counts

Lines in the table: 54 (the K3 rows, plus the K3 parts of rows 89, 114, 120, 126).
- Rows with at least one X9 place: 15 (70, 71, 72, 83, 86, 87, 88, 89, 97, 99, 102, 110, 120, 123, 129).
- Rows with a snapshot place: 4 (71, 81, 100, 104).
- Echo: 0.
- Unclear: 0.
- Sound only: 35 lines (74, 76 to 80, 82, 84, 85, 90 to 92, 94, 95, 98, 105, 107, 109, 111, 112, 114, 115, 116, 121, 125 to 128, 130, 132, 133, 135 to 138).

Rows not reached: none. Limits: places I read only as the asserted line (the many ms and tick inputs in rows 78, 79, 80, 85, 105, 109, 127 to 138) are judged from the sampled contexts and the stated rules around them. I did not search `test_sp_end.cpp` or `test_sp_end_clamp.cpp` for unmarked pinned numbers.
