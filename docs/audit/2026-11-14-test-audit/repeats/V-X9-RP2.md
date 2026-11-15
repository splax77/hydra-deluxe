agent a48c5edfa2be252dc (verifier T5-V-X9-RP2, checking X9-RP2.md; session 63c70014-4328-49dc-9205-742235732752)

How I worked: I printed the asserted line of every place of every sound row in `repeats.tsv` (about 550 lines, rows 70 to 138 minus the K1, K2 and K4 rows), then opened the test around every place that holds a typed expected value, and searched all of `tests/` for the words "one run", "pinned", "oracle" and "the engine's record". I ran nothing. Where a number comes from arithmetic stated in the test (a chart's tempo, a measure length, a stated game rule) I called it sound. Where only a comment says "pinned from one run" and no rule is given, I called it a snapshot.

## Verdict on the reader's X9 instances

I agree with all of them. I opened each place.
- `activation_badges` fed a hand-built `Activation` (test_activation_rows:745-837, 877-921): agree. The hand-built record is typed, not engine output.
- `LibraryModel::set_summary_for` fed `ready(378315, "3- 1 2", 7, 163.0)` (test_library_model:566-816): agree. `ready()` types a stored result.
- Overlay layout and report view (test_overlay_layout:340,344,472,505; test_report_view:423,447): agree.
- Fill report (test_fill_report:128,131,248): agree. The scores and "old-path-A" are typed stand-ins for stored engine results.
- test_app_library:406,550; test_preview_boxes:795; test_store_results:641,645; test_report_rows:360; test_replay:508-529 (a typed path JSON fed to `windows_from_json`): agree.
- test_activation_rows:579 `format_ms(163.0)` (weak): agree.

Two of them are a judgement call, and I would send them to the user. In test_activation_rows:749,910,912,921 the 12.6 is chosen to test the stated one-decimal rule (D265, no rounding to whole ms). In test_library_query:353 the -12.0 is chosen to test that a negative bound reads as a number, and its neighbours 12.5, 20.0 and 2.5 in the same case are typed the same way but the reader did not flag them. The reader's own test was "a hand-built engine record fed to code that handles engine output". These numbers pass that test, but they are boundary values picked by a rule, not made-up labels. Either all the typed numbers of those two tests are X9, or only the copies of Burnout's real figures (163.0, -315.8) are. The reader's list is inconsistent between them. This is a split, not mine to choose.

## Verdict on the reader's snapshot instances

All four agreed (the place and the comment are as the reader says), with the extent widened.
- test_pb_coach:163,174,273; test_pb_text:159,177; pbtest_coach:182 (130.4 ms): agree. The whole case at test_pb_text:146-160 is "Pinned from one run", so `"m132 → m135"` (:153), `"2 phrases"` (:154) and `acts.size() == 5` (:150) are snapshots too.
- test_replay:1535,1536: agree. The comment at :1513-1515 says "Every value is a literal pinned from one run", so the whole case is the snapshot, not two lines. Add test_replay:1537-1590 (straddle.sp 360, sqout_sp 310, accent 50, the ghost row 100/150/300/200, the solo row 100/100/200/100/200, every combo_before/after and note range) and test_replay:1551 (`dynamics_bonus == 130`, row 71).
- test_sp_meter_curve:465,466 (13440, 14400): agree, with a limit. 12960 and 13440 follow from stated arithmetic (see test_sp_end:210). 14400 and the pair `{480, 1920}` at test_sp_end:812 have no rule beside them, and test_sp_end:813 repeats the same pair.

## Instances the reader missed (snapshots)

Each is a typed expected value taken from an engine run, with the file's own comment saying so, inside a row the reader called sound.
- Rows 71 and 104 (130, 180): test_preview_boxes.cpp:294 `CHECK(scene.score.steps[1].total == 180)`. The comment at :292 says "180 was pinned from one run as a fixed oracle". Also test_preview_boxes.cpp:515-560: the comment says "The literals below were pinned from one run", and :527 `before.score == "180"` sits in it (row 104).
- Row 114 (140): pbtest_build.cpp:168,177; pbtest_coach.cpp:529,535; pbtest_window.cpp:203 (`"Cost | −140"`, `"m20 → m24 2 phrases −140"`, `"−140"`). The file headers (pbtest_build:5-8, pbtest_coach:2) say every tick, sentence and figure "is pinned from one run", and the line at pbtest_build:175 repeats it. The reader marked these sound. The same holds for the neighbouring figures there ("378,175", "378,315", "313,635", "0:25.123 / 2:10.088"). Verdict: snapshot.
- Row 138 (120.0): test_fill_deadline.cpp:116-138. The `want[2][3][3]` table of deadlines (23666.666666666668 and so on) is "pinned from one run of that code" (:116-118), and the file header (:9) says all the file's expected values are "literals from one run". The place the reader listed, :130, is only the tempo input. The expected values it feeds are the snapshot. Verdict: snapshot.
- Row 70 (5000), found beside it: test_dm_report.cpp:86-90. `CHECK(*rows[0].pct_h == 9956)` carries the comment "99.56% for 1,000 under the first corpus chart's optimal, read from one run". It is in the file the reader opened for :491 (where 5000 is optimal/2, sound). It is a snapshot, though 9956 is not a row-70 value.
- test_sp_meter_curve:424-432 (`REQUIRE(sp_end_steps == {{5760, 13440, Activation}, {13920, 17280, SqIn}})` after a real search): a snapshot in the same sense as :465,466; the comment names "The engine's record".

Outside the K3 rows, so not counted here but found by the same search (other passes should check them): test_model.cpp:88-110 (the `Row rows[]` of cuts "pinned from one run of category_scores"); test_tied_variants:347 (2650); test_audio_decode:166,167; test_audio_player:139; test_label_layout:56; test_activation_row_layout and test_badge_pill (many "literal from one run" pixel values); test_best_finish:92; test_path_costs:4,74; test_engine_sweeps:579,742; test_fills_and_skips:350,446; test_note_shuffle:3; test_library_model:1092; test_library_query:650; test_dynamics_breakdown:434; test_store_results:606 (the hardest figure "none"); test_timeline_view:3; test_app_shell:344; pbtest_build:40,57,151,333 and pbtest_coach:88-100.

## Instances the reader missed (made-up stand-ins, X9)

- Rows 78, 79, 82, 130, 136, 137 (ms and ticks on hand-built curves): test_sp_meter_curve.cpp:85-135, 207-210, 281, 490-495, 605, 650. These build a `Path` and `Activation` by hand (`sp_act_at`, then `bank_rise_ticks = {960, 1920}`, `sp_end_steps = {{3840, 11520, Activation}, {5760, 15360, Collected}}`, `trailing_bank_ticks = {12480}`) and feed it to `build_preview_scene`, which handles engine records. That is the same shape as the hand-built `Activation` fed to `badge_texts`, which the reader called X9. The reader called these sound because the expected bars follow stated arithmetic (a measure is 2000 ms and so on). The expected values are sound, but the input is a made-up record. Some of these cases need a made-up record (a record "the record stamped nothing on", old records), and others have an engine-run twin in the same file (:405-470 run the search). Verdict: X9 for the hand-built record input; the reader's "sound" is wrong for those places. Which cases truly need a hand-built record is for the user. Fed function: `build_preview_scene`, via `sp_act_at` (`tests/preview_song_fixtures.h:139`).
- test_dm_report.cpp:118-130 (rows 111, 112, 115: ids "111", "222", elo 1500): a typed DMBot payload fed to `net::parse_users_json`. It is a made-up stand-in for outside data, not engine output, so it is not X9 as the reader defined it, and the reader's "sound" holds under that definition. Under the user's words ("not make up an arbitrary meaningless string") it is a canned string whose shape nobody checked against the real API. I mark it unclear and leave the verdict as the reader gave it.

## Sound verdicts I checked and keep

Rows 74, 76, 77, 84, 90, 91, 92, 94, 95, 98, 107, 109, 112, 115, 116, 121, 125, 126, 127, 128, 130 (test_cli:707 is a typed chart with a stated tempo map), 132, 133, 135: I opened every place. The expected values are arithmetic from the typed chart or a stated rule: test_sp_end:210-251 and 485-538 (240 BPM, "SP ends at 13440"), test_sp_end_clamp:100-140 and 327-340 (min(5376 + 1536, 3456 + 4 x 768) = 6528; at 2400 BPM a tick is 25/192 ms), test_tied_variants:100-170 (each end is its start plus 3072), test_stars:222,259 (the game's multiplier table times 1300), test_song_length:100-146 (stated delay and note times), test_timing and test_generated_fills:151,152 (the 7/8 restart grid is spelled out), test_model:600-650 and 1285-1320 (the test sets the fields it reads), test_replay:408-411 and 770-792 (the comments state the tempo arithmetic). The names (Bloodline, Acid Romance and the chart titles) are library rows typed as scan input, which is X1's copy, not X9.

## Sound verdicts moved

- Row 114 (140): sound to snapshot.
- Row 138 (120.0): the expected table of test_fill_deadline moves from sound to snapshot; the tempo place stays sound.
- Rows 71 and 104 (130, 180): "rest sound" is wrong at test_preview_boxes:294 and :527, and at test_replay:1551 with the rest of the :1513-1590 case.
- Rows 78, 79, 82, 130, 136, 137 (test_sp_meter_curve hand-built records): sound to X9 for the hand-set record input; the expected values stay sound.
- Row 70: test_dm_report:90 joins the snapshots (found beside the row's place).

## Counts

- Reader's X9 instances agreed: all (15 rows). Two groups (12.6 in activation_rows and -12.0 in library_query) are a judgement call for the user. None disputed outright.
- Reader's snapshot instances agreed: 4 of 4 rows, with the extent widened (the test_replay case, the test_pb_text case).
- Added snapshots: 6 places (test_preview_boxes:294 and :515-560, the five pb_ui places for 140, the test_fill_deadline table, test_dm_report:90, test_sp_meter_curve:424-432).
- Added X9: 1 group (test_sp_meter_curve hand-built records, about 8 cases).
- Sound verdicts moved: rows 71, 104, 114 and 138 to snapshot (in part); rows 78, 79, 82, 130, 136 and 137 to X9 for their input.

## Not reached

I read the places of rows 78, 79, 130, 136 and 137 in test_preview_timeline, test_preview_transport and test_preview_boxes (600-660, 910-965) only as the asserted line plus the test title; where I looked they follow stated tempo arithmetic. I did not open test_sp_end:134 (`extended_tick = 6912`) beyond its lines. The comment near :150 says the extended end "came from no such count" and gives no arithmetic, so it may be an unmarked snapshot. I did not check test_audio_*, test_cli:526 or the row-111 uitest places beyond the asserted line. The "outside the rows" list is a grep result I did not open.

## Addendum after the user's three rulings (X10 and X12)

Rulings in `docs/agents/rulings/tests-and-runs.md`: "tests should not be pinned to typed constants. they should be pinned to the behavior of the code.", "tests exist to verify the behavior of the code. a test citing something that isn't code is always wrong", and "tests should test their intended behavior directly, not make up an arbitrary meaningless string". Two consequences for the file above.

First, the two soft disputes under "Verdict on the reader's X9 instances" (12.6 in test_activation_rows, -12.0 in test_library_query:353) are withdrawn as disputes. A made-up engine record fed to code that handles engine records is X9 even when the expected text follows from the input. Treat both as X9, with their neighbours 12.5, 20.0 and 2.5, and the hand-built records in test_sp_meter_curve (already added above). The DMBot payload (test_dm_report:118-130) stays unclear: it stands in for outside data, and I did not check it against the real API.

Second, every instance I called a snapshot is also X12, because its stated source is a run, a saved file or "the engine's record". That covers the reader's four rows, my widened extents and my added places: test_replay:1513-1590; test_pb_coach, test_pb_text and pbtest_coach (130.4); test_preview_boxes:294 and :515-560; the pbtest files (140 and the figures beside it); test_fill_deadline:116-138; test_dm_report:86-90; test_sp_meter_curve:424-432 and :465,466. Add the places whose comment cites a person's chart rather than code: test_library_query:340 ("Burnout's real 163.0 ms"), test_activation_rows:745-800 (-163.0) and test_library_model:589 (`ready(378315, "3- 1 2", 7, 163.0)`).

X10 (an expected value that is a typed product constant or text) in the rows I opened. These come from what I read. I did not scan every place of every row for X10.
- Row 87 (170): test_report_rows:468 (`"'Beyond' means past the 170 ms window."`) and test_report_view:423,447 (`"Beyond 170 ms"`). The 170 px in test_overlay_layout:344,472,505 is a typed layout fixture (`ten_per_char`), not a product constant.
- Row 98 (12345): test_batch_text:22,32,33 type the product sentences "12,345 charts" and "12,345 of these can't be analyzed. Check the box to try them again." The behaviour there is the thousands grouping and the singular or plural noun. The sentence is product text typed back.
- Rows 72, 83, 86, 97, 110 (badge texts): test_activation_rows:745-837,877-921 type "early fill -10.0 ms", "squeeze in 12.6 ms", "squeeze out 5.0 ms" and so on as expected product text.
- Row 100 (130.4): test_pb_coach:163,174 and test_pb_text:159,177 type "needs 130.4 ms, your limit is 10 ms" and "squeeze out 130.4 ms" (also X12).
- Rows 71 and 94: test_stars:215 types the multiplier table `{0.1, 0.5, 1.0, 2.0, 2.8, 3.6, 4.4}` against `kStarMultipliers`. test_model:37-45 types the note score table (50, 65, 100, 130) against `basescore()`. test_stars:341 types "the game stops at 7". The products at test_stars:222-223 and :259 are stated arithmetic from that table, so I leave them sound.
- Row 70 (5000): test_path_list:303 types "Within 5,000 points" (product text).
- test_config:407-410 types the hit window 85 (`early_fill_window_ms == 85`) where lines 372-405 use `kSqueezeWindowMs` by name. The by-name checks are not a constant pin.

Echo (X11): test_store_results:641,645 type -315.8 in and read it back through the store. It checks that the store keeps the sign, but the value is typed, not made by the engine. I found no other echo in my rows.
