# Plan: play Hydra's 5000% path in Clone Hero

Date: 2026-11-25. Task id: speed-run-plan. Planner session: aa0ce9ac.

## The short answer first

The run tool can press fast enough on paper. On the evidence we have, the run would still lose notes, so it would not match Hydra. This plan therefore proves or disproves the match offline first, and it starts the game only if the offline check shows no lost press.

Here is why, with each number's source.

The tool presses once per game frame. At frame multiplier 16 the frame step is 0.1894 ms (TM-55 on `docs/clone-hero/03-timing.md`, LS-M16 run 2). A pad needs one frame down and one frame to lift before it can go down again (the frame loop in `tools/ch_mcp/agent/press_schedule.ts`, lines 761 to 776 and 854). So one pad can take a press about every 0.38 ms, and different pads can share a frame. The chart at 5000% has a chord every 3.0 ms at the median (the main session's figure, from `C:\Users\Patrick\Downloads\Hydra\hydra-data\disco_score100.json` divided by 50). That is about 16 frames per chord. Speed is not the problem.

The hit window is the problem. The game's hit window runs in real time at every speed, so it does not shrink or grow with the song speed (TM-43, proven in the game's code). At this density every note sits on the window's floor, 48.15 ms a side (TM-22). The tool drops a press when no frame falls inside its window (`press_schedule.ts` lines 846 to 849). So any single frame longer than about 48 ms skips past whole windows. At a chord every 3 ms, a 72 ms frame passes about 24 chords, and roughly 8 of them have a window that opened and closed inside that one frame.

Such frames happen. In the full As Icicles Fall run at multiplier 16, I counted the frame log `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-m16-run2\frame_clock-1.csv`. Of 2,647,130 frame steps, 5 were longer than 48.15 ms and 14 were longer than 20 ms. The longest was 71.83 ms (TM-55). That run still matched Hydra, because at 100% a long frame rarely lands on a note. At 5000% it always lands on several. Nobody knows what causes these frames (TM-55 and IN-96 do not say).

So a 409 s run at 5000% should expect a few long frames, and each one costs notes. You ruled that a run that does not match Hydra is never wanted ("why would i ever want a run that doesnt match hydra?", `docs/sessions/2026-11-23-537dcff7.md` line 16). No step below aims at a run we already expect to miss.

## What the plan does

The work has two halves.

The offline half needs no game. It teaches `hydra_replay` the song speed, so it can write the 5000% path's press schedule. It then replays that schedule through the tool's real frame picker, fed with frame steps recorded from a real run. That tells us how many presses would be dropped, without starting Clone Hero. This half is useful even if the answer is "it cannot match", because it answers your question "can we test the path for this song?" with a number.

The live half starts only if the offline half shows zero dropped presses and zero refusals. It sets the game's song speed, checks the game really runs at 5000%, plays the path and compares the score with Hydra's 38,976,455.

This plan changes the run tool and `hydra_replay`. It changes nothing Hydra decides or shows, so it needs no Owner map.

## Global constraints

The run must not need focus and must not take your keyboard. Every live step uses the kit route only. No step calls `ch_key`, `ch_press` or `ch_play_chart` with the keys route (`docs/agents/clone-hero-mcp.md`, "Safety rules").

No agent builds or runs `hydra_replay`, `hydra_tests` or the game. The main session does those (agent rule 14).

Nothing runs over the song library. Every step names this one song folder.

The song speed has one owner in Hydra: `Settings::speed_percent`, carried by `Settings::reading()` into the parser. `hydra_replay` must use that owner and add no speed arithmetic of its own (agent rule 15).

The schedule file is the only source of press times. The player and the kit agent must not scale, shift or widen any time for the speed.

## The tasks

T1, T2 and T3 touch different files and can run at once. T4 needs T1 merged and built. T5 can start with T1 and finishes after T4's files exist. T6 comes last.

### T1: hydra_replay takes the song speed and Safe paths

