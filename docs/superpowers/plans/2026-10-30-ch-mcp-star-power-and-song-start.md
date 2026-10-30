# Plan: Star Power reads and a song start without the user, for the `ch` MCP server (2026-10-30)

Task id `CHSP-PLAN`, session `9a82ff65-26fd-45d5-8f6c-5766bbb06a37`. Plan only; no code changed. Every path, number and claim below was checked today against the files named beside it, on main's tip 9f007234, unless it says otherwise.

## The plain version

The user built the `ch` server so an agent could test Clone Hero with nobody at the keyboard. Two things stop that today, and this plan adds both.

First, the server cannot read Star Power. It needs the meter amount, the full-bar size, whether Star Power is on, when it ends, and the multiplier, both as one read and as a watch over a whole run. The overlay spike already found and proved those five fields from outside the game on 2026-10-28: `sp_watch.py` logged 40,922 rows of them during one song, and `sp_compare.py` checked them against Hydra's path with 31 PASS and 3 FAIL, where every FAIL was a fill the game offered at a different place than Hydra's path, not a wrong field (`tools/ch_probe/experiments/results/overlay-spike/run1_cal0_sp_compare.txt`). So this plan does not find anything new. It moves the proven watcher into the server as tools, and gives it one place to live.

Second, the server cannot start a song that keys can play. A command-line launch always adds a bot, and the bot never uses Star Power. The only way a key-played song has ever started is the user picking it in the menu. This plan surveys every real way to start one without a person at the menu, and recommends driving the menus by key presses through the same `SendInput` path `ch_press` already uses, with a state read after every press so the server knows where it is. That breaks one written rule ("no tool opens any menu"), so question 1 puts it to the user.

The first use is a fill check on the Senri Kawaguchi chart: its written fills skip a drum solo from 114.6 s to 166.5 s, Star Power comes ready inside it, and the question is whether the game offers a fill there anyway. Hydra says no (`Song::check_activations` in `src/core/song.cpp` generates no fills when a chart has any). Live step LS3 is that check, with pass and fail lines and a cap.

## Questions for the user

Each has my recommendation first, then the strongest case against. Say "go with the recommendations" to approve them all. Questions 1 to 4 change what gets built.

**1. Tools that open menus.** `docs/agents/clone-hero-mcp.md` says, under "Safety rules": "Play stays offline. The server launches the game only with the command line `bot_run.launch_command` builds, and no tool opens any menu. The user picks songs in the menu by hand." The server plan's question 1 set that because nothing had proved another way to reach a key-played song. The only non-bot route this plan found is the game's own menus (see "Every way to start a song"). Recommendation: change the rule to "no tool opens the Online menu; `ch_open_song` drives only the main menu, Quickplay's song list and the player screen, by keys, and refuses to press anything while the menu read says it is anywhere else". The offline guarantee stays what it was: the plan's own words say "the network adapter off is the only hard guarantee". Both ends: `ch_open_song("C:\Clone Hero\songs\Hydra Probe\Window Map")` presses Enter, types "Window Map" into the search, confirms, and the song starts with the user watching but not touching anything; a run where the menu read says the Online menu is open stops at once with "refused: the Online menu is open", and presses nothing. The case against: a key sequence that goes wrong in a menu could change a setting or a profile; the answer is the read after every press, the per-step cap, and `ch_stop`'s restore of `settings.ini` and the four profile and score files, which already undoes any such change (`game_files.py`, the five files). If you keep the rule as written, the song-start half of this plan is dropped and the user keeps starting songs by hand; the Star Power half still goes ahead.

**2. Calling the game's own methods.** The same page says "The Frida agents only read and trace. They never write game memory, set a field or call a game method", and `tools/ch_mcp/tests/test_frida_attach.py` refuses any agent that matches `.invoke(` or `new NativeFunction`. The feasibility study imagined "pick a song and difficulty for a human-style player through `SongSelect`" (`docs/research/2026-10-28-clone-hero-mcp-feasibility.md`, "Song control"). Recommendation: do not do it, and keep the rule and its test unchanged. Two reasons from the data: the rule exists because of the EULA ruling and the user's "Proceed, my call" covered reading and hooks only; and `SongSelect` has 72 methods of which 5 are readable (`Start`, `OnEnable`, `OnDisable`, `Update`, `Awake`, from `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\facts.json`), so nobody knows which one starts a song, and finding it is a hooking study of its own. Both ends: with the rule kept, a song starts only through key presses the game treats like a player's; with the rule dropped, an agent could call an obfuscated method that starts the song in one step, but the first wrong guess could do anything from changing the selection to crashing the game. The case for dropping it: it would be faster and would not depend on the menu layout. I recommend against.

**3. Three new numbers for the menu driver, and one changed cap.** (a) `MENU_STEP_CAP_S = 10`: after each press, the menu read must show the change the step expects within 10 s, else the driver stops and reports the step and the read. 10 s is a guess, chosen to be well over the 4.3 s the "Loading Song" log line took to appear after the user pressed Play in run 1 (`run1_cal0_log_tail.txt`), which is the slowest menu transition anyone has timed. (b) `MENU_KEY_GAP_S = 0.25`: the pause between two presses inside one step (typing a name, or Down several times), a guess with no measurement behind it; LS1 records whether a shorter gap drops keys. (c) `SEARCH_DOWN_CAP = 10`: when the search result the read shows is not the target folder, the driver presses Down at most 10 times before it stops with "the target is not among the first 11 results". (d) The run cap. `play_chart.RUN_CAP_S` and `sp_watch.RUN_CAP_S` are both 240 s of wall time, and the Senri song is 260,504 ms long (`song_length` in `C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check\song.ini`), so both would stop 20 s before its end. The solo question is settled by 166.5 s, so LS3 still works. Recommendation: the server passes a cap of the song length plus `bot_run.POLL_CAP_EXTRA_S` (60 s, already approved) when it knows the song folder, and the modules keep 240 s as their default for the command line; no new number, one new place that reads an old one. The case against: 240 s was an approved number and raising it means a stuck run waits longer before it stops; the stall check (`live.STALL_S`, 5 s of frozen clock) still ends a run whose song stopped.

