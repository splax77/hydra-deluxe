# Claude drives Hydra's GUI through an MCP server (plan, 2026-10-14)

**Status:** approved by the user on 2026-10-14, with every open question answered as recommended. The build is under way. Planner a50ef6abfe09f7389 (task MCP-PLAN, session 719dc0ea) read every file named below on main at 07594fee.

## What this is, in one paragraph

Today Claude checks the GUI by writing a command file and running `hydra_uitest --script file.txt`. The file runs top to bottom. Claude sees nothing until it ends, so it cannot look at the screen and then decide what to click next. This plan adds a live channel: the same interpreter, but one command in, one answer out, with the app kept alive between commands. A small helper program speaks MCP to Claude Code on one side and that channel on the other. MCP (Model Context Protocol) is the convention where Claude Code starts a helper, asks it which tools it has, and sends each tool call to it as a text message. Claude then clicks, types and reads the window as text, by widget label, with no screenshots.

## The settled decisions (from the user, not reopened here)

1. Two modes. Headless runs the UI with no window on the software GPU, the `testdata/input` charts and an empty database, exactly as `hydra_uitest` does now. Real-window mode runs the real `Hydra.exe` window on a copy of the user's library (hydra.db with its -wal and -shm) and a copy of the user's ini files, so the data and the layout are real and the real library can never be written.
2. On the copy, Claude has full control: click, type, start analyses. The real hydra.db must be impossible to open in this mode, not just avoided.
3. Dev builds only. The `ship` preset gains nothing.
4. The server is registered per project in a checked-in `.mcp.json`, never in `~/.claude`.

## What exists today, checked in the code

The test engine drives the real UI by label, and a command-file interpreter already turns one line into one engine action. `run_script` in `tests/ui/uitest_script.cpp` reads a file line by line, splits each line into a verb and the rest (`split_verb`, line 20), and runs one of fourteen verbs: `window`, `click`, `check`, `uncheck`, `type`, `wait`, `timeout`, `wait-idle`, `wait-text`, `expect-text`, `expect-not-text`, `text`, `dump`, `state` and `screenshot`. Its waits are capped by one number, `timeout`, which starts at 30 seconds (line 42) and the `timeout` verb changes. Every answer it produces goes to stdout with `printf`: the script echoes each line (line 51), `text` prints `visible_text` (line 101), `dump` calls `dump_widgets` and `state` calls `dump_state`, both of which print directly (`uitest_harness.cpp` lines 561 and 574). That is the one thing the live channel cannot use as it is: an answer printed to stdout cannot be sent back as the reply to one command. So the printing functions become string-returning functions first, and the script mode prints what they return.

The script runs as a test. `register_script_test` makes a test named `script` whose body is `run_script` (line 126). The engine runs a test's body on a coroutine, a thread that hands control back to the GUI thread every time it calls `ctx->Yield()`. A frame draws only while the body is yielding. This matters for the live channel: if the body blocked waiting for the next command, no frame would draw, background jobs' results would never land, and the real window would freeze and go "Not responding". So the live loop must never block; it polls a queue and yields between polls, the way `wait_until` already does (`uitest_harness.cpp` line 382: yield, then sleep 2 ms).

When an engine action fails (a click finds no such label), the engine marks the test failed. `IsError()` is `TestOutput->Status == ImGuiTestStatus_Error || Abort` (`imgui_te_context.h` line 274), and after that every engine action returns at once without doing anything. A live session must survive a bad click, so after each failed command the loop sets the status back to `ImGuiTestStatus_Running`. That is the engine's own trick: it does exactly this around a test's teardown function (`imgui_te_engine.cpp` lines 1924 to 1927). The failure's words are in the test's log buffer (`Test->Output.Log.Buffer`, which `print_results` prints today); the live loop notes the buffer's length before each command and returns only the part added since.

The engine has a watchdog. In fast mode (headless) it fails a test that runs longer than `ConfigWatchdogKillTest`, which the harness sets to 600 seconds (`uitest_harness.cpp` line 116), and the header says `FLT_MAX` turns it off (`imgui_te_engine.h` line 300). A live session runs for a whole Claude session, so the live test turns the watchdog off for itself. In normal speed (the attached real window) the watchdog does nothing anyway (`imgui_te_engine.cpp` line 797).

