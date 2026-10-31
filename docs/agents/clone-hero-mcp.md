# Driving Clone Hero with the `ch` MCP server

The `ch` server lets an agent drive Clone Hero as text. It can start the game, read its state through Frida (a tool that loads a small script into a running program), trace each note the player hits, press drum lanes and menu keys, start a song from the menus, read Star Power while a song plays, read the end-of-song stats, and stop the game with the user's files put back. Claude Code starts it from the `ch` entry in `.mcp.json`. It was built from two plans: `docs/superpowers/plans/2026-10-28-ch-mcp-server.md`, and `docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md` for Star Power and the menus.

## When to use it

Use it when a question needs the real game: how Clone Hero scores a chart, whether a note fires a hook, what the game's clock says while a song plays. Use Hydra's own tools for anything Hydra can answer alone. A live run needs nobody at the keyboard. The run holds the window focus, so nobody can use that machine for anything else while it runs; the user says in chat when a run may start, and the server's backup and restore undo the files.

## The tools

The list of tools lives in one place: `build_server()` in `tools/ch_mcp/server.py`. Each tool there has a one-paragraph description, and Claude Code shows the same text. Read the list there rather than from a copy. `tools/ch_mcp/README.md` says how the server is registered and how its Frida agents are built.

## Safety rules

The server backs up the user's game files before it touches the game, and puts them back when it stops. `ch_start` backs up before it launches. The first tool that touches a game the server did not start takes the backup then. `ch_stop` restores the files and prints one line per file. If a restore fails, the reply names the file and the backup folder; nothing else runs in that call.

Play stays offline. The server launches the game only with the command line `bot_run.launch_command` builds. `ch_open_song` drives the main menu, the song list, the search and the player screen by key presses, and `ch_start_song` calls the game's own start method; both read whether the Online menu is on screen before every step and refuse to go on while it is. No tool opens the Online menu, Settings, Practice or the control mapper.

The Frida agents read, trace, call game methods and, where a feature needs it, set game fields. The user allowed method calls and field writes on 2026-10-30. Every call goes through a name listed in `tools/ch_mcp/agent/menu.ts`'s one list or in `tools/ch_mcp/start_method.json`, and a test in `tools/ch_mcp/tests/test_frida_attach.py` checks that every call in every agent is one of those. The field sets a feature needs are listed in `tools/ch_mcp/start_method.json` too, so a reviewer sees them in one place. The same test file still checks that no agent reads game memory by raw offset. A trace hook patches the start of the traced method in memory while it is attached; the user approved that on 2026-10-28.

Three facts about the menu driver matter for safety, and its code holds the details. The Online check refuses on more than an Online menu that reads as open: an error in place of the flag, or a read with no menu block, stops the run too. `online_refusal` in `tools/ch_mcp/menu_tools.py` says which reads count as closed. The step file may never press Space, because the manual's Keyboard and Gamepads page says Space opens the control mapper from the main menu; `load_steps` in the same file refuses such a step, and `FORBIDDEN_KEYS` beside it is the list. The menu walk presses Green and Blue as drum-lane keys, written as `lane:` items in the step file, because they are not in `input_driver.MENU_KEYS`. On this install those are A and K; `DEFAULT_BINDINGS` in `tools/ch_probe/input_driver.py` is where that comes from.

Every wait has a cap and says so in its reply when the cap ends it. No tool retries on its own.

Nothing runs over the user's song library. Each tool takes one song folder or one chart folder.

## The shape of a live run

Each live run names its pass and fail lines before it starts, has a cap on how many times it runs, and ends with `ch_stop`'s restore lines. A typical run goes like this:

1. `ch_start` with a song folder for a bot run, or with none to open the game at its menu.
2. `ch_attach`, then `ch_state`, `ch_snapshot` or `ch_menu` to read the game.
3. For Star Power, `ch_sp_watch_start` before the song starts. The watcher waits for a moving engine clock and logs every change of the Star Power fields to a CSV. `ch_sp` prints its latest read, `ch_sp_watch_status` lists the activations and the rises of the meter, `ch_sp_between` answers a question about one stretch of the song, and `ch_sp_watch_stop` ends it. `ch_sp_compare` checks the CSV against a `hydra_replay` score JSON.
4. For key presses, `ch_play_chart` before the song starts, with `song_folder` so the run cap follows the song's length. `ch_press` presses one chord. A song launched with a bot is played by the bot, so key presses need a song started from the menu.
5. To start the song, `ch_open_song` with its folder. It presses the steps in `tools/ch_mcp/menu_steps.json` and reads the menus after every press. That file is written from the manual and marked unverified until a live run corrects it, so the first runs go one step at a time: `ch_key` for each step's menu keys, `ch_press` for its `lane:` items (`ch_key` takes only the names in `input_driver.MENU_KEYS` and `text:`), and `ch_menu` after each.
6. For a trace, `ch_trace_start` once the song is loaded, `ch_notes` to read the calls in pieces, and `ch_trace_stop`. The default hook fires once per note the player hits; a missed note and an overhit fire nothing, so a run with misses has fewer calls than notes.
7. `ch_stop`, and check that every file was restored.

Steps 3 and 4 come before step 5 because the watcher and the auto-player each search for the engine, and that search waits for a moving clock up to its own cap (`engine_finder.ENGINE_SEARCH_CAP_S`). Started after the song, they miss its first notes.

The method watch is for finding which game method starts a song. `ch_method_watch_start` hooks the methods `tools/ch_mcp/start_candidates.json` names, and `ch_method_watch_stop` lists every call in order and names the call just before each scene load. Live step LS-M of `docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md` runs it around `ch_open_song`. The main session then writes the method it names into `tools/ch_mcp/start_method.json`, and `ch_start_song` (task T8 of the same plan) calls it; until LS-M has run, neither that file nor that tool exists. The watch looks up the song clock once, when it starts (`songClock` in `tools/ch_mcp/agent/menuhooks.ts`). A watch started at the menu therefore has no song time, so its calls are ordered by `t_ms`, the wall time since the watch started.

Two things the live steps of 2026-10-29 found. In the one bot run tried, the results screen took no keys, so the user could not leave it; end a bot run with `ch_stop`. And after a song the finished `GameManager` stays in memory; the state agent skips it as destroyed, so back at the main menu `ch_state` says "menu (no live GameManager)" and its `destroyed` line counts the skipped one.

The agents now skip any `GameManager` the game has destroyed. Every Unity object holds a pointer to its native half in `m_CachedPtr`, a field of `UnityEngine.Object`, and Unity clears that pointer when it destroys the object. The object itself can stay in memory until the garbage collector runs, which is how a destroyed one can still be found. The lookup in `tools/ch_mcp/agent/gamemanager.ts` reads that field by name, and the state, clock and trace agents all go through it. Each `ch_state` reply carries `destroyed`, the number of destroyed `GameManager`s the read skipped. When none is alive, the reply says `found: false` and the state word is "menu (no live GameManager)".

One live run checked this (2026-10-29). After a song the user went back to the main menu, and `ch_attach` and `ch_state` both said `state: menu (no live GameManager)` with `found: false` and `destroyed: 1`. So the game destroyed that `GameManager` on the way back, and the skip is what tells the menu from the results screen. If a later read at the menu still says "song over" with `destroyed: 0`, the game kept a `GameManager` alive, and the skip would not tell them apart.

If the game is updated, every Frida tool refuses until a dump of the new build exists; the refusal names the scripts to run.
