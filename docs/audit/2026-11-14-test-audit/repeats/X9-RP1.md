agent ac0330056d3dc2589 (reader T5-X9-RP1, rows 1 to 69 of repeats.tsv, the K3 rows and K3 parts of RP1.md)

How I worked: for every place of every row I printed the source line, then opened the surrounding test or its helper wherever the line could be a typed engine result (all places of rows 16, 20, 24 and 37, every `ready(`, `analyzed(`, `Activation`, `SPSqueeze` and `BackendSqueeze` build, `DynamicsBreakdown`, `PathRow`, `scored_record`). I read the production code the test calls (`src/ui/library_model.cpp:47`, `src/app/library_query.h:117-120`, `src/store/record_store.cpp:629`, `src/app/path_view.h`, `src/app/dynamics_breakdown.h`) to find what the code under test receives. The coordinator's correction is applied: the verdicts are X9, sound, unclear, snapshot (an expected value pinned from one engine run, or a real-chart result typed with no stated rule; `docs/agents/test-rubric.md:23`) and echo (a string typed in and checked back out).

The line I drew. A scalar handed to a formatter whose rule the expected text states (`format_ms(163.0) == "163.0 ms"`, `group_thousands(1000)`) is sound: it tests the format directly. A struct that stands for an engine record (a lookup summary, a facts row, an activation with squeeze rows, a stored path row, dynamics counts), typed by hand and then shown, listed, stored or queried, is X9. Ticks, widths, waits, resolutions and BPM that the test writes into a hand-built chart or layout are inputs to the engine or the layout, so they are sound. I did not re-derive every expected number of the engine-on-hand-chart tests (preview boxes, sp_meter_curve, replay, sp_end); they are sound by the brief's rule unless a comment says "pinned".

## Table

`ready()` is `tests/test_library_model.cpp:52`. `analyzed()` is `tests/test_library_query.cpp:30`. "All places" means every place in `repeats.tsv` for that row; I printed each line and judged it as the reason says.

