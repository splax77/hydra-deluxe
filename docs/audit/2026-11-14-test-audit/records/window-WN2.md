# T5-WN2: the window round for tests/test_sp_end.cpp, test_sp_end_clamp.cpp, test_sp_meter_curve.cpp, test_best_finish.cpp, test_stars.cpp, test_track_state.cpp, test_fast_tempo.cpp, test_squeeze_rating.cpp and tests/fast_tempo_charts.h

Auditor: a2a0eb1e7d9e56c06. Worktree HEAD holds `5ec682c7`'s code. Nothing was built or run.

## Read once for the whole group

**Which window every search here gets.** No test case in these files sets `early_fill_window_ms` (grep of the eight files and `fast_tempo_charts.h` for `early_fill`, `efill`, `window_ms`: no hit). Every search reaches the default:
- `ScoreGraph graph(song, cap)` takes `early_fill_window_ms = kDefaultEarlyFillWindowMs` (`src/search/graph.h:201`, 60.0 at `src/core/model.h:112`), and `run_search(graph, ...)` reads it from the graph (`src/search/engine.cpp:2030`, `:2176`).
- `Settings().to_analysis_settings()`, `test::scores_settings(cap)` (`tests/record_fixtures.h:582`, built on `Settings()`) and a bare `SearchSettings cfg;` (`test_squeeze_rating.cpp:150`, `pather.h:45`) carry the 60 default into `search_graph` (`src/search/pather.cpp:265`).
- `corpus::analyzed` keys its cache on the window (`tests/corpus_util.h:228`) but is handed the same default.
- So there is **no A row** in this group.

**What the window can move.** Two reads of `e_offset = fill_deadline_ms - sp_ready_ms` (`src/core/model.h:329-337`): `fill_refuses` when `e_offset < -window`, and `is_e0` (E mark) when `e_offset < window`. Between 60 and 85 ms, two bands change: `-85 < e_offset < -60` (refused at 60, spawns at 85) and `60 <= e_offset < 85` (not E0 at 60, E0 at 85). The deadline for a Ch11 fill is the fill's start tick minus `kCh11FillLeadBeats = 4` beats (`src/search/graph.h:71`, `src/search/graph.cpp:62-70`). The graph itself never reads the window (`src/search/graph.h:195-197`: "The graph does not read it"; `graph.cpp:95-100` only stores it).

**How a hand-built song is sorted (B).** I worked out the `e_offset` of every fill and ready pair in each fixture, from the tick grid. None falls in either band, so the result cannot move. The pairs:
- `build_tail_song` songs, 120 BPM, 192 ticks a beat (2.604 ms a tick). Fill at 2304 (length 384): deadline tick 1152; SP ready at the phrase at 768. `e_offset` = 384 ticks = 1000 ms (not refused, not E0 at either window). Used by `past_last_note_song`, `mid_sp_phrase_song`, `double_clamp_notes`, `make_overfill_song`, `clamp_song`, `sqin_then_collect_song`, and the five songs in test_sp_end.cpp:597-605 (the 98 BPM one is 1224 ms).
- `beat_song`, 240 BPM, 480 ticks a beat. Every phrase, fill and deadline sits on a 480-tick (250 ms) multiple, so every `e_offset` is a multiple of 250 ms. First fill: 500 ms. Second fill of `make_early_sqout_song`: 0 ms (E0 at both windows).
- `ready_time_fold_song`: every tick a multiple of 48 (125 ms), so every `e_offset` is a multiple of 125 ms.
- `banked_phrase_window_song`, 4000 BPM: fill 2304, ready at 1536, `e_offset` = -384 ticks = -30 ms. Inside both windows, so it spawns and is E0 at both.
- `banked_phrase_fold_song`: pairs are 3000 ms, 3000 ms, -210 ms, +30 ms (the `0 E0` path). None in a band.
- `test_sp_end_clamp.cpp` fixtures at 2400 BPM (0.1302 ms a tick, so the bands are 461 to 652 ticks): `kOneActivation` 50 ms; `kPlainTiesClamp` 0 and 25 ms; `kTiedClamps` 50 ms and multiples of 25 ms of 350 ms or more; `kFoldSharedClamp` at 1200 BPM: 100, 300, 700, -100, -500, -850 ms. **`kTwoActivations` is the exception: its second fill gives 576 ticks = 75 ms, inside `[60, 85)`.** That is row C below.

