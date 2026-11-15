Slice WN4b, record file `docs/audit/2026-11-14-test-audit/records/window-WN4b.md` (checkout `worktrees/test-audit-t8-questions-handoff`). No file of this slice changed since a19b9cb2 (`git log a19b9cb2..HEAD` on test_pb_coach.cpp, test_pb_text.cpp, test_saved_figure_fill.cpp, dm_fixture.h is empty). Line numbers below are current and match the record's. Read only; nothing was run. Verifier id a466437a25e6b1233.

Records 2 and 11 are B records with no question 8 answer, so they have no rows. Record numbers are the record file's own. "Q8 line" is the line the record's question 8 names. Where one named line stands for two different bugs, it has two rows.

| Rec | File:line | What Q8 says it catches | Verdict | Input or run behind it |
|---|---|---|---|---|
| 1 | pb_coach 66 | any change in the ladder's best finishes | holds | typed `{378315,364675,364675}`; any scoring change in `build_ladder` moves it |
| 1 | pb_coach 69 | `build_ladder` returns a total from another place than the path's | does not hold (for one variant) | `player_total` is set at path_costs.cpp:86. If it were set to `bf.back()` it is 364675 here, equal to `two.path->totalscore()` (364675), and to line 67's typed value. Holds for `bf.front()` (378315) |
| 1 | pb_coach 95 | `finish_for_me` returns `best[n]` for the wrong n | does not hold (n-1) | run.best[1] and run.best[2] have the same score (bf[1]==bf[2]==364675), and pathstring "9 1" is typed for both (this test line 96; the Finish test line 464 types best[1] as "9 1"). Wrong n of 0 or 3 does go red (score 378315, or no path) |
| 1 | pb_coach 87-89 | `end_choices` returns a duplicate end | holds | the catching line is 86 (`ends.size()==1`), not 87-89; a duplicate makes size 2 |
| 3 | pb_coach 145 | limit comparison turned around | holds (if the turn is inside `within_ms_limit`/`fill_within_limit`) | the line calls `acts[3].within_ms_limit(10.0)` directly (model.cpp:532); figure 130.4 vs limit 10, so a reversed comparison flips it |
| 3 | pb_coach 145 | same line, as the guard of the coach note ("a window over the limit shows no note") | does not hold | 145 never calls `tight_window_note` (pb_coach.cpp:14). A change there leaves it green; this is the planted.md finding. Lines 154 and 163 catch it |
| 3 | pb_coach 163 | note formatter drops the limit | holds | exact string "needs 130.4 ms, your limit is 10 ms" against `tight_window_note` output |
| 3 | pb_coach 175 | note on a row inside the limit | holds | rows 0-2 of `own_list` must be empty; a note-always change turns them red |
| 4 | pb_coach 194 | sentence names a timing for a squeeze that needs none | holds | exact "over your limit of -200 ms"; the `!needs` branch at pb_coach.cpp:19 |
| 4 | pb_coach 196 | note on a window inside the limit | holds | limit -10 must give an empty note |
| 5 | pb_coach 216, 217 | click accepted though it breaks a later window | holds | both must be empty; an accepted click sets `windows` |
| 5 | pb_coach 218 | `failed_tick == clicked` inverted | holds | repin (pb_coach.cpp:155) picks the at-click sentence if inverted, not "later windows" |
| 6 | pb_coach 259 | `choose_end` pins the wrong end | holds | typed deact 257280 against ends[0]; the other end is 253440 |
| 6 | pb_coach 268 | squeeze-in end wrongly needing timing | holds | `CHECK_FALSE(in[0].required)` |
| 6 | pb_coach 273 | figure formatter | holds | exact "130.4 ms" |
| 7 | pb_coach 285 | `is_kept_end` compares the wrong field | holds | true expected for ends[1]; a mismatching key turns it red (284 catches always-true) |
| 7 | pb_coach 284 | same, other end | holds | `CHECK_FALSE` for ends[0] |
| 8 | pb_coach 346 | ends stored and not recomputed | does not hold | `DecisionList::select` (pb_coach.cpp:632) clears and recomputes every call. A cache keyed on the row would give the same list for opening 3-of-3 and 3-of-5, since both calls hit row 2 and the first one is the last-window list. A bug that adds the later windows to `before` could change the list; that needs a run (plant: `before` = all windows except the row) |
| 8 | pb_coach 361 | same | does not hold | same reason, row 3 |
| 9 | pb_coach 382 | removal drops only one window | holds | `size()==1`; `windows_before` (pb_coach.cpp:117) |
| 9 | pb_coach 386 | any change at all | holds | typed 334495 |
| 10 | pb_coach 423 | `PlayerPath` rule changed | holds | a path returned for an empty window list flips `has_value()==!empty()` |
| 10 | pb_coach 413 | any change at all | holds | typed 358235 |
| 12 | pb_coach 461 | Finish makes two edits | holds | `generation == before+1` |
| 12 | pb_coach 463 | path not BF(1)'s | holds | n-1 gives best[0], a three-activation path, not "9 1"; n+1 has no entry, so no edit |
| 13 | pb_coach 615 | zero cost on the wrong fill | holds | exact equivalence on each of six rows |
| 13 | pb_coach 613 | BF subtraction wrong | holds | typed costs; `candidate_costs` (path_costs.cpp:38) |
| 13 | pb_coach 614 | `cost_tier` thresholds moved | does not hold (small moves) | costs 540 (0.08% of 706180) and 14380 (2.04%) are the nearest rows to the 1% line (path_costs.cpp:26). Moving 1% anywhere between 0.08% and 2.04% leaves every typed tier unchanged. Large moves go red |
| 14 | pb_coach 634, 635 | sort order dropped | holds | ticks handed `{199680,165120}`; `candidate_finishes` sorts (pb_coach.cpp:248) |
| 14 | pb_coach 641 | finish not `best_finish` | holds | compared with a fresh `best_finish` on the same pins (a bug inside `best_finish` itself would move both sides) |
| 15 | pb_coach 666 | only the new tick priced | holds | `got.size()==1` |
| 15 | pb_coach 675 | stale prices after a new path | holds | `candidates().empty()` |
| 15 | pb_coach 662, 673 | cancel not counted | holds | counter must be 1 and 2 (race noted by the record: the queue may finish first) |
| 16 | pb_text 81 | `window_end_tick` replaced by stored end | holds | m135 vs m136 (record's own comment: chord 253320, stored 253440); `window_end_tick` model.cpp:717 |
| 16 | pb_text 83 | pending text | holds | exact "..." string |
| 16 | pb_text 100 | badge words dropped | holds | whole string with badges |
| 16 | pb_text 108 | zero-offset squeeze read as "-0.0" | holds | exact "squeeze in 0.0 ms" |
| 17 | pb_text 126 | pending fills first | holds | expected tail 90240,151680,167040 |
| 17 | pb_text 126 | `stable_sort` replaced by an unstable sort | does not hold | only 8 rows; MSVC `std::sort` uses insertion sort for short ranges, which keeps ties in order. Not run |
| 18 | pb_text 139 | `step_short` uses stored end, not the activation's | does not hold | acts[0] has no squeeze words (line 162), so its window end equals the stored end; both give m29. Line 157 (acts[3]) is what catches it |
| 18 | pb_text 142 | same, acts[2] | cannot tell | needs acts[2].`sqout_tick`; run: plant `deact_tick` instead of `window_end_tick` at pb_text.cpp:159 and run this test case |
| 18 | pb_text 140, 143 | phrase count wrong | holds | typed "2 phrases", "4 phrases" |
| 19 | pb_text 159 | `step_short` words its own squeezes | holds | exact "squeeze out 130.4 ms" |
| 19 | pb_text 162 | spurious squeeze on first step | holds | `squeezes.empty()` |
| 20 | pb_text 196 | end text words differ from the tab's | does not hold | 196 compares `activation_badges` (the owner) with a typed list. A different wording inside `end_choice_text`/`badges_text` leaves 196 green; line 192 and 189 would catch it |
| 20 | pb_text 177 | window-end replaced by stored end | holds | m135 vs m136 |
| 21 | pb_text 227 | fill inside a window offered | holds | `SpRunning` for fill 5 |
| 21 | pb_text 250, 252 | boundary at `after_sp_end` | holds | fill at 195360 must read "Star Power is already on" (pb_text.cpp:61). Needs the test as it stands to pass |
| 22 | pb_text 307 | subtraction reversed | holds | the fixture calls `app::candidate_costs`; reversed gives a non-positive cost, so "best" |
| 22 | pb_text 310 | best-fill name from the wrong fill | does not hold | the card only prints `numbers->best_fill` (pb_text.cpp:219). The test's own fixture builds it (line 284). The real choice is made in pb_window.cpp:121 `best_fill_name`, which the test never calls |
| 23 | pb_text 319 | zero cost worded as a figure | holds | exact "Cost=best" (`cost_or_best`) |
| 24 | pb_text 340, 344 | constants swapped | holds | exact sentences |
| 24 | pb_text 337 | heading measure | holds | exact "m34 · 0:45.435" |
| 25 | saved 111 | any wrong figure written | holds | typed five-figure vector |
| 25 | saved 113 | marker not set | holds | direct check |
| 25 | saved 118 | marker ignored | does not hold | after the first fill, ranks 1-3 hold figures, so `rows_missing_hardest` (record_store.cpp:1630, SQL `hardest_ms IS NULL AND sqin_count>0`) returns nothing and `charts` is 0, so no call is made even without the marker check at saved_figure_fill.cpp:116 |
| 26 | saved 142 | fill ignores a mismatch | holds | rank 1 score +1; `replay_matches` (saved_figure_fill.cpp:28) |
| 26 | saved 143 | fill fails to go on to rank 2 | holds | `continue` at line 72; abort leaves rank 2 empty |
| 27 | saved 155, 156, 158, 162, 165, 164 | stop and resume rules | holds | each can fail; 164 is typed 0.0 |
| 27 | saved 157 | stopped fill marks itself done | holds (early stop only) | the stop here happens at `progress(0,charts)` (line 128), before any chart. The `done < charts` return (line 150) is never reached in this test; case at line 212 covers it |
| 28 | saved 204 | replay called for another text | holds | `thrown==1` |
| 28 | saved 206, 207 | rows written for a skipped chart | holds | rank 2 would be written if a per-row catch kept going |
| 28 | saved 208 | fill stops after the first chart | cannot tell | depends on whether Smash Mouth's id sorts after Everlong's in `rows_missing_hardest` (ORDER BY song_id); run: plant a rethrow in `chart_figures` catch and run this case |
| 29 | saved 300 | any change in the figure | holds | typed exact double |
| 29 | saved 301 | negative timing shown | holds | `shown_timing` of a negative figure must be empty |
| 30 | saved 328 | graph rebuilt per row | cannot tell | a graph rebuilt in a loop is a local freed per row; its stack address and the first node's heap block can well be reused (the test's own comment: "not a proof"). Run: plant a per-row `search_graph` in `replayed_figures` and run this case |
| 30 | saved 329 | same | cannot tell | same run |
| 31 | saved 359 | `hardest_ms` unset on a squeeze path | holds | `CHECK(p.hardest_ms.has_value())` |
| 31 | saved 362 | loop ran on no squeeze path | holds | `squeezed > 0` |
| 31 | saved 363 | a missing row | holds | `rows_missing_hardest(...).empty()` |
| 32 | saved 398 | row revived by the fill | does not hold | `set_hardest` (record_store.cpp:1662) only writes `hardest_ms`, so the status stays Stale even if the fill read the row. 401-403 catch that bug instead |
| 32 | saved 401, 402, 403 | stale row read, replayed or written | holds | counter calls, replays, and the SQL count 3 |

Not a record row: `tests/dm_fixture.h` has no record and no question 8 answer. The record's own "splits" section (dm_fixture reads as search) I saw in code: `sample_chart` does call a search; I judged no test of it.

Counts: holds 70, does not hold 12, cannot tell 4, not read 0.
