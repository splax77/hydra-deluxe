Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task T1: viewport spike

Task id: RW-T1. Base: the commit your prompt gives. Branch `claude/rw-t1-viewport`, worktree `.claude\worktrees\rw-t1-viewport`; make it as the preamble says.

Your spec is the plan's "Task 1: Viewport spike" section, plus the spec's "How the windows work". This brief adds only what the plan leaves to dispatch time.

## What this brief adds

**There is no examples folder.** The plan says to follow the Win32 + DX11 example in `third_party/imgui/examples`, but our vendored copy has only `third_party/imgui/backends`. Follow the upstream docking-branch `examples/example_win32_directx11/main.cpp` instead (github.com/ocornut/imgui, branch `docking`), and the backends' own comments. Our backends are already the docking versions: `imgui_impl_win32.cpp` and `imgui_impl_dx11.cpp` carry the platform-window code.

**Where the spike window is drawn.** `run_frame` in `src/ui/app_shell.cpp` draws every frame for both the real app (`main.cpp`) and the headless harness (`tests/ui/uitest_harness.cpp`, `Harness::frame`). Draw the spike window there, so both see it. It is an empty window named "Report spike" that uses `report_window_class()` and `place_report_window`. It shows only when a flag is set. Set the flag from a `--report-spike` command-line switch in `main.cpp`, and through a small setter in `app_shell.h` that the uitest calls. T5 replaces the window and the switch with the real report windows, so keep them small and say so in a comment.

**The headless harness has no platform backend.** ImGui then turns viewports off by itself (`third_party/imgui/imgui.cpp:11823-11843`), so under `hydra_uitest` the spike window is an ordinary in-app window. Don't add viewport code to the harness.

**What "every place a popup or tooltip can now become its own OS window" means.** With viewports on, any ImGui window that leaves the main window's rectangle can get its own OS window unless it merges. Find each `BeginPopup`, `BeginPopupModal`, `BeginTooltip`, `SetTooltip` and `BeginCombo` caller in `src/ui`, and say for each whether it can now tear off, and whether `io.ConfigViewportsNoAutoMerge` (keep it false) keeps it inside. A plain list in your report, grouped by file, with one sentence each.

**The ini file.** Viewports add a `Viewport` line per window to `hydra_ui.ini`. That's expected; don't change the ini path.

**The uitest.** Add one script, `report-spike`, in a new file `tests/ui/uitest_report_spike.cpp`. It sets the flag, checks the window "Report spike" exists by name, closes it and checks it's gone. Register it the way `uitest_paths.cpp` registers itself in `tests/ui/uitest_tests.cpp` (a declaration and one call), and add the file to `hydra_uitest_harness` in `CMakeLists.txt`.

**Modal scripts to rerun.** Run, by name, the existing uitest scripts that open "Song folders", "Remove folder?", "Scanning charts", "Analyze library" and "Compare dmleaderboards user". Grep `tests/ui` for those labels to find the script names. Run only those and `report-spike`.

**Stop and report, don't fix,** if a modal or tooltip misbehaves at normal window sizes once viewports are on. The ImHex-style off switch is a user question, not yours.

## Owned files

`src/ui/main.cpp`, `src/ui/app_shell.h`, `src/ui/app_shell.cpp`, the new `tests/ui/uitest_report_spike.cpp`, its two lines in `tests/ui/uitest_tests.cpp` (declaration and call) and its one line in `CMakeLists.txt`.

Owned-file check: every T1 acceptance item is met inside the files above; the popup list goes in your report, not in a file.

## Preflight

Command: `Grep "Occluded|placement_on_screen|void run_frame|NewFrame\(\)" src/ui` and `Get-ChildItem third_party/imgui -Recurse -Filter *win32*`, run by the orchestrator on base 658abcf on 2026-10-08.
Output: the occluded skip at `main.cpp:326-333` (`g_SwapChainOccluded` at line 44, set at 379); `placement_on_screen` at `app_shell.cpp:118`, used at `main.cpp:189`; `run_frame` at `app_shell.cpp:310`; the viewports-off comment at `app_shell.cpp:230-232`; frames begin at `main.cpp:355-357` and `uitest_harness.cpp:232-233`. Only `third_party/imgui/backends/imgui_impl_win32.{h,cpp}` and `imgui_impl_dx11.{h,cpp}` exist; there is no examples folder.

## Tests

The `report-spike` uitest and the modal scripts above, by name. Build `Hydra` too (`-Target Hydra`) so the real exe compiles with viewports on; the main session runs it by hand.

## Return

`complete`, `branch`, `worktree`, `tip`, `report` (including the popup list and how to launch the spike: the exe path and switch), `questions`, `handoff`.
