# The schedule reads every Star Power ratio from Hydra

Task id PL-SP-RATIOS. Written 2026-11-12 for the user's "Yes, plan both". Nothing here starts before the user's yes (agent rule 3).

## The short version

The press schedule borrowed a number. When a chord's press moves a Star Power end, the schedule must say how far the end moves per millisecond of press. That figure is the ratio (ms of end moved per ms of press). Hydra has one ratio per end, because each end sits at a different tempo and time signature. The schedule took the ratio of one end and used it for another end. The player then made the same mistake again: it moved a later press by the full error it read, as if every end moved alike.

On Pitch Dark this cost a squeeze. The end the player read sits in a 5/4 measure, where the ratio is 1.3926. The end the squeeze chord needs sits in 4/4, where the ratio is 1.1067. The schedule priced the read with 1.1067. The read came back 14.2 ms later than that wrong price. The player moved the squeeze press 14.2 ms later. The end the press needed had moved only 5.1 ms. The press went down 3.8 ms before the end and the game did not count it inside Star Power.

The fix has three parts. Hydra's one owner of the ratio answers for every end, not just two of them. The schedule prices each read with the ratio of the end that read sees, and writes into the file how a read error converts to each later press. The player multiplies by the number in the file and works out nothing itself.

The user's words this plan answers: "the whole point of this plan is to hand off all the starpower planning to hydra which already knows what to do" (2026-11-08), and "why is there any guessing involved? there are rules that govern all of this and the rules are known and hydra accounts for them" and "the only the schedule needs to do is read what hydra already knows" (2026-11-12).

## What was checked for this plan

Every number below was read from the code, the chart or the run folder on 2026-11-12, not from the brief. The Premise map at the end names each source.

The chart's own tempo map gives both ratios with no model. The clamp on chord 466 (tick 182280) sets the end at tick 198570, inside the 5/4 measure that runs from tick 196320 to 198720. The length of a measure there, over the length of a measure at tick 182280, is 1.3926146644. The end after chord 567's collect is tick 202440, in 4/4, and the same division gives 1.1066660027. That second figure is exactly what `SPSqueeze::transfer` holds for the squeeze at chord 644. So Hydra's rule (`transfer_scale_between` in `src/core/squeeze_rating.cpp`) already gives the right number for any pair of ticks. Nothing was missing in the engine but a way to ask it about a middle step.

The schedule file from the run shows the borrowed number. The read after chord 466 is planned at 158,254.774 ms. That is the stored end 158,224.757 plus 1.1067 times 27.124 (the anchor's 24.124 ms lead plus one 3 ms frame). With the read's own ratio it is 158,224.757 plus 1.3926 times 27.124, which is 158,262.531.

The run's press log shows the rest. The game's read after chord 466 was 158,268.988. Against the right plan that is 6.457 ms late, not 14.2. An end 6.457 ms late in the 5/4 measure is 6.457 times 1.1067 over 1.3926, or 5.13 ms late, at the squeeze's end. Hydra's planned squeeze end is 161,386.460, so the true end is 161,391.59. The game's own final end was 161,391.6.

## The design

### 1. One owner answers for every step

An activation stores a list of steps (`Activation::sp_end_steps` in `src/core/model.h`). Each step is one place the end moved, with the end in force after it. `Activation::end_anchor_tick` names the note whose timing moves that end. The ratio of a step is `transfer_scale_between` called with that anchor and that end.

Today `frontend_transfer_scales` makes that call for two kinds of step only: the last one (stored as `Activation::transfer_post`) and the one each squeeze-in was measured from (stored as `SPSqueeze::transfer`). The plan adds one small function beside it in `src/core/squeeze_rating.cpp`, called here the step scale. It takes an activation, a step number and the song's timing, and returns the ratio of that step. `frontend_transfer_scales` is changed to call it for its two cases, so the stored numbers and the new ones come from one line of code. A test proves the step scale of the last step equals the stored `transfer_post`, and the step scale of each squeeze-in's step equals its stored `transfer`, on every activation of the fixture charts.

The ratio is not added to the stored record. Decision and reason: the record's format would change, and the only reader of a middle step's ratio is `hydra_replay schedule`, which already holds the song's timing (it calls `steady_spans` with it). Derive-once is kept because one function owns the rule and both the stored fields and the schedule go through it. Question 1 puts this choice to the user, because the user's words say "read what hydra already knows" and storing is the other way to read them.

### 2. The schedule prices each read with that read's own ratio

A read happens after a chord, and it sees the end in force after that chord's step. Its planned value is the step's stored end, moved by the chord's press, times the step scale of that same step. This replaces both branches in `tools/replay_schedule.cpp` that price a read today: the one for a chord that leads a squeeze (it uses the squeeze's ratio) and the one for a plain chord (it uses the final end's ratio).

### 3. The schedule writes the conversion; the player multiplies

Each press that is timed from an end has two ends in play. The read end is the one the player reads, after the anchor chord. The own end is the one the press must land before or after. Both move with the same anchor, by the same count of measures. So an error of 1 ms at the read end is an error of (own end's ratio over read end's ratio) ms at the own end.

The schedule writes three numbers on each such press. The first is the read's planned end, as now. The second is new: the own end as planned. The third is new: the conversion, the own end's step scale over the read end's step scale, both through the owner of point 1. The player then does one thing: it moves the press by the conversion times (the read minus the read's planned end). On Pitch Dark chord 644 the conversion is 0.7947. When the read end is the own end the conversion is exactly 1 and the player behaves as it does today.