Today `hydra_replay` builds an `app::Settings` from its flags and has no flag for the song speed or for Safe paths (`tools/replay.cpp` lines 196 to 242). Your path was found with "Song speed 5000% · Safe paths", so the tool cannot reproduce it. The executor adds two flags, `--speed N` and `--safe-paths`, that set `Settings::speed_percent` and `Settings::safe_paths`. The speed joins the list of fields that `Settings::clamp` pulls into range, as the other number flags do. Nothing else changes: `load_songpath` already takes `s.reading()`, which already carries the speed (`src/app/config.cpp` lines 351 to 356).

The dump, score and schedule files each record the speed and the Safe paths value they were made with, read from the song and the settings, so a later reader can refuse a mismatch. The comment at `tools/replay_schedule.h` line 8 and the refusal text at `tools/ch_probe/experiments/play_chart.py` line 329 both say "at 100% speed"; the executor rewords them to name the schedule's own speed.

Files: `tools/replay.cpp`, `tools/replay_schedule.cpp`, `tools/replay_schedule.h`, `tools/ch_probe/experiments/play_chart.py`, the `hydra_replay` test file that already tests `--efill`, and `docs/development.md` ("Developer tools").

Acceptance criteria, each with its check:

1. A test loads one corpus chart from `testdata/input` through `hydra_replay`'s settings path at `--speed 200` and at no flag. It requires every chord's time at 200 to be the chord's time at 100 divided by the song's `speed_ratio`, read from the song. It fails if the flag does not reach the parser.
2. A test requires `--speed 0` and `--speed abc` to be refused with a message, and a value above the range to land on `kMaxSpeedPercent`, read by name. It fails if the flag skips `Settings::clamp`.
3. A test requires the schedule JSON's recorded speed to equal the song's `speed_percent()`, for a run with the flag and one without. It fails if the file records a typed constant.
4. A test requires `--safe-paths` to set `Settings::safe_paths`, by checking the settings the run built.
5. The player's refusal message names the schedule's recorded speed. A Python test feeds a schedule with one infeasible squeeze and a recorded speed of 200 and requires "200%" in the message and no "100%".

### T2: ch_start sets the game's song speed

`ch_start` writes two lines of `settings.ini` and has no speed argument (`tools/ch_mcp/game_tools.py` lines 858 to 911). The game's setting is named `song_speed`, default 100, limits 10 to 5000 (`docs/clone-hero/code/6142/settings_load_374D50.asm` lines 2017 to 2027). Your `settings.ini` holds no such line today: a search of the file for "speed" finds only `pitched_speedups`, `lowsongspeed` and `maxsongspeed`. The one writer, `set_game_setting`, refuses a key with no line (`tools/ch_mcp/game_files.py` lines 298 to 310). So this task rests on premise P9, which is unchecked, and on your answer to question 2.

The executor adds an optional `song_speed` argument to `ch_start`. It follows the frame multiplier's pattern exactly: a refusal function that runs before any backup or launch and changes no file, a setter that says the old and new value and "(not put back after the run)", and the value recorded in the frame settings file. A value outside the game's limits is refused. Left out, the line is not touched. If question 2 is answered "add the line", `game_files.py` gains one function that adds a `key = value` line to the `[game]` section when it has none, and `ch_start` says in its reply that it added the line.

Files: `tools/ch_mcp/game_files.py`, `tools/ch_mcp/game_tools.py`, `tools/ch_mcp/server.py`, `tools/ch_mcp/tests/test_game_tools.py`, `tools/ch_mcp/tests/test_bot_run.py`, `docs/agents/clone-hero-mcp.md`.

Acceptance criteria:

1. A test on a temp `settings.ini` requires `ch_start(song_speed=5000)` to leave the file reading `song_speed = 5000` in `[game]`, with every other byte unchanged. It fails if another line moves.
2. A test requires speeds below the minimum and above the maximum to be refused before the backup, with the file's bytes unchanged. The limits are read by name from `game_files.py`.
3. A test requires a start with no `song_speed` to leave a present `song_speed` line untouched.
4. A test requires the start reply to hold the old value, the new value and "(not put back after the run)".
5. A test requires the frame settings file to hold the speed before and after.

