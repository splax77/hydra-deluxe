Agent: abcf346ec5c19a6cd (T5-V-WN3)

# Verifier, window round WN3

Checked: `records/window-WN3.md` (written by agent a2b3770b5af468654), read against the worktree HEAD (code of `5ec682c7`). Nothing built or run. "By reading" means I read the code and fixtures. No claim here is "proven by" a named run, because the only engine records (`records-engine/`) are at 85 ms.

## 1. Scope

I listed every `TEST_CASE` in the six files and compared it with the tables.

- test_engine_sweeps.cpp 16 cases, test_fills_and_skips.cpp 11, test_tied_variants.cpp 13, test_path_costs.cpp 10, test_helpers.cpp 4: every case has a row. The counts per kind match the auditor's (sweeps 11 B, 3 C, 2 delta; fills 2 B, 5 C, 4 delta; tied 10 B, 3 C; costs 6 B, 4 C; helpers 4 B).
- test_model.cpp has 57 test cases, not "100+" as the record says. A grep for ScoreGraph, analy, search, corpus, Engine, load_song and build_ finds one search entry, `ScoreGraph graph(song, 2)` at line 280, inside the case at 275. That case is delta round (`delta-DL4.md` section 6). The other delta-DL4 sections (147, 171, 206, 239, 255, 654) are real headings at those lines. No test_model case is missing.
- Delta rows checked against `delta-DL5.md`: records 1 to 4 (fills 82, 181, 220, 249), 18 (sweeps 581), 19 (sweeps 745) and 25 (corpus_util.h `settings_key`) all exist with those titles.
- Old record numbers checked against headings: A13 records 2, 3, 9; A14 records 28 to 32; A35 records 4, 5, 13; C6 PC1, PC2, PC8, PC9. All match.
- Helpers: `corpus_util.h` and `analyzed_chart.h` set no window. `scores_settings` (record_fixtures.h:582) and `default_cfg()` (corpus_default_cfg.h) both come from `Settings().to_analysis_settings()`, which carries 60 (`config.h:195`). Agree.

Test cases missing from the tables: none.

## 2. The window source (A rows)

No row is kind A. The only `early_fill_window_ms` set in these files is `test_engine_sweeps.cpp:760`, inside delta-round case 745. `ScoreGraph` defaults to 60 (`graph.h:201`), `SearchSettings` to 60 (`pather.h:45`), and `corpus::analyzed` keys its cache on the window (`corpus_util.h:228`). The refusal (`engine.cpp:1007`) and the passed-over push (`engine.cpp:1051`) are as the auditor quotes. Agree.

## 3. B rows (33 checked, 0 disputed)

All by reading. Notes on the reasons:

- 74, 245, 286, 325, 410, 480, 670, 729, 806: the asserted values are relations between same-window runs, parse facts, or pure functions. I read each body. 325 asserts `sp_cap == 4`, `totalscore() > 0` and `analyzed > 0`; a path with no activation always scores above 0. Agree.
- 695 and 772: the 250 ms lattice holds for `past_last_note_song` and `mid_sp_phrase_song` (record_fixtures.h:179 to 205; 5136 and 5280 are plain notes). Agree. Weak point for 772: its loop also runs corpus charts with fewer than 4 phrases, and the auditor's reason covers only the hand-built song. The assertion is a `record_hash` equality between two same-window runs, so the B holds, but the written reason is incomplete.
- 37 and 60: for 37 I re-derived the numbers (2304 ticks is 6000 ms, deadline 3000, ready 2000, so +1000; later fills give +14000, -2000, +15000, -1000). For 60, the fixture comment (record_fixtures.h:97 to 100) gives +68.75 and -131.25, and the test reads only `skipped_fill_ticks == {19200}`. Agree.
- 193 (folded_sqin): I re-derived from the chart (resolution 192, 4000 BPM from tick 9600). The deadline for the fill ending 10752 is 25000 ms and for 13824 it is 25240 ms. The dump at 85 shows `sp_ready_ms` 6000 and 25210. Offsets are +3000, -210 and +30, none in [60, 85) or [-85, -60). B by reading; the 85 dump only evidences the ready times.
- 101, 134, 399, 443, 503: every tick in the note lists (test_tied_variants.cpp:50 to 70, 405 to 407, 435 to 440) is a multiple of 96. B by reading.
- 553: the one off-lattice tick, 9619, gives -49.479 (typed at line 583). It is above -60 so unrefused, and below 60 so E0, at both 60 and 85. B by reading. It would flip for any window under 49.479, so the typed 49.479 is safe only because the allowed default and maximum are both above it.
- 337 and 361 (twin_end_clamped.chart, resolution 480): deadlines about 17000 and 19000 ms, ready 6000 ms or earlier, offsets about +11000 and +13000. B by reading.
- 296: no search. 169, 176, 180, 195, 249: pure functions on typed integers. 205: throws before any search. test_helpers 37, 67, 85, 111: agree (37 compares a cached record with a direct run at the same settings).