**Real charts.** The real-chart tests run the whole corpus (`corpus::chart_paths()`), or the fast-tempo fixture folders. I did not compute their `e_offset`s. A test with a count, floor or typed number over them is C. The old rounds read these tests at 85. Question 3 for every C needs `hydra_replay dump` at `--efill 60` of a named chart, so each C says which chart.

**The rulings.** `docs/agents/rulings/code-and-engine.md` lines 55 to 57 were not re-read in this group's work beyond what the brief points at; no verdict below rests on them.

**Property tests on a real chart (B by property).** Four rows are B because the asserted line holds for every path the search returns, and has no count or typed value that depends on which paths come back. They are the likeliest to be disputed. Each is named in the table.

## Summary table

Kinds: A (window set), B (window cannot move it), C (record below), N (does not reach the search; named so the verifier can check), D (delta round).

### tests/test_sp_end.cpp (24 test cases; 20 B, 4 C, 0 N)

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 81 | SP past the last note: backends measured from the tracked SP end | B | `past_last_note_song`, one fill, `e_offset` 1000 ms (read once, above) |
| 120 | SP past the last note: a mid-activation phrase extends the end | B | `mid_sp_phrase_song`, same single fill |
| 157 | SP past the last note: tail rows use the 500 ms window from the SP end | B | `past_last_note_song`, same |
| 179 | graph: every deactivation edge lists the chords squeeze_window_phrases names | B | builds `ScoreGraph` only; no search is run; the graph does not read the window (`graph.h:195-197`) |
| 212 | graph: a squeeze choice before or after the SP end, pinned | B | graph only, three subcases |
| 265 | graph: the tick and ms tests of 'after the SP end' agree across a tempo change on the end | B | graph only |
| 327 | SP cap overfill: where the end lands and which note clamped it | B | `build_tail_song` rows, one fill at 2304, 1000 ms |
| 410 | collected phrases: none when no phrase lands during the activation | B | `past_last_note_song` |
| 421 | collected phrases: one phrase mid-activation is recorded | B | `mid_sp_phrase_song` |
| 431 | collected phrases: two phrases under a full meter are both recorded, in order | B | `double_clamp_notes`, one fill |
| 445 | collected phrases: the corpus agrees with the squeezes and the SP end | C | record 1 |
| 475 | SP end history: a late squeeze-in is a SqIn step on its phrase | B | `make_late_sqin_song`, `beat_song`, 500 ms |
| 492 | SP end history: an early squeeze-in measures from the end before it | B | `sqin_then_collect_song`, one fill |
| 515 | SP end history: a squeeze-out measures from the deact node | B | `make_early_sqout_song`: 500 ms and 0 ms |
| 543 | search: scales anchor on the collecting note and the SqIn's own end | B | `clamp_song`, `sqin_then_collect_song`, one fill each |
| 590 | search: no fresh record stores an unknown transfer scale (constructed songs) | B | five single-fill songs (1000 ms, 1224 ms at 98 BPM); the floors `acts > 0`, `sqins > 0`, `sqouts > 0`, `clamps > 0` count shapes of those fixed songs |
| 635 | squeeze-out: one SqOut, last, and its offset is the search's and D's, on every corpus path | C | record 2 |
| 676 | SP end history: a squeezed-out phrase leaves no step | B | `make_early_sqout_song` |
| 698 | SP end history: no activation squeezes a phrase it banked before it started | B | `banked_phrase_window_song`, -30 ms |
| 724 | SP end history: a variant never takes a squeeze on a phrase it banked | B | `banked_phrase_fold_song`, no pair in a band |
| 759 | SP end history: every corpus activation is consistent | C | record 3 |
| 796 | Bank: a squeezed-out bar arrives at the deact node | B | `make_early_sqout_song` |
| 821 | Bank: every corpus path banks in order | C | record 4 |
| 837 | graph: a deactivation edge's rows are a range of the graph's rows | B | graph only. The typed 622, 28 and 134 are graph facts, not search results (not window-dependent). They are snapshot numbers; the old record for this test covers them |

### tests/test_sp_end_clamp.cpp (7 test cases; 6 B, 1 C)

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 97 | SP end clamp: the squeeze-out edge expects extend_deacts' clamped end | B | graph only |
| 114 | SP end clamp: a clamped window still offers its squeeze-out | B | `kOneActivation`, one fill, 50 ms (E0 at both) |
| 128 | SP end clamp: two ends clamped to one tick each keep their own squeeze-out | C | record 5 |
| 168 | SP end clamp: tied paths clamped from different ends keep their own squeeze-outs | B | `kTiedClamps`, pairs 50 ms and 350 ms or more |
| 210 | SP end clamp: a plain bar tying the ceiling squeezes out only for its own end | B | `kPlainTiesClamp`, 0 and 25 ms |
| 284 | SP end clamp: a folded variant's Clamped step on its leader's SqIn phrase becomes SqIn | B | `kFoldSharedClamp`, 100, 300, 700, -100, -500, -850 ms |
| 322 | SP end clamp: an unclamped window is unchanged | B | `kOneActivation` at cap 3, 50 ms |