**4. The first menu key sequence is a guess until LS1.** The game's manual gives Enter for Start, A for Green (confirm), S for Red (back), the arrow keys for up and down, and "a short press on the Blue fret opens the search tool" ([Keyboard and Gamepads](https://wiki.clonehero.net/books/guitars-drums-controllers/page/keyboard-and-gamepads); [How to Play](https://wiki.clonehero.net/books/clone-hero-manual/page/how-to-play)). It does not say which keyboard key the search box takes text from or what the player screen's Ready press is, and the user's drums profile binds the pads to A, S, J, K, L, U (`input_driver.DEFAULT_BINDINGS`), which are the guitar frets' letters. Recommendation: T6 writes the sequence as data (`tools/ch_mcp/menu_steps.json`) from the manual, marked unverified, and LS1 walks it one press at a time with the user watching and corrects the file in a one-file data commit (reviewed like any other). The case against: writing a guessed sequence at all; the alternative is for the user to narrate the clicks first. I recommend the data file because LS1 fixes it in one sitting either way.

Everything else below is decided and recommended.

## What exists today (checked in the files)

The Star Power fields are read from outside the game. `tools/ch_probe/constants.py` holds `OFF_SP_AMOUNT = 0x68`, `OFF_SP_MAX = 0x1B0`, `OFF_SP_ACTIVE = 0x42`, `OFF_SP_MULTIPLIER = 0x48`, `OFF_SP_END_TIME = 0x120` and `OFF_SP_OVERFLOW = 0x130`, each still commented "not yet confirmed live". `engine.decode_snapshot` turns one `SNAPSHOT_SIZE` read of the engine object into a `Snapshot` with `sp_amount_u32`, `sp_amount_u64`, `sp_max_u32`, `sp_max_u64`, `sp_active_byte`, `sp_active_u32`, `sp_mult_byte`, `sp_mult_u32`, `sp_end_s` and `sp_overflow_u32`, beside `clock_s` and `score`. `EngineModel.snapshot()` does that read. The Il2Cpp dump settled the widths (`il2cpp-fields.md`, "The six Star Power offsets"): the amount and the maximum are 8-byte `long`s, the flag at `0x42` is one byte with three other flags right after it, the multiplier a 4-byte `int`, the end time a `double`. `sp_compare.DEFAULT_WIDTH = 64` and it reads the flag from `sp_active_byte` for that reason.

`sp_watch.watch(snapshot, write_row, *, now, sleep, poll_s=0.004, cap_s=240.0)` is pure: it polls every 4 ms, writes a row only when a logged field changed, and stops on a frozen or backward clock (`live.StoppedCheck`), a failed read, Ctrl-C or the cap, returning a `WatchSummary` with the reason. `sp_watch.run` is the command line around it: open read-only, `verify_targets`, `engine_finder.find_live_engine_capped` (120 s, `ENGINE_SEARCH_CAP_S`), then `watch`. Run 1 of the sitting ran it beside `play_chart.py`, each with its own engine search, and the presser still hit 857 of 857 (`run1_cal0_play_chart.txt`), so two scans at once did not hurt the presses.

`sp_compare.compare(rows, score, *, resolution, sp_bar_ticks=None, width=64) -> Report` is pure and prints PASS and FAIL lines; `_find_activations(rows)` finds each 0-to-1 flip of the flag; `--resolution` is required only to print Hydra's full meter beside `sp_max`. Run 1 proved the fields: 11 phrase ends each raised the amount by 1,438 to 1,440; `sp_max` held at 5,760 (30 times the chart's resolution of 192, where Hydra's meter is 32 times); each activation set the flag, doubled the multiplier 4 to 8, and set an end time at which the flag cleared; the two activations on the path ended within 2.9 and 1.2 ms of Hydra's deactivation (`run1_cal0_sp_compare.txt`, lines 2 to 27). The write-up `docs/research/2026-10-28-overlay-spike-results.md` is still the empty template, so the result file and the sitting handoff (`docs/handoffs/2026-10-28-overlay-spike-sitting-handoff.md`) are the record.

The server's Frida side reads `GameManager` by field name only (`agent/state.ts`: `songTime`, `isPaused`, `isSongOver`, `isSongPlaying`, `songLoaded`), finds it on every read with a heap walk that L1 timed under 0.1 s, and `frida_tools.state_words` turns the fields into one word. `ch_snapshot` adds the score after the play session's last press. Nothing in the server reads the engine object. `test_frida_attach.py`'s `NO_WRITE_PATTERNS` refuses, in every `agent/*.ts`, writes, `.value =`, `.invoke(`, `new NativeFunction`, `Interceptor.replace` and `add(0x` (reads by offset).

