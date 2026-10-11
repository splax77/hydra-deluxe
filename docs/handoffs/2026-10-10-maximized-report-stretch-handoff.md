# Handoff: a maximized report window is drawn too small and stretched (2026-10-10)

**For the next session: fix this, starting from the findings below.** Nothing is changed in the code for it yet. The user has not yet said yes to building the fix, so ask first (CLAUDE.md rule 3). It is a code-only fix: the report shows the same thing, only drawn at its real size.

## What the user saw

In the path report, the first "0" of `0.0 ms` in the Hardest ms column looks heavier than the same `0.0 ms` in Early fill (ms). The user asked why that column uses different code. It doesn't.

The user's two crops are `2026-10-10-maximized-report-stretch/user-crop-hardest-ms.png` and `user-crop-early-fill.png`; `cmp.png` stacks them at 12x. Lined up glyph by glyph, only the first "0" differs. In Hardest ms it is one pixel wider, with an extra full-bright column in its left stroke. The second "0" and the "ms" are pixel-identical.

## What was ruled out

Both columns are built the same way. `path_columns` in `src/app/path_report_view.cpp` gives each one `ms_cell`, which calls `format_ms` (`"%.1f ms"`). Both go through `report_frame::cell` in `src/ui/report_window.cpp`, which right-aligns with `move_to_right_edge` and draws with `text_ellipsized`.

A temporary log in `cell` (since removed) recorded the inputs for row 9 of the real app on the user's data:

| | Text | Width | x | Font size | Baked size |
|---|---|---|---|---|---|
| Hardest ms (column 9) | `0.0 ms` | 40 px | 2,206 | 18 | 18 |
| Early fill (column 11) | `0.0 ms` | 40 px | 2,402 | 18 | 18 |

Both positions are whole pixels, and ImGui snaps text starts to whole pixels as well (`ImFont::RenderText`, `imgui_draw.cpp` around line 5836). Nobody sets `ImDrawListFlags_TextNoPixelSnap`. So Hydra hands ImGui identical text in both cells.

## The cause

When the path report is maximized, ImGui's viewport for it keeps the restored size and position. The same log read, every frame:

- ImGui viewport: `Pos=(8,31)`, `Size=2544x1401`, `FramebufferScale=1`.
- The real OS window: `IsZoomed=1`, client area `2560x1417`, client origin `(0,23)`, window rect `(-8,-8,2568,1448)`.

So every frame is drawn at 2,544 × 1,401 and Windows stretches it to 2,560 × 1,417. That is about 0.6% wider and 1.1% taller. Stretching makes Windows duplicate a pixel column about every 160 px, and a pixel row about every 88 px. A glyph that a duplicated column passes through gets a doubled stroke. In the user's screenshot it landed on the first "0" of Hardest ms. The whole maximized report is slightly soft for the same reason. In the capture `rep-0.png`, rows are 22.25 px apart, where ImGui lays them 22 px apart.

The maximize happens in `follow_report_os_window` (`src/ui/app_shell.cpp`, around line 539). It calls `::ShowWindow(hwnd, SW_MAXIMIZE)` on the report viewport's own HWND, once, after `place_report_window` queued it in `g_maximize_pending`. ImGui's own record of that window, and the viewport size its renderer uses, never catches up. Why not is the open question. Two leads, unchecked:

1. ImGui may push its window size back onto the viewport every frame. The report window is an ImGui window that owns its viewport, so its `Size` and `Pos` drive the viewport. Check whether the Win32 backend's `WM_SIZE` handling (`PlatformRequestResize`) ever reaches the window, and whether anything in Hydra (`SetNextWindowPos`/`SetNextWindowSize` in `place_report_window`, or the report frame) sets the size again after the maximize.
2. The renderer's swap chain may follow the viewport's `Size` rather than the client rectangle. If the viewport stays at 2,544 × 1,401, the swap chain does too, and DWM scales it to the client.

