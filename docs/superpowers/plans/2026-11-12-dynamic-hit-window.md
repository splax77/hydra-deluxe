# Dynamic Hit Window row on the Paths tab

Date: 2026-11-12. Session 375e58cd. Status: waiting for the user's yes. No work starts before it (agent rule 3).

## The plain version

Hydra judges every note with one fixed hit window, 85 ms a side. Clone Hero does not. It gives each note its own window, from 48.15 to 85.72 ms a side, sized by how close the notes before and after it are.

This plan adds one row to the Paths tab that shows the game's own windows for the notes a timing involves. The row appears for each of the three timing badge types: squeeze in, squeeze out and early fill. It ends in a tag that says how much room the game's windows leave ("24.1 ms spare") or how far they fall short ("65.9 ms short").

Nothing else changes. The user ruled "just show it alongside." So the badges, the Rating words, the eff. figures, the timing tiers and the search all keep the fixed 85 (`kHitWindowMs`). The user also ruled "hydra searching for squeezes up to 500 ms is an intentional deviation from the game, not a bug.", so the search still finds squeezes the game's windows cannot cover. The new row is how the user sees that such a squeeze is out of reach.

The window rule is worked out in one place. Today it lives inside the `hydra_replay schedule` tool. This plan moves it into `src/core`, and the tool and the Paths tab both call it there.

## What the user decided

These four quotes are in `docs/agents/rulings/ui-and-display.md` and `docs/agents/rulings/clone-hero.md`, top lines, session 375e58cd.

"just show it alongside. try again with a maximum of 8 words for the new info." The row adds the game's numbers and replaces nothing. This is a deliberate exception to D270 call 8, which rules one window number everywhere.

"change Clone Hero to Dynamic Hit Window and mock it up.", "add ms to the numbers" and "spare should be more clearly distinguished so it doesnt look the same as the other two". These set the row's label, its units and the separate tag.

"ok well i am ruling it now: hydra searching for squeezes up to 500 ms is an intentional deviation from the game, not a bug." The search does not change.

"go ahead and plan this for all three badge types: squeeze in, squeeze out, early fill". That is this plan's scope.

The squeeze-out design is the one the user clicked through in `docs/superpowers/specs/2026-11-12-dynamic-hit-window-mockup/index.html`. The row sits at the bottom of the squeeze box, under a thin divider, in the figure font. It reads `Dynamic Hit Window:  85.7 ms early   79.4 ms late`. A separate outlined tag sits at the right: yellow `24.1 ms spare`, or orange `65.9 ms short` when the windows cannot cover the squeeze.

## The game's rule

A note's window comes from its two neighbours. A neighbour is the nearest note at another tick, so the gems of one chord are never each other's neighbours, and a chord's window is set by the chords either side of it (TM-51).

For each side, the game halves the gap to the neighbour and holds the result between 37.5 and 85 ms. A missing neighbour gives 85 (TM-18, TM-19). It adds the two halves into one number x, from 75 to 170 ms. The formula turns x into a whole window: (x x 0.0110924369747899 - x^2 x 2.628539e-05) x 170 - 20, in ms (TM-21). The note gets half of that on each side (TM-20). So a side runs from 48.15 ms, when both neighbours are close, to 85.72 ms, when both are far (TM-22). All five blocks are labelled "Proven in the game's code" on `docs/clone-hero/03-timing.md`.

A side is the same number early and late. The words "early" and "late" on the row say which way the player presses that note, not that the two sides differ.

The row covers normal mode at 100% speed only. Precision mode and other speeds are in "Out of scope" below, for the user to approve.

## One owner for the rule

Hydra already computes this rule, in `tools/replay_schedule.cpp`: `half_gap_ms`, `whole_window_ms`, `window_side_ms` and `chord_sides_ms`, with the six constants above them. The header says only `hydra_replay` and its tests read it, and nothing under `src/` includes it.

I searched for a second copy before writing that there is none. A search of the whole repo outside `docs/` for the two formula constants finds one place that computes with them: `tools/replay_schedule.cpp`. The other hits are prose in `tools/ch_probe/SPEC.md` and schedule files under `tools/ch_probe/tests/` that carry the constants as data the tool wrote.

Task T1 moves the four functions and their constants into a new core file, `src/core/note_window.h` and `.cpp`. The tool then calls the core functions and keeps no copy. Its `constants` block in the schedule file reads the same core constants, so the file it writes does not change. After T1 the rule has three readers: the schedule tool, the Paths tab's new row, and the tests.

`kHitWindowMs` (85) stays a separate number. It happens to equal the game's back clamp for one half-gap, but TM-18 records that they are two different things, and D270 call 8 owns the 85.