| Row | Value | Verdict | Reason | Production function fed | Places opened |
|---|---|---|---|---|---|
| 1 | 1000 | X9 part, rest sound | `ready(1000,"1",3,0.0)` is a made-up lookup summary (score 1000, path "1"). `gated_scores` returns a made-up record whose score is `1000*turn`, shown as "1,000" in the library rows. Rest: layout sizes, counts, `frames_of_ms`, a cap the user types (`sp_cap`), a DM leaderboard score from typed JSON | `LibraryModel::set_summary_for/set_summaries` (`src/ui/library_model.cpp`); `AppState::tick_library` row scores | X9: test_library_model:426,720,723,754,783,796; test_app_library:204-214,354,368,387,523,537. Sound: test_app_shell:197, test_column_widths:145, test_display_format:23,42,52, test_batch_text:165, test_cli:526, test_path_costs:170-190, test_dm_report:86,154, all others |
| 2 | 300 | X9 part, rest sound | `many.rows[Kick2x].accent = 300` and Kick 1700 are hand-set counts of a `DynamicsBreakdown`, shown as "300 of 2,000 kick notes (15%)". Rest: widths, ticks, waits; `test_search_target` `made(10,{300})` is a hand-built path fed to the engine's own prune rule; `test_model:1243` is the engine's cost rule on hand rows | `dynamics_kick2x_line`, `dynamics_kick2x_fact` (`src/app/dynamics_breakdown.h:143,152`) | test_dynamics_breakdown:270-281,312,320,322,326,376; test_model:1243; test_search_target:240; test_replay:195; all others |
| 3 | 480 | sound | Resolution and ticks of hand-built charts; `record_fixtures.h:113` is a fixture fill length | none (engine or parser on a hand chart) | all places |
| 4 | 200 | X9 part, snapshot part, rest sound | `analyzed(7,163.0)` and `analyzed(7,163.5)` made-up facts fed to the query (421,424,427). `test_preview_boxes:520,706` `score_solo == 200` sits under the comment "The literals below were pinned from one run" (512-516). Rest: widths, ticks, a clamp the test types (test_config:397,414); the hand-built backend rows at test_activation_rows:364 are the row 41 and 51 instance | `query_matches` (`src/app/library_query.cpp:451`); `build_preview_scene` score box | test_library_query:421,424,427; test_preview_boxes:512-520,706,730; test_activation_rows:364; test_config:397,414; all others |
| 5 | 500 | sound | Widths, ms, ticks, a size cap; `test_dm_report:661` is a test-set wait. The K2 pins (`app_state.h:617`, `uitest_paths:408`) are copies of constants, not stand-ins | none | test_app_reports:637; test_dm_report:661; uitest_paths:408,410; pbtest_window:597; all others |
| 6 | 768 | sound | Ticks on hand-built charts at 192 | none | test_activation_rows:930; test_cli:704; test_note_window:192-384; test_replay:737-855; test_replay_schedule; test_song_parse:845; all others |
| 7 | 120 | sound | BPM, dpi, widths, waits. `test_preview_boxes:794` "60 / 120 notes" is the solo box on a hand chart | none | test_app_shell:330; test_fill_landing:235; test_preview_boxes:695-698,794; UI waits |
| 8 | 1000.0 | sound | ms to seconds, positions on hand scenes, typed geometry of `sqout(...)` handed to the replay schedule helpers (engine rule on typed inputs) | none | test_replay_schedule:529-543,553,608; test_sp_meter_curve; test_preview_*; all others |
| 9 | 1920 | sound | Screen width, ticks of a hand chart | none | test_app_shell:36; test_generated_fills:90-122; test_sp_end_clamp:236; all others |
| 10 | 500.0 | X9 part, rest sound | `path_row("A","","","1",1,500.0,true)` is a made-up report row with a hardest ms of 500.0, fed to the path filter. Rest: scene positions; `test_replay_schedule:356` reads the text of model.h (K2); `pbtest_window:602` checks a leeway the test wrote itself | `path_report_view::path_keep` | test_report_view:450-451; test_replay_schedule:356; pbtest_window:597,602; test_model:174; all others |
| 11 | 10.0 | X9 part, rest sound | `ready(400000,"1 1",6,10.0)` is a made-up summary. Rest: `ms_filter == 10.0` follows the setting the test sets; `step_shown_ms` and `time_to_z` scalars | `LibraryModel::set_summaries_for` | test_library_model:817; test_path_costs:150; test_pb_coach:141; pbtest_coach:172; all others |
| 12 | 3840 | sound | Measure-6 tick, fill ends on hand charts | none | all places |
| 13 | 2000 | X9 part, unclear part, rest sound | `row_scores(*app,1,"2,000")` shows the made-up `gated_scores` record (355,538). Line 253 `best_label().find("2,000")` I did not trace to its record (unclear). "2,000 points" is a setting the test types (`depth_value`), so it is sound | `AppState::tick_library` | test_app_library:253,355,538; test_batch_text:66,126; test_config:1031; test_midi:790; all others |
| 14 | 960 | sound | Ticks, frame counts | none | all places |
| 15 | 192 | sound | Resolution the test writes into its chart text; dpi | none | all places |
| 16 | 163.0 | split: X9, snapshot, sound | X9: `ready(378315,"3- 1 2",7,163.0)` and `analyzed(7,163.0)` copy Burnout's real result into made-up summaries (library_query:393 says "Burnout's real 163.0 ms"); `test_activation_rows:755` builds `SPSqueeze{SqOut,-163.0}` by hand and shows "squeeze out 163.0 ms". Snapshot: "163.0 ms" read from the real Burnout record and typed as the expected text. Sound: scalar formatter tests | X9: `LibraryModel::set_summaries`, `query_matches`, `activation_badges` (`src/app/path_view.h:168`). Snapshot: `build_activations`, `build_path_buttons`, pb and GUI text | X9: test_library_model:353,589,599,834; test_library_query:332,336,350,393,395,397,429; test_activation_rows:755. Snapshot: test_activation_rows:601; test_path_list:281; pbtest_build:156; uitest_details:1071; uitest_paths:123,264,273,293; uitest_preview:782,1039. Sound: test_activation_rows:579; test_display_format:62; test_model:973; test_rules:197 |
| 17 | 10000.0 | sound | Song length and position; DM percent sort keys of rows built from typed leaderboard scores | none | all places |
| 18 | 3072 | sound | Ticks (engine on hand charts) | none | all places |
| 19 | 2000.0 | sound | ms positions on hand scenes; `test_cli:706` act_ms of a hand chart (768 ticks at 192 and 120 BPM is 2000 ms by rule) | none | test_cli:706; test_preview_boxes; test_replay:787,809; all others |
| 20 | 378315 | snapshot part, X9 part | Snapshot: Burnout's real best score typed as the expected value (no rule beside it; X1). X9: the library's expected "378,315  3- 1 2" is read back from the made-up `ready(378315,...)` | X9 part: `LibraryModel` / `library_cell_text` | Snapshot: test_activation_rows:587; test_path_list:278; test_pb_coach:66; test_stars:362; pbtest_build:147,148,162; pbtest_coach:522; pbtest_window:604,613; uitest_details:1032,1057; uitest_paths:119,121. X9: test_library_model:176,859 |
| 21 | 150 | sound | Speed percent and a clamp the test types (`preview_volume=150`); DM speed from typed JSON; `test_replay:193` chord totals on a hand chart | none | all places |
| 22 | "Artist" | sound | Default metadata text of fixtures and test-written metadata; the header names in `uitest_library:951` copy the owner (K2), they are not an engine value | none | all places |
| 23 | 2304 | sound | Tick (measure 4); `test_tied_variants:476` checks the engine's pathstring on a hand chart against ticks the test states | none | all places |
| 24 | "3- 1 2" | X9 part, snapshot part, sound part | X9 (already in X9): `test_overlay_layout:342` wraps the typed label and `:510` measures it. Snapshot: the real Burnout path text typed. Sound: `test_sp_meter_curve:948` uses it as a lookup key into the real paths; `uitest_paths:479` only checks "not Burnout" | X9: wrap and trim helpers in `src/ui/overlay_layout.*`. Owner of the real text: `Path::pathstring()` `src/core/model.h:891` | X9: test_overlay_layout:342,510. Snapshot: test_activation_rows:586,1334; test_path_list:276,280; test_store_records:1195; uitest_details:1060; uitest_paths:102,122; uitest_preview:679. Sound: test_sp_meter_curve:948; uitest_paths:479 |
| 25 | 1234 | X9 part, rest sound | `put()` writes `notecount = 1234`, "old-path-A" and typed scores into a stored record, then the fill report joins it ("r.notes == 1234"). Rest: scalars given to `analyze_shown_label`, `scan_*_text`, `format_cost`, `count_line` (the rule is stated in the expected text) | the fill report join (`src/app/fill_report.*`) | test_fill_report:62-70,139; test_app_click:1069; test_batch_text:171,172; test_path_costs:202; test_report_view:236; all others |
| 26 | 1536 | sound | Tick | none | all places |
| 27 | 0.25 | sound | Chart offset typed into chart text; parse input | none | all places |
| 28 | 384 | sound | Tick | none | all places |
| 29 | "notes.mid" | sound | File name the test creates | none | all places |
| 30 | 100.0 | X9 part, rest sound | `a.sqinouts.push_back(SPSqueeze{SqIn,100.0})` then `box_for(a).notation == "0+ (100.0 ms)"`: a hand-built activation with a squeeze ms | `build_next_act_box`, `next_act_box_at` (preview boxes) | test_preview_boxes:965-974; test_audio_player:25-36; test_note_window:87; test_song_length:304; all others |
| 31 | "Expert" | sound | A setting typed, or the owner's default (K2) | none | all places |
| 32 | 5000.0 | sound | Song length and audio tail set by the fixture | none | all places |
| 33 | 400 | sound | Widths, ms; `test_tied_variants:414` filters engine paths on a score of a hand chart; `test_preview_boxes:818` "+400" is the solo bonus on a hand scene | none | all places |
| 34 | "Charter" | sound | Fixture default text; `test_library_query:257,544` are marked "made up" and feed a search row built from typed text (input to a text search, not engine output) | none | all places |
| 35 | -250.0 | sound | A delay typed into an ini, mix padding; engine sqout offsets on hand charts (sp_end_clamp states "3456 against 5376") | none | all places |
| 36 | "\\notes.chart" | sound | A path the test builds | none | all places |
| 37 | "No Expert Pro Drums notes in this chart." | echo part, sound part | `test_user_messages:121` types the sentence into `ChartFileError(AlreadyPlain, msg)` and checks it comes back (the following lines check the real `NoNotesError`). The other places check wording that the real `NoNotesError` or `prepare_failed_chart` produces against the typed copy: a copy of the owner `no_notes_message` (an X1/K2 kind), not a stand-in | `plain_error` (`src/app/user_messages.*`) | Echo: test_user_messages:121. Sound: test_analysis:1008; test_app_click:804,808; test_library_model:439; test_report_rows:564,597; test_store_records:876; uitest_report_windows:1191,1196 |
| 38 | 1500.0 | sound | Positions and lengths the test builds | none | all places |
| 39 | 48000 | sound | Sample rate of audio the test builds; `test_audio_decode:155` asserts the real Opus decode rate with its reason in a comment; the three untraced places (test_preview_load_progress:341, test_preview_transport:62, test_stream_mix:168) assert the output rate of a hand-built source, not a chart result | none | all places |
| 40 | 4000.0 | sound | Lengths and positions of hand scenes; `test_sp_meter_curve` bars on hand charts | none | all places |
| 41 | 260 | X9 part, rest sound | `near.backends[0].offset_ms = -5.0` on a hand-built `Activation`, then `points == "260"` shown by the row. Rest: layout px; `backend_row_value(...,460,260,...)` scalars of the engine rule | `build_activations` (`src/app/path_view.h:155`) | test_activation_rows:500-515; test_model:881-905,1335; test_column_widths; test_long_paths:219 |
| 42 | "Burnout" | sound | A title of a chart or library row used as search or sort input, or read from the real corpus chart | none | all places |
| 43 | 0.001 | sound | Tolerances and offsets | none | all places |
| 44 | 50.0 | sound | Scalars of the squeeze helpers; `word(50.0,false,TransferScale{1.0,2.0})` states its input in the call | none | all places |
| 45 | "Normal" | sound part, unclear part | `summarystr` and tier-table tests state their input ms. Unclear: `test_activation_rows:558,560` (expected rating on a view whose setup I did not trace) and `test_report_view:339` (song names list, as RP1 said) | none | test_model:935; test_rules:179-241; test_report_rows:769,771; test_squeeze_rating:756,917; test_activation_rows:558,560; test_report_view:339,446 |
| 46 | 4000 | sound | Screen sizes, ticks | none | all places |
| 47 | 1200 | sound | Sizes; library copy counts of a hand-made `ChartIndex` (a library scan input, not engine output); a settings label and count typed into `nothing_under_settings` | none | test_report_page:173; test_report_rows:897; all others |
| 48 | "song.ini" | sound | File name | none | all places |
| 49 | 250 | sound | Inputs; `test_replay:1306` message on a hand chart | none | all places |
| 50 | 800 | snapshot (probable), rest sound | `test_preview_boxes:775` `path.score_solo == 800` is a literal typed in the same style as the "pinned from one run" case at line 515; the comment at 773 does not state a rule. `test_stars:258` is the engine rule on a hand chart (sound) | none | test_preview_boxes:775; test_stars:258; all others |
| 51 | 60.0 | X9 part, rest sound | `limited[1].timing == "60.0"` is a hand-built `BackendSqueeze{+60 ms}` row shown by `build_activations`. Rest: waits, the default constant (K2) | `build_activations` | test_activation_rows:364-402; test_config:255,284; pb waits |
| 52 | 400.0 | X9 part, rest sound | `SPSqueeze{SqIn,-400.0}` plus `set_transfer(1.6,1.0)` on a hand activation; the comment copies a real result "Sun of Nothing act 5: ... eff. 307.7 ms" | `sentence_of`, `build_activations` (`src/app/path_view.h`) | test_activation_rows:974-982; test_model:686; all others |
| 53 | 7680 | sound | Ticks on hand charts | none | all places |
| 54 | 12.5 | X9 part, rest sound | Hand-built squeeze or summary values shown or queried: `SPSqueeze` 12.4/12.6/12.5 (badges), a `Path` with `SqOut -12.5` (`button_of`), `PathRow.hardest_ms = 12.5` then `summary()`, `analyzed(7,12.5)`. Rest: `test_library_jobs:179` clock estimate, `test_model:1343` offset rule | `activation_badges`; `build_path_buttons`; the record summary from stored paths (`src/store/record_store.cpp:170,382,629`); `query_matches` | test_activation_rows:745-751; test_path_list:55-61; test_chart_index:100,127; test_library_query:344; test_library_jobs:179; test_model:1343 |
| 55 | 255 | sound | Colour channel | none | all places |
| 56 | "src/a.cpp" | sound | Fixture file name in temp repos | none | all places |
| 57 | "(unknown)" | sound | Checks of the owner's text on real loaders (`display_title`, `title_or_unknown`); a typed copy of `kUnknownTitle`, an X1/K2 copy, not a stand-in | none | all places |
| 58 | 9600 | sound | Ticks; `test_fills_and_skips` act_ticks of the engine on a hand chart with 9600 stated in the chart | none | all places |
| 59 | 600 | sound | Layout inputs | none | all places |
| 60 | "000" | sound | Digit-separator artifact | none | all places |
| 61 | 44100 | snapshot part, rest sound | `frames_of_ms(1.0, 44100) == 44  // pinned from one run` is a snapshot (the rule, 44.1 rounded, is derivable, but the comment says pinned). Rest: a sample rate the test builds | none | test_audio_player:139; all others |
| 62 | 2026 | sound | Dates in test text | none | all places |
| 63 | 1440 | sound | Screen height, ticks | none | all places |
| 64 | 4800 | sound | Frames and ticks of the test's song | none | all places |
| 65 | 240 | sound | Ticks | none | all places |
| 66 | 3000.0 | sound | Positions and lengths | none | all places |
| 67 | 3000 | sound | Ticks, screen | none | all places |
| 68 | "feature" | sound | Branch name in temp repos | none | all places |
| 69 | 20.0 | X9 part, rest sound | `analyzed(7,20.0)` made-up facts (345,352,360,362); `-20.0` SqIn on a hand activation shown as "no more than 20.0 ms late" (971). Rest: the typed query text and its parse (343,609,616), an `ms_filter` setting read back (config:838), a clock | `query_matches`; `sentence_of` | test_library_query:343,345,352,360,362,609,616; test_activation_rows:971; test_config:838; test_engine_sweeps:148; test_library_jobs:139 |

