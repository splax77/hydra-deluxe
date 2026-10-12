# Plan part B1: optimal, squeezes, scoring, the average multiplier (D115, spec section 9)

Planner a449306f9e574421d, task D115-plan-B1, session 08261223-f3e8-41dc-bce8-512c18b2bbe9. Written 2026-10-12 against the main checkout at 47dd4706. Every file, function and line number below was checked in that tree by Grep or Read; nothing is taken from the spec on trust.

## What this part does, in one paragraph

None of this moves a score. The engine, the graph and the scoring table keep every number; what changes is that a few rules written twice get one home, some names that mean two things get one meaning each, two labels change, and three numbers the docs copy get pinned to the code. The spec says this part moves no score, and the plan proves it with a test that already exists: `tests/test_perf_digest.cpp` pins a digest of the engine's whole answer (every path field) for every chart in `testdata/input` under fixed settings, and fails if a single field moves. `hydra_replay selfcheck` is the second proof: it re-analyzes the corpus and replays every path the engine found, checking the six score categories against the engine's own. Every task that touches scoring code runs both before and after its change and must not repin anything.

## How the tasks are cut

Eight tasks in four waves. Wave 1 has five tasks that own disjoint files and run together. The three later waves are renames. A rename touches the same files the wave-1 tasks touch (model.h, engine.cpp, graph.cpp), so it cannot run beside them. Each rename task forks from the merged tip of the wave before it and owns its files again, alone. That is the only way to keep the brief's "no two tasks own one file" and still rename across twenty files; I read the brief's rule as "one owner per file per wave".

Each task is one Opus executor in its own worktree, with a cold build of `hydra_tests` (3 to 6 minutes, `.\build_cpp.ps1 -Target hydra_tests`). Tasks that run the replay proof also build `-Target hydra_replay`. Every task writes its status line with `status_append.ps1` every 10 tool calls and every 5 minutes, commits by its 110th call, and stops at 150 calls or 30 minutes (brief preamble).

Files other parts also touch, which I flag on every task that owns one: `CONTEXT.md` (every part), `docs/UserGuide.md` (every part, public page), `tests/single_owner/rules/*.json` (every part adds rows; this plan puts each new row in its own new file under the area folder, as ADR 0029 allows, so no two tasks edit one rules file), `src/core/timing.h` and `src/core/timing.cpp` (B2 owns the timing code; B1-1 adds a few lines after `to_multiplier`), `src/search/engine.cpp` (B2 may touch its measure calls; B1-2 touches two score lines and a field name).

## Wave 1

### B1-1: one optimal score, one multiplier-step test, the Paths tab label

Goal: `HydraRecord::optimal_score()` becomes the one place that says what the top score is; one function in `core/timing` says whether a chord crosses a multiplier step; the Paths tab reads the "Average Multiplier" label from a constant; the dead "above optimal" branch and the stale comments in these files go.

Depends on: nothing.

Owns: `src/core/model.h`, `src/core/model.cpp`, `src/core/timing.h` (shared with B2), `src/core/timing.cpp` (shared with B2), `src/core/scoring.cpp`, `src/core/scoring.h`, `src/app/path_view.cpp`, `src/app/path_view.h`, `src/app/display_format.h`, `tests/test_model.cpp`, `tests/test_path_list.cpp`, `tests/single_owner/rules/core/d115-optimal.json` (new), `tests/single_owner/rules/core/d115-multiplier-step.json` (new), `tests/single_owner/rules/app/d115-labels.json` (new), `tests/single_owner/baseline.json` (shared: one dated entry, see below).

What it builds. Today the Paths tab works out the top score itself: `path_view.cpp` line 484 reads `record.paths.empty() ? 0 : record.best_path().totalscore()`, and `HydraRecord::is_optimal` (`model.cpp` line 888) spells the same thing a second time. The task adds `std::optional<int64_t> HydraRecord::optimal_score() const` next to `is_optimal` in `model.h` (line 860): the best path's `totalscore()`, empty when the record has no paths. `is_optimal` becomes "has a value and the path's total equals it". `build_path_buttons` (`path_view.cpp` line 476) reads `optimal_score()` for its `best`, and the all-0 button's "N above optimal" line (line 510) is deleted: `tests/test_engine_sweeps.cpp` line 458 already pins that no all-0 path scores above the optimum on the whole corpus, so that branch can never run. The "N below optimal" line (509) stays, and so does its test ("100 below optimal", `test_path_list.cpp` line 120).

