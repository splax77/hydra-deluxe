Slice WN3, record file docs/audit/2026-11-14-test-audit/records/window-WN3.md (records 1 to 15), read in the checkout worktrees/test-audit-t8-questions-handoff.
Files changed since a19b9cb2: tests/test_engine_sweeps.cpp (e300f2fd, 39e27526) and tests/test_fills_and_skips.cpp (e561ba5c, b1b0ed0f, 43f676cc, 89b03f1c). test_tied_variants.cpp and test_path_costs.cpp are unchanged. src/core/model.h and src/search/engine.cpp also changed (abb6b45d, eef6b1d6 and others). All line numbers below are current and match the record's. The WN3 plant (engine.cpp:2032) is not in this checkout. Nothing was run; every verdict is from reading.

Key fact for record 1 (model.h:369-372, model.cpp:532-535, engine.cpp:1214-1225, 2030-2032): fill_within_limit is true when the fill is not E0, or when -e_offset <= limit. The window only picks E0 for e_offset below it. A positive e_offset is within the limit either way (its difficulty is negative). A negative e_offset is E0 at 60 and at 85, with the same difficulty. Only an e_offset below -60 differs (refused at 60, E0 at 85), and an engine run at 60 never makes one. So setting the model walk's window to 85 instead of 60 gives the same answer on every input. Only a window of 0 or less could differ.