The tool also holds one more rule the row needs: which chord a squeeze names. A squeeze-in's chord is the SP-end step it set, and a squeeze-out's is the chord the engine stamped (`Activation::sqout_tick`). That sits inline in `activation_json` today. T1 moves it to one accessor on `Activation`, and the tool calls it, so the row and the tool cannot disagree.

## What each row shows

Every row has the same shape: the label, one or two windows, and the tag. The spare is always "what the game's windows give, minus what the timing needs". At zero or more the tag is yellow and says "spare". Below zero it is orange and says "short", with the size printed without a minus sign. Exactly zero reads "0.0 ms spare". That edge is a wording choice, not a game fact: the plan does not claim the game hits a press exactly on a window's edge.

### Squeeze out (settled by the mockup)

The player hits the anchor early, which pulls the SP end earlier, and hits the squeezed note late, so it lands after the end. The anchor is the note whose timing moves the SP end: the activation chord, or the chord that overfilled the meter when the SP cap cut the end short (`Activation::squeeze_anchor_tick`).

The row names the anchor's window first, as "early", then the squeezed note's window, as "late". An early anchor hit of d ms moves the end by r x d, where r is the early side of the stored transfer scale at the SP end (`transfer_post->early`). So:

spare = anchor side x r + squeezed side - how far the note sits before the SP end.

Dead on Arrival (O), activation 4: 85.72 x 1.033 + 79.41 - 143.88 = 24.1 ms spare. Tapped Out, activation 4: 77.85 x 1.0014 + 71.97 - 215.8 = -65.9, so 65.9 ms short. Hydra's own budget is the same sum with 85 in place of both sides, 85 x (1 + r), which is the 172.8 ms its tooltip shows for Dead on Arrival (`squeeze_budget_ms`).

### Squeeze in (settled by the code)

A squeeze in is the mirror. The note sits past the SP end. The player hits the anchor late, which pushes the end later, and hits the squeezed note early, so it lands before the end. The tool's comment above `equal_room_lead_ms` says exactly this, and `moving_ratio` picks the late side of the squeeze-in's own stored scale (`SPSqueeze::transfer->late`).

So the row names the anchor's window first, as "late", then the squeezed note's window, as "early": `Dynamic Hit Window:  85.7 ms late   79.4 ms early`. The anchor comes first in both squeeze rows because it comes first in the chart.

spare = anchor side x r + squeezed side - how far the note sits past that squeeze's SP end.

The code settles which notes, which sides and which scale, so this is not a question.

### Early fill

An early fill has one note: the note whose hit makes Star Power ready (`Activation::sp_ready_tick`). The figure on the "Early fill" line is how many ms after the fill's deadline Star Power comes ready. The player must hit that one note at least that early. There is no second note and no transfer scale, because the deadline does not move.

So the row names one window: `Dynamic Hit Window:  79.4 ms early`, then the tag.

spare = the ready note's side - the early fill figure.

Hydra's own version of this sum uses 85: a fill refuses to spawn when Star Power comes ready more than 85 ms late (`fill_refuses`, D250).

This row rests on a model, not a proven game rule. Questions 1 and 5 are about that.

### When a row is left out

A row is left out, and nothing is drawn in its place, when the owner cannot name its numbers. That is: the song's chord times are not in hand, the stored transfer scale is unset, the squeeze names no chord or no SP end in the record, or no chord sits on the tick the rule names. Each of these is tested in T1 or T2. The owner never guesses.

Whether a row shows for a timing that needs no hitting is question 3, and for an optional early fill question 4. The plan is written to the recommended answers.

### Limits of the row

Each row is worked out for its own timing alone. When one anchor leads both a squeeze in and a squeeze out, it would have to be pressed both late and early, and the rows do not say so. When the note that makes SP ready is also a squeeze's anchor, the rows do not combine either. These limits are in "Out of scope".

## Where the early fill row sits

Today the early fill is a bare line of text above the squeeze boxes, such as `Early fill: 0.0 ms (required)`, drawn by `render_activation_body` in `src/ui/paths_tab.cpp`.

The plan puts that line into the same warm box a squeeze sentence gets, on every activation that shows the line. The row sits at the bottom of that box, under the same thin divider, when the row shows. This keeps one design: every timing is a box, and every Dynamic Hit Window row is the bottom line of its timing's box. The box's border turns warning-coloured when the fill's badge is, the same rule the squeeze box follows.

Boxing the existing line is a visible change the user's words did not ask for. So it is question 2, and T3 does not start before the answer (agent rule 12). The mockup file belongs to the main session; the main session can add the early fill box to it so the user can click it before answering.

## What the user will see

One screen changes: the opened activation row on the Paths tab. `squeeze_sentences` and the `early_fill` line are read only by `src/ui/paths_tab.cpp`. The Path Builder reads `activation_badges` only, and the reports read their own columns, so neither changes.

