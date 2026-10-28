# Plan: the overlay spike, Phase 0 (2026-10-28)

Task id `overlay-spike-plan`, session `f756d547-af2c-46f3-b891-96dea4efb402`. Plan only; no code changed.

## The plain version

Hydra may one day draw a live overlay over Clone Hero: the next activation with a countdown, the player's SP bar beside the path's, a running "points vs optimal" figure, squeeze reminders, and "you activated one note late" feedback. Before anyone builds that, four things have to be true at the real game, and nobody has checked them live. This plan is the short spike that checks them. It adds a few small Python scripts to `tools/ch_probe`, runs them at the game in one sitting of about 25 minutes with the user, and writes the answers up under `docs/research/`. No Hydra C++ changes. No overlay UI.

The four questions, in order of importance. First, do the Star Power fields the disassembly named (amount, maximum, active flag, multiplier, end time) read correctly while a song plays. Second, where the game keeps the playing chart's checksum, and whether a reader can reach it each song by following pointers instead of searching all memory. Third, whether the game's song clock agrees with Hydra's chart times, and whether the game's score climbs chord for chord the way `hydra_replay` says it should. Fourth, a handful of cheap checks: the per-hit field candidate, pausing, whether the session log is written live, what engine finding and polling cost, and whether a plain topmost window shows over the game in fullscreen.

The work splits into four script tasks that run in parallel before the sitting, one sitting where the main session runs the scripts with the user at the game, and one write-up task after. Every live step has a cap and says what to report when the cap is hit.

## Open questions for the user

Each has my recommended answer. Say "go with the recommendations" to approve them all.

1. **Which chart for the Star Power and score runs.** The installed Note Shuffle test charts have no Star Power phrases at all (I scanned every `notes.chart` and `notes.mid` under `C:\Clone Hero\songs\Hydra Note Shuffle tests`: zero phrases in each), so they cannot answer question 1. The smallest checked-in chart with phrases is `testdata/input/common/IB24/T2/Stone Temple Pilots - Trippin on a Hole in a Paper Heart` (11 phrases, 36 KB, 180 s, `delay = 0`). It has no audio file, so it needs a silent `song.ogg`, which `probe_songs.write_silent_ogg` already makes with ffmpeg. Recommendation: install that chart as `C:\Clone Hero\songs\Hydra Probe\Trippin SP Spike` with a silent ogg, and use it for questions 1 and 3. The alternative is a purpose-built 60-second chart, but `probe_chart.py` cannot write phrases or fills today, so that costs a task of its own.

2. **How the spike activates Star Power.** `play_chart.py` presses only the eight pad keys (`input_driver.DEFAULT_BINDINGS` has no Star Power key), and whether hitting Hydra's chord at a fill's end activates in the game is unknown. Recommendation: before the sitting, bind Star Power to a key on the drums profile in the game's Controller Remap (I suggest `P`), and tell the main session the key. The auto-player then presses it at the times Hydra's optimal path activates. Fallback if the bind does not exist for drums: the user hits the green pad at the end of the fill the script announces, and the write-up records that the press times were the user's.

3. **The calibration run.** Question 3 asks whether audio calibration shifts the engine clock. Recommendation: run the Trippin chart twice, once at audio calibration 0 (as now) and once at audio calibration 30 ms, set in the game's Calibration menu between runs. 30 ms is inside the narrowest per-side hit window (about 48 ms), so a full combo still proves the clock either moved with the setting or did not. If the hit rate collapses in the second run, that is itself the answer, and the write-up says so.

