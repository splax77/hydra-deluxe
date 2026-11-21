agent ab4c1cafa49befcf0 (verifier T5-V-RP2, checks `repeats/RP2.md`, rows 70 to 138; session 63c70014-4328-49dc-9205-742235732752)

## Short answer

The sorter's work mostly holds. I agree with all six candidates, with copies removed and added below. I settled row 129 and the RP2-3 question. I found no K3 or K4 row that hides a K1 or K2 part across files. A few single-file pins exist that the sorter did not name (below).

One fact first: no row in 70 to 138 has more than five files. The most any row has is five (rows 70 to 77, counted by distinct file in `repeats.tsv`). So the "unopened files" the sorter worried about are the other places inside the same files, not other files. I printed the asserted line of every one of the 695 places in rows 70 to 138 and read all of them, then opened the code around every place I doubted. I read cross-findings.md first. X1 covers Burnout's score, path text and 163.0 ms. X2 covers the fill ticks. Both cover what the sorter marked K1.

## The two settled questions

**Row 129, the "activations" key.** Nothing in `src/` writes it. A search of all of `src/` for `activations"` finds only the "Your activations" screen label. The writers are in `tools/`: `tools/replay.cpp:484` (the `score` output, which `test_cli:702` reads), `tools/replay_json.cpp:187` (`paths_json`, which `test_replay:670,713,901` read; the same file reads the key back at `:45,51`), and `tools/replay_schedule.cpp:1063` (the schedule, read by `test_replay_schedule:468,516,888,896,1595`). So the kind is K2 with a tool owner, not a `src/` owner: a key of the replay tool's own output format. Every place is a test of that tool's output, so the tests are the schema's own pin. A key rename turns all 13 places red, and also the Python readers in `tools/ch_mcp/game_tools.py:244` and `tools/ch_probe/experiments/play_chart.py:275`. The same case holds for `deact_ms` (row 130, `test_cli:707`, written at `tools/replay_json.cpp`/`tools/replay.cpp`) and `steady_ms_before_end` (`test_replay:789`). The value 12000.0 at `test_cli:707` is the test's own chart (acts "768:3072" at 192 ticks per beat, 120 BPM), so the number is K3 and only the key name is K2.

**RP2-3, "Expert Pro Drums, 2x Bass".** The four `test_store_open` copies (749, 874, 905, 1009) are an old stored value, not today's `chartmode_key`. Line 749 is `kOldDatabaseChartMode`, commented "Every fixture row's chart mode (README 'Schema and stamps')". Line 874 reads `SELECT chartmode FROM profiles` from the real old database file. `testdata/store/README.md:77` says "`chartmode` on every row | Expert Pro Drums, 2x Bass". Lines 905 and 1009 build the profile that must match those stored rows. Those are K4 (a fact of a stored fixture file). `src/app/config.cpp:331-335` even says the words "are stored with every result, so a new spelling would no longer find the results already in the store". That makes these four the correct pin of the stored text, and a change to `chartmode_key` should not turn them red. The real K2 copies of `chartmode_key` are only `ui/uitest_library:164,217,241` (all three read `settings.chartmode_key()`). `dm_fixture.h:26` is a K3 input mode for fixture records. `test_report_rows:891,894` pass it as an argument to `nothing_under_settings` (`src/app/report.cpp:271`) and expect it echoed into a message, so they are K3. So RP2-3 shrinks to three copies in one file, which is not a repeat of three files.

## Table

All places of every row were read. Rows I agree with and did not change are grouped.

