# One owner for where Star Power ends

Task id SPE-P. Written 2026-11-19 by the planning agent. Not yet approved for build: every task waits for the user's yes (agent rule 3).

## What is wrong, in plain words

Hydra holds more than one answer to "where does this activation's Star Power end" when the path squeezes a phrase out. On Blight's first activation the record says the squeezed chord is at tick 64800 and the meter would run out at tick 65280. One function says SP ends at 64800, another says 65280, and the Preview's gauge rewrites the meter's own end to 64800 so the gauge reads empty on the chord. The saved bar has the same problem: the search stamps it at 65280 and a second function moves it to 64800 afterwards.

The user's words settle it:

- "how is the preview deciding how much starpower is left? there is deifnitely more than 0.0 here"
- "yes starpower ends because it's squeezed out. not because it actually empties there. all you have to do is stop lying and trying to convince me that we need multiple rules for where starpower empties."
- "when you squeeze out the starpower ends on the squeezed out chord and you get a bar saved to use later. thats it. so why are you overcomplicating it? why do you keep inventing your own rules and claiming them as truth?"
- "why is the engine's search making up its own rules? thats not how starpower works."
- On a late squeeze-out: "why is this even a question? find the answer yourself and tell me how it works".
- "yes plan it"

## How it works, as found

One rule covers both kinds of squeeze-out: Star Power ends, then the squeezed-out chord is hit, and its bar is saved on that hit.

**Early squeeze-out** (the chord sits before the point where the meter would run out; Blight activation 1). SP ends on the squeezed chord, tick 64800. Until then the meter drains at its normal rate, so it still holds a little on the chord. What is left is dropped when SP ends (SP-25). The saved bar arrives on the same chord, tick 64800.

**Late squeeze-out** (the chord sits after the point where the meter runs out; Allies activation 1). The game ends SP when the clock reaches the stored end (SP-28), and that is the meter's end, tick 13056. The chord at tick 13104 is hit after that, and the phrase is credited on that hit with SP off (SP-35). So SP ends at 13056 and the bar arrives at 13104, 157.9 ms later.

The user's sentence "the starpower ends on the squeezed out chord" is word for word true for the early kind. For the late kind SP has already ended a moment before the chord, because SP cannot run on an empty meter (SP-28, proven in the game's code). The user asked for that answer to be found, and this is it. The main session tells the user this in plain words when it shows the plan (see Questions, item 2); it is the one place the plan's result is not literally the user's sentence.

## Owner map

