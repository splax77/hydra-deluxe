# GUI testing: drive the real UI by label, read back text

Hydra's UI is Dear ImGui. The Dear ImGui Test Engine (vendored at `third_party/imgui_test_engine`) can find a widget by its label, click it, type into it, and read what is on screen. `hydra_uitest` runs the whole UI with no window on a software GPU, so an agent can verify a GUI change in about a second and get **plain text** back — not screenshots.

Use this instead of launching `Hydra.exe` and taking screenshots. Reach for a screenshot only when something looks visually wrong.

## Build and run

```bash
.\build_cpp.ps1 -Target hydra_uitest
```

```bash
.\build-cpp\Release\hydra_uitest.exe --all
```

Agents do not run `--all`. They run only the tests for their own change, with `--test <name>`; the rule is in `docs/agents/brief-preamble.md`. The main session runs the merge's tests once; CLAUDE.md rule 1 says which.

| Command | What it does |
|---|---|
| `hydra_uitest --all` | run every checked-in test, each in its own process |
| `hydra_uitest --test scan` | run one test, in this process; give `--test` more than once and each test gets its own process |
| `--jobs <n>` | how many test processes run at once (default: `kDefaultJobs` in `uitest_main.cpp`) |
| `hydra_uitest --list` | list the tests |
| `hydra_uitest --script file.txt` | run a command file (see below) |
| `hydra_uitest --live --pipe <name>` | take commands one at a time from the named pipe `<name>` (see "Live mode and the MCP server"; `run_live` in `tests/ui/uitest_live.cpp` decides when it ends). The two flags go together, and `--live` runs alone |
| `--keep-temp` | keep the scratch folder (DB, INI) and print its path |
| `--shots <dir>` | where `screenshot` files go (default: the scratch folder) |
| `--db <file>` | start each test from a copy of this database instead of an empty one (the file itself is never opened). A missing file is an error. Close Hydra first: a database it has open is not a clean copy |
| `--ini <file>` | start each test from a copy of this settings file instead of the scratch settings. A missing file is an error. Copied the same way as `--db` |

Output is `[PASS]`/`[FAIL]` per test. A failed test prints the engine's log. The log names the check that failed (like `uitest_library.cpp:120`) and every action before it. A run of more than one test ends with an `N of M passed` line. The exit code is 0 only when everything passed. `ctest` runs `--all` too, so ctest, CI and a run by hand all mean the same thing.

**One run mode.** A run of more than one test gives each test its own `hydra_uitest --test <name>` process, at most `--jobs` at once. Each process has its own scratch folder and a fresh ImGui context, so no test sees another's leftovers, and the order the tests run in does not matter. The results print in registration order. A single `--test`, or a `--script`, runs in the one process instead: a script needs the test engine in that process, and one test is the debugging case. A script runs only on its own, never beside other tests. `hydra_uitest --list` prints the current tests. The full suite took about 90 s at the default job count, measured on 2026-10-10 at commit 4c037634 over 1 run, on a machine whose CPU other builds held at 100%; a quiet machine will be faster.

The 97 test charts analyze in a blink: a whole-library batch can start and finish between two frames. So a test that looks at a running batch (the strip, Pause, Stop, the settings lock) makes a `BatchGate` before it starts the batch. The gate holds each chart until the test lets it through with `allow(n)`, and `started()` says how many charts have reached it. The run is then provably still going when the test looks, however busy the machine is. The gate runs the batch on one worker unless given a count: `BatchGate gate(3)` runs three, so three charts sit at the gate at once (`batch-strip-workers` tests the strip that way).

Each test starts from scratch: a temp folder with a settings INI (song folder = `testdata/input`, 97 charts; "open report automatically" off; search depth 2) and an empty DB. No sound card is touched, and the dmleaderboards API is canned (one user, `alice`, id `111`, whose one score is the first library chart). The reports live in memory and write no file. The seams are `audio::set_headless`, `net::set_fetcher`, and `app::set_path_overrides`.

## Command files (no rebuild)

A command file is one verb per line. Example:

```
click Scan library
wait-idle
click //Scanning charts/Continue
state
type **/##search | Burnout
wait 0.2
click **/Burnout
timeout 300
wait-idle
click **/##DetailsTabs/Stars
wait 0.2
text
dump Hydra
screenshot stars.png
```