### 4. Thresholds and fallbacks go into read terms by the same conversion

The fallback thresholds are worked out against the own end, and the player compares them with the read. Today the schedule turns one into the other by adding a fixed offset. The plan turns them with the conversion instead: the read value at which the own end reaches the threshold. The player still only compares.

The squeeze-out fallback also presses a gem "15 ms before the read end". It must be 15 ms before the own end. The player works out the moved own end as the planned own end plus the conversion times the read error, and uses that.

### 5. The ratio holds only on steady tempo, and the file says where

A ratio is true only while the end stays between two tempo or time signature changes. The schedule already refuses a lead that moves the anchor or the own end past such a change (`steady_spans`). The plan adds the same check for the read end. It also writes, for each read, the range of read values over which the conversion holds. Question 2 asks what the player does with a read outside that range.

## The sweep: every place a number for one tick is used at another

Each row was read in the code on 2026-11-12. "Wrong" means wrong whenever the two ends have different ratios; with equal ratios the output does not change.

| Place | What it does today | Verdict |
|---|---|---|
| `ratio_of` in `tools/replay_schedule.cpp`, set where each anchor's lead is chosen | Keeps the first squeeze's ratio per anchor, then prices the read after that anchor with it | Wrong. The read's end is the anchor's own step, not the squeeze's end. This is the Pitch Dark failure. Fixed in T2. |
| The plain-chord branch of the read pricing | Prices the read after an activation's own chord with `Activation::transfer_post`, the final end's ratio from the final end's anchor | Wrong whenever the activation has a later step. On Pitch Dark the read after chord 437 is priced at 1.0506 and its own ratio is about 1.37. Fixed in T2. |
| `in_read_terms` | Adds the own end's planned shift to the read's stored end | Wrong. Replaced by the read's planned end from design point 2. Fixed in T2. |
| The `terms` offset on `if_end_before_ms` and `if_end_after_ms` | Moves a threshold from own-end terms to read terms by adding a constant | Wrong. Converted by division by the conversion. Fixed in T2. |
| `final_shift` | Moves the final end by the ratio kept in `ratio_of`, and only for an anchor pressed early | Right by accident for a squeeze-out, whose ratio is the final end's. For an anchor pressed late it stays 0, so the chord on the end is aimed at an end the plan knows will move. Pitch Dark chord 676 shows it: its planned end is the unmoved 158,224.757. T2 prices the final end with its own step scale in both directions. |
| The `inside_moved_end` aim | Uses `final_shift` and `in_read_terms` | Wrong with them. Fixed in T2 by the same two changes. |
| `split_at_lead` and `equal_room_lead_ms` | Split the squeeze with the squeeze's own stored ratio at the squeeze's own end | Right. Pitch Dark's aim of 18.316 ms early for chord 644 matches the stored ratio. Not changed; a test pins it. |
| The steady check in `anchor_lead_ms` | Checks the anchor and the own end against tempo changes | Incomplete: the read end is not checked. Added in T2. |
| `retime_from_end` in `tools/ch_probe/experiments/play_chart.py`, the moved-press line | Moves a press by the read error, one for one | Wrong. Multiplies by the conversion. Fixed in T3. |
| `retime_from_end`, the join-group fallback | Presses the late gem a fixed time before the read end | Wrong when the read end is not the own end. Uses the moved own end. Fixed in T3. |
| `squeeze_lines` in `tools/ch_mcp/game_tools.py` | Prints the gap between a press and `aim_end_ms`, which is the read's end | Wrong text: for Pitch Dark chord 644 it would print about 3,117 ms after the end. Reads the own end. Fixed in T3. |
| The kit agent's retime in `tools/ch_mcp/agent/press_schedule.ts` | Its header comment says it takes the entries `retime_from_end` made and does no sum of its own | Not opened past that comment (row P13). T3 reads it and reports; if it holds any sum, T3 stops and says so. |
| A squeeze-in that falls back to plain | Later presses of the same activation still aim at ends that assumed the squeeze worked | Not a ratio error. Listed under Out of scope for the user. |

