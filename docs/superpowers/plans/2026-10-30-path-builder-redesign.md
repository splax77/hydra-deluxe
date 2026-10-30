# Plan: the Path Builder redesign (build your own path, left to right)

Task id `PB-R-PLAN`, session `93b1c61e-76e1-48dc-80a7-ad64e0253498`. Planner: Fable. Branch `claude/pb-redesign-plan`, cut from `claude/pb-2c` (705fb03f), which holds every builder piece built so far. Status: plan written; nothing is built. Every file and line named below was read in that worktree on 2026-10-30.

The spec for the screen is the approved mockup, `docs/superpowers/specs/2026-10-30-path-builder-mockup/mockup_template.html` (the third version; the user approved it this session). The earlier design, `docs/superpowers/specs/2026-10-28-path-builder-design.md`, and its plan, `docs/superpowers/plans/2026-10-28-path-builder.md`, still describe the engine side and the first build; DOC-2 below brings the design up to date.

## The plain version

The builder as built shows too much and explains nothing, and its clicks go nowhere. The user said so in these words, which are the requirements:

- "the builder should start from nothing and let you build your own path"
- "you're just overwhelmimg the user with information and not explaining anything"
- "where does this 19k number come from? what is it comparing to?"
- "what is the squeeze out number? that is useless info without a number"
- "where did the highway go? none if this means anything if youcant visualize it"
- "plan it. make sure the plan is using the preview code that already exists, not reinventing it"

So the redesign starts the player with no activations and lets them build left to right. One whole-song timeline shows the sections, the Star Power phrases, the Star Power saved over time, the player's activations and every fill. Each fill has one of three looks: you can activate here, your activation, or can't activate here. A guide line always says the next step. Three scores show. Each activation gets one plain sentence. The highway follows wherever the player hovers or clicks.

The hard rule is the last quote. The mockup's canvas highway, its Star Power line and its timeline were browser stand-ins. The real builder draws all of it with Hydra's Preview: `PreviewController` renders the 3D highway, `PreviewScene` already holds the sections, phrases, fills, activations and the whole-song SP meter curve, and `song_fraction` already owns "how far along the song is this moment". Where the Preview lacks a piece, the piece is added to the Preview's own code, so Hydra's Preview gains it too. Nothing in `src/pathbuilder/` draws anything the Preview can draw.

## What went wrong, checked in the code

I confirmed the three causes the main session named.

Clicks after the first window did nothing because `apply_preview_overlay` (`src/app/preview_view.cpp` lines 435 to 463) marks every fill Hidden and then marks only the fills each activation lists as passed over (`Activation::skipped_fill_ticks`) Offered and the activation's own fill Taken. Its own comment says the rest stay hidden, "including those after the last activation, which the engine records nothing about". `PbApp::draw_highway` (`src/pathbuilder/pb_window.cpp` lines 313 to 328) acts only on an Offered or a Taken fill, so a click on a Hidden one, which is every fill after the player's last window, is dropped with no message.

The engine does know those fills. `branch_activate` (`src/search/engine.cpp` line 962) refuses an activation when the meter is under `kSpActivationBars` (2, `src/core/timing.h` line 51), when the fill has no branch edge, or when the fill deadline refuses it (`fill_refuses`). A skip is charged only when a child was produced, so a path's skip chain (`p.skip_tail`) holds exactly the fills it passed while it could have activated. At the end of the search the engine emits each activation's chain (`emit_ticks(fills_, a.skip_tail, ...)`, line 1538) and the path's trailing bank chain (`emit_ticks(banks_, ...)`, lines 1712 to 1729, into `Path::trailing_bank_ticks`), but it never emits the trailing skip chain. That one omission is why the Preview hides every fill after the last activation. EN-4 fixes it at the source.

The builder's GUI tests passed because `click_fill` in `tests/pb_ui/pbtest_harness.cpp` (line 108) hands a tick straight to `DecisionList::click_fill`. No test clicked the screen. PB-6 fixes that.

The strip (`src/pathbuilder/pb_strip.cpp`) copies the scrub bar's x rule (`strip_x`, line 25) and its marker literals (`px(2)`, `px(3)`, lines 72 and 73, next to `draw_scrub_marks` in `src/ui/preview_tab.cpp` line 187). That is the copy the user named. PV-3 replaces both with one timeline in the Preview library.

## Each piece of the mockup, and what the Preview already has

The table says, for each thing the mockup draws or computes, the Preview owner the builder reuses, and what is missing.

