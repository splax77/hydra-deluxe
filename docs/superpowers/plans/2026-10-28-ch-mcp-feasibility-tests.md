# Plan: the four Clone Hero MCP feasibility tests (2026-10-28)

Task id `ch-mcp-tests-plan`, session `856d0fac-b682-4ca5-b28f-65f459b0833d`. Plan only; no code changed. It plans the four tests in the "Next steps" section of `docs/research/2026-10-28-clone-hero-mcp-feasibility.md` (the study). Every path below is checked against the files on this machine today unless it says otherwise.

## The plain version

The study says an MCP server for Clone Hero is feasible, with one real unknown: can we see each note's hit or miss from inside the game. Four short tests settle that and the other open points. Test 1 launches the game from the command line with a bot player and checks what the game writes afterwards. Test 2 attaches Frida (a tool that runs our script inside a running program) and reads the game's clock by field name. Test 3 uses that attachment to find the method that decides a hit. Test 4 compares the scrambled names of two game builds, if a second build can be had.

Tests 1 and 4 need nothing from the others. Test 3 needs test 2 to have passed. Tests 1, 2 and 3 need the game running with the user there, so the main session runs those parts with the user; agents never wait on a person. Everything that can be written and unit-tested without the game is written first by executors in worktrees, in parallel.

Two things are the user's call before any Frida work starts: the EULA, and installing two npm packages. Both are open questions below.

**Outcome (2026-10-28).** The user approved the recommendations, the npm install, the Frida work and a second build (v1.0, which needed a new Task G because it is a Mono build). Tasks A to G are merged, and all four tests ran. Their results are in the study's "Results of the four tests". Where the build differed from this plan, the study and the paragraphs below marked "As built" say so.

## Open questions for the user

Each has my recommended answer. Say "go with the recommendations" to approve them all.

1. **The EULA.** The study says Clone Hero's EULA forbids reverse-engineering or modifying the game. Tests 2 and 3 attach to the running game and trace its code (reading only; nothing in the game folder changes and no memory is written). Test 1 and test 4's dump step use only the game's documented command line and its files on disk. Recommendation: this is your decision, not mine. The plan treats tests 2 and 3 as gated on your explicit yes; tests 1 and 4 go ahead either way.

2. **Installing the Frida bridge.** Frida 17.18.0 and its Python bindings are installed (`py -c "import frida; print(frida.__version__)"` prints 17.18.0). The bridge is not installed for a project. Global copies exist from the 2026-09-14 session (`C:\Users\Patrick\AppData\Roaming\npm\node_modules\frida-il2cpp-bridge` 0.14.0 and `frida-compile` 19.0.5; `@types/frida-gum` is absent), but a compile reads packages from the project folder, so the project still needs its own install (not tested; I did not run npm). The September session's card records no result from running the bridge, so it is untested on this build. A project install needs three npm packages: `frida-il2cpp-bridge` (latest 0.14.0 on the npm registry today; its 0.12.0 release note says "Support Frida 17"), `frida-compile` (19.1.0 today; it turns the TypeScript agent into one script file) and `@types/frida-gum` (the bridge's own package pins `^19.10.0`). The project's wiki Installation page shows the same three with older pins (`frida-compile ^16.2.2`, `@types/frida-gum ^18.3.1`); the registry versions win because Frida 17 is newer than that page. Installing is a download, so it waits for your yes. Recommendation: yes, into `tools/ch_mcp/agent/` with a committed `package.json` and `node_modules` ignored, as Task C says.

3. **Where the code lives.** Extend `tools/ch_probe/` or start `tools/ch_mcp/`. Recommendation: a new `tools/ch_mcp/` subtree, which becomes the MCP server later. `ch_probe` is an offset-based tool pinned to build 6142 and documented as the hit-window probe; the new code finds things by name and shape, and mixing the two would blur what each promises. `ch_probe` gets exactly one change, the cap on its engine wait (Task F), because test 3 reuses its auto-player.

4. **The durable dump.** Il2CppDumper's output (dump.cs 36 MB, il2cpp.h 58 MB, script.json 109 MB, stringliteral.json 2 MB; sizes read from the scratch copy today) is too big for the repo. Recommendation: `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\`, next to the Ghidra dumps `ch_probe/constants.py` already cites in `../hydra-data/scratch_ms/`. Task A regenerates it there and writes a small `facts.json` beside it; only that small file's shape is tested.

5. **The caps.** Every waiting step needs a number and these are new, so they need your yes. Recommended: the bot run waits for `scorestats.json` to change for the song's length plus 60 s; the exit check after it waits 30 s; Frida samples the clock every 250 ms for the song's length plus 30 s; the method trace ends when `isSongOver` turns true or at the same cap; `find_live_engine` gives up after 120 s; the Il2CppDumper run and `npm install` get 10 and 5 minutes. Song lengths come from `manifest.json`: Window Map's last note is at 59,080 ms and Edge Walk's at 122,000 ms, and `probe_songs.song_length_ms` adds 3,000 ms of silence, so 62.1 s and 125.0 s.