## The tasks

T1 goes first. T2 and T3 run in parallel after it, because this plan fixes the three new field names between them. T4 follows both. Each task is one executor on its own branch and one fresh reviewer (agent rules 5 and 6). Agents build nothing and run no C++ test; the main session builds and runs the tests each task names (rule 14).

### T1: the step scale in the engine's one owner

Add the step scale beside `transfer_scale_between` and make `frontend_transfer_scales` call it for its two cases. Change nothing stored.

Acceptance criteria:
- A new test, in the file that tests `frontend_transfer_scales` today (the executor greps for it), walks every activation of the corpus charts under `testdata/input/test_folded_sqin` and `testdata/input/test_fast_tempo`. For each, the step scale of the last step equals `transfer_post`, and the step scale of each squeeze-in's step equals that squeeze's `transfer`, exactly. It fails if the two code paths ever differ.
- The same test asks for a step number past the list's end and gets "unset", never a number.
- The existing test 'no fresh record stores an unknown transfer scale' still passes, proving the stored values did not change.

### T2: the schedule prices and converts with the step scale

Change `tools/replay_schedule.cpp` as design points 2 to 5 say, and fix every "Wrong" and "Incomplete" row of the sweep that names T2. Remove `ratio_of` and `in_read_terms`. Add Pitch Dark's chart as a fixture (see "The Pitch Dark fixture" below). Update `docs/development.md`, "Developer tools", for the three new fields, and correct the sentence in `docs/agents/clone-hero-mcp.md` step 5 that says the player moves a press "by the read end minus the end that press was aimed at".

Acceptance criteria, all in `tests/test_replay_schedule.cpp`, on the Pitch Dark fixture, path 0. Each literal is pinned from the chart and the run of 2026-11-12:
- The read after chord 466 is planned at 158,262.53 ms (within 0.01), not 158,254.77. The test fails if the read is priced with any ratio but its own step's.
- The read after chord 437 is priced with the step scale of step 0 of that activation, checked by calling the T1 function in the test, and is no longer 154,250.379.
- Chord 644's press keeps its aim of 18.316 ms early and its own planned end is 161,386.46. Its conversion is 0.79467 (within 0.00001). Its planned read end equals the read's planned end on chord 466, to the last digit.
- Chord 676's press (the chord on the final end) carries the final end's conversion and a planned read end equal to chord 466's read. The test computes neither; it compares the two fields of the file.
- Chord 243's press (activation 0, all 4/4) has its own planned end and conversion pinned from the first green run; its anchor aim 64.571 ms late and squeeze aim 40.919 ms early do not change.
- Chord 644's `if_end_before_ms` equals the read value at which the own end reaches the old own-terms threshold: the test feeds that value through the conversion and gets the own-terms threshold back.
- A hand-built case in the same file puts a tempo change 10 ms past a read end and a lead that moves the read end 20 ms: the schedule refuses, and the refusal names the read end's steady span. A lead that moves it 5 ms is not refused.
- On every Freak, Rescued and Ghost fixture press the existing pinned aims do not change, because there the read end is the own end. The existing tests prove it by still passing unedited, except where they pin `aim_end_ms` of a chord on a final end that follows a late-pressed anchor; the executor lists each such edit in its report.

### T3: the player multiplies and reads the own end

