# Plan: the Path Builder rebuilt to the second mockup (2026-10-31)

Task id `PBR2-PLAN`, session `8bb2c4a3-7349-4459-aacd-89793eb6fbd7`. Planner: Fable. Branch `claude/pbr2-plan`, cut from `claude/pb-redesign` (5efa659e) plus the mockup commit (75547840). Status: plan written; nothing is built. Every file and line named below was read in this worktree on 2026-10-31.

The requirement is the user's own words in `docs/superpowers/specs/2026-10-31-path-builder-mockup-2/answers.md`. The behaviour spec is `mockup_template.html` in that folder (its script). Why the work exists is `docs/handoffs/2026-10-30-path-builder-real-app-check-2-handoff.md`: six problems the user found in the real app. The first redesign's plan, `docs/superpowers/plans/2026-10-30-path-builder-redesign.md`, holds every earlier answer; those still hold unless this plan says `answers.md` replaces them ("What the new answers replace" below).

## The plain version

The first rebuild put the right facts on screen but the user could not read them, and it left most of Hydra's Preview out. The user's last words were "yeah no this is unreadable. we need a new approach." The second mockup fixes that in four ways, and the user approved all four defaults: costs show as colours on the fills plus a sorted "Where to activate next" list; fills before your last activation get a small locked dot instead of the ring; the saved row's caption moves to the left and the stretches where you have enough saved are shaded teal; and the whole of Hydra's Preview panel sits under the timeline with a narrow column beside it for the list and your activations.

So this plan does three builds and one check. One task moves the body of Hydra's Preview panel into shared code, so Hydra's tab and the builder draw the same thing. One task changes the timeline: text at the user's font size, tinted fills with only "best" written under one, the locked look, the caption and the shading. One task lays the builder out around the shared panel, adds the list, makes the steps compact rows, and drops the fixed 600 px highway. Then the click-through test is brought up to date, the user sees it in the real app on The Reaper, the documents are written, one fresh reviewer reads the whole branch, and it merges.

## What is already there and what is missing, checked in the code

The Preview panel is one function, `render_preview_panel` in `src/ui/preview_tab.cpp` (line 352 to 839). It takes Hydra's whole `AppState`. What it reads from it: the controller (`app.preview_controller()`), the selected song and its settings for `pc->open` (lines 365 to 382), the overlay-key cache and the "Show in Preview" jump in `app.details_ui` (lines 374 to 430), the path picker (`render_path_picker`, line 136), and the volume (`app.settings.preview_volume`, read at line 491, written and committed at lines 495 to 498). Everything from the "< Act" button (line 456) to the key bar (line 838) reads only the controller, `Settings::clamp` and the theme. That is the body that moves. `preview_tab.cpp` is in `hydra_ui` (`CMakeLists.txt` line 552), which the builder never links (line 663: "never hydra_ui"). The helpers it uses, `begin_disabled_button`, `hint`, `button_in_slot`, `text_in_slot`, `overflow_tooltip`, `widest_digits`, `fits_in_row`, live in `src/ui/theme.h` and `src/ui/widgets.h`, which are already in `hydra_uibase` (lines 513 to 531). `PreviewController` is built in `hydra_preview` (line 483), which `hydra_uibase` does not link today (line 536); SP-1 below says how that is handled.

The builder draws only a highway image, a Play button and a clock (`PbApp::draw_highway`, `src/pathbuilder/pb_window.cpp` lines 623 to 670), in a child fixed at `kHighwayW` = 600 px (line 38 and line 384). That is problems 3 and 4 of the handoff, confirmed.

The timeline's text is sized in fixed pixels: `font_for` in `src/ui/text_role.h` lines 84 to 86 returns 10.5, 11.5 and 11 for `TimelineCost`, `TimelineSection` and `TimelineAxis`, while every other scaling role returns 0 ("the user's own size", line 52). `tests/test_text_role.cpp` lines 48 to 50 pin those three sizes, and the fonts plan lists them at lines 60 to 62. The cost labels and their stagger are `TimelineCostLabel`, `cost_labels` and `cost_clashes` in `src/ui/preview_timeline.h` (lines 158 to 203) and the layout block at `preview_timeline.cpp` lines 320 to 354, with four tests in `tests/test_preview_timeline.cpp` (lines 235, 269, 284 and 309 in part). Every size there is a CSS pixel from the first mockup, drawn through `px` (`preview_timeline.cpp` lines 18 to 139), and `px` scales by the monitor's DPI only (`src/ui/fonts.h` line 23), never by the font size.

The fills have three looks (`preview_timeline.cpp` lines 543 to 554): offered, taken, and one ring for everything else. The screen hands the timeline `cant_ticks` for every Offered fill that `fill_reason` refuses (`pb_window.cpp` lines 534 to 536), so a fill before the last activation and a fill under two bars both get the ring. `fill_reason` (`src/pathbuilder/pb_text.cpp` line 72) reads its facts in this order: the fill is Taken; Star Power is running at the fill's end (`sp_running_at`); the fill is before the last activation; the fill is Offered; the meter is under `kSpActivationBars` (line 89); otherwise the game hides it. That order is what the locked look splits: "before the last activation" is the builder's own rule (the left-to-right ruling, Q1 of the first plan), the rest are the game's. The saved row's caption, "2 phrases: enough to activate", is drawn at the row's right end (`preview_timeline.cpp` lines 488 to 492, `kActivateLabelRight`), which is problem 6.