## X9 instances, grouped by the function they feed

Each is a made-up value shaped like engine output. "Real result copied" means the typed value equals a real chart result.

`LibraryModel::set_summaries`, `set_summary_for`, `set_summaries_for` (`src/ui/library_model.cpp`). The engine owner of the values is the stored record's best path (`engine_hardest_ms` at `src/store/record_store.cpp:629`, the score from the search). The helper `ready()` at test_library_model.cpp:52 builds a `RecordLookup` by hand.
- test_library_model.cpp:353,589,599,834: `ready(378315,"3- 1 2",7,163.0)`. Real result copied (Burnout score, path, squeeze). Also :589 `ready(500000,"2 2",7,12.0)`.
- test_library_model.cpp:176,859: the expected "378,315  3- 1 2" is read back from that made-up summary.
- test_library_model.cpp:426,720,723,754,783,796: `ready(1000,"1",3,0.0)` and `ready(2000,"2",4,0.0)` (783). Not a copy.
- test_library_model.cpp:817: `ready(400000,"1 1",6,10.0)`.

`query_matches` (`src/app/library_query.cpp:451`), fed a hand-built `RowFacts` (stars, hardest ms) by `analyzed()` at test_library_query.cpp:30. The real facts come from `facts_of` (`src/ui/library_model.cpp:47`) over `shown_timing(summary.hardest_ms)`.
- test_library_query.cpp:332,336,350,393,395,397,429: `analyzed(7,163.0)`. Real result copied (line 393 says so).
- :344 `analyzed(7,12.5)`; :345,352,360,362 `analyzed(7,20.0)`; :421,427 `analyzed(7,163.0)`; :424 `analyzed(7,163.5)`.

