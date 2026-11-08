# player-audit-S2-F3: families 3, 4, 8 (main fd93c684, read in worktree player-audit-plan)

All claims are "by reading" unless marked "proven by <file>". I ran nothing. Where a line says "quote", the quote was read in the named file.

## What I read

Fully: tools/ch_probe/engine.py, tools/ch_probe/constants.py, tools/replay_schedule.h, tools/replay_schedule.cpp (all 989 lines), tools/ch_probe/experiments/score_compare.py (lines 1-250), tools/ch_mcp/compare_scorestats.py, tools/ch_mcp/kit_tools.py lines 40-330, play_chart.py lines 346-460 and 700-820 (on top of S1's read of the whole file), sp_compare.py lines 55-190 and 354-422. By search only: tools/replay_json.cpp, tools/replay.cpp, tools/ch_mcp/game_tools.py, tools/ch_mcp/agent/kit.ts and the other .ts files, input_driver.py, sp_tools/bot_run/trace_hits constants. The Clone Hero pages: the label of every TM, SP, SC and FL block and the full text of TM-26, TM-27, TM-50, SP-19, SP-20, SP-21, SP-45, SP-47, FL-09, FL-31, SC-10, SC-13, SC-31, SC-32, SC-35, SC-38 and parts of IN-76 to IN-86.

## The big finding for family 3: the schedule and the Python player are two full copies of the Star Power planning

`tools/replay_schedule.cpp` (merged, TC2) works out windows, leads, order groups, far-press refusals, fallbacks and rooms. `play_chart.py` still works all of them out again. Nothing in Python reads the schedule file (grep of tools/ and tests/ for `replay_schedule`, `schedule_json`, `"presses"`: only replay.cpp, replay_schedule.* and tests/test_replay_schedule.cpp). The C++ header says "The player reads one file and works no Star Power rule out itself" (replay_schedule.h:1-3), which is not yet true of the code. The plan (docs/superpowers/plans/2026-11-08-replay-press-schedule.md, "Rulings that bind this plan", line 29) quotes the user: "why is the python still doing its own thing? the whole point of this plan is to hand off all the starpower planning to hydra which already knows what to do", then "yes revise the plan". So the copies are known and the plan's later Python tasks (TP) are meant to remove them. They differ today, so each is a split until then.

### F3-1. The per-note hit window: three copies, one fixed number elsewhere

Homes:
- play_chart.py:304-344 `window_side_ms` / `window_sides`, using constants.py:259-272 (WINDOW_FORMULA_NORMAL_*, CAP_FROM_GAP_MS, FLOOR_UP_TO_GAP_MS).
- replay_schedule.cpp:19-29 and 121-155 (`half_gap_ms`, `whole_window_ms`, `window_side_ms`, `chord_sides_ms`) with its own copies of the four formula numbers and the 37.5 / 85 clamps.
- Hydra's fixed `kHitWindowMs` (src/core/model.h:104 per TM-26's Hydra line), used by every screen and by the search.
- Game truth: TM-17 to TM-22, TM-51 (all "Proven in the game's code").

Inputs and answers, side of one chord (same in Python and C++ by reading, I did not run either):

| Case | Python | C++ | Hydra screens |
|---|---|---|---|
| neighbours 1000 ms both sides | 85.72 | 85.72 | 85 |
| no neighbour either side (one chord) | 2 x 85 -> 85.72 | `half_gap_ms(nullopt)` = 85, 85.72 | 85 |
| both gaps <= 75 ms | 48.15 | 48.15 | 85 |
| gap exactly 0 (two chords same ms) | treated as no neighbour, 85 | `*gap_ms == 0.0` -> 85 | 85 |
| gap negative (list out of order) | max(gap/2, 37.5) = 37.5, no error | same | 85 |

The formula numbers are written twice (constants.py and replay_schedule.cpp:26-29) and I found no test tying the two copies (a grep for the digits `0.0110924369747899` finds them in constants.py, replay_schedule.cpp, test_play_chart.py, docs, SPEC.md; tests/test_replay_schedule.cpp:364 only checks that the header names kFrameMs and kRoomMarginMs). Open: whether tests/test_replay_schedule.cpp pins `formula_*` from the schedule's "constants" object; I read only the grep hits.

