# Plan: start the song straight from the command line, as a human player (2026-10-31)

Task id `ch-direct-start-plan`, session `b15bb484`. Every address below was read in `GameAssembly.dll` of build 1.1.0.6142 with `tools/ch_mcp/chdis.py` (capstone) during this session, or is marked as a lead that nobody has checked yet. The code paths in `tools/ch_mcp` were read on `main` at 0adb8fc4.

## The plain version

The user asked why the tools walk the menus at all, and said we had been accepting another agent's framing without checking it. The answer from the code is: the game already has a direct route. `Clone Hero.exe --song <folder> --player ProDrums,Expert` loads the song with no menu. The only reason we could not use it is one instruction. The command-line player loader, at RVA 0x3968D0, sets the player's `is_bot` setting to 1 when no replay was given. One byte of that instruction decides it: the `1` at 0x396EC6.

So the plan is: start the game suspended, attach Frida before the game runs a single line of its own code, change that one byte in memory to 0, and let the game go. The loader then builds a human player on ProDrums Expert, the engine's player kind is Player (0), and Star Power fills and activates like it does for you. The game file on disk is never changed. The kit gives the player its input, as it does today, with one open question about which Rewired player it must sit on. The menu walk, the method watch and the start-method study are not needed for this; what happens to them is the user's call.

A live run is spent only on what the code cannot answer. There are three such things, each named below with the reason: whether Frida attaches to a process we created suspended, whether the kit must sit on the System Player or Player1, and whether the song starts by itself with no Ready press.

## Rulings that bind this plan

1. Runs never need focus (2026-10-30, memory `ch-runs-must-not-need-focus`, plan `2026-10-30-ch-mcp-in-game-input.md` ruling 1). The game window is never shown or raised; the foreground count is a pass line of every live step.
2. The key route stays locked behind a typed `allow key run` (the in-game-input plan, question 1 and 6, answered 2026-10-30). This plan presses nothing through Windows.
3. Reverse engineering may proceed (2026-10-28, "Proceed, my call", memory `ch-mcp-feasibility-2026-10`). The disassembly reads below rest on it.
4. In-memory patches of the game's code are allowed: the trace hook patches a method start while attached, approved 2026-10-28 (`docs/agents/clone-hero-mcp.md`, "The Frida agents"). D272 rulings 2 and 5 allow method calls and field sets. This plan adds one more patch of the same kind, one byte, and no new kind of write.
5. A patch of the game file on disk is ruled out (the brief of this plan). Nothing here opens `GameAssembly.dll` for writing, and a test says so.
6. D272 ruling 1: the server never opens the Online menu, Settings, Practice or the control mapper. The direct route opens no menu at all.
7. Nothing is built until the user says yes (rule 3 of `CLAUDE.md`).

## What was verified in the code, and how

These were read in this session's disassembly (scratch listing of 0x396E60 to 0x396F80 and 0x396110 to 0x3962D0) or by the main session, which re-read them before writing this brief. Every one goes onto page 07 in task D1 with a listing.

The loader's bot write. At 0x396EA0 and 0x396EA3 the loader writes the instrument byte to profile +0x10 and the difficulty byte to +0x11. At 0x396EA7 it compares the replay field `[rsi+0x60]` with null. When it is null, 0x396EB1 loads the `is_bot` setting from profile +0x48, 0x396EC5 is `mov edx, 1` (bytes `BA 01 00 00 00`, so the `1` sits at 0x396EC6), and 0x396ECA calls the setting's int setter at 0xFE5F50. Then 0x396ECF writes 1 to profile +0x14. At 0x396ED6 and 0x396EDB it compares the instrument with 6 and with 9; both jump to 0x396F1A. The scout's claim that ProDrums takes the drums branch is now checked: it does. That branch calls the same setter with 2 on the `controller_type` setting at profile +0x60 (0x396F27 to 0x396F33) and writes 2 to +0x14 (0x396F38). The other instruments set `controller_type` to 1 or 0 by a bit mask at 0x396EE5. When a replay is given (0x396F44 on), the loader skips the bot write and takes `controller_type` from the replay's byte +0x6C. What profile +0x14 means is unknown; its two values track "drums or not", and D2 names the field from the dump.