`build_activations`, `activation_badges`, `sentence_of` (`src/app/path_view.h:155,168`), fed a hand-built `Activation`. The engine makes real ones at `src/search/engine.cpp:2063` and `src/core/model.cpp:828`.
- test_activation_rows.cpp:364-402: backend squeeze rows -30.0, -100.0 and +60.0 ms at ticks 200, 100, 300, shown as "-30.0" and "60.0".
- test_activation_rows.cpp:500-515: `offset_ms = -5.0` on `near.backends` and `sqinouts`, shown as points "260".
- test_activation_rows.cpp:745-751: `SPSqueeze{SqIn,12.4/12.6/12.5}` shown as "squeeze in 12.5 ms".
- test_activation_rows.cpp:755: `SPSqueeze{SqOut,-163.0}` shown as "squeeze out 163.0 ms". Real result copied.
- test_activation_rows.cpp:971: `SqIn -20.0` shown as "no more than 20.0 ms late".
- test_activation_rows.cpp:974-982: `SqIn -400.0` with `TransferScale{1.6,1.0}`; the comment copies a real result (Sun of Nothing act 5, eff. 307.7 ms).

`build_path_buttons` / `button_of` (`src/app/path_view.h:265`):
- test_path_list.cpp:55-61: a hand `Path` with `SqOut -12.5`, shown as "12.5 ms".

