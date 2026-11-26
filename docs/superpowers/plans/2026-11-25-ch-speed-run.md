# Plan: play Hydra's 5000% path in Clone Hero

Date: 2026-11-25. Task id: speed-run-plan. Planner session: aa0ce9ac.

## The short answer first

We do not know yet whether today's settings can play this chart, so the plan finds out offline before it starts the game.

Today's `C:\Users\Patrick\Documents\Clone Hero\settings.ini` holds framerate 330, frame multiplier 16 and vsync off. The game aims for framerate times multiplier frames a second (TM-55 on `docs/clone-hero/03-timing.md`): 5,280 a second, a frame step of 0.1894 ms. You ruled that framerate is a setting you can change ("framerate is not permanently set, it can be changed in the settings", 2026-11-11, quoted on SP-28 on `docs/clone-hero/05-star-power.md`). So 330 and 16 are today's values here, not limits. The game's own limit on framerate was never read (P27). This plan reads it only if today's settings fail the offline check.

Here is the chart's load, with each number's source. At 5000% the chart has 99,163 chords in 409.2 s (`C:\Users\Patrick\Downloads\Hydra\hydra-data\disco_score100.json`, each time divided by 50 for TM-56; the reviewer recounted all of it from the file). The median gap between chords is 3.0 ms, about 16 frames. The smallest gap is 0.117 ms, between two different pads, and different pads can share a frame. The limit that matters is the same pad: a pad needs one frame down and one frame to lift before it can go down again (`tools/ch_mcp/agent/press_schedule.ts` lines 761 to 776). The smallest same-pad gap is 0.838 ms (Red tom, m7632.3.359 to m7632.3.419, and the same pair in other bars). No same-pad gap is under 0.5 ms, and 70 are under 1 ms. Two frames at 0.1894 ms are 0.379 ms, so even frames fit every same-pad gap.

