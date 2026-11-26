# Plan: play Hydra's 5000% path in Clone Hero

Date: 2026-11-25. Task id: speed-run-plan-r2. Planner session: aa0ce9ac.

## What changed in this revision

The earlier draft asked you whether `ch_start` may add a `song_speed = 5000` line to `settings.ini`, and you said "Yes, add it". That question stood on a guess. The game never reads its song speed from `settings.ini` and never writes it there (TM-58 on `docs/clone-hero/03-timing.md`, P9). So the question, your answer to it and the ruling row that quoted your 2026-11-13 choice for it are removed: your answer was given on a false premise, and no file line can set the speed.

The speed lives only in the game's memory. One game method changes it (TM-59). So the plan now sets the speed by calling that method from inside the game, before the chart loads (T2). Your 2026-10-30 ruling already lets the tools call game methods and write game fields (P42), so this needs no question. T2 rewords the one page sentence that says the launch agent calls none.

## The short answer first

We do not know yet whether today's settings can play this chart, so the plan finds out offline before it starts the game.

Today's `C:\Users\Patrick\Documents\Clone Hero\settings.ini` holds framerate 330, frame multiplier 16 and vsync off (P22). The game aims for framerate times multiplier frames a second (TM-55): 5,280 a second, a frame step of 0.1894 ms. You ruled that framerate is a setting you can change (P23), so 330 and 16 are today's values, not limits. The game's own limits are now read: framerate runs from 60 to 2010 and the multiplier from 1 to 25 (TM-61, P27a). Whether the game really reaches the top of both is something only a run can show (P27b).

Here is the chart's load, with each number's source. At 5000% the chart has 99,163 chords in 409.2 s (`C:\Users\Patrick\Downloads\Hydra\hydra-data\disco_score100.json`, each time divided by 50 for TM-56; P15). The median gap between chords is 3.0 ms, about 16 frames. The smallest gap is 0.117 ms, between two different pads, and different pads can share a frame. The limit that matters is the same pad: a pad needs one frame down and one frame to lift before it can go down again (P3). The smallest same-pad gap is 0.838 ms. Two frames at 0.1894 ms are 0.379 ms, so even frames fit every same-pad gap.

The risk is the long frames. The full As Icicles Fall run at multiplier 16 logged 2,647,129 frame steps; 5 were longer than 48.15 ms and the longest was 71.83 ms (P4). Nobody knows what causes them. What a long frame costs at this density is what the dry run (T5) counts. You ruled that a run that does not match Hydra is never wanted (P18), so the game starts only if the dry run is clean.

## What the plan does

You said: "if our existing settings can handle this, i want to try that first". So the plan goes first by the frame route at today's settings, and its first check is whether they can handle this chart.

The offline half needs no game. It teaches `hydra_replay` the song speed, so it can write the 5000% path's press schedule (T1, T4). It then replays that schedule through the tool's real frame picker, fed with frame steps from a real run (T5). That counts how many presses would be dropped or skipped, without starting Clone Hero.

If the dry run is not clean, the fallbacks come in this order. First a higher framerate, up to the game's limit of 2010 (T6, question 1). Second, the bot route, which a separate planner owns in `docs/superpowers/plans/2026-11-25-ch-bot-route.md` on branch `claude/bot-route-plan`. This plan does not design it.

The live half starts only if the dry run shows no dropped press and no skipped band. It sets the game's song speed in memory, checks the game holds 5000%, plays the path and compares the score with Hydra's 38,976,455.

This plan changes the run tool and `hydra_replay`. It changes nothing Hydra decides or shows, so it needs no Owner map.

## Global constraints

The run must not need focus and must not take your keyboard. Every live step uses the kit route only. No step calls `ch_key`, `ch_press` or `ch_play_chart` with the keys route.

No agent builds or runs `hydra_replay`, `hydra_tests` or the game. The main session does those (agent rule 14).

Nothing runs over the song library. Every step names this one song folder.

The song speed has one owner in Hydra: `Settings::speed_percent`, carried by `Settings::reading()` into the parser (P7). `hydra_replay` must use that owner and add no speed arithmetic of its own.

The schedule file is the only source of press times. The player and the kit agent must not scale, shift or widen any time for the speed.

No step treats framerate 330 or multiplier 16 as a limit. The limits are the game's own (TM-61).

No step writes a speed to any file. The game's speed is set in memory only, through the game's own setter (P9, P29).

## The tasks

T1, T2 and T3 touch different files and can run at once. T4 needs T1 merged and built. T5 can start with T1 and finishes after T4's files exist. T6 runs only if T5 reports a drop. T7 comes last.