The candidate costs the list needs already exist: `DecisionList::candidates()` (`src/pathbuilder/pb_coach.h` line 346) holds every priced fill in chart order with its `CandidateCost` (`src/app/path_costs.h` line 71: tick, best finish, cost, tier), and `candidate_end(i)` (line 349) gives the activation Hydra would make there, whose deact node is "ends m36" and whose `badge_timings` are the squeeze. The queue prices the ticks of `PreviewScene::trailing_skipped_fill_ticks` one at a time (line 186), so a row's cost lands at its own moment. `cost_tier` (`path_costs.cpp` line 20) is the one owner of the tiers: 0 is green, a cost at or over 1% of BF(0) is red (`100 * cost >= bf0`), between is yellow.

"< Act" and "Act >" already jump between the drawn path's activations: `PreviewController::jump_activation` (`src/ui/preview_controller.cpp` line 521) reads `scene_.activations`, which in the builder are the player's windows, and `has_activations` (line 517) greys the buttons when the shown path has none, which is the empty path's case. Nothing new is needed there.

The hover rule is built: hovering a fill pauses, seeks to the fill's span start and restores on leave (`pb_window.cpp` lines 547 to 581, the user's answer Q4). A list row reuses it.

Prior art. Chess analysis boards show the engine's candidate moves as a list ranked by evaluation (MultiPV): Lichess's analysis board lists up to five top moves in order, and the Obsidian Chess Studio guide describes the same panel, where clicking a line applies it and the top move is drawn on the board as an arrow (both read 2026-10-31: https://www.mintlify.com/lichess-org/lila/features/analysis and https://mintlify.com/luisrivasnoriega/Obsidian-Chess-Studio/guide/game-analysis). The "Where to activate next" list follows that shape: candidates ranked by cost, the best first, each one applied by a click and shown in place (here by hover on the highway) before it is taken. No source I found previews a candidate on hover; that part is the mockup's own, which the user clicked through.

## What the new answers replace

`answers.md` replaces these earlier answers; everything else in the 2026-10-30 plan's "The user's answers" stands.

- PV-4 decision 1 (cost labels stagger to a second line): gone. No number sits under any fill; only the best fill says "best" (answer 1).
- PV-4 decision 2 (the three roles at the mockup's pixel sizes): gone. The roles follow the user's font size (answer 1's last paragraph: "must follow the user's font size like the rest of Hydra, not fixed pixels"). PV-4 decision 3 stays where it still applies: section names end in an ellipsis, and axis labels never leave the timeline.
- PB-5 decision 2 (`kHighwayW`, the 600 px highway beside the steps): gone. The Preview panel takes the width and a column of about 27 rem sits beside it (answer 4).
- PB-5 decision 6 (short costs like "−1.2k" under fills): gone with the labels. The list, the steps and the end choices write a cost in full, "−2,060", as the mockup's `costShort` does (`"−" + fmt(c)`, line 316 of the second mockup). `format_points_short` and `format_cost_short` in `src/app/display_format.h` then have no caller and go.
- The first plan's `kLegendCantText` ("Can't activate here (hover to see why)") keeps its words, but a new legend entry joins it: "Before your last activation" (answer 2).
- The first mockup's caption "2 phrases: enough to activate" at the row's right end becomes two captions in the label column: "2 phrases: needed to activate" after a dashed sample, and "enough to activate" after a teal swatch (answer 3). Nothing is written at the row's right end.
- PV-3 decisions 2, 3, 4 and 6 (row heights, the label column and the ring size in pixels) are replaced only if the user takes Q1's recommendation below; the plan does not assume it.

The "Open in Path Builder" button is on `claude/pb-hy1` (cfd1fe9c, checked with `git branch`) and merges after this work reaches `main`, as the handoff says. `main` is not an ancestor of `claude/pb-redesign` yet (`git merge-base --is-ancestor main HEAD` says no), so the catch-up merge the handoff names is still to do.

## Engine truth, not screen rules

Nothing in this plan computes a game fact on the screen. Which fills can be activated is the engine's list (`trailing_skipped_fill_ticks`, EN-4). Why a fill can't is `fill_reason`. A cost is `candidate_costs`. A tier is `cost_tier`. Where Hydra would end an activation is the candidate's activation from `candidate_end`. How much is saved at a moment is the Preview's meter curve (`SpMeterCurve` segments, `sp_meter_bars_at`). The shading is a reading of that curve against `kSpActivationBars`, in the timeline's pure layout, beside the saved line it already draws from the same segments. The locked look is the one builder-owned rule, left to right (Q1 of the first plan), and it is read from `fill_reason`'s facts, not re-derived.

## Tasks

Each task fits one Opus executor in 100 tool calls and 20 minutes, in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`, forked from `claude/pb-redesign` at the mockup commit or as "Blockers" says. No two tasks own one file. `CMakeLists.txt` is edited by SP-1 only. Each task runs only the tests it names. Tests pin literals from one run or call the production function. One fresh reviewer per task (D107). Every label the player reads is a named constant with one home. An executor that reaches its budget commits what builds and hands the rest to a fresh executor.

### Wave 1: SP-1 and TL-2 start together

#### SP-1. The shared Preview panel

Owns `src/ui/preview_panel.h` (new), `src/ui/preview_panel.cpp` (new), `src/ui/preview_tab.cpp`, the `hydra_uibase` block of `CMakeLists.txt` (lines 513 to 540), and the scan rule files that name `preview_tab.cpp` as an owner: `tests/single_owner/rules/ui.json`, `tests/single_owner/rules/ui/concept-F5.json`, `tests/single_owner/rules/ui/concept-G7.json`, `tests/single_owner/rules/ui/text-alignment.json`.

What it builds. The body of `render_preview_panel` from the audio-warning line (`preview_tab.cpp` line 410) to the key bar (line 838) moves, unchanged in look, into `ui::draw_preview_panel(PreviewController& pc, PreviewPanelState& s)` in `hydra_uibase`, with the anonymous-namespace helpers it uses (`kJumpSeconds`, `kStepMs`, `key_hints`, `name_box`, `draw_text_box`, `overlay_box_sizes` and the rest of lines 33 to 345). The chart-changed line (line 418) stays in Hydra's tab, because its words tell the user to click the song again. `PreviewPanelState` carries the one setting the body reads, the volume: `int volume_pct`, and two bools the function sets, `volume_changed` (the slider moved this frame) and `volume_committed` (`IsItemDeactivatedAfterEdit`). The function also reports a click on the highway image as `image_click` (the `render::ImagePoint` and the image's width and height), so the builder's highway click through `fill_at_image` keeps working; Hydra ignores it. The panel draws the image as the builder does today (an `InvisibleButton` named `##highway_image` with the image over it, `pb_window.cpp` lines 656 to 663) so one named item serves both the click and the GUI tests that place the highway (`uitest_preview.cpp` line 354).

Hydra's tab keeps everything above line 410 and the picker, and calls the panel; on `volume_changed` it writes `app.settings.preview_volume` through `Settings::clamp` and calls `pc.set_volume`; on `volume_committed` it calls `app.commit_settings()`. So Hydra's settings file is written exactly when it is today. The builder's side is Q2.

Where it links. `PreviewController` is compiled in `hydra_preview` (`CMakeLists.txt` line 483), which `hydra_uibase` does not link (line 536). The panel needs it, so `hydra_uibase` gains `hydra_preview` as a PUBLIC link. That keeps the base's rule ("Nothing here reads AppState", line 512): `hydra_preview` reads no `AppState` and no ImGui. The executor runs `"-sf=*test_layering.cpp"` first; if it refuses the link, the executor stops and reports rather than moving the panel into another library.

Scan rows. Four rule files name `preview_tab.cpp` as the owner of things that move (`ui.json` lines 1020 and 1058, `concept-F5.json` line 19, `concept-G7.json` line 5, `text-alignment.json` line 46). They are re-pointed at `preview_panel.cpp`; no rule is loosened.

Questions and owners: "how loud is the Preview" (the caller's setting; Hydra's `Settings::preview_volume`, the builder's Q2); "what does the panel draw" (`draw_preview_panel`, reading the controller only); "where did the user click on the highway" (the panel reports it; `fill_at_image` on the controller answers which fill).

Tests. Hydra's three Preview GUI tests pass unchanged: `preview`, `preview-path-overlay`, `preview-controls` (`tests/ui/uitest_preview.cpp` lines 1092 to 1095; they find every item by wildcard, `**/< Act##prevact`, `**/##volume`, `**/##scrub`, `**/##score_box`, so the move changes no reference). `test_layering.cpp`. The single-owner scan.

Run: `build-cpp\Release\hydra_tests.exe "-sf=*test_layering.cpp"`; `"-sf=*single_owner*"`; `build-cpp\Release\hydra_uitest.exe --test preview --test preview-path-overlay --test preview-controls`.

Done when they pass with no change to Hydra's screen, and the reviewer's diff of `preview_tab.cpp` against `preview_panel.cpp` shows a move, not a rewrite. Blockers: none. earlyStart: at the user's yes.

#### TL-2. The timeline: font size, tinted fills, the locked look, the caption and the shading

Owns `src/ui/preview_timeline.h`, `src/ui/preview_timeline.cpp`, `src/ui/text_role.h`, `tests/test_text_role.cpp`, `tests/test_preview_timeline.cpp`, `docs/superpowers/plans/2026-10-22-font-role-single-owner.md`.

What it builds, in five parts.

The font. `font_for` returns size 0 for `TimelineCost`, `TimelineSection` and `TimelineAxis` (Q7 says why the roles stay). The test table in `test_text_role.cpp` lines 48 to 50 and the fonts plan lines 60 to 62 change with them, each line now saying the role follows the user's font size.

The fills. `TimelineInput::labels` goes, and with it `TimelineCostLabel`, `cost_labels`, `cost_clashes`, `cost_top`, `cost_line_h` and `cost_width`. In its place `TimelineInput` takes `std::vector<TimelineFillTint> tints`: a chord tick and the colour the caller chose for it (the screen passes `tier_color` of each candidate's tier, Q5). An Offered fill with a tint is drawn filled in that colour, with the ring the offered look has today; an Offered fill with no tint yet (its cost has not landed) keeps the teal `kOfferedColor`. One word, "best", goes under the fill whose tick the caller names in `TimelineInput::best_tick`, drawn in that fill's tint in the `TimelineCost` role, centred and kept inside the timeline as the axis labels are (`inside_start`). The layout places it as a `TimelineBestLabel` so a test can read it without ImGui. The Fills row no longer grows for a second line; its height is one value again. The hovered look also serves a fill the caller names in `TimelineInput::highlighted` (a list row's hover, Q6).

The locked look. `TimelineInput` gains `locked_ticks` beside `cant_ticks`. A fill in `locked_ticks` is drawn as the small grey dot of the mockup's `.fill.lock` (line 80 of the second mockup); the ring (`kCantSize`) now draws only `cant_ticks`. `LegendSwatch` gains `Locked`, drawn the same way, for the legend entry "Before your last activation". `TimelineFillItem` gains a `look` the layout decides (`Offered`, `Taken`, `Cant`, `Locked`), so the test reads it and the draw reads nothing else.

The caption. The saved row's label column holds three lines: "Star Power saved", then a dashed sample (drawn with `draw_dashed` and `kActivateDash`) followed by `kTimelineActivateLabelTail`, whose words become " phrases: needed to activate" with the number from `kSpActivationBars`; then a teal swatch followed by `kTimelineEnoughLabel` = "enough to activate". The right-end label and `kActivateLabelRight`, `kActivateLabelRise` go. The label column must be wide enough for those lines (Q1).

The shading. `timeline_layout` adds `saved_enough`: the rectangles, over the saved row's full height, of every stretch after the last activation where the saved line is at or above `kSpActivationBars`. "After the last activation" means after `scene.activations.back().sp_window()->second`, or from 0 ms when the scene has no activation (the empty path). The stretches are read from the meter curve's own segments: a segment with `start_bars` and `end_bars` both at or above the line is wholly enough; one that crosses the line is split at the crossing by linear interpolation along the segment, which is how the segment is drawn; a segment below is none. Adjacent stretches merge. The draw fills them in the teal of `kOfferedColor` at the mockup's alpha (Q1) under the activate line and the curve. With no activation at all, every stretch at or above the line is shaded, which is right: the player can activate anywhere there.

Questions and owners: "what colour is this fill" (the caller's tint, from `cost_tier`); "which fill is best" (the caller's `best_tick`, from `candidate_costs`); "is this fill locked or can't" (the caller's two lists, from `fill_reason`'s facts; the timeline decides nothing); "where is enough saved" (the meter curve against `kSpActivationBars`, read once in `timeline_layout`).

Tests, `test_preview_timeline.cpp`: the three stagger tests (lines 235, 269, 284) are deleted with the feature; `preview timeline: a tinted fill reads its tint and an untinted offered fill reads none`; `preview timeline: only the best fill has a word under it, kept inside the timeline` (a fixture with a best tick at the left edge, so `inside_start` is exercised); `preview timeline: a locked fill is drawn as locked and a cant fill as cant` (the `look` field); `preview timeline: the saved row's enough stretches start after the last activation and split a segment at the line` (a fixture with one segment crossing 2 bars after the last window and one wholly above it before the window: one rectangle, pinned x from the crossing); `preview timeline: with no activation every stretch at or above the line is shaded`. `test_text_role.cpp`: the table rows at size 0.

Run: `hydra_tests.exe "-sf=*test_preview_timeline.cpp"`; `"-sf=*test_text_role.cpp"`; `"-sf=*single_owner*"` (the `activate-line.json` rule allows `kSpActivationBars` in `preview_timeline.cpp` already; the executor checks the shading's comparison is covered by that row's allow list and stops if it is not).

Done when they pass. Blockers: none for the code; Q1, Q5, Q6 and Q7 are answered before it starts. earlyStart: at the user's yes.

### Wave 2

#### SC-3. The screen around the shared panel, with the list and compact steps

Owns `src/pathbuilder/pb_window.h`, `src/pathbuilder/pb_window.cpp`, `src/pathbuilder/pb_text.h`, `src/pathbuilder/pb_text.cpp`, `src/app/display_format.h`, `src/app/display_format.cpp`, `tests/test_pb_text.cpp`, the test file that pins `format_points_short` (the executor greps `tests/` for it), `tests/pb_ui/pbtest_window.cpp`, `tests/pb_ui/pbtest_coach.cpp`. It adds no new source file, so the two pathbuilder scan rows that list files by name (`tests/single_owner/rules/pathbuilder/activate-line.json`, `ui/pb-timeline-drawing.json`) need no edit.

What it builds. The window in answer 4's order: title and mode with the three scores and Undo, Start over, Finish the rest for me on one head row; the intro line; the guide line; the timeline panel (legend with the new "Before your last activation" entry, then the `PreviewTimeline` as TL-2 draws it: `tints` from `d.candidates()` through `tier_color`, `best_tick` from the first candidate that `costs_nothing`, `cant_ticks` and `locked_ticks` split by `fill_reason`'s facts, see below). Under it, two children side by side: the shared Preview panel (`ui::draw_preview_panel`, SP-1) taking the width that is left, and a side column of the width Q1 gives, holding "Where to activate next" above "Your activations". `kHighwayW`, `kHighwayHeading`, `kPlayLabel`, `kPauseLabel` and `draw_highway` go; the panel owns Play, the clock and the keys, so `pb_window.cpp` line 647 (its own Space handler) goes too. The panel's `image_click` goes to `fill_at_image` and `click_fill` as today. Escape still clears the selected step.

The volume for the builder follows Q2.

Locked against can't. `pb_text` gains `enum class FillRefusal { None, Taken, SpRunning, BeforeLast, UnderBars, GameHides }` and `fill_refusal(scene, index)`, which reads the facts in the order `fill_reason` reads them today; `fill_reason` becomes the sentence for each kind, so there is still one owner of the order. The screen puts `BeforeLast` fills in `locked_ticks` and `SpRunning`, `UnderBars`, `GameHides` in `cant_ticks`. A click on a locked fill puts `kBeforeLastActivationText` in the guide line through `cant_activate_message`, as today. Nothing else about clicks changes.

The list. `draw_choices` draws "Where to activate next" (`kChoicesHeading`) and the hint "Cheapest first. Hover a row to see it on the highway; click it to activate there." (`kChoicesHint`). Each row is one `Selectable` named `##choice_<chord_tick>` so a GUI test clicks it as it clicks a fill. A row reads, left to right: a dot in the tier's colour, the fill's name (`fill_name`), then `candidate_row_text` from `pb_text`: the clock (Q4), the phrases saved (`bars_saved_at`, capped at the meter's cap as the mockup's `Math.min(st.saved, CAP)`), "ends m36" from `candidate_end`'s deact node, and the squeeze in the short form of the mockup's `sqShort` ("squeeze-out 12.3 ms", "squeeze-in, no timing") built from `badge_timings` and `format_ms` as `squeeze_end_text` is; then the cost at the right, "best" or `format_cost`. Which rows exist before every cost lands, and how they order themselves, is Q3. When the engine lists no fill after the last activation the panel says "No fill after your last activation can be activated. Your path is finished." with a path, or "No fill can be activated." with none (both named constants from the mockup's `renderChoices`).

Hovering a row does what hovering its fill does: the row's hover sets the same `hovered_fill_` the timeline's hover sets, so the pause, the seek to the fill's span start and the restore on leave are the one block at `pb_window.cpp` lines 547 to 581, now fed from either place; the timeline gets the row's fill as `highlighted`. The fill's tooltip stays the fill's own; a row shows no tooltip (the mockup shows none on a row). Clicking a row calls `click_fill` with the fill's index.

The steps. Each activation is a compact row: its number in a circle (`draw_star` is not used; a filled circle in `kPathColor`), then `step_short` from `pb_text`: "m32 → m36" (the activation's `fill_name` and the `measure_label` of its deact node), the phrases it spent ("2 phrases", from the bars the step sentence already reads), and the cost at the right, "best" or `format_cost`, in the tier's colour. The selected row opens to the full `step_sentence`, its `why` line and violet `note`, then on the last activation the end choices and Remove, as today (`draw_steps` lines 727 to 757 keep their logic). The leftover line and the glossary stay under the steps. `kStepsHeading` stays "Your activations".

Questions and owners: "which look does this fill get" (`fill_refusal`); "what does a list row say" (`candidate_row_text`, reading `candidate_end` and the meter); "what does a compact step say" (`step_short`); nothing else new; costs, tiers, sentences and the Preview are read from their owners.

Tests. `test_pb_text.cpp`: `fill refusal: the five kinds in fill_reason's order, and fill_reason's sentence for each`; `candidate row: the clock, phrases, end and squeeze of a priced fill` (a corpus chart, pinned from one run); `step short: start, end, phrases and cost`. `pbtest_window.cpp`: `open-starts-empty` gains three checks: the list heading is on screen, the Preview's "< Act" button is on screen and disabled with no activation (`ItemInfo` flags), and the text holds no "Highway". `pbtest_coach.cpp`: `cant-activate-reason` stays; a new `locked-fill-says-left-to-right` clicks a fill before the last activation after one `click_fill` and reads `kBeforeLastActivationText` in the guide line. `tests/single_owner/rules` scan unchanged.

Run: `hydra_tests.exe "-sf=*test_pb_text.cpp"`; the display-format test file; `"-sf=*single_owner*"`; `build-cpp\Release\hydra_pathbuilder_uitest.exe --test open-starts-empty --test no-star-power --test file-open-error --test finish-for-me --test tight-window-note --test cant-activate-reason --test locked-fill-says-left-to-right --test paste-refused-keeps-path`.

Done when they pass and `HydraPathBuilder.exe <corpus chart>` builds a path by clicking rows and fills. Blockers: SP-1 and TL-2 (merged, or their executor commits merged together for an earlyStart), and Q1 to Q7 answered.

#### TS-4. The click-through test, updated

Owns `tests/pb_ui/pbtest_build.cpp`.

What it builds. `build-a-path-by-clicking` still builds a whole path on Burnout by clicking on screen, and now: after the first fill click it reads the list and clicks the row `##choice_<kSecondFillTick>` instead of the fill for the second activation, then asserts the second step's compact row text ("m40 → m44") and the full sentence once selected (a click on the step row); it hovers a row and asserts the clock in the Preview's time slot moved to the fill's span start (read through `visible_text`), then moves off and asserts it moved back; it asserts "best" is on screen exactly once under the fills before any click (the pinned best tick); it clicks a locked fill after the second activation (the first fill, `kFirstFillTick`, is before the last activation then) and reads `kBeforeLastActivationText`; it presses "Act >" and asserts the time slot shows the first activation's clock. Every assertion reads the screen (`visible_text`, `ItemInfo`), never `PbState`.

Run: `hydra_pathbuilder_uitest.exe --test build-a-path-by-clicking`.

Done when it passes. Blockers: SC-3. earlyStart: fork from SC-3's executor commit.

#### RA-3. The real-app check (the main session, right after SC-3 merges)

A check, not a build, capped at 20 minutes. Make a throwaway check branch from `claude/pb-redesign` that also merges `claude/pb-hy1` (the one known conflict is `docs/audit/2026-10-03-fix-decisions.md`; keep both D267 and D268), build it, and run it from a scratch folder on fresh copies of `hydra.db`, `hydra_settings.ini` and `hydra_ui.ini` from `C:\Program Files\Hydra`, with installed Hydra closed (the recipe in the memories `ui-changes-need-real-app-check` and `branch-window-check-recipe`). Open one song, The Reaper, press "Open in Path Builder", and the user builds a path: read the list, hover a row, click a row, click a fill, read a compact step, open it, pick an end, Undo, Start over, Finish the rest for me, Play, "< Act". Also look at Hydra's own Preview on that song: unchanged. Before any number on screen is called right or wrong it is checked against the data (the memory `check-the-screen-against-the-data`; the mockup folder's `reaper-user-path-target.json` has the user's path and its costs). Nothing runs over the library; one song is read. The user says yes or names what to change before DOC-2 starts. The hover-pauses-the-song behaviour, never judged in a real window, is judged here.

#### DOC-2. The design, the record and the pages

Owns `docs/superpowers/specs/2026-10-28-path-builder-design.md` (a dated section "Redesign, 2026-10-31" naming the second mockup as the spec and the six problems it answers), `docs/audit/2026-10-03-fix-decisions.md` (one D-record at the next free number, with the user's four answers and every answer to this plan's questions in their words), `CONTEXT.md` (the Path Builder paragraph: shared Preview panel, timeline, list, compact steps), `docs/agents/ui-testing.md` (`##choice_<tick>` beside `##fill_<tick>`), `docs/UserGuide.md` (the "Path Builder" section, written against SC-3's constants), `README.md` (one line). The two public pages get the doc review.

Run: `hydra_tests.exe "-tc=docs match code*"`.

Done when it passes and the reviewer signs off. Blockers: RA-3's yes.

#### REV and the merge (the main session)

After DOC-2 merges into `claude/pb-redesign`: catch the branch up with `main` (keep both D267 and D268), then one fresh review of the whole `main...claude/pb-redesign` range as the handoff asks (the three main-session fixes f3b519c1, 28fa2471, 91e558d5, the plan edits and the merge commits have had no reviewer). Then the merge order: `claude/pb-redesign` to `main`, then `claude/pb-hy1`, then nothing else. Each merge runs what `hydra_precheck.exe --tests` prints plus the three repo-wide checks (CLAUDE.md rule 1).

## Waves, in one line each

SP-1 and TL-2 start together at the user's yes. SC-3 forks from both. TS-4 forks from SC-3's executor commit. RA-3 runs on SC-3's merge plus `claude/pb-hy1`. DOC-2 after the user's yes. REV, then the merges.

## Out of scope (for the user to confirm)

- The mockup's "Showing the fill you're pointing at (paused)" line drawn on the highway (`drawHighway`, line 792 of the second mockup). The Preview draws no text on the highway; the hover's tooltip says what is shown. Adding it is new Preview drawing.
- The "re-plan from there" option for locked fills: rejected in answer 2.
- The "colours only" option without the list: rejected in answer 1.
- Hydra's Preview tab showing the timeline rows (the first plan's Q5 still holds).
- A row tooltip on the list (the mockup shows none).
- Everything in the first plan's "Out of scope" list: the highway text labels, the squeeze-out ring, the "lost because the meter was full" note, a cap on the candidate queue.

## Open questions for the user

Each is answerable in a line. The code's real rule is quoted with it, and each ends with a recommendation. Q1 to Q7 are answered before TL-2 and SC-3 start; SP-1 needs only Q2.

**Q1. The second mockup's sizes and colours that no answer gives.** The timeline today is sized in CSS pixels from the first mockup through `px`, which scales by the monitor's DPI only (`src/ui/fonts.h` line 23): the sections row is 24 px tall (`kSectionsRowH`), the saved row 46 px, the label column 110 px, the ring 13 px (PV-3 decisions 2, 3, 4, 6). The second mockup sizes everything in rem, one unit being the user's font size (`html{font-size:15px}`, every rule in rem, lines 8 to 9: "so the whole screen follows one font size"). The build needs these numbers and no answer names them: the row heights (axis 1.2, section 1.5, phrases 1.4, saved 3.9, ghost 2, activations 2.2, fills 2.9 rem; mockup lines 451 to 476), the label column (12.5 rem, line 61; the three-line caption needs more than 110 px at any font), the fill (1.15 rem, line 72), the ring (0.8 rem, line 79), the locked dot (0.5 rem, `#4b4f57`, line 80), the hover scale (1.25, line 76), the shade (the glow colour at alpha .16 and the swatch at .28, lines 465 and 546), the side column (27 rem, line 91), the list's height limit (21 rem, line 126), the tooltip width (24 rem, line 87), the legend gap (1.1 rem, line 48). Example one way: at the user's font size, say 20 px, a section name in the `TimelineSection` role at size 0 is about 20 px tall and does not fit a 24 px row with 4 px padding each side; in rem the row is 30 px and it fits. Example the other way: keeping PV-3's pixels keeps the rows the user approved on 2026-10-30, and the text is cut. Recommendation: take the second mockup's rem values, each written as a multiple of `ImGui::GetFontSize()` in `timeline_metrics` and the builder, each constant naming its mockup line, replacing PV-3's four pixel decisions and PB-5's 190 px hint and 360 px tooltip. That is what the user clicked through.

**Q2. Who owns the builder's volume.** Hydra reads `Settings::preview_volume` (`src/app/config.h` line 171, default 40) and writes it to `hydra_settings.ini` beside the exe when the slider is released (`preview_tab.cpp` line 498, `AppState::commit_settings`). The builder already reads that same file once, at the chart's open (`PbState::open` calls `Settings::load()`, `pb_state.cpp` line 109), and has no code that writes it. Example one way: the builder starts at the file's 40, the player drags to 70, closes the builder, opens it again: 40. Example the other way: the builder writes 70 to the file while Hydra is open; Hydra later saves its own copy and the last writer wins, so one of the two programs loses a setting it never changed. Recommendation: the builder reads `preview_volume` from the `Settings` it already loads at the open, keeps it in `PbApp` for the run, and never writes the file. The D-record says so.

**Q3. What a list row shows before its cost lands, and how the list orders itself.** The queue prices the engine's fills one at a time in chart order (`candidate_finishes`, `pb_coach.h` line 186), and `candidates()` holds only the landed ones; on The Reaper the empty path has about 30 fills to price. The mockup lists priced fills only, sorted by cost then tick (`candidates()`, line 340). Example one way: the list starts empty and grows a row at a time, each new row sliding into its place by cost, so the list reorders under the pointer for the first seconds. Example the other way: every fill the engine lists has a row from the first frame, in chart order, with "..." (`kPendingText`) for its cost and its "ends" part; as a cost lands its row takes the figure and moves up among the landed rows, cheapest first, ties in chart order; pending rows stay below, in chart order. Recommendation: the second: the row count never changes after a click, and the player sees at once how many choices there are.

**Q4. The clock in a list row and a compact step.** The mockup writes a row's time as `clockShort` ("0:45", line 272) and the step sentence as `clock` ("0:45.123"). On 2026-10-30 the user chose Hydra's "0:45.789" for the step sentence ("Hydra's 0:45.789 (Recommended)"). Example one way: a row reads "m32 · 0:45 · 2 phrases · ends m36" and its tooltip "0:45.123". Example the other way: both read "0:45.123". Recommendation: `clock_str` everywhere, as the earlier answer chose, so one moment has one spelling on the screen.

**Q5. The tier colours on the fills.** `tier_color` in `pb_window.cpp` line 55 maps the three tiers to the Report's chip colours (`ChipToken::t0`, `t1`, `t3`), and the steps' costs already draw in them. The mockup's fills use its own `--green #4fb35a`, `--yellow #e0b83c`, `--red #e5534b` (line 12). Example one way: a fill and the cost beside its step are the same green the Report's chips use. Example the other way: the fills are the mockup's greens and the steps' costs the Report's, two greens for one meaning. Recommendation: the chip colours, through `tier_color`, so one owner gives every tier its colour on this screen and the Report.

**Q6. The highlighted fill while a list row is hovered.** The mockup draws it as the hovered fill plus a white outline 2 px wide, 2 px out (`.fill.hl`, line 77). The timeline already has a white outline for the selected band: `kSelectedW` 2 px, `kSelectedOffset` 1 px (`preview_timeline.cpp` lines 111 to 113). Example one way: the fill grows as on hover and gets the band's outline, 1 px out. Example the other way: a new 2 px offset constant. Recommendation: reuse the band's outline; no new number.

**Q7. The three timeline roles.** `font_for` has 15 roles; the three timeline ones are the only scaling roles with a pixel size (`text_role.h` lines 84 to 86). At size 0, `TimelineCost` equals `Figure` and `TimelineSection` equals `Label`. Example one way: the roles stay, at size 0, with their names, and the one word "best" draws in `TimelineCost`. Example the other way: the three roles are deleted, `kTextRoleCount` goes back to 12, and the timeline draws in `Figure` and `Label` as it did before PV-4. Recommendation: keep them at size 0 as the brief says ("follow the user's font size like every other role"); deleting them is a tidy-up for another day. The mockup's smaller sizes (`.sec` 0.77 rem, `.axis span` 0.75 rem) would be a ratio of the font size, a new number; not recommended.

**Q8. The Preview's audio warning in the builder.** `draw_preview_panel` as SP-1 moves it starts at the audio-warning line ("Hydra couldn't use your audio device; the preview is muted.", `preview_tab.cpp` line 411). The builder shows no such line today, so a muted preview says nothing. Example one way: the builder shows the line over the transport, as Hydra does. Example the other way: the line stays in Hydra's tab and the builder is silent. Recommendation: in the shared panel, so the player knows why there is no sound; the chart-changed line stays Hydra's.

Nothing in `answers.md` disagrees with the handoff. One thing in `answers.md` cannot be built exactly as written without an answer: "a column about 27 rem wide" names a unit the builder does not have yet, which is Q1.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| The whole Preview panel is in the builder, not a copy | answer 4; the user's "preview code that already exists" | SP-1, SC-3 | `draw_highway`, `kHighwayHeading`, `kPlayLabel`, `kPauseLabel` are gone from `pb_window.*` (reviewer grep); `open-starts-empty` reads "< Act" in the builder; `build-a-path-by-clicking` presses "Act >" |
| Hydra's Preview tab looks exactly as before | handoff problem 4 | SP-1 | `preview`, `preview-path-overlay`, `preview-controls` pass unchanged |
| The move changes no look | SP-1 | SP-1 | the reviewer's diff of the moved block is a move |
| The volume is read and saved by each program's owner | SP-1, Q2 | SP-1, SC-3 | `preview-controls` (Hydra's slider still commits); the reviewer greps `src/pathbuilder/` for `commit_settings` and file writes (none) |
| The highway click still works in the builder | SP-1 (`image_click`) | SP-1, SC-3 | `cant-activate-reason` and the highway-click check in `open-starts-empty` (a click on the image at a fill through `fill_at_image`) |
| Timeline text follows the user's font size | answer 1's last paragraph | TL-2 | `test_text_role.cpp` rows at size 0 |
| No number under any fill; only the best says "best" | answer 1 | TL-2, SC-3, TS-4 | `only the best fill has a word under it`; "best" on screen exactly once in `build-a-path-by-clicking` |
| Each offered fill is tinted by `cost_tier` | answer 1 | TL-2, SC-3 | `a tinted fill reads its tint`; `tier_color` is the one map (reviewer grep for `ChipToken::t0` in `src/pathbuilder/`: one site) |
| The ring means only "the game won't let you" | answer 2 | TL-2, SC-3 | `a locked fill is drawn as locked and a cant fill as cant`; `fill refusal: the five kinds`; `locked-fill-says-left-to-right` |
| The locked look has its own legend entry | answer 2 | TL-2, SC-3 | `open-starts-empty` reads "Before your last activation" |
| A click on a locked fill says the left-to-right sentence | answer 2 | SC-3, TS-4 | `locked-fill-says-left-to-right`; the locked click in `build-a-path-by-clicking` |
| The caption is in the label column; nothing at the row's right end | answer 3 | TL-2 | the layout has no right-end label (reviewer: `kActivateLabelRight` gone); `open-starts-empty` reads "needed to activate" and "enough to activate" |
| Teal shading where the saved line is at or above 2 after the last activation | answer 3 | TL-2 | `the saved row's enough stretches start after the last activation and split a segment at the line`; `with no activation every stretch...` |
| The number 2 is `kSpActivationBars` everywhere | first plan | TL-2, SC-3 | the `activate-line.json` scan row |
| The layout is answer 4's order, Preview full width, column beside it | answer 4 | SC-3 | `kHighwayW` gone (reviewer grep); RA-3 |
| The list names every fill you can activate at next, cheapest first | answer 1 | SC-3, TS-4 | `candidate row: ...`; the list read and the row click in `build-a-path-by-clicking` |
| Hovering a row highlights its fill and moves the Preview; leaving restores | answer 1 | SC-3, TS-4 | the hover-and-leave clock reads in `build-a-path-by-clicking` |
| Clicking a row activates there | answer 1 | SC-3, TS-4 | the row click makes step 2 in `build-a-path-by-clicking` |
| Pending rows show "..." and order as Q3 says | Q3 | SC-3 | `open-starts-empty` reads a `kPendingText` row before `wait_idle`, and none after |
| Steps are compact rows that open when selected | answer 4 | SC-3, TS-4 | `step short: ...`; the compact text then the full sentence in `build-a-path-by-clicking` |
| "< Act" and "Act >" jump between the player's activations | answer 4 | SC-3, TS-4 | disabled with none in `open-starts-empty`; "Act >" lands on activation 1 in `build-a-path-by-clicking` |
| The fill's tooltip keeps the exact numbers | answer 1 | SC-3, TS-4 | the tooltip reads in `build-a-path-by-clicking` (unchanged from PB-6) |
| Costs are one subtraction, tiers one rule | first plan | SC-3 | `candidate_costs` and `cost_tier` untouched (reviewer diffs `src/app/path_costs.*`: empty) |
| Every refusal line through `plain_error` | first plan | SC-3 | reviewer grep of `src/pathbuilder/` for `e.what()` |
| Every assertion reads the screen | brief | TS-4 | the reviewer greps `pbtest_build.cpp` for `h.app->state` (none) |
| The user sees it in the real app on The Reaper before DOC-2 | brief | RA-3 | the user's yes in chat |
| Numbers on screen are checked against the data before judging | memory `check-the-screen-against-the-data` | RA-3 | the main session's check against `reaper-user-path-target.json` and `hydra_replay` |
| One review of the whole range before `main`; merge order kept | handoff | REV | the ref gate's verdict on the range tip; `pb-hy1` merges after |
| No new number without the user | brief | Q1 to Q8 | the questions; executors stop and ask |
| No whole-library run | preamble | every task | every test names `testdata/input`; RA-3 reads one song |
| Agents never run the full suite | rule 1 | every task | each task's Run line |
