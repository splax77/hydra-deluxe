# T3 run 2: records for tests/test_preview_boxes.cpp

Agent a6ad0e80624dcd323, session 5caaef5d-321e-4f84-b720-518c46d428ba. One record per `TEST_CASE`, in file order (53 cases). All line numbers are in `tests/test_preview_boxes.cpp` unless a path is given.

## Shared reads (R0), cited by every record

R0 means: `tests/test_preview_boxes.cpp` lines 1-1097 (all); `tests/preview_song_fixtures.h` lines 1-189; `src/app/preview_view.cpp` lines 270-289, 395-520, 537-835; the `src/` caller grep for every box function (below); `Activation::deact_tick` and `window_end_tick` (`src/core/model.cpp:712-721`); `kMeasuresPerSpBar` (`src/core/timing.h:254`). Not read, anywhere in this file: `src/app/preview_view.h` beyond grep hits (lines 107-127, 366-530 seen in grep output only), `tests/record_fixtures.h`, `clock_str`, `format_measure`, `display_tick_at_ms`, `build_score`, `build_preview_scene` body before line 395, and the commit diffs. For question 7 I read only the commit subjects from `git log -- tests/test_preview_boxes.cpp`; no `git log -S` was run, so "adding commit: not read" holds for every record.

Callers found in `src/` (grep of every `name(` call): `build_time_box`, `build_score_box`, `build_drain_box`, `build_scrub_marks`, `build_next_act_box`, `build_solo_box`, `build_solo_box_samples`, `activation_jump_ms` and `step_shown_ms`: `src/ui/preview_controller.cpp` only (lines 438, 466-527). `struck_at`: `preview_view.cpp:627,707,717,733`, `preview_view.h:409`, `src/render/highway_draw.cpp:247`. `solo_percent`: `preview_view.cpp:720` only. `has_song_length`: `preview_view.cpp:371,758,775`, `src/ui/preview_panel.cpp:396`, `preview_timeline.cpp:401`, `preview_transport.cpp:75`, `timeline_view.cpp:28,82`, `song_length.cpp:14,54`, `path_view.cpp:461`. `song_fraction`: `preview_view.cpp:768`, `path_view.cpp:269`. `scrub_thumb_ms`: `preview_panel.cpp:394`. All of these are display code (a person reads what they return); the game facts they lean on (SP end, replay rows) come from `Activation::window_end_tick`/`sp_end_steps` and `replay_stored_path`, which the engine owns.

Engine record used for question 3 (`docs/audit/2026-11-14-test-audit/records-engine/`, index.md and `pitch_dark_ratio.schedule.json` lines 1-60 read): Pitch Dark, first activation (act tick 109440, chord 144): `sp_end_steps` are activation (end tick 124800), clamped at chord 166 (end 128400), squeeze_in at chord 243, tick 128520, end tick 132240; `deact_tick` 132240, `deact_ms` 103880.974. The Preview's own SP-end reading is `window_end_tick()` (`preview_view.cpp:435-445`), which is the last step's `end_tick` (`model.cpp:712-721`): tick 132240, so the display and the record name the same end. No dumped chart holds a tempo map, a meter change, a solo, a score or a star count, so the engine records say nothing about the time box, the solo box, the stars or the scrubber. Those rows read "claim not checked" and name the chart that would be needed.

Where a test shows the whole setup in its own fixture, "fixture" means `tests/preview_song_fixtures.h`.

Common Q2 answers, written once. TIME: the display owns clock text and measure labels; the owner of tick, tempo and meter is `SongTiming` in core (`scene.timing->display_tick_at_ms`, `tempo_at`, `time_signature_at`), and `build_time_box` is called only by `preview_controller.cpp:466`. That is a display's own formatting of engine values, so it is fine. SOLO: the display owns the solo box text; the percent rule is the game's (SC-23, `docs/clone-hero/06-scoring.md` line 247 header, Label: Unknown) and `solo_percent` is called only from `solo_box_of`. Not a game rule invented by the display, but the page's Label is Unknown. SCORE and DRAIN: the engine owns the window (`Activation::window_end_tick`) and the replay rows (`replay_stored_path`); the display reads both.

---

## 1. build_time_box: timestamp, measure, tempo (tests/test_preview_boxes.cpp:115)

Read: R0; lines 115-135.
1 Claim: the time box shows the playhead as m:ss.mmm over the length, the playhead's measure.beat.ticks, the last measure, the tempo and the signature, and no section line without sections.
2 Owner: TIME (display of core timing). Caller: `preview_controller.cpp:466`.
3 Real chart: claim not checked. Needs a corpus chart with its tempo map and one dumped time box; none dumped. The fixture's own timing is hand-built.
4 L122 `timestamp == "0:01.300 / 0:05.000"`: raw result (clock text of two inputs). L123 `"m1.3.288"`: raw result, working only in the comment at L119 (1300 ms = tick 1248 = 2 beats + 288). L124 `"m3.3.0"`: raw result, working in comment. L125 tempo string: raw result. L126 empty section line: property. L128 `"m1.1.0"` at 0 ms: the first beat, reads as the rule. L131 `"1:04.000 / 1:04.000"`: raw. L134 clamp: reads as the rule (a playhead past the length shows the length).
5 Reason: 1300 and 5000 are free inputs, but the expected strings have no stated working in code. 120 BPM and 4/4 come from the fixture. No D or CH rule is cited.
6 Display: expected side typed, not read from any record. No engine record exists for time text; the chart's timing is the owner and the test types hand-worked results.
7 Failure: not read (commits: f734ef87 renamed the file, no failure named). Named by me: a playhead showing the wrong bar or beat on screen. Still possible.
8 L122 (reasoned: a wrong clock format); L123/L124 (reasoned: tick-to-beat off by a beat); L125 (reasoned: tempo or signature line wrong); L126 (reasoned: stray section text); L128 (covered by L123 shape); L131 (reasoned: minute carry); L134 (reasoned: clamp removed).
9 The arithmetic in the comment is the real risk and is not in code. No floor or skip.
10 `tests/test_display_format.cpp:29-37` repeats clock_str("1:04.000", "0:01.300") and says in its comment that this test pins them "through the time box". L131 and L122 restate that rule: two tests, one rule (clock text).
11 Yes: any real song has these readings.
Verdict: Rewrite. State each reading as the rule on named inputs (`1300 ms -> tick 2*480+288`), not as a typed string with the working in a comment, and check one real chart's reading.
Today it catches: clock, measure, beat, tempo and signature text on a four-note fixture.
Side notes: test name holds a comma (L115). L131 and L122 repeat `test_display_format.cpp:31-36`.

## 2. build_time_box: the timestamp rounds 59,999.6 ms to 1:00.000 (:137)

