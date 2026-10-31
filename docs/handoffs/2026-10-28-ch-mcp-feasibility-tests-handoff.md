# Handoff: Clone Hero MCP feasibility tests

**Superseded.** The work this handoff describes is done. Read `2026-10-28-ch-mcp-results-handoff.md` instead.

Written 2026-10-28 by the main session of session 856d0fac-b682-4ca5-b28f-65f459b0833d.

## Where things stand

The user asked for a feasibility study on a full MCP server for Clone Hero. That is a helper that lets an agent start the game, pick a song, play it with exact inputs and read back what the game saw. The study is done. It says the MCP is feasible, with per-note hit results as the one real unknown. The user then asked for a plan for the study's four tests. That plan is written and reviewed. It waits for the user's answers. Nothing is built, nothing has run at the game, and nothing is on main.

Both documents are on branch `claude/ch-mcp-tests-plan`, in the worktree `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\ch-mcp-tests-plan`. The study is `docs/research/2026-10-28-clone-hero-mcp-feasibility.md`. The plan is `docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md`. The branch holds four commits on top of main 2bf91835. The tip, fe4b6432, is the reviewer's own fix commit, and the reviewer signed it CLEAN. Do not add commits to that branch before merging it, or main will refuse the unsigned tip. Merging it to main is still to do and needs the user's yes.

Read the plan before doing anything. It is the source of truth; this handoff only says where things stand.

## What the user must answer first

The plan lists ten open questions, each with a recommended answer. Three are really the user's.

The first is the EULA. Clone Hero's EULA (last updated 4 February 2021) forbids reverse-engineering or modifying the game. (This line said "allows" until 2026-10-28 22:00, which was wrong; the EULA's License Restrictions forbid you to "disassemble, decrypt, reverse compile or reverse engineer any part of the Application".) Tests 2 and 3 attach Frida to the running game and trace its code, reading only. The plan gates both tests on the user's explicit yes. Tests 1 and 4 go ahead either way.

The second is the npm install. The Frida script needs `frida-il2cpp-bridge`, `frida-compile` and `@types/frida-gum`. Installing them is a download, so it waits for the user's yes.

The third is a second game build. Only v1.1.0.6142 is on this machine. Without a second build, test 4 ships its comparison script and the "do names change on update" question stays open.

The other seven questions cover the new time caps, where the code lives (`tools/ch_mcp/`), where the dump lives, the test profile, offline play, the second chart for test 1, and who plays in test 3. The new caps are numbers, so they need the user's yes too. The user can approve all seven with "go with the recommendations".

At the end of the last session the user had a choice between "approve everything including Frida" and "tests 1 and 4 only". They answered neither and asked for this handoff.

## How the work runs once approved

Executors write and unit-test the code first, offline. Each task runs in its own worktree under `.claude\worktrees\`. Task A builds a durable dump. Task B builds the bot launcher with backup and restore. Task C builds the Frida attach runner and clock script. Task D builds the hit-method tracer. Task E builds the dump comparison. Task F caps `ch_probe`'s endless engine wait. A, B and F start at once. C waits for the EULA and install answers. D starts after C commits, and E after A commits. Each task gets one fresh reviewer, then merges.

Then the main session runs the live steps with the user at the game, in order: L1 the bot launch, L2 the Frida attach, L3 the hit-method search. L4, the second-build comparison, runs only if a build turns up. Each step's pass and dead lines are in the plan, and each has a run cap.

## Facts the next session should know

The game is at `C:\Clone Hero\1.1 june\Clone Hero\`: v1.1.0.6142-final, Unity 2022.3.62f2, IL2CPP, metadata version 31. It is scrambled by Beebyte Obfuscator 3.13.0, but only in four libraries: CloneHero.dll, StrikeCore.dll, CHNet.Core.dll and CHNet.Server.dll. `GameManager`, `ScoreManager`, `DrumsPlayer`, `SongSelect` and `PlayerStatsBox` keep their real names. `GameManager` keeps readable fields such as `songTime` at 0x70 and `isSongOver` at 0x91.

The drum engine that `tools/ch_probe` finds is the scrambled class at TypeDefIndex 8255. All three engine addresses in `ch_probe/constants.py` belong to it. `DrumsPlayer` holds a field of that type, so a readable class leads to the engine.

`ch_probe`'s hit-check address `0x100DAB0` is not the drums version of that method. It belongs to a different instantiation of the base class, which the reviewer believes is guitar. The drum version is at `0x100DAE0`. The main session checked this in `script.json`. Any `ch_probe` debugger result taken at `0x100DAB0` came from the wrong code. The new tests do not depend on it.

`Documents\Clone Hero\scorestats.json` is written at the end of every song, bot runs included, with about 46 fields per player. It is overwritten each time.

The Il2CppDumper output this session made lives in the session scratch folder, `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\856d0fac-b682-4ca5-b28f-65f459b0833d\scratchpad\obfuscation\`. It is temporary. Task A regenerates it into `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\`. Il2CppDumper is at `C:\Users\Patrick\Downloads\Il2CppDumper-net7-v6.7.46\`. It needs `DOTNET_ROLL_FORWARD=Major`, and it crashes on its final "press any key" after printing `Done!` three times, which is harmless.

`frida-il2cpp-bridge` 0.14.0 and `frida-compile` 19.0.5 are already installed globally under `AppData\Roaming\npm\node_modules`, from 2026-09-14. Whether a local install is still needed is untested. Frida 17.18.0 (Python), Node 26.8.1 and Python `mcp` 2.3.0 are installed.

Reddit and the Clone Hero Discord could not be searched. "Nobody has modded Clone Hero v1" means nothing was found, not that nothing exists.

## Not in scope

The plan leaves out building the MCP server itself, finding a switch for the empty `HitDetection-*.log` files, input from inside the process, and precision mode. Each needs the user's yes to start.