Change `retime_from_end` and the schedule loader in `tools/ch_probe/experiments/play_chart.py`, and `squeeze_lines` in `tools/ch_mcp/game_tools.py`. Read `tools/ch_mcp/agent/press_schedule.ts` and confirm it does no sum on an end; if it does, stop and report. A schedule file without the new fields is refused at load with a line naming the missing field, so an old file cannot be played by the new player.

Acceptance criteria, in `tools/ch_probe/tests/test_play_chart.py` and `tools/ch_mcp/tests/test_game_tools.py` (Python tests; the executor may run these two files):
- With Pitch Dark's numbers (planned read 158,262.531, conversion 0.79467, read 158,268.988), chord 644's press moves 5.13 ms later (within 0.01), not 6.46 and not 14.2. The test fails on a one-for-one move.
- With conversion 1 every existing retime test passes unedited.
- The join-group fallback presses the late gem 15 ms before the moved own end: a hand-built press with conversion 0.5 and a read 10 ms late goes 5 ms later, not 10.
- A schedule file missing the conversion or the own end is refused, and the message names the field.
- `squeeze_lines` on the Pitch Dark schedule prints chord 644 as 15.0 ms before the end.
- The answer to question 2 is implemented and tested at both ends: a read just inside the range the file gives, and one just outside.

### T4: fixtures and the reference page

