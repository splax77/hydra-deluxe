# Player fixes before Freak run 2: drop a late press, and model the frame delays

Date: 2026-11-08. Planner: Fable, task PL-FREAK-PLAN, session 6341c618. Follows `docs/superpowers/plans/2026-11-07-player-limits.md` (the approved limits plan) and the handoff `docs/handoffs/2026-11-07-player-limits-handoff.md` (in the `handoff-player-limits` worktree, not on main). The user chose "Planner for both": fix both causes of Freak run 1 before run 2, not a bigger margin.

## The plain version

Freak run 1 scored 476,455 against Hydra's 531,685, with 4 activations instead of 5. Two things went wrong, and each gets a fix.

First, the game stalled for about 130 ms at song time 33.6 s. The player then sent chord 153 107 ms late, far outside its window. That press did not just miss: it took chord 154's yellow cymbal (TM-50), so two chords were lost instead of one. The fix is simple: before sending a press, the player checks where the game will judge it, and if that is past the chord's window it does not send it. The row says "dropped". Nothing can make the stall go away; this makes it cost one chord, not two.

Second, the plan's 6 ms margin was too small, and it was too small for a reason the data now explains. Every press goes through two one-frame steps. The player fires a press when its clock estimate reaches the target, but the game's clock only moves once a frame, so the clock the player reads at the send is 0 to 3 ms past the target (nine probe rows show 0.24 to 2.93 ms). Then the game reads the press in the next frame, so it is judged 3 ms after that clock (four activation reads show the end stamped 2.98 to 3.03 ms after the clock the player read). So a press is judged between 3 and 6 ms after the time it was aimed at. The old lead solver did not know this. It aimed the activation 6 ms earlier than the bare need and the late press 6 ms inside its window, but the activation itself was judged a frame late (so the end came 4 ms later than planned), and the late press was judged a frame late too, and together they ate more than 6 ms. Chord 1144 landed 0.12 ms inside its window by the player's clock and outside it by the game's. The fix is to put both steps into the solver as named frames, so the lead is "what the squeeze needs, plus two frames for the activation press, plus two frames for the late press", and a small margin on top only for a slow frame. With that, Freak's two squeeze-outs each keep 3 ms of room in the worst case; today one of them has 0.5 ms and the other has minus 6.

