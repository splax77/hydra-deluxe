agent id: a6012443f605a5551 (T5-V-X9-RP3, verifier of repeats/X9-RP3.md). Paths are under `tests/` unless they start with `src/`.

I opened every X9 and snapshot place, the places of most sound rows, row 207 and the second and third places of row 205. After my first draft the coordinator passed on the user's rulings (tests/pinned to typed constants, cited non-code, made-up strings; see `docs/agents/rulings/tests-and-runs.md` and X9 to X12 in cross-findings.md). Under them an X9 stays X9 even when the expected text follows correctly from the typed input, so I withdrew my first draft's disputes on that ground. Verdict words used here: X9 (made-up engine-shaped input), snapshot (expected value pinned from one engine run, no rule beside it), X10 (an expected value that is a typed product constant or text), X12 (an expected value whose stated source is a run, plan, mockup, doc or saved file), sound.

## X9 instances: I agree with every one

| Place | Note |
|---|---|
| test_library_model.cpp:357, 599, 834 (`ready(378315,"3- 1 2",...)`) | `ready()` (line 52) builds a record by hand: invented best path, score, stars, hardest ms, fed to `shows_same_as`, `set_summary_for`, `set_summaries_for`. Copy of Burnout's numbers, so X1 too. |
| test_library_model.cpp:517 (`ready(300000,"1 1",6,40.0)`) | Invented record handed to the model. |
| test_library_model.cpp:1081 (`ready(300000+i,"1 2 3",...)`) | Invented path text in a 20,000-row timing test. X9 by the ruling; the weakest case, since size is what the test measures. |
| test_library_query.cpp:424 (`analyzed(7, 163.5)`) | Made-up summary handed to the query. The number is also picked as a boundary, which the ruling does not excuse. |
| test_library_query.cpp:494 (`analyzed(3, 40.0)`) | Invented summary. Its value is never read by the empty-query answer, so it is pure filler, still X9. |
| test_library_query.cpp:556 (`search_field(...,"1 2 3")`) | Made-up path text. The function only folds and joins, so this is also close to an echo (X11). |
| test_activation_rows.cpp:745 (`SPSqueeze{SqIn,12.4}`) | Made-up squeeze result handed to the badge text. Real owner of the value: the search that fills `Activation::sqinouts`. |
| test_model.cpp:1286, 1292 (`one_step(59520,71040)`) | Hand-made Activation with Burnout's numbers. |
| test_pb_text.cpp:118-126 | Made-up cost list fed to `choice_order`. The REQUIRE'd list of offered fills beside it is a typed copy of engine output (X3). |
| test_preview_boxes.cpp:783-796 | Hand-made `PreviewScene` (bonus 12000) fed to `solo_box_at`; the real scene comes from `build_preview_scene`. |

Added X9 instances. The reader named 5 `ready(...)` lines in test_library_model.cpp. The file has 27 uses of the helper. The same invented records go to the same functions at test_library_model.cpp:87, 354, 358, 401, 426 (`ready(1000,"0",1,1.0)`), 541, 566, 608, 618 (`ready(500000,"2 2",7,12.0)`), 589, 688 (`ready(1000,"1",7,100.0)`), 703 (`-315.8`), 720, 723, 754, 783. I read the line, not each test body.

## Snapshots and cited non-code

| Place | Reader | Mine | Why |
|---|---|---|---|
| test_path_costs.cpp:209, 219, 233 (9984, 22272, 37632, 49920) | snapshot | agree, and X12 | Comment at 196 says "Allies' best path by activation only": the engine's answer on a real chart, no rule beside it. The file header (lines 4, 74) says its totals are "pinned from one run" (X12). Added: the expected totals 407610 (about line 219) and 451830 (about line 240) are engine scores typed the same way. |
| test_replay.cpp:197-198 (row 153) | snapshot | agree, X12 | Line 190: "Pinned from one run". All of 150, 50, 300, 350, 500 are pinned. |
| test_replay.cpp:1537 (row 198) | snapshot | agree, X12 | Comment at 1515-1517: "Every value is a literal pinned from one run". The other straddle values (180, 130, 310, 50) are the same. |
| test_sp_meter_curve.cpp:464 (12960) (row 164) | snapshot | partly dispute, X12 | `make_early_sqout_song` (record_fixtures.h:97) states in its own comment that the third phrase ends at 12960 (6750 ms) and the plain end is 13440 (7000 ms), so the tick is a chart input, not an engine answer. But the check's own comment says "The engine's record: ...", which X12 names. Verdict X12, not snapshot. |
| test_sp_end.cpp:220, 689 (12960) | snapshot | dispute | Same chart tick, no run cited at those lines in what I read. Sound as a chart input. |
| test_activation_rows.cpp:1367 (85.1, 61.9, 48.1) (row 187) | snapshot | agree | Focus [Ariana Grande], real chart, no rule beside the typed row and tag. Added: lines 1340-1341 ("frontend 84.5 ms ...", "5.4 ms safe") are the same. |
| pb_ui/pbtest_window.cpp:92 ("0:00.000") (row 149) | snapshot | X12 | The value is `clock_str(0)`, a rule that test_display_format.cpp:36 checks directly. But the comment cites a run ("from one run"), which X12 names, and the string is a typed product text (X10). |
| test_preview_timeline.cpp:556 (19 labels) | snapshot | X12 | The comment beside it derives 19 (30 s is 50 px, the gap, 20 steps less the start), so the value is derivable. The comment also says "Pinned from one run", a cited run: X12. |

### Added snapshot, X10 and X12 places

