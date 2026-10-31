# Plan: the Path Builder redesign (build your own path, left to right)

Task id `PB-R-PLAN`, session `93b1c61e-76e1-48dc-80a7-ad64e0253498`. Planner: Fable. Branch `claude/pb-redesign-plan`, cut from `claude/pb-2c` (705fb03f), which holds every builder piece built so far. Status: plan written and reviewed; nothing is built. Every file and line named below was read in that worktree on 2026-10-30, except the mockup and the 2026-10-29 handoff, which are untracked files in the main checkout and were read there.

Before the first task starts, the main session commits the mockup folder (`docs/superpowers/specs/2026-10-30-path-builder-mockup/`) and the handoff `docs/handoffs/2026-10-29-path-builder-real-app-check-handoff.md` to the branch the executors fork from. Several tasks quote both, and an executor in a fresh worktree cannot read an untracked file.

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

I confirmed the three causes the main session named, and found a fourth.

Clicks after the first window did nothing because `apply_preview_overlay` (`src/app/preview_view.cpp`, the fill block from line 435) marks every fill Hidden and then marks only the fills each activation lists as passed over (`Activation::skipped_fill_ticks`) Offered and the activation's own fill Taken. Its own comment says the rest stay hidden, "including those after the last activation, which the engine records nothing about". `PbApp::draw_highway` (`src/pathbuilder/pb_window.cpp` lines 313 to 328) acts only on an Offered or a Taken fill, so a click on a Hidden one, which is every fill after the player's last window, is dropped with no message.

The engine does know those fills. `branch_activate` (`src/search/engine.cpp` line 962) refuses an activation when the meter is under `kSpActivationBars` (2, `src/core/timing.h` line 51), when the fill has no branch edge, or when the fill deadline refuses it (`fill_refuses`). A skip is charged only when a child was produced, so a path's skip chain (`p.skip_tail`) holds exactly the fills it passed while it could have activated. That holds for a search that pins the player's windows too: the loop at line 1872 calls `branch_activate` on every fill, and on a fill the pins do not list it keeps the declining parent, already charged the skip. At the end of the search the engine emits each activation's chain (`emit_ticks(fills_, a.skip_tail, ...)`, line 1538) and the path's trailing bank chain (`emit_ticks(banks_, ...)`: line 1775 for the leading path, lines 1708 to 1741 for a tied variant, into `Path::trailing_bank_ticks`), but it never emits the trailing skip chain. That one omission is why the Preview hides every fill after the last activation. EN-4 fixes it at the source.

The fourth cause is the empty start. With no window yet, the player's path is empty (`PlayerPath::path`, `pb_state.h` line 79), `PbApp::shown_path()` is null, and `apply_preview_overlay` then marks every fill Offered ("Without a path we cannot know", line 435) and builds no score and no saved-Star-Power curve from a path. The test `highway-click-starts-window` (`tests/pb_ui/pbtest_strip.cpp` line 85) leans on exactly that: it asserts the fill reads Offered with no path. So "start from nothing" would light every fill, including those before two phrases are saved. "The empty path" below fixes it.

The builder's GUI tests passed because `click_fill` in `tests/pb_ui/pbtest_harness.cpp` (line 108) hands a tick straight to `DecisionList::click_fill`, and the one on-screen highway click clicks the first window, where every fill reads Offered. No test clicked a second fill on screen. PB-6 fixes that.

The strip (`src/pathbuilder/pb_strip.cpp`) copies the scrub bar's x rule (`strip_x`, line 25) and its marker literals (`px(2.0f)` and `px(3.0f)`, lines 72 and 73, next to `draw_scrub_marks` in `src/ui/preview_tab.cpp` line 187, which uses `px(1.5f)` and `px(2.0f)`). That is the copy the user named. PV-3 replaces both with one timeline in the Preview's GUI code.

## Each piece of the mockup, and what the Preview already has

The table says, for each thing the mockup draws or computes, the Preview owner the builder reuses, and what is missing.

