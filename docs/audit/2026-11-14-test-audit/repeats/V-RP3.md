agent id: a3b5ab00d3d8dbe35 (T5-V-RP3, verifies repeats/RP3.md, rows 139 to 207)

Method: I read cross-findings.md (X1, X2) first. Then I opened the owner line of every K2 candidate in `src/`, and the test lines of the K1 candidates and the rows the sorter left open. I also sampled 24 K3 and K4 rows across the range. Paths below are under `tests/` unless they start with `src/`.

## Table (what I checked)

| Row | Value | Sorter's kind | My verdict | What I opened |
|---|---|---|---|---|
| 140 | 25.123 | K1 | agree (X2 family) | pb_ui/pbtest_build.cpp:165,198,199; pbtest_coach.cpp:304-309,365,369,524; pbtest_window.cpp:76,204. Comments say "from one run". |
| 142 | "charts" | K3/K2 | disputed, all four are K3 | test_batch_text.cpp:19-22 and test_report_rows.cpp:828,842 and uitest_report_windows.cpp:1143 all pass "chart","charts" as arguments to the real `counted()`. That is an input, not a copied output. The expected "N charts" text is built by calling `counted()` too (src/ui/app_state.cpp:686-688 does the same). |
| 146, 189 | placeholders | K2 | agree | src/parse/song.h:142-143 `kUnknownArtist`, `kUnknownCharter` |
| 149 | "5:59.957" tail | K3 / K1 | split agreed; better K4 | pbtest_window.cpp:427,434 is the Symphony X song length, a chart-file fact (D75, song length = chart metadata), shown through `clock_str`. |
| 154 | "SP drain (if activated)" | K2 | agree | src/app/preview_view.cpp:680 `box.header = ...` |
| 159 | "Analysis settings: defaults" | K2 | agree | src/ui/library_dialogs.cpp:130-132 `settings_button_label`; library_parts.h:83 gives "defaults" |
| 160 | "Stop the batch..." | K2 | agree | src/ui/library_parts.h:97 `kSettingsLockedText` |
| 162 | 71040 | Mixed | resolved, see below | test_sp_meter_curve.cpp:950-957; test_pb_text.cpp:112-126; test_model.cpp:1286 |
| 163, 180 | "Insane", "Insane+" | K2 | agree | src/core/squeeze_rating.cpp:247-248 |
| 164 | 12960 | K3 | partly K1 | test_sp_meter_curve.cpp:450-464 (see below) |
| 165 | 163.5 | Split | agree | uitest_paths.cpp:344 `kBurnoutSqOutRating` is X1's "Insane+ ... 163.5 ms" family |
| 166 | kDot | K2 | owner corrected | src/app/display_format.h:13 is a comment only; there is no constant. The product spells " · " literally in many places. tests/song_details_fixtures.h:36 `kDot` is the one test constant. uitest_paths.cpp:256 types its own `dot` and test_overlay_layout.cpp:324,331 type "\xC2\xB7". Not a copy of a src constant, because none exists. |
| 167, 168 | "m32.1.0", "m58.1.0" | K1 | agree; owner corrected | built by `format_measure`, src/core/timing.cpp:358-362 (line 355 is `measure_label`, the "m32" part only). Burnout activations 1 and 2. |
| 170 | "/all-users" | K2 | agree | src/net/dmbot_client.cpp:122 |
| 183, 184 | widget labels | K2 | agree, weak | ui ids; tests need them to click |
| 185 | "3 bars" | Split | agree | test_activation_rows.cpp:599 is Burnout activation 1 (`a1.sp_bars == 3` just above it) |
| 188 | no-notes message | K2 | agree | src/parse/song.h:82-86 `no_notes_message` |
| 191 | "10 ms" | K2, owner unfound | owner found | src/app/config.h:165 `int mslimit_value = 10;` shown by src/app/config.cpp:466 `format_ms_whole`. test_config.cpp:1021 and test_batch_text.cpp:55 type the default's text. uitest_batch_reports.cpp:393 only finds it as a substring. |
| 192 | "sqlite exec failed: ..." | K2 | agree | src/store/record_store.cpp:1058, 1089 |
| 195 | "cancelled" | K2 | agree | src/core/error_kind.h:158 |
| 202 | " ENDOFSONG" | K2 | agree | src/parse/section_names.cpp:13 `kMidSectionVeto` is "_ENDOFSONG" (underscore), so the tests type a display form; check which one the product shows before calling it a copy |
| 179 | "Burnout — Green Day" | K2 | agree | src/pathbuilder/pb_state.cpp:43 `chart_title_line` |

K3 and K4 sample (rows opened, kind confirmed unless noted): 141 (test_preview_scene.cpp:300 is `make_ch10_fill_song`, hand-built, K3), 143, 144 (see below), 145, 147, 148, 150, 153 (test_preview_timeline.cpp:125 a pixel input), 156, 161 (clock outputs), 164 (below), 174, 175, 176 (test_generated_fills.cpp:182 hand-built, test_preview_scene.cpp:342 comment on its own ticks), 178, 181, 182 (test_sp_end.cpp:249 hand-built `beat_song`), 186, 198 (test_fill_landing.cpp:104 and test_replay.cpp:1537 hand-built), 201, 205, 207. 

## Candidates

