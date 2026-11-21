# Window round WN3 (T5-WN3, agent a2b3770b5af468654)

Files: tests/test_engine_sweeps.cpp, test_fills_and_skips.cpp, test_tied_variants.cpp, test_model.cpp, test_path_costs.cpp, test_helpers.cpp, and the helpers tests/corpus_util.h and tests/analyzed_chart.h. Read at the worktree HEAD (code of `5ec682c7`). Nothing was built or run.

## What every row rests on

The window the search runs under comes from three places, all read in code. `SearchSettings::early_fill_window_ms` defaults to `kDefaultEarlyFillWindowMs` = 60 (`src/search/pather.h:45`, `src/core/model.h:112`). `ScoreGraph`'s constructor default is the same constant (`src/search/graph.h:201`), so `ScoreGraph graph(song, n)` runs at 60. The app's `Settings().to_analysis_settings()` carries `early_fill_window_ms = static_cast<int>(kDefaultEarlyFillWindowMs)` (`src/app/config.h:195`, `config.cpp:411`), which is what `corpus::default_cfg()` (`tests/corpus_default_cfg.h:17`) and `test::scores_settings` (`tests/record_fixtures.h:582`) hand out. `corpus::analyzed` keys its cache on the window too (`corpus_util.h:228`), but no helper in this group sets a window. So no row below is kind A outside the delta round.

Where the window reaches the result: a fill is refused when `e_offset < -window` (`fill_refuses`, `src/core/model.h:331`), an activation is E0 when `e_offset < window` with no skips (`is_e0`, model.h:335). In the engine `branch_activate` returns before it records a passed-over fill when the fill is refused (`engine.cpp:1007`), and records it as skipped otherwise (`engine.cpp:1049-1053`). So the passed-over fill lists, the E marks, the all-0 answer (`allzero_activation`, model.h:381) and the ms-limit walk (`fill_within_limit`, model.h:369; `engine.cpp:1224`) all move with the window. Only an `e_offset` in [60, 85) or in [-85, -60) moves differently between 60 and 85.

Reason used for the hand-built fixtures (kind B). In `build_tail_song` and its relatives, resolution is 192 and the tempo is 120 BPM, so one tick is 500/192 ms and 96 ticks are exactly 250 ms. A fill's deadline is its end minus 6 beats = 1152 ticks (`record_fixtures.h:244`, a multiple of 96). The SP-ready time is the time of a phrase note or an SP end, both at multiples of 96 ticks in these fixtures. So every `e_offset` is a multiple of 250 ms and none can lie in [60, 85) or [-85, -60). The claim "multiples of 96 ticks" was read from the note lists below, tick by tick. It fails for 9619 (test_tied_variants.cpp:556) and 5136 and 5280 (`past_last_note_song`), which are handled in their rows.

The filed rulings about the window, under `## Confirmed` in `docs/agents/rulings/code-and-engine.md` lines 55 to 57, quoted: "so we should use the hit window. and stop inventing our own numbers." / "make the early fill threshold an adjustable setting pinned to one number in both directions" / "analysis settings panel, default 60ms, supports 0ms to 85ms".

Engine records: the folders `records-engine/` hold seven charts dumped at `326cd4e0` under the old fixed 85 ms rule. Only `folded_sqin` is a chart these tests read (test_tied_variants.cpp:193, 229). Every other C record below names the dump at `--efill 60` it needs.

## Counts

| File | A | B | C | delta round |
|---|---|---|---|---|
| test_engine_sweeps.cpp (16) | 0 | 11 | 3 | 2 |
| test_fills_and_skips.cpp (11) | 0 | 2 | 5 | 4 |
| test_tied_variants.cpp (13) | 0 | 10 | 3 | 0 |
| test_model.cpp (100+ cases) | 0 | see note | 0 | 1 |
| test_path_costs.cpp (10) | 0 | 6 | 4 | 0 |
| test_helpers.cpp (4) | 0 | 4 | 0 | 0 |
| corpus_util.h, analyzed_chart.h | see helpers | | | 1 (settings_key, delta DL5 record 25) |

test_model.cpp note: `grep -n -i "analy\|search\|load_song\|graph\|run_search\|search_target"` over the file finds one search entry, `ScoreGraph graph(song, 2)` at line 280, inside the case at line 275, which is delta round (`delta-DL4.md` section 6). Every other test case in the file calls pure functions (`is_e0`, `fill_refuses`, `allzero_activation`, `notationstr`, scoring, `MultSqueeze`, labels) with typed or hand-built values and builds no graph. Cases at 147, 171, 206, 239, 255, 654 set windows by hand and are delta round (`delta-DL4.md` sections 1 to 5 and 7). Their rows are in the first table.

## test_engine_sweeps.cpp