### tests/test_sp_meter_curve.cpp (27 test cases; 3 B, 4 C, 20 N)

Reach the search: 237, 413, 448, 699, 784, 883, 941. The other 20 build hand-made `Activation`s or scenes (`preview_song_fixtures.h`, `priced_path` uses the replay, not the search) and call no search entry point. Grep of the file for `run_search|analyz|search_target|ScoreGraph|corpus::analyzed` found only the lines named in this table, plus comments at 195, 333, 362 and 373. N rows: 64, 85, 119, 155, 186, 274, 303, 329, 351, 375, 389, 406, 485, 503, 527, 549, 570, 598, 631, 759.

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 237 | sp meter curve: two clamped collections refill twice and empty at the deact node | B | `make_overfill_song` (`preview_song_fixtures.h:153`), one fill, 1000 ms |
| 413 | sp meter curve: a late squeeze-in refills on the squeezed chord | B | `make_late_sqin_song` |
| 448 | sp meter curve: a squeezed-out bar arrives on the squeezed chord | B | `make_early_sqout_song` |
| 699 | sp meter curve: every corpus gauge starts at its spent bars and empties at its window's end | C | record 6 |
| 784 | Preview windows end on the squeezed chord on every corpus path | C | record 7 |
| 883 | Preview lights each chord the record says is before the end | C | record 8 |
| 941 | Blight, Pitch Dark and Burnout: literals | C | record 9 |

### tests/test_best_finish.cpp (4 test cases; 1 B, 3 C)

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 48 | best finish: with nothing pinned it is Hydra's best path on every corpus chart | C | record 10 |
| 65 | best finish: every window of a corpus best path pinned in full ... equals search_target and the replay | C | record 11 |
| 93 | best finish: pinning the best path's first window keeps the best total | C | record 12 |
| 117 | best finish: an impossible window reports search_target's reason | B | the pin is `ticks.front() + 1`, a tick that is not a fill node of the graph, so the failure is "no node there" (`failed_reason == "activation"`); the node set is the graph's and the graph does not read the window. The best path it takes the tick from may change, but any activation tick plus one is not a node. Both sides (`best_finish`, `search_target`) run the same search. By reading only |

### tests/test_stars.cpp (17 test cases; 2 C, 15 N)

N rows (pure scoring or cutoff functions on typed values or hand-built paths; grep for the search entry points finds nothing outside 288 and 359): 42, 98, 138, 154, 160, 170, 190, 215, 221, 226, 232, 240, 245, 263, 326. The 326 case (path_stars) takes hand-built paths; I did not read its body beyond the grep.

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 274 | stars: the base score is the sum of every note's basescore, on every path | C | record 13 |
| 353 | stars: Burnout's optimal path earns 7 stars | C | record 14 |

### tests/test_track_state.cpp (23 test cases; 2 B by property, 21 N)

Only lines 1002 and 1034 call the search (`corpus::analyzed`). All other cases build scenes from hand-made overlays and spans.

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 991 | synthesize at an instant's own time agrees with the sweep that filled it | B (property) | the corpus half builds the first chart's scenes from stored paths and compares two derivations of the same scene (`check_synthesize_agrees`); no count, floor or typed value; holds for any path set |
| 1026 | rebuild_overlay_fields: equals a full build on every 8th corpus chart's stored paths | B (property) | `check_rebuild` compares two builds of the same scene pair. Floors `charts > 0` and `swaps > charts`: swaps is scenes minus one per chart, and the base scene and the no-path scene are always present, so a chart with one stored path gives 2 swaps; the floor needs a chart with at least one path, not a path count |

N rows: 148, 159, 167, 173, 218, 250, 316, 377, 415, 439, 476, 524, 555, 600, 623, 638, 665, 772, 819, 1013, 1054.

