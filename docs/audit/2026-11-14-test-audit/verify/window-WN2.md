Verifier agent id: aeaadbbb986ecc66e (task T5-V-WN2). Part checked: records/window-WN2.md, at worktree HEAD (code of `5ec682c7`). Nothing was built or run.

## Scope check (my own search)

I counted `TEST_CASE` in each file: test_sp_end 24, test_sp_end_clamp 7, test_sp_meter_curve 27, test_best_finish 4, test_stars 17, test_track_state 23, test_fast_tempo 8, test_squeeze_rating 29. These match the auditor's tables. I grepped the eight files and `fast_tempo_charts.h` for `run_search`, `analyz`, `search_target`, `search_allzero`, `ScoreGraph`, `Engine`, `best_finish`, `search_graph`, `corpus::`, `scores_settings`, `wide_search`, `early_fill`, `efill`, `window_ms`. No file sets a window, so there is no A row. Every search call lands in a row the auditor lists. No test case that reaches the search is missing from the tables. The N rows in test_stars, test_track_state and test_squeeze_rating have no search call by my grep either. The N rows in test_sp_meter_curve build hand-made `Activation`s (for example the test at 625-698) and call no search.

The default-window chain holds: `ScoreGraph` ctor default `kDefaultEarlyFillWindowMs` (`graph.h:198-202`), `SearchSettings::early_fill_window_ms` default (`pather.h:45`), `scores_settings` built on `Settings()` (`record_fixtures.h:582-589`), `scratch_settings()` (`scratch_settings.h`: only chart folder, depth 2), engine reads `graph.early_fill_window_ms()` (`engine.cpp:2030`, `:2176`).

## B rows, checked against the grid

I redid the e_offset arithmetic (deadline = fill end - fill length - 4 beats; ready = the phrase tick that completes the bank) for each hand-built song. Band 1 is -85 < e < -60 ms, band 2 is 60 <= e < 85 ms.

- `build_tail_song` songs (120 BPM, 192 tick/beat): fill 2304, deadline 1152, ready 768, so 384 ticks = 1000 ms. Holds. The 98 BPM case at test_sp_end.cpp:597-605 gives 1224 ms. Holds.
- `beat_song` (240 BPM): fill length 960, so every deadline and phrase is a multiple of 480 ticks (250 ms). Holds. I took the second fill of `make_early_sqout_song` at the auditor's 0 ms (deadline 14400, phrase 14400).
- `ready_time_fold_song`: deadlines 3000, 16000, 17000 ms against ready 2000 and 18000 ms. Holds.
- `banked_phrase_window_song` (4000 BPM, 0.078125 ms a tick): deadline 1152 against phrases 768 and 1536 gives +30 and -30 ms. Holds.
- `banked_phrase_fold_song`: +30, -210 ms and 3000 ms pairs. Holds.
- test_sp_end_clamp, every deadline-minus-phrase tick pair: kOneActivation 384 ticks (50 ms); kPlainTiesClamp 0 and 192 ticks; kTiedClamps (deadlines 1152, 13056, 13440 against phrases 0, 768, 9600, 10368, 14976) none between 461 and 652 ticks; kFoldSharedClamp (1200 BPM, deadlines 1152, 8064, 9600 against phrases 0, 768, 6144, 6912, 9984, 11520, 12864) none between 230 and 326 ticks. All hold.
- `kTwoActivations`: fill 2496, deadline 1344, ready 768, 576 ticks x 25/192 = 75.0 ms. Confirmed, inside [60, 85).
- test_sp_end_clamp.cpp:97 and the graph-only cases (test_sp_end.cpp:179, 212, 265, 837; test_fast_tempo.cpp:107): no search call in them. The graph stores the window and never reads it. Hold.
- test_track_state.cpp:991 and :1026: both compare two builds from the same stored paths. `swaps > charts` needs at least one stored path on one sampled chart, which every analysis returns. Holds by reading.
- test_best_finish.cpp:117: the pin is one tick past an activation, so the first failure is "activation", not a window refusal. Holds by reading. It needs `rec.best_path()` to have an activation at 60 ms, which I cannot prove without a dump.

B disputes: none. A rows: none exist.

## C rows