| Line | Test case | Kind | Window and reason |
|---|---|---|---|
| 74 | search invariants hold across the corpus and config knobs | B | Default 60 (`SearchSettings cfg` untouched). Every assertion compares records made at the same window: depth does not change the best, the ms filter never raises it, order, notecount, tied count. No typed number; `charts > 0` counts parsed songs. A relation between two same-window runs cannot move with the window unless the engine breaks it. |
| 147 | the engine's limit walk agrees with Activation::within_ms_limit | C | record 1 below |
| 228 | note total: every path of How You Remind Me stores the Song's note count | C | record 2 below |
| 245 | note total: the engine, the Song and the Dynamics tab agree on every corpus chart | B | `p.notecount == total` compares a field the engine copies from the Song (`engine.cpp:2007`, `path.notecount = note_count`) to the Song's own count; `played_total` is parse output. Nothing the search chooses is asserted. |
| 286 | the graph finds the chart's multiplier squeezes once, in chart order | B | `graph.multsqueezes()` and `analyze_chart`'s list are compared with a list built from `chord_score_table`; the comment at 283 says the list depends on combo alone. No fill, no ready time. |
| 325 | legacy fill deadline analyzes a chart end to end | B | Asserts `sp_cap == 4` (an input echoed), `best_path().totalscore() > 0` and `analyzed > 0`. A path with no activation always scores above 0, so no window leaves a chart without a scored path. Not checked: that a record exists for every chart; the case stops at three charts. |
| 410 | stored transfer scales match the display-layer recomputation | B | Stored scale against `frontend_transfer_scales` on the same activation; both read the SP-end steps. `acts > 0` and `run.charts > 0` are the only counts asserted. A relation per activation, true for whatever activations the window leaves. |
| 480 | no activation keeps backends past its squeezed-out note | B | Per-activation property on `sqout_tick` against backend rows; `sqout_acts` is only in MESSAGE. `run.charts > 0` is the only count. |
| 517 | search_allzero returns only all-0 paths inside the 0 ms limit | C | record 3 below |
| 581 | corpus: squeeze-ins at exactly 0 and what they change | delta round | `delta-DL5.md` record 18 |
| 670 | score graph: the backend rows are sized once, one per chord | B | Builds `ScoreGraph` and reads `all_backends()`; no search is run. |
| 695 | run_search: EngineOptions carries each knob to the engine | B | Fixture `past_last_note_song`: one fill, at 2304; SP ready at phrase 768 = 2000 ms; deadline 2304 - 1152 = 1152 ticks = 3000 ms; `e_offset` +1000. The notes at 5136 and 5280 are plain notes, not fills or phrases, so they make no ready time. Compared outputs (`z` against `search_allzero`, pinned ticks against the best path) share one window. |
| 729 | allzero_options: no skips, a hard 0 ms limit, everything else default | B | Reads an options struct; no search. |
| 745 | settings_key: the text for SearchSettings{} is pinned, and every field moves it | delta round | `delta-DL5.md` record 19 |
| 772 | a 4-bar graph built at the song's phrase count stores the same paths | B | Tall graph and built graph both default 60 and the test compares `record_hash` of the two; `compared` goes to MESSAGE only. `mid_sp_phrase_song` ticks (0, 768, 1536, 2304, 3072, 3840, 4608, 5376, 6144, 6720, 6816) are all multiples of 96, so its `e_offset`s are multiples of 250 ms. |
| 806 | graph_build_cap: never taller than the song's phrases, never below one | B | Pure function of two ints. |

### Record 1. the engine's limit walk agrees with Activation::within_ms_limit (tests/test_engine_sweeps.cpp:147)

Delta round: window, replaces A13.md record 2. The old verdict (Rewrite: state the rule on a small chart, keep the corpus run as the agreement check) still holds. At 60 ms the run sees a different set of kept paths than at 85.
Read: test body 147-203; `fill_within_limit` (model.h:369); `Engine::act_within_limit` (`engine.cpp:1224`); `Activation::within_ms_limit` through `e_window_ms` (`engine.cpp:2030`).
1 Claim: every path the engine keeps under a 20 ms limit passes the model's walk, and no path the model's walk passes beats the engine's filtered best.
2 Owner: `fill_within_limit` (model.h:369), called by the engine (`engine.cpp:869`, `1224`) and by `Activation` through its stored `e_window_ms`. One owner for the early-fill part of the limit.
3 Real chart: the claim runs on every corpus chart, but no engine record backs it. Needed: `hydra_replay dump --efill 60` of a corpus chart that has an E0 inside 20 ms of its limit, for example `beg` (its best path "1- 0 0 E0", `records-engine/index.md`), to show both walks at 60. The existing `beg.dump.json` is at 85. Claim not checked at 60.
4 Direct: `within_ms_limit(kLimitMs)` per path against the engine's keep, as a relation. `kept_checked > 0` and `model_rejected > 0` (lines 199-200) are counts from the corpus run.
5 Reason: a count that must not be zero, "or the case proves nothing" (comment 197). The two guards are existence floors over corpus data.
6 Display: record only.
7 Failure: the two limit walks disagreeing on an early fill's timing.
8 Deleted: line 174 hides an engine keep the model rejects; line 189 hides an engine drop the model keeps; both are checks of window-read code, `fill_within_limit` against `e_window_ms`. Line 199 and 200: a corpus that no longer exercises both directions. The window moves these two counts: at 60 fewer fills are E0 than at 85 (is_e0 shrinks), so `model_rejected` can fall.
9 Stand-in: the corpus.
10 Same rule: `test_model.cpp:206`, `:239` (delta round, DL4 sections 3, 4) state `fill_within_limit`'s edges by hand.
11 Real: yes, a corpus run; see 3.
Verdict: Rewrite (old verdict) plus Investigate "claim not checked only": a dump at 60 ms of `beg` and one more chart. The two guard counts are a window-moved number with no value typed; no typed number found.

### Record 2. note total: every path of How You Remind Me stores the Song's note count (tests/test_engine_sweeps.cpp:228)

