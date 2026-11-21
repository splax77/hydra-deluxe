agent a49406ba3020062c8 (verifier T5-V-X9-RP1b, second verifier of repeats/X9-RP1.md, judged by the user's three rulings of 2026-11-15 in docs/agents/rulings/tests-and-runs.md)

Plain version. I opened the X9 places the first verifier left, then the places of the rows it did not reach. All the unopened X9 places stand except one scalar. Beyond the reader's list I found more X9 places in the replay, replay-schedule, preview-box and library-model tests, snapshots that say "pinned from one run", an X12 group in test_note_window.cpp, and several X10 constant pins. For rows 2, 4, 10, 30, 52, 54 and 69 I did not re-argue; I checked only that the input is made-up engine output and that the code handles engine output. It is, and it does.

## X9 places the first verifier did not open

- test_app_click.cpp:1069 (row 25, value 1234): DISPUTED. `analyze_shown_label(1234) == "Analyze shown (1,234)..."` feeds a count of shown charts to a label builder. The number is not engine output, and the expected text states the rule beside the input. Sound. The reader's X9 for row 25 stands only for test_fill_report.
- test_chart_index.cpp:100,127 (row 54): AGREED. `path_row()` builds a PathRow by hand (path "1-2-3", hardest_ms 12.5, avgmult 3.875, counts) and `record_with_paths().summary()` reads it back. The code under test reads engine records. A real input is `store.save(prepare_record(...))` of a real chart's result.
- test_path_list.cpp:55-61 (row 54): AGREED. A hand `Activation` with `SPSqueeze{SqOut,-12.5}` goes through `build_path_buttons` and comes out as "12.5 ms". The real input is a path found for a real chart.
- test_fill_report.cpp:62-70,139 (row 25): AGREED. `put()` takes a real analysed record as its base but overwrites best.path ("old-path-A", "new-path-A"), score, actcount and notecount (1234) with made-up values. `collect_fill_rows` then joins and lists them, so the expected "1.1 higher", delta 50000, paths and notes follow only from the typed values.
- test_library_model.cpp:176 and :859 (rows 20, 42): AGREED. "378,315  3- 1 2" and the library cell text are read back from `ready(378315,"3- 1 2",7,163.0)`, a made-up RecordLookup (helper at :52).
- test_library_model.cpp:426,720,723,754,783,796 (row 1): AGREED. `ready(1000,"1",3,0.0)` and `ready(2000,"2",4,0.0)` are made-up summaries stored and compared by the model. Weak in kind (the score is a marker for "changed or not"), but the code stores and shows engine output.
- test_activation_rows.cpp:364-402 (row 51): AGREED. Hand `BackendSqueeze` rows (-30, -100, +60 ms at ticks 200, 100, 300) go through `build_activations`; the shown "-30.0", "60.0" and "(uncounted) <-- squeezed out" follow from typed input.
- test_activation_rows.cpp:500-515 (row 41): AGREED. `near.backends[0].offset_ms = -5.0` on a hand Activation gives points "260" and "Hard <-- squeezed out (-200)".

## Rows disputed by the first verifier (not re-argued; input and code checked)

Row 2 (test_dynamics_breakdown 270-281): hand `DynamicsBreakdown` counts, fed to `dynamics_kick2x_line`, which formats engine counts. Rows 4 and 69 (test_library_query 343-362,421-427): hand `analyzed(7,x)` RowFacts, fed to `query_matches`, whose real facts come from `facts_of` over a stored summary. Row 10 (test_report_view 450-451): hand `path_row(...)` report rows, fed to the path filter. Rows 30, 52, 54 (test_activation_rows 743-757,965-984; test_preview_boxes 951-974): hand `Activation` with `SPSqueeze` ms, fed to `build_activations`, `sentence_of` and the next-activation box, which show engine output. All are made-up engine output into code that shows it. The reader's X9 stands for all.

## Places added (the reader called these sound; the first verifier did not open them)

X9, made-up Activation or path dump fed to code that serialises or shows it:
- test_replay_schedule.cpp:130,137,138,148,150,186,191,193,866 (rows 5, 6, 23, 28, 44, 46): a hand `Activation` with typed `sp_end_steps`, `sqinouts` and `transfer_post` goes through `schedule::activation_json`, and the JSON anchors ("anchor_tick 2304", "end_tick 3500", ratios 1.02/1.03) are read back. A real input would be a path searched on a chart. Moderate confidence: the code is tool-side serialising of engine output.
- test_replay.cpp:739,740 (row 12), :783,826 (row 26), :825 (row 28), :785,787,809,811,828 (rows 19, 40): `dump_one_activation` builds `act_at(timing, tick)` by hand and dumps it. Same kind.
- test_replay.cpp:518-528 (rows 3, 5, 53): a typed path JSON with act_tick 480, deact_tick 3840 and squeezes of 12.5 and 31.25 ms, fed to `windows_from_json` and read back. This is a parser of a dump file; the old-dump case cannot be regenerated, so I mark it X9 with low confidence. The tests at :574 and :623 use real dumps and are the sound shape.
- test_activation_rows.cpp:658 (row 12, value 3840): hand Activation through `build_activations`, expected `timeline_end == "m6"`.
- test_preview_boxes.cpp:663-668 (context), :888,890,893,894,905,914 (rows 17, 19, 32, 40): `TwoActs` (tests/preview_song_fixtures.h:174) builds two hand Activations, and :667-668 sets `sp_end_steps` by hand; the jump targets, "Next: activation 1 of 2" and "at m2.1.0" are shown from them.
- test_library_model.cpp:543,546,558,560,632,636,824 (row 42): row orders such as {"YYZ","Burnout","Chair",...} follow from the made-up `ready(...)` scores and stars.
- test_library_query.cpp:556 (row 24 kind): `search_field("Burnout","Green Day","<b>Hoph2o</b>","1 2 3")` with a typed path text "1 2 3" expected inside the search text. The same kind of string as the user's example.
- test_sp_meter_curve.cpp:763,764 (rows 8, 40): `sp_meter_readout` on `TwoActs`, hand Activations; X9 (soft).

Snapshot (value pinned from one run, comment says so, or no rule given):
- test_replay.cpp:193,199,200 (rows 5, 21): chord totals 150 and 500 under the comment "Pinned from one run" (:190).
- test_replay.cpp:1555 (row 21): `ghost.combo == 150` in a chord score table whose comment at :1516 says "pinned from one run".
- test_preview_boxes.cpp:535 (row 3): `after.score == "480"` under the comment at 513-516, "The literals below were pinned from one run".
- test_note_window.cpp:175,183,325,354,357,366,371,386,393 (rows 6, 12, 23): `pinned(157.14696125214215)`, `pinned(194.2891343767851)` and the like are expected values to 15 digits with no rule or named inputs; the file's own comment at :40 says "pinned to more places than the plan prints".

Cited non-code (X12): test_note_window.cpp:73-80 (comments "TM-21's two end values, as the plan pins them", "TM-21's three worked values", "TM-22") and :140-147 ("the squeeze-out sums from the mockup", `pinned(24.07876)`, `pinned(-65.87101)`). The stated source of the expected values is a plan and a mockup, not code. Also :265 ("the plan's figures give them"). The first verifier left these unclear; by the ruling they are X12.

Constant pin (X10):
- test_app_reports.cpp:637 (row 5): `AppState::kReportConfirmAbove == 500`.
- test_theme.cpp:91,134-145,185-187 (row 55): colour constants typed as hex or 255/204/51 and compared with the product's colours.
- test_badge_pill.cpp:148-152 (row 55): `check_rgb(safe.ink, 0xf2, 0xc3, 0x00)` and other typed colour values.
- test_model.cpp:1060 (row 57): `kUnknownTitle == "(unknown)"`. The other "(unknown)" literals (test_song_input:347-388, test_library_jobs:331, display_fixtures.h:194, test_model:1063, test_library_model:229-238,332-335) compare a function's output with a typed copy of that text: X10 (soft; blank giving the placeholder is the behaviour the call checks). The sort lists at test_library_model:346,348,975,1031 only use it as a sort key: sound.
- uitest_library.cpp:951,1338 (rows 22, 34): header labels typed as the expected column headers.

## Rows checked and sound

Rows 3, 5, 6, 7, 8, 9, 12, 14, 15, 17, 19, 21, 22, 23, 26 to 29, 31 to 36, 38 to 40, 42 to 44, 46 to 49, 53, 55 to 60, 62 to 68: I printed every place's line. Apart from the places listed above, these are ticks, widths, positions and lengths typed into hand charts or layouts, scalar helper tests whose rule is in the expected text, file names, dates and branch names.

Checked and kept sound with a reason: test_sp_end.cpp:221,512 (row 35: the stated 240 BPM arithmetic, comment 208-210); test_sp_end_clamp (states 5376/3456); test_model.cpp:597-621 (the Activation accessors on typed `sp_end_steps`, with the rule in the comments: the engine's own accessor on typed inputs, not a formatter; a judgement call); test_preview_boxes.cpp:608,633-635 (hand song with no path); test_replay.cpp:1306 (hand chart, message built by the code); test_preview_boxes.cpp:766,767,818 (solo box on `priced_path` of a hand song).

## Counts

- Reader X9 places opened by me and agreed: 8 groups (about 30 lines).
- Disputed: 1 (test_app_click.cpp:1069).
- Added X9: 8 groups (about 30 lines).
- Added snapshot: 4 groups (about 16 lines).
- X12: 1 group (about 8 lines).
- X10: 5 groups (about 45 lines, counting the "(unknown)" literals).
- Echo: 0 new. test_replay.cpp:518-528 is a typed dump read back (listed under X9).
- Sound: the rest, roughly 900 lines.

## Not reached

Row 18 (3072) was not printed. Rows 16 and 20 I left to the first verifier. For the printed rows I read each line but did not open the setup around every one (preview_scene, preview_timeline, sp_meter_curve, test_replay_windows, test_song_parse, test_timing); a hand `Activation` built far from the printed line would be missed. test_replay_schedule.cpp:504-1400, test_replay.cpp:756-860 beyond the printed lines, and the hand curves in test_sp_meter_curve.cpp were not read in full. test_stars, test_scoring:85,93 and test_dm_report were checked only on their printed lines.