The `--player` parser at 0x396110 lower-cases the two comma-split words and matches them by string hash. `prodrums` gives 9 (0x3962CD), and the string table it builds names `drums`, `expert`, `hard`, `medium`, `easy` and the rest. `launch_command` in `tools/ch_mcp/bot_run.py` already sends `ProDrums,Expert`.

The main session verified, and this plan builds on: the loader hands each slot the Rewired System Player (call at 0x396D55 to `ReInput.PlayerHelper.GetSystemPlayer`, dump line 310436); the profile constructor builds the `is_bot` bool setting into profile +0x48 (0x2123CE3 to 0x2123D0A) with its value at setting +0x10; `IsBot` at 0x13F8B0 reads slot +0x10, then +0x48, then the bool getter 0x210BF30; the kind builder in `BasePlayer<DrumsInputs>`'s constructor 0x11676A0 (0x1167862 to 0x11678DB) writes kind 1 for a remote slot (0x337720), else 2 when `IsBot`, else 3 when a replay is set (0x13F8E0), else 0; the engine constructor copies it into engine +0x190 at 0x20F7BCB; the Star Power gain at 0x20F4FD5 skips kinds 2 and 1; `PlayerSelectionMenu` +0x1E8 copies the profile's `controller_type` in OnEnable (0x32C2D7) and Update (0x3315D3), and the profile screen's Controller row writes it (0x1E6C92).

The live path to the loader, verified by the main session: `UnpackingScene.MoveNext` 0x361C60, then 0x3980F0 and 0x398D30, the coroutine 0x3A5A20, 0x398F00, then 0x3968D0. UnpackingScene is the first scene the game runs. So the loader runs once, early, inside the game's first scene, and only when the command line names a song.

How the tools launch today: `bot_run._real_launch` calls `subprocess.Popen` with `SW_SHOWMINNOACTIVE` and nothing else. Frida attaches by process name (`frida_mod.attach(PROCESS_NAME)` in `frida_attach.py` lines 130 and 283), only when a tool asks, seconds after the launch. Nothing today reaches the game before its first scene. The installed Frida is 17.18.0; `frida.spawn`, `Device.resume`, `enable_spawn_gating` exist. The backup list (`game_files.LOCALLOW_FILES`) holds `scoredata.bin`, `scoresext.bin` and `user.bin`; `DOCUMENTS_FILES` holds `settings.ini` and `scorestats.json`. Neither `profiles.ini` nor `profiles.bin` is in it.

## Leads nobody has checked (each one is assigned to a task below, or named as a live question)

The scout's claims: the flag parser is 0x395C10; `--profile` matches `player_name` in the loaded profile list, else 0x1C9300 supplies one; `BasePlayer` 0x208F50 replays the chart as input each frame when the player is bot or remote and re-reads `is_bot` each frame; the engine is made in `DrumsPlayer.CreateEngine` 0x33C560 (call at 0x33C8D5); a compare scan found no other test of kind 2 but did not scan plain loads of +0x190; the profile clone at 0xD8470 has index -1; the stats record builder 0x1144C0 uses the "is_bot" string at 0x114703 and 0x114CC9. Task D2 checks each before it is written anywhere.

## The open questions, settled from code first

### 1. When the loader runs, and how the byte is 0 before it does

The loader is reached only from the first scene, after `GameAssembly.dll` is loaded and il2cpp is initialised. Two routes could change the byte in time.

The early route: create the process suspended, attach Frida, load a small agent, patch the byte, then resume. `subprocess.Popen` takes `creationflags=CREATE_SUSPENDED` (4) and keeps the `STARTUPINFO` we already pass, so the launch stays minimized and not active. Frida's own `spawn` on Windows creates the process suspended and injects into it before resuming, so attaching to a suspended process is what Frida is built to do. The agent waits for il2cpp (`Il2Cpp.perform`, which runs after `il2cpp_init` and before any managed code), checks the five bytes at 0x396EC5 read `BA 01 00 00 00`, writes `BA 00 00 00 00` with `Memory.patchCode`, and hooks the loader's entry so it can report whether the loader ran after the patch. The order is fixed by code: the process cannot run its first scene while its only thread is suspended, and the patch lands before il2cpp hands control to managed code. That is deterministic.

