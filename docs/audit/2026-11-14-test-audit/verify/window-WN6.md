# verify window-WN6 (verifier agent a44a4394b2b75751e, T5-V-WN6)

Checked: `records/window-WN6.md` at branch HEAD (code of `5ec682c7`). I ran nothing. Every claim is "by reading" unless a run is named. The one run I read is `records-engine/believer.dump.json` and `believer.schedule.json` (at 85 ms), for record 6. No record anywhere is at 60 ms, so no claim is "proven by" a 60 ms run.

## 1. Scope

I listed every test case of the six files by `void test_` and by registration, then compared with the auditor's table.

- uitest_batch_reports.cpp: 15 `test_` functions, 15 registrations (lines 727-741). Every one has a row.
- pbtest_build.cpp 3, pbtest_coach.cpp 11, pbtest_files.cpp 1, pbtest_window.cpp 9. Every one has a row. The coach and window B/C counts (B 3 / C 8 and B 4 / C 5) match.
- uitest_tests.cpp holds only `register_tests`, no test body. `register_script_test` (`uitest_script.cpp:57`, name "script") has its row.
- Test cases that reach the search and have no row: none.
- The window facts hold: `scratch_settings()` sets no window, `Settings::early_fill_window_ms` defaults to `kDefaultEarlyFillWindowMs`. No A row exists, and none should. A grep for `efill` and `early_fill` in `tests/pb_ui` finds nothing, so no test of this group sets the window.

## 2. B rows

Rows checked: 25 B (15 batch, 1 script, 3 build/coach "follow-the-playhead", 3 coach, 1 files, 4 window, minus the overlap the auditor's own count lists: B 25 in total). Disputed: 0. Notes:

- settings-and-reports (flag): settles. The status "under" at `dm_report.cpp:131-137` needs the canned 100,000 below the optimal. The optimal is at least the score with no Star Power, and Star Power only adds to it. Acid Romance has 962 played notes, so the no-SP score is about 190,000 at a 4x multiplier. The window cannot bring the optimal under 100,000. B holds. The reason is arithmetic, not a quoted line, but it needs no run: the optimal cannot be below the no-SP score.
- batch-chip-count (flag): settles. `analysis.cpp:855-856` counts a row as analyzed unless `wr.failed`. A failure is the analyzer throwing, not a different path. The window does not move it. B holds.
- batch-done-strip, report-buttons, batch-open-failure: read whole. They assert report build state, window flags, strip words and `skipped == 0`. No path or score. B holds.
- drag-moves-the-clock (coach 405) and activations-row-never-drags (463): they use `fill_ref(kSecondFillTick)` and `fill_ref(kFirstFillTick)` as hit boxes. `preview_timeline.cpp:517-525` makes an item for every fill in view whatever its state, so the item exists at 60. Typed clocks "0:32.428", "1:04.981" come from pointer shares of the track. "0:51.459" is the hit-box centre of a fill, which the chart places. B holds. One caution: the hit box's sides are "set below, once every fill in view is placed" (line 535 comment), so the centre depends on neighbouring fills in view, which are chart data and not the window. B holds.
- paste-refused-keeps-path: pastes the record's own `best_path()` text and compares with the same record. Only the typed refusal strings are literal, and they do not read a path. B holds.
- cant-activate-reason (coach 119): reaches `fill_reason`'s UnderBars branch (`pb_text.cpp:69-73`) for a fill the window cannot unhide. `fill_refuses` is `e_offset < -window`, so a narrower window refuses more, never fewer. B holds. I did not trace how the builder sets fill states when the player's path is empty (`preview_view.cpp:466-468` says that with a null path every fill is Offered, which would contradict this test at any window; the test passes at 85, so the builder must pass an empty path object and not null). That is not a dispute, but the auditor's section on offered states rests on code I could not fully follow either.
- follow-the-playhead, head-row-wraps, drag-stays-in-the-window, no-star-power, file-open-error, script, batch-strip-drift and the other gate-held batch tests: agreed as B.

## 3. C records (15)

I agree with all 15 verdicts: Rewrite 11, Investigate 4 (claim not checked only). Notes:

- Q8 lines checked against the code, all land on the stated check: build 227 (`two_step_score != one_step_score`), build 381, coach 103, 157, 187, 277-278, 366, 491, 555, window 84, 395, 495, 584, 603. By reading.
- Typed figures: I grepped every digit-with-comma and every quoted figure in the four pbtest files. Every line is in the auditor's list or is a number the auditor names as unmovable. I found no figure the auditor missed. Lines 253-256, 262, 271-272 and 292 of the coach file are covered by the auditor's range "240-244, 252-255" only partly: the end-lines at 253, 254, 262, 271, 272 and 292 also type m76/m78 and "−2,680". They move with record 6 and are in its verdict.
- Record 6: the engine record agrees at 85. `believer.dump.json:107` holds "1 0+ 0 0". `believer.schedule.json` holds act_tick 26880 and 52992, deact 59136 and nominal end 57600. I did not find 74496 in the lines I read (the grep stopped at the second activation); the file's third activation was not checked. By the dump, "proven by" at 85 only.
- Record 15: lines 603, 604, 613 type 381,075 and 378,315. They are a pair that the 60 ms run can move, and "378,315" also appears in records 1 and 9. Agreed.
- Record 7 and 12 explain they are C only because a list row of a typed fill must exist. That holds: `choice_ref(kLateFillTick)` and `choice_ref(kFirstFillTick)` need the decision list to offer that fill.
- Records 13 and 14: C only through `click_fill(kFirstFillTick)` at `pbtest_window.cpp:487`, a setup click. Agreed. Investigate fits, since the typed tick is the only thing the window moves; this is the one place a test is red with no bug in what it claims to test.

## 4. Splits

- The same Burnout figures ("378,315", "378,175", m32, m20 to m24 and "−140") are typed in four places: `pbtest_build.cpp`, `pbtest_coach.cpp` (fill-card), `pbtest_window.cpp` (rules-reach, open-starts-empty) and `tests/test_pb_text.cpp`. One 60 ms change moves them in step. Not a disagreement today, but four copies of one answer.
- `kFirstFillTick` and `kSecondFillTick` are typed once in `pbtest_harness.h:60-61`. `kEarliestTick = 36480` (`pbtest_window.cpp:178`) types the same first-fill tick a second time, and `kUnderTwoTick = 21120` is typed twice (`pbtest_window.cpp:161`, `pbtest_coach.cpp:126`). Their comments each say "from one run".

## Not reached

I did not trace how `PbApp::shown_path` and the builder pass an empty path to `apply_preview_overlay`. I did not read `pb_coach.cpp` beyond what the grep showed. I did not read `believer.schedule.json` past the second activation. Nothing else is unread.
