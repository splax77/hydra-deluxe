# The live window keeps drawing while the library opens

Status: a plan, waiting for the user's yes. No code is changed yet.

## The plain version

When Claude starts a live window (`hydra_start("window")`), the window can sit blank and frozen until the library has opened. On 2026-11-11 that took about 22 to 25 seconds (the number comes from the brief for this plan, which reports WinDbg stack samples from that day; this plan did not measure it again).

The cause is one line. The live test starts by calling `reset_app`, and `reset_app` ends by joining the thread that opens the library. That join runs on the thread that draws the window. So nothing is drawn, and Windows messages are not answered, until the open ends.

The real app does not have this problem. It opens the library on a worker thread and draws its startup screen meanwhile. The harness added the block on purpose, so a test "can click at once".

The fix is to give live sessions a second way to wait: keep drawing frames until the library is open, then carry on. The hello line (the first line Hydra sends the helper) still goes out only once the library is open, so every command still finds an open library, as today. Checked-in headless tests and command files keep the old block, unchanged.

A test forces the slow open on purpose. A gate holds the open on its worker thread and lets go only after a frame with the startup words has been drawn. Code that blocks never draws that frame, so the test fails on today's code and passes after the fix.

## What the user will see

This is test tooling. No shipped screen changes. The person affected is someone watching a live window start, or watching `hydra_reset`.

Before, on a slow open: the window appears blank, shows nothing, and Windows may mark it "Not responding". After 22 to 25 seconds (the 11-11 figure above) the library appears all at once.

After, on a slow open: the window shows what the real Hydra shows at startup. It is empty for 0.15 seconds (`kViewProgressDelaySeconds` in `src/ui/app_state.h`), then reads `Opening your library...`. An upgrade of an old library file shows its box, its step line and its bar. The window can be moved and closed during this. Then the library appears.

On a fast open, such as the 0.7 seconds the main session saw on 2026-11-12 (reported in the brief, not measured here), nothing looks different.

For Claude, `hydra_start` and `hydra_reset` return at the same moment as before: once the library is open. One reply is new. A live `reset` whose open outlasts the timeout in force now answers `the library was still opening after N s; use wait-idle` instead of holding the reply. The words are this plan's own, written to sit beside the existing `jobs still running after N s` of `wait-idle`; ADR 0030 puts reply wording in Hydra's C++, so they are the executor's to adjust.

## What I found in the code

**Why the block exists.** `reset_app`'s own comment says it: "A deliberate block, in the harness only: the tests click at once" (`tests/ui/uitest_harness.cpp`). Until the store is open, `run_frame` draws the startup screen instead of the main window, so there is no toolbar, no search box and no library to click. The block gives three guarantees. When `reset_app` returns, the store is open or the test has failed with the reason. No frame of the new app has been drawn yet, so a test can still set a gate before the app's first frame. And in live mode, the hello line is written after `start_session`, so the helper never sends a command to a library that is still opening.

**What would go wrong without it.** `click` and `type` would fail to find their widgets. `open` would search an empty library. `text` would return the startup screen. Two commands already cope: `state` prints `store=opening`, and `wait-idle` waits for the open, because `AppState::any_job_running` counts the store-open job.

**The real app already draws, by decision.** ADR 0026 ("What the user sees") says the open runs on a worker thread so the window keeps drawing: a fast open shows an empty window, and an open still running after 0.15 s shows the startup screen. The `startup-screen` test's comment cites that ADR for the same behaviour. This plan restores it for live sessions. `main.cpp` builds `AppState`, which starts the open on its own thread, and its loop calls `run_frame` every frame. `run_frame` draws `render_startup_screen` until `store_ready()`. Nothing under `src/` calls `wait_store_open`; only tests do. So the comment in `src/ui/library_jobs.h` is true.

**Why a block in a test freezes the window.** The test engine runs a test function on a helper thread that never runs at the same time as the main thread. The main thread waits until the test function yields. A join inside the test function therefore stops the main loop: no frame, no message pump.

