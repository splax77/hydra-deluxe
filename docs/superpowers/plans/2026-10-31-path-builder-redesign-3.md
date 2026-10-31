# Plan: the Path Builder brought to the third mockup (2026-10-31)

Task id `PBR3-PLAN`, session `711858a7-e2d0-4bc0-89e7-f2226a576639`. Planner: Fable. Branch `claude/pbr2-mockup3` (`claude/pb-redesign` plus the third mockup). Status: plan written; nothing is built. Every file and line named below was read in this worktree on 2026-10-31.

The requirement is the user's own words in `docs/superpowers/specs/2026-10-31-path-builder-mockup-3/answers.md`. The behaviour spec is the script of `mockup_template.html` beside it, as the page stood when the user said "yes this looks good": the question bar hidden, each question on its recommended option, the 3-minute window and the scrollbar. Line numbers below are that file's. The second plan, `docs/superpowers/plans/2026-10-31-path-builder-redesign-2.md`, holds every earlier answer; they stand unless this plan says otherwise.

## The plain version

The builder already looks like the second mockup. The user found four problems in it on the 42-minute album: the tooltip was a wall of white text, the whole song was squeezed into one width, hovering a fill started sound, and the dots were too small to hit. The third mockup fixes all four and the user approved it. So this plan does five builds and a test: a pure "view" module that owns the window arithmetic (where the 3 minutes start, how a page turn or a wheel notch moves it, what the readout says); the timeline drawn through that window with an overview strip above it and a scrollbar under it, and a Fills row where the pointer always picks the nearest fill; the tooltip's words as a short card; the screen's hover rule (show, never play) and the card tooltip; then the screen's view state (following the playhead, a row hover that scrolls to its fill, a row click that moves the clock). Then the click-through test grows a long-song run that scrolls past 3 minutes and activates there, the user sees it in the real app on the album and on The Reaper, and the documents from the last plan (DOC-2) are written and the branch catches up with `main`.

## Pricing stays on demand: what the code does, and what the album costs

The user's design requirement, word for word: "important design requirement - the real app should only calculate paths as needed in real time. calculating and storing every possible path combination would be insanely inefficient for long tracks".

What the builder does today, checked in the code. `DecisionList::price_fills` (`src/pathbuilder/pb_coach.cpp` line 463) hands `Coach::request_candidates` the ticks of `PreviewScene::trailing_skipped_fill_ticks`, which is the engine's own list of the fills the player could activate on after their last window (`pb_window.cpp` line 395). The same ticks again start nothing (line 327). `candidate_finishes` (line 240) runs one `best_finish` per tick, in chart order, one at a time, on one worker thread (`CandidateJob`, line 266), and reports each as it lands. A newer request cancels the running queue through `throw_if_cancelled` after each landing (line 285) and retires it (`retire_candidates`, line 334); a new path key retires the queue too (`Coach::request`, line 317). What is stored per fill is one `CandidateFinish` (a tick and a total) and one optional `Activation` (line 281), and `DecisionList` keeps only the current queue's results (`refresh_candidates`, line 472, clears and rebuilds). Nothing is stored per path combination; the mockup's prefix tree exists only in `build_data.py`.

What one priced fill costs on the album, measured on 2026-10-31 with `build-cpp\Release\hydra_replay.exe` on the chart copy in the session's `scratchpad\mock3\album\` (one chart; nothing ran over the library). `best_finish` is `run_search` with the pins and `free_after` (`src/search/pather.cpp` line 379), the same search `hydra_replay dump` runs, and no command line runs `best_finish` alone, so one whole dump process is the ceiling for one priced fill: at the GUI's default depth (4 scores, SP cap 4, 10 ms) a dump of the album took 37, 24 and 24 ms wall over three runs, chart load included. At a 20,000-point band, which keeps 878 paths, one dump took 695 ms. The album has 160 fills and 125 phrases (`score.json`); 159 fills follow the second phrase end, so the empty path lists about 159 candidates, and 155 follow the first activation at m40 (tick 74,880, ending at 90,240) once two phrases are saved again. So the whole queue after a click is about 155 searches: about 6 s at the default depth, about 110 s at a 20,000-point band, on one worker, landing one row at a time, cancelled by the next click. The user's own depth is whatever `hydra_settings.ini` holds (`PbState::open` loads it, `pb_state.cpp` line 109), so the real-app check reads the time off the screen (RA-6).