The menus are readable by name. `facts.json` lists `SongSelect` with readable fields `currentSong`, `mainMenu`, `playerSelection`, `onlineMenu`, `songSelectSearch`, `songOptions`; `MainMenu` with `songSelect`, `settingsMenu`, `onlineMenu`, `songScan`, `controlMapper`; `PlayerSelectionMenu` with `playerIndex`, `instrumentSelection`, `difficultySelection`, `confirmSelection`; `GlobalVariables` with a static `instance` and `currentSongEntry`, `selectedSong`, `gameMode`, `playerCount`, `isPracticeEnabled`; `SongEntry` with `folderPath`, `chartName`, `songLength`, `checksum`. Every field entry in `facts.json` carries `name`, `type`, `static` and `offset`.

Keys reach the game. `input_driver.InputDriver.send_key(vk, key_up)` is the one `SendInput` call, `press_chord` presses lanes, `find_game_window` and `focus_window` are the window owners, and `play_chart.live_press` is "focus, then press". `ch_press` takes lane names only; there is no way to send Enter, an arrow or text. L3 proved `SetForegroundWindow` and `SendInput` work from the server.

The command line is `launch_command`: `[exe]` for the menu, else `--song`, `--player` and an optional `--profile`. The game's string table (`stringliteral.json` from the 2026-10-28 Il2CppDumper run, in session f756d547's scratch) holds exactly these game flags: `--song`, `--player`, `--profile`, `--instrumentNames`, `--versus`, `--replay`, `--lbsToken`, and the short forms `-i`, `-p`, `-r`; everything else starting with `--` is Unity UI style-class text. `--replay` and `--lbsToken` are not in the manual and nobody has tried them.

## Every way to start a song without a person at the menu

I looked for every route and checked each against the evidence. The recommendation follows.