| Mockup piece | Owner in Hydra today | Missing |
|---|---|---|
| The highway with notes, phrases, fills, the SP window tint, the SP end marks | `PreviewController::render` over `PreviewScene`; `build_highway_draws` in `src/render/highway_draw.cpp` (SP end marks, step 6 near line 398, D81) | nothing for these. The mockup's text labels on the highway and the dashed ring round a squeezed-out note are not in the Preview (no "squeez" in `src/render/`, `preview_view.cpp` or `preview_tab.cpp`); see "Out of scope" |
| Hover or click a fill: the highway shows it | `PreviewController::seek_ms`; `seek_activation` is the model (line 530 of `preview_controller.cpp`) | a rule for where the playhead goes to look at a fill (Q4) |
| Play with audio | `PreviewController::toggle`, `playing`, the Preview's Space key | nothing |
| Timeline axis and click-to-seek | `song_fraction(ms, length_ms)` (`src/app/preview_view.h` line 481) is the one x rule; the scrub bar `##scrub` seeks | a widget that draws rows over that rule (PV-3) |
| Sections row | `PreviewScene::sections` (`PreviewSection`) | drawing it (PV-3) |
| Star Power phrases row | `PreviewScene::sp_phrases` | drawing it (PV-3) |
| Star Power saved over time, with the "enough to activate" mark | `PreviewScene::sp_meter` (`SpMeterCurve`, its segments carry `start_bars` and `end_bars`); built by `build_sp_meter_curve` from the record's facts, or `build_unanalyzed_sp_meter_curve` with no path (one bar per phrase, pinned at the cap). The mark is `kSpActivationBars` | drawing the whole curve (PV-3); the gauge today draws one moment |
| Your activations row | `PreviewScene::activations`, each `sp_window()`; `build_scrub_marks` already places them | drawing them as bands (PV-3) |
| Hydra's best path, faint | `PreviewScene::ghost_fills` through `PreviewController::set_ghost_path` (RD-2, built) | the timeline draws ghost bands from the ghost path's windows (PV-3) |
| Fills, three looks | `PreviewScene::fills`, `PreviewFillState` Hidden, Offered, Taken | the Offered state after the last activation (EN-4); a path for the empty start (PB-5) |
| "Can you activate here, and why not" | engine truth: Offered means the search could activate there; the meter level is `sp_meter_bars_at`; "already on" is `PreviewActivation::sp_running_at` | the reason sentences (CO-1) |
| Cost under a fill, and the two numbers that explain it | `best_finish` (`src/search/pather.h` line 220) gives BF(n) and BF(n + this fill); `cost_list` and `cost_tier` (`src/app/path_costs.h`) own costs and colours | a per-fill cost list beside `cost_list` (CO-2) |
| Your score, best possible, best you can still reach | `LadderRun` (`path_costs.h` line 62): `ladder.player_total`, `ladder.bf[0]`, `ladder.bf[n]`; with no window `search_target` over no windows is the no-SP score | nothing |
| One sentence per activation, with "needs X ms" and the violet note | `Activation::badge_timings`, `SPSqueeze::difficulty`, `format_ms`, `tight_window_note` (`pb_coach.cpp` line 18), `Activation::within_ms_limit` | the sentence (CO-1) |
| A fill's or activation's measure ("m12") and the clock | `measure_label` (`src/core/timing.h` line 325, "m96"); `clock_str` (`src/app/display_format.h`) | nothing (`format_measure` prints "m96.1.0", which the mockup does not) |
| End choices for the last activation | `end_choices` (`pb_coach.cpp` line 87) | nothing |
| Undo, Start over, Finish the rest for me | `PathHistory` (`pb_state.h`), `finish_for_me` (`pb_coach.cpp` line 255) | Start over is `PathHistory::edit(PlayerPath{})` |
| Glossary | none | three sentences (CO-1) |
| Every refusal line | `plain_error`, `plain_error_detail`, `impossible_window_text` | nothing |

Prior art. The osu! editor's whole-song timeline sits at the top of every mode and marks each kind of thing with its own coloured tick, a double vertical line shows the current moment, and clicking it seeks; the object strip is a separate, zoomable row (osu! wiki, Client/Beatmap_editor/Timelines, read 2026-10-30). The timeline here follows that shape: one row per kind, a playhead line, click to seek. A second search for a Moonscraper or YARG path planner found nothing usable; CHOpt and the chess tools are in the 2026-10-28 brainstorm and are not repeated.

## What stays and what goes

Stays, as built and reviewed: the `free_after` engine option and `best_finish` (EN-1, EN-2); `cost_list`, `cost_tier` and `build_ladder` (EN-3); `PathHistory`, undo and redo (PB-1); Copy and Paste, Save path and Open path, the four error kinds (PB-4, PB-3, PB-3b); the Preview library split; Hydra's "Open in Path Builder" button (HY-1, on `claude/pb-hy1`); TM-1's one-decimal badges; `end_choices`, `start_window`, `remove_window`, `finish_for_me`, `tight_window_note`, `impossible_window_text`, the `Coach` worker and its supersede rule (PB-2a); `image_to_floor_time`, `TrackState::fill_at`, `fill_at_image` (RD-1, PB-2b); the ghost path (RD-2).

Goes: the left path list (`pb_paths.{h,cpp}`, `##paths`, "Edit a copy") and the saved branches it held (`PbApp::branches_`, the `branches` argument of `open_path_file`); the decision list's column (`DecisionList::draw` in `pb_coach.cpp` from line 515, "Your decisions", the token rows, the end choices under a selected row); the strip (`pb_strip.{h,cpp}`, `##strip`, the step line); the "shown row" idea (`PbApp::shown_row`, `click_windows`'s copy rule). Their tests go with them (`tests/test_pb_strip.cpp`, `tests/pb_ui/pbtest_strip.cpp`). Redo stays in the menu; the mockup has no Redo button and none is added on screen.

## Engine truth, not mockup shortcuts

The mockup decided "can activate here" by counting phrases since the last activation. The builder never re-derives that. The one owner of "could the search activate at this fill" is `branch_activate`, and EN-4 makes the engine write its answer down for the stretch after the last window, the way it already does inside each window. Then `apply_preview_overlay` marks those fills Offered with the same `mark` lambda it uses today (line 454), and every reader of `PreviewFillState`, the highway, `TrackState::fill_at`, the new timeline, sees the same fills light up. Hydra's Preview gains it too: a path's fills after its last activation show as offered, which they are.

