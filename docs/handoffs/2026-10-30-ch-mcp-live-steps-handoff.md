# The `ch` server's live steps: handoff (2026-10-30, session 4c48c8e5)

The build is done and merged. Nothing has run in the game yet. The next session runs the live steps, starting with LS0 and LS1.

## Where things stand

The plan `docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md` was built on 2026-10-30. Tasks T1 to T7 each had one executor and one fresh reviewer, and every reviewer signed off clean. All of it is on `main`, last at 599e852a, and nothing is pushed. The user's rulings behind the build are D272 in `docs/audit/2026-10-03-fix-decisions.md`.

The server now has 29 tools, up from 18. The new ones read Star Power (`ch_sp_watch_start`, `ch_sp`, `ch_sp_watch_status`, `ch_sp_watch_stop`, `ch_sp_compare`, `ch_sp_between`), drive the menus (`ch_key`, `ch_menu`, `ch_open_song`), and watch the game's candidate start methods (`ch_method_watch_start`, `ch_method_watch_stop`). `docs/agents/clone-hero-mcp.md` explains how a live run goes. The four Frida agents (`state.js`, `trace.js`, `clock.js`, `menuhooks.js`) are compiled in the main checkout's `tools/ch_mcp/agent/`.

T8 (`ch_start_song`) is not built. It waits for the method study LS-M, as the plan says.

## Why this is a new session

The old session's `ch` server was started before the build, so it still had the 18 old tools. A connected server can't be reconnected by an agent. Stopping its process was refused by the permission check ("Interfere With Workloads"). A new session starts its own server from the merged code. First check that the new tools are there: a ToolSearch for `ch_sp` should find them. If it doesn't, ask the user to reconnect `ch` from `/mcp`.

## The user's go

In session 4c48c8e5 the user said "Yes, run them" to running LS0 and LS1 in the game. If this session opens with the user pointing at this handoff, take that as the go for LS0 and LS1 only. Ask in chat before LS-M, LS2 and LS3 (ruling 6: the user says in chat when a run may start; nobody needs to be at the keyboard).

## What to run, in order

Each step's pass line, fail line and run cap are in the plan's "The live steps" section. Read it before each run.

1. **LS0, one run.** Launch with `--song` and no `--player`, by hand after a backup, to see whether the game opens the song for the keyboard profile with no bot.
2. **LS1, at most 3 runs or 15 minutes.** Open the game at its menu with `ch_start()`, attach, and walk `tools/ch_mcp/menu_steps.json` one step at a time. Send menu keys with `ch_key` and the `lane:` items (Green and Blue) with `ch_press`. Read `ch_menu` after every step. Stop the run at once if `onlineMenuActive` reads true on a screen where the Online menu is closed. Then correct `menu_steps.json` in a one-file commit and set `"verified": true`.
3. **LS-M**, inside or right after LS1, then **LS2** and **LS3** (the Senri "Ladies Talk" fill check), each after the user's go.

LS3 needs the Senri score file. The old one was in a scratch folder that is gone. Remake it in your own scratch folder:

`build-cpp\Release\hydra_replay.exe score --chart "testdata/input/common/IB24/T6/Senri Kawaguchi - Ladies Talk [Kiyo Sen] (Drumeo Performance)/notes.mid" --out <your scratch>\senri_score.json`

It should hold 2,349 chords in "Expert Pro Drums, 2x Bass", as the plan says (checked on 2026-10-30).

## What the live runs should look at

The reviewers left these as things only the game can settle:

- **The menu key sequence is a guess.** It comes from the manual. Neither manual page names the Ready press on the player screen, and "use Green to confirm" is a settings sentence carried over as a guess.
- **Key hold time.** Menu keys are held 3 ms (`KEY_HOLD_S`), the lane-press hold. If the menus miss presses, `InputDriver.press_key` already takes a longer `hold_s`.
- **The Online check runs on Frida's thread.** `get_activeInHierarchy()` is not called on Unity's main thread. If it throws, `ch_menu` shows an "error: ..." entry and `ch_open_song` refuses to press.
- **Read time.** Each state read now walks the heap four times, not once. L1 timed one walk under 0.1 s, against a 5 s read cap. Time a read in LS1.
- **`gameMode`** is read through the enum's `value__`. Only a live read shows it works.
- **The method watch reads the song clock once, at its start.** A watch started at the menu has no song time, so its calls are ordered by `t_ms`.

## Open questions and small follow-ups

- **`space` (open, the user's call).** The manual says Space opens the control mapper from the main menu. `space` is still in `MENU_KEYS`, so `ch_key("space")` could open it. The step loader refuses any step that presses it, so `ch_open_song` can't. The user hasn't said yet whether to remove it from `MENU_KEYS`.
- **`ch_sp_between` can't name the nearest chord.** The plan's tools table promises it "when a score JSON was given to the watcher", but `ch_sp_watch_start` takes no score file.
- **A stop during the engine search waits.** `ch_sp_watch_stop` only takes effect once `watch` runs, so a stop during the search (up to 120 s) reports the watcher still running.
- **`ch_status` doesn't show the two new watches.**
- **`press_keys` holds the server lock while it types**, with 0.25 s between keys. A long song name blocks other tools, `ch_stop` included, for several seconds.
- **When T8 merges**, two sentences need editing: the agent page's "until LS-M has run, neither that file nor that tool exists", and the README's line that says `start_method.json` "does not exist yet". T8's file list holds neither doc.
- **Small leftovers:** `tools/ch_probe/interfaces.py` still calls `send_key` "the only OS call". `menuhooks.ts`'s `findClass` does the same job as `trace.ts`'s `findEngine` and could move into `hooks_common.ts`. The overlay-spike write-up `docs/research/2026-10-28-overlay-spike-results.md` is still the empty template. The `OFF_SP_*` comments in `tools/ch_probe/constants.py` still say "not yet confirmed live", although run 1 confirmed them.
- **The `rea` (Ghidra) tools now connect from the main session.** A read of the candidate start methods could narrow LS-M's list before it runs. The plan lists that read as optional and out of scope unless the user says yes.

## Housekeeping

The task worktrees `chsp-t1` to `chsp-t7` and `ch-sp-plan` under `.claude\worktrees\` are all merged. The cleanup sweep hook removes merged idle worktrees.

## Related

- The build handoff this one follows: `docs/handoffs/2026-10-30-ch-mcp-star-power-build-handoff.md`.
- The memory note `ch-mcp-server-2026-10`.
- The Clone Hero reference session, which feeds its list of live checks to this server: `docs/handoffs/2026-10-30-clone-hero-reference-session-prompt.md` (removed; `git show ec91f8d1:docs/handoffs/2026-10-30-clone-hero-reference-session-prompt.md`). The list itself is `docs/clone-hero/live-checks.md`.