| Mockup piece | Owner in Hydra today | Missing |
|---|---|---|
| The highway with notes, phrases, fills, the SP window tint, the SP end marks | `PreviewController::render` over `PreviewScene`; `build_highway_draws` in `src/render/highway_draw.cpp` (SP end marks at line 405, D81) | nothing |
| Hover or click a fill: the highway shows it | `PreviewController::seek_ms`; `seek_activation` is the model (line 530 of `preview_controller.cpp`) | a rule for where the playhead goes to look at a fill (Q4) |
| Play with audio | `PreviewController::toggle`, `playing`, the Preview's Space key | nothing |
| Timeline axis and click-to-seek | `song_fraction(ms, length_ms)` (`src/app/preview_view.h` line 481) is the one x rule; the scrub bar `##scrub` seeks | a widget that draws rows over that rule (PV-3) |
| Sections row | `PreviewScene::sections` (`PreviewSection`) | drawing it (PV-3) |
| Star Power phrases row | `PreviewScene::sp_phrases` | drawing it (PV-3) |
| Star Power saved over time, with the "enough to activate" mark | `PreviewScene::sp_meter` (`SpMeterCurve`), read by `sp_meter_bars_at`; built by `build_sp_meter_curve` from the record's facts, or `build_unanalyzed_sp_meter_curve` with no path (one bar per phrase, pinned at the cap). The mark is `kSpActivationBars` | drawing the whole curve (PV-3); the gauge today draws one moment |
| Your activations row | `PreviewScene::activations`, each `sp_window()`; `build_scrub_marks` already places them | drawing them as bands (PV-3) |
| Hydra's best path, faint | `PreviewScene::ghost_fills` through `PreviewController::set_ghost_path` (RD-2, built) | the timeline draws ghost bands from the ghost path's windows (PV-3) |
| Fills, three looks | `PreviewScene::fills`, `PreviewFillState` Hidden, Offered, Taken | the Offered state after the last activation (EN-4) |
| "Can you activate here, and why not" | engine truth: Offered means the search could activate there; the meter level is `sp_meter_bars_at`; "already on" is `PreviewActivation::sp_running_at` | the reason sentences (CO-1) |
| Cost under a fill, and the two numbers that explain it | `best_finish` (`src/search/pather.h` line 220) gives BF(n) and BF(n + this fill); `cost_list` and `cost_tier` (`src/app/path_costs.h`) own costs and colours | a per-fill cost list beside `cost_list` (CO-1) |
| Your score, best possible, best you can still reach | `LadderRun` (`path_costs.h` line 62): `player_total`, `bf[0]`, `bf[n]`; with no window `search_target` over no windows is the no-SP score | nothing |
| One sentence per activation, with "needs X ms" and the violet note | `Activation::badge_timings`, `SPSqueeze::difficulty`, `format_ms`, `tight_window_note` (`pb_coach.cpp` line 20), `Activation::within_ms_limit` | the sentence (CO-1) |
| End choices for the last activation | `end_choices` (`pb_coach.cpp` line 87) | nothing |
| Undo, Start over, Finish the rest for me | `PathHistory` (`pb_state.h`), `finish_for_me` (`pb_coach.cpp` line 255) | Start over is `PathHistory::edit(PlayerPath{})` |
| Glossary | none | three sentences (CO-1) |
| Every refusal line | `plain_error`, `plain_error_detail`, `impossible_window_text` | nothing |

Prior art. The osu! editor's whole-song timeline sits at the top of every mode and marks each kind of thing with its own coloured tick, a double vertical line shows the current moment, and clicking it seeks; the object strip is a separate, zoomable row (osu! wiki, Client/Beatmap_editor/Timelines, read 2026-10-30). The timeline here follows that shape: one row per kind, a playhead line, click to seek. A second search for a Moonscraper or YARG path planner found nothing usable; CHOpt and the chess tools are in the 2026-10-28 brainstorm and are not repeated.

## What stays and what goes

Stays, as built and reviewed: the `free_after` engine option and `best_finish` (EN-1, EN-2); `cost_list`, `cost_tier` and `build_ladder` (EN-3); `PathHistory`, undo and redo (PB-1); Copy and Paste, Save path and Open path, the four error kinds (PB-4, PB-3, PB-3b); the Preview library split; Hydra's "Open in Path Builder" button (HY-1, on `claude/pb-hy1`); TM-1's one-decimal badges; `end_choices`, `start_window`, `remove_window`, `finish_for_me`, `tight_window_note`, `impossible_window_text`, the `Coach` worker and its supersede rule (PB-2a); `image_to_floor_time`, `TrackState::fill_at`, `fill_at_image` (RD-1, PB-2b); the ghost path (RD-2).