| Row | Value | Kind | Reason | Places |
|---|---|---|---|---|
| 70 | 5000 | K3 | Lens depth input, a percent, a score, a user count 5000 (`test_dm_report:729`) | all 12 read |
| 71 | 130 | K3 + single K2 pin | Widths, built-chart bonus. `test_stars:259` types the star cutoffs 130, 650 ... 5720 from the star table; `test_stars` is that table's own test | test_stars:259,337 |
| 72 | -10.0 | K3 | Inputs and the text built from them | all read |
| 73 | 378,315 | K1 | Agree; X1 | test_path_list:278, pbtest_build:147,148, pbtest_window:604,613, uitest_details:1032,1057, uitest_paths:119,121 |
| 74 | .chart | K3 | Agree | all read |
| 75 | 85.0 | K2 (fewer copies) | See disputes. `test_config:285,302` are the test's own value (`at85.early_fill_window_ms = 85` at `:284`): K3. Real copies: `test_replay_schedule:281` (tool output of `kWindowBackMs`), `:355` (greps `model.h` for the text `constexpr double kHitWindowMs = 85.0;`), `test_report_rows:776`, `test_note_window:84,85` and `test_squeeze_rating:769` (the owners' own tests) | see left |
| 76 | Bloodline | K3 + K4 | `test_library_model:180,860`, `test_dm_report`, `test_library_query`, `test_song_input` build their own text. `uitest_library:663,669` read the real corpus charter: K4 | agree |
| 77 | \\notes.mid | K3 | Agree | all read |
| 78, 79 | 6000.0, 8000.0 | K3 | ms of built charts and curves | all read |
| 80 | 2880 | K3 | Ticks of built songs | all read |
| 81 | 13440 | K3 | `beat_song({}, {}, 13440)` hand-built; the other places pin the engine's answer on that built chart (`deact_edge_at`, `deact_tick`) which equals the tick the test placed | record_fixtures.h:509 and 12 more |
| 82 | 11520 | K3 | `sp_act_at(..., end_tick=11520)` at `test_sp_meter_curve:92`; `test_replay:529` reads it from a JSON the test writes; `test_tied_variants` uses a built tail song | all read |
| 83 | 12.6 | K3 | Inputs; `test_display_format:60` is a rounding check | all read |
| 84 | 0.01 | K3 | Tolerances | all read |
| 85 | 250.0 | K3 | Agree | all read |
| 86 | 30.0 | K3 | Agree; `test_live_protocol` timeouts are inputs | all read |
| 87 | 170 | K3 + K2 text part | Agree. Also `test_column_widths:146,214` type the Library column widths 170, 100, 90, 390, 250 from shares in a comment ("Title 17%"); owner `kLibrarySpecs` (a src spec table); one file, so a single pin | agree |
| 88 | 12.0 | K3 | Agree | all read |
| 89 | Expert Pro Drums, 2x Bass | Split: K4 (4 places in `test_store_open`), K2 (3 in `uitest_library`), K3 (`dm_fixture.h`, `test_report_rows`) | See RP2-3 above | see above |
| 90, 91 | 1100.0, 900 | K3 | Agree | all read |
| 92 | main...feature | K3 | Fixture git range | all read |
| 93 | 170.0 | K2 | Agree; owners `kFormulaScale` `note_window.h:39`, `timing_tiers()` edge `nominal_budget_ms()` `squeeze_rating.cpp:246` | agree |
| 94 | 100000 | K3 + single pin | `test_stars:222` star-table row (owner's own test) | agree |
| 95, 97, 98 | -50.0, -12.0, 12345 | K3 | Inputs; `test_batch_text` formats its own number | agree |
| 96 | 0 4 1 | K1 | Agree; X1 | agree |
| 99 | 1000000 | K3 + single pins | `test_midi:524` pins the MIDI message-length limit text "maximum length 1000000" (owner `src/parse/midi.cpp`, near `:389`): one K2 pin. `test_stars:341` "stops at 7": single pin | agree |
| 100 | 130.4 | K3 | `SPSqueeze` built in `test_model:992`; `pbtest_coach:182` builds the expected text from it | agree |
| 101, 113 | 378175, 378,175 | K1 | Agree; X1 | agree |
| 102 | 12000 | K3 | Agree | agree |
| 103 | max_tied_paths | K2 | Agree; owner `src/core/rules.cpp:78`, 7 places in 4 files | agree |
| 104, 105, 107 | 180, 6912, 1200.0 | K3 | Agree | agree |
| 106 | 3- 1 2  (optimal) | K1 for `test_path_list:434`, `test_preview_scene:998`, `uitest_preview:700`; K3 for `test_overlay_layout` | Agree. Label rule `preview_view.cpp:823` (`button.notation + "  (optimal)"`) confirmed. `test_path_list:434` passes the text into `path_item_id`, so it is a K3 input, not a copy. Real copies: `test_preview_scene:998` and `uitest_preview:700` | see left |
| 108 | Extreme | K2 | Agree. `test_report_view:339,342` sort 7 rows named by tier names the test builds as song names: K3 | agree |
| 109, 110 | 5760, -315.8 | K3 | Agree | agree |
| 111, 112 | 111, "111" | K3 | Agree | agree |
| 114 | 140 | K1 + K3 | Agree, `test_song_parse:666` is a tick | agree |
| 115, 116 | 1500, sine220.opus | K3 | Agree | agree |
| 117 | activation | K2 | Agree; `pather.cpp:366` read: `out.failed_reason = "activation"` | agree |
| 118 | 36480 | K1 | Agree; X2 | agree |
| 119 | Hard (uncounted) | K2 | Agree; `model.cpp:354` read | agree |
| 120 | 163.0 ms | Split K1 + K3 | Agree; X1 | agree |
| 121 | 0.05 | K3 + single pin | `test_preview_config:62` pins the default highway config `beats_z_past` read at `src/render/preview_config.cpp:135`; one file | agree |
| 122 | Beyond | K2 | Agree | agree |
| 123 | 50000 | K3 + single pin | `test_stars:222` | agree |
| 124 | cannot open file:  | K2 | Agree. Owners are not one: the text is typed at `src/audio/mapped_file.cpp:28,33,46,49`, `src/core/winstr.cpp:290`, `src/app/analysis.cpp:113`, `src/app/update_verify.cpp:54`. Four owners for one phrase | agree |
| 125 | 30000.0 | K3 | Agree | agree |
| 126 | Acid Romance | Split: K3 (about 15 places in `test_library_model`) and K4 (`uitest_details`, `uitest_library:664,668`) | `test_library_model:81` builds the entry `chart("acid", "Acid Romance", ...)` by hand; it is not the corpus title. Only the two uitests read the real chart (`testdata/input/.../song.ini:5` `name = Acid Romance`) | see left |
| 127, 128 | 5376, 3456 | K3 | Agree; `record_fixtures.h:114` sets the fill length; `uitest_report_windows:272` reads it back as "3,456" | agree |
| 129 | activations | K2 (tool owner) | See above | above |
| 130 | 12000.0 | K3 (+ K2 key part for `deact_ms`) | See above | above |
| 131 | 10.088 | K1 | Agree; `src/app/song_length.h` exists. Owner of the text format is `song_length_ms` plus the clock formatter; 11 places in 3 files | agree |
| 132 | notes.chart | K3 | Agree. `test_srb:259,271` assert `notes_filename` on metadata the test makes with the name `"notes.chart"` at `:298,310`: a pass-through, K3 | agree |
| 133, 135, 136, 137, 138 | 1080, 160, 9000.0, 7000.0, 120.0 | K3 | Agree | agree |
| 134 | Mutemath - Allies | K4 | Agree (a corpus chart name, looked up) | agree |

## Candidates

| Candidate | Verdict |
|---|---|
| RP2-1 Burnout paths, cost, clock | Agree. Copy changes: remove `test_overlay_layout:344,472,505` and `test_path_list:434` (K3 inputs, as V-RP1 also found). `test_pb_coach.cpp` and `test_fills_and_skips` hold X2's tick. The 2:10.088 clock and the cost -140 are new against X1 (confirmed: 11 places in 3 files for the clock, 5 in 3 files for the cost). |
| RP2-2 timing tiers and edges | Agree, with `test_config:285,302` removed from the 85.0 copies (the test sets its own early-fill setting to 85, a different constant). Add `test_replay_schedule:355` as a source-text pin of `kHitWindowMs` (it greps `model.h`). `test_report_view:339,342` are K3 (song names equal to tier names). The tier edges are owned at `squeeze_rating.cpp:241-251`, confirmed. |
| RP2-3 chart-mode key | Agree with changes: four `test_store_open` places are K4 (stored value in the old databases), not copies. Copies of `chartmode_key` are `uitest_library:164,217,241` only. |
| RP2-4 `max_tied_paths` | Agree. All 7 are the error message naming the key. |
| RP2-5 "activation" | Agree. `test_pb_coach:108,218` pass it in as input. |
| RP2-6 "cannot open file: " | Agree, and widen the owner list to the four `mapped_file.cpp` lines (28,33,46,49). The phrase is typed in six `src/` places, none a shared constant. A message change in one does not reach the others; the test pins four of the throw sites. |
| New RP2-7 | The replay tool's output keys (`activations`, `deact_ms`, `act_tick`, `steady_ms_before_end`): K2 with a `tools/` owner, 13+ places in 4 test files, and the Python tools read the same keys. |

## K3 and K4 rows checked and moved

All 62 K3 and K4 rows had every place read. None moved to K1. These carry a K2 part the sorter missed, each in one file only: row 99 (`test_midi:524`), rows 71, 94, 123 (the star cutoffs at `test_stars:222,259`, which is the star table's own test), row 87 (`test_column_widths:146,214`), row 121 (`test_preview_config:62`). Row 126 moved from K4 to a split of K3 and K4. Row 129 moved from Unclear to K2 (tool owner). Row 89 moved from K2 to a split of K4, K2, K3.

## Copies added

`test_replay_schedule:355` (source text of `kHitWindowMs`), `test_replay:670,713,901,903,909` and `test_replay_schedule:468,516,888,896,1595` (readers of the replay output key), `tools/ch_mcp/game_tools.py:244`, `tools/ch_probe/experiments/play_chart.py:275`.

## Copies removed

`test_config:285,302` from the 85.0 copies. `test_overlay_layout:344,472,505` and `test_path_list:434` from the "(optimal)" copies. The four `test_store_open` places from the chart-mode copies. `test_report_rows:891,894` and `dm_fixture.h:26` from the chart-mode copies (K3 inputs).

## Counts per kind (my reading, each row counted once under its main kind)

- K1: 8 rows (73, 96, 101, 106, 113, 114, 118, 131), plus the K1 part of 120.
- K2: 9 rows (75, 93, 103, 108, 117, 119, 122, 124, 129).
- Splits: 76 (K3, K4), 89 (K4, K2, K3), 120 (K1, K3), 126 (K3, K4).
- K4: 2 rows (134, and 126's uitest part).
- K3: the rest.

## Rows not reached

None. Every place was read at its line. I did not read the sorter's source-owner claims for `song_length_ms`'s formatter (only that `src/app/song_length.h` exists) or `test_stars`'s `stars_for_score` owner.