6. **The test profile.** The manual lists `--profile`, but the game keeps no profile file under `Documents\Clone Hero` (that folder holds `Custom`, `Logs`, `Screenshots`, `Songs`, `badsongs.txt`, `scorestats.json`, `settings.ini`, `settings_schema.json`). The last `scorestats.json` (2026-10-09, a bot on "NS E star cutoffs") says `profile_name: Guest`, so bots play as Guest without a `--profile`. Recommendation: play as Guest and skip `--profile`; back up the whole `LocalLow\srylain Inc_\Clone Hero` score and user files (`scoredata.bin`, `scoresext.bin`, `user.bin`) as well as `settings.ini` and `scorestats.json`, since the profile store is not identified. If you would rather make a named test profile in the game first, say so and the launcher passes it.

7. **Offline play.** `settings_schema.json` has no online/offline switch and no leaderboard upload switch. Its `online` section holds lobby and display settings only (for example `minrequiredplayers`, `maxplayers`, `onlyhostchoosesongs`, `maxspectators`, `servertickrate`, `lowsongspeed`, `maxsongspeed`, `showremotenames`), and `server` is a string array in section `servers`. `settings.ini` has `[online]` with `leaderboard_sort_type = 0`. So "offline" means: never open the Online menu, launch songs straight from the command line, and keep playing as Guest. Recommendation: accept that; there is no setting to flip. If you want, the live steps run with the network adapter off, which is the only hard guarantee.

8. **The second chart for test 1.** Window Map is all kicks (142 notes) and has no Star Power phrases, so it cannot check `sp_phrases_total`. Recommendation: the main session also picks one chart from `testdata/input` whose `hydra_replay score` JSON has at least one chord with `is_sp_phrase_end` true, and Task B's installer copies it into `C:\Clone Hero\songs\Hydra Probe\` with a silent `song.ogg` written by `probe_songs.write_silent_ogg` (needs ffmpeg on PATH, as the probe songs did).

9. **Who plays in test 3.** The trace must see real hits. Recommendation: run it twice on Window Map, first with the bot from test 1, then with `ch_probe/experiments/play_chart.py` pressing the keys (proven on "Slipping", 2026-09-25). If the bot's hits go through a different method than key presses, the two runs show it.