4. **Song.ini `delay`.** Hydra's chord `ms` leaves out the chart's Offset and song.ini delay (per the docstring of `play_chart.py`; the engine's own code was not re-read for this plan). Both candidate charts have `delay = 0`, so this spike does not measure what a non-zero delay does to the clock. Recommendation: leave it out of Phase 0 and list it under Out of scope; Hydra already knows each chart's delay (`timing.delay_ms` in `src/app/analysis.cpp`), so Phase 1 can add it to the chord times if the sitting shows the clock is the audio clock.

5. **Installing the chart into the user's songs folder.** `C:\Clone Hero\songs` is the user's library folder and the main session does not write there on its own. Recommendation: the main session prepares the folder in the scratch area; the user copies it in (one drag) and presses Scan Songs in the game. That is step 1 of the sitting.

6. **Where the raw results live.** Recommendation: the CSV and JSON files the scripts write go under `tools/ch_probe/experiments/results/overlay-spike/`, beside the existing `poll_windows.csv`, and the write-up goes to `docs/research/2026-10-28-overlay-spike-results.md`. The Il2CppDumper output (`dump.cs`, tens of MB) stays in the scratch folder and is not committed; the write-up quotes the class and field lines it used.

## What the code does today (checked in the source)

`tools/ch_probe/process.py` opens the game by name with `OpenProcess` asking for `PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION`, finds where `GameAssembly.dll` loaded, and checks two hit-window constants in `.rdata` before any other read (`verify_targets`, raising `BuildMismatchError`). Every read goes through `ReadProcessMemory`. There is no read-only open today; the write right is there for the debugger path, which this spike does not use.

`engine_finder.py` scans every committed read-write region for the 16 bytes an engine object holds at `+0x30` and `+0x38` (the back and front window constants), drops hits inside the module, drops candidates whose total window at `+0x20` is empty, reads each candidate's song clock at `+0x100` twice 0.12 s apart, and returns the first one whose clock moved. It loops forever until a song plays, printing a dot every 0.4 s. It has no cap; the spike's runners wrap it with one.

`engine.py` names the fields: `+0x20` total window, `+0x30`/`+0x38` window constants, `+0x94` score (u32), `+0x100` clock (double seconds), `+0x198` flags, `+0x8C` note count, `+0x2E0` the hit-time candidate ("not yet confirmed live"). `snapshot()` reads the first `0x2E8` bytes of the object in one call. None of the Star Power fields is in `constants.py` yet.

`experiments/play_chart.py` loads a `hydra_replay score` JSON, finds the live engine, and for each chord waits on the clock (`live.wait_until`, 2 ms lead) and presses the chord's keys with `SendInput`. It reads the score 5 ms after each press and calls a rise a hit. It prints a table but writes no file; `diff_ms` in its table is the clock at the press minus the chord's ms, not the engine's own hit time. It handles a restart (clock jumped back) by re-syncing and a frozen clock (over 5 s) by stopping.

`tools/replay.cpp`'s `score` command writes, per chord, `index`, `tick`, `ms`, `notes` (color, cymbal, sp_points, multiplier), `is_fill`, `is_sp_phrase_end`, `combo_before`, `multiplier`, `multiplier_after`, `in_sp`, `points` and `cum` (with `total`). With `--path <dump.json>` it scores one of the dump's paths, so the activation ticks and their ms come from the same file. `dump` writes the optimal paths with `hyhash`.

`docs/audit/ch-evidence.md` answer 37 names, from the disassembly only: SP amount at `engine+0x68` as a whole number of chart ticks, the maximum at `+0x1B0`, the active flag at `+0x42` and the multiplier at `+0x48` (set by activation at `0x20F5640`), the end time at `+0x120`, and the overflow flag at `+0x130`. Answer 222 says score is added at `+0x94` as `points * [engine+0x48]`. None of these was read live. Nothing in the evidence says how wide `+0x42` and `+0x48` are; the reader records both as a byte and a u32 until the run says which.

Chart identity: `song_id_read` in `src/app/analysis.cpp` hashes the notes file with MD5 for a folder chart. I re-hashed `C:\Clone Hero\songs\Hydra Note Shuffle tests\NS E star cutoffs\notes.mid`: `234C917654CC7257E2BAFCF7D761304B`, equal to the `checksum` in `C:\Users\Patrick\Documents\Clone Hero\scorestats.json` for that chart. That file also records `sp_bar_ticks: 14400` for the chart, whose MIDI division is 480. Hydra's full meter is 4 bars of 2 measures each (`kMeasuresPerSpBar` in `src/core/timing.h`), which at 4/4 and 480 ticks per quarter is 15,360 ticks, not 14,400. So either the game's SP tick is not the chart tick, or the meter is not 8 measures; the spike's read of `+0x1B0` settles which.

The game's settings today (`C:\Users\Patrick\Documents\Clone Hero\settings.ini`): `fullscreen = 0`, `vsync = 0`, `[offsets] video = 0`, `audio = 0`. The last session log (`Logs\Session-2026-10-17_19-29-33.log`) holds `Loading Song: ...`, `Seeking to song time:...`, `OnCountdownStart` and `OnCountdownEnd` lines, and its last write was 45 s after its creation, so the file is appended during a session. Whether a line appears within a second of the event is the live check. The `scorestats.json` entry for the Note Shuffle run says `"is_bot": true`: the game has a built-in bot, which the spike can fall back to for a guaranteed full combo if `play_chart.py` misses.

Il2CppDumper v6.7.46 is at `C:\Users\Patrick\Downloads\Il2CppDumper-net7-v6.7.46` and the game's `global-metadata.dat` (15.6 MB) is at `C:\Clone Hero\1.1 june\Clone Hero\Clone Hero_Data\il2cpp_data\Metadata`. It runs as `Il2CppDumper.exe <GameAssembly.dll> <global-metadata.dat> <out dir>` and reads files only; it never touches the running game.

The unit tests in `tools/ch_probe/tests` are `unittest` files that put the repo root on `sys.path`, build a real `Process` over fake memory with `tests/fakes.py`, and never open a process. New tests follow that shape and run one file at a time: `py -m unittest tools.ch_probe.tests.test_<name>`.

## The sitting: what the user does, step by step

One sitting, about 25 minutes. The main session runs every script; the user only handles the game. Everything below the first step happens in this order.

1. Copy the prepared folder `Trippin SP Spike` from the scratch area into `C:\Clone Hero\songs\Hydra Probe\`, start Clone Hero, and press Scan Songs. Check in Options that the drums profile is Pro Drums with 2x Bass on, Note Shuffle off, speed 100%, audio and video calibration 0, Star Power bound to `P` (question 2), and that `settings.ini` still has `fullscreen = 0`. About 4 minutes.
2. Run 1. The main session starts `log_tail.py`, `sp_watch.py` and `play_chart.py --log ... --activate ...` and says "go". The user picks `Hydra Probe - Trippin SP Spike` and starts it; the scripts play it to the end, activating Star Power where Hydra's optimal path does. About 3.5 minutes. If the auto-player misses more than 2% of notes, the main session stops it, and the user plays the same run once more with the game's bot instead (cap: one more run). The user does not drum.
3. Set audio calibration to 30 ms in the game's Calibration menu. Run 2: the same scripts, same chart, same activations. About 3.5 minutes, cap one run. Set calibration back to 0 afterwards.
4. Run 3. The main session starts `find_checksum.py` and `sp_watch.py`; the user picks `NS E star cutoffs` (28 s) and starts it. Ten seconds in, the user presses Escape to pause, waits 5 seconds, and resumes. The script reports the checksum's addresses and the pointer paths it found. About 2 minutes, cap one run (plus one repeat if the finder reports zero hits).
5. Quit the game, set `fullscreen = 1` in `settings.ini`, start the game, and open any song. The main session runs `topmost_probe.py`, which shows a plain always-on-top window with a large label for 60 s. The user says whether the label is visible over the game. Then quit, set `fullscreen = 0` back, done. About 4 minutes.

`poll_cost.py` runs during runs 1 and 3 with no step for the user.

## Tasks before the sitting (four executors on Opus, in parallel, own worktrees)

Each task is sized for one executor in 100 tool calls and 20 minutes. Every task adds its unit tests in `tools/ch_probe/tests` as `unittest` files and runs only its own file. No task builds Hydra. No task launches or attaches to Clone Hero: every live-only function is marked LIVE-ONLY in its docstring and sits behind a callable the tests fake, as `process.py` and `engine_finder.py` do today. No task runs anything over the user's library.

### Task A: the Star Power watcher and its comparison

Owns `tools/ch_probe/constants.py`, `tools/ch_probe/engine.py`, new `tools/ch_probe/experiments/sp_watch.py`, new `tools/ch_probe/experiments/sp_compare.py`, new `tools/ch_probe/tests/test_sp_watch.py`.

Step 1. Add the five Star Power offsets and the overflow flag to `constants.py` beside `OFF_HIT_TIME`, each with a comment that says "from the disassembly, docs/audit/ch-evidence.md answer 37; not yet confirmed live": `OFF_SP_AMOUNT = 0x68`, `OFF_SP_MAX = 0x1B0`, `OFF_SP_ACTIVE = 0x42`, `OFF_SP_MULTIPLIER = 0x48`, `OFF_SP_END_TIME = 0x120`, `OFF_SP_OVERFLOW = 0x130`. `SNAPSHOT_SIZE` in `engine.py` stays `OFF_HIT_TIME + 8`, which already covers `+0x1B0`; `+0x2E0` is still the furthest field.

Step 2. Extend `engine.Snapshot` and `decode_snapshot` with `sp_amount` (u64 at `+0x68`), `sp_max` (u64 at `+0x1B0`), `sp_active_byte` and `sp_active_u32` (at `+0x42`), `sp_mult_byte` and `sp_mult_u32` (at `+0x48`), `sp_end_s` (double at `+0x120`) and `sp_overflow_u32` (at `+0x130`). Two widths are recorded for the two flags because the evidence does not say their width; the write-up picks one. Add a `decode_u64` import from `process.py` (it exists). The existing `Snapshot` fields and their order do not change, and the existing `test_engine.py` cases must still pass unchanged.

Step 3. `sp_watch.py`: open the process, `verify_targets`, find the live engine with a cap of 120 s (print "no playing song found in 120 s; start the song and run again" and exit 2), then poll `snapshot()` every 4 ms until the clock freezes for 5 s (`live.StoppedCheck`) or 240 s pass, writing one CSV row per poll whose decoded fields changed: wall time (`perf_counter`), clock, score, every SP field, hit time. On exit print the row count, the engine address, and the first and last clock. It takes `--out <csv>`; default under `experiments/results/overlay-spike/`. It opens the process read-only through the option Task C adds if that has merged, else the existing `open_process`; the write-up notes which.

Step 4. `sp_compare.py`: pure logic, no process. Inputs: the watcher CSV, the `hydra_replay score --path` JSON of the optimal path, and the activation log the auto-player writes (Task B's CSV, read by column name so the two tasks only share the header names agreed below). It produces a plain-text report with these checks, each printed as `PASS`/`FAIL` with the numbers:

- Every chord with `is_sp_phrase_end` true is followed, within 100 ms of clock, by a rise of `sp_amount`; the rise size is printed next to the phrase, and the report says whether every rise is the same number of ticks and what that is in Hydra's bars (`sp_max / 4` is the expected phrase size if the game's meter is four phrases).
- `sp_max` is constant over the run. The report prints it beside Hydra's full-meter ticks for the chart and beside `scorestats.json`'s `sp_bar_ticks` when `--sp-bar-ticks N` is given. Hydra's full meter is resolution times 4 quarters times 8 measures, assuming 4/4. The score JSON does not carry the resolution, so the comparer takes `--resolution N`, read off the chart's `[Song]` section (`Resolution = 192` and `Offset = 0` for the Trippin chart, so Hydra's full meter there is 6,144 ticks).
- At each activation press (Task B's log), within 100 ms: `sp_active` goes 0 to 1 and `sp_mult` doubles (4 to 8, or whatever the multiplier was), and `sp_end_s` becomes a clock time ahead of now; at that `sp_end_s` the flag returns to 0 and the multiplier halves, within 100 ms. The report prints Hydra's deactivation ms for the same activation beside the game's `sp_end_s`, and their difference.
- `sp_mult` equals the chord's `multiplier` field from the JSON at the moment of every chord press (allowing the one-chord lag the 4 ms poll can cause).
- `+0x2E0` (`hit_time_s`): for each press in Task B's log, whether the hit time changed within 20 ms of the press and how far it sits from the chord's ms. The verdict uses `constants.HIT_TIME_MAX_GAP_MS` (10 ms) as `experiments/watch_window.py` already does.
- Pause: if the clock held still for more than 1 s and then resumed without jumping back, report the pause span and what every SP field did during it (expected: nothing moved).

Step 5. Tests in `test_sp_watch.py`: `decode_snapshot` on a hand-built `0x2E8`-byte block gives every new field (literals); the comparison on a small hand-written CSV and JSON passes when phrase rises, activation, deactivation and multiplier all line up and fails on each of four broken inputs (a missing rise, a flag that never sets, an end time that never clears, a multiplier that stays 4). Run `py -m unittest tools.ch_probe.tests.test_sp_watch` and `py -m unittest tools.ch_probe.tests.test_engine`; report the pass counts.

### Task B: the auto-player logs its presses and activates Star Power; the score comparison

Owns `tools/ch_probe/experiments/play_chart.py`, `tools/ch_probe/input_driver.py`, new `tools/ch_probe/experiments/score_compare.py`, `tools/ch_probe/tests/test_play_chart.py`, new `tools/ch_probe/tests/test_score_compare.py`.

Step 1. `input_driver.py`: add `Lane.STAR_POWER = 8` with no default binding; `InputDriver.press_chord` must raise a clear error when asked to press a lane with no binding. `play_chart.py` takes `--sp-key <letter>` and binds it with `set_binding`.

Step 2. `play_chart.py` takes `--activate <ms,ms,...>` (or `--activate-from <score JSON>`, which takes every activation's ms from the JSON's `activations` and the chord ms of its `act_tick`). A pure function `merge_presses(chords, activations_ms)` returns one timeline of presses in ms order, each either a chord or an SP press; an SP press fires at its ms with the same 2 ms lead. This is the "activation timeline" rule and lives only here.

Step 3. `play_chart.py` takes `--log <csv>` and writes one row per press: `index` (chord index or `sp`), `chart_ms`, `clock_ms_at_press` (the raw clock the wait returned), `score_before`, `score_after`, `hit`. These column names are the contract Task A's comparer reads. The printed table stays as it is.

Step 4. The engine find gets a cap: 120 s, then "no playing song found; start the song and run again", exit 2. The run ends on the frozen clock as today (5 s) or at 240 s of wall time, and prints the hit count, miss count and best streak as today.

Step 5. `score_compare.py`: pure logic. Inputs: the press log and the `hydra_replay score --path` JSON. For each chord it compares the game's `score_after` with Hydra's `cum.total` at that chord, allowing a one-chord lag (a 5 ms read can land before the engine credited the press, so a value that matches the previous chord's total is "lagged", not "wrong"); it prints the first chord where the two totals disagree beyond a lag and the two numbers, the count of matching chords, and the final totals side by side. It also prints clock statistics: the median and the 5th and 95th percentiles of `clock_ms_at_press - chart_ms` (expected: about -2 to +2 ms, the lead plus one poll), and the hit rate. A run with a hit rate under 98% prints a `FAIL` line naming the first miss.

Step 6. Tests: `merge_presses` on three chords and two activations (order, ties: a chord and an SP press at the same ms go chord first); the log row shape on the existing `_DUMP` fixture through a fake clock and fake driver (no process); `score_compare` on a hand-written log that matches, one that lags by one chord (passes), and one where the game's total is 50 points short from chord 3 on (fails at chord 3). Run `py -m unittest tools.ch_probe.tests.test_play_chart` and `py -m unittest tools.ch_probe.tests.test_score_compare`.

### Task C: the checksum finder, the read-only open, and the Il2Cpp dump

Owns `tools/ch_probe/process.py`, new `tools/ch_probe/experiments/find_checksum.py`, new `tools/ch_probe/il2cpp_strings.py`, new `tools/ch_probe/tests/test_find_checksum.py`, new `tools/ch_probe/tests/test_process_readonly.py`. Does not touch `constants.py` or `engine.py` (Task A owns them); the candidate offsets this task finds go into the write-up, not into `constants.py`, until the sitting proves them.

Step 1. `process.open_process(read_only=False)`: with `read_only=True` it asks for `PROCESS_VM_READ | PROCESS_QUERY_INFORMATION` only and installs a writer that raises `PermissionError("process opened read-only")`. The default stays as it is so the debugger runners keep working. Test: a `Process` built with the read-only writer raises on `write` and reads normally; the access mask constant for read-only is a literal in the test.

Step 2. `il2cpp_strings.py`: pure helpers. `encode_forms(hash_hex)` returns the byte patterns to search for: the 16 raw bytes, the upper-case hex as UTF-16LE, the lower-case hex as UTF-16LE, and both as ASCII. `decode_il2cpp_string(raw)` reads an Il2Cpp `System.String` object (length as i32 at `+0x10`, UTF-16LE chars from `+0x14`) and returns the text or `None` when the length is absurd (over 1,024). Tests pin each form as a literal for the known hash `234C917654CC7257E2BAFCF7D761304B`.

Step 3. `find_checksum.py`: open read-only, `verify_targets`, find the live engine (cap 120 s as in Task A), then:
- Scan all committed read-write regions (reuse `engine_finder.scan_for_engine`'s region walk, refactored into a `walk_regions(proc)` generator in `engine_finder.py` only if Task C may touch it; otherwise the finder copies the 30-line walk and the write-up notes the duplication for Phase 1 to fold) for every form of the chart's hash, given on the command line as `--hash`. Print every hit: address, form, and the 32 bytes before it. Cap: one pass, which is the same cost as the engine scan.
- For each UTF-16 hit, work out the candidate `System.String` object address (hit minus `0x14`) and then look for a pointer to it: first in the engine object's first `0x400` bytes, then in every object the engine's pointer fields point to (one hop, each read as `0x400` bytes), then two hops. Cap: 2 hops and 4,096 objects read; when hit, print "pointer path not found within 2 hops" and still exit 0 with the direct hits listed. Each found path prints as `engine+0xAA -> +0xBB -> +0xCC = <string>`.
- Print how long the text scan took and how long the pointer walk took.
- Repeat the direct scan once more 10 s later and say whether the same addresses still hold the hash (a stable location is a weaker but still useful reader).

Tests: the pointer walk over fake memory (a fake engine with a pointer to a fake object that points to a fake string) finds the two-hop path and reports the exact offsets; a fake with no path hits the cap and reports not found; hits at the region edge are not double-counted.

Step 4. The Il2Cpp dump, offline, before the sitting. Make an empty folder in the scratch area and run `C:\Users\Patrick\Downloads\Il2CppDumper-net7-v6.7.46\Il2CppDumper.exe "C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll" "C:\Clone Hero\1.1 june\Clone Hero\Clone Hero_Data\il2cpp_data\Metadata\global-metadata.dat" <that folder>` from the tool's own folder with absolute paths. Cap: 10 minutes; if it has not written `dump.cs` by then, stop and report the last console line. Then grep `dump.cs` for the class whose fields sit at `0x68`, `0x1B0`, `0x42`, `0x48`, `0x120` and `0x130` (the engine class; the dump prints `// 0x68` style offsets) and copy its field lines into the report, with the field types and widths of `+0x42` and `+0x48`. Then grep for fields named like `checksum`, `hash`, `md5`, `Checksum`, `songHash`, `chartHash` and for the class that holds `sp_bar_ticks`-like names (`SpBar`, `starPowerBar`, `sp_bar`), and copy every hit with its class into the report. Say where you looked when a grep finds nothing. The dump itself stays in scratch; the report (the task's final message plus a section the write-up task pastes in) carries the lines.

Run `py -m unittest tools.ch_probe.tests.test_find_checksum` and `py -m unittest tools.ch_probe.tests.test_process_readonly` and the existing `py -m unittest tools.ch_probe.tests.test_process`.

### Task D: the cheap checks

Owns new `tools/ch_probe/experiments/log_tail.py`, new `tools/ch_probe/experiments/topmost_probe.py`, new `tools/ch_probe/experiments/poll_cost.py`, new `tools/ch_probe/tests/test_cheap_checks.py`, and the write-up template `docs/research/2026-10-28-overlay-spike-results.md` (headings only, one per question, with a "Go / No-go" line under each, filled by Task E).

Step 1. `log_tail.py`: finds the newest `Session-*.log` under `C:\Users\Patrick\Documents\Clone Hero\Logs`, then polls it every 100 ms for new bytes and prints each new line prefixed with the wall time to the millisecond. Pure function `parse_log_line(line)` returns `("loading", "Artist - Title")` for a `[Log] Loading Song: Artist - Title` line, `("countdown_start", None)`, `("countdown_end", None)`, `("seek", seconds)` for the lines the 2026-10-17 log shows, else `None`. Cap: runs until Ctrl-C or 300 s. The main session notes the wall time it said "go" and the wall time the Loading Song line appeared; the write-up reports the gap. Test: `parse_log_line` on the four real lines copied from the 2026-10-17 log, and on a warning line (returns `None`).

Step 2. `topmost_probe.py`: a plain Win32 window made with ctypes (`WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT`, `WS_POPUP`), 600 by 120 pixels at the top centre of the primary screen, half-transparent black with a white label "HYDRA OVERLAY PROBE hh:mm:ss" repainted each second, click-through. It closes itself after 60 s. It prints the window handle and exit style. LIVE-ONLY, no unit test beyond importing it and checking the style constants are literals; the user's eyes are the check, recorded in the write-up as "visible" or "not visible" for `fullscreen = 0` and `fullscreen = 1`.

Step 3. `poll_cost.py`: opens the process (read-only when Task C's option exists), times `engine_finder.find_live_engine` five times in a row (cap 120 s each, the five results must agree on the address), then times 1,000 `snapshot()` reads and 1,000 single `read_u32` reads, and prints min, median and max in ms for each. It prints the scan's regions and bytes from `scan_for_engine` too. Test: the timing report on fake callables with fixed durations prints the literal numbers.

Run `py -m unittest tools.ch_probe.tests.test_cheap_checks`.

## During the sitting (the main session)

The main session runs the scripts from the merged `main` in the order the sitting lists. Before the sitting it makes the `hydra_replay dump` and `hydra_replay score --path` JSON files for the Trippin chart (Expert, Pro Drums, 2x Bass, the user's rules file) and for NS E, prepares the `Trippin SP Spike` folder with `probe_songs.write_silent_ogg` at the chart's `song_length` (179,744 ms), and asks the user to copy it. It records which calibration each run used in the result file names. It stops any run at its cap and writes down the cap line the script printed.

Caps for the sitting as a whole: 40 minutes of the user's time, after which the main session stops and reports which questions have data. Each run's own cap is in its script.

## Task E: the write-up (one executor on Opus, after the sitting)

Owns `docs/research/2026-10-28-overlay-spike-results.md` and `tools/ch_probe/experiments/results/overlay-spike/` (the CSV, JSON and report files from the sitting, committed as they are). Runs `sp_compare.py` and `score_compare.py` on the files, pastes their reports, and fills in each question's section with the numbers and one of the three verdicts below. Every number in the write-up names the file it comes from. Pure reading and writing; it runs no live script and no build. It also adds a "What contradicted the known facts" list and the Il2Cpp class lines from Task C's report.

The verdicts, per question:

- Question 1 (SP fields): **go** when every phrase end is followed by a rise of the same size, `sp_max` is constant, the activation sets the flag, doubles the multiplier and sets an end time that matches Hydra's deactivation ms within 50 ms, and the end clears the flag; **go with notes** when the fields behave but the end time differs from Hydra's by more than 50 ms (the write-up says by how much and Phase 1 models it); **no-go** when any of the five fields does not move with the phrases or the activation, in which case Phase 1 must first find the real fields with the debugger path (`engine.capture_object`) before an overlay can show an SP bar.
- Question 2 (checksum): **go** when a pointer path from the engine object reaches the hash within 2 hops in both runs 1 and 3 and the class names from `dump.cs` agree with the offsets; **go with notes** when only a stable direct address was found (the overlay would re-scan once per song, which `poll_cost.py` prices); **no-go** when the hash is not in memory as any searched form, in which case Phase 1 reads the chart's folder path from the game instead (the log line, if live) and hashes the file itself.
- Question 3 (clock and score): **go** when the auto-play hit rate is at least 98%, the score totals match Hydra's chord for chord with at most a one-chord lag through the whole run including Star Power, and the clock-vs-chart median is within 5 ms in run 1; the calibration run says in words whether a 30 ms setting moved the clock (same hit rate and same median) or the notes (hit rate fell or the median moved by about 30 ms); **no-go** when the totals part ways at a chord that is not a lag, which means Hydra's scoring and the game's disagree on this chart and the "points vs optimal" figure is wrong until that is fixed (the write-up names the chord and the two numbers so it becomes a scoring issue).
- Question 4 (cheap checks): each check gets one line: `+0x2E0` is or is not the hit time; a pause shows as a frozen clock and nothing else moved, or not; the Loading Song line appeared N ms after "go" or only at song end; engine finding takes N ms and a snapshot read N ms; the topmost window is visible or not with `fullscreen = 1`. A **no-go** here only means the overlay design has to work around it (poll a stable address instead of the log, use `fullscreen = 0` or a borderless mode), and the write-up says which.

## Merge order

Tasks A, B, C and D run in parallel in separate worktrees and each merges on its own after its one reviewer. A and B share no file: A owns `constants.py`, `engine.py` and the SP scripts; B owns `play_chart.py`, `input_driver.py` and the score comparison; the only thing they share is the column names of the press log, listed in Task B step 3 and read by name in Task A step 4. C owns `process.py`; A, B and D call `open_process` with its default, so they work whether or not C has merged, and use the read-only option when it has. D owns only new files. The sitting runs after all four merge. Task E runs after the sitting. The main session runs what `hydra_precheck.exe --tests` prints for each merge; the Python tests are the ones each task names.

## Out of scope (needs the user's yes to leave out)

- Any Hydra C++ change, any overlay window beyond the 60 s topmost probe, and any reading of the player's own SP bar beyond what the five fields give.
- A non-zero song.ini `delay` (question 4 above). No candidate chart has one; Phase 1 adds `timing.delay_ms` to the chord times if the clock turns out to be the audio clock.
- Reading combo, difficulty, modifiers and song speed live. Task C's dump grep may name their classes; reading them is Phase 1.
- Guitar or any instrument other than drums.
- A purpose-built SP test chart (question 1); the Trippin chart covers it.
- Folding the region walk the checksum finder copies from `engine_finder.py` back into one function (Task C step 3 names the duplication).
- What `fullscreen = 1` means in the game's own terms (exclusive or borderless): the spike records only whether the topmost window shows.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| No Hydra C++ changes and no overlay UI in Phase 0 | The plain version; Out of scope | A, B, C, D, E | each task's `git diff --stat` touches only `tools/ch_probe/**` and `docs/research/**`; the reviewer refuses anything under `src/` |
| New readers open the process read-only where they can | Task C step 1; Task A step 3; Task D step 3 | C (A, D use it) | `test_process_readonly`: read-only `Process` raises `PermissionError` on `write`, reads normally; the access mask literal is pinned |
| The default open is unchanged for the debugger runners | Task C step 1 | C | existing `test_process` and `test_runners` pass unchanged |
| Pure logic has unit tests; live-only steps are named | every task | A, B, C, D | each task runs its named `unittest` file; every function that opens, scans or sends input carries LIVE-ONLY in its docstring (reviewer grep) |
| The SP offsets live in `constants.py` only and are marked unconfirmed | Task A step 1 | A | reviewer grep: `0x68`, `0x1B0`, `0x42`, `0x48`, `0x120`, `0x130` appear in `tools/ch_probe` only in `constants.py` and the Task C report text |
| `decode_snapshot` decodes every new SP field at its offset | Task A step 2 | A | `test_sp_watch`: hand-built `0x2E8`-byte block, literal values for each field |
| Existing `Snapshot` fields and `test_engine` are unchanged | Task A step 2 | A | `py -m unittest tools.ch_probe.tests.test_engine` passes with no edit |
| The watcher stops: 120 s engine cap, 5 s frozen clock, 240 s run cap | Task A step 3 | A | `test_sp_watch`: a fake finder that never finds exits 2 after the cap with the named message; a frozen fake clock ends the run; a wall clock past 240 s ends it |
| Every phrase end is followed by an SP rise, and the rise size is reported | Task A step 4 | A, E | `test_sp_watch`: compare passes on the matching fixture, fails on the missing-rise fixture; write-up quotes the rise ticks |
| `sp_max` is constant and compared with Hydra's meter and `sp_bar_ticks` | Task A step 4; "What the code does today" (14,400 vs 15,360) | A, E | `test_sp_watch`: fixture with a changing `sp_max` fails; the write-up prints the three numbers side by side |
| Activation sets the flag, doubles the multiplier, sets an end time; the end clears them | Task A step 4 | A, E | `test_sp_watch`: passes on the matching fixture; fails on "flag never sets", "end never clears" and "multiplier stays 4" fixtures |
| The game's multiplier equals Hydra's per-chord multiplier (one-chord lag allowed) | Task A step 4 | A, E | `test_sp_watch`: a fixture with the multiplier one chord late passes; one with 3 where Hydra says 4 fails |
| `+0x2E0` is judged with `HIT_TIME_MAX_GAP_MS` | Task A step 4 | A | `test_sp_watch`: a hit time 9 ms from the chord counts, 11 ms does not |
| A pause is reported from the clock and the SP fields are shown not to move | Task A step 4; sitting step 4 | A, E | `test_sp_watch`: fixture with a 2 s clock hold reports the span; write-up records run 3's pause |
| The auto-player presses the SP key at Hydra's activation times | Task B steps 1, 2 | B | `test_play_chart`: `merge_presses` order and tie rule; `--activate-from` takes the ms of each `act_tick` from the JSON (literal fixture) |
| A lane with no binding fails loudly | Task B step 1 | B | `test_play_chart`: `press_chord([Lane.STAR_POWER])` with no binding raises |
| The press log has the agreed columns | Task B step 3; Task A step 4 | B, A | `test_play_chart`: header literal; `test_sp_watch` reads a fixture with that header by name |
| The auto-player stops: 120 s engine cap, 5 s frozen clock, 240 s run cap | Task B step 4 | B | `test_play_chart`: fake finder cap exits 2 with the message; frozen fake clock ends the run; wall cap ends it |
| Score totals are compared chord for chord with a one-chord lag, first mismatch named | Task B step 5 | B, E | `test_score_compare`: matching log passes; lagged log passes; 50-short log fails at chord 3 with both numbers |
| Clock-vs-chart statistics and hit rate are reported; under 98% is FAIL | Task B step 5 | B, E | `test_score_compare`: median and percentiles on a literal fixture; a 97% fixture prints FAIL naming the first miss |
| The hash is searched in every form it could take | Task C step 2 | C | `test_find_checksum`: `encode_forms` pins five literal byte strings for the known hash |
| Il2Cpp strings are decoded, absurd lengths rejected | Task C step 2 | C | `test_find_checksum`: a fake string object decodes to the hash; length 5,000 returns `None` |
| The pointer walk finds a 2-hop path and reports exact offsets | Task C step 3 | C | `test_find_checksum`: fake memory with a 2-hop path prints `engine+0x.. -> +0x.. -> +0x..`; region-edge hits counted once |
| The pointer walk stops at 2 hops and 4,096 objects and says so | Task C step 3 | C | `test_find_checksum`: fake with no path reports "not found within 2 hops", exit 0 |
| The finder re-checks the direct hits 10 s later | Task C step 3 | C | `test_find_checksum`: fake memory that moves the hash between passes reports "moved" |
| The dump is regenerated and the class lines are quoted, with a 10 min cap | Task C step 4 | C, E | Task C's report holds the field lines for the engine class and every hash-named field, or the "where I looked" list; the write-up pastes them |
| Log lines are parsed from the real log's forms | Task D step 1 | D | `test_cheap_checks`: four real lines from the 2026-10-17 log and one warning line |
| The log tail and topmost probe stop on their own | Task D steps 1, 2 | D | reviewer reads the 300 s and 60 s caps; `test_cheap_checks` pins the two literals |
| Engine finding and polling costs are measured and reported | Task D step 3 | D, E | `test_cheap_checks`: the report on fake timings prints literal min, median, max; write-up records the live numbers |
| The five engine finds agree on one address | Task D step 3 | D | `test_cheap_checks`: fake finder returning two addresses makes the script print a disagreement line |
| The user's part is one sitting with listed steps and times, and never drums | "The sitting" | main session | the sitting list; the write-up records which steps ran and how long each took |
| The sitting as a whole stops at 40 minutes | "During the sitting" | main session | the write-up says whether the cap was hit and which questions have data |
| Each live run has a cap in runs | sitting steps 2 to 5 | main session | the write-up records run counts against the caps (1 run, plus 1 bot run or 1 repeat where named) |
| Only installed test charts or `testdata/input` charts are used; nothing over the library | Open question 1; sitting step 1 | main session, E | the write-up names the two charts and their sources; no script takes a folder argument wider than one song |
| The chart install is the user's copy, not a write by the session | Open question 5; sitting step 1 | main session | the sitting list says so; the write-up records who copied it |
| The calibration run answers whether the clock or the notes moved | Open question 3; verdicts | E | the write-up's question 3 section states one of the two outcomes with the hit rate and median from both runs |
| Results go to `docs/research/` and `experiments/results/overlay-spike/` | Open question 6; Task E | E | the files exist at those paths in Task E's commit |
| Every question has a go, go-with-notes or no-go line with the numbers that decided it | Task E verdicts | E | the doc reviewer checks each verdict against the pasted compare reports |
| Known facts the data contradicted are listed | Task E | E | the write-up has the "What contradicted the known facts" list, starting from the three in this plan (no SP phrases in the NS charts, 14,400 vs 15,360, the bot) |
| Song.ini `delay`, combo, modifiers, speed, other instruments, a purpose-built chart, the region-walk fold, and the fullscreen mode's meaning are left out | Out of scope | none | needs the user's yes |
