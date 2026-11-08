# The press schedule comes from Hydra: hydra_replay writes it, the player only sends it

Date: 2026-11-08. Planner: Fable, task PL-REPLAY-SCHEDULE, session 6341c618. Follows `docs/superpowers/plans/2026-11-08-player-freak-fixes.md` (T1 merged; T2 held on `claude/pl-freak-t2`) and `docs/superpowers/plans/2026-11-07-player-limits.md`.

## The plain version

Today the auto-player reads two files Hydra writes (`hydra_replay dump` and `hydra_replay score`) and then works the squeeze timing out again in Python: which chord's press moves each Star Power end, how far the activation must be pressed early, the planned end, the ratios. The user's words: "the solver re-deriving everything hydra's engine already knows ... this never should have been an isue". The Python copy is also wrong in the one case the user named: on Get Ur Freak On, activation 232's end is anchored on chord 263, the phrase that overfills the meter, not on the activation. Hydra's engine knows that (`end_anchor_tick`). The Python solver reads it only to refuse: it can lead the activation or nothing, so chord 263 is pressed plain and the 28.1 ms squeeze at chord 387 lives on luck.

The move is simple to say. Hydra's engine, through a new `hydra_replay schedule` command, writes the press schedule: for every chord, each press's aim time relative to its chart time, which chord leads each squeeze and by how much, the end the game should store after that press, and whether the game can make each squeeze at 100% speed. It is derived once, from the same `Activation` the search stamped, by the same code that gives Hydra's path. The Python player keeps only what Hydra cannot know because it happens live: sending on time, the frame delays, reading the end the game actually stored and re-timing from it, and dropping a press the game would judge past its window.