The late route: attach by name after a normal launch and write `is_bot` 0 on every slot's profile after the loader and before `CreateEngine`. The window is the song's load time, against Frida's attach time (the il2cpp bridge alone takes seconds). That is a race: it works on a slow song load and fails on a fast one, and nothing in the code bounds either side. It is not built. It stays here as the fallback a later plan may take if LS-D0 fails on both launch forms.

Why the one byte rather than a hook that writes `is_bot` 0 afterwards: the byte change needs no object lookup, no setter call and no main-thread hop, and it is the smallest change of the kind ruling 4 already allows. The hook on the loader's entry is read-only and exists only to prove the order.

What the code cannot answer, and LS-D0 settles: whether `frida.attach(pid)` on a process Python created suspended succeeds and the il2cpp bridge initialises after resume. If it does not, the second run of LS-D0 uses `frida.spawn` with the same command line, which gives up `SW_SHOWMINNOACTIVE` (Frida's Windows spawn takes no show-window option that the installed package exposes; its `spawn` signature has `stdio` and `aux` only). The foreground count then decides whether that launch is allowed at all: a count above 0 goes to the user before anything else runs.

### 2. The Rewired player

From code: the loader gives each slot the System Player. The kit is added to Player1 (`KIT_PLAYER = "Player1"` in `kit_tools.py`). Rewired's `Player.ControllerHelper.AddController(Controller, bool)` (dump line 306697) is the same method for any Player, and `kit.ts` already enumerates every player including the System Player (its `allPlayers`). So the smaller change is one string: give the kit to the player named `System`. The alternative, writing Player1 into slot +0x18, is a field set inside the game's own object and must land before the drums player caches it, which puts it in the launch agent on the loader's return. Task D2 traces where `BasePlayer<DrumsInputs>` reads its input and whether it caches the Rewired player at creation, so the write would be placed right if it is ever needed.

What the code cannot answer: whether Rewired lets a custom controller be added to the System Player and reads it there. That is the one live question of LS-D1: a kit press on the System Player must register as a hit. If it does not, LS-D1's second run adds the kit to Player1 and sets slot +0x18 to Player1 from the launch agent (D2 says where), and the field set is listed in `launch_patch.json`.

### 3. What a human run leaves behind

Three places. Profiles: the loader clones a profile; whether the clone is ever saved to `profiles.ini` or `profiles.bin` is a lead (index -1). Task D3 adds both files to `LOCALLOW_FILES` now, so the run cannot leave a trace there whatever the trace finds, and D2 reads the save path to say which. Scores: `scorestats.json`, `scoredata.bin` and `scoresext.bin` are already backed up and restored byte for byte. Online: the game's log shows "Unauthorized" for analytics in bot runs, but that is an observation, not a rule. D2 traces the score submit path from the stats record builder 0x1144C0 and says whether a human score posts anywhere when nobody is logged in. Until D2 answers, LS-D2's stop line keeps the run on the test chart only, and the question of a network block goes to the user if D2 cannot settle it.

### 4. What the built pieces become

The direct route makes these unnecessary for Star Power runs: the menu walk (`ch_open_song`, `menu_steps.json`), the method watch (`ch_method_watch_start`, `ch_method_watch_stop`, `start_candidates.json`), `start_method.json`, task T8 `ch_start_song` of `2026-10-30-ch-mcp-star-power-and-song-start.md`, and live step LS-K3 of `2026-10-30-ch-mcp-in-game-input.md`. LS-K4 and LS-K5 keep their pass lines but start the song by this route. Retiring code is the user's call; it is question 4 below, with a recommendation. Nothing in this plan deletes any of it.

## The design

### The launch

`ch_start(song_folder, human=False)` gains `human`. With `human` true and a song folder, `bot_run.launch_human(cmd, env)` runs this sequence: `Popen` with `CREATE_SUSPENDED` added to today's start-up info; `frida.attach(pid)` with cap `LAUNCH_ATTACH_CAP_S`; load `agent/launch.ts` as a `LiveAgent`; send it `patch` with the contents of `tools/ch_mcp/launch_patch.json`; wait for its reply within `LAUNCH_PATCH_CAP_S`; refuse and kill the process if the reply's `before` bytes differ from the file's `expect`; then resume the main thread. Python's `Popen` closes the thread handle, so the resume opens the process's one thread through `ctypes` (Toolhelp snapshot by pid, `OpenThread`, `ResumeThread`); `bot_run.resume_main_thread(pid)` owns that. A game that is killed on a refused patch never runs a frame, so nothing needs restoring but the backup is taken before the launch as today. Without `human`, nothing changes.

`launch_patch.json` holds, for build 6142: the loader RVA 0x3968D0, the patch RVA 0x396EC5, `expect` `BA 01 00 00 00`, `write` `BA 00 00 00 00`, the dump line of the loader, and an empty `sets` list for the field writes question 2's fallback may add. The agent reads no address literal of its own: every RVA arrives in the message. The agent's `patch` runs inside `Il2Cpp.perform`, patches, hooks the loader's entry with `Interceptor.attach` (recording the frame and time it ran, and reading the five bytes again), and replies with `before`, `after` and `module` (the base and size of `GameAssembly.dll`). A later `status` message replies with whether the loader has run and what the bytes were when it did. That read is LS-D0's pass line, and `ch_status` prints it as "loader ran after patch: yes" or "no" or "not yet".

The Frida session opened by the launch stays, so `ch_attach` is a no-op on a human run and `ch_kit_setup` can run as soon as the kit agent's setup succeeds. Boot takes seconds; LS-D0 measures the time from resume to the engine clock moving, so LS-D2 knows whether `ch_sp_watch_start` and `ch_play_chart` can be armed before the resume within `engine_finder.ENGINE_SEARCH_CAP_S`.

### The kit's player

`ch_kit_setup(player=None)` gains `player`; `None` means `KIT_PLAYER`. `KIT_PLAYER` stays `Player1` until LS-D1 decides, then a one-line data commit records the answer with the step's reply quoted. The agent already takes the player name in its `setup` message, so `kit.ts` does not change for this.

### Who owns which question

| Question | Owner | The other side |
|---|---|---|
| Which byte is patched, what it must read first, and what it becomes | `tools/ch_mcp/launch_patch.json` | `launch.ts` refuses when `before` differs; `test_bot_run` checks the refusal |
| The order: suspend, attach, patch, resume | `bot_run.launch_human` | `test_bot_run` records the order on fakes |
| Whether the loader ran after the patch | `launch.ts` `status` | `ch_status` prints it; LS-D0's pass line |
| The game file on disk is never written | ruling 5 | `test_bot_run`: no open of `GAME_EXE` for writing; LS-D0 compares the MD5 before and after |
| Which Rewired player holds the kit | `kit_tools.KIT_PLAYER`, `ch_kit_setup(player)` | LS-D1 decides and the data commit quotes it |
| The two launch caps | `bot_run.LAUNCH_ATTACH_CAP_S`, `LAUNCH_PATCH_CAP_S` (question 1) | replies name the cap when it ends them |
| Which files a run can touch | `game_files.LOCALLOW_FILES` gains `profiles.ini`, `profiles.bin` | LS-D3 restore lines |
| Whether a human score posts online | D2's trace, written on page 07 | LS-D2's stop line until then |
| The game facts | `docs/clone-hero/07-internals.md` and `code/facts-6142.tsv` | the docs test and `dump_facts.py --check` |

## Questions for the user

1. **The two launch caps.** No ruling sets them. Recommended: `LAUNCH_ATTACH_CAP_S = 20` (Frida's injection plus the il2cpp bridge load) and `LAUNCH_PATCH_CAP_S = 10` (one memory read and write inside `Il2Cpp.perform`). The other side: the existing `READ_CAP_S` (5 s) for both, one number fewer, but a cold Frida start on a busy machine passes 5 s and the run would fail for nothing.
2. **If `frida.attach` on a suspended process fails** (LS-D0 run 1), the second run uses `frida.spawn`, which gives up the minimized, not-active launch. Recommended: run it once with the foreground watch as judge, and stop for good if the count is above 0. The other side: skip it and go to the late route (a race, see question 1 above).
3. **Profiles in the backup.** Recommended: add `profiles.ini` and `profiles.bin` to the backup list now, before D2 says whether the clone is saved. The other side: wait for D2; one more trace before the first human run.
4. **The built pieces.** Recommended: keep the menu walk, the method watch, `start_method.json` and T8 on disk and registered, mark them in `docs/agents/clone-hero-mcp.md` as the route for a day the direct route breaks, and strike LS-K3 and T8 from the two older plans with a one-line note pointing here. The other side: delete `ch_open_song`, `menu_steps.json`, the method watch and `start_candidates.json`, so there is one route. Both leave `menu.ts`'s reads, which `ch_menu` and the Online check use.
5. **Which chart LS-D2 plays.** Recommended: Window Map (142 hits with keys in L2), so LS-K4's pass line carries over. The other side: Senri, which LS-K5 needs anyway, with no hit count to compare.

## Tasks

Each task is one executor on Opus, under 100 tool calls and 20 minutes, in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`, owning only the files it names. Unit tests run on fakes and never start the game or attach Frida. No task runs the full suite. Every commit carries `Task:`, `Agent:` and `Session:` trailers.

### D1: the Clone Hero write-back (earlyStart: yes)

Owns `docs/clone-hero/07-internals.md`, `docs/clone-hero/code/facts-6142.tsv`, `docs/clone-hero/code/6142/cli_loader_3968D0.asm` and `.md` (new), `docs/clone-hero/code/6142/player_kind_11676A0.asm` and `.md` (new), `docs/clone-hero/code/README.md` (the two new listing rows), `docs/clone-hero/README.md` (the block counts and the write-back date).

Writes the listings with `dis_range.py`'s method (the `chdis.Image` disassembler; `py` without `-I`, because capstone sits in the user's site-packages) for 0x396D40 to 0x396F80 and 0x1167862 to 0x11678DB, each with its `.md` explaining the instructions that decide the rule. Adds blocks for the loader's bot write and its skip on a replay, the ProDrums-takes-the-drums-branch fact, the `--player` parser's words and values, the player-kind rule, the profile layout (+0x10 instrument, +0x11 difficulty, +0x14 unknown, +0x48 `is_bot`, +0x60 `controller_type`), the System Player hand-off, the Star Power gain's kind test, and the Controller row and menu copies of `controller_type`. Updates IN-31 (names the kind's source), IN-32 (the caller that passes Bot is now followed), IN-53 (the parser is read; label becomes Proven in the game's code), IN-57 (the OnEnable and Update copies, the Controller row). Corrects IN-38 only after disassembling 0x1144C0 around 0x114703 and 0x114CC9 and seeing the "is_bot" string reference; if the reference is not there, IN-38 keeps its label and the executor's report says what the bytes showed. Each new backticked address gets its row by running `py tools/ch_mcp/dump_facts.py --tsv`. Next free block id is checked by grep, never assumed.

Tests: `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"` passes; `py tools/ch_mcp/dump_facts.py --check` reports no difference; the reviewer reads each new block's address against the listing beside it.

### D2: three code traces (earlyStart: no; starts from D1's branch, because it adds blocks to the same page)

Owns `docs/clone-hero/07-internals.md` (new blocks only), `docs/clone-hero/code/facts-6142.tsv`, `docs/clone-hero/code/6142/drums_input_source.asm` and `.md` (new), `docs/clone-hero/code/6142/profile_save.asm` and `.md` (new), `docs/clone-hero/code/6142/score_submit.asm` and `.md` (new), `docs/clone-hero/code/README.md`.

Trace one: where `BasePlayer<DrumsInputs>` reads its input. Start at the constructor 0x11676A0 and at `DrumsPlayer.CreateEngine` 0x33C560 (a lead), find the read of slot +0x18 (the Rewired player), and say whether it is stored at creation or read each frame. Also check the lead that 0x208F50 replays chart notes as input when the player is bot or remote and re-reads `is_bot` each frame. Trace two: the profile save. Find the writer of `profiles.ini` (the string is in the string table) and say whether a clone with index -1 (the clone at 0xD8470, a lead) is written. Trace three: the score submit. From the stats record builder 0x1144C0, follow where the record goes and say whether anything is posted online with no login, and whether `is_bot` gates it. Each trace becomes a block with the right label; a trace that cannot be finished in budget is a block labelled Unknown whose evidence line says where the executor looked, and the report names it.

Tests: the same two as D1. The reviewer reads each listing against its block.

### D3: the suspended launch, the patch agent and the backup list (earlyStart: yes)

Owns `tools/ch_mcp/bot_run.py`, `tools/ch_mcp/tests/test_bot_run.py`, `tools/ch_mcp/agent/launch.ts` (new), `tools/ch_mcp/agent/package.json` (the `build:launch` script), `tools/ch_mcp/launch_patch.json` (new), `tools/ch_mcp/game_files.py` (the two file names), `tools/ch_mcp/tests/test_game_files.py`, `tools/ch_mcp/game_tools.py` (`ch_start`'s `human` argument and the "loader ran after patch" line in `ch_status`), `tools/ch_mcp/tests/test_game_tools.py`, `tools/ch_mcp/tests/test_frida_attach.py` (the `launch.ts` entry: it calls no game method, so its `ALLOWED_CALLS` is empty and `CALL_SITES` gives it 0).

Builds what "The launch" says. `Env` gains `attach_pid`, `resume_thread` and `load_agent` seams so the tests use fakes. The two caps are constants with the values question 1 settles. `launch.ts` uses `Memory.patchCode` on `Process.getModuleByName("GameAssembly.dll").base.add(m.rva)` with the RVA from the message, never a literal; `Interceptor.attach` on the loader RVA from the message; and replies as the design says. The executor builds `launch.js` with `npm run build:launch` in its own worktree and reports the output. Adds `profiles.ini` and `profiles.bin` to `LOCALLOW_FILES` if question 3 is answered as recommended.

Tests, on fakes: `launch_human` passes `CREATE_SUSPENDED` and the same `STARTUPINFO` fields as today (`STARTF_USESHOWWINDOW`, 7); the recorded order is attach, load, patch, resume; a patch reply whose `before` differs from `expect` is refused naming the bytes, the process is killed and `resume_thread` is never called; an attach that never returns ends at `LAUNCH_ATTACH_CAP_S` naming it and kills the process; a patch reply that never comes ends at `LAUNCH_PATCH_CAP_S` the same way; `launch_patch.json` parses, its `expect` is `BA 01 00 00 00`, its `write` differs from `expect` in exactly one byte, and each RVA in it has a row in `facts-6142.tsv`; no function in `bot_run.py` or `launch.ts` opens `GAME_EXE` for writing (a grep test over the two files for `open(` with a write mode and for `GAME_EXE`, and the reviewer reads the diff); `ch_start(human=True)` with no song folder refuses, naming the reason (the loader runs only with `--song`); `ch_start(human=True)` records the backup before the launch; `ch_status` prints the loader line from the fake agent's `status`; the backup on temp folders copies the two new files and restores them byte for byte; every existing test in the four test files passes unedited; the `launch.ts` fixture tests of `test_frida_attach` (no `add(0x` literal, no `.method(` outside its empty list) pass. Run: `py -m pytest tools/ch_mcp/tests/test_bot_run.py tools/ch_mcp/tests/test_game_files.py tools/ch_mcp/tests/test_game_tools.py tools/ch_mcp/tests/test_frida_attach.py -q`.

### D4: the kit's player argument (earlyStart: yes)

Owns `tools/ch_mcp/kit_tools.py`, `tools/ch_mcp/tests/test_kit_tools.py`.

`ch_kit_setup(player=None)`; `None` sends `KIT_PLAYER`; a given name is sent as is and printed in the reply's first line. `KIT_PLAYER` stays `Player1`.

Tests, on the fake agent: `ch_kit_setup()` sends `Player1`; `ch_kit_setup(player="System")` sends `System` and the reply names it; a reply from the agent naming no such player is printed as the refusal it already is; `KIT_PLAYER` is a pinned literal; every existing test passes unedited. Run: `py -m pytest tools/ch_mcp/tests/test_kit_tools.py -q`.

### D5: wiring and docs (earlyStart: no; after D3 and D4)

Owns `tools/ch_mcp/server.py`, `tools/ch_mcp/tests/test_server.py`, `tools/ch_mcp/README.md`, `docs/agents/clone-hero-mcp.md`.

Registers the changed `ch_start` and `ch_kit_setup` signatures (no new tool; the tool count is unchanged, and `TOOL_NAMES` says so). The agent page's "The shape of a live run" gets the direct route as the way to start a song for a kit run, step 6 becomes "or `ch_open_song` on a day the direct route breaks" (or is struck, per question 4), and "Play stays offline" names the patch: one byte of the command-line loader, in memory only, from `launch_patch.json`. The README names `launch.ts`, `launch_patch.json` and the `build:launch` script. Tests: `test_server.py` passes with `TOOL_NAMES` unchanged. Run: `py -m pytest tools/ch_mcp/tests/test_server.py -q`.

### Order

D1, D3 and D4 start together from `main`. D2 starts from D1's branch after D1 merges. D5 after D3 and D4. After D3 merges the main session builds `launch.js` in the main checkout. The live steps wait for D5 and for D2's third trace (the online question) unless the user answers it another way.

## The live steps (main session; the user says in chat when a run may start; never any `ch_*` tool from a planning agent)

Each names its pass and fail lines, has a run cap and a minutes cap, and ends with `ch_stop`, whose reply must show every file line (seven now), the registry line, the kit teardown checks and the foreground count. A restore failure stops the sequence and goes to the user with the line. The game's MD5 (`Get-FileHash` on `GameAssembly.dll`) is taken before LS-D0 and after each step's `ch_stop`; a change is a fail of ruling 5 and stops everything.

### LS-D0: the suspended launch and the byte (2 runs, 10 minutes)

What only a run can settle: whether Frida attaches to a process Python created suspended, and how long the boot takes. `ch_start(<Window Map folder>, human=True)`, then `ch_status` every 5 s up to 60 s. Pass: the reply shows `before` `BA 01 00 00 00` and `after` `BA 00 00 00 00`, `ch_status` shows "loader ran after patch: yes" within 60 s, `ch_state` shows a live `GameManager` and a moving clock (the song started with no press; if the clock never moves while the loader line says yes, the song waits for a Ready press, and the report says so, with `ch_snapshot`), the foreground count is 0, and the MD5 is unchanged. Record the seconds from resume to the moving clock. Fail run 1: `frida.attach` ends at its cap, or the loader line says "no". Then run 2 is `frida.spawn` with the same command line if the user answered question 2 as recommended; its pass line is the same plus the foreground count, and a count above 0 ends the plan's launch question for the user. Fail on both: the late route goes back to the user as a new plan.

### LS-D1: a hit through the kit, and which player (2 runs, 10 minutes)

What only a run can settle: whether the System Player takes the kit. `ch_start(..., human=True)`, `ch_kit_setup(player="System")`, then `ch_kit_press("red")` timed at a red note by hand with `ch_state`'s clock, and a read of the score. Pass: the score rises or `ch_kit_press`'s `seen` shows the action true on the kit's player, and `IsBot` is 0 for the slot (read through the state agent if D2's trace names the slot; else the Star Power meter of LS-D2 is the proof). Then `KIT_PLAYER` is recorded as `System` in a one-line data commit quoting the reply. Fail run 1: no hit and no `seen`. Run 2 uses `Player1` and the field set of slot +0x18 from `launch_patch.json`'s `sets` (D2 says where the slot is read); pass and the data commit record `Player1` and the set. Fail on both: the report quotes both replies and goes to the user.

### LS-D2: Star Power on Window Map (2 runs, 15 minutes)

`ch_start(..., human=True)`, `ch_kit_setup`, `ch_sp_watch_start`, `ch_play_chart(<Window Map score JSON>, song_folder=..., route="kit")`, armed before the resume if LS-D0's boot time fits `ENGINE_SEARCH_CAP_S`, else right after `ch_kit_setup`. Pass: `ch_play_status` ends `done`, `ch_sp_watch_status` lists at least one rise of the meter and one activation, `ch_compare` passes against Hydra's score file, the hit count is 142 or the missing rows are all before the kit's setup time, and the foreground count is 0. Fail: no rise of the meter with hits landing (the kind is not 0: report `ch_status`'s loader line and go to the user); or hits missing after setup (report the rows' frames as LS-K4 says). Stop line, before the run: D2's online trace must be on page 07, or the user has answered the network question.

### LS-D3: nothing left behind (1 run, 5 minutes; inside LS-D2's `ch_stop`)

Pass: every file line says "unchanged" or "restored", `profiles.ini` and `profiles.bin` among them, the registry line is one of the two, the teardown checks are all "ok", and the MD5 matches. Fail: any other line; it goes to the user with the line and the backup folder.

### LS-K4 and LS-K5

They stand as written in `2026-10-30-ch-mcp-in-game-input.md`, with the song started by `ch_start(..., human=True)` instead of `ch_start_song` or the kit walk.

## Out of scope (needs the user's yes to leave out)

- The late route (writing `is_bot` 0 between the loader and `CreateEngine`); only if LS-D0 fails on both launch forms.
- Deleting the menu walk, the method watch or T8 (question 4).
- A network block for the run; only if D2's third trace cannot say whether a human score posts.
- Changing instrument or difficulty by anything but `--player`.
- Build 1.0 (4045): the loader's addresses are for 6142 only, and `launch_patch.json` is refused for any other build by the existing dump check.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| The byte is patched in memory before the loader runs, and the order is observable | Question 1; The launch | D3; LS-D0 | `test_bot_run`: recorded order attach, load, patch, resume; LS-D0 "loader ran after patch: yes" |
| The game file on disk is never written | Ruling 5 | D3; every live step | `test_bot_run` grep test for a write open of `GAME_EXE`; MD5 before and after each step |
| The patch refuses when the bytes are not what the file expects, and the game never runs then | The launch | D3 | `test_bot_run`: a differing `before` is refused naming the bytes, the process is killed, `resume_thread` never called |
| Every address the agent touches comes from the data file, none from a literal | The launch | D3 | `test_bot_run`: each RVA in `launch_patch.json` has a facts row; `test_frida_attach`: no `add(0x` in `launch.ts` |
| The launch stays minimized and not active | Ruling 1 | D3; LS-D0 to LS-D3 | `test_bot_run`: fake `Popen` sees `STARTF_USESHOWWINDOW`, 7 and `CREATE_SUSPENDED`; foreground count 0 |
| `human=True` without a song refuses | The launch | D3 | `test_game_tools`: the refusal names the reason |
| The backup is taken before a human launch | Safety rules kept | D3 | `test_game_tools`: `ensure_backup` recorded before `launch_human` |
| Every wait has a cap and names it | Preamble rule | D3 | `test_bot_run`: attach and patch each end at their cap naming it |
| `profiles.ini` and `profiles.bin` are backed up and restored | Question 3 | D3; LS-D3 | `test_game_files`: the two copied and restored byte for byte; LS-D3's lines |
| The kit's player is a parameter with one default | Question 2 | D4; LS-D1 | `test_kit_tools`: `None` sends `Player1`, a name is sent as is; the data commit quotes LS-D1 |
| The game facts are on page 07 with a facts row each and a listing | The write-back | D1, D2 | `hydra_tests.exe "-tc=docs match code*"`; `dump_facts.py --check`; reviewer reads blocks against listings |
| IN-38 is corrected only on a read of the bytes | Leads | D1 | D1's report quotes the disassembly at 0x114703 and 0x114CC9 or keeps the label |
| Each lead is checked before it is written as a fact | Leads | D1, D2 | each block's label and evidence line; a trace out of budget is Unknown with where we looked |
| No human run before the online question is answered | Question 3 | main session; LS-D2 stop line | the LS-D2 report quotes D2's block or the user's answer |
| The agent page and README describe the direct route and the patch | D5 | D5 | the doc reviewer reads them against this plan |
| No new tool; the tool count is unchanged | D5 | D5 | `test_server.py` `TOOL_NAMES` |
| Every live step has pass, fail, a run cap and a minutes cap, and ends with the restore lines | Live steps | main session | each step's report names its count, minutes and deciding reply |
| No planning agent and no task touches the running game | Brief | all | every task is offline on fakes; the live steps are the main session's |
| Nothing runs over the user's library | Rule 8 | all | every tool takes one folder |

## Prior art

Frida's documentation describes spawning a process suspended and resuming it after the instrumentation is in place as its early-instrumentation flow (`frida.spawn`, `Device.resume`), and the installed 17.18.0 exposes both. Windows' `CREATE_SUSPENDED` creation flag starts a process with its main thread suspended until `ResumeThread`, the same primitive Frida's spawn uses. frida-il2cpp-bridge's `Il2Cpp.perform` runs after `il2cpp_init` and before managed code, which is why the patch lands before the first scene. Rewired's `GetSystemPlayer` returns the System Player as an ordinary `Player`, so `AddController` on it is the same call as on Player1; whether it reads a custom controller is LS-D1's question.