### T3: a schedule made at one speed never plays in a game at another

`ch_run_chart` already refuses a schedule made from another path (`path_index_refusal`, named in `docs/agents/clone-hero-mcp.md`). Nothing ties the schedule's speed to the game's. A 100% schedule played into a 5000% game would miss every note. The executor adds one refusal beside `path_index_refusal`: the schedule's recorded speed (T1) must equal the speed this session's `ch_start` set, and a schedule with no recorded speed is treated as the speed `hydra_replay` wrote before T1, which the refusal reads from one named constant. `ch_compare` already checks the game's `playback_speed` against the run's (`tools/ch_mcp/compare_scorestats.py` lines 108 to 116); it compares two integers, so 5000 needs no change there.

Files: `tools/ch_mcp/game_tools.py`, `tools/ch_mcp/tests/test_game_tools.py`, `docs/agents/clone-hero-mcp.md`.

Acceptance criteria:

1. A test requires `ch_run_chart` and `ch_play_chart` to refuse, before the attach, a schedule recorded at 5000 when the start set no speed, and one recorded at 100 when the start set 5000. It fails if either direction passes.
2. A test requires a matching pair to pass the check.
3. A test feeds `ch_compare` a real saved stats file with `playback_speed` given as 5000 and requires the settings line to FAIL when the file says another speed. It fails if 5000 is special-cased.

### T4: the main session makes the 5000% files and checks the path against the GUI

This is the main session's own step, because agents never run `hydra_replay`. After T1 is merged and built, it runs `hydra_replay dump`, `score` and `schedule` on the song folder with `--speed 5000 --safe-paths`, Pro Drums Expert, into `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-25\speed-run\`.

Acceptance criteria:

1. The score file's total equals 38,976,455 and its activation count equals 302, the values Hydra's screen showed you. A mismatch is stop S1.
2. The schedule's `refusals` list is empty and no squeeze has `feasible` false. Either one is stop S2, reported with the schedule's own words.
3. The schedule's recorded speed is 5000.

This settles premises P10 and P11.

### T5: an offline dry run of the frame picker on the 5000% schedule

This is the check that decides whether the game is started at all. The kit agent's frame picker is plain logic: `PressSchedule.step(frame, clockMs, wallMs)` takes a frame and returns what to write (`tools/ch_mcp/agent/press_schedule.ts` lines 706 to 895). The executor first searches `tools/ch_mcp/agent` and its tests for an existing harness that drives `step` from a list of frames, and uses it if one exists. Otherwise it writes a small script that loads a schedule the way the play message carries it, feeds `step` one frame at a time, and counts each outcome the picker itself reports: sent, dropped, and `no_frame_in_band`. It works out no window and no drop rule of its own; it only counts what `step` returns.

The frames come from a real run. The script takes a frame clock log and replays its frame steps, starting at the schedule's first press. The log to use is LS-M16 run 2's `frame_clock-1.csv`, which holds the 5 frames over 48.15 ms. The script also runs once on a perfectly even 0.1894 ms step, which shows what the chart costs with no long frame at all.

The script also reports the largest number of presses the picker looked at in one frame (`examined`, line 708). That is the check for the run-size worry: IN-96 says the agent's per-frame work once grew with the presses waiting, and this shows whether it still does with 99,163 chords.

Files: one new script under `tools/ch_mcp/agent/` or its tests folder, one test file beside it, `docs/agents/clone-hero-mcp.md`.

Acceptance criteria:

1. A test feeds the script a three-press schedule and an even frame list and requires three sent, none dropped. A second test puts one frame longer than a press's window across that press and requires exactly that press dropped. It fails if the script counts by its own rule instead of the picker's outcome.
2. The script's report for the 5000% schedule states, for each of the two frame lists: presses sent, presses dropped, presses marked `no_frame_in_band`, and the largest `examined`.
3. Any dropped press, or any `no_frame_in_band` press, on either frame list is stop S3. The live steps do not start.
4. A largest `examined` that grows with the schedule's length (compared between the three-press test and the full schedule) is stop S4.