The "before" and "after" texts below are from real charts. The two squeeze-out rows are the mockup's. Figures written N are not known yet; T0 reads them from the charts before any executor starts, and they are rows P20 to P22 of the Premise map.

Dead on Arrival (O), optimal path, activation 4 (squeeze out, room to spare). Before, the box reads: "Hit the [ RY  ] note more than 143.9 ms late so it lands after Star Power ends. It scores 200 fewer points, and its SP phrase banks for later." After, the same sentence, then a divider, then `Dynamic Hit Window:  85.7 ms early   79.4 ms late` and a yellow tag `24.1 ms spare`.

Tapped Out, optimal path, activation 4 (squeeze out, out of reach). Before: "Hit the [ R B ] note more than 215.8 ms late so it lands after Star Power ends. It scores 200 fewer points, and its SP phrase banks for later." After, the same sentence, a divider, `Dynamic Hit Window:  77.9 ms early   72.0 ms late` and an orange tag `65.9 ms short`. The same activation shows `Early fill: 0.0 ms (optional)`. Under questions 2 and 4 as recommended, that line moves into a box and gets no row.

Dead on Arrival (O), activation 3 (a squeeze in that needs no timing). Before and after are the same: "Hit the SP phrase's last note no more than 444.9 ms late (eff. 443.9 ms) so it lands before Star Power ends. The phrase then counts while Star Power runs, which makes Star Power last longer." No row, under question 3 as recommended.

Burnout, optimal path, activation 1 (squeeze out, in the test corpus). Before: "Hit the [  Y  ] note more than 163.0 ms late so it lands after Star Power ends. It scores 260 fewer points, and its SP phrase banks for later." After, the same sentence, a divider, `Dynamic Hit Window:  N ms early   N ms late` and a tag `N ms spare` or `N ms short`.

A squeeze in that needs timing. T0 looks for one in `testdata/input` (the `test_folded_sqin` folder is the first place). Before: "Hit the SP phrase's last note more than N ms early so it lands before Star Power ends. ..." After, the same sentence, a divider, `Dynamic Hit Window:  N ms late   N ms early` and its tag.

A required early fill: Tapped Out, activation 3 (E0). Before: a bare line `Early fill: N ms (required)`. After: the same line in a box, a divider, `Dynamic Hit Window:  N ms early` and its tag.

## Stored data

No stored record and no stamp moves. The row is worked out when a path's details are built, from the stored activation and from the chord times of the song Hydra has just read. Nothing new is saved. `src/store/stored_versions.h` is not touched, and no chart is re-analysed.

## Questions

Each question stands alone. The recommended answer comes first. The plan is written to the recommended answers; a different answer changes the task it names.

1. **Should the early fill row ship while its game rule is unproven?** Recommended: yes, ship it. The row says the player can make a fill appear by hitting the SP-ready note early, by up to that note's window. Hydra's existing "Early fill" line and its 85 ms reach already rest on the same idea, and FL-24 on `docs/clone-hero/04-fills.md` labels it "Hydra assumes it, never checked". The game's own check is an exact tick compare made at the moment Star Power is added (FL-19). FL-25 records that on Trippin SP Spike the bot was not offered an E0 fill in three runs; the bot hits on time, so that neither proves nor disproves the model. The other choice is a live check first: one run with the SP-ready note pressed early. That needs your permission for a game run and its own plan.

2. **May the "Early fill" line move into a box?** Recommended: yes. Today the line is bare text, for example `Early fill: 0.0 ms (optional)` on Tapped Out's activation 4. The plan draws it inside the same warm box as a squeeze sentence on every activation that shows it, so the Dynamic Hit Window row has the same home for all three badge types. This changes how every early fill line looks, including those with no row. If you say no, the line stays bare and the row is drawn directly under it with no box and no divider.

3. **Does a timing that needs no hitting get a row?** Recommended: no row. The real test is `timing_needs_hitting`, which is `difficulty_ms >= 0.0`. A squeeze shows a row only when its figure is 0.0 ms or more. One end: Dead on Arrival's activation 3, badge `squeeze in -444.9 ms`, already lands 444.9 ms inside Star Power, so it gets no row. The other end: Beg (Evans Blue), badge `squeeze out 0.0 ms`, gets a row, because a 0.0 ms figure is still a timing to hit (D302). A badge of -0.1 ms gets none. The same test applies to an early fill: `early fill -56.2 ms` gets no row. If you want a row there too, it would show the windows and a spare larger than the windows themselves, which says little.

4. **Does an optional early fill get a row?** Recommended: no row. The real test is `Activation::is_E0`: the fill is required only when the path skipped no fill before it. One end: Tapped Out's activation 3 is `E0`, its fill is required, and it gets a row. The other end: Tapped Out's activation 4 is `E1-`, its line reads `Early fill: 0.0 ms (optional)`, and it gets no row. The path works whether or not that fill appears, so there is no timing the game's window could fail.