Goes: the left path list (`pb_paths.{h,cpp}`, `##paths`, "Edit a copy"); the decision list's column (`DecisionList::draw` in `pb_coach.cpp` from line 515, "Your decisions", the token rows, the end choices under a selected row); the strip (`pb_strip.{h,cpp}`, `##strip`, the step line); the "shown row" idea (`PbApp::shown_row`, `click_windows`'s copy rule). Their tests go with them (`tests/test_pb_strip.cpp`, `tests/pb_ui/pbtest_strip.cpp`). Redo stays in the menu; the mockup has no Redo button and none is added on screen.

## Engine truth, not mockup shortcuts

The mockup decided "can activate here" by counting phrases since the last activation. The builder never re-derives that. The one owner of "could the search activate at this fill" is `branch_activate`, and EN-4 makes the engine write its answer down for the stretch after the last window, the way it already does inside each window. Then `apply_preview_overlay` marks those fills Offered with the same `mark` lambda it uses today (line 454), and every reader of `PreviewFillState`, the highway, `TrackState::fill_at`, the new timeline, sees the same fills light up. Hydra's Preview gains it too: a path's fills after its last activation show as offered, which they are.

The reason a fill cannot be activated is read from facts, in one function (`fill_reason`, CO-1), never from a rule of its own. A fill inside a player's window: `PreviewActivation::sp_running_at(fill.span.end_ms)`, the owner of "is SP running now". A fill before the last activation: the position rule of the screen (left to right), which is this plan's design, not a game rule (Q1). A fill after it that is not Offered: the meter at that fill from `sp_meter_bars_at(scene.sp_meter, fill.span.end_ms)`, the Preview's meter; under `kSpActivationBars` it reads "You have N phrases saved here. You need 2 to activate." and otherwise "The game doesn't show this fill here." (the engine refused it for the fill deadline or the graph, `fill_refuses` and the branch edge in `branch_activate`). The number 2 is `kSpActivationBars`; nothing writes a literal.

The cost under an Offered fill f is BF(n) minus BF(n plus f), where BF(n plus f) is `best_finish` over the player's windows plus f pinned by its activation alone. The hover explains it with those two numbers: "If you activate here, the best you can still finish with is <BF(n plus f)>. Your best choice right now is <fill g>, which keeps <BF(n)>. So activating here costs <difference> points." The fill g is the next activation of the path behind BF(n) (`LadderRun::best[n]`), so the "best" fill always costs 0 and the numbers always agree. `candidate_costs` in `path_costs` is the one subtraction (CO-1), beside `cost_list`.

Squeeze figures come from `Activation::badge_timings` and `format_ms`, as the Paths tab prints them (D265). The violet note is `tight_window_note` as built. Every refusal goes through `plain_error` and `plain_error_detail`; the reviewer greps `src/pathbuilder/` for `e.what()`.

## Tasks

Each task fits one Opus executor in 100 tool calls and 20 minutes, in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`, forked from `claude/pb-2c` (or as "Blockers" says). Two tasks never own one file. Each runs only the tests it names. Tests pin literals from one run or call the production function. One fresh reviewer per task (D107). Every label the user reads is a string constant with one home, so DOC-2 can check it.

### Wave 1: EN-4, PV-3 and CO-1 start together

#### EN-4. The engine records the fills after the last activation

Owns `src/core/model.h`, `src/core/model.cpp`, `src/search/engine.h`, `src/search/engine.cpp`, `src/app/preview_view.h`, `src/app/preview_view.cpp`, `tests/test_fills_and_skips.cpp`, `tests/test_preview_scene.cpp`, `tests/test_tied_variants.cpp`.

What it builds. `Path` gains `trailing_skipped_fill_ticks`, beside `trailing_bank_ticks` (`model.h` line 791): the fills passed over after the last window while the search could have activated, in chart order, with no cap. The engine emits the path's `skip_tail` chain into it where it emits the trailing bank (`OutPath` gains `skip_begin`, `skip_end`; the rebuild at line 1960 copies it as it copies the bank). A variant takes its own chain by the same three cases the bank uses at lines 1712 to 1729 (finished, folded inside SP, folded between windows); `splice_bank` is the model, and the executor stops and asks if a case has no bank analogue. `apply_preview_overlay` marks every tick in `scene.trailing_skipped_fill_ticks` Offered through its `mark` lambda, after the activations' loop, so a Taken fill stays Taken. `PreviewScene` gains the field beside `trailing_bank_ticks`. The record store keeps path summaries only (`record_store.cpp` line 282 lists the columns: path text, score, actcount, maxskip, hardest_ms, ...) and Hydra re-analyzes a clicked song live (D87 item 1), so nothing stored changes and no stamp in `src/store/stored_versions.h` moves. The executor states that check in its report.

Questions and owners: "which fills after the last window could the path have activated on" (the engine's skip chain, emitted; one owner); "which fills does the Preview light" (`apply_preview_overlay`, reading it).

Tests: `trailing fills: a corpus best path lists the fills after its last activation that the meter allows` (a corpus chart whose best path leaves phrases unspent; pins the list from one run, and asserts every listed tick is a fill node whose meter at that tick, from `sp_meter_bars_at` over the scene, is at least `kSpActivationBars`); `trailing fills: a path whose last window outlasts the chart lists none`; `trailing fills: a full pin of a corpus path agrees with the free search` (same list from `search_target` as from `analyze_chart`); `preview scene: fills after the last activation read Offered when the engine lists them, Hidden otherwise` (`test_preview_scene.cpp`, pinned from one run); `tied variants: a variant's trailing fills are its own` (`test_tied_variants.cpp`, a chart with a variant folded between windows).