| Question | Owner | Its own words | Status |
|---|---|---|---|
| E1: On a path with no squeeze-out, where does an activation's Star Power end? | `Activation::deact_tick()` (`src/core/model.cpp:712`, last of `sp_end_steps`), worked out by `activation_initial_end_times` (`src/search/graph.cpp:391-394`), `ScoreGraph::extend_deacts` (`graph.cpp:301-323`) and `Engine::advance`'s `sp_end_time` (`engine.cpp:909-947`). `window_end_tick()` returns the same with no `sqout_tick`. | `model.h:670`: "deact_tick() - the deactivation node D: where this window's SP ends, every extension included." `model.h:635-641`: "Every SP-end fact the record holds is read from this list... Nothing re-derives it." | found |
| E2: On a squeeze-out, where does Star Power end? | (a) `Activation::window_end_tick()` (`model.cpp:717-721`): `sqout_tick` when not after `deact_tick`, else `deact_tick`; read by the Preview scene (`preview_view.cpp:435-445`) and `pb_text.cpp:61,159,167,181`. (b) `deact_tick()`: the search's `p.sp_end_time`, `deactivation_type` (`engine.cpp:1095`), record check (`engine.cpp:2099-2102`), replay window (`replay.cpp:271-276`), `path_identity` (`model.cpp:638`), press schedule (`tools/replay_schedule.cpp:416,508,724,806,990`), Path Builder pin (`pb_coach.cpp:74`, `pather.cpp:222`), `squeeze_rating`. (c) the replay payment cut `core::sqout_position(row.tick, w.sqout_tick)` (`replay.cpp:188-195`, `backend_value.h:18-24,47`). (d) the gauge rewrite (`preview_view.cpp:435-439`). | (a) `model.h:679-685`: "Where this window's Star Power is over as played: the squeezed-out chord when core::after_sp_end says it is not after deact_tick()... deact_tick() stays the stored end the search, the replay and the press schedule read." (c) `backend_value.h:14-15`: "Where a row sits against the activation's squeezed-out chord, compared by tick." (d) `preview_view.cpp:433-434`: "the last step's end being the window's end (Activation::window_end_tick)." | split. Blight act 1: (a) 64800, (b) 65280, (c) pays up to and including 64800, nothing after, 64800 earns its `sqout_sp` share, (d) 64800. |
| E3: On a squeeze-out, when does the saved bar arrive? | (a) the search's stamp `engine.cpp:1109-1114` into `Activation::bank_rise_ticks`: `sq->late ? sq->chord : sp_end` (`sp_end` = meter end). (b) `Path::bank_arrivals(i)` (`model.cpp:574-579`) overwrites `front()` with the previous activation's `sqout_tick`. | `model.h:645-648`: "A squeezed-out phrase's bar is the first entry, stamped where the search's own bookkeeping puts it (search/engine.cpp, the SqOut branch); Path::bank_arrivals gives the arrival as played. Stamped by the search." `engine.cpp:1109-1111`: "A squeeze-out banks one bar when the player hits the phrase: just after SP ends for an early phrase, on its own tick for a late one." | split. Blight: stored 65280, `bank_arrivals` 64800. The Preview reads only `bank_arrivals` (`preview_view.cpp:432,461`); a direct read of `bank_rise_ticks` gets 65280. |
| E4: Is a squeeze-out early or late? | `core::after_sp_end(chord_tick, end_tick)` (`backend_value.h:34`); callers: graph `late` flag (`graph.cpp:435`), `model.cpp:719`, `replay.cpp:186,442`, `pb_text.cpp:61`, `sqout_chord.h:92,152`. | `backend_value.h:30-33`: "is this chord after the SP end? A chord exactly on the end is not... The graph stamps its answer on each squeeze choice (SqueezeChoice::late); the engine reads that." | found |
| E5: Which chords does an activation's Star Power pay? | `core::backend_row_value` / `core::paid_by_sp` (`backend_value.h:45-66`) with `paid_by_sp_walk`, `counted_without_squeeze`, `sqout_position`; callers engine (`engine.cpp:1144-1151`), replay (`replay.cpp:188-196`), details table (`path_view.cpp:385-386`), `Activation::display_backends` / `set_sqout` (`model.cpp:813-848`); `sqout_pays_outside` (`src/core/scoring.cpp:29`). | `backend_value.h:1-2`: "What one backend row is worth on one path: the single answer the search, the replay and the details table all read." | found |
| E6: How much SP is in the meter at a moment while SP runs? | Only the `bars_left` lambda in `build_sp_meter_curve` (`preview_view.cpp:145-149`), read via `sp_meter_bars_at` (`:522`), `sp_meter_readout` (`:815`), `pb_text::bars_saved_at` (`pb_text.cpp:45`). The engine stores whole bars and end steps only. | `preview_view.cpp:142-144`: "Bars left at `tick` while SP ends at `end_tick`: the measures between, two to a bar." | carry out (user approved an engine owner). The engine works out the end in force (`sp_end_time`, `engine.cpp:945`) and the step list (`engine.cpp:2095`). |
| E7: Is SP running at a moment? | `PreviewActivation::sp_window()` (`preview_view.h:118`, from `window_end_tick`), evaluated by `sp_running_at` (`preview_view.h:407-410`) and the `sp_active` sweep (`src/render/track_state.cpp:219-223`); `preview_timeline.cpp:487`; `pb_text.cpp:61`. | `preview_view.h:122-127`: "The one home of that rule (M48)... the highway's sp_active sweep answers it separately, and test_track_state pins the two together." | split (two evaluations of one window, test-pinned at `tests/test_sp_meter_curve.cpp:570`) |
| E8: When does the meter next reach activation level after a squeeze-out? | Search: `p.sp_ready_ms` (`engine.cpp:982-989`, read `:853-861,1006,1694`) from `eo->sp_times[k]`, never the saved bar's stamp. Record: `Activation::sp_ready_tick()` (`model.cpp:702-706`, `bank_rise_ticks[1]`), read by `early_fill_window_end` (`src/core/note_window.cpp:133`). | `model.h:654-656`: "Where the meter reached the activation level (kSpActivationBars): that bar's stored arrival in bank_rise_ticks. Unset when fewer bars are stored, never a guess." | found (index 1, so the E3 split does not reach it today) |
| E9: Stored-version stamps? | `kResultsStamp` (`src/store/stored_versions.h:55`, "3.1.2+efillwindow"). | "BUMP IT ... whenever analysis output changes ... the engine (src/search), the scoring (src/core)... A change to how a saved figure is shown is not a bump". | found. A change to stored ticks needs one bump; display-only changes need none. Where tick lists are rebuilt on load was not traced. |
| T5-1: At what moment does this window stop paying the doubling, as the replay prices a chord? | `windows_for_path` (`src/core/replay.cpp:271-276`) copies `deact_tick()` into `ReplayWindow::deact_tick`; used at `replay.cpp:186-200`, the SqOut chord handled separately through `sqout_tick` / `sqout_position`. | replay.h:42-44: "`deact_tick` is the deactivation node D ... the tick the search stamped onto it; it need not be a chord tick." replay.cpp:181-185: "A chord on or before the deactivation node is inside the window whatever its ms says." | split: "reference node for offsets"; `sqout_tick` carries the real stop. |
| T5-2: Does Hydra warn that a window could have been squeezed out, and what would it cost? | `ambiguous_window_warnings` (`replay.cpp:419-470`, reads `w.deact_tick` at 427, 438, 442, 447) | replay.cpp:423-425: "The chords the engine could squeeze out at this D, for this window (core::sqout_chords, D36)". | found |
| T5-3: Which phrase chord did the typed offset name? (`--acts`) | `resolve_sqout_note` (`replay.cpp:300-400`, `sqout_candidates(song, w.deact_tick)`) | replay.h:56-57: sqout_offset_ms is "the same squeeze-out as an ms offset from D, for display, and as typed by hand in `--acts`". | found (D is the offset origin) |
| T5-4: What is this path's identity? | `path_identity` (`src/core/model.cpp:633-643`) | model.cpp:634-635: "Only compared, so the form just has to be unambiguous." | found (a key) |
| T5-5: Does the finished window end on the node the search's graph offered? | record check (`src/search/engine.cpp:2099-2102`), `deactivation_type` (`engine.cpp:1059-1096`) | engine.cpp:2097-2098: "D36: a closed window's record ends on the node SP ended at, tied variants included." engine.cpp:2091-2094: "Its last end is the deactivation node D ... Nothing here invents a value." | found (the search's node) |
| T5-6: Which end did the player pin for this window? | `end_key` (`src/pathbuilder/pb_coach.cpp:74`), `ended_as_pinned` (`src/search/pather.cpp:213-227`), `full_pin` (`pather.cpp:447-453`), engine `pinned_end` (`engine.cpp:564-573`, used 1897-1911) | pb_coach.cpp:70-71: "What tells two ends of one window apart: end_choices lists each once". pather.h:94-97: "the engine keeps only the path whose Star Power ends on that node (Activation::deact_tick, EngineOptions::target_deact_ticks)". | found (node plus chord, a window identity) |
| T5-7: How far does a timing error on the activation note carry to where SP ends? | `transfer_scale_between` / `step_transfer_scale` / `frontend_transfer_scales` (`src/core/squeeze_rating.cpp:12-51`), `rate_activation` (`:164-199`) | squeeze_rating.h:11-13: "The deactivation node itself is not worked out here ... everything below reads it through Activation::deact_tick." squeeze_rating.cpp:172-175: "every offset is measured from the deact node D ... A squeezed-out row is about its phrase, which the SP end itself decides." | found (D is the timing origin) |
| T5-8: When must each press happen? (press schedule) | `activation_json` (`tools/replay_schedule.cpp:396-473`), `final_ends` (508), `end_act` (724), `final_link` (805-811), inside-moved-end loop (984-998), `equal_room_lead_ms` (161-170) | replay_schedule.cpp:146-152: "A squeeze-out: the anchor goes L early ... the end lands r(L - j) early; the late gem goes kSpOutLagMs after that end". | split: needs D as the unsqueezed end (`gap = end_ms - squeeze_ms`, line 164, the 444 ms to cover) |
| T5-9: What does the dump or failure print as the end? | `paths_json` (`tools/replay_json.cpp:121-168`), `tools/replay.cpp:319-326,400,435,601,757,774-780`, `tools/ch_probe/experiments/sp_compare.py:461-467` | replay_json.cpp:108-110: "nominal_deact_tick comes off the stored history (nominal_end())". | found (prints the stored value) |
| T5-10: Where does SP end as played, for display? | `Activation::window_end_tick` (`model.cpp:717-721`), read by `preview_view.cpp:435`, `pb_text.cpp:127,159,167,181`, `track_state.h:277` | model.h:679-684: "Where this window's Star Power is over as played: the squeezed-out chord when core::after_sp_end says it is not after deact_tick() ... otherwise deact_tick()." | found |

