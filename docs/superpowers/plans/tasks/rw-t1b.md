Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task T1b: the spike, after the user's two answers

Task id: RW-T1b. Base: T1's signed-off tip, `31437ba08ce5f8a7729d769ef6fd38c7a64937ab`. Don't make a new worktree: work in T1's, `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\rw-t1-viewport`, on branch `claude/rw-t1-viewport`. Its derive-once review is finished, so nobody else is working there.

T1 asked the user two questions. The answers are D103 items 14 and 15 in `docs/audit/2026-10-03-fix-decisions.md`. Read both, and T1's report (in your prompt).

## What to do

**1. First open at the main window's rectangle (D103 item 14).** `place_report_window` (`src/ui/app_shell.cpp:388`) centres the window over the main window today. Change its first-open placement to the main window's position and size: the main viewport's rectangle in ImGui coordinates. Everything else stays: the saved rectangle still goes through `placement_on_screen`, and a failed check still falls back to first-open placement. The minimum size is measured from the content (the controls row and the first five columns), and the spike has no content. So add no minimum size; T5 does that. Its brief will say so.

**2. Popups and tooltips from the main window stay inside it (D103 item 15).** Today, with viewports on, ImGui lets every popup and tooltip reach the whole monitor. `third_party/imgui/imgui.cpp:17570-17584` sets `ViewportAllowPlatformMonitorExtend` for every window with the Tooltip or Popup flag, and `GetPopupAllowedExtentRect` (`imgui.cpp:13647`) then uses the monitor's work area instead of the host viewport's rectangle. The orchestrator found no ImGui setting that turns this off. So:

- First, check that yourself. Search `imgui.h`, `imgui_internal.h` and the docking branch's issues on GitHub for a supported way to keep popups and tooltips inside their host viewport (a config flag, a window-class field, a backend hook). If one exists, use it, and skip the next bullet.
- Otherwise, make the smallest change to our copy of `imgui.cpp` so that a tooltip or popup whose host is the main viewport keeps the old limit, the main viewport's rectangle. Mark it the way `third_party/imgui/imconfig.h` marks Hydra's changes: a comment starting `// Hydra:` that says why (D103 item 15) and what upstream does. Decide for yourself, and say in your report, whether popups opened from a report window (its column menu and its dropdowns) should stay inside that report window too. Keep the rule as simple as one condition allows.
- Every combo dropdown, context menu, modal and tooltip in T1's popup list must then stay inside the main window. That includes modals bigger than a very small main window: they get clipped or pushed inside as before viewports, never a new OS window.

**3. Tests.** Headless runs have viewports off, so no uitest can show this. Rerun T1's 15 uitest scripts by name (the command is in T1's report) to prove nothing else moved. Build `Hydra` (`-Target Hydra`) too. The main session checks the OS windows by hand.

## Owned files

`src/ui/app_shell.h`, `src/ui/app_shell.cpp`, `src/ui/main.cpp`, and the one marked block in `third_party/imgui/imgui.cpp`.

Owned-file check: both answers are met inside `place_report_window` and, failing a supported setting, one block of `imgui.cpp`; no other file needs to change.

## Preflight

Command: `Grep "ViewportAllowPlatformMonitorExtend" third_party/imgui/imgui.cpp` and `Grep "place_report_window|report_window_class|imgui_work_areas|ConfigViewports" src/ui` in T1's worktree, run by the orchestrator on 2026-10-08.
Output: `ViewportAllowPlatformMonitorExtend` is set for Tooltip and Popup windows at `imgui.cpp:17573-17584` and for other top-level windows at 17603-17608, and read by `GetPopupAllowedExtentRect` at 13651; `place_report_window` is at `app_shell.cpp:388` (declared `app_shell.h:155`); `imgui_work_areas` at `app_shell.cpp:337`; `report_window_class` at 380; `ConfigViewportsNoTaskBarIcon = true` at 243.

## Return

`complete`, `branch`, `worktree`, `tip`, `report` (say which route item 2 took, and why), `questions`, `handoff`.
