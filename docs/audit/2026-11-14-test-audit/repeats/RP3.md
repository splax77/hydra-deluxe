agent id: ad984b242ea2eeb1e (T5-RP3, rows 139 to 207)

Method: for every row I read the source line at each place the script listed (all places of all 69 rows) and grepped `src/` for owners. Each owner named below was found by grep at the line given. Some chart-of-origin questions were not opened; they are listed at the end.

## Table (row, value, kind, reason, places opened)

| Row | Value | Kind | Reason | Places |
|---|---|---|---|---|
| 139 | "beyonce" | K3 | folded search word the test types as input and expected output | test_library_query.cpp:62,184,504; test_report_view.cpp:186-189; test_library_model.cpp:551 |
| 140 | 25.123 (clock "0:25.123") | K1 | Burnout's first fill dot as the Preview clock reads it, "from one run" (pbtest_coach.cpp:303-305 comment) | pb_ui/pbtest_build.cpp:165,199; pbtest_coach.cpp:305,365,369,524; pbtest_window.cpp:76,204 |
| 141 | 19200 | K3 | tick on hand-built charts and a hand-built fixture | record_fixtures.h:113; test_fills_and_skips.cpp:71-257; test_preview_scene.cpp:300 |
| 142 | "charts" | K3 for test_batch_text.cpp:19-22; K2 for the other 3 | the batch_text lines test counted() with their own words; report_rows 828/842 and uitest_report_windows.cpp:1143 pass the same noun pair the product passes | test_batch_text.cpp:19-22; test_report_rows.cpp:828,842; ui/uitest_report_windows.cpp:1143 |
| 143 | 1024 | K3 | byte sizes the tests choose | test_docs_match_code.cpp:2024-25; test_library_jobs.cpp:733-34; test_sng.cpp:183 |
| 144 | 9984 | K3 | pinned window ticks on hand-built charts | test_path_costs.cpp:209,219,233; test_replay.cpp:739; test_tied_variants.cpp:522,575 |
| 145 | 24000 | K3 | audio frame counts and a hand-built deact tick | test_preview_transport.cpp:213-14; test_sp_meter_curve.cpp:203; test_stream_mix.cpp:249,255 |
| 146 | "<unknown artist>" | K2 | `kUnknownArtist`, src/parse/song.h:142 | test_analysis.cpp:1141; test_library_query.cpp:167 (K3, input to strip_rich_tags); test_song_input.cpp:371,400 |
| 147 | "9.9.9" | K3 | version the tests set | test_config.cpp:148-168; test_update_state.cpp:201,241; ui/uitest_library.cpp:1933 |
| 148 | 999 | K3 | inputs | test_display_format.cpp:23,41; test_precheck_git.cpp:145; test_winstr.cpp:258 |
| 149 | "00.000" | K3 | outputs of clock_str on inputs the test picks. The "5:59.957" tails at pbtest_window.cpp:427,434 are the Symphony X song length (K1, owner clock_str src/app/display_format.cpp:64 on the chart's length) | test_display_format.cpp:31,36; test_preview_boxes.cpp:142; pbtest_window.cpp:92,427,434 |
| 150 | 1300.0 | K3 | inputs | test_display_format.cpp:34; test_preview_boxes.cpp:248-252; test_song_length.cpp:253 |
| 151 | 576 | K3 | hand-built tempo / sp phrase | test_fill_landing.cpp:178,191; test_replay.cpp:799,1067; test_song_parse.cpp:413-14 |
| 152 | "burnout" | K3 | a library key string the test picks | test_library_model.cpp:357,599,834,872; test_library_query.cpp:461; ui/uitest_live.cpp:519 |
| 153 | 350 | K3 | layout px and a hand-built score | test_overlay_layout.cpp:25; test_preview_timeline.cpp:125,129; test_replay.cpp:197-98 |
| 154 | "SP drain (if activated)" | K2 | `box.header`, src/app/preview_view.cpp:680 (declared preview_view.h:428) | test_overlay_layout.cpp:311; test_preview_boxes.cpp:580,592,647; ui/uitest_preview.cpp:435,490 |
| 155 | 750.0 | K3 | hand-built scene ms | test_preview_boxes.cpp:746; test_preview_scene.cpp:88,104,220; test_preview_transport.cpp:284,287 |
| 156 | 256 | K3 | sizes | test_preview_golden.cpp:286,297; test_stream_mix.cpp:424,431; test_update_verify.cpp:78 |
| 157 | 600000.0 | K3 | song length a test picks | test_preview_timeline.cpp:231,240,556; test_timeline_view.cpp:82; ui/uitest_paths.cpp:249 |
| 158 | "md5" | K3 | a key string the test picks | test_analysis.cpp:112; test_song_parse.cpp:1285,1289; test_store_library.cpp:75,77 |
| 159 | "Analysis settings: defaults" | K2 | label built at src/ui/library_dialogs.cpp:132 (`"Analysis settings: " + settings_changes_summary(s)`; "defaults" per library_parts.h:83 comment) | test_batch_text.cpp:81; test_config.cpp:995; ui/uitest_library.cpp:254,273,322 |
| 160 | "Stop the batch to change these." | K2 | `kSettingsLockedText`, src/ui/library_parts.h:97 | test_batch_text.cpp:153,157; ui/uitest_details.cpp:1313,1330; ui/uitest_library.cpp:1981 |
| 161 | "04.000" | K3 | clock outputs of chosen inputs | test_display_format.cpp:35; test_preview_boxes.cpp:131,142; test_preview_controller.cpp:541 |
| 162 | 71040 | Mixed | test_model.cpp:1286,1292: K3 (hand-built). test_pb_text.cpp:118: K1 (fills offered; a real chart's answer; chart not named on the line). test_sp_meter_curve.cpp:954,956: Unclear, I did not open which chart | as listed |
| 163 | "Insane" | K2 | tier names, src/core/squeeze_rating.cpp:247 | test_report_rows.cpp:777-78; test_report_view.cpp:339,342; test_squeeze_rating.cpp:773 |
| 164 | 12960 | K3 | ticks on hand-built charts; test_sp_meter_curve.cpp:464 fixture not opened | test_sp_end.cpp:220,227,689; test_sp_meter_curve.cpp:464; test_squeeze_rating.cpp:256 |
| 165 | 163.5 | Split | K3 at test_activation_rows.cpp:73 (format_ms input) and test_library_query.cpp:424 (analyzed(7,163.5) input); K1 at ui/uitest_paths.cpp:344 (`kBurnoutSqOutRating`, Burnout's rating text) | as listed |
| 166 | `+ kDot +` | K2 | `tests/song_details_fixtures.h:36` copies the separator documented at src/app/display_format.h:13; used by name, so one typed copy | test_activation_rows.cpp:592,659; test_path_list.cpp:336; test_preview_boxes.cpp:1051 |
| 167 | "m32.1.0" | K1 | Burnout's first activation measure (test_activation_rows.cpp:582 opens `burnout()`); measure text built at src/core/timing.cpp:355 | test_activation_rows.cpp:597; ui/uitest_paths.cpp:261; ui/uitest_preview.cpp:1002,1048 |
| 168 | "m58.1.0" | K1 | same, Burnout's other activation | test_activation_rows.cpp:615; ui/uitest_paths.cpp:262; ui/uitest_preview.cpp:1005,1050 |
| 169 | "early fill 0.0 ms" | K2 | label "early fill" at src/app/path_view.cpp:58 plus its ms format; the 0.0 is a test-built row at 760/880/196; uitest_paths.cpp:339 reads a real chart's screen (K1-leaning) | test_activation_rows.cpp:760,880; test_pb_text.cpp:196; ui/uitest_paths.cpp:339 |
| 170 | "/all-users" | K2 | DMBot path, src/net/dmbot_client.cpp:122; the fakes match on the URL | test_app_reports.cpp:477,861; ui/uitest_harness.cpp:202; ui/uitest_report_windows.cpp:914 |
| 171 | 380 | K3 | hand-built widths and cutoffs | test_column_widths.cpp:242; test_overlay_layout.cpp:43; test_preview_boxes.cpp:523,549 |
| 172 | 10000 | K3 | inputs | test_dm_report.cpp:489-90; test_precheck_check3.cpp:114; test_stars.cpp:222 |
| 173 | "README.md" | K3 | file name | test_docs_match_code.cpp:725,2147; test_fuzz_regressions.cpp:41; test_precheck_git.cpp:167 |
| 174 | "Allister - Overrated" | K4 | corpus chart folder name | test_fills_and_skips.cpp:439; test_pb_text.cpp:115,263; ui/uitest_details.cpp:572 |
| 175 | 2400 | K3 | hand-built ticks | test_five_lane.cpp:270; test_phrase_end.cpp:24,94; test_timing.cpp:346 |
| 176 | 3360 | K3 | hand-built ticks | test_generated_fills.cpp:182; test_preview_boxes.cpp:227; test_preview_scene.cpp:342,347 |
| 177 | 40.0 | K3 | inputs | test_library_model.cpp:517; test_library_query.cpp:494; test_preview_load_progress.cpp:240,242 |
| 178 | "Green Day" | K4 | Burnout's artist as the chart file states it (test_pb_state.cpp:37); test-chosen rows elsewhere | test_library_model.cpp:854; test_library_query.cpp:530,556; test_pb_state.cpp:37 |
| 179 | "Burnout — Green Day" | K2 | built by `chart_title_line`, src/pathbuilder/pb_state.cpp:43 | test_pb_state.cpp:38; pbtest_build.cpp:144; pbtest_window.cpp:88,499 |
| 180 | "Insane+" | K2 | src/core/squeeze_rating.cpp:248 | test_report_rows.cpp:779-80; test_report_view.cpp:342; test_squeeze_rating.cpp:778 |
| 181 | 120000 | K3 | tempo of hand-built charts | test_song_input.cpp:182,312; test_song_parse.cpp:809; test_user_messages.cpp:96 |
| 182 | 21120 | Split | K1 for the three pb_ui places (Burnout's fill chord tick "from one run", pbtest_coach.cpp:123-126); K3 for test_sp_end.cpp:249 | pbtest_coach.cpp:126,150; pbtest_window.cpp:161 |
| 183 | "**/< Act##prevact" | K2 | widget label from src/ui; the tests must name it, but it is a typed copy | pbtest_build.cpp:50; pbtest_window.cpp:149; uitest_preview.cpp:111,706 |
| 184 | "##timeline" | K2 | widget id from src/ui, same as 183 | pbtest_build.cpp:490; pbtest_coach.cpp:353; pbtest_window.cpp:374,421 |
| 185 | "3 bars" | Split | K1 at test_activation_rows.cpp:599 (Burnout's activation; text from `counted()` src/core/model.cpp:929); K3 for the two layout tests (comments naming a string whose width they choose) | test_activation_rows.cpp:599; test_activation_row_layout.cpp:46; test_badge_pill.cpp:36 |
| 186 | 12.4 | K3 | inputs | test_activation_rows.cpp:745; test_display_format.cpp:59; test_model.cpp:387 |
| 187 | 85.1 | K3 | tier cutoff input, hand-built tempo, hand-built row | test_activation_rows.cpp:1367; test_report_rows.cpp:777; test_timing.cpp:277 |
| 188 | "No Hard Pro Drums notes in this chart." | K2 | `no_notes_message`, src/parse/song.h:82-86 | test_analysis.cpp:826; test_model.cpp:1045; test_song_parse.cpp:1202 |
| 189 | "<unknown charter>" | K2 | `kUnknownCharter`, src/parse/song.h:143 | test_analysis.cpp:1142; test_sng.cpp:294; test_song_input.cpp:386 |
| 190 | 90.0 | K3 | inputs | test_batch_text.cpp:45; test_fill_deadline.cpp:130; test_live_protocol.cpp:31 |
| 191 | "10 ms" | K2 (owner partly unfound) | the path limit text is held in `path_limit` (src/ui/library_parts.h:77, set at library_dialogs.cpp:82); I did not find the default value's own constant | test_batch_text.cpp:55; test_config.cpp:1021; ui/uitest_batch_reports.cpp:393 |
| 192 | "sqlite exec failed: file is not a database" | K2 | prefix `"sqlite exec failed: "` at src/store/record_store.cpp:1058; the rest is sqlite's own text | test_cli.cpp:238; test_store_open.cpp:1158; test_user_messages.cpp:407 |
| 193 | "1 2 3" | K3 | a path string the test picks | test_column_widths.cpp:66; test_library_model.cpp:1081; test_library_query.cpp:556 |
| 194 | "Bold Title" | K3 | title the tests give | test_dm_report.cpp:361; test_fill_report.cpp:596; test_report_rows.cpp:731 |
| 195 | "cancelled" | K2 | default message of `Cancelled`, src/core/error_kind.h:158 | test_dm_report.cpp:660; test_preview_controller.cpp:355; test_preview_load_progress.cpp:313 |
| 196 | "status" | K3 | a git argument / a column id | test_dm_report.cpp:822; test_precheck.cpp:111; test_precheck_git.cpp:288 |
| 197 | 200.0 | K3 | inputs | test_fill_deadline.cpp:130; test_preview_boxes.cpp:795; test_sp_meter_curve.cpp:513 |
| 198 | 360 | K3 | hand-built ticks | test_fill_landing.cpp:104; test_preview_scene.cpp:369; test_replay.cpp:1537 |
| 199 | 101 | K3 | MIDI note number, a size input | test_five_lane.cpp:35; test_model.cpp:911; test_precheck_git.cpp:201 |
| 200 | 100000.0 | K3 | inputs | test_live_protocol.cpp:24; test_replay_schedule.cpp:488; test_timeline_view.cpp:83 |
| 201 | "\song.ini" | K3 | file the tests write | test_long_paths.cpp:128; test_preview_load_progress.cpp:326; test_preview_source.cpp:278 |
| 202 | " ENDOFSONG" | K2 | section-name rule at src/parse/section_names.cpp:13 (`kMidSectionVeto "_ENDOFSONG"` and its display name) | test_midi.cpp:712; test_section_names.cpp:87; test_song_parse.cpp:270 |
| 203 | 0.125 | K3 | test sets the rule value and picks a rate (rules.h:51's default is 1/32, not 0.125) | test_preview_boxes.cpp:864; test_rules.cpp:98; test_sp_meter_curve.cpp:324 |
| 204 | -250 | K3 | inputs | test_preview_load_progress.cpp:329; test_preview_timeline.cpp:176; test_song_input.cpp:132 |
| 205 | `  0 = TS 4\n  0 = B 120000\n` | K3 | hand-built chart text | test_song_input.cpp:312; test_song_parse.cpp:809; test_user_messages.cpp:96 |
| 206 | "ExpertDrums" | K3 | chart-file section name the tests write | test_song_input.cpp:313; test_user_messages.cpp:95; pbtest_window.cpp:451 |
| 207 | "script" | K3 | test-engine test name | pbtest_harness.cpp:76; uitest_harness.cpp:135; uitest_main.cpp:139 |

## Candidates (K1 and K2)

For every K2 below, a change to the owner turns every copy red at once (the X1 harm) unless I say otherwise.

- **RP3-1 (K1, covered by X2): Burnout's first fill typed as ticks and as its clock.** 25.123 s ("0:25.123", row 140, with the song-length tail 2:10.088) and 21120 (row 182) are Burnout fills "from one run", beside kFirstFillTick 36480 in pb_ui/pbtest_harness.h:60. Owner: the early-fill rule `fill_refuses` src/core/model.h:331, and `clock_str` src/app/display_format.cpp:64. Copies: pb_ui/pbtest_build.cpp:165,199; pbtest_coach.cpp:126,150,305,365,369,524; pbtest_window.cpp:76,161,204. X2 names the ticks; the clock text is the same fact in another form, and the Symphony X "5:59.957" at pbtest_window.cpp:427,434 is a sibling.
- **RP3-2 (K1; X1 covers 163.5 and Insane+, the measures are not named in what I read of X1): Burnout's best-path activations on screen.** "m32.1.0", "m58.1.0" (rows 167, 168), "3 bars" (row 185), and 163.5 as `kBurnoutSqOutRating` "Insane+ (eff. 163.5 ms) <-- squeezed out (-260)" (row 165). Owner: the engine's path for Burnout, `src/core/timing.cpp:355` (measure text), `counted` src/core/model.cpp:929. Copies: test_activation_rows.cpp:597,599,615; ui/uitest_paths.cpp:261,262,344; ui/uitest_preview.cpp:1002,1005,1048,1050. A path change turns all red together.
- **RP3-3 (K2): tier names "Insane" and "Insane+".** Owner src/core/squeeze_rating.cpp:247-248. Copies: test_report_rows.cpp:777-780; test_report_view.cpp:339,342; test_squeeze_rating.cpp:773,778. test_squeeze_rating and test_report_rows are the owner's own tests; test_report_view builds lists with the names.
- **RP3-4 (K2): unknown-artist and unknown-charter placeholders.** Owner `kUnknownArtist`/`kUnknownCharter`, src/parse/song.h:142-143. Copies: test_analysis.cpp:1141-42 (pins the constants), test_sng.cpp:294, test_song_input.cpp:371,386,400. Only test_sng.cpp:294 reads the product's output; the others feed the string in.
- **RP3-5 (K2): Library settings words, and the Path Builder title line.** "Analysis settings: defaults" (owner src/ui/library_dialogs.cpp:132; copies test_batch_text.cpp:81, test_config.cpp:995, uitest_library.cpp:254,273,322). "Stop the batch to change these." (owner `kSettingsLockedText` src/ui/library_parts.h:97; copies test_batch_text.cpp:153,157, uitest_details.cpp:1313,1330, uitest_library.cpp:1981). "10 ms" path limit (copies test_batch_text.cpp:55, test_config.cpp:1021, uitest_batch_reports.cpp:393; default's owner not found). "Burnout — Green Day" (owner `chart_title_line` src/pathbuilder/pb_state.cpp:43; copies test_pb_state.cpp:38, pbtest_build.cpp:144, pbtest_window.cpp:88,499).
- **RP3-6 (K2): Preview and badge words.** "SP drain (if activated)" (owner preview_view.cpp:680; copies test_overlay_layout.cpp:311, test_preview_boxes.cpp:580,592,647, uitest_preview.cpp:435,490). "early fill 0.0 ms" (owner label src/app/path_view.cpp:58; copies test_activation_rows.cpp:760,880, test_pb_text.cpp:196, uitest_paths.cpp:339). Separator kDot (tests/song_details_fixtures.h:36 copies src/app/display_format.h:13; used at test_activation_rows.cpp:592,659, test_path_list.cpp:336, test_preview_boxes.cpp:1051).
- **RP3-7 (K2): product messages.** "No Hard Pro Drums notes in this chart." (owner `no_notes_message` src/parse/song.h:82; copies test_analysis.cpp:826, test_model.cpp:1045, test_song_parse.cpp:1202). "cancelled" (owner src/core/error_kind.h:158; copies test_dm_report.cpp:660, test_preview_controller.cpp:355, test_preview_load_progress.cpp:313). "sqlite exec failed: ..." (owner src/store/record_store.cpp:1058; copies test_cli.cpp:238, test_store_open.cpp:1158, test_user_messages.cpp:407). "/all-users" (owner src/net/dmbot_client.cpp:122; copies test_app_reports.cpp:477,861, uitest_harness.cpp:202, uitest_report_windows.cpp:914; these are fakes, so a changed path would silently return the wrong canned answer rather than fail).
- **RP3-8 (K2): parse rule " ENDOFSONG".** Owner src/parse/section_names.cpp:13. Copies test_midi.cpp:712, test_section_names.cpp:87, test_song_parse.cpp:270.
- **RP3-9 (K2, weak): widget labels "**/< Act##prevact" and "##timeline".** Owner is the widget id in src/ui; the tests need it to click. Copies in rows 183 and 184. Listed because a rename turns eight places red at once; whether that is a fault is not mine to say.
- **RP3-10 (K2, weak): noun pair "chart","charts".** Owner `counted()` src/core/model.cpp:929 and the calls src/ui/app_state.cpp:686-688. Copies: ui/uitest_report_windows.cpp:1143, test_report_rows.cpp:828,842.

## Not opened

The chart behind 71040 at test_sp_meter_curve.cpp:954 (row 162) and 71040 in test_pb_text.cpp:118 (the line does not name its chart), and the fixture behind 12960 at test_sp_meter_curve.cpp:464 (row 164). The default's owner for "10 ms" (row 191).

## Counts per kind (rows, by main kind)

K1: 140, 167, 168, 182 (rows 165, 185 and 162 have K1 parts). K2: 146, 154, 159, 160, 163, 166, 169, 170, 179, 180, 183, 184, 188, 189, 191, 192, 195, 202 (18 rows; 142 has a K2 part). K4: 174, 178. Unclear: 162 (mixed). K3: the remaining rows. Rows not reached: none.
