# verify window-WN5 (verifier agent ae9d344eafeb0d106, T5-V-WN5)

Checked: `records/window-WN5.md`, at branch HEAD (code of `5ec682c7`). I ran nothing. Every claim below is "by reading" unless it names a run. No run in `records-engine/` or `records-screen/` covers Burnout, Tapped Out, Tapestry or any chart at 60 ms, so no claim here is "proven by" a run.

## 1. Scope

I listed every `void test_*` in the six test files and the registrations in the others, and compared the names with the auditor's tables.

- uitest_details.cpp 26, uitest_preview.cpp 16, uitest_paths.cpp 12, uitest_library.cpp 31, uitest_live.cpp 16, uitest_report_windows.cpp 24. Every name has a row and no row is for a missing test. The counts match the auditor's final counts (the auditor's own fix for library, 31 and 29, is right).
- Test case with no row: one. `register_script_test` (`tests/ui/uitest_script.cpp:57`) registers a test named "script". The auditor files uitest_script.cpp under "helper". It is a registered test case that runs whatever a command file opens. Its window is the app's, 60, unless the script's ini says otherwise. It needs a row.
- Files in the brief's WN5 list with no test case (harness, engine, main, commands, pbtest_harness, pbtest_main): agreed, they register none. `uitest_harness.cpp:237-249` lets a `--ini` seed or a test's own bytes replace `scratch_settings()`. So "the default is 60" holds only when neither is used. `uitest_details.cpp:738` writes its own ini with no window line, so it still gets the default 60 (`src/app/config.h:195`).
- Files that run the search through this harness and sit in no window group (coverage.md lines 83, 96-103): `tests/ui/uitest_batch_reports.cpp`, `tests/ui/uitest_tests.cpp`, `tests/pb_ui/pbtest_build.cpp`, `pbtest_coach.cpp`, `pbtest_files.cpp`, `pbtest_window.cpp`. The brief's table gives them to no WN group. `pbtest_build.cpp:163,178,202-215` and `pbtest_coach.cpp:364-385` use `kFirstFillTick` and `kSecondFillTick`, so the auditor's typed-number finding reaches test cases nobody reads in this round. I did not reach them either. This is a gap for the main session.

Window facts checked in code: `Settings::early_fill_window_ms = kDefaultEarlyFillWindowMs` (`src/app/config.h:195`). `config.cpp:411` copies it into the analysis settings. `scratch_settings()` (`tests/scratch_settings.h:20-25`) sets only the chart folder, `auto_open_report` and `depth_value`. `pather.cpp:265` passes it to the graph. `engine.cpp:2030` stamps `act.e_window_ms`. `model.h:331-336` and `model.cpp:473-475` read as quoted. `path_view.cpp:280` prints "(required)" or "(optional)" from `act.is_E0()`. All agree with the auditor. No A row exists, and I found none (grep for `efill` and `early_fill` in tests/ui and tests/pb_ui hits only uitest_library 316-383, the delta rows).

## 2. B rows (reasons checked)

Rows checked: 24 + 11 + 1 + 29 + 15 + 24 = 104 B rows. Disputed: 0. Agreed, with these notes.

- test_stars (uitest_details.cpp:467): the auditor left the typed `"4.4"` (line 504) unread. Resolved: `kStarMultipliers` ends in `4.4f` (`src/core/stars.h`) and the Stars tab prints it as the 7-star factor (`src/ui/stars_tab.cpp`, `"%.1f"`). It is a constant, not a search output. `chart_base_score()` is `notes_points + score_ghosts + score_accents` (`model.cpp:859`), and the expected side is read from the same record. B holds.
- test_dynamics, test_dynamics_reopen, test_dynamics_hard: `Ghosts: 5`, `37 of 455` and `of 3,426` come from `DynamicsBreakdown` (`dynamics_tab.cpp:191,263`, `played_total(bass2x)`), built by `dynamics_for_settings(path, reading, song)` (`library_jobs.cpp:731`). It takes no `SearchSettings`. B holds.
- test_preview_solo_box: the auditor's reason rests on a comment (`uitest_preview.cpp:426`). I checked the chart: `testdata/input/common/IB24/T6/Levitating (feat. DaBaby) (Dua Lipa) (L)/notes.chart` has no `S 2` line (count 0), so no phrase and no activation. B holds.
- test_preview_controls (lines 260-376, unread by the auditor): the typed values are timeline positions (15000, 10000, 12010 ms); the score box reads `shown->totalscore()` and `path_stars(*shown)` from the record. B holds.
- test_paths_long_path (540): asserts only widths against `kMinPathListW`, `kMaxPathListShare`, `kMinPathDetailsW` and content fit (lines 561-566). The expected side is a width rule, not a typed value. B holds.
- test_layout_sweep (879): opens Burnout, The Spiraling Void, Tapestry and Themata and asserts no window overflows. The text drawn depends on the paths, but the assertion is "nothing overflows", not a value. B holds. I note it because a changed path set changes what gets laid out.
- uitest_library (the 29): the unread regions (541-704, 805-1012) assert counts of seeded rows (98 charts, 5 matches), titles and widths on an unanalyzed library. The analysis-reaching ones (test_difficulty, test_analyze_follows_chip, test_library_widths_hold, test_view_settings) assert statuses, chip counts, and `best` read from the record. Whether a record exists does not depend on the window. B holds.
- uitest_live (the 15): the unread regions 1-375 and 432-606 assert reply text, `selected->title`, refusal sentences and modal names. No chart value. B holds.
- uitest_report_windows (the 24): I read the bodies at 1051-1105, 1171-1256 and 1345-1620. The "n of 6 paths" and "n of 4 scores" come from the sample report fixture (`open_path_all_rows` comment: "none of the sample rows is an optimal path"), not the search. test_open_analyzes (1112) asserts `songs + cant_analyze_charts == library_charts` and `no_result_charts == 0`, which count records, not paths. B holds.