Delta round: window, replaces A13.md record 3. The old verdict (Remove: case 4 states the same property without a typed count) still holds.
Read: test body 228-243; A13 record 3.
1 Claim: every stored path and variant of one chart carries the chart's note total, 905.
2 Owner: `ScoreGraph::note_count`, copied at `engine.cpp:2007`.
3 Real chart: the chart is real; no engine record. The note total is a parse fact, 905 (line 235 compares it to `song.note_count()`).
4 Direct: `p.notecount == 905` per path (typed, a parse fact, not a search fact); `paths > 1` (line 242): a count of stored paths and variants at depth 200 and cap 4.
5 Reason: line 242 is the only line the window can move: it needs more than one path stored.
6 Display: record only.
7 Failure: a path with a different note total.
8 Deleted: 240 is covered by record 4 (test at 245); 242 fails only if a window leaves one path.
9 Stand-in: none.
10 Same rule: test at 245.
11 Real: yes.
Verdict: Remove (old verdict). The window-moved line is 242, `paths > 1`; the chart's engine record at 60 is not available and no run was made. Typed number found: 905 is parse output, cannot move.

### Record 3. search_allzero returns only all-0 paths inside the 0 ms limit (tests/test_engine_sweeps.cpp:517)

Delta round: window, replaces A13.md record 9. The old verdict (Rewrite: put D85's two edge cases on two small fixtures) still holds, and the window sharpens it.
Read: test body 517-574; `allzero_activation` (model.h:381); `delta-DL5.md` record 4 (the E1 at 85 that is a skip at 60).
1 Claim: every path the all-0 pass reports is all-0 and inside the 0 ms limit, and never beats the optimum; the corpus keeps a chart with a 0 ms timing and a chart with an E1.
2 Owner: `allzero_activation` (model.h:381) and `Path::is_allzero`; `allzero_options` for the search.
3 Real chart: claim not checked at 60. The only E1 evidence is `beg` and similar dumps at 85. Needed: `hydra_replay dump --efill 60` for `beg` and `pitch_dark_ratio`, and a corpus sweep at 60 (needs the user's `allow library run` if it is the whole library; the checked-in corpus alone does not).
4 Direct: relations per path; then `CHECK_FALSE(zero_ms_charts.empty())` and `CHECK_FALSE(e1_charts.empty())` (lines 569-570), existence floors over corpus content (comments 520-525).
5 Reason: D85 and D102 quoted in the comments. The window rulings quoted above change what an E1 is: `delta-DL5.md` record 4 shows a path that is an E1 at 85 and a skip at 60.
6 Display: record only.
7 Failure: the all-0 list shows a path with a skip, or hides the best all-0 path.
8 Deleted: 569 and 570 catch a corpus where no chart keeps a zero-slack early fill or an E1. This is the line that can move: `is_e0` and `allzero_activation` are narrower at 60, so `e1_charts` can fall to zero while the engine is correct. The relations catch an all-0 path with a skip.
9 Stand-in: floors over the corpus.
10 Same rule: `test_model.cpp:171` (delta DL4 section 2); `test_fills_and_skips.cpp:249` (delta DL5 record 4).
11 Real: yes.
Verdict: Rewrite (old verdict) and Investigate "claim not checked only" for the 60 ms dump. Finding on its own line: `CHECK_FALSE(e1_charts.empty())` (line 570) and `CHECK_FALSE(zero_ms_charts.empty())` (line 569) are existence floors that can fail at 60 with no engine fault; no number is typed.

## test_fills_and_skips.cpp

| Line | Test case | Kind | Window and reason |
|---|---|---|---|
| 37 | search: a later-ready path never knocks out one an upcoming fill still spawns for | B | `ready_time_fold_song` at `ScoreGraph(song, 2)`, default 60. Its three fills end at 2304, 7296, 7680 (ticks), deadlines 3000, 16000, 17000 ms; ready times are 2000 ms (path B) and 18000 ms (path A). `e_offset`s are +1000, +14000, -2000, +15000, -1000: none in the bands. The fixture comment (record_fixtures.h:239-249) names the same numbers. |
| 60 | Skipped fills: the 1.0 rule's offered fill is the one stored | B | `make_ch10_fill_song`, `ScoreGraph(song, 4, FillDeadlineRule::Ch10)`, default 60. SP is ready at 18900 ms; A's deadline is 18968.75 ms (`e_offset` +68.75), B's 18768.75 ms (-131.25). A is not refused at either window (-85 < +68.75); B is refused at both (-131.25 < -85). The test asserts only `skipped_fill_ticks == {19200}`, which depends on refusal, not on is_e0. +68.75 sits in [60, 85) so A's E mark would differ, but the test does not read it. |
| 82 | Skipped fills: under the 1.1 rule a fill exactly at the reach edge is stored | delta round | `delta-DL5.md` record 1 |
| 181 | search: a fill that needs a 70 ms early hit is taken at an 85 ms window, not at 60 | delta round | `delta-DL5.md` record 2 |
| 220 | search: SP ready 70 ms before a fill's deadline marks the activation E at 85, not at 60 | delta round | `delta-DL5.md` record 3 |
| 249 | all-0 pass: passing over a fill with 70 ms to spare is an E1 at 85, a skip at 60 | delta round | `delta-DL5.md` record 4 |
| 277 | Skipped fills: every corpus root passes over real fills in order | C | record 4 below |
| 347 | trailing fills: a corpus path lists the fills after its last activation that the meter allows | C | record 5 below |
| 383 | trailing fills: a path whose last window outlasts the chart lists none | C | record 6 below |
| 416 | trailing fills: a full pin of a corpus path agrees with the free search | C | record 7 below |
| 435 | trailing fills: the path with no activation lists every fill the meter allows | C | record 8 below |

Why 277 to 435 are C and not B: a passed-over fill is stored only when `branch_activate` did not refuse it (`engine.cpp:1007`, then `1049-1053`). So every list of passed-over fills in these cases is cut by `fill_refuses(e_offset, window)`. At 60 a fill whose SP was ready 60 to 85 ms late is not listed; at 85 it was.

### Record 4. Skipped fills: every corpus root passes over real fills in order (tests/test_fills_and_skips.cpp:277)

Delta round: window, replaces A14.md record 28. The old verdict (Rewrite: B8 floor, B9 silent skip; claim half checked on a real chart) still holds.
Read: test body 277-306; `engine.cpp:1007-1053`; A14 record 28.
1 Claim: each fill a path lists as passed over is a real fill of the chart, lies before the activation, after the previous activation, and in order.
2 Owner: `Activation::skipped_fill_ticks`, filled by `branch_activate` (`engine.cpp:1051`). Engine.
3 Real chart: runs on the whole corpus under the app's defaults. No engine record at 60 for a chart that skips. Needed: `hydra_replay dump --efill 60` of `pitch_dark_ratio` (skips 86400, 94080, 101760) and `folded_sqin` (skips 4608, 10752), both in `records-engine/` at 85.
4 Direct: ordering relations per fill; line 305: `skipped > 100`, "a floor the user approved (D43)".
5 Reason: D43 as quoted; the floor is a count from corpus content.
6 Display: the Preview lights these fills (`engine.cpp:1050`), record only here.
7 Failure: a skipped fill listed out of order or not a fill.
8 Deleted: 292-299 the order and fill-ness; 305 an engine that lists too few fills. The window moves line 305: passed-over fills are stored only when not refused, so a narrower window lists fewer.
9 Stand-in: the corpus.
10 Same rule: delta DL5 record 1 (the edge).
11 Real: yes.
Verdict: Rewrite (old verdict). Typed number that can move: `skipped > 100` (line 305). Whether it holds at 60 is not known: no run. Claim not checked at 60.

### Record 5. trailing fills: a corpus path lists the fills after its last activation that the meter allows (tests/test_fills_and_skips.cpp:347)

Delta round: window, replaces A14.md record 29. The old verdict (Rewrite: pins at l249-251; gauge as referee; silent skip; SPLIT between the engine's trailing list and the Preview's `sp_meter_bars_at`) still holds.
Read: test body 347-381; `engine.cpp:2010`; A14 record 29.
1 Claim: the best path that lists any trailing fill is `"3"`, activates once at 123838 and passes two fills, 244800 and 281759, after that; every listed fill is after the last activation, in order, on the scene, with the meter holding at least `kSpActivationBars`.
2 Owner: the trailing list (`engine.cpp:2010`) and `sp_meter_bars_at` (Preview). Two owners for one question (the A14 SPLIT, not mine to close).
3 Real chart: "Hail The Sun - Missed Injections", no engine record. Needed: `hydra_replay dump --efill 60` of that chart, depth 40 at cap 4.
4 Direct: line 362 `pathstring() == "3"`, 363 `{123838}`, 364 `{244800, 281759}`: typed from one run (comment 350, "pinned from one run"). 380 `listed > 0`.
5 Reason: none beyond "pinned from one run".
6 Display: the Preview's offered fills.
7 Failure: a wrong trailing list.
8 Deleted: 362-364 a changed best listing path; 340-341 the meter rule.
9 Stand-in: corpus chart by name.
10 Same rule: records 4 and 8.
11 Real: yes.
Verdict: Rewrite (old verdict). Typed numbers that can move with the window: "3", 123838, 244800, 281759 (lines 362-364). The run was made before the window setting existed (the pinned numbers date from the fixed 85 rule). A fill listed at 85 could be refused at 60, and the first path to list one could be another path.

### Record 6. trailing fills: a path whose last window outlasts the chart lists none (tests/test_fills_and_skips.cpp:383)

Delta round: window, replaces A14.md record 30. The old verdict (Rewrite: silent skip; floors; claim not checked) still holds.
Read: test body 383-414; A14 record 30.
1 Claim: a path whose SP is still running at the chart's last note lists no trailing fill.
2 Owner: the trailing list, `engine.cpp:2010`.
3 Real chart: claim not checked. Needed: a dump at 60 ms of a chart where the last activation outlasts the chart.
4 Direct: line 400 the list is empty for every outlasting path (a property); 410 `outlasting > 0`; 413 `with_fills_after > 0`.
5 Reason: floors from the corpus.
6 Display: record only.
7 Failure: a trailing fill listed after a window that never ends.
8 Deleted: 400 catches the engine listing a fill that was in the SP; 410, 413 catch a corpus with none.
9 Stand-in: the corpus; counts.
10 Same rule: records 4, 5, 7, 8.
11 Real: no.
Verdict: Rewrite (old verdict). The property (400) holds at any window. The floors 410 and 413 are window-moved counts; no number typed beyond zero. Claim not checked at 60.

### Record 7. trailing fills: a full pin of a corpus path agrees with the free search (tests/test_fills_and_skips.cpp:416)

Delta round: window, replaces A14.md record 31. The old verdict (Rewrite: apply record 29's rule check to the pinned result; drop the silent skip; floor becomes "ran at least once") still holds.
Read: test body 416-433; A14 record 31.
1 Claim: pinning a path's full windows and searching again gives the same trailing list.
2 Owner: `search_target` (`pather.h`), engine.
3 Real chart: corpus; no dump at 60.
4 Direct: relation between two runs at the same window (`search_target(song, cfg, ...)` and `corpus::analyzed(cfg)` share `cfg`); 432 `compared > 0`.
5 Reason: none.
6 Display: record only.
7 Failure: a pin that drops or adds a trailing fill.
8 Deleted: 427 `paths.size() == 1`, 429 the equality, 432 a corpus with no listing path.
9 Stand-in: corpus.
10 Same rule: record 5.
11 Real: no.
Verdict: Rewrite (old verdict). The equality holds at any window. `compared > 0` is a window-moved count (a path with a trailing fill must exist at 60). Claim not checked at 60.

### Record 8. trailing fills: the path with no activation lists every fill the meter allows (tests/test_fills_and_skips.cpp:435)

Delta round: window, replaces A14.md record 32. The old verdict (Rewrite: state "a fill is listed iff the meter there holds kSpActivationBars" over `scene.fills`, no typed tick) still holds.
Read: test body 435-455; `engine.cpp:2010`; A14 record 32.
1 Claim: for "Allister - Overrated" with no activation, the path lists 8 of 9 fills, `{55680, 71040, 90240, 105600, 120960, 136320, 151680, 167040}`; the ninth (36480) comes with one bar saved.
2 Owner: engine trailing list; `sp_meter_bars_at`.
3 Real chart: no engine record. Needed: `hydra_replay dump --efill 60` of Allister - Overrated, with `search_target` over no windows.
4 Direct: line 447-448 the eight ticks (typed from one run, comment 446), 452 `scene.fills.size() == 9`, 453 `36480`, 454 `bars_at < kSpActivationBars`.
5 Reason: "pinned from one run".
6 Display: Preview.
7 Failure: a listed fill the meter does not hold.
8 Deleted: 447 any change in which fills are listed.
9 Stand-in: corpus chart by name.
10 Same rule: record 5.
11 Real: yes, a corpus chart.
Verdict: Rewrite (old verdict). The window reaches the eight ticks: a fill is listed only when not refused, and for a path with no activation the ready time is fixed by the chart. A fill whose deadline lies 60 to 85 ms before that ready time would be listed at 85 and not at 60. Typed numbers that can move: the eight ticks (447-448), 9 (452). No run was made; whether any of the eight is in that band is not known.

## test_tied_variants.cpp

| Line | Test case | Kind | Window and reason |
|---|---|---|---|
| 101 | tied variants: a variant folded mid-SP takes its leader's steps after the fold | B | `midsp_notes(false)` at `ScoreGraph(song, 2)`, default 60. Every tick of the list (loop over 0..3072 step 96, then 4608, 8448, 9216, 10752, 11520, 11904, 12672, 13056, 15360, 16896, 17664, 18432) is a multiple of 96 at 120 BPM, resolution 192. `e_offset`s are multiples of 250 ms. |
| 134 | tied variants: a variant folded mid-SP takes its leader's closing SqIn or SqOut | B | Same list plus 6144, 11328, 16032, 16224, all multiples of 96. Same reason. |
| 193 | tied variants: a path that banked its leader's SqIn phrase folds into it only after SP | B | `folded_sqin.chart` (resolution 192, 120 BPM, 4000 BPM from 9600). Its dump at 85 (`records-engine/folded_sqin.dump.json`) shows `sp_ready_ms` 6000 and 25210. Fills end at 4608, 10752, 13824; deadlines 9000, 25000, 25240 ms (13824 is 4224 ticks past 9600 at 0.078125 ms a tick). `e_offset`s are +3000, +3000, -210 (refused at both windows), +30 (an E0 at both). None lies in [60, 85) or [-85, -60). The only other ready times, SP ends after 10752, are after 25450 ms and are refused at both. The test's `"0 E0"` and 6750 therefore hold at 60 by reading; they are not proven by a 60 dump. |
| 229 | tied variants: the banked-phrase charts analyze and every variant prices as alone | C | record 9 below |
| 263 | clamped_sqin charts: every squeeze-out ends SP at the end its record names | C | record 10 below |
| 296 | banked_phrase_in_reach banks exactly what activation_can_squeeze refuses | B | Loads a song and calls `core::banked_phrase_in_reach` on every chord; no search. |
| 337 | squeeze rule: twin SP ends a tick apart squeeze one phrase in once | B | `twin_end_clamped.chart` has two fills, `S 64 960` at 18240 and 20160, ending at 19200 and 21120 (resolution 480, 120 BPM: 1.0417 ms a tick). SP is first ready at phrase 5760 (6000 ms) and every later phrase (23041, 26000) is after both fills. Deadlines are 4 beats before the fill's start: about 17000 and 19000 ms. `e_offset`s are about +11000 and +13000. Nothing in the bands. |
| 361 | squeeze rule: twin SP ends a tick apart each squeeze only their own path | B | Same chart, same reason; the pinned activations are 19200 and 21120. |
| 399 | tied variants: a variant that finished the song keeps its own bank | B | `build_tail_song`, ticks 0, 768, 2304, 3072, 5760, 8448: all multiples of 96. `e_offset`s are multiples of 250 ms. The typed 400 is a score of two doubled notes (comment 403), not a search choice among windows. |
| 443 | tied variants: a variant folded between windows keeps its own banked bars | B | `fold_between_windows_song`, ticks 0, 768, 2304, 3072, 5760, 8448, 9216, 9984, 10752, 11520, 12288, 16128: all multiples of 96. Typed scores 1100 and 850 and counts 2, 2, 2 depend on which fills are refused; with offsets in multiples of 250 none can flip. |
| 503 | tied variants: a variant's trailing fills are its own | B | Same chart as 443. Typed `{9984, 10752}` and `{10752}`: the listed fills are not refused at either window by the same lattice argument. |
| 553 | tied variants: a variant folded between windows keeps its own early fill | B | The one fixture with a tick off the 96 grid: 9619 = 25049.48 ms. The test says the fill at 10752 has deadline 25000, so `e_offset` -49.479 (typed 49.479, line 583). -49.479 is above -60 so it is not refused at either window, and below +60 so it is an E0 at both. The fill at 9984 (deadline 23000) is 2049 ms early for that ready time: refused at both. All other ready times are multiples of 250 ms. |
| 596 | tied variants: Don Broco - Actors at depth 40 shows each tied path's own bank | C | record 11 below |

### Record 9. tied variants: the banked-phrase charts analyze and every variant prices as alone (tests/test_tied_variants.cpp:229)

Delta round: window, replaces A35.md record 4. The old verdict (Rewrite: keep the properties; add that the loops checked at least one variant; claim not checked on `relabel_case`) still holds, and the window adds a reason it is still not checked.
Read: test body 229-251; `relabel_case.chart` header lines (resolution 192, 120 BPM, 4000 BPM from 9600; phrases at 768, 2304, 8448, 9000, 13824; fills at 4224, 10368, 13440).
1 Claim: both charts analyze, every activation has SP-end steps after its start, and every tied variant prices as a search of its own would.
2 Owner: `analyze_chart`, `lone_pricing_mismatch` (record_fixtures.h, a test helper that searches again).
3 Real chart: `folded_sqin` has an engine record at 85 (`records-engine/folded_sqin.dump.json`); `relabel_case.chart` does not. Needed: `hydra_replay dump --efill 60` of `relabel_case.chart`.
4 Direct: properties per activation and per variant; no typed number.
5 Reason: none.
6 Display: record only.
7 Failure: a variant that prices differently from its own search.
8 Deleted: 241, 243 the SP-end steps follow the activation; 248 the lone-pricing check.
9 Stand-in: hand-built charts.
10 Same rule: `test_replay.cpp` ("every tied variant stores what a search pricing it alone stores").
11 Real: partly (`folded_sqin` at 85).
Verdict: Rewrite (old verdict). `relabel_case.chart` has a phrase at 9000 (23437.5 ms, off the 250 ms grid) and a 4000 BPM stretch, so I could not show every `e_offset` stays out of the 60-85 bands. Not proven B. No typed number. The loop compares two searches at one window, so the relation holds at any window; only the coverage is unknown.

### Record 10. clamped_sqin charts: every squeeze-out ends SP at the end its record names (tests/test_tied_variants.cpp:263)

Delta round: window, replaces A35.md record 5. The old verdict (Investigate: claim not checked only; needs the dump for clamped_sqin_a and b) still holds.
Read: test body 263-288; chart headers: `clamped_sqin_a.chart` has fills `S 64 384` at 5760, 8928, 14784, 14880, 21024 and a tempo change to 4000 BPM at 5280; `clamped_sqin_b.chart` has fills at 5760, 7872, 9120, 14496, 20352 and 4000 BPM from 8160. Cap 3.
1 Claim: for each squeeze-out, the stored offset equals the offset from the SP end node the activation really ended on.
2 Owner: `offset_from_sp_end` (`core`), compared with the stored `offset_ms`.
3 Real chart: claim not checked. Needed: `hydra_replay dump --efill 60` of `clamped_sqin_a.chart` and `clamped_sqin_b.chart`.
4 Direct: the offset equals the computed offset (a relation); `sqouts > 0` (286).
5 Reason: none beyond "analyze and check".
6 Display: record only.
7 Failure: an SP end that disagrees with the offset's node.
8 Deleted: 281 the equality; 286 a chart with no squeeze-out.
9 Stand-in: hand-built charts.
10 Same rule: `test_sp_end_clamp.cpp`.
11 Real: no.
Verdict: Investigate: claim not checked only. At 4000 BPM a tick is 0.078 ms, so ready times and deadlines are not on a 250 ms lattice; two fills (14784, 14880) sit 96 ticks apart. I could not show the window cannot move which squeeze-outs exist, so `sqouts > 0` is a count the window may move. No typed number.

### Record 11. tied variants: Don Broco - Actors at depth 40 shows each tied path's own bank (tests/test_tied_variants.cpp:596)

Delta round: window, replaces A35.md record 13. The old verdict (Rewrite: compare each tied path's leftover with a lone search's result for the same activation ticks instead of typing it) still holds.
Read: test body 596-619; A35 record 13.
1 Claim: two paths tying at 233100, one activating at 150720 and one at 82560, end the song with 1 and 3 bars of SP.
2 Owner: `Path::leftover_sp`, engine.
3 Real chart: "Don Broco - Actors", no engine record. Needed: `hydra_replay dump --efill 60` of it at cap 4, depth 40, ms limit 10.
4 Direct: 233100, 150720, 82560, 1, 3, 2: typed from one run (audit finding 89, comment 597).
5 Reason: "Audit finding 89: paths 29 '7' and 30 '3' tie at 233,100". The path numbers 29 and 30 were of the old run.
6 Display: record only.
7 Failure: a tied path showing the other's bank.
8 Deleted: 615, 616 the two leftovers; 618 that both paths were found.
9 Stand-in: corpus chart by name.
10 Same rule: tests 399, 443.
11 Real: yes.
Verdict: Rewrite (old verdict). Typed numbers that can move with the window: 233100 (the tie), 150720, 82560, and the two leftovers 1 and 3 (lines 613-616), plus `seen == 2`. A different window can change which fills spawn and so which paths tie at all. No run made.

## test_path_costs.cpp

| Line | Test case | Kind | Window and reason |
|---|---|---|---|
| 110 | cost list: the costs sum exactly to the gap | C | record 12 below |
| 130 | cost list: no cost is below zero | C | record 13 below |
| 169 | cost tier: the 1% boundary | B | `cost_tier` on typed integers; no search. |
| 176 | cost tier: a negative cost is a bug | B | Pure function throws; no search. |
| 180 | candidate costs: a negative cost is a bug | B | Pure function on typed integers. |
| 195 | end costs: each end against the best of them, in the order given | B | Pure function on typed integers. |
| 205 | build ladder: a list the target searches refuse is refused before any search | B | `search_graph` builds the graph; `build_ladder` throws on the duplicate pin `{9984, 9984}` before any search; the callback is never called (`heard.empty()`). No search output asserted. |
| 216 | search target: a built graph answers as the song does | C | record 14 below |
| 231 | build ladder: stops at the first impossible prefix and names it | C | record 15 below |
| 249 | costs nothing: only a cost of zero is a best choice's | B | Pure function on typed integers. |

### Record 12. cost list: the costs sum exactly to the gap (tests/test_path_costs.cpp:110)

Delta round: window, replaces C6.md record PC1. The old verdict (Rewrite: drop the pinned totals and costs; state cost k as bf[k-1] - bf[k]) still holds.
Read: test body 110-128; fixture `allies_cases` (75-101); C6 PC1.
1 Claim: each window's cost is the best total with earlier windows pinned minus the best total with it pinned, and the costs sum to the gap.
2 Owner: `cost_list` and `build_ladder` (`app/path_costs.cpp`).
3 Real chart: "Mutemath - Allies". No engine record. Needed: `hydra_replay dump --efill 60` of Allies, plus its `schedule` for the all-0 path.
4 Direct: bf lists {451830, 451830, 451830, 447770, 447770}, {451830, 451830, 438490, 434430, 434430}, {451830, 451830, 450430}; player totals 447770, 434430, 407610; costs {0,0,4060,0}, {0,13340,4060,0}, {0,1400}; after_last 0, 0, 42820. Line 79-80: `rec.allzero_paths.front()` activates at `{9984, 19200, 34560, 49920}`. All typed from one run (comment 74).
5 Reason: "pinned from one run" (comment 74).
6 Display: the Path Builder's cost list; record only here.
7 Failure: a cost that does not add up.
8 Deleted: 126 a dropped or doubled term. The pinned numbers are snapshots.
9 Stand-in: corpus chart by name and the 10 ms limit.
10 Same rule: records 13, 14, 15.
11 Real: yes.
Verdict: Rewrite (old verdict). Typed numbers that can move: every number in 4, above all the all-0 path's ticks `{9984, 19200, 34560, 49920}` (79-80) and its totals, since the all-0 answer is the one the window changes (`delta-DL5.md` record 4). The best path "0- 1- 1 0" (comment 73) carries no E; whether any fill on Allies has an `e_offset` in [60, 85) is not known.

### Record 13. cost list: no cost is below zero (tests/test_path_costs.cpp:130)

Delta round: window, replaces C6.md record PC2. The old verdict (Rewrite: replace 706180 and 5 with the best path's own total and activation count) still holds.
Read: test body 130-167; C6 PC2.
1 Claim: pinning more of a path never raises the best finish; pinning the engine's own best path costs nothing, even with a window tighter than the 10 ms limit.
2 Owner: `build_ladder`, `cost_list`.
3 Real chart: "August Burns Red - White Washed", "Mutemath - Allies"; no record. Needed: `hydra_replay dump --efill 60` of White Washed.
4 Direct: 137, 139, 140, 164, 166: relations and zero costs. 150 `ms_filter == 10`; 152 `totalscore() == 706180`; 154 `acts.size() == 5`; 155 the fourth activation is beyond the limit; 159 `bf == 6 x 706180`; 160 `player_total == 706180`.
5 Reason: the best path "read once" (C6).
6 Display: record only.
7 Failure: a pinned search beating the free one.
8 Deleted: 152, 154, 155, 159: a changed best path. 155 reads `within_ms_limit`, which reads `e_window_ms` (`fill_within_limit`).
9 Stand-in: corpus chart by name.
10 Same rule: records 12, 14, 15.
11 Real: yes.
Verdict: Rewrite (old verdict). Typed numbers that can move: 706180 (lines 152, 159, 160), 5 (154), and the claim at 155 that the fourth activation is over the limit (a limit walk that depends on the window if that activation has an early fill).

### Record 14. search target: a built graph answers as the song does (tests/test_path_costs.cpp:216)

Delta round: window, replaces C6.md record PC8. The old verdict (Remove: line 224 cannot fail while the song form calls the graph form) still holds.
Read: test body 216-226; C6 PC8.
1 Claim: `search_target` on a built graph and on a song return the same best total, 407610.
2 Owner: `search_target` (`pather.h`).
3 Real chart: Allies, no record at 60.
4 Direct: 224 equality; 225 `== 407610` typed.
5 Reason: the number is "pinned from one run".
6 Display: none.
7 Failure: the two forms differing.
8 Deleted: 224 none (the song form calls the graph form); 225 a changed total.
9 Stand-in: corpus chart.
10 Same rule: record 12 (407610 also appears there).
11 Real: yes.
Verdict: Remove (old verdict). Typed number that can move: 407610 (line 225), for the activations pinned at {9984, 22272}; whether the two pins' fills are inside the 60-85 band is not known.

### Record 15. build ladder: stops at the first impossible prefix and names it (tests/test_path_costs.cpp:231)

Delta round: window, replaces C6.md record PC9. The old verdict (Rewrite: drop 451830; state bf[0] == bf[1]) still holds.
Read: test body 231-247; C6 PC9.
1 Claim: with a second window moved one tick, to 22273 where no fill node exists, the ladder keeps the first two totals and names that window.
2 Owner: `build_ladder`; `search_target`.
3 Real chart: Allies, no record at 60.
4 Direct: 240 `bf == {451830, 451830}`; 241 `best.size() == 2`; 243 `realized_prefix == 1`; 244 `failed_tick == 22273`; 245 `failed_reason == "activation"`; 246 callback sizes.
5 Reason: "pinned".
6 Display: the Path Builder's refusal.
7 Failure: a ladder that goes on after an impossible window.
8 Deleted: 240-246 each a part of the stop.
9 Stand-in: corpus chart.
10 Same rule: record 12.
11 Real: yes.
Verdict: Rewrite (old verdict). Typed number that can move: 451830 (line 240); the failure at 22273 is a missing node, which does not depend on the window, but whether activation 9984 is refused at 60 does. A 60 dump of Allies decides it.

## test_helpers.cpp

| Line | Test case | Kind | Window and reason |
|---|---|---|---|
| 37 | corpus cache: one parse and one analysis per chart and settings | B | The cached record is compared to `analyze_chart` on the same `cfg` (default window, both sides). Identity of cache entries and `different settings, different entry` (depth_value) are pointer checks. |
| 67 | corpus cache: a failure is thrown again on every call | B | Asserts the `NoNotesError` text; thrown before any search (`require_notes`, corpus_util.h:248). |
| 85 | temp_util: the scratch path is the temp folder, the tag and this process | B | Paths and folders; no search. |
| 111 | scratch_settings: the GUI tests' settings are the app's defaults at depth 2 | B | Reads settings fields (`depth_value`, `sp_cap`, `mslimit_value`, `depth_mode`); no search. The window is not read, so a change to its default would not show here (a gap, not a finding on a row). |

## Helpers

`tests/corpus_util.h`: `settings_key` is delta round (`delta-DL5.md` record 25). `analyzed`, `analyzed_with_paths`, `charts_with_paths`, `first_chart_with_paths` run `analyze_chart` or `analyze_chart_file` under the settings passed in and do not set a window. `analyzed_with_paths` and `first_analyzed_with_paths` take the caller's `AnalysisSettings`, so the window is the caller's (the default 60 when the caller used `Settings().to_analysis_settings()` or a bare `AnalysisSettings`). `unknown_scale_reason` reads stored scales, no window.
`tests/analyzed_chart.h`: `analyzed_by_ten_scores()` builds an `AnalysisSettings` with `depth_mode`, `depth_value` 10 and `ms_filter` 10.0 and leaves the window at its default 60 (analyzed_chart.h:17-22). It is read by tests in other groups (its header names `test_activation_rows.cpp`, `test_path_list.cpp`, `test_preview_boxes.cpp`, `test_preview_scene.cpp`); the first corpus chart with paths is the chart, so each of those test cases takes its first chart's result at 60. That is a note for the groups that own those tests, not a row here.

## Findings, one line each

- Typed numbers that can move (kind C): 905 does not move (parse output). `skipped > 100` (test_fills_and_skips.cpp:305). `"3"`, 123838, 244800, 281759 (362-364). The eight ticks and the 9 (447-448, 452). 233100, 150720, 82560, 1, 3 (test_tied_variants.cpp:613-616). 451830, 447770, 438490, 434430, 450430, 407610, 4060, 13340, 1400, 42820 and the all-0 ticks `{9984, 19200, 34560, 49920}` (test_path_costs.cpp:79-99, 240); 706180 and 5 (152-154, 159-160); 407610 (225).
- Existence floors that can fail at 60 with no engine fault: `e1_charts` non-empty (test_engine_sweeps.cpp:570), `zero_ms_charts` non-empty (569), `kept_checked > 0` and `model_rejected > 0` (199-200), `paths > 1` (242).
- Split (from the earlier round, carried): the engine's trailing list (`engine.cpp:2010`) and the Preview's `sp_meter_bars_at` both answer "does the meter hold an activation here" (A14 record 29). Not closed here.
- Test case 60 (Ch10 skip): fill A's `e_offset` is +68.75, inside [60, 85). The test reads only the skipped list, so it does not move, but any assertion added on A's E mark would.
- Gap: `scratch_settings` check (test_helpers.cpp:111) does not read `early_fill_window_ms`.

## Not reached

Nothing named in the brief was left unread by file. Not proven (stated in the rows): the B rows for 193 (folded_sqin) rest on one 85 ms dump and reading, not on a 60 ms dump; the B rows that use the 96-tick lattice rest on reading the note lists, not on a run. The old record numbers for test_tied_variants.cpp (A35 records 1 to 13) were taken from the file order; A35's record 5 and 13 headings were checked by their verdict lines, the others by order.

## Count

16 + 11 + 13 + 1 + 10 + 4 = 55 test cases with a row (test_model's cases without a search are not rowed). Kind A: 0. Kind B: 11 + 2 + 10 + 0 + 6 + 4 = 33. Kind C: 3 + 5 + 3 + 0 + 4 + 0 = 15 (15 full records above). Delta round: 2 + 4 + 0 + 1 + 0 + 0 = 7 (plus helper settings_key).