**The command line with a bot.** Proven (Test 1 of the feasibility study): the game plays the chart with a bot and writes `scorestats.json`, and "the bot never uses Star Power. In both Trippin runs (11 phrases) it earned 0 phrases, with 0 activations". The manual says `--player` "adds a bot player" and `--profile` is "a template for all of the bot players" ([the command-line page](https://wiki.clonehero.net/books/clone-hero-manual/page/chart-previews-via-the-command-line)). So it cannot answer any Star Power question, and its results screen took no keys in L1.

**The command line with `--song` and no `--player`.** Untried. The manual does not say what the game does with a song and no players. The game's `CliGameLauncher` class has no readable method, so the dump does not say either. It costs one launch to find out, so LS0 tries it once: if the game opens the song for the keyboard profile and `is_bot` is false in the stats, this is the simplest route of all and `ch_open_song` is never needed for charts that are installed. I rate it unlikely, because the page describes the feature as a preview by bots, but a guess is not evidence.

**`--replay`.** A flag in the string table and a `replayPath` field on `GlobalVariables`. A replay plays back recorded inputs, not live keys, so even if it loads a song it does not give a key-played run. Left out.

**The game's own methods through Frida.** Question 2. Not recommended; it breaks the no-writes rule and the dump does not name the method.

**Key navigation of the menus, checked by reads.** The game opens at its main menu with `ch_start()` (proven, L2 and L3). The manual documents the menu keys (Enter for Start, A to confirm, arrows to move, Blue for the search tool), and the song list can be searched by name. Every state the driver needs is a readable field: whether a `SongSelect` or `MainMenu` object is live, which `SongEntry` is `currentSong` and its `folderPath`, and whether a live `GameManager` exists with `songLoaded`. No public tool does this for Clone Hero; the general Unity UI drivers (AltTester, for one) need the game built with their SDK ([AltTester overview](https://alttester.com/docs/desktop/v.2.0.0/pages/overview.html): "The instrumented build opens a communication protocol"), which a shipped game is not, and the community Frida servers expose raw JavaScript, which the no-writes rule forbids. What the server already has, `SendInput` to a focused window plus a Frida read of named fields, is the same shape AltTester uses from the inside. **This is the recommendation**, with question 1's rule change.

**The results screen.** A song started from the menu ends on a results screen that takes keys (L2 run 1: the user went back to the main menu by hand), so `ch_open_song` can be called again for a second song in one game process; after a bot launch the results screen took no keys (L1), so that route still ends with `ch_stop`.

## The design

### Where the Star Power read lives, and who calls it

The read lives outside the game, in `ch_probe`: `engine.decode_snapshot` says where each field is and how wide (through `constants.OFF_SP_*`), `EngineModel.snapshot()` reads it, and `sp_watch.watch` polls it. Nothing new reads memory. The server gets one new module, `tools/ch_mcp/sp_tools.py`, with a class `SpTools` beside `GameTools` and `FridaTools`. It runs `watch` in a daemon thread, keeps every row and the latest `Snapshot` in memory, writes the CSV live in the session folder, and formats the fields in one function, `sp_lines(snapshot)`. `ch_sp` prints the latest row the watcher read (at most one poll old), so the single read never touches the process from a second thread and never blocks on an engine search. The alternative was reading the fields inside the game through the Frida agent, resolving the scrambled field names from `facts.json` by offset the way the trace resolves method names; it is possible (the fields are listed with offsets, and `BasePlayer.engine` is a readable path to the object) but unproven live, it would run a 4 ms timer inside the game, and it would add a second home for "where the Star Power fields are". The outside read is proven and already has one home.

The watcher finds the engine the way `sp_watch.run` does today, with its own capped search. It does not share the play session's engine address, because the play session's `EngineModel` lives inside `live_play` and exposing it would make the watcher depend on a presser. Two searches at once are proven harmless (run 1).

### The tools

Every tool returns plain text cut by `cap_reply`. The owner column names the code that does the work.

| Tool | Owner of the work | Returns |
|---|---|---|
| `ch_sp_watch_start(song_folder=None, out=None)` | `sp_tools.SpTools`; `sp_watch.watch`; `engine_finder.find_live_engine_capped` | "watching: waiting for a moving clock (cap 120 s)", the CSV path, and the run cap it will use (question 3d) |
| `ch_sp` | `sp_tools.sp_lines` over the latest `Snapshot` | engine clock in ms, amount, max, amount as a fraction of max and whether it is at or over the ready level (`kSpActivationBars` 2 of `kCloneHeroSpCap` 4, so half), active flag, end time in ms and how far ahead of the clock, multiplier, overflow flag; or "no engine read yet (the watcher is waiting for a moving clock)"; or "no watcher has run; call ch_sp_watch_start" |
| `ch_sp_watch_status(since_ms=0)` | `sp_compare.find_activations`, `sp_compare.phrase_rises` over the rows | rows so far, every activation (flip clock, multiplier before and after, end time, when the flag cleared), every rise of the amount with its size, the engine address, and the `WatchSummary` reason when it ended |
| `ch_sp_watch_stop` | the stop flag `watch` checks (T1 adds `should_stop`) | the summary line and the CSV path |
| `ch_sp_compare(watch_csv, score_json, resolution=None, sp_bar_ticks=None, width=64)` | `sp_compare.compare` | the report text |
| `ch_sp_between(start_ms, end_ms)` | `sp_compare.flips_between`, `sp_compare.ready_between` | whether the flag flipped on inside the window (each flip's clock and the nearest chord when a score JSON was given to the watcher), and the highest amount read inside it against the ready level |
| `ch_key(keys)` | `input_driver.MENU_KEYS`, `InputDriver.press_key`, `type_text`; `play_chart.live_press`'s focus | the keys sent; refuses an unknown name with the list |
| `ch_menu` | `agent/state.ts` menu block; `frida_tools.menu_lines` | which menu objects are live, the selected song's folder and name, `gameMode`, `playerCount`, the live-`GameManager` word |
| `ch_open_song(song_folder)` | `menu_tools.MenuTools` over `menu_steps.json`, `ch_key` and the menu read | one line per step: the keys, the read it waited for, how long it took; or the step it stopped on and the read it saw |

`ch_snapshot` is unchanged: `ch_sp` is the single read, and its row carries the engine clock, so the two clocks can still be compared row against read.

### Who owns which question

| Question | Owner | The other side |
|---|---|---|
| Where each Star Power field is and how wide | `constants.OFF_SP_*`, `engine.decode_snapshot` | `sp_tools` prints `Snapshot` fields by name |
| When a watch stops (frozen clock, failed read, cap, stop flag) | `sp_watch.watch` | `sp_tools` reports the `WatchSummary` reason |
| The engine search and its cap | `engine_finder.find_live_engine_capped` | `sp_tools` reports the timeout message |
| The run cap for a known song | `bot_run.song_length_ms` plus `bot_run.POLL_CAP_EXTRA_S`, read in `game_tools.run_cap_s(song_folder)` (T3) | `sp_tools` and `ch_play_chart` pass it down |
| What an activation and a rise are in the rows | `sp_compare.find_activations`, `sp_compare.phrase_rises` | `sp_tools` prints them |
| Whether the flag flipped inside a window, and the highest amount there | `sp_compare.flips_between`, `sp_compare.ready_between` | `ch_sp_between` prints them |
| The ready level | `kSpActivationBars` and `kCloneHeroSpCap` in `src/core/timing.h` and `src/core/model.h`, copied into `sp_compare` beside the three constants it already copies, with the owners named | `sp_lines` reads `sp_compare.READY_FRACTION` |
| Which keys menus take, and their codes | `input_driver.MENU_KEYS` | `ch_key` parses names through it |
| The menu step list | `tools/ch_mcp/menu_steps.json` | `menu_tools` runs it; LS1 corrects it |
| Which menu objects are live and which song is selected | `agent/state.ts` (reads by name) | `frida_tools.menu_lines` prints them and never names a menu state of its own |
| Is the Online menu open | `state.ts` reports `onlineMenuLive`; `menu_tools` refuses on it | |
| The three new numbers | `menu_tools.py` | |

### Safety rules kept

Backups: `ch_sp_watch_start`, `ch_key` and `ch_open_song` call `game.ensure_backup()` before they touch the game, like `ch_press`. `ch_stop` restores as today. Offline: `ch_open_song` refuses when the read says the Online menu is live, and `launch_command` is untouched. Frida: `state.ts` gains reads by name only; the no-writes grep covers it with no change. Caps: every new wait has one and names it in its reply. No whole-library anything: every tool takes one folder. The user is present and watching every live step; they press nothing, which is the point.

## Tasks

Each task is one executor on Opus, under 100 tool calls and 20 minutes, in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`, owning only the files it names. Unit tests run on fakes and never start the game. No task launches the game or attaches Frida. Tasks whose files sit under `tools/` run the single-owner scan once before committing (`build-cpp\Release\hydra_tests.exe "-sf=*single_owner*"`), and remember it reads the main checkout (`repo-scans-read-main-checkout`), so pre-scan the branch by the rules in `tests/single_owner/rules/probe.json`.

### T2: the pure Star Power checks (no blockers)

Owns `tools/ch_probe/experiments/sp_compare.py` and `tools/ch_probe/tests/test_sp_compare.py`.

Make `_find_activations` public as `find_activations` (the old name stays as an alias so nothing else moves). Add `phrase_rises(rows, amount_col)`: every positive step of the amount between consecutive rows, as (clock ms, before, after). Add `flips_between(rows, start_ms, end_ms)`: the activations whose flip clock lies inside the window. Add `ready_between(rows, start_ms, end_ms, amount_col, max_col)`: the highest amount inside the window, `sp_max`, and whether the amount reached `READY_FRACTION` of it. Add `READY_FRACTION = SP_ACTIVATION_BARS / SP_CAP_BARS` with `SP_ACTIVATION_BARS = 2` beside the existing copied constants, naming `kSpActivationBars` in `src/core/timing.h` as the owner the way `SP_CAP_BARS` names `kCloneHeroSpCap`. Make `resolution` optional in `compare` and `main`: with none, the `sp_max` line prints "Hydra full meter: not computed (no resolution given)" instead of the number. Tests on hand-written rows: `phrase_rises` on a three-step fixture gives three sizes; `flips_between` with a flip inside and one outside returns only the inside one; `ready_between` passes at exactly the half point and fails one unit below it (the boundary rule); `compare` with no resolution prints the "not computed" line and every other check unchanged; the existing tests pass unedited. Run: `py -m unittest tools.ch_probe.tests.test_sp_compare`.

### T1: the Star Power tools (blocked on T2; `earlyStart: T2's executor branch`)

Owns `tools/ch_mcp/sp_tools.py` (new), `tools/ch_mcp/tests/test_sp_tools.py` (new), and `tools/ch_probe/experiments/sp_watch.py` plus `tools/ch_probe/tests/test_sp_watch.py` for one change: `watch` gains `should_stop: Callable[[], bool]`, checked where the cap is, ending the run with reason "stopped".

`SpTools(game, lock, *, opener=open_process, find_engine=sp_watch.find_engine_live, watch=sp_watch.watch, now, sleep)`. `ch_sp_watch_start` refuses when a watch is running; otherwise takes `game.ensure_backup()`, makes the CSV path (`out` if given, else `sp_watch-<n>.csv` in `game.session_dir()`), picks the cap through `game.run_cap_s(song_folder)` (T3; until T3 merges, a fake returning 240 in the tests, and `sp_watch.RUN_CAP_S` when the folder is None), and starts a daemon thread that opens the process read-only, calls `verify_targets`, finds the engine (an `EngineSearchTimeout` ends the thread with that message as the reason), builds the `EngineModel`, and runs `watch` with a `write_row` that appends to the row list, updates `latest`, and writes the CSV line. `ch_sp` prints `sp_lines(latest)`. `ch_sp_watch_status`, `ch_sp_watch_stop`, `ch_sp_compare` and `ch_sp_between` are as the table says; `ch_sp_between` uses the rows in memory. It registers a stop hook with `game.add_stop_hook` that sets the stop flag and joins the thread for `READ_CAP_S`. Nothing in it names an offset, a probe song or a `1000` factor (`C.s_to_ms`).

Tests on a fake opener, a fake finder and a scripted fake `snapshot`: a start then a stop ends the thread with reason "stopped" and the CSV holds the header plus one row per change; `ch_sp` before the engine is found says so; `ch_sp` after two scripted snapshots prints the second's amount, max, fraction, ready word, flag, end time and multiplier; a finder that raises the timeout puts its message in the status; `ch_sp_between` on a scripted flip inside the window reports it and on none says "no activation inside"; a second start while running refuses; the stop hook stops the thread; `watch` with `should_stop` returning True after two polls ends with reason "stopped" (in `test_sp_watch.py`) and every existing `test_sp_watch` case passes unedited. Run: `py -m pytest tools/ch_mcp/tests/test_sp_tools.py -q` and `py -m unittest tools.ch_probe.tests.test_sp_watch`.

### T3: menu keys and the run cap (no blockers)

Owns `tools/ch_probe/input_driver.py`, `tools/ch_probe/tests/test_input_driver.py`, `tools/ch_probe/experiments/play_chart.py`, `tools/ch_probe/tests/test_play_chart.py`, `tools/ch_mcp/game_tools.py` and `tools/ch_mcp/tests/test_game_tools.py`.

`input_driver.MENU_KEYS` is the one table of non-lane keys by name: `enter` 0x0D, `escape` 0x1B, `space` 0x20, `backspace` 0x08, `tab` 0x09, `up` 0x26, `down` 0x28, `left` 0x25, `right` 0x27, each with a comment naming the manual page as the source for the menu meaning. `InputDriver.press_key(vk, *, hold_s=KEY_HOLD_S, sleep)` presses one key through `send_key`, and `type_text(text, *, gap_s, sleep)` sends each character as a Unicode key event (`KEYEVENTF_UNICODE`, 0x0004, with `wScan` the character and `wVk` 0), the standard way to type text with `SendInput`. `play_chords` and `live_play` gain `cap_s=RUN_CAP_S`, so a caller can pass a longer cap; the default and the command line stay 240 s. `game_tools.run_cap_s(song_folder)` returns `bot_run.song_length_ms(folder)` in seconds plus `bot_run.POLL_CAP_EXTRA_S` when the folder has a length, else `play_chart.RUN_CAP_S`; `ch_play_chart` gains `song_folder=None` and passes `run_cap_s` down. `ch_key(keys)` takes a comma list of `MENU_KEYS` names or `text:<string>`, focuses through `live_press`'s focus, presses in order with `MENU_KEY_GAP_S` between (read from `menu_tools` once T6 lands; until then a module constant here with the same name that T6 moves; the brief says which), and refuses an unknown name with the list. `ch_press` is unchanged.

Tests: `press_key` on the recorder fake sends down then up; `type_text("Wi")` sends two Unicode events with the right flag; `play_chords` with `cap_s=1` and a fake clock past 1 s ends with the cap reason, and with the default the existing 240 s test passes unedited; `run_cap_s` on a folder with `song_length` 260504 gives 320.504 and on a folder without one gives 240; `ch_play_chart(song_folder=...)` passes the cap to the fake `live_play`; `ch_key("enter,down,text:Window Map")` focuses once, sends Enter, Down, then the eleven characters; `ch_key("f1")` refuses with the list; every existing test in the four test files passes unedited. Run: `py -m unittest tools.ch_probe.tests.test_input_driver`, `py -m unittest tools.ch_probe.tests.test_play_chart`, `py -m pytest tools/ch_mcp/tests/test_game_tools.py -q`.

### T4: the menu read (no blockers)

Owns `tools/ch_mcp/agent/state.ts`, `tools/ch_mcp/agent/menu.ts` (new), `tools/ch_mcp/frida_tools.py` and `tools/ch_mcp/tests/test_frida_tools.py`.

`menu.ts` exports `readMenu()`: for each of `MainMenu`, `SongSelect`, `PlayerSelectionMenu`, the count of live instances (`Il2Cpp.gc.choose`, skipping destroyed ones through the same `m_CachedPtr` test `gamemanager.ts` uses); from the first live `SongSelect`, `currentSong`'s `folderPath` and `chartName` when it is not null, and whether its `onlineMenu` object is live (`onlineMenuLive`); from `GlobalVariables`' static `instance` (read as `klass.field("instance").value`, a read by name), `currentSongEntry.folderPath`, `gameMode` and `playerCount`. Every read is by field name; no method is called. `state.ts` adds the result as a `menu` block to every reply. `frida_tools.menu_lines(menu)` prints it, `ch_menu` is one read printing those lines, and `read_state()` is a public method returning the raw dict for `menu_tools`. `state_words` is unchanged. The executor builds `state.js` with `npm run build:state` in its own worktree only and reports the output.

Tests: the no-writes grep (`test_no_agent_writes_to_the_game`) finds `menu.ts` and passes; `menu_lines` on a hand-written block prints the folder, name, counts and `onlineMenuLive`; `ch_menu` on the fake agent prints them; `read_state` returns the fake's dict; every existing `test_frida_tools` and `test_frida_attach` case passes unedited. Run: `py -m pytest tools/ch_mcp/tests/test_frida_tools.py tools/ch_mcp/tests/test_frida_attach.py -q`.

### T6: the menu driver (blocked on T3 and T4; `earlyStart: either executor branch merged into one`)

Owns `tools/ch_mcp/menu_tools.py` (new), `tools/ch_mcp/menu_steps.json` (new) and `tools/ch_mcp/tests/test_menu_tools.py` (new).

`menu_tools.py` holds the three numbers of question 3 (`MENU_STEP_CAP_S`, `MENU_KEY_GAP_S`, `SEARCH_DOWN_CAP`) and `MenuTools(game, frida, lock, *, now, sleep)`. `ch_open_song(song_folder)` loads `menu_steps.json`, refuses at once if the first read's `onlineMenuLive` is true or no agent is attached, then runs each step: send its keys through `game.ch_key` (with `{name}` replaced by the song's `song.ini` name through `game_files`' reader), then poll `frida.read_state()` every `READ_CAP_S / 10` until the step's `expect` holds or `MENU_STEP_CAP_S` passes. The `expect` words are a fixed list the module owns: `song_select_live`, `selected_folder_is_target`, `player_screen_live`, `game_manager_live`, `song_loaded`. The search step is special: after typing, if `selected_folder_is_target` is false it presses Down and reads again, up to `SEARCH_DOWN_CAP` times. On a cap it stops with the step name, the keys sent, and the last read's lines. The file's first content is the manual's sequence, with a top-level `"verified": false` and the manual's URL as its source, so LS1 knows to check it.

Tests on a fake `game` recording keys and a scripted fake `frida` whose reads change after each key: a sequence that reaches `song_loaded` prints one line per step; a step whose read never changes stops at the cap naming the step; `onlineMenuLive` true refuses before any key; a search that needs two Downs presses exactly two and stops; a search that never shows the target stops after `SEARCH_DOWN_CAP` with the message; the three numbers are the literals of question 3. Run: `py -m pytest tools/ch_mcp/tests/test_menu_tools.py -q`.

### T5: wiring and docs (blocked on T1, T6; no `earlyStart`)

Owns `tools/ch_mcp/server.py`, `tools/ch_mcp/tests/test_server.py`, `tools/ch_mcp/README.md` and `docs/agents/clone-hero-mcp.md`.

Registers the nine new tools in `build_server()`, builds one `SpTools` and one `MenuTools` in `Helper` sharing the one lock, and updates `TOOL_NAMES` in `test_server.py` to the twenty-seven names. The agent page's safety rule changes as question 1 says (if approved) and the "shape of a live run" adds the watcher and `ch_open_song`; the README's server section names the new modules and the `menu_steps.json` data file and its owner. Tests: `build_server()` registers exactly the twenty-seven names; everything else in `test_server.py` passes unedited. Run: `py -m pytest tools/ch_mcp/tests/test_server.py -q`.

### Order

T2, T3 and T4 start together. T1 starts from T2's branch. T6 starts from T3 and T4 as they land. T5 last. After T4 merges the main session builds `state.js` in the main checkout. Each task gets one fresh reviewer (D107), and the main session runs what `hydra_precheck.exe --tests` prints plus the task's own test commands.

## The live steps (main session, user present and watching, pressing nothing)

Each names pass and fail before it runs, has a run cap, and ends with `ch_stop`'s restore lines.

### LS0: a song on the command line with no player (1 run)

`ch_start` cannot build this command (`launch_command` always adds `--player`), so the main session runs `"C:\Clone Hero\1.1 june\Clone Hero\Clone Hero.exe" --song "C:\Clone Hero\songs\Hydra Probe\Window Map"` by hand after `game_files.backup`, then `ch_attach`, `ch_state` and `ch_sp_watch_start`. Pass means: the song starts, `ch_sp` shows a moving engine clock, and the notes go by unhit (no bot) within 30 s; then the route exists and `launch_command` gets a no-player form in a follow-up task. Fail means a bot plays, the game shows an error, or the menu opens; record the reply. One run; restore through `bot_run.py --restore`.

### LS1: the menu sequence, one press at a time (3 runs, 15 minutes)

`ch_start()`, `ch_attach`, `ch_menu` (expect `MainMenu` live, `SongSelect` not). Then, step by step from `menu_steps.json`, `ch_key` and `ch_menu` after each, writing down what each read showed. Pass means a sequence reaches `song_loaded` on Window Map with the user touching nothing, and the file is corrected to that sequence in a one-file data commit with `"verified": true`. Fail means three runs or 15 minutes pass without it; report the last step that worked and the read after the one that did not. If the read shows `onlineMenuLive` at any point, press Escape through `ch_key` once and stop the run.

### LS2: `ch_open_song` end to end (2 runs)

`ch_start()`, `ch_attach`, `ch_open_song("C:\Clone Hero\songs\Hydra Probe\Window Map")`, `ch_play_chart(<window-map score JSON>, song_folder=...)`. Pass means `ch_open_song` prints every step as done, `ch_play_status` ends `done` with 142 hits (the count from L2), and `ch_stop` prints five restore lines. Fail means a step stops on its cap; record the step and the read.

### LS3: the Senri fill check (2 runs)

The chart is installed as `C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check` (`notes.mid`, `song.ini` with `song_length = 260504`). The score JSON is `senri_score.json` in this session's scratch folder: 2,349 chords, 32 written fills, no activations (it is a plain `hydra_replay score`, so Hydra's run spends no Star Power). From that file: the fill before the solo is chord 1019 at tick 132,960, 114,628.9 ms, measure label m76; the fill after is chord 1514 at tick 193,200, 166,546.0 ms, m108; the phrase ends inside the solo are chords 1072 (120,732.4 ms), 1183 (134,180.6 ms), 1313 (147,411.3 ms) and 1452 (159,031.3 ms). A simulation of the hit-everything run under Hydra's meter rule (`chsp_meter_sim.py` in the same scratch folder: a phrase adds a quarter of the meter, activation needs half, the meter drains one measure per measure) says the meter is empty at the m76 fill, because the activation at m72 (108,835.8 ms) ends exactly at m76, and reaches half after chord 1183 at 134.2 s. The brief says ready after chord 1313 at 147.4 s. The watcher settles which; the pass line does not depend on it.

Steps: `ch_start()`, `ch_attach`, `ch_open_song(<the folder>)` (or the user starts it, if question 1 is refused), `ch_sp_watch_start(song_folder=<the folder>)`, `ch_play_chart(<senri_score.json>, song_folder=<the folder>, log="senri_presses.csv")`. Poll `ch_play_status` at most 30 times, 10 s apart (300 s, over the 320.5 s cap only because the solo ends at 166.5 s and the check can stop there). Then `ch_sp_between(114728.9, 166446.0)`: the two fill times moved inward by `sp_compare.EVENT_WINDOW_MS` (100 ms), so an activation on either fill itself is outside the window. Then `ch_sp_compare(<csv>, <senri_score.json>)` for the full record, and `ch_stop`.

Pass (Hydra's rule confirmed) means all three: `ch_sp_between` reports no flip inside the window; it reports the amount reached the ready level inside the window (so Star Power was ready and the game still offered no fill); and `ch_play_status` shows no miss before chord 1514. Fail (Hydra's rule wrong for this chart) means a flip inside the window; report its clock, the nearest chord and measure, and the multiplier change, and open a Hydra issue with those numbers. Not decided means the amount never reached the ready level inside the window, or a miss before chord 1514; then run once more (the cap is 2 runs) and, if it repeats, report what the meter did at the m76 fill, because that is the boundary the simulation flagged. Also record what `ch_sp` showed at 114.6 s, whichever way it went.

## Out of scope (needs the user's yes to leave out)

- Reading the Star Power fields inside the game through Frida (the alternative in "Where the read lives").
- Combo and per-note hit results; the song-clock owner question from the server plan.
- A no-player form of `launch_command` (only if LS0 passes; then a follow-up task).
- `--replay` and `--lbsToken`.
- Driving Settings, Practice, Online or the control mapper by keys.
- Filling in `docs/research/2026-10-28-overlay-spike-results.md` (Task E of the spike plan, still open).
- Confirming the `OFF_SP_*` comments ("not yet confirmed live") in `constants.py`: run 1 confirmed them; a one-line comment fix is a follow-up.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| The Star Power read has one home: `decode_snapshot` through `constants.OFF_SP_*`; nothing new reads memory | Where the read lives | T1 | reviewer grep: no `0x68`, `0x1B0`, `0x42`, `0x48`, `0x120`, `0x130` and no `read(` outside `ch_probe` in the diff; `test_sp_tools` drives `SpTools` through a fake `snapshot` only |
| `ch_sp` never blocks on an engine search and reads only the latest row | The tools | T1 | `test_sp_tools`: `ch_sp` before the engine is found returns the waiting line at once; after two scripted snapshots prints the second |
| The watcher stops on the stop flag, the frozen clock, a failed read or the cap, and says which | T1 | T1 | `test_sp_watch`: `should_stop` ends with "stopped"; existing frozen-clock and cap tests unedited; `test_sp_tools`: stop ends the thread and the status prints the reason |
| The watcher takes a backup first and the stop hook ends it | Safety rules kept | T1 | `test_sp_tools`: `ensure_backup` called before the fake opener; the hook stops the thread |
| Activations, rises, flips in a window and the ready level have one owner in `sp_compare` | Who owns which question | T2 | `test_sp_compare`: each function on fixtures; `ready_between` passes at exactly half and fails one below |
| `compare` works with no resolution | T2 | T2 | `test_sp_compare`: the "not computed" line and every other check unchanged |
| The run cap for a known song is its length plus 60 s, else 240 s (question 3d) | Question 3; T3 | T3 | `test_game_tools`: 260504 gives 320.504; no length gives 240; the cap reaches the fake `live_play`; `test_play_chart`: `cap_s=1` ends at the cap and the default test is unedited |
| Non-lane keys and text have one table and one typing path | T3 | T3 | `test_input_driver`: `press_key` down then up; `type_text` sends Unicode events; `test_game_tools`: `ch_key` refuses an unknown name with the list and focuses once |
| `ch_press` and the pressing constants are unchanged | T3 | T3 | existing `test_input_driver` and `test_play_chart` pass unedited; reviewer diff shows `KEY_HOLD_S`, `PRESS_LEAD_MS` untouched |
| The menu read is by field name only, no method call, no offset | T4; Safety rules kept | T4 | `test_no_agent_writes_to_the_game` finds `menu.ts` and passes |
| `state_words` is unchanged | T4 | T4 | every existing `test_frida_tools` word test passes unedited |
| `ch_open_song` refuses when the Online menu is live and when nothing is attached | Question 1; T6 | T6 | `test_menu_tools`: refusal before any key in both cases |
| Every menu step has a cap and the reply names the step when it is hit | Question 3a; T6 | T6 | `test_menu_tools`: a read that never changes stops at `MENU_STEP_CAP_S` naming the step |
| The search hunts at most `SEARCH_DOWN_CAP` results | Question 3c; T6 | T6 | `test_menu_tools`: two Downs when the target is third; the cap message after ten |
| The three new numbers live in `menu_tools.py` and nowhere else | Question 3 | T3, T6 | `test_menu_tools` pins the literals; reviewer grep for `10`, `0.25` in the diff outside that file (T3's temporary constant moves in T6) |
| The step file starts unverified and LS1 marks it verified | Question 4; LS1 | T6, main session | `menu_steps.json` has `"verified": false` at T6; the LS1 data commit flips it |
| Every new tool takes a backup before touching the game | Safety rules kept | T1, T3, T6 | `test_sp_tools`, `test_game_tools` (`ch_key`), `test_menu_tools`: `ensure_backup` recorded before the first touch |
| The server registers exactly the twenty-seven tools | T5 | T5 | `test_server.py` `TOOL_NAMES` |
| The agent page's menu rule changes only with question 1's yes | Question 1; T5 | T5 | the doc reviewer reads the page against the user's answer |
| LS0 to LS3 each have pass and fail lines and a run cap (1, 3, 2, 2) | Live steps | main session | the main session's report names the run count and the deciding reply |
| LS3's window is the two fill times moved inward by `EVENT_WINDOW_MS` | LS3 | main session | the `ch_sp_between` call in the report shows 114728.9 and 166446.0 |
| LS3's pass needs the ready level reached inside the window | LS3 | main session, T2 | `ready_between` in the `ch_sp_between` reply |
| No tool, test or step runs over the user's library | Safety rules kept | all | every new tool takes one folder; reviewer greps the diff for the library folder and `--whole-library` |
| Agents never wait on the user; no task launches the game | Tasks | all | every task is offline; the live steps are the main session's |
| Every number names its source | whole plan | planner | reviewer reads each number against the file or decision beside it |

## Prior art

Clone Hero's command line is documented only as a bot preview ([chart previews via the command line](https://wiki.clonehero.net/books/clone-hero-manual/page/chart-previews-via-the-command-line)); the game's string table adds `--replay` and `--lbsToken`, undocumented. The manual's keyboard defaults are Enter for Start, A for Green, arrows for up and down, and a short Blue press for the search tool ([Keyboard and Gamepads](https://wiki.clonehero.net/books/guitars-drums-controllers/page/keyboard-and-gamepads); [How to Play](https://wiki.clonehero.net/books/clone-hero-manual/page/how-to-play)). Unity UI automation from outside a stock build has no general tool: AltTester drives menus and keys but only in a build that carries its SDK ([overview](https://alttester.com/docs/desktop/v.2.0.0/pages/overview.html)). frida-il2cpp-bridge reads instance fields as `object.field(name).value` and finds live objects with its choose call ([README](https://npmjs.org/package/frida-il2cpp-bridge); [a field-reading gist](https://gist.github.com/eybisi/28f05a3d0bfd7d12de3c5c947ea0e80f)); the server already uses both in `state.ts` and `gamemanager.ts`, and T4 adds the static-field read the same way.