| Verb | Meaning |
|---|---|
| `window <ref>` | set the window the following refs are relative to (default `//Hydra`) |
| `click <ref>` | click a widget |
| `check <ref>` / `uncheck <ref>` | set a checkbox |
| `type <ref> \| <text>` | put text into an input. The text is everything after the first ` \| `, kept exactly, spaces included. A line that ends in a bare ` \|` types nothing, which clears the field |
| `open <title>` | open a song: type the title into `**/##search`, then open the row whose title is exactly `<title>` (`open_titled_reason` in `uitest_harness.cpp`). When no shown row has that title, it refuses and says what the search shows. It waits for the song's tabs to settle for at most the timeout in force (the `timeout` verb); past that it refuses with `opened "<title>", but its analysis was still running after N s; use wait-idle`, and the song stays open. The ref goes back to `//Hydra` afterwards |
| `key <keys>` | press a key or a chord, such as `Escape`, `Enter`, `Down`, `Ctrl+W` or `Ctrl+Shift+Tab`. The names it knows are in `key_named` and `parse_chord` in `uitest_commands.cpp`; an unknown name fails |
| `wait <seconds>` | let frames run |
| `wait-idle` | wait until every background job has finished; the list is `AppState::any_job_running` |
| `wait-text <substring>` | wait until the text appears on screen |
| `expect-text <substring>` / `expect-not-text <substring>` | assert on screen text now |
| `text [\| <substring>]` | print everything on screen as text. After a bar, only the lines holding the substring, or a line saying none did |
| `state` | print the key app state (library count, rows, selection, jobs, preview, status, and the store line: `store=opening`, `store=ready` or `store=failed`) |
| `dump [<window>] [\| <substring>]` | print the widget tree of every window whose name holds `<window>`, child windows included, at every depth; each item is listed under its own window: label, id, rect, checked/disabled/opened/inputable. No window means every window. After a bar, only the item lines holding the substring, each under its window's line |
| `rect <ref>` | print one widget's line, in the dump's shape. A ref that finds nothing fails |
| `screenshot <file.png> [\| <ref>]` | save the frame, and print `screenshot: <path>`. After a bar, save only that window's or widget's rectangle, clamped to the frame: the ref is tried as a window first, then as a widget, such as `screenshot report.png \| //###pathreport`. A ref that finds neither fails with `screenshot found no window or item "<ref>"` |
| `timeout <seconds>` | change the wait-text / wait-idle limit (default: `CommandSession::timeout` in `uitest_commands.h`) |
| `reset` | a fresh app on a fresh scratch library, with the ref back on `//Hydra`. With `--db` or `--ini`, the seeds are copied again |
| `quit` | end the run here. In a script that is a pass |
| `#` | comment |

`timeout` and `wait` take a plain number of seconds, 0 or more, and nothing else. What counts as a plain number is `hydra::parse_finite_number`'s rule (`src/core/strutil.h`). So `timeout 90s`, `wait abc` and a bare `wait` fail, naming what they were given.

A bar with nothing after it (a line ending in ` |`) is the same as a bar with empty text. So `text |` and `dump Hydra |` filter on nothing and keep every line. `split_bar` in `uitest_commands.cpp` is the one place a line is split at its bar.

Lines print as they run. A failed engine action, like a click that finds nothing, prints `!! failed at line N: <line>`. A line the interpreter itself refuses (an unknown verb or key, a bad number, an `expect-text` that does not hold, a `rect` that finds nothing, a wait that ran out) fails the test, and its log names the line as `line N: <reason>`. The script stops at the first failure.

Two guards sit in front of `click`, `check`, `uncheck` and `type`; `item_ref` in `uitest_commands.cpp` owns both. First, a lookup of a library row's title is refused while the search box is empty, because finding a row that is not drawn makes the test engine scroll the whole library; use `open <title>` instead, or type into `**/##search` first. A widget outside the library that is drawn with the same label (a button that shares a song's title) is not refused. Second, a bare label that finds nothing in the current window is tried once more as `**/<label>`, and the reply starts with `used ref **/<label>` so you learn the path that works. The test engine keeps only the last widget a `**/` lookup matches, so the guard counts the drawn widgets under the current window with that label first (`drawn_items_labelled`, the gather `dump` walks) and refuses the retry when two or more carry it, naming them. A ref of several labels is not counted; the engine matches its whole path.