### T1: hydra_replay takes the song speed and Safe paths

Today `hydra_replay` builds an `app::Settings` from its flags and has no flag for the song speed or for Safe paths (P6). Your path was found with "Song speed 5000% · Safe paths", so the tool cannot reproduce it. The executor adds two flags, `--speed N` and `--safe-paths`, that set `Settings::speed_percent` and `Settings::safe_paths`. The speed joins the list of fields that `Settings::clamp` pulls into range. Nothing else changes: `Settings::reading()` already carries the speed (P7).

The dump, score and schedule files each record the speed and the Safe paths value they were made with, read from the song and the settings. The comment at `tools/replay_schedule.h` line 8 and the refusal text in `tools/ch_probe/experiments/play_chart.py` both say "at 100% speed" (P19, P20); the executor rewords them to name the schedule's own speed.

Files: `tools/replay.cpp`, `tools/replay_schedule.cpp`, `tools/replay_schedule.h`, `tools/ch_probe/experiments/play_chart.py`, the `hydra_replay` test file that already tests `--efill`, and `docs/development.md`.

Acceptance criteria:

1. A test loads one corpus chart from `testdata/input` through `hydra_replay`'s settings path at `--speed 200` and at no flag. It requires every chord's time at 200 to be the chord's time at 100 divided by the song's `speed_ratio`, read from the song. It fails if the flag does not reach the parser.
2. A test requires `--speed 0` and `--speed abc` to be refused with a message, and a value above the range to land on `kMaxSpeedPercent`, read by name. It fails if the flag skips `Settings::clamp`.
3. A test requires the schedule JSON's recorded speed to equal the song's `speed_percent()`, for a run with the flag and one without. It fails if the file records a typed constant.
4. A test requires `--safe-paths` to set `Settings::safe_paths`, by checking the settings the run built.
5. A Python test feeds the player a schedule with one infeasible squeeze and a recorded speed of 200 and requires "200%" in the refusal and no "100%".

### T2: ch_start sets the game's song speed in memory, and ch_state shows it

`ch_start` takes a new optional argument, `song_speed`. It writes no file. The launch agent sets the speed inside the game by calling the game's own int setter on the `song_speed` object, and reads the value back.

Where the call goes. A human launch already loads the launch agent, which changes one byte and then hooks the entry of the command-line player loader `0x3968D0` (`applyPatch` in `tools/ch_mcp/agent/launch_core.ts`, P35). Today that hook only counts the loader's runs and reads the patch site. The loader loads the chart, and the chart load reads the speed (TM-60, P36). So the loader's entry is the last moment before the speed is used, and the hook is already there.

What the hook does, on the loader's first run only, when the arm message carries a speed:

1. It finds the `song_speed` object: the settings class's TypeInfo slot, its statics at +0xB8, then static +0x10 (P29). If any of the three is null it sets nothing and the launch is refused. P30 (the object exists at the loader's entry) is unchecked and can only be shown by a run, so this refusal is what makes a wrong P30 fail loud, and LS1 is the run that shows it.
2. It reads the object's minimum (+0x14) and maximum (+0x18) and requires them to equal the two limits the arm message carries. If they differ, the object is not the one the plan read, and the launch is refused with what it found.
3. It calls the setter `0xFE5F50` with the object and the value (P29).
4. It reads +0x10 back and requires it to equal the value. If not, the launch is refused.

A refused launch ends as it does today: `bot_run.launch_human` kills the game and `ch_start` reports the reason. The status reply gains the speed before and after.

What stays as it is. The agent holds no address of its own: the setter's RVA, the TypeInfo slot's RVA, the offsets and the two limits ride in the arm message from `tools/ch_mcp/launch_patch.json`, as the patch's RVAs do (P35). `ch_start` refuses a `song_speed` outside the game's 10 to 5000 before any backup or launch (P8), and refuses one on a launch that loads no launch agent (a bot start or a menu start), because nothing there could set it.

The game must also be seen at 5000%. No tool prints the song speed today (P13). The executor adds two lines to what `ch_state` prints: `GameManager.practiceSpeed` (+0xD8), which `GameManager.Awake` sets to the speed ratio (TM-57, P37), and the `song_speed` setting's value, read the way the state agent reads its other fields.

After the run. The game's end-of-song methods put the speed back to a number they saved, in the game's memory (TM-59, P38). That happens after the stats file's `playback_speed` was taken from the setting (TM-63, P32). `ch_stop` then closes the game, so the speed lives nowhere after the run. Your own next game session starts at 100%, because the speed is never loaded from a file (TM-58, P33). Nothing on disk changes.

