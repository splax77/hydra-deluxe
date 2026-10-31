# Plan: start the song straight from the command line, as a human player (2026-10-31)

Task id `ch-direct-start-plan`, session `b15bb484`. Every address below was read in `GameAssembly.dll` of build 1.1.0.6142 with `tools/ch_mcp/chdis.py` (capstone) during this session, or is marked as a lead that nobody has checked yet. The code paths in `tools/ch_mcp` were read on `main` at 0adb8fc4.

## The plain version

The user asked why the tools walk the menus at all, and said we had been accepting another agent's framing without checking it. The answer from the code is: the game already has a direct route. `Clone Hero.exe --song <folder> --player ProDrums,Expert` loads the song with no menu. The one obstacle found in the code is a single instruction. The command-line player loader, at RVA 0x3968D0, sets the player's `is_bot` setting to 1 when no replay was given. One byte of that instruction decides it: the `1` at 0x396EC6.

So the plan is: start the game with its main thread paused, attach Frida, and tell Frida to watch for `GameAssembly.dll` (the file that holds all of the game's own code). Let the game go. The moment the game maps that file, and before the game has used it, Frida changes that one byte in memory to 0. The loader then builds a human player on ProDrums Expert, the engine's player kind is Player (0), and the Star Power gain code, which skips only kinds 2 and 1, no longer skips it. That the meter then fills and activates in a real run is LS-D2's pass line, not yet seen. The game file on disk is never changed. The kit gives the player its input, as it does today, with one open question about which Rewired player it must sit on. The menu walk, the method watch and the start-method study are not needed for this; what happens to them is the user's call.

The byte cannot be changed while the game is still paused. A game created paused has not mapped `GameAssembly.dll` yet: the program file lists only `UnityPlayer.dll` and `KERNEL32.dll` to load at start-up, and `UnityPlayer.dll` loads the game's code later. So the order is: pause, attach, arm the watch, resume, patch on load. Which parts of that order are checked in code and which only a live run can settle is written in section 1 of "The open questions".

A live run is spent only on what the code cannot answer. There are four such things, each named below with the reason: whether Frida attaches to a process we created paused and gets its watch called on the load, whether the kit must sit on the System Player or Player1, whether the song starts by itself with no Ready press, and whether the patch lands before the loader runs.

## Rulings that bind this plan

1. Runs never need focus (2026-10-30, memory `ch-runs-must-not-need-focus`, plan `2026-10-30-ch-mcp-in-game-input.md` ruling 1). The game window is never shown or raised; the foreground count is a pass line of every live step.
2. The key route stays locked behind a typed `allow key run` (the in-game-input plan, question 1 and 6, answered 2026-10-30). This plan presses nothing through Windows.
3. Reverse engineering may proceed (2026-10-28, "Proceed, my call", memory `ch-mcp-feasibility-2026-10`). The disassembly reads below rest on it.
4. What the user has allowed so far, and what this plan adds. The trace hook patches a method start in memory while attached, approved 2026-10-28 (`docs/agents/clone-hero-mcp.md`, "The Frida agents"); that hook only records. D272 (`docs/audit/2026-10-03-fix-decisions.md`) says in its headline that the `ch` server may "write game memory", item 2 lets agents call game methods, and item 5 lets them "set game fields where a feature needs it". None of those names a change to the game's code that alters what the game does. This plan's byte is exactly that: it changes who the loader makes the player. So the plan does not call it covered. It is question 6 below, and nothing is built before the user answers it.
5. A patch of the game file on disk is ruled out (the brief of this plan). Nothing here opens `GameAssembly.dll` for writing, and a test says so.
6. D272 ruling 1: the server never opens the Online menu, Settings, Practice or the control mapper. The direct route opens no menu at all.
7. Nothing is built until the user says yes (rule 3 of `CLAUDE.md`).

## What was verified in the code, and how

These were read in this session's disassembly (scratch listing of 0x396E60 to 0x396F80 and 0x396110 to 0x3962D0) or by the main session, which re-read them before writing this brief. Every one goes onto page 07 in task D1 with a listing.

The loader's bot write. At 0x396EA0 and 0x396EA3 the loader writes the instrument byte to profile +0x10 and the difficulty byte to +0x11. At 0x396EA7 it compares the replay field `[rsi+0x60]` with null. When it is null, 0x396EB1 loads the `is_bot` setting from profile +0x48, 0x396EC5 is `mov edx, 1` (bytes `BA 01 00 00 00`, so the `1` sits at 0x396EC6), and 0x396ECA calls the setting's int setter at 0xFE5F50. Then 0x396ECF writes 1 to profile +0x14. At 0x396ED6 and 0x396EDB it compares the instrument with 6 and with 9; both jump to 0x396F1A. The scout's claim that ProDrums takes the drums branch is now checked: it does. That branch calls the same setter with 2 on the `controller_type` setting at profile +0x60 (0x396F27 to 0x396F33) and writes 2 to +0x14 (0x396F38). The other instruments set `controller_type` to 1 or 0 by a bit mask at 0x396EE5. When a replay is given (0x396F44 on), the loader skips the bot write and takes `controller_type` from the replay's byte +0x6C. What profile +0x14 means is unknown; its two values track "drums or not", and D2 names the field from the dump.

The `--player` parser at 0x396110 lower-cases the two comma-split words and matches them by string hash. `prodrums` gives 9 (0x3962CD), and the string table it builds names `drums`, `expert`, `hard`, `medium`, `easy` and the rest. `launch_command` in `tools/ch_mcp/bot_run.py` sends the `player` it is given, which `ch_start` defaults to `bot_run.DEFAULT_PLAYER` (`Drums,Expert`); a caller passes `ProDrums,Expert` for a pro-drums chart. Both words (6 and 9) take the loader's drums branch.

The main session verified, and this plan builds on: the loader fetches the Rewired System Player (call at 0x396D55 to `ReInput.PlayerHelper.GetSystemPlayer`, dump line 310436; this session confirmed the call and that the result is kept in a local, and did not trace where it goes next); the profile constructor builds the `is_bot` bool setting into profile +0x48 (0x2123CE3 to 0x2123D0A) with its value at setting +0x10; `IsBot` at 0x13F8B0 reads slot +0x10, then +0x48, then the bool getter 0x210BF30; the kind builder in `BasePlayer<DrumsInputs>`'s constructor 0x11676A0 (0x1167862 to 0x11678DB) writes kind 1 for a remote slot (0x337720), else 2 when `IsBot`, else 3 when a replay is set (0x13F8E0), else 0; the engine constructor copies it into engine +0x190 at 0x20F7BCB; the Star Power gain at 0x20F4FD5 skips kinds 2 and 1; `PlayerSelectionMenu` +0x1E8 copies the profile's `controller_type` in OnEnable (0x32C2D7) and Update (0x3315D3), and the profile screen's Controller row writes it (0x1E6C92).

The live path to the loader, verified by the main session: `UnpackingScene.MoveNext` 0x361C60, then 0x3980F0 and 0x398D30, the coroutine 0x3A5A20, 0x398F00, then 0x3968D0. UnpackingScene is the first scene the game runs. So the loader runs once, early, inside the game's first scene, and only when the command line names a song.

How the game loads its code, read from the files in `C:\Clone Hero\1.1 june\Clone Hero`. The import tables (the lists of DLLs a program asks Windows to load before it starts) were read with a small script this session. `Clone Hero.exe` lists `UnityPlayer.dll` and `KERNEL32.dll` only. `UnityPlayer.dll` lists no `GameAssembly.dll` either; it holds the text `GameAssembly.dll` beside the text `Failed to load il2cpp`, referenced from one spot (UnityPlayer RVA 0x805CFB, which sets up a string; the load call itself was not traced). So `GameAssembly.dll` is not mapped when a process is created, and the code facts stop there: that `UnityPlayer.dll` loads it at run time is the reading of those two strings, not a traced call. The installed bridge (`frida-il2cpp-bridge` 0.14.0, `dist/index.js` lines 1191 to 1236) takes the same view: its `Il2Cpp.initialize` waits for `GameAssembly.dll` with `Process.attachModuleObserver`, then hooks `il2cpp_init` and resolves on its return with `setImmediate`, which runs on Frida's own thread while the game's thread goes on. So `Il2Cpp.perform` does not hold the game back, and it is not a guarantee of "before managed code".

How the tools launch today: `bot_run._real_launch` calls `subprocess.Popen` with `SW_SHOWMINNOACTIVE` and nothing else. Frida attaches by process name (`frida_mod.attach(PROCESS_NAME)` in `frida_attach.py` lines 130 and 283), only when a tool asks, seconds after the launch; `LiveAgent` has no way to attach by process id, so task D3 adds one. Nothing today reaches the game before its first scene. The installed Frida is 17.18.0; `frida.spawn`, `Device.resume`, `enable_spawn_gating` exist. The backup list (`game_files.LOCALLOW_FILES`) holds `scoredata.bin`, `scoresext.bin` and `user.bin`; `DOCUMENTS_FILES` holds `settings.ini` and `scorestats.json`. That is five files. Neither `profiles.ini` nor `profiles.bin` is in it.

## Leads nobody has checked (each one is assigned to a task below, or named as a live question)

The scout's claims: the flag parser is 0x395C10; `--profile` matches `player_name` in the loaded profile list, else 0x1C9300 supplies one; `BasePlayer` 0x208F50 replays the chart as input each frame when the player is bot or remote and re-reads `is_bot` each frame; the engine is made in `DrumsPlayer.CreateEngine` 0x33C560 (call at 0x33C8D5); a compare scan found no other test of kind 2 but did not scan plain loads of +0x190; the profile clone at 0xD8470 has index -1; the stats record builder 0x1144C0 uses the "is_bot" string at 0x114703 and 0x114CC9. Task D2 checks each before it is written anywhere.

## The open questions, settled from code first

### 1. When the loader runs, and how the byte is 0 before it does

The loader is reached only from the first scene, after `GameAssembly.dll` is loaded and il2cpp is initialised. Two routes could change the byte in time.

The early route, in six steps. (1) Create the process paused: `subprocess.Popen` takes `creationflags=CREATE_SUSPENDED` (4) and keeps the `STARTUPINFO` we already pass, so the launch stays minimized and not active. (2) Attach Frida by process id and load `launch.ts`. (3) Tell `launch.ts` to arm: it calls `Process.attachModuleObserver` and waits for a module named `GameAssembly.dll`. (4) Resume the game's main thread. (5) When the game maps `GameAssembly.dll`, Frida calls the observer. Inside that call `launch.ts` checks the loader's first twelve bytes and the five bytes at 0x396EC5, writes `BA 00 00 00 00` with `Memory.patchCode`, and hooks the loader's entry with `Interceptor.attach`. (6) Python asks `launch.ts` for its status until it says patched, refused or the cap passes. The patch is never applied before the resume, because there is nothing to patch until the game has mapped the file.

What is checked, and what is not.
- Code fact, from the files: `GameAssembly.dll` is on neither import list (see "How the game loads its code"), so it is not mapped while the process is paused. A patch that waits for the file before the resume would wait for something that cannot load while the thread is stopped, and the launch would end at its cap.
- Code fact, from the loader's disassembly: the loader at 0x3968D0 is reached only through the first scene's coroutines (the chain above), so it cannot run before `GameAssembly.dll` is mapped and `il2cpp_init` has run.
- Documentation, not yet seen working here: Frida's `Process.attachModuleObserver` calls `onAdded` "synchronously right after that module has been loaded, but before the application has had a chance to use it" (frida.re JavaScript API page). That is why the patch goes inside the observer's call and does not wait on `Il2Cpp.perform`. The documentation does not say which thread makes the call or whether patching inside it is safe while Windows holds its loader lock. The bridge's own code defers its work with `setImmediate`, which hints at the same doubt.
- Live question, LS-D0: whether `frida.attach` works on a process Python created paused, before Windows has finished starting it; whether the observer is called for `GameAssembly.dll` at all in that case; whether the patch can be written inside the call or must be deferred with `setImmediate`; and whether it lands before the loader runs. The hook on the loader's entry answers the last one: it re-reads the five bytes each time the loader starts, and the status reply carries what it saw. A deferred patch would race the game's start-up; the loader is seconds later, but no code bounds that, so only the hook's reading counts.
- Live question, LS-D0: what Windows has loaded at the moment of the attach. The arm reply lists the module names it sees, so the report has them.

The late route: attach by name after a normal launch and write `is_bot` 0 on every slot's profile after the loader and before `CreateEngine`. The window is the song's load time, against Frida's attach time (the il2cpp bridge alone takes seconds). That is a race: it works on a slow song load and fails on a fast one, and nothing in the code bounds either side. It is not built. It stays here as the fallback a later plan may take if LS-D0 fails on both launch forms.

Why the one byte rather than a hook that writes `is_bot` 0 afterwards: the byte change needs no object lookup, no setter call and no main-thread hop, and it is the smallest change that does the job. Whether even that is inside what the user allowed is question 6. The hook on the loader's entry is read-only and exists only to prove the order.

If the paused attach does not work, LS-D0's later runs change one thing at a time, each set by a field in `launch_patch.json` so no code changes between runs. First `"defer": true`, which patches from a `setImmediate` call as the bridge does. Then `"launch": "spawn"`, which makes the process with `frida.spawn` and resumes it with `Device.resume`; that gives up `SW_SHOWMINNOACTIVE`, because the installed `spawn` takes `argv`, `envp`, `env`, `cwd`, `stdio` and open-ended `aux` options and lists no show-window one (whether `aux` hides one is not known). The foreground count then decides whether that launch is allowed at all: a count above 0 goes to the user before anything else runs.

### 2. The Rewired player

From code: the loader calls `GetSystemPlayer` at 0x396D55 and keeps the result (checked). Where that player goes next, and whether it is the player the drums input reads, is not traced here: the main session's note that "each slot gets the System Player" and the lead that the slot field is at +0x18 are unchecked until D2's first trace. The kit is added to Player1 (`KIT_PLAYER = "Player1"` in `kit_tools.py`). Rewired's `Player.ControllerHelper.AddController(Controller, bool)` (dump line 306697) is the same method for any Player, and `kit.ts` already enumerates every player including the System Player (its `allPlayers`, line 189). So the smaller change is one string: give the kit to the player named `System`. The alternative is to set the slot's Rewired player to Player1. That is a field set inside the game's own object, it must land before the drums player caches it, and the agents may name fields only, never raw offsets (the test in `test_frida_attach.py`), so D2 must give the field's name from the dump before this is ever built.

What the code cannot answer: whether the drums input reads the System Player at all, and whether Rewired lets a custom controller be added to it and reads it there. That is the one live question of LS-D1: a kit press on the System Player must register as a hit. If it does not, the plan stops for the user with a new plan for the Player1 route, because the field set is not designed yet (it needs D2's answer and the user's yes under question 6). It is not a run-time option of this plan.

LS-D1 run 1 answered part of this on 2026-10-31: the kit can be added to System, but the game loads no drum map for it there (rows P19 and P20). The choice between loading that map onto System and the Player1 route is now question 7.

### 3. What a human run leaves behind

Three places. Profiles: the loader clones a profile; whether the clone is ever saved to `profiles.ini` or `profiles.bin` is a lead (index -1). Task D6 adds both files to the backup list now (D2's trace shows the game keeps them in the Documents folder, so they belong in `DOCUMENTS_FILES`), so the run cannot leave a trace there whatever the trace finds, and D2 reads the save path to say which. Scores: `scorestats.json`, `scoredata.bin` and `scoresext.bin` are already backed up and restored byte for byte. Online: the game's log shows "Unauthorized" for analytics in bot runs, but that is an observation, not a rule. D2 traces the score submit path from the stats record builder 0x1144C0 and says whether a human score posts anywhere when nobody is logged in. Until D2 answers, LS-D2's stop line keeps the run on the test chart only, and the question of a network block goes to the user if D2 cannot settle it.

### 4. What the built pieces become

The direct route makes these unnecessary for Star Power runs: the menu walk (`ch_open_song`, `menu_steps.json`), the method watch (`ch_method_watch_start`, `ch_method_watch_stop`, `start_candidates.json`), `start_method.json`, task T8 `ch_start_song` of `2026-10-30-ch-mcp-star-power-and-song-start.md`, and live step LS-K3 of `2026-10-30-ch-mcp-in-game-input.md`. LS-K4 and LS-K5 keep their pass lines but start the song by this route. Retiring code is the user's call; it is question 4 below, with a recommendation. Nothing in this plan deletes any of it.

## The design

### The launch

`ch_start(song_folder, human=False)` gains `human`. With `human` true and a song folder, `bot_run.launch_human(cmd, env)` runs this sequence, and the order is the one code fact the tests pin:

1. `Popen` with `CREATE_SUSPENDED` added to today's start-up info (or, when `launch_patch.json` says `"launch": "spawn"`, `frida.spawn`).
2. `LiveAgent` attaches to the process by id and loads `agent/launch.ts`. This needs the new `pid` argument of `LiveAgent` (task D3). The cap is `LAUNCH_ATTACH_CAP_S`.
3. Python sends `arm` with the contents of `tools/ch_mcp/launch_patch.json`. The agent starts the module observer and replies `armed` with the module names it sees now. The cap is `LAUNCH_ARM_CAP_S`.
4. Python resumes the main thread. Python's `Popen` closes the thread handle, so the resume opens the process's one thread through `ctypes` (Toolhelp snapshot by pid, `OpenThread`, `ResumeThread`); `bot_run.resume_main_thread(pid)` owns that. A spawned process is resumed with `Device.resume`.
5. Python sends `status` every `LAUNCH_POLL_S` until the reply says `patched` or `refused`, for at most `LAUNCH_PATCH_CAP_S` counted from the resume. A reply of `refused`, or the cap, kills the process and names the bytes or the cap. The game is already running by then, with the original byte, so it would play as the bot; the kill happens within one poll.

Python's `Popen` handle and the `LiveAgent` stay in the run's state, so `ch_status` can ask `status` again and `ch_stop` detaches the agent before it stops the game. `ch_attach` is not changed: it opens its own session for the state agent, as today. Without `human`, nothing changes. The backup is taken before step 1, as for any launch.

The messages `launch.ts` understands are these. All are request and reply with an id, the shape `LiveAgent` already has; the agent never sends a message unasked.
- `read` (what `LiveAgent.start` sends first): replies `{state: "idle"}`.
- `arm` with `patch` (the file's contents): replies `{state: "waiting", modules: [...]}`, or an error naming the field that is missing.
- `status`: replies `{state, before, after, entry_before, module: {base, size}, loader_runs, bytes_at_first_run, patched_at_ms, reason}`. `state` is `waiting`, `patched` or `refused`. `reason` says why a status is `refused` (or what a hook read failed on). It is not called `error`, because `LiveAgent` raises on an `error` key in any reply, as the other agents rely on. `before` and `entry_before` are the bytes found at the patch and at the loader's entry when the module appeared. `after` is the bytes read back after the write. `loader_runs` counts calls of the loader since the hook was placed; `bytes_at_first_run` is the five bytes the hook read at the first of them.

When the observer sees `GameAssembly.dll` (the match is on the exact name), the agent does this in one go: read the twelve bytes at the loader's RVA and compare them with `entry_expect`; read the five bytes at the patch RVA and compare them with `expect`; if both match, write `write` with `Memory.patchCode`, read it back, and attach the read-only hook at the loader's RVA; if either differs, write nothing, place no hook and set `refused` with both byte strings. With `"defer": true` the same work runs from `setImmediate` instead of inside the observer's call.

`launch_patch.json` holds, for build 6142: `launch` (`"suspended"`), `defer` (`false`), the module name, the loader RVA 0x3968D0, `entry_expect` (`40 56 57 41 55 48 81 EC C0 00 00 00`, the loader's first twelve bytes read from the DLL), the patch RVA 0x396EC5, `expect` `BA 01 00 00 00`, `write` `BA 00 00 00 00`, and the dump line of the loader. The agent reads no address literal of its own: every RVA arrives in the message. `ch_status` prints the agent's reading as "loader ran after patch: yes" (`loader_runs` above 0 and `bytes_at_first_run` equal to `write`), "no" (the loader ran and the bytes were other) or "not yet". That line is LS-D0's pass line.

Why two byte checks: `BA 01 00 00 00` is the common instruction `mov edx, 1`, so the five bytes alone could match by chance at the same RVA in another build. The loader's own first twelve bytes make a wrong build very unlikely to pass. There is no other build check in the code today; the server never compares the game's version.

Boot takes seconds; LS-D0 measures the time from resume to the engine clock moving, so LS-D2 knows whether `ch_sp_watch_start` and `ch_play_chart` can be armed before the resume within `engine_finder.ENGINE_SEARCH_CAP_S`. As built, they cannot: `ch_start` returns only after the resume (row P38), so LS-D2 starts both after `ch_kit_setup`.

### The kit's player

`ch_kit_setup(player=None)` gains `player`; `None` means `KIT_PLAYER`. `KIT_PLAYER` stays `Player1` until LS-D1 decides, then a one-line data commit records the answer with the step's reply quoted. The agent already takes the player name in its `setup` message, so `kit.ts` does not change for this.

### Who owns which question

| Question | Owner | The other side |
|---|---|---|
| Which bytes are patched, what they must read first, and what they become | `tools/ch_mcp/launch_patch.json` | `launch.ts` refuses when `entry_before` or `before` differ; `test_launch_patch` checks the file |
| The order: pause, attach, arm, resume, wait for the patch | `bot_run.launch_human` | `test_bot_run` records the order on fakes |
| That the patch waits for `GameAssembly.dll` and never runs before the resume | `launch.ts` (the observer) | `test_frida_attach`: the fixture has an observer for the module name from the message and no patch outside it |
| Whether the loader ran after the patch | `launch.ts` `status` | `ch_status` prints it; LS-D0's pass line |
| The game file on disk is never written | ruling 5 | `test_bot_run`: no open of `GAME_EXE` for writing; LS-D0 compares the MD5 before and after |
| Which Rewired player holds the kit | `kit_tools.KIT_PLAYER`, `ch_kit_setup(player)` | LS-D1 decides and the data commit quotes it |
| The three launch caps | `bot_run.LAUNCH_ATTACH_CAP_S`, `LAUNCH_ARM_CAP_S`, `LAUNCH_PATCH_CAP_S` (question 1) | replies name the cap when it ends them |
| Which files a run can touch | `game_files.DOCUMENTS_FILES` gains `profiles.ini`, `profiles.bin` | LS-D3 restore lines |
| Whether a human score posts online | D2's trace, written on page 07 | LS-D2's stop line until then |
| The game facts | `docs/clone-hero/07-internals.md` and `code/facts-6142.tsv` | the docs test and `dump_facts.py --check` |

## Questions for the user

Answered 2026-10-31: shown all six with their recommendations, the user said "approve" (row P40: that session has no card yet, so the quote is not filed and the row is `unchecked`). So every question below takes its recommended answer, including question 6 (the one-byte patch, in memory only, is allowed), and the build may start.

1. **The three launch caps.** No ruling sets them, and none of the three has been measured; these are guesses LS-D0 then measures. Recommended: `LAUNCH_ATTACH_CAP_S = 20` (Frida's injection into a paused process plus loading the agent), `LAUNCH_ARM_CAP_S = 5` (one call that starts the observer; the same size as the existing `READ_CAP_S`) and `LAUNCH_PATCH_CAP_S = 30`, counted from the resume (the game's start-up until it maps `GameAssembly.dll`, then one read and write). The poll is `LAUNCH_POLL_S = 0.5`. The other side: the existing `READ_CAP_S` (5 s) for all three, but a cold Frida start or a slow game start on a busy machine passes 5 s and the run would fail for nothing.
2. **If the paused attach fails** (LS-D0), the later runs change one field of `launch_patch.json` at a time: first `"defer": true`, then `"launch": "spawn"`, which gives up the minimized, not-active launch. Recommended: run the spawn form once with the foreground watch as judge, and stop for good if the count is above 0. The other side: skip it and go to the late route (a race, see section 1 above).
3. **Profiles in the backup.** Recommended: add `profiles.ini` and `profiles.bin` to the backup list now, before D2 says whether the clone is saved. The other side: wait for D2; one more trace before the first human run.
4. **The built pieces.** Recommended: keep the menu walk, the method watch, `start_method.json` and T8 on disk and registered, mark them in `docs/agents/clone-hero-mcp.md` as the route for a day the direct route breaks, and strike LS-K3 and T8 from the two older plans with a one-line note pointing here. The other side: delete `ch_open_song`, `menu_steps.json`, the method watch and `start_candidates.json`, so there is one route. Both leave `menu.ts`'s reads, which `ch_menu` and the Online check use.
5. **Which chart LS-D2 plays.** Recommended: Window Map (142 hits with keys in L2), so LS-K4's pass line carries over. The other side: Senri, which LS-K5 needs anyway, with no hit count to compare. Replaced on 2026-10-31: Window Map has no Star Power phrases (row P25), so LS-D2 now plays the Senri chart (row P26), under the premise-map plan's question 2 (row P41).
6. **Is the one-byte patch allowed?** Ruling 4 says what D272 and the 2026-10-28 hook approval cover and what they do not. The patch changes the game's own code so that the loader makes a human player. Recommended: yes, as proposed: one byte, in memory only, checked against the bytes it must read first, with the disk file never touched. The other side: no, and the plan then needs the late route (set `is_bot` through the game's setter after the loader), which is a race that nothing in the code bounds, and no hook or code patch is placed at all. If the answer is no, this plan does not start.

Questions 7 to 11 were added on 2026-10-31 by the retrofit (task PM-4 of the premise-map plan). Answered 2026-10-31: shown all five with their recommendations, the user said "Take all five". So each takes its recommended answer: the kit stays on System with the drum map loaded onto it (7), the game's network was to be blocked before any step that finishes a song (8, since overruled), LS-D2g compares against a score file made with `--prodrums 1 --bass2x 1` and the recorded activations (9), both readings of "seen" stand and `docs/agents/premise-map.md` says so (10), and one focus grab at 0.0 s passes in every direct-start step (11). The map onto System is a new build, planned separately in `docs/superpowers/plans/2026-10-31-ch-mcp-kit-map-and-network-block.md` (task M1), and the kit steps wait for it. Later on 2026-10-31 the user overruled question 8: "all this network stuff is a waste of time. im trying to get functionality up to speed, not add blockers" (`docs/agents/rulings/clone-hero.md`). So no network block is built, and LS-D2d plays the song to its end with the game's network as it is.

7. **Which Rewired player gets the kit.** The loader puts the System Player in the slot, unless a launcher bool is set (IN-66, row P5), and the drums input reads that slot's player at each button read (IN-68, row P23). LS-D1 run 1 handed the kit to System, and the game gave it no drum map, because System's list of default maps for custom controllers is empty while Player1 to Player4 each list the Gameplay category with the Default and Drums layouts (rows P19 and P20). The drum map for the kit's definition 6 is category 1, layout 2, and Rewired's map helper has a `LoadMap` that takes ids or names for them (row P21). Recommended (the main session's view): keep the kit on System, the player the drums input reads, and load that one map onto it in `ch_kit_setup`, a small change to the kit's setup. At this end, a `kick` press reads `Orange` true on System and the hit trace shows a hit; whether it does is LS-D1a to LS-D1d's question (row P22), and nobody has seen it. The other side: the Player1 route, where the kit keeps its default map on Player1 and the slot's Rewired player is set to Player1 by a field set inside the game's own object. At that end the kit gets its map with no new call, but a press reaches the drums input only if the set lands before the drums player reads the slot, and the field set is not designed and needs the field's name from the dump (section 2).
8. **Can a song be played to its end before the online question is settled?** D2's trace did not settle whether a finished song posts anything online when nobody is logged in; page 07 IN-73 is labelled Unknown (row P39). The direct callers found write only local files (IN-72); a gateway session-request text at 0x285AD0 has no direct caller; and "Unauthorized" for analytics in bot runs is one logged observation (section 3), not a code fact. LS-D0 and LS-D1 stopped mid-song, but LS-D2d plays the Senri chart to its end, so the game's end-of-song code runs. Recommended: block the game's network for the run. At this end, a post that exists cannot leave the machine; how the block is made is not designed, so it is a new task before LS-D2d. The other side: run as is, on the reading that nobody is logged in and the gateway text has no direct caller, which is not a code fact. At that end, if the reading is wrong, a machine-played score could reach the user's online account and nothing in this plan could take it back; if it is right, the block would have cost a task for nothing.
9. **Which Hydra score file LS-D2d plays and compares against.** The direct start sets the DoubleKick modifier on the drums branch (row P2), and the Senri chart has 40 double-kick notes (row P28). Whether the game's DoubleKick means those notes are played, and whether `hydra_replay score --prodrums 1 --bass2x 1` prices the same chart, was not checked (row P35). The auto-player presses every chord, so the game activates Star Power at every fill reached with the meter at least half full (row P34), and a score file with no activations would not match that. Recommended: make the score file with `--prodrums 1 --bass2x 1`, play it, then make a second score file whose `--acts` are the activations `ch_sp_watch_status` recorded, and compare against that one. At this end the compare checks the score, so a pricing difference such as the 40 double-kick notes shows as a failed compare. The other side: compare only the hit count and the meter, and drop `ch_compare` from LS-D2 (LS-D2g). At that end such a difference passes unseen, but no run fails over a score file nobody has shown to be right.
10. **How a call is marked seen in this plan.** `docs/agents/premise-map.md` says a call is "seen" when "the call ran on this exact target before: the same player, the same route and the same chart", with the step and run that showed it named, and any call that has not run on this exact target is "never tried". A step may hold at most one never-tried call. Read to the letter, the first step on a new chart holds several. LS-D2a plays the Ladies Talk chart, and no run has played it, so `ch_start`, `ch_status`, `ch_attach`, `ch_state` and `ch_stop` are all new there, and a launch cannot be split into smaller steps. This plan reads the page more loosely in two ways. First, a call that takes no chart or player word counts as seen when it ran on the same kind of launch (a human direct start) with the same kit player: LS-D2a marks `ch_attach` seen from LS-D0 run 1, which ran on Window Map. Second, a call that an earlier step of this plan will prove is marked seen with that step's passing run, before the run exists: LS-D2b marks `ch_kit_setup` `(seen: LS-D1a's passing run)`, and that run has not happened. The later step does not start until that run's folder exists, and its go request names the folder. The docs test counts only the `(never tried)` markers, so it cannot tell either reading from the other. Recommended: accept both readings for this plan, and change the page to say so. The case against: a call marked seen on Window Map can still fail on a new chart, which is the kind of unchecked fact that cost earlier runs (Window Map had no Star Power phrases). The other side: keep the page as written. Then LS-D2a holds five never-tried calls, and each chart's first step is written as an exception the user allows by name.
11. **Does the launch-time focus grab pass in every direct-start step?** Ruling 1 makes a foreground count of 0 a pass line of every live step. LS-D0 run 1 and LS-D1 run 1 both counted 1, at 0.0 s after the watch started, which is the launch (row P16). After LS-D0 run 1 the user was asked "Count the launch-time focus grab as a pass, and go on to LS-D1?" and said "Yes, run LS-D1" (row P17). That answer is quoted for LS-D0 run 1 only. Recommended: a count of 1 at 0.0 s passes in every step that starts a direct launch, and any later grab, or a count above 1, fails; LS-D2d's pass line reads that way. The other side: the pass line stays 0 as written, and then no direct-start step can pass, because both launches so far counted 1, and the plan has no launch form known to avoid the grab (the spawn form of question 2 gives up the minimized launch).

## Tasks

Each task is one executor on Opus, under 100 tool calls and 20 minutes, in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`, owning only the files it names. Unit tests run on fakes and never start the game or attach Frida. No task runs the full suite. Every commit carries `Task:`, `Agent:` and `Session:` trailers.

### D1: the Clone Hero write-back (earlyStart: yes)

Owns `docs/clone-hero/07-internals.md`, `docs/clone-hero/code/facts-6142.tsv`, `docs/clone-hero/code/6142/cli_loader_3968D0.asm` and `.md` (new), `docs/clone-hero/code/6142/player_kind_11676A0.asm` and `.md` (new), `docs/clone-hero/code/README.md` (the two new listing rows), `docs/clone-hero/README.md` (the block counts and the write-back date).

Writes the listings with `dis_range.py`'s method (the `chdis.Image` disassembler; `py` without `-I`, because capstone sits in the user's site-packages) for 0x3968D0 to 0x396F80 (the loader from its first instruction, which D5's test reads) and 0x1167862 to 0x11678DB, each with its `.md` explaining the instructions that decide the rule. Adds blocks for the loader's bot write and its skip on a replay, the ProDrums-takes-the-drums-branch fact, the `--player` parser's words and values, the player-kind rule, the profile layout (+0x10 instrument, +0x11 difficulty, +0x14 unknown, +0x48 `is_bot`, +0x60 `controller_type`), the System Player hand-off, the Star Power gain's kind test, and the Controller row and menu copies of `controller_type`. Updates IN-31 (names the kind's source), IN-32 (the caller that passes Bot is now followed), IN-53 (the parser is read; label becomes Proven in the game's code), IN-57 (the OnEnable and Update copies, the Controller row). Corrects IN-38 only after disassembling 0x1144C0 around 0x114703 and 0x114CC9 and seeing the "is_bot" string reference; if the reference is not there, IN-38 keeps its label and the executor's report says what the bytes showed. Each new backticked address gets its row by running `py tools/ch_mcp/dump_facts.py --tsv`. Next free block id is checked by grep, never assumed.

Tests: `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"` passes; `py tools/ch_mcp/dump_facts.py --check` reports no difference; the reviewer reads each new block's address against the listing beside it.

### D2: three code traces (earlyStart: no; starts from `main` after D1 is merged, because it adds blocks to the same page and the same table)

Owns `docs/clone-hero/07-internals.md` (new blocks only), `docs/clone-hero/code/facts-6142.tsv`, `docs/clone-hero/code/6142/drums_input_source.asm` and `.md` (new), `docs/clone-hero/code/6142/profile_save.asm` and `.md` (new), `docs/clone-hero/code/6142/score_submit.asm` and `.md` (new), `docs/clone-hero/code/README.md`.

Trace one: where `BasePlayer<DrumsInputs>` reads its input. Start at the constructor 0x11676A0 and at `DrumsPlayer.CreateEngine` 0x33C560 (a lead), follow the System Player the loader fetches at 0x396D55 to the field that keeps it (the lead is slot +0x18, unchecked), name that field from the dump, find where the drums input reads the Rewired player, and say whether it is stored at creation or read each frame. Also check the lead that 0x208F50 replays chart notes as input when the player is bot or remote and re-reads `is_bot` each frame. Trace two: the profile save. Find the writer of `profiles.ini` (the string is in the string table) and say whether a clone with index -1 (the clone at 0xD8470, a lead) is written. Trace three: the score submit. From the stats record builder 0x1144C0, follow where the record goes and say whether anything is posted online with no login, and whether `is_bot` gates it. Each trace becomes a block with the right label; a trace that cannot be finished in budget is a block labelled Unknown whose evidence line says where the executor looked, and the report names it.

Tests: the same two as D1. The reviewer reads each listing against its block.

### D3: the launch agent (earlyStart: yes)

Owns `tools/ch_mcp/agent/launch.ts` and `tools/ch_mcp/agent/launch_core.ts` (both new), `tools/ch_mcp/agent/package.json` (the `build:launch` script), `tools/ch_mcp/launch_patch.json` (new), `tools/ch_mcp/frida_attach.py` (the new `pid` argument of `LiveAgent`), `tools/ch_mcp/tests/test_frida_attach.py`, `tools/ch_mcp/tests/test_launch_core.py` (new).

Builds the agent side of "The launch". `launch_core.ts` holds the decision as a plain function, `applyPatch(message, io)`, with no Frida in it. `io` is `{readBytes(rva, n), writeBytes(rva, bytes), hookEntry(rva)}`, and the function returns the `status` object. `launch.ts` is the glue: it answers `read`, `arm` and `status`; on `arm` it calls `Process.attachModuleObserver` and, when the module named in the message appears, calls `applyPatch` with an `io` made from `Memory.patchCode` on the module base plus the RVA from the message, and `Interceptor.attach` for the read-only entry hook; with `defer` true it calls `applyPatch` from `setImmediate`. No address literal appears in either file. `LiveAgent` gains `pid=None` beside `agent_path` and `log_path`: with a pid it attaches to that process id instead of `PROCESS_NAME`, and nothing else about it changes. The executor builds `launch.js` with `npm run build:launch` in its own worktree and reports the output; `agent/*.js` is git-ignored, so nothing compiled is committed. `launch_patch.json` is written exactly as "The launch" lists it.

Tests, on fakes and plain Node (version 26 runs `.ts` files that hold only type annotations; the test skips with its reason when `node` is missing): `applyPatch` with matching bytes writes `write` once, hooks the entry after the write, and returns `patched` with `before`, `after` and `entry_before`; with a differing entry it writes nothing, hooks nothing and returns `refused` naming both byte strings; with a differing patch site it does the same; `status` before the module appears returns `waiting`. `LiveAgent(pid=1234)` calls the fake Frida's `attach` with 1234, and with no pid with `PROCESS_NAME`. The `test_frida_attach` fixture tests that already scan every agent file (no `add(0x`, no `.method(` outside a list, no `NativeFunction`, `CALL_SITES` 0 for the two new files) pass on them. Two more text checks on `launch.ts`: it holds exactly one `attachModuleObserver` and exactly one `Memory.patchCode`, and that `patchCode` sits in the function the observer's callback and the `setImmediate` branch both call (so no patch can run outside the observer). Every existing test in the two Python files passes unedited. Run: `py -m pytest tools/ch_mcp/tests/test_frida_attach.py tools/ch_mcp/tests/test_launch_core.py -q`.

### D6: the paused launch and the backup list (earlyStart: yes)

Owns `tools/ch_mcp/bot_run.py`, `tools/ch_mcp/tests/test_bot_run.py`, `tools/ch_mcp/game_files.py` (the two file names), `tools/ch_mcp/tests/test_game_files.py`, `tools/ch_mcp/game_tools.py` (`ch_start`'s `human` argument, the "loader ran after patch" line in `ch_status`, and the detach in `ch_stop`), `tools/ch_mcp/tests/test_game_tools.py`.

Builds the Python side of "The launch". `Env` gains `start_process`, `attach_agent`, `resume_thread` and `kill` seams so the tests use fakes; `attach_agent` calls `LiveAgent(agent_path, log_path=..., pid=pid)`, the signature D3 adds. The three caps and the poll interval are constants with the values question 1 settles. Adds `profiles.ini` and `profiles.bin` to `DOCUMENTS_FILES` (the game's folder for them, page 07 IN-70) if question 3 is answered as recommended.

Tests, on fakes: `launch_human` passes `CREATE_SUSPENDED` and the same `STARTUPINFO` fields as today (`STARTF_USESHOWWINDOW`, 7); the recorded order is start, attach, load, arm, resume, status, and `resume_thread` is never called before the `armed` reply; a `status` reply of `refused` ends the launch naming the bytes and kills the process; an attach that never returns ends at `LAUNCH_ATTACH_CAP_S` naming it and kills the process; an `arm` reply that never comes ends at `LAUNCH_ARM_CAP_S` the same way, and `resume_thread` is not called; a `status` that stays `waiting` ends at `LAUNCH_PATCH_CAP_S` counted from the resume, the same way; `launch_patch.json`'s `launch` of `"spawn"` takes the spawn seam and resumes through the device, not the thread; no function in `bot_run.py` opens `GAME_EXE` for writing (a grep test for `open(` with a write mode and for `GAME_EXE`, and the reviewer reads the diff); `ch_start(human=True)` with no song folder refuses, naming the reason (the loader runs only with `--song`); `ch_start(human=True)` records the backup before the launch; `ch_status` prints the loader line from the fake agent's `status`, all three of "yes", "no" and "not yet"; `ch_stop` detaches the launch agent before it stops the game; the backup on temp folders copies the two new files and restores them byte for byte; every existing test in the three test files passes unedited. Run: `py -m pytest tools/ch_mcp/tests/test_bot_run.py tools/ch_mcp/tests/test_game_files.py tools/ch_mcp/tests/test_game_tools.py -q`.

### D4: the kit's player argument (earlyStart: yes)

Owns `tools/ch_mcp/kit_tools.py`, `tools/ch_mcp/tests/test_kit_tools.py`.

`ch_kit_setup(player=None)`; `None` sends `KIT_PLAYER`; a given name is sent as is and printed in the reply's first line. `KIT_PLAYER` stays `Player1`.

Tests, on the fake agent: `ch_kit_setup()` sends `Player1`; `ch_kit_setup(player="System")` sends `System` and the reply names it; a reply from the agent naming no such player is printed as the refusal it already is; `KIT_PLAYER` is a pinned literal; every existing test passes unedited. Run: `py -m pytest tools/ch_mcp/tests/test_kit_tools.py -q`.

### D5: wiring, the data-file test and docs (earlyStart: no; after D1, D3, D4 and D6 are merged)

Owns `tools/ch_mcp/server.py`, `tools/ch_mcp/tests/test_server.py`, `tools/ch_mcp/tests/test_launch_patch.py` (new), `tools/ch_mcp/README.md`, `docs/agents/clone-hero-mcp.md`.

Registers the changed `ch_start` and `ch_kit_setup` signatures (no new tool; the tool count is unchanged, and `TOOL_NAMES` says so). The agent page's "The shape of a live run" gets the direct route as the way to start a song for a kit run, step 6 becomes "or `ch_open_song` on a day the direct route breaks" (or is struck, per question 4), and "Play stays offline" names the patch: one byte of the command-line loader, in memory only, from `launch_patch.json`. The README names `launch.ts`, `launch_core.ts`, `launch_patch.json` and the `build:launch` script.

`test_launch_patch.py` checks the data file against the rest of the repo, which is why it waits for D1: the file parses; its `expect` is `BA 01 00 00 00`; its `write` differs from `expect` in exactly one byte (the byte at +1); its loader RVA has a method row in `docs/clone-hero/code/facts-6142.tsv`; the listing `docs/clone-hero/code/6142/cli_loader_3968D0.asm` has a line at the patch RVA that reads `mov    edx, 1`; and that listing opens with the instructions `push rsi`, `push rdi`, `push r13` and `sub rsp, 0xc0`, which is what `entry_expect` (`40 56 57 41 55 48 81 EC C0 00 00 00`, read from the DLL's file bytes at the loader's offset by this plan's reviewer) encodes. The listing holds mnemonics and no bytes, and the game is not in the repo, so the twelve bytes themselves are pinned as a literal in the test with a comment naming the DLL's md5 from the facts table's header. It computes none of these from the file it checks. Tests: `test_server.py` passes with `TOOL_NAMES` unchanged, and the new test passes. Run: `py -m pytest tools/ch_mcp/tests/test_server.py tools/ch_mcp/tests/test_launch_patch.py -q`.

### Order

D1, D3, D4 and D6 start together from `main`; their files do not overlap. D2 starts from `main` once D1 is merged (D1 and D2 both own `07-internals.md` and `facts-6142.tsv`, so they never run at once). D5 starts after D1, D3, D4 and D6 are merged. After D3 merges the main session builds `launch.js` in the main checkout. The live steps wait for D5 and for D2's third trace (the online question) unless the user answers it another way.

## The live steps (main session; the user says in chat when a run may start; never any `ch_*` tool from a planning agent)

Each names its pass and fail lines, has a run cap and a minutes cap, and ends with `ch_stop`, whose reply must show every file line (five today, seven after D6), the registry line, the kit teardown checks and the foreground count. A restore failure stops the sequence and goes to the user with the line. The game's MD5 (`Get-FileHash` on `GameAssembly.dll`) is taken before LS-D0 and after each step's `ch_stop`; a change is a fail of ruling 5 and stops everything. No step starts before the user has answered question 6 (answered "approve" on 2026-10-31; row P40 stays unchecked until that session's card quotes the words). The foreground count in every `ch_stop` reply reads 1 at 0.0 s on both direct starts so far (row P16); question 11 asks whether that counts as a pass.

### What the retrofit changed, and why the order is new

Task PM-4 of `docs/superpowers/plans/2026-10-31-premise-map.md` (removed; `git show 85fa0fe0:docs/superpowers/plans/2026-10-31-premise-map.md`) rewrote this section on 2026-10-31, after LS-D0 run 1 and LS-D1 run 1. The user approved it by answering that plan's question 2 (row P41). Three things changed.

First, each step now tries one new tool call. The old LS-D1 chained four calls no human launch had used (`ch_kit_setup` on a new player, `ch_trace_start`, `ch_kit_press`, `ch_notes`), and the old LS-D2 chained six. A failure there could not say which piece broke. So the old LS-D1 is now LS-D1a to LS-D1d, and the old LS-D2 is now LS-D2a to LS-D2g. Some of the new steps run inside an earlier step's game, after that step's pass line, so the split costs few extra launches. Each step's `Calls:` line lists its calls in order, with the one new call marked `(never tried)` (`docs/agents/premise-map.md`, "The call list of a live step").

Second, LS-D2 plays the Senri chart, not Window Map. Window Map has no Star Power phrases (row P25), so the old pass line could never be met. The Senri chart, "Ladies Talk", has 25 (row P26). That replaces the answer to question 5.

Third, the kit's player is open again. LS-D1 run 1 showed that the kit on the System Player gets no drum map (rows P19 and P20). Which way to go next is question 7, and no kit step runs before the user answers it.

How a call is marked here. A call that takes a chart or a player word (`ch_start`, `ch_play_chart`, `ch_trace_start`) counts as seen only on that same chart and player word. A call that takes neither is judged on the launch kind (a human direct start) and the kit's player. A `seen` marker that names a step of this plan still to run, such as `(seen: LS-D1a's passing run)`, means that step must pass first; the later step does not start until that run's folder exists, and the go request names the folder. Both readings are question 10, because `docs/agents/premise-map.md` asks for "the same chart" on every call and names only runs that have happened.

### LS-D0: the paused launch and the byte (3 runs, 15 minutes)

Calls: `ch_start(<Window Map folder>, human=True)` (seen: LS-D0 run 1), `ch_status` (seen: LS-D0 run 1), `ch_attach` (seen: LS-D0 run 1), `ch_state` (seen: LS-D0 run 1), `ch_stop` (seen: LS-D0 run 1)

Done. Run 1 on 2026-10-31 met every pass line but the foreground count: the game came to the front once, at the launch (row P16). The user, shown that, said "Yes, run LS-D1", which accepts the launch-time focus grab as a pass (row P17). The result is on page 07 as IN-74 (rows P15 and P18). Runs 2 and 3 are not needed.

What only a run can settle: the four live questions of section 1. Run 1: `ch_start(<Window Map folder>, human=True)`, then `ch_status` every 5 s up to 60 s from the resume. Pass: the status shows `before` `BA 01 00 00 00` and `after` `BA 00 00 00 00`, `entry_before` equal to `entry_expect`, "loader ran after patch: yes" within 60 s, `ch_attach` then `ch_state` show a live `GameManager` and a moving clock (the song started with no press; if the clock never moves while the loader line says yes, the song waits for a Ready press, and the report says so, with `ch_snapshot`), the foreground count is 0, and the MD5 is unchanged. Record: the module names the arm reply listed, the seconds from resume to `patched`, and the seconds from resume to the moving clock. Stop line: a state of `refused` ends the step and goes to the user with both byte strings, because the build differs from 6142. Fail run 1: the attach or the arm ends at its cap, the patch cap passes with the state still `waiting`, the agent errors inside the observer, or the loader line says "no". The report then names which. Run 2, only after a fail of run 1, sets `"defer": true` in `launch_patch.json` (the patch moves out of the observer's call). Run 3, only after a fail of run 2 and only if the user answered question 2 as recommended, sets `"launch": "spawn"` as well; its pass line is the same plus the foreground count, and a count above 0 ends the plan's launch question for the user. Fail on all three: the late route goes back to the user as a new plan.

### LS-D1a: the kit's setup on the player question 7 picks (2 runs, 10 minutes)

Calls: `ch_start(<Window Map folder>, human=True)` (seen: LS-D0 run 1), `ch_attach` (seen: LS-D0 run 1), `ch_kit_setup(player=<question 7's player>)` (never tried), `ch_stop` (seen: LS-D0 run 1)

Run 1 of the old LS-D1 counts as this step's first run. It handed the kit to `System` with no map load, and the setup refused because the kit had no map (row P19). The next run waits for the user's answer to question 7 and for the code change that answer needs (row P22). No press is sent in this step. Pass: the setup reply prints all ten bindings matching `KIT_BINDINGS`, every player's keyboard and mouse off, and the teardown in `ch_stop` all "ok". Fail: a refusal or a binding that differs; the report quotes the setup reply and the kit agent's log, and the step goes back to the user. Stop line, before the run: question 7 is answered and its change is merged.

### LS-D1b: one press, read back on the kit's player (2 runs, 10 minutes)

Calls: `ch_start(<Window Map folder>, human=True)` (seen: LS-D0 run 1), `ch_attach` (seen: LS-D0 run 1), `ch_kit_setup(player=<question 7's player>)` (seen: LS-D1a's passing run), `ch_kit_press("kick", read="Orange")` (never tried), `ch_stop` (seen: LS-D0 run 1)

What only a run can settle: whether a press on the kit reaches the player question 7 picks. The press is the kick, because every note of Window Map is a kick (row P46), so LS-D1d can hit one with the same press. The kick's game action is `Orange`, not "Kick" (row P37). Pass: the press reply's `seen` shows `Orange` true on the kit's player during the hold. Fail: `Orange` never reads true; the report quotes the reply. LS-D1c and LS-D1d run inside this step's game, after its pass line, before its `ch_stop`.

### LS-D1c: the hit trace starts (inside LS-D1b's run)

Calls: `ch_trace_start(<Window Map folder>)` (never tried)

Window Map's folder holds the manifest the trace reads. Pass: the reply says the trace started. Fail: an error or a refusal; the report quotes it, and LS-D1d does not run.

### LS-D1d: a hit through the kit (inside LS-D1b's run)

Calls: `ch_state` (seen: LS-D0 run 1), `ch_kit_press("kick")` (seen: LS-D1b's passing run), `ch_notes` (never tried)

What only a run can settle: whether the drums input reads the kit on the player question 7 picks. No tool reads the score or `IsBot` today, so the proof is the hit trace. The press is timed at a kick note by hand against `ch_state`'s clock; Window Map's note times are in its manifest (row P44). Pass: `ch_notes` lists a hit at that press. Then `KIT_PLAYER` is recorded as question 7's player in a one-line data commit that quotes the reply. Fail: no hit in `ch_notes`; the report quotes both replies and the plan stops for the user.

### LS-D2a: the Senri chart starts by the direct route as ProDrums (2 runs, 10 minutes)

Calls: `ch_start(<Ladies Talk folder>, human=True, player="ProDrums,Expert")` (never tried), `ch_status` (seen: LS-D0 run 1), `ch_attach` (seen: LS-D0 run 1), `ch_state` (seen: LS-D0 run 1), `ch_stop` (seen: LS-D0 run 1)

The folder is `C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check`, the same chart LS-K5 plays (row P26). The player word is named, because the default `Drums,Expert` drops accents and ghosts (rows P4 and P32), and this is a pro-drums chart (row P29). No kit, no press. Pass: the status shows "loader ran after patch: yes", `ch_state` shows a moving clock, and the MD5 is unchanged. Fail: the launch or the patch fails, or the clock never moves; the report quotes `ch_status`. Stop line, before the run: question 8 is answered, because LS-D2d plays a song to its end (row P39).

### LS-D2b: the Star Power watch on the Senri chart (2 runs, 10 minutes)

Calls: `ch_start(<Ladies Talk folder>, human=True, player="ProDrums,Expert")` (seen: LS-D2a's passing run), `ch_attach` (seen: LS-D0 run 1), `ch_kit_setup(player=<question 7's player>)` (seen: LS-D1a's passing run), `ch_sp_watch_start(<Ladies Talk folder>)` (never tried), `ch_stop` (seen: LS-D0 run 1)

`ch_start` returns only after the resume, and `ch_attach` waits about 5 s after the launch, so nothing can be armed before the resume (row P38). The watch starts after `ch_kit_setup`. No press. Pass: the watch's CSV, read from disk, holds rows with a moving clock and an empty meter. Fail: the watch finds no engine or ends at its cap; the report quotes its reply. LS-D2c runs inside this step's game, before its `ch_stop`.

### LS-D2c: the watch's status (inside LS-D2b's run)

Calls: `ch_sp_watch_status` (never tried)

Pass: the reply names the engine's address and lists the rows the CSV holds. Fail: an error, or rows that disagree with the CSV; the report quotes both.

### LS-D2d: Star Power on the Senri chart (2 runs, 15 minutes)

Calls: `ch_start(<Ladies Talk folder>, human=True, player="ProDrums,Expert")` (seen: LS-D2a's passing run), `ch_attach` (seen: LS-D0 run 1), `ch_kit_setup(player=<question 7's player>)` (seen: LS-D1a's passing run), `ch_sp_watch_start(<Ladies Talk folder>)` (seen: LS-D2b's passing run), `ch_play_chart(<Senri score JSON>, song_folder=<Ladies Talk folder>, route="kit")` (never tried), `ch_sp_watch_status` (seen: LS-D2c's passing run), `ch_stop` (seen: LS-D0 run 1)

The score JSON is the one question 9 settles (rows P35 and P36). `ch_play_chart` is called right after `ch_sp_watch_start`. Pass: `ch_sp_watch_status` lists at least one rise of the meter and one activation, and the foreground count is 0 (question 11 says whether the launch-time grab at 0.0 s, which LS-D0 run 1 and LS-D1 run 1 both showed, still counts). A rise proves the player's kind is not Bot (rows P7 and P45) and that the game reads note 116 as a phrase (row P30). An activation needs a hit on a fill's activation note with the meter at least half full (rows P33 and P34). Fail: no rise of the meter with presses landing (the kind is not 0: report `ch_status`'s loader line and go to the user); or presses missing after setup (report the rows' frames as LS-K4 says). Run 2 is allowed once, only after a missed-hits fail that LS-K4's reading explains as the frame grain. LS-D2e, LS-D2f and LS-D2g run inside this step's game, before its `ch_stop`, and LS-D3 reads that `ch_stop`.

### LS-D2e: the play session's status (inside LS-D2d's run)

Calls: `ch_play_status` (never tried)

Pass: the session ends `done`, and every press row with `hit` 0 lies before the kit's setup time. Fail: rows with `hit` 0 after setup; the report gives their frames against the chart time.

### LS-D2f: the stats file after the song (inside LS-D2d's run)

Calls: `ch_wait_stats` (never tried)

Pass: the reply names a stats copy written after the song ended. Fail: it ends at its cap; the report quotes it.

### LS-D2g: the score against Hydra's (inside LS-D2d's run)

Calls: `ch_compare(<stats copy>, <Senri score JSON>)` (never tried)

Pass: the compare passes against the score file question 9 settles. Fail on the compare alone: the report quotes its three lines and goes to the user; the score file, not the run, may be what differs (question 9).

### LS-D3: nothing left behind (1 run, 5 minutes; inside LS-D2d's `ch_stop`)

Calls: `ch_stop` (seen: LS-D0 run 1)

Pass: every file line says "unchanged" or "restored", `profiles.ini` and `profiles.bin` among them, the registry line is one of the two, the teardown checks are all "ok", and the MD5 matches. Fail: any other line; it goes to the user with the line and the backup folder.

### LS-K4: the auto-player on Window Map

Calls: `ch_start(<Window Map folder>, human=True)` (seen: LS-D0 run 1), `ch_attach` (seen: LS-D0 run 1), `ch_kit_setup(player=<question 7's player>)` (seen: LS-D1a's passing run), `ch_play_chart(<Window Map score JSON>, song_folder=<Window Map folder>, route="kit")` (never tried), `ch_play_status` (seen: LS-D2e's passing run), `ch_wait_stats` (seen: LS-D2f's passing run), `ch_compare(<stats copy>, <Window Map score JSON>)` (seen: LS-D2g's passing run), `ch_stop` (seen: LS-D0 run 1)

It stands as written in `2026-10-30-ch-mcp-in-game-input.md`, with the song started by `ch_start(..., human=True)` instead of `ch_start_song` or the kit walk. It runs after LS-D2g, so its only new piece is `ch_play_chart` on Window Map.

### LS-K5: the Senri fill check

Calls: `ch_start(<Ladies Talk folder>, human=True, player="ProDrums,Expert")` (seen: LS-D2a's passing run), `ch_attach` (seen: LS-D0 run 1), `ch_kit_setup(player=<question 7's player>)` (seen: LS-D1a's passing run), `ch_sp_watch_start(<Ladies Talk folder>)` (seen: LS-D2b's passing run), `ch_play_chart(<Senri score JSON>, song_folder=<Ladies Talk folder>, route="kit")` (seen: LS-D2d's passing run), `ch_sp_between(114778.9, 166544.0)` (never tried), `ch_stop` (seen: LS-D0 run 1)

It stands as written in `2026-10-30-ch-mcp-in-game-input.md`, with the song started by `ch_start(..., human=True)` and the same player word as LS-D2a. It runs after LS-D2d, so its only new piece is `ch_sp_between`.

## Out of scope (needs the user's yes to leave out)

- The late route (writing `is_bot` 0 between the loader and `CreateEngine`); only if LS-D0 fails on all its runs, or the user answers question 6 no.
- The Player1 route for the kit (setting the drums player's Rewired player by field); only if LS-D1 fails, and then as a new plan.
- Deleting the menu walk, the method watch or T8 (question 4).
- A network block for the run. The user ruled it out on 2026-10-31 (question 8).
- Changing instrument or difficulty by anything but `--player`.
- Build 1.0 (4045): the loader's addresses are for 6142 only. Nothing in the server compares the game's version today; a wrong build is stopped by the two byte checks of "The launch" (the loader's first twelve bytes and the five at the patch), and the run is killed.

## Promise map

| Promise | Plan line | Task | Check that would fail if the promise broke |
|---|---|---|---|
| The patch is applied only inside the call Frida makes when `GameAssembly.dll` appears, never before the resume | Section 1; The launch | D3 | `test_frida_attach`: exactly one `attachModuleObserver` and one `Memory.patchCode` in `launch.ts`, the latter in the function only the observer and the `setImmediate` branch call |
| The game is resumed only after the agent has armed, and never before | The launch | D6 | `test_bot_run`: recorded order start, attach, load, arm, resume, status; `resume_thread` not called before the `armed` reply |
| Whether the patch landed before the loader ran is observable | The launch; section 1 | D3, D6; LS-D0 | `test_launch_core`: the hook is placed only after the write; `test_game_tools`: the three loader lines; LS-D0 "loader ran after patch: yes" |
| The game file on disk is never written | Ruling 5 | D6; every live step | `test_bot_run` grep test for a write open of `GAME_EXE`; MD5 before and after each step |
| The patch refuses, and writes nothing, when either byte check differs | The launch | D3, D6 | `test_launch_core`: a differing entry or patch site writes nothing, hooks nothing, returns `refused` with both strings; `test_bot_run`: a `refused` status kills the process |
| No address literal in the agent; every RVA arrives in the message | The launch | D3, D5 | `test_frida_attach`: no `add(0x` in the two new files; `test_launch_patch`: the loader RVA has a facts row and the patch RVA is `mov edx, 1` in D1's listing |
| The launch stays minimized and not active | Ruling 1 | D6; LS-D0 to LS-D3 | `test_bot_run`: fake `Popen` sees `STARTF_USESHOWWINDOW`, 7 and `CREATE_SUSPENDED`; foreground count 0 |
| `human=True` without a song refuses | The launch | D6 | `test_game_tools`: the refusal names the reason |
| The backup is taken before a human launch | The launch | D6 | `test_game_tools`: `ensure_backup` recorded before `launch_human` |
| Every wait has a cap and names it | Preamble rule | D6 | `test_bot_run`: attach, arm and patch each end at their cap naming it, and kill the process |
| `ch_stop` detaches the launch agent | The launch | D6 | `test_game_tools`: detach recorded before the stop |
| `profiles.ini` and `profiles.bin` are backed up and restored | Question 3 | D6; LS-D3 | `test_game_files`: the two copied and restored byte for byte; LS-D3's lines |
| The kit's player is a parameter with one default | Section 2 | D4; LS-D1 | `test_kit_tools`: `None` sends `Player1`, a name is sent as is; the data commit quotes LS-D1 |
| The kit on the player question 7 picks registers hits | Section 2; question 7 | LS-D1a to LS-D1d | `ch_kit_press` `seen` true (LS-D1b) and a hit in `ch_notes` (LS-D1d); the step fails otherwise |
| Each live step tries one new call | Live steps; premise-map plan question 2 | PM-4 | `hydra_tests.exe "-tc=docs match code*"`, the Premise map check |
| The game facts are on page 07 with a facts row each and a listing | The write-back | D1, D2 | `hydra_tests.exe "-tc=docs match code*"`; `dump_facts.py --check`; reviewer reads blocks against listings |
| IN-38 is corrected only on a read of the bytes | Leads | D1 | D1's report quotes the disassembly at 0x114703 and 0x114CC9 or keeps the label |
| Each lead is checked before it is written as a fact | Leads | D1, D2 | each block's label and evidence line; a trace out of budget is Unknown with where we looked |
| No human run before the online question is answered | Section 3 | main session; LS-D2 stop line | the LS-D2 report quotes the user's answer to question 8 (no block, 2026-10-31) |
| No build before the user answers question 6 | Ruling 4; question 6 | main session | the user's reply is quoted in the first task brief |
| The agent page and README describe the direct route and the patch | D5 | D5 | the doc reviewer reads them against this plan |
| No new tool; the tool count is unchanged | D5 | D5 | `test_server.py` `TOOL_NAMES` |
| Every live step has pass, fail, a run cap and a minutes cap, and ends with the restore lines | Live steps | main session | each step's report names its count, minutes and deciding reply |
| No planning agent and no task touches the running game | Brief | all | every task is offline on fakes; the live steps are the main session's |
| Nothing runs over the user's library | Rule 8 | all | every tool takes one folder |

## Premise map

The facts this plan stands on, in the shape `docs/agents/premise-map.md` owns. Added on 2026-10-31 by the retrofit (PM-4). A row marked `unchecked` is not settled; the step or question in its Status settles it.

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: With no replay, the command-line loader sets the profile's `is_bot` to 1 with `mov edx, 1` at 0x396EC5, so the byte that decides it is 0x396EC6 | data | `docs/clone-hero/07-internals.md` IN-61 | checked 2026-10-31, label Proven in the game's code |
| P2: Drums (6) and ProDrums (9) both take the loader's drums branch, which sets controller type Drums and the modifiers to DoubleKick | data | `docs/clone-hero/07-internals.md` IN-62 | checked 2026-10-31, label Proven in the game's code |
| P3: The `--player` parser matches two comma-split words lower-case; `prodrums` gives 9 and `drums` gives 6 | data | `docs/clone-hero/07-internals.md` IN-63 | checked 2026-10-31, label Proven in the game's code |
| P4: `ch_start` sends the player word it is given, and its default is `Drums,Expert` | code | `tools/ch_mcp/bot_run.py` line 60 (`DEFAULT_PLAYER`), `tools/ch_mcp/game_tools.py` line 248 | checked 2026-10-31 |
| P5: The loader stores the Rewired System Player in the slot's player field, unless a bool of the launcher is set; then it takes an entry of Rewired's player list. What sets the bool was not traced | data | `docs/clone-hero/07-internals.md` IN-66 | checked 2026-10-31, label Proven in the game's code |
| P6: The profile keeps instrument, difficulty, modifiers, `is_bot` and controller type at fixed places (+0x10, +0x11, +0x14, +0x48, +0x60) | data | `docs/clone-hero/07-internals.md` IN-65 | checked 2026-10-31, label Proven in the game's code |
| P7: A drums player's kind is RemotePlayer, then Bot when `is_bot` is not 0, then Replay, else Player (0) | data | `docs/clone-hero/07-internals.md` IN-64 | checked 2026-10-31, label Proven in the game's code |
| P8: The player screen copies the profile's controller type, and the profile screen's Controller row writes it | data | `docs/clone-hero/07-internals.md` IN-57 and IN-67 | checked 2026-10-31, both labelled Proven in the game's code |
| P9: The loader is reached only from the first scene's coroutines, starting at `UnpackingScene.MoveNext` 0x361C60 | unchecked | this plan's "What was verified in the code"; no page 07 block holds the chain (searched `docs/clone-hero/07-internals.md` for 0x361C60) | LS-D0: run 1 showed the loader running once, early (row P15), which is all the live steps use; the chain itself stays unread |
| P10: `GameAssembly.dll` is not loaded while the game is paused, so the patch can only wait for it | run | `hydra-data\ch-mcp-runs\2026-10-31\ls-d0-run1\replies.txt` line 8 (39 modules at the arm, none of them GameAssembly.dll) | checked 2026-10-31 |
| P11: The installed il2cpp bridge waits for `GameAssembly.dll` with `Process.attachModuleObserver` | code | `dist/index.js` line 1198 of `frida-il2cpp-bridge` under `tools/ch_mcp/agent` in the main checkout | checked 2026-10-31 |
| P12: `launch_human` starts the game paused, attaches, arms, resumes, and returns only once the status says patched | code | `tools/ch_mcp/bot_run.py` line 615 (`launch_human`) | checked 2026-10-31 |
| P13: The backup list holds `profiles.ini` and `profiles.bin` beside `settings.ini` and `scorestats.json` | code | `tools/ch_mcp/game_files.py` line 47 (`DOCUMENTS_FILES`) | checked 2026-10-31 |
| P14: The game keeps `profiles.ini` and `profiles.bin` in its Documents folder | data | `docs/clone-hero/07-internals.md` IN-70 | checked 2026-10-31, label Proven in the game's code |
| P15: A paused direct start patches the byte 0.5 s after the resume, the loader runs once on the patched byte, and the song plays with no press | run | `hydra-data\ch-mcp-runs\2026-10-31\ls-d0-run1\replies.txt` lines 10 to 27 | checked 2026-10-31 |
| P16: In LS-D0 run 1 and again in LS-D1 run 1 the game came to the front once, at 0.0 s after the watch started | run | `hydra-data\ch-mcp-runs\2026-10-31\ls-d0-run1\replies.txt` lines 19 and 30, `hydra-data\ch-mcp-runs\2026-10-31\ls-d1-run1\replies.txt` line 30 | checked 2026-10-31 |
| P17: The user accepted the launch-time focus grab as a pass | ruling | the user's words "Yes, run LS-D1", session `90d40bc9`, said after the main session showed LS-D0 run 1's result and asked "Count the launch-time focus grab as a pass, and go on to LS-D1?"; the PM-4 reviewer's brief quotes both and confirms them; no card holds them yet | checked 2026-10-31, from that brief's quote |
| P18: Page 07 holds LS-D0's result as a live-run block | data | `docs/clone-hero/07-internals.md` IN-74 | checked 2026-10-31, label Proven in a live run |
| P19: The kit on the System Player got no map, so `ch_kit_setup(player="System")` refused before any press | run | `hydra-data\ch-mcp-runs\2026-10-31\ls-d1-run1\replies.txt` lines 16 to 27 and `kit-1.jsonl` | checked 2026-10-31 |
| P20: System's default custom-controller maps are empty; Player1 to Player4 list category 1 with layouts 0 and 2, and the kit's drum map is definition 6 at category 1, layout 2 | data | `docs/clone-hero/07-internals.md` IN-76 | checked 2026-10-31, label Proven in a live run |
| P21: Rewired's map helper has a `LoadMap` that takes a category and a layout | data | `docs/clone-hero/07-internals.md` IN-76 (it cites `dump.cs` lines 303867 and 303889) | checked 2026-10-31, label Proven in a live run; the call itself was never made |
| P22: With the drum map loaded onto System, kit presses reach the drums input | unchecked | nothing yet; `docs/clone-hero/07-internals.md` IN-68 says this was not checked | question 7, then LS-D1a, LS-D1b and LS-D1d |
| P23: The drums input reads the slot's Rewired player again at each button read | data | `docs/clone-hero/07-internals.md` IN-68 | checked 2026-10-31, label Proven in the game's code |
| P24: The kit's player is a `ch_kit_setup` argument whose default is `KIT_PLAYER`, `Player1` | code | `tools/ch_mcp/kit_tools.py` line 57 | checked 2026-10-31 |
| P25: Window Map has no Star Power phrases: no `S` line, and 142 `N` lines under ExpertDrums | chart | `C:\Clone Hero\songs\Hydra Probe\Window Map\notes.chart` (md5 292c54ff4c4c3f36bdd5e89c6ae5f904) | checked 2026-10-31, counted by script |
| P26: The Senri chart has 25 Star Power phrases: 25 note-116 notes on its PART DRUMS track, and none on any other track | chart | `C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check\notes.mid` (md5 9569049a5f3100d1eb759359e6eaaef5, the same bytes as `testdata/input/common/IB24/T6/Senri Kawaguchi - Ladies Talk [Kiyo Sen] (Drumeo Performance)/notes.mid`) | checked 2026-10-31, counted by script; first phrase at tick 11520, last at tick 286320, resolution 480 |
| P27: The two run folders hold only `ch_start`, `ch_status`, `ch_attach`, `ch_state`, `ch_kit_setup` and `ch_stop`, so no other call has run on a human launch | run | `hydra-data\ch-mcp-runs\2026-10-31\ls-d0-run1\replies.txt` and `ls-d1-run1\replies.txt` (each call headed `== ch_...`) | checked 2026-10-31 |
| P28: The Senri chart has 40 double-kick notes (MIDI note 95) on PART DRUMS | chart | `C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check\notes.mid` | checked 2026-10-31, counted by script |
| P29: The Senri chart is marked pro drums | chart | `C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check\song.ini` (`pro_drums = True`) | checked 2026-10-31 |
| P30: The game reads note 116 on PART DRUMS as a Star Power phrase | unchecked | `docs/clone-hero/05-star-power.md` SP-07 is the only block that names note 116 as a phrase, and its label is Unknown (searched `docs/clone-hero/*.md` for "note 116") | LS-D2d, whose pass line is a rise of the meter |
| P31: Hydra reads note 116 as a Star Power phrase | code | `src/parse/song.cpp` line 683 (`kSpMarkerPitch`) | checked 2026-10-31 |
| P32: Plain Drums clears accents and ghosts; ProDrums keeps them | data | `docs/clone-hero/06-scoring.md` SC-15 | checked 2026-10-31, label Proven in the game's code |
| P33: A phrase adds a quarter of the full bar to the meter | data | `docs/clone-hero/05-star-power.md` SP-12 | checked 2026-10-31, label Proven in the game's code |
| P34: Drums activate only by hitting a fill's activation note, with Star Power able to activate | data | `docs/clone-hero/05-star-power.md` SP-17 | checked 2026-10-31, label Proven in the game's code |
| P35: The game's DoubleKick modifier plays the double-kick notes, and `hydra_replay score --prodrums 1 --bass2x 1` prices the same chart the run plays | unchecked | `docs/clone-hero/07-internals.md` IN-62 names the modifier, not what it does | question 9 |
| P36: `hydra_replay score` takes `--prodrums`, `--bass2x` and `--acts` | code | `tools/replay.cpp` line 100 (the usage text) | checked 2026-10-31 |
| P37: The kit's kick pad reads as the game action `Orange`, not "Kick" | code | `tools/ch_mcp/kit_tools.py` line 139 (`KIT_BINDINGS` element 22) | checked 2026-10-31 |
| P38: Nothing can be armed before the resume: `ch_start` returns after it, and `ch_attach` waits 5 s from the launch | code | `tools/ch_mcp/bot_run.py` line 615 (`launch_human`), `tools/ch_mcp/frida_tools.py` line 36 (`ATTACH_DELAY_S`) | checked 2026-10-31 |
| P39: Whether a finished song posts anything online when nobody is logged in | unchecked | `docs/clone-hero/07-internals.md` IN-73 is labelled Unknown | question 8 |
| P40: The user allowed the one-byte patch in memory | unchecked | this plan's question list reports "approve"; no card for session `b15bb484` sits in `docs/sessions/index.md` | question 6, once its card quotes the user's words |
| P41: The user approved the retrofit, which changes LS-D2's chart and the order of the live steps | ruling | the user's words "Approve all six" on `docs/superpowers/plans/2026-10-31-premise-map.md` (removed; `git show 85fa0fe0:docs/superpowers/plans/2026-10-31-premise-map.md`) question 2, as the main session quotes them in the PM-4 reviewer's brief, confirming that they approved that plan's six recommended answers, question 2 among them; no card holds them yet | checked 2026-10-31, from that brief's quote |
| P42: The game's DLL kept its MD5 through both runs | run | `hydra-data\ch-mcp-runs\2026-10-31\ls-d0-run1\replies.txt` lines 2 and 3, `ls-d1-run1\replies.txt` lines 2 and 3 | checked 2026-10-31 |
| P43: A bot is fed the chart's notes as input, and `is_bot` is read again on every call | data | `docs/clone-hero/07-internals.md` IN-69 | checked 2026-10-31, label Proven in the game's code |
| P44: Window Map's folder holds the manifest `ch_trace_start` reads | chart | `C:\Clone Hero\songs\Hydra Probe\Window Map\manifest.json` | checked 2026-10-31 |
| P45: The Star Power gain method returns early only for player kinds Bot (2) and RemotePlayer (1), so a Player-kind (0) drums player gains meter | data | `docs/clone-hero/07-internals.md` IN-31 | checked 2026-10-31, label Proven in the game's code; the block says no other method that adds meter was searched |
| P46: All 142 notes of Window Map's ExpertDrums are lane 0 (the kick); it has no red note | chart | `C:\Clone Hero\songs\Hydra Probe\Window Map\notes.chart` (142 `N 0` lines, none with another lane) | checked 2026-10-31, counted by script |

## Prior art

Frida's JavaScript API documents `Process.attachModuleObserver`: its `onAdded` is called for a newly loaded module "synchronously right after that module has been loaded, but before the application has had a chance to use it" (frida.re, JavaScript API). That is the hook this plan uses. The installed `frida-il2cpp-bridge` 0.14.0 uses the same call to wait for `GameAssembly.dll` (`dist/index.js`, lines 1198 to 1208), then waits for `il2cpp_init` to return and resumes its own work with `setImmediate`, so `Il2Cpp.perform` does not hold the game back and is not used for the patch. Windows' `CREATE_SUSPENDED` creation flag starts a process with its main thread suspended until `ResumeThread`. A process created that way has not yet run its loader, so its module list is not yet filled in (Raymond Chen, The Old New Thing, 2025-05-26); that is why nothing but a watch can wait for `GameAssembly.dll`. I did not find a source that says what `frida.attach` does on such a process, and `frida.spawn` is a separate path that this plan does not assume behaves the same; both are LS-D0's questions. Rewired's `GetSystemPlayer` returns the System Player as an ordinary `Player`, so `AddController` on it is the same call as on Player1; whether the drums input reads a custom controller there is LS-D1's question.