10. **A second build (test 4).** Only one build is on this machine (`C:\Clone Hero\` holds `1.1 june` and `songs`). Do you have, or can you fetch, any other Clone Hero build, for example the v1.0 release or an older v1.1 preview? Recommendation: if you have one, point Task A's script at it; if not, test 4 ships its comparison script, is run on two copies of the same dump to prove it works, and the update question stays "assume the worst", exactly as the study already does.

## What exists today (checked in the files)

The game is `C:\Clone Hero\1.1 june\Clone Hero\Clone Hero.exe`, with `GameAssembly.dll` and `Clone Hero_Data\il2cpp_data\Metadata\global-metadata.dat` beside it (both present). The user files are listed under question 6; `Logs\` holds 20 `Session-*.log` files and the two empty `HitDetection-*.log` files. A session log carries `[Log] Loading Song: <artist> - <name>`, `[Log] OnCountdownStart` and `[Log] OnCountdownEnd` lines (read from `Session-2026-10-17_19-29-33.log`). No session log, and no `Player.log`, mentions a profile or hit-detection logging; the schema has no key with "log" or "hitdetect" in its name, so the empty hit logs have no switch anyone has found.

`scorestats.json` already proves one thing test 1 wanted: a bot run writes it. The file on disk is from 2026-10-09, `game_mode: Quickplay`, one player, `is_bot: true`, `no_fail: true`, `profile_name: Guest`, `total_notes: 106`, `sp_phrases_total: 0`, `base_score: 5780`, `note_score: 5780`, `score: 19925`, `combo_score: 14145`. Nobody recorded whether that bot came from the command line or from the in-game bot toggle, so test 1 still has to run; but "the game writes stats for bots" is settled.

The Il2CppDumper dump in this session's scratch folder (`...\scratchpad\obfuscation\dump\`) is temporary. In its `dump.cs`, `GameManager` (TypeDefIndex 449) keeps `songTime` (double, 0x70), `isPaused` (0x90), `isSongOver` (0x91), `isSongPlaying` (0xC0), `songLoaded` (0xC1), `songLength` (0x50), `playerObjects` (0x58) and `scoreManager` (0xB8); its singleton static field at 0x0 is scrambled. `ScoreManager` (592) keeps its name and a scrambled array field at 0x38. `DrumsPlayer` (156) keeps its name.

The drum engine `ch_probe` finds by memory scan has a name in the dump. `ch_probe/constants.py` gives its constructor as RVA `0x20DF680`; `script.json` lists that address (34469504) as `ʺʺʸʶʿʵˀʸʲˀʲ$$.ctor`. The same class owns the window formula at `0x20DDDA0` and the note-processing method at `0x20DCC90`. In `dump.cs` it is `public sealed class ʺʺʸʶʿʵˀʸʲˀʲ : ʿʺʸʺʼʴʸʼʻʳʼ<ʾʸʸʻʷʸˁʾˁʽʸ>`, TypeDefIndex 8255, and it has a private double at 0x2E0, which is `ch_probe`'s unconfirmed hit-time field. `DrumsPlayer` holds a private property of that type and a field (0x230) of a generic wrapper type that takes the engine as its first type argument, so a readable class leads straight to the scrambled engine. `ch_probe/constants.py` also names `RVA_HIT_CHECK = 0x100DAB0` and `RVA_BASE_ENGINE_CTOR_DRUMS = 0x100E960`. In `dump.cs` both addresses are listed under the base class's instantiation for `ʻʹʻʿʿʺʸʹʽʻʻ`, the type argument of `GuitarPlayer` and `GuitarInputViewer`, not the drum one. The drum engine's own instantiation (`<ʾʸʸʻʷʸˁʾˁʽʸ>`, the type argument of `DrumsPlayer`) lists the same two methods at `0x100DAE0` and `0x100EAE0`. Which address `ch_probe` really hits live is not checked here. Task D therefore finds the base class's methods through the live engine class's parent, never through these two addresses. The string `"combo: {0}\n"` sits at `0x3CCA850` in `stringliteral.json`; nothing says yet which method uses it.

`Il2CppDumper.exe` sits in `C:\Users\Patrick\Downloads\Il2CppDumper-net7-v6.7.46\`. Its `config.json` has `RequireAnyKey: true`, so it ends with "Press any key to exit...". The scout's scripted run (`dumper_out.txt`, `dumper_err.txt` in the scratch folder) had stdin redirected. The dumper printed `Done!` for all three stages and then died with `InvalidOperationException: Cannot read keys when either application does not have a console or when console input has been redirected`. So a newline on stdin does not help, and the exit code of a good run is not zero. A script must run it with stdin redirected and judge success by the three `Done!` lines plus the output files, not by the exit code.

`tools/ch_probe` is as the study describes. `engine_finder.find_live_engine` loops `while True` until a candidate's clock moves (`engine_finder.py` lines 179 to 208); its test `test_keeps_waiting_until_a_clock_moves` pins that it waits across a retry. `experiments/live.py` caps its own waits: `StoppedCheck` raises `ClockFrozen` after `STALL_S = 5.0` and `ClockJumpedBack` after a 1 s fall. `experiments/play_chart.py` reads a `hydra_replay score` JSON, finds the engine with `find_live_engine`, and presses each chord with `InputDriver.press_chord` after `focus_window`. `probe_songs.py` builds Window Map (142 notes: ten blocks of 5 marker plus 8 run notes, plus four uneven groups of 3) and Edge Walk (120 notes, 1,000 ms apart), each with a `manifest.json` listing every note's `time_ms`; both manifests are on disk under `C:\Clone Hero\songs\Hydra Probe\` and the counts above were read from them today.

`hydra_replay` is built on demand (`CMakeLists.txt` line 692, `tools/replay.cpp`). `hydra_replay score --chart <notes.chart> --out <json>` with no `--acts` and no `--path` scores the chart with no Star Power. Its JSON has a `chords` list, each chord with a `notes` list (one entry per gem, with `color`, `cymbal`, `sp_points`) and `is_sp_phrase_end`, and a `final` object with the fields of `kReplayScoreFields` in `src/core/replay.h`: `notes`, `combo`, `sp`, `solo`, `accent`, `ghost`, plus `total`.

The Hydra MCP server `tools/hydra_ui_mcp.py` is the shape to copy later: a Python stdio server, one tool per action, text answers. Nothing in this plan builds the server; it only settles whether it can be built.

## Which tests are independent, and who runs what

| Test | Needs | Live or offline | Who runs the live part |
|---|---|---|---|
| 1 Bot launch | nothing | live (game runs by itself, user present) | main session |
| 2 Frida attach | question 1 and 2 yes | live | main session |
| 3 Find the hit method | test 2 passed | live | main session |
| 4 Second build | a second build (question 10) | offline | executor (Task E), then main session if a build turns up |

Code is written first, offline, in five executor tasks. A, B and F can start at once. C needs question 2's yes (the install). D needs C's compiled agent to exist, so it starts after C commits; it can still run in parallel with A, B, E and F. E needs A's `facts.json` shape, so it starts after A commits. Each task is one executor on Opus, under 100 tool calls and 20 minutes, owning only the files it names, in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`. The unit tests run with `py -m pytest tools/ch_mcp/tests -q` (plus `tools/ch_probe/tests/test_engine_finder.py` for Task F) and never touch the game.

## Safety rules for every live step

These hold for L1, L2 and L3 below, and the launcher code in Task B is their one owner, so no step types them by hand.

Before the game starts, the launcher copies `settings.ini` and `scorestats.json` from `Documents\Clone Hero\` and `scoredata.bin`, `scoresext.bin` and `user.bin` from `LocalLow\srylain Inc_\Clone Hero\` into a dated folder under the session scratch folder, and prints the folder. After the step, it copies the originals back, byte for byte, and prints a line per file saying restored or unchanged. A restore that fails stops with the file name and the error; nothing else runs after that. The user may keep the backup folder.

Play is offline as question 7 defines it. No step opens the Online menu.

Frida attaches with `frida.attach("Clone Hero.exe")`, loads one compiled script, and detaches with `session.detach()` in a `finally` block, so a Python error, a cap, or Ctrl-C still detaches. The agent scripts contain no `Memory.write*`, no `.value =` on a field, no method replacement (`.implementation =`, `Interceptor.replace`) and no method call into the game; a grep in the unit tests pins that. These tests read and trace only.

Every wait has a cap from question 5 and prints one line when the cap is hit, naming the cap and what was seen, then stops. Nothing retries on its own.

## Task A: the durable dump and its facts file (one executor, Opus; offline)

Owns `tools/ch_mcp/__init__.py`, `tools/ch_mcp/dump_build.py`, `tools/ch_mcp/dump_facts.py`, `tools/ch_mcp/tests/__init__.py`, `tools/ch_mcp/tests/test_dump_facts.py` and `tools/ch_mcp/README.md`.

`dump_build.py` runs Il2CppDumper over a game folder into an output folder: `Il2CppDumper.exe <GameAssembly.dll> <global-metadata.dat> <out>` with `DOTNET_ROLL_FORWARD=Major` in the environment and stdin redirected (so it never waits for a key). It counts the run as good when the output has three `Done!` lines and `dump.cs`, `script.json` and `stringliteral.json` exist, whatever the exit code; the final "Press any key" crash is expected. It refuses an output folder inside the repo. It stops at the cap (10 minutes, question 5) with the line `Il2CppDumper did not finish in 600 s; output folder left as is`. The executor runs it once for build 6142 into `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\` (question 4) and reports the four file sizes.

`dump_facts.py` reads a `dump.cs` and writes `facts.json` beside it: the game version string the caller passes, and for every class its name, TypeDefIndex, base class, whether the name is readable (ASCII) or scrambled, and its fields (name, type, offset) and methods (name, RVA, parameter list). The parser only needs the `// Namespace:`, `public ... class X : Y // TypeDefIndex: N`, `type name; // 0xNN` and `// RVA: 0x... ` plus the method line shapes that `dump.cs` uses. It also writes a `signatures` block for the classes this plan names: `GameManager` with its readable field names and offsets, `ScoreManager`, `DrumsPlayer`, and the engine class found by the constructor RVA from `ch_probe.constants.RVA_DRUMS_ENGINE_CTOR`. If that RVA matches no `.ctor` or more than one class, it stops with the message `engine ctor RVA matched N classes`.

Unit tests, in `test_dump_facts.py`, run on a 60-line hand-written `dump.cs` fragment in the test file (no real dump): `GameManager` comes out with `songTime` at 0x70, `isSongOver` at 0x91 and `isSongPlaying` at 0xC0; a scrambled class is flagged scrambled; the ctor RVA lookup finds the one class and refuses zero or two; the in-repo output folder is refused; the cap path reports the line above (the subprocess is a fake); a fake run that prints three `Done!` lines and then exits non-zero counts as good; a fake run with fewer than three `Done!` lines counts as failed and reports which stage was missing. The real-dump run is an executor report line, not a test.

## Task B: the bot launcher and the scorestats comparison (one executor, Opus; offline)

Owns `tools/ch_mcp/game_files.py`, `tools/ch_mcp/bot_run.py`, `tools/ch_mcp/compare_scorestats.py`, `tools/ch_mcp/install_chart.py`, `tools/ch_mcp/tests/test_game_files.py`, `tools/ch_mcp/tests/test_bot_run.py`, `tools/ch_mcp/tests/test_compare_scorestats.py`.

`game_files.py` is the one home of the game's paths (`Clone Hero.exe`, the `Documents\Clone Hero` and `LocalLow\srylain Inc_\Clone Hero` folders, the five files to back up from the safety rules) and of `backup()` and `restore()`. Both take the folders as arguments so the tests use a temp folder.

`bot_run.py` takes a song folder, the player string (default `Drums,Expert`) and an output folder. It backs up, records `scorestats.json`'s size and modification time, starts `"Clone Hero.exe" --song <folder> --player Drums,Expert` (and `--profile <name>` only when given), then polls the file every second until it changes or the cap passes (song length from `manifest.json` or `song.ini`'s `song_length` plus 60 s). On change it copies the file to the output folder as `scorestats-<song>-<time>.json`, then watches the process for 30 s and prints `game exited after N s` or `game still running after 30 s (menu or results screen)`. On the cap it prints `scorestats.json unchanged after N s; bot launch wrote no stats` and still restores. It never kills the game unless `--stop` is given. With `--no-wait` (used by L2) it backs up, starts the game, prints the backup folder and the game's process id, and returns without polling and without restoring. `bot_run.py --restore <backup folder>` then puts the five files back and prints a line per file, so the safety rules hold for L2 too. The command line uses the manual's long forms (`--song`, `--player`); the manual's player instruments are `Drums` and `ProDrums`, so a chart whose `song.ini` says `pro_drums = True` is launched with `ProDrums,Expert`, and `--player` is a flag so the caller can say so. Launch, clock and process-check are injectable so the tests use fakes.