**Past decisions.** `docs/sessions/index.md` has no line holding "wait_store_open", "live window" or "hello". `docs/agents/rulings/README.md` names no ruling on the live channel. No ADR names the harness block; the nearby ones are ADR 0026 (the window keeps drawing while the store opens, above) and ADR 0030. ADR 0030 says Hydra's C++ side owns every command rule, including the wording of every reply and the timeout in force, and the helper only carries messages. This plan follows it: the wait and the new reply live in C++, and the helper is not changed.

**Prior art.** The test engine's own wiki ("Setting Up") describes the same threading: the test coroutine and the main thread "will NEVER run in parallel", and control returns to the main thread only at a yield. I found no page about waiting on a background load from a test function. The pattern used here, poll the state and yield a frame between checks, is what Hydra's own `wait_until` already does. Sources: https://github.com/ocornut/imgui_test_engine/wiki/Setting-Up and https://github.com/ocornut/imgui_test_engine/wiki/Automation-API.

## The design

`reset_app` gets split in two at its last step. Building the fresh app stays as it is. What follows is one of two waits, and both end in one shared function that reports a failed open with the same words as today.

The blocking wait is today's. Checked-in tests, command files and the non-live `reset` verb keep it.

The drawing wait is new. It yields frames until the store is open, the open has failed, the cap runs out, or the engine is stopping (the user closed the window). Live sessions use it in two places: the start of `run_live`, and the `reset` verb when the session is live. It works the same in headless live mode and in the window, so a headless test can prove it.

The caps are existing numbers. At session start the cap is `testwait::kUitestProcessCap` (5 minutes, `tests/wait_util.h`), the same cap `run_live` already gives the wait for a client. For a live `reset` the cap is the timeout in force, the same rule `open` and `wait-idle` follow, so no live command outlasts it.

To force a slow open, `OpenGate` learns a second way to hold. Today it holds only at the conversion step, which only an old library file reaches. The new form holds at the library-read step, which every open reaches, and lets go once the drawing wait has seen a frame with the startup words on it. A dev-only environment variable, `HYDRA_UITEST_HOLD_OPEN_S`, sets that gate for a live session and adds a minimum hold in seconds, so the real window can be photographed mid-open. The seconds come from whoever sets the variable; the code holds no new number. The gate's own wait is today's `kWaitCap` (30 seconds, `tests/wait_util.h`) added on top of the minimum hold, so a hold of 40 seconds is not cut off by the 30-second cap; a gate with no hold keeps the cap as it is.

## Global constraints

No file under `src/` changes. The helper `tools/hydra_ui_mcp.py` does not change. The wire format of the hello line and the replies does not change. Tests run on `testdata/input` with a scratch database; nothing here opens or copies the user's library.

## Tasks

T1 and T2 touch the same files, so one executor does them in order on one branch. T3 follows on the same branch.

### T1: A drawing wait for live sessions

Owns `tests/ui/uitest_harness.h`, `tests/ui/uitest_harness.cpp`, `tests/ui/uitest_engine.h`, `tests/ui/uitest_commands.h`, `tests/ui/uitest_commands.cpp` and `tests/ui/uitest_live.cpp`.

First commit, tests only, so they can run red. `OpenGate` gains the hold described above. Three new tests go in `command_tests()` in `uitest_live.cpp`. Each drives the live `reset` verb through `run_command`, which exists today, so the tests compile before the fix.

Second commit, the fix. `EngineHooks` gains a hook for a reset that draws while it waits. `start_session` takes a flag that picks it. `run_live` passes the flag, and the `reset` verb passes `session.live`. Both waits call one function that reports a failed open. The comment on `reset_app` in the header is updated to point at the two waits.

Acceptance criteria:

1. `live-reset-draws-while-opening`: with the new gate holding the open, a live `reset` returns ok, and `state` then holds `store=ready`. The startup words come from `StoreOpenProgress::label()`, never a second literal. Red on today's code: the block draws no frame, so the gate never lets go. Its own wait gives up after 30 seconds, the open fails, and `reset` reports `The scratch library did not open` (premise P9), so the red run takes about 30 seconds and fails cleanly. The brief asks the main session to run this test red after the first commit.
2. `live-reset-open-outlasts-timeout`: with a gate that never lets go and `timeout 0.4`, a live `reset` fails with exactly `the library was still opening after 0.4 s; use wait-idle`. `state` holds `store=opening`. The test then opens the gate, `wait-idle` answers ok, and `state` holds `store=ready`.
3. `live-reset-open-failed`: a live `reset` whose open fails is answered as a failure whose text holds `The scratch library did not open`. The same test runs the non-live `reset` on the same broken library and checks the engine's log holds the same sentence, so one function owns it. To break the open, the test sets `h.seed_db` to a file of junk bytes, as `startup-screen` sets it to its seed (premise P10), and puts it back after. It writes the file itself, or includes `../db_file_util.h` for `write_junk_db`, the way `uitest_library.cpp` includes `../db_file_sql.h`.
4. `reset-app-blocks`: after a plain `reset_app(h)`, `store_ready()` is true and `ImGui::GetFrameCount()` is what it was before the call. This pins the headless guarantee.
5. The session-start cap: a unit of the drawing wait takes its cap as an argument. The test passes 0.4 seconds with a gate that never lets go and checks the error holds `the library did not open within`. `run_live` passes `kUitestProcessCap`, which the reviewer reads.
6. The existing tests `startup-screen`, `command-open`, `command-modal-named` and the command-file test still pass. The main session runs them with `hydra_uitest --test <name>`.

### T2: A way to force a slow open from outside

Owns `tests/ui/uitest_live.cpp` (the part that reads the variable) and `tools/test_hydra_ui_mcp.py`.

`run_live` reads `HYDRA_UITEST_HOLD_OPEN_S` before it starts the session. A value that is not a plain number of seconds, by `hydra::parse_finite_number`'s rule, ends the run with an error naming the value. A good value sets T1's gate with that minimum hold.

Acceptance criteria:

1. In `RealExeLayer`, a test starts the real headless `hydra_uitest --live` with the variable at 2 seconds. The hello line arrives no sooner than 2 seconds after launch, and the first `state` reply holds `store=ready`. A start that blocks never draws the frame the gate waits for, so no hello arrives and this test fails.
2. A second test sets the variable to `abc`. Hydra exits with a nonzero code and its output names `abc`.
3. With the variable unset, the existing `test_the_real_hello_line` still passes.

The main session runs these, because `RealExeLayer` needs the built exe.

### T3: The docs say what the code now does

Owns `docs/agents/ui-testing.md`.

Four places change. The rule of thumb about `reset_app` says live sessions wait by drawing. "The hello line" says it is written once the library is open, and that the window shows the startup screen until then. The `reset` row of the verb table names the new reply. A short paragraph documents `HYDRA_UITEST_HOLD_OPEN_S` as dev-only.

Acceptance criteria:

1. `hydra_tests.exe "-tc=docs match code*"` passes; the main session runs it.
2. The reviewer reads each changed sentence against T1's and T2's code.

## The live steps

These run after T1 to T3 are built by the main session in the branch's worktree. They need the user's go, like any live step. They use the scratch library on `testdata/input`: `Hydra.exe --live` with no `--db` starts from an empty scratch database (premise P11). PowerShell's own cmdlets (`Start-Process`, `Get-Process`, `Stop-Process`, `Start-Sleep`) are not listed as tool calls. Each step is capped at 2 minutes and one retry; past that, stop and report the log file.

### LS1: The branch's live window starts with the hold set

Calls: `Hydra.exe --live --pipe <name> --uitest-log <file>` with `HYDRA_UITEST_HOLD_OPEN_S=20` (never tried)

Pass: within 10 seconds the process has a main window and `Responding` is true. After 20 seconds the log shows no error. Then stop the process. Uses P11.

### LS2: The held window shows the startup words

