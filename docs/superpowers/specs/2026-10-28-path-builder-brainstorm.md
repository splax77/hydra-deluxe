# Path Builder: brainstorm report (2026-10-28)

Status: research record. The decisions and the design now live in
`2026-10-28-path-builder-design.md` beside this file, which wins wherever the
two differ. Keep this file for the scouts' findings and sources.

## The short version

Hydra answers one question: what is the best path? Players also ask two others.
"What is my path worth?" and "Why is the best path better than mine?"

A Path Builder would be a second program, shipped in the same installer. You
open a chart, see the best path on the Preview highway, and click fills to
build your own path. The score updates as you click. The program shows how many
points each of your choices costs against the best path, and it can finish your
path for you from any point. For any activation on the best path, you can ask
"why here?" and see the best path that does not activate there, with the points
it loses and where in the song it loses them.

Most of the engine work already exists. Hydra can already price any path chord
by chord, and it can already price a path you name the engine's own way. Two
small new search options would unlock the "coach" features. The biggest job is
the program's screen, and the Preview can be reused for most of it.

Nobody else offers this. The scouts found no rhythm-game tool that lets a
player build a path by hand and compare it with the best one.

## What players can't do today

Hydra shows the paths its search kept: the best one, the paths within the depth
setting, and the best all-0 path. A player who wants to try their own path has
no way to do that in the app.

The engine can already do it from the command line. `hydra_replay` prices any
path you type in and lists every chord's points (tools/replay.cpp). Its
`target` mode hands your activations to the engine and gets your path back,
priced the engine's own way. But it is a developer tool. It is not built by
default (it is `EXCLUDE_FROM_ALL` in CMakeLists.txt), so it does not ship.

This has come up before. In a session on 2026-09-02 (card 2059d526), a
player's 28 activations were priced by the engine by hand. That session split
the player's lost score into 47,020 points lost to path choice and 8,040 lost to
timing. A Path Builder would make that kind of answer a click away.

## What the scouts found

Two scouts searched the web. One looked at rhythm-game path tools. The other
looked at how chess, Go, maps and planning tools explain a computed best answer.
Reddit could not be reached by either scout or by me, so player opinion from
Reddit is missing.

### Rhythm-game tools

CHOpt (github.com/GenericMadScientist/CHOpt) is the only mature path tool. It
draws the best path onto an image of the chart. It has no manual path entry, no
score comparison and no list of alternatives.

Its issue tracker shows the gap clearly. In issue 29, a player was sure their
own activation scored more than CHOpt's. The maintainer had to explain by hand
that the player's path was "a net loss of 2676 points", because Star Power
lasts a number of measures while whammy fills it by quarter notes. (I checked
that figure on the issue page itself.) A Path Builder would give that answer
automatically. Issue 5 asks to see how much Star Power each section drains, so a
player can adjust a path. Issue 9 asks to see the path live in a window instead
of saving an image. Both are still open.

CHOpt's guide to reading paths teaches the notation players already know:
`2-2(+1)-3`. Each number is how many phrases you collect before an activation,
and `(+1)` means a phrase collected during the activation. Hydra writes its
own notation per activation (`Path::pathstring` in src/core/model.cpp). Showing
the CHOpt-style count beside it would help players read the builder at once.

### How other fields explain a best answer

Chess and Go review tools are the closest match. Lichess and Chess.com score
every move by how much worse it is than the engine's best, and label it (best,
inaccuracy, mistake, blunder). A graph under the board shows where the game
swung. KaTrain, a Go review tool, marks every move with a coloured dot for points
lost and can play out the rest of the game from any move. Lichess's "learn from
your mistakes" jumps to your worst move, lets you try again, then shows the
engine's answer.

Google Maps draws the best route in bold and the alternatives as thin grey
lines, each labelled with how much longer it is. Tapping one switches to it.

Timefold, a scheduling solver, splits a schedule's score by rule. It can
subtract two schedules' breakdowns so only the rules that changed remain. That
is a direct model for "your path versus the best path, broken into notes, combo,
Star Power and squeezes".

Research on explaining plans agrees on one point: people ask "why A and not B",
not "why A". The useful answer is a contrast (Miller, arXiv 1706.07269). One
well-studied method turns the user's "why not B?" into a rule, plans again under
that rule, and shows both plans side by side (Krarup et al., arXiv 2103.15575).
That is exactly what Hydra's pinned search can do.

## What Hydra already has

Four pieces exist and are proven.

`core/replay.h` prices any path chord by chord. It owns no scoring rule of its
own; it reads the engine's tables. `hydra_replay selfcheck` proves its totals
equal the engine's for every corpus path. This is fast, because it is a single
walk of the chart, not a search. It can drive the live score as the player
clicks.