So today's queue is already on demand, and this plan adds no pricing code. The three rules the brief asks for hold as follows. The fills the user can see and the top of the list are priced first: the queue runs in chart order from the last activation, and after a click the window sits on the clicked fill (SC-4b's "a click keeps the view where the hover put it"), so the fills in view are the first in the queue; the list's pending rows are in chart order too (`choice_order`), so its top rows land first. Nothing is priced that no screen element needs: every candidate's cost is read by the list (one row each), the timeline's tint and the strip's tint (answer 3 of the bar: the strip shows fills tinted by cost over the whole song), so every queued search has a reader. Nothing is stored past the current path: a new key retires the queue and `candidates_` is rebuilt from the newest queue only. Each of these is pinned by an existing test in `tests/test_pb_coach.cpp` (`candidate queue: it prices only the ticks it is given, in order`, line 483; `candidate queue: a newer request supersedes the older`, line 509) and the promise map names them.

## Prior art

The pattern is "overview+detail": a zoomed detail view with an overview that shows where you are. Cockburn, Karlson and Bederson's survey is the standard reference and separates it from zooming and focus+context ([A Review of Overview+Detail, Zooming, and Focus+Context Interfaces](https://www.microsoft.com/en-us/research/?p=155152), [PDF](https://www.microsoft.com/en-us/research/wp-content/uploads/2008/01/cockburn-ComputingSurveys09.pdf)). Schwab et al. (CHI 2019) measured pan-and-zoom timelines and found the surrounding context helps far navigation more than near ([Evaluating pan and zoom timelines and sliders](https://vis.khoury.northeastern.edu/pubs/Schwab2019EvaluatingPanZoom)). Commercial controls call the strip a navigator or zoombar; Infragistics' xamZoombar pairs a timeline with a thumb that sets the visible range and a preview of the whole series ([xamTimeline zoombar](https://www.infragistics.co.kr/help/wpf/xamtimeline-xamwebzoombar.html)). The mockup's strip is that navigator with a fixed zoom; the scrollbar under it is the ordinary one.

## What is already there and what changes, checked in the code

The timeline lays the whole song over one width: `timeline_layout` (`src/ui/preview_timeline.cpp` line 338) maps every ms through `song_fraction`, and `timeline_hit` (line 490) reads each fill's square hit box (`item.hit`, line 473, `fill_size` wide). The draw (line 541) makes one `InvisibleButton` per fill named `timeline_fill_id` and one `##timeline` item over the rows for bands and moments (line 712). Text drawn by the draw list goes through `log_text` (line 146) so GUI tests can read it. Sections already alternate over the shown sections: the hidden End section is skipped before the push (line 380) and the shade reads the index in `l.sections` (line 596), which is what mockup 3's `shown++` does (line 554). The saved caption already wraps (`draw_saved_caption`, line 154, wraps at `wrap_w`). The side column is `kSideColumnRem` = 27 (`pb_window.cpp` line 42). So "the rest stays" needs no code; the promise map points at the lines and the tests that read them.

The hover rule is in `PbApp::draw_timeline` (`pb_window.cpp` lines 633 to 647): on enter it saves the position and whether it was playing, pauses and seeks; on leave it seeks back and plays again if it was playing (`hover_return_playing_`, `pb_window.h` line 146). Answer 1 removes the play-again. The tooltip is `fill_hover_text` (`src/pathbuilder/pb_text.h` line 150), six lines wrapped at `kTooltipRem` (line 667). The list row's hover already sets the same `hovered_fill_` (line 783 and 625); a row click calls `click_fill` (line 782), which does not move the Preview's clock (the timeline click sets `hover_return_ms_` at line 676; the row path does not), which is bar question 7.

`PreviewController` has what the view needs: `position_ms`, `playing`, `play`, `pause`, `seek_ms`, `playback_end_ms` (`src/ui/preview_controller.h` lines 210 to 220). The panel's Play button and Space both call `pc->toggle()` (`src/ui/preview_panel.cpp` lines 355 and 435), so "Play turns following back on" is read as `playing()` going from false to true between frames, with no change to the shared panel.

## Engine truth, not screen rules

Nothing here computes a game fact. The window is pure arithmetic over milliseconds (TV-1) and reads the song's end from `playback_end_ms`, which is the one song end (D255). Which fill is nearest is geometry over the fills the layout already placed. Costs, tiers, refusals and the card's numbers come from their owners as before (`candidate_costs`, `cost_tier`, `fill_refusal`, `DecisionList::candidate_end`).

## Tasks

Each task is sized for one Opus executor at about 60 tool calls and 15 minutes (the last plan's SC-3b took two executors, so these are smaller). Each runs in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`, forked from `claude/pbr2-mockup3` or as "Blockers" says. No two tasks own one file. `CMakeLists.txt` is edited by TV-1 only. Each task runs only the tests it names; tests pin literals from one run or call the production function. One fresh reviewer per task (D107). Every word the player reads is a named constant with one home, and every new size names its mockup line. Any text a GUI test must read is an ImGui text call or goes through `log_text`. An executor that reaches its budget commits what builds and hands the rest to a fresh executor.

### Wave 1: TV-1 and TX-3 start together; SC-4a and TL-2 fork from their commits

#### TV-1. The view: where the window is and how it moves (pure)

Owns `src/ui/timeline_view.h` (new), `src/ui/timeline_view.cpp` (new), `tests/test_timeline_view.cpp` (new), and the `hydra_uibase` sources and the test list in `CMakeLists.txt`.

What it builds. `struct TimelineView { double start_ms; double span_ms; }` and pure functions over it, each a line of the mockup: `kTimelineWindowMs` = 180,000 (line 312, the 3 minutes the user approved as drawn); `clamp(view, length_ms)` keeps `start_ms` between 0 and `length_ms - span_ms` (`clampView`, line 354; Q1 says what a song shorter than the window does); `centre_on(view, ms, length)` (line 360); `page(view, n, length)` moves n whole windows (lines 711, 713, 714); `wheel(view, notches, length)` moves a third of the window per notch (line 734); `follow_playing(view, pos, length)` returns the window with the playhead a quarter in when the playhead is before the window or past three quarters of it, else the same view (line 362); `show(view, pos, length)` does the same move only when `pos` is outside the window (`seek`, line 813); `fraction(view, ms)` is the x share, and `in_view(view, ms)` its 0..1 test (lines 355 and 358); `thumb(view, length)` gives the scrollbar thumb's left share and width share (lines 697 and 698); `readout(view, length)` is the text "6:00 – 9:00 of 42:02" (line 699: `clockShort` of the start, the end capped at the length, and the length), through `app::format_duration` so a moment has one spelling; the dash is the en dash the mockup writes. `scrollbar_click(view, share, length)` moves one window toward the click when the click is beside the thumb (line 711). Nothing here reads ImGui.

Questions and owners: "where does the window start and end" (TV-1); "does the playhead turn a page" (`follow_playing`); "what does the readout say" (`readout`).

Tests, `test_timeline_view.cpp`, pinned from one run: `timeline view: the window clamps to the song's ends`; `timeline view: a page, a wheel notch and a centre move by the mockup's shares` (one window, a third, and the centre); `timeline view: the playhead turns a page past three quarters and before the window, and not between` (three positions: 0.5, 0.76 and -1 of the window); `timeline view: show moves only when the moment is outside` ; `timeline view: the readout reads start, capped end and length` (a window at 6:00 of a 42:02 song reads "6:00 – 9:00 of 42:02"; one at the end reads a capped end); `timeline view: the thumb's shares`.

Run: `build-cpp\Release\hydra_tests.exe "-sf=*test_timeline_view.cpp"`; `"-sf=*test_layering.cpp"`.

Done when they pass. Blockers: none; Q1 answered before it starts. earlyStart: at the user's yes.

#### TX-3. The tooltip's words as a card (no screen code)

Owns `src/pathbuilder/pb_text.h`, `src/pathbuilder/pb_text.cpp`, `tests/test_pb_text.cpp`.

What it builds. `fill_hover_text` becomes `fill_hover_card(scene, fill_index, numbers)` returning `struct FillHoverCard { std::string heading; std::vector<CardRow> rows; std::optional<app::CostTier> cost_tier; std::vector<std::string> grey; }`, where a `CardRow` is a label and a value, from the mockup's `fillTip` (lines 613 to 627). The heading is the fill's name, " · " and its `clock_str` ("m87 · 2:25.655", line 615; the old "Fill at" goes). A Taken fill: the heading and one grey line, `kYourActivationHoverText` (line 617). A locked fill: the heading and `kBeforeLastActivationText` (line 618). A refused fill: the heading and its `fill_reason` (line 619). An offered fill whose cost has landed: three rows, "Phrases saved" with `bars_saved_at`, "Best finish here" with the best finish, and "Cost" with `cost_or_best`'s word (a cost of 0 reads `kBestFillLabel`, else "−" and the figure, line 624 and `costShort`, line 406), the Cost row's tier for its colour; then the grey lines "This is the best choice now." when the cost is 0 or "Best choice now is m28 (908,085)." with the best fill's name and BF(n) (line 625), and "Hydra would end it at m97" with `squeeze_end_text` and a full stop (line 626). An offered fill still being priced: the heading, the "Phrases saved" row, and the other two rows reading `kPendingText` (Q3 of the last plan: a figure whose search has not landed reads "..."). The row labels are named constants. `cost_or_best` moves here from `pb_window.cpp` if it is not already shared; the executor greps first.

Tests, `test_pb_text.cpp`, pinned from one run on a corpus chart: `fill card: an offered fill reads the heading, three rows and two grey lines`; `fill card: a costless fill reads best and "This is the best choice now."`; `fill card: taken, locked, refused and pending fills read the heading and their one line`; the old `fill_hover_text` test goes with the function.

Run: `hydra_tests.exe "-sf=*test_pb_text.cpp"`; `"-sf=*single_owner*"`.

Done when they pass. Blockers: none. earlyStart: at the user's yes.

#### SC-4a. The screen: hover shows and never plays; the card tooltip; a row click moves the clock

Owns `src/pathbuilder/pb_window.h`, `src/pathbuilder/pb_window.cpp`, `tests/pb_ui/pbtest_coach.cpp`.

What it builds. The hover block (`pb_window.cpp` lines 633 to 647) keeps the pause and the seek to the fill's span start, and on leave seeks back to `hover_return_ms_` and stays paused: `hover_return_playing_` goes (answer 1: "moving off jumps back but stays paused. Only Play or Space starts sound"). The tooltip draws `fill_hover_card` (TX-3): the heading in the normal text colour, each row as a label in the disabled colour with its value bold-weight right of it (a two-column `ImGui::BeginTable` with the label column auto-sized; the mockup's `.tip .kv` grid, lines 90 to 93), the Cost value in `tier_color` of the card's tier, then the grey lines in the disabled colour, all inside `kTooltipRem`. A list row click sets `hover_return_ms_` to the fill's span start before `click_fill`, as the timeline click does at line 676 (bar question 7: "moves the Preview's clock to the fill, like a fill click"). Nothing else about rows changes here; SC-4b adds the scrolling.

Questions and owners: "what does the Preview do while a fill is hovered" (this block, the one owner); "what does the card say" (TX-3).

Tests, `pbtest_coach.cpp`: `hover-shows-never-plays` on Burnout: press the panel's Play button (label "Play"; it reads "Pause" while playing, `preview_panel.cpp` line 355), hover the first fill, read "Play" on screen and not "Pause", move off, read "Play" still and the clock back at its return time (both read through `visible_text`); `fill-card` reads on the first fill the heading "m20 · 0:25.123", the rows "Phrases saved", "Best finish here", "378,175", "Cost", "−140", and the lines "Best choice now is m32 (378,315)." and "Hydra would end it at m24." (the figures pinned by `build-a-path-by-clicking` today, lines 129 to 132); `row-click-moves-clock`: after one `click_fill`, click the row `##choice_74880` and read the clock "0:50.095 / 2:10.088" (the span start the hover test already pins, line 163). `cant-activate-reason` and `locked-fill-says-left-to-right` keep passing. `build-a-path-by-clicking` goes red here by design (its tooltip reads); TS-5 mends it and the two merge together.

Run: `build-cpp\Release\hydra_pathbuilder_uitest.exe --test hover-shows-never-plays --test fill-card --test row-click-moves-clock --test cant-activate-reason --test locked-fill-says-left-to-right`.

Done when they pass. Blockers: TX-3 (earlyStart from its executor commit).

#### TL-2. The timeline through the window: culling, the nearest-fill hit area, the strip and the scrollbar

Owns `src/ui/preview_timeline.h`, `src/ui/preview_timeline.cpp`, `tests/test_preview_timeline.cpp`.

What it builds, in four parts; if the executor reaches its budget after parts one and two, a fresh executor takes three and four.

The window. `timeline_layout` takes a `TimelineView` (TV-1) and maps x through `fraction`; `TimelineInput` carries the view. Sections, bands, ghost bands, the saved line and the shading are laid out in window coordinates and the draw clips the track to the rows' rectangle, so a band half in view shows its visible half. Fills outside the window get no item and no label (mockup line 578); the axis labels run over the window only (line 548, every 30 s, as today). `timeline_hit` reads the window.

The hit area. A fill's `hit` is no longer a square: it spans the Fills row's full height, from halfway to the previous visible fill (or the track's left edge) to halfway to the next (or the right edge), so the pointer picks the nearest fill anywhere in the row (answer 2; mockup lines 599 to 608, `nearest`). The dots draw as today at `item.x`. The `##timeline` item then never sees a click in the Fills row when any fill is in view, which is the mockup's `fillTrack` (a click there is a fill click, not a seek).

The strip. Above the rows, a row `kStripRem` = 2.2 tall (line 110) labelled `kTimelineStripLabel` "Whole song" with the small line `kTimelineStripHint` "click or drag the white frame" (line 247, both through `log_text`): the whole song at once with the sections' stripes (`.os`, `.os.odd`, lines 104 and 105), each fill as a 2 px bar (`.of`, lines 106 and 111) in its tint, the path's colour for a Taken fill, or `kStripFillGrey` #5a5e66 for the rest (line 684), the activations as bars along the bottom (`.oa`, lines 107 and 112), the window's frame (`.frame`, line 108: a 2 px white border, 7% white fill, at least 4 px wide) and a 1 px playhead (`.oph`, line 109). The strip is one `InvisibleButton` named `##strip`; a press on the frame starts a drag that moves the window with the pointer (lines 718 to 723); a press elsewhere centres the window there (line 726). The events come back as `TimelineEvents::view` (the new view, when anything moved it).

The scrollbar. Under the rows, in the label column the dim text `kTimelineScrollLabel` "Scroll (or mouse wheel)" (line 249, `log_text`), then a button one window back, the bar `##scrollbar` (`.sb`, line 118: 1.1 rem tall, rounded) whose thumb is `thumb()` (`.thumb`, line 119, at least 1.2 rem wide) and can be dragged, a button one window forward, and the readout `readout()` as `ImGui::TextUnformatted` in the muted colour, right-aligned in `kReadoutMinRem` = 9.5 (line 121). The buttons are `kScrollBackLabel` and `kScrollForwardLabel` (Q2 says their text), 1.8 by 1.3 rem (line 116). A click on the bar beside the thumb moves one window (line 711). The mouse wheel over the rows or the strip moves a third of a window a notch (line 734), through `ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY)` on the rows item so the window's own scroll does not take it.

Questions and owners: "which fill is under the pointer" (the layout's hit boxes); "what did the user ask the view to do" (the events; TV-1 does the arithmetic); the strip's colours are the fills' tints and `kTimelinePathColor`, no new map.

Tests, `test_preview_timeline.cpp`, pinned from one run: `a point on a fill names that fill` changes to `preview timeline: a point anywhere in the Fills row names the nearest fill, halfway to its neighbours` (two fills; a point past the midpoint names the second; a point in the Fills row left of the first names the first; a point in the Activations row still names a band or a time); `preview timeline: a fill outside the window has no item and a band half outside keeps its visible half`; `preview timeline: the window moves every x by the view's start` (the same scene at two starts); `preview timeline: the strip lays every fill and activation over the whole song and frames the window`; the existing stagger-free tests keep passing, adjusted for the view argument (a view over the whole song reproduces today's numbers).

Run: `hydra_tests.exe "-sf=*test_preview_timeline.cpp"`; `"-sf=*single_owner*"` (the scan row `ui/pb-timeline-drawing.json` must pass: every draw-list call stays in this file).

Done when they pass. Blockers: TV-1 (earlyStart from its executor commit); Q2 answered.

### Wave 2

#### SC-4b. The screen's view state: following, scrolling to a row's fill, the strip and scrollbar wired

Owns `src/pathbuilder/pb_window.h`, `src/pathbuilder/pb_window.cpp`, `tests/pb_ui/pbtest_window.cpp`.

What it builds. `PbApp` keeps a `TimelineView view_`, `bool follow_` and `double last_position_ms_`. At the chart's open the view starts at 0 with `kTimelineWindowMs` clamped to `playback_end_ms` (Q1). Each frame, before the timeline draws: if `playing()` went from false to true since last frame, `follow_` = true (line 985, `togglePlay`); while playing and `follow_`, `view_ = follow_playing(view_, position_ms)` (line 362); if not playing and the position changed since last frame and the change was not the hover's own seek or restore (the hover block marks the frames it seeks in), `view_ = show(view_, position_ms)` (line 813: a scrub, "-5s", "Act >" or a timeline click that leaves the window moves it). Any `TimelineEvents::view` from the strip, the scrollbar or the wheel replaces `view_` and sets `follow_` = false while playing (line 359, `byUser`). A list row whose fill is outside the window centres the window on it on hover (line 755) and on click (line 754); the window then stays there when the pointer leaves (the mockup restores nothing). A click on a fill keeps the view where the hover put it. The timeline gets `view_` in its input.

Questions and owners: "is the window following the playhead" (`follow_`, here); every move's arithmetic is TV-1's.

Tests, `pbtest_window.cpp`: `scroll-the-timeline` on a corpus chart longer than 3 minutes (`Symphony X - Set the World on Fire`, 5:58 by its `song.ini`; the executor pins its values from one run): the readout reads "0:00 – 3:00 of 5:58" at the open; the forward button makes it "2:58 – 5:58 of 5:58" (the clamp); the back button "0:00 – 3:00 of 5:58"; a wheel notch over the rows (`ctx->MouseWheelY(-1)`) reads "1:00 – 4:00 of 5:58"; a click at the strip's three-quarter point reads a window centred there; hovering the list row of a fill past 3:00 moves the readout to a window centred on that fill and the fill's item `##fill_<tick>` exists (`ItemInfo`); a press on Play, then the playhead past three quarters (seek there through the scrub by `seek_ms` is not a screen action, so the test presses "+5s" until the readout changes, capped at the window's length in presses) reads a window with the playhead a quarter in; a wheel notch while playing stops the following (the readout stays after the next page point), and Play again resumes it. `open-starts-empty` gains: the readout "0:00 – 2:10 of 2:10" on Burnout (Q1), and the strip's "Whole song" and "Scroll (or mouse wheel)" on screen.

Run: `hydra_pathbuilder_uitest.exe --test scroll-the-timeline --test open-starts-empty --test no-star-power --test file-open-error --test finish-for-me --test tight-window-note --test paste-refused-keeps-path`.

Done when they pass. Blockers: TL-2 and SC-4a (merged, or their executor commits merged together for an earlyStart).

#### TS-5. The click-through tests, updated and extended

Owns `tests/pb_ui/pbtest_build.cpp`.

What it builds. `build-a-path-by-clicking` reads the card instead of the six sentences (the words `fill-card` pins), and after the row hover asserts the Play button still reads "Play" when the pointer leaves (nothing resumes). A new `build-a-path-on-a-long-song` on the chart `scroll-the-timeline` uses: open, read the readout, press the forward button, read the new readout, hover a fill past 3:00 by its item name and read its card heading (a clock past 3:00), click it, `wait_for_edit`, read its compact step row "mNN → mMM ..." and the "Your score" figure (pinned from one run), then press "< Act" and read the clock at that activation. Every assertion reads the screen (`visible_text`, `ItemInfo`), never `PbState`.

Run: `hydra_pathbuilder_uitest.exe --test build-a-path-by-clicking --test build-a-path-on-a-long-song`.

Done when they pass. Blockers: SC-4b (earlyStart from its executor commit; the two merge together).

#### RA-6. The real-app check (the main session, after SC-4b and TS-5 merge)

A check, not a build, capped at 20 minutes. As the last plan's RA-3: a throwaway check branch from `claude/pb-redesign` plus `claude/pb-hy1`, built, run from a scratch folder on fresh copies of the user's `hydra.db`, `hydra_settings.ini` and `hydra_ui.ini` with installed Hydra closed. Two songs, opened by search, never by scrolling the library: the album (Divine Inner Tension) and The Reaper. On the album the user scrolls (wheel, strip, scrollbar, buttons), hovers fills in the row's empty space, reads a card, plays and watches the page turn, scrolls by hand while playing, presses Play again, hovers a list row whose fill is off screen, clicks a row, and reads how long the list takes to fill at their own depth (the pricing numbers above are at the default depth). On The Reaper (under 3 minutes) the whole song fills the width (Q1). Each number on screen is checked against the data before it is called right or wrong (`hydra_replay score` with the built path, as `answers.md`'s check did). The user says yes or names what to change before DOC-7 starts.

#### DOC-7. The last plan's DOC-2, plus this redesign

Owns what the last plan's DOC-2 owns: `docs/superpowers/specs/2026-10-28-path-builder-design.md` (a dated section naming the second and third mockups as the spec), `docs/audit/2026-10-03-fix-decisions.md` (one D-record numbered after the last on `main`, checked again at the catch-up merge, with the user's four answers from `answers.md`, the design requirement and the answers to Q1 and Q2 in their words), `CONTEXT.md`, `docs/agents/ui-testing.md` (the builder's uitest, the item names `##fill_<tick>`, `##choice_<tick>`, `##strip`, `##scrollbar`), `docs/UserGuide.md` and `README.md` (none of the four says anything about the Path Builder today: a grep for "Path Builder" over them on 2026-10-31 finds nothing). The User Guide's Path Builder section describes the window, the strip and the scrollbar in the constants' words. The two public pages get the doc review.

Run: `hydra_tests.exe "-tc=docs match code*"`.

Done when it passes and the reviewer signs off. Blockers: RA-6's yes.

#### REV and the merge (the main session)

After DOC-7 merges into `claude/pb-redesign`: the catch-up merge with `main` (keep both D267 and D268 as the last plan says; renumber the D-record if `main` moved), one fresh review of the whole `main...claude/pb-redesign` range, then `claude/pb-redesign` to `main`, then `claude/pb-hy1`. Each merge runs what `hydra_precheck.exe --tests` prints plus the three repo-wide checks (CLAUDE.md rule 1).

## Waves, in one line each

TV-1 and TX-3 start together at the user's yes. SC-4a forks from TX-3's commit; TL-2 forks from TV-1's commit (after Q2). SC-4b forks from TL-2 and SC-4a together. TS-5 forks from SC-4b's commit and merges with it. RA-6, then DOC-7 after the user's yes, then REV and the merges.

## Out of scope (for the user to confirm)

- The mockup's "Showing the fill you're pointing at (paused)" text on the highway (line 973): still out, as the last plan said; the Preview draws no text on the highway.
- The bar's rejected options: a fixed width per measure (question 1), always or never following (2), a strip without fills or with the frame only (3), a strip click that also moves the clock (4), click-only row scrolling (5), a wheel that does nothing (6), a row click that leaves the clock (7), a side column that narrows (8), stripes that count End (9), a caption cut with "…" (10). Each was "one click away" and the user approved the page without them.
- Zooming (a window of any other length): the approved page has one window length.
- Keyboard scrolling of the timeline: the mockup has none.
- Any change to the candidate queue: the numbers above say it is already on demand.
- Everything in the earlier plans' "Out of scope" lists.

## Open questions for the user

**Q1. A song shorter than the window.** The mockup's `clampView` (line 354) keeps the window's start at 0 when the song is shorter than 3 minutes, and `fr` (line 355) still divides by the 3-minute span, so a 2:10 song fills 72% of the track and the rest is empty; the readout would read "0:00 – 2:10 of 2:10" either way. Example one way: Burnout (2:10, the GUI tests' chart) and The Reaper draw as they do today, over the whole width, because the window is the song's length when the song is shorter than 3 minutes. Example the other way: Burnout's timeline stops at 72% of the width and the strip's frame covers the whole strip. Recommendation: the window is the smaller of 3 minutes and the song's length (`min(kTimelineWindowMs, length_ms)`), so every song under 3 minutes keeps today's layout and the scrollbar's thumb fills its bar. The case against: it is not literally what the page's script does, though the page never showed a short song.

**Q2. The scrollbar's arrow buttons.** The mockup writes them as the glyphs ◀ and ▶ (`&#9664;`, `&#9654;`, line 249). Hydra's own transport writes its arrows as "<" and ">" (`< Act`, `Act >`, `< 10 ms`, `10 ms >`), and the font Hydra loads is not checked for those two glyphs (the atlas is built in `src/ui/imgui_setup.cpp`; nothing in `src/ui/` names a glyph range for the geometric shapes block). Example one way: the buttons read "<" and ">", as the row above them does, and a GUI test clicks `**/<##scrollback`. Example the other way: the buttons read "◀" and "▶", and if the font lacks the glyph they draw as "?". Recommendation: "<" and ">", one home for how an arrow is written on this screen; the mockup's "Back one window" and "Forward one window" titles (line 249) become their hover hints.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| Hovering a fill pauses and shows it; moving off jumps back and stays paused; only Play or Space starts sound | answer 1 | SC-4a, TS-5 | `hover-shows-never-plays` (Play reads "Play" after leave, clock back); `build-a-path-by-clicking`'s row-hover leave read; reviewer grep: `hover_return_playing_` gone |
| The pointer picks the nearest fill anywhere in the Fills row, halfway to its neighbours; dots look the same | answer 2 | TL-2 | `a point anywhere in the Fills row names the nearest fill...`; reviewer: the draw's radii (`kFillRem`, `kLockedRem`, `kCantRem`) unchanged |
| The tooltip is a heading, three labelled figures (the cost in its tier colour) and grey lines | answer 3 | TX-3, SC-4a, TS-5 | `fill card: ...` tests; `fill-card` reads the words on screen; the card reads in `build-a-path-by-clicking` |
| A costless fill's card reads "best" and "This is the best choice now."; taken, locked and refused fills get the heading and one line | `answers.md`, after the four answers | TX-3 | `fill card: a costless fill...`; `fill card: taken, locked, refused and pending...` |
| The timeline shows a 3-minute window | the user's approval of the page with the 3-minute window | TV-1, TL-2, SC-4b | `kTimelineWindowMs` names line 312; `open-starts-empty`/`scroll-the-timeline` readouts |
| The scrollbar: a draggable thumb, a click beside it moves one window, the two buttons move one window, the readout reads "6:00 – 9:00 of 42:02" | `answers.md`, the scrollbar paragraph | TV-1, TL-2, SC-4b | `timeline view: a page...`; `timeline view: the readout...`; `scroll-the-timeline` reads the readout after each button |
| The wheel moves a third of the window a notch | same paragraph; line 734 | TV-1, SC-4b | `timeline view: a page, a wheel notch...`; the wheel step in `scroll-the-timeline` |
| The strip shows sections, tinted fills, activations and the frame; a click centres, dragging the frame scrolls | bar questions 3 and 4 | TL-2, SC-4b | `the strip lays every fill and activation...`; the strip click in `scroll-the-timeline` |
| While playing the window turns a page past three quarters, until the user scrolls; Play turns following back on | bar question 2; line 362 | TV-1, SC-4b | `timeline view: the playhead turns a page...`; the play, wheel and Play-again reads in `scroll-the-timeline` |
| A seek that leaves the window moves it (the clock a quarter in) | line 813 | TV-1, SC-4b | `timeline view: show moves only when the moment is outside`; the "< Act" read in `build-a-path-on-a-long-song` |
| A list row whose fill is off screen scrolls the timeline to it on hover or click | bar question 5 | SC-4b | the row hover in `scroll-the-timeline` (readout moves, fill item exists) |
| A row click moves the Preview's clock to the fill | bar question 7 | SC-4a | `row-click-moves-clock` |
| The side column stays 27 rem | bar question 8 | none (built: `kSideColumnRem`, `pb_window.cpp` line 42) | reviewer grep of the constant; RA-6 |
| Section stripes alternate over the sections shown | bar question 9 | none (built: `preview_timeline.cpp` lines 380 and 596) | `the Section row leaves out a section named End` (existing) |
| The saved caption wraps if it must | bar question 10 | none (built: `draw_saved_caption`) | `open-starts-empty` reads "needed to activate" |
| Pricing only as needed: one search per listed fill, in chart order, cancelled by a newer request; nothing stored per path combination | the user's design requirement | none (built: `pb_coach.cpp` lines 240 to 350) | `candidate queue: it prices only the ticks it is given, in order`; `candidate queue: a newer request supersedes the older`; RA-6 reads the list filling on the album |
| Visible fills and the top of the list are priced first | brief | none | the queue's chart order from the last activation (line 245) and the click keeping the view on the fill (SC-4b); RA-6 |
| Nothing priced that no screen element needs | brief | none | every candidate is read by a list row, a timeline tint and a strip tint (TL-2's strip test reads every fill's tint) |
| Canvas text a test reads goes through `log_text` or an ImGui text call | brief | TL-2 | the readout, "Whole song", "click or drag the white frame" and "Scroll (or mouse wheel)" read in `scroll-the-timeline` |
| Every new size names its mockup line | brief | TV-1, TL-2, SC-4a | reviewer reads each constant's comment |
| A click-through test scrolls past 3 minutes, activates there, reads the result | brief | TS-5 | `build-a-path-on-a-long-song` |
| Every assertion reads the screen | brief | SC-4a, SC-4b, TS-5 | reviewer greps the three test files for `h.app->state` (none new) |
| No new number without the user | brief | Q1, Q2 | the questions; executors stop and ask |
| The user sees it in the real app on the album and The Reaper before DOC-7 | brief | RA-6 | the user's yes in chat |
| Numbers on screen are checked against the data | memory `check-the-screen-against-the-data` | RA-6 | `hydra_replay score` on the built path |
| DOC-2's documents and the catch-up merge come last; D-numbers from `main` | brief, last plan | DOC-7, REV | `docs match code` passes; the reviewer's sign-off on the range |
| No whole-library run | preamble | every task | every test names a corpus chart; RA-6 opens two songs by search |
| Agents never run the full suite | rule 1 | every task | each task's Run line |