Files: `tools/ch_mcp/agent/launch.ts`, `tools/ch_mcp/agent/launch_core.ts`, `tools/ch_mcp/launch_patch.json`, `tools/ch_mcp/bot_run.py`, `tools/ch_mcp/game_tools.py`, `tools/ch_mcp/server.py`, `tools/ch_mcp/frida_tools.py`, the state agent's field list, `tools/ch_mcp/tests/test_launch_core.py`, `tools/ch_mcp/tests/test_frida_attach.py`, `tools/ch_mcp/tests/test_game_tools.py`, `docs/agents/clone-hero-mcp.md` (the sentences "The launch agent calls no game method" and "that hook only counts the loader's runs" are reworded to name the one call).

Acceptance criteria:

1. A test of `launch_core.ts` with a made-up memory requires the order: find the object, check the limits, call the setter once, read back. It requires no call when no speed was armed.
2. The same test file requires a refusal, with no setter call, for each of: a null slot, null statics, a null object, a minimum that differs, a maximum that differs. One case per failure point.
3. A test requires a refusal when the read-back differs from the value.
4. A test requires the setter to be called on the loader's first run only, when the loader runs twice.
5. A test requires `ch_start(song_speed=N)` to refuse N below the minimum and above the maximum before the backup and before any launch, with the limits read by name. A second test requires the refusal on a bot start.
6. A test compares the bytes of a temp `settings.ini` before and after `ch_start(song_speed=5000)` on a faked launch and requires them equal apart from the two lines `ch_start` already sets.
7. `test_frida_attach.py` requires `launch.ts` and `launch_core.ts` to hold no address, as today, and requires the launch agent's one method call to be on a list of its own, as the kit's and menu's are.
8. A test feeds `ch_state` a saved field read that holds `practiceSpeed` and the setting's value and requires both lines.
9. The start reply names the speed before and after, and says it was set in the game's memory and no file was changed.

### T3: a schedule made at one speed never plays in a game at another

`ch_run_chart` already refuses a schedule made from another path. Nothing ties the schedule's speed to the game's. The executor adds one refusal beside it: the schedule's recorded speed (T1) must equal the speed this session's `ch_start` set, and a schedule with no recorded speed counts as the speed `hydra_replay` wrote before T1, read from one named constant. `ch_compare` already checks the game's `playback_speed` against the run's as two integers (P17).

Files: `tools/ch_mcp/game_tools.py`, `tools/ch_mcp/tests/test_game_tools.py`, `docs/agents/clone-hero-mcp.md`.

Acceptance criteria:

1. A test requires `ch_run_chart` and `ch_play_chart` to refuse, before the attach, a schedule recorded at 5000 when the start set no speed, and one recorded at 100 when the start set 5000. It fails if either direction passes.
2. A test requires a matching pair to pass the check.
3. A test feeds `ch_compare` a real saved stats file with `playback_speed` given as 5000 and requires the settings line to FAIL when the file says another speed.

### T4: the main session makes the 5000% files and checks the path against the GUI