The reason a fill cannot be activated is read from facts, in one function (`fill_reason`, CO-1), never from a rule of its own. A fill inside a player's window: `PreviewActivation::sp_running_at(fill.span.end_ms)`, the owner of "is SP running now". A fill before the last activation: the position rule of the screen (left to right), which is this plan's design, not a game rule (Q1). A fill after it that is not Offered: the meter at that fill from `sp_meter_bars_at(scene.sp_meter, fill.span.end_ms)`, the Preview's meter; under `kSpActivationBars` it reads "You have N phrases saved here. You need 2 to activate." and otherwise "The game doesn't show this fill here." (the engine refused it for the fill deadline or the graph, `fill_refuses` and the branch edge in `branch_activate`). The number 2 is `kSpActivationBars`; nothing writes a literal. One consequence the screen must handle: the engine also lists as Offered the fills the player passed between two earlier windows (they sit in the later activation's `skipped_fill_ticks`). Those read Offered on the highway, which is true to the game, but the screen draws them as "can't" and refuses a click on them, through `fill_reason`'s left-to-right sentence.

The cost under an Offered fill f is BF(n) minus BF(n plus f), where BF(n plus f) is `best_finish` over the player's windows (pinned whole) plus f pinned by its activation alone. The hover explains it with those two numbers: "If you activate here, the best you can still finish with is <BF(n plus f)>. Your best choice right now is <fill g>, which keeps <BF(n)>. So activating here costs <difference> points." The fill g is the next activation of the path behind BF(n) (`LadderRun::best[n]`), so the "best" fill always costs 0 and the numbers always agree. `candidate_costs` in `path_costs` is the one subtraction (CO-2), beside `cost_list`.

Squeeze figures come from `Activation::badge_timings` and `format_ms`, as the Paths tab prints them (D265). The violet note is `tight_window_note` as built. Every refusal goes through `plain_error` and `plain_error_detail`; the reviewer greps `src/pathbuilder/` for `e.what()`.

### The empty path

The player's path with no activation is a path too. `search_target` over no windows returns it: the engine follows an empty pin list, declines every fill, and is charged a skip for each fill it passed while the meter allowed an activation, so EN-4's list on that path is exactly the fills the meter allows. PB-5 has `load_open_chart` (`pb_state.cpp`) compute it once, when the chart opens, and keep it on `OpenChart` beside the graph. `PbApp::shown_path()` falls back to it while the player's history holds no path. Then the empty start has a real scene: the right fills glow, the saved-Star-Power curve and the score come from a path, and the three fill looks hold from the first frame. Copy, Save path and the menu flags keep reading `PathHistory::current().path` only, so the empty path is not copyable or saveable, as today. The executor times that one search on the largest corpus chart and reports it, and stops and reports if it returns no path.

## Tasks

Each task fits one Opus executor in 100 tool calls and 20 minutes (rule 7 of CLAUDE.md), in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`, forked from `claude/pb-2c` (or as "Blockers" says). Two tasks never own one file, with one exception: `CMakeLists.txt` is edited by PV-3, CO-1 and PB-5, each on its own lines (PV-3 the `hydra_uibase` list, CO-1 and PB-5 the `hydra_pathbuilder` and `hydra_pathbuilder_uitest` lists), and the merges go in the wave order below, so no two edits touch the same line. Each task runs only the tests it names. Tests pin literals from one run or call the production function. One fresh reviewer per task (D107). Every label the user reads is a string constant with one home, so DOC-2 can check it. PB-5 is the largest task; if its executor reaches 100 calls, it commits what builds and hands the steps panel (the last paragraph of "What it builds") to a fresh executor.

### Wave 1: EN-4, PV-3, CO-1 and CO-2 start together

#### EN-4. The engine records the fills after the last activation

Owns `src/core/model.h`, `src/core/model.cpp`, `src/search/engine.h`, `src/search/engine.cpp`, `src/app/preview_view.h`, `src/app/preview_view.cpp`, `tests/test_fills_and_skips.cpp`, `tests/test_preview_scene.cpp`, `tests/test_tied_variants.cpp`.

What it builds. `Path` gains `trailing_skipped_fill_ticks`, beside `trailing_bank_ticks` (`model.h` line 791): the fills passed over after the last window while the search could have activated, in chart order, with no cap. The engine emits the path's `skip_tail` chain into it where it emits the trailing bank (`OutPath` gains `skip_begin`, `skip_end`; `emit_path` at line 1775 writes the leading path's; the rebuild at line 1960 copies it as it copies the bank). A variant takes its own chain by the four cases the bank uses at lines 1708 to 1741: finished, folded inside SP, folded between windows with the leader activating again, folded between windows with the leader never activating again. `splice_bank` is the model, and the executor stops and asks if a case has no bank analogue. `prepare_variants` (`model.cpp` line 664 comment) leaves the new field alone as it leaves the bank. `apply_preview_overlay` clears the new scene field beside `trailing_bank_ticks` (line 401) and marks every tick in `scene.trailing_skipped_fill_ticks` Offered through its `mark` lambda, after the activations' loop, so a Taken fill stays Taken. `PreviewScene` gains the field beside `trailing_bank_ticks`. The record store keeps path summaries only (`record_store.cpp` line 282 lists the columns: path text, score, actcount, maxskip, hardest_ms, ...) and Hydra re-analyzes a clicked song live (D87 item 1), so nothing stored changes and no stamp in `src/store/stored_versions.h` moves. The executor states that check in its report.

Questions and owners: "which fills after the last window could the path have activated on" (the engine's skip chain, emitted; one owner); "which fills does the Preview light" (`apply_preview_overlay`, reading it).

Tests: `trailing fills: a corpus best path lists the fills after its last activation that the meter allows` (a corpus chart whose best path leaves phrases unspent; pins the list from one run, and asserts every listed tick is a fill node whose meter at that tick, from `sp_meter_bars_at` over the scene, is at least `kSpActivationBars`); `trailing fills: a path whose last window outlasts the chart lists none`; `trailing fills: a full pin of a corpus path agrees with the free search` (same list from `search_target` as from `analyze_chart`); `trailing fills: the path with no activation lists every fill the meter allows` (`search_target` over no windows, pinned from one run: the empty path of "The empty path"); `preview scene: fills after the last activation read Offered when the engine lists them, Hidden otherwise` (`test_preview_scene.cpp`, pinned from one run; the scene comparison helper at line 569 also compares the new field); `tied variants: a variant's trailing fills are its own` (`test_tied_variants.cpp`, a chart with a variant folded between windows).

