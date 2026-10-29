# Path Builder (Coach): design

Date: 2026-10-28. Status: design approved section by section in chat; waiting
for the user to read this file before a plan is written.

The research behind it (web scouts, prior art, timing) is in
`2026-10-28-path-builder-brainstorm.md` beside this file. Every decision lives
here; the brainstorm report keeps only the research.

## What it is

The Path Builder is a second program shipped with Hydra Deluxe. You open a
chart, build your own Star Power path by clicking fills, and see what each of
your choices costs against the best path. It answers two questions Hydra does
not: "what is my path worth?" and "where did I lose points against the best?"

This first version is the "Coach" size (user decision, 2026-10-28). It has
click-to-build, a live score, a cost for every choice, "Finish for me", and a
strip that shows where the points went.

These are left out on purpose, for a later version: the "Why here?" button
(the best path that avoids one activation), the score diff by category,
practice mode, a browser version, and a CHOpt-style phrase-count notation.

## User decisions (2026-10-28, except where dated)

1. Size of the first version: the Coach.
2. Opening a chart: both from Hydra's song panel and on its own.
3. Cost colours: green at exactly 0 points lost, yellow above 0 and under 1%
   of the best path's score, red at 1% or more. The 1% is of Hydra's best
   path total for the chart under the active settings.
4. Path text: Hydra's own notation only (`Path::pathstring`), so text copies
   between the two programs unchanged.
5. Name: Path Builder. Window title "Hydra Deluxe Path Builder", file
   `HydraPathBuilder.exe`.
6. "Optimal, flag tight" (2026-10-29, replacing the 2026-10-28 wording).
   Costs are measured against Hydra's optimal path. A window that needs
   tighter timing than the Path limit keeps its cost colour and gets a
   violet note naming the timing it needs and the limit (see "Costs"). The
   user turned down a within-limit baseline (a search with the limit made
   hard) and a switch between two baselines.
7. Fills can be clicked on the 3D highway and on the flat strip below it.

## How it is built and shipped

`HydraPathBuilder.exe` is a second executable in the same CMake project. The
Windows installer puts it beside `Hydra.exe`. It links the same engine, render
and audio libraries as Hydra, so it scores every path exactly as Hydra does.

The live Preview moves out of `hydra_ui` into a library of its own that both
programs link. That covers `PreviewController`, `PreviewLoadJob` and
`PreviewTransport` in src/ui. The load job borrows `time_left_text` from
src/ui/library_parts.h, which pulls in Hydra's whole `AppState`, so that
helper moves to a neutral home first. Hydra must look and behave the same
after the move. The Preview tab panel (src/ui/preview_tab.cpp) stays in Hydra;
the builder writes its own panel around the same controller.

The builder reads Hydra's settings file through the same owner Hydra reads it
with (src/app/config.h). So the rules, the fill version, Pro Drums, 2x kick
and the Path limit always match Hydra's. On opening a chart it analyzes it
itself with `analyze_chart` (src/search/pather.h), which gives the best path,
the near-best paths and the best all-0 path. It neither reads nor writes
Hydra's database.

Opening a chart:
- Hydra's song panel gets an "Open in Path Builder" button. It starts the
  builder with that chart's notes path on the command line.
- The builder has File > Open and accepts a chart dragged onto its window.

Saving: the builder's own path files use the JSON shape `hydra_replay`
already reads and writes (tools/replay_json.cpp). Copy and paste use Hydra's
path text.

## The engine change

The engine gets one new option on `EngineOptions` (src/search/engine.h):
"free after". Today a pinned search (`target_act_ticks`) follows the caller's
activations and declines every other fill. The rule sits at the fill branch
point in src/search/engine.cpp. With "free after" set, that rule applies only
to fills at or before the last pinned activation. Past it, every fill
branches as in a normal search. The pinned windows' ends
(`target_deact_ticks`) and squeeze-out checks work as they do today.

Two rules for the free part:
- It runs under the same settings as Hydra's main search, the Path limit
  included. With nothing pinned, the result is Hydra's best path exactly.
- The Path limit judges only the windows the search chose. A pinned window is
  the player's choice, so the limit never drops it.

On top of the option sits one function, called "best finish" here. Given the
player's first k windows, it returns the best path that starts with them, and
that path's total. The plan names the function and places it in
src/search/pather.h beside `search_target`. When the first k windows cannot
happen, it returns `search_target`'s existing failure: which window broke and
whether the activation, the window's end or the squeeze-out was the reason.

## Costs

Write BF(k) for the best-finish total with the player's first k windows
pinned. BF(0) is Hydra's best path total.

- The cost of the player's window k is BF(k-1) minus BF(k).
- One last line, "after your last window", is BF(n) minus the player's own
  total. It catches fills the player could still have used.

These costs add up exactly to the gap between the best path's total and the
player's total. So the decision list, the strip and the score box can never
disagree. Every cost is read from these BF values; nothing works one out a
second way.

Colours (decision 3): green at exactly 0, yellow above 0 and under 1% of
BF(0), red at 1% of BF(0) or more. A cost exactly at 1% is red.

BF(0) is the optimal path, whatever the Path limit says. Hydra's best path is
the top of the sorted search result (`HydraRecord::best_path`), and the Path
limit is a soft filter: it drops an over-limit path only when a same-group
path inside the limit, or one tying the optimal score, scores more. The
Path limit tooltip says the same: a path that ties the best score is kept
whatever its timing. Nothing computes a "best path within the limit", and
the best finish runs under the same options as the main search, so BF(0) is
that same optimal path.