The main session remakes the Python tests' schedule fixtures with the merged `hydra_replay schedule`, and an executor updates any literal that moved. `docs/clone-hero/05-star-power.md` takes the write-back: SP-40's evidence gains the Pitch Dark run (the clamped end moved with the collecting note's press at the 5/4 ratio, read 158,268.988 against a plan of 158,262.531), in the same commit as LS-R2's result.

Acceptance criteria:
- `tools/ch_probe/tests` and `tools/ch_mcp/tests` pass on the remade fixtures.
- The docs test (`hydra_tests.exe "-tc=docs match code*"`, run by the main session) passes with the page edit and this plan in the tree.

### The Pitch Dark fixture

Pitch Dark is a library chart, and the tests cannot read the library. The repo already keeps whole library charts under `testdata/input/common`, so the plan copies Pitch Dark's `notes.mid` and `song.ini` into a new folder of its own under `testdata/input`, beside `test_folded_sqin`, not under `common`. T2's tests run the engine on it and pick path 0.

Two facts here are unchecked (rows P14 and P15): that nothing walks all of `testdata/input` and pins a count a new folder would change, and that a fresh engine run's path 0 is the path the run played. T2's executor greps the tests for the first before adding the folder. The main session's first build of T2 settles the second: the test's first assertion is that path 0 has three activations with the step ticks of row P3. If either fails, T2 stops and reports; the fallback is a hand-cut chart with one clamp, one collect and one squeeze-in under a 5/4 measure, with its literals pinned from its first run.

No chart already in `testdata/input` is claimed to have this case. Forty-four files there hold time signature lines, but whether any path has a timed squeeze whose read end and own end sit in different measures needs a Hydra run, which a planning agent may not make.

## The live steps

Run rules for all three: the kit route, a human player on System, the game never brought to the front, no `SendInput`. The user allowed Pitch Dark human runs on 2026-11-12 ("Human run, allowed"). Each go request shows the user the step's call list first ("The shape of a live run" on `docs/agents/clone-hero-mcp.md`).

### LS-R1: remake Pitch Dark's three files with the merged build (no game; 1 run)

Calls: `hydra_replay` dump, score and schedule on Pitch Dark path 0 (never tried: the schedule mode is new code)

The main session writes dump, score and schedule into a new prep folder under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\`. Pass: no refusals; Hydra's total is 393,885 (row P16); the read on chord 466 is planned at 158,262.53; chord 644's conversion is 0.79467. Fail: any other value, or a refusal; stop and report the line. Cap: one run, 10 minutes.

### LS-R2: Pitch Dark path 0 on the kit from the new schedule (2 runs, 10 minutes each)

Calls: `ch_start` (seen: pitch-dark-run1), `ch_run_chart` (never tried: the schedule's new fields), `ch_play_status` (seen: pitch-dark-run1), `ch_sp_watch_status` (seen: pitch-dark-run1), `ch_stop` (seen: pitch-dark-run1)

Starts only when LS-R1's folder exists; the go request names it. Uses rows P11, P12, P19 and P20. Row P20 is unchecked: before the go request, the main session confirms from its own record of run 1 that the three status and stop calls ran; if it cannot, they are marked never tried and this step is split so each step holds one. Pass lines, read in this order:
1. The frame-rate line says 330 and the song check names Pitch Dark's folder.
2. The press row of chord 466 holds a read. The row of chord 644 is not `retimed=plain`.
3. Chord 644's press went down between 3 ms and 27 ms before the game's final end, read from the watch log. Both bounds are from the schedule's constants: the 15 ms lead, less and plus the room the split gives, floored at one 3 ms frame.
4. Chord 644 is paid inside Star Power: the game's score step across it equals the score file's value for that chord.
5. 1111 of 1111 notes and a game total of 393,885.

Fail lines: no read on 466 (stop: the player, not the ratio); line 3 fails (the conversion is still wrong: report the read, the conversion and the game's end); line 3 passes and line 4 fails (the ratio is right and the open oddity below decides it: stop, do not rerun, report the frames); line 5 alone fails (report the first chord whose score step differs). Every run also records, for each of the reads, the game's read minus the planned read, so the frame question below gets data either way. Cap: 2 runs. After the second, stop and put the design to the user (the memory rule on two failed runs).

### LS-R3: the watch log against the score file (no game; 1 run)

Calls: `ch_sp_compare` (never tried on this chart)

Runs on LS-R2's passing folder. Pass: no FAIL line; three activations, each end within the tool's own limit. Fail: quote the FAIL line. Cap: one run.

## Questions

1. **Where a middle step's ratio lives.** The rule is one line: the ratio of a step is `transfer_scale_between(anchor of the step, end of the step, timing)`. (a) One function owns it; the stored `transfer_post` and `transfer` and the schedule's new ratios all come from it; nothing new is stored. (b) The engine also stores the ratio on every step at copy-out, and the schedule reads the stored number. Examples from both ends: Pitch Dark activation 1 has four steps; under (a) the record still holds two ratios (1.1067 on the squeeze, 1.0506 on the end) and the schedule asks the owner for 1.3926; under (b) the record holds four. An activation with one step (Pitch Dark's third) is the same under both. Under (b) the stored format changes, so stored paths made before it lack the numbers; what that costs in re-analysis was not measured for this plan (row P17). Recommended: (a). Nothing the user sees in Hydra changes under either.

2. **A read outside the range the ratio holds for.** The predicate: the read end, as the game stored it, lies past the nearest tempo or time signature change on either side of the planned read end. Then the conversion in the file is not the true one. (a) The press takes its written fallback (a squeeze-in is pressed plain, a squeeze-out's late gem joins its group), a press with no fallback is pressed plain, and the row says so in `retimed`. (b) The run stops with a line naming the read and the range. Examples from both ends: on Pitch Dark the range around chord 466's read runs from about 157,545 to 158,351 ms, and the run's read of 158,268.988 is inside; a read past 158,351 needs the anchor judged about 90 ms late, which is outside its 57 ms window, so it cannot happen there. On a made-up chart with a tempo change 10 ms past the planned read, an anchor judged 8 ms later than planned lands outside. Recommended: (a); it keeps the chord's score and the combo, as the user chose for the two existing fallbacks.

## Out of scope, for the user to approve

- **When the game judges a press.** SP-47 says one frame after the clock the player read. On this run the reads fit two frames better: the read after chord 437 is about 4.2 ms later than one frame gives, the read after chord 144 about 3.4 ms, and the clamp read 4.0 ms, each close to one more frame times that end's ratio. Chord 644 went down 3.8 ms before the end and was not counted. This is not settled and nothing in this plan builds on it. LS-R2 records the data. It needs its own question after that run.
- **A squeeze-in that falls back to plain.** Later presses of that activation still aim at ends that assumed the squeeze. No rule is proposed here.
- **Reading the end again after every step.** It would make every conversion 1. It was dropped: it adds a game read per step, a step can sit close to the press it times, and the planned values would still need the right ratio.

## Prior art

CHOpt's author describes the same ground in "how chopt works" (https://genericmadscientist.github.io/posts/how-chopt-works): Star Power length is counted in measures, so tempo and time signature decide it, and hitting a phrase note early or late moves the window. That page does not give a per-end conversion; CHOpt plans in measure positions and has no live read to correct. Hydra's `transfer_scale_between` is the same idea turned into ms for one end. The search did not reach CHOpt's source, so no claim is made about its code.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| One function gives every step's ratio, and the stored ratios come from it | Design 1 | T1 | The corpus walk: last step equals `transfer_post`, each squeeze-in step equals `transfer`, exactly |
| A step number past the list gives "unset", never a number | T1 criteria | T1 | The out-of-range case in the same test |
| Nothing stored changes | Design 1 | T1 | 'no fresh record stores an unknown transfer scale' passes unedited |
| Every read is priced with its own step's ratio | Design 2 | T2 | Read on 466 at 158,262.53; read on 437 against the T1 function |
| The file carries the own end and the conversion on every press timed from an end | Design 3 | T2 | Chords 644, 676 and 243 pins |
| A press whose read end is its own end does not move | Design 3 | T2, T3 | Freak, Rescued and Ghost pins unedited; retime tests unedited at conversion 1 |
| Thresholds reach read terms through the conversion | Design 4 | T2 | The round trip on chord 644's `if_end_before_ms` |
| The squeeze-out fallback presses before the own end | Design 4 | T3 | The conversion 0.5 case |
| A lead that moves the read end past a tempo change is refused | Design 5 | T2 | The 20 ms and 5 ms hand-built cases |
| A read outside the range does what question 2 decides | Design 5 | T3 | The just-inside and just-outside cases |
| The player moves a press by conversion times read error, never one for one | Design 3 | T3 | Chord 644 moves 5.13 ms on Pitch Dark's numbers |
| An old schedule file is refused by the new player | T3 | T3 | The missing-field case |
| Every "Wrong" row of the sweep is fixed or named | The sweep | T2, T3 | Each row's task; the `squeeze_lines` pin; T3's report on the kit agent |
| The split of a squeeze is unchanged | The sweep | T2 | Chord 644's 18.316 and chord 243's 64.571 and 40.919 |
| Pitch Dark's numbers are pinned in tests | The Pitch Dark fixture | T2, T3 | The literals above; the three-activation first assertion |
| The fixture is added only if no corpus count breaks | The Pitch Dark fixture | T2 | The executor's grep, named in its report; else the hand-cut chart |
| The docs say what the player now does | T2 | T2, T4 | The docs test |
| The clamped end's live result reaches page 05 | T4 | T4 | The SP-40 edit in LS-R2's commit |
| Pitch Dark path 0 scores 393,885 on the kit, or fails on a named line | LS-R2 | LS-R2 | Pass lines 1 to 5 and the four fail lines |
| Every live step has a cap, a stop line and at most one never-tried call; no focus, no `SendInput` | The live steps | LS-R1, LS-R2, LS-R3 | Each step's Calls line and cap sentence |
| Two failed runs put the design to the user | LS-R2 | LS-R2 | The cap sentence |
| Agents build nothing; the main session runs the C++ tests | The tasks | T1 to T4 | Each executor's report lists no build |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: `transfer_scale_between` divides the measure length at the end tick by the measure length at the anchor tick, with an early and a late side | code | `src/core/squeeze_rating.cpp` lines 12 to 22 | checked 2026-11-12 |
| P2: the engine stores a ratio only for the last step and for each squeeze-in's step, both through `frontend_transfer_scales` at copy-out | code | `src/search/engine.cpp` lines 2094 to 2105 and `src/core/squeeze_rating.cpp` lines 24 to 45 | checked 2026-11-12 |
| P3: Pitch Dark path 0's second activation has steps at ticks 178560, 182280 (clamped, end 198570), 194080 (collected, end 202440) and 202480 (squeeze-in, end 206280), with chord 466 as the anchor of the last three | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\pitch-dark-prep\schedule.json` | checked 2026-11-12 |
| P4: Pitch Dark has 5/4 measures from tick 190080 to 192480 and 196320 to 198720 at resolution 480, and its tempo map gives 1.3926146644 from tick 182280 to 198570 and 1.1066660027 to 202440 | chart | `C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\SoundHaven\CHON\CHON - Pitch Dark (B)\notes.mid` | checked 2026-11-12, by dividing measure lengths read from its tempo track |
| P5: the schedule priced the read after chord 466 at 158,254.774 from a stored end of 158,224.757, an anchor lead of 24.124 and a squeeze ratio of 1.1067 | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\pitch-dark-prep\schedule.json` | checked 2026-11-12 |
| P6: the schedule keeps one ratio per anchor from the first squeeze on it and prices the read with it; the plain branch uses `transfer_post` | code | `tools/replay_schedule.cpp` lines 684 to 692 and 753 to 764 | checked 2026-11-12 |
| P7: `in_read_terms` adds a shift to the read's stored end, and the thresholds are moved by an added offset | code | `tools/replay_schedule.cpp` lines 822 to 826, 864 to 871, 920 and 934 | checked 2026-11-12 |
| P8: the player moves a timed press by the read minus `aim_end_ms`, one for one, and its join-group fallback presses before the read end | code | `tools/ch_probe/experiments/play_chart.py` lines 346 to 365 | checked 2026-11-12 |
| P9: in the run, the read after chord 466 was 158,268.988, chord 644 was pressed at clock 161,387.796 and chord 676 at 164,371.252 | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\pitch-dark-run1\press-1.csv` | checked 2026-11-12 |
| P10: the game counts Star Power in measures, cuts an overfull meter to a full bar measured from the collecting moment, and stores one end time through the tempo map | data | `docs/clone-hero/05-star-power.md` blocks SP-15, SP-24 and SP-27, each "Proven in the game's code" | checked 2026-11-12, labels read; rule text read for SP-24 and SP-27 |
| P11: the game's clamp anchor is the note Hydra measures the scale from | unchecked | `docs/clone-hero/05-star-power.md` SP-40 is labelled "Hydra assumes it, never checked"; the Pitch Dark read fits it within 4 ms | LS-R2 |
| P12: the game judges a press one frame after the clock the player read | unchecked | `docs/clone-hero/05-star-power.md` SP-47 is proven on four activations, but three reads of `pitch-dark-run1` fit two frames | LS-R2 |
| P13: the kit agent does no sum on an end and only takes the entries Python made | unchecked | `tools/ch_mcp/agent/press_schedule.ts` line 78, a comment only; the body was not read | T3 |
| P14: no test walks all of `testdata/input` and pins a count that a new folder changes | unchecked | `testdata/input` was searched for time signature lines only; the tests were not searched | T2 |
| P15: a fresh engine run on Pitch Dark gives, as path 0, the path the run played | unchecked | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\pitch-dark-prep\schedule.json` names `path_source` index 0 of a dump, not a fresh run | T2 |
| P16: Hydra's total for Pitch Dark path 0 is 393,885 and the game scored 383,575 with 1110 of 1111 notes | unchecked | the brief reports it from `score.json` and the scorestats file in the two run folders; neither was opened for this plan | LS-R1 |
| P17: adding a field to a stored step forces stored paths to be made again | unchecked | `src/store/stored_versions.h` was not opened | question 1 |
| P18: the user allowed human runs of Pitch Dark: "Human run, allowed" | ruling | the user's message of 2026-11-12, as the main session quotes it in the brief for `PL-SP-RATIOS`; no file under `docs/agents/rulings/` was opened for it, so the main session confirms the quote in the reviewer's brief | checked 2026-11-12, the brief's quote only |
| P19: `ch_start`, `ch_run_chart` and the watch ran on Pitch Dark with a human kit player | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\pitch-dark-run1` holds the launch log, `builds-1.json`, `press-1.csv` and `sp_watch-1.csv` | checked 2026-11-12, by the folder's file list |
| P20: `ch_play_status`, `ch_sp_watch_status` and `ch_stop` ran in that run | unchecked | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\pitch-dark-run1` holds no file that names them | LS-R1 |
| P21: the equal-room split uses the squeeze's own ratio at its own end | code | `tools/replay_schedule.cpp` lines 204 to 235 and 661 to 673 | checked 2026-11-12 |
| P22: `squeeze_lines` prints the press time against `aim_end_ms` | code | `tools/ch_mcp/game_tools.py` lines 347 to 373 | checked 2026-11-12 |
| P23: the earlier plan left the clamped end's ratio unchecked as its P15, to be settled by its LS-S2 | code | `docs/superpowers/plans/2026-11-08-replay-press-schedule.md` line 340 | checked 2026-11-12 |