Waits for: T4's schedule file.

### T6: write the results back

The executor adds what the steps proved to `docs/clone-hero/03-timing.md` (TM-43 and TM-44 gain their first live result, or the offline result that stopped the run) and to `docs/agents/clone-hero-mcp.md`. This plan file leaves the repo in the same merge.

Acceptance criteria:

1. Each new or changed block has its seven lines and one of the four labels, and cites a run folder or a file in the repo.
2. `py -I tools/ch_mcp/dump_facts.py --check` passes, run by the main session.

## The live steps

No live step starts unless T5 reported zero dropped and zero `no_frame_in_band` presses on both frame lists, and you have said go to that step's own message. Each step runs at most twice. A second failure stops the plan and goes to you with the design question, not a third try.

### LS1: the game starts with the speed line set

Calls: `ch_start` (never tried), `ch_stop` (seen: LS-M16 run 2)

`ch_start` gets the song folder, `human=True`, `player="ProDrums,Expert"`, `frame_multiplier=16` and `song_speed=5000`. Pass: the reply shows the speed line set, the launch agent says patched, and `ch_stop` prints every check "ok" and the registry line "restored" or "unchanged". Fail: any refusal or failed check; report its words. Uses P9 and P12.

### LS2: the game really runs at 5000%

Calls: `ch_start` (seen: LS1's passing run), `ch_attach` (seen: LS-M16 run 2), `ch_state` (never tried), `ch_stop` (seen: LS-M16 run 2)

Pass: the game's own state shows the song at 5000%. Which read shows it is premise P13, unchecked; T2's executor names the read or reports that none exists, and this step does not run until it is named. Fail: the game reads 100%, which means the `settings.ini` line does not set the speed. That is stop S5: the speed must then be set another way (the game's own setter, TM-43), and that design goes to you first. Uses P9 and P13.

### LS3: the full run

Calls: `ch_start` (seen: LS1's passing run), `ch_run_chart` (never tried), `ch_play_status` (seen: LS-M16 run 2), `ch_sp_watch_status` (seen: LS-M16 run 2), `ch_wait_stats` (seen: LS-M16 run 2), `ch_compare` (seen: LS-M16 run 2), `ch_sp_compare` (seen: LS-M16 run 2), `ch_stop` (seen: LS-M16 run 2)

`ch_run_chart` gets T4's score and schedule files and `kit_player="System"`. `ch_compare` gets `playback_speed=5000` with the other three settings. Pass: `ch_compare` passes every line, with the game's score equal to Hydra's 38,976,455; `ch_play_status` shows no DROPPED row; `ch_sp_compare` passes; `ch_stop` restores cleanly. Fail: anything else. A fail is reported with the frame clock log's count of frames longer than 48.15 ms, so a miss is tied to its frame. Uses P12 and P14. The seen markers here rest on P14, which T2's executor checks against the run folder before the go message; a call that did not run there is re-marked and the step split.

## The stops

S1: `hydra_replay` at 5000% Safe paths does not give the screen's score or activation count. Report both numbers; nothing live runs.

S2: the schedule refuses the path or holds a squeeze the game cannot make. Report the schedule's words; nothing live runs.

S3: the dry run drops a press. Report the counts for both frame lists. If the even list is clean and the recorded list is not, the cause is the long frames, and the next question is what makes them, which nobody has read.

S4: the picker's per-frame work grows with the schedule. Report it; it goes to a fresh plan.

S5: the settings line does not set the speed.

## The bot-style route (added at the user's request, "Yes, add it")

This section was added late and is a first sketch, not a finished design. It names the reads that must come first. No task builds on it until they are done.

The idea in plain words: the game's own bot never presses a pad. Each frame, `BasePlayer.Update` asks a method for the next chart note whose time has come, and hands that note's own time to the engine as a hit (IN-69, proven in the game's code). So a bot plays many notes inside one frame, each at its exact time, and a long frame costs nothing. That removes the one thing that sinks the frame route: the 5 frames over 48.15 ms in P4. But a Bot-kind player gains no Star Power (SP-44), and Hydra's path has 302 activations. So the route would keep the player kind Player and feed our schedule's judge times through that same loop.

I recommend this route be read first, and the frame route kept as the fallback. The reason is the numbers at the top: the frame route's expected result is a miss on today's evidence, and the bot-style route is the only idea on the table that does not depend on frame length. T1, T3 and T4 serve both routes, so they lose nothing. T2 serves both. T5's dry run is still worth its small cost, because it says whether the fallback exists at all.

Four things are unknown, and each is a read before any build. The main session or an `rea` read does them; nobody guesses.

### R1: does the engine update hit one pad or the whole chord?

Read the engine update at RVA 0x20F4550 on the path `BasePlayer.Update` takes at 0x20A425. The question: when it is called with a time as a hit, which notes does it mark hit, and can the caller choose pads? We need per-pad control for fills the path passes over (`kept_gems`), for squeeze order groups, and for cymbal against tom. Result: a new block on page 07. If the answer is "the whole chord, no choice", stop SB1.

### R2: does a hit fed this way earn Star Power for a Player-kind player, and how is activation sent?

Read the Star Power gain method (RVA 0x20F4FA0, SP-44) for what it needs besides the kind, and find where the fed hit reaches it. Then read how an activation is asked for on this path, since no pad is pressed. If activation needs a real pad press, the route still needs the kit for those 302 presses, and the result says so.

### R3: how does the Replay kind feed its inputs?

IN-64 says a profile holding a replay (its +0xE0) gets the Replay kind. Read what the replay object holds and how `BasePlayer.Update` reads it. The question: could our schedule be written as a replay, with no hook at all? Also read whether the gain method lets a Replay kind earn Star Power; SP-44 names only Bot and RemotePlayer as blocked.

### R4: what would a hook on 0x208F50 change?

Write down exactly what the hook would do: replace the method's return value and its out time for a Player-kind player. Check it against the allowed list in `docs/agents/clone-hero-mcp.md` ("Safety rules"), which allows method calls, field writes and trace hooks, and names the launch agent as the only one that changes what the game's own code does. A hook that changes a return value is not on that list, so it is question 4.

Stops: SB1, R1 shows no per-pad control. SB2, R2 shows a fed hit earns no Star Power for a Player. Either one drops this route, and the frame route's T5 result decides whether anything goes live.

After R1 to R4, a second plan designs the feed, with its own tasks, live steps, Promise map and Premise map. This plan does not design it, because every design choice depends on those reads.

## Questions

4. If the reads favour the hook: may an agent hook the game method at RVA 0x208F50 and replace what it returns, in memory only, for the length of a run? Today's allowance covers method calls, field writes and trace hooks, and one launch byte. If yes, the game on disk is still never written, and the run would play notes at their exact times whatever the frame length. If no, the bot-style route is limited to the Replay kind (R3), or dropped.

1. May the offline half (T1 to T5) start? You will see no change in Hydra or the game. You will get the dry run's counts: how many of the path's presses would be dropped on recorded frames and on even frames. If you say no, nothing is built and the answer to "can we test the path" stays "not with today's tool".

2. May `ch_start` add a `song_speed = 5000` line to the `[game]` section of your `settings.ini`? The file has no such line now. If yes, the line stays after the run, as the pause and multiplier lines do, so the game may start your own next session at 5000% until you change the speed in its menu. If no, T2 and the live steps do not run, and the plan ends after T5.

3. If the dry run shows drops only on the recorded long frames, do you want a separate investigation of what causes those frames? You would see a report, no run.

## Out of scope

Finding the cause of the long frames. Changing where presses are aimed to survive a long frame; that would be a new rule for the tool, and no ruling asks for it. Any change to Hydra's own path search or screens.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| The run needs no focus and takes no keyboard | Global constraints, paragraph 1 | LS1, LS2, LS3 | every `Calls:` line holds kit-route calls only; `ch_stop` prints the foreground count |
| No agent builds or runs hydra_replay or the game | Global constraints, paragraph 2 | T4 | T4 is the main session's step |
| Nothing runs over the library | Global constraints, paragraph 3 | T4, LS1 to LS3 | each names the one song folder |
| hydra_replay uses the one speed owner | Global constraints, paragraph 4 | T1 | T1 criterion 1 |
| The player scales no time for the speed | Global constraints, paragraph 5 | T5 | T5 criterion 1, second test |
| `--speed` reaches the parser | T1 | T1 | criterion 1 |
| A bad speed is refused or clamped | T1 | T1 | criterion 2 |
| Files record their speed and Safe paths | T1 | T1 | criteria 3 and 4 |
| The refusal names the schedule's speed | T1 | T1 | criterion 5 |
| ch_start sets only the speed line | T2 | T2 | criterion 1 |
| An out-of-range speed changes no file | T2 | T2 | criterion 2 |
| Left out, the line is untouched | T2 | T2 | criterion 3 |
| The reply says old, new, not put back | T2 | T2 | criterion 4 |
| The frame settings file holds the speed | T2 | T2 | criterion 5 |
| A schedule at another speed is refused | T3 | T3 | criteria 1 and 2 |
| ch_compare checks 5000 like any speed | T3 | T3 | criterion 3 |
| The 5000% files match the screen | T4 | T4 | criterion 1, stop S1 |
| No refusal or infeasible squeeze goes live | T4 | T4 | criterion 2, stop S2 |
| The dry run counts the picker's own outcomes | T5 | T5 | criterion 1 |
| No live step starts after a dropped press | T5, "The live steps" | T5 | criterion 3, stop S3 |
| The picker's work does not grow with the schedule | T5 | T5 | criterion 4, stop S4 |
| Each live step runs at most twice | "The live steps" | LS1 to LS3 | the main session's run count in the result folder |
| The game is confirmed at 5000% before the full run | LS2 | LS2 | LS2's pass line, stop S5 |
| The game's score equals Hydra's | LS3 | LS3 | `ch_compare` passes every line |
| Results reach the Clone Hero pages | T6 | T6 | criteria 1 and 2 |
| No build rests on the bot-style route before its reads | "The bot-style route", paragraph 1 | R1 to R4 | each read ends in a labelled block on page 07 or 05; this plan holds no build task for the route |
| A route with no per-pad control or no Star Power is dropped | "The bot-style route", Stops | R1, R2 | stops SB1 and SB2 |
| A return-value hook waits for the user's yes | "The bot-style route", R4 | R4 | question 4 |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The game's hit window runs in real time at every speed, 48.15 to 85.72 ms a side. | data | `TM-43` and `TM-22` on docs/clone-hero/03-timing.md, both "Proven in the game's code" | checked 2026-11-25 |
| P2: At 5000% the chart's times are the file's times divided by 50. | data | `TM-56` on docs/clone-hero/03-timing.md, "Proven in the game's code" | checked 2026-11-25 |
| P3: The kit agent drops a press whose window has passed, and defers a press whose pad is held or lifting. | code | `tools/ch_mcp/agent/press_schedule.ts:761-776` and `tools/ch_mcp/agent/press_schedule.ts:846-856` | checked 2026-11-25 |
| P4: In LS-M16 run 2, 5 of 2,647,130 frame steps were over 48.15 ms and 14 over 20 ms. | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-m16-run2` (its `frame_clock-1.csv`, counted by the planner) | checked 2026-11-25 |
| P5: At multiplier 16 a full kit run held a median frame step of 0.1894 ms. | data | `TM-55` on docs/clone-hero/03-timing.md, "Proven in a live run" | checked 2026-11-25 |
| P6: hydra_replay builds its settings with no speed and no Safe paths flag. | code | `tools/replay.cpp:196-242` | checked 2026-11-25; the flag list at lines 835 to 854 was seen by search only |
| P7: `Settings::reading()` carries `speed_percent` to the parser, and `set_speed_percent` is the one place the percent becomes the ratio. | code | `src/app/config.cpp:351-356` and `src/core/song.h:205-218` | checked 2026-11-25 |
| P8: The game's setting is named `song_speed`, default 100, limits 10 to 5000. | code | `docs/clone-hero/code/6142/settings_load_374D50.asm:2017-2027` | checked 2026-11-25 |
| P9: A `song_speed` line in the `[game]` section of settings.ini makes the game load a song at that speed. | unchecked | `C:\Users\Patrick\Documents\Clone Hero\settings.ini` holds no `song_speed` line (searched for "speed"); the listing in P8 does not show the section | LS2; blocks nothing (T2 is built either way; LS3 waits on LS2) |
| P10: hydra_replay at `--speed 5000 --safe-paths` reproduces the screen's 38,976,455 and 302 activations. | unchecked | `tools/replay.cpp:196-242` shows only that the settings path is shared | T4 |
| P11: The 5000% path's schedule holds no refusal and no infeasible squeeze. | unchecked | `tools/replay_schedule.cpp:314-361` is the refusal rule; nobody has run it at 5000% | T4 |
| P12: `set_game_setting` refuses a key that has no line, and `ch_start` follows a refuse-then-set pattern for the multiplier. | code | `tools/ch_mcp/game_files.py:298-310` and `tools/ch_mcp/game_tools.py:872-911` | checked 2026-11-25 |
| P13: Some read the tools already have shows the game's song speed. | unchecked | `docs/agents/clone-hero-mcp.md:48` names `ch_state` but not what it prints | T2 |
| P14: Each call marked "seen: LS-M16 run 2" ran in that run. | unchecked | `docs/sessions/2026-11-23-537dcff7.md` reports the run; its folder's replies were not opened | T2 |
| P15: The chart at 5000% has 99,163 chords in 409 s, median gap 3.0 ms, minimum 0.117 ms. | unchecked | `C:\Users\Patrick\Downloads\Hydra\hydra-data\disco_score100.json` as the main session reported it; the planner did not open it | T4 |
| P16: A press inside two unhit same-pad windows goes to the earlier note, and the schedule refuses a press that would land that way. | code | `tools/replay_schedule.cpp:314-361` | checked 2026-11-25; the game side is `TM-50` |
| P17: `ch_compare` compares the game's `playback_speed` with the run's as two integers. | code | `tools/ch_mcp/compare_scorestats.py:99-117` | checked 2026-11-25 |
| P18: The user never wants a run that does not match Hydra. | ruling | `docs/sessions/2026-11-23-537dcff7.md` line 16: "why would i ever want a run that doesnt match hydra?" | checked 2026-11-25 |
| P19: The player refuses a schedule with an infeasible squeeze, with text that says "at 100% speed". | code | `tools/ch_probe/experiments/play_chart.py:320-330` | checked 2026-11-25 |
| P21: A bot is fed the next due chart note and the loop hands that note's time to the engine update as a hit. | data | `IN-69` on docs/clone-hero/07-internals.md, "Proven in the game's code" | checked 2026-11-25 |
| P22: A Bot or RemotePlayer kind gains no Star Power; a profile holding a replay gets the Replay kind. | data | `SP-44` on docs/clone-hero/05-star-power.md and `IN-64` on docs/clone-hero/07-internals.md, both "Proven in the game's code" | checked 2026-11-25 |
| P23: The engine update called with a time can hit chosen pads, not only a whole chord. | unchecked | `IN-69` says only that the note's time is passed; RVA 0x20F4550 was not read for this | R1 |
| P24: A hit fed through the loop earns Star Power for a Player kind, and an activation can be sent on that path. | unchecked | `SP-44` covers the Bot kind only | R2 |
| P25: The Replay kind plays back timed inputs our schedule could be written as. | unchecked | `IN-64` names the kind; the replay object was not read | R3 |
| P26: A hook that replaces a method's return value is outside today's allowance. | unchecked | `docs/agents/clone-hero-mcp.md:35` lists what agents may do and does not name it | question 4 |
| P20: The schedule header says the per-note window is at 100% speed. | code | `tools/replay_schedule.h:8` | checked 2026-11-25 |
