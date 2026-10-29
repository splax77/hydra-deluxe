# Driving Clone Hero with the `ch` MCP server

The `ch` server lets an agent drive Clone Hero as text. It can start the game, read its state through Frida (a tool that loads a small script into a running program), trace each note the player hits, press drum lanes, read the end-of-song stats, and stop the game with the user's files put back. Claude Code starts it from the `ch` entry in `.mcp.json`. The plan it was built from is `docs/superpowers/plans/2026-10-28-ch-mcp-server.md`.

## When to use it

Use it when a question needs the real game: how Clone Hero scores a chart, whether a note fires a hook, what the game's clock says while a song plays. Use Hydra's own tools for anything Hydra can answer alone. The game runs on the user's machine and changes the user's files, so every live run needs the user present and watching.

## The tools

The list of tools lives in one place: `build_server()` in `tools/ch_mcp/server.py`. Each tool there has a one-paragraph description, and Claude Code shows the same text. Read the list there rather than from a copy. `tools/ch_mcp/README.md` says how the server is registered and how its Frida agents are built.

## Safety rules

The server backs up the user's game files before it touches the game, and puts them back when it stops. `ch_start` backs up before it launches. The first tool that touches a game the server did not start takes the backup then. `ch_stop` restores the files and prints one line per file. If a restore fails, the reply names the file and the backup folder; nothing else runs in that call.

Play stays offline. The server launches the game only with the command line `bot_run.launch_command` builds, and no tool opens any menu. The user picks songs in the menu by hand.

The Frida agents only read and trace. They never write game memory, set a field or call a game method; a test in `tools/ch_mcp/tests/test_frida_attach.py` checks every agent for that. A trace hook does patch the start of the traced method in memory while it is attached; the user approved that on 2026-10-28.

Every wait has a cap and says so in its reply when the cap ends it. No tool retries on its own.

Nothing runs over the user's song library. Each tool takes one song folder or one chart folder.

## The shape of a live run

Each live run names its pass and fail lines before it starts, has a cap on how many times it runs, and ends with `ch_stop`'s restore lines. A typical run goes like this:

1. `ch_start` with a song folder for a bot run, or with none to open the menu so the user can start a song by hand.
2. `ch_attach`, then `ch_state` or `ch_snapshot` to read the game.
3. For a trace, `ch_trace_start` once the song is loaded, `ch_notes` to read the calls in pieces, and `ch_trace_stop`. The default hook fires once per note the player hits; a missed note and an overhit fire nothing, so a run with misses has fewer calls than notes.
4. For key presses, `ch_press` or `ch_play_chart` on a song the user started from the menu. A song launched with a bot is played by the bot.
5. `ch_stop`, and check that every file was restored.

Two things the live steps of 2026-10-29 found. In the one bot run tried, the results screen took no keys, so the user could not leave it; end a bot run with `ch_stop`. And `ch_state` says "menu" only before the first song: after a song, the finished `GameManager` stayed in memory, and `ch_state` kept saying "song over" even though the user was back at the main menu.

If the game is updated, every Frida tool refuses until a dump of the new build exists; the refusal names the scripts to run.