`search_target` in src/search/pather.h prices a path you name, the engine's own
way. You pin the activations, and optionally each window's end and squeeze-out.
When the path cannot happen, it says which window broke and why: the engine
cannot activate there, Star Power cannot end on the pinned node, or the
squeeze-out differs. That gives the builder honest error messages ("you have no
Star Power banked here").

`run_search` with a depth setting already returns the near-best paths. They can
be the Google-Maps-style grey alternatives.

The Preview already draws everything a builder needs to show. Each fill has a
state: hidden, offered (the path could have activated here) or taken
(`PreviewFillState` in src/app/preview_view.h). It also draws the Star Power
meter curve, the running score box and the next-activation box. The live
Preview lives in `PreviewController`, its load job, its transport and the
renderer. Those mostly depend on each other and on src/app, with a few small
helpers from the GUI. The Preview tab panel itself is tied to the song details
window, so the builder would write its own panel around the same controller.

## What the program could feel like

You open a chart, from Hydra's song panel or by itself. The highway shows the
best path. Every fill you could activate on glows as "offered".

You click a fill to activate there, and click again to skip it. The score box
updates at once. The best path stays drawn as a faint ghost, so you always see
where you differ. If you pick something impossible, the builder tells you in
plain words, from `search_target`'s reason.

At each window's end you choose the squeeze: hold the edge, squeeze a phrase in,
or squeeze one out. The builder shows the timing each choice needs, as the
Paths tab does today.

A strip under the highway plots two lines over the song: your running score and
the best path's. The gap between them shows where you fell behind. Clicking the
strip jumps the highway there.

A decision list sits beside the highway. Each of your activations shows the
points it cost against the best path that keeps your earlier choices. A coloured
dot marks it: green for free, yellow for small, red for costly. A "Finish for
me" button completes your path optimally from any point.

On the best path, each activation has a "Why here?" button. It finds the best
path that does not activate there and shows the cost. It also shows a short
score diff, by category and by section. An invented example of the wording:
"activating two measures later covers
the 16th-note run under Star Power and gains 3,100 points there, but loses the
next phrase to the cap and 1,400 points later".

Your attempts are kept as branches. You can flip between "my path", "best
path", "all-0 path" and any of the kept alternatives, each with its score. You
can copy a path as text to share it, and paste one in.

An optional practice mode works like Lichess's "learn from your mistakes". It
jumps to your costliest decision, lets you move it, then reveals the best
answer.

## Engine work this needs

The "calculator" part needs no engine change. `core/replay.h` prices clicks,
and `search_target` checks them.

The "coach" part needs one new search option: pin your first N windows and let
the engine choose freely after that. Today a pin means "these activations and
nowhere else" (`EngineOptions::target_act_ticks` in src/search/engine.h). With
a "free after this tick" option, one search gives "best finish from here". The
cost of each decision is then the best finish before it minus the best finish
after it.

The "explainer" part needs a second option: never activate at this fill. One
search per activation of the best path then answers every "why here?".

Speed looks fine. Each of these is a full search, so a chart with 20
activations needs about 20 searches to fill the decision list. A scout timed
`hydra_replay dump` on the five charts in testdata/input with the most notes
(up to 6,045 notes and 9 activations). Each full search took about 0.02
seconds, process start-up included, and the slowest of all 97 corpus charts
took 0.04 seconds. I re-ran The Faceless three times myself: 35, 22 and 20 ms.
A pinned search was no faster at that scale. So the list can fill at once on
charts like these. Charts longer than the corpus are unmeasured; the list
should still fill on a worker thread so a slow chart never freezes the window.

## How it would ship

A second executable, built from the same CMake project and installed beside
`Hydra.exe` by the same installer. It links the same core, render and audio
libraries, so it can never disagree with Hydra's numbers. The Preview pieces
would move out of the GUI library into a small library of their own that both
programs link. That move is the main refactor.

Hydra's song panel would get an "Open in Path Builder" button. It starts the
builder with the chart's file and the active profile on the command line. The
builder reads the same settings file, so the rules, the fill version and the
Pro Drums setting match. It would not write to Hydra's database. Saved paths
would be the builder's own files, probably in the JSON shape `hydra_replay`
already reads and writes (tools/replay_json.cpp).

Two other ways were possible. A browser version, with the engine compiled to
WebAssembly, would make paths shareable as links, but the Preview draws through
Direct3D 11 and would have to be rewritten. A mode inside Hydra would be cheaper,
but you asked for a standalone program.

## Three sizes to choose from

**Calculator.** Click to build a path, live score, impossible choices
explained, compare any two paths, save and share as text. No engine change.
The main work is the Preview library split and the new screen.

**Coach.** The calculator plus the decision list with costs, "Finish for me",
and the two-line score strip. One new engine option (pin a prefix, free after).

**Explainer.** The coach plus "Why here?" with the score diff, and practice
mode. A second engine option (forbid a fill), and the diff and wording work.

My recommendation is to build the Coach first. The calculator alone repeats
what `hydra_replay` does with a nicer screen. The coach's per-decision cost is
the feature chess and Go players rely on most, and it needs only one engine
option. The explainer can follow once the coach shows how fast the searches are.

## Decisions

The user's decisions and the design are in
`2026-10-28-path-builder-design.md`.

## Sources

Rhythm-game tools: CHOpt (github.com/GenericMadScientist/CHOpt), its guide
misc/How-to-read-paths.md, and issues 5, 9 and 29 in that repo.

Other fields: lichess.org/page/accuracy, KaTrain (github.com/sanderland/katrain),
Chess.com's move classification article on support.chess.com, Timefold's
"understanding the score" page on docs.timefold.ai, Miller (arXiv 1706.07269),
Krarup et al. (arXiv 2103.15575) and Chakraborti, Sreedharan and Kambhampati
(IJCAI 2020, paper 669).

Hydra code: src/core/replay.h, src/search/pather.h, src/search/engine.h,
src/app/preview_view.h, src/ui/preview_controller.h, tools/replay.cpp.