Run: `build-cpp\Release\hydra_tests.exe "-sf=*test_fills_and_skips.cpp"`; then `"-sf=*test_preview_scene.cpp"`; `"-sf=*test_tied_variants.cpp"`; `"-sf=*test_search_target.cpp"`; `"-sf=*test_best_finish.cpp"`; `"-sf=*test_track_state.cpp"`; and `hydra_uitest.exe --test preview --test preview-path-overlay`. Hydra's Preview changes on purpose here (offered fills past the last activation), so the executor takes one headless screenshot of the Preview on a corpus chart for RA-2.

Done when they pass. Blockers: none. earlyStart: at the user's yes.

#### PV-3. The whole-song timeline, in the Preview's GUI code

Owns `src/ui/preview_timeline.h` (new), `src/ui/preview_timeline.cpp` (new), `src/ui/preview_tab.cpp` (only `draw_scrub_marks` and `draw_star`, which move out of its anonymous namespace into the new files), `tests/test_preview_timeline.cpp` (new), and one line in the `hydra_uibase` source list in `CMakeLists.txt` (lines 513 to 534, beside `list_scroll.cpp`).

Where it lives. `hydra_preview` has no ImGui (the comment above it, `CMakeLists.txt` lines 478 to 482), so the widget goes in `hydra_uibase`, the GUI base that both Hydra's window (`hydra_ui`) and the builder link. It reaches `PreviewScene` through `hydra_render`'s public link to `hydra_app` (lines 469 and 535). The executor runs `"-sf=*test_layering.cpp"`; if the layering test refuses the include, it stops and reports rather than adding a link.