### tests/test_fast_tempo.cpp (8 test cases; 1 B, 7 C)

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 107 | fast tempo: graph tracks run forward and every phrase sits on its own edge | B | builds `ScoreGraph` only; asserts node and edge ticks; the graph does not read the window |
| 147 | fast tempo: the crash charts analyze, one SqIn step per SqIn | C | record 15 |
| 177 | fast tempo: a phrase a SqIn spent banks no bar when SP ends before it | C | record 16 |
| 229 | fast tempo: an SP end offers the next phrase after a banked or spent one | C | record 17 |
| 274 | fast tempo: a squeeze-out at an SP end gives back only the newest phrase | C | record 18 |
| 298 | fast tempo: early_sqin_twice offers its window one squeeze, 13248 at 16224 | C | record 19 |
| 329 | fast tempo: fuzzed charts analyze and their variants price as alone | C | record 20 |
| 369 | fast tempo: two spent phrases ahead, with or without a banked one, price as alone | C | record 21 |

`tests/fast_tempo_charts.h` is the helper: `analyzes(song, cfg, rec)` calls `analyze_chart(song, cfg)` (line 28) with the caller's settings, which are `scores_settings(cap)` (60 default). It sets no window. `fuzz_chart` builds the seeded charts.

### tests/test_squeeze_rating.cpp (29 test cases; 1 C, 28 N)

Only line 178 calls the search (`corpus::analyzed(path, cfg)` with a bare `SearchSettings`, so the 60 default). The two "field fixture" cases (300, 370) and the rest hand-build activations or stored scales; their comments mention "the search" (373, 396, 730) but the grep finds no search call. N rows: 28, 87, 122, 224, 240, 300, 370, 447, 462, 485, 502, 519, 557, 597, 623, 651, 681, 709, 729, 750, 795, 839, 875, 897, 939, 966, 972, 976.

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 149 | step_transfer_scale: equals every stored scale on the squeeze corpus charts | C | record 22 |

### Delta round

No test case in this group has a record in `records/delta-DL*.md` (grep for the eight file names in those files: no hit on these test cases). So there are no "delta round" rows.

## C records

Format: the rubric's. Each question 3 asks for a dump at `--efill 60` of the named chart from the main session; until it comes the claim is not checked at 60. The old records read the real-chart claims against the dumps in `records-engine/` (made at 85).

### 1. collected phrases: the corpus agrees with the squeezes and the SP end (tests/test_sp_end.cpp:445)

```
Delta round: window, replaces records/A28.md record 11 ("collected phrases: the corpus agrees with the squeezes and the SP end"). Old verdict Rewrite (drop 462-463, keep 457 and 466, count and name empty songs). It still holds at 60 ms: every fault named there is the test's own.
Read:       lines 445-473; run_search (graph default window 60, graph.h:201); old record A28 #11.
1 Claim:    On every corpus chart and every path, an activation records a squeeze-out exactly when it squeezed one out, its collected phrases rise, start at or after the activation, and sit before the squeeze-out.
2 Owner:    The search stamps sqout_tick and the collected list; the test reads them back. Engine's.
3 Real chart: Claim holds for any path (a property). `sqouts_seen > 0` (line 472) counts squeeze-outs over the whole corpus, and which paths exist is what the window moves. Needs hydra_replay dump --efill 60 of a corpus chart that has a squeeze-out in the old dumps (the old record names one), to see that one still does. Not checked.
4 Direct:   457, 462-463, 466 state properties. 472 is a floor.
5 Reason:   floor 0 is fine (zero is a property's one free literal); it only has to be reached.
8 Deleted:  472 deleted: a corpus with no squeeze-out would pass silently (reasoned). The window moves it only if every squeeze-out path used a fill with e_offset in -85..-60 ms; I can not rule that out without the dump.
Verdict:    Investigate, claim not checked only (at 60 ms). The old Rewrite also stands.
```

### 2. squeeze-out: one SqOut, last, and its offset is the search's and D's, on every corpus path (tests/test_sp_end.cpp:635)

```
Delta round: window, replaces records/A28.md record 17. Old verdict Rewrite (silent skip; line 661 true by construction). Still holds at 60 ms.
Read:       lines 635-674; old record A28 #17.
1 Claim:    On every corpus path an activation has at most one SqOut, it is last, and its row offset equals the offset measured again from the deact node.
2 Owner:    Engine stamps the row; D is the deact node. Engine's.
3 Real chart: the properties hold for any path. `sqouts > 0` (672) counts squeeze-outs over the corpus; it can move with the window. Needs the --efill 60 dump of the chart named in A28 #17. Not checked.
8 Deleted:  672: a corpus where nothing squeezed out would pass (reasoned).
Verdict:    Investigate, claim not checked only (at 60 ms); old Rewrite stands.
```

### 3. SP end history: every corpus activation is consistent (tests/test_sp_end.cpp:759)

