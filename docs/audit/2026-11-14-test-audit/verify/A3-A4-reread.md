# Re-read of A3 and A4 records the round 1 verifier did not open

Verifier agent id: acf9297624f305afb (T5-V-R3a, session 16892967-2cf8-4437-a7cd-e3dad250f621). Read-only. Nothing built or run, so every mark is "by reading". I ran out of time at the preamble's cap. Reached: A3 records 7, 9, 10, 11. Not reached: A3 records 19 and 21, and every A4 record on the list (12, 13, 15, 17, 18, 19, 20, 21, 22, 34, 35, 38, 39, 40). A fresh verifier takes those.

Read for the four: `tests/test_activation_rows.cpp` 940-1259, `src/app/path_view.cpp` 95-148 and 281-335, `records-screen/` lines named below, `records-engine/late_sqin_twice.schedule.json` lines 5-12, `records/A3.md` 118-200.

## A3 record 7: squeeze sentences (test line 945)

By reading, every sentence the test types matches `squeeze_sentences` (`path_view.cpp:95-148`): the "no more than" versus "more than" choice follows `sq.needs_no_timing()` (line ~108), the side word follows `squeeze_presses(sq).squeezed`, and a squeeze-out names the squeezed chord by `chord.notationstr()` when a row exists. The line 1017 cost (460 - 260 = 200) and the uncounted case (1006) are the display's reading of the backend row; I did not open `core::sqout_cost`.

Dispute with the Q3 answer, small: the record says the screen at `pitch_dark.txt:41` has "the same words" as the claim. The screen line reads "Hit the SP phrase's last note more than 94.3 ms early (eff. 93.7 ms) so it lands before Star Power ends. The phrase then counts ...". It carries an "(eff. 93.7 ms)" that the test's first sentence (962-965, no transfer scale set) does not have, so it agrees with the scaled case at 982-983, not with the sentence the auditor paired it to. The pairing should name line 982. The other two screen pairings hold: `believer_path4.txt:58` reads "Hit the [K   G] note more than 0.0 ms late so it lands after Star Power ends. It scores 200 fewer points, and its SP phrase banks for later." Agree with Rewrite.

## A3 record 9: backend table eff. figure (test line 1080)

By reading, lines 1090-1093 check a row at -400.0 with TransferScale 1.6/1.0 and the early-scale line. Dispute with Q3: the record pairs it with `late_sqin_twice.schedule.json:9` (`end_ratio` early 0.06) and screen rows such as "-495.0 ... Normal (eff. 934.0 ms)" (`late_sqin_twice.txt:61`). 495 / 0.06 is not 934, so the engine line cited does not produce the screen figure by simple division; the figure must use another ratio the auditor did not name. "Agree on shape" is the most this supports; "Engine:" and "Claim:" are not a checked pair. Q3 should read "claim not checked" until the ratio behind 934.0 is named. Verdict Rewrite stays right.

## A3 record 10: near-1 multiplier (test line 1096)

By reading, the scale-line cases at 1113, 1137, 1146, 1153 and 1161-1164 are real scenarios, one test case with seven scenarios. The record's Q3 pairs `pitch_dark.txt:45` ("x1.02 (early) / x1.02 (late) at the SP end; x1.01 (late) at the SqIn's SP end") with Pitch Dark activation 0 `end_ratio` 1.01899. By reading the screen line also names a second figure at the SqIn's SP end ("x1.01 (late)") that the record does not pair with an engine value; partly checked, as the auditor said. Agree Rewrite.

## A3 record 11: unknown scale (test line 1167)

By reading the test matches the record. I searched `records-screen/*.txt` for "Transfer scale": no hit in any capture, as the record found. Agree with Investigate: the state exists only if a stored record lacks a scale.

## Not reached

A3 records 19 and 21. A4 records 12, 13, 15, 17, 18, 19, 20, 21, 22, 34, 35, 38, 39, 40.