`compare_scorestats.py` takes a copied `scorestats.json` and a `hydra_replay score` JSON. It prints three lines. `total_notes`: the game's value against the number of gem entries summed over Hydra's `chords[].notes`, and also against the number of chords, saying which one matches (Hydra counts a chord as one combo step, and the test learns which the game counts). `sp_phrases_total` against the count of chords with `is_sp_phrase_end` true. `base_score` and `note_score` against Hydra's `final.notes`, `final.notes + accent + ghost` and `final.total`, printed side by side; the first two lines are pass or fail, the third is reported, because what the game's `base_score` includes is one of the things this test learns. Every number names its file.

`install_chart.py` copies a chart folder from `testdata/input` into `C:\Clone Hero\songs\Hydra Probe\<name>` and writes a silent `song.ogg` of the chart's `song_length` through `probe_songs.write_silent_ogg`, for question 8. It refuses to overwrite an existing folder. It needs a folder that holds a `song.ini` with `song_length` (all 97 `song.ini` files under `testdata/input` have one, checked today) and a `notes.chart` or `notes.mid`; the flat `.chart` fixtures such as those in `testdata/input/test_fast_tempo/` have no `song.ini` and are not usable. `hydra_replay score --chart` then needs the chart file the installer copied.

Unit tests: backup and restore round-trip in a temp folder, including a file that does not exist before the run; a restore failure stops with the file name; the poll notices a changed file and copies it; the cap prints the exact line above and still restores; the 30 s exit check prints both lines; `--no-wait` starts the fake game, polls nothing and restores nothing, and `--restore` then restores all five files; the comparison on a hand-written pair of JSON files prints the three lines and flags a mismatch; the installer refuses an existing folder. Fakes for the launch and the clock live in `test_bot_run.py`.