```
Delta round: window, replaces records/A28.md record 21. Old verdict Rewrite (replace the floor). Still holds at 60 ms.
Read:       lines 759-794; old record A28 #21.
1 Claim:    Every activation of every stored path (including all-zero paths) has a history that starts with its own activation step, rises in tick, and never repeats a SqIn.
2 Owner:    Engine stamps `sp_end_steps`. Engine's.
3 Real chart: property; the floor `acts > 1000` at 793 is a typed number over the corpus. It is a count of activations over all paths, so it moves with the window and with any chart added. The user's approval is cited ("D43") and is a label, not a reason. FINDING: line 793, number 1000. Needs: --efill 60 total activation count over testdata/input from the main session, compared with 1000.
8 Deleted:  793: a corpus losing most activations would pass at less than 1000 (reasoned).
Verdict:    Investigate, claim not checked only (the count at 60 ms); old Rewrite stands.
```

### 4. Bank: every corpus path banks in order (tests/test_sp_end.cpp:821)

```
Delta round: window, replaces records/A28.md record 23. Old verdict Rewrite (replace the floor). Still holds at 60 ms.
Read:       lines 821-831; old record A28 #23.
1 Claim:    Every stored path's banked bars land in order between its windows, tied variants included.
2 Owner:    `bank_check::check_record_banks` reads the engine's `bank_rise_ticks`; engine's.
3 Real chart: property. FINDING: line 830, `acts > 1000`, same floor as record 3, over the activations `check_record_banks` returns.
Verdict:    Investigate, claim not checked only; old Rewrite stands.
```

### 5. SP end clamp: two ends clamped to one tick each keep their own squeeze-out (tests/test_sp_end_clamp.cpp:128)

```
Delta round: window, replaces records/C11.md record "two ends clamped to one tick each" (line 128). Old verdict Investigate, claim not checked only (question 11: reachability on a real chart). It still holds at 60 ms, with one new fact below.
Read:       lines 20-45 (`kTwoActivations`), 128-165; sqouts_of; model.h:329-337; graph.cpp:62-70.
1 Claim:    When two fills at 2304 and 2496 each end SP inside the window after the phrase at 3456, and both ends clamp to 6528, a search that takes only one of them squeezes out 3456 back to that activation's own end.
2 Owner:    The search (`search_target` with a pinned activation) stamps the squeeze-out and the row; the test reads them.
3 Real chart: claim not checked (D255, per the old record, notes no corpus chart has the case).
New fact:   at 2400 BPM (25/192 ms a tick) the fill at 2496 has deadline tick 1344 and SP is ready at the phrase at tick 768: e_offset = 576 ticks = 75.0 ms. That is inside [60, 85). The activation at 2496 is E0 at 85 and not E0 at 60 (`is_e0`, model.h:335). The fill at 2304 is 50 ms, E0 at both. The test's assertions (squeeze-out tick, end, row offset) do not read the E mark; the search's group key does read the window (`engine.cpp:859-870`). By reading, the paths are the same; not proven.
8 Deleted:  the assertions on the squeeze-out offsets (-200/-225 style): none read E.
Verdict:    Investigate, claim not checked only (at 60 ms, and the old reachability question).
```

### 6. sp meter curve: every corpus gauge starts at its spent bars and empties at its window's end (tests/test_sp_meter_curve.cpp:699)

```
Delta round: window, replaces records/A29.md record 23. Old verdict Rewrite (count skipped charts; replace `windows > charts`). Still holds at 60 ms.
1 Claim:    On every corpus chart the gauge of each stored window starts at its spent bars and is empty at the window's end.
2 Owner:    Engine stamps windows; the Preview draws. Property read from the record.
3 Real chart: property, but `windows > charts` is a count of stored windows over all charts; it moves with the window. Needs an --efill 60 count.
Verdict:    Investigate, claim not checked only; old Rewrite stands.
```

### 7. Preview windows end on the squeezed chord on every corpus path (tests/test_sp_meter_curve.cpp:784)

```
Delta round: window, replaces records/A29.md record 25. Old verdict Rewrite (count skipped charts; read expected ms from the record's tick). Still holds at 60 ms.
1 Claim:    On every corpus path, a squeezed-out window's Preview end is the squeezed chord, and a late squeeze-in's refill is on its chord.
2 Owner:    Engine stores steps; the Preview reads them. Counters `early_out` and `late_out` (line 785 on) count squeeze types seen over the corpus; they move with the window.
3 Real chart: claim not checked at 60 ms. Needs --efill 60 dumps of the Burnout and Pitch Dark charts from the old record (A29 #27 names the ticks).
Verdict:    Investigate, claim not checked only; old Rewrite stands.
```