This came in with the "maximized report reopens on its own monitor" change (1f61e768, plus the earlier maximize-on-open work). As the column-widths handoff noted, its maximize, restore and reopen were never checked by hand in the real app.

## How to reproduce (no clicks on the user's screen)

The scripts are in `2026-10-10-maximized-report-stretch/`. They copy nothing themselves; set up a scratch folder first:

1. Copy `build-cpp\Release\Hydra.exe`, `mimalloc.dll`, `mimalloc-redirect.dll`, `assets\` and `resource\` into a scratch folder. Also copy the user's `hydra_settings.ini`, `hydra_ui.ini` and `hydra.db`; Hydra reads the ini files and db next to the exe. Copy the ini and db again before each run, because a run saves over them.
2. `report_shot.ps1 -Dir <folder> -Out <path>.png` launches it. It posts a hovered click to "Open path report" 80 px in from the client area's right edge and 22 px down (client (2480, 22) on the user's 2,560 px window; change it with `-ClickFromRight`). It stops with an error if no "Path report" window opens within 30 s, then captures every window of the process with `PrintWindow(hwnd, dc, 2)`. The main window comes out as `-0.png` or `-1.png`, and the report as the other. The user's main window is maximized at 2,560 px, so the report opens maximized.
3. `shot.ps1` and `steps.ps1` do the same for the main window. `steps.ps1` can minimize, restore and maximize it, click a row (`click<x>x<y>`, client coordinates) and press Escape. All three dot-source `tools/capture_window.ps1`, which holds the launch, capture and window-message code they share.

Notes on the scripts: `SetForegroundWindow` fails from the shell, so `CopyFromScreen` grabs whatever window is in front. Use `PrintWindow`. ImGui needs a few hover moves before a press, or a posted click on a button is lost. The scripts send 10.

A cheap check for the fix is to log the report viewport's `Size` against `GetClientRect` of its HWND, as above. A pixel check: after the fix, the two `0.0 ms` cells in a capture should be identical, and rows should sit exactly 22 px apart.

## A test that would have caught it

No GUI test can see a real OS window (`hydra_uitest` has no platform backend). A unit-level check is still possible on the placement side: after the maximize has run, the report viewport's size should equal its OS client size. If that needs a live HWND, it belongs in a small Windows-only test, or as a debug assertion in `follow_report_os_window`. Decide when designing the fix.

## Where things stand otherwise (same session)

- The column-width fix is merged to `main` at 912d43bd: usual width, then grow, decision D110. It has not been pushed. `build-cpp\Release\Hydra.exe` was rebuilt from 912d43bd at 17:08. The column-widths handoff (`2026-10-10-column-widths-ui-regression-handoff.md`) has the diagnosis.
- `main` holds 1ece5792, a commit from another session (b5e8d95a) that lists 339f9df9 as a second parent by accident. A merge refused by the review hook left `MERGE_HEAD` in the main checkout, and that session's next commit became a merge. Its files are right. It was repaired with an explicit-base merge (4b5c91a5, reviewed CLEAN). Nothing more is needed, but don't be surprised by it in `git log`. The memory note `refused-merge-abort` has the lesson.
- The `cwf-join` worktree's own build of `Hydra.exe` (`.claude\worktrees\cwf-join\build-cpp\Release\Hydra.exe`) was built with the temporary debug log. The source is restored, but rebuild before using that exe for anything. The worktrees cw-t1 to cw-t7, cw-w2, cwf-rule, cwf-lib and cwf-join can be cleaned up.
- Journal state at handoff time: subagents active in the last 30 minutes and not finished: agent-a48c88f3f95cf359e (unfinished, last tool call SubagentHandback, touched 17:05) and agent-a95c49d4b8c6e33a7 (unfinished, last tool call SubagentHandback, touched 16:57). Both are the two reviewers. Each submitted its review and handed back its report; the journal still lists them as unfinished.
