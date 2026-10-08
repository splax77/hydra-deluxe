# Report windows: design done, build not started (handoff, 2026-10-08)

The path report and the dmleaderboards comparison are moving out of the browser and into Hydra. Each opens in its own Hydra window, a real Windows window you can drag to another monitor. This session did the research, the mock, the spec and the plan. No code has changed. The next session starts building, from the plan's first wave.

## Where everything is

- **The spec:** `docs/superpowers/specs/2026-10-08-path-report-window-design.md`. It covers both windows, despite the file name.
- **The mock,** five boards: `docs/superpowers/specs/2026-10-08-path-report-window-mockup/report-window.html`. Board 1 is the path report window, board 2 its other states, board 3 the main window's changes, board 4 the comparison window, and board 5 the player picker handing over to it. Open it in a browser.
- **The plan:** `docs/superpowers/plans/2026-10-08-path-report-window.md`. It has three waves of parallel agents, a table of who owns which files, and the join.
- **The decisions:** D103 in `docs/audit/2026-10-03-fix-decisions.md`, thirteen items, all the user's.

All four were committed together with this note.

## What the user approved, and what they haven't yet

The user approved the design on the mock, boards 1 to 5, and every recommendation behind D103.

They haven't yet said "go" on the written spec. The brainstorming process asks them to review it before the plan runs. So the next session's first step is to ask the user whether the spec reads right, and to fix anything they raise before dispatching wave 1.

## How the design hangs together

The shape is short; the details are in the spec.

Dear ImGui's docking branch, which Hydra already vendors, can give one window its own OS window. ImGui calls these "viewports". Only the two report windows opt in, through one shared window class, so nothing else in Hydra can tear off. The report windows are owned by the main window, so they minimize and restore with it.

Both windows are one shared frame (`ui/report_window`) over one shared table view (`app/report_view`). Each report supplies only its columns, its filter and its tiles. The pages' JavaScript rules move into C++, each with one owner. That includes the dm page's second copy of `tally_dm_rows`.

The rows don't change. The same data code builds them, and the join checks them field for field against the old pages on `testdata/input`.

Once both pages move, the GUI never writes a report file or opens a browser again. `hydra_report` goes. So do the Documents\Hydra lookup, the two file names, `report_outcome.h` and `show_in_folder`. The fill-spawn comparison stays HTML, because only `hydra_fillcompare` makes it. That tool keeps `open_in_browser` and `write_report_file`.

## Facts this session verified, so the next one needn't

- **ImGui version:** the docking branch, `IMGUI_VERSION` "1.93.0 WIP". Viewports are off today, and a comment at `src/ui/app_shell.cpp:230` says so on purpose.
- **Viewport defaults in our copy** (`third_party/imgui/imgui.cpp:1731-1734`): NoAutoMerge false, NoTaskBarIcon false, NoDecoration true, NoDefaultParent true. The report window class has to set its parent and clear NoDecoration itself.
- **The harness copes without viewports.** With no OS backend, ImGui switches viewports off by itself (`imgui.cpp:11823-11843`). Under `hydra_uitest`, each report window is an ordinary in-app window that tests can click by name.
- **The main loop would freeze a report window.** It skips frames while the main window is minimized or covered (`src/ui/main.cpp:326-332`). That check has to change; plan task T1 does it.
- **An off-screen check already exists.** `placement_on_screen` at `src/ui/app_shell.cpp:118` is the one the main window uses, and the report windows reuse it.
- **Real numbers:** the real path page is 28 MB. Clone Hero's SP cap (`kCloneHeroSpCap`) is 4. The default hit window is 85 ms, so the Beyond edge in the mock reads 170 ms.

## Agents from this session

The progress journal listed these three as unfinished when the handoff was written:

```
agent-a0d2448852b505f2f: unfinished, last tool call SubagentHandback, touched 17:59
agent-a6295d74044e8fe9b: unfinished, last tool call SubagentHandback, touched 17:57
agent-ae5d73285b9dbee0e: unfinished, last tool call SubagentHandback, touched 17:58
```

All three were read-only scouts, and each ended by delivering its final report through that SubagentHandback call. The reports are in this session's transcript, and the spec's "What the research says" section summarises them. None edited a file, and nothing needs relaunching.

## Next steps

1. Ask the user to review the spec, and fix whatever they raise.
2. Load the workflow-authoring skill and run wave 1 as the plan describes. That's T1 (viewport spike), T2 (data layer) and T3 (table view), each in its own worktree on Opus, with a Sonnet derive-once review piped after each one.
3. Merge T2, then T3. Before merging T1, run the hands-on spike check with the user and send screenshots. If the OS window misbehaves, stop and ask the user about an off switch.
4. Then run wave 2, wave 3 and the join, as the plan says.