Calls: `Hydra.exe --live --pipe <name> --uitest-log <file>` with `HYDRA_UITEST_HOLD_OPEN_S=20` (seen: LS1's passing run), `Save-WindowShot` from `tools/capture_window.ps1` (never tried)

Pass: a shot taken 5 seconds after launch reads `Opening your library...`. The main session reads the PNG and shows it to the user. Then stop the process.

## Out of scope, for the user to approve

`Hydra.exe --uitest <test or script>` (attached mode, a test watched at human speed) keeps the block. It runs on the scratch library, which opens in a blink, and its tests rely on no frame being drawn inside `reset_app`.

Making `fill_saved_hardest` cheaper is a separate plan.

The helper's 60-second wait for the hello line stays as it is unless question 1 says otherwise.

## Questions

1. **Should the helper give up on a library that takes more than 60 seconds to open?** The real rule is `HELLO_TIMEOUT_S = 60.0  # how long hydra_start waits for the hello line` in `tools/hydra_ui_mcp.py`. The clock starts at launch and covers the whole open. At one end, a 0.7-second start and the 22-to-25-second start of 11-11 are both well inside it, and nothing changes. At the other end, an open that takes 61 seconds (say, a dev build that must upgrade every row of an old library file) is killed mid-open with `Hydra did not open its live channel within 60 s`, even though the window would now be showing a moving progress bar. That already happens today; this plan does not make it worse. My recommendation: leave 60 seconds alone now, because no open that long has been seen, and raise it only when one is. The alternative is a second, longer number for the open, which you would need to pick. One caution: the 22-to-25-second and 0.7-second figures come from the main session's report, and I could not check them against a run folder (premises P13a and P13b). If the real freeze is longer than reported, the margin under 60 seconds is smaller than it looks here.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| A live session keeps drawing while the store opens | The design, "The drawing wait is new" | T1 | `live-reset-draws-while-opening`, red on today's code |
| The live start, not only `reset`, draws | The design, "Live sessions use it in two places" | T1 (T2's test checks it) | RealExeLayer hold test: no hello unless a startup frame was drawn; LS2's shot |
| The hello line goes out only once the store is open | The plain version | T2 | RealExeLayer hold test: hello no sooner than the hold, first `state` holds `store=ready` |
| Headless tests keep the block, with no frame drawn inside `reset_app` | The design, "The blocking wait is today's" | T1 | `reset-app-blocks`; the existing tests in T1 criterion 6 |
| A failed open reads the same from both waits | The design, "one shared function" | T1 | `live-reset-open-failed` |
| A live `reset` never outlasts the timeout in force | The design, "The caps are existing numbers" | T1 | `live-reset-open-outlasts-timeout`, exact reply text |
| After that refusal the open still finishes and `wait-idle` waits for it | What the user will see | T1 | `live-reset-open-outlasts-timeout`, second half |
| The session start gives up at its cap with a reason | The design, "The caps are existing numbers" | T1 | criterion 5's cap test |
| A bad `HYDRA_UITEST_HOLD_OPEN_S` is refused by the strict number rule | T2 | T2 | RealExeLayer `abc` test |
| The startup words in tests come from the production label | T1 criterion 1 | T1 | reviewer reads the test for a second literal |
| No file under `src/` and no helper code changes | Global constraints | T1, T2 | `git diff --stat main` on the branch, read by the reviewer |
| The docs match the code | T3 | T3 | `docs match code*`; reviewer reads the four places |
| The real window shows the startup screen during a held open | What the user will see | LS2 | the PNG |
| Tests use `testdata/input`, never the user's library | Global constraints | T1, T2, LS1 | the tests set no `--db`; LS1's command line has none |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: `reset_app` ends in `h.app->wait_store_open()` unless `wait_store` is false | code | `tests/ui/uitest_harness.cpp:264` to line 267 | checked 2026-11-12 |
| P2: `wait_store_open` joins the open's thread, then collects it | code | `src/ui/app_state.cpp:118` and `src/ui/library_jobs.cpp:76` | checked 2026-11-12 |
| P3: `run_live` calls `start_session` before it makes the pipe and writes the hello line | code | `tests/ui/uitest_live.cpp:265` to line 287 | checked 2026-11-12 |
| P4: `start_session` calls the reset hook, which is `reset_app(*this)` | code | `tests/ui/uitest_commands.cpp:268` and `tests/ui/uitest_harness.cpp:84` | checked 2026-11-12 |
| P5: the real app draws the startup screen until the store is ready and never calls `wait_store_open` | code | `src/ui/app_shell.cpp:467`, `src/ui/main.cpp:312` to line 377; a grep of `src/` for `wait_store_open` finds only its definition and declaration | checked 2026-11-12 |
| P6: a test function and the main thread never run at once, so a block in one stops the other | code | `third_party/imgui_test_engine/imgui_te_coroutine.cpp:17` | checked 2026-11-12 |
| P7: `wait-idle` waits for the store open and `state` prints `store=opening` | code | `src/ui/app_state.cpp:611` and `tests/ui/uitest_harness.cpp:413` | checked 2026-11-12 |
| P8: every open reports the library-read step, a fresh scratch database included | code | `src/ui/library_jobs.cpp:155` | checked 2026-11-12 |
| P9: on today's code the new test fails cleanly rather than hanging: the gate's wait throws `WaitTimeout` (a `std::runtime_error`) after `kWaitCap`, `StoreOpenJob::run` keeps the error and rethrows, `run_guarded` catches it, and `reset_app` reports it | code | `tests/wait_util.h:19` and `:45`, `src/ui/library_jobs.cpp:169` to line 173, `src/app/job_base.h:101`, `tests/ui/uitest_harness.cpp:264` to line 276 | checked 2026-11-12 |
| P10: the uitest harness can be given a library whose open fails: `reset_app` copies `h.seed_db` over the scratch database, and a junk file there fails an `AppState` open with `DatabaseOpen` | code | `tests/ui/uitest_harness.cpp:233`, `tests/ui/uitest_library.cpp:1361`, `tests/test_app_library.cpp:589` to line 600, `tests/db_file_util.h:43` | checked 2026-11-12 |
| P11: `Hydra.exe --live` with no `--db` runs on a scratch database: `init_attached` calls `init_scratch`, which points the app's database, settings and rules paths at the scratch folder, and the seed is copied only when `--db` gave one | code | `tests/ui/uitest_harness.cpp:47` to line 59, `:98` to line 101, `:233`, `src/ui/main.cpp:157` to line 167 | checked 2026-11-12 |
| P12: the helper waits `HELLO_TIMEOUT_S`, 60 seconds, for the channel and the hello line together | code | `tools/hydra_ui_mcp.py:43` and line 402 | checked 2026-11-12 |
| P13a: the freeze took 22 to 25 seconds on 2026-11-11 | unchecked | the brief for `plan-live-unfreeze` reports it from WinDbg samples; no run folder was given, so the search for a source found nothing | question 1 |
| P13b: a start took 0.7 seconds on 2026-11-12 | unchecked | the brief for `plan-live-unfreeze` reports it; no run folder was given, so the search for a source found nothing | question 1 |
| P14: the startup screen is empty for `kViewProgressDelaySeconds`, 0.15 s, then reads `Opening your library...` | code | `src/ui/app_state.h:430` and `src/ui/library_jobs.cpp:30` | checked 2026-11-12 |
| P16: ADR 0026 decides that the open runs on a worker thread so the window keeps drawing, with the startup screen after 0.15 s | code | `docs/adr/0026-the-store-keeps-summaries-the-engine-gives-details.md:169` to line 172 | checked 2026-11-12 |
| P17: ADR 0030 gives Hydra's C++ the wording of every reply and the timeout in force, and leaves the helper as transport | code | `docs/adr/0030-hydra-owns-every-live-command-rule.md:17` to line 29 | checked 2026-11-12 |
| P15: the session index and the rulings index hold no decision on the live channel's start | code | `docs/sessions/index.md` (no line for "wait_store_open", "live window" or "hello") and `docs/agents/rulings/README.md` | checked 2026-11-12 |