| Record | Test line (current) | Line question 8 names | Verdict | Input or run behind it |
|---|---|---|---|---|
| 1 limit walk | test_engine_sweeps.cpp:174 | engine keeps a path the model's walk rejects; "window-read code" | does not hold | A wrong e_window_ms on the activation (85 for 60) never changes within_ms_limit, as above. Line 174 does go red for a bug in the engine's own limit walk (act_within_limit, engine.cpp:1224), but the record names window-read code. |
| 1 | :189 | engine drops a path the model's walk keeps | does not hold | Same reason: the model walk's answer is the same at either window, so :189 cannot see a wrong e_window_ms. |
| 1 | the sentence "at 60 fewer fills are E0 than at 85, so model_rejected can fall" | the window moves the counts | does not hold | Fills that gain or lose E0 between 60 and 85 have a positive e_offset, so they are never rejected at a 20 ms limit. The window can change model_rejected only through which fills are refused (e_offset between -85 and -60), not through E0. |
| 1 | :199 `kept_checked > 0` | a corpus that exercises no kept path | holds | A bug that leaves the filtered list holding only the optimum leaves kept_checked at 0, so :199 goes red. |
| 1 | :200 `model_rejected > 0` | a corpus with nothing the model rejects | holds | A walk that always says yes leaves model_rejected at 0, so :200 goes red. Whether it is above 0 today is not known (no run). |
| 2 note total | test_engine_sweeps.cpp:240 | covered by record 4 | holds | A path with a wrong notecount fails `== 905`; 905 is the Song's own count (:235). |
| 2 | :242 `paths > 1` | fails only if a window leaves one path | holds | A search that stores one path at depth 200, cap 4 turns it red. |
| 3 all-0 | :569 `zero_ms_charts` not empty | a corpus where no chart keeps a 0 ms timing | holds | An all-0 pass that drops 0 ms timings leaves the set empty (:555 feeds it). Needs the corpus to have one today (no run). |
| 3 | :570 `e1_charts` not empty | a corpus where no chart keeps an E1 | holds | An all-0 pass that drops E1 activations leaves the set empty (:557). Same proviso. |
| 3 | :546-553 relations | an all-0 path with a skip | holds | A skip in the all-0 list fails :546 or :550. |
| 4 skipped fills | test_fills_and_skips.cpp:292-299 | order and fill-ness | holds | A listed tick that is not a fill fails the any_of at :295; out of order or past the activation fails :292-294. |
| 4 | :305 `skipped > 100` | an engine that lists too few fills | cannot tell | Needs the real count from a run of this test on the corpus. If it is near 100 a small cut turns red; if far above, a bug that drops some fills stays green. Run: this test case with a plant that drops every second passed-over fill. |
| 5 trailing, corpus path | :362-364 | a changed best listing path | holds | Exact equality on "3", {123838}, {244800, 281759}; any change in the first listing path turns it red (whether the values are right today is not run). |
| 5 | :340-341 (in check_trailing) | the meter rule | holds | A listed fill where the Preview meter holds under kSpActivationBars fails :341; a non-scene tick fails :340. |
| 6 outlasting | :400 | engine lists a fill that was in the SP | holds | CHECK on an empty list for every path whose last deactivation is past the last note. |
| 6 | :410 `outlasting > 0` | corpus with none | holds | Fails if no path outlasts. |
| 6 | :413 `with_fills_after > 0` | corpus with none | holds | Fails if no outlasting path has a fill after its last activation. |
| 7 full pin | :427 `paths.size() == 1` | pin returns one path | holds | REQUIRE on size. |
| 7 | :429 equality | pin drops or adds a trailing fill | holds | Red for a bug only the pinned search has. A trailing-list bug the free and the pinned search share stays green; the record's wording covers only the first. |
| 7 | :432 `compared > 0` | corpus with no listing path | holds | Fails if no path lists a trailing fill. |
| 8 no activation | :447 | any change in which fills are listed | holds | Exact equality on the eight ticks for Allister - Overrated. |
| 9 banked-phrase charts | test_tied_variants.cpp:241, :243 | SP-end steps follow the activation | holds | A step tick at or before the activation tick fails :241 or :243; :240 requires steps. |
| 9 | :248 | lone-pricing check | holds | A variant priced differently from its lone search gives a non-empty diff. Variants folded under a root of their own are skipped by the helper (record_fixtures.h:406-411), so a bug only in those stays green. |
| 10 clamped_sqin | :281 | the equality | holds | A stored offset_ms that differs from offset_from_sp_end of the real deact tick fails it. |
| 10 | :286 `sqouts > 0` | a chart with no squeeze-out | holds | Fails if neither chart gives a squeeze-out. |
| 11 Don Broco | :615, :616 | the two leftovers | holds | Swapping or changing 1 and 3 turns either red. |
| 11 | :618 `seen == 2` | both paths found | holds | Fails if either path (first activation 150720 or 82560 at score 233100) is missing. |
| 12 cost sum | test_path_costs.cpp:126 | a dropped or doubled term | holds | The three cases have costs 4060, 13340, 1400 and 42820, so a dropped or doubled cost term breaks the sum (:121 also pins them). |
| 13 no cost below zero | :152 | a changed best path | holds | `totalscore() == 706180`. |
| 13 | :154 | a changed best path | holds | `acts.size() == 5`. |
| 13 | :155, squeeze half | acts[3] is over the limit | holds | The comment (:143-144) says the fourth window needs a squeeze tighter than 10 ms; a squeeze check that passes it turns :155 red. |
| 13 | :155, window half (the record's "reads e_window_ms (fill_within_limit)") | the record's own words | does not hold | Same math as record 1: a different window cannot flip fill_within_limit at a 10 ms limit, so the early-fill part is not what keeps :155 red. |
| 13 | :159 | a changed best path | holds | `bf == 6 x 706180`. |
| 14 built graph | :224 | none | holds | The record says it catches nothing. Confirmed: pather.cpp:402-404 makes the song form call the graph form. |
| 14 | :225 | a changed total | holds | `== 407610`. |
| 15 ladder | :240-246 | each part of the stop | holds | A ladder that goes past 22273 makes :239, :240 or :241 red; a wrong realized_prefix, failed_tick, failed_reason or callback list fails :243, :244, :245 or :246. |

Splits seen: the three "does not hold" rows under record 1, and the window half of line 155 under record 13. I judge only question 8.

Counts: holds 32, does not hold 4, cannot tell 1, not read 0.
