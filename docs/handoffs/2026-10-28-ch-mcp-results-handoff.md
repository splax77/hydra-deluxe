# Handoff: Clone Hero MCP feasibility, after the four tests

Written 2026-10-28 by the main session of session 6a549de0-baf9-484b-bc31-5c46be6059e6. It replaces `2026-10-28-ch-mcp-feasibility-tests-handoff.md`, which was written before anything was built.

## Where things stand

The feasibility work is finished. All seven tool tasks (A to G) are reviewed and on main, and all four tests ran with the user at the game. The results are reviewed and merged too. Main is at aa714392, and its three repo-wide tests pass.

Read the "Results of the four tests" section of `docs/research/2026-10-28-clone-hero-mcp-feasibility.md` first. It is the record. The plan, `docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md`, now has an outcome line and "As built" notes where the build changed from it.

In one line each:
- **Test 1, the bot launch:** passed. A command-line bot run writes `scorestats.json`, and its note and Star Power phrase counts match Hydra. The bot never uses Star Power.
- **Test 2, the Frida attach:** passed. Frida reads `GameManager.songTime` and the other fields by name.
- **Test 3, the hit method:** passed. Engine methods `0x20DEEC0` and `0x20DE770` and base method `0x100E6E0` each run once per note, for both the bot and key presses.
- **Test 4, a second build:** v1.0 against v1.1 reshuffles most scrambled names. The MCP must find classes again by shape after an update.

The tools live in `tools/ch_mcp/`, and its README says how to run each one. The game dumps are in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\4045\` (v1.0) and `...\6142\` (v1.1). The raw evidence from the runs is in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-28\`. That covers the stats copies, clock samples, trace tables, game-code decompiles and the disassembly check script. The study still points at the old scratch paths, which no longer exist; this folder holds the same files under the same names (`l1` to `l4`, `ch-dynamics`, `ch-dynamics-2`).

## The user's rulings, in their words

- **The EULA:** it forbids reverse-engineering. The user's ruling: "Proceed, my call." That came after the user learned the approval question had wrongly said "allows". A stray edit had flipped the word in the old handoff and in an uncommitted copy of the plan. That edit's diff is kept as `stray-eula-edit-ch-mcp-tests-plan.diff` in the evidence folder. Check any EULA claim against https://clonehero.net/eula, never against a summary.
- **Plain Drums and dynamics:** the game clears every accent and ghost on plain Drums (`0x20EFEE0`, reached only when the instrument isn't ProDrums). The user said "there is no way this is intentional, it must be a bug in clone hero" and then "Keep Hydra as is". Hydra's normal-drums mode keeps scoring dynamics. The memory note `ch-plain-drums-drops-dynamics` has the details.
- **Task F:** "Cap them too". The old probe scripts now stop at the existing 120 s cap.
- **Task G:** yes, add a reader for the Mono v1.0 build.

## Follow-ups, now merged

The user started two follow-up sessions from this one. Both have merged to local main, which is at 18825c8a and not pushed (origin/main is at 2bf91835):
- [Fix three small ch_mcp tool faults](#local_0df9bab5-8ab4-4beb-a29d-b2fdbc113385) fixes three things. A cap end is reported as an error with exit 1. The restore hint says `py -I`. And a trace's cap runs during a pause, which cut the key-press run to 139 of 142 notes.
- [Single-owner scan: skip gitignored files](#local_61f898a7-1155-4a98-8c64-13edace35543). The scan walks gitignored folders, and once failed main on Frida's vendored Python under `tools/ch_mcp/agent/node_modules`.

The trace cap's 120 s wall-clock backstop is a new number the user has not ruled on yet.

## What comes next

None of these has started, and each needs the user's yes:
- **Build the MCP server itself,** in the shape of `tools/hydra_ui_mcp.py`. The study's "Recommendation" section has the four tool groups.
- **Read a note's result and offset** from the per-note hook's arguments. `0x20DE770` takes the note object, and `0x20DEEC0` takes the note and the base points.
- **Find the switch for the empty `HitDetection-*.log` files.**
- **Move input inside the process** instead of `SendInput`.

## Things that bite

- **Running Frida:** use `py`, not `py -I`, because frida is installed in the user site-packages.
- **Attaching after a launch:** after a command-line launch, the session log writes only its `Loading Song` line at first. Waiting on it for `OnCountdownStart` never works. Attach about 5 s after launch instead, because the first probe note is at 3 s.
- **Starting the tracer:** it needs a live `GameManager` when it attaches, and the menu has none. For a human-started song, start the song, pause it, attach, then unpause. Since main 6bdbc494 the trace's cap counts song time, so the pause no longer uses it up; a wall-clock backstop of the song cap plus 120 s still ends a song that never starts.
- **`node_modules` in the main checkout:** since main 18825c8a the repo scans read git's file list, so a `npm install` in `tools/ch_mcp/agent` no longer fails them. Run `npm install` and `npm run build:clock` or `build:trace` there when needed.
- **Backups:** every live run backs up the five user files and restores them. A human run changes `scoredata.bin`, so restoring discards that run's saved score.
- **Hung subagent shells:** subagents sometimes leave a hung `bash -c "... python - <<'EOF' ..."` behind. It holds a worktree folder open, so `git worktree remove` fails with "Permission denied". Find it with `Get-CimInstance Win32_Process` by the folder name in its command line, and stop it and its child `python.exe`.

## Cleaned up

All ten `ch-mcp-*` worktrees and branches from this work are removed; every one was merged first. This session's scratch folder is empty. The previous session's (856d0fac) temporary dump is deleted, because `hydra-data\ch-dumps\6142\` replaces it.
