Slice WN4a, record file docs/audit/2026-11-14-test-audit/records/window-WN4a.md (checkout test-audit-t8-questions-handoff). Only the 15 "C records" carry a question 8 answer; the B rows carry none, so there was nothing to re-read in them. Changed since a19b9cb2: tests/test_preview_controller.cpp (d664ac4d, 5 lines, outside the lines judged here); src/search/engine.cpp (abb6b45d, eef6b1d6; `follows_pins` is now lines 589-593, so the comparison the plant touched is at 592). The test files test_replay.cpp, test_replay_schedule.cpp, test_preview_scene.cpp, and src/search/pather.cpp, src/core/replay.cpp, src/app/preview_view.cpp: pather.cpp changed (abb6b45d) but `free_after = true` is still set only at pather.cpp:384 (best_finish_options); `run_target` (pather.cpp:288-291) never sets it. Nothing was run. Line numbers below are the checkout's.

| Record | Test line (question 8 names) | Verdict | Why |
|---|---|---|---|
| 1 replay reproduces engine score | test_replay.cpp:144 `mismatches == 0` (replay drops a doubling / skips a window) | holds | `totals_match()` (replay.h:297) compares the replay's total with the stored total on every corpus path; a dropped doubling or window changes the total, and `REQUIRE(pr.all_windows())` at 130 catches a skipped window. |
| 1 | :142 `run.paths >= c.min_paths` (382/382/294) | cannot tell | It goes red on a path-dropping bug only if the real count today equals the typed number. The test prints it (MESSAGE at 136). Run: the test at the current engine, read the "paths" count for each of the three settings. |
| 1 | :141 `run.charts == loading` (loader skips a chart) | cannot tell | `loading_chart_count` (song_digest.h:310) counts charts whose `chart_parse_hash` does not fail; the loop counts charts whose `corpus::song`/`analyze_chart_file` is not empty. Whether one loader bug moves both the same way I could not read out. Run: plant a loader that returns an empty song for one corpus chart and see if 141 goes red. |
| 2 targeted search | test_replay.cpp:253 `mismatches == 0` (named bug: `follows_pins` at engine.cpp:592 stops following pins one fill early) | does not hold | `search_target` reaches `run_target`, which never sets `free_after`, so `follows_pins` returns true at engine.cpp:591 and never reaches the `fill_tick <= back()` comparison at 592. Same input every time: the changed comparison is never evaluated by this test. The general idea (search_target prices a path differently) is checked by 253; only the named example is not. |
| 2 | :251 `REQUIRE(run.paths >= 300)` (loop walks fewer charts) | does not hold | Floor is 300; the record's own count for the same settings is 382 (line 119). A loop that silently skips charts holding up to about 80 paths stays green. It only goes red for a large loss. |
| 3 tied variants | :330 `differing == 0` (variant takes leader's facts) | holds | `lone_pricing` compares each variant's stored facts with a lone search; at cap 4/depth 40 and cap 2/depth 40 the file's comment says hundreds of variants are compared. A variant stored with leader facts differs. I read the comparison, not a run; the comment's "dozens differ" is a claim. |
| 3 | :331 `skipped() == {0,0,4}` (compared variant turns skipped) | holds | A compared variant becoming skipped raises `skipped()` above the typed count. Caveat: the typed 0/0/4 come from an older run; if today's count differs it is red already. |
| 3 | :328 `variants >= {10,200,200}` (engine lists no ties) | holds | With no ties `variants` is 0, below every floor. A partial loss is not caught. |
| 4 SqIn dump | :626 sqin_ticks equal (dump drops sqin_ticks) | holds | Dump writes `sqin_ticks`, `windows_from_json` reads it back; `with_sqins > 0` (637) forces at least one nonempty window to compare. |
| 4 | :635 no warning contains "17360" (warning stops skipping squeezed-in phrase) | holds | It runs only on path "0++" (629), and 638 `saw_0pp` fails if that path is missing, so it cannot pass silently. If the warning named 17360 the CHECK_MESSAGE goes red. |
| 4 | :638 `saw_0pp` | holds | Red when no path is exactly "0++". |
| 4 | :644 old dump reads as empty sqin (reader rejects old dump) | holds | `REQUIRE(old.size()==1)` and the empty check; a reader that throws or drops the window goes red. |
| 5 result block | :874 empty bestpath is "" | holds | `result_json` (tools/replay_json.cpp:199) takes it from `best_path_text()` of an empty record. |
| 5 | :873 empty score is null | holds | Line 198: `best ? score : nullptr`; a real-looking 0 turns `is_null()` red. |
| 5 | :881 score 228710 (typed snapshot) | holds | Any change in the best score goes red. Whether 228710 is still the current value I could not tell without a run. |
| 5 | :882 bestpath "1 2" (typed snapshot) | holds | Same as 881. |
| 6 paths_json fills | :904 dump equals walk's skipped fills (dump prints nearest fill, not stored) | holds | Dump line 168 writes `act.skipped_fill_ticks`; 904 compares with `walk_activations()`'s value; printing the nearest fill (24960) would differ from the stored 19200. |
| 6 | :910 first activation's list is {19200} | holds | Same bug gives {24960}; red. |
| 7 Pitch Dark scale | :1541-1542 read after 466 is step 1, planned end 158262.53 (read priced at squeeze's ratio) | holds | The squeeze-ratio price is the value rejected at 1543 and the step check at 1541. Typed values come from the live path 0. |
| 7 | :1556-1559 reads after 437 and 144 at own step 0 | holds | 1556-1557 compare the read's ratio with `step_transfer_scale` of step 0, 1559 compares the planned-minus-stored gap with that scale; another step's scale fails them. |
| 7 | :1586-1588 fallback threshold in read terms | holds | `own_end_at_read` converts with the press's conversion (0.79467, not 1); a threshold left in own terms comes out different. |
| 7 | :1601-1602 conversion times read ratio equals end ratio | holds | Loops p676 presses, `REQUIRE_FALSE(p676.empty())` at 1594; a wrong conversion breaks the product. |
| 7 | typed figures 1565, 1567, 1574-1580, 1598, 1608-1611 (snapshots) | holds | Any change in path, ticks or scale moves them. The 1521-1532 REQUIREs stop the case if path 0 changed. Whether they all match today's engine I did not run. |
| 8 Pitch Dark range | :1638-1639 range 157545.2 / 158350.6 within 0.1 | holds | A range off the step's span moves it by more than 0.1 ms. |
| 8 | :1644-1647 press with end carries range, without carries null | holds | Loops every press; chord 644 etc. have ends, so a missing range goes red. |
| 9 1.0 rule scene | test_preview_scene.cpp:301 (named as "passed-over fill turned Hidden"; the line is `fills[1]==Hidden`, and the passed-over fill A is `fills[0]` checked at 301-300 in the file's current text: 300 end_tick, 301 Offered) | holds | Read by content: the check `fills[0].state == Offered` is the passed-over fill; Hidden would be red. (Record's numbering is off by one: current lines are 301 Offered, 302 Hidden, 303 Taken.) Split: the record's line numbers differ from the checkout by one on 302. |
| 9 | :303 taken fill is Taken | holds | `fills[2]` must be Taken. |
| 10 ghost list | test_preview_scene.cpp:909 `ghost_fills == taken` | holds | Ghost list vs the same path drawn; a differently built list differs. Needs `taken` nonempty, which 903 forces. |
| 10 | :903 `{2, 7}` | holds | Typed snapshot; any change goes red. |
| 11 trailing fills | :942 scene copies the trailing list | holds | Dropping the list leaves it empty against the path's two ticks (REQUIRE 938). |
| 11 | :961 path without list draws fills Hidden | holds | There are fills after `last_act` (157922 etc. at 953), so a scene that lights them without the list goes red. |
| 11 | :953 string of states | holds | Snapshot; any state change goes red. |
| 12 overlay job | test_preview_controller.cpp:263 overlay not empty | holds | A dropped path leaves `activations` empty. Needs the first chart's best path to have an activation (it must today or the line is red already). |
| 12 | :266-268 (named "base reuse") | does not hold | 266 (`out.base != pro_base`), 267 (`track_opts == opts`) and 268 (base holds no activations) catch a base built for the other pro setting being reused, or a path leaked into the base. They do not catch the base being rebuilt on every job instead of shared; that is line 278 (`out2.base == out.base`), which the answer does not name. The record's own question 7 failure is "base rebuilt on every path change". Split: the label "base reuse" fits two bugs, the named lines see only one. |
| 13 ghost lands | :643 ghost list `{2, 7}` | holds | An unbuilt ghost list is empty. |
| 13 | :620 `path_identity(found.best) == path_identity(ghost)` | holds | As the record says, it stops the case when two searches pick different paths; it is not a bug detector. |
| 14 changed chart no ghost | :692 `ghost_fills.empty() == changed` | holds | If a changed chart still drew the ghost, ghost_fills would be non-empty and red. For the unchanged run it forces a nonempty ghost list. |
| 14 | :693 `activations.empty() == changed` | holds | Same for the drawn path. |
| 15 Preview hides path | :886 `chart_changed() == changed` | holds | Detection error in either direction goes red. |
| 15 | :887 `scrub_marks().empty() == changed` | holds | `build_scrub_marks` (preview_view.cpp:762) makes one mark per scene activation when a song length is usable; a changed chart still drawing marks goes red. |

Counts: holds 35, does not hold 3, cannot tell 2, not read 0.
(Counting rows with several lines as one verdict each: 1541-1542, 1556-1559, 1586-1588, 1601-1602, the typed-figures row and 1638-1639 are one row each.)

Does not hold: test_replay.cpp:253, test_replay.cpp:251, test_preview_controller.cpp:266-268. Cannot tell: test_replay.cpp:142, test_replay.cpp:141.