Split A: Python and C++ per-note window vs Hydra's fixed 85 (TM-22 says so itself; TM-29 "Hydra assumes it"). The user ruled "hydra does not currently account for the dynamic hit window, although that is a good idea for a new feature" (plan line 31), so the screens stay at 85. This one is covered by a quoted ruling for the screens, not for the two copies of the formula.

Split B (kind 8): both copies use one side for early and late. TM-26 (Proven in a live run) says early reaches past 88.43 and late ends before 84.92 on an 85.72 side, and TM-27 sends the question to Q-32. The page's own Hydra line says "Ruling: none found".

### F3-2. The player's timing numbers: six constants twice, one of them different

| Name | Python (play_chart.py) | C++ (replay_schedule.cpp) | Source named in comments |
|---|---|---|---|
| press lead | PRESS_LEAD_MS 2.0 (:78) | kPressLeadMs 2.0 (:43) | D51 (C++); plans (Python) |
| end lead | SP_END_LEAD_MS 15 (:83) | kSpEndLeadMs 15 (:49) | "question 3 of the optimal-score run plan", file removed |
| out lag | SP_OUT_LAG_MS 15 (:88) | kSpOutLagMs 15 (:54) | question 2 of player-refinements; plan line 111 quotes "yes all create handoff" (session 490d6781) |
| order gap | CHORD_ORDER_GAP_MS 8 (:113) | kChordOrderGapMs 8 (:61) | user: "8 ms" and "Only where it matters" (player-refinements.md:204) |
| frame | FRAME_MS 3.0 (:101) | kFrameMs 3.0 (:35) | freak-fixes Q1 (a); plan P38 quotes "Yes, start it" |
| margin | ACT_LEAD_MARGIN_MS 6.0 (:95) | kRoomMarginMs 3.0 (:66) plus kJudgeLateMs 6.0 | freak-fixes Q2 (a), margin 3.0 approved by "Yes, start it" |
| judged-late | none (Python uses FRAME_MS once, at :1115) | kJudgeEarlyMs 3, kJudgeLateMs 6 | TM-27, SP-47 |

So the margin is a split: Python 6.0 ("ACT_LEAD_MARGIN_MS", the stand-in for two frames) against C++ 3.0 room after two named frames. The freak-fixes plan Q2 says the Python one changes "meaning, and its value" once frames are named, and P38 says the user approved 3.0. The Python file on main still says 6.0 (play_chart.py:95), so either the Python task (T2 of freak-fixes) is not on main or it was replaced; I did not check git history. Open: whether freak-fixes T2 ever merged.

### F3-3. How far to press an activation early or late for a squeeze: two different formulas

Python `activation_leads` / `_solve_lead` (play_chart.py:347-428): lead = need / ratio + 6, with need = end + 15 - chord.ms - side (squeeze-out) or chord.ms + 15 - end - side (squeeze-in). Refuses when lead > side - 6. Always anchors on the activation chord and raises if `end.anchor_index != act.index` (:399-404).

C++ `equal_room_lead_ms` + `anchor_lead_ms` (replay_schedule.cpp:224-327): equal room on both presses, using both chords' sides and the two judged-frame figures; anchors on `squeeze_anchor_tick` (the overfill chord when the cap clamped); refuses when feasible room < 3.