### 8. Preview lights each chord the record says is before the end (tests/test_sp_meter_curve.cpp:883)

```
Delta round: window, replaces records/A29.md record 26. Old verdict Rewrite (silent skip at 890). Still holds at 60 ms.
1 Claim:    Each chord the record places before the SP end is lit by the Preview, on every corpus path.
2 Owner:    The record; the Preview reads it. Property over every path; no typed value read beyond the skip counter.
3 Real chart: holds for any path set. I did not read lines 883-940 in full for a floor; the old record names only the skip. By reading, a property. Strictly this could be B; I put it C because the body was not read in full (see "Not reached").
Verdict:    Investigate, claim not checked only; old Rewrite stands.
```

### 9. Blight, Pitch Dark and Burnout: literals (tests/test_sp_meter_curve.cpp:941)

```
Delta round: window, replaces records/A29.md record 27. Old verdict Remove (test 25 states both promises from the record without typed ticks). Still holds at 60 ms, and the window makes the case worse.
1 Claim:    On Burnout the first window ends on the squeezed-out chord (tick 70800); on Pitch Dark the bar refills on the red snare (tick 128520).
2 Owner:    Engine record.
3 Real chart: the test finds its path by TYPED PATH STRING: `path_named(..., "3- 1 2")` (line 948) and `"3+ 2+ 1"` (line 970), and checks typed ticks 59520, 71040, 70800 and 48,783.075 ms. These are results of one run at 85. FINDING (numbers that can move): "3- 1 2" and "3+ 2+ 1" (lines 948, 970), and every tick pinned from those paths. If a path at 60 ms has a different name, `path_named` returns null and `REQUIRE(p != nullptr)` fails, though nothing is wrong.
Verdict:    Remove (the old verdict; test 25/record 7 covers it from the record). At 60 ms it also fails for a reason that is not a bug if the path string changes.
```

### 10. best finish: with nothing pinned it is Hydra's best path on every corpus chart (tests/test_best_finish.cpp:48)

```
Delta round: window, replaces records/A8.md record 14. Old verdict Rewrite (silent skip; the identity stays). Still holds at 60 ms.
1 Claim:    With nothing pinned, `best_finish` returns the main search's best path string and total on every chart.
2 Owner:    Engine. Both sides run the search under the same cfg, so they use the same window.
3 Real chart: identity between two runs at one window; holds at any window. The `charts > 0` floor at 41 is `>0`. By reading, only the path set moves. Needs one --efill 60 dump to check the identity holds.
Verdict:    Investigate, claim not checked only; old Rewrite stands.
```

### 11. best finish: every window of a corpus best path pinned in full ... equals search_target and the replay (tests/test_best_finish.cpp:65)

```
Delta round: window, replaces records/A8.md record 15. Old verdict Rewrite (shared helper's silent skip). Still holds at 60 ms.
1 Claim:    Pinning every window of the best path gives the same total from `best_finish`, `search_target` and the replay.
3 Real chart: pins come from the 60 ms best path, so a pinned activation always passes `fill_refuses` at 60 ms. Identity at one window. Not checked.
Verdict:    Investigate, claim not checked only; old Rewrite stands.
```

### 12. best finish: pinning the best path's first window keeps the best total (tests/test_best_finish.cpp:93)

```
Delta round: window, replaces records/A8.md record 16. Old verdict Rewrite (keep 103 and 110, drop 111). Still holds, and 111 is now a number that can move.
1 Claim:    On Mutemath - Allies, pinning the best path's first window leaves the best total where it was.
3 Real chart: FINDING: line 111 `CHECK(bf0.paths.front().totalscore() == 451830)`, a total taken from a run at 85 ("pinned from one run" in the comment above it). The best total on Allies depends on which fills spawn. Needs an --efill 60 dump of Mutemath - Allies. Line 103 asserts the first window fits the default 10 ms limit; it reads the 60 ms best path, so it is not typed.
Verdict:    Investigate, claim not checked only for 110; line 111 goes (old verdict) and cannot be trusted at 60 until the dump.
```

### 13. stars: the base score is the sum of every note's basescore, on every path (tests/test_stars.cpp:274)

