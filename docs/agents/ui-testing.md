# GUI testing: drive the real UI by label, read back text

Hydra's UI is Dear ImGui. The Dear ImGui Test Engine (vendored at `third_party/imgui_test_engine`) can find a widget by its label, click it, type into it, and read what is on screen. `hydra_uitest` runs the whole UI with no window on a software GPU, so an agent can verify a GUI change in about a second and get **plain text** back — not screenshots.

Use this instead of launching `Hydra.exe` and taking screenshots. Reach for a screenshot only when something looks visually wrong.

## Build and run

```bash
.\build_cpp.ps1 -Target hydra_uitest
```

```bash
.\build-cpp\Release\hydra_uitest.exe --all --jobs 4
```

| Command | What it does |
|---|---|
| `hydra_uitest --all` | run every checked-in test, one after another |
| `hydra_uitest --all --jobs <n>` | run every test in its own process, at most n at once |
| `hydra_uitest --test scan` | run one test (repeatable) |
| `hydra_uitest --list` | list the tests |
| `hydra_uitest --script file.txt` | run a command file (see below) |
| `--keep-temp` | keep the scratch folder (DB, INI) and print its path |
| `--shots <dir>` | where `screenshot` files go (default: the scratch folder) |
| `--db <file>` | start each test from a copy of this database instead of an empty one (the file itself is never opened). A missing file is an error. Close Hydra first: a database it has open is not a clean copy |

Output is `[PASS]`/`[FAIL]` per test. A failed test prints the engine's log. The log names the check that failed (like `uitest_library.cpp:120`) and every action before it. The exit code is 0 only when everything passed. `ctest` runs it too.

**Running tests in parallel.** `--jobs <n>` starts each test as its own `hydra_uitest --test <name>` process, at most n at once. Each process has its own scratch folder and a fresh ImGui context, so no test sees another's leftovers. The results still print in the usual order. The full suite takes about 14 s at `--jobs 4`, against about 22 s one after another. `--jobs` runs named tests only, not scripts.

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
| `type <ref> \| <text>` | put text into an input |
| `wait <seconds>` | let frames run |
| `wait-idle` | wait until every background job has finished; the list is `AppState::any_job_running` |
| `wait-text <substring>` | wait until the text appears on screen |
| `expect-text <substring>` / `expect-not-text <substring>` | assert on screen text now |
| `text` | print everything on screen as text |
| `state` | print the key app state (library count, rows, selection, jobs, preview, status, and the store line: `store=opening`, `store=ready` or `store=failed`) |
| `dump [window]` | print the widget tree of a window and its child windows: label, id, rect, checked/disabled/opened/inputable |
| `screenshot <file.png>` | save the frame |
| `timeout <seconds>` | change the wait-text / wait-idle limit (default 30) |
| `#` | comment |

Lines print as they run; a failing line prints `!! failed at line N`.

Clicking a song starts its analysis (D87 item 1), and `wait-idle` waits for it. There is no `Done!` flash, so the tabs show as soon as it finishes. A tab click still needs a frame or two before its text shows.

## Refs (how to name a widget)

- A bare label is looked up in the current window: `Scan library`.
- `//Window/Label` is absolute: `//Scanning charts/Continue`, `//Analyze library/Start analyzing`.
- `**/Label` searches every child window too. Most of the screen now lives in child windows (the settings bar, the library, the song panel, the batch strips), so use it for anything below the toolbar: `**/##search`, `**/Try again`, `**/Expand all`, `**/Pause`.
- `##id` labels work as written: `**/##spcap`, `**/##DetailsTabs/Preview`, `**/##scrub`.
- `###` labels are addressed by the part after `###`: DM picker rows are `**/###<discord id>`.
- Library rows: `**/<title>`; escape `/` and `#` in the title with a backslash.
- Child window names are mangled (`Hydra/##songpanel_BBB91B00`), so a C++ test points at one with `ctx->WindowInfo("//Hydra/##songpanel").Window`, the way `set_panel_ref` does.

## Label cheat-sheet

These are the labels the merged app draws. Every GUI test finds widgets by them. The `...` is three ASCII dots.

