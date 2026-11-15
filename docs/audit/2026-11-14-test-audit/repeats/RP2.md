agent a0620b94ad66aea85 (sorter T5-RP2, rows 70 to 138)

How I worked: I printed the line text of the first place in up to five files per row (all places for the K1 and K2 rows were read from the row's own list), then read surrounding lines where the kind was not plain. Owners were found with `git grep` in `src/` (code of this worktree, HEAD 7babd1cc). I ran nothing. Rows with many files were judged from the sampled files. The `tests/` prefix is dropped below.

## Table

| Row | Value | Kind | Reason | Places opened |
|---|---|---|---|---|
| 70 | 5000 | K3 | Lens bounds, percent figures and a score the test hands in | test_app_library:406, test_config:641, test_dm_report:491, test_path_list:303, test_store_results:202 |
| 71 | 130 | K3 | Widths, a basescore of a built chart, a 50+15 dynamics sum the test sets | test_column_widths:342, test_model:37, test_overlay_layout:340, test_replay:1436, test_stars:259 |
| 72 | -10.0 | K3 | A timing the test feeds in; badge text is built from it | test_activation_rows:805, test_model:519, test_pb_coach:196, test_preview_boxes:249, test_replay:409 |
| 73 | 378,315 | K1 | Burnout's best score as screen text. Covered by X1 | test_path_list:278, pbtest_build:147,148, pbtest_window:604,613, uitest_details:1032,1057, uitest_paths:119,121 |
| 74 | .chart | K3 | Extension of a file the test creates or asks the corpus for | test_analysis:1053, test_app_click:1037, test_section_names:62, test_srb:370, test_winstr:196 |
| 75 | 85.0 | K2 | The hit window `kHitWindowMs` (`src/core/model.h:106`, also `kWindowBackMs` `src/core/note_window.h:33`) typed in config, note-window, schedule, tier and squeeze-rating tests. See RP2-2 | test_config:285, test_note_window:84, test_replay_schedule:281, test_report_rows:776, test_squeeze_rating:769 |
| 76 | Bloodline | K3 (+ K4) | Mostly a charter string the test builds. `uitest_library:663,669` read the charter of a corpus chart (K4: a field of the chart file) | test_dm_report:386, test_library_model:180, test_library_query:163, test_song_input:353, uitest_library:663 |
| 77 | \\notes.mid | K3 | File the test writes into its own temp dir | test_analysis:1332, test_app_click:847, test_long_paths:107, test_pb_state:88, test_sng:255 |
| 78 | 6000.0 | K3 | ms on hand-built charts and curves | test_preview_boxes:265, test_preview_scene:421, test_preview_timeline:80, test_sp_meter_curve:82 |
| 79 | 8000.0 | K3 | Same | test_preview_boxes:839, test_preview_timeline:146, test_replay:789, test_sp_meter_curve:113 |
| 80 | 2880 | K3 | Ticks of hand-built songs | test_generated_fills:151, test_preview_boxes:184, test_replay:1245, test_timing:151 |
| 81 | 13440 | K3 | Deactivation tick of a hand-built song (`beat_song`, `record_fixtures.h:509`) | record_fixtures.h:509, test_model:1296, test_sp_end:216, test_sp_meter_curve:465 |
| 82 | 11520 | K3 | Tick of a built song ("6000 ms is tick 11520", `test_preview_scene:416`) | test_preview_scene:419, test_replay:529, test_sp_meter_curve:104, test_tied_variants:108 |
| 83 | 12.6 | K3 | A squeeze ms the test feeds in; the display text is checked against it | test_activation_rows:749, test_app_click:1048, test_display_format:60, test_model:132 |
| 84 | 0.01 | K3 | Float tolerances | test_audio_decode:114, test_highway_draw:84, test_obj_loader:173, test_report_rows:797 |
| 85 | 250.0 | K3 | ms inputs of built charts | test_preview_scene:53, test_preview_source:300, test_preview_transport:151, test_sp_end:243 |
| 86 | 30.0 | K3 | ms inputs, timeouts, a clock | test_activation_rows:814, test_library_jobs:142, test_live_protocol:15, test_model:1243 |
| 87 | 170 | K3 (+ K2 part) | Widths in most. K2 part: the text "past the 170 ms window" / "Beyond 170 ms" at `test_report_rows:468`, `test_report_view:423,447` (see RP2-2) | test_column_widths:146, test_overlay_layout:344, test_report_rows:468, test_report_view:423 |
| 88 | 12.0 | K3 | Seconds, scores and a difficulty the test builds | test_library_jobs:190, test_library_model:566, test_overlay_layout:110, test_squeeze_rating:455 |
| 89 | Expert Pro Drums, 2x Bass | K2 | The chart-mode key `app::chartmode_key` builds (`src/app/config.cpp:331`). See RP2-3 | dm_fixture.h:26, test_report_rows:891, test_store_open:749, uitest_library:164 |
| 90 | 1100.0 | K3 | Song length of a built chart or a test input | test_app_click:1027, test_preview_controller:299, test_replay_schedule:656, test_song_length:123 |
| 91 | 900 | K3 | Screen sizes, ticks, ini values | test_app_shell:153, test_config:372, test_preview_timeline:112, test_replay_schedule:1669 |
| 92 | main...feature | K3 | A git range the test fixture builds | test_precheck:384, test_precheck_check3:140, test_precheck_git:91, test_precheck_rows:147 |
| 93 | 170.0 | K2 | `kFormulaScale` (`src/core/note_window.h:39`) and the Insane+ edge `nominal_budget_ms()` typed in tier tests. See RP2-2 | test_activation_rows:1119, test_note_window:74, test_report_rows:780, test_squeeze_rating:781 |
| 94 | 100000 | K3 | Inputs, a test's own totalscore comment, star table row | test_live_protocol:24, test_model:688, test_stars:222, test_stem_reader:357 |
| 95 | -50.0 | K3 | Timings the test feeds in | test_model:207, test_preview_boxes:269, test_replay_schedule:1070, test_squeeze_rating:959 |
| 96 | 0 4 1 | K1 | Burnout's second path. Covered by X1 | test_path_list:287,435, test_preview_scene:1001, uitest_paths:126,140,151,155, uitest_preview:763,768 |
| 97 | -12.0 | K3 | Inputs | test_activation_rows:760, test_highway_draw:45, test_library_query:353, test_model:490 |
| 98 | 12345 | K3 | Numbers the test formats | test_batch_text:22, test_column_widths:79, test_note_shuffle:185, test_timing:266 |
| 99 | 1000000 | K3 | Score inputs. `test_stars:341` pins "the game stops at 7", a star-table fact (stars_for_score, owner not opened) | test_fill_report:128, test_midi:524, test_precheck_check3:106, test_stars:341 |
| 100 | 130.4 | K3 | An `SPSqueeze{SqOut, -130.4}` the test builds (`test_model:992`); the note text is checked against it | test_model:1013, test_pb_coach:163, test_pb_text:159, pbtest_coach:182 |
| 101 | 378175 | K1 | Burnout's second best score. Covered by X1 | test_path_list:286, pbtest_build:167,193,297, pbtest_coach:527,534, uitest_paths:125,150 |
| 102 | 12000 | K3 | Audio frames and a solo bonus of built charts | test_preview_boxes:795, test_preview_load_progress:345, test_song_audio:131, test_stream_mix:361 |
| 103 | max_tied_paths | K2 | The rules key name (`src/core/rules.cpp:78`). See RP2-4 | test_app_click:882, test_rules:120, uitest_library:393, uitest_report_windows:393 |
| 104 | 180 | K3 | Widths, score totals, notes of built charts | test_column_widths:286, test_preview_boxes:294, test_preview_timeline:574, test_replay:1535 |
| 105 | 6912 | K3 | Ticks of built songs | test_model:610, test_sp_end:134, test_sp_end_clamp:333, test_sp_meter_curve:250 |
| 106 | 3- 1 2  (optimal) | K1 | Burnout's best path with the label suffix (`src/app/preview_view.cpp:823`). The path text is X1; the suffix is a second copy of the label rule. See RP2-1 | test_overlay_layout:344,472,505, test_path_list:434, test_preview_scene:998, uitest_preview:700 |
| 107 | 1200.0 | K3 | ms inputs of built charts | test_preview_transport:93, test_replay_schedule:529, test_song_length:159, test_sp_end_clamp:285 |
| 108 | Extreme | K2 | A tier name of `timing_tiers()` (`src/core/squeeze_rating.cpp:241`). See RP2-2. `test_report_rows:775` feeds 42.6 | test_report_rows:775, test_report_view:339, test_rules:226, test_squeeze_rating:766 |
| 109 | 5760 | K3 | Ticks of built songs; `kMaxFrame` in the stem test | test_sp_end:249, test_sp_meter_curve:315, test_stem_reader:105, test_tied_variants:419 |
| 110 | -315.8 | K3 | An `SPSqueeze{SqIn, -315.8}` the tests build (`test_activation_rows:713`) | test_activation_rows:768, test_model:1012, test_report_rows:360, test_store_results:641 |
| 111 | 111 | K3 | A DM user id the tests write and read back | test_app_reports:491, test_dm_report:130, test_utf8_paths:45, uitest_batch_reports:261 |
| 112 | "111" | K3 | Same as row 111 (string form) | same |
| 113 | 378,175 | K1 | Burnout's second best score as text. Covered by X1 | test_path_list:286, pbtest_build:193,297, pbtest_coach:527, uitest_paths:125,150 |
| 114 | 140 | K1 (+ K3) | Burnout's cost "−140" on screen (`pbtest_build:168,177`, `pbtest_coach:529,535`, `pbtest_window:203`). `test_song_parse:666` is a tick of a hand-written lane chart (K3). Covered by X1's "other Burnout figures" | test_song_parse:666, pbtest_build:168, pbtest_coach:529, pbtest_window:203 |
| 115 | 1500 | K3 | Rect sizes, an elo the fixture sets, a tick | test_app_shell:153, test_dm_report:132, test_song_parse:329, uitest_live:616 |
| 116 | sine220.opus | K3 | The audio fixture file the tests ship | test_audio_decode:60, test_long_paths:88, test_preview_load_progress:130, test_stem_reader:333 |
| 117 | activation | K2 | `failed_reason` string of the pather (`src/search/pather.cpp:366`; doc `pather.h:129`). See RP2-5. `test_pb_coach:108` passes it in as input | test_best_finish:132, test_path_costs:245, test_pb_coach:108, test_search_target:363 |
| 118 | 36480 | K1 | Burnout's first fill tick, typed as `kFirstFillTick`, `kFillTick`, `kEarliestTick`. Covered by X2 | test_fills_and_skips:453,454, test_pb_coach:209, pbtest_harness.h:60, pbtest_window:178 |
| 119 | Hard (uncounted) | K2 | Tier name plus the `" (uncounted)"` tag (`src/core/model.cpp:354`). See RP2-2 | test_activation_rows:562, test_model:937, test_rules:178, test_squeeze_rating:963 |
| 120 | 163.0 ms | Split: K1 + K3 | K1 part: Burnout's timing shown in the path list and details (`test_path_list:281`, `uitest_details:1071`, `uitest_paths:123`). K3 part: `test_activation_rows:579` feeds 163.0 to `format_ms` | test_activation_rows:579, test_path_list:281, uitest_details:1071, uitest_paths:123 |
| 121 | 0.05 | K3 | Tolerances, a camera offset, a scroll fraction | test_audio_mixer:98, test_obj_loader:183, test_preview_config:62, test_timeline_view:135 |
| 122 | Beyond | K2 | Tier name (`squeeze_rating.cpp:249`). See RP2-2. `test_column_widths:105` is a test-built cell | test_column_widths:105, test_report_rows:782, test_report_view:342, test_squeeze_rating:787 |
| 123 | 50000 | K3 | Scores and ticks the tests build; star table row | test_fill_report:131, test_model:1315, test_stars:222, test_timing:307 |
| 124 | cannot open file:  | K2 | Error text thrown at `src/audio/mapped_file.cpp:28`, `src/core/winstr.cpp:290`, `src/app/analysis.cpp:113`. See RP2-6 | test_midi:97, test_pb_state:92, test_winstr:202, pbtest_window:476 |
| 125 | 30000.0 | K3 | ms of built timelines | test_preview_clock:40, test_preview_timeline:282, test_timeline_view:61, uitest_preview:866 |
| 126 | Acid Romance | K4 | A corpus chart's own title as the library lists it. Three files only: `test_library_model` (about 15 places), `uitest_details`, `test_library_query:3,48` | test_library_model:174, uitest_details:258 |
| 127 | 5376 | K3 | Ticks of built songs | test_model:619, test_sp_end:94, test_sp_end_clamp:100 |
| 128 | 3456 | K3 | A fill length the fixture sets (`record_fixtures.h:114`), checked later | record_fixtures.h:114, test_sp_end_clamp:102, uitest_report_windows:272 |
| 129 | activations | Unclear | A JSON key of the replay/CLI output, used in test_cli and test_replay*. I did not open the writer in `src/` | test_cli:702, test_replay:670, test_replay_schedule:468 |
| 130 | 12000.0 | K3 | A deact ms of a built chart and a playhead position | test_cli:707, test_sp_meter_curve:105, uitest_preview:289 |
| 131 | 10.088 | K1 | Burnout's song length on screen ("2:10.088"), from `app::song_length_ms` (`src/app/song_length.h`, used at `src/app/analysis.cpp:817`). Covered by X1's Burnout figures | pbtest_build:199, pbtest_coach:302, pbtest_window:204 |
| 132 | notes.chart | K3 (+ K2 pin) | File the test creates. `test_srb:259` pins the name the reader reports (owner not opened) | test_cli:586, test_five_lane:323, test_srb:259 |
| 133 | 1080 | K3 | Screen heights and ticks | test_app_shell:37, test_overlay_layout:33, test_preview_scene:369 |
| 134 | Mutemath - Allies | K4 | The name of a corpus chart the tests look up | test_best_finish:95, test_path_costs:76, test_search_target:69 |
| 135 | 160 | K3 | Widths, image sizes, sample counts | test_column_widths:199, test_preview_renderer:94, test_stream_mix:215 |
| 136 | 9000.0 | K3 | ms on built charts | test_preview_boxes:176, test_preview_transport:297, test_sp_meter_curve:403 |
| 137 | 7000.0 | K3 | Same | test_preview_boxes:368, test_preview_timeline:403, test_sp_meter_curve:324 |
| 138 | 120.0 | K3 | BPM of built tempo maps | test_fill_deadline:130, test_preview_scene:398, test_replay:737 |

## Candidates

**RP2-1. Burnout's paths, costs and clock typed by hand (X1 and X2 already cover it).** Fact: Burnout's best score 378,315, second path "0 4 1" with 378,175, label "3- 1 2  (optimal)", timing 163.0 ms, cost −140, song length 2:10.088, first fill tick 36480. Owners: the search result (`corpus::burnout()` record), the path label `src/app/preview_view.cpp:823`, `app::song_length_ms`. A change to the engine moves every copy red at once. Copies beyond what X1 lists, by row: 378,315 (row 73) at test_path_list:278, pbtest_build:147,148, pbtest_window:604,613, uitest_details:1032,1057, uitest_paths:119,121. 378175 and 378,175 (rows 101, 113) at test_path_list:286, pbtest_build:167,193,297, pbtest_coach:527,534, uitest_paths:125,150. "0 4 1" (row 96) at test_path_list:287,435, test_preview_scene:1001, uitest_paths:126,140,151,155, uitest_preview:763,768. "(optimal)" label (row 106) at test_overlay_layout:344,472,505 (the label text is only the wrap input there, so K3 in that file), test_path_list:434, test_preview_scene:998, uitest_preview:700. 163.0 ms (row 120) at test_path_list:281, uitest_details:1071, uitest_paths:123. Cost −140 (row 114) at pbtest_build:168,177, pbtest_coach:529,535, pbtest_window:203. Clock 2:10.088 (row 131) at pbtest_build:199,236,242, pbtest_coach:302,322,414,417,425,439,510,555, pbtest_window:204. First fill tick 36480 (row 118; X2) at test_fills_and_skips:453,454, test_pb_coach:209, pbtest_harness.h:60, pbtest_window:178. New against X1: the song length 2:10.088 (not in X1's list) and the cost −140.

**RP2-2. The squeeze timing tiers and their edges typed in many tests.** Fact: tier names Normal to Beyond and "(uncounted)", edges at the hit window 85 ms and 170 ms. Owners: `timing_tiers()` `src/core/squeeze_rating.cpp:241` (names and edges from `kHitWindowMs` `src/core/model.h:106` and `nominal_budget_ms`), `src/core/model.cpp:354` for " (uncounted)", `kFormulaScale` `src/core/note_window.h:39`, `kWindowBackMs` `note_window.h:33`. A change to a name or edge turns all copies red at once. Copies: "Extreme" test_report_rows:775, test_report_view:339, test_rules:226, test_squeeze_rating:766,959,961; "Beyond" test_report_rows:782, test_report_view:342, test_squeeze_rating:787; "Hard (uncounted)" test_activation_rows:562, test_model:937, test_rules:178, test_squeeze_rating:963; 85.0 test_config:285,302, test_note_window:84,85, test_replay_schedule:281,355, test_report_rows:776, test_squeeze_rating:769; 170.0 test_activation_rows:1119,1121, test_note_window:74,77, test_report_rows:780,781, test_squeeze_rating:781,969,973; text "170 ms window" test_report_rows:468, test_report_view:423,447. Note: `test_squeeze_rating` and `test_note_window` are the tier table's own tests, so those lines are the owner's pin; the rest are copies in other slices. Not covered by X1 or X2.

**RP2-3. The chart-mode key "Expert Pro Drums, 2x Bass".** Owner: `app::chartmode_key`, `src/app/config.cpp:331`. A change to its wording turns every copy red. Copies: dm_fixture.h:26, test_report_rows:891,894, test_store_open:749,874,905,1009, uitest_library:164,217,241. The `test_store_open` copies may stand for an old stored value, not today's key; I did not tell which.

**RP2-4. The rules key name `max_tied_paths`.** Owner: `src/core/rules.cpp:78` (field `rules.h:38`). Copies: test_app_click:882, test_rules:120,133,136, uitest_library:393,400, uitest_report_windows:393. Each checks that an error message names the key, so a key rename turns all red. The key is also what the test writes into the ini text, which is a sound input.

**RP2-5. The pather's failure reason "activation".** Owner: `src/search/pather.cpp:366`, documented `pather.h:129`. Copies: test_best_finish:132, test_path_costs:245, test_search_target:363 (pb_coach:108 passes it in as input).

**RP2-6. The error text "cannot open file: ".** Owners: `src/audio/mapped_file.cpp:28`, `src/core/winstr.cpp:290`, `src/app/analysis.cpp:113`, `src/app/update_verify.cpp:54`. Copies: test_midi:97, test_pb_state:92, test_winstr:202, pbtest_window:476.

## Counts per kind

Each row counted once under its main kind.
- K1: 8 rows (73, 96, 101, 106, 113, 114, 118, 131). Row 120 is a split, K1 and K3, counted apart.
- K2: 9 rows (75, 89, 93, 103, 108, 117, 119, 122, 124). Rows 87 and 132 carry a K2 part.
- K3: 48 rows (every other row). Row 76 carries a K4 part.
- K4: 2 rows (126, 134).
- Split: 1 row (120).
- Unclear: 1 row (129).

Rows not reached: none. For rows with more than five files, the unopened files are judged from the sampled ones.