All 22 are real corpus or fast-tempo-folder runs (or, record 5, the 75 ms fixture). The old verdicts the auditor quotes match the old record files. I opened each: A28 #11, #17, #21, #23; C11; A29 #23, #25, #26, #27; A8 #14, #15, #16; C12 both; C2 all seven; A30 #4.

Nothing is proven by a named run at 60 ms. No dump in `records-engine/` is at 60 (the dumps are from 326cd4e0, at 85). Every claim is "by reading". Every verdict is Investigate (claim not checked only, at 60) or the old Rewrite/Remove. I agree with all 22 verdicts. Corrections and gaps:

1. Record 8 (test_sp_meter_curve.cpp:883): the auditor says there is no typed value beyond a skip counter. Wrong. Lines 930-931 are `CHECK(lit > 0)` and `CHECK(unlit > 0)`, floors over corpus paths that have an early squeeze-out. They can move with the window. The auditor read lines 883-940 only by grep. Verdict C stands, with those two floors named.
2. Record 14 (test_stars.cpp): the typed `378315` is at line 362, not 359. Line 359 is the `corpus::analyzed` call.
3. Record 20 (test_fast_tempo.cpp:329): `variants > 50` is at line 355 (the auditor says about 360). The record omits `CHECK(analyzed == 48)` at line 354. That count rests on every seed analyzing without a throw, so it can move.
4. Record 12 (test_best_finish.cpp:93): line 103 is `REQUIRE(best.all_activations().front().within_ms_limit(*cfg.ms_filter))`. The auditor says it is "not typed". It is a result of the best path at the chosen window: if the 60 ms best path's first window is over the 10 ms limit, the test stops before checking anything. Treat it as a number that can move, with line 111.
5. Records 6 to 22 are cut-down records. They lack Owner, Direct, Reason, Deleted and the other lines of the rubric shape the brief asks for. Records 1 to 5 carry more of them. This is a form fault.
6. Record 9 (test_sp_meter_curve.cpp:941): typed values confirmed at lines 948 and 970 (`"3- 1 2"`, `"3+ 2+ 1"`), 951-956 (59520, 71040, 70800, 48,783.075, 48,620.0315) and 961. The comment at 950-952 says they come from "PS0b" on 2026-11-14, before the 60 ms default (2026-11-15 per `model.h:107-110`).
7. Records 15 to 21: the fixture ticks exist at the lines named. The e_offsets of the fast-tempo fixtures were not computed by the auditor, and I did not compute them either. `found >= 1` (record 16) and `at_15648 > 0` (record 18) fail loudly if the path is gone. The silent skips are the `continue` when `window_at` is null (records 18 and 19). The auditor's claim holds.
8. Record 5: the auditor's new fact (75 ms, inside [60, 85)) is right. The test asserts squeeze-out tick, end and row offset, none of which reads the E mark, and the activation is pinned through `search_target`; `fill_refuses(75, 60)` is false. By reading the paths come out the same, so it could be B, but the old question 11 (reachability on a real chart) keeps it C. I do not dispute it.

## Typed numbers (additions)

- test_best_finish.cpp:111 `451830` (as the auditor). The same number is typed in `tests/test_path_costs.cpp:73, 84, 90, 96, 240`, which is WN3's group. I did not judge those. WN3's verifier should see them.
- test_best_finish.cpp:103 (item 4).
- test_fast_tempo.cpp:354 `analyzed == 48` (item 3).
- test_sp_meter_curve.cpp:930-931 `lit > 0`, `unlit > 0` (item 1).
- Everything else in the auditor's list is confirmed at the line named, with the line-number corrections above.

## Splits

None found. The auditor's note about `kTwoActivations` (one E mark that differs between 60 and 85) is one rule read at two values, not two answers. I agree.

## Not reached

- The e_offsets of the real-chart corpus and of the test_fast_tempo fixture charts (not computed, no run allowed).
- `tests/preview_song_fixtures.h` helpers other than `make_overfill_song` (I checked its fill at 2304 only). `make_late_sqin_song` and `make_early_sqout_song` live in `record_fixtures.h`, and I checked them.
- The 28 N rows of test_squeeze_rating and the 21 N rows of test_track_state were sorted by repeating the grep, not read body by body.
- I did not re-read the filed window rulings (`code-and-engine.md` lines 55-57); no verdict here rests on them.