| Area | Labels |
|---|---|
| Toolbar (window `Hydra`) | `Manage folders... (N)`, `Scan library`, `Analyze library...` or `Analyze search (N)...` while searching, `Compare with dmleaderboards...`, `Open path report` / `Building path report...` |
| Status line | a problem stays until `X##dismissstatus` |
| Settings bar, child `##settingsbar` | combo `##difficulty`, checkboxes `Pro Drums` and `2x Bass`, input `##spcap` with checkbox `1.0 fills` beside it, input `##depthvalue`, combo `##depthmode`, checkbox `Path limit##mslimit`, input `##mslimitvalue`. Only a batch locks it, and it then shows `Stop the batch to change these.` (D90 item 2) |
| Library, child `##library` | search input `##search`, clear button `X##clearsearch`, chips `All (N)##chipall`, `Not analyzed (N)##chipnew`, `Stale (N)##chipstale`, `Analyzed (N)##chipdone`, table `##librarytable` with columns `Title`, `Artist`, `Charter`, `Folder`, `Best path`. A Best path cell reads `Not analyzed`, `Stale`, or `<score>  <path>`. Under a search: `Clear search` |
| Song panel, child `##songpanel` | `Hide library` / `Show library`, `<##prevsong`, `>##nextsong`, `X##closepanel` (Escape does the same). A click's analysis shows `Analyzing chart...` with `Cancel` once it has run 0.15 s; a cancelled one shows `Analysis cancelled.` with `Try again`; a failed one shows its error with `Continue`. Tab bar `##DetailsTabs` with tabs `Paths`, `Preview`, `Dynamics`, `Stars` |
| Paths tab | path buttons `##path<i>` (0-based), `Expand all` / `Collapse all`, activation rows `##act<i>` (1-based), links `Show in Preview >##showact<i>`, folds `Backend timings##act<i>`, `Multiplier squeeze##mult`, `Score breakdown##breakdown`, button `Copy path` (flashes `Copied!`), checkbox `Hide backend rows beyond##backendlimit`, input `##backendlimitvalue` |
| Preview tab | text `Showing` beside the combo `##previewpath`, `< Act##prevact`, `Act >##nextact`, transport `-5s`, `< 5 Ticks`, `Play` / `Pause`, `5 Ticks >`, `+5s`, sliders `##volume` and `##scrub` |
| Batch confirm, popup `Analyze library` | checkbox `Also re-analyze charts that already have a result##redo`, buttons `Start analyzing` and `Cancel` |
| Running batch, child `##batchstrip` | `Pause` / `Resume`, `Stop` |
| Finished batch, child `##batchdone` | `Open report`, `X##dismissdone`, checkbox `Open automatically` |
| Path report window, `###pathreport` (title `Path report — Hydra`) | header `Refresh`; out-of-date strip `Refresh##outofdate`; left-out strip `Show files`; search `##search`, combo `##timing` (`All timing tiers` first), checkbox `Best path only`; table `##pathtable`, whose rows are clicked by their row number (`**/1`); when nothing passes, `Clear filters`; while building, `Cancel`; cancelled or failed, `Try again`. `Escape` and `Ctrl+W` close it |
| Comparison window, `###dmreport` (title `dmleaderboards: <player> — Hydra`) | the same frame, plus `Compare another player...` in the header and the failed state; combo `##status` (`All charts` first, then the statuses, like `##status/Not in your library`); table `##dmtable`. A `not in library` row has no click |
| Startup screen (window `Hydra`, drawn instead of everything above until the store is open) | an empty window until the click's progress delay (`kViewProgressDelaySeconds`) has passed. Then a slow normal open reads `Opening your library...` with no bar. An upgrade of an old library file shows a bordered box: `Updating your library file for this version of Hydra`, `This happens once. Your charts and results are kept.`, then a step line (`Copying your library...`, `Updating the results table...` on a 1.8.x file, `Finishing...`) over a bar whose overlay reads `rows done / rows total` (`12,345 / 38,009`). A time-left line follows the Song Preview's rule and words. There are no buttons |
| Other modals | `Scanning charts` (`Continue`, `Cancel`); `Song folders` (`Add folder...`, `Scan now`, `Close`); `Compare dmleaderboards user` (`Cancel` while the list loads, `Retry` and `Close` when it fails, then `##dmfilter`, player rows `###<discord id>` and `Close`; picking a player closes the box and opens the comparison window) |

