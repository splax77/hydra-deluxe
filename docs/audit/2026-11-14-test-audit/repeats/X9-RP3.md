agent id: a73c3e215d7706c3e (T5-X9-RP3, reads the K3 rows and K3 parts of repeats/RP3.md, rows 139 to 207)

Verdicts: X9 (a made-up stand-in for engine output), sound, snapshot (an expected value pinned from one engine run, no rule stated beside it; the coordinator's added verdict), echo (types a string in and checks it comes back; none found), unclear. Paths are under `tests/` unless they start with `src/`. I opened every place listed in repeats.tsv for these rows (about 3 lines around each) and read more only where the verdict turned on it. Rows 140, 154, 159, 160, 163, 166 to 170, 174, 178 to 180, 183, 184, 188, 189, 191, 192, 195 and 202 are K1, K2 or K4 in the sorter's file and are not mine.

## Table

| Row | Value | Verdict | Reason | Function fed | Places opened |
|---|---|---|---|---|---|
| 139 | "beyonce" | sound | folding rule on a typed name; title and artist are chart metadata, an outside input | `fold_for_search`, `matches`, `search_words` | test_library_query.cpp:62,184,504; test_report_view.cpp:186-189; test_library_model.cpp:551 |
| 141 | 19200 | sound | a note and fill tick on hand-built charts | `search`, `build_preview_scene` | record_fixtures.h:113; test_fills_and_skips.cpp:71,257; test_preview_scene.cpp:300 |
| 142 | "charts" (K3 part, and the other three) | sound | the noun pair is an argument to the real `counted()`; the expected text is the function's rule on a count the test picks | `counted` | test_batch_text.cpp:19-22; test_report_rows.cpp:828,842; ui/uitest_report_windows.cpp:1143 |
| 143 | 1024 | sound | byte sizes the tests choose | file and size limits | test_docs_match_code.cpp:2024-25; test_library_jobs.cpp:733-34; test_sng.cpp:183 |
| 144 | 9984 | snapshot (test_path_costs.cpp); sound (other two) | test_path_costs.cpp:209,219,233 type pins 9984, 22272, 37632, 49920 taken from "Allies' best path by activation only" (comment at 196) on the real chart Mutemath - Allies, with no rule beside them. test_replay.cpp:739 is a hand-built tick input. test_tied_variants.cpp:522,575 state the rule in the comments at 474 and 496-502, on a hand-built chart | `build_ladder`, `search_target`; `dump_one_activation`; `Path::trailing_skipped_fill_ticks` | test_path_costs.cpp:196,209,219,233; test_replay.cpp:739; test_tied_variants.cpp:474-575 |
| 145 | 24000 | sound | audio frame counts, and a deact tick worked out in the comment (23040 ticks is 24000 ms) | `read_frames`, `sp_meter_bars_at` | test_preview_transport.cpp:213-14; test_sp_meter_curve.cpp:203; test_stream_mix.cpp:249,255 |
| 146 (K3 part) | "<unknown artist>" | sound | a rich-tag test; the string is only text to be left alone | `strip_rich_tags` | test_library_query.cpp:167 |
| 147 | "9.9.9" | sound | a release version the test sets, as the update feed gives it; not engine output | `Settings::load_file`, update state | test_config.cpp:148,168; test_update_state.cpp:201,241; ui/uitest_library.cpp:1933 |
| 148 | 999 | sound | inputs to a number formatter and a git line list | `format_percent`, `group_thousands`, git helper | test_display_format.cpp:23,41; test_precheck_git.cpp:145; test_winstr.cpp:258 |
| 149 | "00.000" | sound (display_format, preview_boxes); snapshot (pbtest_window.cpp:92) | the clock rule on chosen ms values. pbtest_window.cpp:92 says "from one run" for the Preview clock at the song's start. "5:59.957" at 427,434 is the Symphony X song length, a chart fact | `clock_str`, `build_time_box` | test_display_format.cpp:31,36; test_preview_boxes.cpp:142; pb_ui/pbtest_window.cpp:92,427,434 |
| 150 | 1300.0 | sound | ms inputs to clock_str, a step and a length | `clock_str`, `step_shown_ms`, song length | test_display_format.cpp:34; test_preview_boxes.cpp:248-252; test_song_length.cpp:253 |
| 151 | 576 | sound | ticks per measure of a hand-built 6/8 and 3/4 chart, worked out in the comment | `chart_landing`, `paths_json`, parser | test_fill_landing.cpp:178,191; test_replay.cpp:799,1067; test_song_parse.cpp:413 |
| 152 | "burnout" | X9 (lines 357, 599, 834); sound for the key | "burnout" is a row key the test picks. The summaries it is paired with, `ready(378315, "3- 1 2", 7, 163.0)`, are made up and copy Burnout's real score and path (X1 as well) | `LibraryModel::set_summary_for`, `set_summaries_for`, `shows_same_as`, `sort_if_out_of_date` | test_library_model.cpp:357,599,834,872; test_library_query.cpp:461; ui/uitest_live.cpp:519 |
| 153 | 350 | snapshot (test_replay.cpp:197-98); sound (overlay, timeline) | test_replay.cpp:180-196, on the hand-built two-solo chart, says "Pinned from one run" and gives no rule for the totals 150, 50, 300, 350. In overlay and timeline 350 is a pixel input | `replay_path` chord totals; `track_height`; `timeline_hit` | test_replay.cpp:180-198; test_overlay_layout.cpp:25; test_preview_timeline.cpp:125,129 |
| 155 | 750.0 | sound | ms of a hand-built scene; the test's own chart gives 750 | `build_preview_scene`, `solo_box_at`, transport | test_preview_boxes.cpp:746; test_preview_scene.cpp:88,104,220; test_preview_transport.cpp:284,287 |
| 156 | 256 | sound | tile and buffer sizes and a key size | `over_share`, `read_frames`, ECDSA key | test_preview_golden.cpp:286,297; test_stream_mix.cpp:424; test_update_verify.cpp:78 |
| 157 | 600000.0 | sound (the value); snapshot nearby at test_preview_timeline.cpp:556 | the value is a song length the test picks (uitest_paths.cpp:249 says it is an input). Line 556, "19 labels. Pinned from one run", pins a layout answer in the same file | `timeline_layout`, `show` | test_preview_timeline.cpp:231,240,556; test_timeline_view.cpp:82; ui/uitest_paths.cpp:249 |
| 158 | "md5" | sound | a key string for a song key and a store row; chart metadata, not engine output | `PreviewSongKey`, `ChartLibraryEntry` insert | test_analysis.cpp:112; test_song_parse.cpp:1285,1289; test_store_library.cpp:75,77 |
| 161 | "04.000" | sound | clock rule on chosen ms | `clock_str`, `build_time_box` | test_display_format.cpp:35; test_preview_boxes.cpp:131,142; test_preview_controller.cpp:541 |
| 162 (K3 part, and test_pb_text) | 71040 | X9 | test_model.cpp:1286,1292: `one_step(59520, 71040)` is a hand-made Activation with Burnout's numbers (X1 copy), fed to the record's own readers. The real value comes from the search that fills `Activation` (`sqout_tick`, `deact_tick`). test_pb_text.cpp:118-126 is Allister's real fill list (a typed copy of the engine's offered fills, X3) plus a made-up cost list, see below | `Activation::window_end_tick`, `deact_tick`; `app::choice_order` | test_model.cpp:1275-1292; test_pb_text.cpp:112-126 |
| 164 | 12960 | snapshot | test_sp_meter_curve.cpp:464 "The engine's record: the squeezed-out chord is 12960"; test_sp_end.cpp:220 and 689 "pinned" on hand-built songs (the subcase title states a 250 ms rule, but the expected chord is the engine's answer). test_sp_end.cpp:227 and test_squeeze_rating.cpp:256 use 12960 as an input tick: sound | `run_search`, `ScoreGraph` squeeze choices | test_sp_meter_curve.cpp:445-464; test_sp_end.cpp:210-232,689; test_squeeze_rating.cpp:256 |
| 165 (K3 parts) | 163.5 | sound (test_activation_rows.cpp:73); X9 (test_library_query.cpp:424) | line 73 gives `format_ms` a number. Line 424 hands the made-up summary `analyzed(7, 163.5)` to the query | `format_ms`; `matches` | test_activation_rows.cpp:73; test_library_query.cpp:424 |
| 171 | 380 | sound | widths and a star cutoff on hand-built inputs | column layout, `track_rect`, `build_preview_scene` | test_column_widths.cpp:242; test_overlay_layout.cpp:43; test_preview_boxes.cpp:523,549 |
| 172 | 10000 | sound | a point value and scores the test picks | `collect_dm_rows`, `number_of`, `cutoffs_for` | test_dm_report.cpp:489-90; test_precheck_check3.cpp:114; test_stars.cpp:222 |
| 173 | "README.md" | sound | file name | docs and git helpers | test_docs_match_code.cpp:725,2147; test_fuzz_regressions.cpp:41; test_precheck_git.cpp:167 |
| 175 | 2400 | sound | ticks on hand-built charts; comments at test_phrase_end.cpp:24,94 give the rule | parser, `chart_flags`, `measures_at_tick_f` | test_five_lane.cpp:270; test_phrase_end.cpp:24,94; test_timing.cpp:346 |
| 176 | 3360 | sound | ticks of hand-built meters and fills | `fills_of`, `build_preview_scene` | test_generated_fills.cpp:182; test_preview_boxes.cpp:227; test_preview_scene.cpp:342,347 |
| 177 | 40.0 | X9 (test_library_model.cpp:517, test_library_query.cpp:494); sound (load progress) | `ready(300000, "1 1", 6, 40.0)` and `analyzed(3, 40.0)` are made-up summaries. In test_preview_load_progress.cpp:240,242, 40.0 is a rate the test sets | `set_summary_for`, `query_matches`; `time_left_text` | test_library_model.cpp:517; test_library_query.cpp:494; test_preview_load_progress.cpp:240,242 |
| 181 | 120000 | sound | tempo of hand-built chart text | chart parser | test_song_input.cpp:182,312; test_song_parse.cpp:809; test_user_messages.cpp:96 |
| 182 (K3 part) | 21120 | sound | last tick of a `beat_song` the test builds | `deact_edge_at` | test_sp_end.cpp:249 |
| 185 (K3 parts) | "3 bars" | sound | comments naming the text whose width `mono(6)` stands for; the string is never fed to code | width constants | test_activation_row_layout.cpp:46; test_badge_pill.cpp:36 |
| 186 | 12.4 | X9 (test_activation_rows.cpp:745); sound (other two) | `SPSqueeze{SqIn, 12.4}` is a made-up squeeze result handed to the badge text. test_display_format.cpp:59 and test_model.cpp:387 test a rounding rule and `hardest()` on chosen numbers | `activation_badges`, `badge_texts`; `format_ms_whole`; `Activation::hardest` | test_activation_rows.cpp:730-745; test_display_format.cpp:59; test_model.cpp:387 |
| 187 | 85.1 | snapshot (test_activation_rows.cpp:1367); sound (other two) | line 1367 types "frontend 61.9 ms ... backend 48.1 ms ..." and "85.1 ms safe" for Focus, activation 2, with no rule beside it (an engine answer on a real chart). test_report_rows.cpp:777 is a tier edge and test_timing.cpp:277 a tempo | `window_rows_of`, `squeeze_row`; `tier_for`; `SongTiming` | test_activation_rows.cpp:1340-1367; test_report_rows.cpp:765-782; test_timing.cpp:277 |
| 190 | 90.0 | sound | inputs | `time_left_text`, tempo list, fail line | test_batch_text.cpp:45; test_fill_deadline.cpp:130; test_live_protocol.cpp:31 |
| 193 | "1 2 3" | X9 (test_library_model.cpp:1081, test_library_query.cpp:556); unclear (test_column_widths.cpp:66) | a made-up path text: `ready(300000 + i, "1 2 3", ...)` and `search_field(..., "1 2 3")`. In test_column_widths.cpp:66 the comment says the text columns never read the path column | `set_summary_for`; `search_field`; column measure | test_column_widths.cpp:66; test_library_model.cpp:1081; test_library_query.cpp:556 |
| 194 | "Bold Title" | sound | input is "<b>Bold Title</b>" in the store or library and the test checks the tag is stripped, a rule | `collect_dm_rows`, `compare`, `collect_rows` | test_dm_report.cpp:351,361; test_fill_report.cpp:592,596; test_report_rows.cpp:722,731 |
| 196 | "status" | sound | a column id and a git argument | `dm_columns`, git helper | test_dm_report.cpp:822; test_precheck.cpp:111; test_precheck_git.cpp:288 |
| 197 | 200.0 | X9 (test_preview_boxes.cpp:795); sound (other two) | test_preview_boxes.cpp:783-796 builds a `PreviewScene` by hand (`PreviewSoloNotes` with steps {100,60},{200,120}, notes 120, bonus 12000) and shows it through `solo_box_at`. The real scene comes from `build_preview_scene`. The others use 200.0 as a tempo and a boundary time | `solo_box_at` | test_fill_deadline.cpp:130; test_preview_boxes.cpp:783-796; test_sp_meter_curve.cpp:513 |
| 198 | 360 | sound (test_fill_landing.cpp:104, test_preview_scene.cpp:369); snapshot (test_replay.cpp:1537) | the first two are hand-built tick inputs. test_replay.cpp:1537 `straddle.sp == 360` is in the `chord_score_table` test whose comment (1515-1517) says "Every value is a literal pinned from one run" | `chord_has_fill`, beat lines; `chord_score_table` | test_fill_landing.cpp:104; test_preview_scene.cpp:369; test_replay.cpp:1515-1537 |
| 199 | 101 | sound | MIDI note number, a count and a line count | MIDI reader, `sqout_position`, `large_range_line` | test_five_lane.cpp:35; test_model.cpp:911; test_precheck_git.cpp:201 |
| 200 | 100000.0 | sound | a timeout input, an end ms, a song length input | `ok_line`, `split_squeeze`, `show` | test_live_protocol.cpp:24; test_replay_schedule.cpp:488; test_timeline_view.cpp:83 |
| 201 | "\song.ini" | sound | a file the test writes | loaders | test_long_paths.cpp:128; test_preview_load_progress.cpp:326; test_preview_source.cpp:278 |
| 203 | 0.125 | sound | a rule value the test sets, and a rate it works out in the comment | `Rules` fingerprint, `build_scrub_marks`, `sp_meter_bars_at` | test_preview_boxes.cpp:864; test_rules.cpp:98; test_sp_meter_curve.cpp:324 |
| 204 | -250 | sound | a chart delay the test writes and a layout x | `parse_finite_number`, ini delay, `timeline_layout` | test_preview_load_progress.cpp:329; test_preview_timeline.cpp:176; test_song_input.cpp:132 |
| 205 | `  0 = TS 4\n  0 = B 120000\n` | sound | hand-built chart text | chart parser | test_song_input.cpp:312; test_song_parse.cpp:809 (line only from the sorter); test_user_messages.cpp:96 (same) |
| 206 | "ExpertDrums" | sound | chart section name the test writes | chart parser | test_song_input.cpp:313; test_user_messages.cpp:95; pb_ui/pbtest_window.cpp:451 |
| 207 | "script" | sound | a test-engine test name (not opened past the sorter's line) | harness | pb_ui/pbtest_harness.cpp:76; ui/uitest_harness.cpp:135; ui/uitest_main.cpp:139 |

## X9 instances, grouped by the function they feed

`LibraryModel::set_summary_for`, `set_summaries_for`, `shows_same_as`, `sort_if_out_of_date` (the made-up summary is `ready(score, path, stars, ms)`; the real one comes from the analysis record store, read by the Library):
- test_library_model.cpp:357, 599, 834: `ready(378315, "3- 1 2", 7 or 6, 163.0)`, a copy of Burnout's real score, path and squeeze (X1 as well).
- test_library_model.cpp:517: `ready(300000, "1 1", 6, 40.0)`, invented.
- test_library_model.cpp:1081: `ready(300000 + i, "1 2 3", 5 + ..., i % 90)`, invented.

`query_matches`, `matches` (library query over a summary row):
- test_library_query.cpp:424: `analyzed(7, 163.5)`, beside `analyzed(7, 163.0)`, the "Burnout's real 163.0 ms" copy.
- test_library_query.cpp:494: `analyzed(3, 40.0)`, invented.

`search_field` (builds report search text from a path string):
- test_library_query.cpp:556: path text "1 2 3", invented.

`activation_badges`, `badge_texts` (screen text for an activation):
- test_activation_rows.cpp:745: `SPSqueeze{SqueezeKind::SqIn, 12.4}` on a no-timing activation, an invented squeeze ms. The engine owner is the search that fills `Activation::sqinouts`.

`Activation::window_end_tick`, `deact_tick` (read an engine record):
- test_model.cpp:1286, 1292: `one_step(59520, 71040)`, invented but with Burnout's activation numbers.

`app::choice_order` (orders offered fills by cost and tier; the real costs come from `search_target` and `end_costs`):
- test_pb_text.cpp:118-126: a `CandidateCost` list the comment calls "made up to put a tie and two pending fills in the list", built on Allister's real fill ticks. The sorter filed this under K1 (row 162); I add it because it feeds a made-up cost and tier to a ranking.

`solo_box_at` (Preview solo box):
- test_preview_boxes.cpp:783-796: a hand-made `PreviewScene` with bonus 12000, 120 notes, steps 60 and 120. The owner of the real value is `build_preview_scene`'s `solo_notes`.

## Snapshots found (expected value pinned from one engine run)

- test_path_costs.cpp:209,219,233 (row 144): Allies' best-path ticks.
- test_replay.cpp:197-198 and 1537 (rows 153 and 198): chord score totals "pinned from one run".
- test_sp_meter_curve.cpp:464, test_sp_end.cpp:220 and 689 (row 164): 12960 as the engine's squeezed-out chord.
- test_activation_rows.cpp:1367 (row 187): Focus window row and "85.1 ms safe".
- pb_ui/pbtest_window.cpp:92 (row 149): "0:00.000" "from one run".
- test_preview_timeline.cpp:556 (near row 157): 19 axis labels, "Pinned from one run".
- Seen, not a row of mine: test_replay_schedule.cpp:476-487 types a table of per-chart squeeze numbers (Freak 1144, Rescued 561 and others) with no rule beside them.

## Counts

Rows covered: 45 rows (K3 rows and K3 parts of split rows), 51 table lines.
- X9, whole or part: rows 152, 162, 165, 177, 186, 193, 197 (7 rows), 14 places in 6 functions plus the test_pb_text.cpp:118 line.
- Snapshot, whole or part: rows 144, 149, 153, 164, 187, 198 (6 rows), and one near row 157.
- Echo: none.
- Unclear: test_column_widths.cpp:66 (part of 193).
- Sound: all other rows and parts.
- Rows not reached: none. Row 207 and the second and third places of row 205 were not opened past the sorter's line.
