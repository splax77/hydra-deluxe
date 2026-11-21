# Delta DL5b: the full records DL5 wrote short

Auditor: T5-DL5b, agent a08fec8d32dafeb73. Read-only; nothing run, nothing built. Lines are as at `5ec682c7` (the files are unchanged on this branch since). Each record below replaces the short record named in its delta line in `records/delta-DL5.md`.

## What every record here shares

Plain version first. Main made the early-fill window a setting (default 60 ms, up to 85 ms). Three helpers in the tests then pin the window at 85 ms so their typed figures stay what they were: `analyzed_like_the_gui` in `tests/test_activation_rows.cpp:1221`, `digest_settings` in `tests/song_digest.h:327`, and the compare fixture in `tools/test_compare_db.py`. This file judges the tests that stand on those helpers, one record each.

Shared reading, for the nine activation-row and docs records:

- The window: `kDefaultEarlyFillWindowMs = 60.0` (`src/core/model.h:112`), `kMaxEarlyFillWindowMs = kHitWindowMs` = 85 (`model.h:115`), `Settings::early_fill_window_ms` starts at the default (`src/app/config.h:195`). The filed rulings that name 60 and 85 are the `## Confirmed` lines at `docs/agents/rulings/code-and-engine.md` 56 and 57 ("make the early fill threshold an adjustable setting pinned to one number in both directions", "analysis settings panel, default 60ms, supports 0ms to 85ms").
- The helper: `tests/test_activation_rows.cpp:1215-1236`, and `tests/scratch_settings.h:18-24` (every field the app's default but the chart folder and depth 2). The helper's change at `b1b0ed0f` is one added line, `settings.early_fill_window_ms = kHitWindowMs`. `burnout()` (`tests/song_details_fixtures.h:24-33`) did not change: it analyses with `scratch_settings().to_analysis_settings()` alone, so Burnout runs at the default 60 ms while Tapped Out, Focus, Epidermis and Get Ur Freak On run at 85 ms. The helper's own comment ("analyzed the way burnout() analyzes it") is no longer true. This is a split inside one test file, see "Splits".
- The screen code: `src/app/path_view.cpp:205-245` (`squeeze_window_view`, `early_fill_window_view`), `:246-362` (`build_activations`, the `if (chords)` block at 355-361), `:625-645` (`PathsTabCache::details`); `src/core/note_window.cpp:67-145` (`squeeze_safe_ms`, `early_fill_safe_ms`, `safe_is_short`, `squeeze_presses`, `squeeze_window_ends`, `early_fill_window_end`); `src/ui/paths_tab.cpp:716-720`; `src/ui/app_state.cpp:480-517`.
- Callers in `src/` and `tools/` (grep for each name over both folders): `squeeze_window_ends` is called at `path_view.cpp:227` only; `early_fill_window_end` at `path_view.cpp:238` only; `build_activations` at `path_view.cpp:637` only, which `PathsTabCache::details` wraps and `paths_tab.cpp:716` alone calls. `tools/replay_schedule.cpp` calls none of the three; it answers "does the squeeze fit" its own way (`feasible`, `kRoomMarginMs = 3.0`, lines 55-57 and 189-191).
- What the engine and the screen say on the seven charts: `records-engine/index.md` ("An early fill (E0)" is Beg's activation 3, `e_offset` 0, `sp_ready_ms` 199079.9; "A squeeze-out" is Believer path 4 and Beg activation 0); `records-engine/beg.schedule.json:38,44,55` (`anchor_room_ms` 73.85, `squeeze_room_ms` 73.85, `feasible` true); `records-screen/beg.txt` (opened: activation 1 "squeeze out 0.0 ms", "frontend 84.4 ms [K  B ]", "backend 84.4 ms [K Y  ]", "168.7 ms safe"; activation 4 "E0 ... early fill 0.0 ms", "Early fill: 0.0 ms (required)", "Dynamic Hit Window: 84.4 ms [K   G]", "84.4 ms safe"). The engine records were made at the 85 ms code (`index.md:5`), so they say nothing about the window at 60. Tapped Out, Focus, Epidermis, Get Ur Freak On and Burnout are not among the seven charts. For them: claim not checked; the capture needed is `hydra_replay dump` and `schedule` for each chart at `--efill 60` and at `--efill 85`, plus a Paths tab capture at each window.
- The user's words for the row. The only `## Confirmed` line about it is `docs/agents/rulings/ui-and-display.md:14`: "change Clone Hero to Dynamic Hit Window and mock it up." The lines the tests' comments call "the user's ruling" ("No word", "xx ms safe >=0 ms, xx ms short <0ms", "the frontend and backend are wrong, they are switched", the 2026-11-12 "every badge gets a row") are at `ui-and-display.md:105-111`, under `## Unconfirmed` (header at line 102). They are not filed rulings.
- Old records: `records/A3.md` records 14 to 21 (read in full, lines 235-367 and the shared block 1-13).
- Adding commits, from A3: `d36f0bf9` (first Dynamic Hit Window tests), `eeac4a80`, `e23a63df`, `62fe7b2b`; none re-opened for their diffs.

---

## test_activation_rows.cpp

### 1. dynamic hit window: Tapped Out's squeeze out is out of reach (tests/test_activation_rows.cpp:1308)

Delta round: changed, replaces `delta-DL5.md` record 9 and `records/A3.md` record 14. The old verdict (Investigate, the split on "out of reach") still holds. The only line that changed under it is the helper's added window line.

Read: the shared list; test lines 1308-1329; helpers `squeeze_row` and `only_squeeze` (1246-1263), `window_rows_of` (1239-1243). Adding commit not re-read (A3: `d36f0bf9`).
1 Claim: On Tapped Out's best path, the fourth activation's squeeze out reads "frontend 77.9 ms [K G], backend 72.0 ms [ R B ]" and "65.9 ms short", meaning the two hit windows cannot cover the squeeze.
2 Owner: `squeeze_window_ends` (`note_window.cpp:113`) and `squeeze_safe_ms` (`:67`, sum below zero is short, `safe_is_short` :73). Called at `path_view.cpp:227` only. So only the Paths tab works out "does the squeeze fit the hit windows". The engine answers the same question in `tools/replay_schedule.cpp:189-191` with another formula (rooms against a 3 ms margin). Two answers, one question.
3 Real chart: claim not checked. Tapped Out has no engine record or screen capture in the seven. On Beg activation 1, a squeeze out that fits: Engine `beg.schedule.json:38-55`, `anchor_room_ms` 73.85, `squeeze_room_ms` 73.85, `feasible` true. Claim shape: a fitting squeeze reads "safe". Screen `beg.txt`: "168.7 ms safe". Both say it fits; the numbers differ (84.4 against 73.85), because they are two formulas. No chart in the seven holds a squeeze that does not fit, so "short" has never been compared with the engine's `feasible: false`. Needed: Tapped Out's `schedule` at `--efill 85` and its Paths tab capture.
4 Direct: 1318 a window string with three chords and two figures, a raw result. 1319 "65.9 ms short", a raw result whose working (anchor side times ratio plus squeeze side minus needed) is not written. 1320 `is_short` follows the sum's sign (a property of the sign rule). 1322-1328 the same figures split out, so the strings are checked twice.
5 Reason: 77.9, 72.0 and 65.9 come from Tapped Out's note gaps through the window formula (`note_window.cpp:9-25` comment), so their only source is the code under test. The tag edge ("short" below zero) is the Unconfirmed line `ui-and-display.md:107`, not a filed ruling. The test also depends on the best path being the one the search picked at 85 ms: no `pathstring()` guard sits here, so a different best path fails the test at `acts[3]` or at a figure, not at a stated cause.
6 Display: fails. Every expected side is typed. The record holds no safe or short figure to read, so the display decides "short" alone (the split).
7 Failure: from A3, the first Dynamic Hit Window tests on five real activations (`d36f0bf9`): the row wrong on real charts. Still possible. The window line adds one: the best path changing when the default window moves, which the helper now prevents.
8 Deleted: 1318: a window figure or a chord letter changed (reasoned). 1319: the tag word or sum changed (reasoned). 1320: `is_short` stuck (reasoned). 1322-1328: the colour split's word, figure or chord changed (reasoned); each repeats 1318.
9 Stand-in: None beyond the typed strings standing for the rule.
10 Same rule: tests at 1331 (Burnout, which now runs at 60 ms), 1354, 1469 and the corpus test at 1607 (tag word follows the sum's sign on every corpus chart).
11 Real: yes, Tapped Out is a corpus chart; whether activation 4 keeps its squeeze out at the default 60 ms is not known.
Verdict: Investigate. The word "out of reach" is a game fact with two owners that answer in different units (screen sum against engine `feasible`). That goes to the user before any rewrite. A rewrite after the answer would read the two ends and the sum from the engine's record and drop the typed figures.
Production finding: `squeeze_window_ends`, `squeeze_safe_ms`, `early_fill_safe_ms`, `safe_is_short` work out a game fact and only the Paths tab calls them (kind A, as A3's finding). Not new.
Split: "does the squeeze fit" (see Splits, A).
Side note: the window pin is stated in the helper (line 1219) but this test says nothing about it.

### 2. dynamic hit window: a squeeze in with a timing names its anchor the frontend (tests/test_activation_rows.cpp:1354)

Delta round: changed, replaces `delta-DL5.md` record 10 and `records/A3.md` record 16. The old verdict (Remove, covered by the corpus test) still holds.

Read: the shared list; test lines 1354-1394; the corpus test body 1607-1700 for its role checks (the lines A3 cites, 1665-1674 at the old numbering, re-found by grep of `squeeze_windows` at 1659-1663); `squeeze_presses` and `role_word` (`note_window.cpp:79-99`).
1 Claim: Whichever way a squeeze-in sentence says to press, its row names the anchor (the activation's chord) the frontend and the squeezed note the backend, the anchor first; on Focus activation 2 the figures read 61.9 and 48.1 ms with "85.1 ms safe", on Epidermis activation 2 48.1 and 67.8 ms with "184.1 ms short".
2 Owner: `squeeze_window_ends` (`note_window.cpp:124-126` fixes the order frontend then backend; the ticks come from `Activation::squeeze_anchor_tick` and `squeeze_chord_tick`). Called at `path_view.cpp:227` only. The role words are display words, the ticks are the engine's.
3 Real chart: Engine on Beg activation 1 (a squeeze out): the anchor chord is the activation chord and the squeezed chord is the later one (`beg.schedule.json`, A3 cites 5, 22, 40). Claim: frontend is the activation chord, backend the squeezed chord. Screen `beg.txt`: "frontend 84.4 ms [K  B ]" (the activation chord [Kick - Blue cymbal]) and "backend 84.4 ms [K Y  ]" (the squeezed chord). They agree for a squeeze out. For a squeeze in (this test's case) the seven have only Pitch Dark's squeeze-in rows (A3 cites `pitch_dark.txt:43`); the backend half was not read. Claim not checked for squeeze ins; Focus and Epidermis are not in the seven.
4 Direct: 1366-1368 window text, tag and `is_short`, raw results. 1380-1387 two figures with word and chord, raw. 1390 the sentence holds " ms early" (the claim that the note is pressed early). 1391-1392 "184.1 ms short" and `is_short`.
5 Reason: the role order is the Unconfirmed line `ui-and-display.md:104,109` ("the frontend and backend are wrong, they are switched"). The ms have no working. `REQUIRE(best.pathstring() == "0 3+ 0 0")` at 1358 is a pinned search result, valid at the helper's 85 ms.
6 Display: typed; the roles could be read from the engine's anchor and chord ticks, and the corpus test already does.
7 Failure: from A3, Satellites (Periphery) showed the two switched (`eeac4a80`). The roles switching is still possible; the corpus test would catch it.
8 Deleted: 1366-1368 and 1380-1387: roles or figures changed (reasoned). 1358: the search picking another path, which says nothing about the row. 1390: the sentence losing "early". 1391-1392: the tag or sign.
9 Stand-in: the two figure sets stand in for "the roles are right"; the pathstring stands in for "this is the chart's case".
10 Same rule: the corpus test at 1607 checks the roles and ticks on every corpus row against `squeeze_anchor_tick` and `squeeze_chord_tick` (lines 1659-1674). That is the one verdict.
11 Real: yes.
Verdict: Remove. The corpus test already makes the roles promise on every chart, against the engine's ticks. The typed ms and tags go with record 1's decision.
Today it catches: the typed figures and tags of two real activations (dropped on purpose); the roles (kept by the corpus test).
Production finding: none new.
Side note: the corpus test at 1607 analyses at `Settings().to_analysis_settings()`, the default 60 ms, while this test runs at 85. A role bug that appears only at one window shows in one place only.

### 3. dynamic hit window: an early fill gets one window, required or optional (tests/test_activation_rows.cpp:1396)

Delta round: changed, replaces `delta-DL5.md` record 11 and `records/A3.md` record 17. The old verdict (Rewrite) still holds, with a new reason: the early fill is this test's subject and the window it runs under is the subject's own setting.

Read: the shared list; test lines 1396-1437; `early_fill_window_end` (`note_window.cpp:132-140`), `early_fill_window_view` (`path_view.cpp:233-239`); `Activation::badge_timings`, `is_E0` (model.h:705, not opened); the rulings above.
1 Claim: On Tapped Out's best path, the third activation (a required early fill, E0) shows "Early fill: 0.0 ms (required)" with one window row "82.9 ms [K   G]" tagged "82.9 ms safe", no word on that single note; the fourth, which skipped a fill, shows "(optional)" with "85.7 ms [K  B ]"; and each line's warning border follows its badge's colour.
2 Owner: `early_fill_window_end` (`note_window.cpp:132`), called at `path_view.cpp:238` only; the badge's text and colour come from `badge_timings`. The E0 status itself is the engine's (`is_e0(e_offset, skips, window_ms)`, `model.h:335`), stamped per activation (`e_window_ms`, `model.h:606`). So the very answer the test checks, "is activation 3 required", depends on the window the helper pins.
3 Real chart: Engine, Beg activation 3: E0 at `e_offset` 0, `sp_ready_ms` 199079.9 (`records-engine/index.md`, "An early fill (E0)"). Claim: "Early fill: 0.0 ms (required)" with one window row. Screen `beg.txt`: "Early fill: 0.0 ms (required)", "Dynamic Hit Window: 84.4 ms [K   G]", "84.4 ms safe". Agree on the text shape for a required fill at 0.0 ms, which fits inside any window from 0 to 85. Different chart from Tapped Out, and the optional case (activation 4) is in none of the seven: claim not checked for it and for 82.9 and 85.7. Whether Tapped Out has an E0 at all at the default 60 ms is not known.
4 Direct: 1405, 1409-1415 typed strings and a figure, raw results; 1413 `word.empty()` (the "No word" line, Unconfirmed). 1423-1424 the row's `early_fill_warn` against the same `badge_timings` loop the display used. 1432-1436 typed again for the optional fill.
5 Reason: 82.9 and 85.7 come from the chart's note gaps through the formula; no working is written. "No word" and "every badge gets a row" are Unconfirmed lines (`ui-and-display.md:105,111`). The comment at 1429 says the figures are "hydra_replay schedule's", which is a claim: I did not run it.
6 Display: fails. Expected values are typed. The record's `sp_ready_ms` for the SP-ready chord could supply the end.
7 Failure: the row dropped on an optional fill (the card quote at `docs/sessions/2026-11-13-375e58cd.md:29`, not a filed ruling). Still possible. The window change adds a second: the E mark appearing or vanishing when the window moves.
8 Deleted: 1405, 1432: the line's text or required/optional word changed (reasoned). 1409-1415: figure, tag or word changed (reasoned). 1423: by construction. `early_fill_warn` is filled at `path_view.cpp` from the same `badge_timings` loop the test re-runs, so it can fail only if the loop or the copy is removed (reasoned). 1424: a warn on an empty line. 1433-1436: the optional fill losing its row.
9 Stand-in: 1423 compares a field with the data it was copied from.
10 Same rule: the test at 1469 (subcases 2 and 3) and the corpus test at 1607 (fill rows).
11 Real: yes for required fills (Beg). The optional fill at activation 4 stands for a real skipped-fill case; Beg and Pitch Dark have skipped fills (`index.md`, "A skipped fill") but I did not check one shows an optional E.
Verdict: Rewrite. Take the row's end from the engine's SP-ready chord for Beg activation 3, drop 1423, add an optional case when a chart is named, and run the check at the default window or say why only 85 is meant.
Today it catches: the row shown on required and optional fills (kept), the warn border (dropped; by construction).
Production finding: none.

### 4. dynamic hit window: a squeeze in that needs no timing still gets its row (tests/test_activation_rows.cpp:1439)

Delta round: changed, replaces `delta-DL5.md` record 12 and `records/A3.md` record 18. The old verdict (Rewrite) still holds.

Read: the shared list; test lines 1439-1465; `squeeze_presses` (`note_window.cpp:79-84`); `SPSqueeze::needs_no_timing` (not opened; the test requires it at 1456).
1 Claim: On Get Ur Freak On's best path, activation 2 holds a squeeze in at -84.27 ms that needs no timing; it still gets a row, the anchor is the frontend (73.8 ms) and the squeezed note the backend (53.6 ms), "211.6 ms safe".
2 Owner: `squeeze_window_ends` (`note_window.cpp:128`, the anchor counted on the scale's early side via `squeeze_presses`); called at `path_view.cpp:227` only.
3 Real chart: Engine `late_sqin_twice`, activation 1, first SqIn `offset_ms` -90.0, `needs_no_timing` true (`records-engine/late_sqin_twice.schedule.json:96-97`, per A3). Claim: a no-timing squeeze in has a row. Screen `records-screen/late_sqin_twice.txt:36-43`: "squeeze in -90.0 ms", the sentence, then "Dynamic Hit Window: frontend 85.7 ms [  Y  ]". They agree that the row exists. Get Ur Freak On's own figures (73.8, 53.6, 211.6) are not in the seven: claim not checked.
4 Direct: 1451 a pinned pathstring `"5- 2+ E2- E0 E0"`, a snapshot of one search result at 85 ms. 1456 `needs_no_timing()` (the claim's premise). 1459-1463 typed figures and tag, raw results.
5 Reason: the figures have no working; the comment names `hydra_replay schedule` as their source, which I did not run. The pathstring holds three E marks that exist only because Star Power comes ready inside the window before a fill (comment 1448-1450, `is_e0` owns the window); at the default 60 ms they may not appear, so this test's input exists only at a window the app does not start at.
6 Display: fails; typed expected side.
7 Failure: a no-timing squeeze left with no row (`ui-and-display.md:110`, Unconfirmed). Still possible.
8 Deleted: 1451: the search picking another path (a stand-in guard). 1456: the case not being the one meant. 1459-1463: a figure, a tag or the roles changed.
9 Stand-in: 1451 stands for "this is the chart's case".
10 Same rule: the test at 1469 subcase 1 and the corpus test at 1607 (shown rows equal badges).
11 Real: yes (`late_sqin_twice` and Get Ur Freak On).
Verdict: Rewrite. Use `late_sqin_twice` activation 1: for each SqIn in the record whose `needs_no_timing` is true the row exists and its two ends are the record's anchor and squeezed chord. Drop 1451 and the typed figures.
Today it catches: a no-timing squeeze losing its row (kept).
Production finding: none.
Split: this test and the corpus test run at two windows (see Splits, B).

### 5. dynamic hit window: a row for every timing badge, edited figures too (tests/test_activation_rows.cpp:1469)

Delta round: changed, replaces `delta-DL5.md` record 13 and `records/A3.md` record 19. The old verdict (Rewrite) still holds.

Read: the shared list; test lines 1469-1541; `squeeze_window_ends` and `early_fill_window_end`; `test::set_skips` (not opened).
1 Claim: Every timing badge gets a Dynamic Hit Window row whichever side of zero its figure sits: a squeeze in edited to 0.0 and to -0.1 ms on Focus, a required early fill with 20 ms to spare on Tapped Out ("Early fill: -20.0 ms (required)", 82.9 ms end, "102.9 ms safe"), and that fill made optional.
2 Owner: as records 1 and 3: `path_view.cpp:227,238` only.
3 Real chart: Engine, Beg activation 3, E0 at 0.0 ms (`records-engine/index.md`). Screen `beg.txt`: a row at 0.0 ms. They agree for that one. The in-test edits (-0.1 ms, 20 ms to spare, an optional fill) are not on any seven-chart screen: claim not checked.
4 Direct: 1490-1491 the same typed window text for both offsets; 1514 "-20.0 ms (required)"; 1519-1521 "82.9 ms [K   G]" and "102.9 ms safe" (the comment says 82.9 plus 20; the line types the sum); 1536-1539.
5 Reason: 0.0 and -0.1 straddle `timing_needs_hitting` (model.h:332; D302 in the decision log, "0.0 ms is not the same thing as no timing", per A3). 20.0 is the test's own input. 102.9 is a raw sum.
6 Display: typed.
7 Failure: rows dropped for timings that need none (`ui-and-display.md:110-111`, Unconfirmed; card `375e58cd.md:29`). Still possible.
8 Deleted: 1496, 1512, 1532 (`REQUIRE` that the owner has numbers): the row built when the owner has none. 1500: the row differing by offset. 1511: the edit not making an E0. 1514: sign flipped. 1519-1520: figure or sum. 1536-1539: optional text or figures.
9 Stand-in: None.
10 Same rule: the corpus test at 1607.
11 Real: the edits are mutations of real activations; a spare of 20 ms and an optional fill are reachable.
Verdict: Rewrite. Write 102.9 as the end plus the spare, and take the 0.0 and required cases from Beg's record.
Today it catches: a row on each badge kind and each sign (kept).
Production finding: none.

### 6. dynamic hit window: no chord times, no row anywhere (tests/test_activation_rows.cpp:1543)

Delta round: changed, replaces `delta-DL5.md` record 14 and `records/A3.md` record 20. The old verdict was Investigate with the question "does any screen call with no chord times?". This record answers that question from the code, so the verdict becomes Remove.

Read: test lines 1543-1554; `build_activations` and its null guard (`path_view.cpp:246-362`, guard at 355-361), `path_view.h:153-161`; `PathsTabCache::details` (`path_view.cpp:625-645`); `paths_tab.cpp:716-720`; `app_state.cpp:480-517`; every `viewed =` and `.chords` in `src/ui` and `src/app` (grep, listed above).
1 Claim: When no chord times are passed in, no activation shows a Dynamic Hit Window row, and each row list is as long as its sentence list, on Tapped Out, Burnout, Focus and Epidermis.
2 Owner: `build_activations`' `if (chords)` block (`path_view.cpp:355-361`). Callers: `PathsTabCache::details` (`path_view.cpp:637`), called by `paths_tab.cpp:716`, which passes `app.viewed.chords ? &*app.viewed.chords : nullptr`. `ViewedSong::chords` is set at `app_state.cpp:512`, in the same block that sets `v.record` (514) and `v.timing` (511); `viewed` is replaced whole (`viewed = std::move(v)`, 517; other writes at 370, 384, 423, 436 reset it to an empty `ViewedSong{}`). So no screen holds a record with no chord times. The null branch has no caller but this test and the default argument (`path_view.h:161`).
3 Real chart: not a chart claim. The real thing is the app's own path: a song becomes Ready only with chords and record together. Checked in `src/` as above; no capture can show the null case because the app never builds it.
4 Direct: 1548 row list size equals sentence list size (a property); 1550 no row; 1551 no early-fill row.
5 Reason: no numbers.
6 Display: n/a.
7 Failure: a row drawn from a missing source. From A3 the adding commit is `d36f0bf9`. No screen can reach it today.
8 Deleted: 1548: the `resize` removed (reasoned). 1550-1551: the `if (chords)` guard removed (reasoned). With the guard removed the build would dereference a null pointer, so the test would catch a crash, but only for a call no screen makes.
9 Stand-in: the four charts stand for "any chart"; the loop includes `burnout()` at 60 ms beside three charts at 85 ms.
10 Same rule: none; the with-chords side is the corpus test at 1607 (line 1659 requires `squeeze_windows.size() == sqinouts.size()`).
11 Real: no. The state cannot happen in the app (see 2).
Verdict: Remove. The failure it guards, rows built with no chord times, is not reachable from any screen. One thing goes with it: the size-equality line at 1548 for the null case; the same equality for the real case is at 1659.
Production finding: the null branch and the default argument `chords = nullptr` (`path_view.h:161,324`) serve no caller; that is a decision for the main session, not for this record.

### 7. dynamic hit window: the row sits alongside and changes no other field (tests/test_activation_rows.cpp:1556)

Delta round: changed, replaces `delta-DL5.md` record 15 and `records/A3.md` record 21. The old verdict (Rewrite: drop the floor, name the charts) still holds. DL5 wrote "same four charts at 85"; that was wrong for Burnout, which runs at 60 ms.

Read: the shared list; test lines 1556-1577; `check_same_but_windows` (1266-1300).
1 Claim: Building the rows with chord times changes no other field of the activations view (summary, timeline, number, notation, badges, backends and the rest), on Tapped Out, Burnout, Focus and Epidermis, and the comparison covers at least five rows that really show.
2 Owner: `build_activations` (`path_view.cpp:246`): the window rows are written last into two fields (`squeeze_windows`, `early_fill_window`, 355-361). Callers as record 6; the app always passes chord times.
3 Real chart: claim not checked on the seven; the comparison is between a real build and a build with null chord times, which the app never makes (record 6). On a real chart the screen's other fields are checked by the tests that name them.
4 Direct: 1563-1565 and 1568 (`check_same_but_windows`): a property over every field. 1566 equal sizes: a property. 1576 `rows_shown >= 5`: a floor whose reason sits in the comment (the five rows the cases above pin).
5 Reason: 5 stands for "the cases above"; no working. Zero is not used.
6 Display: n/a; the expected side is the display's own other build.
7 Failure: a change that makes the row build edit another field. Possible only if someone moves other-field code inside the `if (chords)` block.
8 Deleted: 1563-1568: by construction, the rows are written by the same function into separate fields, so only a rewrite of the function can fail it (reasoned). 1576: a floor; it fails when the charts or the window change, not when the row build breaks.
9 Stand-in: 1576 stands for "the comparison covered rows that show"; the charts stand for "the screen". Two windows in one loop: Tapped Out, Focus and Epidermis at 85 ms, Burnout at 60 ms.
10 Same rule: the corpus test at 1607 reads every row field against the owner.
11 Real: the baseline is a state the app never reaches (record 6); the with-chords state is real.
Verdict: Rewrite. Drop the floor and the null-chords baseline. Compare the other fields against the engine's own data for each named activation, and name a chart that must show a row (Beg activation 1).
Today it catches: the row build editing another field (kept in spirit; the baseline is unreachable).
Production finding: none.
Split: the loop's two windows (see Splits, B).

---

## test_docs_match_code.cpp

### 8. docs match code: the marker check catches a wrong number and a wrong word (tests/test_docs_match_code.cpp:486)

Delta round: changed, replaces `delta-DL5.md` record 17 and `records/A11.md` record 3 (there headed at line 482). Its own lines are unchanged since the old record; `known_defaults()` beneath it gained `Settings::early_fill_window_ms` and `kDefaultEarlyFillWindowMs` in `318373f3`. The old verdict (Rewrite) still holds. DL5 wrote that it could not find the old record; it is A11 record 3.

Read: test lines 486-504; `check_markers` (357-405) and `known_defaults` (319-353); `Settings::mslimit_value`/`mslimit_enabled` (`config.h:164-165`), `Rules::backend_leeway_ms` (`rules.h:32`), `kStarMultipliers` (`src/core/stars.h:25-26`: 0.1, 0.5, 1.0, 2.0, 2.8, 3.6, 4.4); A11 record 3 (lines 57-72); the marker lines for the two new names (`docs/UserGuide.md:135`, `CONTEXT.md:613`, opened by grep with context).
1 Claim: The marker check reports a doc number that differs from the code, a flag word that differs, an unknown marker name, and a float typed as a decimal that differs, and lets a correct one pass.
2 Owner: `check_markers`, a helper in this file. Callers in `src/`: none (a test file helper). Test callers: this test and the test at 472. Nothing a person reads calls it.
3 Real chart: not a chart claim. The real thing is a real doc line. The input is six hand-built lines; no real marked line (`UserGuide.md:135` reads "60 ms<!-- default: Settings::early_fill_window_ms -->") is fed in with a changed number. Claim not checked on real input.
4 Direct: 498 exactly four problems (a count). 499 `"the doc says 11, the code says 10"` types the default of `mslimit_value` (a copy of production's 10). 500-501 `"the code starts on"` types that `mslimit_enabled` defaults to on. 502 `unknown default marker` text. 503 `"the doc says 0.5, the code says 0.1"` types `kStarMultipliers[1]`. The passing lines (3 ms against 3.0; 2.8 against star 5) show only through the count at 498.
5 Reason: 11 is "one more than 10" and 0.5 is "not 0.1"; neither says why. A default moving from 10 to 11 turns this test red while the check works. No D-number or ruling is cited.
6 Display: n/a. The docs are the display of the defaults, but this test judges the helper, not a doc.
7 Failure: `e01ab097` ("a test that fails when a doc names gone code or a wrong default") names the helper letting a wrong marker through. Still possible.
8 Deleted: 498: the helper reporting extra or too few (reasoned). 499: ints compared where doubles are meant (reasoned). 500-501: flag logic inverted. 502: unknown names skipped. 503: the float-cast branch (`k.single`) lost; this is the only line that fails then, because the 2.8 case passes either way only if both agree.
9 Stand-in: the count at 498 stands for "each of the six lines did what it should"; a failure says "5 not 4" and names no line.
10 Same rule: the test at 472 runs the same helper on the real docs; no other test states it.
11 Real: the lines look like real marker lines; yes.
Verdict: Rewrite. Build each wrong line from the owner's value in the test (default plus one, the opposite flag word, a star multiplier plus a step), compare messages built from the same owner values, and assert per line. Add one case fed from a real marked line, such as `UserGuide.md:135`, with its number changed.
Today it catches: the helper no longer reporting a wrong number, a wrong flag word or an unknown name.
Production finding: none.
Side note: `known_defaults()` now holds two names, `Settings::early_fill_window_ms` and `kDefaultEarlyFillWindowMs`, for one owner (`config.h:195` starts the setting from `model.h:112`); they cannot disagree.

---

## tools/test_compare_db.py

Shared reading for records 9 and 10: `tools/test_compare_db.py` 1-315 whole; `tools/compare_db.py` 95-150, 149-320 by grep for `unique_key`, `plan_for`, `OWN_IDS`, `REFERENCES`; `src/store/record_store.cpp:176-192` (the lens column list "ms_enabled, ms_value, depth_mode, depth_value, legacy_fills, efill_window_ms", line 183) and 271-280 (the real `profiles` CREATE: id, chartmode, sp_cap, the lens columns, hyversion, rules_fp, with `UNIQUE (chartmode, sp_cap, <lens columns>, hyversion, rules_fp)`); `.github/workflows/ci.yml:467` (the test runs in CI); old record `records/D14.md` lines 225-281; the fixture change `55e81eee` (diff read). Callers of `compare_db.py` in `src/`: none; it is a hand tool.

### 9. tools/test_compare_db.py::test_profile_id_order_alone_compares_clean (:286)

Delta round: changed, replaces `delta-DL5.md` record 28 and `records/D14.md` ("test_profile_id_order_alone_compares_clean (:284)"). The old verdict (Investigate, claim not checked only) still holds. It runs twice, once with no flags and once with `--summary-only`.

Read: as the shared list; test lines 285-296; `add_records` (161-186), `RECORD_SCHEMA` (129-145), `PROFILES` (148-151).
1 Claim: Two stores that hold the same profiles and records, with the profiles numbered in the opposite order, compare as identical in `profiles`, `records` and `record_paths`, with and without `--summary-only`.
2 Owner: `compare_db.py`'s matching of a profiles row by its unique key and of `records` and `record_paths` rows by their profile's key instead of `profile_id` (`OWN_IDS` and `REFERENCES`, lines 142-146). Callers in `src/`: none; it is a hand tool also run in CI. Not a game rule. The fixture's `profiles` table must have the store's columns, and now does: its column order and `UNIQUE` key (efill_window_ms after legacy_fills, before hyversion) match `record_store.cpp:183` and 279.
3 Real chart: not a chart claim; the claim is about two store files. Real input: not used. The only input is a two-profile hand-built database whose `records` and `record_paths` hold four columns each (the file's own comment, 126-128, says so). The real input would be two stores written by `hydra_batch` over `testdata/input` (the main session runs it) with the profile ids made to differ, compared against an independent row-by-row read. Claim not checked on real input.
4 Direct: 293 exit code 0. 294-296 three exact lines, "profiles: 2 rows compared, 0 differ", "records: 3 rows compared, 0 differ", "record_paths: 3 rows compared, 0 differ"; the counts follow from the fixture's two profiles and three records.
5 Reason: the counts are the test's own inputs. The `60` in each `PROFILES` tuple is a window value the fixture needs; any value would do for this claim, and it equals `kDefaultEarlyFillWindowMs`.
6 Display: not a display test; the output text is typed, and the tool's output is the thing under test.
7 Failure: `d91501fb` ("compare_db.py matches profiles, records and record_paths across files"): the profile id differs between files while the rows are the same, and the run reports a difference. Still possible.
8 Deleted: 293: the tool comparing by `profile_id` (reasoned). 294: profiles compared by id or dropped. 295-296: records or record_paths joined on the raw `profile_id`. The new `efill_window_ms` column changes none of these: with or without the column in the fixture the lines pass, so the fixture edit is fidelity, not coverage.
9 Stand-in: the two-row schema stands for the real store. Its tables other than `profiles` are cut down; a column added to the real `records` tomorrow is compared by no test (D14's finding F1).
10 Same rule: the test at 299 (a changed path is still reported) and none else; grep `compare_db` over `tests/` and `tools/` finds only this file and the CI line.
11 Real: yes; two baselines numbered differently by worker order exist in practice.
Verdict: Investigate, claim not checked only. Needs: two stores written by the real `hydra_batch` over `testdata/input`, one run with its profiles inserted in another order, compared against an independent row-by-row read. Every other question holds for the lines.
Production finding: none.

### 10. tools/test_compare_db.py::test_one_changed_record_path_is_reported (:299)

Delta round: changed, replaces `delta-DL5.md` record 28 and `records/D14.md` ("test_one_changed_record_path_is_reported (:297)"). The old verdict (Investigate, claim not checked only) still holds.

Read: as the shared list; test lines 299-310; `add_records`.
1 Claim: When one record path's score differs between two stores that number their profiles differently, the tool reports exactly one differing `record_paths` row, naming the song and the profile's SP cap, and leaves `records` clean.
2 Owner: `compare_db.py`'s row compare and its column naming for a referred row (`REFERENCES`, "its columns named with this" prefix, line 144-146): the output writes the profile's own unique-key columns under `profile_<column>` in place of `profile_id`. Callers in `src/`: none.
3 Real chart: not a chart claim. Same as record 9: hand-built two-profile database; claim not checked on real input.
4 Direct: 306 exit code 1. 307 "record_paths: 3 rows compared, 1 differ". 308 `song_id='aaaa'` is in the output. 309 `profile_sp_cap=8` is in the output (shows the id is replaced by the profile's key). 310 "records: 3 rows compared, 0 differ".
5 Reason: 1101 against 1100 is a one-point edit the test chose; 8 is the second profile's cap in `PROFILES`.
6 Display: the output text is typed; the tool's output is the subject.
7 Failure: `d91501fb` names it: a changed row hidden by id matching, or reported without saying which profile. Still possible.
8 Deleted: 306: a changed row passing as clean. 307: the differ count wrong (two or zero). 308: the song missing from the report. 309: the report naming `profile_id` (a number that differs between files) instead of the key. 310: `records` falsely differing because ids differ. With the new column the report line also carries `profile_efill_window_ms=60`; the test does not check that.
9 Stand-in: as record 9. Substring checks (308, 309) pass on any output holding the words, not on the row they came from.
10 Same rule: record 9's test.
11 Real: yes.
Verdict: Investigate, claim not checked only. Needs the same two real stores as record 9, with one result changed.
Production finding: none.

---

## The digest tests under `digest_settings` (tests/song_digest.h:327)

Shared reading for records 11 to 20: `tests/song_digest.h` 1-30, 165-215, 285-336 (`row_hash` 173, `chart_parse_hash` 289, `loading_chart_count` 310, `digest_settings` 327); `tests/test_perf_digest.cpp` whole (1-166); `tests/test_five_lane.cpp:287-311`; `tests/test_song_parse.cpp:1311-1392`; the old records `records/C7.md` 544-615 (the eight perf-digest tests), `records/C3.md` 254-275 (five-lane) and `records/B6.md` ~1082-1100 (the crafted edge files); the filed ruling `docs/agents/rulings/code-and-engine.md:50` ("we don't need golden anymore, remove it all", under `## Confirmed`) and the next line quoted in C7; `docs/audit/2026-10-03-fix-decisions.md:426-437` (D86) was read by C7, not by me. Commits: the helper's window line was added at `e300f2fd` ("Tests key the corpus cache by the early-fill window and pin the default's counts") and renamed to `kMaxEarlyFillWindowMs` at `5ea45a09`. The test lines themselves did not change.

What the helper's change does to each test. `chart_parse_hash` calls `load_songpath_with_notes(notespath, st.reading(), st.rules, fill_rule_for(st.legacy_fills))` and hashes the song (`song_digest.h:289-304`). It never reads `early_fill_window_ms`, so the window line cannot move any parse digest, the five-lane digest or the crafted-edge digests. The four prepared-row digests run the engine (`analyze_chart_file(it.notespath, settings)`, `test_perf_digest.cpp:74`), so the window matters to them: they hash the whole engine result (`record_hash`, `row_hash` 173), and they run at 85 ms, not the app's 60. Callers of `digest_settings` in `src/`: none; callers are the three test files above and `tools/bench.cpp` (the header's own comment names it as the other user of this header).

The finding that covers all ten: each test compares a hash with a literal captured from one run. `docs/agents/rulings/code-and-engine.md:50` reads "we don't need golden anymore, remove it all". Whether a digest of the whole corpus is "golden" in the user's sense is a question for the user (C7's split). That question is not new, and the helper change neither adds to it nor removes it.

### 11. five-lane: four-lane charts parse to their pinned digests (tests/test_five_lane.cpp:287)

Delta round: changed through its helper only (`digest_settings`), replaces `delta-DL5.md` record 26 (no record of its own) and `records/C3.md` ("five-lane: four-lane charts parse to their pinned digests (tests/test_five_lane.cpp:287)"). The old verdict (Remove, naming the crafted-files test) still holds, because the window line does not reach it.

Read: test lines 285-311; the rest in the shared list. `testdata/parse_edge/expected_expert.tsv` is not opened.
1 Claim: Three four-lane charts (`m01_base.mid`, `c01_base.chart`, `c20_disco.chart`) are not changed by the five-lane conversion: their parse hash equals the one saved in `expected_expert.tsv`.
2 Owner: `digest::chart_parse_hash` (`song_digest.h:289`) over the parser; the conversion is `convert_drum_pads` in `src/parse`. Callers of the parse digest in `src/`: none (tests and `tools/bench.cpp`). The window line in `digest_settings` is never read by this path.
3 Real chart: the input is real files in `testdata/parse_edge`, but the expected side is a saved hash file, a golden file, not an independent owner. Claim not checked against an owner.
4 Direct: 308 `REQUIRE(found)` that a line exists. 309 the parse hash equals the saved hash: a snapshot from one run.
5 Reason: only the saved file; the ruling at `code-and-engine.md:50` is the reason against it.
6 Display: n/a.
7 Failure: the conversion altering four-lane charts; `6f002f3a` ("T2: five-lane drum tests and fixtures"). Still possible.
8 Deleted: 308: a missing line passing silently as zero. 309: any change to those three charts' parse (reasoned); the crafted-files test at `test_song_parse.cpp:1385` covers the same file line by line.
9 Stand-in: a hash stands for "the chords are the chords".
10 Same rule: `tests/test_song_parse.cpp:1385` reads the same expected file under the same settings (record 20).
11 Real: yes.
Verdict: Remove, naming `tests/test_song_parse.cpp:1385`. If that test is itself judged golden, both go together (C3's wording).
Today it catches: a four-lane chart changing under the five-lane branch (kept by 1385's per-row check).
Production finding: none.
Side note: nothing in the window change touches it.

### 12. the corpus's prepared-row digest is pinned (tests/test_perf_digest.cpp:128)

Delta round: changed through its helper, replaces `delta-DL5.md` record 26 and `records/C7.md` record 33. The old verdict (Investigate) still holds.

Read: test lines 126-130; `pinned_settings` (36-44), `engine_digest` (68-83), `check_engine_digest` (96-102); the shared list.
1 Claim: At Expert, Pro Drums, 2x Bass, the whole corpus's saved rows hash to 0xca0b6c9c664d9f63, and every chart that loads also analysed, under an early-fill window of 85 ms.
2 Owner: `analyze_chart_file` and `store::prepare_record` are the engine's; the hash and its literal are the test's own. The window: `digest_settings` sets `kMaxEarlyFillWindowMs` (`model.h:115`), reaching `batch_run().settings` (`config.cpp:453`) and so the search.
3 Real chart: the whole corpus hashed against itself from an earlier run; no independent owner. Claim not checked against any rule. The window at 85 is the old behaviour (the literals were not changed in `5ec682c7`'s range); no digest pins the 60 ms default.
4 Direct: 99-100 analysed equals loading: a property. 101 the hash equals a literal: a snapshot.
5 Reason: "pinned from one run" (header 6-10; comment 126 says "six digests", the file holds eight). A new chart changes the literal.
6 Display: n/a.
7 Failure: per the header and D86, a speed change that moves output. Still possible. The window line adds that the pin now says nothing about the 60 ms the app starts at.
8 Deleted: 101: any change at all (a snapshot). 99-100: a chart failing analysis; the empty `catch` at 79-80 hides it and 99-100 catches it (reasoned).
9 Stand-in: the hash for "the engine's answer did not change". The empty `catch` is a silent skip made safe by 99.
10 Same rule: records 13 to 15 and the four parse digests (16 to 19); `tests/test_replay.cpp` uses the same helper.
11 Real: yes.
Verdict: Investigate. The saved-hash form is what `code-and-engine.md:50` rejects, and whether it applies here goes to the user. Not a defect of the window line.
Today it catches: any byte-level change to the saved rows on the corpus at 85 ms, and a chart that stops analysing (99-100, which survive any rewrite).
Production finding: none.
Split: digests pinned at 85 ms while the app defaults to 60 ms (see Splits, C).

### 13. the corpus's prepared-row digest is pinned at Hard, Pro Drums off, 2x Bass off (tests/test_perf_digest.cpp:132)

Delta round: changed through its helper, replaces `delta-DL5.md` record 26 and `records/C7.md` record 34. The old verdict (Investigate) still holds.

Read: test lines 132-134 and helpers as record 12.
1 Claim: At Hard, Pro off, 2x Bass off, the corpus's saved rows hash to 0x33d8af8fe630b3b7 under the 85 ms window, and every chart that loads at Hard also analysed.
2 Owner: as record 12.
3 Real chart: as record 12: corpus against itself; claim not checked.
4 Direct: 99-100 a property; 101 a snapshot literal.
5 Reason: "from one run".
6 Display: n/a.
7 Failure: as record 12.
8 Deleted: as record 12 (101: any change; 99-100: charts failing analysis).
9 Stand-in: as record 12.
10 Same rule: records 12, 14, 15.
11 Real: yes.
Verdict: Investigate (the golden question, as record 12).
Today it catches: as record 12 at Hard.
Production finding: none.

### 14. the corpus's prepared-row digest is pinned with the CH 1.0 fill rule (tests/test_perf_digest.cpp:136)

Delta round: changed through its helper, replaces `delta-DL5.md` record 26 and `records/C7.md` record 35. The old verdict (Investigate) still holds.

Read: test lines 136-140 and helpers as record 12. The Clone Hero fill rule page was not opened: `legacy_fills` is the CH 1.0 rule, and the claim is that the digest at that setting equals a literal, not that the rule is right.
1 Claim: With `legacy_fills` on (the CH 1.0 fill rule), the corpus's saved rows hash to 0x1fdd099740952788 under the 85 ms window.
2 Owner: as record 12; `legacy_fills` reaches the parser through `fill_rule_for(st.legacy_fills)` (`song_digest.h:293`) and the search.
3 Real chart: as record 12. Claim not checked.
4 Direct: as record 12.
5 Reason: "from one run"; the window line (85) is stated by the helper's comment.
6 Display: n/a.
7 Failure: as record 12. Whether the early-fill window applies under the CH 1.0 rule at all is not read; if it does not, the pin at 85 is only the helper's.
8 Deleted: as record 12.
9 Stand-in: as record 12.
10 Same rule: records 12, 13, 15.
11 Real: yes.
Verdict: Investigate (the golden question, as record 12).
Today it catches: as record 12 under the CH 1.0 rule.
Production finding: none.

### 15. the corpus's prepared-row digest is pinned with Note Shuffle on and the ms limit off (tests/test_perf_digest.cpp:142)

Delta round: changed through its helper, replaces `delta-DL5.md` record 26 and `records/C7.md` record 36. The old verdict (Investigate) still holds.

Read: test lines 142-147 and helpers as record 12.
1 Claim: With Note Shuffle on and the ms limit off, the corpus's saved rows hash to 0x75fcc8d443856343 under the 85 ms window.
2 Owner: as record 12.
3 Real chart: as record 12. Claim not checked.
4 Direct: as record 12.
5 Reason: "from one run". `mslimit_enabled = false` is set after `pinned_settings` has set it true (line 145): the case overrides its own helper.
6 Display: n/a.
7 Failure: as record 12.
8 Deleted: as record 12.
9 Stand-in: as record 12.
10 Same rule: records 12 to 14.
11 Real: yes.
Verdict: Investigate (the golden question, as record 12).
Today it catches: as record 12 with Note Shuffle on and the limit off.
Production finding: none.

### 16. the corpus's parse digest is pinned (tests/test_perf_digest.cpp:149)

Delta round: changed through its helper, replaces `delta-DL5.md` record 26 and `records/C7.md` record 37. The old verdict (Investigate) still holds, and the window line cannot reach it.

Read: test lines 149-152; `parse_digest` (106-112); `chart_parse_hash` (`song_digest.h:289-304`).
1 Claim: At Expert, Pro Drums, 2x Bass, every chart's parsed song and dynamics count hash to 0x2d2b6ba616fa0df2.
2 Owner: `chart_parse_hash` over `load_songpath_with_notes` (`src/parse/song.cpp`). The helper's window line is never read by this path (`song_digest.h:292-293` read only `reading()`, `rules` and `legacy_fills`).
3 Real chart: the corpus against a hash of itself from a past run; no independent owner. Claim not checked.
4 Direct: 151 the hash equals a literal: a snapshot. No count of loading charts (the helper `loading_chart_count` exists for that and this test does not call it).
5 Reason: "from one run".
6 Display: n/a.
7 Failure: a parser change moving a field. With no loading count, a repin could hide charts that stopped loading.
8 Deleted: 151: any change at all.
9 Stand-in: a hash for "the parser's answer is the same".
10 Same rule: records 17 to 19 and 11 and 20.
11 Real: yes.
Verdict: Investigate (the golden question).
Today it catches: any change to the parse of any corpus chart at Expert.
Production finding: none.

### 17. the corpus's parse digest is pinned at Hard, Pro Drums off, 2x Bass off (tests/test_perf_digest.cpp:154)

Delta round: changed through its helper, replaces `delta-DL5.md` record 26 and `records/C7.md` record 38. Old verdict (Investigate) holds; the window line cannot reach it.

Read: test lines 154-157, helpers as record 16.
1 Claim: At Hard, Pro off, 2x Bass off, the corpus's parse hash is 0xdbbb09ba79b73c78.
2 Owner: as record 16.
3 Real chart: as record 16. Claim not checked.
4 Direct: 156 a snapshot literal; no loading count.
5 Reason: "from one run".
6 Display: n/a.
7 Failure: as record 16.
8 Deleted: 156: any change at all.
9 Stand-in: as record 16.
10 Same rule: records 16, 18, 19.
11 Real: yes.
Verdict: Investigate (the golden question).
Today it catches: as record 16 at Hard.
Production finding: none.

### 18. the corpus's parse digest is pinned at Medium, Pro Drums off, 2x Bass off (tests/test_perf_digest.cpp:160)

Delta round: changed through its helper, replaces `delta-DL5.md` record 26 and `records/C7.md` record 39. Old verdict (Investigate) holds; the window line cannot reach it.

Read: test lines 159-162; `check_parse_digest` (117-122); `loading_chart_count` (88-92).
1 Claim: At Medium, Pro off, 2x Bass off, 68 corpus charts load, and their parse hash is 0x1f78ad0ac3f9a90a.
2 Owner: as record 16; the loading count is `digest::loading_chart_count` (`song_digest.h:310`).
3 Real chart: as record 16. Claim not checked.
4 Direct: 120 `loading == pinned_loading` against the literal 68: a snapshot total over the corpus (the very shape the rubric's "A standing scan for totals" names). 121 the hash against a literal: a snapshot.
5 Reason: 68 is "the count pinned beside it" (the comment at 114-116); a new chart changes it. The helper already computes the same figure the test compares with the literal, so the typed 68 adds nothing a reader can use.
6 Display: n/a.
7 Failure: charts stopping loading at Medium, hidden by a repin. The count alone does not say which chart.
8 Deleted: 120: a chart that stops loading (the loading count against a typed number). 121: any parse change.
9 Stand-in: a hash and a count stand for "the parser's answer is the same".
10 Same rule: records 16, 17, 19.
11 Real: yes.
Verdict: Investigate (the golden question). Side finding for any rewrite: the typed 68 here and in record 19 is a snapshot total over `testdata/input`.
Today it catches: any parse change, and a drop in how many charts load at Medium.
Production finding: none.

### 19. the corpus's parse digest is pinned at Easy, Pro Drums off, 2x Bass off (tests/test_perf_digest.cpp:164)

Delta round: changed through its helper, replaces `delta-DL5.md` record 26 and `records/C7.md` record 40. Old verdict (Investigate) holds; the window line cannot reach it.

Read: test lines 164-166; helpers as record 18.
1 Claim: At Easy, Pro off, 2x Bass off, 68 corpus charts load, and their parse hash is 0xa5187516819c3195.
2-11: as record 18 (the literals are 68 and the Easy hash).
Verdict: Investigate (the golden question). The typed 68 is a snapshot total over the corpus.
Today it catches: any parse change, and a drop in how many charts load at Easy.
Production finding: none.

### 20. the crafted edge files parse to their pinned lines (tests/test_song_parse.cpp:1385)

Delta round: changed through its helper `edge_run` (`test_song_parse.cpp:1354`, which calls `digest_settings`), replaces `delta-DL5.md` record 26 and `records/B6.md` ("the crafted edge files parse to their pinned lines", file line 1385 then). The old verdict (Rewrite: no saved expected file) still holds, and the window line cannot reach it. This is the test that reaches line 1355.

Read: test lines 1311-1392; `testdata/parse_edge/expected_expert.tsv` and `expected_hard.tsv` and `gen_edge.py` are not opened; the old B6 record.
1 Claim: Each of 41 small crafted charts parses to its pinned hash, or fails with its pinned message, at Expert/Pro/2x Bass and at Hard/Pro off/2x Bass off.
2 Owner: `digest::chart_parse_hash` over the parsers; the expected files are saved outputs. Callers of the helper in `src/`: none.
3 Real chart: the inputs are crafted files, not real charts; the expected lines are a saved file captured "from the readers as they stood before P1" (comment 1313-1321). Claim not checked against an independent owner.
4 Direct: 1370 `REQUIRE(listed.size() == expect.size())`: a property. 1374 each line's file name equals the list's name: a property. 1378 the failure text equals the saved one. 1379 the hash equals the saved one: a snapshot from one run, per file.
5 Reason: a saved expected-output file; `code-and-engine.md:50` reads "we don't need golden anymore, remove it all". The comment says `git log` on the expected files says which lines were re-pinned and why: I did not run that.
6 Display: n/a.
7 Failure: the P1 reader rewrite changing output (comment 1313-1321). Still possible; `13c0657c` only moved the test into this file.
8 Deleted: 1370 and 1374: a generator line and its expected line drifting apart. 1378: a failure message changing. 1379: any change in how a crafted file parses (reasoned).
9 Stand-in: a hash for "what each crafted chart tests"; a failure names the file but not the part that moved.
10 Same rule: record 11 reads the same Expert file for three of its rows; one verdict.
11 Real: odd files built on purpose; each says what it tests in `gen_edge.py` (comment 1313; not opened).
Verdict: Rewrite. No saved expected file. Each generated file already states what it tests; check that stated intent per file instead.
Today it catches: any change in how 41 odd files parse.
Production finding: none.

---

## Removal of `lone_fill_activation`

The commit that removed `lone_fill_activation` from `tests/test_fills_and_skips.cpp` is `b1b0ed0f`, "Tests run the early-fill search at 85 and at 60 on every route" (found with `git log --oneline -S lone_fill_activation c10c7f54..5ec682c7 -- tests/test_fills_and_skips.cpp`; the same commit also changed `tests/test_activation_rows.cpp`). A later commit in the range, `e561ba5c`, touched the same file for the all-0 test.

---

## Splits

- A. "Does this squeeze fit the hit windows": the Paths tab's sum (`squeeze_safe_ms`, `note_window.cpp:67`, short below zero) against the engine's schedule (`feasible`, rooms and a 3 ms margin, `tools/replay_schedule.cpp:189-191`). On Beg both say it fits and the numbers differ (84.4 against 73.85). For Tapped Out's "short" only the screen's answer exists. Record 1; as A3's Split A.
- B. Two windows in one test file: `analyzed_like_the_gui` pins 85 ms for Tapped Out, Focus, Epidermis and Get Ur Freak On, while `burnout()` and the corpus test at 1607 (`Settings().to_analysis_settings()`) run at the 60 ms default. Records 6 and 7 loop over both kinds together, and the helper's comment still says it analyses "the way burnout() analyzes it". A screen on these charts at the default may show other E marks and paths than the figures typed here.
- C. The four prepared-row digests (records 12 to 15) are pinned at 85 ms; the app starts at 60 ms. Parse digests and the five-lane and crafted-file tests are not touched by the window.
- D. The user's words for the row: the tests' comments call lines under `## Unconfirmed` (`ui-and-display.md:105-111`) "the user's ruling"; only line 14 is filed. Records 1 to 5.

## Charts and captures needed and not available

- `hydra_replay dump` and `schedule` at `--efill 60` and `--efill 85` for Tapped Out, Focus, Epidermis, Get Ur Freak On and Burnout, with the Paths tab captured at each window (records 1 to 7).
- A chart whose squeeze does not fit, with the engine's `feasible: false` beside the screen's "short" (record 1).
- A real marked doc line fed to the marker check with its number changed (record 8): needs no capture, only a test change.
- Two stores written by `hydra_batch` over `testdata/input` with profile ids in another order, one with a result changed (records 9, 10).

## Count

Records written in full: 20 (activation rows 7, docs 1, compare_db 2, digest-helper tests 10).

Verdicts: Rewrite: 3, 4, 5, 7, 8, 20 (6). Investigate: 1, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19 (11). Remove: 2, 6, 11 (3). Total 20.

The commit that removed `lone_fill_activation`: `b1b0ed0f`.