After T1 is merged and built, the main session runs `hydra_replay dump`, `score` and `schedule` on the song folder with `--speed 5000 --safe-paths`, Pro Drums Expert, into `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-25\speed-run\`.

Acceptance criteria:

1. The score file's total equals 38,976,455 and its activation count equals 302, the values Hydra's screen showed you. A mismatch is stop S1.
2. The schedule's `refusals` list is empty and no squeeze has `feasible` false. Either one is stop S2.
3. The schedule's recorded speed is 5000.
4. The schedule's chord count is 99,163 and its smallest same-pad gap is at least 0.838 ms. A different count or gap goes back to the planner.

This settles P10 and P11.

### T5: an offline dry run of the frame picker on the 5000% schedule

This is the check that decides whether the game is started at all. The kit agent's frame picker is plain logic, `PressSchedule.step` in `tools/ch_mcp/agent/press_schedule.ts` (P3). The executor first searches `tools/ch_mcp/agent` and its tests for an existing harness that drives `step` from a list of frames, and uses it if one exists. Otherwise it writes a small script that loads a schedule, feeds `step` one frame at a time, and counts each outcome the picker itself reports: sent, dropped, and `no_frame_in_band`. It works out no window and no drop rule of its own.

The frames come from LS-M16 run 2's `frame_clock-1.csv` (P4), looped if the schedule is longer. The script also runs on an even 0.1894 ms step. Before it runs, the executor reads `settings.ini` and requires framerate 330 and multiplier 16. The script also reports the largest number of presses the picker looked at in one frame.

Files: one new script under `tools/ch_mcp/agent/` or its tests folder, one test file beside it, `docs/agents/clone-hero-mcp.md`.

Acceptance criteria:

1. A test feeds the script a three-press schedule and an even frame list and requires three sent, none dropped. A second test puts one frame longer than a press's whole window across that press and requires exactly that press dropped.
2. The report for the 5000% schedule states, for each of the two frame lists: presses sent, dropped, marked `no_frame_in_band`, and the largest number examined in a frame.
3. Any dropped or `no_frame_in_band` press on either list is stop S3. The live steps do not start.
4. A largest-examined figure that grows with the schedule's length is stop S4.

Waits for: T4's schedule file.

### T6: a higher framerate (only if T5 stops at S3)

The setting's limits are read: framerate 60 to 2010, multiplier 1 to 25 (TM-61, P27a). So a higher rate is allowed by the settings. This task writes the follow-up plan that adds a `framerate` argument to `ch_start` and reruns T5 with a frame log taken at that rate. It builds nothing itself, and it starts only after question 1 is answered yes.

Acceptance criteria:

1. The follow-up plan names the framerate it tries and reads its limit from TM-61.
2. It holds a live step that measures the frame step the game really reaches (P27b) before any full run.

### T7: write the results back

The executor adds what the steps proved to `docs/clone-hero/03-timing.md` and `docs/agents/clone-hero-mcp.md`. This plan file leaves the repo in the same merge.

Acceptance criteria:

1. Each new or changed block has its seven lines and one of the four labels, and cites a run folder or a file in the repo.
2. `py -I tools/ch_mcp/dump_facts.py --check` passes, run by the main session.

## The live steps

No live step starts unless T5 reported zero dropped and zero `no_frame_in_band` presses on both frame lists, and you have said go to that step's own message. Each step runs at most twice. A second failure stops the plan and goes to you with the design question, not a third try.

### LS0a: the compare tool on a saved run

Calls: `ch_compare` (never tried)

No game runs. The main session calls `ch_compare` on LS-M16 run 2's `scorestats-*.json` with that run's `score.json` and four settings. Pass: the reply is a verdict per line, not a refusal. Uses P14.

### LS0b: the Star Power compare tool on a saved run

Calls: `ch_sp_compare` (never tried)

No game runs. The main session calls `ch_sp_compare` on LS-M16 run 2's `sp_watch-1.csv` with that run's score and schedule files. Pass: a verdict per line, not a refusal. Uses P14.

### LS1: the game starts with the speed set in memory

Calls: `ch_start` (never tried), `ch_stop` (seen: LS-P2 run 3)

`ch_start` gets the song folder, `human=True`, `player="ProDrums,Expert"`, `frame_multiplier=16` and `song_speed=5000`. The one new piece is the setter call inside `ch_start`. Pass: the launch agent says patched, the reply shows the speed read back as 5000, and `ch_stop` prints every check "ok". Fail: any refusal; report its words, which name the failed step of T2's four. Uses P29, P30 and P39.

### LS2: the game holds 5000%

Calls: `ch_start` (seen: LS1's passing run), `ch_attach` (seen: LS-P2 run 3), `ch_state` (never tried), `ch_stop` (seen: LS-P2 run 3)

Pass: `ch_state` prints `practiceSpeed` as 50.0 and the `song_speed` setting's value as 5000. Fail: any other value. That is stop S5. Uses P37.

Live only, and not settled by this step: that the game plays the song at that speed, and that the audio library accepts a tempo of 4900 (P40). LS3 shows both.

### LS3: the full run

Calls: `ch_start` (seen: LS1's passing run), `ch_run_chart` (never tried), `ch_wait_stats` (seen: LS-M16 run 2), `ch_compare` (seen: LS0a's passing run), `ch_sp_compare` (seen: LS0b's passing run), `ch_stop` (seen: LS-P2 run 3)

`ch_run_chart` gets T4's score and schedule files and `kit_player="System"`. `ch_compare` gets `playback_speed=5000` with the other three settings. Pass: `ch_compare` passes every line, with the game's score equal to Hydra's 38,976,455; no row of `press-1.csv` says dropped; `ch_sp_compare` passes; `ch_stop` restores cleanly. Fail: anything else, reported with the frame clock log's count of steps longer than 48.15 ms. A song that never starts or ends at once is reported as the audio question (P40). Uses P14, P32 and P40.

## The stops

S1: `hydra_replay` at 5000% Safe paths does not give the screen's score or activation count. Nothing live runs.

S2: the schedule refuses the path or holds a squeeze the game cannot make. Nothing live runs.

S3: the dry run drops a press or marks one `no_frame_in_band`. Report both lists' counts. The next steps are question 1 and T6, then the bot route.

S4: the picker's per-frame work grows with the schedule. It goes to a fresh plan.

S5: the game does not hold 5000% after the setter call. Report both values; the design goes back to you.

## The bot route

A separate planner owns it: `docs/superpowers/plans/2026-11-25-ch-bot-route.md` on branch `claude/bot-route-plan`. This plan holds no task for it and does not wait for it. It is the fallback after T5 stops at S3 and the framerate step is tried or ruled out. T1, T3 and T4 serve both routes.

## Questions

1. Asked only if the dry run fails at today's settings: may `ch_start` set `framerate` in your `settings.ini`, as it sets the multiplier? The game allows 60 to 2010 (TM-61). You would see the game run at a higher frame rate for the run, and `framerate` stay at the new value in `settings.ini` afterwards, because the tools do not put the file back (`docs/agents/clone-hero-mcp.md`). Your 2026-11-13 choice "Leave the line changed" (`docs/agents/rulings/clone-hero.md`, under `## Confirmed`) was asked about the pause line only, so it does not answer this. If no, the frame route is closed at today's settings and the bot route is next.