`build_next_act_box` / `next_act_box_at` (preview boxes):
- test_preview_boxes.cpp:965-974: `SqIn 100.0` shown as "0+ (100.0 ms)".

`dynamics_kick2x_line` and `dynamics_kick2x_fact` (`src/app/dynamics_breakdown.h:143,152`). The real value is `count_dynamics(song)` at line 167.
- test_dynamics_breakdown.cpp:270-281,312,320,322,326: Kick 1700 and Kick2x accent 300 set by hand.

`AppState::tick_library` row scores, through the batch analyzer `gated_scores` (test_app_library.cpp:204-214; `scored_record` at :159 sets `best.notes_points` by hand on an empty record):
- test_app_library.cpp:354,355,368,387,523,537,538: shown as "1,000" and "2,000". Weak: the score is a marker that tells charts apart while the test checks job gating, but the code under test reads and shows it.

Stored record summary (`src/store/record_store.cpp:170,382,629`) and the fill report join:
- test_chart_index.cpp:100,127: `PathRow.hardest_ms = 12.5` on a hand record, the summary read back.
- test_fill_report.cpp:62-70,139: `put()` writes `score`, `actcount`, the path text "old-path-A" and `notecount = 1234` into a stored record that the report joins.

Path filter (`path_report_view::path_keep`):
- test_report_view.cpp:450-451: `path_row("A",...,500.0,true)`, a made-up report row with a hardest ms.