The user's word is needed before the tasks start on seven questions; the three that matter most are: where the game's per-note hit window lives for now (Hydra does not model it, and the user called modelling it "a good idea for a new feature"), how a squeeze is split between its two presses (the user's "aim for -45/+45" says leave room on both sides), and what happens to the held T2 branch.

## What was checked, and how

Every claim below was read in the code or the run files, never from a handoff. The Premise map at the end names each source.

The engine already owns every fact the solver rebuilds. `Activation::end_anchor_tick(step)` in `src/core/model.cpp` returns the latest clamp's tick at or before a step, else the activation's tick (D1); `deact_anchor_tick` and `squeeze_anchor_tick(k)` read it for the final end and for each squeeze's own end. `frontend_transfer_scales` stamps the ratios from those ticks, and `transfer_post` and each `SPSqueeze::transfer` store them. The dump already writes `end_anchor_tick`, `end_scale_early`, `end_scale_late`, `nominal_deact_tick` and the four `steady_ms_*` spans (`paths_json` in `tools/replay_json.cpp`); the score file writes each chord's `ms`, `in_sp`, `is_sp_phrase_end` and per-gem `multiplier`.

The Python side rebuilds the rest. `_act_ends` and `_stored_end_ms` in `tools/ch_probe/experiments/play_chart.py` work the stored end out "with no tempo map" from which chord sits on the nominal tick, or from the first squeeze's end when no phrase was collected before it. `_squeezes` pairs `sqinouts` offsets with ticks by position. `activation_leads` and `_solve_lead` turn a need into a lead through the ratio, and `_solve_lead` raises when `end.anchor_index != act.index`: "chord 232's end is anchored on chord 263; pressing the activation early does not move it". So the overfill case is refused, never solved. On Freak today that refusal does not fire only because the 387 need comes out negative, so 263 is pressed plain.

The held T2 branch found the same thing. Its test comment reads: "387 squeezes out of the activation on chord 232, whose end the dump anchors on chord 263, so no lead on 232 moves it: the 2026-11-08 plan's 4.50 ms for 232 needs the user, and the plain press stays." So the freak-fixes plan's "232 at 4.50" was wrong, and T2 left it plain and asserted `anchor_index == 263`.

Run 1's rows agree with the engine. Chord 263 was pressed 0.37 ms early (plain); the end read after 232's press was 57,935.81, the nominal end 57,932.89 plus one frame; chord 387 was pressed 47.19 ms late and hit (the clamped end is 60,573.35 plus 263's frame, so the press landed about 19 to 22 ms past it, well inside 387's 53.59 ms side). The squeeze the user named, 1144, was pressed 53.47 ms late and missed.

Chord 263 is a single yellow cymbal at 49,786.77 ms with its neighbours 2,387.65 and 7,809.03 ms away, so its side is the formula's widest, 85.72 ms. That is why the user's "why cant you just account for that?" has a good answer: the chord whose press moves this end has the biggest window on the chart, and the squeeze at 387 has 28.09 ms to cover. Split between them, both presses keep over 40 ms of room.

## Rulings that bind this plan

- "we currently calculate squeezes up to 500ms as an intentional deviation from the game, for two reasons: 1 - it is possible to hit otherwise impossible squeezes when the song is sped up 2 - it is useful to know how close a squeeze is to being possible, or if it is impossible" (the user, this session, quoted in the brief). `kSqueezeWindowMs` stays 500, every Hydra screen stays as it is, and the schedule says per squeeze whether the game can make it at 100% speed.
- "hydra does not currently account for the dynamic hit window, although that is a good idea for a new feature" (same). The per-note window is not folded into Hydra's model here; question 1 asks where it lives for now.
- "get ur freak on is 84.3ms so you have tons of room if you just aim for -45/+45" and "starpower overfills on the 28.1ms squeeze so the frontend becomes the overfill instead of the activation. why cant you just account for that?" (the user, this session). Question 2 proposes the split rule with the numbers; the overfill anchor is design point 3.
- "there should never be two different answers for where star power ends. ever." (2026-10-29, `docs/agents/rulings/code-and-engine.md`). The schedule reads one end per step from `sp_end_steps`.
- "why are you still letting different areas do their own thing? this is why everything needs to be derived once" (2026-10-14, same file). The owner table below names one side for every question.
- The user's player numbers keep their meaning and their home: `SP_END_LEAD_MS` 15, `SP_OUT_LAG_MS` 15, `CHORD_ORDER_GAP_MS` 8, `FRAME_MS` 3 in `play_chart.py`. They cross to C++ as inputs, never as a second copy.

## Prior art

A quick search (2026-11-08, "rhythm game bot precomputed input schedule from chart analysis engine hit window") found no bot that takes its press schedule from the game's own path analysis. The nearest patterns are a browser rhythm game's scheduler, which fills a short lookahead from one audio clock ([Mintlify, Funkin gameplay](https://www.mintlify.com/FunkinCrew/Funkin/systems/gameplay)); the osu!mania bot RhythmSharp, which reads the notes first and runs them through a scheduler ([RhythmSharp](https://github.com/rwdkor/RhythmSharp)); and the Sonolus guide, which judges a press by the window around its note's time and says the engine can judge on the touch's own time once latency is calibrated ([Sonolus play guide](https://wiki.sonolus.com/sonolus.py-guide/play/11)). All three keep the timing in one place and leave the sender to send. ADR 0030 in this repo is the same shape: the Python helper is transport, Hydra owns every rule.

## The design, point by point

### 1. One new command: `hydra_replay schedule`

`hydra_replay schedule --chart <chart> --path <dump> --index <n> --player <player.json> --out <schedule.json>`. It loads the chart as `score` does, takes the path's activations off the dump the way `windows_from_file` does, and runs the same `replay_path` so every chord's gems, multipliers and Star Power payment are the engine's. Then it writes one file the player reads and nothing else needs. The writer is a new file, `tools/replay_schedule.cpp` with `tools/replay_schedule.h`, in the `hydra_replay_json` library, so `tests/test_replay_schedule.cpp` can call it. No Hydra screen reads it: `hydra_replay` is a command-line tool. Rule 11: no screen changes, before or after (point 9 says what is touched in `src/`).

`player.json` is the one input Hydra cannot know. The Python side writes it from its own constants (point 7): the per-chord window sides (question 1), `press_lead_ms`, `sp_end_lead_ms`, `sp_out_lag_ms`, `chord_order_gap_ms`, the two judging bounds `judge_early_ms` and `judge_late_ms` (the player's frame model, 1 and 2 frames of `FRAME_MS`), the decline mode and the top-pad order. Each number is written once by its owner and read once by Hydra. The schedule echoes the player block it was built from, so a run whose constants changed shows it.

### 2. The geometry, read off the engine

For each activation, in path order: the activation chord's index, the nominal end in ms (`nominal_end()`), the final end (`deact_tick()`), and every SP-end step as the engine stored it: the chord that moved it, its kind (activation, collected, clamped, squeeze-in), the end in force after it, and that step's anchor chord (`end_anchor_tick(step)`). The activation also names `read_end_after`: the chord whose press the live player reads the game's end after. That is the anchor of the final end: 263 for Freak's 232, 1062 for 1062. A second read right after the activation press stays, because the nominal end there checks the frame model (SP-47).

For each squeeze (`sqinouts`): its chord, its kind, its `offset_ms` as stored, the end it is measured against (`squeeze_end_tick(k)`), the chord whose press moves that end (`squeeze_anchor_tick(k)`), the ratios on that end (`transfer_post` for a SqOut, the SqIn's own `transfer` for a SqIn, never recomputed), the steady spans on both ticks (`steady_spans`), and `needs_no_timing` (`SPSqueeze::needs_no_timing`). A squeeze-out's chord is `sqout_tick`.

For each chord: index, tick, ms, its gems in `Chord::note_list(true)` order with multiplier and dynamic (the score file's order today), `ends_sp` (its tick is a final end), `declined` (its tick is in some activation's `skipped_fill_ticks`), and `paid_by`: the activations whose Star Power paid it, with its offset from each one's end. `paid_by` comes from the one loop in `src/core/replay.cpp` that already decides `in_sp` per window; `ReplayChord` gains the list and the loop fills it where it counts `sp_paid`. Nothing else works that out again.

### 3. The aims: which press leads a squeeze, and by how much

The schedule writes, per chord, one or more presses. Each press has its gems, `aim_ms` (relative to the chord's time: negative is early, positive is late), `after_ms` for a later order group, `timed_from_end` (the activation whose end it is measured against, when it is), and `room_ms` (how far inside its own window its judged press stays in the worst case, from the window the player gave). The rules, each stated once in `tools/replay_schedule.cpp`:

- A plain chord is pressed `press_lead_ms` early (2 today).
- A chord on a final end, or paid inside near an end a lead moved earlier, is pressed `sp_end_lead_ms` before that end (the limits plan's design point 3, unchanged in meaning).
- A squeeze that needs no timing (Freak's 717, 84.27 ms inside its end) gets the plain press and the line says so.
- A squeeze that needs timing is split between its two presses: the anchor chord's press (the chord `squeeze_anchor_tick` names, which is the overfill chord when the cap clamped, else the activation) and the squeeze chord's press. Question 2 sets the split rule; the recommended rule is equal room: pick the anchor's lead so that the anchor press's room inside its own window equals the squeeze press's room inside its own, with the squeeze press aimed `sp_out_lag_ms` past the end the anchor press produces in the worst case (anchor judged `judge_late_ms` late, so the end moves by `ratio x (lead - judge_late_ms)`), and its judged position `judge_late_ms` past its aim. In numbers, for a squeeze-out with anchor side `side_a`, squeeze side `side_s`, `gap` ms between the squeeze chord and the end, ratio `r`, lag `lag` and `jl` = `judge_late_ms`: the anchor lead is `L = (side_a - side_s + gap + lag) / (1 + r) + jl`, the squeeze aim is `gap + lag - r x (L - jl)` late, the anchor room is `side_a - L` and the squeeze room is `side_s - aim - jl`. Both rooms equal by construction. A squeeze-in after the end is the mirror: the anchor goes late, the squeeze press `sp_end_lead_ms` before the moved end.
- The split uses the ratio on the side it moves (`end_scale_early` for an early anchor, `end_scale_late` for a late one) and refuses a lead that crosses a steady span, as today.
- A chord with several gems is pressed in groups `chord_order_gap_ms` apart: an activation's flagged gem first (row P27 of the Premise map says who names it), then by multiplier, lowest first; a squeeze-out's late gem last, after every inside group. The refusals today's `_squeeze_out_inside_groups` makes stay, in C++.

Every refusal that today raises `ValueError` in Python is a `refusals` list in the file, each naming the chords and the rule in plain words: an activation that would need both directions, a lead that leaves its window, a steady span crossed, a press that reaches another note on its pad (today's `_refuse_far_presses`, moved whole, TM-50's rule included), a squeeze-out whose late gem pays less than an inside gem. A squeeze also carries `feasible`: true when every press of it has `room_ms >= 0` at 100% speed. Hydra's own 500 ms horizon is untouched: the schedule may list a squeeze Hydra's screen shows as Insane+ and say `feasible: false`; that is the user's second reason for the horizon, now in the file.

The planned end per activation is written too: the end the game should store once the `read_end_after` chord is judged `judge_early_ms` after its aim, so the live reply's "later than planned" line reads 0 to one frame when the game does what the model says.

### 4. The numbers on Freak, Rescued and Ghost

Worked from the committed fixtures (`tools/ch_probe/tests/*_limits_*.json`) with the player's numbers today (`FRAME_MS` 3, so `judge_late_ms` 6; lag 15) and the equal-room rule of design point 3; row P24 says which files and formula give each number and stays unchecked until TC2's tests pin what the C++ gives.

| Chart | Squeeze | Anchor chord (side) | Squeeze chord (side) | Gap to end | Anchor aim | Squeeze aim | Room each | Today |
|---|---|---|---|---|---|---|---|---|
| Freak | 232 -> 387 (SqOut, clamped) | 263 (85.72) | 387 (53.59) | 28.09 | 43.61 early | 5.48 late | 42.11 | 263 plain, 387 at +43.1 aim, hit by 19 to 22 ms past the end |
| Freak | 633 -> 717 (SqIn) | 633 | 717 (53.59) | -84.27 (inside) | none | plain | needs no timing | same |
| Freak | 1062 -> 1144 (SqOut) | 1062 (73.76) | 1144 (53.59) | 84.27 | 65.72 early | 39.55 late | 8.04 | 51.68 early / room 0.5 (missed in run 1); T2 60.68 / 3.0 |
| Rescued | 530 -> 561 | 530 (75.93) | 561 (61.47) | 99.04 | 69.36 early | 48.89 late | 6.58 | 57.13 / -6; T2 65.96 / 3.0 |
| Rescued | 820 -> 931 | 820 (76.49) | 931 (61.94) | 100.00 | 70.63 early | 50.08 late | 5.86 | 58.82 / -6; T2 67.79 / 3.0 |
| Ghost | 661 -> 756 | 661 (78.75) | 756 (66.34) | 109.25 | 73.88 early | 55.47 late | 4.87 | 63.15 / -6; T2 72.07 / 3.0 |

Rescued's ratios are 1.0284 (530, early) and 1.0045 (820, early); Ghost's 1.0133; Freak's are 1.0 at every activation. The anchor's room in this table is measured from its aim, one frame more cautious than its judged position; TC2's rule says which it uses and its tests pin the result.

### 5. What the Python player keeps, and what it does with the schedule

`play_chart.py` keeps `play_chords`, the clock, `live.wait_until`, the press log, the frame constants and the drop check (T1, merged). It changes in three places. First, `load_schedule(path)` replaces `load_dump_chords`: it reads the chords and presses as written, builds `Press` rows from `aim_ms` and `after_ms`, and refuses to start when `refusals` is not empty or any squeeze is not `feasible`, quoting the file's words. Second, after a press of a chord an activation names as `read_end_after` (and after the activation's own press), it reads the game's end once, writes `end_read_ms`, and `retime_from_end` shifts every later press with `timed_from_end` equal to that activation by the read end minus the planned end: one subtraction per press, as today. Third, the drop check reads the chord's side from the same window table the player wrote into `player.json`, so one number serves both.

The window table itself stays where question 1 puts it. With the recommended answer, `window_sides` keeps its home in `play_chart.py` and is written into `player.json` by `write_player_json`, a new small function; the constants it reads (`WINDOW_FORMULA_NORMAL_*`, `EXPECT_NORMAL_*`) keep their one home in `tools/ch_probe/constants.py`.

`prep_inputs.py` chains the third command: dump, score, then `write_player_json` and `hydra_replay schedule`, and dry-loads the schedule the way `ch_run_chart` will. `ch_run_chart` and `ch_play_chart` take `schedule_json` in place of `dump_json`; `score_json` stays for `ch_compare`. The reply's squeeze lines come off the schedule: "chord 263 pressed 43.6 ms early for the squeeze-out at chord 387 (end planned 60,532.7 ms, ratio 1.0000, room 42.1 ms both presses)", and `activation_end_lines` keeps its planned-against-read form, now keyed by `read_end_after`.

### 6. What Python goes away

Deleted from `play_chart.py`: `Squeeze`, `ActEnd`, `ActLead`, `_END_SCALE`, `_STEADY_ACT`, `_STEADY_END`, `activation_leads`, `_solve_lead`, `press_plan`'s lead logic, `_order_groups`, `_squeeze_out_inside_groups`, `_chord_presses`, `_timed_presses`, `_refuse_far_presses`, `_dump_path`, `declined_chord_indexes`, `load_dump_chords`, `_sp_acts`, `_act_ends`, `_stored_end_ms`, `_squeezes`, `load_dump_notes`, `SQUEEZE_IN`, `SQUEEZE_OUT`, and `ACT_LEAD_MARGIN_MS` (the margin was the frames in disguise; the judging bounds replace it). From `game_tools.py`: `decline_refusal`'s dump wording, `squeeze_lines`, `order_lines` and `planned_ends` as they stand, rewritten to read the schedule. The fixture pairs `*_dump.json`/`*_score.json` under `tools/ch_probe/tests/` stay for `score_compare` and `sp_compare`; each gains a `*_schedule.json` the main session writes with the built `hydra_replay` (TP1 cannot run it).

Three more files read what point 6 deletes, and each changes with it. `tools/ch_probe/experiments/sp_compare.py` (`press_reach`) calls `load_dump_chords` and `press_plan` to learn when each chord's presses fire, and takes `--dump`; it takes `--schedule` instead and reads each chord's press times (`chart_ms + aim_ms`) from the schedule. `tools/ch_probe/experiments/edge_probe.py` (`_first_presses`, `_fire_times`, `probe_chords`) calls `press_plan` over `Chord` rows and refuses a probe chord that a squeeze times from an end; it reads the schedule's presses the same way and refuses a press whose `timed_from_end` is set. `tools/ch_mcp/server.py` declares `dump_json` on `ch_play_chart`, `ch_run_chart` and `ch_sp_compare`; those three become `schedule_json` (question 6). `press_plan` itself stays as the name of the function that returns the schedule's presses in firing order; only its lead logic goes.

Kept: `_kept_gems` and `_flagged_gem` move to C++ only if row P27 finds the engine names the activating note; otherwise the top-pad order goes in as an input and the C++ applies it, and the two Python functions go too. `half_gap_ms`, `window_side_ms`, `note_window_ms`, `window_sides` stay under question 1 (a) or (c), go under (b).

### 7. The owner table

| Question | Owner | Where |
|---|---|---|
| Where each Star Power end is, at every step, and which chord moves it | C++ engine | `Activation::sp_end_steps`, `end_anchor_tick`, `deact_anchor_tick`, `squeeze_anchor_tick` (`src/core/model.cpp`) |
| The ratio an early or late anchor press moves the end by | C++ engine | `transfer_post`, `SPSqueeze::transfer`, stamped by `frontend_transfer_scales` |
| Which activation pays each chord and its offset from that end | C++ engine | `replay_path`'s payment loop (`src/core/replay.cpp`), new `ReplayChord::paid_by` |
| Which chord leads each squeeze, each press's aim, room, planned end, feasibility, every refusal | C++ `hydra_replay schedule` | `tools/replay_schedule.cpp` |
| The press groups of a chord and their order | C++ `hydra_replay schedule` | same file |
| The per-note hit window at 100% speed | question 1 | Python `window_sides` (recommended) |
| The player's numbers: press lead, end lead, out lag, order gap, frame period, judging bounds | Python | the constants in `play_chart.py`, written once into `player.json` |
| The frame model (judged between one and two frames after the aim) | Python | `judge_early_ms`, `judge_late_ms` in `player.json`, from `FRAME_MS` |
| Sending on time, the clock, the run cap | Python | `play_chords`, `live.wait_until` |
| Reading the end the game stored and shifting the presses timed from it | Python | `play_chords`, `retime_from_end` (one subtraction per press) |
| Dropping a press the game would judge past its window | Python | `play_chords` (T1) |
| The press log, the status lines, the compare | Python | `overlay_logs.py`, `game_tools.py`, `score_compare.py` |
| Hydra's 500 ms horizon and 85 ms window on every screen | C++ engine, unchanged | `kSqueezeWindowMs`, `kHitWindowMs` in `src/core/model.h` |

### 8. The held T2 branch

`claude/pl-freak-t2` holds three things. The lead solver with frame counting (`judged_ms`, `earliest_judged_ms`, `_lead_parts`, `_end_and_ratio`) is replaced whole by point 3, so it does not merge. The `retimed` press-log column (`late` or `inside`) and the room check in `retime_from_end`, with `SqueezeOutDropTest`, are live-side and survive in meaning: TP2 writes them against the schedule (the room check reads the press's side and `judge_late_ms`; question 3 of the freak-fixes plan, "(a) press the late gem with the last inside group", keeps its answer as a question the user has not yet given, so TP2 builds only the `late` form and the refusal, and the `inside` form waits on that question). The test fix in `test_score_compare.py` and `test_overlay_logs.py` is read by TP2 and re-made where the columns still apply. Recommendation (question 4): do not merge T2; close the branch after TP2 merges. Merging it first would land 240 lines of solver that TP1 deletes a day later, and each merge costs a review.

### 9. What is touched in `src/`, and what a player sees

One struct gains one field: `ReplayChord::paid_by` in `src/core/replay.h`, filled in `replay_path`. `src/app/preview_view.cpp` reads `ReplayChord` for the Preview's running score; Its read (row P20) names six fields and prints nothing from a new one; TC1 quotes those lines in its report. No other `src/` file changes. No Hydra screen changes: not the library, not the Paths tab, not the report, not the Preview. Hydra's screens keep the 500 ms horizon and the 85 ms window.

## Questions

1. **Where the game's per-note hit window lives for now.** The real rule is TM-21 on `docs/clone-hero/03-timing.md`: "With x the summed gap in ms, the whole window in ms is (x x 0.0110924369747899 - x^2 x 2.628539e-05) x 170 - 20", each half-gap held between 37.5 and 85 ms (TM-19), the note getting half (TM-20). Examples from both ends: Freak chord 1144, with a note 84.27 ms away on each side, gets 53.59 ms a side; chord 232 (84.27 and 168.54) gets 73.48; chord 263 and any note with both neighbours 170 ms or more away get 85.72, the cap. Hydra uses a fixed 85 everywhere (TM-29, D270 item 8), and the user said the dynamic window is "a good idea for a new feature", not this one. Sides: (a) it stays in Python (`half_gap_ms`, `window_side_ms`, `note_window_ms`, `window_sides` in `play_chart.py`, constants in `constants.py`) and the player hands the per-chord sides to `hydra_replay schedule` in `player.json`; Hydra models nothing new and the feature stays open. (b) it moves into `tools/replay_schedule.cpp` as a hydra_replay-only rule with a test pinning its constants against `constants.py`; Python's four functions go; no screen reads it, but Hydra's tool then holds a game rule Hydra's model does not, and the formula's constants have two homes. (c) as (a), and the window is also echoed into the schedule per chord so the drop check and the schedule read one number. Recommended: (c). It keeps the user's "new feature" whole for later (the window would then move inside Hydra and `player.json` would lose the table), keeps one home for the constants, and the schedule still says per squeeze what the game can make at 100% speed.

2. **How a squeeze is split between its two presses.** The code's rule after TC2 is one formula in `tools/replay_schedule.cpp`. Sides: (a) equal room: the anchor press and the squeeze press keep the same distance inside their own windows in the worst frame case. Freak 1144: 1062 at 65.72 ms early, 1144 at 39.55 late, 8.04 ms of room each; Freak 387: 263 at 43.61 early, 387 at 5.48 late, 42.11 each; Rescued 6.58 and 5.86 each; Ghost 4.87 each. (b) the user's literal "-45/+45" on Freak 1144: 1062 at 45 early (room 28.76) and 1144 at 45 late, judged 48 to 51 late against a side of 53.59 (room 2.59), with the end 2.7 to 5.7 ms before the judged press; on a 100 ms squeeze such as Rescued 931 the same aims leave the note 7.9 to 12.8 ms inside the end, so the squeeze fails: -45/+45 is not a rule for every chart. (c) the minimal lead, T2's rule: move the anchor only as far as the squeeze needs plus 3 ms, so the squeeze press always has 3.0 ms of room and the anchor the rest (13.08 on Freak 1062, 81.21 on 263). Recommended: (a), because it is what "tons of room if you just aim for -45/+45" means on Freak's 84 ms squeeze (both presses well inside), and it is the one rule that gives the most room on the chart where room is scarcest (Ghost, 4.87 each, against (c)'s 3.0 on one side). The user's own numbers on 1144 would be (b); the plan takes them as the goal, not the formula.

3. **Who owns the frame model.** Today `FRAME_MS`, `judged_ms` and `earliest_judged_ms` are the player's (T1 merged, T2 held). The brief keeps the frame delays in Python. Sides: (a) Python writes `judge_early_ms` (one frame) and `judge_late_ms` (two frames) into `player.json` and C++ shrinks every room by them; the model stays Python's, the arithmetic C++'s. (b) C++ takes `frame_ms` alone and holds the one-and-two-frames rule itself. Recommended: (a); the two bounds are live facts (SP-47, TM-27) the player owns, and C++ needs only numbers.

4. **The T2 branch.** Sides: (a) do not merge; TP2 re-makes the `retimed` column and the room check against the schedule; the branch is deleted after TP2 merges. (b) merge T2 first, then let TP1 delete its solver. Recommended: (a); see design point 8.

5. **The `inside` fallback when the read end leaves the late gem no room.** The freak-fixes plan's question 3 is still open. With the schedule, the case is rarer (Freak 1144 has 8 ms, not 0.5) but not gone. Sides as there: (a) the late gem joins the last inside group and the row says `retimed=inside`; (b) send it anyway; (c) drop it (T1 does this already when it is past the window). Recommended: (a). Until the user answers, TP2 builds the `late` form and the refusal only, and a read end that leaves no room falls to T1's drop.

6. **`ch_run_chart`'s arguments.** `dump_json` is replaced by `schedule_json`; `score_json` stays for `ch_compare` and the trace. Sides: (a) replace, so one file carries the plan and a stale dump cannot be passed by habit; (b) keep both for a while. Recommended: (a).

7. **A squeeze the game cannot make at 100% speed.** The schedule marks it `feasible: false` and the plan has the player refuse to start (TP1), as today's solver refuses a squeeze no lead works for. Hydra still lists it, because the user gave a second reason for the 500 ms horizon ("it is possible to hit otherwise impossible squeezes when the song is sped up"). Examples from both ends: Freak 387 has 28.09 ms to cover and both presses keep 42.11 ms (feasible); a squeeze whose gap exceeds both windows together (a made-up 200 ms gap against two 53.59 ms sides) has negative room on both presses (not feasible). Sides: (a) refuse the whole chart, naming the squeeze; (b) play the chart, press that squeeze's chords plain, and write a line naming it and its negative room; the run's score then falls short by that squeeze and `ch_compare` says so. Recommended: (a), because a run that cannot match Hydra's score is a failed run and the refusal says why before the game starts.

## Tasks

Each task is one Opus executor under 100 tool calls and 20 minutes, cut from current `main`, never from this plan's branch. Agents never build and never run `hydra_tests`, `hydra_replay` or `hydra_uitest`. Order: TC1 and TD run first, in parallel; TC2 after TC1 merges; then the main session builds `hydra_replay`, writes the fixture schedules (point 6) and runs `prep_inputs.py` on the `ls-l0` charts; then TP1, cut from the `main` that holds TC2 and the fixtures; then TP3 (cut from TP1's branch, merged with it); then TP2 and TP4 in parallel, cut from the `main` that holds TP1 and TP3. Python tasks run only `py -I -m unittest tools.ch_probe.tests.test_play_chart`, `tools.ch_mcp.tests.test_game_tools`, `tools.ch_probe.tests.test_prep_inputs`, `tools.ch_probe.tests.test_overlay_logs`, `tools.ch_probe.tests.test_sp_compare`, `tools.ch_probe.tests.test_edge_probe` from their worktree (each task runs the ones its files touch), plus `hydra_precheck.exe` from the main checkout. C++ tasks run `hydra_precheck.exe` only; the main session builds and runs `hydra_tests.exe "-tc=replay*"` and `"-tc=schedule*"`.

### TC1: the schedule's geometry (C++; Opus)

Owns: new `tools/replay_schedule.h` and `tools/replay_schedule.cpp` (added to `hydra_replay_json` in `CMakeLists.txt`); `ReplayChord::paid_by` in `src/core/replay.h` and its fill in `src/core/replay.cpp`'s payment loop; the `schedule` command and `--player` flag in `tools/replay.cpp` (`cmd_schedule`, `usage`, the flag parse); `first_row_difference` in `tests/replay_util.h` (it gains `paid_by`, row P34, so the scores-only and full-replay equality tests cover the new field); new `tests/test_replay_schedule.cpp` (added to `hydra_tests`). Reads but does not change `src/core/model.h`, `src/core/squeeze_rating.h`, `tools/replay_json.cpp`.

Design points 1, 2 and 9. The command may write the geometry alone before TC2 adds the aims; its `presses` list is then empty and `refusals` holds one line saying so. Acceptance, each a `TEST_CASE` in `tests/test_replay_schedule.cpp`:
- "schedule: every end step names its anchor": on a hand-built `Activation` with steps Activation, Collected, Clamped, SqIn, the written steps carry anchors equal to `end_anchor_tick(step)` for each step and `read_end_after` equals the Clamped step's chord; on an activation with no clamp `read_end_after` is the activation. Fails if the anchor is taken from the activation alone.
- "schedule: a squeeze carries its own end, anchor and stored ratio": a SqIn's ratios equal its `SPSqueeze::transfer`, a SqOut's equal `transfer_post`; a `transfer_post` left unset writes null and a refusal line, never a recomputed number.
- "schedule: paid_by matches in_sp on every corpus path": for every chart in `testdata/input` and every path of its analysis, a chord's `paid_by` is non-empty exactly when `in_sp` is true, and each entry's offset equals `offset_from_sp_end(chord.ms, deact_ms)` of that window.
- "schedule: declined chords are the skipped fill ticks": the chords marked `declined` are exactly the chords on some activation's `skipped_fill_ticks`.
- "schedule: a missing --player refuses by name" and "schedule: a chart with no path refuses by name": `cmd_schedule` prints one `error:` line each.
- Row P20: the report quotes `src/app/preview_view.cpp`'s read of `ReplayChord` (lines 240 to 255) and says what it prints; the field adds nothing to it.
- Row P27: the report says whether the engine names an activation chord's activating note (grep `src/core` and `src/search` for the frontend note) and TC2 reads that answer.

### TC2: the aims, rooms and refusals (C++; Opus; after TC1)

Owns: the aim rules in `tools/replay_schedule.cpp` (`split_squeeze`, `press_groups`, `refusals`), the `player.json` reader (`read_player_input`), the `presses`, `feasible`, `planned_end_ms` and echoed `player` fields; new test cases at the end of `tests/test_replay_schedule.cpp`. Reads `tools/ch_probe/experiments/play_chart.py` on `main` for the rules it moves (`_order_groups`, `_squeeze_out_inside_groups`, `_refuse_far_presses`, `_timed_presses`) and quotes each as it moves it.

Design points 3 and 4, questions 1 to 3 as answered. Acceptance:
- "schedule: the equal-room split on Freak's clamped squeeze": a hand-built geometry with Freak's numbers (end 60,573.35, chord 387 at 60,545.26 with side 53.59, anchor chord 263 at 49,786.77 with side 85.72, ratio 1.0, lag 15, judging bounds 3 and 6) gives chord 263 an aim near -43.6 and chord 387 near +5.5 with equal `room_ms`, within 0.01 of what the formula gives; the test names the formula's two inputs it would change if the numbers differ. Fails if the lead is put on chord 232.
- "schedule: Freak 1144, Rescued 561 and 931, Ghost 756": the same, with the table's inputs; each pair's rooms equal within 0.01 and are positive.
- "schedule: a squeeze that needs no timing is pressed plain": Freak 717's geometry gives aim -2 and `needs_no_timing` true.
- "schedule: a squeeze the game cannot make is not feasible": a made-up squeeze whose gap exceeds both windows together gives `feasible` false with both rooms negative, and no refusal (Hydra still lists it, the user's reason 2).
- "schedule: an activation that needs both directions is refused naming both chords"; "a lead across a steady span is refused naming the span"; "a press that reaches a neighbour on its pad is refused naming both chords" (the Edge Walk Pair shape: two kicks 100 ms apart); "a late gem that pays less than an inside gem is refused": each quotes the plain-words line.
- "schedule: press groups": a three-gem activation chord with multipliers 1, 2, 2 and a flagged green cymbal gives the cymbal first, then the x1 gem, then the x2 gem, `after_ms` 8 and 16; a two-gem squeeze-out gives the inside gem at `sp_end_lead_ms` before the end and the late gem last.
- "schedule: the planned end counts one frame": planned end equals the stored end moved by `ratio x (lead - judge_early_ms)` for an early anchor.
- The echoed `player` block equals the input.

### TP1: `play_chart.py` reads the schedule (Python; Opus; after the fixture schedules exist)

Owns: in `tools/ch_probe/experiments/play_chart.py`, new `load_schedule`, `write_player_json`, the deletions of point 6, `Press` and `Chord` trimmed to what the schedule gives, `press_plan` reduced to returning the schedule's presses in firing order; every test class in `tools/ch_probe/tests/test_play_chart.py` that pinned a solved lead, a planned end, a window check or a decline list; the new fixtures `tools/ch_probe/tests/*_schedule.json` (read, never written: the main session wrote them). Does not touch `play_chords` (TP2), `game_tools.py`, `server.py`, `prep_inputs.py` (TP3), `sp_compare.py` or `edge_probe.py` (TP4). `game_tools.py` still calls the deleted names until TP3 lands, so TP3 is cut from TP1's branch and the two merge to `main` together, one review each; `main` is never left with a caller of a deleted name.

Design points 5 and 6. Acceptance:
- `test_load_schedule_gives_one_press_per_group`: the Freak schedule fixture loads to presses whose `aim_ms`, `after_ms` and gems equal the file's; no number is computed in Python (the test greps `play_chart.py` for the deleted names and fails if any remains).
- `test_a_refusal_in_the_file_stops_the_load_quoting_it` and `test_an_infeasible_squeeze_stops_the_load_naming_it` (question 7 sets whether the second is a refusal).
- `test_write_player_json_holds_every_constant_once`: the written block's values equal `PRESS_LEAD_MS`, `SP_END_LEAD_MS`, `SP_OUT_LAG_MS`, `CHORD_ORDER_GAP_MS`, `FRAME_MS` and `2 * FRAME_MS`, and the window table equals `window_sides` over the score's chords.
- `ACT_LEAD_MARGIN_MS` and its pin are gone; the test that pinned it is replaced by one pinning `FRAME_MS == 3.0` alone.

### TP3: the MCP tools and `prep_inputs.py` take the schedule (Python; Opus; after TP1)

Owns: `tools/ch_mcp/game_tools.py`'s `chords_to_play`, `decline_refusal`, `_activation_leads`, `squeeze_lines`, `order_lines`, `planned_ends`, `activation_lead_lines` (now `schedule_lines`), and the `schedule_json` argument of `ch_play_chart`, `ch_run_chart` and `ch_sp_compare` (question 6); `tools/ch_mcp/server.py`'s three declarations (P33); `tools/ch_probe/prep_inputs.py`'s third command and dry load; `tools/ch_mcp/tests/test_game_tools.py` and `tools/ch_probe/tests/test_prep_inputs.py` (every class that pinned a solved lead, a planned end, a window check or a decline list). Does not touch `activation_end_lines` or `play_chords` (TP2).

Design point 5. Acceptance:
- `test_prep_inputs_runs_three_commands_and_dry_loads_the_schedule` in `test_prep_inputs.py`: the fake `run` sees dump, score and schedule in order, with `--player` naming the written file.
- `test_game_tools`: `chords_to_play(score_json, schedule_json)` prints the schedule's squeeze lines ("chord 263 pressed 43.6 ms early for the squeeze-out at chord 387 ...") and refuses a schedule whose `player` block differs from the constants now in force, naming the constant.
- A signature test: `ch_play_chart`, `ch_run_chart` and `ch_sp_compare` in `game_tools.py` and `server.py` take `schedule_json` and no `dump_json`.

### TP4: the two probe tools read the schedule (Python; Opus; after TP1 and TP3; parallel with TP2)

Owns: `tools/ch_probe/experiments/sp_compare.py` (`press_reach` and its `--dump`/`--path-index` flags become `--schedule`) and `tools/ch_probe/experiments/edge_probe.py` (`_first_presses`, `_fire_times`, `probe_chords`), with `tools/ch_probe/tests/test_sp_compare.py` and `tools/ch_probe/tests/test_edge_probe.py`. Reads `load_schedule` and `press_plan` from TP1's `play_chart.py`; changes neither.

Design point 6 (the three readers). Acceptance:
- `test_sp_compare`: the Bomber and Mouths cases of today (`test_bomber_147_reaches_its_solved_early_press` and its neighbours) read the `*_schedule.json` fixtures and give the same reaches as before, within 0.01 ms.
- `test_edge_probe`: a probe chord whose press the schedule times from an end is refused ("timed by another rule"); a plain chord's probe still moves its press to the asked time.

### TP2: the live loop reads the end after the chord the schedule names (Python; Opus; after TP1 and TP3; parallel with TP4)

Owns: `play_chords` and `retime_from_end` in `play_chart.py`; `RETIMED_LATE`, the `retimed` column in `tools/ch_probe/experiments/overlay_logs.py` (`PRESS_LOG_COLUMNS`); `activation_end_lines` in `game_tools.py`; new test classes `ReadAfterAnchorTest` and `RetimedColumnTest` at the end of `test_play_chart.py` and the matching status test in `test_game_tools.py`; `tools/ch_probe/tests/test_overlay_logs.py`. Reads `claude/pl-freak-t2`'s `retime_from_end`, `SqueezeOutDropTest` and the `retimed` tests, and re-makes what survives (point 8). Does not touch `load_schedule` (TP1); the tasks meet on `Press`'s fields, which the plan fixes: `index, chart_ms, aim_ms, lanes, dynamics, axes, declined, after_ms, timed_from_end, read_end, planned_end_ms, late_gem, retimed`.

Design point 5 (second change), point 8, question 5. Acceptance:
- `test_the_end_is_read_after_the_anchor_chord_not_only_the_activation`: a fake run of a schedule with activation 232 anchored on 263 reads the end twice (after 232's first group and after 263's), writes both `end_read_ms`, and shifts 387's press by the second read minus 263's planned end; 232's read shifts nothing. Fails if the read happens only after the activation.
- `test_a_read_end_later_than_planned_moves_the_late_gem_by_the_same_ms`: Freak 1144's press moves by exactly the delta and its row says `retimed=late`.
- `test_a_read_end_that_leaves_no_room_falls_to_the_drop_check`: a read end so late that `aim + judge_late_ms` passes the side leaves the press to T1's drop, whose row says `dropped`; no `inside` press is sent (question 5 open).
- `test_no_activation_read_leaves_the_plan`: `NO_ACTIVATION_READ` as today.
- `activation_end_lines` prints one line per read, keyed by the chord read after, with planned, game and the difference; one string test.
- `test_overlay_logs`: `retimed` is a column after `probe`, blank on every other row.

### TD: the docs (docs only; Opus or Sonnet; parallel with TC1)

Owns: `docs/development.md`'s `hydra_replay` paragraphs (lines 436 to 459 on `main`), `docs/agents/clone-hero-mcp.md` step 5 (the `dump_json`/`decline_mode` sentences and `ch_run_chart`'s arguments), a new "Press schedule" entry under Paths in `CONTEXT.md` naming `tools/replay_schedule.cpp` as the owner and `player.json` as the one input. Writes nothing to `docs/clone-hero/` (the live result write-back is the main session's, after LS-S2).

Acceptance: `hydra_precheck.exe` passes; the main session runs `hydra_tests.exe "-tc=docs match code*"`; each changed sentence is quoted in the report with the code line it describes (the names must exist after TC1 and TC2 merge, so TD's branch merges after TC2's).

## Before the run

The main session, not an agent, makes the files: it builds `hydra_replay` through `hydra_build_pass`, runs `py -I tools/ch_probe/prep_inputs.py --chart <Freak notes.mid> --out <dated folder>` (which now writes dump, score, player and schedule and dry-loads), checks the dry load prints chord 263 at about 43.6 ms early and chord 1062 at about 65.7 early with their rooms, and copies the four files beside the `ls-l0` folder in `hydra-data`. It also writes the fixture schedules for Freak, Rescued, Ghost, Bomber, Mouths and Senri into `tools/ch_probe/tests/` on a branch for TP1. A dry load that refuses stops here and goes to the user with the refusal's words.

## The live steps

The main session runs these; the limits plan's run rules hold (hold other sessions, MD5 of the game's files before and after, copy the stats and the watch out before `ch_stop`, `replies.txt` in a dated folder, `ch_reload` after the merge). Each step caps at 2 runs and 10 minutes; a step at its cap stops and goes to the user. Nobody focuses the game window and nothing uses `SendInput`; every press goes through the kit. No library run.

### LS-S1: the Freak launch (2 runs, 10 minutes)

Calls: `ch_start(<Get Ur Freak On folder>, human=True, player="ProDrums,Expert")` (seen: LS-F1a run 1)

Uses rows P9, P22. Pass: quiet, recorder pid and folder, game pid, song length 192,934 ms, loader ran after the patch. Fail: a refusal (wait for a held machine) or another song length (stop). LS-S2 starts at once.

### LS-S2: Freak against 531,685 from the schedule (inside LS-S1's run; 2 runs)

Calls: `ch_run_chart(<Freak folder>, <freak score>, kit_player="System", decline_mode="all_but_top_pad", schedule_json=<freak schedule>)` (never tried), `ch_play_status` (seen: ls-r1-run1, 2026-11-07), `ch_sp_watch_status` (seen: ls-r1-run1, 2026-11-07), `ch_wait_stats` (seen: LS-F1b run 1), `ch_compare(<stats copy>, <freak score>)` (seen: LS-F1b run 1), `ch_stop` (seen: LS-F1b run 1)

Uses rows P6, P7, P9, P10, P13, P14, P15, P23, P24 (P14, P15 and P24 are unchecked: the first and second reads settle P14 and P15, TC2's tests settle P24 before this runs). Reads, in order. Zeroth: the reply's schedule lines name chord 263 at about 43.6 ms early for 387 and chord 1062 at about 65.7 early for 1144, with rooms. First: chord 232's row lead 2 to -1 ms (plain), its end line 0 to 3.1 ms later than the nominal 57,932.89. Second: chord 263's row lead 43.6 to 40.6 early, its end line 0 to 3.1 ms later than planned (about 60,532.7); this read is what settles whether the clamped end follows the press's judged time (P15). Third: chord 387 `hit` 1, pressed about 5.5 to 11.5 ms late by its row, `retimed` `late`. Fourth: chord 1062's row lead 65.7 to 62.7 early, end line 0 to 3.1 later than planned. Fifth: chord 1144 `hit` 1, at most 45.6 ms late, `retimed` `late`. Sixth: no `dropped` row, or one dropped row with the next chord hit and the score short by that chord alone. Seventh: `ch_compare` passes at 531,685 with 5 activations, MD5 unchanged, every restore "unchanged" or "restored".

Pass: all seven, or six with the sixth's stall form. Fail, second read: an end line outside 0 to 3.1 ms after 263's press (the clamped end does not follow the press as the model says: stop, the two numbers to the user and to page 05; no number is changed by hand). Fail, third or fifth read: `hit` 0 with `retimed` `late` (the room was not enough: stop, the row's clock and the end to the user). Fail, first or fourth read: an end line outside 0 to 3.1 (row P10 wrong at this tempo: stop). Run 2 only when a fail line names it or the machine was not held.

After LS-S2 passes, the limits plan's Rescued and Ghost steps run with their schedules (first reads: 530 at about 69.4 early, 820 at 70.6, 661 at 73.9), the same seven reads and stop lines. The page 05 write-back for the clamped end's live result (SP-40's label) is the main session's, in the merge that records the run.

## Out of scope (needs the user's yes to leave out)

- Modelling the game's per-note hit window inside Hydra's search or screens (the user's "new feature").
- Playing at a speed other than 100%: the schedule's `feasible` is at 100% only.
- The `inside` fallback (question 5) until the user answers it.
- Merging the T2 branch (question 4).
- A `hydra_replay` schedule for a path from `target` rather than `dump`: the writer takes any `paths_json` output, but no task tests `target`'s.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Every end step's anchor comes from `end_anchor_tick`, never from the activation alone; `read_end_after` is the final end's anchor | design 2 | TC1 | "every end step names its anchor" |
| A squeeze's ratios are the stored ones; an unset ratio writes null and a refusal, never a recomputed number | design 2 | TC1 | "a squeeze carries its own end, anchor and stored ratio" |
| `paid_by` is filled in the one payment loop and agrees with `in_sp` on every corpus path | design 2, 9 | TC1 | "paid_by matches in_sp on every corpus path" |
| Declined chords are exactly the skipped fill ticks | design 2 | TC1 | "declined chords are the skipped fill ticks" |
| A missing `--player` or path refuses by name | design 1 | TC1 | the two refusal cases |
| No Hydra screen changes; the Preview's text is unchanged | design 9, rule 11 | TC1 | the report quotes `preview_view.cpp`'s read (P20); the docs test at merge |
| The lead goes on the chord `squeeze_anchor_tick` names (263, not 232) and the rooms are equal | design 3, 4; Q2 | TC2 | "the equal-room split on Freak's clamped squeeze" fails if 232 is led |
| The table's aims on Freak 1144, Rescued and Ghost, rooms equal and positive | design 4 | TC2 | the second split case |
| A squeeze that needs no timing is pressed plain | design 3 | TC2 | the Freak 717 case |
| A squeeze the game cannot make at 100% is listed and marked not feasible, with no refusal | design 3; ruling on 500 ms | TC2 | "a squeeze the game cannot make is not feasible" |
| Every Python refusal of today has a C++ refusal naming the chords | design 3, 6 | TC2 | the four refusal cases |
| Press groups: flagged gem first, then by multiplier, late gem last, `chord_order_gap_ms` apart | design 3 | TC2 | "press groups" |
| The planned end counts one frame of judging | design 3 | TC2 | "the planned end counts one frame"; LS-S2 first, second and fourth reads |
| The schedule echoes the player block it was built from | design 1 | TC2, TP3 | the echo case; `test_game_tools` refuses a changed constant |
| Python computes no lead, need, split, order or window check; the listed names are gone | design 6 | TP1 | `test_load_schedule_gives_one_press_per_group`'s grep |
| A refusal or an infeasible squeeze in the file stops the load, quoting it | design 5 | TP1 | the two load refusals |
| Every player constant is written once into `player.json`, with the window table | design 1, 7; Q1, Q3 | TP1 | `test_write_player_json_holds_every_constant_once` |
| `prep_inputs.py` chains dump, score, player, schedule and dry-loads | design 5 | TP3 | `test_prep_inputs_runs_three_commands_and_dry_loads_the_schedule` |
| `ch_play_chart`, `ch_run_chart` and `ch_sp_compare` take `schedule_json` in place of `dump_json`, in `game_tools.py` and `server.py` | design 5, 6; Q6 | TP3 | the signature test |
| `sp_compare` and `edge_probe` read press times from the schedule and still give today's reaches and refusals | design 6 | TP4 | the Bomber and Mouths cases; the probe refusal case |
| `first_row_difference` covers `paid_by` | design 9 | TC1 | the existing replay equality tests, now comparing the field |
| `ACT_LEAD_MARGIN_MS` is gone; `FRAME_MS` 3.0 stays the one frame number | design 6, 7 | TP1 | the pin test |
| The end is read after the chord the schedule names, and again after the activation | design 5 | TP2 | `test_the_end_is_read_after_the_anchor_chord_not_only_the_activation` |
| A read end shifts every press timed from that activation by the same ms; the row says `retimed=late` | design 5, 8 | TP2 | the two retime tests |
| No `inside` press is sent until question 5 is answered; a press left no room falls to the drop | design 8; Q5 | TP2 | `test_a_read_end_that_leaves_no_room_falls_to_the_drop_check` |
| `retimed` is a press-log column blank on other rows | design 8 | TP2 | `test_overlay_logs` |
| The docs name the new command and the schedule's owner, and no doc still says the player times squeezes from the dump | TD | TD | the docs test at merge; each sentence quoted with its line |
| Every live step has a cap, a stop line and at most one never-tried call; no focus, no `SendInput` | the live steps | LS-S1, LS-S2 | each step's Calls line and last sentence |
| Freak run 2 passes at 531,685 with 5 activations or fails on a named read | LS-S2 | LS-S2 | the seventh read |
| The clamped end's live behaviour (P14, P15) is settled by a read and written back to page 05 | LS-S2 | LS-S2, main session | the second read; the write-back in the run's merge |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: `end_anchor_tick(step)` returns the latest clamp at or before the step, else the activation; `deact_anchor_tick` and `squeeze_anchor_tick` read it | code | `src/core/model.cpp` lines 702 to 776 | checked 2026-11-08 |
| P2: the dump writes `end_anchor_tick`, `end_scale_early`, `end_scale_late`, `nominal_deact_tick`, `sqout_tick`, `sqin_ticks`, `skipped_fill_ticks` and the four `steady_ms_*` fields from stored facts | code | `tools/replay_json.cpp` lines 121 to 174 | checked 2026-11-08 |
| P3: the score file writes per chord `index`, `tick`, `ms`, `notes` with `multiplier` and `dynamic`, `is_sp_phrase_end`, `in_sp`; per activation `act_ms`, `deact_ms`, `sqout_tick` | code | `tools/replay.cpp` lines 413 to 455 | checked 2026-11-08 |
| P4: today's Python solver refuses a lead when the anchor is not the activation, rebuilds the stored end from the files, and computes windows itself | code | `tools/ch_probe/experiments/play_chart.py` lines 335 to 428 (`window_sides`, `activation_leads`, `_solve_lead` at 399 to 404), 827 to 877 (`_act_ends`, `_stored_end_ms`) | checked 2026-11-08 |
| P5: the T2 branch holds a frame-counting solver, a `retimed` column and a test that leaves 232 plain naming anchor 263 | code | `git diff main...claude/pl-freak-t2` on `tools/ch_probe/experiments/play_chart.py` and `tools/ch_probe/tests/test_play_chart.py` (the comment "whose end the dump anchors on chord 263") | checked 2026-11-08 |
| P6: Freak's path: 232's end anchored on chord 263 (tick 69040), nominal end 57,932.89 (tick 80640), final end 60,573.35; 387 SqOut offset -28.09; 633/717 SqIn -84.27 (nominal end 98,444.63); 1062/1144 SqOut -84.27, end 141,656.25; ratios 1.0 at every activation | chart | `tools/ch_probe/tests/freak_limits_dump.json` and `freak_limits_score.json`, read through `load_dump_chords` | checked 2026-11-08 |
| P7: chord 263 is one yellow cymbal at 49,786.77 ms with gaps 2,387.65 and 7,809.03 (side 85.72); 387 a red at 60,545.26, gaps 84.27 both sides (side 53.59); 232 side 73.48; 1062 side 73.76; 1144 side 53.59 | chart | `tools/ch_probe/tests/freak_limits_score.json` chords 232, 263, 387, 1062, 1144, through `note_window_ms` | checked 2026-11-08 |
| P8: Rescued 530/561 (end 126,292.59, gap 99.04, r 1.0284) and 820/931 (end 186,408.03, gap 100.00, r 1.0045); Ghost 661/756 (end 97,624.61, gap 109.25, r 1.0133); sides 75.93/61.47, 76.49/61.94, 78.75/66.34 | chart | `tools/ch_probe/tests/rescued_limits_*.json` and `ghost_limits_*.json` through `load_dump_chords` | checked 2026-11-08 |
| P9: in Freak run 1, chord 263 was pressed 0.37 ms early and hit; 232's end read 57,935.81; chord 387 pressed 47.19 ms late and hit; 1062's end read 141,608.66; chord 1144 pressed 53.47 ms late and missed | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-f1-run1\press-2.csv` rows 232, 263, 387, 1062, 1144 (`chart_ms`, `clock_ms_at_press`, `hit`, `end_read_ms`) | checked 2026-11-08 |
| P10: the end is stamped as if the press came one frame (2.98 to 3.03 ms) after the clock the player read | data | `docs/clone-hero/05-star-power.md` SP-47 (line 349), Proven in a live run | checked 2026-11-08 |
| P11: both window sides are equal and the check holds a press to them, judged some ms after the send | data | `docs/clone-hero/03-timing.md` TM-26 and TM-27 (lines 293 to 311), Proven in a live run | checked 2026-11-08 |
| P12: the per-note window is the TM-21 formula on the summed clamped half-gaps, 48.15 to 85.72 ms a side; Hydra uses a fixed 85 | data | `docs/clone-hero/03-timing.md` TM-19 to TM-22 (lines 223 to 261), Proven in the game's code | checked 2026-11-08 |
| P13: a phrase that overfills the meter is cut to a full bar, and the end is worked out again one full bar after the collecting moment; the recompute runs in the next update | data | `docs/clone-hero/05-star-power.md` SP-24 (line 247) and SP-34 (line 361), Proven in the game's code | checked 2026-11-08 |
| P14: the game's clamp anchor is the same note Hydra measures the scale from (the collecting note) | unchecked | `docs/clone-hero/05-star-power.md` SP-40 (line 423) is labelled "Hydra assumes it, never checked" | LS-S2 second read |
| P15: the clamped end the game stores after chord 263's hit moves with the press's judged time, by the stored early ratio | unchecked | `docs/clone-hero/05-star-power.md` SP-24 (line 247): its listing names the collecting moment, not the press; no live run has pressed a clamping phrase chord early | LS-S2 second read |
| P16: `kHitWindowMs` is 85 and `kSqueezeWindowMs` 500, both fixed | code | `src/core/model.h` lines 105 and 111 | checked 2026-11-08 |
| P17: the user's words on the 500 ms deviation, the dynamic window as a new feature, "aim for -45/+45" and the overfill frontend | ruling | the user's messages in session `6341c618`, quoted in this plan's brief and confirmed by the main session | checked 2026-11-08 |
| P18: `hydra_replay_json` is a static library linked by `hydra_replay`, `hydra_tests` and `hydra_pathbuilder` | code | `CMakeLists.txt` lines 689 to 693 and 936 to 944 | checked 2026-11-08 |
| P19: `hydra_replay score` takes the path's windows through `windows_from_file` and `replay_path`, and `dump` through `paths_json` | code | `tools/replay.cpp` lines 337 to 404 and 492 to 515 | checked 2026-11-08 |
| P20: `src/app/preview_view.cpp` reads `ReplayChord` only for the running score (`ms`, `cum_onscreen_total`, `multiplier_shown`, `multiplier_after`, `combo_after`, `cum`), and a new field changes no text it prints | code | `src/app/preview_view.cpp` lines 240 to 255; the only other `ReplayChord` readers are `tools/replay.cpp` line 427, `tests/replay_util.h`, `tests/test_replay.cpp` line 373 and `tests/test_preview_boxes.cpp` line 429 | checked 2026-11-08 |
| P33: `sp_compare.press_reach` calls `load_dump_chords` and `press_plan`; `edge_probe` calls `press_plan` over hand-built `Chord` rows; `server.py` declares `dump_json` on `ch_play_chart`, `ch_run_chart` and `ch_sp_compare` | code | `tools/ch_probe/experiments/sp_compare.py` lines 85 to 98; `tools/ch_probe/experiments/edge_probe.py` lines 92 to 109; `tools/ch_mcp/server.py` lines 364, 391 and 514; `tools/ch_probe/tests/test_edge_probe.py` line 122 | checked 2026-11-08 |
| P34: `first_row_difference` in `tests/replay_util.h` lists every `ReplayChord` field by name and its comment says it covers every field | code | `tests/replay_util.h` lines 37 to 74 | checked 2026-11-08 |
| P21: `ch_run_chart` takes `song_folder, score_json, kit_player, decline_mode, dump_json` and loads through `chords_to_play` | code | `tools/ch_mcp/game_tools.py` lines 410 to 439 and 1026 to 1054 | checked 2026-11-08 |
| P22: LS-F1b run 1 ran `ch_start`, `ch_run_chart`, `ch_wait_stats` and `ch_compare` on Freak with the human launch | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-f1-run1\replies.txt` lines 3 to 6 | checked 2026-11-08 |
| P23: `ch_play_status` and `ch_sp_watch_status` ran in a human-launch `ch_run_chart` run (`ch_stop`, `ch_wait_stats` and `ch_compare` ran in LS-F1b run 1, P22) | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-r1-run1\replies.txt` lines 10 and 12 (`ch_play_status`, `ch_sp_watch_status`), line 13 (`ch_stop`); `ls-f1-run1\replies.txt` line 10 (`ch_stop`) | checked 2026-11-08 |
| P24: the equal-room aims in the table (263 at 43.61 / 387 at 5.48; 1062 at 65.72 / 1144 at 39.55; Rescued 69.36/48.89 and 70.63/50.08; Ghost 73.88/55.47) | unchecked | recomputed by the reviewer from `tools/ch_probe/tests/freak_limits_dump.json`, `freak_limits_score.json`, `rescued_limits_*.json` and `ghost_limits_*.json` (deact_ms, ratio `end_scale_early`, chord ms) with the sides from `note_window_ms` in `tools/ch_probe/experiments/play_chart.py` (the TM-21 formula, `docs/clone-hero/03-timing.md`) and the formula in design point 3 at lag 15, `jl` 6; all five timed pairs agree to 0.01 ms; no committed code computes them yet | TC2 |
| P25: `prep_inputs.py` runs `dump` then `score --path <dump> --index 0` and dry-loads through `chords_to_play` | code | `tools/ch_probe/prep_inputs.py` lines 89 to 118 | checked 2026-11-08 |
| P26: `PRESS_LOG_COLUMNS` ends with `end_read_ms` and `probe` | code | `tools/ch_probe/experiments/overlay_logs.py` lines 91 to 99 | checked 2026-11-08 |
| P27: Hydra's engine names the gem of an activation chord the game flags (the activating note) | unchecked | `CONTEXT.md` "Frontend squeeze" names "the activation chord's activation note"; no owner function was found by this planner | TC1 |
| P28: the docs test checks every plan dated on or after its cut-off for a Premise map in the shape, and every `### LS` step for a `Calls:` line | code | `tests/test_docs_match_code.cpp` lines 1506 to 1779 | checked 2026-11-08 |
| P29: the helper-is-transport, Hydra-owns-rules pattern is the project's | code | `docs/adr/0030-hydra-owns-every-live-command-rule.md` lines 17 to 55 | checked 2026-11-08 |
| P30: `docs/development.md` describes `hydra_replay`'s `dump`, `score`, `target` and `selfcheck` and no `schedule` | code | `docs/development.md` lines 436 to 459 | checked 2026-11-08 |
| P31: `docs/agents/clone-hero-mcp.md` step 5 says the player times squeezes from `dump_json` | code | `docs/agents/clone-hero-mcp.md` line 49 | checked 2026-11-08 |
| P32: `squeeze_budget_ms`, `nominal_budget_ms`, `timing_tiers` and every screen read `kHitWindowMs`, and none reads `tools/` | code | `src/core/squeeze_rating.cpp` lines 126 to 246; `src/core/model.h` line 105 | checked 2026-11-08 |