For the multiplier step: `MultSqueeze::applies` (`model.cpp` line 374) asks `to_multiplier(combo + 1) < to_multiplier(combo + chord.count())`, and `category_scores` (`scoring.cpp` lines 30 to 43) finds the same two multipliers as `out.multiplier` (the first note's) and `out.multiplier_after` (the last note's) inside its payment loop. The task adds to `core/timing` one function that returns both ends of the step for a chord of N notes hit at combo C, and one that says whether they differ. Its declaration goes right after `to_multiplier` in `timing.h` (after line 23) and its body right after `to_multiplier`'s in `timing.cpp` (after line 14), nowhere else, so B2's merge of that file is a two-hunk add. `MultSqueeze::applies` calls the "differs" form. `category_scores` sets `multiplier` and `multiplier_after` from the pair and keeps its per-note loop only for paying notes. One edge must not change: a chord with no notes today leaves both at `to_multiplier(combo)`; the executor pins that case in a test first, then makes the function give the same answer for N = 0. `squeeze_advice` (`model.cpp` line 408) walks the per-note multipliers to find which notes sit past the step; that is a different question (which notes cross), read off the owner's output, and it stays.

The label: `display_format.h` gains `inline constexpr const char* kAverageMultiplierLabel = "Average Multiplier";` next to `format_avg_mult`. The Paths tab's breakdown line (`path_view.cpp` line 414, `"Avg. Multiplier:      "`) is built from the constant, padded to the same width as its neighbours (the lines are column-aligned with `right10`; `"Average Multiplier:"` is 19 characters, so the padding shrinks by four spaces to keep the numbers aligned). The Stars tab is B1-4's; the report is A2's (hand-off below).

Comments in these files: `model.h` lines 796 to 798 restate "50 a gem, 65 a cymbal, doubled for a ghost or accent" and now point at `ChordNote::basescore` instead; `model.h` lines 626, 630 and 641, `model.cpp` lines 525 and 526, `path_view.h` line 130 and `path_view.cpp` lines 87 and 100 say "free squeeze" and now say "a squeeze that needs no timing (`SPSqueeze::needs_no_timing`)". `display_format.h` lines 1 and 2 ("so Song Details and the report cannot drift apart") name two callers; they now say "every screen reads these" without a count. I could not find a comment that literally counts `format_avg_mult`'s callers; the other comment the spec means is `report.h` line 52 ("like the Python row"), which is A2's file (hand-off below).

Tests it writes. In `tests/test_model.cpp`, beside the existing `is_optimal` case (line 228): `optimal_score()` equals the best path's total on the fixture, is empty on a record with no paths, and `is_optimal` is false on that empty record. In `tests/test_path_list.cpp`: an all-0 path that ties the top score gets an empty `detail` (neither "below" nor "above"). A new case pins the step function against `to_multiplier` for combos 8, 9, 10, 19, 29 and chord sizes 1 to 5, and pins the empty-chord answer of `category_scores` (red first, before the change, if the executor can make it fail; otherwise it is a pin, not a fix). Existing pins stand: `test_stars.cpp` lines 69 to 89 and `test_replay.cpp` 1206 to 1255 hold the category numbers.

Commands: `.\build_cpp.ps1 -Target hydra_tests`, then `build-cpp\Release\hydra_tests.exe -sf=*test_model.cpp`, `-sf=*test_path_list.cpp`, `-sf=*test_stars.cpp`, `-sf=*test_perf_digest.cpp` (the no-score-move proof; it must pass unchanged), and `-tc="single-owner*"`.

Single-owner rows (ADR 0029, each in its own new file so no task edits a shared rules file). Row 1, "What is a record's optimal score?": owner `HydraRecord::optimal_score` in `src/core/model.cpp`; pattern `best_path\(\)\.totalscore\(\)`; anchor `best_path`; must_match the old `path_view.cpp` line 484 and the old `is_optimal` body; must_not_match `record.optimal_score()`; owner_lines the new body. Row 2, "Does a chord cross a multiplier step?": owner the new function in `src/core/timing.cpp`; pattern `to_multiplier\([^)]*\+\s*1\s*\)\s*<\s*to_multiplier\(`; anchor `to_multiplier`; must_match the old `applies` line. Row 3, "What is the average multiplier called on screen?": owner `kAverageMultiplierLabel` in `src/app/display_format.h`; pattern `"Avg\.? [Mm]ultiplier|"Average Multiplier"`; anchor `ultiplier`; must_match `path_view.cpp`'s old line and `path_report_view.cpp` line 129 (`c.title = "Avg multiplier";`). That report line is A2's file, so until A2 switches it the row would fail; the task parks that one line in `tests/single_owner/baseline.json`'s `known_copies` with a `remove_by` inside the 14-day limit (`kBaselineMaxDays`) and A2 removes the entry when it uses the constant.

Stop conditions. If `test_perf_digest.cpp` goes red, a number moved: stop, do not repin, report the failing digest and the last edit. If the cold build runs past 6 minutes, report it once and keep waiting up to 10; past that, stop and report. At 100 tool calls or 20 minutes, commit what passes and report the rest.

### B1-2: one place adds Star Power points in the engine, and the limit walks agree