The risk is the long frames. The full As Icicles Fall run at multiplier 16 logged its frames in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-m16-run2\frame_clock-1.csv`: 2,647,130 frames, so 2,647,129 steps. The planner counted 5 steps longer than 48.15 ms and 14 longer than 20 ms, and the longest is 71.83 ms (TM-55). Nobody knows what causes them.

What a long frame costs is not known, and an earlier draft of this plan guessed. The game's hit window runs in real time at every speed (TM-43). At this density each side is the 48.15 ms floor, so the window is 96.3 ms wide (TM-22). The schedule aims an ordinary press at its chord's whole window (`window_band` in `tools/replay_schedule.h` line 124). The picker drops a press only when a frame lands past the window's end (`press_schedule.ts` lines 850 to 852), and the longest frame seen, 71.83 ms, is shorter than 96.3 ms. But the picker presses each pad once per frame, so a long frame holds back every later chord on that pad. A squeeze press has a narrow band of its own that a long frame can skip (`no_frame_in_band`, lines 846 to 849). Whether that loses a note or only judges it late is what the dry run (T5) counts. You ruled that a run that does not match Hydra is never wanted ("why would i ever want a run that doesnt match hydra?", `docs/sessions/2026-11-23-537dcff7.md` line 16). So the game starts only if the dry run is clean.

## What the plan does

You said: "if our existing settings can handle this, i want to try that first". So the plan goes first by the frame route at today's settings, and its first check is whether they can handle this chart.

The offline half needs no game. It teaches `hydra_replay` the song speed, so it can write the 5000% path's press schedule (T1, T4). It then replays that schedule through the tool's real frame picker, fed with frame steps from a real run (T5). That counts how many presses would be dropped or skipped, without starting Clone Hero. It answers "can we test the path for this song?" with a number, even if the number says no.

If the dry run is not clean, the fallbacks come in this order. First, read the game's limit on framerate (T6), and ask your leave to let `ch_start` set it (question 2). Second, the bot route, which a separate planner owns in `docs/superpowers/plans/2026-11-25-ch-bot-route.md` on branch `claude/bot-route-plan`. This plan does not design it.

The live half starts only if the dry run shows no dropped press and no skipped band. It sets the game's song speed, checks the game really runs at 5000%, plays the path and compares the score with Hydra's 38,976,455.

This plan changes the run tool and `hydra_replay`. It changes nothing Hydra decides or shows, so it needs no Owner map.

## Global constraints

The run must not need focus and must not take your keyboard. Every live step uses the kit route only. No step calls `ch_key`, `ch_press` or `ch_play_chart` with the keys route (`docs/agents/clone-hero-mcp.md`, "Safety rules").

No agent builds or runs `hydra_replay`, `hydra_tests` or the game. The main session does those (agent rule 14).

Nothing runs over the song library. Every step names this one song folder.

The song speed has one owner in Hydra: `Settings::speed_percent`, carried by `Settings::reading()` into the parser. `hydra_replay` must use that owner and add no speed arithmetic of its own (agent rule 15).

The schedule file is the only source of press times. The player and the kit agent must not scale, shift or widen any time for the speed.

No step treats framerate 330, multiplier 16 or 330 times 25 as a limit. Each is a setting's current value; the limits are read from the game's code (P8, P27) or from `FRAME_MULTIPLIER_MAX` in `tools/ch_mcp/game_files.py`.

## The tasks

T1, T2 and T3 touch different files and can run at once. T4 needs T1 merged and built. T5 can start with T1 and finishes after T4's files exist. T6 runs only if T5 reports a drop. T7 comes last.

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

### T2: ch_start sets the game's song speed, and ch_state shows it

`ch_start` writes two lines of `settings.ini` and has no speed argument (`tools/ch_mcp/game_tools.py` lines 858 to 911). The game's setting is named `song_speed`, default 100, limits 10 to 5000 in steps of 5 (`docs/clone-hero/code/6142/settings_load_374D50.asm` lines 2017 to 2027). Your `settings.ini` holds no `song_speed` line: a search for "speed" finds only `pitched_speedups`, `lowsongspeed = 50` and `maxsongspeed = 250`. The one writer, `set_game_setting`, refuses a key with no line (`tools/ch_mcp/game_files.py` lines 298 to 310). So this task rests on P9 and P28, which are unchecked, and on your answer to question 1.

The executor adds an optional `song_speed` argument to `ch_start`. It follows the frame multiplier's pattern: a refusal function that runs before any backup or launch and changes no file, a setter that says the old and new value and "(not put back after the run)", and the value recorded in the frame settings file. A value outside the game's limits is refused. Left out, the line is not touched. If question 1 is answered yes, `game_files.py` gains one function that adds a `key = value` line to the `[game]` section when it has none, and `ch_start` says in its reply that it added the line.

The game must also be seen at 5000%. No tool prints the song speed today: a search of `tools/ch_mcp` for `practiceSpeed` finds nothing (P13). `GameManager.practiceSpeed`, at +0xD8 (IN-01 on `docs/clone-hero/07-internals.md`, line 15), is set to the speed ratio when the game wakes (TM-57), so reading it shows the speed. The executor reads how `_field_lines` in `tools/ch_mcp/frida_tools.py` and the agent's field list are built, and adds `practiceSpeed` to what `ch_state` prints.

Files: `tools/ch_mcp/game_files.py`, `tools/ch_mcp/game_tools.py`, `tools/ch_mcp/server.py`, `tools/ch_mcp/frida_tools.py`, the agent source that reads the fields, `tools/ch_mcp/tests/test_game_tools.py`, `tools/ch_mcp/tests/test_bot_run.py`, `docs/agents/clone-hero-mcp.md`.

Acceptance criteria:

1. A test on a temp `settings.ini` requires `ch_start(song_speed=5000)` to leave the file reading `song_speed = 5000` in `[game]`, with every other byte unchanged. It fails if another line moves.
2. A test requires speeds below the minimum and above the maximum to be refused before the backup, with the file's bytes unchanged. The limits are read by name from `game_files.py`.
3. A test requires a start with no `song_speed` to leave a present `song_speed` line untouched.
4. A test requires the start reply to hold the old value, the new value and "(not put back after the run)".
5. A test requires the frame settings file to hold the speed before and after.
6. A test feeds `ch_state` a saved field read that holds `practiceSpeed` and requires its line to name the value. It fails if the field is dropped from the print.

### T3: a schedule made at one speed never plays in a game at another

`ch_run_chart` already refuses a schedule made from another path (`path_index_refusal`, named in `docs/agents/clone-hero-mcp.md`). Nothing ties the schedule's speed to the game's. A 100% schedule played into a 5000% game would miss every note. The executor adds one refusal beside `path_index_refusal`: the schedule's recorded speed (T1) must equal the speed this session's `ch_start` set, and a schedule with no recorded speed counts as the speed `hydra_replay` wrote before T1, which the refusal reads from one named constant. `ch_compare` already checks the game's `playback_speed` against the run's (`tools/ch_mcp/compare_scorestats.py` lines 108 to 116); it compares two integers, so 5000 needs no change there.

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
4. The schedule's chord count is 99,163 and its smallest same-pad gap is at least 0.838 ms, the figures this plan was written from. A different count or gap means the plan's load figures are wrong and goes back to the planner.

This settles P10 and P11.

### T5: an offline dry run of the frame picker on the 5000% schedule

This is the check that decides whether the game is started at all, and it answers your condition: can today's settings handle this chart. The kit agent's frame picker is plain logic: `PressSchedule.step(frame, clockMs, wallMs)` takes a frame and returns what to write (`tools/ch_mcp/agent/press_schedule.ts` lines 706 to 895). The executor first searches `tools/ch_mcp/agent` and its tests for an existing harness that drives `step` from a list of frames, and uses it if one exists. Otherwise it writes a small script that loads a schedule the way the play message carries it, feeds `step` one frame at a time, and counts each outcome the picker itself reports: sent, dropped, and `no_frame_in_band`. It works out no window and no drop rule of its own.

The frames come from a real run at today's settings. The script takes a frame clock log and replays its frame steps from the schedule's first press, looping the log if the schedule is longer than it. The log is LS-M16 run 2's `frame_clock-1.csv`, which holds the 5 steps over 48.15 ms. The script also runs on a perfectly even 0.1894 ms step, which shows what the chart costs with no long frame at all. Before it runs, the executor reads `settings.ini` and requires framerate 330 and multiplier 16, so the log and today's settings agree.

The script also reports the largest number of presses the picker looked at in one frame (`examined`, line 708). That checks the run-size worry: IN-96 says the agent's per-frame work once grew with the presses waiting, and this shows whether it still does with 99,163 chords.

Files: one new script under `tools/ch_mcp/agent/` or its tests folder, one test file beside it, `docs/agents/clone-hero-mcp.md`.

Acceptance criteria:

1. A test feeds the script a three-press schedule and an even frame list and requires three sent, none dropped. A second test puts one frame longer than a press's whole window across that press and requires exactly that press dropped. It fails if the script counts by its own rule instead of the picker's outcome.
2. The script's report for the 5000% schedule states, for each of the two frame lists: presses sent, presses dropped, presses marked `no_frame_in_band`, and the largest `examined`.
3. Any dropped press, or any `no_frame_in_band` press, on either frame list is stop S3. The live steps do not start.
4. A largest `examined` that grows with the schedule's length (compared between the three-press test and the full schedule) is stop S4.

Waits for: T4's schedule file.

### T6: read the game's limit on framerate (only if T5 stops at S3)

This is a read of the game's code, with no game running. Where the game turns `framerate` and `simulated_frame_multiplier` into its target frame rate was never read (TM-55 says so). The executor finds that place with the dump and `tools/ch_mcp/chdis.py`, and writes a block on `docs/clone-hero/03-timing.md` with the setting's minimum, maximum and any cap on the product. If the read shows a higher rate is possible, the follow-up is a new plan that adds a `framerate` argument to `ch_start` (today `ch_start` writes only the multiplier and the pause line, `tools/ch_mcp/game_files.py`) and reruns T5 with a frame log taken at that rate. This plan builds no such argument.

Acceptance criteria:

1. The new block holds its seven lines and one of the four labels, and cites the listing and RVA it read. It names the limit, or says none was found.
2. `py -I tools/ch_mcp/dump_facts.py --check` passes, run by the main session.

Settles P27.

### T7: write the results back

The executor adds what the steps proved to `docs/clone-hero/03-timing.md` (TM-43 and TM-44 gain their first live result, or the offline result that stopped the run) and to `docs/agents/clone-hero-mcp.md`. This plan file leaves the repo in the same merge.

Acceptance criteria:

1. Each new or changed block has its seven lines and one of the four labels, and cites a run folder or a file in the repo.
2. `py -I tools/ch_mcp/dump_facts.py --check` passes, run by the main session.

## The live steps

No live step starts unless T5 reported zero dropped and zero `no_frame_in_band` presses on both frame lists, and you have said go to that step's own message. Each step runs at most twice. A second failure stops the plan and goes to you with the design question, not a third try.

### LS0a: the compare tool on a saved run

Calls: `ch_compare` (never tried)

No game runs. The main session calls `ch_compare` on LS-M16 run 2's `scorestats-*.json` with `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-m16-prep\score.json` and that run's four settings. Pass: the reply is a verdict per line, not a refusal. It exists because no folder shows `ch_compare` ever ran (P14). Uses P14.

### LS0b: the Star Power compare tool on a saved run

Calls: `ch_sp_compare` (never tried)

No game runs. The main session calls `ch_sp_compare` on LS-M16 run 2's `sp_watch-1.csv`, with `ls-m16-prep\score.json` and `ls-m16-prep\schedule.json` from the same folder tree. Pass: a verdict per line, not a refusal. Uses P14.

### LS1: the game starts with the speed line set

Calls: `ch_start` (never tried), `ch_stop` (seen: LS-P2 run 3)

`ch_start` gets the song folder, `human=True`, `player="ProDrums,Expert"`, `frame_multiplier=16` and `song_speed=5000`. Pass: the reply shows the speed line set, the launch agent says patched, and `ch_stop` prints every check "ok" and the registry line "restored" or "unchanged". Fail: any refusal or failed check; report its words. Uses P9, P12 and P28.

### LS2: the game really runs at 5000%

Calls: `ch_start` (seen: LS1's passing run), `ch_attach` (seen: LS-P2 run 3), `ch_state` (never tried), `ch_stop` (seen: LS-P2 run 3)

Pass: `ch_state` prints `practiceSpeed` as 50, the ratio for 5000% (TM-57, TM-56). Fail: it prints 1 (100%) or another value, which means the `settings.ini` line does not set the speed, or `maxsongspeed = 250` caps it (P28). That is stop S5: the speed must then be set another way (the game's own setter, TM-43), and that design goes to you first. Uses P9, P13 and P28.

### LS3: the full run

Calls: `ch_start` (seen: LS1's passing run), `ch_run_chart` (never tried), `ch_wait_stats` (seen: LS-M16 run 2), `ch_compare` (seen: LS0a's passing run), `ch_sp_compare` (seen: LS0b's passing run), `ch_stop` (seen: LS-P2 run 3)

`ch_run_chart` gets T4's score and schedule files and `kit_player="System"`. After it, the main session reads the run's `press-1.csv`: its `send` column shows each press as sent or dropped. `ch_compare` gets `playback_speed=5000` with the other three settings. Pass: `ch_compare` passes every line, with the game's score equal to Hydra's 38,976,455; no row of `press-1.csv` says dropped; `ch_sp_compare` passes; `ch_stop` restores cleanly. Fail: anything else, reported with the frame clock log's count of steps longer than 48.15 ms, so a miss is tied to its frame. Uses P12 and P14.

## The stops

S1: `hydra_replay` at 5000% Safe paths does not give the screen's score or activation count. Report both numbers; nothing live runs.

S2: the schedule refuses the path or holds a squeeze the game cannot make. Report the schedule's words; nothing live runs.

S3: the dry run drops a press or marks one `no_frame_in_band`. Report the counts for both frame lists. If the even list is clean and the recorded list is not, the cause is the long frames. The next steps are T6 and question 2, then the bot route.

S4: the picker's per-frame work grows with the schedule. Report it; it goes to a fresh plan.

S5: the settings line does not set the speed.

## The bot route

A separate planner owns it: `docs/superpowers/plans/2026-11-25-ch-bot-route.md` on branch `claude/bot-route-plan`. It exists because the game's own bot hands each due note's own time to the engine, so a long frame costs it nothing (IN-69 on `docs/clone-hero/07-internals.md`), while a Bot-kind player gains no Star Power (SP-44 on `docs/clone-hero/05-star-power.md`). This plan holds no task for it and does not wait for it. It is the fallback after T5 stops at S3 and the framerate step (T6, question 2) is tried or ruled out. T1, T2, T3 and T4 serve both routes.

## Questions

1. May `ch_start` add a `song_speed = 5000` line to the `[game]` section of your `settings.ini`? The file has no such line now. On 2026-11-13 you chose to leave the tools' one-line edits in place after a run, so the line would stay, as the pause and multiplier lines do. You would see Clone Hero start your own next session at 5000% until you change the speed in its menu. If no, T2's speed argument and the live steps do not run, and the plan ends after T5.

2. Asked only if the dry run fails at today's settings and T6 shows a higher framerate is possible: may `ch_start` set `framerate` in your `settings.ini`, as it sets the multiplier? You would see the game run at a higher frame rate for the run, and `framerate` left at the new value afterwards, as you chose for the other lines on 2026-11-13. If no, the frame route is closed at today's settings and the bot route is next.

## Out of scope

Finding the cause of the long frames. Changing where presses are aimed to survive a long frame; that would be a new rule for the tool, and no ruling asks for it. Any change to Hydra's own path search or screens. The bot route's design, which its own plan holds.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| The run needs no focus and takes no keyboard | Global constraints, paragraph 1 | LS1, LS2, LS3 | every `Calls:` line holds kit-route calls only; `ch_stop` prints the foreground count |
| No agent builds or runs hydra_replay or the game | Global constraints, paragraph 2 | T4 | T4 is the main session's step |
| Nothing runs over the library | Global constraints, paragraph 3 | T4, LS1 to LS3 | each names the one song folder |
| hydra_replay uses the one speed owner | Global constraints, paragraph 4 | T1 | T1 criterion 1 |
| The player scales no time for the speed | Global constraints, paragraph 5 | T5 | T5 criterion 1, second test |
| No step treats 330 or 16 as a limit | Global constraints, paragraph 6 | T2, T6 | T2 criterion 2 reads `FRAME_MULTIPLIER_MAX` by name; T6 reads the game's own limit |
| `--speed` reaches the parser | T1 | T1 | criterion 1 |
| A bad speed is refused or clamped | T1 | T1 | criterion 2 |
| Files record their speed and Safe paths | T1 | T1 | criteria 3 and 4 |
| The refusal names the schedule's speed | T1 | T1 | criterion 5 |
| ch_start sets only the speed line | T2 | T2 | criterion 1 |
| An out-of-range speed changes no file | T2 | T2 | criterion 2 |
| Left out, the line is untouched | T2 | T2 | criterion 3 |
| The reply says old, new, not put back | T2 | T2 | criterion 4 |
| The frame settings file holds the speed | T2 | T2 | criterion 5 |
| The game's speed can be read in a run | T2 | T2 | criterion 6, LS2 |
| A schedule at another speed is refused | T3 | T3 | criteria 1 and 2 |
| ch_compare checks 5000 like any speed | T3 | T3 | criterion 3 |
| The 5000% files match the screen and the load figures | T4 | T4 | criteria 1 and 4, stop S1 |
| No refusal or infeasible squeeze goes live | T4 | T4 | criterion 2, stop S2 |
| Today's settings are tried first | "What the plan does" | T5 | criterion 2 reports on the 0.1894 ms step of today's settings |
| The dry run counts the picker's own outcomes | T5 | T5 | criterion 1 |
| No live step starts after a dropped press | T5, "The live steps" | T5 | criterion 3, stop S3 |
| The picker's work does not grow with the schedule | T5 | T5 | criterion 4, stop S4 |
| A higher framerate is tried only after today's settings fail | "What the plan does" | T6, question 2 | T6 runs only at S3; question 2 asks before any write |
| Each live step runs at most twice | "The live steps" | LS0a to LS3 | the main session's run count in the result folder |
| The game is confirmed at 5000% before the full run | LS2 | LS2 | LS2's pass line, stop S5 |
| The game's score equals Hydra's | LS3 | LS3 | `ch_compare` passes every line |
| Results reach the Clone Hero pages | T7 | T7 | criteria 1 and 2 |
| The bot route is not designed here | "The bot route" | none | the pointer names the other plan and holds no task |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The game's hit window runs in real time at every speed, 48.15 to 85.72 ms a side. | data | `TM-43` and `TM-22` on docs/clone-hero/03-timing.md, both "Proven in the game's code" | checked 2026-11-25 |
| P2: At 5000% the chart's times are the file's times divided by 50. | data | `TM-56` on docs/clone-hero/03-timing.md, "Proven in the game's code" | checked 2026-11-25 |
| P3: The kit agent drops a press whose window has passed, defers a press whose pad is held or lifting, and marks a press `no_frame_in_band` when a frame passes its band. | code | `tools/ch_mcp/agent/press_schedule.ts:761-776` and `tools/ch_mcp/agent/press_schedule.ts:830-856` | checked 2026-11-25 |
| P4: In LS-M16 run 2, 5 of 2,647,129 frame steps were over 48.15 ms, 14 over 20 ms, and the largest was 71.83 ms. | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-m16-run2` (its `frame_clock-1.csv`, recounted by the reviewer) | checked 2026-11-25 |
| P5: At multiplier 16 a full kit run held a median frame step of 0.1894 ms. | data | `TM-55` on docs/clone-hero/03-timing.md, "Proven in a live run" | checked 2026-11-25 |
| P6: hydra_replay builds its settings with no speed and no Safe paths flag. | code | `tools/replay.cpp:196-242` | checked 2026-11-25; the flag list at lines 835 to 854 was seen by search only |
| P7: `Settings::reading()` carries `speed_percent` to the parser, and `set_speed_percent` is the one place the percent becomes the ratio. | code | `src/app/config.cpp:351-356` and `src/core/song.h:205-218` | checked 2026-11-25 |
| P8: The game's setting is named `song_speed`, default 100, limits 10 to 5000, step 5. | code | `docs/clone-hero/code/6142/settings_load_374D50.asm:2017-2027` | checked 2026-11-25 |
| P9: A `song_speed` line in the `[game]` section of settings.ini makes the game load a song at that speed. | unchecked | `C:\Users\Patrick\Documents\Clone Hero\settings.ini` holds no `song_speed` line; `TM-56` shows the song load reads the `song_speed` setting but not that the ini line is its source | LS2; blocks nothing (T2 is built either way; LS3 waits on LS2) |
| P10: hydra_replay at `--speed 5000 --safe-paths` reproduces the screen's 38,976,455 and 302 activations. | unchecked | `tools/replay.cpp:196-242` shows only that the settings path is shared | T4 |
| P11: The 5000% path's schedule holds no refusal and no infeasible squeeze. | unchecked | `tools/replay_schedule.cpp:314-361` is the refusal rule; nobody has run it at 5000% | T4 |
| P12: `set_game_setting` refuses a key that has no line, and `ch_start` follows a refuse-then-set pattern for the multiplier. | code | `tools/ch_mcp/game_files.py:298-310` and `tools/ch_mcp/game_tools.py:872-911` | checked 2026-11-25 |
| P13: No tool prints the game's song speed today, and `GameManager.practiceSpeed` (+0xD8) is set to the speed ratio at wake. | data | `IN-01` field list at `docs/clone-hero/07-internals.md:15` and `TM-57` on docs/clone-hero/03-timing.md; a search of `tools/ch_mcp` for `practiceSpeed` found nothing | checked 2026-11-25 |
| P14: `ch_compare`, `ch_sp_compare` and `ch_play_status` ran on LS-M16 run 2. | unchecked | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-m16-run2` holds only `builds-1.json`, `frame-settings.json`, `frame_clock-1.csv`, `press-1.csv`, `scorestats-*.json` and `sp_watch-1.csv`; none of those is a compare or status reply | LS0a, LS0b; blocks LS3 |
| P15: The chart at 5000% has 99,163 chords in 409.2 s, median chord gap 3.0 ms, smallest 0.117 ms between different pads, smallest same-pad gap 0.838 ms (none under 0.5 ms, 70 under 1 ms). | data | `C:\Users\Patrick\Downloads\Hydra\hydra-data\disco_score100.json`, recounted by the reviewer with times divided by 50 and a pad being colour plus cymbal flag | checked 2026-11-25 |
| P16: A press inside two unhit same-pad windows goes to the earlier note, and the schedule refuses a press that would land that way. | code | `tools/replay_schedule.cpp:314-361` | checked 2026-11-25; the game side is `TM-50` |
| P17: `ch_compare` compares the game's `playback_speed` with the run's as two integers. | code | `tools/ch_mcp/compare_scorestats.py:99-117` | checked 2026-11-25 |
| P18: The user never wants a run that does not match Hydra. | ruling | `docs/sessions/2026-11-23-537dcff7.md` line 16: "why would i ever want a run that doesnt match hydra?" | checked 2026-11-25 |
| P19: The player refuses a schedule with an infeasible squeeze, with text that says "at 100% speed". | code | `tools/ch_probe/experiments/play_chart.py:320-330` | checked 2026-11-25 |
| P20: The schedule header says the per-note window is at 100% speed. | code | `tools/replay_schedule.h:8` | checked 2026-11-25 |
| P21: An ordinary press is aimed at its chord's whole window. | code | `tools/replay_schedule.h:123-124` | checked 2026-11-25 |
| P22: Today's settings.ini holds framerate 330, vsync 0, multiplier 16, and `lowsongspeed = 50`, `maxsongspeed = 250`. | code | `C:\Users\Patrick\Documents\Clone Hero\settings.ini:52` and `:89-90` and `:121-122` | checked 2026-11-25 |
| P23: The frame rate target is framerate times the multiplier, and framerate is a setting the user can change. | ruling | `TM-55` on docs/clone-hero/03-timing.md; `docs/sessions/2026-11-12-428cf079.md` (the user's words: "framerate is not permanently set, it can be changed in the settings" is quoted by the main session in the brief) | checked 2026-11-25 |
| P24: The user ruled that the tools' one-line `settings.ini` edits stay after a run. | ruling | `docs/agents/rulings/clone-hero.md:8`, "Leave the line changed" | checked 2026-11-25 |
| P25: The user wants today's settings tried first, and a separate planner owns the bot route. | ruling | the user's words as the main session quoted them in the reviewer's brief: "if our existing settings can handle this, i want to try that first. but have another planner flesh out the bot route in parallel in case we need it" | checked 2026-11-25 |
| P26: A Bot-kind player gains no Star Power, and a bot is fed each due note's own time. | data | `SP-44` on docs/clone-hero/05-star-power.md and `IN-69` on docs/clone-hero/07-internals.md, both "Proven in the game's code" | checked 2026-11-25 |
| P27: The game's own limit on `framerate` and on the product of framerate and multiplier. | unchecked | `TM-55` on docs/clone-hero/03-timing.md says where the game turns the two settings into a target rate was never read | T6 |
| P28: `maxsongspeed = 250` in settings.ini does not cap a `song_speed` of 5000 loaded from the ini. | unchecked | `C:\Users\Patrick\Documents\Clone Hero\settings.ini:90`; no page or listing names what reads `maxsongspeed` (a search of docs/clone-hero found none) | LS2; blocks nothing |