- Row 171 moves from sound to snapshot and X12 at test_preview_boxes.cpp:523 and 549 (380, "star 4 in 380"). Comment at line 516: "The literals below were pinned from one run". Scores 180, 480, 580 are the same. Lines 505-510: "Pinned from one run: this chart's best path earns all seven" (`end.stars == 7`, a corpus chart).
- test_fill_deadline.cpp:116-140 (the file of rows 190 and 197; the 200.0 value there is a tempo input and stays sound): the `want[2][3][3]` table (23666.67, 22666.67 and 16 more) is declared "pinned from one run of that code", and the header (line 9) says all expected values are. Snapshot and X12. It copies the output of `activation_fill_deadline_ms` in src/search/graph.cpp.
- test_replay_schedule.cpp:476-497 (file of row 200): the reader's "seen" is confirmed. Real squeeze numbers go in as input; expected aims and rooms (-62.72, 39.55, 11.04 ...) are typed to 0.01 ms with no rule beside them. Snapshot.
- test_tied_variants.cpp:347 (`totalscore() == 2650  // pinned from one run`): snapshot, X12. :583 (`e_difficulty(true) == Approx(49.479)`): the comment at 546-552 gives 49.5 ms but not the rule behind 49.479. Snapshot.
- test_song_length.cpp (header line 5: "Every expected length is a literal pinned from one run") and test_timeline_view.cpp (header line 3, same): X12 on the header's words. The values I read (test_song_length.cpp:240-262, test_timeline_view.cpp:70-90) follow from stated rules and arithmetic (600000 - 45000 = 555000), so they are not snapshots, but the cited source is a run.
- X10, typed constants and product text found in the files I opened: test_stars.cpp:219-221 (`kStarMultipliers == {0.1,0.5,1.0,2.0,2.8,3.6,4.4}`, the constant retyped), test_stars.cpp:222-223 (`cutoffs_for(100000)` against 10000 ... 440000, which is the same table times 100000), test_dm_report.cpp:822 (the status definition text "No paths: analyzed, but the analysis kept no path." searched for in `dm_columns()`), test_user_messages.cpp:100-107 (the "Hydra can't analyze this chart because ..." messages typed as the expected text; the behaviour is which refusal gives which message, not the spelling). test_library_query.cpp:393 ("Burnout's real 163.0 ms") is X12.

## The four questions in the brief

- test_column_widths.cpp:66 (row 193, reader's unclear): X9 as a made-up value, but nothing depends on it. The rows go to `measure_widths` with five `text_spec` columns, and the test at lines 202-215 builds the same table with 200-character cells and checks the widths do not change. So the text is never read. By the ruling it is still a made-up Library path ("1 2 3", "2- 1"); by effect it is filler. I record it as X9, weakest.
- test_tied_variants.cpp:522 and 575: the reader's sound verdict stands for the expected values. Line 522 expects `{9984, 10752}` for '0' and `{10752}` for '1'. The comment at 496-502 gives the rule and `fold_between_windows_song` (434-441) names the fills at 9984 and 10752, so the lists follow from rule plus chart. Line 575 expects `{9984}`, worked out in the comment at 541-552 from ready times (8448 is 22,000 ms, before fill 9984's end at 23,000 ms). The chart itself is hand-built, not engine output, so it is not X9. The `49.479` and `2650` in the same file are snapshots, see above.
- Row 207 ("script"): sound. pbtest_harness.cpp:76, uitest_harness.cpp:135 and uitest_main.cpp:139 compare a test's name with the harness's script-test name.
- Row 205 second and third places: sound for X9 (hand-built chart text at test_song_parse.cpp:809 and test_user_messages.cpp:96). The expected messages in test_user_messages.cpp:100-107 are X10 as above.

## Counts

- X9: agreed on all 10 places the reader listed (3 of the 5 library-model/query ones weak); added 16 `ready(...)` lines; test_column_widths.cpp:66 recorded as X9, weakest. None disputed.
- Snapshot: agreed on rows 144, 153, 187, 198; row 164 moved to X12; test_sp_end.cpp:220 and 689 disputed (chart input); rows 149 and near 157 moved to X12 (value is a rule or derivable, but a run is cited). Added: test_path_costs totals, test_activation_rows 1340-1341, test_fill_deadline table, test_replay_schedule table, test_tied_variants 347 and 583, test_preview_boxes row-171 group.
- X10 added: 4 places (above). X12 added: the headers of test_song_length and test_timeline_view, test_library_query.cpp:393, plus the X12 marks in the tables.
- Echo (X11): none clean; nearest is test_library_query.cpp:556.
- Sound verdicts moved: row 171 to snapshot and X12.

## Not reached

- The 111 "from one run" comments in the test folder. I read only those in files that hold a place of the reader's rows. Not read: test_pb_coach.cpp (14), pb_ui/pbtest_coach.cpp (16), pb_ui/pbtest_window.cpp (11 more), test_preview_timeline.cpp (16 more), test_activation_row_layout.cpp (8), pb_ui/pbtest_build.cpp (7), test_badge_pill.cpp (6) and 25 other files with 1 to 5 each. All are X12 candidates.
- X10 was not hunted across the folder; the four above came from files I opened for other reasons.
- Sound-row places I did not open past the sorter's line: test_fill_report.cpp:592, 596; test_report_rows.cpp:722, 731, 765-782; test_live_protocol.cpp:24, 31; test_long_paths.cpp:128; test_preview_source.cpp:278; test_preview_load_progress.cpp:240, 242, 326, 329; test_precheck_git.cpp:167, 201, 288; test_precheck.cpp:111; test_docs_match_code.cpp:725, 2147; test_fuzz_regressions.cpp:41; test_sp_end.cpp:249; test_sp_meter_curve.cpp:324, 513; test_rules.cpp:98; test_update_verify.cpp:78; test_activation_row_layout.cpp:46; test_badge_pill.cpp:36.