When unsure, `dump Hydra` and read the labels off it. The dump cuts long labels short (the test engine keeps about 30 characters), so `Hide backend rows beyond##backendlimit` prints as `Hide backend rows beyond##backe`. Combos print with an empty label; take their `##id` from the source.

## The report windows

The path report and the dmleaderboards comparison are their own windows (ADR 0027). In the app each one is a separate OS window. `hydra_uitest` has no platform backend, so ImGui turns that off by itself and each report window draws inside the main viewport. The test engine finds windows by name across viewports, so the same refs work in both. A report window's name ends in `###pathreport` or `###dmreport`, so its title can change without moving its saved place. A C++ test points at one with `window_named("###pathreport")` and `ctx->SetRef(...)`; a command file can try `window //###pathreport`.

Their tests live in `tests/ui/uitest_report_windows.cpp`, in two groups.

The first group opens the windows on sample rows, with no AppState. The test's own GUI function (`draw_windows`) draws both windows from a `ReportWindowInput` it fills itself, and records what each button hands back. The rows are the shared samples in `tests/report_samples.{h,cpp}`, the same ones `tests/test_report.cpp` pins the tiles on. These tests are `report-window-sort`, `report-window-filters`, `report-window-states`, `report-window-keys` and `report-windows-both`.

The second group goes end to end through AppState: the toolbar, the batch strip, the dm picker and the settings bar. These are `report-window-open-path`, `report-window-dm-handover`, `report-window-row-click`, `report-window-reopen` and `report-window-out-of-date`. They use the canned dmleaderboards API above and never reach the real server.

## Watching it run: attached mode

```bash
.\build-cpp\Release\Hydra.exe --uitest scan
```

Runs the same test inside the real window at human speed, with the Test Engine's own panel showing. Accepts a test name, `all`, or a command-file path. Results and any `text`/`state`/`dump` output go to `hydra_uitest.log` next to the exe (`--uitest-log <file>` to change); the window stays open afterwards so the end state can be inspected. Attached mode also runs on the scratch library, never on the real `hydra.db` — the tests wipe their DB at start.

Attached mode exists only in dev builds (`build-cpp`). The installer builds with the `ship` preset, which leaves the GUI tests out, so an installed Hydra.exe ignores `--uitest`.

## Adding a C++ test

Tests are split by area into four files: `uitest_library.cpp`, `uitest_details.cpp`, `uitest_preview.cpp` and `uitest_batch_reports.cpp`. Each file ends with an entry table (`library_tests()`, `details_tests()`, and so on). Write the test in the file for its area, then add `{"thing", test_thing}` to that file's table. `uitest_paths.cpp` holds the Paths tab tests and registers them itself, from `register_paths_tests`. `uitest_report_windows.cpp` does the same for the report windows, from `register_report_window_tests`.

`register_tests` in `uitest_tests.cpp` walks the four tables. Its `kRunOrder` list keeps the order the tests had before the split, because one ImGui context carries tab and input state from one test to the next under `--all`. A test that isn't on the list runs after the listed ones, in its file's order, so a new test needs no line there. A renamed test must be renamed in `kRunOrder` too, or the runner stops with an error.

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
- `tests/ui/uitest_script.cpp` — the command-file interpreter.
- `tests/ui/uitest_tests.cpp` — `register_tests` and its `kRunOrder` list.
- `tests/ui/uitest_library.cpp`, `uitest_details.cpp`, `uitest_preview.cpp`, `uitest_batch_reports.cpp` — the checked-in tests by area, each with its entry table.
- `tests/ui/uitest_paths.cpp` — the Paths tab tests.
- `tests/ui/uitest_report_windows.cpp` — the report windows' tests; their sample rows are in `tests/report_samples.{h,cpp}`.
- `tests/ui/uitest_main.cpp` — the CLI and `--jobs`.
- `src/ui/app_shell.{h,cpp}` — `setup_imgui` / `run_frame`, shared by `Hydra.exe` and the runner. `run_frame` can capture every string ImGui drew (`FrameText`), which is what `text`/`wait-text` read.