The scratch files have one owner. `init_scratch` (`uitest_harness.cpp` line 87) makes the folder with `testtemp::temp_dir("uitest")`, which is `<Windows temp>\hydra_test_uitest_<pid>` (`tests/temp_util.h` line 43, the one temp-folder lookup under `tests/`, pinned by a single-owner row), names `hydra.db`, `hydra_settings.ini` and `hydra_rules.ini` inside it, and hands all three to `app::set_path_overrides`. The app asks `app::db_path()` and `app::ini_path()` (`src/app/config.cpp` lines 41 and 46), which return the override when set and the file beside the exe otherwise. Those two functions are the only way the app learns where its database and settings are: `AppState` opens the store at `app::db_path()` (`src/ui/app_state.cpp` line 43), the report job reads it from the same call (`src/ui/library_jobs.cpp` line 573), and `Settings::load` and `save` go through `ini_path()` (`config.cpp` lines 61 and 220). The command-line tools take their own `--db` and are not part of this.

Starting from a copy of a database already exists. `hydra_uitest --db <file>` sets `seed_db`, and `reset_app` copies the file and its `-wal` and `-shm` into the scratch folder before building the app (`uitest_harness.cpp` lines 334 to 346). The comment says it: "the app never opens the original". `main()` refuses a `--db` that is not a file (`uitest_main.cpp` line 279). The ini has no seed yet: `reset_app` always writes `scratch_settings()` (`tests/scratch_settings.h`: the corpus folder, no auto-open, depth 2).

Attached mode is `Hydra.exe --uitest <what>`. `main.cpp` reads `--uitest` and `--uitest-log` (lines 176 to 183), sends stdout to a log file because a GUI exe has no console (line 306), builds the harness on the real device and swapchain with `init_attached` (line 314), and keeps the window open after the queue drains (line 442). The whole path compiles only under `HYDRA_UITEST_ATTACHED` (lines 36, 184, 298, 402, 437, 452), which `CMakeLists.txt` line 464 turns on for dev builds and the `ship` preset turns off (`CMakePresets.json` line 25). `hydra_uitest` itself is never installed: `install(TARGETS Hydra hydra_batch hydra_fillcompare ...)` at `CMakeLists.txt` line 631 is the list. So anything added to the harness library or under that define stays out of the shipped exe by the rules that exist already.

ImGui's own layout file is a third file. `setup_imgui` reads `ImGuiSetupOptions::ini_file`: empty means `hydra_ui.ini` beside the exe, `-` means none (`src/ui/app_shell.cpp` lines 414 to 422). `hydra_uitest` passes `-` (`uitest_harness.cpp` line 219); `main.cpp` passes nothing, so the real window reads and writes `build-cpp\Release\hydra_ui.ini`, not the user's. That file holds the window placement and the library split (`app_shell.h` lines 43 to 138). Column widths are no longer in it in any way that wins: they come from the shares rule (`07594fee`, "text columns take fixed shares of the room").

Two more seams. `audio::set_headless(true)` makes the Preview's audio device open nothing while Play and Pause still track state (`src/audio/device.h` line 22). `net::set_fetcher` replaces the dmleaderboards HTTP fetch; `reset_app` installs a canned one (one user, `alice`), and an empty function restores WinHTTP (`src/net/dmbot_client.h` line 78). `reset_app` sets both every time it runs (lines 317 to 326).

The user's installed library sits in `C:\Program Files\Hydra`: `hydra.db` (18 MB on 2026-10-14), `hydra_settings.ini` and `hydra_ui.ini` (74 KB). The `-wal` and `-shm` files exist only while Hydra has the database open. On 2026-10-14 at planning time no `Hydra.exe` was running and the two files were absent.

Python's `mcp` package is not installed. The system Python is 3.14 (`py`). PyPI's `mcp` 2.3.0 (released 2026-10-02) requires Python 3.10 or newer and lists 3.14 among its classifiers.

## Prior art (web, 2026-10-14)

No MCP server that drives the Dear ImGui Test Engine exists that the search could find. The nearest things:

- [SaloQT/imgui-mcp](https://github.com/SaloQT/imgui-mcp) is an MCP server whose Python side (the official `mcp` SDK on stdio) talks to a C++ ImGui process over that process's stdin and stdout, exchanging JSON lines. It renders its own canvas for an agent to design UIs on; it does not drive an existing app's widgets by label. Its split, Python for MCP and a C++ process for ImGui joined by a line protocol, is the one this plan uses.
- [lauripiispanen/game-mcp](https://github.com/lauripiispanen/game-mcp), the "Generic game engine MCP bridge", is a thin server that launches the game if it is not running, keeps a TCP connection to it, forwards each command as newline-delimited JSON and returns the reply. It has one connect timeout (5 s) and one command timeout (10 s), both overridable per call, and a `game_connect` tool that can kill and relaunch the game. This plan copies the shape: launch, keep one connection, cap every wait, and a start tool that restarts.
- The [Dear ImGui Test Engine](https://github.com/ocornut/imgui_test_engine) addresses items by label path and injects input; that is what `hydra_uitest` already builds on. [NetImgui](https://github.com/sammyfreg/netImgui) mirrors an ImGui UI to a remote viewer over TCP; it moves pixels and input, not labels, so it does not help here.
- Claude Code reads project servers from `.mcp.json` at the repo root, commits it, asks the user to approve the server on first use, and expands `${VAR:-default}` in `command`, `args` and `env` ([Claude Code MCP docs summary](https://mcpplaygroundonline.com/blog/claude-code-mcp-setup-best-servers-guide), [config reference](https://www.host-tracker.com/en/learn/ai-assistants/claude-code-mcp-json-config)).

## The design

### The shape

Three parts, each with one job.

The command interpreter in C++ answers "what does this verb do to the UI". It is today's `run_script` body, pulled into its own file so the script mode and the live mode both call it. One call takes one line and returns one result: ok or failed, plus any text the verb produced.

The live channel in C++ answers "how does a line get in and a result get out while the app keeps drawing". It is a test, like `script`, that opens a named pipe, reads lines from it on a reader thread, runs each through the interpreter on the engine's coroutine, and writes one JSON line back per command.

The helper in Python answers "what does Claude see". It launches the right exe, connects to the pipe, turns each MCP tool call into one line and each reply into tool output, caps every wait, reports a dead or silent Hydra, keeps replies small, and in real-window mode takes the snapshot of the user's library.

### The live channel

A new test named `live` lives in `tests/ui/uitest_live.cpp`, registered the way `script` is, and started by a new flag `--live` on both `hydra_uitest` and `Hydra.exe`. Its body does this. It calls `reset_app` like every test, then opens a named pipe `\\.\pipe\hydra-live-<pid>` (the process id, which the helper knows because it launched the process, so nothing has to be passed on the command line). A reader thread sits in `ReadFile` and pushes each complete line onto a queue behind a mutex. The body loops: if the queue has a line, run it through the interpreter and write the reply; otherwise yield one frame and sleep two milliseconds, as `wait_until` does. The loop ends when the client disconnects (the read fails) or the `quit` verb arrives. Then the test returns, which ends `hydra_uitest` and, in the real window, closes Hydra (see T5).

One request is one line, the verb and its rest, exactly as a script line. One reply is one JSON line, so the helper never has to guess where an answer ends: `{"ok":true,"text":"..."}` or `{"ok":false,"error":"...","log":"..."}`. The C++ side only writes JSON, never parses it, so it needs a string escaper and nothing else; `third_party/json` stays a test-only dependency. On connect the channel writes one hello line first: `{"ok":true,"hello":{"mode":"headless","scratch":"<folder>","pid":1234}}`.

A failed command does not end the session. Before each command the loop notes the log buffer's length. After a command that left `ctx->IsError()` true, it takes the log text added since, puts it in `log`, and sets `ctx->TestOutput->Status` back to `ImGuiTestStatus_Running`, as the engine does around teardown. The next command runs normally. A command that threw a C++ exception is caught the same way and reported with its text; the test engine's crash handler and `on_crash` in `uitest_main.cpp` cover a real crash, which prints the exception code and writes `crash.dmp` into the scratch folder.

Every wait keeps the cap it has today. The interpreter's `timeout` (30 s by default, changed by the `timeout` verb) bounds `wait-idle` and `wait-text`, so no command can hold the pipe for longer than that plus the time of one action. The live test sets the engine watchdog to `FLT_MAX` for its own run, because the session is meant to last; the per-command cap replaces it.

The reader thread and the pipe are Win32 (`CreateNamedPipeW`, `ConnectNamedPipe`, `ReadFile`, `WriteFile`). Python opens the client end with plain `open(r"\\.\pipe\hydra-live-1234", "r+b", buffering=0)`, no extra package, retrying until the server has made the pipe, with a cap. T2's first step proves that open works with a 20-line spike before anything else is built; if it does not within that step, the fallback is TCP on localhost, the game-mcp way, with the port written to the hello line of the log file. The plan does not expect to need it.

### The interpreter

`run_script`'s verb table moves to `tests/ui/uitest_commands.{h,cpp}` as `run_command(ctx, session, line) -> CommandResult { bool ok; std::string text; }`, where `session` holds the mutable bits a script keeps between lines (the `timeout`). `run_script` becomes: read a line, echo it, call `run_command`, print its text, stop on the first failure. Nothing a script does today changes.

`dump_widgets`, `dump_state` and the `text` verb become string-returning (`widget_dump`, `state_text`; `visible_text` already is). The script prints them; the live reply carries them. The dump keeps its exact line shape: `window "<name>" id=... pos=(x,y) size=(w,h)` and `  "<label>" id=... rect=(x,y wxh) <flags>`, because those numbers are how Claude checks layout. A column edge is the x of a header cell; a row pitch is the difference of two rows' y; a stretched row shows as a height that is not the pitch. The engine shortens labels to about 30 characters in the dump (ui-testing.md says so); that stays.

Four verbs are added, each a few lines over an engine call that exists:

- `key <name>` presses a key or chord: `Escape`, `Enter`, `Ctrl+W`, `Down`. It maps a short table of names to `ImGuiKey` values and calls `ctx->KeyPress` / `ctx->KeyChord`. The table is the one place those names live.
- `rect <ref>` returns one widget's line in the dump's shape, through `ctx->ItemInfo(ref)`, so Claude can ask for one edge without the whole tree.
- `reset` runs `reset_app` again: a fresh scratch library, the same process. In real-window mode it copies the seed files again, so the library goes back to the snapshot.
- `quit` ends the session.

`screenshot` stays for the case where a number looks wrong and a picture is the only way to see why. The helper passes `--shots <its own scratch>` so the PNG lands where Claude can read it, and the reply names the path.

### The helper (MCP server)

`tools/hydra_ui_mcp.py`, Python 3.14, run as `py -I tools/hydra_ui_mcp.py`. Python because the helper is glue: launching a process, copying five files, polling for exit, trimming text, timing waits. The repo already keeps that kind of tool in `tools/` in Python (`compare_db.py` with `test_compare_db.py` as its test), and a change to the helper then needs no C++ build. The UI driving stays in C++ where the engine is. It uses the official `mcp` package (`FastMCP`, stdio transport) so the protocol handshake, the tool list and the message framing are the reference implementation's and not ours. That is a one-time install on this machine: `py -m pip install mcp` (question 1).

The tools Claude sees, each returning plain text:

| Tool | What it does | Returns |
|---|---|---|
| `hydra_start(mode, library_dir=None, keep=False)` | starts a session; `mode` is `headless` or `window`; stops any session already running | the hello line's facts: mode, scratch folder, pid, and in window mode which files were snapshotted and when |
| `hydra_stop()` | sends `quit`, waits for exit with a cap, kills if needed | exit code |
| `hydra_reset()` | `reset`: fresh library in the same session | ok |
| `hydra_click(ref)` | `click` | ok, or the engine's log lines for the failure |
| `hydra_type(ref, text)` | `type` | ok or failure |
| `hydra_check(ref, on)` | `check` / `uncheck` | ok or failure |
| `hydra_key(keys)` | `key` | ok or failure |
| `hydra_window(ref)` | `window`: the window later refs are relative to | ok |
| `hydra_wait_idle(timeout=None)` | `timeout` (when given) then `wait-idle` | ok, or "jobs still running after N s" |
| `hydra_wait_text(text, timeout=None)` | `wait-text` | ok, or "did not appear within N s" |
| `hydra_text(contains=None)` | `text`; with `contains`, only the lines holding that substring | the screen as text |
| `hydra_state()` | `state` | the state block |
| `hydra_dump(window="Hydra", contains=None)` | `dump`; with `contains`, only matching item lines plus their window line | the widget tree with rects |
| `hydra_rect(ref)` | `rect` | one widget line |
| `hydra_screenshot(name)` | `screenshot` | the PNG's path |
| `hydra_run(script)` | several lines at once, stopping at the first failure | one line of result per line sent |

`hydra_run` exists so a known sequence (scan, continue, search, open) costs one tool call instead of five. The per-verb tools exist so a single step reads clearly in Claude's transcript and so a failure names the step.

Output stays small by three rules. Every reply is cut at a character cap, with a last line saying how many lines were dropped and that `contains` narrows it (question 5 names the cap). `text` and `dump` take `contains`. The library table is clipped, so `text` only ever holds the rows on screen; a 19,000-chart library produces no more lines than the 97-chart corpus.

A dead or silent Hydra is reported, never waited on. Each tool call waits for its reply at most the command's own timeout plus a margin (question 4). If the process has exited, the reply says "Hydra exited with code X" and gives the last lines of its log (`hydra_uitest`'s stdout, or `hydra_uitest.log` beside `Hydra.exe`) and the path of `crash.dmp` if the scratch folder holds one. If it is alive and silent past the cap, the reply says "Hydra did not answer within N s", the helper kills the process, and the session is marked stopped. Every later call answers "no session; call hydra_start" until one is started. The helper also kills its child when Claude Code ends it (an `atexit` hook, with the pipe closing as the second line of defence, since the C++ side exits when the read fails).

A session is one process per Claude Code session, started by the first `hydra_start` (or the first tool that needs one, which starts headless) and kept until `hydra_stop`. The headless process holds the engine's scratch folder; `reset` is the cheap way back to a clean library (about a second). The exes come from `build-cpp\Release`; when one is missing the reply says which build command makes it, and the helper never builds (a build takes minutes and competes with every other build on the machine).

The registration is `.mcp.json` at the repo root:

```json
{
  "mcpServers": {
    "hydra": {
      "command": "py",
      "args": ["-I", "tools/hydra_ui_mcp.py"],
      "env": { "HYDRA_LIBRARY_DIR": "${HYDRA_LIBRARY_DIR:-C:\\Program Files\\Hydra}" }
    }
  }
}
```

Claude Code asks the user to approve a project server the first time it is used. The helper resolves `build-cpp` and `tools/` from its own file's location, so the working directory does not matter.

### Headless mode

`hydra_start("headless")` runs `build-cpp\Release\hydra_uitest.exe --live --shots <helper scratch>`. The harness does what it does for every test: WARP device, offscreen target, `init_scratch`, `reset_app` with `scratch_settings()` and an empty database, canned leaderboards, headless audio. The only difference from `--script` is where the lines come from and where the answers go. The fast run speed stays, so a click lands in a frame or two, and the 2 ms sleep between polls keeps the process quiet while Claude thinks.

### Real-window mode

`hydra_start("window")` does two things before launching anything.

First, the snapshot. The helper takes the library folder (`library_dir`, else `HYDRA_LIBRARY_DIR`, else `C:\Program Files\Hydra`) and copies `hydra.db`, `hydra.db-wal` and `hydra.db-shm` when present, `hydra_settings.ini` and `hydra_ui.ini` into a fresh folder of its own under the Windows temp folder, `hydra_live_<helper pid>`. Only this folder is ever named on the command line. The helper refuses to snapshot while a `Hydra.exe` is running from that folder, because a database being written has a `-wal` that a copy can tear; the reply says to close Hydra first (question 3). The reply to `hydra_start` lists each file copied with its size and modification time, so the user and Claude both know how old the copy is. `hydra_start` again takes a new snapshot; `hydra_reset` goes back to the one taken at start.

Second, the launch: `build-cpp\Release\Hydra.exe --live --db <snapshot>\hydra.db --ini <snapshot>\hydra_settings.ini --ui-ini <snapshot>\hydra_ui.ini --shots <helper scratch>`. Three things in C++ make the real library unreachable from here:

1. The app only ever asks `app::db_path()` and `app::ini_path()`, and in live mode those return the scratch files `init_scratch` set, under `hydra_test_uitest_<pid>`. The `--db` and `--ini` seeds are copied into that folder by `reset_app` (the existing `--db` copy, plus the same for the ini); the seed path itself is only ever read by `copy_file`. This is today's rule for `--db`, extended to the ini.
2. Live mode refuses a seed that is not inside the Windows temp folder. The check is `testtemp::is_under_temp(path)`, a new function next to `temp_dir` in `tests/temp_util.h`, which is the one owner of the temp folder lookup. `C:\Program Files\Hydra\hydra.db` can never pass it, so even a helper bug that handed the real path over gets "refused: a live seed must be a copy under the temp folder" and no app is built.
3. `Hydra.exe` reads its ImGui layout from `--ui-ini`, the snapshot copy, through `ImGuiSetupOptions::ini_file`; the write-back goes to that copy. Nothing in the process holds the real folder's path.

So the real `hydra.db` is two copies away from the app, and the one that is checked by code is the second. The user's `hydra_settings.ini` makes the real song folders, settings and leaderboard user show up; a settings change saves to the scratch copy. The user's `hydra_ui.ini` makes the window open where it was and the library split as it was. Column widths come from the shares rule, which does not read the ini.

Audio stays headless and the leaderboards stay canned, because `reset_app` sets both seams. In window mode the canned fetcher means "Compare with dmleaderboards" shows one user named alice, not the real list; question 6 asks whether to add a switch for the real server. Play in the Preview advances the clock and shows Pause but makes no sound (question 7). A batch over the real library analyzes real charts for real; that is the point of the mode, and it writes to the copy.

The attached engine runs at normal speed, with the mouse drawn and human-speed moves, so the user can watch. Claude's waits are the same caps. When the helper disconnects or sends `quit`, the live test returns; T5 makes `main.cpp` post a close so the window goes away rather than sitting open as `--uitest` leaves it today (question 8).

### Dev builds only

The interpreter, the live test and the seed guard live in `hydra_uitest_harness`, which only `hydra_uitest` and a dev `Hydra.exe` link. The `--live` and `--ui-ini` handling in `main.cpp` goes under `#ifdef HYDRA_UITEST_ATTACHED` with the rest. The `ship` preset sets that off, and the installer's `install(TARGETS ...)` list does not name `hydra_uitest`. No new option, define or target is needed.

### Tests, none of which need the user's library

The C++ unit tests go in `hydra_tests` (the doctest runner picks up any `tests/test_*.cpp`). `tests/test_live_protocol.cpp` pins the JSON reply line for an ok, a failure with a quote and a newline in it, and the hello line, as literals, and pins `is_under_temp` on `<temp>\hydra_live_1\hydra.db` (true) and `C:\Program Files\Hydra\hydra.db` (false). The live protocol helpers live in `tests/ui/live_protocol.h` with no engine dependency so this runner can include them.

The GUI tests go in `hydra_uitest`. A checked-in script, `tests/ui/scripts/smoke.txt` (scan, continue, state, dump, rect, key Escape), proves the interpreter split by running `--script` as before, and the `scan` test proves nothing else moved.

The Python tests go in `tools/test_hydra_ui_mcp.py` (unittest, like `test_compare_db.py`), in two layers. The first runs without any build: `tools/fake_hydra_live.py` is a stand-in that makes the same pipe and answers the same verbs with canned text, and can be told to exit, to hang or to answer with 50,000 characters. Against it the tests pin: the hello line is parsed; a tool call sends one line and returns the reply; a hanging stand-in is reported as "did not answer within N s" and killed; an exiting stand-in is reported with its exit code and log tail; a long reply is cut at the cap with the dropped-lines line; `hydra_run` stops at the first failure; the snapshot copies the five files from a fake library folder and leaves the originals' bytes and modification times unchanged; the command line the helper builds never contains the library folder's path; the snapshot is refused while a process named `Hydra.exe` runs from the fake folder (the check is a function the test can hand a fake process list). The second layer runs only when `build-cpp\Release\hydra_uitest.exe` exists: start headless, `state` reads `store=ready`, `run` of the smoke lines ends with `charts=97` in the state, `reset` gives `charts=0`, `stop` exits 0. The window-mode smoke runs only when `HYDRA_LIVE_WINDOW_TEST=1` is set, because it opens a window; it uses a seed database made from the headless session's scratch folder, not the user's.

### Docs and an ADR

`docs/agents/ui-testing.md` gains a section "Live mode and the MCP server": the two modes, the tool table, the four new verbs in the verb table, the snapshot rule, the caps, and the one-line rule that `hydra_start("window")` never opens the real library. The GUI testing section of `CLAUDE.md` gains one sentence pointing there and naming the server (`hydra` in `.mcp.json`). The label cheat-sheet is unchanged.

An ADR is warranted: "0030: Claude drives the GUI through the test engine's live channel, and the real library is only ever a snapshot". It records the three choices that will not be obvious later: the pipe-and-JSON-line channel instead of stdio or TCP, Python for the helper, and the two-copies-with-a-guard rule for the user's library. ADRs are internal and not review-gated.

## Who owns what after the change

- "What does this verb do to the UI": `run_command` in `tests/ui/uitest_commands.cpp`. The script interpreter and the live test both call it.
- "How long may a wait take": the session's `timeout`, owned by `uitest_commands.cpp` (moved from `run_script`), 30 s unless the `timeout` verb changes it. The helper's margin on top is the one new number (question 4).
- "Where are the app's database and settings": `app::db_path()` and `app::ini_path()`, unchanged. `init_scratch` sets them for every harness run.
- "Which files start a live session, and are they copies": `reset_app`'s seed copy (today's `--db`, now `--ini` too) and `testtemp::is_under_temp`.
- "What is snapshotted from the user's folder": one function in `tools/hydra_ui_mcp.py`, the only place the library folder's path is known.
- "What does Claude see": the tool table in the helper; the doc's table is a copy of it checked by the reviewer, not a second owner.

## Tasks, waves and files

Every task is sized for one Opus executor inside 100 tool calls and 20 minutes, in its own worktree, with the executor running only its own tests. One fresh Sonnet reviewer per merge fixes its findings and signs off (D107). The main session runs the full suite once per merge.

**Wave 1**, two tasks together, no shared files.

- **T1, the interpreter split and the string dumps (C++).** Owns `tests/ui/uitest_script.cpp`, the new `tests/ui/uitest_commands.h` and `.cpp`, `tests/ui/uitest_harness.h` and `.cpp` (the string forms of `dump_widgets` and `dump_state`, the `--ini` seed copy in `reset_app`, nothing else), `tests/ui/uitest_main.cpp` (the `--ini` flag and its passthrough), `tests/temp_util.h` (`is_under_temp`), the harness source list in `CMakeLists.txt`, the new `tests/ui/scripts/smoke.txt`, and `tests/test_live_protocol.cpp` for the `is_under_temp` cases only. Adds the `key`, `rect`, `reset` and `quit` verbs. Runs `hydra_uitest --script tests/ui/scripts/smoke.txt`, `hydra_uitest --test scan`, and `hydra_tests.exe -sf=test_live_protocol.cpp`.
- **T3, the helper and its stand-in (Python).** Owns the new `tools/hydra_ui_mcp.py`, `tools/fake_hydra_live.py`, `tools/test_hydra_ui_mcp.py` and `.mcp.json`. Builds every tool in the table against the stand-in, the caps, the crash and hang reports, the snapshot, the running-Hydra refusal and the truncation. Its first step installs `mcp` if question 1 says yes; if the package will not install on 3.14, it stops and reports rather than hand-rolling the protocol. Runs `py -I tools/test_hydra_ui_mcp.py` (the first layer only; no exe exists yet for the second).

**Wave 2**, one task, forked from wave 1's merge tip.

- **T2, the live channel (C++).** Owns the new `tests/ui/uitest_live.cpp` and `tests/ui/live_protocol.h`, the JSON cases in `tests/test_live_protocol.cpp`, the `--live` flag in `tests/ui/uitest_main.cpp`, the `register_live_test` declaration in `uitest_harness.h`, and the harness source list line. Step one is the pipe spike: a `--live` process whose test only echoes lines, opened from a 20-line Python client; if the plain `open` on the pipe path does not work within that step, it stops and reports with the TCP fallback named. Then the reader thread, the queue, the poll-and-yield loop, the error capture and status reset, the watchdog off, the hello line. Runs `hydra_tests.exe -sf=test_live_protocol.cpp` and the second layer of `py -I tools/test_hydra_ui_mcp.py` (headless only), which it may now un-skip.

**Wave 3**, two tasks together, forked from wave 2's merge tip.

- **T5, the real window (C++).** Owns `src/ui/main.cpp` only: `--live`, `--ui-ini` into `ImGuiSetupOptions::ini_file`, and `PostQuitMessage` when the live test has returned (question 8). Everything under `HYDRA_UITEST_ATTACHED`. Runs the window-mode smoke in `tools/test_hydra_ui_mcp.py` with `HYDRA_LIVE_WINDOW_TEST=1`, on a seed made from a headless scratch folder, and `hydra_uitest --test scan` to show nothing else moved. It must also build the `ship` preset once (`.\build_cpp.ps1 -Preset ship`) and confirm the exe does not contain the string `--live`, with the command and its output in the report.
- **T6, the helper against the real exes (Python).** Owns `tools/hydra_ui_mcp.py` and `tools/test_hydra_ui_mcp.py`. Runs the real headless session end to end from a Claude Code session with the server registered, fixes what the stand-in did not predict (reply timing, the exact hello line, the scratch and shots folders), and pins what it found in the second test layer. Runs `py -I tools/test_hydra_ui_mcp.py`.

**Wave 4**, one task, forked from wave 3's merge tip.

- **T7, docs and the ADR.** Owns `docs/agents/ui-testing.md`, the GUI testing section of `CLAUDE.md`, and the new `docs/adr/0030-...md`. Every verb and tool name is checked against `uitest_commands.cpp` and `hydra_ui_mcp.py`. No tests; the reviewer checks the names.

T4 is deliberately absent: the protocol header's JSON side went to T2 and the `.mcp.json` to T3, so no task was left too small to be worth a worktree.

## The real-app check before the last merge

Before T7 merges, the main session runs `hydra_start("window")` from a Claude Code session on the user's real library folder with Hydra closed, and with the user watching: opens a song, flips 2x Bass and back, narrows the library with a search to a few charts and runs `Analyze shown (N)...` on them, then `hydra_stop`. The batch is narrowed on purpose: a batch over the whole library, even on the copy, is a library run that needs the user's yes in chat (CLAUDE.md rule 8). It then shows the user that `C:\Program Files\Hydra\hydra.db` has the same size and modification time as before the session, and that the snapshot folder holds the changed copy. That is the proof of decision 2 on the real files, which no test can give.

## What this plan does not do

It does not change the app's code outside `main.cpp`'s attached-mode block. It does not touch `src/app/config.cpp`, the store, the stamps or any stored data. It does not make the helper build anything. It does not add a way for a shipped Hydra to be driven. It does not run anything over the user's whole library on its own; a batch in window mode starts only when Claude clicks it under the user's instruction, and the library-run rule (brief preamble, "Tests") still applies to Claude.

## Open questions

Each one quotes the code behaviour it depends on and gives a recommended answer. A new number is a question unless the code already owns it.

**Answered 2026-10-14:** the user accepted every recommended answer below, and approved the build in the same message. The numbers are therefore settled: a 15 s margin over each command's timeout, 60 s for the hello line, and a 12,000-character reply cap.

**1. Install the `mcp` package?** The helper imports `mcp` (`FastMCP`), which is not on this machine; PyPI's 2.3.0 supports Python 3.14. The alternative is to hand-write the stdio JSON-RPC handshake (initialize, tools/list, tools/call) in about 150 lines with no dependency, which then has to track protocol versions by hand. Recommended: run `py -m pip install mcp` once; T3's first step does it and stops if it fails.

**2. One session at a time?** The helper keeps one child process and one pipe. `hydra_start` while a session runs stops the old one first. The alternative, headless and window sessions side by side, needs a session name on every tool. Recommended: one at a time.

**3. Refuse or warn while the installed Hydra is running?** A copy of `hydra.db` taken while Hydra holds it open can be torn: ui-testing.md already tells users of `--db` to "Close Hydra first: a database it has open is not a clean copy". Recommended: refuse, with that sentence as the reply. The alternative is to copy anyway and say so in the reply.

**4. The helper's margin on each reply.** Every wait inside a command is bounded by the session's `timeout` (30 s default, `uitest_commands.cpp`). The helper must wait a little longer than that before calling Hydra silent, and `hydra_start` must bound the time to the hello line, which in window mode includes copying an 18 MB database and opening it (the startup open of the installed library has taken about half a second cold in D100's measurements, but a slow disk is possible). Recommended: a margin of 15 s over the command's timeout, and 60 s for the hello line. Both are new numbers and live in one place at the top of `hydra_ui_mcp.py`.

**5. The reply size cap.** `text` on the main window is a few hundred lines; a `dump Hydra` with the song panel and a report window open can be a thousand lines. Claude reads all of it per call. Recommended: 12,000 characters, with the dropped-lines line and `contains` to narrow. It is a new number; one constant in the helper.

**6. Real leaderboards in window mode?** `reset_app` cans the fetcher every time (`uitest_harness.cpp` line 320), so "Compare with dmleaderboards..." in window mode shows the one canned user. A `--net real` flag that skips the canned fetcher would let Claude check the real comparison window against the user's real library, with real HTTP GETs to dmleaderboards. Recommended: not in this plan; canned stays, and the flag is a follow-up if the user wants it, because the comparison's logic is already covered by the report-window tests.

**7. Real audio in window mode?** `set_headless(true)` means the Preview's Play opens no device; the clock advances and Pause shows. Recommended: keep it headless. Nobody is listening while Claude drives, and a real device adds a failure mode the user would hear and Claude would not.

**8. Close the window when the session ends?** Today `--uitest` keeps the window open after the queue drains "so the end state can be inspected" (`main.cpp` line 441). In live mode the helper is the one that ends the session, and a stray window left behind each time would pile up. Recommended: close on `quit` or disconnect, and give `hydra_stop` a `keep_window=True` option for the case where the user wants to look.

**9. Does `hydra_start("window")` ask before it copies?** The copy reads from the user's install folder and takes 18 MB plus the ini files. It writes nothing there. Recommended: no extra confirmation inside the tool; the user's instruction to Claude to use window mode is the yes, and the reply names what was copied.

**10. Keep the helper's scratch folder after `hydra_stop`?** The snapshot and the screenshots live in `hydra_live_<pid>` under the temp folder. Recommended: delete it on `hydra_stop` unless `keep=True` was passed to `hydra_start`, and let the SessionStart cleanup sweep (`cleanup-sweep-hook-2026-10` in memory, which removes 7-day-old scratch) catch what a killed helper leaves.