## Task C: the Frida attach runner and the clock agent (one executor, Opus; offline; waits for questions 1 and 2)

Owns `tools/ch_mcp/agent/package.json`, `tools/ch_mcp/agent/tsconfig.json`, `tools/ch_mcp/agent/clock.ts`, `tools/ch_mcp/.gitignore` (ignores `agent/node_modules/` and `agent/*.js`), `tools/ch_mcp/frida_attach.py`, `tools/ch_mcp/tests/test_frida_attach.py`.

`package.json` lists `frida-il2cpp-bridge`, `frida-compile` and `@types/frida-gum` at the versions in question 2, and two build scripts, `build:clock` = `frida-compile -o clock.js clock.ts` and `build:trace` = `frida-compile -o trace.js trace.ts` (Task D owns `trace.ts`; the script only names it). The wiki's own form adds `-w`, which watches for changes and never exits, so these scripts leave it out: a build with no end has no cap. The executor runs `npm install` (5-minute cap) and `npm run build:clock` (2-minute cap) once and reports the two outputs; the compiled `.js` is not committed.

`clock.ts` does one thing. Inside `Il2Cpp.perform`, it takes the `CloneHero` assembly's image, asks for the class named `GameManager`, finds its live instances with `Il2Cpp.gc.choose`, and every 250 ms sends one message `{t: <wall ms>, songTime, isSongOver, isSongPlaying, isPaused, instances: N}` read through the field names, never through an offset. It sends `{error: "GameManager not found"}` or `{error: "no live GameManager"}` when that is so, and stops itself at the cap the Python side passes in. It writes nothing.

`frida_attach.py` takes an agent `.js`, a cap in seconds and an output file. It attaches to `Clone Hero.exe`, loads the script, appends every message as one JSON line, detaches in `finally`, and prints a summary: how many samples, first and last `songTime`, whether `isSongOver` went true, and the error if one came. If `Clone Hero.exe` is not running it says so and stops. The `frida` module is injected so tests run with a fake.