Rows T5-1 to T5-10 came from the scout SPE-O2 after the first draft of this plan, and are copied in unchanged.

## Which side of each split wins

**E3, the saved bar: side (b) wins, and the search itself stamps it.** The bar arrives on the squeezed chord, the previous activation's `sqout_tick`, for both kinds. SP-35 says a phrase is credited when its last chord is hit, and the user's words say the same. The search's stamp for an early phrase (the meter end, 65280 on Blight) is the rule the user called made up. After T1 the search stamps the chord for both kinds, and `Path::bank_arrivals` stops overwriting anything.

**E2, where SP ends: side (a) wins.** `Activation::window_end_tick()` gives 64800 on Blight and 13056 on Allies, which is the rule above for both kinds. Side (d), the gauge rewrite, is removed (T2): it is what makes the gauge read 0.0 on Blight. Side (c) is not an owner of this question by its own words ("Where a row sits against the activation's squeezed-out chord"): it is part of E5, which chords SP pays, and E5 is not split. Its answer already agrees with (a) on Blight, so the plan leaves it alone and says so here as a finding against the map's E2 row.

**Side (b) asks a different question, so it keeps its value and loses its claim.** `deact_tick()`'s own comment claims "where this window's SP ends", and on an early squeeze-out that is false: 65280 is where the meter would have run out with nothing squeezed out, not where SP ends. Rows T5-1 to T5-9 give each reader's own words, and none of them asks where SP ends. They ask for the deact node D: the node the search's graph offered (T5-5), the point every squeeze offset is measured from (T5-1, T5-3, T5-7, T5-8), or a key that tells two windows apart (T5-2, T5-4, T5-6, T5-9). Blight's SqOut offset of -444.444 ms is the distance from D to the squeezed chord, so it exists only because D stays at 65280. `CONTEXT.md` already draws this line under "Deact node": "_Avoid_: SP end tick (when the anchored search position is meant)".