Inputs and answers (from the freak-fixes plan's own table, lines 60-66, by reading; I did not recompute): Bomber chord 147, need 45.27, ratio 1.0: Python 51.27 (45.27 + 6), plan says the new rule gives 60.27. Freak chords 1062 to 1144: Python today fails (room -6.00 for every led activation, plan line 66), new lead 60.68 early. Edge: an anchor that is not the activation (Freak 263 style overfill): Python raises, C++ plans it.

This is a rule worked out twice with different answers on the same input. The plan states the C++ one is the intended one.

### F3-4. Order groups, inside groups, far-press check, kept gems: C++ "moved" from Python, both still live

C++ comments say "_order_groups in play_chart.py, moved" (replay_schedule.cpp:343), "_squeeze_out_inside_groups ..., moved" (:368), "_refuse_far_presses in play_chart.py, moved" (:391), "_timed_presses in play_chart.py, moved" (:845). Python still has all of them (`_order_groups`, `_squeeze_out_inside_groups`, `_refuse_far_presses`, `_kept_gems` at play_chart.py:700-720, `_timed_presses`). Differences I could see by reading:
- Python `_kept_gems` raises ValueError in all_but_top_pad for a chord with two gems of the top pad's color. C++ `kept_gems` cannot see that case; its comment says "A Chord holds one note per colour". Cymbal and tom of one colour: Python's `_KEY_FOR_PAD` keys by (colour, cymbal); C++ keys by colour alone. If the dump can hold a yellow tom and a yellow cymbal on one tick (open: not read), the two answer differently.
- Python `_refuse_far_presses` (S1 C-notes: 632-673) raises on the first bad press for the whole load; C++ records one refusal line per chord and carries on (`far_refused`, :934-946).
- C++ drops a press into `refusals` when the late gem pays less than an inside gem; Python raises.
- C++ adds `fallback` rows (join_group, plain) for a read end that leaves no room; Python's version of the same fallback is the freak-fixes T2 work (retimed column). I did not find `retimed` or `join_group` in play_chart.py (grep for `retim|fallback|join_group` found only `retime_from_end`), so Python does not do the fallback yet.

### F3-5. Rules re-stated in the judging scripts (copies, not splits so far)

- `sp_compare.py:109-122, 176-178`: SP_MULTIPLIER 2, MEASURES_PER_SP_BAR 2, SP_CAP_BARS 4, SP_ACTIVATION_BARS 2, QUARTERS_PER_MEASURE 4; READY_FRACTION = 0.5; `hydra_full_meter_ticks` = resolution x 4 x 2 x 4 (printed as "resolution x 32", :414). Comment says these are "copied because this script cannot read C++". Game side: SP-09/SP-10/SP-11 (SP-11: "The full bar is 30 times the resolution; Hydra's is 32 in 4/4", Proven in a live run; the page records the split). So the check prints Hydra's 32 x resolution beside the game's constant and calls it a pass if the game value is constant; `_check_sp_max` (:404-421) passes whenever `sp_max` is the same in every row, whatever its value. Inputs: sp_max constant 30 x res: PASS (Hydra number only shown beside). sp_max changing: FAIL. Hydra 32 vs game 30: no failure.
- `sp_compare.py:549-590`: expected multiplier at a chord = chord.multiplier x 2 if in Star Power, else 1 (SC-03/SC-05/SP-19 re-stated).
- `score_compare.py:150-197`: the "running difference" rule and the "lagged" and "ghost_accent" statuses. The "ghost_accent" status quietly moves the running difference at any chord with a ghost or accent gem or the chord after a lagged one. The comment justifies it with "A keyboard press carries no strike velocity" (SC-14, Proven in a live run, keyboard only). On the kit route the strengths are set from `pad_axis` (SC-13, SC-18 live), so a ghost or accent score difference is now a real signal that this rule excuses. Candidate (kind 8 / 6): the forgiveness was written for the keys route and still applies to the kit route.
- `score_compare.py:64`: MIN_HIT_PERCENT 98, "the overlay spike plan, Task B step 3". No ruling quoted; I did not open that plan.
- `compare_scorestats.py:94-99`: `total_notes` passes when the game's number equals Hydra's gem count OR chord count ("matches gems and chords / neither"). Page 06 (line 427) lists `total_notes` as "gems in the chart | Unknown". So the check accepts either meaning of a key the page has not settled. Also keys `sp_phrases_total`, `accent_notes_hit`, `ghost_notes_hit` are Unknown on SC-37's table (lines 432, 442); `base_score` is "reported only", correctly. `score` is compared to `final.total`, backed by SC-31 (Proven in a live run).
- `kit_tools.pad_axis` (kit_tools.py:262-283): axis = 0.5 + v / 254, accent and plain at 127, ghost at threshold - 1. Backed by SC-13 (Proven in the game's code) and IN-86 (Proven in the game's code). The comment "A plain gem's velocity is never read, so it presses like an accent" is a claim; SC-13 says the helper "gives -1 unless the player is on a registered MIDI kit or holds custom controller id 6"; I did not find a block saying a plain gem's byte is unread. Marked candidate (F8-6).
- `engine.pressed_input_hit` (engine.py:46) and constants OFF_SCORE comment: "the score rises only on a hit, proven by play_chart". This is the second home of "hit means score rose". TM-45 (Proven in the game's code) says a missed lane note is granted after a recent lane press, FL-31/SP-20 say a missed fill-end note is granted, and the plan's P7 says a declined chord's granted points land in the next row. So "score rose" is not "pressed note hit" in at least three cases the pages already record. Split: engine.py:46 docstring ("rises only on a hit") vs SP-20/FL-31/TM-45.

## Family 4: every path that presses fewer, other, or later gems than the chart has

For each: where, what, the user's words or proven fact, quoted with a file and line, or "none found" and where I searched.

### F4-1. Declined chord sends nothing / one kick / all but the top pad

Homes: play_chart.py:120-134 `DECLINE_*`, `_kept_gems` (:709-728), `declined_chord_indexes` (:690-702), `sends = not (chord.declined and not chord.lanes)` (:1114); replay_schedule.cpp:157-205 `kept_gems`, :571-577 and :596-607; game_tools.py:224-228 `declined_line`.
What it does: a fill the path passes over (an activation chord Hydra does not use) is not pressed, or pressed with fewer gems, so the game does not activate Star Power there. Chords of the fill that end in only a kick, and chords with no pad gem, send nothing in all_but_top_pad.
Rulings and facts found:
- All_but_top_pad and the nothing-for-kick-only rule: player-refinements.md line 111: "The user answered all eight on 2026-11-07, each with the recommended side: 'yes all create handoff' (session 490d6781, answering 'Take all eight recommendations and start the build?'). So ... the new decline mode is `all_but_top_pad` ...". Line 13 of the same plan states the reason for "send nothing" on a kick-only fill chord. The user's yes is a blanket yes to eight recommendations; the exact wording of the decline question is not in the card (`docs/sessions/2026-11-07-490d6781.md` keeps only the typed words). Covered only through that blanket yes.
- `kick_only` and `nothing` (the first two modes): I found no user quote naming them. Session cards 5b92c813 and f1fabefd list the user's words ("Yes, take all seven", "Go, LS-O1", ...) and mention "declining chord 405" in the topic line only. `docs/agents/clone-hero-mcp.md` line 50 says the mode "is the user's pick per run" (I read the plan, not that file's full line). Searched: docs/agents/rulings/*.md (no `declin`, `kick_only` hit for this sense), docs/audit/2026-10-03-fix-decisions.md, docs/sessions/*.md, docs/sessions/index.md. Result: none found for the original two modes except through "Yes, take all seven" (5b92c813, 02:02 UTC), which is a blanket yes.
- The game fact that makes it work: FL-31 and SP-20 (both "Proven in the game's code"). FL-31 itself says the no-activation reading of the grant is "read from the slot spacing" and a single run (LS-O3 run 1, Senri chord 405) tried it. So the label is Proven in the game's code but the key sentence rests on spacing plus one run. Candidate under kind 8.
Table:
| Declined chord | Python all_but_top_pad | C++ all_but_top_pad |
|---|---|---|
| kick + yellow + green (top = green? by FL-09, pad order highest) | kick, yellow | kick, yellow |
| kick only | nothing | nothing (`top == Kick`, :201) |
| two gems of the top colour | ValueError at load | cannot occur (one note per colour) |
| empty `notes` | `[]` then still `press([], [])`? S1 C15 | `notes.empty()` -> nothing, but `press_groups` returns one empty group so a row with no gems is written (:352) |
Split: Python and C++ answer "which gem is the top pad" from different data. Python: `_flagged_gem`, the score file's list order (S1 C9). C++: `Chord::activation_note().colortype`. FL-09's Hydra line names `Chord::activation_note`.

### F4-2. A chord split into order groups (activating gem first, then lowest multiplier first, 8 ms apart)

Homes: play_chart.py `_order_groups` (:488-527), `CHORD_ORDER_GAP_MS` (:113), message `then` groups (kit_tools.py:286-330, kit.ts:490-540); replay_schedule.cpp:343-366 `press_groups`, :783-789.
Ruling: the user's "Only where it matters" and "8 ms" (player-refinements.md:204, "The user answered two questions in chat on 2026-11-07").
Proven fact: SC-38 (Proven in a live run, LS-R1b run 1, ls-r1-run1 press-1.csv) says one-frame chords were scored in the game's order, not Hydra's, and the same block says "Whether the order is a fixed lane order, and which gem activates, was not read in the code." SC-10 (Hydra assumes it, never checked): Hydra prices the best order. SC-21 (Unknown): whether the activating note itself scores doubled.
So the presses move gems (not time of the chord) by 8 ms steps to force the order Hydra priced, resting on a live observation whose cause is unread. Also: the first group keeps its lead and later groups arrive 8 ms later, up to 8 x (n-1) ms; for a 4-group chord the last gem is 24 ms later, inside any side (>= 48) but the check applies only to the first group's drop test (S1 C2).
Split: SC-10 says Hydra assumes the player chooses the order; the player forces it with a gap that the game's frames may collapse (a frame is about 3 ms, so 8 ms is two to three frames; the agent puts the later group down "in the first frame at or after after_ms", kit.ts:523-525). I cannot tell from reading whether 8 ms always crosses a frame. Open: a run with groups on a 330 fps game (proven fact for the ordering, not for the 8 ms).

### F4-3. Squeeze-out: the late gem pressed alone after the end, inside gems pressed early

Homes: play_chart.py `_timed_presses` (:588-607), replay_schedule.cpp:844-878.
What: gem 0 of the chord (the one Hydra's FirstNote rule pays outside Star Power) goes `SP_OUT_LAG_MS` after the end; every other gem goes `SP_END_LEAD_MS` before it. A late gem with a lower multiplier than an inside gem is refused.
Ruling/fact: SP_OUT_LAG_MS is "question 2 of docs/superpowers/plans/2026-11-07-player-refinements.md" (C++ comment, :51-53), answered by the same blanket yes. SP-28 (Proven in the game's code) gives the end rule: the first frame whose clock reaches the end. SP-29 (Unknown): whether a hit is scored before the end check in each frame. The 15 ms lead and lag come from that unknown, so the numbers are judgements, not game facts. The "FirstNote rule pays outside" claim is Hydra's, from the score file's gem order (S1 C9). Searched the CH pages for FirstNote: no block under that name (I did not open page 02 for it; open).

### F4-4. Fallbacks that change what is pressed: late gem joins the last inside group; squeeze-in chord pressed plain

Home: replay_schedule.cpp:868-891 (`fallback` rows). Python: none found (grep above).
Ruling: freak-fixes Q3 (a), approved by "Yes, start it" (plan P38 in replay-press-schedule.md:343). The question text (freak-fixes.md:86) reads "When the read end leaves the late gem no room ... Sides: (a) press the late gem wit..." (I read it trimmed). Covered by a blanket yes.
Fact base: TM-27 and SP-47 give the one-frame judge lag; the second frame ("kJudgeLateMs = 2 frames", :40) is a worst case with no block (F8-3).

### F4-5. Far-press refusal: a chord is not played at all

Homes: play_chart.py `_refuse_far_presses` (:632-673), also the run-time drop (:1115); replay_schedule.cpp:391-422 `far_press_refusal`, :934-946.
What: when a press would leave its own chord's side, or fall inside the earlier same-pad chord's side, or inside a later same-pad chord's side that the player does not press, the load refuses (Python raises, C++ adds a refusal line). At run time the press is dropped when `raw_ms + 3 > chord + side` (S1 C2) and the row says DROPPED.
Ruling: the refinements plan line 111, "the load check takes the game's window and lanes", under the blanket yes. The user's later answer about a dropped row: freak-fixes Q4 (a) "`hit` holds `dropped`, as it holds `declined` today" under "Yes, start it".
Fact base: TM-50 (Proven in a live run). C++ comment at replay_schedule.h:170-174 says "the game gives a press inside two unhit same-pad windows to the earlier note (the matching rule on docs/clone-hero/03-timing.md)". TM-50's own text says "Not proven: whether an early press inside two windows also goes to the earlier note; any pad but the kick for the two-window case; any pair but these". Its Ruling line says "none needed: no Hydra rule depends on it", but this C++ rule now depends on it for every pad. Candidate (kind 8 and 4).

### F4-6. Retiming a press after the end is read

Home: play_chart.py `retime_from_end` (:449-459) and S1 C11 (no recheck of the early edge); C++ `reads` and `aim_end_ms` in the schedule (:706-759), with fallbacks.
Fact: SP-47 (Proven in a live run, four activations 2.98 to 3.03 ms). Ruling: none found specific to retiming; the player-limits plan design point 1 (cited in play_chart.py:455 and engine.py:243) is the source; the plan's questions were approved but I did not open the limits plan's question list.

### F4-7. Duplicate elements collapsed to one press

Home: kit_tools.py:312-316 `element_list`: `seen.setdefault(e, value)` keeps the first value for an element listed twice in a group and drops the rest; no message. Another chord gem on the same element is therefore pressed once, at the first gem's strength. Ruling or fact: none found. Searched kit_tools tests by name only (not opened). This is reachable only if two gems of one chord map to one element, which the kit table makes unlikely (colour x cymbal), but a ghost and an accent on one element would collapse. Candidate, low weight.

### F4-8. Gems with no lane in the kit

Home: kit_tools.py:144-155: the table has no Orange / five-lane / double-kick entry (S1 C15). game_tools.py:207-212 `_LANE_WORDS` the same eight lanes. A chart gem outside these raises KeyError at load (loud). Not a silent drop. Noted for completeness.

### F4-9. Start of song / chords before the cursor are not pressed

play_chart.py:1063, 1098; live.START_LEAD_MS 150 (S1 C5). Ruling: none found; plan "player-limits" not searched for START_LEAD. Open.

## Family 8: Clone Hero assumptions in the playing code and their blocks

| # | Assumption (file:line) | Block and label | Verdict |
|---|---|---|---|
| 1 | Window is the formula over neighbour gaps, per side (constants.py:233-262; play_chart.py:304-344; replay_schedule.cpp:19-29) | TM-17..TM-22, TM-51: Proven in the game's code; TM-23: Proven in a live run | ok |
| 2 | Early side equals late side (play_chart.py:321; replay_schedule.cpp:137) | TM-26 Proven in a live run says they are not equal; TM-27 sends it to Q-32 | candidate, contradicted |
| 3 | A press is judged "one to two frames after" the clock the player read (replay_schedule.cpp:37-40 kJudgeEarlyMs, kJudgeLateMs) | SP-47 Proven in a live run says one frame for the stored end on four activations. TM-27 says the lag is "not proven" and bounds it at >0.80 ms and >=2.71 ms. No block gives a second frame or a maximum | candidate, "2 frames" has no block |
| 4 | Frame = 3.0 ms (play_chart.py:101; replay_schedule.cpp:35) | TM-49 Proven in a live run (about 330 a second, and builds cut it); SP-47 2.98-3.03 | ok for a quiet machine; page says the rate falls under load (TM-49), so a fixed 3.0 is a model. Ruled by the user (freak-fixes Q1(a)) |
| 5 | A chord's gems pressed in one frame score in the game's order (play_chart `_order_groups`) | SC-38 Proven in a live run; the block itself says the order rule was not read; SC-10 Hydra assumes it | candidate: the rule rests on one run |
| 6 | A plain gem's velocity is never read, so it presses like an accent (kit_tools.py:258-261) | SC-13 Proven in the game's code (velocity helper, gate on id 6). I found no block that says plain gems' bytes go unread | candidate: comment states a fact I could not locate |
| 7 | Ghost = threshold - 1 on the axis (kit_tools.py:283); `OFF_DYN_THRESHOLD` read live (constants.py:146-148) | SC-13 "A ghost is marked when the byte is at least 0 and below the threshold" (Proven in the game's code); SC-18 ghost kick pays double, Proven in a live run | ok |
| 8 | Kit must hold controller id 6; hold 2 frames; one write per press (kit_tools.py:64-107) | IN-81 Proven in the game's code; IN-77 Proven in a live run (kit reads one frame later); hold 2 frames is "the user's answer" | ok; hold 2 frames is a ruling, not a game fact |
| 9 | Definition 1 "MIDI Drums" ids and the bindings 0..7, 18, 19 to action ids 1,2,3,0,37,38,39,4,16,6 (kit_tools.py:144-172) | IN-82 Proven in the game's code (element ids); the action ids are cited to "row P7/P9 of the kick-axis-bind plan" and "the game's action table"; I did not find the action ids on a page (`grep YellowCymbal docs/clone-hero` found nothing). Note the kick (element 7) is bound to the action named Orange (4) | candidate: action ids and names have no block I could find |
| 10 | Pad axis = 0.5 + v / 254 (kit_tools.py:271) | IN-86 Proven in the game's code | ok |
| 11 | Activation needs SP off and half the meter (sp_compare.py:114-122) | SP-16 Proven in the game's code | ok |
| 12 | SP multiplier doubles (SP_MULTIPLIER 2) | SP-19 Proven in the game's code | ok |
| 13 | Hydra's full meter is 32 x resolution (sp_compare.py:178, :414) | SP-11 Proven in a live run says the game's is 30 x; split recorded on the page | known split |
| 14 | SP end: stored time, first frame whose clock reaches it (play_chart ends, engine.sp_end) | SP-27, SP-28 Proven in the game's code; SP-29, SP-30 Unknown (hit scored before the end check? hit judged between frames?) | the 15 ms lead and lag sit on SP-29 and SP-30, which are Unknown: candidate |
| 15 | Activation note = the highest pad of the fill chord, flagged and pressed first (play_chart `_flagged_gem`; replay_schedule.cpp:795-802 comment "row P27") | FL-09 Proven in the game's code ("the drum-lanes notes read as the highest-pad note"); the refinements plan's P22 says "unchecked", FINDINGS line 50 only | page label and plan row disagree; page label is stronger |
| 16 | A missed fill-end note is granted without activating (declined chords) | FL-31 / SP-20 Proven in the game's code, with the "no activation" conclusion "read from the slot spacing" and one run (ls-o3-run1) | candidate: one run |
| 17 | SP phrase counts, the 4-quarter-bar rises etc. in sp_compare (PHRASES_PER_FULL_METER 4, :140) | SP-12 Proven in the game's code | ok |
| 18 | Window clamps 37.5 / 85 and the two precision pairs (constants.py:163-166) | TM-18 Proven in the game's code; precision pair TM-31 Proven in the game's code, TM-32 Unknown whether menu sets precision | ok for normal mode (the run only finds normal-mode engines, game_tools.py:196-198) |
| 19 | "Score rises only on a hit" (engine.py:46-49; constants.py:100; engine.py:234-237) | No block says it; SP-20, FL-31, TM-45 (all Proven in the game's code) describe grants that raise the score without a press; plan P7 observed one | candidate, contradicted in part |
| 20 | scorestats keys total_notes, sp_phrases_total, accent_notes_hit, ghost_notes_hit (compare_scorestats.py) | SC-35 Proven in the game's code (the file); SC-37 Unknown (which counter feeds each key); page line 427 total_notes "Unknown" | candidate: Unknown keys decide a PASS/FAIL |
| 21 | Engine offsets (constants.py:87-148) | listed in the facts table by the docs test (README "How the pages are checked"); the file's own comment says the SP offsets were "not yet confirmed live" (engine.py:65-69) while page labels for SP-09/SP-13/SP-19 are Proven in the game's code | ok per the test; the engine.py comment is stale or the offsets unconfirmed live, I cannot tell |
| 22 | Kick-only: first kick, ignoring a second | none (kick_only is player policy) | not a game fact |

## All splits found (one line each)

1. Per-note window: Python formula vs C++ formula vs Hydra fixed 85 (TM-22, TM-29; screens covered by the user's "although that is a good idea for a new feature").
2. Early side equals late side in Python and C++, against TM-26 (Proven in a live run), open Q-32.
3. Margin: Python ACT_LEAD_MARGIN_MS 6 vs C++ kRoomMarginMs 3 (+ kJudgeLateMs 6).
4. Activation lead formula: Python need/ratio + 6, anchored on the activation; C++ equal-room, anchored on the overfill chord.
5. Python raises when a late gem pays less than an inside gem; C++ writes a refusal line.
6. Python's far-press check stops the load at the first bad press; C++ collects one refusal per chord.
7. C++ has fallback rows (join_group, plain); Python has none.
8. Python `_flagged_gem` picks the top pad by (colour, cymbal) in score-file order; C++ uses `Chord::activation_note().colortype`.
9. engine.py "score rises only on a hit" vs SP-20 / FL-31 / TM-45 grants.
10. sp_compare Hydra full meter 32 x resolution vs the game's 30 x (SP-11).
11. TM-50's Ruling "no Hydra rule depends on it" vs replay_schedule.h:170-174 depending on it for all pads.
12. SP-47's Hydra line "no frame model in `src/`; the player's `retime_from_end`" vs `replay_schedule.cpp` kJudge* (the page is a step behind the code).
13. replay_schedule.h:1-3 says the player works no Star Power rule out itself; play_chart.py works them all out.
14. constants.py comments mark SP offsets "not yet confirmed live" (engine.py:65-69, constants.py:108) against pages that label them Proven in the game's code.

## Candidates, one line each (file:line)

- F3-1 Window formula in two languages, no tie test found: tools/ch_probe/constants.py:259-272 and tools/replay_schedule.cpp:26-29.
- F3-2 Six timing constants twice, margin differs: play_chart.py:78-113 vs replay_schedule.cpp:35-66.
- F3-3 Activation lead formula differs: play_chart.py:347-428 vs replay_schedule.cpp:224-327.
- F3-4 Order groups, inside groups, far press, kept gems in both: play_chart.py:488-527, 632-673, 700-728 vs replay_schedule.cpp:183-205, 343-422.
- F3-5a sp_compare re-states Hydra's SP constants and passes any constant sp_max: sp_compare.py:109-122, 404-421.
- F3-5b score_compare forgives ghost/accent score differences written for keyboard presses, still applies to the kit: score_compare.py:150-197.
- F3-5c compare_scorestats total_notes passes on gems or chords: compare_scorestats.py:94-99 against page 06 line 427 (Unknown).
- F3-5d engine.pressed_input_hit "rises only on a hit": engine.py:46-49.
- F4-1 Decline modes: kick_only and nothing have no quoted ruling; all_but_top_pad only a blanket yes: play_chart.py:709-728, replay_schedule.cpp:188-205.
- F4-2 Order groups rest on SC-38, whose order rule is unread, plus the user's "8 ms" and "Only where it matters": play_chart.py:488-527, replay_schedule.cpp:343-366.
- F4-3 Squeeze-out late gem alone, 15 ms numbers rest on SP-29/SP-30 (Unknown): replay_schedule.cpp:844-878.
- F4-4 Fallbacks that change what is pressed: replay_schedule.cpp:868-891, covered by "Yes, start it".
- F4-5 Far-press refusal rests on TM-50, which proves only the kick pair: replay_schedule.h:170-174, replay_schedule.cpp:391-422.
- F4-6 retime_from_end does not recheck the early edge (S1 C11): play_chart.py:449-459.
- F4-7 Same element in one group collapses to one press: kit_tools.py:312-316.
- F8-2/3 Early/late sides equal; "2 frames" late judge has no block: replay_schedule.cpp:37-40.
- F8-6 "A plain gem's velocity is never read": kit_tools.py:258-261.
- F8-9 Kit action ids 37/38/39/4/16/6 and the kick bound to action "Orange" have no page block I could find: kit_tools.py:161-172.
- F8-16 Decline works because a grant does not activate: FL-31 evidence is slot spacing plus one run.
- F8-19 "Score rises only on a hit": constants.py:100, engine.py:46.
- F8-20 compare_scorestats keys whose meaning is Unknown (SC-37).

## Open (needs a live run or a file I did not read)

- Whether a dump can hold a tom and a cymbal of one colour on one tick (would make the Python/C++ top-pad answers differ). Run: none needed, read src/core/model.h `Chord` and replay_json.cpp.
- Whether freak-fixes T2 (the Python lead, margin, retimed column) is merged. Settle by git log on play_chart.py.
- Whether 8 ms always crosses a game frame at the real frame rate (order groups). A live run on a 330 fps machine with a recorder.
- Whether the Python and C++ formula constants are tied by a test. Read tests/test_replay_schedule.cpp fully.
- Whether the early and late edges of a note with close neighbours match the formula (TM-27 not proven).

## Not reached (never assumed clean)

tools/ch_mcp: server.py, bot_run.py, game_tools.py (read lines 196-290 only), sp_tools.py, trace_hits.py, frida_tools.py, frida_attach.py, front.py, quiet.py, game_files.py, menu_tools.py, method_watch.py, foreground.py, recorder_link.py, install_chart.py, build_facts.py, machine_recorder.py, machine_report.py, and all five JSON data files; tools/hydra_ui_mcp.py. tools/ch_probe: engine_finder.py, process.py, input_driver.py (header and tables only), probe_songs.py, probe_chart.py, interfaces.py, prep_inputs.py; experiments live.py, overlay_logs.py, walk_edges.py, analysis.py, edge_probe.py, sp_watch.py; score_compare.py lines 250-326; sp_compare.py lines 190-354 and 422-663. Agents: kit.ts (grep only, lines 490-540 by name), clock.ts, gamemanager.ts, hooks_common.ts, launch.ts, launch_core.ts, menu.ts, menuhooks.ts, state.ts, trace.ts, package.json, package-lock.json, the built .js files. C++: tools/replay.cpp, tools/replay_json.cpp (grep only), tests/test_replay_schedule.cpp (one grep hit). All Python tests under tools/ch_mcp/tests and tools/ch_probe/tests. Run folders under hydra-data\ch-mcp-runs. The plans' full question lists (limits, refinements) were read only at the lines quoted above; session card 490d6781 was not opened. Clone Hero page blocks not read in full: NL, CR, the rest of IN.