Read: R0; lines 137-143; `test_display_format.cpp:29-37`; D48's text (`fix-decisions.md:122-156`) and question 20 (`2026-10-04-phase-3-5-questions.md:98-100`): "Today the Preview clock can read 0:60.000 for half a millisecond before each minute... Recommended: the clock reads 1:00.000." Decision D48: "The user answered 'all recommended' to all 33 questions." That is the user's approval of the recommendation.
1 Claim: a playhead 0.4 ms before a minute shows 1:00.000, never 0:60.000.
2 Owner: `clock_str` (display), not opened. Caller of the box: `preview_controller.cpp:466`.
3 Claim not checked: needs a real playhead near a minute on a real chart; no chart data decides this.
4 L142 one string equals "1:00.000 / 1:04.000": directly the rule (the minute carries). Input 59999.6 is the rule's own named input (D48 Q20 states 59,999.6 as an input in `fix-decisions.md:149`).
5 59,999.6 has its reason: D48 Q20 and the line at `fix-decisions.md:149` list it as an input. 64000 is a free input.
6 Display typed value, but it is the rule itself.
7 Failure: the clock reading 0:60.000 (Q20). Still possible if rounding moves.
8 L142 catches rounding after the split (reasoned).
9 None.
10 Same rule as `test_display_format.cpp:31` (`clock_str(59999.6) == "1:00.000"`). One rule, two tests.
11 Yes.
Verdict: Remove. `test_display_format.cpp:29-31` makes the same promise on `clock_str` directly; this adds only that the time box calls it (which test 1's L122 checks).
Today it catches: the box not calling `clock_str`.

## 3. build_time_box: the end measure runs past the last beat line (:145)

Read: R0; lines 145-155.
1 Claim: when the length is past the last beat line the end measure keeps counting at the last tempo and signature (30 s is bar 16 exactly).
2 TIME. `scene.beats` is built by `build_beat_events` (`preview_view.cpp:537`); the box reads `display_tick_at_ms`.
3 Claim not checked; needs a corpus chart whose audio runs past the last note and its time box.
4 L150 `scene.beats.back().tick == 4320`: a fixture fact (REQUIRE). L152 `"m16.1.0"`, L153 same: raw, working in the L148-149 comment (30 s = tick 28800 = 15 bars of 1920). L154 clock: raw.
5 28800 and 16: working in comment only. 4320 pinned from the fixture; no reason beyond "the beat grid stops there".
6 Typed.
7 Not read. Named: the end label capping at the last beat line.
8 L152 (reasoned: end clamps to last beat line); L150 guards the fixture, would not catch a bug in the code.
9 REQUIRE guards an accident of the fixture's grid length.
10 Related to `build_time_box: every lookup...` (test 52), L1078-1081 which reads a length past the last note again.
11 Yes: audio past the last note is real (D255).
Verdict: Rewrite. Say bar = 30000 ms / (ms per measure) as named inputs in code, and drop the REQUIRE on the grid's end.
Today it catches: the end measure stopping at the last beat line.

## 4. build_time_box: the practice section in force (:157)

Read: R0; lines 157-177.
1 Claim: the time box names the practice section at or before the playhead, and nothing before the first section.
2 `last_at_or_before` over `scene.sections` (`preview_view.cpp:604`). Sections come from the chart (`song.practice_sections`, scene build line 401-402). Display of chart data.
3 Claim not checked; needs a corpus chart with sections. `tests/test_song_input.cpp:61-80` reads a chart's sections through the box, which I did not open further.
4 L164-168 section tick, ms 500/2500 and name: the scene's copy of the chart's data (inputs, 480 ticks = 500 ms by the 120 BPM fixture). L171-176 section_line strings: each is "Section " plus the name of the section whose tick is at or before the playhead: reads as the rule on named inputs (L159-160). L172 400 ms: before the first section, empty.
5 480 and 2400 ticks are chosen inputs; their ms in comments.
6 Display typed, but it types the section names it was given. Fine.
7 Not read. Named: wrong section on the box.
8 L173 (reasoned: boundary `<=` turned `<`); L175 (same, second section); L172 (reasoned: first-section edge); L164-168 are fixture copies; a scene-building bug in `sections` would show here.
9 None.
10 `test_song_input.cpp:61-80` checks the section line from a real chart; related. Not duplicate.
11 Yes.
Verdict: Rewrite (only question 3 open; the checks stand): run the same promise on a corpus chart with sections and take the names from the parsed chart.
Today it catches: the section line choosing the wrong section at the boundary.

## 5. build_time_box: a mid-measure meter change restarts the grid (:179)

Read: R0; lines 179-203; `docs/clone-hero/code/6142/grid_restart_20DACA0.md` named by grep only, not opened.
1 Claim: after a signature change at a non-measure tick, the box and the drawn measure lines both restart the measure at that tick.
2 The measure owner is `MeasureIndex` in core; the box reads it through `format_measure` and the scene's beats read it through `build_beat_events` (`preview_view.cpp:543`). Two display consumers of one owner.
3 Claim not checked: needs a corpus chart with a mid-measure signature change; the Clone Hero rule page is not opened.
4 L190 `"m3.2.240"`: raw, working in comment L187-188 (tick 3600 = 3750 ms; 720 ticks into measure 3). L191 tempo `3/4`: reads as chart input. L198 REQUIRE size >= 4 guard. L199-202 line ticks 0, 1920, 2880, 4320: the grid, each the end of the earlier measure plus its length; stated in comment L181-183. This checks the box against the same drawn lines (property) but the lines themselves are typed.
5 2880 and 3/4: free inputs. "Clone Hero's grid restarts there": the CH page not opened, so the rule is not traced.
6 Typed, not from a record.
7 Not read. Named: grid not restarting at a mid-bar change.
8 L190 (reasoned); L199-202 (reasoned: a restart rule turned off moves them).
9 `REQUIRE(lines.size() >= 4)` is a floor to avoid out-of-range reads.
10 Test 6 states the neighbouring rule (the next beat). Same owner, different rule.
11 Mid-measure meter changes occur in real charts (the grid restart is a Clone Hero behaviour, source page not opened).
Verdict: Rewrite. Open the Clone Hero page for the restart, cite its rule id at the numbers, and check the box and lines on a real chart with such a change.
Today it catches: box and drawn lines disagreeing after a mid-bar change.

## 6. build_time_box: a change between two beats shows from the next beat (:205)

Read: R0; lines 205-221.
1 Claim: a 3/4 written 20 ticks after a beat first shows at the next beat (tick 3360), in the position and in the signature.
2 `MeasureIndex` / `time_signature_at`; display reads it. Same as 5.
3 Claim not checked; needs a corpus chart with a signature written off the beat.
4 L215 `m2.3.120` raw (working: tick 3000 = 1920+960+120). L216 4/4 chart input. L219-220 `m3.1.0` and `3/4`: raw.
5 2900 chosen; 3360 = next beat (2880 + 480) in comment.
6 Typed.
7 Not read. Named: the signature switching at the written tick instead of the grid's.
8 L219-220 (reasoned: lookup uses the written tick); L215-216 (reasoned: early switch).
9 None. 10 Test 5. 11 Real charts write signature changes off the beat? Not established; no corpus chart named.
Verdict: Rewrite (question 3 open; add the real-chart evidence that the case occurs).
Today it catches: early signature switching.

## 7. build_time_box: the time signature in force, as the chart wrote it (:223)

Read: R0; lines 223-245.
1 Claim: the box shows the signature the chart wrote (6/8 vs 3/4 both 1440 ticks a measure), and a beat is an eighth in 6/8.
2 Display reading `time_signature_at`. Caller as above.
3 Claim not checked; needs a chart with 6/8 and 3/4.
4 L230-233 tempo labels at 0, 1999, 2000, 3500 ms: each reads as the rule on the inputs (L227). L236 `m1.4.479`, L237 `m2.1.0`: raw, one tick from the change. L241 `m2.3.20`: raw, working in comment L238-240. L243-244: default scene reads "BPM 0.000 · 4/4" and `m1.1.0`: a typed default, with no source.
5 1920 and 3360: chosen inputs. 0.000 default: no reason found (`PreviewTimeSig` default read only through the comment at preview_view.cpp:589-590: "which is Song's").
6 Typed.
7 Not read. Named: a 6/8 section shown as 3/4.
8 L232 (reasoned: signature label from tick); L241 (reasoned: beat size 480 in 6/8); L243 (reasoned: default signature changes).
9 L243-244 pin a default for a scene nobody builds in the app (`build_time_box(PreviewScene{}...)`). Does the Preview ever show a scene built from nothing? The caller passes `scene_`; not read whether it can be empty.
10 None. 11 L243-244: reachable? Not shown.
Verdict: Rewrite. Replace the L243-244 literal with a property (a scene built from nothing reads the same as a song with the default grid) and run the 6/8 case on a real chart.
Today it catches: signature labels, 6/8 beat size.

## 8. step_shown_ms: a step moves the shown moment by exactly its size (:247)

Read: R0; lines 247-253; `step_shown_ms` (`preview_view.cpp:751-753`).
1 Claim: a Preview step of +10 or -10 ms moves the shown time by 10 ms.
2 Display (transport step). Caller: `preview_controller.cpp:438`.
3 Claim not checked: no chart input. The step size (10 ms) is from commit 1ebe6a60 ("Preview step moves 10 ms instead of 5 ticks"), subject only.
4 L248 `1310`, L249 `1290`, L250 `1310.25`, L252 `1300`: each the input plus the delta, reads as the rule.
5 Inputs only; 10 is a chosen step, not a rule number.
6 A display test with no engine side; it is pure arithmetic on the playhead.
7 Not read beyond the subject. Named: the step moving by a different amount than shown (ticks vs ms).
8 L248 (reasoned: `+ delta` replaced); L249 (sign); L250 (rounding to whole ms); L252 (zero delta).
9 None. 10 Test 10 repeats L248-style. 11 Yes.
Verdict: Rewrite (only question 3 open: it has no chart). Give it a chart's length and playhead from a corpus song.
Today it catches: a step moving by the wrong amount.

## 9. step_shown_ms: never before 0 or past where playback ends (:255)

Read: R0; lines 255-260.
1 Claim: a step never lands before 0 or past the length.
2 Same.
3 Claim not checked.
4 L256-259: results 0, 0, 5000, 5000: clamps; read as the rule.
5 5000 length, deltas: free inputs.
6 No engine side.
7 Not read; the transport owns the range (5178b821, subject only).
8 L256 (reasoned: lower clamp); L258 (upper clamp); L257, L259 (already at the edge).
9 None. 10 `shown_ms` clamp also hit by test 10 and test 1 L134. 11 Yes.
Verdict: Rewrite (question 3 only, as 8).
Today it catches: either clamp removed.

## 10. step_shown_ms: a playhead outside the song steps from the moment shown (:262)

Read: R0; lines 262-270.
1 Claim: a playhead past the end steps from the end the time box shows, not from itself.
2 Same.
3 Claim not checked.
4 L265-266: two timestamps equal each other: a property (two displays of one moment). L267 `4990`: input minus 10 (reads as rule: the shown moment 5000 less 10). L269 `10`.
5 Free inputs.
6 Compares the box with itself: fine because the owner is `shown_ms`.
7 Not read. Named: a step from 6000 landing on 5990.
8 L265 (reasoned: box and step reading different moments); L267 (reasoned: step from `now_ms` not shown); L269 (negative start).
9 None. 10 Overlaps test 9 for clamping; the new part is starting from the shown moment. 11 Yes.
Verdict: Rewrite (question 3 only).
Today it catches: stepping from the raw playhead.

## 11. score box: no path hides the box (:272)

Read: R0; lines 272-278.
1 Claim: a song with no path has no score box.
2 `build_score_box` returns early for State::None (`preview_view.cpp:614`). Caller: `preview_controller.cpp:470`.
3 Claim not checked; no chart needed beyond one with no path.
4 L275 state None: a state read. L276 steps empty: property. L277 `CHECK_FALSE(shown)`: directly the claim.
5 None needed.
6 It reads the scene state, fine.
7 Not read. Named: a box with 0 points over a song that was never analyzed.
8 L275 (state set when no path: reasoned); L276 (reasoned: stale steps); L277 (reasoned: box drawn with no path).
9 None. 10 `unavailable` and None both appear in test 24. 11 Yes.
Verdict: Rewrite (question 3 only; keep the three checks).
Today it catches: score box shown without a path.

## 12. score box: a solo's bonus lands on its last note (:280)

Read: R0; lines 280-309; `replay_path` / `replay_stored_path` not opened.
1 Claim: the on-screen total stays without the solo bonus until the solo's last chord, and then includes it.
2 The engine owns the replay rows (`r.chords[i].cum_onscreen_total`, `cum`); the display copies them into `scene.score.steps` (`build_score`, not opened). Caller of the box: `preview_controller.cpp:470`.
3 Claim not checked: the dumped charts have no solo and no score rows.
4 L286-287 state and 4 steps: guard. L290 `points.solo > 0`: guard. L293 `steps[1].total == r.chords[1].cum_onscreen_total`: the owner's answer, item by item (good). L294 `== 180`: snapshot, the comment L291-292 says "pinned from one run". L295 and L296: owner's answers. L302 score text equals `group_thousands(...)` of a step: property. L303 `"x1 · combo 2"`: raw result. L307-308: same with combo 3.
5 180: no reason (the comment names it as pinned from one run). Combo 2/3 follow from the fixture's chord list.
6 L293-296 read the engine's rows for the step totals; L294 types a value. Only three of the seven expected values come from the replay.
7 Not read. Named: the bonus paid early or late on the Preview's score.
8 L293 (reasoned: the withheld total); L294 (nothing a snapshot adds that L293 does not catch); L295 (bonus paid on last chord); L296 (final total); L303, L308 combos.
9 L294 is a snapshot standing in for L293; L287 `steps.size()==4` ties to the fixture's chord count.
10 Test 23 L538-541 repeats the solo bonus rule.
11 Yes.
Verdict: Rewrite. Delete L294; take "x1 · combo N" from the replay's `multiplier_after` and `combo_after`.
Today it catches: the withheld total and the final total, which stay.

## 13. score box: before the first note nothing is hit yet (:311)

Read: R0; lines 311-322.
1 Claim: before the first chord the box reads 0 points, x1, combo 0.
2 Display (`it == steps.begin()` leaves `at` default, `preview_view.cpp:629-630`).
3 Claim not checked.
4 L319 `score == "0"`, L321 `"x1 · combo 0"`: raw results, but "0" and "x1" are the start values of the rule; reads as the rule.
5 100 ms is a chosen input before 500 ms. 
6 Typed; the replay's first row cannot say "before the first".
7 Not read. Named: the box showing the first chord's score before it is hit.
8 L319 (reasoned: first chord applied early); L321 (default multiplier).
9 None. 10 Test 14 L328 covers the struck rule at the edge. 11 Yes.
Verdict: Rewrite (question 3 only).
Today it catches: pre-first-note default.

## 14. struck_at: a chord exactly on the playhead counts as hit (:324)

Read: R0; lines 324-330; D48 Q27 (`2026-10-04-phase-3-5-questions.md:128-130`): "Today, after '< Act' or 'Act >', the score box already counts the chord under the playhead while the highway shows it unlit. Recommended: it counts as hit in both, so a jump lands on a struck gem." The user answered "all recommended" (D48).
1 Claim: a chord at the playhead's exact ms, and any later playhead, counts as struck; an earlier one does not.
2 Owner: `struck_at` (`preview_view.h:402`). Callers: the score box (`preview_view.cpp:627`), the solo box (707, 717, 733), the playing window `preview_view.h:409`, and the highway flash (`src/render/highway_draw.cpp:247`). Shared by two displays, so one owner.
3 Claim not checked: highway and score box on a real chart at an activation chord. `tests/test_highway_draw.cpp:364` states the same rule for the highway (comment only).
4 L327-329: three comparisons with 2500, 2499.9, 2500.1: each directly the rule (inclusive at the exact point), with the rule's source D48 Q27.
5 2500 ms comes from the comment (activation chord at tick 2400 = 2500 ms in the fixture below). Inputs.
6 Pure function; no engine side.
7 Failure: a jump landing on an unlit gem (D48 Q27). Still possible.
8 L327 (reasoned: `<=` turned `<`); L328 (reasoned: always-true); L329 (reasoned: reversed).
9 None. 10 Test 17 (:384) states the same rule through the box; test 15 L362 too. `test_highway_draw.cpp:364` too. One rule, one verdict; this one is the pure statement.
11 Yes.
Verdict: Rewrite (question 3: no chart yet). The checks stand.
Today it catches: struck_at turned exclusive.

## 15. score box: the multiplier is the replay's, doubled on chords Star Power pays (:332)

Read: R0; lines 332-369; D11 text (`fix-decisions.md:29`): "The Preview score box shows the plain multiplier from the SP end on. That includes the 40 test-set chords that land under 2 ms past the SP end, which Hydra's 3 ms leeway still pays doubled." No quoted user words in that entry.
1 Claim: while Star Power runs the box shows the doubled disc; from the SP end on it shows the plain disc, even though the chord on the SP end was paid doubled in the replay.
2 Engine: replay rows `multiplier_shown`, `multiplier_after`. The display chooses between them by its own window test (`running_activation`, `preview_view.cpp:634-635`), not by the row's `in_sp` flag.
3 Engine: Pitch Dark, first activation: Star Power ends at tick 132240 (the record's `deact_tick`, from `sp_end_steps` last step). Claim: the box reads plain from that tick on (`window_end_tick` = 132240). The replay row for the chord at the end is `multiplier_shown` doubled (the test's own L352-353 comment says "the chord on it (6500 ms) was paid doubled, but from this instant on the box is plain"). The box and the row differ at that moment. No chart's per-chord rows are in records-engine; the difference is the test's own statement.
4 L347 `steps[i].multiplier == multiplier_shown` for each step: the owner's answer, item by item. L349 `multiplier_plain == multiplier_after`: same. L354 `"x4 · combo 13"`, L355 `"x2 · combo 14"`, L356-357 same at `sp_end_ms`, L360 `"x1 · combo 5"`, L362 `"x2 · combo 6"`, L363 `"x2 · combo 7"`, L366 `"x2 · combo 14"`, L368 `"x2 · combo 15"`: raw results; working in the comments (a note every 500 ms; combo 10 first doubles). L341 `sp_end_ms == 6500`: raw (2400 + 3840 ticks). L339-340 REQUIRE: guards.
5 6240 and 6500: from the rule "one bar of SP lasts two measures" (`kMeasuresPerSpBar`, `timing.h:254`) applied by hand: the test passes the end tick itself (L336), so the engine never produced it. D11 backs the plain reading, but its text has no quoted user words.
6 L347/L349 compare with the replay (good); the typed combos do not.
7 Not read (D11 text cited in a comment at :398; subject 148ea547 etc. unrelated). Named: the score box showing x4 after Star Power ended.
8 L347 (reasoned: multiplier read from the wrong field); L349 (plain disc); L355 (reasoned: window end turned `<=`); L366 (reasoned: leaves doubled after end); L368; L354 (reasoned: end moved early). The typed lines duplicate each other.
9 Typed combos stand in for "the box reads the replay at that chord"; none is derived. One test packs three claims.
10 Test 18 (:401) states the same D11 rule for the leeway chord. One verdict covers both.
11 Yes: Pitch Dark's first activation ends at a chord that was paid doubled (leeway).
Verdict: Investigate. The box shows plain at the SP end while the replay's row for that chord says doubled; D11 backs the display but has no user quotation. Goes to the user as a split.
Production finding: `build_score_box` (`preview_view.cpp:634-635`) decides doubled-or-plain from the display's own window (`running_activation`) rather than from the replay row's `in_sp` and `multiplier_shown`: a game fact (which chord Star Power pays) decided by display code.
Split: Preview score box says plain from the SP end on; the replay says the chord on the end (and the leeway chord) was paid doubled (D11 against the record).
Today it catches: the step fields equal the replay's rows; the box multiplier by moment.
Side notes: name holds a comma (:332).

## 16. score box: past the length it reads the moment the time box shows (:371)

Read: R0; lines 371-382.
1 Claim: a playhead past the playback end shows the score as at the end.
2 `shown_ms` (`preview_view.cpp:563-566`), shared by all boxes. Caller `preview_controller.cpp:470`.
3 Claim not checked: no chart; the length comes from the transport (`playback_end_ms`).
4 L378-379 `"x1 · combo 5"` at length 2000: raw, working in comment L372 (2000 ms five notes). L380-381 `"x2 · combo 15"` at 7000: raw.
5 7000 and 2000 are inputs; the combos follow from a note every 500 ms.
6 Typed.
7 Not read: d0d91262 "Preview playback ends at the song's length, the one end (D255 answer 5)", subject only.
8 L378 (reasoned: length ignored); L380 (reasoned: clamp removed).
9 None. 10 Same rule in tests 28 (drain), 39 (solo), 48 (next act); `shown_ms` is one function, so four tests repeat one clamp. Test 1 L134 and test 8 too.
11 Yes.
Verdict: Rewrite (question 3 open; and merge the four copies into one property over all boxes at one `now`).
Today it catches: the score box not clamping.

## 17. score box: a chord exactly on the playhead counts as hit (:384)

Read: R0; lines 384-396.
1 Claim: "Act >" from the start lands on the activation chord and the score box counts it as hit and paid.
2 `activation_jump_ms` (display) and `struck_at`. Same owners as 14.
3 Claim not checked; Pitch Dark's activation 0 chord 144 would be the real case; no score rows dumped.
4 L395 `"x2 · combo 6"`: raw, working in test 15's comment ("the activation chord is hit and paid").
5 D48 Q27 (opened above).
6 Typed.
7 Failure: the jump landing on an unlit gem. Possible.
8 L395 (reasoned: struck_at exclusive gives x1 combo 5).
9 None. 10 Duplicates test 14 (rule) and test 15 L362 (same number at the same ms). Test 14 states the rule; test 15 L362 states this result.
11 Yes.
Verdict: Remove. Test 14 and test 15 L362 make the same promise; the jump's landing on `a.ms` is also checked by test 46 (L888).
Today it catches: struck_at exclusive combined with the jump.

## 18. score box: a leeway chord past the SP end is paid doubled, the box stays plain (:401)

Read: R0; lines 398-443; D11 as above; `core::default_rules().backend_leeway_ms` not opened.
1 Claim: a chord a hair past the SP end that Hydra's leeway still pays doubled is doubled in the score and plain on the box.
2 Engine owns the leeway (`default_rules().backend_leeway_ms`) and `in_sp`; the display owns the plain reading by its own window.
3 Engine: the replay's row for that chord, `in_sp` true and `multiplier_shown` doubled (test L434-435). Claim: the box reads plain at that instant (L441-442). They differ, by the test's own design. No record chart carries per-chord rows.
4 L421 `sp_end == 6500`; L432-433 leeway ms within `backend_leeway_ms` (a property on a typed input 6241); L434-435 `in_sp`, doubled: the owner's answer. L436-437 step fields equal the row: owner's answer. L441-442 box text equals `multiplier_after` and `combo_after` of the row: item-by-item from the record.
5 6241 is "1 tick past" an end the test passes by hand (comment L402-403: "about 1 ms", inside the "3 ms leeway"; the 3 ms is not a typed number here).
6 Good for L441; but the claim is that the display differs from the record (the hard rule applies).
7 Not read. Named: the box going doubled at the leeway chord.
8 L432-433 (reasoned: placing the chord outside leeway); L434-435 (reasoned: leeway not applied); L441 (reasoned: window end `<=`).
9 The `std::find_if` + `insert` into `song.sequence` builds a state; fine.
10 Same rule as test 15 (:332). One verdict.
11 D11 says 40 test-set chords land under 2 ms past the SP end, so it happens on real charts.
Verdict: Investigate. The claim (plain on the box) differs from the replay's row (doubled) and D11 backs the display; goes to the user with test 15.
Production finding: as test 15 (`preview_view.cpp:634-635`).
Split: same as test 15.
Today it catches: the leeway chord being paid doubled in the replay and the Preview's step fields.
Side notes: name holds a comma (:401).

## 19. score box: a path the replay can't reproduce says so (:445)

Read: R0; lines 445-466.
1 Claim: if the stored score disagrees with the replay by one point, or an activation has no deactivation node, the box says "Score unavailable".
2 `build_score` (not opened) sets State::Unavailable; the box prints it (`preview_view.cpp:616-618`).
3 Claim not checked; the case occurs for old records.
4 L452, L464 state read. L455 `CHECK_FALSE(available)`; L456 `"Score unavailable"`: raw text of a typed message; L457 detail empty; L465.
5 The one-point difference: a chosen input.
6 Text typed (display wording); the state is the record's.
7 Not read. Named: a wrong score shown for a record the replay cannot reproduce.
8 L452 (reasoned: tolerance widened); L456 (reasoned: wording); L464 (reasoned: no-deact path priced anyway).
9 None. 10 None found. 11 Old records without SP ends: stored-version stamps, not read.
Verdict: Rewrite (question 3 open).
Today it catches: the unavailable state on both triggers.

## 20. score box: the analyzed chart ends on the path's total (:468)

Read: R0; lines 468-481; `corpus::analyzed_by_ten_scores` not opened (which chart: unknown).
1 Claim: on an analyzed corpus chart the box's last step carries the best path's total, steps are in time order and combo grows.
2 Display of replay rows; owner `Path::totalscore` and the replay.
3 Claim not checked; the chart behind `analyzed_by_ten_scores` is not read, and it is not among the six dumped charts.
4 L473 steps count equals chord count: property. L474 last total equals `best.totalscore()`: owner's answer. L476 `ms >=` previous: property. L477 `combo >` previous: property. L480 end score text equals `group_thousands(best.totalscore())`.
5 None needed (properties).
6 Good: the expected side is the engine's (`totalscore`).
7 Not read. Named: a Preview score disagreeing with the Paths tab.
8 L474 (reasoned: final total off by the solo); L476 (reasoned: unsorted steps); L477 (reasoned: a chord skipped: combo equal); L473 (reasoned: step skipped).
9 The loop covers one chart only; a failure names the index? `CHECK` inside a loop does not name the chord. `CHECK(a > b)` prints values.
10 Test 22 uses the same chart; the final total fact also in tests 12 (L296).
11 Yes.
Verdict: Rewrite (question 3: the chart is not in the engine records; name it and have the main session run it). The checks otherwise stand.
Today it catches: the box's last total.

## 21. score box stars: before the first chord no star is earned (:485)

Read: R0; lines 485-496; `star_cutoffs` not opened (`core/stars.h`).
1 Claim: before the first chord the box shows 0 stars and "star 1 in <first cutoff>".
2 `stars_for_score` (core/stars.h) owns stars; display formats the text (`preview_view.cpp:642-647`). Caller as above.
3 Claim not checked.
4 L492 stars 0: property of "before first". L493 `next_star == "star 1 in " + group_thousands(star_cutoffs(path).cutoffs[0])`: reads from the owner (good). L495 `"star 1 in 20"`: snapshot, the comment L494 says "Pinned from one run."
5 20: no reason; a snapshot.
6 L493 reads the owner; L495 types.
7 Not read. Named: the wrong cutoff on the first star line.
8 L493 (reasoned: cutoff index off by one); L495 catches nothing L493 does not (reasoned: duplicates).
9 L495 stands in for L493.
10 Test 22 and 23 state the star line rule.
11 Yes.
Verdict: Rewrite. Delete L495.
Today it catches: the first-star text on a no-notes-hit playhead.

## 22. score box stars: the analyzed chart ends on the path's own star count (:498)

Read: R0; lines 498-511.
1 Claim: at the end of an analyzed chart the box's stars equal the path's star count, and "next star" is empty exactly at the max.
2 `path_stars` (owner) vs the box. 
3 Claim not checked: which chart? not read.
4 L505 `stars == path_stars(best)`: owner's answer. L506 `next_star.empty() == (stars == kMaxStars)`: property. L509 `stars == 7`: snapshot, comment L507-508 "Pinned from one run". L510 `empty()`: same.
5 7: no reason; the chart's own count; a snapshot that changes when the chart or scoring changes.
6 L505 reads the owner.
7 Not read. Named: stars not matching the details header.
8 L505 (reasoned: stars computed from total not stars score); L506 (reasoned: line shown at max); L509-510 duplicate L505-506.
9 Stand-in snapshot.
10 Test 23 L547.
11 Yes.
Verdict: Rewrite. Delete L509-510; keep L505-506.
Today it catches: stars agreeing with `path_stars`.

## 23. score box stars: a solo's bonus raises the score but not the stars (:513)

Read: R0; lines 513-550; `star_cutoffs` not opened.
1 Claim: the solo bonus adds to the score the box shows but not to the stars, which count the score without the solo.
2 `stars_for_score(cut, at.score_without_solo)` (`preview_view.cpp:644`). Display uses the cutoffs from the scene (`build_score`).
3 Claim not checked: no chart with a solo among the dumps.
4 L520 `path.score_solo == 200` REQUIRE (input). L523 `cutoffs[2] == 380`: REQUIRE of a snapshot value (comment L516-517 "pinned from one run"). L527 `"180"`, L528 `1`, L529 `"star 2 in 10"`, L535 `"480"`, L536 `2`, L537 `"star 3 in 100"`, L546 `"580"`, L548 `3`, L549 `"star 4 in 380"`: snapshots, each from one run; L547 `== path_stars(path)` owner. L540-541: the score step difference equals the chord's own points plus `score_solo`: a property.
5 The comment at L515-516 says the literals "were pinned from one run": no reason.
6 Typed values.
7 Not read. Named: stars counting the solo bonus (a 3-star showing at 480 vs 2).
8 L536 (reasoned: stars from total not `score_without_solo`); L540-541 (reasoned: bonus not on last chord); the rest duplicate.
9 The REQUIRE on 380 turns a snapshot into a precondition: if scoring changes, the test fails there, not at the claim.
10 Test 12 L295-296 covers the bonus step; test 22 L505.
11 Yes.
Verdict: Rewrite. State: the star count at a moment equals `stars_for_score` on the replay's score without the solo; the score text equals the replay's total.
Today it catches: stars ignoring the solo bonus (kept by the rewrite).
Side notes: none.

## 24. score box stars: no score, no stars (:552)

Read: R0; lines 552-565.
1 Claim: with no path, or an unreproducible path, the star counter is 0 and empty.
2 Display. 
3 Claim not checked.
4 L555-556, L563-564: zero and empty: reads as the rule.
5 None.
6 Display's own.
7 Not read. 
8 L555 (reasoned: stars shown with no score); L561-562 REQUIRE guard the setup; L563-564.
9 None. 10 Test 11 and 19 overlap. 11 Yes.
Verdict: Remove. Test 11 (no path hides the box) and test 19 (unavailable) already pin the state; this adds only the star fields, which nothing else pins (so the star fields' two lines move into those tests).
Today it catches: star fields staying blank on no path and an unavailable one.
Side notes: name holds a comma (:552).

---
## Extra reads for records 25-53

`src/core/timing.h:254-255` (`kMeasuresPerSpBar`, `sp_bars_to_measures`); `src/search/graph.cpp:303-309,388-395` (grep lines only: the search ends SP with `plusmeasure(tc, sp_bars_to_measures(n))`); `src/app/preview_view.cpp:334-341` (grep lines: the solo bonus is `solo_bonus` from `core/scoring.h:77-79`, `kSoloBonusPerNote = 100` at `core/model.h:147`); `docs/clone-hero/06-scoring.md` SC-23 (grep output of lines 247-253 only, Label: Unknown); `tests/test_preview_controller.cpp:507,523` and `tests/test_preview_transport.cpp:301` (grep lines, bodies not opened).

## 25. drain box: idle on a chart with no Star Power (:569)

Read: R0; lines 569-583; `preview_view.cpp:651-688`; timing.h:254.
1 Claim: with no activation the drain box still shows, idle, saying one SP bar lasts 4.0 s and a full meter 16.0 s at 120 BPM 4/4; a scene built from nothing shows no box.
2 Display of an engine constant: `kMeasuresPerSpBar` (core) times `ms_per_measure_at` at the playhead's tick (`preview_view.cpp:663-664`), and the meter cap `scene.sp_meter.cap`. The search works SP length out by `plusmeasure(..., sp_bars_to_measures(n))` across tempo changes (`graph.cpp:303-309`), and keeps no "how long a full bar lasts from here" anywhere a screen can read. Callers: `preview_controller.cpp:474`. The "if activated" times are the display's own estimate, one measure length times a constant. Row: carry out (engine code `plusmeasure`, `src/search/graph.cpp:303`).
3 Real chart: claim not checked. The dumps hold activations only, no idle drain.
4 L571 default scene not shown: property. L576 REQUIRE no segments: guard. L578-579 shown, not active. L580 header text. L581 `"1 bar / 4.0 s"`: raw, working only in the fixture comment (`preview_song_fixtures.h:105-108`: one SP bar burns 4000 ms). L582 `"full meter 16.0 s"`: raw, 4 bars x 4 s.
5 The 4.0 s and 16.0 s follow from `kMeasuresPerSpBar` x 2000 ms x cap 4; the cap 4 is not shown in the test. No page or ruling cited for a bar being two measures (the rule number lives in `timing.h:254`; CH page not opened).
6 Typed values; the expected side is the display's own working. No engine value to compare.
7 Not read (bdce7147 "the SP gauge and drain box show on every song", subject only). Named: the idle box missing on a no-SP song.
8 L571 (reasoned: box shown with no gauge); L580 (header); L581 (reasoned: rate formula); L582 (cap factor).
9 L576 stands in for "no segments" of the fixture.
10 Test 26 L593-595 pins the same strings at 120 BPM.
11 Yes.
Verdict: Rewrite. Express "1 bar / N s" as `kMeasuresPerSpBar` measures of the fixture's own measure length (2 x 2000 ms by named inputs), and put the estimate to the user: the display's "full meter" is its own number.
Production finding: the idle "1 bar" and "full meter" times (`preview_view.cpp:663-684`) are the display's own estimate of a game fact (how long a full meter lasts), local measure length times a constant, while the search integrates across tempo changes (`graph.cpp:303-309`).
Today it catches: idle box present, texts.

## 26. drain box: idle at a steady tempo reads the rate and a full meter (:585)

Read: R0; lines 585-596.
1 Claim: at a steady tempo the idle box reads 1 bar / 4.0 s and full meter 16.0 s.
2 As 25.
3 Claim not checked.
4 L590 shown, L591 not active, L592 header, L593 rate, L595 detail: the same five values test 25 pins (L578-582). Raw results; the comment on L594 "Cap 4: eight measures, 16000 ms" works it out.
5 As 25.
6 Typed.
7 Not read.
8 Every line repeats test 25's.
9 None.
10 Same rule and same numbers as test 25 L578-582 and test 27 L603 and L608.
11 Yes.
Verdict: Remove. Test 25 and test 27 (L603-604, L608-610) make the same promise.
Today it catches: nothing the other two do not (only the fixture has a phrase at tick 960, which the box ignores).

## 27. drain box: the rate switches exactly at a tempo change (:598)

Read: R0; lines 598-611.
1 Claim: at a tempo change (120 to 60 BPM at 6000 ms) the rate and the full-meter time change at that moment, not before.
2 Display; the tempo is the chart's. Engine's SP length across a tempo change: `plusmeasure` (`graph.cpp:303`), which the box does not use.
3 Claim not checked; Pitch Dark's tempo map is not dumped, and the estimate has no engine record to compare.
4 L603 `"1 bar / 4.0 s"` at 5990, L604 `"8.0 s"` at 6000: raw, working in L599 comment. L608-610 full-meter 16.0, 16.0, 32.0: raw, working in L606-607.
5 5760 ticks = 6000 ms (comment); 60 BPM: input.
6 Typed; and the displayed 32.0 s is a number the engine never computes (it would integrate any later tempo change).
7 Not read. Named: the rate lagging a tempo change.
8 L603 (reasoned: tempo looked up at the previous tick); L604 (same, new side); L608-610 (reasoned: cap computed at a fixed tempo).
9 None. 10 Test 25/26 for the 120 BPM values; tests 29 and 53 for other lookups.
11 Yes.
Verdict: Rewrite (question 3: claim not checked; same finding as 25).
Today it catches: tempo lookup at the right instant.

## 28. drain box: past the length it reads the moment the time box shows (:613)

Read: R0; lines 613-625.
1 Claim: a playhead past the playback end reads the end's rate and full-meter time.
2 `shown_ms`; as 16.
3 Claim not checked.
4 L620-621 `4.0 s`, `16.0 s` at length 5990; L623-624 `8.0 s`, `32.0 s` at 6000: raw results; the working is in test 27's fixture.
5 Free inputs.
6 Typed.
7 Not read (d0d91262). 8 L620 (reasoned: length ignored); L623.
9 None. 10 Copy of the same clamp as tests 16, 39, 48 (all call `shown_ms`).
11 Yes.
Verdict: Remove. Tests 16, 39 and 48 check the same clamp through three other boxes, and `shown_ms` is one function; one test of it through any box is enough. (If the user wants one test per box, this one stays as a Rewrite on question 3.)
Today it catches: the drain box not calling `shown_ms`.

## 29. drain box: a 7/8 section drains faster at the same BPM (:627)

Read: R0; lines 627-636.
1 Claim: in 7/8 at 120 BPM a bar lasts 3.5 s (a 7/8 measure is 1750 ms), so the full meter is 14.0 s.
2 `ms_per_measure_at` (core timing) times the constant. Display estimate as 25.
3 Claim not checked; a corpus chart in 7/8 is not named.
4 L633 `"1 bar / 4.0 s"` at 2000: raw. L634 `"1 bar / 3.5 s"` at 4000: raw, working in L628-629. L635 `"full meter 14.0 s"`: raw.
5 7/8 and tick 3840: inputs.
6 Typed.
7 Not read. Named: the drain box ignoring the signature.
8 L634 (reasoned: measure length from the BPM alone); L635; L633 (before the change).
9 None. 10 Test 7 reads the signature in the time box; different rule (bar length).
11 7/8 sections occur in real charts: not shown here.
Verdict: Rewrite (question 3; same finding as 25).
Today it catches: bar length ignoring the signature.

## 30. drain box: active inside the stored SP window, idle outside it (:638)

Read: R0; lines 638-660; `PreviewActivation::sp_running_at` (`preview_view.h:407`, grep only, body not read).
1 Claim: the box counts down while the playhead is inside the activation's stored SP window and reads idle from the stored end on.
2 The engine owns the window (`Activation::window_end_tick`, `model.cpp:717-721`); the display reads it (`preview_view.cpp:435-445`) through `running_activation` shared with the score box.
3 Engine: Pitch Dark, first activation: `sp_end_steps` end with squeeze_in end tick 132240, `deact_tick` 132240. Claim: the box reads `window_end_tick()` = the last step's end tick = 132240, and a squeeze-out ends it at `sqout_tick` when earlier (`model.cpp:719`). They agree (by reading; the screen was not captured).
4 L646 not active at 3999, L647 header `"SP drain (if activated)"`, L650 active at 4000, L651 `"SP drain"`, L652 rate raw, L653 `"empties in 8.0 s"` raw (working L639: 12000 - 4000), L654 `"empties in 3.0 s"` raw, L658 inactive at the end moment 12000, L659 detail begins "full meter ": a prefix read.
5 4000 ms and 12000 ms: from the test's own input (`sp_act_at(song, 3840, 2, 11520)`); the end tick is typed, not from the engine.
6 Expected side typed, but the window itself is hand-built where the engine would stamp it. The comparison to `sp_end_ms` is missing: the test never reads `scene.activations[0].sp_end_ms`.
7 Not read. Named: the box counting down past the SP end.
8 L646 (reasoned: window start `<` vs `<=`); L650; L653 (reasoned: end minus now; wrong end); L658 (reasoned: end boundary `<=`).
9 None. 10 Test 15 L354-357 states the same boundary through the score box (sp_end exclusive).
11 Yes (every activation).
Verdict: Rewrite. For each record step, the box is active from the activation's tick up to the record's last end tick and idle from it, with "empties in" equal to (end ms - now); no typed seconds.
Today it catches: the window's start and end boundaries; the countdown.
Side notes: name holds a comma (:638).

## 31. drain box: empties in reads the stored end, not a recount (:662)

Read: R0; lines 662-675.
1 Claim: when the stored steps move the SP end two measures later (a phrase collected during SP), "empties in" counts to the stored end.
2 Engine owns `sp_end_steps`; display reads the last (`deact_tick`).
3 Engine: Pitch Dark, first activation: the end chain moves from the nominal 124800 to 132240 (a clamped step, then a squeeze-in). Claim: the box counts to 132240. They agree (by reading). The fixture's `SpEndKind::Collected` kind does not appear in the Pitch Dark record (kinds seen: activation, clamped, squeeze_in).
4 L673 active; L674 `"empties in 4.0 s"`: raw, working in L663-664 (16000 - 12000).
5 Hand-built steps `{3840,11520,Activation},{5760,15360,Collected}`: the end 15360 = 11520 + 3840 (two measures) by hand.
6 Typed; the end is hand-typed where the record would carry it.
7 Not read. Named: the box counting down to the original end.
8 L673 (reasoned: window ends at first step); L674 (reasoned: first step's end used).
9 None.
10 Test 30; the "reads the last step's end" rule is also in test 15's fixture path (window_end_tick).
11 Yes (Pitch Dark).
Verdict: Rewrite (checks stand but the claim has no real-chart screen capture; take the end from the record's step).
Today it catches: the box using the first step instead of the last.
Side notes: name holds a comma (:662).

## 32. drain box: an activation with no stored end stays idle (:677)

Read: R0; lines 677-689.
1 Claim: an activation with no SP-end steps reads idle, not active.
2 `has_sp_end` false when `window_end_tick()` is empty (`preview_view.cpp:441-445`).
3 Claim not checked. Whether the engine ever stamps an activation with no steps: not read (`deact_tick` returns nullopt when `sp_end_steps` is empty, `model.cpp:713`).
4 L683 REQUIRE no sp end. L686-688 shown, not active, detail begins "full meter ".
5 None needed.
6 Display reads engine's absence.
7 Not read. Named: a record with no deactivation drawing a running box.
8 L683 (guard); L687 (reasoned: window defaulted to a long end).
9 None. 10 Test 19 L459-465 uses the same kind of activation for the score box.
11 Old records only (test 19's comment). No real chart named; this may be unreachable.
Verdict: Investigate. Nobody has shown a chart whose activation has no SP-end steps; if none can exist, the case is dead.
Today it catches: idle on missing steps.

## 33. solo_percent: the game's float32 truncation (:691)

Read: R0; lines 691-700; `solo_percent` (`preview_view.cpp:690-697`); SC-23 header lines (grep): "During a solo the game shows trunc(float(h) / float(N) times 100.0f) percent", Label: Unknown.
1 Claim: the solo percent is the game's, hit/total in float32 times 100, truncated (53 of 100 reads 52).
2 Display's own copy of a game rule. Only `solo_box_of` calls it (`preview_view.cpp:720`). The game rule is on page `06-scoring.md` SC-23; the engine does not carry a solo percent.
3 Claim not checked: no chart or game run was compared. SC-23's Label is Unknown for the box.
4 L694 `52`, L695 `0`, L696 `85`, L697 `33`, L698 `100`, L699 `0`: the first is justified by the comment (float just under 0.53); the others are results of the same function with no working shown.
5 SC-23 says trunc(float(h) / float(N) x 100.0f). The test's values are not on the page. 53/100 is an input with a reason (comment).
6 No engine value; the rule is the game's.
7 Not read. Named: whole-number math reading 53 where the game shows 52.
8 L694 (reasoned: float steps removed); L697 (reasoned: rounding vs truncation); L699 (zero total); the others are repeats.
9 None. 10 Only here. 11 53/100 and similar occur in real solos.
Verdict: Rewrite. Cite SC-23 where 52 is typed, and choose the inputs from the rule (hit/total pairs where float and whole-number math differ), and check one real solo against the game.
Today it catches: float truncation.
Side notes: SC-23's Label is Unknown.

## 34. solo box: from before the solo to its bonus, timed off the last chord (:702)

Read: R0; lines 702-736; SC-23 timing sentence (grep): "After the last note it shows a message for 1.5 s and the bonus for 1.5 s"; `kSoloResultMs = 1500`, `kSoloTailMs = 2 x` (`preview_view.h:438-439`).
1 Claim: the box is hidden before the first solo chord, counts notes during the solo, shows "Perfect Solo!" after the last chord for kSoloResultMs, then "+200" for the same time, then hides.
2 Display, with `solo_bonus` from core (`scoring.h:77`) for the amount. Caller: `preview_controller.cpp:505`.
3 Claim not checked: no dumped chart has a solo. The 1.5 s comes from SC-23 (Label Unknown).
4 L710 hidden at 249.9; L714-717 "50%" and "1 / 2 notes" at 250 and 400; L721-724 end message at 500, detail empty; L725 still the message just before `kSoloResultMs`; L728-731 "+200" at the result ms; L732 still "+200" before the tail; L735 hidden at tail. Expected values: percent and count raw; timings use the named constants (reads as the rule). L706 REQUIRE `path.score_solo == 200` (2 notes x 100).
5 200: two notes x `kSoloBonusPerNote` (100): working not shown.
6 The bonus text compared with `path.score_solo`? No: the 200 is typed.
7 Not read (f700167a "a solo box like Clone Hero's", subject only). Named: the end message lasting the wrong time.
8 L710 (reasoned: shows early); L714 (percent); L721 (end message at last chord); L725/L728 (boundary at 1.5 s); L735 (hides at 3 s).
9 None. 10 Test 36 re-states the sequence for two solos.
11 Yes.
Verdict: Rewrite (question 3: needs a real solo; and L706/L730 should read `path.score_solo`).
Today it catches: the solo box's timeline.

## 35. solo box: the count is notes, the bonus thousands-grouped (:738)

Read: R0; lines 738-751.
1 Claim: the count is notes (chords with two notes count two), and percents follow.
2 Display; `scene.solo_notes` from `solo_note_count` (core).
3 Claim not checked.
4 L742-743 `notes == 4` each: raw, working in two_solo_song's comment (1 + 2 + 1). L747-748 `"75%"` and `"3 / 4 notes"` at 750: raw. L749-750 `"25%"`, `"1 / 4 notes"` at 500.
5 Inputs.
6 Typed.
7 Not read. Named: counting chords instead of notes.
8 L742 (reasoned: chords counted); L747 (reasoned: first chord counted); L749.
9 The name says "the bonus thousands-grouped" but no line here checks a bonus: that is test 38. The name claims more than the body.
10 Test 38 covers grouping.
11 Yes.
Verdict: Rewrite (question 3 open; rename).
Today it catches: notes versus chords in the count.
Side notes: name holds a comma (:738); name claims a bonus check the body does not make.

## 36. solo box: a solo that starts during the last one's results takes over (:753)

Read: R0; lines 753-769; SC-23 header sentence (grep): the box appears when the player hits the solo's first note; no sentence there says a new solo replaces the previous results.
1 Claim: a solo starting while the last one's message or bonus shows replaces it; the second solo's results run their own full time.
2 Display (`build_solo_box` picks the solo whose first chord was struck last, `preview_view.cpp:731-735`). The game's behaviour is not stated on a page I read.
3 Claim not checked; the Clone Hero behaviour is not in SC-23's text I read.
4 L759 `"Perfect Solo!"` at 1499.9; L762-763 `"50%"`, `"2 / 4 notes"` at 1500; L766-768 end message, `"+400"`, hidden at tail: raw results.
5 "The new solo wins" has no cited source; D-number: none cited. Session cards not searched for it.
6 Typed.
7 Not read. Named: the old solo's results hiding the new solo's counter.
8 L762 (reasoned: first solo kept).
9 None. 10 Test 34 states the single-solo sequence.
11 Two solos 500 ms apart: no real chart named.
Verdict: Investigate. What the game shows when a solo starts during the last one's results is not on a page I read; the claim has no source.
Today it catches: a new solo taking over.

## 37. solo box: the same with or without a path (:771)

Read: R0; lines 771-785; `preview_view.cpp:334-341` (grep lines).
1 Claim: the solo box reads the same whether or not a path is drawn.
2 `scene.solo_notes` is built from the song with `solo_bonus` (core), not from a path (grep lines 334, 341).
3 Claim not checked.
4 L775 `path.score_solo == 800`: raw (two solos x 400; working missing). L778-783 for eight playheads, `shown`, `headline`, `detail` equal: property.
5 800 = 8 notes x 100 (`kSoloBonusPerNote`), not shown. The eight playheads: free inputs.
6 Compares two displays of one source; fine.
7 Not read. Named: the box depending on a path's replay.
8 L775 (it would fail only if scoring changes); L779-783 (reasoned: stale path data in the box).
9 L775 is a snapshot-style guard for the fixture.
10 None. 11 Yes.
Verdict: Rewrite (question 3 open); delete L775 or derive it from `kSoloBonusPerNote`.
Today it catches: path-dependent solo text.

## 38. solo box: a bonus over a thousand has a separator; none without solos (:787)

Read: R0; lines 787-798.
1 Claim: a bonus of 12000 prints "+12,000"; a song with no solos shows no solo box.
2 `group_thousands` (core/util, called in many places). Display.
3 Claim not checked.
4 L794 `"60 / 120 notes"`, L795 `"+12,000"`: raw results of the formatter; L797 hidden: property.
5 12000 and 60/120: free inputs.
6 Display formatting of an input.
7 Not read. Named: no thousands separator.
8 L795 (reasoned: grouping dropped); L797 (reasoned: box on a no-solo song).
9 L795 duplicates `test_display_format.cpp:42` (`group_thousands(1000)`), where the formatter is tested directly.
10 `test_display_format.cpp:39-45` pins `group_thousands`.
11 Yes.
Verdict: Rewrite. Keep L797 (the box is hidden with no solos), and drop L795 (the formatter has its own test).
Today it catches: the solo box absent without solos (kept); grouping (dropped, covered).
Side notes: none.

## 39. solo box: past the length it reads the moment the time box shows (:800)

Read: R0; lines 800-806.
1 Claim: a playhead past the playback end reads the end message, not hidden.
2 `shown_ms`.
3 Claim not checked.
4 L804 `"Perfect Solo!"` at length 500: raw. L805 hidden with no end: raw.
5 4500, 500: inputs.
6 Typed.
7 Not read. 8 L804 (reasoned: clamp ignored). 9 None.
10 Same clamp as tests 16, 28, 48.
11 Yes.
Verdict: Remove. The same clamp is pinned through the score box (test 16); one test of `shown_ms` through any box is enough.
Today it catches: the solo box not calling `shown_ms`.

## 40. solo box samples: each solo's widest boxes (:808)

Read: R0; lines 808-821; `build_solo_box_samples` (`preview_view.cpp:738-749`).
1 Claim: for each solo the sampler returns the first chord, the last before the end, the end message and the bonus, all shown.
2 Display layout sampling; caller `preview_controller.cpp:510`.
3 Claim not checked.
4 L813 REQUIRE `samples.size() == 8`: a count (2 solos x 4). L814-819 headlines at fixed indices, L816 detail, L820 every sample shown.
5 8 = 2 x 4 (the comment says per solo: four samples).
6 Typed.
7 Not read. Named: a sample missing, so the box sized too small.
8 L813 (reasoned: a sample dropped); L814-819 (order); L820 (shown).
9 L813 is a count standing for "the right items": the headlines check the items.
10 Tests 34-36 pin the same headline texts.
11 Yes.
Verdict: Rewrite. Say `samples.size() == 4 * solos`, with the 4 named.
Today it catches: sample set shape.

## 41. scrub marks: each activation's onset over the scrubber's length (:823)

Read: R0; lines 823-833; `TwoActs` (`preview_song_fixtures.h:174-187`).
1 Claim: each activation's mark is its start ms over the scrubber's length, clamped to 0..1; no length, no marks.
2 `song_fraction` (`preview_view.cpp:757-760`), called by the scrub marks and by `path_view.cpp:269`: one owner, two displays.
3 Engine: Pitch Dark, first activation: tick 109440 (chord 144). Claim: the mark is that tick's ms over the length (`a.ms = song.timecode(pa.tick).ms()`, `preview_view.cpp:430`). They agree in construction. The song's length for Pitch Dark is not dumped: claim not checked for the number.
4 L827 `0.2`, L828 `0.8`: raw (2000/10000 and 8000/10000; working on the fixture at `preview_song_fixtures.h:170-173`). L830 clamp to 1.0: rule applied. L831 length 0: empty. L832 no path: empty.
5 10000 and 4000: free inputs.
6 Typed fractions.
7 Not read. Named: marks off the bar.
8 L827 (reasoned: wrong divisor); L830 (clamp removed); L831 (no-length guard).
9 `REQUIRE(marks.size() == 2)`: count; the values follow.
10 Tests 42 and 44.
11 Yes.
Verdict: Rewrite. Each mark equals `scene.activations[i].ms / length`, written from named inputs, with a real chart's length.
Today it catches: marks, clamp and the no-length guard.

## 42. song_fraction: a clamped share, none without a length (:835)

Read: R0; lines 835-842.
1 Claim: song_fraction is ms over length, clamped to 1, and none for length 0.
2 Owner of the fraction; callers listed in R0.
3 Claim not checked.
4 L838 `0.2`, L840 `1.0`, L841 none: each reads as the rule.
5 Inputs.
6 Pure function.
7 Not read. 8 L838 (reasoned: divisor); L840 (clamp); L841 (guard). 9 None.
10 Test 41 L827-831 covers the same fraction through the marks.
11 Yes.
Verdict: Remove. Test 41 makes the same promise at the marks (L827, L830, L831), and test 44 compares the two real callers.
Today it catches: song_fraction directly (negative ms to a clamp at 0 is checked nowhere else).
Side notes: name holds a comma (:835).

## 43. song_fraction: has_song_length takes only a positive length (:844)

Read: R0; lines 844-848.
1 Claim: a length of 0 or less is no length.
2 `has_song_length` (`preview_view.cpp:755`): called at the 12 sites in R0.
3 Claim not checked; D255 states where a song's length comes from (the chart's end event), not what counts as none.
4 L845-847: three results.
5 None needed.
6 Pure.
7 Not read. Named: a zero length drawing an empty scrubber.
8 L846 (reasoned: `>=`); L847 (negative).
9 None. 10 Test 42 L841 relies on it (0.0).
11 Yes.
Verdict: Rewrite (question 3 only; checks stand).
Today it catches: positivity rule.

## 44. scrub marks: an activation sits at the Paths timeline's fraction when the audio outlasts the notes (:856)

Read: R0; lines 850-872; D48 Q25 and its replacement D255 item 5 (`fix-decisions.md:379`: "One length everywhere. The Paths timeline, the Preview's scrub bar and its SP meter all read this length.").
1 Claim: with audio lasting 5 s after the last note, an activation's mark on the Preview scrubber is the same number as its dot on the Paths timeline.
2 `song_fraction`, called by both (`preview_view.cpp:768`, `path_view.cpp:269`). One owner.
3 Claim not checked on a real chart: the audio-tail case is Rime Of The Ancient Mariner (D255); it is not dumped. No engine record names the length.
4 L863 REQUIRE one mark; L864 `0.125` raw (750/6000, comment L851-852). L869-870 REQUIRE the view has one act with a fraction. L871 marks[0] equals the Paths view's fraction: property (two displays agree).
5 750 ms and 6000 ms: the fixture's own.
6 L871 compares two displays; the owner is one function both call, so it fails only when their inputs differ.
7 Not read. Named: the two displays placing the same activation differently (D255 item 5).
8 L864 (reasoned: divisor); L871 (reasoned: one display given a different length, as before D255).
9 L871 compares two callers of one function; the inputs are the risk, which is what the test targets.
10 Test 41; `test_activation_rows.cpp:626-664` checks the Paths side.
11 Yes (D255).
Verdict: Rewrite (question 3 open: name the chart, Rime Of The Ancient Mariner, and capture both marks).
Today it catches: scrubber and timeline disagreeing on the length.
Side notes: test name is long, no comma.

## 45. scrub marks: a playhead past the last note parks the thumb at the right end (:877)

Read: R0; lines 874-884; `scrub_thumb_ms` (`preview_view.cpp:772-776`); `test_preview_controller.cpp:507,523` and `test_preview_transport.cpp:301` (grep lines only).
1 Claim: the thumb follows the playhead past the last note, all the way to the audio end.
2 `scrub_thumb_ms`; caller `preview_panel.cpp:394`. Display.
3 Claim not checked.
4 L880 `end == 6000`: fixture value. L881 `scrub_thumb_ms(6000, 6000) == 6000`; L882 3000 -> 3000; L883 500 -> 500: each reads as the rule, but no line passes a playhead past the end.
5 6000: the fixture's audio end.
6 n/a.
7 Not read. Named: the thumb stopping at the last note.
8 L881-883 (none can fail if `std::min` is deleted: `min(now, end)` equals `now` for all three; reasoned). L880 guards the fixture.
9 The name says "past the last note" and "parks"; the test never gives a playhead beyond the end.
10 The controller and transport tests (lines above) call it too.
11 Yes.
Verdict: Rewrite. Add a playhead past the end (e.g. end + 1000) and require end; the current lines cannot fail on the clamp.
Today it catches: the thumb stopping early (the last note) before the audio end.

## 46. activation jumps: nearest activation before or after the playhead (:886)

Read: R0; lines 886-897; `preview_view.cpp:778-793`.
1 Claim: "Act >" goes to the next activation after the playhead (0.5 ms of slack), "< Act" to the one before; none at the ends or with no path.
2 `activation_jump_ms` display; callers: `preview_controller.cpp:527`.
3 Engine: Pitch Dark, activations at ticks 109440 and 202480-region (index lists them). Claim: the jump lands at each activation's own ms. Agree by construction (`a.ms`). Claim not checked on the screen.
4 L888 `2000`, L890 `8000` raw (fixture working in `preview_song_fixtures.h:170-173`). L891 `2000.3` +1 -> 8000: uses the 0.5 ms tolerance with no reason in the test. L892-896 none cases: property.
5 0.5 ms: no reason; see test 50.
6 Typed.
7 Not read. Named: a jump landing on the same activation.
8 L890 (reasoned: tolerance 0); L893 (reasoned: backward jump skipping); L895 (end).
9 None. 10 Test 50 pins the same 0.5 ms for the box.
11 Yes.
Verdict: Rewrite. Expected ms read from `scene.activations[i].ms` (record's ticks), and the 0.5 ms question goes with test 50.
Today it catches: jump order and ends.

## 47. next activation box: the activation at or after the playhead (:899)

Read: R0; lines 899-910.
1 Claim: the box names the first activation not yet behind the playhead with its measure and its chord, hidden past the last or without a path.
2 Display of the record's activation tick and chord; measure label `format_measure` (not opened). Caller `preview_controller.cpp:489`.
3 Engine: Pitch Dark, first activation: act tick 109440, chord 144. Claim: the box says "at <measure of tick 109440>" and its chord. The record names the tick; the measure text is the display's format of it. Colour/chord not in the schedule file I read. Claim not checked on the screen.
4 L903 shown; L904 `"Next: activation 1 of 2"` raw; L905 `"at m2.1.0 · [Red snare]"` raw (tick 1920 = measure 2, the fixture); L906 `2000.0` still 1; L907 `2001.0` -> 2, `"at m5.1.0"` raw (tick 7680 = bar 5); L908 hidden at 9000; L909 no path.
5 Inputs and fixture.
6 Typed.
7 Not read (17c3a358). Named: the box naming a passed activation.
8 L904 (reasoned: index); L906 (reasoned: at-or-after turned strict); L907 (second); L908.
9 `2000.0` with `2001.0` depend on the 0.5 ms (same as 50). 
10 Test 49-51 repeat the same box and detail text.
11 Yes.
Verdict: Rewrite. Expected measure and chord read from the activation's tick and chord in the record.
Today it catches: ordinal and visibility.

## 48. next activation box: past the length it reads the moment the time box shows (:912)

Read: R0; lines 912-917.
1 Claim: a playhead past the end reads the end's next activation.
2 `shown_ms`.
3 Claim not checked.
4 L914-915 header at length 2000; L916 hidden at length 9000: raw.
5 Inputs.
6 Typed.
7 Not read. 8 L914 (reasoned: clamp ignored). 9 None.
10 Same clamp as 16, 28, 39.
11 Yes.
Verdict: Remove. Same clamp as test 16.
Today it catches: the box not calling `shown_ms`.

## 49. next activation box: the note names follow the Pro Drums setting (:919)

Read: R0; lines 919-927; D48 Q11 (`2026-10-04-phase-3-5-questions.md:60-62`): "use the Dynamics wording everywhere ('Green tom', 'Yellow cymbal', '2x kick', plain 'Yellow' with Pro Drums off)". D48: "all recommended".
1 Claim: the box's note name is "Red" with Pro Drums off and "Red snare" with it on.
2 Owner: `Chord::rowstr(pro)` (core; the box calls it at `preview_view.cpp:811`). Display only prints it.
3 Claim not checked: "Red snare" is not among Q11's examples, and the Dynamics tab's words were not opened.
4 L923-924 `"at m2.1.0 · [Red]"`, L925-926 `"[Red snare]"`: raw results of the owner's function typed by hand.
5 D48 Q11 names the wording policy but not this pair.
6 Typed; the test could compare with `rowstr` of the same chord (the owner).
7 Failure: the Paths tab said "GreenTom" while Dynamics said "Green tom" (Q11). Possible again.
8 L924 (reasoned: pro flag ignored); L926 (reasoned: label swapped).
9 None.
10 Test 47 L905 and test 51 L963 type the same `[Red snare]`. `rowstr` probably has its own tests; not searched.
11 Yes.
Verdict: Rewrite. Compare the box with `chord.rowstr(pro)` for both settings, not typed labels.
Today it catches: the pro flag reaching the label.

## 50. next activation box: on an activation within half a millisecond, and no chord (:929)

Read: R0; lines 929-947; `kOnActivationMs` (`preview_view.cpp:780`).
1 Claim: the box counts the playhead as on an activation up to 0.5 ms past it; an activation with no chord names only its measure.
2 `kOnActivationMs = 0.5` is typed in the display (`preview_view.cpp:780`), used by `activation_jump_ms` and the box. `struck_at` (exact at the moment) is a different tolerance for the same idea. 
3 Claim not checked.
4 L933 -100 -> 1; L934 `2000.5` -> 1; L935 `std::nextafter(2000.5, 1e300)` -> 2; L937 `8000.5` -> 2; L938 hidden just past 8000.5; L945 `"Next: activation 1 of 1"`; L946 `"at m2.1.0"`: edges at the exact tolerance; raw.
5 0.5 ms: no cited reason. The source comment says only "How far from an activation the playhead may sit and still count as on it". No D-number, no rule page.
6 Typed constants.
7 Not read. Named: the box skipping an activation the playhead sits on.
8 L934-935 (reasoned: `<` vs `<=`; the tolerance moved by one ulp); L945-946 (chord missing).
9 `nextafter` pins the tolerance to the last bit of a number nobody chose.
10 Test 46 L891 uses the same 0.5. Test 14 states `struck_at` exactly.
11 Playhead within 0.5 ms of an activation happens after a jump; yes.
Verdict: Investigate. The 0.5 ms tolerance has no reason I can find, and two parts of Hydra answer "is the playhead on this chord" differently (`struck_at` exact, `activation_jump_ms` and the box with 0.5 ms).
Split: `struck_at(now, t)` (exact) against `kOnActivationMs` (0.5 ms) for the same activation chord.
Today it catches: tolerance edges, no-chord text.
Side notes: name holds a comma (:929).

## 51. next activation box: the notation line is the activation's own, header and detail unchanged (:949)

Read: R0; lines 949-983; `notationstr_verbose` and `e_offset`/`is_e0` known only from `records-engine/index.md` line 26 (`model.h` 316-323 not opened).
1 Claim: the box shows the activation's own notation line, with "+" and the timing for a squeeze-in, and "E0 (0.0 ms)" for an e-critical one, and the notation is empty past the last activation.
2 Owner: `Activation::notationstr_verbose()` (core); display copies it at `preview_view.cpp:452`. 
3 Claim not checked: the engine's replay record does not carry `e_offset` or the E-critical flag (`records-engine/index.md`, the "early fill" paragraph). Carry out: `is_e0`, `src/core/model.h:316-323`.
4 L959 scene notation equals `act.notationstr_verbose()`; L961 box equals the same: owner's answer. L970 `"0"`, L974 `"0+ (100.0 ms)"`, L979 `"E0 (0.0 ms)"`: raw typed results of the owner's function. L980 `e.notation == t.path.activations[0].notationstr_verbose()`: owner's answer. L962-963, L981-982 header/detail: raw. L965 empty past last: property.
5 `a.e_offset = 300.0` "not e-critical" (L953): 300 against the 60 ms slack (`kEarlyFillSlackMs`, `model.h:136` per the index) has no reason shown. The squeeze-in 100.0 ms: input. D111 (the user's notation option B) is cited by nobody here.
6 L959-961 and L980 read the owner (good); L970-979 type.
7 Not read (17c3a358 "the next-activation box shows the activation's notation", subject only). Named: the box's notation differing from the Paths tab's.
8 L959/L961 (reasoned: notation not copied); L970-979 duplicate the owner's tests (not searched); L965 (stale notation).
9 TwoActs' first activation reads "E0" from a default `e_offset` the fixture does not set: an accident of the default.
10 The notation's own text rule is probably tested elsewhere (not searched).
11 Yes.
Verdict: Rewrite. Keep L959, L961, L965, L980; drop the typed notation strings.
Today it catches: notation copied through scene and box (kept).
Side notes: name holds a comma (:949).

## 52. build_time_box: every lookup at its boundaries on a chart of many changes (:1027)

Read: R0; lines 985-1085; D48 Q23 (`2026-10-04-phase-3-5-questions.md:112-114`): "all four switch together".
1 Claim: on a chart with four tempos, three signatures and four sections, the time box reads the right tempo, measure, signature and section at each change and half a tick before it, with a length clamp and a negative length.
2 Display of core timing, as 1.
3 Claim not checked; the chart is hand-built. A corpus chart with tempo, signature and section changes is not named.
4 L1030-1032 REQUIRE counts 4, 3, 4: guards. L1046-1071 nineteen typed strings: raw results with the working in a 10-line comment (L1035-1044). L1078-1081 two more typed strings. Each line = one boundary (e.g. L1060: 3332.7 ms -> "m2.3.0 ... Section Verse").
5 Every boundary and every typed time: the comment's own arithmetic; D48 Q23 says only that the four switch together.
6 Typed.
7 Not read (D48 Q23 fixed the BPM line lagging, L1087 test).
8 Each row (reasoned): a lookup off by a tick fails the row at that ms. The rows overlap in pairs (e.g. L1053-1054 identical).
9 The test leans on the fixture's tick sums; a failure prints the whole table, not one chord (check_lines).
10 Tests 1, 4, 5, 6, 7 each pin one lookup; test 53 repeats rows L1057-1058.
11 Yes.
Verdict: Rewrite. The row for each change comes from the song's tempo map and sections as named inputs (change tick, a tick before it), not 19 literals; keep one real chart.
Today it catches: every lookup at each boundary.

## 53. build_time_box: inside the half tick before a tempo change every line shows the new values (:1087)

Read: R0; lines 1087-1097.
1 Claim: half a tick before a tempo change the BPM line reads the new tempo like the measure line (D48 Q23).
2 As 52.
3 Claim not checked.
4 L1095 `"m2.1.0"`, L1096 `"BPM 90.000 · 4/4"`: raw, with the working in the comment.
5 D48 Q23: "Recommended: all four switch together" (the user answered "all recommended").
6 Typed.
7 Failure: the BPM line lagging by half a tick (Q23). Possible.
8 L1095, L1096 (reasoned: tempo looked up at the raw tick).
9 None.
10 Rows L1057-1058 of test 52 (`std::nextafter(2000.0, -1e300)` -> "m2.1.0 ... BPM 90.000") already state it.
11 Yes.
Verdict: Remove. Test 52's rows at L1057-1058 make the same promise at the same input.
Today it catches: nothing test 52 misses (only the box's tempo and measure fields, both in the row).

---
End of records: 53 of 53 test cases (lines 115-1097).