So there is one answer to "where does SP end", `window_end_tick()` (row T5-10), and D is not a second answer to it. Every reader of D keeps reading `deact_tick()` with its value untouched. What changes is the words: each comment or printed label that calls D "the SP end" is corrected (T2 for `deact_tick()`'s own comment, T5 for the rest). One printed label does ask where SP ends by its own name, the press schedule's `ends_sp`; T5 points it at `window_end_tick()` and leaves the press timing on D. Nothing in T1 to T4 depends on T5.

**E7** is a split the brief did not put in scope. Both of its evaluations read the window that `window_end_tick` gives, so neither changes value here. It is listed under Out of scope.

## What changes in scores, paths and stored results

Nothing the search decides changes. The saved bar's stamp is written by the search and never read back by it: every use of `banks_` and `bank_tail` in `src/search/engine.cpp` pushes, pops or copies out a tick, and none compares one (P7). The path's score after a squeeze-out comes from `backend_row_value`, which T1 does not touch. When the next activation becomes possible comes from `sp_ready_ms`, which reads the phrase times, not the stamp (E8).

So no score, path string, offered fill or optimal path should move. The main session proves it read-only, with no library run: before the T1 build and after it, run `hydra_replay.exe dump` on the 98 charts of `testdata/input` and compare. Every path's `pathstring`, `total`, `skipped_fill_ticks`, `sp_ready_tick`, `deact_tick` and `sqout_tick` must be identical. The before side already exists: the 98 dumps in the session's scratch folder, `dumps\d0.json` to `dumps\d97.json`, written by `find_late_sqout.py` there.

No `kResultsStamp` bump. The store's path row keeps the path string, the score, counts and the hardest timing (`path_row_of`, `src/store/record_store.cpp:611-633`) and none of the ticks this plan changes. T1 checks the rest of the stored record before it relies on that (P9).

## What the user will see (rule 11)

Every figure below is worked from the record dumps, not captured from the screen. Blight is in the user's library, so the main session captures the real before and after text in T4.

Inputs for Blight activation 1, from `blight.json`: activation tick 57600, squeezed chord 64800, meter end 65280, and the chord is 444.444 ms before the meter end. So 480 ticks last 444.444 ms. The drain box in the user's screenshot says "1 bar / 3.6 s", which is 8 times 444.444 ms, so one bar is 3840 ticks. The window 57600 to 65280 is 7680 ticks, two bars, and the record's `sp_meter` is 2. The chord is at 1:00.000 on the user's clock and the meter end at 1:00.444, which the user's screen writes 0:60.000 and 0:60.444.

Three screens read a changed value: the Preview's gauge readout, the Preview's gauge drawing, and any Path Builder text that reads the gauge's level (`bars_saved_at`). The drain box and the "Star Power is already on here" line read the window's end, which does not change.

| Moment on Blight | Gauge today | Gauge after | Drain box (same before and after) |
|---|---|---|---|
| The activation chord, tick 57600 | (64800 - 57600) / 3840 = 1.875 bars just after the chord | (65280 - 57600) / 3840 = 2.0 bars, "2.0/4" | "SP drain", "1 bar / 3.6 s", "empties in 6.7 s" |
| 0:59.953, 47 ms before the chord | 47 / 3555.6 = 0.013 bars, "0.0/4" | (47 + 444.4) / 3555.6 = 0.138 bars, "0.1/4" | "empties in 0.0 s" (0.047 s) |
| 0:60.000, the squeezed chord | drains to 0.0, then the saved bar: "1.0/4" | drains to 0.125, drops to 0, then the saved bar: "1.0/4" | SP ends here in both |
| 0:60.444, the old meter end | "1.0/4" | "1.0/4" | "SP drain (if activated)", "full meter 14.2 s" |

Two cells are not settled by arithmetic: how 1.875 prints to one decimal, and which side of the step the readout shows at the exact millisecond of the chord. T4 captures both (P12).

The drain box keeps the words "empties in" for the time until SP ends. On an early squeeze-out SP ends before the meter is empty, so the word is loose there. Changing it is a visible change the user did not ask for; it is under Out of scope for the user to rule on.

Allies activation 1, from `dumps\d29.json`: activation 9984, meter end 13056, squeezed chord 13104, 157.9 ms after the end. The window is 3072 ticks and `sp_meter` is 2. Nothing changes: the gauge empties at 13056 before and after, and the saved bar arrives at 13104 before and after, because the search already stamps a late phrase on its own chord.

T5 changes two pieces of printed tool text and no screen. Both lines below are worked from the code's format strings and the Blight figures above (P10), not captured; T5's main-session check captures the real ones (P27).

| Tool output on Blight activation 1 | Today | After T5 |
|---|---|---|
| `hydra_replay schedule`, the chord row at tick 64800 (the squeezed chord) | `"ends_sp": false` | `"ends_sp": true` |
| `hydra_replay schedule`, a chord row at tick 65280, if the chart has a chord there (P26) | `"ends_sp": true` | `"ends_sp": false` |
| `hydra_replay score --acts`, the note for a typed window 57600:65280 with a SqOut of -444.44 ms | `note: window 57600:65280 SqOut -444.44 ms -> phrase chord at tick 64800 (-444.44 ms from the SP end)` | `note: window 57600:65280 SqOut -444.44 ms -> phrase chord at tick 64800 (-444.44 ms from the deact node)` |

Everything else those tools print for Blight stays as it is: the `deact_tick` 65280 and `deact_ms` fields of the dump, the score and the schedule, every `offset_ms`, every press, and the failure line's `deact 65280`. The squeeze-out warning (row T5-2) is printed only for a window with no squeeze-out, where D is where SP ends (row E1), so its words "window ... ends ..." are true and stay.

## Tasks

T1 and T2 are independent and run in parallel. T3 needs both. T4 is the main session's. T5 starts once T1 and T2 are merged, because it edits comments in files they own. T6 runs alongside T1 and T2.

### T1: the search stamps the saved bar on the squeezed chord

Uses rows E3, E4, E8, E9. Owned files: `src/search/engine.cpp`, `src/core/model.h`, `src/core/model.cpp`, `tests/test_sp_end.cpp`.

In `Engine::create_deactivated_path` the squeeze-out's bar is stamped on the squeezed chord for both kinds, and the comment above it points at SP-35 instead of restating a rule. `Path::bank_arrivals` then returns the stored list unchanged, with no overwrite. The comment on `bank_rise_ticks` loses "Path::bank_arrivals gives the arrival as played".

Before relying on "no stamp bump", the executor reads every field `record_of` stores (`src/store/record_store.cpp`) and reports any that holds a bank tick. If one does, it stops and reports; the plan then needs a bump (P9).

Acceptance criteria:

1. On every path of every chart in `testdata/input`, for every activation that squeezed out, the next activation's first stored bar (or the path's first trailing bar when it is the last activation) is that activation's `sqout_tick`. The test reads both from the record. It would fail if the old stamp came back: `dumps\d10.json` path 0 activation 7 has the chord at 441120 and the meter end at 441600.
2. The test counts the early and the late squeeze-outs it checked and requires at least one of each, so an empty loop cannot pass.
3. `Path::bank_arrivals(i)` equals the stored list item by item for every activation and for the trailing list. It would fail if the overwrite stayed and the stamp were wrong.
4. `sp_ready_tick()` is unchanged on every activation: the main session's dump comparison (above) shows it.
5. The dump comparison shows no `pathstring`, `total` or `skipped_fill_ticks` differs on any of the 98 charts.

### T2: one end, and the meter level comes out of the engine's record

Uses rows E1, E2, E6. Owned files: `src/core/model.h`, `src/core/model.cpp`, `src/app/preview_view.cpp`, `src/app/preview_view.h`, `tests/test_sp_meter_curve.cpp`, `tests/test_model.cpp`. T1 and T2 both touch `model.h` and `model.cpp` in different functions; the main session merges T1 first.

The level is carried out as row E6 says. A function on the engine's record answers "bars left at this tick": the measures from the tick to the end in force at that tick, two to a bar, read from `sp_end_steps`. It is the `bars_left` lambda moved, not rewritten, and the lambda goes. `build_sp_meter_curve` calls it and nothing else for a level.

The gauge rewrite goes. The Preview copies each stored step's end as stored, and still draws the window up to `window_end_tick()`. So on an early squeeze-out the last stretch ends above empty on the squeezed chord, and the gauge then drops to the bank, which is the dropped remainder of SP-25.

The comment on `deact_tick()` stops claiming it is where SP ends. It says what the value is in the code's own words (the last stored step's end; `engine.cpp:2091-2093` calls it "the node every backend row's offset was measured against") and points at `window_end_tick()` for where SP ends. The comment on `window_end_tick()` drops "deact_tick() stays the stored end the search, the replay and the press schedule read"; T5 says what those readers ask.

Acceptance criteria:

1. For every activation with an early squeeze-out in `testdata/input`, the gauge's level just before the window's end equals the record function's answer at `window_end_tick()`, and that answer is above zero. It would fail if the rewrite stayed, because the gauge would read zero there.
2. For every activation with no squeeze-out or a late one, the gauge reads zero at `window_end_tick()`. It would fail if the drain ran against the wrong end.
3. At every stored step of every activation, the gauge's level just after the step's tick equals the record function's answer for that tick. The expected side is the record's function, never a typed number and never the display's own sum.
4. No function under `src/app`, `src/ui`, `src/pathbuilder` or `src/render` works out a level from two ticks: a single-owner rule row in `tests/single_owner/rules/` flags `measures_at_tick_f` beside `kMeasuresPerSpBar` outside `src/core`.
5. The existing tests in `tests/test_sp_meter_curve.cpp` that type a level in the window are rewritten to read the record function, per `docs/agents/test-rubric.md` question 6. Any test that pins "the gauge empties on the squeezed chord" is rewritten to criterion 1, and the executor lists each one it changed.

### T3: every screen that reads the level reads the one owner

