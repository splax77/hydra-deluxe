# R4h: stopped. A Path Builder script cannot open a chart.

No script.txt written, because nothing in the script could load a chart.

## What the runner takes (read, not guessed)
- `tests/pb_ui/pbtest_main.cpp` usage line: `hydra_pathbuilder_uitest (--test <name> | --script <file> | --list) [--keep-temp] [--shots <dir>]`. Argument loop (lines ~41-47) knows only --list, --keep-temp, --test, --script, --shots; anything else prints usage and exits 2. No chart flag, no database flag.
- `tests/pb_ui/pbtest_harness.cpp` Harness::queue (line 67): a path to an existing file is queued as the shared "script" test (`uitest::register_script_test`, tests/ui/uitest_script.cpp). So --script itself works.
- The verbs are the shared interpreter `run_command` in `tests/ui/uitest_commands.cpp` (line 279): window, click, check, uncheck, type, open, key, wait, timeout, wait-idle, wait-text, expect-text, expect-not-text, text, dump, rect, state, screenshot, reset, quit; anything else is "unknown verb". Same 20 as hydra_uitest.
- Verbs that mean something different here: `state` prints the PB hook (harness.cpp line 31): "chart: <path or (none)>", "windows: N", "error: ...". `reset` calls reset_app, which makes a fresh PbApp and opens `h.chart_arg` only if set.

## Why a script cannot open a chart
1. `chart_arg` is set only inside test bodies (pbtest_window.cpp:70, pbtest_files.cpp:28 and others: `h.chart_arg = corpus::burnout_notes(); reset_app(h);`). Neither main nor any flag sets it; grep for getenv in the runner found nothing.
2. The `open <title>` verb needs `hooks.open_song`. It is set only in tests/ui/uitest_harness.cpp:92 (Hydra's runner), not in the PB Harness constructor. In the PB runner `open` answers "open has no songs to open in this program".
3. The only in-app route is File > Open (`PbApp::act_on_menu`, pb_window.cpp:315), which calls `ask_for_chart` (src/pathbuilder/pb_open.cpp:28), a native Windows file dialog. A headless script cannot drive it. Open path... and Save path use native dialogs the same way (pb_open.cpp:37), so WN7 4's save through the UI is unreachable too.

So every record on a chart (WN7 4, A49 5, A38 1-13) is blocked. A script could only show the empty app: verbs run, `state` prints "chart: (none)". That shows A49 26 only in part (the --script and --test flags, and the usage exit 2 for any other flag, which needs a shell run with no script).

## Other findings
- A19 PL13 (tests/test_path_list.cpp:362) tests `PathsTabUi`, which lives in src/ui/paths_tab.cpp and src/app/path_view.cpp, Hydra's Paths tab, not the Path Builder window. It belongs in an hydra_uitest run on the library's Paths tab. It is misfiled under R4h (CLEAN.md line 246).
- WN7 4 test body (test_pb_files.cpp:84) calls `open_path_file` on a file the test writes itself in a temp dir; the save path (`save_path_file`) writes where a dialog says. Nothing in the app writes a file unprompted, so no repo or library write risk, but also no script route.
- A38: the audit records A38 1-13 carry no record text beyond "run_command (no test case)" (CLEAN.md 507-519); I found no per-verb mapping. A38 in CL2.md line 70 lists "all 12 verbs" for hydra_uitest. The interpreter has 20 verbs.
- Charts exist in testdata/input: Burnout `common/Summer Blast _25 Setlist/Tier 4/Green Day - Burnout/notes.mid`, White Washed `common/IB24/T5/August Burns Red - White Washed/notes.mid`, Set the World on Fire `common/IB24/T5/Symphony X - Set the World on Fire/notes.chart`.
- hydra_pathbuilder_uitest.exe exists only in the main checkout's build-cpp/Release, not in this worktree's.

## What would unblock it (for the main session to choose; I propose no code)
A way to give the PB runner a chart: a runner flag that sets `chart_arg`, or an `open_song` hook in the PB harness. Without that, run the PB tests by name (`--test`) instead; they already open Burnout and White Washed.