Goal: the engine adds Star Power points to the SP category and the running score through one helper; a test pins that `Engine::act_within_limit` and `Activation::within_ms_limit` agree; the engine's `Path::sp` field (banked bars) is renamed so "sp" no longer means both bars and points.

Depends on: nothing.

Owns: `src/search/engine.cpp` (shared with B2: B2 may change its measure calls; this task touches only the lines named here), `tests/test_engine_sweeps.cpp`, `tests/single_owner/rules/search/d115-sp-points.json` (new).

What it builds. `engine.cpp` adds Star Power points by hand twice: lines 973 and 974 (`c.sc[kSlotSp] += e.frontend_points; c.score += e.frontend_points;`) and lines 1086 to 1089 (the same pair for `sp_delta`). One small helper in the engine, say `add_sp_points(Path& p, int32_t points)`, does both adds, and both sites call it. The running-total check in `emit_path` (line 1726, `score_total` of the six slots against `p.score`) already throws if the pair ever drifts, so this is a one-spelling change with a runtime guard behind it.

The limit test. `Engine::act_within_limit` (line 1149) walks the engine's flat arrays; `Activation::within_ms_limit` (`model.cpp` line 543) walks a stored activation. Both call `SPSqueeze::within_limit` and `fill_within_limit`. The `Engine` class is private to `engine.cpp`, so the test goes through `run_search` on the corpus. For every chart in `corpus::chart_paths()`, with the same settings the invariants case uses (lines 54 to 63: cap 4, depth 200 scores, then `ms_filter` 20.0): every path in the filtered record that scores below the top score passes `Path::within_ms_limit(20.0)` (the top score is exempt on purpose, D51 call 2, `engine.cpp` line 1185); and the best path in the deep unfiltered record that passes `within_ms_limit(20.0)` scores no more than the filtered record's best, so the engine's walk drops nothing the model's walk would keep. Both records are already cached by `corpus::analyzed`, so the test costs no new analysis.

The rename: the engine's `Path::sp` (line 282, the banked bars; read at lines 799, 890 to 903, 932, 947 to 949, 963, 1038, 1411, 1420 and the comment at 306) becomes `sp_bars`. Grep `\.sp\b` and `\bsp\b` in `engine.cpp` for the full list; nothing outside the file reads it.

Tests: the new case above in `test_engine_sweeps.cpp`, "the engine's limit walk agrees with Activation::within_ms_limit". Proof of no movement: `-sf=*test_perf_digest.cpp` and `hydra_replay selfcheck` before and after (the before run can be on the unchanged worktree right after the cold build).

Commands: `.\build_cpp.ps1 -Target hydra_tests`, `.\build_cpp.ps1 -Target hydra_replay`, `build-cpp\Release\hydra_tests.exe -sf=*test_engine_sweeps.cpp`, `-sf=*test_perf_digest.cpp`, `-tc="single-owner*"`, `build-cpp\Release\hydra_replay.exe selfcheck`.

