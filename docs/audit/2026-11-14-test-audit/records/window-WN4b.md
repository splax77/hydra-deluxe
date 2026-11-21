# Window round WN4b (agent aff6de71a16cb248e)

Task T5-WN4b, session e633f7f6-f313-4787-b622-8d901b577b64. Re-send of WN4 for `tests/test_pb_coach.cpp`, `tests/test_pb_text.cpp`, `tests/test_saved_figure_fill.cpp` and the helper `tests/dm_fixture.h`. Code read at the worktree HEAD (`5ec682c7`'s code). Nothing was run. Every claim is "by reading" unless it names a run. I read every test body in the three files, line by line, and the old records `records/C7.md` (coach 1 to 19), `records/A19.md` (PB1 to PB10) and `records/A27.md` (SF1 to SF10), and the WN4 file `records/window-WN4.md` and its verifier `verify/window-WN4.md`.

## What the window does here (read in code)

- The default is 60: `kDefaultEarlyFillWindowMs = 60.0` (`src/core/model.h:112`). The rule is `fill_refuses(e_offset, window_ms) = e_offset < -window_ms` and `is_e0(e_offset, skips, window_ms) = e_offset < window_ms && skips == 0` (`src/core/model.h:331,335`). The engine reads it at `src/search/engine.cpp:859,862,1007`.
- Direction: a smaller window refuses more fills. A fill the engine refuses at 85 is refused at 60, so an unreachable tick stays unreachable. A fill that was reachable at 85 with SP ready 60 to 85 ms late is gone at 60. And an activation whose early fill was 60 to 85 ms early is no longer an E0 at 60. So a typed fill tick, a typed path, a typed score and a typed E can all move; a tick that holds no fill cannot gain one.
- `pb_coach` and the Path Builder: `PbState::open_with` (`src/pathbuilder/pb_state.cpp:15-28`) sets `c.settings = settings.batch_run().settings`, builds the graph with `search_graph(c.song, c.settings)` and runs `analyze_chart(c.song, c.settings, ...)`. `scratch_settings()` (`tests/scratch_settings.h`) changes only folders, depth 2 and auto_open_report, so `early_fill_window_ms` is the default 60. Every `open_and_wait(s, ..., scratch_settings())` case runs the search at 60. `limited` in the 133 case copies it and sets only the Path limit.
- `pb_text`: `default_cfg()` is `Settings().to_analysis_settings()` (`tests/corpus_default_cfg.h:17`), which copies `early_fill_window_ms` (`src/app/config.cpp:411`): 60. `player_path` runs `search_target(song, default_cfg(), ...)`; `best_finish` and `corpus::analyzed` take `default_cfg()` too.
- The Preview's Offered and Hidden fills come from the engine's `trailing_skipped_fill_ticks` (`src/app/preview_view.cpp:462,494`), so what `fill_refusal` calls "the game hides this fill" is the engine's answer under the window.
- `within_ms_limit` reads the window too: `fill_within_limit(e_offset, skips(), limit_ms, e_window_ms)` (`src/core/model.cpp:535`). So a coach row's "Path limit" note can change with the window.
- `saved_figure_fill`: `fill_settings()` is `Settings{}`. The test saves the record from `corpus::analyzed(chart, settings.to_analysis_settings())` (`tests/store_fixtures.h:78-79`, and the same in the cases at 168, 341, 371), at 60. The fill then builds `search_graph(song, settings)` from the same `AnalysisSettings` (`src/app/saved_figure_fill.cpp:100`), also at 60. The saved path text and the replay graph agree on the window, so a path text always places. What can move is which path text the search stores, and the figure it gets.
- Engine records: `records-engine/` holds seven charts, made under the old fixed 85 ms rule (`records-engine/index.md`). Only Smash Mouth "I'm A Believer" (`believer.dump.json`, ranks 1 to 3 "1 0+ 0 0", "1 0+ 2", "1 0+ 1", rank 5 "1 0- 1") is a chart these three files use. Burnout, White Washed, Allister "Overrated", Trivium "Feast of Fire" and Foo Fighters "Everlong" have no engine record at all. No dump exists at 60 for any chart.
- The filed rulings on the window, under `## Confirmed` in `docs/agents/rulings/code-and-engine.md` lines 55 to 57: "so we should use the hit window. and stop inventing our own numbers." / "make the early fill threshold an adjustable setting pinned to one number in both directions" / "analysis settings panel, default 60ms, supports 0ms to 85ms".

## Kinds, by file

### tests/test_pb_coach.cpp: A 0, B 6, C 13, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 38 | coach rows: a two-click path fills the ladder... | C | `open_and_wait` at `scratch_settings()`, 60; types scores, a deact tick and a path string |
| 99 | coach click: an impossible click leaves the path unchanged and says why | B | One click at tick 0 and the sentence for "no fill at this tick". The window only refuses more fills as it shrinks (`fill_refuses`), so a tick that fails to hold a fill at any window cannot hold one at 60. Asserted values are the sentence from `impossible_window_text("activation", true)` (`pb_coach.cpp:54-60`) and empty fields. No typed number. Tick 0 holding no fill is the test's own claim (see record 2 below) |
| 112 | coach click: a refusal is current only while the player's path stays as it was | B | Same click at tick 0; the assertions read `history().generation()` counters and `message_current`, which compare generations (`pb_coach.h:357-359`), no search value |
| 133 | coach rows: a window over the Path limit carries the tight-window note | C | 60; types 5 activations, row index 3, token "0-", note text |
| 178 | coach rows: a squeeze with time to spare under a lower limit names the limit alone | C | 60; types the all-0 path "0 0 0 0 0 0+" and 6 activations |
| 202 | coach click: a fill that rules out a later window is refused in the later-window sentence | C | 60; types fill tick 36480 and 3 windows |
| 242 | coach end: choosing the last window's end pins it | C | 60; types two ends with ticks, notations, a path string and 130.4 ms |
| 276 | coach end: the kept end is the window's own end | C | 60; reads the index of the kept end of a search's end list |
| 299 | coach end: an earlier window's endings are the same list | C | 60; types 5 windows, ticks, finishes |
| 368 | coach remove: removing a window drops it and every later one | C | 60; types 3 windows, "3-", 334495 |
| 397 | coach remove: removing every window leaves the empty path | C | 60; types 3 windows, 2 windows, "3- 1", 358235 |
| 427 | coach remove: a tick with no window changes nothing | B | `remove_window` returns `{}` when no pinned window has the tick (`pb_coach.cpp:195-198`: `found` is false), before any search. The windows are the best path's own (`pinned_windows`), and tick 0 is not one. No typed engine value |
| 439 | coach finish: Finish the rest for me makes BF(n)'s path the player's path as one edit | C | 60; types the path "9 1" at line 464 |
| 469 | coach click: a result that arrives after the player's path changed is dropped | B | The assertions are generation counters (488, 497), an empty windows list (489), `take_player_changed` and `message` empty; the click tick is taken from the same search's `best[1]` (476), so it is reachable at any window. The one search-shaped line is `REQUIRE(best.size() >= 2)` (475): Burnout's best path has 3 activations at 85 (case 202 types 3), and at 60 a best path of fewer than 2 would stop the case, not pass a wrong value |
| 503 | coach rows: a path with no window has no after-your-last-window line | B | `ladder(c, {})` with an empty `Path{}`: `coach_list` of a path with no activations has no rows (511) and no after-last (512, 513). `REQUIRE(run.complete)` (509) is the ladder of zero windows, one search of the whole chart. No window-dependent value is asserted |
| 516 | coach worker: a newer request supersedes the older | B | Asserts `superseded() == 1` (527), key (528), `bf.size() == 1` (533, one entry for no windows), no error (534). None reads a score or a path. The count of 1 depends on timing (the first ladder still running), not on the window; old record C7 16 already says so |
| 572 | candidate costs: the best next fill costs 0 and every other cost is BF(n) minus its best finish | C | 60; types BFs, path "0 1 3 0- 0", six fill ticks, six finishes and costs |
| 619 | candidate queue: it prices only the ticks it is given, in order | C | 60; types fill ticks 199680 and 165120 |
| 645 | candidate queue: a newer request supersedes the older | C | 60; types six fill ticks and 199680 |

### tests/test_pb_text.cpp: A 0, B 1, C 9, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 52 | choices hint: a row is clicked, and a hover promises nothing | B | Compares the constant `kChoicesHint` to a string. No search runs. |
| 56 | candidate row: the clock, phrases, end and squeeze of a priced fill | C | `default_cfg()` 60; types ticks and row text |
| 112 | choice order: cheapest first, ties in chart order, pending last | C | `player_path(song, {})` at 60; line 118 types the eight offered fill ticks of Overrated as a `REQUIRE` |
| 131 | step short: start, end and phrases | C | `corpus::analyzed(..., default_cfg())` 60; types measures and phrase counts |
| 146 | step short: the row carries the Paths tab's badge words | C | 60; types spans and "squeeze out 130.4 ms" |
| 165 | end choice text: the end measure and the Paths tab's badge words | C | 60; types "ends m135 · squeeze out 130.4 ms" |
| 203 | fill refusal: the five kinds in fill_reason's order... | C | 60; types fill indexes, tick 72480, 180000, 195360, 156960 and the sentences |
| 298 | fill card: an offered fill reads the heading, three rows and two grey lines | C | `OverratedCard` runs `best_finish` and `corpus::analyzed` at `default_cfg()` 60; types 226,210, 2,500, 228,710 |
| 313 | fill card: a costless fill reads best and "This is the best choice now." | C | same card; types m38, 228,710, m44 |
| 326 | fill card: taken, locked, refused and pending fills read the heading and their one line | C | 60; types headings m34, m30, m38, clocks, m40, m20, m30 |

### tests/test_saved_figure_fill.cpp: A 0, B 2, C 8, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 93 | a saved path with a squeeze-in on the SP end gets its figure back | C | 60 on both the save and the replay; types "1 0+ 0 0", five figures |
| 121 | a saved path whose chart is gone keeps its empty figure | B | `chart_still_there` is false for the missing file, so `chart_figures` returns before the replay or any graph (`saved_figure_fill.cpp:96-97`); the asserts are an empty figure on rank 1 and the marker. No search value is typed |
| 133 | a row that disagrees with its replay keeps its empty figure | C | 60; types rank 2's figure 0.0 (line 143) |
| 147 | a stop leaves the marker unset | C | 60; types rank 1's figure 0.0 (line 164) |
| 168 | a chart whose replay throws keeps its rows empty, and the fill goes on | C | 60; types Everlong's rank 1 path text "1 2 3+" (189, 200) and Smash Mouth's 0.0 (208) |
| 212 | the fill on several workers writes the same figures as on one | B | Compares four runs of one fill on one store for equality (279, 271) and counts calls against `want` (281-283). The six charts are the first six whose store rows `rows_missing_hardest` returns, chosen by the store's own question (234), whatever the window made of them. The one search-shaped floor, `figures > 0` (278), holds whenever a chart is picked, because `rows_missing_hardest` lists only rows with `sqin_count > 0` (the SQL at 375 says so) and a replay of the engine's own path on the same graph returns its figure (`replay_matches`, `saved_figure_fill.cpp:28-34`) |
| 286 | an inside-Star-Power squeeze-in fills a negative figure that shows as none | C | 60; types "1 2 3+", counts 1 and 0, and -189.83750000002328 |
| 308 | the fill hands every row of one chart the same graph | C | 60; types five figures (333-334) and 3 replays (327) |
| 341 | a result this build saves holds a saved figure on every path with a squeeze-in | C | 60; the floor `squeezed > 0` (362) rests on the two named charts having a squeeze-in path |
| 371 | a row under an older stamp is left alone by the fill | C | 60; types 3 empty rows (389, 403) which require ranks 1 to 3 to each hold a squeeze-in |

### Helper `tests/dm_fixture.h`

It does reach the search. The WN4 file and its verifier both said it had no search entry point; that is wrong. `testdm::sample_chart()` (`tests/dm_fixture.h:46-52`) calls `corpus::first_analyzed_with_paths(AnalysisSettings{depth_value = 0})`, an `AnalysisSettings` whose `early_fill_window_ms` is the `SearchSettings` default, 60 (`src/search/pather.h:45`). `fill_store` saves that record under `kHash` and returns `result.record.best_path().totalscore()` (the leaderboard side). The window is not set; it is the default. Which chart is "the first with paths" and its best score both come from a search at 60. The tests that read it are `tests/test_dm_other_speed.cpp`, `test_dm_report.cpp`, `test_fill_report.cpp`, `test_report_rows.cpp` and `test_store_library.cpp` (grep of `dm_fixture.h`); they belong to other groups and I judged none of them. The helper's other functions (`settings`, `profile`, `save`, `save_no_paths`, `make_score`, `dm_tile`) run no search. Kind: C for the helper (default window, a returned value that comes from a search); no record of its own, because it asserts nothing. It matters to a caller that types a score or a count taken from `fill_store`.

## Typed numbers on assertion lines that a run at 60 ms can move

Every one below is a value taken from a search at the default window. Each is a finding on its own line; "from one run" in the test's own comment is the only source, and no run at 60 exists.

`tests/test_pb_coach.cpp`:
- 66 `bf == {378315, 364675, 364675}`; 67 `player_total == 364675`; 68 and 96 path "9 1"; 74 `cost == 13640`; 88 `deact_tick == 128640`; 89 `best_finish == 364675`; 44, 53, 60, 86 counts.
- 144 `acts.size() == 5`; 162 token "0-"; 163 and 174 "needs 130.4 ms, your limit is 10 ms".
- 187 "0 0 0 0 0 0+"; 189 `acts.size() == 6`.
- 208 `best.size() == 3`; 209 `kFillTick = 36480` (the comment at 213 also names 59520).
- 249, 250, 251, 252, 253, 258, 259, 260, 266, 273: two ends, "0+" at 257280, "0-" at 253440, four windows, "0 1 3 0+", "130.4 ms".
- 282.
- 305 `five.size() == 5`; 342 to 347: one end "3" at 215040, finish 706180; 354 to 360: "0+" 257280 finish 692360, "0-" 253440 finish 706180.
- 374 `best.size() == 3`; 385 "3-"; 386 `totalscore == 334495`.
- 403, 409, 412 "3- 1", 413 `totalscore == 358235`.
- 445 and 464 "9 1".
- 582 `bf == {706180, 706180, 706180}`; 583 "0 1 3 0- 0"; 585 `next == 199680`; 587 `{149760, 165120, 180480, 199680, 245760, 272640}`; 605 to 607 the six (tick, finish, cost, tier) rows: finishes 705640, 678120, 691800, 706180, 664160, 653220, costs 540, 28060, 14380, 0, 42020, 52960.
- 628 `{199680, 165120}`; 634, 635; 651 the six ticks again; 661 and 666 199680; 662 `== 1` and 673 `== 2` (timing, not the window, but typed counts of a race).

`tests/test_pb_text.cpp`:
- 68, 136, 154, 172 `acts.size() == 5`; 78 and 245760; 81 "2:51.485 · 2 phrases · ends m135 · squeeze out 130.4 ms"; 83; 87 and 88 tick 272640 and "3:20.702 · 2 phrases · ends m150"; 101 to 102 and 109 "m136".
- 118 to 119 the eight offered fill ticks 55680 to 167040; 125 to 126 the order built on them.
- 139 to 143 "m23 → m29", "2 phrases", "m108 → m116", "4 phrases"; 157 to 162 "m132 → m135", "2 phrases", "squeeze out 130.4 ms", "m23 → m29".
- 177 "ends m135 · squeeze out 130.4 ms"; 179 "ends m29".
- 209 `fills.size() >= 4`; 210 to 215 fill indexes 0, 2, 3; 222 `fills.size() > 5`; 223 to 233 indexes 4, 5, 3; 226 tick 72480; 228 "m40"; 246 and 249 and 247 tick 180000, 195360; 252 "m102"; 253 tick 156960.
- 304 "m30 · 0:45.789"; 306 "226,210"; 307 "2,500"; 308 Red; 310 "m38 (228,710)", "m34"; 317 "m38 · 0:58.421"; 319 "228,710", 318 "Phrases saved=3"; 323 "m44"; 337 "m34 · 0:45.435"; 342 "m30 · 0:40.041"; 346 "m38 · 0:50.828"; 349 "m40"; 350 "m20 · 0:30.000"; 354 "m30 · 0:45.789".

`tests/test_saved_figure_fill.cpp`:
- 99 `before.size() == 5`; 110 "1 0+ 0 0"; 111 to 112 `{0.0, 0.0, 0.0, nullopt, 0.0}`; 107 `first.calls == 2`; 108 `charts_total == 1`.
- 143 `after[1] == 0.0`; 164 `figures_of(...)[0] == 0.0`; 189 and 200 "1 2 3+"; 208 `== 0.0`.
- 293 "1 2 3+"; 294 `sqin_count == 1`; 295 `sqout_count == 0`; 300 `-189.83750000002328`.
- 327 `seen.size() == 3`; 333 to 334 the five figures.
- 362 `squeezed > 0`; 389 and 403 `== 3`.

## C records

The shared answers, so each record can be short:

- **Q2 shared (pb_coach and the Path Builder):** `start_window`, `choose_end`, `remove_window`, `finish_for_me`, `coach_list`, `end_choices` own the click rules (`src/pathbuilder/pb_coach.cpp:24-215`); each is a thin call into `search_target` and `best_finish` and the ladder `app::build_ladder`. Callers in `src/`: `DecisionList` in `pb_coach.cpp` (`refresh_list` 462, `click_fill` 591, `select` 643, `finish` 463) and the Path Builder window (`pb_window.cpp:388,437`, from C7). The engine owns the rule; the screen draws the answer. No display rule found.
- **Q3 shared (pb_coach):** Burnout and White Washed have no engine record at any window (`records-engine/` holds seven other charts). It is "claim not checked". What is now needed: `hydra_replay dump` and `schedule` at `--efill 60` of Burnout (`testdata/input/common/Summer Blast _25 Setlist/Tier 4/Green Day - Burnout/notes.mid`) and of White Washed (`testdata/input/common/IB24/T5/August Burns Red - White Washed/notes.mid`), at the Path Builder's settings (default, depth 2). Because C7 asked for dumps at 85 and none exists, a dump at 85 beside it also shows what moved.
- **Q7 shared:** the commit that added `test_pb_coach.cpp` is "Path Builder names the window's end; old refill tests rewritten from the engine's record" in part (`4ace207d`, subject only) and the old record names commit `a73e2097` (PB-2a, message only); no failure is named in either. The failure is named below per record.

### 1. coach rows: a two-click path fills the ladder, the rows, the ends and Finish the rest for me (tests/test_pb_coach.cpp:38)
Delta round: window, replaces records/C7.md record 1
Read: lines 38-97; `start_window`, `coach_list`, `end_choices`, `finish_for_me`, `build_ladder` callers above; the old record; `pb_open_util.h`, `scratch_settings.h`, `pb_state.cpp:15-47`.
Does the old verdict hold at 60: Rewrite stays. Its reason is in the test (typed snapshots), not in the chart. It cannot become Keep: the numbers it types are unproven at 60.
1 Claim: On Burnout, after the player clicks the best path's second and third fills, the decision list shows two rows costing 13,640 (red) and 0 (green), the first window has one end choice, and Finish gives the path "9 1".
2 Owner: shared answer above. Display-only parts: `list.rows[i].tier` is `app::cost_tier`.
3 Real chart: claim not checked. Needs Burnout at `--efill 60`: its best path's activations 1 and 2 (ticks, ends), and the ladder `bf` list.
4 Direct: 54 to 55 property. 66, 67, 68, 74, 77, 88, 89, 96 are typed engine values (13640, 364675, 378315, "9 1", 128640). 69, 79, 81, 82, 94, 95 properties.
5 Reason: every number is "from one run" (65 comment). The skip cost 13,640 has its working only in the comment: 378315 - 364675.
6 Display: Fails; expected values typed.
7 Failure: a click pins the wrong window, or the ladder and the rows disagree on a window's cost; named by me, reasoned.
8 Deleted: 66 (`bf` list) any change at all; 69 (`player_total` against `totalscore`): `build_ladder` returning a total another place than the path's, reasoned; 95: `finish_for_me` returning `best[n]` for the wrong `n`, reasoned; 87 to 89: `end_choices` returning a duplicate end, reasoned.
9 Stand-in: typed totals stand in for "the ladder is consistent".
10 Same rule: `tests/pb_ui/pbtest_coach.cpp:101-102` pins "9 1" and 364675 through the GUI; line 464 below pins "9 1" again.
11 Real: yes, Burnout is a corpus song.
Verdict: Rewrite. Take each row's cost and tier from `app::cost_list` and `cost_tier` on the ladder the test built; type no number but zero. Window finding: 66, 67, 74, 88, 89 move if the best path at 60 differs.

### 2. coach click: an impossible click ... (test_pb_coach.cpp:99), B
Not a C record. The old record C7 2 was Investigate, claim not checked (tick 0 holds no fill). The B reason is above. The unread part of that claim is that tick 0 holds no fill on Burnout; that stays "claim not checked", and it is not window-dependent (a fill refused at 85 is refused at 60).

### 3. coach rows: a window over the Path limit carries the tight-window note (tests/test_pb_coach.cpp:133)
Delta round: window, replaces records/C7.md record 4
Read: lines 133-176; `within_ms_limit` (`model.cpp:532-536`, which reads `e_window_ms`); `tight_window_note`; the old record.
Does the old verdict hold at 60: Rewrite stays, and a second window finding is added: `within_ms_limit` reads the window, so the row that is "over the limit" is the one whose figure is over 10 ms, and an early fill's figure at 60 is not what it was at 85.
1 Claim: On White Washed at a 10 ms Path limit, Hydra's best path's fourth window is over the limit and its row says "needs 130.4 ms, your limit is 10 ms"; the same window on the player's own four-window path says the same, and the other three say nothing.
2 Owner: shared; the note is `tight_window_note` (pb_coach.h), a thin word-maker over `Activation::within_ms_limit` and `difficulty`.
3 Real chart: claim not checked. Needs White Washed at `--efill 60`: the best path's five activations, which one is over 10 ms, and its figure.
4 Direct: 141, 145, 154, 159, 175 properties. 144 (5), 162 ("0-"), 163 and 174 ("130.4 ms") are typed engine values. 151 `rows.size() == 5` follows 144.
5 Reason: "from one run" (161).
6 Display: Fails; the note's number is typed.
7 Failure: a window over the Path limit shows no note, or the note names another figure; reasoned.
8 Deleted: 145 (`acts[3].within_ms_limit(10.0)` false): the limit comparison turned around, reasoned. 163 (typed text): the note formatter dropping the limit, reasoned but typed. 175 (first three rows silent): a note on a row inside the limit, reasoned.
9 Stand-in: row index 3 stands in for "the row that is over the limit".
10 Same rule: `tests/test_pb_text.cpp:165,177` pins "130.4 ms" of the same activation; `tests/pb_ui/pbtest_coach.cpp` reads notes through the GUI (not judged here, WN6).
11 Real: yes.
Verdict: Rewrite. Find the tight row by `within_ms_limit`, and take the note's figure from `format_ms(act.difficulty())` and the limit.

### 4. coach rows: a squeeze with time to spare under a lower limit names the limit alone (tests/test_pb_coach.cpp:178)
Delta round: window, replaces records/C7.md record 5
Read: lines 178-197; the old record.
Does the old verdict hold at 60: Rewrite stays. Add: the path "0 0 0 0 0 0+" comes from `allzero_paths` of the 60 run.
1 Claim: On White Washed the all-0 path ends on a squeeze-in with 130.4 ms to spare, so under a limit of -200 the window is over it and the note reads "over your limit of -200 ms"; under -10 it has no note.
2 Owner: shared (`tight_window_note`, `within_ms_limit`).
3 Real chart: claim not checked. Needs White Washed's all-0 path at `--efill 60`: its text, its sixth activation's squeeze-in figure.
4 Direct: 187 and 189 and 191 typed or fixture facts; 193 to 196 are the rule on named inputs -200 and -10 (inputs the test chose).
5 Reason: -200 and -10 are free inputs but the test never says why a limit of -200 exists. The old record asks whether the settings allow a negative limit. Not read here.
6 Display: Fails on 194 (a typed sentence).
7 Failure: a squeeze with time to spare gets a note that wrongly names a timing; reasoned.
8 Deleted: 194: the sentence naming a timing for a squeeze that needs none, reasoned. 196: a note on a window inside the limit, reasoned.
9 Stand-in: the path text stands in for "some activation ends on a squeeze-in with room".
10 Same rule: none found in tests/ for `tight_window_note`.
11 Real: a negative limit may not be settable; not read.
Verdict: Rewrite. Choose the activation by property (it has a squeeze-in with no needed timing), and name the limits from its own figure.

### 5. coach click: a fill that rules out a later window is refused in the later-window sentence (tests/test_pb_coach.cpp:202)
Delta round: window, replaces records/C7.md record 6
Read: lines 199-220; `start_window` and `repin` (`pb_coach.cpp:143-200`); the old record.
Does the old verdict hold at 60: Rewrite stays. A window finding on the typed tick: 36480 is "a fill that can end one way, with which the first window at 59520 cannot happen". That is a statement about a fill's reachability at 85. At 60 the fill can be refused outright: then the click fails at the click ("activation", `failed_tick == clicked`) and the sentence is the generic one, not the later-window sentence, and the test fails.
1 Claim: On Burnout, clicking a fill (tick 36480) before the best path's first window, with the best path's three windows pinned, is refused with "This window makes one of your later windows impossible, so the path stays as it was."
2 Owner: `impossible_window_text` and `repin` (`pb_coach.cpp:54-66,143-200`). The engine's `failed_tick` and `failed_reason` choose the branch.
3 Real chart: claim not checked. Needs Burnout at `--efill 60`: whether tick 36480 is still an offered fill, and the first window's tick.
4 Direct: 210 and 216 to 219 properties of the sentence; 209 types the tick; 208 types 3 windows.
5 Reason: 36480 has "From one run" only.
6 Display: n/a; checks a sentence.
7 Failure: the old code gave the generic sentence (the comment 199 to 201 names it), so a refusal read as "you can't do this here" when the fix is "an earlier choice breaks a later one". Possible still.
8 Deleted: 218 (the sentence): the branch `failed_tick == clicked` inverted, reasoned. 216, 217: the click accepted although it breaks a later window, reasoned.
9 Stand-in: 36480 stands in for "an unused fill before a window of the path that rules it out".
10 Same rule: none found.
11 Real: yes.
Verdict: Rewrite. Pick the fill by property (the last unused fill before the first window that the search refuses), no typed tick, as the old record says.

### 6. coach end: choosing the last window's end pins it (tests/test_pb_coach.cpp:242)
Delta round: window, replaces records/C7.md record 7
Read: lines 222-274; `choose_end`; `end_choices`; the old record.
Does the old verdict hold at 60: Rewrite stays.
1 Claim: On White Washed the best path's fourth window, with the three before it, has two ends: a squeeze-in end at 257280 and a squeeze-out end at 253440; choosing the first pins the fourth window to it and the path reads "0 1 3 0+"; the squeeze-in end needs no timing and the squeeze-out end needs 130.4 ms.
2 Owner: `choose_end`, `end_choices`; `badge_timings` is the engine's own word on timing (`Activation`).
3 Real chart: claim not checked. Needs White Washed at `--efill 60`: the fourth activation's `sp_end_steps` (two ends?) and its squeeze figures.
4 Direct: 249 to 253, 259, 260, 266, 273 are typed; 255 to 258, 267 to 272 are properties (kind, required).
5 Reason: "from one run" (248, 263).
6 Display: Fails on 273 ("130.4 ms").
7 Failure: choosing an end pins another one, or the squeeze-in end reads as needing timing; reasoned.
8 Deleted: 259 (`deact_tick == 257280`): `choose_end` pinning the wrong end, reasoned; 268 (`required` false): the badge for a squeeze-in wrongly needing timing, reasoned; 273: the figure formatter, reasoned.
9 Stand-in: ticks and path text stand in for "the chosen end is the one pinned".
10 Same rule: record 7 below and `test_pb_text.cpp:165`.
11 Real: yes.
Verdict: Rewrite. Pin by property: the chosen end's `deact_tick` equals the end's own window; the squeeze-in end is not `required` and the squeeze-out end is (from `badge_timings`); no typed tick, string or "130.4".

### 7. coach end: the kept end is the window's own end (tests/test_pb_coach.cpp:276)
Delta round: window, replaces records/C7.md record 8
Read: lines 276-294; `is_kept_end`; the old record.
Does the old verdict hold at 60: Investigate stands, claim not checked only.
1 Claim: On White Washed the best path kept the squeeze-out end for its fourth window; once the player chooses the squeeze-in end, that end is the kept one.
2 Owner: `is_kept_end` (pb_coach). 
3 Real chart: claim not checked. Needs White Washed at `--efill 60`: the fourth window's ends and which one the best path took.
4 Direct: 282 `ends.size() == 2` and the index lookups 284, 285, 286, 292, 293 assume the order [squeeze-in, squeeze-out].
5 Reason: the order is "from one run".
6 Display: n/a.
7 Failure: the wrong end marked as the kept one; reasoned.
8 Deleted: 285 (`is_kept_end(four[3], ends[1])`): the kept-end test comparing the wrong field, reasoned. 284: the same for the other end.
9 Stand-in: indexes 0 and 1 stand in for "squeeze-in" and "squeeze-out".
10 Same rule: record 6 above, record 9 below.
11 Real: yes.
Verdict: Investigate (claim not checked only). Window finding: the end order and count (282) can differ at 60.

### 8. coach end: an earlier window's endings are the same list (tests/test_pb_coach.cpp:299)
Delta round: window, replaces records/C7.md record 9
Read: lines 296-364; `DecisionList::select`, `show`, `ends`; the old record.
Does the old verdict hold at 60: Rewrite stays. 
1 Claim: A window's end choices are the same list in the same order with the same finishes whether it is opened as the last window or as an earlier one; on White Washed the third window has one end "3" at 215040 (finish 706180), the fourth has "0+" at 257280 (692360) and "0-" at 253440 (706180).
2 Owner: `DecisionList::select` computes ends when a step is opened (comment 296-298).
3 Real chart: claim not checked. Needs White Washed at `--efill 60`: ends of windows 3 and 4.
4 Direct: `same_list` (321-331) is a property across two ways of opening. 305 (5), 342 to 347 and 354 to 363 are typed.
5 Reason: "from one run".
6 Display: n/a.
7 Failure: an earlier step's ends differ from the list it had as the last window (the comment 296 to 298 names it); still possible.
8 Deleted: 346 and 361 (`same_list`): ends stored and not recomputed, reasoned. 344, 345, 356, 357, 359, 360 are "any change" snapshots, no named bug.
9 Stand-in: ticks and finishes stand in for "the same list".
10 Same rule: records 6, 7.
11 Real: yes.
Verdict: Rewrite. Keep the lambdas `ends_when_opened`, `same_list` and `kept_count`; drop the typed ticks, strings and finishes (342 to 345, 354 to 360).

### 9. coach remove: removing a window drops it and every later one (tests/test_pb_coach.cpp:368)
Delta round: window, replaces records/C7.md record 10
Read: lines 366-395; `remove_window` (`pb_coach.cpp:191-200`); the old record.
Does the old verdict hold at 60: Rewrite stays. 386's 334495 and 385's "3-" are the best path's first window's own score at 60.
1 Claim: On Burnout, removing the second window leaves the first with its end and the path "3-" worth 334495; removing the first leaves nothing.
2 Owner: `remove_window`.
3 Real chart: claim not checked. Needs Burnout at `--efill 60`.
4 Direct: 383, 384 property; 385, 386 typed; 391 to 394 properties.
5 Reason: "from one run" (376).
6 Display: n/a.
7 Failure: a removal that keeps a later window; reasoned.
8 Deleted: 382 (`size() == 1`): the removal dropping only one window, reasoned. 386: any change at all.
9 Stand-in: the score stands in for "the path still ends where the window ended".
10 Same rule: record 10 below.
11 Real: yes.
Verdict: Rewrite. Drop 385 and 386; compare the path's activation ticks to the kept windows' ticks (383, 384 already state it).

### 10. coach remove: removing every window leaves the empty path (tests/test_pb_coach.cpp:397)
Delta round: window, replaces records/C7.md record 11
Read: lines 397-425; the old record.
Does the old verdict hold at 60: Rewrite stays.
1 Claim: Removing windows last to first on Burnout's best path keeps the earlier windows' ends, ends at no windows, and a window left always has a path.
2 Owner: `remove_window`.
3 Real chart: claim not checked. Needs Burnout at `--efill 60`.
4 Direct: 410, 411, 414 to 423 properties; 403, 409, 412 ("3- 1") and 413 (358235) typed.
5 Reason: "from one run".
6 Display: n/a.
7 Failure: the path of a shorter window list missing or kept; reasoned.
8 Deleted: 423 (`path.has_value() == !windows.empty()`): PlayerPath rule changed, reasoned. 413 any change.
9 Stand-in: none beyond 412 to 413.
10 Same rule: record 9.
11 Real: yes.
Verdict: Rewrite. Delete 412 and 413; the rest is the property.

### 11. coach remove: a tick with no window changes nothing (tests/test_pb_coach.cpp:427), B
Not a C record. Old record C7 12 was Investigate (claim not checked, "needs NEED-B to show tick 0 is no window of any path it lists"). The B reason: `remove_window` returns `{}` before any search when no window has the tick (`pb_coach.cpp:195-198`), and no window of the best path sits on tick 0 at any window setting because a window's tick is a fill tick and a smaller window only removes fills.

### 12. coach finish: Finish the rest for me makes BF(n)'s path the player's path as one edit (tests/test_pb_coach.cpp:439)
Delta round: window, replaces records/C7.md record 13
Read: lines 439-467; `DecisionList::finish`, `can_finish`; the old record.
Does the old verdict hold at 60: Rewrite stays (remove line 464). 464's "9 1" is a typed path of the 60 search, where the best path's second window is the first click.
1 Claim: After one click on Burnout, Finish the rest for me adds one edit to the player's history holding the path behind BF(1) and tells the screen to show it.
2 Owner: `DecisionList::finish` and `finish_for_me`.
3 Real chart: claim not checked. Needs Burnout at `--efill 60`.
4 Direct: 459 to 463, 465, 466 properties. 464 typed "9 1". 445 `best.size() >= 2` floor.
5 Reason: 464 "9 1" "from one run" by the sister test.
6 Display: n/a.
7 Failure: Finish making two edits or none, or showing a different path; reasoned.
8 Deleted: 461 (`generation == before + 1`): two edits, reasoned. 463: the path not BF(1)'s, reasoned. 464: nothing (463 and record 1 catch it).
9 Stand-in: none.
10 Same rule: record 1 (`finish_for_me` at 92 to 96) and `pbtest_coach.cpp:101-102`.
11 Real: yes.
Verdict: Rewrite. Remove 464; the rest stays.

### 13. candidate costs: the best next fill costs 0 and every other cost is BF(n) minus its best finish (tests/test_pb_coach.cpp:572)
Delta round: window, replaces records/C7.md record 17
Read: lines 541-617; `price_fills`, `candidate_costs`, `Coach::request_candidates`; the old record.
Does the old verdict hold at 60: Rewrite stays, with a worse window finding: the six fill ticks of 587 and the table 605 to 607 are the set of fills the search offered after two windows at 85. At 60 a fill in that set can be refused, so `got.size() == ticks.size()` (597) and the table can fail even though `candidate_ticks` reads the ticks from the run (559 to 568): the table is typed independently.
1 Claim: On White Washed with two windows pinned, each candidate fill's cost is BF(2) minus its best finish, the best next fill costs 0, and tiers follow.
2 Owner: `app::candidate_costs` through `DecisionList::price_fills`.
3 Real chart: claim not checked. Needs White Washed at `--efill 60`.
4 Direct: 585 and 615 property; 582, 583, 587, 605 to 607 typed.
5 Reason: "from one run" (580, 598).
6 Display: Fails; the table is typed.
7 Failure: a candidate priced against the wrong BF or in the wrong order; reasoned.
8 Deleted: 615 (`(cost == 0) == (act_tick == next)`): can fail; it catches a zero cost on the wrong fill. 613 (`cost`): the BF subtraction wrong; reasoned but typed. 614: `cost_tier` thresholds moved, reasoned but typed.
9 Stand-in: the table stands in for the rule in the test's own name.
10 Same rule: records 14 and 15 below.
11 Real: yes.
Verdict: Rewrite. Require `cost == bf[2] - best_finish` read from the ladder and the candidate; cost 0 exactly at the next activation; the tier equals `cost_tier` of the cost; no table.

### 14. candidate queue: it prices only the ticks it is given, in order (tests/test_pb_coach.cpp:619)
Delta round: window, replaces records/C7.md record 18
Read: lines 619-643; `request_candidates`; the old record.
Does the old verdict hold at 60: Rewrite stays. Window finding: 199680 and 165120 are typed fill ticks; if either is refused at 60, `best_finish` returns no paths (`REQUIRE_FALSE(bf.paths.empty())` at 640) or the queue drops it (`got.size() == 2` at 633).
1 Claim: The queue prices only the two fills it is given, returned in chart order, and each best finish equals a fresh `best_finish`.
2 Owner: `Coach::request_candidates`.
3 Real chart: claim not checked. Needs White Washed at `--efill 60`: are 165120 and 199680 offered fills after two windows.
4 Direct: 629 (the same ticks start nothing), 634 and 635 (typed order), 641 (property, against a fresh `best_finish`).
5 Reason: the two ticks are typed and unexplained.
6 Display: n/a.
7 Failure: a queue that prices a tick it was not given, or in handed order; reasoned.
8 Deleted: 634 and 635: sort order dropped, reasoned. 641: a finish that is not `best_finish`, reasoned.
9 Stand-in: none.
10 Same rule: records 13 and 15.
11 Real: yes.
Verdict: Rewrite. Take the two ticks from `candidate_ticks(run)` (the helper at 559), not typed.

### 15. candidate queue: a newer request supersedes the older (tests/test_pb_coach.cpp:645)
Delta round: window, replaces records/C7.md record 19
Read: lines 645-677; `request_candidates`, `candidates_superseded`; the old record.
Does the old verdict hold at 60: Rewrite stays. Window finding: `all` (651) is a typed six-tick list; at 60 any refused tick changes what is priced.
1 Claim: A newer set of ticks cancels the queue running and only the new ticks are priced; a newer path cancels the queue and prices nothing until new ticks come.
2 Owner: `Coach::request_candidates`, `request`.
3 Real chart: claim not checked. Needs White Washed at `--efill 60`.
4 Direct: 662 (`== 1`) and 673 (`== 2`) are counts true only when the queue is still on its first search; 660 and 671 prove a job exists, not that it will not finish first. 665 and 666 typed (199680).
5 Reason: the six ticks are typed.
6 Display: n/a.
7 Failure: an old queue's prices arriving after a newer request; reasoned.
8 Deleted: 666 (only the new tick priced), 675 (nothing priced after a new path): stale prices shown, reasoned. 662 and 673: the cancel not counted, reasoned but a race.
9 Stand-in: counts stand in for "the older was cancelled".
10 Same rule: record 14, and `coach worker` (516, B).
11 Real: yes.
Verdict: Rewrite. Assert what holds in both orders (only the newest ticks are priced; none after a new path) and take the ticks from `candidate_ticks`.

### 16. candidate row: the clock, phrases, end and squeeze of a priced fill (tests/test_pb_text.cpp:56)
Delta round: window, replaces records/A19.md record PB2
Read: lines 56-110; `candidate_row_text`, `fill_refusal`, `step_short` (pb_text.h); `scene_for`, `player_path`; the old record; the file's own comment about the squeezed-out chord on tick 253320.
Does the old verdict hold at 60: Rewrite stays. Window finding: the text "2:51.485 · 2 phrases · ends m135 · squeeze out 130.4 ms", the tick 245760 of the fill and 253320 in the comment are all from the 85 search.
1 Claim: On White Washed with three windows pinned, the fourth fill's row reads its clock, 2 phrases, the end measure of the candidate activation and "squeeze out 130.4 ms"; on the best path's last fill the row stops at the end; a hand-built activation reads its squeezes as the Paths tab's badges do.
2 Owner: `candidate_row_text` (pb_text.h) is the one place that words the row; it reads the badges (`activation_badges`). Callers in `src/pathbuilder` (the decision list). The words are the display's; the activation's values are the engine's.
3 Real chart: claim not checked. Needs White Washed at `--efill 60`: fourth activation's end tick, its squeeze-out, and fill tick 245760 as an offered fill.
4 Direct: 79 and 83 properties; 81 and 87 to 88 typed; 100 to 109 are hand-built inputs that name their meaning in comments (early fill 0.0 ms, -0.001 and 0.001 offsets).
5 Reason: "pinned from one run".
6 Display: Fails on 81 and 87 to 88: typed text from the engine's run.
7 Failure: a row that names the stored end instead of the squeezed-out chord, or drops the badge words; reasoned.
8 Deleted: 81 (end measure): `window_end_tick` replaced by the stored end, reasoned. 83: the pending text, reasoned. 100: badge words dropped, reasoned. 108: a zero-offset squeeze reading "-0.0", reasoned (D302).
9 Stand-in: the typed row stands in for "the row reads the record".
10 Same rule: `tests/test_pb_text.cpp:165` (end_choice_text) and `:146`.
11 Real: yes for 81; 100 to 109 are hand-built on purpose.
Verdict: Rewrite. Read the end measure, phrase count and squeeze from the record's activation (`window_end_tick`, `sqinouts`) and drop the typed strings. A hand-built block (96 to 109) stays.

### 17. choice order: cheapest first, ties in chart order, pending last (tests/test_pb_text.cpp:112)
Delta round: window, replaces records/A19.md record PB3
Read: lines 112-129; `choice_order`; the old record.
Does the old verdict hold at 60: Rewrite stays, with a worse reason: the `REQUIRE(offered == {55680, ..., 167040})` at 118 stops the case if the engine's offered list for Overrated differs at 60.
1 Claim: Offered fills sort cheapest first, ties in chart order, unpriced last.
2 Owner: `choice_order` (pb_text.h).
3 Real chart: claim not checked for the chart. The order rule is made-up costs; the real chart only supplies the tick list. Needs Allister "Overrated" at `--efill 60`: `trailing_skipped_fill_ticks` of the no-window path.
4 Direct: 125 to 126 and 128 compare with a typed order built on the typed ticks.
5 Reason: the costs are "made up" (114), the ticks are from one run.
6 Display: n/a; the order rule is an owner's.
7 Failure: ties re-ordered, or pending fills not last; reasoned.
8 Deleted: 126 (the order): the sort unstable or the pending fills first, reasoned. 118: nothing (a fixture guard that moves with the window).
9 Stand-in: eight real ticks stand in for "a list with a tie and two pending".
10 Same rule: none found.
11 Real: the order rule is real; the tick list need not come from a chart.
Verdict: Rewrite. State the property on a small made-up list of ticks; no real chart ticks.

### 18. step short: start, end and phrases (tests/test_pb_text.cpp:131)
Delta round: window, replaces records/A19.md record PB4
Read: lines 131-144; `step_short`; the old record (verdict text "Rewrite." only).
Does the old verdict hold at 60: Rewrite stays.
1 Claim: On White Washed's best path the first step reads "m23 → m29" with 2 phrases and the third "m108 → m116" with 4 phrases.
2 Owner: `step_short` (pb_text.h) words the span from the activation's act and deact ticks and `timing`.
3 Real chart: claim not checked. Needs White Washed at `--efill 60`.
4 Direct: 140 to 143 typed measures.
5 Reason: none beyond the run.
6 Display: Fails; typed measures.
7 Failure: the end measure computed from the wrong tick; reasoned.
8 Deleted: 139 and 142 (spans): `step_short` using the stored end instead of the activation's end, reasoned; 140 and 143: phrase count taken wrong, reasoned.
9 Stand-in: measures stand in for "the span is the activation's own ticks".
10 Same rule: records 19, 20.
11 Real: yes.
Verdict: Rewrite. Compute the expected span from the record's activation ticks and the chart's own measure map; do not type measures.

### 19. step short: the row carries the Paths tab's badge words (tests/test_pb_text.cpp:146)
Delta round: window, replaces records/A19.md record PB5
Read: lines 146-163; the old record.
Does the old verdict hold at 60: Rewrite stays.
1 Claim: On White Washed's best path the fourth step spans "m132 → m135" with 2 phrases and "squeeze out 130.4 ms"; the first has no squeeze words.
2 Owner: `step_short`; `activation_badges` words them.
3 Real chart: claim not checked. Needs White Washed at `--efill 60`.
4 Direct: 157 to 162 typed.
5 Reason: "pinned from one run" (149).
6 Display: Fails.
7 Failure: the squeeze words differ from the Paths tab's badges; reasoned.
8 Deleted: 159 (squeeze words): `step_short` building its own words, reasoned; 162: a first step with a spurious squeeze, reasoned.
9 Stand-in: typed words stand in for "the same words as the Paths tab".
10 Same rule: `test_pb_text.cpp:196` compares to `activation_badges` directly.
11 Real: yes.
Verdict: Rewrite. Compare `fourth.squeezes` with the joined `activation_badges(acts[3])` and the end with the record's squeezed-out chord tick.

### 20. end choice text: the end measure and the Paths tab's badge words (tests/test_pb_text.cpp:165)
Delta round: window, replaces records/A19.md record PB6
Read: lines 165-201; `end_choice_text`; the old record.
Does the old verdict hold at 60: Rewrite stays. Lines 193 to 200 are hand-built and name their meaning; those keep.
1 Claim: On White Washed's fourth window the end text reads "ends m135 · squeeze out 130.4 ms", the first "ends m29"; hand-built ends read "early fill 0.0 ms" first and the squeeze badge.
2 Owner: `end_choice_text`; words from `activation_badges`.
3 Real chart: claim not checked. Needs White Washed at `--efill 60`.
4 Direct: 177 and 179 typed; 189, 192, 196, 200 hand-built; 195 to 196 compare to `activation_badges`.
5 Reason: typed text "pinned from one run"; hand-built ones explain themselves.
6 Display: Fails on 177 and 179; 196 compares to the owner.
7 Failure: the early-fill badge dropped or put after the squeeze; reasoned.
8 Deleted: 196 (the badge list): words differing from the tab's, reasoned. 177: the window-end tick replaced by the stored end, reasoned.
9 Stand-in: none.
10 Same rule: records 16, 19.
11 Real: hand-built ends stand for ends with no timing (D302).
Verdict: Rewrite. Keep 195 to 196; read the rest from the record.

### 21. fill refusal: the five kinds in fill_reason's order, and fill_reason's sentence for each (tests/test_pb_text.cpp:203)
Delta round: window, replaces records/A19.md record PB7
Read: lines 203-254; `fill_refusal`, `fill_reason`; `build_preview_scene` fill states (`preview_view.cpp:415-494`); the old record.
Does the old verdict hold at 60: Rewrite stays. This is the case most exposed to the window. `FillRefusal::GameHides` (line 212) is the engine not offering a fill: the fill states come from `trailing_skipped_fill_ticks`, so at 60 the fills offered and hidden change. And `player_path(song, {180000})` (246) fails its `REQUIRE(tr.paths.size() >= 1)` (35) if a window at 180000 is not reachable at 60.
1 Claim: On Trivium "Feast of Fire" with no activation, fill 0 is under bars, fill 2 is hidden by the game, fill 3 is offered; on the best path fills 4 (taken), 5 (Star Power running until m40), 3 (before the last) and 0 (before the last, although hidden) read their sentences; a fill on the SP end tick is still under Star Power.
2 Owner: `fill_refusal` and `fill_reason` (pb_text.h). The kinds come from the Preview scene's fill states, which come from the engine's `trailing_skipped_fill_ticks`.
3 Real chart: claim not checked. Needs Feast of Fire at `--efill 60`: its fill ticks, which are offered with no window pinned, the best path's activation and end ticks (72480, 180000, 195360, 156960).
4 Direct: 210 to 215 and 223 to 241 are index-based expectations; 226, 246, 249, 253 typed ticks; 228 and 252 typed measures.
5 Reason: indexes and ticks are "from one run".
6 Display: partly; `fill_reason` is the display's own sentence, its expected side typed.
7 Failure: a fill reads the wrong reason (for example a fill on the SP end tick read as "before your last activation"; comment 242 to 245).
8 Deleted: 227 (SpRunning): a fill inside a window offered; reasoned. 250 and 252 (the end-tick boundary): `after_sp_end` at the boundary, reasoned (the comment at 251 names core::after_sp_end).
9 Stand-in: fill indexes stand in for "a fill in each state".
10 Same rule: `tests/test_pb_text.cpp:326` (record 25) uses the same chart and fills.
11 Real: yes.
Verdict: Rewrite. Find fills by their state in the record, not by index; keep the end-tick boundary line as a property.

### 22. fill card: an offered fill reads the heading, three rows and two grey lines (tests/test_pb_text.cpp:298)
Delta round: window, replaces records/A19.md record PB8
Read: lines 256-311; `fill_hover_card`; `candidate_costs`; the old record.
Does the old verdict hold at 60: Rewrite stays. Window finding: 226,210, 228,710 and 2,500 are scores of searches at 85.
1 Claim: On Allister "Overrated" with no activation, hovering the m30 fill reads "Phrases saved=2", "Best finish here=226,210", "Cost=−2,500" in red, and "Best choice now is m38 (228,710)." and "Hydra would end it at m34."
2 Owner: `fill_hover_card` words it; `candidate_costs` owns the cost.
3 Real chart: claim not checked. Needs Overrated at `--efill 60`: BF(0), the best finish with the m30 fill pinned, the best path's first fill.
4 Direct: 304 to 310 typed.
5 Reason: "Pinned from one run".
6 Display: Fails.
7 Failure: the cost row shows the wrong sign or the best-choice line names the wrong fill; reasoned.
8 Deleted: 307 (cost): the subtraction reversed, reasoned. 310: the best-fill name taken from the wrong fill, reasoned.
9 Stand-in: typed finishes stand in for "cost = BF(0) minus this fill's finish".
10 Same rule: record 23.
11 Real: yes.
Verdict: Rewrite. Take the cost from the two searches' scores the test itself ran (`bf0 - with_total`).

### 23. fill card: a costless fill reads best and "This is the best choice now." (tests/test_pb_text.cpp:313)
Delta round: window, replaces records/A19.md record PB9
Read: lines 313-324; the old record.
Does the old verdict hold at 60: Rewrite stays. The same card: 228,710 is BF(0) at 85.
1 Claim: The fill Hydra's best path starts on reads "Cost=best", "Best finish here=228,710", green, "This is the best choice now." and "Hydra would end it at m44."
2 Owner: `fill_hover_card`.
3 Real chart: claim not checked. Needs Overrated at `--efill 60`.
4 Direct: 317 to 323 typed.
5 Reason: from one run.
6 Display: Fails.
7 Failure: a costless fill read as a cost; reasoned.
8 Deleted: 319 ("Cost=best"): the zero cost worded as a figure, reasoned.
9 Stand-in: none.
10 Same rule: record 22.
11 Real: yes.
Verdict: Rewrite (as record 22): `Cost=best` when the search's score for the fill equals `bf0`, by the test's own run.

### 24. fill card: taken, locked, refused and pending fills read the heading and their one line (tests/test_pb_text.cpp:326)
Delta round: window, replaces records/A19.md record PB10
Read: lines 326-365; the old record.
Does the old verdict hold at 60: Rewrite stays.
1 Claim: On Trivium "Feast of Fire"'s best path, fill 4 reads "m34 · 0:45.435" and the your-activation line, fill 3 "m30 · 0:40.041" and the before-last line, fill 5 "m38 · 0:50.828" and "Star Power is already on here. Activation 1 runs until m40."; on Overrated with no activation fill 0 reads "m20 · 0:30.000" and the under-bars line, and the fill at 55680 is pending.
2 Owner: `fill_hover_card`; the heading's measure and clock come from the chart's timing.
3 Real chart: claim not checked. Needs Feast of Fire and Overrated at `--efill 60`.
4 Direct: 337 to 365 typed headings, measures and counts.
5 Reason: the file says "pinned from one run".
6 Display: Fails.
7 Failure: a taken fill reads as an offered one; reasoned.
8 Deleted: 340, 344 (the sentences): the constants swapped, reasoned. 337: heading measure, reasoned.
9 Stand-in: indexes for states.
10 Same rule: record 21.
11 Real: yes.
Verdict: Rewrite. Heading measure and clock from the chart's own timing for the fill's chord tick (an independent source); sentences from the owner's constants.

### 25. a saved path with a squeeze-in on the SP end gets its figure back (tests/test_saved_figure_fill.cpp:93)
Delta round: window, replaces records/A27.md record SF1
Read: lines 93-119; `save_as_old_build` (`store_fixtures.h:73-84`); `fill_saved_hardest`, `chart_figures`, `replay_matches` (`saved_figure_fill.cpp:28-34,86-110`); the old record; `records-engine/believer.dump.json`.
Does the old verdict hold at 60: Rewrite stays. The saved path text "1 0+ 0 0" matches Believer path 0 of the 85 dump; whether it is still rank 1 at 60 is unproven.
1 Claim: An older build's rows for Smash Mouth "I'm A Believer" with empty figures on ranks 1 to 3 get their figures back on the first run, which reports before and after one chart, and the second run reads no chart.
2 Owner: `fill_saved_hardest`. Caller `library_jobs.cpp:207`. Not a display.
3 Real chart: Engine, Believer at 85 (`believer.dump.json`): ranks 1 to 3 "1 0+ 0 0", "1 0+ 2", "1 0+ 1". Claim at 60: rank 1 "1 0+ 0 0", figures {0.0, 0.0, 0.0, none, 0.0} over five ranks. Not checked at 60. Needs `hydra_replay dump --efill 60` of Believer and a schedule of its ranks.
4 Direct: 100 to 102 (emptied precondition) and 113, 118 properties; 99 (5), 107, 108, 110, 111 to 112 typed.
5 Reason: "pinned from one run".
6 Display: not a display.
7 Failure: commit 9215a072 "Fill old rows' hardest figure once per profile at open, by replaying their saved paths" (subject only): old rows showed no squeeze figure. Still possible.
8 Deleted: 111 (any wrong figure written), reasoned; 113 (marker not set so every open re-reads), reasoned; 118 (marker ignored), reasoned; 100 to 102 redundant with 111; 99 guards the fixture.
9 Stand-in: a literal vector stands in for "each emptied row gets the figure a fresh save gave".
10 Same rule: records 26, 27, 30 below and SF9/SF10 repeat the 0.0 figures; 333 to 334 repeat 111 to 112.
11 Real: yes, any library saved by an older build.
Verdict: Rewrite. For each emptied rank the figure after the fill equals the figure a fresh save holds, the untouched ranks are unchanged, and the path text and the count are not typed.

### 26. a row that disagrees with its replay keeps its empty figure (tests/test_saved_figure_fill.cpp:133)
Delta round: window, replaces records/A27.md record SF3
Read: lines 133-145; `replay_matches`; the old record.
Does the old verdict hold at 60: Investigate stands, claim not checked only. Window finding: 143 (`after[1] == 0.0`) holds only if Believer's rank 2 carries a squeeze-in with figure 0.0 at 60. At 85 rank 2 is "1 0+ 2" (a squeeze-in, `believer.dump.json` index 1).
1 Claim: A row whose score no longer matches its replay keeps its empty figure while the next rank, which still matches, gets its figure.
2 Owner: `replay_matches` (`saved_figure_fill.cpp:28-34`).
3 Real chart: claim not checked at 60 (agrees at 85 for rank 1 only). Needs Believer's rank 2 at `--efill 60` and its squeeze figure.
4 Direct: 142 (empty rank 1) property; 143 typed 0.0; 144 property.
5 Reason: 0.0 is the engine's figure at 85.
6 Display: n/a.
7 Failure: a mismatched row filled from a replay of another path; reasoned.
8 Deleted: 142: the fill ignoring a mismatch, reasoned. 143: the fill failing to go on to rank 2, reasoned.
9 Stand-in: 0.0 stands in for "the figure a fresh save holds".
10 Same rule: record 25.
11 Real: a scoring change without a stamp bump (old record).
Verdict: Investigate (claim not checked only).

### 27. a stop leaves the marker unset (tests/test_saved_figure_fill.cpp:147)
Delta round: window, replaces records/A27.md record SF4
Read: lines 147-166; the old record.
Does the old verdict hold at 60: it was Keep. It cannot stay Keep: 164 types 0.0, which agreed with the 85 record of Believer (the old record says so, `believer.schedule.json:96`), and no record exists at 60. A Keep with "claim not checked" is not allowed.
1 Claim: When the progress callback answers stop, the fill returns false, leaves the marker unset and changes no figure; the next run finishes and rank 1 holds 0.0.
2 Owner: `fill_saved_hardest` (stop at `:128,148`).
3 Real chart: Engine, Believer at 85: rank 1 figure 0.0. Claim at 60: the same. Unproven at 60. Needs `hydra_replay dump --efill 60` of Believer.
4 Direct: 155 to 158, 162, 163, 165 properties. 164 typed 0.0.
5 Reason: 0.0 is the engine's figure at 85 only.
6 Display: n/a.
7 Failure: a stopped fill marking itself finished; still possible.
8 Deleted: 155, 156, 157, 158, 162, 165 as in the old record (all can fail, reasoned); 164: a resume that writes another figure, reasoned.
9 Stand-in: none.
10 Same rule: the six-chart stop and resume at 212 (B).
11 Real: yes.
Verdict: Investigate (claim not checked only), because of 164. The rest was already right.

### 28. a chart whose replay throws keeps its rows empty, and the fill goes on (tests/test_saved_figure_fill.cpp:168)
Delta round: window, replaces records/A27.md record SF5
Read: lines 168-210; the old record.
Does the old verdict hold at 60: Rewrite stays. Window finding: `throwing_text = "1 2 3+"` is Everlong's rank 1 text at 85 (`CHECK` at 200 compares the stored text to it); if the stored text at 60 differs, `thrown` is 0 and 204 fails.
1 Claim: A chart whose replay throws keeps all its rows empty and the fill goes on to other charts and sets the marker.
2 Owner: `chart_figures` (catches all, `saved_figure_fill.cpp:101-109`).
3 Real chart: claim not checked for Everlong (no engine record at any window). Smash Mouth rank 1 0.0 agrees at 85. Needs Everlong and Believer at `--efill 60`.
4 Direct: 204, 206, 207, 209 properties; 189, 200, 208 typed.
5 Reason: the text is typed.
6 Display: n/a.
7 Failure: one chart's exception stopping the whole fill (commit 3cb753e0, subject only). Still possible.
8 Deleted: 204 (`thrown == 1`): the replay called for another text, reasoned. 206 and 207: rows written for a skipped chart, reasoned. 208: the fill stopping after the first chart, reasoned.
9 Stand-in: a typed text stands in for "one stored path text".
10 Same rule: none.
11 Real: a replay that throws after a search change; injected here.
Verdict: Rewrite. Pick the throwing row as Everlong's rank 1 text read from the store, not typed (the old record).

### 29. an inside-Star-Power squeeze-in fills a negative figure that shows as none (tests/test_saved_figure_fill.cpp:286)
Delta round: window, replaces records/A27.md record SF7
Read: lines 286-302; `shown_timing` (`model.h`); the old record.
Does the old verdict hold at 60: Rewrite stays. 293 to 295 and 300 are all typed engine values on Everlong (no record at any window); the exact double -189.83750000002328 is the figure of one squeeze at one end.
1 Claim: On Everlong, a saved path "1 2 3+" with its squeeze-in inside Star Power fills a figure below zero which `shown_timing` shows as none.
2 Owner: `engine_hardest_ms` and `shown_timing`; the screen draws `shown_timing`'s answer.
3 Real chart: claim not checked. Needs Everlong's schedule at `--efill 60` for the squeeze and the Paths tab text.
4 Direct: 293 to 295, 300 typed; 301 property.
5 Reason: "pinned from one run"; the double's tail has no reason.
6 Display: partly; 301 asks the display's rule on a typed figure.
7 Failure: a negative figure (squeeze-in before the end) shown as a timing. Still possible.
8 Deleted: 300 (any change, snapshot); 301: a negative timing shown, reasoned; 293 to 295 guard the fixture.
9 Stand-in: the exact figure stands in for "negative".
10 Same rule: `shown_timing` in tests/ not grepped in this slice.
11 Real: yes.
Verdict: Rewrite. The figure is below zero (a squeeze-in inside Star Power is early) and `shown_timing` of it has no value; read the figure from the engine's record, not typed.

### 30. the fill hands every row of one chart the same graph (tests/test_saved_figure_fill.cpp:308)
Delta round: window, replaces records/A27.md record SF8
Read: lines 304-335; the old record.
Does the old verdict hold at 60: Rewrite stays (drop 332 to 334). 327 (`seen.size() == 3`) requires ranks 1 to 3 to be three rows with a squeeze-in; at 85 Believer ranks 1 to 3 are, at 60 unproven.
1 Claim: The fill builds one graph per chart and replays every row on it.
2 Owner: `chart_figures` builds the graph once (`saved_figure_fill.cpp:86-110`).
3 Real chart: claim not checked at 60. Needs Believer at `--efill 60`.
4 Direct: 328 to 331 property; 327 typed 3; 333 to 334 typed figures.
5 Reason: the three rows are Believer's.
6 Display: n/a.
7 Failure: a graph rebuilt per row; reasoned.
8 Deleted: 328, 329 (address equality): a graph per row, reasoned (comment 313 to 316 says it is a sign, not a proof).
9 Stand-in: addresses stand in for "built once".
10 Same rule: record 25 repeats 333 to 334.
11 Real: yes.
Verdict: Rewrite. Drop 332 to 334 (record 25 owns them). For "built once", count graph constructions directly; that is a production counter and a code change.

### 31. a result this build saves holds a saved figure on every path with a squeeze-in (tests/test_saved_figure_fill.cpp:341)
Delta round: window, replaces records/A27.md record SF9
Read: lines 337-364; the old record.
Does the old verdict hold at 60: Investigate stands, claim not checked only. Window finding: the floor `squeezed > 0` (362) rests on Believer or Everlong having a stored path with a squeeze-in at 60.
1 Claim: Every path of a saved result with a squeeze-in has its figure, and the store finds no row missing a figure.
2 Owner: the store (`PathRow::hardest_ms`, `rows_missing_hardest`).
3 Real chart: Engine, Believer at 85: ranks 1 to 3 hold squeezes; agrees. Everlong: no record. Needs Everlong and Believer at `--efill 60`.
4 Direct: 359 (property, per path), 363 (property); 362 is a floor.
5 Reason: the floor stands in for "the loop checked something".
6 Display: n/a.
7 Failure: a fresh save leaving a figure empty for the fill to find; reasoned.
8 Deleted: 359: `hardest_ms` unset on a squeeze path, reasoned. 363: a missing row, reasoned. 362: the loop running on no squeeze path (a floor).
9 Stand-in: `squeezed > 0` stands in for "the loop ran". Check it directly: require that the named charts were skipped zero times.
10 Same rule: none.
11 Real: yes.
Verdict: Investigate (claim not checked only).

### 32. a row under an older stamp is left alone by the fill (tests/test_saved_figure_fill.cpp:371)
Delta round: window, replaces records/A27.md record SF10
Read: lines 366-404; the old record.
Does the old verdict hold at 60: Rewrite stays (the old record says: compare the empty-row count after with the count before). Window finding: 389 and 403 type 3, which is the number of emptied ranks (384) and also the number of rows the SQL matches (`sqin_count > 0`). If rank 2 or 3 has no squeeze-in at 60, 389 fails first (`REQUIRE`), before the fill is tried.
1 Claim: A row saved under a stamp this build does not accept is never read by the fill: no chart read, no progress call, no replay, and the figure stays empty.
2 Owner: `rows_missing_hardest`'s stamp filter (store). Caller `library_jobs.cpp:207`.
3 Real chart: Engine, Believer at 85: ranks 1 to 3 all carry a squeeze-in (`believer.dump.json`), so the SQL matches three. At 60 unproven.
4 Direct: 387 (Stale), 389 (3), 397, 398, 401 to 403.
5 Reason: `3` is the number of emptied ranks, typed twice.
6 Display: n/a.
7 Failure: the fill rewrote rows that belong to a stamp this build cannot read; still possible.
8 Deleted: 398 (the row revived), 401 (a chart read), 402 (a replay), 403 (a row written).
9 Stand-in: `== 3` stands in for "the count before".
10 Same rule: none in tests/.
11 Real: yes, any library after a stamp bump.
Verdict: Rewrite. Compare the empty-row count after with the count before (a property), not 3 twice.

## Splits

- `tests/dm_fixture.h` is read as "no search" in `records/window-WN4.md` and `verify/window-WN4.md`, while `sample_chart()` (`tests/dm_fixture.h:46-52`) runs the search at the default 60 window. Two readings of one helper.
- `Settings::early_fill_window_ms` is an `int` in whole ms (`src/app/config.h:195`) while `SearchSettings::early_fill_window_ms` and `Activation::e_window_ms` are `double` (`pather.h:45`, `model.h:606`); the hand-built `Activation` in `test_pb_text.cpp:96-109,186-200` carries 60.0 by default, so it reads "early fill 0.0 ms" as an E0 only because 0 < 60; a window of 0 would make e_offset 0 not an E0 (`is_e0`: `e_offset < window_ms`). Not a disagreement of two answers, a dependence worth the line.

## Verdict counts for the 30 C records

Rewrite 26, Investigate 4 (claim not checked only), Keep 0, Remove 0. Record numbers skip 2 and 11, which are B rows kept in the list so the old C7 numbers can be followed (pb_coach 13 C = records 1, 3 to 10, 12 to 15; pb_text 9 C = 16 to 24; saved_figure_fill 8 C = 25 to 32).

Rewrite: 1, 3, 4, 5, 6, 8, 9, 10, 12, 13, 14, 15 (pb_coach), 16 to 24 (pb_text), 25, 28, 29, 30, 32 (saved_figure_fill). Investigate: 7 (pb_coach 276), 26 (saved_figure_fill 133), 27 (147, was Keep), 31 (341).

## Not reached

Every test case in the three files was read. Not read: `Coach::request_candidates` internals (`pb_coach.cpp:329-353`) beyond the old record's reading; `app::build_ladder`, `app::cost_list`, `app::cost_tier`, `candidate_costs` bodies; `Activation::difficulty` and `badge_timings`; `tight_window_note` body; `shown_timing` body; `engine_hardest_ms`; the adding commits of the tests (subjects only). `tests/dm_fixture.h`'s five users were not judged (other groups). No test case is unread.