## 4. C records (15 checked, 15 agreed, 0 disputed)

Every Keep and typed number was read in the test body. None is "proven by" a run; every question 3 correctly names a 60 ms dump that does not exist yet.

- Record 1 (sweeps 147): agree. The floors at 199 and 200 are real.
- Record 2 (sweeps 228): agree with Remove. The kind is thin: `paths > 1` (line 242) is the only window-reachable line, and nothing I read makes one stored path a realistic outcome. I leave it C as written.
- Record 3 (sweeps 517): agree. Omission: `CHECK(found > 0)` (line 567) is also a count the window can move, and the record does not name it.
- Record 4 (fills 277): agree. `skipped > 100` is at line 305.
- Records 5 to 8 (fills 347, 383, 416, 435): agree. Typed "3", 123838, 244800, 281759 at 362 to 364; floors at 400, 410, 413; 427, 429, 432; eight ticks at 447 and 448, 9 and 36480 at 452 and 453. The trailing list is pushed by the same `branch_activate` push (`engine.cpp:1051`), so the refusal reaches it. Record 7's two runs both use `test::scores_settings(4)`.
- Record 9 (tied 229) and 10 (tied 263): agree. I did not re-open `relabel_case.chart` or the two clamped_sqin charts; their headers are the auditor's read.
- Record 11 (tied 596): agree. Typed 233100, 150720, 82560, 1, 3 at lines 613 to 616.
- Records 12, 14, 15 (costs 110, 216, 231): agree. The all-0 ticks `{9984, 19200, 34560, 49920}` are at test_path_costs.cpp:79 to 80; 407610 at 225; 451830 at 240, 22273 at 244 and 245.
- Record 13 (costs 130): agree. 706180 (line 152) and 5 (154) confirmed. Line 155: the case's own comment says the fourth window "needs a squeeze tighter than the 10 ms limit", so line 155 may not depend on an early fill at all. The auditor's "if that activation has an early fill" is right but unproven either way.

Question 8 names a bug at a line in every record I read.

Typed numbers: the auditor's list matches what I found; I found none extra. 905 is a parse fact and cannot move.

## 5. Splits

- Carried from A14 record 29, not re-opened here: the engine's trailing list (`engine.cpp:2010`) and the Preview's `sp_meter_bars_at` both answer whether the meter holds an activation there. The fills 435 case compares them at line 453.
- None new.

## 6. Not reached

Nothing named in the brief. Not re-opened: `relabel_case.chart`, `clamped_sqin_a.chart`, `clamped_sqin_b.chart` (records 9 and 10 rest on the auditor's header read), and test_model.cpp beyond the grep.

## Counts

Rows checked: A 0, B 33, C 15, delta round 7 (plus helper `settings_key`). Disputed: 0 of each kind. Notes without dispute: sweeps 772 reason covers only the fixture; record 3 omits `found > 0`; record 13 line 155 may not touch an early fill; test_model has 57 cases, not 100+.