Single-owner row: "Where do Star Power points enter a running path?": owner the new helper in `src/search/engine.cpp`; pattern `sc\[kSlotSp\]\s*\+=`; anchor `kSlotSp`; must_match the two old lines; scope `src/search`. (B1-6 renames `kSlotSp`'s sibling `kSlotBase`; `kSlotSp` keeps its name, so the row survives.)

Stop conditions: as B1-1, plus: if `selfcheck` reports any FAIL after the change, stop and report the chart and path it names.

### B1-3: one parse rule for "a cymbal counts only under Pro Drums"

Goal: both chart readers, and the Preview's gem builder, ask one function whether a cymbal marker counts.

Depends on: nothing.

Owns: `src/parse/song.cpp`, `src/parse/song.h`, `src/render/track_state.cpp`, `tests/test_song_parse.cpp`, `tests/single_owner/rules/parse/d115-cymbals.json` (new).

What it builds. The rule is spelled three times: the MIDI reader's `op_note` (`song.cpp` line 906, `allows_cymbals(color) && mode_pro_`), the .chart reader's `note_optype` (lines 1713 to 1715, `allows_cymbals(*pad)` then `if (mode_pro_)`), and the Preview's `gem_of` (`track_state.cpp` line 42, `pro && n.cymbal && allows_cymbals(...)`). The task adds one free function in `parse/song.h`, declared beside the readers, say `bool cymbal_counts(NoteColor pad, bool pro)`, whose body is `pro && allows_cymbals(pad)`, with a comment saying it is the one home of the rule (and that `allows_cymbals` in `core/model.cpp` stays the owner of which pads can carry one). All three sites call it. The answer is identical on every input, so no parse digest moves (`test_song_parse.cpp` line 1101 and `test_perf_digest.cpp` pin the parse digests).

Tests: in `test_song_parse.cpp`, a case that pins the function on the four corners (Yellow with Pro on: true; Red with Pro on: false; Yellow with Pro off: false; Kick: false) and reads one `.mid` and one `.chart` from `testdata/input` with Pro Drums off, checking every note's `cymbaltype` is `Normal`. Not a rule fix, so no red-first run is needed.

Commands: `.\build_cpp.ps1 -Target hydra_tests`, `build-cpp\Release\hydra_tests.exe -sf=*test_song_parse.cpp`, `-sf=*test_track_state.cpp`, `-sf=*test_perf_digest.cpp`, `-tc="single-owner*"`.

Single-owner row: "Does a cymbal marker count?": owner the new function in `src/parse/song.cpp`; pattern `allows_cymbals\([^)]*\)\s*&&\s*(mode_pro_|pro)\b|\b(mode_pro_|pro)\s*&&\s*(n\.cymbal\s*&&\s*)?allows_cymbals\(`; anchor `allows_cymbals`; must_match the three old lines; must_not_match the new calls. The existing row in `rules/core.json` line 1125 lists the old `gem_of` line as a must_not_match example; examples need not exist in the code, so it stays.

Stop conditions: as B1-1.

### B1-4: the Stars tab's column, the Path limit tooltip, and the docs-match markers

Goal: the Stars tab's "Multiplier" column reads "Average Multiplier" from the constant, with a tooltip saying it is the average multiplier that star needs; the Path limit tooltip says what the search does; the star factors, "10 notes" and "15 points" in the guide and `CONTEXT.md` carry markers that `tests/test_docs_match_code.cpp` checks against the code.

Depends on: B1-1 (the constant lives in `display_format.h`). Runs in wave 1 only if B1-1 has merged first; otherwise it is the first task of wave 2. To let it run beside B1-1, the executor may instead write the Stars tab's label as the constant's name and let the build fail until B1-1 merges; I do not recommend that. Simplest: start B1-4 when B1-1's merge lands.

Owns: `src/ui/stars_tab.cpp`, `src/ui/settings_bar.cpp`, `tests/test_docs_match_code.cpp`, `tests/ui/uitest_details.cpp`, and three single lines of shared files: `docs/UserGuide.md` lines 212 and 296 and `CONTEXT.md` line 398 (markers only; the words stay). Both docs are shared with every part, and `docs/UserGuide.md` is a public page under the doc-review gate; a changed line on a document already signed off passes with no new review (CLAUDE.md, Doc review), and the merge will say if it is not.

What it builds. `stars_tab.cpp` line 38 holds `{"Multiplier", false, 0.0f, text}`; it becomes `kAverageMultiplierLabel`. `ColumnSpec` (`ui/column_widths.h` line 57) has no tooltip slot, and `begin_small_table` draws the headers with `ImGui::TableHeadersRow()` (line 51), so the tooltip is drawn after that call the way `library_table.cpp` line 386 does it: when `ImGui::TableGetHoveredColumn()` is the second column, show the text through `wrapped_tooltip` (`ui/widgets.h` line 38). The text is a user decision and is question 2 below; the proposed sentence is "The average multiplier a path needs for this many stars. The cutoff is the Base score times this factor."

`settings_bar.cpp` lines 177 and 178 read "Keep extra paths only when their hardest squeeze or required early fill is within this many ms. Lower or negative values demand more slack." The search tests every squeeze and every required early fill on a path by its own figure, squeezes that need no timing included (`Activation::within_ms_limit`, `model.cpp` line 543, and `Engine::act_within_limit`), so for a negative limit the old sentence is wrong. The new sentence is question 1 below; proposed: "Keep extra paths only when every squeeze and required early fill on them is within this many ms. A path that ties the best score is kept whatever its timing. Lower or negative values demand more slack." No test pins the old text (grep of `tests/` for it finds only the report's column tooltip, which stays).

The markers. `check_markers` (`test_docs_match_code.cpp` line 257) reads `<!-- default: NAME -->` and compares the last number before it on the line with `known_defaults()` (line 236). The task adds entries: `kStarMultipliers[1]` through `kStarMultipliers[7]` (one per star, from `hydra::kStarMultipliers` in `src/core/stars.h` line 25; the marker regex allows brackets), `to_multiplier:x2` (the first combo at which `to_multiplier` returns 2, found by a loop over the production function, never typed as 10), and `kCymbalBonusPoints` (`src/core/model.h` line 146). Then the docs: `docs/UserGuide.md` line 296 lists "0.1, 0.5, 1.0, 2.0, 2.8, 3.6 and 4.4", and each number gets its own marker right after it, because a marker checks only the last number before it; `CONTEXT.md` line 398 lists the same seven and gets the same seven markers; `docs/UserGuide.md` line 212 has "15 points" and "10 notes in", which get `kCymbalBonusPoints` and `to_multiplier:x2` after their numbers. `CONTEXT.md` has no copy of "10 notes" or "15 points" (grep). The test's "every known marker is used somewhere" check (line 443) means the entries and the doc markers land in one commit.

Tests: the marker self-test at line 451 gets one more line for a bracketed name; `tests/ui/uitest_details.cpp` (the Stars tab checks at lines 405 to 448) gains a check that the header "Average Multiplier" is on screen. GUI checks run through `hydra_uitest` as `docs/agents/ui-testing.md` says; the executor runs only the details script that file names, never `--all`.

Commands: `.\build_cpp.ps1 -Target hydra_tests` (and the uitest target `docs/agents/ui-testing.md` names), `build-cpp\Release\hydra_tests.exe -sf=*test_docs_match_code.cpp`, the one uitest script, `-tc="single-owner*"`.

Single-owner rows: none new; the label row is B1-1's and now matches `stars_tab.cpp`'s call. The markers are the pin.

Stop conditions: as B1-1, plus: if the doc-review gate refuses the guide line, stop and report the key the hook prints; never ask for a waiver.

### B1-5: one owner for "is Star Power running at this moment" (M48)

Goal: the half-open rule "running from the chord's strike up to, not including, the window's end" is written once, `running_activation` calls it, and a test pins the highway's sweep to it at every edge.

Depends on: nothing.

Owns: `src/app/preview_view.h`, `src/app/preview_view.cpp`, `tests/test_sp_meter_curve.cpp`, `tests/test_track_state.cpp`, `tests/single_owner/rules/app/d115-sp-running.json` (new).

What it builds. `running_activation` (`preview_view.cpp` line 239) tests `struck_at(now, w->first) && now < w->second` over each activation's `sp_window()` (`preview_view.h` line 113). The highway's `SpanSweep::at` (`render/track_state.cpp` line 78) answers the same rule for the `sp_active` intervals, as the concept audit's V6 verifier showed row by row (`docs/audit/2026-10-11-concept-audit/findings/V6.md`, M48). The task adds one function beside `sp_window()` in `preview_view.h`, say `bool PreviewActivation::sp_running_at(double now) const`, the half-open test through `struck_at`, with the comment naming it the one home. `running_activation` calls it. The sweep is a generic interval machine shared by six span kinds (overdrive, solo, drum lanes, fills, SP, lane), so it keeps its own walk; the test below pins its `sp_active` answer to the owner. If the user wants the sweep to call the owner instead, that is question 6.

Tests: in `test_sp_meter_curve.cpp` (which already tests `sp_window()` at lines 527 to 538), pin `sp_running_at` at six times: before the start, at the start (running), inside, at the end (not running), after, and the join of two back-to-back windows (the second runs). In `test_track_state.cpp`, build a scene with two back-to-back activations and check that `toggle_on_after(sweep state of sp_active at t)` equals `sp_running_at(t)` at the same six times.

Commands: `.\build_cpp.ps1 -Target hydra_tests`, `build-cpp\Release\hydra_tests.exe -sf=*test_sp_meter_curve.cpp`, `-sf=*test_track_state.cpp`, `-sf=*test_preview_boxes.cpp`, `-tc="single-owner*"`.

Single-owner row: "Is Star Power running at this moment?": owner the new function in `src/app/preview_view.h`; pattern `struck_at\(\s*now\s*,\s*\w+->first\s*\)\s*&&\s*now\s*<\s*\w+->second`; anchor `struck_at`; must_match the old `running_activation` line; owner_lines the new body. The existing `sp_window` and `struck_at` rows in `rules/app.json` (line 1198 names the `running_activation` line as an example) stay.

Stop conditions: as B1-1.

## Wave 2

### B1-6: the Notes category stops being called "base" (Path, engine, graph)

Goal: every name for the Notes category on the path side reads "notes", so "base score" means only the chart's 1x total (`chart_base_score`) and a note's value (`ChordNote::basescore`).

Depends on: every wave-1 task merged (it edits `model.h`, `model.cpp`, `engine.cpp`, `graph.cpp`, `path_view.cpp`, `test_model.cpp`, `test_path_list.cpp`, `test_engine_sweeps.cpp`).

Owns: `src/core/model.h`, `src/core/model.cpp`, `src/core/replay.cpp`, `src/search/engine.cpp`, `src/search/graph.h`, `src/search/graph.cpp`, `src/app/path_view.cpp`, `tests/song_digest.h`, `tests/record_fixtures.h`, `tests/test_model.cpp`, `tests/test_path_list.cpp`, `tests/test_preview_scene.cpp`, `tests/test_preview_boxes.cpp`, `tests/test_replay.cpp`, `tests/test_stars.cpp`, and the rules files whose `owner_lines` or `must_match` name these fields: `tests/single_owner/rules/core.json`, `tests/single_owner/rules/core/concept-G4.json`, `tests/single_owner/rules/search/concept-F7.json` (all shared; the executor changes only the lines that hold a renamed name). `CONTEXT.md`'s "Score categories" entry (line 333, "base (the notes)") is a docs sentence listed below, not this task's.

The renames, each checked in the tree:

- `Path::score_base` (`model.h` line 750; read in `model.cpp` lines 586, 664, 849, `replay.cpp` 216 and 226, `engine.cpp` 1921, `path_view.cpp` 423, and the tests above) becomes `Path::notes_points`. The spec fixes this name.
- `OutPath::score_base` (`engine.cpp` line 325, set at 1734) becomes `notes_points`.
- `kSlotBase` (`engine.cpp` line 42, read at 831, 1726, 1734) becomes `kSlotNotes`.
- `EdgeView::basescore` (`engine.cpp` line 77, set at 176, read at 831 and 837) becomes `notes_points`.
- `ScoreGraphEdge::basescore` (`graph.h` line 151) and `ScoreGraph::store_basescore` (`graph.h` 240, `graph.cpp` 239 to 241, called at 140) become `notes_points` and `store_notes_points`.

`CategoryScores::base`, `ChordScoreRow::base`, `ReplayScore::base` and `StarCutoffs::base` are B1-7's, because `graph.cpp` line 140 (`store_basescore(sg.base)`) sits on the seam between the two and the second task must follow the first. Whether the other five `Path` fields (`score_combo`, `score_sp`, `score_solo`, `score_accents`, `score_ghosts`) follow to `combo_points` and so on is question 3; this task does only what is listed unless the user says otherwise.

The digest in `tests/song_digest.h` hashes field values, not names, so the pins in `test_perf_digest.cpp` stay as they are. The scan's "every row's owner is still where the row says" and "listed lines each cover one line of source" cases (`tests/single_owner/scan.cpp` lines 421 and 721) fail if an `owner_lines` entry still spells the old name; the executor runs the scan and fixes each such line in the three rules files.

Tests: none new; the renames are mechanical. The executor uses `Edit` with `replace_all` once per name per file (the preamble forbids patch scripts and `sed -i`), which is about 12 files times 1 to 3 names.

Commands: `.\build_cpp.ps1 -Target hydra_tests`, `.\build_cpp.ps1 -Target hydra_replay`, `build-cpp\Release\hydra_tests.exe -sf=*test_model.cpp`, `-sf=*test_replay.cpp`, `-sf=*test_stars.cpp`, `-sf=*test_path_list.cpp`, `-sf=*test_perf_digest.cpp`, `-tc="single-owner*"`, `build-cpp\Release\hydra_replay.exe selfcheck`.

Single-owner rows: none new. The existing `score_total` row (`rules/core.json` line 2004, `rules/search/concept-F7.json` line 5) keeps its owner; only its example lines may need the new name.

Stop conditions: as B1-2. If the executor passes 80 tool calls with files still unrenamed, it commits the files that build and names the rest in its report for a fresh executor; it never leaves the build broken in a commit.

## Wave 3

### B1-7: the per-chord and replay structs stop calling the Notes share "base"

Goal: `CategoryScores`, `ChordScoreRow`, `ReplayScore` and `StarCutoffs` each name their number for what it is.

Depends on: B1-6 merged (shares `graph.cpp` line 140 and `scoring.cpp`).

Owns: `src/core/scoring.h`, `src/core/scoring.cpp`, `src/core/replay.h`, `src/core/replay.cpp`, `src/core/stars.h`, `src/core/stars.cpp`, `src/search/graph.cpp`, `src/core/model.cpp` (`chart_base_score` and `avg_mult` read `score_ghosts`/`score_accents`, not these; the file is listed in case `squeeze_advice` needs a touch, which by reading it does not), `tools/replay.cpp`, `tools/replay_json.h` (if it names the field), `tests/test_model.cpp`, `tests/test_replay.cpp`, `tests/test_replay_windows.cpp`, `tests/test_preview_controller.cpp`, `tests/test_preview_boxes.cpp`, `tests/test_stars.cpp`, `tests/ui/uitest_details.cpp`, `tests/single_owner/rules/core.json` (shared, example lines only).

The renames:

- `CategoryScores::base` (`scoring.h` line 20; set at `scoring.cpp` 51, 62, 71; read at `graph.cpp` 140 and `scoring.cpp` 111) becomes `notes`.
- `ChordScoreRow::base` (`scoring.h` line 68, set at `scoring.cpp` 111, read by `replay.cpp`'s row lines around 180) becomes `notes`.
- `ReplayScore::base` (`replay.h` line 106, with `total`, `add`, `operator==`, `score_of` and `assign_score` at `replay.cpp` 214 to 232) becomes `notes`. The JSON key `"base"` in `kReplayScoreFields` (`replay.h` line 136) is `hydra_replay`'s output format, pinned by `test_replay.cpp` lines 1395 to 1418; whether it changes to `"notes"` is question 4. Until the user answers, the C++ field is renamed and the key string stays `"base"`.
- `StarCutoffs::base` (`stars.h` line 33, a copy of `Path::chart_base_score()`) and the first parameter of `star_cutoff` (`stars.h` line 29) become `chart_base_score`; `stars_tab.cpp` line 27 and `uitest_details.cpp` lines 416 and 448 read `sc.base`. `stars_tab.cpp` is B1-4's file in wave 1 and is free again here; this task owns that one read.

The `.base` spelling is shared by four structs, so `replace_all` is per file only where a file reads one of them; `test_stars.cpp` (17 hits) and `test_model.cpp` (16) mix `sc.base` and `n.base` and are edited by hand.

Tests: none new. Proof and commands as B1-6, plus `-sf=*test_replay_windows.cpp` and `-sf=*test_preview_controller.cpp`.

Stop conditions: as B1-6.

## Wave 4

### B1-8: "points" means one thing

Goal: a bare `points` no longer names three different numbers.

Depends on: B1-7 merged (shares `model.h`, `graph.h`, `engine.cpp`, `replay.cpp`, `path_view.cpp`, `test_replay.cpp`, `test_model.cpp`).

Owns: `src/core/model.h`, `src/core/model.cpp`, `src/core/backend_value.h`, `src/core/replay.cpp`, `src/search/graph.h`, `src/search/graph.cpp`, `src/search/engine.cpp`, `src/app/path_view.h`, `src/app/path_view.cpp`, `src/ui/paths_tab.cpp`, `tools/replay.cpp`, `tests/test_activation_rows.cpp`, `tests/test_replay.cpp`, `tests/test_replay_windows.cpp`, `tests/test_model.cpp`, `tests/test_path_list.cpp`, `tests/test_preview_boxes.cpp`, `tests/test_store_results.cpp`, `tests/song_digest.h`, `tests/record_fixtures.h`, `tests/replay_util.h`, `tests/ui/uitest_paths.cpp`, and the rules files with example or owner lines naming these: `tests/single_owner/rules/core.json`, `rules/ui.json`, `rules/core/concept-G4.json`, `rules/search/concept-F7.json` (shared; the named lines only).

The renames (question 5 asks the user to confirm the names):

- `BackendSqueeze::points` and `sqout_points` (`model.h` lines 388 and 389; read by `core/backend_value.h` and `engine.cpp` lines 1079 to 1081, `replay.cpp`, `path_view.cpp`) become `sp_points` and `sqout_sp_points`: they are the row's Star Power points, the same thing `ReplayNote::sp_points` (`replay.h` line 148) already calls by that name.
- `ScoreGraphEdge::frontend_points` (`graph.h` line 165), `EdgeView::frontend_points` (`engine.cpp` 78, 182, 973) and `Activation::frontend_points` (`model.h` line 528, set at `engine.cpp` 1946) become `frontend_sp_points`.
- `MultSqueeze::points()` (`model.h` line 424, body at `model.cpp` 460, which just calls `multsqueeze_gain`) and `MultSqueezeView::points` (`path_view.h` line 51, read at `path_view.cpp` 200 and `paths_tab.cpp` 539) become `gain`, the name its owner `multsqueeze_gain` (`scoring.h` line 107) already uses.

The grep that lists every site: `frontend_points|\.points\b|sqout_points|\.points\(\)` over `src`, `tests` and `tools` (102 hits in 25 files at the time of planning; 11 of them are rules-file example lines).

Tests: none new. Proof and commands as B1-6, plus `-sf=*test_activation_rows.cpp` and the one uitest script for the Paths tab that `docs/agents/ui-testing.md` names (`uitest_paths.cpp` line 94 and 348 read on-screen text that does not change; the file is owned only in case a fixture names a field).

Stop conditions: as B1-6.

## Hand-offs to the other parts

To A1 (the store): `SortColumn::AvgMult` and `SortColumn::NoteCount` (`src/store/record_store.h` line 255, `record_store.cpp` lines 198 and 199) have no caller and go; when A1 rebuilds the store's sort keys it leaves them out. The stored average multiplier is already unrounded (`record_store.cpp` line 555 stores `path.avg_mult()` as is); the only rounding is the report's, which is A2's.

To A2 (the report and its window): `path_report_view.cpp` line 129 (`c.title = "Avg multiplier"`) reads `kAverageMultiplierLabel` from `src/app/display_format.h` once B1-1 lands, and A2 removes the baseline entry B1-1 parks for that line. `report.cpp` line 102 stops calling `py_round3` and hands the stored value to `format_avg_mult` at display time; `report.h` line 52's comment ("already round(x, 3)'d, like the Python row") then points at `format_avg_mult`. The report's "Best path only" box becomes "Optimal paths only", its Timing chip "No squeezes" becomes "No timing needed", its "Notes" count column (`path_report_view.cpp` line 146) becomes "Note count", and the stale stylesheet comment in `report_window.cpp` is fixed; all four are A2's by the brief. The fill page's "Notes" count column is in neither brief (question 8).

To B2 (timing): B1-1 adds one declaration after `timing.h` line 23 and one definition after `timing.cpp` line 14, both right under `to_multiplier`, and touches nothing else in those files. B1-2 touches `engine.cpp` at lines 973 to 974, 1086 to 1089, one new helper near `advance` (around line 830), and the `sp` field's reads listed above; if B2 edits `engine.cpp` in the same wave, the merge is hunk-local.

## The docs sentences this area needs (listed, not written)

`docs/UserGuide.md`:

- Line 100 (Path limit): the sentence "keeps an extra path only when its hardest squeeze or required early fill is within this many milliseconds" says what the search does instead, in the tooltip's words (question 1), keeping the rest of the paragraph.
- Line 212 (Multiplier squeeze): "15 points" and "10 notes in" each get a docs-match marker after the number (B1-4 writes these two markers, since the test cannot pass without them).
- Line 214 (Score breakdown): one sentence is added: Notes, Accent Notes and Ghost Notes together are the Base score on the Stars tab; the Accent and Ghost lines hold only the extra 50 each dynamic note adds, and a cymbal's extra 15 stays in Notes. In the same paragraph "A double squeeze" is replaced with the glossary's term for what the sentence describes, a backend squeeze whose note lands within the leeway past the SP end; the writer checks the sentence against `core::backend_row_value` (`src/core/backend_value.h`).
- Line 296 (Stars tab table): the row `Multiplier` becomes `Average Multiplier`, described as the average multiplier that star needs (the base score times this factor), and each of the seven factors gets a marker (B1-4 writes the markers; the row's label and words are the docs task's).
- Lines 342, 374 and 384 (report controls): `Best path only` becomes `Optimal paths only`, and line 384's "It keeps just the optimal paths" gains "of the paths shown", because the report keeps 5 paths per chart.
- The report's column table (line 358 region) renames `Avg multiplier` to `Average Multiplier`, and the `Notes` count column to `Note count`.
- Wherever the guide names the rows of the Backend timings table, it calls them backend rows, the term `CONTEXT.md` defines below.
- The Timing chip's "No squeezes" wording, where the guide quotes it, becomes "No timing needed".

`CONTEXT.md`:

- A new "Optimal path" entry: a path is optimal when its score ties the record's top score, so tied variants are optimal too; owner `HydraRecord::is_optimal` reading `HydraRecord::optimal_score` in `src/core/model.cpp`; the all-0 rule's owners are `allzero_activation` and `allzero_options`.
- A new "Backend row" entry: a chord within the squeeze window of an SP end, as the Backend timings table lists it; its value and leeway come from `src/core/backend_value.h` and the window from `within_squeeze_window`.
- The "Score categories" entry (line 333): "base (the notes)" becomes the Notes line, `Path::notes_points`.
- The "Star cutoff" entry (line 398): the seven factors carry markers (B1-4 writes them).

## Open questions for the user

1. The Path limit tooltip's exact words. Proposed: "Keep extra paths only when every squeeze and required early fill on them is within this many ms. A path that ties the best score is kept whatever its timing. Lower or negative values demand more slack." The guide's line 100 would say the same.
2. The Stars column tooltip's exact words. Proposed: "The average multiplier a path needs for this many stars. The cutoff is the Base score times this factor."
3. Rename breadth: only `score_base` becomes `notes_points` (the spec's words), or all six `Path` fields become `notes_points`, `combo_points`, `sp_points`, `solo_points`, `accent_points`, `ghost_points`? The six-field form reads better beside `notes_points` but roughly doubles B1-6's edits (the names appear in 14 test files, 105 hits).
4. `hydra_replay`'s JSON key `"base"` (the Notes category in its output, `kReplayScoreFields`): keep it as an output format, or rename it to `"notes"` with the field? Nothing under `tools/` reads it by that key (grep), so the cost is the pin in `test_replay.cpp` and any outside reader.
5. The "points" names in B1-8: `sp_points`, `sqout_sp_points`, `frontend_sp_points` and `gain`. Any of them can be something else; they are my choice, not the spec's.
6. M48: the owner function plus a pin test (B1-5 as written), or should the highway's generic sweep call the owner for its `sp_active` field, which means special-casing one of its six span kinds?
7. The marker names `kStarMultipliers[1]`..`[7]`, `to_multiplier:x2` and `kCymbalBonusPoints`, and whether `CONTEXT.md`'s star factors get markers or the numbers leave its prose (the spec allows either).
8. Who owns the fill page's "Notes" count column rename to "Note count": A2 (the report part) or a B1 follow-up? It is in neither brief.