Unit tests: the fake session sees `detach` called after a normal end, after the cap, and after a thrown error; the summary lines are right for a hand-written message list; the "not running" path prints its line; a grep over `agent/*.ts` finds no `Memory.write`, no `.value =` assignment, no `.invoke(`, no `.implementation =` (the bridge's way to replace a method) and no `Interceptor.replace`; hooks use `Interceptor.attach`, which observes without changing a return value. The attach to the real game is L2, not a test.

## Task D: the hit-method tracer (one executor, Opus; offline; starts after Task C commits)

Owns `tools/ch_mcp/agent/trace.ts`, `tools/ch_mcp/trace_hits.py`, `tools/ch_mcp/tests/test_trace_hits.py`. It compiles with `npm run build:trace` from Task C (2-minute cap) and reports the exit.

`trace.ts` finds the engine class the way `facts.json` names it: by its obfuscated name and, as a check, by its shape (sealed, a generic base, a double field at 0x2E0, a constructor whose RVA matches). It also takes the base class. For every method of both, it installs a counting hook that records the method name, the call count, and for the first 200 calls the raw argument values (doubles and integers printed as numbers, objects as their class name). On `isSongOver` turning true, or at the cap, it sends the table and stops. It never changes a return value or an argument. A second mode hooks only the methods the Python side names, and for those sends every call with its arguments and `GameManager.songTime` at that moment, so a per-note method can be lined up with `manifest.json`.

`trace_hits.py` runs `frida_attach.py`'s machinery with `trace.js`, then reads the table and `manifest.json` and prints the methods whose count equals the song's note count (142 for Window Map, 120 for Edge Walk), the methods within one of it, and the full table sorted by count. In the second mode it prints, for the named method, each call's arguments next to the nearest manifest note and the difference in ms, and the count of calls whose nearest note is within 85 ms (`ch_probe.constants.EXPECT_NORMAL_BACK_MS`). An offline sub-step, capped at 20 minutes of the executor's time: use REA or Ghidra to find which method reads the string literal at `0x3CCA850` (`"combo: {0}\n"`) and write its RVA into the report; if not found in time, say so.

Unit tests on hand-written tables and manifests: the once-per-note filter picks exactly the 142-count methods and the "within one" list; the line-up gives the right nearest note and ms differences on a five-note manifest; the within-85 ms count is right at the boundary (a call exactly 85.0 ms away counts, 85.1 does not); the no-write grep from Task C runs over `trace.ts` too.

## Task E: the dump comparison (one executor, Opus; offline; starts after Task A commits)

Owns `tools/ch_mcp/compare_dumps.py`, `tools/ch_mcp/tests/test_compare_dumps.py`.

It takes two `facts.json` files and prints: readable classes present in both, with any field offset that moved; readable classes missing on either side; for scrambled classes, how many names appear in both files with the same shape (field count and types, method count) and how many only in one. The last line is one of `names stable: N of M scrambled classes keep their names`, or `names reshuffled: only N of M scrambled names survive`. The executor runs it on two copies of the 6142 facts file and reports the (trivially all-stable) line, which proves the script runs. Unit tests on two hand-written facts files cover a moved offset, a missing class, a kept scrambled name and a renamed one, and the two final lines.

## Task F: cap the engine wait in ch_probe (one executor, Opus; offline)

Owns `tools/ch_probe/engine_finder.py`, `tools/ch_probe/tests/test_engine_finder.py`, `tools/ch_probe/experiments/play_chart.py`, `tools/ch_probe/constants.py`.

`find_live_engine` gains `max_wait_s`, which defaults to `ENGINE_WAIT_CAP_S = 120.0`, a new constant in `constants.py` (question 5). The default means the other callers (`experiments/active_probe.py`, `experiments/passive_probe.py`) stop waiting forever too, and they end with a traceback naming the message below, which is the loud failure the rules ask for. `EngineNotFound` does not exist anywhere in `tools/ch_probe` today (grepped); Task F defines it in `engine_finder.py` as an exception class. When the clock of no candidate has moved after that long, `find_live_engine` raises `EngineNotFound("no engine with a moving clock after 120 s; is a song playing?")`. The elapsed time is measured with an injectable `now`, so the test does not sleep. The existing three tests stay green; a new `test_gives_up_after_the_cap` drives `now` past the cap and pins the message. `play_chart.py` prints the message and exits instead of a traceback. Run: `py -m pytest tools/ch_probe/tests/test_engine_finder.py tools/ch_probe/tests/test_play_chart.py -q`.

As built: the cap already existed on main from the overlay spike (`ENGINE_SEARCH_CAP_S`, `EngineSearchTimeout`, `find_live_engine_capped`), and `play_chart.py` already used it. The user chose to move that cap into the bare `find_live_engine`, so its four uncapped callers stop at 120 s too. `EngineNotFound` and `ENGINE_WAIT_CAP_S` were never made. The pinning test is `test_gives_up_at_the_default_cap_with_the_exact_message`.

## Task G: read the Mono v1.0 build (added 2026-10-28)

As built: the user's second build, v1.0.0.4045, is a Mono build, so Il2CppDumper cannot read it. `tools/ch_mcp/mono_facts.py` and `mono_reader.ps1` read its .NET assemblies with System.Reflection.Metadata and write a facts file in Task A's schema, with null offsets and addresses, to `hydra-data\ch-dumps\4045\`.

## The live steps (main session, user present)

Each starts by naming the backup folder from Task B's launcher and ends with its restore lines. Each has its pass and fail lines first, so the result is read against them, not argued afterwards.

### L1: test 1, the bot launch

Pass means: `scorestats.json` changes within the cap, its player has `is_bot: true`, and `total_notes` equals Hydra's count (gems or chords, whichever the comparison says) on both charts, and `sp_phrases_total` equals Hydra's `is_sp_phrase_end` count on the SP chart. Then layer A can check Hydra's chart reading against the game at scale.

Dead means: the file does not change within the cap on either chart. Then the command line gives no per-run result and layer A shrinks to launching only; the in-game bot toggle, started by hand, is the fallback and gets its own question.

Steps. First, `hydra_replay score --chart "C:\Clone Hero\songs\Hydra Probe\Window Map\notes.chart" --out <scratch>\window-map.json`, and the same for the SP chart from question 8 after `install_chart.py` has copied it in. Then `py -I tools/ch_mcp/bot_run.py --song "C:\Clone Hero\songs\Hydra Probe\Window Map" --out <scratch>\bot` with the user at the game, who notes whether the bot's Star Power bar fills and whether it activates (the on-screen check; `sp_activations` in the file is the written check). Then the same for the SP chart, with `--player ProDrums,Expert` if its `song.ini` says `pro_drums = True`. Then `compare_scorestats.py` on each pair. Record, from the file, `is_bot`, `sp_activations`, `time_in_sp`, `no_fail`, `profile_name`, and from the launcher's line whether the game exited or stayed. Two runs at most; if the first run's cap trips, check the session log for `Loading Song:` and report that before trying the second.

### L2: test 2, the Frida attach

Needs questions 1 and 2 answered yes and Task C merged. Pass means: with a song playing (launched as in L1, so the user only watches), `frida_attach.py` prints samples whose `songTime` rises between the first and last sample, `isSongPlaying` is true during the song, and `isSongOver` is true in the last samples. Dead means: the bridge reports an error before `Il2Cpp.perform` runs (then this build's metadata version 31 is not supported, and layer C falls back to script.json addresses, which is option B of the study), or `GameManager` is not found by name (then the dump and the live game disagree, which would be a first; stop and report).

Steps. Start the game with the bot on Window Map through `bot_run.py` with `--no-wait`, wait for `OnCountdownStart` in the newest session log (cap 60 s), then `py tools/ch_mcp/frida_attach.py --agent tools/ch_mcp/agent/clock.js --cap 93 --out <scratch>\clock.jsonl` (62.08 s song plus 30 s, rounded up). Not `py -I`: Frida lives in the user site, which `-I` skips. Last, `bot_run.py --restore <backup folder>`. One run; a second only if the first failed to attach because the process name was not found, after checking Task Manager.

As built: the log wait never fires, because a command-line launch writes only its `Loading Song` line until later. The passing run attached 8 s after launch instead. That was a second run, which the line above does not allow; the first had failed in this wait step, not in the attach.

### L3: test 3, the hit method

Needs L2 passed. Pass means: at least one method on the engine class or its base fires exactly 142 times on Window Map and 120 on Edge Walk, in both the bot run and the key-press run, and in the second mode its calls line up with manifest notes with every nearest-note difference inside 85 ms. That method, with its argument list, is the per-note hook, and the study's lowest-confidence layer is feasible. A method with the right count but no usable arguments is a partial pass: the event exists, and the note and offset must come from fields read at that moment (reported, not dead).

Dead means: no method on either class fires once per note in either run (the hit decision is inlined, or lives in a class this plan did not name). Then the per-note layer needs either the polled 0x2E0 field (option B) or a mod loader (option D), and the study's "55%" becomes a no for option C.

Steps. Run one: bot on Window Map, `trace_hits.py --mode count --song "C:\Clone Hero\songs\Hydra Probe\Window Map"`. Run two: the same with Edge Walk. Run three: Window Map with the user starting the song from the menu and `play_chart.py <scratch>\window-map.json` pressing the keys (window focus as the tool already handles it), tracing at the same time. Run four, second mode: `--mode args --method <name>` on Window Map with the bot. Four runs at most. If run one's table is empty, the hooks did not install; stop, keep the output, and report before anything else.

### L4: test 4, the second build

If question 10 names a build, the main session runs Task A's `dump_build.py` on it into `hydra-data\ch-dumps\<version>\`, then `compare_dumps.py` on the two facts files. Pass (names stable) means the signature map survives updates and the MCP's re-derive step is cheap. Reshuffled means every update needs the shape-based re-derive the study already planned; nothing else changes. If no build is available, the step is not run, and the plan's answer stays "unknown, assume the worst", which the study already assumes.

## Merge order

A, B, C and F merge as each is reviewed; D after C; E after A. Each has its own fresh reviewer (D107). The main session runs what `hydra_precheck.exe --tests` prints for each; the Python tests are `py -m pytest tools/ch_mcp/tests -q` and, for F, the two `ch_probe` test files named above. The live steps follow the merges, in the order L1, L2, L3, with L4 whenever a build appears.

## Out of scope (needs the user's yes to leave out)

- Building any part of the MCP server itself; this plan only settles feasibility.
- Finding the switch for the empty `HitDetection-*.log` files. It is a lead in the study, not one of the four tests.
- Input inside the process (replacing `SendInput`). Test 3 uses the bot and the existing key presser.
- A test of precision mode; everything here runs in the game's normal hit-window mode.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| Backups of `settings.ini`, `scorestats.json`, `scoredata.bin`, `scoresext.bin`, `user.bin` before every live step, restored after | Safety rules; question 6 | B | `test_game_files.py`: round-trip restores all five byte for byte, including one absent before the run; a failed restore stops with the file name |
| A restore failure stops everything | Safety rules | B | `test_bot_run.py`: fake restore raises, the run prints the file name and no exit check runs |
| Play offline: no Online menu, Guest profile, no profile flag unless given | Question 7; Task B | B | `test_bot_run.py`: command line contains `--song` and `--player` only; `--profile` appears only when passed |
| Frida detaches cleanly on end, cap and error | Safety rules; Task C | C | `test_frida_attach.py`: fake session's `detach` called in all three paths |
| No game memory is written, no game method is called or replaced | Safety rules; Tasks C, D | C, D | grep test over `agent/*.ts`: no `Memory.write`, no `.value =`, no `.invoke(`, no `.implementation =`, no `Interceptor.replace` |
| The dumper's expected end-of-run crash does not fail a good dump | What exists today; Task A | A | `test_dump_facts.py`: three `Done!` lines and a non-zero exit count as good; two `Done!` lines count as failed |
| The L2 run restores the user files too | Safety rules; Task B; L2 | B | `test_bot_run.py`: `--no-wait` restores nothing, `--restore` restores all five |
| Builds end by themselves | Task C | C | `package.json` scripts have no `-w`; the executor reports the build's exit |
| Every wait has a cap and a report line | Question 5; Tasks A, B, C, D, F | A, B, C, D, F | `test_dump_facts.py` cap line; `test_bot_run.py` unchanged-file line and 30 s exit lines; `test_frida_attach.py` cap path; Task D cap via Task C's runner; `test_gives_up_at_the_default_cap_with_the_exact_message` |
| `find_live_engine` no longer waits forever | Task F | F | `test_gives_up_at_the_default_cap_with_the_exact_message` pins `EngineSearchTimeout` and its existing message; the three existing tests stay green |
| Fields are read by name, never by offset, in the Frida agents | Task C | C | reviewer diff: `clock.ts` uses `.field("songTime")`-style access and no numeric offsets; grep test for `add(0x` finds nothing |
| `GameManager` facts come from the dump, not from memory of the scratch copy | Task A | A | `test_dump_facts.py`: parser yields 0x70, 0x91, 0xC0 from the fragment; the real run's sizes in the executor report |
| The engine class is found from the ctor RVA and must match exactly one class | Task A | A | `test_dump_facts.py`: one match passes, zero and two refuse with `engine ctor RVA matched N classes` |
| The dump lives outside the repo | Question 4; Task A | A | `test_dump_facts.py`: in-repo output folder refused |
| `total_notes` and `sp_phrases_total` are compared to Hydra's `hydra_replay score` JSON, and `base_score` is reported against three Hydra numbers | Task B; L1 | B | `test_compare_scorestats.py`: the three lines on a hand-written pair, mismatch flagged |
| The SP chart install never overwrites | Question 8; Task B | B | `test_bot_run.py` or `test_compare_scorestats.py`: `install_chart` refuses an existing folder |
| Once-per-note filter uses the manifest's count (142 / 120) | Task D | D | `test_trace_hits.py`: exact-count and within-one lists on a hand-written table |
| The 85 ms line-up boundary follows the rule | Task D | D | `test_trace_hits.py`: 85.0 ms counts, 85.1 ms does not |
| The dump comparison tells stable from reshuffled names | Task E | E | `test_compare_dumps.py`: both final lines, moved offset, missing class |
| Each live step has pass and dead lines stated before it runs | L1, L2, L3, L4 | main session | reviewer reads them against the test's purpose in the study's "Next steps" |
| Live steps have a run cap (L1 two, L2 one or two, L3 four) and say what to report when a run fails | L1, L2, L3 | main session | reviewer check of the plan text; the main session's report names the run count |
| Agents never wait on the user | Which tests are independent | all | every executor task is offline; the live steps are the main session's |
| The EULA and the install are the user's decisions and gate tests 2 and 3 | Questions 1, 2 | C, D | Task C's brief is not written until both answers are yes; reviewer confirms the gate line in the brief |
| No whole-library run anywhere | Question 8 | B | `install_chart` copies one named folder; nothing in `tools/ch_mcp` names the user's library |
| Every number names its source | whole plan | planner | reviewer reads each number against the file named beside it |