Three small doc write-backs ride along, each checked against the source first: page 05 gets the frame-read fact, page 03's TM-50 loses the words that blame the player's process for the stall (the recorder shows the game's own threads stopped), the live-checks page and the README count table are brought up to date, and `ch_kit_press`'s text in `server.py` is checked.

## What was checked, and how

Every number here was read from the run folders and the code, not from the handoff. The Premise map at the end names each source. Three handoff claims needed correcting or sharpening:

- The handoff and `replies.txt` disagree on who stalled. `replies.txt` says "the worker process stalled, not the machine"; the handoff says the game stopped. The recorder settles it: in `machine-2\threads.csv` the game's (pid 9604) thread switches fall from about 600 per 20 ms sample to 26 to 28 for samples 1974 to 1978 (wall 1794112541.38 to 541.46), with 8 or 9 of its 27 threads left. The press rows agree: chords 152 and 153 are 44 frames apart over 262 ms of wall time, where 3 ms a frame gives 87. The game's frames stopped for about 130 ms. Page 03's TM-50 says "a 131.5 ms stall in the player's own process", which is the superseded reading; T3 corrects it.
- "Two one-frame delays" is right as a count of steps, but they are not both game lags. One is the player's own: the clock it reads at the send is already 0 to 3 ms past the target (`SongClock.read` returns `raw` as the estimate whenever no fresh frame change was seen, and `wait_until` fires at the first read at or past the target). The other is the game's: it judges the press in the frame after the one whose clock the player read. The end reads prove the second one directly: the end came 2.98 ms after the clock the player read (Bomber 147, run `ls-r2ab-run1`: lead read 53.78, end moved 50.8) and 3.03 ms after it on each of Freak 232, 633 and 1062 (the judged time, worked back from `end_read_ms` and the dump's stored end, minus `clock_ms_at_press`), and the LS-E rows bound it from the other side (TM-27: at least 2.71 ms). The brief's "about 4 ms late" for Freak's end is the sum: 1.08 ms of send overshoot plus about 3 of judging.
- The 6 ms margin was not only too small for chord 1144. Freak's other squeeze-out, chord 387, has 0.5 ms of room in the worst case under today's rule (its activation 232 is pressed plain, 2 ms early, so it is judged 1 to 4 ms late and the end moves later by that much). It hit in run 1 by luck of the frame.

The lane branch `claude/pl-lane-earliest` is already merged on `main` (merge `02190724`), so there is nothing to plan around; T2 leaves `_refuse_far_presses` alone and T1 does not touch it either.

Prior art: frame-locked input polling adds up to one frame of latency between an input and the frame that reads it; Unity's own Input System manual describes it and the fix of reading events with their own timestamps (https://docs.unity3d.com/Packages/com.unity.inputsystem@1.18/manual/timing-optimize-fixed-update.html), and rhythm-game tutorials put the same point as "if frames are at 0, 20, 40 ms and the beat is at 50 ms, there is a 10 ms delay until it is seen" (https://fizzd.notion.site/How-To-Make-a-Rhythm-Game-Part-2-Audio-WIP-12c56c4457a947458e1c7a0cc9b2b448). A bot driving a game cannot timestamp its own input inside the game, so it does what this plan does: model the frame step and aim inside it. No published bot for Clone Hero with a frame-latency model was found (searched 2026-11-08: "rhythm game bot input latency frame delay compensation one frame late judged next frame Unity Input polling").

## Rulings that bind this plan

- "Planner for both" (the user, 2026-11-08, quoted in this plan's brief): fix both causes before Freak run 2.
- "we should address these limits instead of accepting them" (session d0a3868c): the reason the limits plan exists.
- "Yes, all recommended" (the limits plan, including 6 ms as `ACT_LEAD_MARGIN_MS`); this plan asks to change that number (question 2) because its meaning changes.
- The user's numbers `SP_END_LEAD_MS` 15 and `SP_OUT_LAG_MS` 15 keep their meaning: the time the press is aimed at, measured from the end. This plan adds the frame steps around them and changes neither.

## The design, point by point

### 1. One frame, named once

`FRAME_MS` is a new constant in `play_chart.py` beside `ACT_LEAD_MARGIN_MS`: the game's frame period, 3.0 ms (question 1). TM-49 gives about 330 frames a second; the four end reads above give 2.98, 3.03, 3.03 and 3.03 ms. Every rule below counts in frames of it. Nothing else in the file holds a 3.

### 2. Where a press is judged

`judged_ms(target_ms)` is one small function: the latest clock the game can judge a press aimed at `target_ms`, which is `target_ms + 2 * FRAME_MS` (one frame of send overshoot, one frame of reading). Its early twin, the earliest, is `target_ms + FRAME_MS`. Both exist because a rule that needs the press late uses the latest, and a rule that needs it early uses the earliest. The lead solver and the drop check call these and nothing else works the frames out.

### 3. The lead solver counts the frames

`activation_leads` and `_solve_lead` change in three places, and the margin stays only as a margin.

For a squeeze-out chord (press late, after the end): the late press is aimed at the moved end plus `SP_OUT_LAG_MS`, and the game may judge it up to two frames later, so the need is `end + SP_OUT_LAG_MS + 2 * FRAME_MS - chord.ms - side`. The activation press aimed `lead` early is judged between one and two frames late, so the end moves by only `(lead - 2 * FRAME_MS) * ratio` in the worst case. So `lead = need / ratio + 2 * FRAME_MS + ACT_LEAD_MARGIN_MS`. A need that comes out at or under zero still gets this rule applied: if the resulting lead is not more than `PRESS_LEAD_MS`, the chord is pressed plain as today; otherwise it gets the lead. That is what gives Freak's chord 232 its new 4.5 ms lead.

For a squeeze-in after the end (press early, before the end; the activation goes late): the inside press is aimed at the moved end minus `SP_END_LEAD_MS` and is judged at least one frame later, which helps, so the need is `chord.ms + SP_END_LEAD_MS - FRAME_MS - end - side`; the activation aimed `lead` late is judged at least one frame later still, which also helps, so the end moves by at least `(lead + FRAME_MS) * ratio`, and `lead = need / ratio - FRAME_MS + ACT_LEAD_MARGIN_MS`, floored at zero. No chart in the run order has this case (Yandere was refused at LS-L0); the test proves it on a made-up fixture.

The fit checks use the judged times too: the activation's own press, judged between one and two frames after its aim, must sit inside its own side, so the check is `lead - FRAME_MS <= side_ms - ACT_LEAD_MARGIN_MS` for an early lead (the press is judged no earlier than `lead - 2 * FRAME_MS` and no later than `lead - FRAME_MS`; the earlier one is the one that can leave the window). The steady-span checks keep the full `lead`.

`planned_end_ms` becomes the end the game should store if it judges the activation one frame after the aimed time: `stored_end_ms + end_shift` with the shift from `lead - FRAME_MS`. The reply's line then reads near zero when the game does what the model says, and the live read's pass band is 0 to one frame later (the send overshoot).

The worked numbers, from the files in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-l0` with `FRAME_MS` 3.0 and the margin 3.0 (`freak_margins.py`, this session's scratch script; row P14 is unchecked until T2's tests pin them):

| Chart | Activation | Squeeze-out | Side | Old lead | New lead | Worst judged late press | Room |
|---|---|---|---|---|---|---|---|
| Freak | 232 | 387 | 53.59 | plain (2.0) | 4.50 early | 50.59 | 3.00 |
| Freak | 1062 | 1144 | 53.59 | 51.68 | 60.68 early | 50.59 | 3.00 |
| Rescued | 530 | 561 | 61.47 | 57.13 | 65.96 early | 58.38 | 3.09 |
| Rescued | 820 | 931 | 61.94 | 58.82 | 67.79 early | 58.93 | 3.01 |
| Ghost | 661 | 756 | 66.34 | 63.15 | 72.07 early | 63.30 | 3.04 |

Under today's rule the worst-case room is 0.50 for Freak 387 and minus 6.00 for every led activation. Every new lead fits its activation's side (Freak 73.48 and 73.76, Rescued 75.93 and 76.49, Ghost 78.75) with the judged press 54.7 to 69.1 ms early. Bomber's fixture (chord 147, need 45.27, ratio 1.0) comes out at 60.27; T2's test pins what the formula gives and its commit message names the change from 51.27. No activation still fails. Freak's squeeze-in 717 sits 84.27 ms before its end and needs nothing.

### 4. The live re-timing checks the room again, and says what it did

After the activation's first group, `play_chords` reads the game's end and `retime_from_end` moves the late press to the read end plus `SP_OUT_LAG_MS`, as today. New: it also asks whether that press can still land, `read_end + SP_OUT_LAG_MS + 2 * FRAME_MS <= chord.ms + side`. When it cannot, question 3 says what happens; the recommended answer is that the late gem goes down with the last inside group instead, the row's `hit` stays the chord's, and a new column `retimed` on the late gem's row says `inside` (or `late` when the squeeze stands). This is a game-visible change against Hydra's path (the chord scores inside Star Power, so the squeeze's points are not scored as Hydra scored them), so it is the user's call, not a default.

### 5. A press past its window is not sent

In `play_chords`, after `wait_until` returns, the press is dropped when the latest clock the game can judge it at, `raw_ms + FRAME_MS`, is past `chord.ms + side` (the chord's own side from `window_sides`; both sides are equal and the check holds the press to them, TM-26 and TM-27). `raw_ms` is used because the overshoot has already happened at that read. A dropped message sends nothing, reads no end, and writes one row per part with `hit` set to `DROPPED` ("dropped"), the clock it was dropped at in `clock_ms_at_press`, and no frame columns. `RunResult` gains `dropped`; `sent` and `hit` leave dropped presses out, as they leave declined ones out, and the streak resets. A later order group goes with its first press: if the first press is past its window, the whole message is dropped, because a later group is at most 16 ms after it and the chord has already missed. `ch_play_status` prints "dropped: chords 153" after the misses line, or "dropped: none". A deliberately late press (a squeeze-out's late gem) passes the same check; by design point 3 it has room, and by point 4 it is moved inside when it has none, so a drop there means the model is wrong and the row shows it.

Forcing the case: the fake clock in `test_play_chart.py` jumps 131 ms during the wait for one chord, so the run reaches the chord 107 ms late, as run 1 did. The test shows the row dropped, no press call for it, and the next chord pressed and hit.

### 6. The reply lines

`activation_lead_lines` prints the lead and, new, the two frames it holds: "activation chord 1062 pressed 60.7 ms early for the squeeze-out at chord 1144 (bare need 51.7, two frames 6.0, margin 3.0; end planned 141,607.6 ms, ratio 1.0000)". `activation_end_lines` is unchanged. `ch_play_status` gains the dropped line and the retimed words of point 4.

## Questions

1. **`FRAME_MS`.** A new number. Sides: (a) 3.0 ms, one frame at TM-49's about 330 a second; the four end reads gave 2.98, 3.03, 3.03 and 3.03; (b) 3.03 (1000/330); (c) read it live from the kit agent's frame rate each run. Recommended: (a); it is what the reads show, it is pinned by one test, and a live read adds a never-tried step before the first press. Example from each end: at 3.0 the worst judged late press on Freak 1144 is 50.59 against 53.59; at a 4 ms frame (250 fps, a dip TM-49 has seen only under a build) it is 52.59, still inside.
2. **`ACT_LEAD_MARGIN_MS` changes meaning, and its value.** Today it stands in for the frames (6 = two frames). Once the frames are named, it covers only a slow frame and the read latency. Sides: (a) 3.0 ms (one slow frame); (b) 0 (pure model: Freak 1144's worst-case room becomes 0.0, and one frame at 300 fps instead of 330 puts it outside); (c) keep 6 (Rescued 820 then leads 70.79 against a side of 76.49, judged 64.8 to 67.8 early: still fits, and so do the others). Recommended: (a). The constant keeps its name and its home; the test that pins it changes from 6.0 to the chosen value.
3. **When the read end leaves the late gem no room.** The code's rule after T2: the late press is kept only when `read_end + 15 + 2 * FRAME_MS <= chord.ms + side`. Sides: (a) press the late gem with the last inside group, so the chord scores inside Star Power and keeps the combo, and the row says `retimed=inside`; (b) send it anyway, as today, and take the likely miss, the combo reset and the chance it takes the next same-pad note (what run 1 did at 1144); (c) drop it (design point 5 would do this anyway when it is past the window). Recommended: (a). Example from each end: with Freak 1144's run-1 end read of 141,608.66, the rule gives 141,629.66 against 141,625.57, so (a) presses it inside and the chord scores x2 at 1144 instead of Hydra's split; with the new lead the read end should be 141,598.6 to 141,601.7, which leaves 6.0 down to 2.9 ms after the two frames, and the squeeze stands.
4. **The dropped row's words.** `hit` holds `dropped`, as it holds `declined` today, and the status line names the chords. Sides: (a) as written; (b) a separate column. Recommended: (a); `score_compare.py` and `ch_compare` read `hit` by value and treat anything but 1 as not hit, which is right for a drop.
5. **TM-50's stall sentence.** The page says the stall was "in the player's own process"; the recorder shows the game's threads stopped (What was checked). Sides: (a) T3 rewrites the sentence to name the game's stall with the recorder's numbers, and names the player's watch gap as the same 131.5 ms; (b) leave it and add the game's side as a second sentence. Recommended: (a); one sentence that says what happened.
6. **The run order.** Freak run 2 (LS-F2a/b below) is the limits plan's LS-F1b with new reads, and its two runs are not yet used by this plan. After it passes, the limits plan's LS-R1 (Rescued) and LS-G (Ghost backup) run with the new leads from the table (their first reads change to 65.96 and 67.79, and 72.07). Sides: (a) as written; (b) rerun Freak only. Recommended: (a).

## Tasks

Each task is sized for one Opus executor, under 100 tool calls and 20 minutes, cut from current `main` (which holds `02190724`), never from this plan's branch, which was cut before that merge. Agents never build and never run `hydra_tests`, `hydra_replay` or `hydra_uitest`. T1, T2 and T3 are independent and run in parallel; T1 and T2 both edit `tools/ch_probe/experiments/play_chart.py` and `tools/ch_probe/tests/test_play_chart.py`, in different functions and in new test classes, so the main session merges T1 first and T2's reviewer resolves the one-file overlap if git cannot. Every task runs only `py -I -m unittest tools.ch_probe.tests.test_play_chart` and `py -I -m unittest tools.ch_mcp.tests.test_game_tools` from its worktree, plus `hydra_precheck.exe` from the main checkout.

### T1: a press past its window is dropped (Python; Opus)

Owns: `play_chords`, `RunResult`, `DROPPED` and `FRAME_MS` (the constant only; T2 adds the solver's use) in `tools/ch_probe/experiments/play_chart.py`; the dropped line and the row counts in `tools/ch_mcp/game_tools.py` (`ch_play_status`'s builder: its `presses_sent` and `chords_sent` sums and its `logged` count test `hit != DECLINED`, so a dropped row must be left out of the sums and counted in `logged`, or the status reports a clock jump; read them on `main`); new test classes at the end of `tools/ch_probe/tests/test_play_chart.py` and `tools/ch_mcp/tests/test_game_tools.py`; the `retimed` and dropped words in `tools/ch_probe/experiments/overlay_logs.py` only if the column list needs them (question 4 (a) needs none).

Design point 5. Acceptance:
- `FrozenClockDropTest`: a fake clock that jumps 131 ms during the wait for chord k reaches it with `raw_ms + FRAME_MS` past its side; the row has `hit == "dropped"`, the press callable was not called for k, `result.dropped == 1`, `result.sent` leaves it out, the streak is 0 after it, and chord k+1 is pressed and hit. The test fails if the press is sent (the fake press records every call).
- The boundary: a press whose `raw_ms + FRAME_MS` equals `chord.ms + side` exactly is sent (inside, `<=`); one 0.01 ms further is dropped.
- A split chord whose first press is past its window sends no message and writes one dropped row per group.
- A squeeze-out's late press is checked the same way (a fixture with the end read far too late shows the drop and the row).
- `test_game_tools`: a run with one dropped row reports "sent" counts without it and does not say the song clock jumped; the status prints "dropped: chords 153" from a row list with one dropped row and "dropped: none" without.
- `FRAME_MS == 3.0` pinned (question 1), with a comment naming this plan's question 1.

### T2: the lead solver counts the frames, and the re-timing checks the room (Python; Opus)

Owns: `activation_leads`, `_solve_lead`, `ActLead`, `_timed_presses`'s squeeze-out branch, `retime_from_end`, the `judged_ms` pair, `ACT_LEAD_MARGIN_MS`'s value and comment in `play_chart.py`; `activation_lead_lines` in `game_tools.py`; `SolvedLeadTest`, `ReadEndTest` and the margin pin in `test_play_chart.py` (changing their numbers), the reply-line test in `test_game_tools.py`; the fixtures `tools/ch_probe/tests/*_limits_*.json` and `*_refine_*.json` are read, never rewritten. Also owns every assertion in `test_play_chart.py` and `test_game_tools.py` that pins a solved lead, a row's lead, a planned end or the margin: grep both files for `51.28`, `43.28`, `51.27`, `51.68`, `58.82`, `57.12`, `ACT_LEAD_MARGIN_MS`, `126233.85` and `186348.95`. On `main` those sit in `DeclineAndLeadTest`, `SqueezeTimingTest`, `ChordOrderTest`, `LaneAwareCheckTest`, `SolvedLeadTest`, `MovedEndTest` and `ReadEndTest`; the margin is pinned twice (lines 1200 and 1780). Does not touch `_refuse_far_presses` or `play_chords`' drop check (T1).

Design points 1 to 4 and 6. Acceptance:
- The margin pin changes to the chosen value (question 2) and names this plan.
- Freak: the existing fixture pair (`freak_limits_score.json`, `freak_limits_dump.json`, `_FREAK` in the test file) is used; `test_freak_loads_with_1062_led_by_the_formula_and_387_needing_none` changes, because 387 now gets a lead; `activation_leads` gives leads for 232 (4.50, for 387) and 1062 (60.68, for 1144), each within 0.01; chord 387's late press at `planned_end + 15` lands 50.59 ms or less after 387 when judged two frames late; 1144 the same within 0.01; `planned_end_ms` of 1062 is `141,656.25 - (60.68 - 3.0)` within 0.01. The test fails if the frames are folded back into one margin, because 4.50 then becomes "plain".
- Rescued: leads 65.96 (530 for 561) and 67.79 (820 for 931); Ghost: 72.07 (661 for 756); Bomber: the formula's number, named in the commit message against 51.27. A test that gets another number says which input differs (the fixture's side, ratio or stored end).
- A squeeze-in-after-the-end fixture (made up) gets a late lead with `- FRAME_MS`, floored at zero, and the fit check uses the judged times.
- An activation whose judged press would leave its own side is refused naming the chord, the lead and the side, with the frames in the words.
- `retime_from_end` with a read end that leaves room keeps the late press at `read_end + 15` and marks it `retimed=late`; one that leaves none applies question 3's answer ((a): the gem joins the last inside group, `retimed=inside`); the Bomber `ReadEndTest` numbers move by the new planned end and still pass with a 3 ms later read.
- `activation_lead_lines` prints the bare need, the two frames and the margin as design point 6 shows, pinned by one string test.

### T3: the write-backs (docs and one tool text; Opus or Sonnet; no code change)

Owns: `docs/clone-hero/05-star-power.md`, `docs/clone-hero/03-timing.md` (TM-50's rule text only), `docs/clone-hero/live-checks.md`, `docs/clone-hero/README.md` (the count table and its date), the `ch_kit_press` docstring in `tools/ch_mcp/server.py`. Checks each is stale before changing it, and reports any that is not.

- Page 05: a new block (next free SP id) "The end is stamped from the frame that reads the press", Label "Proven in a live run": the end came 2.98 ms (Bomber 147, `ls-r2ab-run1\replies.txt`) and 3.03 ms on each of Freak 232, 633 and 1062 (`ls-f1-run1\press-2.csv`: the judged time is `end_read_ms` less the dump's stored end plus `chart_ms`; subtract `clock_ms_at_press`) after the song clock the player read at the send, one frame at TM-49's rate; r read as 1.0 at these; Hydra line `retime_from_end`; Found this session and d0a3868c. SP-39's label stays (r was 1.0 everywhere, which checks nothing about r); the block's evidence line says so.
- Page 03 TM-50: question 5's answer; cite `machine-2\threads.csv` samples 1974 to 1978 and press rows 152 and 153. The page's shape holds (seven lines, labels, backticked names exist).
- live-checks.md lines 81 and 83 (TM-22/TM-27 and TM-26) are answered by LS-E; move them to the page's answered form, or strike them the way the page strikes answered rows, and bump its write-back date.
- README count table: recount every page's `###` headings and labels (TM-50 and the new SP block change two rows and the total), and the "Last write-back" line.
- `server.py` `ch_kit_press` docstring: compare with `kit_tools.ch_kit_press`'s parameters and reply; if nothing is stale, the report says so and the file is untouched.

Acceptance: `hydra_precheck.exe` from the main checkout passes on the branch; each changed line is quoted in the report with its source.

## The live steps

The main session runs these; the limits plan's run rules hold (hold other sessions, MD5 before and after, copy the stats and the watch out before `ch_stop`, `replies.txt` in a dated folder, `ch_reload` after the merge). Each step caps at 2 runs and 10 minutes; a step at its cap stops and goes to the user. A lead is read as `chart_ms` minus `clock_ms_at_press`: aimed lead down to aimed lead less one frame for an early press.

### LS-F2a: the Freak launch (2 runs, 10 minutes)

Calls: `ch_start(<Get Ur Freak On folder>, human=True, player="ProDrums,Expert")` (seen: LS-F1a run 1)

Uses rows P1, P10. Pass: quiet, recorder pid and folder, game pid, song length 192,934 ms, loader ran after the patch. Fail: a refusal (wait for a held machine) or another song length (stop). LS-F2b starts at once.

### LS-F2b: Freak against 531,685 with the new model (inside LS-F2a's run; 2 runs)

Calls: `ch_run_chart(<Freak folder>, <ls-l0 freak score>, kit_player="System", decline_mode="all_but_top_pad", dump_json=<ls-l0 freak dump>)` (seen: LS-F1b run 1), `ch_play_status` (seen: LS-F1b run 1), `ch_sp_watch_status` (seen: LS-F1b run 1), `ch_wait_stats` (seen: LS-F1b run 1), `ch_compare(<stats copy>, <ls-l0 freak score>)` (seen: LS-F1b run 1), `ch_stop` (seen: LS-F1b run 1)

Uses rows P6, P8, P9, P13, P14 (settled by T2 before this runs). Reads, in order. Zeroth: the reply's lead lines name 232 at 4.5 ms and 1062 at 60.7 ms with the frames and margin. First: chord 232's row lead 4.5 to 1.5 ms early; its end line 0 to 3.1 ms later than planned. Second: chord 1062's row lead 60.7 to 57.7 early; end line 0 to 3.1 later than planned. Third: chord 387 `hit` 1, pressed at most 50.6 ms late by its row. Fourth: chord 1144 `hit` 1, at most 50.6 ms late, `retimed` `late`. Fifth: no row with `hit` `dropped`; if the game stalls again, the dropped row and the next chord's `hit` 1 are the read instead, and the run's score is short by that chord alone. Sixth: `ch_compare` passes at 531,685 with 5 activations, MD5 unchanged, restores "unchanged" or "restored".

Pass: all six, or five with the fifth's stall form and the score short by the dropped chord's points alone. Fail, first or second read: an end line outside 0 to 3.1 ms (row P9 is wrong at this tempo: stop, the numbers to the user). Fail, fourth read: `hit` 0 with `retimed` `late` (the model's room was not enough: stop, the row's clock and the end to the user; no number is changed by hand). Fail, fourth read with `retimed` `inside`: the squeeze was given up; the score differs from Hydra's by that chord and the user decides whether that is a pass. Run 2 only when a fail line names it or the machine was not held.

After LS-F2b passes, the limits plan's LS-R1a/b and LS-G1a/b run as written there, with their first reads changed to the table's leads (Rescued 65.96 and 67.79; Ghost 72.07) and the end lines read 0 to 3.1 ms later than planned.

## Out of scope (needs the user's yes to leave out)

- Reading the frame period live from the kit agent (question 1 (c)).
- The stall's root cause inside the game (garbage collection or other); the drop rule limits its cost, and the recorder's samples are kept for a later look.
- Yandere Complex's late activation: refused at LS-L0; the late-direction rule is built and tested on a fixture only.
- The page 07 write-back the handoff names (Rewired's `ControllerElementType`, Axis 0 and Button 1) belongs to the held T5 and is not touched here.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| A press the game would judge past its window is not sent; the row says `dropped`; the next chord is still pressed | design 5 | T1 | `FrozenClockDropTest`: no press call for k, row `hit == "dropped"`, k+1 hit |
| The drop boundary follows TM-27: equal is inside, further is dropped | design 5 | T1 | the boundary test, both ends |
| A split chord past its window sends nothing and writes one dropped row per group | design 5 | T1 | the split-chord drop test |
| `sent` and `hit` leave dropped presses out; `dropped` counts them; the streak resets | design 5 | T1 | `FrozenClockDropTest` asserts on `RunResult` |
| `ch_play_status` names dropped chords or says none | design 5 | T1 | the two status tests in `test_game_tools` |
| `FRAME_MS` is 3.0 and the only frame number | design 1; Q1 | T1 (pin), T2 (use) | the pin test; the reviewer greps `play_chart.py` for a bare `3.0` outside it |
| A squeeze-out's lead is need over ratio plus two frames plus the margin, with the need counting two frames of the late press | design 3 | T2 | Freak 232 at 4.50 and 1062 at 60.68; Rescued 65.96 and 67.79; Ghost 72.07; Bomber's named number |
| An activation with a need at or under zero still gets the lead when it is above `PRESS_LEAD_MS` | design 3 | T2 | Freak 232 (old need -10.5) at 4.50 |
| A squeeze-in after the end uses one frame each way and floors at zero | design 3 | T2 | the made-up late-lead fixture test |
| The fit check uses the judged press times; a misfit is refused naming chord, lead, side and frames | design 3 | T2 | the refusal test's exact message |
| `planned_end_ms` is the end if the activation is judged one frame after its aim | design 3 | T2 | 1062's planned end 141,598.57 within 0.01; LS-F2b first and second reads 0 to 3.1 later |
| After the read the late press keeps its place only with two frames of room, else question 3's answer; the row says which | design 4; Q3 | T2 | the two `retime_from_end` tests; LS-F2b fourth read |
| The reply names bare need, frames and margin | design 6 | T2 | the string test |
| The margin is the chosen value and only a margin | Q2 | T2 | the pin test with this plan named |
| Page 05 holds the frame-read fact as a proven block; TM-50 names the game's stall; live-checks, the README table and `ch_kit_press`'s text are checked and fixed only where stale | design, write-backs; Q5 | T3 | each changed line quoted with its source in T3's report |
| Every live step has a cap and a stop line; no step repeats on its own | the live steps | LS-F2a, LS-F2b | each step's first and last sentence |
| Freak run 2 passes at 531,685 with 5 activations, or fails with the named read | LS-F2b | LS-F2b | the sixth read |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: Freak run 1 scored 476,455 against Hydra's 531,685, with 4 activations of 5 | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-f1-run1\replies.txt` line "ch_compare: FAIL" | checked 2026-11-08 |
| P2: chord 153 was pressed 107.47 ms late and hit for 65; chord 154 pressed 0.83 ms early missed | run | `ls-f1-run1\press-2.csv` rows 153 and 154 (`chart_ms`, `clock_ms_at_press`, `score_before`, `score_after`, `hit`) | checked 2026-11-08 |
| P3: the watch has a 131.5 ms gap, wall 1794112541.3528 to 541.4843, clock 33.618 to 33.771 s | run | `ls-f1-run1\sp_watch-1.csv` rows at those `wall_s` | checked 2026-11-08 |
| P4: the game's own threads stopped: switches 568 to 686 per sample fall to 26 to 28 for samples 1974 to 1978, 8 or 9 of 27 threads left | run | `ls-f1-run1\machine-2\threads.csv` rows with `pid` 9604 at those samples; `samples.csv` gives their `wall_s` 541.38 to 541.46 | checked 2026-11-08 |
| P5: chords 152 and 153 are 44 frames apart over 262 ms of wall time | run | `ls-f1-run1\press-2.csv` rows 152 and 153, `down_frame` 11518 and 11562, `down_ms` 1794112541224 and 541486 | checked 2026-11-08 |
| P6: chord 1144 (side 53.59) was aimed 47.59 ms late, the end read 141,608.66 against a planned 141,604.6, the press read 53.47 ms late and missed | run | `ls-f1-run1\replies.txt` "cause 2" line; `press-2.csv` rows 1062 (`end_read_ms`) and 1144 | checked 2026-11-08 |
| P7: at an 85.72 ms side, presses read 80.91 to 88.43 ms early hit and 84.92 to 91.65 ms late missed | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-e-run1\press-1.csv` rows with a `probe` value; `probes.json` there | checked 2026-11-08 |
| P8: the clock read at the send is 0.24 to 2.93 ms past the aimed time in all nine probe rows and 1.08 (1062) and 1.78 (1144) on Freak | run | `ls-e-run1\press-1.csv` `clock_ms_at_press` against `chart_ms` less the asked lead; `ls-f1-run1\press-2.csv` rows 1062 and 1144 | checked 2026-11-08 |
| P9: the end is stamped 2.98 to 3.03 ms after the clock read at the activation's send: Bomber 147 +2.98, Freak 232, 633 and 1062 +3.03 each | run | `ls-r2ab-run1\replies.txt` "the end moved 50.8 ms for a 53.78 ms early press" (53.78 is the lead read from the clock, press-2.csv line of the replies); `ls-f1-run1\press-2.csv` `end_read_ms` of 232, 633, 1062 less the dump's stored ends 57,932.89, 98,444.63, 141,656.25, plus `chart_ms`, minus `clock_ms_at_press` | checked 2026-11-08 |
| P10: the game runs about 330 frames a second | data | `docs/clone-hero/03-timing.md` TM-49 (line 151) | checked 2026-11-08 |
| P11: `wait_until` fires at the first read whose estimate is at or past the target; `SongClock.read` returns the raw clock as the estimate when no fresh change was seen | code | `tools/ch_probe/experiments/live.py` lines 131 to 145; `tools/ch_probe/experiments/walk_edges.py` lines 114 to 125 | checked 2026-11-08 |
| P12: today's solver adds `ACT_LEAD_MARGIN_MS` to the bare need and checks the aimed press against the side less the margin; `retime_from_end` moves presses by the read end and checks no window; `play_chords` sends every press `wait_until` returns for | code | `tools/ch_probe/experiments/play_chart.py` lines 336 to 417, 438 to 448, 1050 to 1105 | checked 2026-11-08 |
| P13: Freak's dump gives ratio 1.0 both ways at 232, 633 and 1062, stored ends 57,932.89, 98,444.63 and 141,656.25, squeeze-outs 387 and 1144 with side 53.59, squeeze-in 717 at -84.27 | chart | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-l0\Eric Moore - Get Ur Freak On [Missy Elliot]-dump.json` and `-score.json`, loaded through `load_dump_chords` | checked 2026-11-08 |
| P14: the new leads are Freak 4.50 and 60.68, Rescued 65.96 and 67.79, Ghost 72.07, each with 3.0 to 3.1 ms of worst-case room | chart | the `ls-l0` Freak, Rescued and Ghost dumps and scores, loaded through `load_dump_chords` on `main`, with design 3's formula applied | checked 2026-11-08 (recomputed by the plan reviewer; T2's tests pin them) |
| P15: the lane change is on `main` | code | `git log main` shows merge `02190724` with `5a7f8dd4` | checked 2026-11-08 |
| P16: TM-50 says the stall was "in the player's own process" | code | `docs/clone-hero/03-timing.md` line 315 | checked 2026-11-08 |
| P17: the README counts page 03 at 48 blocks while TM-50 exists; live-checks lines 81 and 83 still pose TM-22/TM-27 and TM-26 as open | code | `docs/clone-hero/README.md` lines 59 and 53; `docs/clone-hero/live-checks.md` lines 81 and 83; `03-timing.md` line 313 | checked 2026-11-08 |
| P18: `ch_kit_press`'s docstring in `server.py` is stale | unchecked | `tools/ch_mcp/server.py` lines 315 to 327 read; nothing stale found by this planner against the parameters shown | T3 |
| P19: the user chose "Planner for both" | ruling | the user's words "Planner for both" in session `6341c618`, quoted in the reviewer's brief and confirmed by the main session | checked 2026-11-08 |
| P20: the user chose 6 ms as `ACT_LEAD_MARGIN_MS` ("Yes, all recommended") | code | `docs/superpowers/plans/2026-11-07-player-limits.md` question 1 (line 98) and the two test pins at `tools/ch_probe/tests/test_play_chart.py` lines 1200 and 1780 on `main` | checked 2026-11-08 |
| P21: the press handler gives a press to the earlier of two unhit same-pad notes in window, so a late press can take the next note | data | `docs/clone-hero/03-timing.md` TM-50 (line 313), Proven in a live run | checked 2026-11-08 |
| P22: both window sides are equal and the check holds a press to them, judged some ms after the send | data | `docs/clone-hero/03-timing.md` TM-26 and TM-27 (lines 293 to 311), Proven in a live run | checked 2026-11-08 |