```
Delta round: window, replaces records/C12.md record "the base score is the sum" (274). Old verdict Rewrite (drop `== 137950`; replace `checked > 1`). Still holds at 60 ms.
1 Claim:    On Polyphia - 87, every path (and every all-zero path) has the chart's base score, which equals the sum of the notes' base scores, and the same solo bonus.
2 Owner:    Chart parse (note sum); the engine stores it on each path.
3 Real chart: 137950 and `score_solo` are chart facts (the notes sum), not search results, so the window cannot move them. `checked > 1` (line 309ish) counts paths in the record and the all-zero list; it can move with the window. FINDING: `checked > 1`.
Verdict:    Investigate, claim not checked only (path count at 60); old Rewrite stands.
```

### 14. stars: Burnout's optimal path earns 7 stars (tests/test_stars.cpp:353)

```
Delta round: window, replaces records/C12.md record "Burnout's optimal path earns 7 stars" (353). Old verdict Rewrite (check the star count from the cutoffs table). Still holds.
1 Claim:    Burnout's best path totals 378315, the 7-star cutoff is 335500, and the path earns 7 stars.
3 Real chart: FINDING: 378315 (line 359) is the best path's total from a run at 85 and can move at 60 (it depends on which fills spawn). 335500 comes from the chart's base score (notes only), so it does not move. 7 stars holds as long as the total stays at or above 335500. Needs --efill 60 dump of Burnout.
Verdict:    Investigate, claim not checked only for 378315; old Rewrite stands.
```

### 15. fast tempo: the crash charts analyze, one SqIn step per SqIn (tests/test_fast_tempo.cpp:147)

```
Delta round: window, replaces records/C2.md record "the crash charts analyze, one SqIn step per SqIn" (:147). Old verdict Investigate, claim not checked only (lone-pricing and bank parts; the silent skip of variants with no oracle). Still holds at 60 ms.
1 Claim:    The five crash charts analyze without a throw; every SqIn has its own step; banked bars are in order; every tied variant prices as it does alone.
2 Owner:    Engine; `analyze_chart` throws on a broken state (`fast_tempo_charts.h:28`), `lone_mismatch` runs `search_target`.
3 Real chart: the fixtures are hand-written charts in `testdata/input/test_fast_tempo` at very high tempos. I did not compute their `e_offset`s; at these speeds (a measure of tens of ms) a fill's deadline and the ready time can easily differ by 60 to 85 ms. Whether the crash shape still arises at 60 ms is the open question: a chart that guards a throw (D32) can stop reaching it. Needs --efill 60 dumps of node_before_phrase, end_on_window_node, sqin_end_before_phrase, late_sqin_twice and early_sqin_twice.
Verdict:    Investigate, claim not checked only.
```

### 16. fast tempo: a phrase a SqIn spent banks no bar when SP ends before it (tests/test_fast_tempo.cpp:177)

```
Delta round: window, replaces records/C2.md record ":177". Old verdict Rewrite (take phrase, activation and end from the record's step; claim not checked). Still holds.
1 Claim:    On sqin_end_before_phrase.chart at cap 3, the path that activates at 5472 late-squeezes in the phrase at 13920, the new SP end is 11616, and no bank list holds 13920.
3 Real chart: FINDING: 5472, 13920 and 11616 (the comment and the typed `kEnd`) are activation and end ticks from a run at 85. A fill the search can use at 85 and not at 60 changes whether a path activates at 5472. Needs --efill 60 dump of sqin_end_before_phrase.chart.
Verdict:    Rewrite (old) and Investigate (claim not checked), not Keep.
```

### 17. fast tempo: an SP end offers the next phrase after a banked or spent one (tests/test_fast_tempo.cpp:229)

```
Delta round: window, replaces records/C2.md record ":229". Old verdict Investigate, claim not checked only. Still holds.
3 Real chart: FINDING: typed pairs (12000, 16000) on banked_then_next.chart and (17360, 18360) on spent_then_next.chart are phrase ticks of the fixtures (chart facts), but the test needs a path that reaches the SP end; whether one does depends on which fills spawn. Needs --efill 60 dumps of both charts.
Verdict:    Investigate, claim not checked only.
```

### 18. fast tempo: a squeeze-out at an SP end gives back only the newest phrase (tests/test_fast_tempo.cpp:274)

```
Delta round: window, replaces records/C2.md record ":274". Old verdict Rewrite (typed 15648 is an engine result; claim not checked). Still holds.
3 Real chart: FINDING: 8832 (the activation window looked up with `window_at`, line 282) and 15648 (`at_15648`, the squeezed chord count) are results from a run at 85. `window_at(*p, 8832)` returns null if no path activates there at 60. Needs --efill 60 dump of node_before_phrase.chart.
Verdict:    Rewrite and Investigate; not Keep.
```