- **RP3-1 (K1, X2 family): agree.** Burnout's first fill and clock text. Added copies: the song-length tail "2:10.088" at pbtest_build.cpp:199, pbtest_coach.cpp:309 and pbtest_window.cpp:204 (the same run; the length is a chart fact, the clock text is `clock_str`). X2 does not name the clock text.
- **RP3-2 (K1, Burnout, X1 family): agree, owner corrected.** Owner of "m32.1.0" is `format_measure`, src/core/timing.cpp:358, not :355. Copies the sorter missed: uitest_paths.cpp:293 ("Activation 1  m32.1.0  squeez..."), uitest_preview.cpp:783 and :1040 ("at m32.1.0"), test_overlay_layout.cpp:324,331,397,398 ("at m58.1.0", "at m32..."). The overlay_layout ones are layout tests that choose Burnout-sized strings, so they are weak copies; the ui tests are real ones. X1 names "m32, m20 to m24" but not m58.1.0 or the ".1.0" form.
- **RP3-3 (K2, "Insane", "Insane+"): agree.** Owner src/core/squeeze_rating.cpp:247-248. A rename turns all copies red at once; test_squeeze_rating and test_report_rows are the owner's own tests.
- **RP3-4 (placeholders): agree.** Owner src/parse/song.h:142-143.
- **RP3-5 (Library settings words): agree.** "10 ms" owner is now found: src/app/config.h:165 default 10, text from config.cpp:466. test_config.cpp:1021 and test_batch_text.cpp:55 are two checks of one fact.
- **RP3-6 (Preview and badge words): agree for "SP drain (if activated)"; kDot disputed.** Its "owner" is a comment (see table row 166), so it is not a copy of product code.
- **RP3-7 (product messages): agree** on all four owners. Verified each owner line above.
- **RP3-8 (" ENDOFSONG"): agree with a caution.** The product's veto string has an underscore. Whether the test values are the product's displayed form was not opened.
- **RP3-9 (widget labels): agree, weak.**
- **RP3-10 ("chart","charts"): disputed.** Not a candidate. The tests pass the noun pair as arguments to `counted()`, the product function, and do not copy its output text.

## Added by me

- **V-RP3-A (K1, a chart's offered fills; X2 family, not X2's chart).** Allister - Overrated's offered fills, typed as `{55680, 71040, 90240, 105600, 120960, 136320, 151680, 167040}` in test_fills_and_skips.cpp:448 (comment 434-437 "the empty path ... lists every fill the meter allows") and test_pb_text.cpp:118 (comment 115 "the engine lists these fills"). The cost tiers on test_pb_text.cpp:121 and order at :126 are typed beside it. Owner: the engine's fill list, `search_target` trailing fills and the early-fill rule (X2's `fill_refuses`, src/core/model.h:331). A rule change turns both red at once. This resolves the sorter's "71040 in test_pb_text.cpp:118": it is Allister's, a different fact from the 71040 below.
- **V-RP3-B (K1, Burnout, X1 family).** 71040 at test_sp_meter_curve.cpp:954,956 is Burnout's first activation's stored end (activation 59520, squeezed-out chord 70800), "as PS0b saved it" (comment 950-951 and the TEST_CASE head at 935-940, which names `3- 1 2` on "Green Day - Burnout"). It is Burnout's engine record typed. The test also types 70800 and 48783.075 ms. test_model.cpp:1286 is K3 (a hand-built `one_step(59520, 71040)`), though it reuses the same numbers.
- **12960 (row 164), moved from K3.** test_sp_meter_curve.cpp:464 is on a hand-built song, but the value is the engine's search answer ("The engine's record: the squeezed-out chord is 12960"). So it is K1 on a hand-built chart; the same value at test_sp_end.cpp:220 reads from `make_early_sqout_song` and is also an engine answer on a hand-built song ("pinned"). The chart is the test's own, so a change to the engine's squeeze choice turns both red, but a change to the shared fixture would too.
- **9984 (row 144), partly moved.** test_path_costs.cpp:209,219,233 use 9984, 22272, 22273, 37632, 49920 on corpus chart "Mutemath - Allies": the comment at 196 says "Allies' best path by activation only". Those ticks are Allies' best-path activations typed in as pins (K1 dependency, input use). 22273 is the test's own "one tick later". test_replay.cpp:739 and test_tied_variants.cpp:522,575 are hand-built (`build_tail_song`, `fold_between_windows_song`): K3. So only test_path_costs.cpp is chart-bound.

## Rows not opened, and settled questions

- Sorter's open question on 71040 (row 162) and 12960 (row 164): settled above.
- "10 ms" default owner: found, src/app/config.h:165.
- Rows I did not open past the sorter's reading: 139, 151, 155, 157, 171, 172, 177, 190, 194, 197, 199, 200, 203, 204, 206 (K3 per the sorter; I did not re-open them). Row 203: the sorter's note about rules.h:51's default I did not check.

## Counts

Candidates agreed: RP3-1, 2 (with a corrected owner and 7 added copies), 3, 4, 5, 7, 8 (caution), 9. Disputed: RP3-10 (not a candidate), and the kDot part of RP3-6. Added: V-RP3-A, V-RP3-B. K3/K4 rows checked: 24; moved: 3 (row 142 to K3 whole, row 164 to K1-on-hand-built, row 144 partly to K1 for test_path_costs.cpp). Row 149's "5:59.957" part is a K4, not a K1 as the sorter said.