Run: `build-cpp\Release\hydra_tests.exe "-sf=*test_fills_and_skips.cpp"`; then `"-sf=*test_preview_scene.cpp"`; `"-sf=*test_tied_variants.cpp"`; `"-sf=*test_search_target.cpp"`; `"-sf=*test_best_finish.cpp"`; `"-sf=*test_track_state.cpp"`; and `hydra_uitest.exe --test preview --test preview-path-overlay`. Hydra's Preview changes on purpose here (offered fills past the last activation), so the executor takes one headless screenshot of the Preview on a corpus chart for RA-2.

Done when they pass. Blockers: none. earlyStart: at the user's yes.

#### PV-3. The whole-song timeline, in the Preview library

Owns `src/ui/preview_timeline.h` (new), `src/ui/preview_timeline.cpp` (new), `src/ui/preview_tab.cpp` (only `draw_scrub_marks`, which moves), `tests/test_preview_timeline.cpp` (new), the `hydra_preview` source list line in `CMakeLists.txt` (line 483 block; its own line beside `preview_controller.cpp`).

What it builds. `ui::PreviewTimeline`: rows drawn with ImGui over one width, each x from `song_fraction(ms, playback_end_ms)`, the scrub bar's own rule. Rows: the time axis; sections (`scene.sections`, named); Star Power phrases (`scene.sp_phrases`, a star glyph as the Preview's `draw_star` draws one); Star Power saved (`sp_meter_bars_at` sampled at every segment end of `scene.sp_meter`, which is exact because the curve is piecewise linear, with a dashed line at `kSpActivationBars` labelled "2 phrases: enough to activate" from that constant); the ghost path's windows as dashed bands when a ghost is set; the drawn path's windows as numbered bands (`sp_window()`); the fills, one `InvisibleButton` each named `##fill_<chord_tick>` so a GUI test and the player click the same item, drawn by `PreviewFillState` (Offered glowing, Taken in the path's colour, Hidden as an outline) with a caller-supplied label under each (the cost); and a playhead line at `position_ms`. It returns what happened this frame: a fill hovered, a fill clicked, a band clicked, or the axis clicked at a time. The marks' sizes are the scrub bar's (`draw_scrub_marks` moves in here and the scrub bar calls it, so one place owns the mark geometry; sizes unchanged). The timeline draws nothing of its own invention: every row reads a `PreviewScene` field named above. Hydra's Preview tab keeps its scrub bar and gains nothing visible (Q5).

Questions and owners: "where on the timeline is a moment" (`song_fraction`); "what is a fill's look" (`PreviewFillState`); "how many bars are saved at a moment" (`sp_meter_bars_at`); "where is the activate line" (`kSpActivationBars`).

Tests: `preview timeline: every fill is an item named by its chord tick` (a scene fixture from `tests/preview_scene_fixtures.h`; the widget's returned item list); `preview timeline: a click on a fill names that fill and a click on the axis names its time` (through the test engine's `ItemClick` in a `hydra_tests` ImGui context the file sets up as `test_preview_boxes.cpp` does, or pinned through the pure layout function the executor splits out, `timeline_layout(scene, width)`, which is the preferred shape: layout pure and tested, drawing thin); `preview timeline: the saved line samples every meter segment end and marks kSpActivationBars`.

Run: `hydra_tests.exe "-sf=*test_preview_timeline.cpp"`; `hydra_uitest.exe --test preview --test preview-path-overlay --test preview-controls` (the scrub marks moved; Hydra's look is unchanged).

Done when they pass with no change to Hydra's screen. Blockers: none. earlyStart: at the user's yes.

#### CO-1. Candidate costs, reasons and the sentences

Owns `src/app/path_costs.h`, `src/app/path_costs.cpp`, `src/pathbuilder/pb_coach.h`, `src/pathbuilder/pb_coach.cpp`, `tests/test_path_costs.cpp`, `tests/test_pb_coach.cpp`.

What it builds. In `path_costs`: `struct CandidateCost { int64_t act_tick; int64_t best_finish; int64_t cost; CostTier tier; }` and `candidate_costs(bf_n, candidates)`, cost being `bf_n - best_finish`, the second and last subtraction in the file; a negative cost throws as `cost_tier` does. In `pb_coach`: `fill_reason(scene, fill, windows)` (the facts in "Engine truth" above; returns empty for an Offered fill after the last window); `fill_hover_text` (the two numbers; "..." while the candidate's search has not landed); `guide_text(state)` (the mockup's `guideText`, one owner: "Step 1. Click a glowing fill..." naming the earliest Offered fill, "Your Star Power runs until m<N>...", "your path is finished", and the refusal form); `step_sentence(activation, cost, before, after, saved)` (the mockup's step text: "Activate at m<N> (<clock>) with <k> phrases saved. Star Power runs until m<M>, ending with a squeeze-out that needs 12.3 ms (over your 10 ms Path limit). This choice costs 1,234 points." with the measure from `format_measure`, the clock from `clock_str`, each timing from `badge_timings` through `format_ms`, the note from `tight_window_note`, the count from `bank_rise_ticks.size()`); `glossary_text()`; and the worker: `Coach` gains a candidate queue that, once the ladder is complete, runs `best_finish` for each Offered fill after the last window in chart order, one at a time, reporting each as it lands, superseded by a newer request like the ladder is (`LadderJob`'s `throw_if_cancelled`). The queue is finite (one search per Offered fill) and ends with the path change that supersedes it; the executor times the largest corpus chart, The Faceless "The Spiraling Void", and reports the longest single `best_finish` and the whole queue (Q7). `DecisionList::draw` and the end-choices-under-a-row UI are deleted here; `click_fill`, `click_remove`, `choose_end`, `poll`, `message`, `notice`, `run` stay. `kFinishForMeLabel` becomes "Finish the rest for me" (the mockup's words).

Questions and owners: "what does activating at this fill cost" (`candidate_costs`); "why can't I activate here" (`fill_reason`, reading the Preview's facts); "what is the next step" (`guide_text`); "what does this activation say" (`step_sentence`).

Tests, `test_pb_coach.cpp`: `fill reason: inside a window, before the last activation, under two bars, refused by the game, and none for an offered fill` (one corpus chart, five fills pinned from one run); `candidate costs: the best next fill costs 0 and every other cost is BF(n) minus its best finish` (a two-window corpus path; asserts the zero fill is `best[n]`'s next activation); `step sentence: a squeeze-out over the limit carries the ms figure and the violet note` ("White Washed" at 10 ms, the EN-3 chart; pins the sentence from one run); `guide text: step 1 names the earliest offered fill; a finished path says so`; `candidate queue: a newer request supersedes the older` (the cancel). `test_path_costs.cpp`: `candidate costs: a negative cost is a bug`.

Run: `hydra_tests.exe "-sf=*test_path_costs.cpp"`; `"-sf=*test_pb_coach.cpp"`.

Done when they pass. Blockers: `fill_reason` reads `trailing_skipped_fill_ticks`, so CO-1 forks from EN-4's executor commit (earlyStart), not its review.

### Wave 2

#### PB-5. The screen, rebuilt on the timeline

Owns `src/pathbuilder/pb_window.h`, `src/pathbuilder/pb_window.cpp`, `src/pathbuilder/pb_state.h`, `src/pathbuilder/pb_state.cpp`, `src/pathbuilder/pb_strip.{h,cpp}` and `src/pathbuilder/pb_paths.{h,cpp}` (deleted), `src/pathbuilder/pb_menu.{h,cpp}` (Start over joins Edit), the `hydra_pathbuilder` source list in `CMakeLists.txt` (lines 664 to 672), `tests/test_pb_strip.cpp` (deleted), `tests/test_pb_state.cpp`, `tests/pb_ui/pbtest_harness.{h,cpp}`, `tests/pb_ui/pbtest_window.cpp`, `tests/pb_ui/pbtest_coach.cpp`, `tests/pb_ui/pbtest_strip.cpp` (deleted), `tests/pb_ui/pbtest_build.cpp` (new, an empty `register_build_tests` for PB-6 to fill), `tests/single_owner/rules/*.json` rows naming deleted files.

What it builds. The window in the mockup's order: title and mode line; the intro sentence; the three scores (`player_total`, `bf[0]`, `bf[n]`, from `DecisionList::run()`; "..." until each lands) with the mockup's hints under them; Undo, Start over, Finish the rest for me; the guide line (`guide_text`, red-edged for a refusal); the legend with "Show Hydra's best path" (sets `set_ghost_path` to `record.best_path()` or null); the `PreviewTimeline` (PV-3) over `preview_->scene()`, each Offered fill labelled by its `CandidateCost` ("best" or "−1.2k" as `short`, the mockup's rule, through one formatter in `app/display_format`); below, the highway (`render` into `##highway_image`, Play, the clock) and the steps panel (`step_sentence` per activation, the selected one offering Remove and, on the last, its `end_choices` with timing and cost, then the leftover line and the glossary). A hover on a fill seeks the Preview to it (Q4's rule) and shows `fill_hover_text` as a tooltip; a click on an Offered fill calls `click_fill`; a click on a Taken fill or a band selects that step; a click on a "can't" fill sets the guide line to its reason; the axis seeks. A click on the highway keeps working through `fill_at_image`. The player's path is the only path drawn; `shown_row`, `click_windows`'s copy rule and the left list go. Open path replaces the player's path as one undo step (Q2). A chart with no Star Power shows `kNothingToBuildText` and no timeline. Every string the player reads is a named constant.

Questions and owners: none new; the screen reads `guide_text`, `candidate_costs`, `fill_reason`, `step_sentence`, the ladder and the Preview.

Tests: `test_pb_state.cpp` keeps undo and redo and loses the row pins. `pbtest_window.cpp`: `open-starts-empty` (opens a corpus chart; `text` holds the intro, "Step 1." and three score labels, and no "Your decisions"); `no-star-power`; `file-open-error`. `pbtest_coach.cpp`: `finish-for-me` and `tight-window-note` rewired to the steps panel's text; `impossible-click-message` becomes `cant-activate-reason` (click `**/##fill_<tick>` on a fill under two bars; the guide line holds `fill_reason`'s sentence).

Run: `hydra_tests.exe "-sf=*test_pb_state.cpp"`; `hydra_pathbuilder_uitest.exe --test open-starts-empty --test no-star-power --test file-open-error --test finish-for-me --test tight-window-note --test cant-activate-reason`.

Done when they pass and `HydraPathBuilder.exe <corpus chart>` builds a path by clicking. Blockers: EN-4, PV-3, CO-1 (merged, or their executor commits merged together for an earlyStart).

#### RA-2. The real-app check (the main session, right after PB-5)

A check, not a build, capped at 20 minutes. Copy PB-5's build into a scratch folder with a fresh copy of `hydra.db`, `hydra_settings.ini` and `hydra_ui.ini` from `C:\Program Files\Hydra`, installed Hydra closed. Start `Hydra.exe`, open one real song, press "Open in Path Builder", and the user builds a path: hover a fill, click it, read the sentence, pick an end, Undo, Start over, Finish the rest for me, Play. Also look at Hydra's own Preview on that song: fills after the last activation now show offered (EN-4). Rendering is checked on corpus charts; the library copy is read for one real song only. Nothing runs over the whole library. The user says yes or names what to change before PB-6 and DOC-2 merge.

#### PB-6. One GUI test builds a whole path by clicking

Owns `tests/pb_ui/pbtest_build.cpp`.

What it builds. `build-a-path-by-clicking`: opens a corpus chart with at least three activations on its best path (the executor picks one and pins its ticks from one run), waits idle, asserts the guide says "Step 1.", hovers `**/##fill_<tick>` (the test engine's `MouseMove`) and reads the tooltip's two numbers through `visible_text`, clicks it, waits for the edit, asserts the steps panel gained one sentence and the score line changed, clicks a second Offered fill, clicks a "can't" fill and reads the reason in the guide line, clicks Undo and asserts the step went, clicks "Finish the rest for me" and asserts the player's score equals the "Best you can still reach" figure, then Start over and asserts "None yet". Every assertion reads the screen (`text`, `dump`), never `PbState`.

Run: `hydra_pathbuilder_uitest.exe --test build-a-path-by-clicking`.

Done when it passes. Blockers: PB-5. earlyStart: fork from PB-5's executor commit.

#### DOC-2. The design, the record and the pages

Owns `docs/superpowers/specs/2026-10-28-path-builder-design.md` (a dated section "Redesign, 2026-10-30" replacing "The screen" and the left-list, strip and decision-list paragraphs, quoting the user's six sentences above, and naming the mockup as the spec), `docs/audit/2026-10-03-fix-decisions.md` (one D-record at the next free number, with the user's rulings from `docs/handoffs/2026-10-29-path-builder-real-app-check-handoff.md` and this plan's answered questions in their words), `CONTEXT.md` (the Path Builder paragraph: timeline, guide line, steps, candidate costs), `docs/agents/ui-testing.md` (`hydra_pathbuilder_uitest` and `##fill_<tick>`), `docs/UserGuide.md` (the "Path Builder" section, written against PB-5's constants), `README.md` (one line). The two public pages get the doc review.

Run: `hydra_tests.exe "-tc=docs match code*"`.

Done when it passes and the reviewer signs off. Blockers: PB-6 (labels final). earlyStart: fork from PB-5's merge.

## Waves, in one line each

EN-4, PV-3 and CO-1 start together at the user's yes (CO-1 forks from EN-4's executor commit). PB-5 forks from the three. RA-2 runs on PB-5's build, with PB-6 running beside it. DOC-2 last. Each merge runs what `hydra_precheck.exe --tests` prints plus the three repo-wide checks (rule 1).

## Out of scope (for the user to confirm)

- A cap on the candidate queue's time. CO-1 measures and reports; no number is invented (Q7).
- Early-fill timing on the timeline (the mockup left it out; `tight_window_note` still names an E0 fill's figure in the sentence).
- Hydra's Preview tab showing the timeline rows (Q5).
- The spec's `kSideColumnShare` and the strip sizes: deleted with their owners.

## Open questions for the user

Each one is answerable in a line. The code's real rule is quoted with it.

**Q1. Left to right, or edit anywhere?** The mockup refuses a fill before the last activation ("This is before your last activation. You build left to right; Undo goes back a step.") and removes "this and later ones". The earlier rulings read: "Keep it if possible", meaning "The later window keeps the end you chose while it's still possible. Hydra picks a new end only when it isn't, and says so", and for a removal that breaks a later window, "Own sentence": "Removing this window makes one of your later windows impossible, so the path stays as it was." The code today applies those: `repin` in `pb_coach.cpp` (line 190) keeps later windows whole through `best_finish_keeping_later` and falls back to re-picked ends with `kLaterEndRepickedText`. Example one way: on Burnout, removing the first of three windows under the rulings keeps windows two and three and may say the end of three changed. Example the other way: under the mockup, clicking a fill before window three refuses with the sentence above, and Remove on window one drops two and three. Recommendation: the mockup's rule, because it is what the user approved and it has one sentence per case; `repin`'s keep-later branch and the removal sentence then go unused and are deleted.

**Q2. The left list, "Edit a copy", and the rulings about it.** The mockup has no path list; Hydra's best path is a checkbox that shows it faintly. Three rulings named the list: "Copy it, then apply", "Show each path once", and the saved rows from Open path. Recommendation: delete the list; Open path replaces the player's path as one undo step; Paste does the same; the three rulings become moot and the D-record says so.

**Q3. The sentences.** The user kept PB-2a's labels "as written" on 2026-10-29. The mockup's sentences replace them (the guide line, the step sentence, the hover, "Finish the rest for me"). Recommendation: the mockup's words win, as the later approval.

**Q4. Where the highway goes when you hover a fill.** The mockup seeks 1.4 s before the fill, its own number. `seek_activation` puts the activation's chord on the strike line. Recommendation: seek so the fill's span start (`PreviewFill::span.start_ms`) lands on the strike line, so the whole fill is drawn ahead of it; no new number. The other choice is the mockup's lead, which needs a number from you.

**Q5. Should Hydra's Preview tab show the timeline rows too?** PV-3 puts the widget in the Preview library; the Preview tab keeps its scrub bar. Recommendation: not now.

**Q6. End choices only on the last activation?** The mockup offers ends only on the last one. The code offers `end_choices` on any selected row. Under Q1's recommendation only the last matters; say if you want ends on earlier activations too.

**Q7. The candidate searches.** Each Offered fill after the last window costs one `best_finish`. CO-1 reports the time on the largest corpus chart. Do you want a cap, and if so what happens past it?

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| The builder starts from nothing | the user's first quote; PB-5 | PB-5 | `open-starts-empty` |
| Fills after the last activation are offered from engine truth, not a UI rule | "Engine truth"; EN-4 | EN-4 | `trailing fills: a corpus best path lists...` (meter at least `kSpActivationBars`), `...agrees with the free search`, the preview-scene pin; the reviewer greps `src/pathbuilder/` and `src/ui/preview_timeline.cpp` for `sp_phrase_count`, `bank_rise_ticks.size() >=` and `kSpActivationBars` used in a comparison (none allowed outside `fill_reason`'s meter sentence) |
| A variant's trailing fills are its own | EN-4 | EN-4 | `tied variants: a variant's trailing fills are its own` |
| Nothing stored changes; no stamp moves | EN-4 | EN-4 | the executor's report quotes the `record_store.cpp` column list; the reviewer diffs `src/store/` (empty) |
| Every can't-activate fill says why, from facts | "Engine truth"; CO-1 | CO-1, PB-5 | `fill reason: ...` (five cases); `cant-activate-reason` |
| Each cost shows the two numbers it compares | the user's "19k" quote; CO-1 | CO-1, PB-6 | `candidate costs: the best next fill costs 0...`; the hover read in `build-a-path-by-clicking` |
| Costs are one subtraction each, nothing re-priced | "Engine truth" | CO-1 | `candidate_costs` and `cost_list` are the only subtractions in `path_costs.cpp`; the reviewer greps `src/pathbuilder/` for `totalscore` and `replay_path` |
| Green at 0, yellow under 1%, red at 1% or more | spec decision 3 | CO-1 | `cost_tier` unchanged; `candidate costs: a negative cost is a bug` |
| Squeeze figures are Hydra's "needs X ms", violet over the limit | the user's "squeeze out" quote | CO-1 | `step sentence: a squeeze-out over the limit...` |
| The highway is the Preview's and follows hover, click and Play | the user's "highway" quote | PB-5, RA-2 | `build-a-path-by-clicking` reads the clock change after a hover; RA-2 sees Play with audio |
| No drawing routine in `src/pathbuilder/` repeats one the Preview has | the user's "preview code" quote | PV-3, PB-5 | `pb_strip.cpp` is deleted; the reviewer greps `src/pathbuilder/` for `AddRectFilled`, `AddLine`, `song_fraction` and `/ end_ms` (none; the timeline is `ui::PreviewTimeline`) |
| One x rule for the scrub bar and the timeline | PV-3 | PV-3 | `draw_scrub_marks` lives in `preview_timeline.cpp`; `preview --test preview-path-overlay` unchanged |
| The saved line is the Preview's meter curve and marks `kSpActivationBars` | PV-3 | PV-3 | `preview timeline: the saved line samples...` |
| Hydra's Preview tab looks the same (except offered trailing fills) | PV-3, EN-4 | PV-3, EN-4 | the three Preview uitests pass; EN-4's screenshot at RA-2 |
| Three scores, each from the ladder | mockup | PB-5, PB-6 | `build-a-path-by-clicking`: after Finish, your score equals "best you can still reach" |
| The guide line always says the next step | mockup | CO-1, PB-6 | `guide text: ...`; "Step 1." read on screen |
| Undo, Start over, Finish the rest for me | mockup | PB-5, PB-6 | the three clicks in `build-a-path-by-clicking` |
| Click results that arrive after the path changed are dropped | ruling "Drop it" | CO-1 | the generation check in `DecisionList::start_edit` stays; `candidate queue: a newer request supersedes the older` |
| Every refusal line through `plain_error` | brief | every task | reviewer grep of `src/pathbuilder/` for `e.what()` |
| The user sees it in the real app before the GUI merges | brief | RA-2 | the user's yes in chat |
| One GUI test builds a whole path by clicking real widgets | brief | PB-6 | `build-a-path-by-clicking` asserts only screen text |
| Copy, Paste, Save path, Open path, HY-1, TM-1 stay | "What stays" | PB-5 | `test_pb_files.cpp` and `paste-refused-keeps-path` pass untouched in PB-5's precheck run |
| No new number without the user | brief | Q4, Q7 | the questions; executors stop and ask |
| No whole-library run | preamble | every task | every test names `testdata/input`; RA-2 reads one song |