5. **Should the rows ship while one part of the window rule is unmeasured?** Recommended: yes. That the game stores the formula's window on each note is proven (TM-17 to TM-23). That the hit check holds a press to it was measured only on notes with the widest side, 85.72 ms (TM-27, a live run). TM-27 says the edge on notes with closer neighbours, whose side is smaller, is "Not proven". The Tapped Out row uses sides of 77.85 and 71.97 ms. The other choice is a live run on a chart with close neighbours first, which needs your permission.

6. **Does "just show it alongside" close Q-32?** Recommended: keep Q-32 open and record your two rulings in it. Q-32 on `docs/clone-hero/questions.md` is the open split between the game's per-note window and Hydra's 85. Your ruling settles that the Paths tab shows both. It does not say whether the ratings, tiers or the early-fill reach should ever follow the game, and Q-32 also covers precision mode. Closing it is yours to decide.

## Tasks

T0 is the main session's and can start as soon as the plan is approved (`earlyStart`). T1 starts at the same time. T2 and T4 start together from T1's tip. T3 starts from T2's tip, after question 2 is answered. T5 is the main session's and comes last. Executors run on Opus, and each change gets one fresh Sonnet reviewer (agent rules 2 and 5). No executor builds or runs a test that needs a build (agent rule 14).

### T0: read the real numbers from the charts (main session; earlyStart)

The main session runs `build-cpp\Release\hydra_replay.exe` from the main checkout, dump then `schedule`, on at most six charts: Dead on Arrival (O), Tapped Out, Burnout, Beg, Eric Moore's Get Ur Freak On, and one chart from `testdata/input/test_folded_sqin`. It never runs the whole library.

It writes down, for each timing listed in "What the user will see": the two chords' `side_ms`, the stored scale, the offset, and the spare the plan's formula gives. These go into T2's and T3's briefs as the literals their tests pin.

It also checks row P22: the schedule's `side_ms` for Freak's chord 1062 must be 73.76 ms, the figure TM-51 gives from the game's code and a live run. If it is not, Hydra's chord list is not the game's neighbour list, and the plan stops there and goes back to the user.

Done when: every N in "What the user will see" has a number, or the report says which chart has no such timing. If the six charts hold no squeeze in that needs timing, T2 tests that row on a hand-built activation and the report says so. If Tapped Out's activation 4 does not come out at 77.85, 71.97 and 65.9 short, stop and report.

### T1: the core owner (one executor; cut from main)

Owned files: `src/core/note_window.h` and `src/core/note_window.cpp` (new), `src/core/model.h` and `src/core/model.cpp` (one new accessor), `tools/replay_schedule.h` and `tools/replay_schedule.cpp`, `tests/test_note_window.cpp` (new), `tests/test_replay_schedule.cpp`, and the root `CMakeLists.txt`, which names the core sources, the test files and the `hydra_replay_json` library the tool's code is built into.

The executor moves `half_gap_ms`, `whole_window_ms`, `window_side_ms`, `chord_sides_ms` and their six constants into the new core file, unchanged. The tool calls them there. It adds one type that holds a song's chord ticks and times in chart order, built once from a `Song`, with a lookup from a tick to its chord. It adds `Activation::squeeze_chord_tick`, and `activation_json` calls it in place of its inline rule.

It adds the three sums as plain functions on plain numbers, and one function per badge type that reads an `Activation` and the chord times and returns the row's numbers or nothing: the first side, the second side when there is one, and the spare. These read `squeeze_anchor_tick`, `squeeze_chord_tick`, `sp_ready_tick`, the stored scales and the stored offsets. They work nothing out again.

Before committing, the executor greps for anything else that answers "how wide is this note's window in the game" or "which chord does this squeeze name", as the preamble's "If you write code" section says, and calls the owner where one exists.

Acceptance, each a named test in `tests/test_note_window.cpp` unless said otherwise:

- The formula gives 171.4313075 ms at x = 170 and 96.2931672 ms at x = 75, pinned as literals from TM-21. The tests at lines 266 to 278 of `tests/test_replay_schedule.cpp` move here with their literals unchanged.
- The two formula constants appear in exactly one file under `src/` and `tools/`. The test reads the source files, the way `tests/test_replay_schedule.cpp` line 366 reads `model.h`, and fails if a second file holds either.
- The squeeze-out sum gives 24.1 ms from 85.72, 79.41, 1.033 and 143.88, and reports "short" by 65.9 from 77.85, 71.97, 1.0014 and 215.8. Both pinned from the mockup.
- A spare of exactly 0 is "spare", and the smallest value below 0 is "short".
- The squeeze-in function uses the late side of the squeeze's own scale, and the squeeze-out function the early side of `transfer_post`. One test builds an activation whose early and late scales differ and fails if either function reads the wrong one.
- A cap-clamped activation's anchor is the overfill chord, not the activation chord.
- Each function returns nothing for: an unset scale, a squeeze with no chord or no end in the record, a tick with no chord on it, and an activation with fewer stored bar arrivals than the activation level. One case each.
- Every existing test in `tests/test_replay_schedule.cpp` keeps its expected values. The only edits there are the moved tests and names.