### 19. fast tempo: early_sqin_twice offers its window one squeeze, 13248 at 16224 (tests/test_fast_tempo.cpp:298)

```
Delta round: window, replaces records/C2.md record ":298". Old verdict Rewrite (express the phrase and end as named positions; claim not checked). Still holds.
3 Real chart: FINDING: 10176 (activation looked up at line 305), 13248 and 16224 are engine results from a run at 85; the loop `continue`s silently when `window_at` finds nothing (line 307), so at 60 a missing window passes without checking anything. Needs --efill 60 dump of early_sqin_twice.chart.
Verdict:    Rewrite and Investigate; not Keep.
```

### 20. fast tempo: fuzzed charts analyze and their variants price as alone (tests/test_fast_tempo.cpp:329)

```
Delta round: window, replaces records/C2.md record ":329". Old verdict Rewrite ("no seed skipped" as `analyzed == seed count`; replace `variants > 50`). Still holds.
3 Real chart: seeded random charts from the fixed generator; `variants > 50` is a count of tied variants over all the seeds and moves with the window. FINDING: line ~360, `variants > 50`. Needs --efill 60 variant count over the same seeds.
Verdict:    Rewrite (old); Investigate for the count.
```

### 21. fast tempo: two spent phrases ahead, with or without a banked one, price as alone (tests/test_fast_tempo.cpp:369)

```
Delta round: window, replaces records/C2.md record ":369". Old verdict Investigate, claim not checked only (seeds depend on the generator). Still holds.
3 Real chart: FINDING: `shown > 0` (line 410) counts paths that show the two-spent-ahead state; it depends on which paths exist at 60. Seeds from the generator. Needs --efill 60 dump for the seeds the test names.
Verdict:    Investigate, claim not checked only.
```

### 22. step_transfer_scale: equals every stored scale on the squeeze corpus charts (tests/test_squeeze_rating.cpp:149)

```
Delta round: window, replaces records/A30.md record 4. Old verdict Rewrite (the expected side must not be the function that made the stored value). Still holds at 60 ms.
1 Claim:    On every chart in test_folded_sqin and test_fast_tempo, each stored SqIn scale equals `step_transfer_scale` for its step, and a step past the list's end is unset.
3 Real chart: property over whichever paths exist; floors `charts > 0`, `acts > 0`, `sqins > 0` (lines 214 to 216). `sqins > 0` needs at least one SqIn path on these fast-tempo folders at 60 ms; that depends on which fills spawn. Needs --efill 60 dump of one folded_sqin chart.
Verdict:    Investigate, claim not checked only; old Rewrite stands.
```

## Typed numbers found that can move

- tests/test_best_finish.cpp:111, `451830`, Mutemath - Allies best total, from a run at 85.
- tests/test_stars.cpp:359, `378315`, Burnout best total, from a run at 85. (`335500` and `137950` are chart facts, not moved by the window.)
- tests/test_sp_meter_curve.cpp:948 and :970, the path strings `"3- 1 2"` and `"3+ 2+ 1"`, and the ticks 59520, 71040, 70800 and 128520 and the ms 48,783.075 and 48,620.0315 pinned from those paths.
- tests/test_fast_tempo.cpp: activation ticks 5472, 8832, 10176, 15648 and ends 11616, 13248 (13248 at 16224 is in the test's name) from runs at 85.
- tests/test_sp_end.cpp:793 and :830, `acts > 1000` (two floors over corpus activations).

## Splits

None found in this group. (The test_sp_end_clamp.cpp:128 case shows one E mark that differs between 60 and 85 on `kTwoActivations`, but both windows are one rule, `is_e0`, so it is not two answers to one question.)

## Not reached, by exact name

- tests/test_sp_meter_curve.cpp "Preview lights each chord the record says is before the end" (883): lines 883-940 were not read in full, only checked by grep and the old record. Record 8 is marked C on that ground.
- The 20 N rows of test_sp_meter_curve.cpp, the 15 N rows of test_stars.cpp, the 21 N rows of test_track_state.cpp and the 28 N rows of test_squeeze_rating.cpp were sorted by grep for every search entry point (`run_search`, `analyze_chart`, `search_target`, `search_allzero`, `ScoreGraph`, `Engine`, `corpus::analyzed`, `analyzed_with_paths`) and not read body by body. A verifier should repeat that grep.
- The `e_offset` of the real-chart corpus and of the test_fast_tempo fixture charts: not computed (no run allowed).
- `tests/preview_song_fixtures.h` and `tests/bank_check.h`: opened only at `make_overfill_song`; the other helpers in them were not checked for a search call.