So a cost is never below zero (decision 6, "Optimal, flag tight"). A pinned
search can only choose among the paths the free search could, so BF(k) never
beats BF(k-1). There is no gain and no fourth cost colour.

A window that needs tighter timing than the Path limit keeps its cost colour
and gets a violet note (`chip_color(ChipToken::t5)`) naming the timing it
needs and the limit, for example "needs 130 ms, your limit is 10 ms". The
note can sit on any window shown in the builder: the player's own, and the
windows of Hydra's best path when it is the path on screen. Whether a window
is over the limit is `Activation::within_ms_limit`, the same pieces the
search's limit check asks; the timing named is `Activation::difficulty`,
the figure the Paths tab's path button shows.

When the player clicks a fill to start a new window, the window gets the end
that gives the best finish: the builder pins only the new activation, lets
the search choose the end and the rest, and keeps the end it chose. The
player can change it under "end choices".

"Finish for me" replaces everything after the player's last window with the
path behind BF(n).

Speed: a scout timed `hydra_replay dump` on the five test charts with the most
notes. Each full search took about 0.02 seconds, process start-up included.
The slowest of all 97 test charts took 0.04 seconds. A 20-window path needs
about 21 searches, under a second. They run on a worker thread anyway, with
the list filling as answers arrive, so a long chart never freezes the window.
Charts longer than the test set are unmeasured.

## The screen

The highway sits in the middle and reuses Hydra's Preview renderer and audio.
Fills show their state as they do now: hidden, offered or taken
(`PreviewFillState`). The best path's activations are drawn as faint outlines,
so the player sees where the two paths differ. Clicking an offered fill on
the highway starts a window there; clicking a taken one removes it. This needs
new renderer code that turns a mouse position into a chart time.

Below the highway is the strip. It marks every fill, and clicking a marker
does what clicking the fill on the highway does. It also draws a step line
like a chess eval graph. The line starts at BF(0) on the left, steps down at
each window by that window's cost, and ends at the player's total on the
right. It does not plot two running scores, because the best path often
banks Star Power early and scores later, so running totals mislead mid-song.

The decision list sits on the right. Each window has one row: its notation,
its colour, its cost in points and the timing it needs. Clicking a row moves
the highway there. Selecting a window shows its end choices underneath: a
plain end, a squeeze-in or a squeeze-out, each with its timing and the best
finish it leads to.

The path list sits on the left: Hydra's best path, the best all-0 path, the
near-best paths Hydra's search keeps under the active depth setting, and the
player's saved branches. Each shows its total and its gap from the best.
Picking one shows it; "Edit a copy" makes it the player's to change.

Undo and redo cover every change to the player's path.

## Errors

Every error line comes from the shared owners, `plain_error` and
`plain_error_detail`, as everywhere in Hydra.

- An impossible click shows `search_target`'s reason in plain words, for
  example that no Star Power is banked there, or that Star Power cannot end on
  that note. The click changes nothing.
- A chart that fails to load shows the same message Hydra shows for it.
- A chart with no Star Power phrases still opens and plays. The builder says
  there is nothing to build.
- A pasted path that does not fit the chart is refused with the reason, and
  the player's path stays as it was.

## Testing

Engine tests run on the test charts in testdata/input. They check three
identities, each against numbers the engine produces:

1. With nothing pinned, the best finish equals Hydra's best path total on
   every test chart.
2. With every window of a path pinned, the best finish with no further
   activations equals what `search_target` and the replay in core/replay.h
   say the path scores.
3. For a few hand-built paths, the costs add up exactly to the gap. The
   expected numbers are pinned from one run.

No cost is below zero, on any test chart; the cost tests assert it. The
corpus has charts whose optimal path needs a squeeze tighter than 10 ms (a
scout counted 10 of 97, August Burns Red "White Washed" among them), so a
test pins the tight-window note on one of them under a 10 ms limit, with
figures from the executor's own run.

The Preview move is checked by Hydra's existing Preview tests and UI scripts:
Hydra must not change.

The builder gets a headless test runner like `hydra_uitest`, so its screen
can be driven by label and read back as text. Before it merges, the user sees
it in the real app on their own library (the standing rule for layout
changes).

## Docs and installer

The User Guide gets a Path Builder section, and the README gets a line. Both
go through the doc review. The installer script adds `HydraPathBuilder.exe`.

## How the work splits

Four pieces touch different files and can start at the same time:

- The free-after engine option, the best-finish function and the cost list,
  with their tests.
- Moving the Preview into its own library.
- The renderer code that turns a mouse position into a chart time.
- Hydra's "Open in Path Builder" button and the installer entry.

Two pieces wait. The builder's main window (opening a chart, the path list,
copy and paste, save and open, undo) waits on the Preview move. The coach
features (the decision list, end choices, the strip and "Finish for me") wait
on the engine option and the main window.

## Open items

- Charts longer than the test set are untimed. The worker thread covers a
  slow one, but no number backs it yet.
- The tight-window note's colour (violet, the user's Q1 answer) is seen with
  the screen at the real-app check.
- A negative Path limit ("lower or negative values demand more slack") can
  fail a squeeze that needs no timing, which `Activation::difficulty` leaves
  out. On such a window the note has a flag but no figure to name. The plan
  sends that case to the user; nothing is invented for it.