## Out of scope

Finding the cause of the long frames. Changing where presses are aimed to survive a long frame. Any change to Hydra's own path search or screens. The bot route's design. A command-line speed option: the game has none (P41). Checking the audio library's tempo range by reading its code: only a run is planned for it (P40).

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| The run needs no focus and takes no keyboard | Global constraints, paragraph 1 | LS1, LS2, LS3 | every `Calls:` line holds kit-route calls only |
| No agent builds or runs hydra_replay or the game | Global constraints, paragraph 2 | T4 | T4 is the main session's step |
| Nothing runs over the library | Global constraints, paragraph 3 | T4, LS1 to LS3 | each names the one song folder |
| hydra_replay uses the one speed owner | Global constraints, paragraph 4 | T1 | T1 criterion 1 |
| The player scales no time for the speed | Global constraints, paragraph 5 | T5 | T5 criterion 1, second test |
| No step treats 330 or 16 as a limit | Global constraints, paragraph 6 | T6 | T6 criterion 1 reads the limit from TM-61 |
| No speed is written to any file | Global constraints, paragraph 7 | T2 | T2 criterion 6 |
| `--speed` reaches the parser | T1 | T1 | criterion 1 |
| A bad speed is refused or clamped | T1 | T1 | criterion 2 |
| Files record their speed and Safe paths | T1 | T1 | criteria 3 and 4 |
| The refusal names the schedule's speed | T1 | T1 | criterion 5 |
| The setter is called before the chart load, once | T2 | T2 | criteria 1 and 4 |
| A missing object or wrong limits refuse with no call | T2 | T2 | criterion 2, one case per failure point |
| The value is read back and a mismatch refuses | T2 | T2 | criterion 3 |
| A speed outside 10 to 5000 is refused before any launch | T2 | T2 | criterion 5 |
| A speed on a launch with no launch agent is refused | T2 | T2 | criterion 5, second test |
| The agent holds no address of its own | T2 | T2 | criterion 7 |
| The reply says before, after and no file changed | T2 | T2 | criterion 9 |
| The game's speed can be read in a run | T2 | T2 | criterion 8, LS2 |
| A schedule at another speed is refused | T3 | T3 | criteria 1 and 2 |
| ch_compare checks 5000 like any speed | T3 | T3 | criterion 3 |
| The 5000% files match the screen and the load figures | T4 | T4 | criteria 1 and 4, stop S1 |
| No refusal or infeasible squeeze goes live | T4 | T4 | criterion 2, stop S2 |
| Today's settings are tried first | "What the plan does" | T5 | criterion 2 reports on the 0.1894 ms step |
| The dry run counts the picker's own outcomes | T5 | T5 | criterion 1 |
| No live step starts after a dropped press | T5, "The live steps" | T5 | criterion 3, stop S3 |
| The picker's work does not grow with the schedule | T5 | T5 | criterion 4, stop S4 |
| A higher framerate is tried only after today's settings fail | "What the plan does" | T6, question 1 | T6 runs only at S3 and after question 1 |
| The real frame rate is measured before a run at a new framerate | T6 | T6 | criterion 2 |
| Each live step runs at most twice | "The live steps" | LS0a to LS3 | the run count in the result folder |
| The game is confirmed at 5000% before the full run | LS2 | LS2 | LS2's pass line, stop S5 |
| The game's score equals Hydra's | LS3 | LS3 | `ch_compare` passes every line |
| Results reach the Clone Hero pages | T7 | T7 | criteria 1 and 2 |
| The bot route is not designed here | "The bot route" | none | the pointer names the other plan |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The game's hit window runs in real time at every speed, 48.15 to 85.72 ms a side. | data | `TM-43` and `TM-22` on docs/clone-hero/03-timing.md | checked 2026-11-25 |
| P2: At 5000% the chart's times are the file's times divided by 50. | data | `TM-56` on docs/clone-hero/03-timing.md | checked 2026-11-25 |
| P3: The kit agent lifts a pad a frame before it takes a new press, drops a press whose window has passed, and marks a press `no_frame_in_band` when a frame passes its band. | code | `tools/ch_mcp/agent/press_schedule.ts:761-776` and `tools/ch_mcp/agent/press_schedule.ts:830-856` | checked 2026-11-25 |
| P4: In LS-M16 run 2, 5 of 2,647,129 frame steps were over 48.15 ms and the largest was 71.83 ms. | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-m16-run2\frame_clock-1.csv:1-3` (the file's header and first rows: frame number and clock in ms) and `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-m16-run2\frame-settings.json:1-6` (framerate 330, vsync 0, multiplier 16 before and after), and TM-55 on `docs/clone-hero/03-timing.md:181-189` (2,647,130 frames, largest step 71.8327 ms) | checked 2026-11-25 for the settings and the file's shape; the count of 5 steps over 48.15 ms was made by script by the first draft's reviewer, because the 71 MB file cannot be opened whole with Read |
| P5: At multiplier 16 a full kit run held a median frame step of 0.1894 ms. | data | `TM-55` on docs/clone-hero/03-timing.md | checked 2026-11-25 |
| P6: hydra_replay builds its settings with no speed and no Safe paths flag. | code | `tools/replay.cpp:196-242` | checked 2026-11-25 |
| P7: `Settings::reading()` carries `speed_percent` to the parser, and `set_speed_percent` is the one place the percent becomes the ratio. | code | `src/app/config.cpp:351-356` and `src/core/song.h:205-218` | checked 2026-11-25 |
| P8: The game's `song_speed` setting has default 100, minimum 10, maximum 5000, step argument 5. | code | `docs/clone-hero/code/6142/settings_load_374D50.asm:2017-2027` | checked 2026-11-25 |
| P9: `song_speed` is built under the ini class's key ('', 'tmp'); the saver skips a key whose first string is empty; a file load takes only the keys under the name it is given. | code | `docs/clone-hero/code/6142/settings_load_374D50.asm:2015`, `docs/clone-hero/code/6142/ini_class_cctor_21159A0.asm:106-117`, `docs/clone-hero/code/6142/ini_save_2112DB0.asm:100-105` and `docs/clone-hero/code/6142/ini_file_load_2114A80.asm:145-163` | checked 2026-11-25 |
| P10: hydra_replay at `--speed 5000 --safe-paths` reproduces the screen's 38,976,455 and 302 activations. | unchecked | `tools/replay.cpp:196-242` shows only that the settings path is shared | T4 |
| P11: The 5000% path's schedule holds no refusal and no infeasible squeeze. | unchecked | `tools/replay_schedule.cpp:314-361` is the refusal rule; nobody has run it at 5000% | T4 |
| P12: `ch_start` refuses a bad multiplier before any launch and then sets it, the pattern T2's range refusal follows. | code | `tools/ch_mcp/game_tools.py:872-911` | checked 2026-11-25 |
| P13: No tool prints the game's song speed today. | code | `tools/ch_mcp/agent/state.ts:49-59` (the state reply holds five GameManager fields and none is the speed) and `tools/ch_mcp/agent/gamemanager.ts:50-66`; a search of the folder tools/ch_mcp, .py and .ts files with node_modules left out, for practiceSpeed and for the offset 0xd8 finds nothing | checked 2026-11-25 |
| P14: `ch_compare` and `ch_sp_compare` ran on LS-M16 run 2. | unchecked | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-m16-run2\builds-1.json:1-5` and `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-m16-run2\frame-settings.json:1-6`; the folder holds also frame_clock-1.csv, press-1.csv, sp_watch-1.csv and one scorestats file, and no compare reply | LS0a, LS0b; blocks nothing (LS3 waits on both) |
| P15: The chart at 5000% has 99,163 chords in 409.2 s, median chord gap 3.0 ms, smallest same-pad gap 0.838 ms. | unchecked | `C:\Users\Patrick\Downloads\Hydra\hydra-data\disco_score100.json` is one 61 MB line that Read cannot open; two earlier sessions counted these figures by script from it | T4 criterion 4 (hydra_replay at 5000% recounts the chord count and the smallest same-pad gap); the plan's short answer and T5 use the figures as the first draft's counts |
| P16: The schedule refuses a press that would land inside an earlier unhit same-pad window. | code | `tools/replay_schedule.cpp:314-361` | checked 2026-11-25 |
| P17: `ch_compare` compares the game's `playback_speed` with the run's as two integers. | code | `tools/ch_mcp/compare_scorestats.py:99-117` | checked 2026-11-25 |
| P18: The user never wants a run that does not match Hydra. | ruling | `docs/sessions/2026-11-23-537dcff7.md:16`: "why are you offering a miss as an option? why would i ever want a run that doesnt match hydra?" | checked 2026-11-25 |
| P19: The player's refusal of an infeasible squeeze says "at 100% speed". | code | `tools/ch_probe/experiments/play_chart.py:320-330` | checked 2026-11-25 |
| P20: The schedule header says the per-note window is at 100% speed. | code | `tools/replay_schedule.h:8` | checked 2026-11-25 |
| P21: An ordinary press is aimed at its chord's whole window. | code | `tools/replay_schedule.h:123-124` | checked 2026-11-25 |
| P22: Today's settings.ini holds framerate 330, vsync 0, multiplier 16, `pitched_speedups = 0`, and no `song_speed` line anywhere. | code | `C:/Users/Patrick/Documents/Clone Hero/settings.ini:1-125` | checked 2026-11-25, the whole file read |
| P23: Framerate is a setting the user can change. | ruling | `docs/sessions/2026-11-12-428cf079.md:25`: "sounds like edge cases you invented. #3 isnt even possible to quantify because framerate is not permanently set, it can be changed in the settings" | checked 2026-11-25 |
| P25: The user wants today's settings tried first, and a separate planner owns the bot route. | ruling | the user's words as the main session quoted them in the planner's brief, "if our existing settings can handle this, i want to try that first", as the plan quotes them at `docs/superpowers/plans/2026-11-25-ch-speed-run.md:23`; no session card holds them yet | checked 2026-11-25 |
| P27a: The `framerate` setting runs from 60 to 2010 and the multiplier from 1 to 25. | code | `docs/clone-hero/code/6142/settings_load_374D50.asm:449-459` and `docs/clone-hero/code/6142/settings_load_374D50.asm:1481-1491` | checked 2026-11-25 |
| P27b: The game reaches framerate times multiplier frames a second at the top of both ranges. | unchecked | `TM-61` on docs/clone-hero/03-timing.md says it is not known; live only, because it depends on this machine | T6 (its follow-up plan holds the live step) |
| P28: `maxsongspeed` and `lowsongspeed` are separate online settings, and the song speed's clamp reads only its own minimum and maximum, so they do not cap a value given to the setter. | code | `docs/clone-hero/code/6142/online_settings_26410.asm:93-127` and `docs/clone-hero/code/6142/int_setting_clamp_FE5B10.asm:15-45` | checked 2026-11-25 |
| P29: The setter `0xFE5F50` stores its second argument at the object's +0x10 and tail-calls vtable +0x2C8 without reading its third argument; the `song_speed` object is static +0x10 of the class at TypeInfo slot 0x3C98320, statics at +0xB8. | code | `docs/clone-hero/code/6142/int_setting_set_FE5F50.asm:1-4` and `docs/clone-hero/code/6142/settings_load_374D50.asm:2028-2030` | checked 2026-11-25 |
| P30: At the entry of the loader `0x3968D0` the `song_speed` object already exists. | unchecked | `docs/clone-hero/code/6142/cli_loader_3968D0.asm:58` and `docs/clone-hero/code/6142/cli_loader_3968D0.asm:72` (the loader's one game call before the chart load is the list getter, so it builds nothing) and `docs/clone-hero/code/6142/songentry_load_35C110.asm:141-145` (the chart load reads the object and throws when it is null; scored command-line runs, LS-P2 run 2, passed it) | LS1. Live only: the settings loader `0x374D50` has no direct caller (byte scan, 2026-11-25) and one pointer to it in the DLL's data, so no listing shows what runs it or when; only a run shows the object exists at the loader's entry. T2's null refusal makes a wrong P30 fail loud before anything is set |
| P31: The int setting's vtable slot +0x2C8 holds the clamp `0xFE5B10`. | code | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs:533834-533840` (slot 25 of the generic setting class, the int instance at RVA 0xFE5B10; the vtable starts at +0x138 with 16 bytes a slot, so slot 25 is +0x2C8), `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs:534655-534743` (TypeDefIndex 8354 overrides slots 20 to 24 and 26, not 25), `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs:532111-532198` (TypeDefIndex 8308 overrides none) and `docs/clone-hero/code/6142/int_setting_set_FE5F50.asm:1-4` | checked 2026-11-25 |
| P32: The stats file's `playback_speed` is the setting's value when `EndOfSong.Start` reads it, before any restore method runs. | code | `docs/clone-hero/code/6142/endofsong_start_speed_read_15DF80.asm:28-48`, `docs/clone-hero/code/6142/stats_song_fields_252200.asm:8`, `docs/clone-hero/code/6142/stats_song_fields_252200.asm:77`, `docs/clone-hero/code/6142/scorestats_json_252630.asm:138-140`, and the four call sites of the three restore methods, none inside Start, at `docs/clone-hero/code/6142/endofsong_speed_restore_160260.md:11` | checked 2026-11-25; the one gap, a restore reached by a method pointer from inside Start, is named on TM-63 and LS3's `ch_compare` would show it |
| P33: No path is ever registered for the empty ini name, so a file load with it loads nothing. | code | `docs/clone-hero/code/6142/ini_register_caller_235340.asm:42-64` (the ninth registration, 'highway_' plus a name), `docs/clone-hero/code/6142/ini_save_2112DB0.md:27` (the other eight registrations' names), `docs/clone-hero/code/6142/ini_file_load_2114A80.asm:61-73` and `docs/clone-hero/code/6142/ini_file_load_2114A80.asm:95-104` (the lookup, then no file, so the saver and a return) | checked 2026-11-25 |
| P35: The launch agent takes every RVA from the arm message, writes one byte, then hooks the loader's entry; the hook only counts runs and reads the patch site. The page says "The launch agent calls no game method." | code | `tools/ch_mcp/agent/launch_core.ts:141-198`, `tools/ch_mcp/agent/launch.ts:45-72` and `docs/agents/clone-hero-mcp.md:35` | checked 2026-11-25 |
| P36: The command-line start calls the loader, then the scene change; the loader's chart load reads the speed ratio. | code | `docs/clone-hero/code/6142/cli_start_398F00.asm:44-58`, `docs/clone-hero/code/6142/cli_start_398F00.asm:115-122`, `docs/clone-hero/code/6142/cli_loader_3968D0.asm:72` and `docs/clone-hero/code/6142/songentry_load_35C110.asm:141-152` | checked 2026-11-25 |
| P37: `GameManager.Awake` stores the speed ratio at GameManager+0xD8. | code | `docs/clone-hero/code/6142/gm_awake_B41A0.asm:85-92` | checked 2026-11-25 |
| P38: An end-of-song method calls the setter with `GlobalVariables` +0x5C, and the clamp turns a value below 10 into 10. | code | `docs/clone-hero/code/6142/endofsong_speed_restore_160260.asm:41-57` and `docs/clone-hero/code/6142/int_setting_clamp_FE5B10.asm:30-45` | checked 2026-11-25; the link between setter and clamp is P31 |
| P39: A call of a game method from the launch agent's entry hook, on the game's own thread, works. | unchecked | `tools/ch_mcp/agent/launch.ts:63-69` is the hook; no run has made a call from it; live only | LS1; blocks nothing (T2 is built first; LS2 and LS3 wait on LS1) |
| P40: The game plays the song at 5000%, and the audio library accepts the tempo 4900 the audio load passes (value minus 100). | unchecked | `docs/clone-hero/code/6142/audio_load_36B700.asm:114-120` shows the subtraction only; the library's range is outside the game's code; live only | LS3; blocks nothing |
| P41: The command-line parser knows no speed option. | code | `docs/clone-hero/code/6142/cli_args_parse_395C10.asm:12-35` | checked 2026-11-25 |
| P42: The user's 2026-10-30 ruling allows the tools to call game methods and write fields. | ruling | `docs/agents/rulings/clone-hero.md:40-42`, under the Confirmed heading: "nobody picked that rule. create a way to navigate menus if it doesnt exist.", "again, nobody selected this rule. figure out the correct method." and the choice "Allow writes too" | checked 2026-11-25 |