## 3. C records (17)

I agree with all 17 verdicts. The old verdicts (A39, A44, A45) are quoted correctly in every record. Notes per record, all by reading:

1. test_panel_headline: agree, Rewrite. Lines 1032, 1057, 1066-1071 checked.
2. test_preview_path_overlay: agree, Investigate, claim not checked only. Line 171 `other != nullptr` checked. Which chart is "chart 0" stays open.
3. test_preview_drain_box: agree, Rewrite. Line cite is wrong. The record puts the check at 516 and the loop at 508-514. The code has `IM_CHECK(active_ms >= 0.0)` at `uitest_preview.cpp:505`. Question 8's named bug belongs at 505. The auditor also says it did not read the middle of the body.
4. test_preview_path_picker: agree, Rewrite. Lines 679, 754, 763, 768, 781-783 checked.
5. test_preview_overlay_steady: agree, Rewrite. Helper lines 679 and 684 checked. Read by assertion grep only, as the auditor says.
6. test_preview_activation_jumps: agree, Rewrite. `m99.4.352` is at 1073, `Next: activation` at 1038 and 1051.
7. test_paths_list: agree, Rewrite. Lines 119-128 checked.
8. test_paths_rows: agree, Rewrite, with the Beg half as Investigate. `fill_refuses` is `e_offset < -window_ms`, so an early fill of 0.0 ms is not refused at 60 or 85. Agreed. Gap: `records-engine/beg.dump.json` exists at 85 and was not used on Beg's row 4. The auditor says only "likely stable".
9. test_paths_backend_timings: agree, Rewrite. `kBurnoutSqOutRating` at line 344 checked.
10. test_paths_folds_copy: agree, Rewrite.
11. test_paths_uncounted: agree, Rewrite. The comment at lines 485-487 ("found by searching the corpus under the GUI tests' settings") and line 479 checked. This is the auditor's sharpest finding: Tapestry was picked by a search at the old window.
12. test_paths_fit_narrow: agree, Investigate, claim not checked only. Precondition at line 102 checked.
13. test_paths_backend_fit: agree, Rewrite.
14. test_paths_row_layout: agree, Rewrite.
15. test_paths_length_from_click: agree, Rewrite. `130087.87999999993` at lines 925 and 935 is parse output (D255), so the window cannot move it.
16. test_paths_hit_window: agree, Rewrite. The words "(required)" and "(optional)" come from `is_E0()` at `path_view.cpp:280`, confirmed. "Optional" also needs skips above 0 (`is_e0(e_offset, skips, window)`). No run shows activation 4's skips.
17. test_paths_hit_window_narrow: agree, Rewrite. `kTappedOutWindowPieces` at 941-942 checked.

Keeps: the auditor gives none. I checked the old Keeps among B rows for a claim that rested on an 85 ms engine record. A45's Keeps are test_preview_buttons_keys, test_scrub_hold and test_layout_drift (timeline numbers, no engine record). A47's Keep is test_hidden_lets_go (sample rows). A39's screen-record cite (`records-screen/believer.txt`) is for a test that reads the record. None moves to C.

Question 8 named bugs, lines checked: record 1 at 1057 and 1066-1067, record 2 at 171, record 3 at 505 (not 516), record 5 at 679 and 684. The others I did not find wrong.

## 4. Typed numbers

The auditor's list holds: `uitest_paths.cpp` 102, 119-128, 264-293, 338-344, 393-404, 941-1010; `uitest_details.cpp` 1032, 1057, 1066; `uitest_preview.cpp` 679, 700, 754, 763, 781-783; `pbtest_harness.h:60-61` (`kFirstFillTick = 36480`, `kSecondFillTick = 74880`, comment "from one run under the GUI tests' settings"). I found nothing new in the six files. The `"4.4"` entry is resolved above: a constant, not window-dependent.

## 5. Splits

- Three tests and two helpers each pin Burnout's `3- 1 2` and `378,315` (auditor's split (a)). Confirmed at `uitest_paths.cpp:102,121-122`, `uitest_preview.cpp:679`, `uitest_details.cpp:1057,1060`.
- `scratch_settings.h` says the settings are the ones "the Burnout reference results in the unit tests were made with", while the default window is 60 and `tests/song_digest.h` pins 85 (auditor's split (b)). I confirmed the `scratch_settings.h` text and did not open `song_digest.h`.
- New: Tapped Out's activation 4 text is typed in `uitest_paths.cpp:941-942` and again in `tests/test_activation_rows.cpp` (named in the test's comment at line 940).
- New: `kFirstFillTick` and `kSecondFillTick` are read by five files in `tests/pb_ui`, and only the harness that defines them is in a window group.

## 6. Not reached

- `uitest_preview.cpp` 534-660 and 849-921: assertion grep only.
- `uitest_library.cpp` 805-1012 and 1125-1800: assertion grep, plus the analysis-reaching tests read.
- `uitest_paths.cpp` 604-733 and 734-900: assertion grep, not a line-by-line read.
- `tests/test_activation_rows.cpp` and `tests/song_digest.h` (WN4's files).