Clicking a song starts its analysis (D87 item 1), and `wait-idle` waits for it. There is no `Done!` flash, so the tabs show as soon as it finishes. A tab click still needs a frame or two before its text shows.

## Refs (how to name a widget)

- A bare label is looked up in the current window: `Scan library`.
- `//Window/Label` is absolute: `//Scanning charts/Continue`, `//Analyze library/Start analyzing`.
- `**/Label` searches every child window too. Most of the screen now lives in child windows (the library, the song panel, the batch strips), so use it for anything below the toolbar: `**/##search`, `**/Try again`, `**/Expand all`, `**/Pause`.
- `##id` labels work as written: `**/##spcap`, `**/##DetailsTabs/Preview`, `**/##scrub`.
- `###` labels are addressed by the part after `###`: DM picker rows are `**/###<discord id>`.
- Library rows: `**/<title>`; escape `/` and `#` in the title with a backslash.
- Child window names are mangled (`Hydra/##songpanel_BBB91B00`), so a C++ test points at one with `ctx->WindowInfo("//Hydra/##songpanel").Window`, the way `set_panel_ref` does.

## Label cheat-sheet

These are the labels the merged app draws. Every GUI test finds widgets by them. The `...` is three ASCII dots.

| Area | Labels |
|---|---|
| Version bar, child `##versionbar` (first in window `Hydra`, above the toolbar) | the name text `Hydra Deluxe <version>` (from `named_version`), which has no click and no hover; then the status from `version_bar_status`: `Up to date`, `Checking for updates...`, `Hydra couldn't check for updates when it started.` with the reason, `Hydra Deluxe <new> is available.` (with `Skipped.` after a skip), or `Only the installed copy of Hydra Deluxe checks for updates.`. Update buttons, by label: `Update##update`, `Skip this version`, `Later`, `Cancel##update` while downloading, `OK##update` on the `Updated to Hydra Deluxe <version>.` note |
| Toolbar (window `Hydra`) | `Manage folders... (N)`, `Scan library`, `Analyze library...`, or `Analyze shown (N)...` while the search or a status chip narrows the table (N is the heading's count, `analyze_shown_label`), `Compare with dmleaderboards...`, `Open path report` / `Building path report...` |
| Status line | a problem stays until `X##dismissstatus` |
| Analysis settings button (window `Hydra`) and its panel | The button is `###analysissettings`; its text is `Analysis settings: <changes>`, made by `settings_button_label` (`src/ui/library_dialogs.cpp`). Clicking it opens the panel, a popup: a top-level window of its own, outside `//Hydra`, that takes focus. One ref reaches a control in it: `//$FOCUSED/**/<label>`. The `**/` is there because the controls sit in tables. (That ref works only through a Hydra patch to the vendored test engine; `THIRD_PARTY_NOTICES.txt` lists the patches.) Each row has the setting's name and its `(?)` on the left and only the control on the right, so the checkboxes have no visible label. Combo `##difficulty`, checkboxes `##prodrums`, `##bass2x` and `##noteshuffle`, input `##spcap`, checkbox `##legacyfills`, input `##depthvalue`, combo `##depthmode`, checkbox `##mslimit`, input `##mslimitvalue`. A `**/` ref can't find a combo, because the test engine files combos with an empty label; find one through a checkbox in its group with `settings_control`. The helpers in `uitest_harness.h` do all this: `open_settings_panel`, `settings_control`, `settings_combo_pick` and `close_settings_panel`. Close the panel before a test clicks anything else or ends: an open popup keeps other windows from being hovered, and it outlives the test. Only a batch locks the settings; the label then ends in ` (locked)` and the panel shows `Stop the batch to change these.` (D90 item 2) |
| Library, child `##library` | search input `**/##search`, clear button `X##clearsearch`, chips `All (N)##chipall`, `Not analyzed (N)##chipnew`, `Stale (N)##chipstale`, `Analyzed (N)##chipdone`, table `##librarytable` with columns `Title`, `Artist`, `Album`, `Charter`, `Folder`, `Best path` (an open song hides `Album`, `Charter` and `Folder`). A Best path cell reads `Not analyzed`, `Stale`, or `<score>  <path>`. A search that matches nothing shows `No charts match your search.` in place of the table |
| Song panel, child `##songpanel` | `Hide library` / `Show library`, `<##prevsong`, `>##nextsong`, `X##closepanel` (Escape does the same). A click's analysis shows `Analyzing chart...` with `Cancel` once it has run 0.15 s; a cancelled one shows `Analysis cancelled.` with `Try again`; a failed one shows its error with `Continue`. Tab bar `##DetailsTabs` with tabs `Paths`, `Preview`, `Dynamics`, `Stars` |
| Paths tab | path buttons `##path<i>` (0-based), `Expand all` / `Collapse all`, activation rows `##act<i>` (1-based), links `Show in Preview >##showact<i>`, folds `Backend timings##act<i>`, `Multiplier squeeze##mult`, `Score breakdown##breakdown`, button `Copy path` (flashes `Copied!`), checkbox `Hide backend rows beyond##backendlimit`, input `##backendlimitvalue` |
| Preview tab | text `Showing` beside the combo `##previewpath`, `< Act##prevact`, `Act >##nextact`, transport `-5s`, `< 10 ms`, `Play` / `Pause`, `10 ms >`, `+5s`, sliders `##volume` and `##scrub`. Over the highway, while each is drawn: `##time_box`, `##score_box`, `##drain_box`, `##next_act_box`, `##solo_box` (during a solo and its result) and the gauge `##sp_gauge` |
| Batch confirm, popup `Analyze library` | checkbox `Also re-analyze charts that already have a result##redo`, buttons `Start analyzing` and `Cancel` |
| Running batch, child `##batchstrip` | `Pause` / `Resume`, `Stop` |
| Finished batch, child `##batchdone` | `Open report`, `X##dismissdone`, checkbox `Open automatically` |
| Path report window, `###pathreport` (title `Path report — Hydra`) | header `Refresh` (Ready only) and `Close` (every state); out-of-date strip `Refresh##outofdate`; search `##search`, combo `##timing` (`All timing tiers` first), checkbox `Optimal paths only`; table `##pathtable`, whose rows are clicked by their row number (`**/1`); when nothing passes, `Clear filters`; while building, `Cancel`; cancelled or failed, `Try again`. `Escape` and `Ctrl+W` close it |
| Comparison window, `###dmreport` (title `dmleaderboards: <player> — Hydra`) | the same frame, plus `Compare another player...` in the header and the failed state; combo `##status` (`All charts` first, then the statuses, like `##status/Not in your library`); table `##dmtable`. A `not in library` row has no click |
| Startup screen (window `Hydra`, drawn instead of everything above until the store is open) | an empty window until the click's progress delay (`kViewProgressDelaySeconds`) has passed. Then a slow normal open reads `Opening your library...` with no bar. An upgrade of an old library file shows a bordered box: `Updating your library file for this version of Hydra`, `This happens once. Your charts and results are kept.`, then a step line (`Copying your library...`, `Updating the results table...` on a 1.8.x file, `Finishing...`) over a bar whose overlay reads `rows done / rows total` (`12,345 / 38,009`). A time-left line follows the Song Preview's rule and words. There are no buttons |
| Other modals | `Scanning charts` (`Continue`, `Cancel`); `Song folders` (`Add folder...`, `Scan now`, `Close`); `Compare dmleaderboards user` (`Cancel` while the list loads, `Retry` and `Close` when it fails, then `##dmfilter`, player rows `###<discord id>` and `Close`; picking a player closes the box and opens the comparison window) |

When unsure, `dump Hydra` and read the labels off it. The dump lists items at every depth, so controls inside tables and pushed IDs show too, indented by their depth. It prints whole labels, through a Hydra patch to the test engine (`THIRD_PARTY_NOTICES.txt` lists the patches). Combos print with an empty label; take their `##id` from the source.

## The report windows

The path report and the dmleaderboards comparison are their own windows (ADR 0027). In the app each one is a separate OS window. `hydra_uitest` has no platform backend, so ImGui turns that off by itself and each report window draws inside the main viewport. The test engine finds windows by name across viewports, so the same refs work in both. A report window's name ends in `###pathreport` or `###dmreport`, so its title can change without moving its saved place. A C++ test points at one with `window_named("###pathreport")` and `ctx->SetRef(...)`; a command file can try `window //###pathreport`.

Their tests live in `tests/ui/uitest_report_windows.cpp`, in two groups.

The first group opens the windows on sample rows, with no AppState. The test's own GUI function (`draw_windows`) draws both windows from a `ReportWindowInput` it fills itself, and records what each button hands back. The rows are the shared samples in `tests/report_samples.{h,cpp}`, the same ones `tests/test_report_page.cpp` pins the tiles on. These tests are `report-window-sort`, `report-window-filters`, `report-window-states`, `report-window-keys` and `report-windows-both`.

The second group goes end to end through AppState: the toolbar, the batch strip, the dm picker and the analysis settings. These are `report-window-open-path`, `report-window-dm-handover`, `report-window-row-click`, `report-window-reopen`, `report-window-reopen-keeps-filters` and `report-window-out-of-date`. They use the canned dmleaderboards API above and never reach the real server.

## Watching it run: attached mode

```bash
.\build-cpp\Release\Hydra.exe --uitest scan
```

Runs the same test inside the real window at human speed, with the Test Engine's own panel showing. Accepts a test name, `all`, or a command-file path. Here `all` runs every test one after another in the one window, so a test can see the previous test's leftovers; `hydra_uitest --all` is the run that counts. Results and any `text`/`state`/`dump` output go to `hydra_uitest.log` next to the exe (`--uitest-log <file>` to change); the window stays open afterwards so the end state can be inspected. Attached mode also runs on the scratch library, never on the real `hydra.db` — the tests wipe their DB at start.

Attached mode exists only in dev builds (`build-cpp`). The installer builds with the `ship` preset, which leaves the GUI tests out, so an installed Hydra.exe ignores `--uitest` and `--live`.

## Live mode and the MCP server

A command file runs top to bottom, and you see nothing until it ends. Live mode keeps the app running and takes one command at a time, so Claude can read the screen and then decide what to click next. The commands are the same verbs as a command file, run by the same interpreter.

Claude reaches live mode through the project's MCP server, `hydra`. MCP (Model Context Protocol) is how Claude Code starts a helper program, asks it which tools it has, and sends it each tool call. The server is registered in `.mcp.json` at the repo root, and its code is `tools/hydra_ui_mcp.py`. Each tool call becomes one command line, and each reply comes back as plain text. So Claude clicks, types and reads Hydra by widget label, with no screenshots.

### Setting it up

The helper needs Python's `mcp` package, installed once with `py -m pip install mcp`. Claude Code asks the user to approve the server the first time it is used. The helper runs the exes in `build-cpp\Release` and never builds them. When one is missing, its reply names the `build_cpp.ps1` command that makes it.

### The two modes

**Headless**, `hydra_start("headless")`, runs `hydra_uitest.exe --live`. It is exactly a GUI test: no window, the `testdata/input` corpus, an empty database, no sound and the canned leaderboards. A command tool called before the first `hydra_start` starts a headless session by itself. After a session stops, every tool answers `no session; call hydra_start` until one is started.

**Window**, `hydra_start("window")`, runs the real `Hydra.exe --live` window on a copy of the user's library, so the charts, results, settings and layout are real. The user can watch it, because it runs at normal speed with the mouse drawn. The test engine's own panel is hidden in this mode, so it covers none of the layout and none of its text reaches `hydra_text`. Sound stays off and the leaderboards stay canned, as in every test.

Before it launches anything, window mode takes a snapshot. The helper copies `hydra.db`, its `-wal` and `-shm` when present, `hydra_settings.ini` and `hydra_ui.ini` into a fresh folder of its own under the Windows temp folder. The library folder is `library_dir` if given, else the `HYDRA_LIBRARY_DIR` variable, else `DEFAULT_LIBRARY_DIR` in the helper. The reply lists each file copied with its size and modification time, so you know how old the copy is. The helper refuses to copy while a `Hydra.exe` runs from that folder, because a database Hydra has open is not a clean copy. `snapshot_library` is the only code that knows the library folder's path.

The real `hydra.db` can never be opened in window mode. Three things keep it out:

1. The helper names only its snapshot on Hydra's command line, as `--db`, `--ini` and `--ui-ini`.
2. Hydra copies those seeds again into its own scratch folder (`reset_app`) and opens only that copy. The seed itself is only read.
3. The live test refuses a seed that is not under the temp folder (`testtemp::is_under_temp` in `tests/temp_util.h`). Even a helper bug that passed the real path would get "refused: a live seed must be a copy under the temp folder", and no app would be built.

Changes Claude makes in window mode, a batch included, land in the copy. A batch there is still real work on real charts, so the library-run rule in `docs/agents/brief-preamble.md` applies: narrow the library with a search first. `hydra_reset` goes back to the snapshot taken at start; a new `hydra_start` takes a new one. When the session ends the window closes, unless `hydra_stop(keep_window=True)` leaves it open for the user to look at. If the user closes the window, Hydra exits and the next tool reply says the session is stopped.

Both modes exist only in dev builds. The live code sits in the test harness and under `HYDRA_UITEST_ATTACHED`, which the `ship` preset leaves out.

### The tools

Each tool sends one command line and returns Hydra's reply text, unchanged except for the length cap below. The tool list in `tools/hydra_ui_mcp.py` is the owner; this table is a copy of it.

| Tool | Sends | Notes |
|---|---|---|
| `hydra_start(mode, library_dir=None, keep=False)` | launches the exe | `mode` is `headless` or `window`. Stops any running session first. The reply gives the pid, the engine's scratch folder, the wait cap, the log file and the helper's scratch folder, plus the snapshot list in window mode. `keep=True` keeps the helper's scratch folder after `hydra_stop` |
| `hydra_stop(keep_window=False)` | `quit` | waits for Hydra to exit, kills it if it does not, and returns the exit code. Then deletes the helper's scratch folder unless `keep` was set |
| `hydra_reset()` | `reset` | a fresh library in the same process |
| `hydra_open_song(title)` | `open <title>` | the way to open a song. Clicking a row's title with the search box empty is refused (see the two guards above) |
| `hydra_click(ref)` | `click <ref>` | widgets below the toolbar need `**/`; a bare label that finds nothing is retried as `**/<label>`, unless several widgets carry it |
| `hydra_type(ref, text)` | `type <ref> \| <text>` | an empty `text` clears the field. The library search is `**/##search`; the same retry applies |
| `hydra_check(ref, on)` | `check <ref>` or `uncheck <ref>` | |
| `hydra_key(keys)` | `key <keys>` | |
| `hydra_window(ref)` | `window <ref>` | |
| `hydra_wait_idle(timeout=None)` | `wait-idle` | a `timeout` applies to this call only: the helper sends `timeout`, the wait, then the old timeout back |
| `hydra_wait_text(text, timeout=None)` | `wait-text <text>` | the same one-call `timeout` |
| `hydra_text(contains=None)` | `text`, or `text \| <contains>` | |
| `hydra_state()` | `state` | |
| `hydra_dump(window=None, contains=None)` | `dump [<window>] [\| <contains>]` | no `window` means every window |
| `hydra_rect(ref)` | `rect <ref>` | |
| `hydra_screenshot(name)` | `screenshot <name>` | give the `.png` yourself. The file lands in the helper's scratch folder, which `hydra_stop` deletes unless `keep` was set |
| `hydra_run(script)` | each line in turn | stops at the first failure with `stopped at line N of M`, counting the script's own lines |

`hydra_run` exists so a known sequence (scan, continue, search, open) costs one tool call instead of five. To open one song, `hydra_open_song` does the search and the click in one call.

### An open dialog

A dialog of the kind that blocks the rest of the app (a modal) stops every click and key outside it. `Scan library` leaves one open: the "Scanning charts" dialog stays on its "Done" line until you click `//Scanning charts/Continue`. The checked-in tests never see it, because the harness's `scan_library` clicks Continue itself.

The replies name an open dialog, with its buttons, in one line that `open_modal_note` in the test harness writes. `wait-idle` still answers ok once jobs stop, but its text is that line while a dialog is open. A command that fails while a dialog is open adds the line after its first line, or after the `!! failed at line N` line in a command file. `dump` lists the open dialog's window first, so a long dump that the helper cuts from the end still shows it.

### The protocol

The helper and Hydra talk over a Windows named pipe. The helper picks the pipe's name and passes it with `--pipe <name>`, which `--live` requires on both exes. Hydra opens exactly that name and never makes one up.

**The hello line.** On connect, Hydra writes one line first: `{"ok":true,"hello":{...}}`, holding `mode`, `scratch` (the engine's scratch folder), `pid`, `timeout` (the starting wait cap), `log` and `crash_dump`. `log` is the file Hydra writes its output to; it is empty for `hydra_uitest`, which writes to stdout, and the helper captures that itself. For `Hydra.exe` the helper asks for a `window.log` in its own scratch folder with `--uitest-log`. `crash_dump` is where a crash's minidump would go: `crash.dmp` in the engine's scratch folder for `hydra_uitest`, and empty for `Hydra.exe`, where nothing writes one.

**Requests and replies.** A request is one line, exactly as a command-file line. Every line gets exactly one reply, blank and `#` lines too. A reply is one JSON line: `{"ok":<true or false>,"text":"<finished text>","timeout":<seconds>}`.

- On success, `text` is what the verb printed, or `ok` when it printed nothing.
- On a failure, `text` is the reason on its first line, then the engine's log lines from this command. A live reply names no line number; only `hydra_run` numbers lines.
- `timeout` is the wait cap in force after the command.
- A failed command does not end the session. The next command runs normally.

`live_protocol.h` writes these lines, and `tests/test_live_protocol.cpp` pins them.

**Numbers and the wait cap.** The strict number rule above holds in live mode too. On top of it, a live `wait` longer than the timeout in force fails. So no command can run longer than the timeout Hydra last reported, plus the time of one action. The helper waits that long plus `REPLY_MARGIN_S` for each reply, and `HELLO_TIMEOUT_S` for the hello line. All its numbers live at the top of `tools/hydra_ui_mcp.py`.

**A dead or silent Hydra.** The helper reports it and never waits on it. When Hydra has exited, the reply says `Hydra exited with code X`, quotes the last lines of its log, and names the crash dump if one was written. When Hydra is alive but silent past the cap, the reply says `Hydra did not answer within N s`, and the helper kills it. Either way the session is stopped, and the helper's scratch folder is kept so the log can be read.

**Long replies.** A reply longer than `REPLY_CAP_CHARS` is cut at a line boundary, with a last line saying how many lines were dropped. `contains` on `hydra_text` and `hydra_dump` narrows the output instead.

### Who owns what

Hydra's C++ side owns every command rule: what a verb means, how its arguments parse, which numbers it takes, the timeout, line numbers, the reply's wording, the filters, and where the log and crash dump go. The helper only carries messages. It owns launch and stop, the pipe name, the reply caps (worked out only from numbers Hydra reports), the snapshot, the running-Hydra check and the tool list. The stand-in Hydra its tests use, `tools/fake_hydra_live.py`, plays only transport: a hello line, a plain echo, a hang, an exit, a closed pipe and an oversized reply. Every test of what a command does runs on the real headless `hydra_uitest --live` (`RealExeLayer` in `tools/test_hydra_ui_mcp.py`).

So a question about a command is answered in C++, never in Python. The full table is in "T8: one owner for every command rule" in `docs/superpowers/plans/2026-10-14-hydra-mcp-server.md`, and the reasons are in `docs/adr/0030-hydra-owns-every-live-command-rule.md`.

## Adding a C++ test

Tests are split by area into four files: `uitest_library.cpp`, `uitest_details.cpp`, `uitest_preview.cpp` and `uitest_batch_reports.cpp`. Each file ends with an entry table (`library_tests()`, `details_tests()`, and so on). Write the test in the file for its area, then add `{"thing", test_thing}` to that file's table. `uitest_paths.cpp` holds the Paths tab tests and registers them itself, from `register_paths_tests`. `uitest_report_windows.cpp` registers its own: the sample-row tests from `register_report_window_tests`, and the end-to-end ones from `report_window_flow_tests()`, which `register_tests` calls last.

`register_tests` in `uitest_tests.cpp` registers the four tables, then the Paths tab and report window tests. Order does not matter, because every run of more than one test gives each test its own process. So a new or renamed test needs no other change.

Template:

```cpp
void test_thing(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);                 // fresh scratch library
    scan_library(ctx);            // helper: Scan library + Continue
    if (ctx->IsError()) return;   // helpers only return from themselves
    open_details(ctx, 0);         // click row 0, wait for the song panel, land on Paths
    if (ctx->IsError()) return;
    wait_song_analyzed(ctx);      // the click's analysis finishes and is Ready
    if (ctx->IsError()) return;
    ctx->ItemClick("##DetailsTabs/Stars");  // the ref points at the panel now
    IM_CHECK(visible_text(h).find("Base score") != std::string::npos);
}
```

The shared helpers live in `uitest_harness.{h,cpp}`. `open_titled(ctx, search, title)` searches and opens a song by title. `set_panel_ref(ctx)` points refs at the song panel. `wait_song_analyzed(ctx)` waits for the open song's analysis, which a click or a setting change starts, and checks it is Ready. `open_preview(ctx)` opens row 0 on the Preview tab and waits for it to load.

Rules of thumb:

- Wait on app state (`h.app->…`) with `wait_until`, never on frame counts. Jobs are real threads.
- To look at a running batch, hold it open with `BatchGate` (see above); never hope it is still running.
- To look at a click's running analysis (its progress box, Cancel, a setting changed under it), hold it with `ViewGate`. Make the gate after `reset_app`; every click's job then waits at it until `open()`, and `started()` counts the jobs that reached it.
- The app opens its store on a worker thread. `reset_app(h)` waits for that open (`AppState::wait_store_open`) and fails the test if it failed, so every test can click at once. To look at the startup screen instead, make an `OpenGate` **before** `reset_app` (the open starts in AppState's constructor), then call `reset_app(h, "", false)`. The open then waits at the gate until `open()`, and `h.app->store_ready()` stays false. The `startup-screen` test seeds an old-layout library file through `h.seed_db` to get the upgrade's screen.
- Click a song-panel tab only once the click's analysis has settled or a closed `ViewGate` holds it (`wait_tabs_placed`). The headline above the tabs (`render_headline`) grows when the record lands and pushes them down, so a click whose mouse is still travelling misses. `open_details` and `wait_song_analyzed` already wait for it.
- `wait_until` yields one extra frame after its condition holds, so `visible_text` reflects it.
- `wait_until` returns false at once when the test has already failed (`ctx->IsError()`). A click that found no item no longer sits out the whole timeout.
- An `IM_CHECK` inside a helper only returns from the helper. Check `ctx->IsError()` after calling one.
- The Dynamics tab's counts come from the click's own analysis; nothing is read from the database. So wait for the click (`wait_song_analyzed`, or `wait-idle` in a script) before reading the tab.

## Layout

- `tests/ui/uitest_harness.{h,cpp}` — WARP device, offscreen target, engine setup, scratch files, seams, `wait_until`, `dump_*`, `screenshot`, and the shared helpers every area file calls.
- `tests/ui/uitest_commands.{h,cpp}` — the command interpreter: what one line does (`run_command`), shared by command files and live mode.
- `tests/ui/uitest_script.cpp` — the command-file test, and `command_line`, which decides which lines are commands.
- `tests/ui/uitest_live.cpp` and `tests/ui/live_protocol.h` — the live channel: the pipe, the loop that keeps the app drawing between commands, and the JSON lines.
- `tools/hydra_ui_mcp.py` — the MCP server; `tools/test_hydra_ui_mcp.py` tests it, and `tools/fake_hydra_live.py` is its stand-in Hydra.
- `tests/ui/uitest_tests.cpp` — `register_tests`.
- `tests/ui/uitest_library.cpp`, `uitest_details.cpp`, `uitest_preview.cpp`, `uitest_batch_reports.cpp` — the checked-in tests by area, each with its entry table.
- `tests/ui/uitest_paths.cpp` — the Paths tab tests.
- `tests/ui/uitest_report_windows.cpp` — the report windows' tests; their sample rows are in `tests/report_samples.{h,cpp}`.
- `tests/ui/uitest_main.cpp` — the CLI and the one-process-per-test runner.
- `src/ui/app_shell.{h,cpp}` — `run_frame` and Hydra's window placements, shared by `Hydra.exe` and the runner. `src/ui/imgui_setup.{h,cpp}` — `setup_imgui` and the `FrameText` capture that `run_frame` calls around its draw (`begin_frame_text_capture` / `end_frame_text_capture`); `FrameText` holds every string ImGui drew, which is what `text`/`wait-text` read. Callers of `setup_imgui` set `ImGuiSetupOptions::before_ini_load` to `install_placement_handler` (app_shell.h).