Uses rows E2, E3, E6. Waits for T1 and T2. Owned files: `src/pathbuilder/pb_text.cpp`, `tests/test_pb_text.cpp`, `tests/test_preview_scene.cpp`, `tests/ui/uitest_preview.cpp`.

`pb_text::bars_saved_at` keeps reading the gauge's curve, which now holds the record's level. The comment in `pb_text.cpp` that says "The end is the Preview's" points at `Activation::window_end_tick` as the engine's. The executor checks whether `bars_saved_at` is ever shown for a fill inside a running window, and reports the text if it is (P11).

Acceptance criteria:

1. The scene's `bank_rise_ticks` for each activation equal the record's stored `bank_rise_ticks` item by item, on every chart in `testdata/input`. It would fail if the scene took the arrival from anywhere else.
2. A `hydra_uitest` case opens a corpus chart whose optimal path has an early squeeze-out, puts the playhead on the squeezed chord's tick less one tick, and checks the gauge readout equals the record function's answer printed by `sp_meter_readout`'s own format. The chart is picked by reading the record in the test, not by name.
3. The same case puts the playhead on a late squeeze-out's meter end and checks the readout's level is zero.

### T4: the main session's checks

Not an agent task. After T1 to T3 are reviewed and built in one build:

1. The dump comparison on `testdata/input` described above. Any difference in a path string, total, skipped fill, `sp_ready_tick`, `deact_tick` or `sqout_tick` stops the merge and goes to the user.
2. GUI check, case 1 of `docs/agents/ui-testing.md` (behaviour and text): `hydra_uitest --test` for the Preview tests T3 names.
3. GUI check, case 3 (how an analysis-backed view draws): the real app on `testdata/input`, the gauge seen draining and dropping on an early squeeze-out. Case 2 does not apply: no layout, width or spacing changes.
4. Rule 11 capture: the main session opens Blight by search (never by scrolling) in the window mode, and reads the gauge readout and drain box at the four moments in the table above, before and after. It shows the user both columns. This settles P12.
5. For T5, after its own build: `hydra_replay.exe schedule` on the 98 dumps before and after, compared with `ends_sp` left out (T5 criterion 3); check 1 again with every squeeze's `offset_ms` added (T5 criterion 4); and on Blight, the chord rows at ticks 64800 and 65280 of the schedule and the `--acts` note, before and after, shown to the user beside the table's lines (T5 criterion 7). Any press, score, offset, `deact_tick` or `sqout_tick` that differs stops the merge and goes to the user.

### T5: the readers of `deact_tick()` keep the node and stop calling it the SP end

Uses rows T5-1 to T5-10. Starts after T1 and T2 are merged. Owned files: `src/core/replay.cpp`, `src/search/engine.cpp`, `src/search/pather.h`, `src/core/squeeze_rating.h`, `tools/replay.cpp`, `tools/replay_schedule.cpp`, `tests/single_owner/rules/core.json`, and the test file that already covers the press schedule (P28).

The rule for every reader is the one under "Which side of each split wins": a reader that asks where SP ends reads `window_end_tick()`, and a reader that asks for the deact node D keeps `deact_tick()`. Here is each row, with the question its own words claim.

**The replay window (T5-1) keeps D.** Its own words call the field "the deactivation node D", and the code measures each chord's offset from it (`replay.cpp:187`) and uses it to know when a window can be dropped from the open list (`replay.cpp:186,200`). Where the doubling really stops on a squeeze-out is `sqout_tick`, through `backend_row_value` (row E5), which this plan does not touch. Only the comment at `replay.cpp:180` changes: "The row's offset from the SP end" becomes the offset from the deact node.

**The squeeze-out warning (T5-2) keeps D and its text.** It asks which chords the engine could squeeze out "at this D". It skips every window that has a squeeze-out (`replay.cpp:421`), so it only speaks about windows where D is where SP ends (row E1). Its sentence "window ... ends ..." is true as printed. No change.

**The typed offset (T5-3), the path identity (T5-4), the record check (T5-5) and the pins (T5-6) keep D.** The typed offset is "an ms offset from D". The identity string is "only compared". The record check compares the record with the node the search's graph offered. A pin is the node and the squeezed chord together. No code line changes in any of them. Two comments do: `engine.cpp:2097` says the record "ends on the node SP ended at", and `pather.h:94-95` says the engine keeps "the path whose Star Power ends on that node". Both become "its deactivation node".

**The squeeze rating (T5-7) keeps D.** Its code says "every offset is measured from the deact node D". The header's first line, "where an activation's Star Power ends, how frontend timing error carries to that end", is reworded to name the deact node. No code line changes.

**The press schedule (T5-8) keeps D for timing and moves one label.** The lead it plans covers the gap from the squeezed chord to D (`replay_schedule.cpp:164`), 444.444 ms on Blight, so `activation_json`, `end_act`, `final_link`, the inside-moved-end loop and the `on_end` test (`replay_schedule.cpp:861`) keep reading `deact_tick()`. The set `final_ends` feeds two things today: `on_end`, which is timing, and the printed chord field `"ends_sp"` (`replay_schedule.cpp:562`). The label's own name asks whether SP ends on this chord, which is row T5-10's question. So `"ends_sp"` is filled from a second set built from `window_end_tick()`, and `on_end` keeps the set built from `deact_tick()`. Neither set is worked out from the other. No press moves: `ends_sp` is only printed here, and `play_chart.py` stores it and never reads it (P25).

**The dumps and failure lines (T5-9) keep D.** They print the stored value under the names `deact_tick`, `deact_ms` and `deact`, which do not claim an SP end. Two lines in `tools/replay.cpp` do: the `--acts` note ends "ms from the SP end" (line 399) and becomes "ms from the deact node", and the comment at line 432 calls D "the end". `sp_compare.py` reads `deact_tick` only as the upper bound of a search whose answer is the dump's own `in_sp` flag (row E5), so it is left alone.

**The display readers (T5-10) already read `window_end_tick()`.** No change. The executor greps `src/app`, `src/ui`, `src/pathbuilder` and `src/render` for `deact_tick()` and reports any call it finds; a search of `src` on 2026-11-19 found calls only in `model.cpp`, `replay.cpp` and `engine.cpp` (P24).

The executor also lists, without changing them, each test whose name or comment says an activation's SP ends at `deact_tick()` on a squeezed-out window. The tests' checks hold either way, because `deact_tick()` returns what it returned before (P29).

Acceptance criteria:

1. In the files of rows T5-1 to T5-7, the T5 diff changes comment lines only. The reviewer reads the diff of `src/core/replay.cpp`, `src/search/engine.cpp`, `src/search/pather.h` and `src/core/squeeze_rating.h`, and any changed line that is not a comment is a finding. `src/core/model.cpp`, `src/core/squeeze_rating.cpp`, `src/search/pather.cpp` and `src/pathbuilder/pb_coach.cpp` have no T5 diff at all.
2. A schedule test, on every chart in `testdata/input`: the chord rows the schedule marks `ends_sp` are exactly the chords whose tick is some activation's `window_end_tick()`, read from the record in the test. It counts the early squeeze-outs it saw and requires at least one. It would fail if `ends_sp` still read D: on `dumps\d10.json` path 0 activation 7 the squeezed chord at 441120 would be unmarked.
3. No press moves. The main session runs `hydra_replay.exe schedule` on the 98 dumps before and after the T5 build (T4 check 5); with the `ends_sp` field left out, the two outputs must be identical byte for byte.
4. No score, offset, identity or pin moves. T4 check 1 is run again after the T5 build and must show the same `pathstring`, `total`, `deact_tick`, `sqout_tick` and every squeeze's `offset_ms` on all 98 charts.
5. A single-owner rule row in `tests/single_owner/rules/core.json` asks "Which code reads the deact node D?", names `Activation::deact_tick` as owner, and flags a `deact_tick()` call in any file under `src` or `tools` other than the ones rows T5-1 to T5-9 name. Its note gives the map row for each allowed file. It would fail if a screen file under `src/app` started calling `deact_tick()`. If the rule format cannot exempt files by name, the executor stops and reports (P30).
6. No comment or printed text in T5's files calls D the SP end. The reviewer greps for the old phrases and finds none: "offset from the SP end" in `replay.cpp`, "the node SP ended at" in `engine.cpp`, "whose Star Power ends on that node" in `pather.h`, "where an activation's Star Power ends" in `squeeze_rating.h`, "from the SP end" in `tools/replay.cpp`.
7. The two printed changes match the table under "What the user will see": the main session captures the real lines on Blight (T4 check 5), which settles P26 and P27.

### T6: the pages that state the old rule

Independent. Owned files: `docs/clone-hero/05-star-power.md`.

SP-26 on page 05 says the gauge "drains to the stored deactivation tick". It is rewritten to say the gauge drains at the meter's rate until SP ends and drops what is left, with its Hydra line naming the record function from T2, in the same merge as T2. SP-37 gains the two worked cases (Blight early, Allies late) as Hydra's reading, and its label stays "Hydra assumes it, never checked".

`CONTEXT.md` is not an agent's to edit. Its "SP meter gauge" entry says "It empties exactly at the deact node" and "as SP ends for an early phrase", and its "Deact node" entry says "The exact chart position where an activation's Star Power ends". Both are false for an early squeeze-out after this plan. The main session puts the new wording to the user (Questions, item 3).

Acceptance criteria:

1. The docs check (`hydra_tests.exe "-tc=docs match code*"`) passes with SP-26's Hydra line naming a function that exists.
2. No line of page 05 says the gauge empties at the deactivation tick: the reviewer greps for "drains to the stored".

## The live steps

None. No step of this plan runs Clone Hero.

## Questions

1. Answered, nothing left to ask: the scout SPE-O2 returned one owner-map row per reader of `deact_tick()`. They are rows T5-1 to T5-10 of the Owner map, and T5 is built from them.
2. For the user, as a statement and not a choice: on a late squeeze-out SP ends where the meter runs out, a moment before the squeezed chord, and the bar arrives on the chord. This follows SP-28 and SP-35, both proven in the game's code. The user asked for this answer to be found.
3. For the user: the new wording of the `CONTEXT.md` entries "SP meter gauge" and "Deact node". Only the user changes a definition. The "Deact node" entry opens "The exact chart position where an activation's Star Power ends". On an early squeeze-out that is not so: on Blight activation 1 the deact node is tick 65280 and Star Power ends on the squeezed chord at tick 64800. The rest of the entry holds after this plan: "backend squeeze timings are measured against it", and every reader asks `Activation::deact_tick`. The entry's own last line already keeps the two apart: "_Avoid_: SP end tick (when the anchored search position is meant)". The "SP meter gauge" entry says "It empties exactly at the deact node" and "as SP ends for an early phrase"; after T2 the gauge on an early squeeze-out still holds a little on the squeezed chord and drops it there.

## Out of scope, for the user to approve