What it builds. `ui::PreviewTimeline`: rows drawn with ImGui over one width, each x from `song_fraction(ms, playback_end_ms)`, the scrub bar's own rule. Rows: the time axis; sections (`scene.sections`, named); Star Power phrases (`scene.sp_phrases`, a star glyph drawn by `draw_star`, which moves in here with `draw_scrub_marks`); Star Power saved (the curve drawn from each segment's own start and end, `SpMeterSegment::start_ms`, `start_bars`, `end_ms`, `end_bars`, which keeps the jumps between segments; `sp_meter_bars_at` is the reader for one moment and on a boundary answers with the later segment, so it does not find a segment's end; a dashed line at `kSpActivationBars` labelled "2 phrases: enough to activate", the 2 written from that constant); the ghost path's windows as dashed bands when a ghost is set; the drawn path's windows as numbered bands (`sp_window()`); the fills, one `InvisibleButton` each named `##fill_<chord_tick>` so a GUI test and the player click the same item, drawn by `PreviewFillState` (Offered glowing, Taken in the path's colour, Hidden as an outline) with a caller-supplied label under each (the cost); and a playhead line at `position_ms`. The caller may also pass a list of fills to draw as Hidden although the scene says Offered (the screen's left-to-right rule, Q1). It returns what happened this frame: a fill hovered, a fill clicked, a band clicked, or the axis clicked at a time. The marks' sizes are the scrub bar's (`draw_scrub_marks` moves in here and the scrub bar calls it, so one place owns the mark geometry; sizes unchanged). The timeline draws nothing of its own invention: every row reads a `PreviewScene` field named above. Hydra's Preview tab keeps its scrub bar and gains nothing visible (Q5).

The layout is a pure function: `timeline_layout(scene, width)` returns every row's rectangles and `timeline_hit(layout, x, y)` says what a point lands on. The ImGui drawing is thin over them.

Questions and owners: "where on the timeline is a moment" (`song_fraction`); "what is a fill's look" (`PreviewFillState`); "how many bars are saved at a moment" (`sp_meter_bars_at`, and the curve's own segments for the drawn line); "where is the activate line" (`kSpActivationBars`).

Tests: `preview timeline: every fill is an item named by its chord tick` (a scene fixture from `tests/preview_scene_fixtures.h`; the layout's item list); `preview timeline: a point on a fill names that fill and a point on the axis names its time` (`timeline_hit`); `preview timeline: the saved line has a point at each segment start and end and a mark at kSpActivationBars`. The tests need no ImGui context (the only one in `hydra_tests` is `test_theme.cpp`'s); the on-screen click is PB-6's.

Run: `hydra_tests.exe "-sf=*test_preview_timeline.cpp"`; `"-sf=*test_layering.cpp"`; `hydra_uitest.exe --test preview --test preview-path-overlay --test preview-controls` (the scrub marks moved; Hydra's look is unchanged).

Done when they pass with no change to Hydra's screen. Blockers: none. earlyStart: at the user's yes.

#### CO-1. The sentences

Owns `src/pathbuilder/pb_text.h` (new), `src/pathbuilder/pb_text.cpp` (new), `tests/test_pb_text.cpp` (new; the test glob finds it), and one line in the `hydra_pathbuilder` source list in `CMakeLists.txt`.

What it builds. Every sentence the screen says, as pure functions over plain values (so CO-1 and CO-2 share no type), each string a named constant:

- `fill_name(scene, fill)`: `measure_label(scene.timing->timecode(fill.chord_tick))`, "m96".
- `fill_reason(scene, fill_index, windows)`: the facts in "Engine truth"; empty for an Offered fill after the last window. It reads `trailing_skipped_fill_ticks`, so CO-1 forks from EN-4's executor commit.
- `fill_hover_text`: the fill's name and clock, the bars saved, BF(n plus f), BF(n), the best fill's name, and the end text, in the sentences of "Engine truth"; "..." while the candidate's search has not landed.
- `guide_text`: from the earliest Offered fill's name, the best next fill's name (or none), the last window's end measure, how many fills the engine lists after the last window (none means the path is finished), the player's score, and a one-shot message with a refusal flag. The mockup's forms: "Step 1. Click a glowing fill...", "Your Star Power runs until m<N>...", "your path is finished", and "Can't activate at <fill>: <reason>".
- `step_sentence(activation, cost, before, after, saved)`: the mockup's step text, for example "Activate at m<N> (<clock>) with <k> phrases saved. Star Power runs until m<M>, ending with a squeeze-out that needs 12.3 ms (over your 10 ms Path limit). This choice costs 1,234 points." (the figures here are examples). The clock is `clock_str`, each timing is from `badge_timings` through `format_ms`, the note is `tight_window_note`, and the count is `bank_rise_ticks.size()`, the bars the activation spends.
- `leftover_text(bf_n, player_total)`, `glossary_text()`, and the one-shot messages the mockup shows after an action: removed, started over, finished for the player.

Questions and owners: "why can't I activate here" (`fill_reason`, reading the Preview's facts); "what is the next step" (`guide_text`); "what does this activation say" (`step_sentence`).

Tests, `test_pb_text.cpp`: `fill reason: inside a window, before the last activation, under two bars, refused by the game, and none for an offered fill` (one corpus chart, five fills pinned from one run); `step sentence: a squeeze-out over the limit carries the ms figure and the violet note` ("White Washed" at 10 ms, the EN-3 chart of `test_path_costs.cpp` line 149; pins the sentence from one run); `guide text: step 1 names the earliest offered fill; a finished path says so`; `fill hover text: the two numbers and the difference agree`.

Run: `hydra_tests.exe "-sf=*test_pb_text.cpp"`.

Done when it passes. Blockers: `fill_reason` reads `trailing_skipped_fill_ticks`, so CO-1 forks from EN-4's executor commit (earlyStart), not its review.

#### CO-2. Candidate costs and the candidate queue

Owns `src/app/path_costs.h`, `src/app/path_costs.cpp`, `src/pathbuilder/pb_coach.h`, `src/pathbuilder/pb_coach.cpp`, `tests/test_path_costs.cpp`, `tests/test_pb_coach.cpp`.

What it builds. In `path_costs`: `struct CandidateCost { int64_t act_tick; int64_t best_finish; int64_t cost; CostTier tier; }` and `candidate_costs(bf_n, candidates)`, cost being `bf_n - best_finish`; a negative cost throws as `cost_tier` does. It is the third subtraction in the file, beside the two in `cost_list`. In `pb_coach`: the `Coach` gains a candidate queue. Given the ticks the screen hands it (`PreviewScene::trailing_skipped_fill_ticks`, EN-4's list) and the player's windows, once the ladder is complete it runs `best_finish` for each tick in chart order, one at a time, reporting each as it lands, superseded by a newer request like the ladder is (`LadderJob`'s `throw_if_cancelled`). `DecisionList::candidates()` gives what has landed. The queue is finite (one search per tick) and ends with the path change that supersedes it; the executor times the largest corpus chart, The Faceless "The Spiraling Void", and reports the longest single `best_finish` and the whole queue (Q7). In `DecisionList`: `draw` and the end-choices-under-a-row UI are deleted (with `kDecisionsHeading`); `click_fill`, `click_remove`, `choose_end`, `poll`, `message`, `notice`, `run` stay; `select` becomes public so the steps panel can select a step and start its end choices; `finish(PbState&)` and `can_finish()` carry the body of the old button (`finish_for_me`, then `player_copy_of` as one edit); `kFinishForMeLabel` becomes "Finish the rest for me" (the mockup's words). If the user answers Q1 as recommended, `repin`'s keep-later branch (`best_finish_keeping_later`, `later_end_changed`, `kLaterEndRepickedText`, `removal_impossible_text`) and its tests are deleted here too.

Questions and owners: "what does activating at this fill cost" (`candidate_costs`).

Tests: `test_pb_coach.cpp`: `candidate costs: the best next fill costs 0 and every other cost is BF(n) minus its best finish` (a two-window corpus path; asserts the zero fill is `best[n]`'s next activation); `candidate queue: it prices only the ticks it is given, in order`; `candidate queue: a newer request supersedes the older` (the cancel). `test_path_costs.cpp`: `candidate costs: a negative cost is a bug`.

Run: `hydra_tests.exe "-sf=*test_path_costs.cpp"`; `"-sf=*test_pb_coach.cpp"`.

Done when they pass. Blockers: none. earlyStart: at the user's yes.

### Wave 2

#### PB-5. The screen, rebuilt on the timeline

Owns `src/pathbuilder/pb_window.h`, `src/pathbuilder/pb_window.cpp`, `src/pathbuilder/pb_state.h`, `src/pathbuilder/pb_state.cpp`, `src/pathbuilder/pb_files.{h,cpp}` (the `branches` argument goes), `src/pathbuilder/pb_strip.{h,cpp}` and `src/pathbuilder/pb_paths.{h,cpp}` (deleted), `src/pathbuilder/pb_menu.{h,cpp}` (Start over joins Edit), the `hydra_pathbuilder` source list in `CMakeLists.txt` (lines 664 to 672, taking out `pb_paths.cpp` and `pb_strip.cpp`) and the `hydra_pathbuilder_uitest` list (lines 697 to 706, taking out `pbtest_strip.cpp` and adding `pbtest_build.cpp`), `tests/test_pb_strip.cpp` (deleted), `tests/test_pb_state.cpp`, `tests/pb_ui/pbtest_harness.{h,cpp}`, `tests/pb_ui/pbtest_window.cpp`, `tests/pb_ui/pbtest_coach.cpp`, `tests/pb_ui/pbtest_strip.cpp` (deleted), `tests/pb_ui/pbtest_build.cpp` (new, an empty `register_build_tests` for PB-6 to fill), and `tests/single_owner/rules/*.json`.

What it builds. The window in the mockup's order: title and mode line; the intro sentence; the three scores (`ladder.player_total`, `ladder.bf` first and last entries, from `DecisionList::run()`; "..." until each lands) with the mockup's hints under them; Undo, Start over, Finish the rest for me (`DecisionList::finish`); the guide line (`guide_text`, red-edged for a refusal); the legend with "Show Hydra's best path" (sets `set_ghost_path` to `record.best_path()` or null); the `PreviewTimeline` (PV-3) over `preview_->scene()`, each Offered fill after the last window labelled by its `CandidateCost` ("best" or "−1.2k"; the mockup's `short` rule is a figure under 1,000 written whole and from 1,000 as thousands with one decimal and a "k"; it goes through one formatter in `app/display_format`); below it, the highway (`render` into `##highway_image`, Play, the clock). Which fills are "can't" is `fill_reason` non-empty; the list handed to the timeline is built from it. The empty start uses "The empty path". The screen hands the scene's `trailing_skipped_fill_ticks` to the candidate queue (CO-2).

How the screen reacts. While the pointer is on a fill, the Preview is paused and sought to it (Q4's rule), and `fill_hover_text` shows as a tooltip; when the pointer leaves, the Preview goes back to the time and play state it had, as the mockup's `hoverMs` does. A click on an Offered fill that `fill_reason` accepts calls `click_fill`; a click on a Taken fill or a band selects that step; a click on a "can't" fill puts its reason in the guide line; a click on the axis seeks for good. A click on the highway keeps working through `fill_at_image` and follows the same three rules. Escape clears the selected step, as in the mockup. The player's path is the only path drawn; `shown_row`, `click_windows`'s copy rule and the left list go. `open_path_file` drops its `branches` argument and Open path makes the first path in the file the player's path as one undo step, as it does today (Q2). A chart with no Star Power shows `kNothingToBuildText` and no timeline. Every string the player reads is a named constant.

The steps panel, beside the highway: `step_sentence` per activation, the selected one offering Remove ("Remove" on the last, "Remove this and later ones" on an earlier one, as in the mockup) and, on the last, its `end_choices` with timing and cost, then `leftover_text` and `glossary_text`; with no activation it says "None yet." and the mockup's sentence.

Scan rows. The executor adds two rows to `tests/single_owner/rules/` in the form ADR 0029 describes: one that flags a drawing call or a `song_fraction` use under `src/pathbuilder/` (the timeline is the Preview's), and one that flags `kSpActivationBars` used in a comparison under `src/pathbuilder/` and `src/ui/preview_timeline.cpp` outside `fill_reason`. No rule names a deleted file today (checked in `tests/single_owner/rules/app.json`, `core.json` and `store/concept-H4.json`); the executor greps again.

Questions and owners: none new; the screen reads `guide_text`, `candidate_costs`, `fill_reason`, `step_sentence`, the ladder and the Preview.

Tests: `test_pb_state.cpp` keeps undo and redo and loses the row pins, and gains a pin that a newly opened chart has its empty path. `pbtest_window.cpp`: `open-starts-empty` (opens a corpus chart; `text` holds the intro, "Step 1." and three score labels, and no "Your decisions"; a pinned fill under two saved phrases is not the earliest glowing one); `no-star-power`; `file-open-error`. `pbtest_coach.cpp`: `finish-for-me` and `tight-window-note` rewired to the steps panel's text; `impossible-click-message` becomes `cant-activate-reason` (click `**/##fill_<tick>` on a fill under two bars; the guide line holds `fill_reason`'s sentence).

Run: `hydra_tests.exe "-sf=*test_pb_state.cpp"`; `"-sf=*test_pb_files.cpp"`; `"-sf=*single_owner*"`; `hydra_pathbuilder_uitest.exe --test open-starts-empty --test no-star-power --test file-open-error --test finish-for-me --test tight-window-note --test cant-activate-reason --test paste-refused-keeps-path`.

Done when they pass and `HydraPathBuilder.exe <corpus chart>` builds a path by clicking. Blockers: EN-4, PV-3, CO-1, CO-2 (merged, or their executor commits merged together for an earlyStart).

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

EN-4, PV-3, CO-1 and CO-2 start together at the user's yes (CO-1 forks from EN-4's executor commit). PB-5 forks from the four. RA-2 runs on PB-5's build, with PB-6 running beside it. DOC-2 last. Each merge runs what `hydra_precheck.exe --tests` prints plus the three repo-wide checks (rule 1).

## Out of scope (for the user to confirm)

- A cap on the candidate queue's time. CO-2 measures and reports; no number is invented (Q7).
- Early-fill timing on the timeline (the mockup left it out; `tight_window_note` still names an E0 fill's figure in the sentence).
- Hydra's Preview tab showing the timeline rows (Q5).
- The spec's `kSideColumnShare` and the strip sizes: deleted with their owners.
- The mockup's text labels drawn on the highway ("Fill m3: activate here (−1.2k)", "Star Power 1 ends (m12)", "Squeeze-out: needs 12.3 ms") and the dashed violet ring round a squeezed-out note. The Preview does not draw them; the same facts show in the hover tooltip and the steps panel. Adding them would be new Preview drawing, which is the user's call.
- The mockup's note "(N more were lost because the meter was full)" in a step. The engine records the bars an activation spends, not the phrases lost to a full meter, and counting them in the UI would be a second rule. The step says how many phrases were saved and no more.

## Open questions for the user

Each one is answerable in a line. The code's real rule is quoted with it.

**Q1. Left to right, or edit anywhere?** The mockup refuses a fill before the last activation ("This is before your last activation. You build left to right; Undo goes back a step.") and removes "this and later ones". The earlier rulings read: "Keep it if possible", meaning "The later window keeps the end you chose while it's still possible. Hydra picks a new end only when it isn't, and says so", and for a removal that breaks a later window, "Own sentence": "Removing this window makes one of your later windows impossible, so the path stays as it was." The code today applies those: `repin` in `pb_coach.cpp` (line 190) keeps later windows whole through `best_finish_keeping_later` and falls back to re-picked ends with `kLaterEndRepickedText`. Example one way: Burnout's best path has three activations (`tests/test_activation_rows.cpp` line 545); removing the first under the rulings keeps windows two and three and may say the end of three changed. Example the other way: under the mockup, clicking a fill before window three refuses with the sentence above, and Remove on window one drops two and three. Recommendation: the mockup's rule, because it is what the user approved and it has one sentence per case; `repin`'s keep-later branch and the removal sentence then go unused and CO-2 deletes them. This also settles the question left open in the 2026-10-29 handoff (a click that fits no end, Burnout fill tick 36480): under the mockup's rule that click is before the last activation and is refused with the left-to-right sentence.

**Q2. The left list, "Edit a copy", and the rulings about it.** The mockup has no path list; Hydra's best path is a checkbox that shows it faintly. Two rulings named the list: "Copy it, then apply" (a click while one of Hydra's paths is on screen) and "Show each path once" (opening the same path file twice). Recommendation: delete the list. Open path already makes the first path in the file the player's path as one undo step, and Paste does the same; what goes is the saved rows that a file's other paths used to add to the list, so a path file with several paths now opens its first and ignores the rest. The two rulings become moot and the D-record says so.

**Q3. The sentences.** The user kept PB-2a's labels "as written" on 2026-10-29 ("Keep as written"). The mockup's sentences replace them (the guide line, the step sentence, the hover, "Finish the rest for me"). Recommendation: the mockup's words win, as the later approval.

**Q4. Where the highway goes when you hover a fill.** The mockup seeks 1.4 s before the fill, its own number (`focusTo(f.ms - 1400)`). `seek_activation` puts the activation's chord on the strike line. Recommendation: seek so the fill's span start (`PreviewFill::span.start_ms`) lands on the strike line, so the whole fill is drawn ahead of it; no new number. The other choice is the mockup's lead, which needs a number from you.

**Q5. Should Hydra's Preview tab show the timeline rows too?** PV-3 puts the widget in the Preview's GUI code; the Preview tab keeps its scrub bar. Recommendation: not now.

**Q6. End choices only on the last activation?** The mockup offers ends only on the last one. The code offers `end_choices` on any selected row. Under Q1's recommendation only the last matters; say if you want ends on earlier activations too.

**Q7. The candidate searches.** Each tick in the engine's list after the last window costs one `best_finish`. CO-2 reports the time on the largest corpus chart. Do you want a cap, and if so what happens past it?

## The user's answers (2026-10-30)

The user answered all seven questions in one line: "Yes, as recommended". So:

- Q1: left to right. CO-2 deletes `repin`'s keep-later branch, `best_finish_keeping_later`, `later_end_changed`, `kLaterEndRepickedText`, `removal_impossible_text` and their tests.
- Q2: the left list and "Edit a copy" go. Open path and Paste replace the player's path as one undo step; a path file opens its first path.
- Q3: the mockup's sentences replace the PB-2a wording.
- Q4: hovering a fill seeks so the fill's span start (`PreviewFill::span.start_ms`) is on the strike line. No new number.
- Q5: Hydra's Preview tab does not show the timeline rows.
- Q6: end choices only on the last activation.
- Q7: no cap. CO-2 measures and reports the times.

The same message approved the plan with the "Out of scope" list as written, so the highway text labels, the squeeze-out ring and the "lost because the meter was full" note stay out.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| The builder starts from nothing | the user's first quote; PB-5 | PB-5 | `open-starts-empty` |
| The empty start lights only fills the meter allows | "The empty path" | EN-4, PB-5 | `trailing fills: the path with no activation lists every fill the meter allows`; the pinned fill in `open-starts-empty`; the pin in `test_pb_state.cpp` |
| Fills after the last activation are offered from engine truth, not a UI rule | "Engine truth"; EN-4 | EN-4 | `trailing fills: a corpus best path lists...` (meter at least `kSpActivationBars`), `...agrees with the free search`, the preview-scene pin; the scan row on `kSpActivationBars` comparisons (PB-5) and the reviewer's grep of `src/pathbuilder/` and `src/ui/preview_timeline.cpp` for `sp_phrase_count` and `bank_rise_ticks.size() >=` (none allowed) |
| A variant's trailing fills are its own | EN-4 | EN-4 | `tied variants: a variant's trailing fills are its own` |
| Nothing stored changes; no stamp moves | EN-4 | EN-4 | the executor's report quotes the `record_store.cpp` column list; the reviewer diffs `src/store/` (empty) |
| Every can't-activate fill says why, from facts | "Engine truth"; CO-1 | CO-1, PB-5 | `fill reason: ...` (five cases); `cant-activate-reason` |
| Each cost shows the two numbers it compares | the user's "19k" quote; CO-1, CO-2 | CO-1, CO-2, PB-6 | `fill hover text: ...`; `candidate costs: the best next fill costs 0...`; the hover read in `build-a-path-by-clicking` |
| Costs are one subtraction each, nothing re-priced | "Engine truth" | CO-2 | `candidate_costs` and `cost_list` are the only subtractions in `path_costs.cpp`; the reviewer greps `src/pathbuilder/` for `totalscore` and `replay_path` |
| Green at 0, yellow under 1%, red at 1% or more | spec decision 3 | CO-2 | `cost_tier` unchanged; `candidate costs: a negative cost is a bug` |
| Squeeze figures are Hydra's "needs X ms", violet over the limit | the user's "squeeze out" quote | CO-1 | `step sentence: a squeeze-out over the limit...` |
| The highway is the Preview's and follows hover, click and Play | the user's "highway" quote | PB-5, RA-2 | `build-a-path-by-clicking` reads the clock change after a hover; RA-2 sees Play with audio |
| No drawing routine in `src/pathbuilder/` repeats one the Preview has | the user's "preview code" quote | PV-3, PB-5 | `pb_strip.cpp` is deleted; PB-5's scan row on drawing calls and `song_fraction` under `src/pathbuilder/` |
| One x rule for the scrub bar and the timeline | PV-3 | PV-3 | `draw_scrub_marks` lives in `preview_timeline.cpp`; `preview --test preview-path-overlay` unchanged |
| The saved line is the Preview's meter curve and marks `kSpActivationBars` | PV-3 | PV-3 | `preview timeline: the saved line has a point at each segment start and end...` |
| Hydra's Preview tab looks the same (except offered trailing fills) | PV-3, EN-4 | PV-3, EN-4 | the three Preview uitests pass; EN-4's screenshot at RA-2 |
| Three scores, each from the ladder | mockup | PB-5, PB-6 | `build-a-path-by-clicking`: after Finish, your score equals "best you can still reach" |
| The guide line always says the next step | mockup | CO-1, PB-6 | `guide text: ...`; "Step 1." read on screen |
| Undo, Start over, Finish the rest for me | mockup | PB-5, PB-6 | the three clicks in `build-a-path-by-clicking` |
| Click results that arrive after the path changed are dropped | ruling "Drop it" | CO-2 | the generation check in `DecisionList::start_edit` stays; `candidate queue: a newer request supersedes the older` |
| Every refusal line through `plain_error` | brief | every task | reviewer grep of `src/pathbuilder/` for `e.what()` |
| The user sees it in the real app before the GUI merges | brief | RA-2 | the user's yes in chat |
| One GUI test builds a whole path by clicking real widgets | brief | PB-6 | `build-a-path-by-clicking` asserts only screen text |
| Copy, Paste, Save path, Open path, HY-1, TM-1 stay | "What stays" | PB-5 | `test_pb_files.cpp` and `paste-refused-keeps-path` pass in PB-5's run (only `open_path_file`'s `branches` argument goes) |
| No new number without the user | brief | Q4, Q7 | the questions; executors stop and ask |
| No whole-library run | preamble | every task | every test names `testdata/input`; RA-2 reads one song |
