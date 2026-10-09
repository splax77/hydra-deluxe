# Reports are Hydra windows

Hydra's two GUI reports used to be web pages. After a batch, Hydra wrote the
path report to `Documents\Hydra\hydra_paths.html`, 28 MB on a real library,
and opened it in the browser. The dmleaderboards comparison wrote
`hydra_dmcompare.html` and opened it the same way.

Now each report opens in its own Hydra window. It is a real Windows window, so
it can sit on another monitor while Hydra is in use. This was user decision
D103 (2026-10-08). The spec is
docs/superpowers/specs/2026-10-08-path-report-window-design.md, and the plan
is docs/superpowers/plans/2026-10-08-path-report-window.md.

## The decision

The path report and the dmleaderboards comparison are Hydra windows. Each is
its own OS window, owned by the main window. Owned means Windows keeps it in
front of Hydra and minimizes and restores it with Hydra. Each has its own
title bar and taskbar button.

Both windows share one frame (header, strips, tiles, controls, table, footer
and states) and one table view. They differ only in their rows and columns.
The rows don't change: the same code builds them from the same analysis and
the same leaderboard join.

The HTML pages, the code that wrote and opened them, and the `hydra_report`
command-line tool are gone (D103 item 5). Its only output was the page. The
fill-spawn comparison stays a page, because only `hydra_fillcompare` makes it
and it never appears in the GUI (ADR 0016's 2026-10-08 note).

A user who upgrades keeps any old `hydra_paths.html` and `hydra_dmcompare.html`
in `Documents\Hydra`. Hydra never reads them again. It doesn't delete them
either, because deleting a user's files needs their say.

The pages' JavaScript used to work out the tiles, the filters, the search
match and the sort. Those were second copies of rules C++ also needs. The dm
page even carried a mirror of `tally_dm_rows`, with a test whose only job was
to check the two agreed. Each of those rules now has one C++ owner. The
"Report windows" section of docs/development.md points at each one.

## Why multi-viewports, not a second ImGui context

Think of an ImGui context as one painter. Multi-viewports give the same
painter more canvases. A second context is a second painter, with their own
brushes, fonts, input and saved settings, who has to be kept in step with the
first.

Hydra vendors Dear ImGui's docking branch (1.93.0 WIP). It can put any ImGui
window in its own OS window. ImGui calls each extra OS window a "viewport".
The DX11 backend says this is "STRONGLY preferred" over running a second
context in a hand-made second window
([imgui_impl_dx11.cpp](https://codebrowser.dev/imgui/imgui/backends/imgui_impl_dx11.cpp.html),
[Multi-Viewports wiki](https://github.com/ocornut/imgui/wiki/Multi-Viewports)).

One window can opt in on its own, through `ImGuiWindowClass` overrides. The
maintainer suggests exactly that in
[ocornut/imgui#3152](https://github.com/ocornut/imgui/issues/3152). So only
the report windows tear off, and the rest of Hydra stays in the main window as
before. `report_window_class` in src/ui/app_shell.h owns the opt-in, and
`setup_imgui` beside it says why viewports are on.

It also keeps the GUI tests simple. When the backend has no viewport support,
ImGui turns viewports off by itself and keeps every window inside the main
one. `hydra_uitest` runs with no OS backend, so there each report is an
ordinary in-app window. The test engine finds windows by name across
viewports, so the same references work in both setups.

## Drawing while Hydra is covered

Hydra's main loop used to skip drawing while its window was minimized or
covered. A report window on another monitor would then have frozen. Now the
loop skips a frame only when the main window is hidden and no other Hydra OS
window shows (`other_platform_window_showing` in src/ui/main.cpp). The report
windows are owned, so minimizing Hydra minimizes them too, and idle costs what
it did before. The hands-on check measured 5.7% CPU while a report window kept
drawing with Hydra covered.

## Popups stay inside their window (D103 item 15)

With viewports on, upstream ImGui lets any popup or tooltip reach the whole
monitor under the mouse. One that spills past its window becomes a borderless
OS window of its own. The T1 spike showed this. The user chose to keep every popup and tooltip inside the window
that opened it, as before viewports.

ImGui has no setting for this. GitHub issue
[ocornut/imgui#4624](https://github.com/ocornut/imgui/issues/4624) (2021)
asked how to keep some popups and menus inside the main window. The answer was
a call per window, `SetNextWindowViewport` with the main viewport's id, and
the maintainer closed the issue as completed. In 2022 a commenter asked for
Hydra's exact case: every window inside, a chosen few outside. The reply was
that there is no simple way beyond making that call for most windows. For
Hydra that would mean the call at every popup and tooltip it opens, which is
one rule written in many places. No setting was added.

So it is one marked `// Hydra:` block in `third_party/imgui/imgui.cpp`, in
`WindowSelectViewport`. It turns off the upstream lines that let a popup or
tooltip reach the whole monitor. The OS window it opened in stays its limit,
and no OS window is made for it. The block's own comment says what it
replaces. **Keep it when ImGui is updated.** After an update, search
`imgui.cpp` for `// Hydra:` and put the block back if it is gone.

One old quirk is left alone. A long help tooltip in a very narrow window is
cut off at its right edge. It did that before viewports too.

## Placement and `hydra_ui.ini` (D103 item 14)

A report window first opens at the main window's position and size. After
that, Hydra remembers where it was. Its minimum size is where the controls row
and the first five columns still fit, measured from the content.

The placement lives in `hydra_ui.ini`, with the rest of ImGui's window
settings. A report window has its own OS window, so ImGui saves its section
(`[Window][###pathreport]` or `[Window][###dmreport]`) with two extra lines,
`ViewportPos=x,y` and `ViewportId=0x...`. Its `Pos` is then relative to that
viewport. Older `hydra_ui.ini` files have no such lines, and ImGui reads them
as before.

A saved spot can be on a monitor that has since been unplugged
([ocornut/imgui#4236](https://github.com/ocornut/imgui/issues/4236)). So every
open checks the saved rectangle with `placement_on_screen`, the same check the
main window uses. A spot that is off every monitor falls back to the main
window's rectangle. `place_report_window` in src/ui/app_shell.h owns this.

## What this costs

**Mixed DPI.** On two monitors with different scaling, the report windows keep
the main window's scale. Text may look too large or too small on the second
monitor. ImGui marks its per-monitor DPI options experimental, so per-monitor
scaling stays out of scope.

**Screen readers** can't read ImGui windows. The HTML pages could be read by
one. That is a loss for anyone who used a screen reader on the reports. It is
named here so it is a decision, not an accident.

**Popups.** Turning on viewports reaches the whole app, not just the report
windows. The spike found that upstream ImGui would have torn popups and
tooltips off into their own OS windows. Hydra now carries a patch to ImGui
(above) that every ImGui update has to keep.

**No off switch.** ImHex, a large ImGui app, ships a setting to turn its extra
OS windows off, because some systems handle them badly
([ImHex v1.24.2](https://github.com/WerWolv/ImHex/releases/tag/v1.24.2)).
Hydra has none. One gets added only if a machine turns up where the OS windows
misbehave.

**No export.** The windows only show the report (D103 item 2). There is no
file to keep or send.

**Memory.** A report's rows stay in memory after its window closes, so it
reopens at once. The plan measures that cost when the windows merge.

## Rejected

**A second ImGui context** in a hand-made second window. See above.

**Keeping the HTML pages.** The pages carried the second copies of the tile,
filter, search and sort rules. They were also files the user had to open
outside Hydra, and a 28 MB one on a real library.

## The decisions made along the way (D103 items 14 to 22)

Items 1 to 13 came with the design. These came up while it was built:

14. A report window first opens at the main window's size and position, and
    its minimum size is measured from the content.
15. Tooltips, dropdowns and popups stay inside the window that opened them.
16. Text columns sort the way the Library table does.
17. An empty Posted date sinks to the bottom in both directions.
18. The count line's noun is singular when the total is one: "1 of 1 path".
19. While a batch locks the settings bar, a row of another mode selects
    nothing, and the status line says "A batch is running.".
20. "Open path report" also shows while a report is in memory or building.
21. The building subtitle and "(a batch finished at HH:MM)" are drawn, as in
    the mock.
22. A report goes out of date only when a setting it reads changes.
23. The Path column sorts by item 16's rule too, so "+" now sorts before "-".
24. With analysis off and no report in memory, the path window shows the
    toolbar's rules-error sentence.
25. The building bar reads "Analyzing n of N records", one per chart and mode.
26. With nothing left to analyze, the building bar moves with no count.

The full wording of each is in docs/audit/2026-10-03-fix-decisions.md.

## What this supersedes

ADR 0002 and ADR 0016 each carry a dated note that points here. Only the fill
page still uses ADR 0016's shared stylesheet and script. ADR 0010 carries a
note that `hydra_report` is gone.