Overlay wrap and trim (already in X9; listed for completeness; copy of Burnout's path text):
- test_overlay_layout.cpp:342,510: "3- 1 2  (optimal)" and its trimmed form.

## Snapshots (expected value typed, no rule beside it)

Burnout's score 378,315, path "3- 1 2" and "163.0 ms" typed as expected values at the places listed in rows 16, 20 and 24 (test_activation_rows, test_path_list, test_pb_coach, test_stars, test_store_records, pbtest_*, uitest_*). They overlap X1. Also test_preview_boxes.cpp:520,706 (`score_solo == 200`, comment "pinned from one run"), :775 (800, probably the same kind), and test_audio_player.cpp:139 (`frames_of_ms(1.0,44100) == 44  // pinned from one run`).

## Echo

test_user_messages.cpp:121: the sentence typed into `ChartFileError(AlreadyPlain, msg)` and checked back. The lines after it check the real `NoNotesError`, so the test as a whole is not empty.

## Counts

Rows: 69.
- Rows with at least one X9 place: 1, 2, 4, 10, 11, 13, 16, 20, 24, 25, 30, 41, 51, 52, 54, 69 (16 rows). X9 places in total: about 70.
- Rows with a snapshot part: 4, 16, 20, 24, 50, 61 (6 rows).
- Rows with an echo place: 37 (1 place).
- Rows with an unclear place: 13 (line 253), 45 (lines 558, 560, 339).
- Sound only: 3, 5, 6, 7, 8, 9, 12, 14, 15, 17, 18, 19, 21, 22, 23, 26, 27, 28, 29, 31, 32, 33, 34, 35, 36, 38, 39, 40, 42, 43, 44, 46, 47, 48, 49, 53, 55, 56, 57, 58, 59, 60, 62, 63, 64, 65, 66, 67, 68 (49 rows). Row 37's other places and row 45's rule tests are also sound.

Rows not reached: none. Limits: for the big engine-on-hand-chart rows (1, 2, 4, 6, 8, 9, 40) I read every place's line but not every test's surrounding setup, and I did not re-derive each expected number, so a number pinned with no comment would be a snapshot I did not catch. Lines I did not trace: test_app_library:253, test_activation_rows:558,560, test_report_view:339, test_preview_boxes:775.