- The function names `core::after_sp_end` and `offset_from_sp_end`. Both take whatever end they are handed, and several callers hand them the deact node D (`replay.cpp:186-187`, `model.cpp:719`). T5 corrects comments and printed text only; renaming the functions is not done here. SP-28's Hydra line on page 05 names `offset_from_sp_end` as Hydra's "one end point" and is left as it is.
- The printed field names `deact_tick`, `deact_ms` and `nominal_deact_tick` in the dump, score and schedule JSON. They name the deact node and keep their names and values.
- Blight's activations after the first. Every Blight figure in this plan is activation 1's.
- Row E7: SP-running is evaluated in two places that a test pins together. Not merged here.
- The drain box's words "empties in" on an early squeeze-out, where SP ends before the meter is empty. The text is left as it is.
- SP-37's label. No live run checks the squeeze-out rule in this plan.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| The saved bar is stamped on the squeezed chord for both kinds | "the search itself stamps it" | T1 | T1 criterion 1, fails on `d10.json` activation 7 with the old stamp |
| The test cannot pass on an empty loop | T1 criterion 2 | T1 | at least one early and one late case counted |
| `bank_arrivals` overwrites nothing | "stops overwriting anything" | T1 | T1 criterion 3 |
| No score, path string, offered fill or optimal path changes | "Nothing the search decides changes" | T1, T4 | T1 criteria 4 and 5; T4 check 1 |
| No stored figure changes, so no stamp bump | "No `kResultsStamp` bump" | T1 | the executor's read of `record_of`; stops if a bank tick is stored |
| The gauge reads above zero on an early squeezed chord | "ends above empty on the squeezed chord" | T2 | T2 criterion 1 |
| The gauge reads zero at the end with no squeeze-out or a late one | "empties at 13056" | T2 | T2 criterion 2 |
| The level is the engine record's, never the display's | "It is the `bars_left` lambda moved" | T2 | T2 criteria 3 and 4 |
| The gauge rewrite is gone | "The gauge rewrite goes" | T2 | T2 criterion 1 |
| Old tests that pin the old claim are rewritten, not kept | T2 criterion 5 | T2 | the executor's list; the reviewer reads each |
| The scene's bar arrivals are the record's | T3 criterion 1 | T3 | T3 criterion 1 |
| The readout on screen is the record's level | "reads the one owner" | T3 | T3 criteria 2 and 3 |
| The GUI checks ui-testing.md asks for are run | T4 checks 2 and 3 | T4 | the main session's run output |
| The user sees before and after text from Blight | "captures the real before and after text" | T4 | T4 check 4 |
| Every reader of D keeps `deact_tick()` with its value untouched | "Every reader of D keeps reading `deact_tick()`" | T5 | T5 criterion 1 (comment-only diff, read by the reviewer) and criterion 4 |
| Scores, offsets, identity strings and pins do not change | "No score, offset, identity or pin moves" | T5, T4 | T5 criterion 4; T4 check 5 |
| The press schedule's presses do not change | "No press moves" | T5, T4 | T5 criterion 3; T4 check 5, byte for byte with `ends_sp` left out |
| `ends_sp` marks the chord SP ends on, from `window_end_tick()` | "`\"ends_sp\"` is filled from a second set built from `window_end_tick()`" | T5 | T5 criterion 2, fails on `d10.json` activation 7 if it still reads D; needs at least one early squeeze-out |
| The two sets in the schedule are not worked out from each other | "Neither set is worked out from the other" | T5 | T5 criterion 2 (the label) and criterion 3 (the timing) fail separately |
| No comment or printed text calls D the SP end | "each comment or printed label that calls D \"the SP end\" is corrected" | T2, T5 | T5 criterion 6, the reviewer's greps; T2 for `model.h` |
| The squeeze-out warning's text does not change | "No change." under T5-2 | T5 | T5 criterion 1: `replay.cpp` has comment changes only |
| A new caller of `deact_tick()` outside the listed files is caught | T5 criterion 5 | T5 | the single-owner rule row; fails on a call from `src/app` |
| The printed changes are shown to the user from Blight | the tool-output table | T5, T4 | T5 criterion 7; T4 check 5 |
| Tests that call D the SP end are listed, not silently kept | "The executor also lists" | T5 | the executor's list in its report; the reviewer reads it |
| The executor reports any `deact_tick()` call in a screen folder | T5, row T5-10 paragraph | T5 | the executor's grep, and criterion 5 afterwards |
| Page 05 stops stating the old gauge rule | T6 | T6 | T6 criteria 1 and 2 |
| `CONTEXT.md` is changed only by the user | "not an agent's to edit" | T6 | Questions item 3; no task owns `CONTEXT.md` |
| Blight at 0:59.953 reads more than 0.0 | the rule 11 table | T2, T4 | T2 criterion 1 for the rule; T4 check 4 for Blight itself |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The game ends SP when the clock reaches one stored end time, and the end is a time. | data | `docs/clone-hero/05-star-power.md` block SP-28, Label "Proven in the game's code" | checked 2026-11-19 |
| P2: At the end the game sets the meter to zero and drops unspent SP. | data | `docs/clone-hero/05-star-power.md` block SP-25, Label "Proven in the game's code" | checked 2026-11-19 |
| P3: While SP is on the meter drains by the ticks that passed. | data | `docs/clone-hero/05-star-power.md` block SP-22, Label "Proven in the game's code" | checked 2026-11-19 |
| P4: A phrase is credited when its last chord is hit, with that hit's own SP state. | data | `docs/clone-hero/05-star-power.md` block SP-35, Label "Proven in the game's code" (its conditions at lines 201 to 208 "were not read") | checked 2026-11-19 |
| P5: A squeeze-out hits the phrase's last note late so SP has ended and the phrase stays for later; one rule covers the early and the late kind. | unchecked | `docs/clone-hero/05-star-power.md` block SP-37, Label "Hydra assumes it, never checked" | question 2 |
| P6: The user said SP ends on the squeezed-out chord and a bar is saved. | ruling | the user's message as the main session quotes it in brief `SPE-P`: "when you squeeze out the starpower ends on the squeezed out chord and you get a bar saved to use later. thats it." | checked 2026-11-19 |
| P7: The search never reads the saved bar's stamp to decide anything. | code | `src/search/engine.cpp:670-676,976-980,1038-1039,1114,1336,1573,1746,1770,1782,1821` (every `banks_` and `bank_tail` use pushes, pops or copies out) | checked 2026-11-19, by grep and by reading lines 905-992 and 1099-1157; the copy-out lines 1336 to 1821 were read as grep lines only |
| P8: The search stamps an early squeeze-out's bar on the meter end and a late one's on its chord. | code | `src/search/engine.cpp:1109-1114` | checked 2026-11-19 |
| P9: The store keeps none of the ticks this plan changes. | unchecked | `src/store/record_store.cpp:611-633` (`path_row_of` holds none; the rest of `record_of` was not read) | T1 |
| P10: Blight activation 1: activation 57600, squeezed chord 64800, meter end 65280, chord 444.444 ms before the end, 2 bars. | chart | `C:\Users\Patrick\AppData\Local\Temp\claude\bba5e4e61f1e9ed6\d85370f4-a09f-42fa-b527-e5d6ac770ddc\scratchpad\blight.json`, path 0, the engine's record of `C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\HopH2O\Hail The Sun\Hail The Sun - cut. turn. fade. back\Hail The Sun - Blight\notes.mid` | checked 2026-11-19 (the dump was read; the chart file was not opened) |
| P11: `bars_saved_at` is never shown for a fill inside a running window. | unchecked | `src/pathbuilder/pb_text.cpp:45-47` (the function; its callers were not read) | T3 |
| P12: The readout's text at the activation chord and at the exact millisecond of the squeezed chord on Blight. | unchecked | `src/app/preview_view.cpp:815-819` (the format; the step side and the tie were not traced) | T4 |
| P13: Allies activation 1: activation 9984, meter end 13056, squeezed chord 13104, 157.9 ms after the end, 2 bars. | chart | `C:\Users\Patrick\AppData\Local\Temp\claude\bba5e4e61f1e9ed6\d85370f4-a09f-42fa-b527-e5d6ac770ddc\scratchpad\dumps\d29.json`, path "0- 1- 1 0", the engine's record of `testdata/input/common/Summer Blast _25 Setlist/Tier 3/Mutemath - Allies/notes.chart` | checked 2026-11-19 (the dump was read) |
| P14: `testdata/input` holds both kinds: 38 squeeze-outs at or before the meter end and 21 after it, over 98 dumps. | chart | `C:\Users\Patrick\AppData\Local\Temp\claude\bba5e4e61f1e9ed6\d85370f4-a09f-42fa-b527-e5d6ac770ddc\scratchpad\dumps`, files `d0.json` to `d97.json`; one early case is `d10.json` path 0 activation 7 (441120 against 441600) | checked 2026-11-19 |
| P15: The gauge reads 0.0 on Blight's chord today because the Preview replaces the last step's end with the window's end. | code | `src/app/preview_view.cpp:435-440` and the `bars_left` lambda at `src/app/preview_view.cpp:145-149` | checked 2026-11-19 |
| P16: `window_end_tick()` gives the squeezed chord when it is not after the last step's end, else that end. | code | `src/core/model.cpp:717-721` | checked 2026-11-19 |
| P17: `bank_arrivals` overwrites the first entry with the previous activation's `sqout_tick`. | code | `src/core/model.cpp:574-579` | checked 2026-11-19 |
| P18: The drain box's "empties in" reads the window's end, not the meter's. | code | `src/app/preview_view.cpp:673-678` | checked 2026-11-19 |
| P19: `kResultsStamp` is bumped for anything a fresh analysis would compute differently, and not for how a saved figure is shown. | code | `src/store/stored_versions.h:43-55` | checked 2026-11-19 |
| P20: `CONTEXT.md` states the old gauge and deact-node wording. | code | `CONTEXT.md:962-975` and `CONTEXT.md:733-739` | checked 2026-11-19 |
| P21: A reviewer or executor never rewrites a `CONTEXT.md` definition. | code | `docs/agents/brief-preamble.md`, "Project words and rulings" | checked 2026-11-19 |
| P22: No reader in rows T5-1 to T5-9 asks where SP ends; each asks for the deact node as an offset origin, the search's node or a key. | code | `src/core/replay.h:42-44,56-57`, `src/core/replay.cpp:180-200,271-276,300-313,419-471`, `src/core/model.cpp:633-643`, `src/search/engine.cpp:564-573,1059-1096,1897-1911,2090-2102`, `src/pathbuilder/pb_coach.cpp:70-74`, `src/search/pather.h:92-97`, `src/search/pather.cpp:213-227,447-453`, `src/core/squeeze_rating.h:1-13`, `src/core/squeeze_rating.cpp:12-51,164-199`, `tools/replay_schedule.cpp:146-170,396-475,508,724,805-811,984-998`, `tools/replay_json.cpp:107-168`, `tools/replay.cpp:392-441,753-783`, `tools/ch_probe/experiments/sp_compare.py:456-469` | checked 2026-11-19, each range read |
| P24: Under `src`, `deact_tick()` is called only in `model.cpp` (638, 718), `replay.cpp` (271) and `engine.cpp` (2100); under `tools`, only in `replay_json.cpp` (133), `replay_schedule.cpp` (416, 508, 724, 806, 990) and `replay.cpp` (779). | code | a grep for `deact_tick\(\)` over `src` and `tools` on branch `claude/spm-plan` | checked 2026-11-19 (a grep; fields named `deact_tick` on `ReplayWindow` and `PinnedWindow` are separate and are rows T5-1 and T5-6) |
| P25: The schedule's `ends_sp` field is printed and decides no press: `final_ends` is read at `replay_schedule.cpp:562` for the label and at 861 for `on_end`, and `play_chart.py` stores `ends_sp` (lines 199, 333) and reads it nowhere. | code | `tools/replay_schedule.cpp:500,508,562,861`, `tools/ch_probe/experiments/play_chart.py:191-199,333`; a grep for `ends_sp` over every `.py`, `.cpp` and `.h` on the branch | checked 2026-11-19 (a grep that finds no other reader; T5 criterion 3 proves no press moves) |
| P26: Whether Blight has a chord on tick 65280, the row the schedule marks `ends_sp` today. | unchecked | `tools/replay_schedule.cpp:562` (the label; Blight's chord list was not opened) | T5 |
| P27: The exact text of the `--acts` note and the `ends_sp` rows on Blight, before and after. | unchecked | `tools/replay.cpp:397-401` and `tools/replay_schedule.cpp:562` (the format strings; no run was made) | T5 |
| P28: A test file already covers `tools/replay_schedule.cpp`, for T5 criterion 2 to sit in. | unchecked | `tools/replay_schedule.cpp` (the code; its tests were not searched for) | T5 |
| P29: `deact_tick()` returns the same value after this plan, so the 43 reads of it in 15 test files keep their meaning. | code | `src/core/model.cpp:712-715`, which no task changes; a count grep for `deact_tick\(\)` over `tests` | checked 2026-11-19 (the tests' own wording was not read; T5's executor lists it) |
| P30: The single-owner rule format can exempt named files from a pattern. | unchecked | `tests/single_owner/rules/app.json:75-95` (one rule read: it has `pattern`, `scope`, `must_match`, `must_not_match`; no exempt list was seen) | T5 |
| P31: The user's filed ruling says there is never more than one answer to where Star Power ends. | ruling | `docs/agents/rulings/code-and-engine.md` line 12, under `## Confirmed`: "there should never be two different answers for where star power ends. ever." | checked 2026-11-19 |
| P32: If a reader of D read 64800 on Blight instead, the record check would throw, pins would match no node, the offsets of T5-3, T5-7 and T5-8 would lose their 444 ms, and identity strings would change. | unchecked | `src/search/engine.cpp:2099-2102` (the throw was read; the other effects are the scout SPE-O2's report) | T5 (criteria 3 and 4 hold every one of those values still) |
| P23: The other late cases the brief names (Epidermis, SoundHaven - Focus, Four Year Strong - Wasting Time, Tapestry, Stabwound, Pitch Dark ratio) behave as Allies does. | unchecked | `C:\Users\Patrick\AppData\Local\Temp\claude\bba5e4e61f1e9ed6\d85370f4-a09f-42fa-b527-e5d6ac770ddc\scratchpad\dumps` (only the count of 21 late cases was read, not each chart by name) | T1 |