Done when the branch is committed and the report lists the new function names for T2 and T4.

### T2: the view-model text (one executor; cut from T1's tip)

Owned files: `src/app/path_view.h`, `src/app/path_view.cpp`, `src/ui/app_state.h`, `src/ui/app_state.cpp`, the one `cache.details(...)` call in `src/ui/paths_tab.cpp`, and `tests/test_activation_rows.cpp`.

The viewed song already keeps the analysed song's timing (`v.timing = out.analysis->song.timing()` in `src/ui/app_state.cpp`). The executor keeps that song's chord times beside it, built by T1's function, and passes them through `PathsTabCache::details` to `build_activations` as a pointer that may be null, the way `timing` is passed.

`build_activations` fills a small row struct for each squeeze sentence and one for the early fill: the windows text, the tag text and whether the tag is "short". The label `Dynamic Hit Window:` is one constant. Figures are printed by `format_ms`, the function the sentences already use. The row is filled only when T1's function returns numbers and the timing passes the tests in questions 3 and 4 as answered.

Acceptance, each a named test in `tests/test_activation_rows.cpp`:

- Tapped Out's optimal path, activation 4, gives windows `77.9 ms early   72.0 ms late` and tag `65.9 ms short`, marked short. The chart is in `testdata/input`.
- Burnout's activation 1 gives the windows and tag T0 measured.
- The squeeze-in row reads `N ms late   N ms early` on T0's chart, or on a hand-built activation if T0 found none.
- The required early fill row on Tapped Out's activation 3 reads `N ms early` and its tag, with T0's figures.
- No row for a squeeze that needs no timing, for an early fill with time to spare, and for an optional early fill (Tapped Out's activation 4), one case each. These follow questions 3 and 4.
- No row anywhere when the chord times pointer is null.
- With and without chord times, every other field of every activation row is equal: badges, sentences, early fill line, scale line, backend rows, ratings and tooltips. This is the test that the row is "alongside" and moves nothing.
- Every existing literal in `tests/test_activation_rows.cpp` is unchanged.

Done when committed. The report says no file under `src/store` changed.

### T3: the drawing and the GUI test (one executor; cut from T2's tip; after question 2's answer)

Owned files: `src/ui/paths_tab.cpp`, `src/ui/badge_pill.h`, `tests/ui/uitest_paths.cpp`, and the test file that covers `badge_pill.h` if one exists, else `tests/test_activation_rows.cpp` for one colour test.

`squeeze_box` draws the row at the bottom of its box: a thin divider, the label, the windows in the figure font, and the tag pushed to the right edge with a one-pixel outline. The tag's colours come from one function that takes "short or not": yellow `#f2c300` on `#2a2410` for spare, orange `#ff7f00` on `#2e1a08` for short. These are the mockup's values, which the user approved; if the theme already holds the same colour under a name, the function returns that name's value. The early fill line is drawn through the same box, per question 2.

When the box is too narrow for the row on one line, the tag drops to its own line at the right. Nothing is cut off.

Acceptance:

- A new `hydra_uitest` test opens Tapped Out, opens activation 4 of the optimal path, and finds `Dynamic Hit Window:`, `77.9 ms early`, `72.0 ms late` and `65.9 ms short` on screen. It opens Burnout's activation 1 and finds T0's figures. This is case 1 of "Which check a GUI change needs" on `docs/agents/ui-testing.md`, behaviour and text.
- The same test opens an activation with no timing badge and finds no `Dynamic Hit Window:` on screen.
- At the narrowest details panel, all four pieces of Tapped Out's row are still on screen.
- A unit test pins the tag colour function's two results to the four mockup values.
- The existing checks in `tests/ui/uitest_paths.cpp` are unchanged.

Done when committed. The executor hands the branch back unbuilt.

### T4: the documents (one executor; cut from T1's tip; parallel with T2 and T3)

Owned files: `docs/UserGuide.md`, `CONTEXT.md`, `docs/clone-hero/03-timing.md`, `docs/clone-hero/04-fills.md`, `docs/clone-hero/questions.md` and `docs/clone-hero/README.md` (its "Last write-back" date only).

`docs/UserGuide.md` is public, so it follows `docs/agents/public-doc-style.md`: second person, no first person, every label and number checked against the code. The list of what an opened activation shows gains the row, with the Tapped Out and Dead on Arrival texts as examples. It says in one sentence that the ratings and tiers stay on the 85 ms hit window. Its early fill item changes only if question 2 is a yes.

`CONTEXT.md`'s "Hit window" entry gains a short paragraph: the Dynamic Hit Window row shows the game's per-note window beside the 85, it is display only, and its owner is the new core file.

On the Clone Hero pages, the `Hydra:` lines of TM-17 to TM-22 and TM-51 name the new core owner in place of "no owner" and of the tool. TM-51's "no screen reads it" becomes "the Paths tab's Dynamic Hit Window row reads it". TM-22's `Ruling:` line and Q-32's "Hydra today" paragraph quote the two rulings of 2026-11-12. FL-24's `Hydra:` line names the early fill row. No label changes and no block is added, so the README's counts stay. Q-32 stays open unless question 6 says otherwise.

Acceptance: the docs test passes on the branch (`hydra_tests.exe "-tc=docs match code*"`, run by the main session). It checks that every backticked Hydra name on these pages exists in the code, which is why T4 is cut from T1's tip. The reviewer checks each quoted screen text against T2's test literals.

### T5: build, test and the real-app look (main session; last)

The main session merges nothing before this. It builds each branch through a pass from `hydra_build_pass`, runs the tests `hydra_precheck.exe --tests` names for the branch, and runs the new `hydra_uitest` test.

A new row in a box is a layout, so case 2 of `docs/agents/ui-testing.md` applies: the headless check is not enough. Before merge the main session opens the real dev app with the user's `hydra_ui.ini` and a copy of their database, opens Tapped Out's activation 4 and Dead on Arrival's activation 4, and shows the user the captures. If the user says to merge first, the main session says plainly that this check is still owed.

This plan leaves the repo in the merge that finishes T5, and the docs check proves nothing still points at it.

## Out of scope (needs the user's yes to leave out)

- Precision mode. The row shows normal-mode windows only. In precision mode the game's clamps are 25 and 40 ms, and TM-31 to TM-33 are not fully traced.
- Speeds other than 100%. The tool's window is stated "at 100% speed", and the row keeps that.
- Any change to the search, the ratings, the eff. figures, the timing tiers or the badges. The rulings above keep them on 85.
- Two timings that share a note. Each row is worked out alone. A squeeze in and a squeeze out on one anchor, or an early fill whose ready note is also an anchor, are not combined.
- A row in the Path Builder, the path report or `hydra_batch`. Only the Paths tab shows it.
- An early fill whose Star Power came ready at the previous SP end rather than on a chord (a bar from a squeezed-out phrase, `bank_rise_ticks`). No chord sits on that tick, so the row is left out.
- Proving FL-24 or TM-27's open part in the game. Questions 1 and 5 ask whether to ship without them.

## The live steps

This plan has no live steps. Nothing in it starts the game.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| The badges, ratings, eff. figures, tooltips and sentences do not change | "Nothing else changes" | T2 | T2's with-and-without test: every other field equal; existing literals in `tests/test_activation_rows.cpp` unchanged |
| The search does not change | "The search does not change" | T1, T2 | no file under `src/search` in either diff (`git diff --stat main -- src/search` is empty); the reviewer runs it |
| The window rule is worked out in one place | "One owner for the rule" | T1 | T1's one-file test on the two formula constants |
| The formula's values are the game's | T1 acceptance | T1 | T1's literals 171.4313075 at 170 and 96.2931672 at 75 |
| The schedule file the tool writes does not change | "the file it writes does not change" | T1 | every existing expected value in `tests/test_replay_schedule.cpp` unchanged and passing |
| Which chord a squeeze names has one owner | "one accessor on `Activation`" | T1 | `activation_json` calls `squeeze_chord_tick`; the reviewer greps for the old inline rule; existing schedule tests pass |
| Squeeze out: anchor early, squeezed late, early scale of `transfer_post` | "Squeeze out" | T1 | the two mockup sums; the wrong-side test |
| Squeeze in: anchor late, squeezed early, late scale of the squeeze's own | "Squeeze in" | T1, T2 | the wrong-side test; T2's squeeze-in text test |
| Early fill: the ready note's side minus the fill figure | "Early fill" | T1, T2 | T2's Tapped Out activation 3 test |
| A cap-clamped activation uses the overfill chord | "Squeeze out" | T1 | T1's cap-clamped test |
| Zero or more is yellow "spare"; below zero is orange "short", printed without a minus | "What each row shows" | T1, T2, T3 | T1's zero-edge test; T2's `65.9 ms short` literal; T3's colour test |
| The owner never guesses: no row when a number is missing | "When a row is left out" | T1, T2 | T1's four "returns nothing" cases; T2's null-pointer test |
| No row for a timing that needs no hitting (question 3) | Question 3 | T2 | T2's two no-row cases |
| No row for an optional early fill (question 4) | Question 4 | T2 | T2's Tapped Out activation 4 case |
| The row is the bottom line of its box, under a divider, tag at the right | "What the user decided" | T3, T5 | T3's on-screen text test; T5's real-app captures shown to the user |
| The early fill line is boxed only after the user's yes | "T3 does not start before the answer" | T3 | T3's brief quotes the user's answer to question 2; the reviewer checks the quote |
| Nothing is cut off at the narrowest panel | T3 | T3 | T3's narrowest-panel test |
| Tag colours are the mockup's | T3 | T3 | T3's colour test |
| Only the Paths tab changes | "One screen changes" | T2, T3 | existing Path Builder and report tests pass unedited; T3's no-badge check |
| No stored record or stamp moves | "Stored data" | T2 | `git diff --stat main -- src/store` is empty on every branch; the reviewer runs it |
| The public guide's labels and numbers match the code | T4 | T4 | the docs test; the reviewer checks each quoted text against T2's literals |
| The Clone Hero pages name the new owner | T4 | T4 | the docs test (every backticked Hydra name exists) |
| The layout is seen in the real app before merge | T5 | T5 | T5's captures, shown to the user |
| Hydra's chord list is the game's neighbour list | T0 | T0 | Freak chord 1062 reads 73.76 ms, or the plan stops |
| T0 runs at most six charts, never the library | T0 | T0 | T0's report lists the charts it ran |
| No executor builds or runs a built test | "Tasks" | T5 | the progress hook's refusals; the main session runs every built test in T5 |
| The plan leaves the repo when its work is merged | T5 | T5 | the docs test (`docs match code*`) |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: each half-gap is held between 37.5 and 85 ms in normal mode, and a missing neighbour gives 85 | data | `docs/clone-hero/03-timing.md` blocks TM-18 and TM-19 | checked 2026-11-12 |
| P2: a note's neighbours are the nearest notes at another tick, so a chord's gems share one window | data | `docs/clone-hero/03-timing.md` block TM-51 | checked 2026-11-12 |
| P3: the two half-gaps are summed, the formula gives the whole window, and the note gets half a side; 171.4313075 ms at 170 and 96.2931672 ms at 75 | data | `docs/clone-hero/03-timing.md` blocks TM-20 and TM-21 | checked 2026-11-12 |
| P4: a side runs from 48.15 to 85.72 ms | data | `docs/clone-hero/03-timing.md` block TM-22 | checked 2026-11-12 |
| P5: Hydra's copy of the rule is `half_gap_ms`, `whole_window_ms`, `window_side_ms` and `chord_sides_ms` with six constants | code | `tools/replay_schedule.cpp` lines 23 to 31 and 126 to 156 | checked 2026-11-12 |
| P6: no other code computes the formula; the other hits are prose and data | code | a search for `0110924` and `2\.628539` outside `docs/`: `tools/replay_schedule.cpp`, `tools/ch_probe/SPEC.md` and the schedule files under `tools/ch_probe/tests/` | checked 2026-11-12 |
| P7: nothing under `src/` includes the tool's header; it is built into the `hydra_replay_json` library and included by `tools/replay.cpp` | code | a search for `replay_schedule` under `src/`, `tools/` and the root `CMakeLists.txt` (line 945) | checked 2026-11-12 |
| P8: a squeeze out's chord is `Activation::sqout_tick`, a squeeze in's is the step it set, and the anchor is `squeeze_anchor_tick` | code | `tools/replay_schedule.cpp` lines 455 to 466; `src/core/model.cpp` lines 739 to 784 | checked 2026-11-12 |
| P9: the anchor's press moves a squeeze out's end by the early scale of `transfer_post` and a squeeze in's by the late scale of its own `transfer` | code | `tools/replay_schedule.cpp` lines 174 to 176 and 661 to 662; `src/core/model.h` lines 371 to 423 | checked 2026-11-12 |
| P10: a squeeze in's anchor is pressed late and its note early; a squeeze out's the other way | code | `tools/replay_schedule.cpp` lines 189 to 203 | checked 2026-11-12 |
| P11: Hydra's own budget is 85 x (1 + r) | code | `src/core/squeeze_rating.cpp` lines 111 to 128 | checked 2026-11-12 |
| P12: a timing needs hitting when its figure is 0.0 or more, and `needs_no_timing` is the opposite | code | `src/core/model.h` lines 332 and 411 | checked 2026-11-12 |
| P13: the early fill figure is ready time minus deadline; the fill refuses past 85 ms; it is required only when `is_E0` | code | `src/core/model.h` lines 316 to 327; `src/core/model.cpp` lines 473 to 491 | checked 2026-11-12 |
| P14: the "Early fill" line is bare text and a squeeze sentence is a box | code | `src/app/path_view.cpp` lines 225 to 231; `src/ui/paths_tab.cpp` lines 330 to 344 and 462 to 464 | checked 2026-11-12 |
| P15: only the Paths tab reads `squeeze_sentences` and the `early_fill` line; the Path Builder reads `activation_badges` | code | a search of `src/` for both names: `src/ui/paths_tab.cpp` lines 462 to 464 and `src/pathbuilder/pb_text.cpp` line 97 | checked 2026-11-12 |
| P16: the viewed song's timing comes from the analysed `Song`, so its chords are in hand at the same line | code | `src/ui/app_state.cpp` line 544; `src/ui/app_state.h` line 181 | checked 2026-11-12 |
| P17: `sp_ready_tick` is the stored arrival of the bar that reached the activation level, and a squeezed-out phrase's bar may arrive at the previous SP end | code | `src/core/model.h` lines 617 to 630; `src/core/model.cpp` lines 695 to 699 | checked 2026-11-12 |
| P18: the game tests the fill deadline at the moment Star Power is added, by an exact tick compare | data | `docs/clone-hero/04-fills.md` block FL-19 | checked 2026-11-12 |
| P19: an early hit of the ready note makes a late fill appear, as far as that note's window | unchecked | `docs/clone-hero/04-fills.md` block FL-24, labelled "Hydra assumes it, never checked" | question 1 |
| P20: the hit check holds a press to the note's side on notes with closer neighbours | unchecked | `docs/clone-hero/03-timing.md` block TM-27, which measured only 85.72 ms sides and says the rest is not proven | question 5 |
| P21: Dead on Arrival's activation 4 has sides 85.72 and 79.41, scale about 1.033, offset 143.88; Tapped Out's has 77.85, 71.97, 1.0014 and 215.8 | unchecked | the main session's figures in this plan's brief; the texts they give are in `docs/superpowers/specs/2026-11-12-dynamic-hit-window-mockup/index.html` lines 159 and 171 | T0 |
| P22: Hydra's chord list gives the same neighbours as the game's note list | unchecked | `docs/clone-hero/03-timing.md` block TM-51 gives 73.76 ms for Freak's chord 1062; Hydra's figure for it was not read | T0 |
| P23: Burnout's, Beg's and Tapped Out's activation 3 figures, a squeeze in that needs timing in the corpus, and that the real charts give the sentences the mockup shows (P25) | unchecked | `tests/test_activation_rows.cpp` lines 579 to 606 and `tests/ui/uitest_paths.cpp` lines 326 to 339 hold the sentences and badges, not the windows | T0 |
| P24: the approved squeeze-out texts and tag colours | code | `docs/superpowers/specs/2026-11-12-dynamic-hit-window-mockup/index.html` lines 73 to 76, 159 and 171 | checked 2026-11-12 |
| P25: the mockup shows Dead on Arrival's activation 3 as "no more than 444.9 ms late (eff. 443.9 ms)" and Tapped Out's activation 4 as `Early fill: 0.0 ms (optional)` | code | `docs/superpowers/specs/2026-11-12-dynamic-hit-window-mockup/index.html` lines 154 to 155 and 169 to 170 | checked 2026-11-12 |
| P26: Tapped Out is in the test corpus | chart | `testdata/input/common/IB24/T5/Mike Orlando - Tapped Out/song.ini` | checked 2026-11-12, the folder was found; the notes file was not opened |
| P27: the user ruled "just show it alongside." and asked for the label, the units and the separate tag | ruling | `docs/agents/rulings/ui-and-display.md` line 5 | checked 2026-11-12 |
| P28: the user ruled the 500 ms squeeze search "an intentional deviation from the game, not a bug" | ruling | `docs/agents/rulings/clone-hero.md` line 5 | checked 2026-11-12 |
| P29: the user asked to "plan this for all three badge types: squeeze in, squeeze out, early fill" | ruling | `docs/agents/rulings/ui-and-display.md` line 5 | checked 2026-11-12 |
| P30: D270 call 8 rules one window number everywhere, and the 85 is separate from the game's back clamp | code | `docs/clone-hero/03-timing.md` TM-18's `Hydra:` line and TM-22's `Ruling:` line; `src/core/model.h` lines 101 to 105 | checked 2026-11-12 |
| P31: a text change needs `hydra_uitest`, and a layout change must also be seen in the real app before merge | code | `docs/agents/ui-testing.md` lines 9 and 10 | checked 2026-11-12 |
| P32: the stamps cover stored, computed data only | code | `src/store/stored_versions.h` lines 1 to 12 | checked 2026-11-12 |
| P33: the tool's window is the one at 100% speed | code | `tools/replay_schedule.h` lines 8 to 9 | checked 2026-11-12 |
