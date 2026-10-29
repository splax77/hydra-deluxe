# Plan: the Path Builder (Coach version)

Task id `path-builder-plan`, session `d9bea2e5-9589-4187-a0d6-c231e61d8921`. Planner: Fable. Status: plan written and reviewed; the user answered the open questions below on 2026-10-28 and approved this plan and the library plan (`docs/superpowers/plans/2026-10-28-hydra-libraries.md`) on 2026-10-29. Nothing is built yet.

The design is `docs/superpowers/specs/2026-10-28-path-builder-design.md` (the user's decisions; this plan changes none of them). The research is `2026-10-28-path-builder-brainstorm.md` beside it. Every path and line number below was checked against the worktree on 2026-10-28.

## The plain version

The Path Builder is a second program. You open a chart, click fills to build your own Star Power path, and see what each click costs against Hydra's best path. The engine already prices a named path (`search_target`). It needs one new option so a search can follow the player's first windows and then run free. The Preview's live highway moves into a library both programs link (the library plan does that move). A small piece of new render code turns a mouse click on the highway into a chart time. Hydra's song panel gets a button that starts the builder on the open chart.

The work is eleven executor tasks here (EN-1, EN-2, EN-3, RD-1, RD-2, HY-1, PB-1, PB-2a, PB-2b, PB-3, PB-4), plus a docs task that gets the doc review (DOC-1), the user's real-app check (RA-1), and the library plan's tasks T2, T4 and T5, which stand in for four tasks this plan first had (PV-1, PV-2, SH-1, UT-1; their text below stays as the record and is not built). Two tasks start at once (EN-1 and RD-1), beside the library plan's first waves. Everything else waits on one of them or on a library task; "Waves, in one line each" says which. A task that adds a `.cpp` to a library adds only its own line to that library's source list in `CMakeLists.txt`, beside its neighbours, and only after the library plan's T1c has rewritten those lists; the library plan's tasks rearrange the library lines, and PB-1 waits for them.

## Open questions for the user

Each quotes what the code really does and gives my recommendation. All seven are answered; the answers come first, and each question keeps the text it was asked with, so where a recommendation below differs from the answer, the answer wins.

**The user's answers (2026-10-28).** Q1: violet, `chip_color(ChipToken::t5)`. Q2: add a Start Menu shortcut; HY-1 adds the `[Icons]` line to `installer/hydra.iss`, and CMake's install line still gets the exe (PB-1). Q3: the button sits left of `Hide library`, greyed with a hint when the exe is missing. Q4 and Q5: the recommendations stand (the Q4 sizes are judged at RA-1). Q6: the user asked for a wider plan that makes each part of Hydra a reusable library for Hydra's own programs (no outside users); that plan absorbs PV-1, PV-2, SH-1 and UT-1, and this plan's waves wait on it for those pieces. Q7: dropped. A scan of every path the engine keeps on all 97 test charts (`hydra_replay dump` then `target --ticks` per path) found no two variants that print the same text, so the reader takes the only reading; PB-3's corpus round-trip test is what would catch a chart that reads two ways, and such a chart goes to the user. On 2026-10-29 the user approved this plan, the library plan and that plan's three recommendations (no folder moves, `Song` moves to `core/`, the 14-day cap on its known-gaps list). Also on 2026-10-29, asked whether the builder should share Hydra's taskbar entry, the user answered "Its own entry": the builder's shortcut and process set no `AppUserModelID`, so Windows gives it a taskbar button of its own.

**Q1. The flagged-gain colour (the spec's open item).** The three cost colours reuse the report chips: green is `ChipToken::t0` (79,191,148), yellow `t1` (224,177,63) and red `t3` (242,104,107), all from `app/html_page.h`'s `kChipRgb*` through `ui::chip_color` (theme.h line 107). The fourth colour must not be mistaken for one of them. The three colours in `src/ui/theme.h` that are not cost colours all sit close to one: `kAccentColor` teal (0,180,180) is near the green, `kStarPowerColor` gold (255,204,51) is near the yellow, and `kWarningColor` orange (255,127,0) is near the orange `t2` and reads as a problem. A gain is good news with a warning attached ("needs 4 ms, the limit is 10 ms"). Recommendation: the report chip violet, `chip_color(ChipToken::t5)` (167,139,250). No cost tier uses it, it is not an error colour, and `tests/test_theme.cpp` already checks its contrast on the window background and on a selected row (the loop over every `ChipToken`, lines 120 to 126), so no new contrast row is needed. The other choice is the teal; nothing in `tests/` checks `kAccentColor`'s contrast on the window background today, so it would need a new row. You see the result at the real-app check (RA-1).

**Q2. The installer needs a CMake line, not an .iss line.** The spec says "the installer script adds HydraPathBuilder.exe". The script (`installer/hydra.iss` line 76) copies the whole staging folder: `Source: "{#HYDRA_STAGE}\*"`. What puts an exe in that folder is `install(TARGETS Hydra hydra_batch hydra_fillcompare RUNTIME DESTINATION .)` in `CMakeLists.txt` line 660. So the real change is adding `HydraPathBuilder` to that line (task PB-1), and the .iss needs no edit for the file itself. One thing the .iss does decide: Start Menu shortcuts (`[Icons]`, line 88, Hydra only). Recommendation: no shortcut for the builder. It is reached from Hydra's song panel, and a shortcut would make a second Start Menu entry for one install. If you want one, HY-1 adds an `[Icons]` line under the same `AppUserModelID`.

**Q3. Where the "Open in Path Builder" button sits.** Rule 3 sends a display change to you first. The song panel's header (`render_panel_header`, `src/ui/details_panel.cpp` lines 37 to 82) draws `Hide library`, `<`, `>` and `X` at the right edge. Recommendation: a button `Open in Path Builder` left of `Hide library` in that row, greyed with a hint when `HydraPathBuilder.exe` is not beside `Hydra.exe` (a dev build that did not build the target). The alternative is a button next to `Copy path` under the Paths tab, which ties it to a path; the builder opens a chart, not a path, so the header fits better.

**Q4. The strip's marker size and the decision list's dot size.** The spec decides no pixel sizes for the strip's fill markers or the colour dot in each decision row. These are layout sizes like every other `px()` value in Hydra, not thresholds, but they are new numbers. Recommendation: the executor sizes them like the Preview's scrub marks (`draw_scrub_marks` in `preview_tab.cpp`) and the version bar's dot (`version_bar.cpp`), and a marker's click area is exactly its drawn rectangle, no extra tolerance. The faint outline's line width (RD-2) is the same kind of layout number. You see them at RA-1 and can change them there.

**Q5. The 1% rule in whole numbers.** Decision 3 says yellow under 1% of BF(0), red at 1% or more. Scores are whole numbers. The new `cost_tier` (EN-3) would compare `100 * cost >= bf0` for red, so no float is involved and a cost exactly at 1% is red, as the spec says. When BF(0) is not a multiple of 100, "under 1%" means `100 * cost < bf0`. Recommendation: that rule, pinned by a boundary test at `bf0 = 1000` (cost 10 is red, 9 is yellow) and at `bf0 = 1001` (cost 10 is yellow, because 1% of 1001 is 10.01). Say so if you want rounding instead.

**Q6. What the builder borrows from `hydra_ui`.** The spec moves only the Preview into its own library. But the window set-up the builder needs sits in `hydra_ui` beside `AppState`: `setup_imgui`, `ui_scale_for_dpi` and `kClearColor` are in `src/ui/app_shell.h`, whose `.cpp` also holds `run_frame(AppState&)`; the colours and `apply_theme` are in `theme.cpp`; the file dialog is in `win32_dialogs.cpp`. A builder that links `hydra_ui` for those pulls `AppState` and, through it, nearly all of Hydra's GUI into its exe, which is what the Preview move was meant to avoid. Recommendation: one more task (SH-1, wave 2) that moves `setup_imgui`, `shutdown_imgui`, `ui_scale_for_dpi`, `kClearColor`, `theme`, `win32_dialogs` and the capture of frame text into a second small library beside `hydra_preview`, leaving `run_frame` and the layout notes in `hydra_ui`. The alternative is to skip SH-1 and link `hydra_ui` whole: less work, a bigger exe, and PV-2 then buys nothing for the builder. The builder passes its own `ImGuiSetupOptions::ini_file` (`hydra_pathbuilder_ui.ini`), so it never shares `hydra_ui.ini`'s window placement with Hydra.

**Q7. A pasted path text that fits more than one path.** Hydra's path text drops the ticks: `3-` means "three fills passed over, then a squeeze-out", and it does not say which note was squeezed out. When the reader (PB-3) tries a fill for a token, the engine can return several squeeze variants of that window whose text is the same token, for example two different phrases squeezed out, each printing `-`. Which one a later token continues from changes where the next window can start. Recommendation: take the first variant `search_target` returns, which is the best-scoring one, as the Paths tab lists variants. The alternative is to refuse a text that has more than one reading and say so. Say if you want the refusal.

PB-3 (reading Hydra's path text back) itself is not a question. Decision 4 says path text copies both ways between the programs, and the spec's Errors section covers a pasted path, so PB-3 is required and has no "no" answer.

## What the code does today (checked)

**The pinned search.** `EngineOptions` (`src/search/engine.h` lines 25 to 56) has `target_act_ticks` and `target_deact_ticks`. The rule the spec names sits in `Engine::run` (`src/search/engine.cpp` lines 1848 to 1863): on every fill node, when `target_act_ticks_` is set, a listed fill keeps only the activating child and any other fill keeps only the declining parent. The pinned end is answered at the SP node by `pinned_end` (lines 560 to 571), which looks the activation's tick up in `target_act_ticks_` and reads the matching `target_deact_ticks_` entry; that part stays as it is for free-after.

**The Path limit.** `act_within_limit` (lines 1155 to 1163) walks one activation's squeezes and its early fill against `ms_filter_`. `close_last_activation` (line 1165) marks a path `over_prefix` when its newest window fails it, and `passes_ms_filter` (line 1169) is `!over_prefix && act_within_limit(act_tail)`. `reduce_iteration_paths` (line 1351) flags every failing path in `filtered_`; with `hard_ms_filter_` it removes them (line 1375); otherwise `reduce_group` (line 1210) lets an over-limit path survive only while no inside path in its group outscores it (`can_outscore`, line 1193). A second reader is the lookahead at line 811, which counts upcoming fills outside the limit for the group key (`ready_class`). So "the limit never drops a pinned window" is one change in `act_within_limit`: an activation whose fill tick is in `target_act_ticks_` passes, through the same lookup `pinned_end` makes, plus the same exemption in the line-811 lookahead so the group key agrees. That is what makes a negative cost possible (decision 6): the pinned window is judged by the player, the rest by the limit.

**The target run.** `run_target` (`src/search/pather.cpp` lines 256 to 280) builds `EngineOptions` with the widest points band (`kKeepEveryPathBand`) and no `ms_filter`, runs the search, then `keep_target_paths` keeps only paths whose activations are exactly the pinned ticks. `search_target` over windows (lines 303 to 357) sorts and validates the pins, runs once, and on failure finds the breaking window by binary search and names the reason (`activation`, `window_end`, `sqout`). `search_graph` (line 245) builds the graph; it sits in an anonymous namespace. `analyze_chart` (line 437) builds the graph once and runs the main search and the all-0 pass over it.

**Costs from best-finish values only.** The builder stores one list of totals, BF(0) to BF(n), plus the player's own total from a full pin. Every cost is a difference of two entries in that list. Nothing re-prices a window with `core/replay.h`; the replay is used only by the test that checks identity 2.

**The Preview's dependencies.** `preview_controller.cpp` includes `app/config.h`, `app/user_messages.h`, `audio/device.h`, `audio/player.h` and `ui/preview_load_job.h`. `preview_load_job.cpp` includes `ui/library_parts.h` for `time_left_text` (which pulls `ui/app_state.h` and so the whole GUI) and `ui/widgets.h` for `progress_fraction` (which pulls `imgui.h` and `ui/theme.h`). `preview_load_job.h` includes `ui/job_base.h`, a header-only base that includes only `app/user_messages.h` and `core/error_kind.h`. `preview_transport.*` include only `app/` and `audio/` headers. `preview_tab.cpp` (stays in Hydra) includes `ui/preview_controller.h`, `ui/details_parts.h`, `ui/theme.h`, `ui/text_role.h` and `ui/widgets.h`. So the snags are exactly two helpers: `time_left_text` (with `format_duration`, declared in `library_parts.h` lines 66 to 72, defined in `library_dialogs.cpp`) and `progress_fraction` (`widgets.h` line 86). `PreviewController::open` takes a `store::ChartLibraryEntry`; the load job reads only `entry.notespath`, `entry.md5` and `entry.timing` (`preview_load_job.cpp` lines 70 to 81, `preview_controller.cpp` line 84). The builder has no database, so it fills an entry from the chart file (`app::hash_chart_file` for md5, `app::chart_timing_meta` for timing), which is what the Preview does for a row with no stored timing.

**The highway on screen.** `project_to_image` (`src/render/overlay_layout.cpp` lines 18 to 28) projects a world point through `make_camera` and `track_rect` onto the image. Fills light all four lanes (`highway_draw.cpp` lines 323 to 331), between `pad_x` left and right, at depths `z_of(t1)` to `z_of(t2)` on the floor `T.y`. `z_to_time` (`highway_draw.h` line 107) already inverts depth to time. Which fill a time falls in is the track state's `fill` and `fill_taken` sweeps (`SpanSweep::at`, `track_state.h` line 120), which end a fill `kSpanEndTicks` past its last note. `name_box("##highway_image", ...)` in `preview_tab.cpp` line 633 gives the image's screen rectangle, so a test can turn a widget rectangle into image pixels.

**Settings and launch.** `app::Settings::load()` reads `ini_path()`, which is `exe_dir()\hydra_settings.ini` (`src/app/config.h` lines 29 and 49). The installer puts every exe in one folder, so the builder beside `Hydra.exe` reads Hydra's own file with no path change. `Settings::reading()`, `search_depth_mode()` and `batch_run()` give the chart reading and the `SearchSettings`; `fill_rule_for(settings.legacy_fills)` gives the fill rule, as `preview_tab.cpp` passes it. The one process launcher in `src/` is `launch_html_viewer` in `src/app/report_files.cpp` (lines 40 to 59, `CreateProcessW`); it is file-local and names the html viewer, so a sibling-exe launcher is new code beside it.

**The headless runner.** `tests/ui/uitest_harness.{h,cpp}` holds the WARP device, the engine, `frame()`, `visible_text`, `widget_dump`, `screenshot` and the shared helpers; `uitest_commands.cpp` is the verb interpreter. The harness owns `std::unique_ptr<AppState> app` and refers to it 36 times in `uitest_harness.cpp`; the interpreter touches it once (`state`) and `wait-idle` asks `jobs_busy(h)`. So the builder cannot link the harness as it is. The library plan's T5 (first called UT-1 here) splits the app-free part out.

**The uitest test engine files combos with an empty label**, so the builder's combos are reached through a sibling checkbox like Hydra's (`settings_control`), or by `##id`.

## Tasks

Each task fits one Opus executor inside 100 tool calls and 20 minutes, in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`. Each runs only the tests it names. After each task one fresh reviewer (Sonnet) reads the change and its documents, fixes what it finds, commits on the same branch and signs off; then it merges (D107, one review per change, no second round). Tests pin literals from one run or call the production function; none recomputes an expected value. The engine tests run on `testdata/input` only.

### Wave 1 (EN-1 and RD-1 start together; HY-1 waits for the library plan's T1c; PV-1 is replaced by that plan's T2)

#### EN-1. The free-after option and the limit exemption

Owns `src/search/engine.h`, `src/search/engine.cpp`, `src/search/pather.h` (the `keep_target_paths` comment and a new `keep_prefix_paths` declaration only), `src/search/pather.cpp` (`keep_target_paths` and the new prefix keeper only), `tests/test_search_target.cpp`.

What it builds. `EngineOptions` gains `bool free_after = false`. The fill rule at engine.cpp line 1848 becomes: a targeted search follows the pins for every fill whose tick is at or before the last pinned activation; with `free_after` set, a fill past it branches as a normal search does (the `no_skips` branch is not involved, because a free-after search runs with `no_skips` off). With `free_after` set and nothing pinned there is no last pin, so every fill branches and the search is the plain one; the engine builds `target_act_ticks_` from an empty vector in that case, so the rule must test for it. Every reader of the Path limit asks one new private helper, "is this fill tick pinned", which is the lookup `pinned_end` already makes (`std::binary_search` on `target_act_ticks_`): `act_within_limit` (which feeds `close_last_activation`, `passes_ms_filter`, the soft filter in `reduce_group` and the hard removal in `reduce_iteration_paths`) looks up the activation's tick, `node(acts_[act].act_node).tick`, and a pinned activation passes; the `ready_class` lookahead (engine.cpp line 811) skips a pinned fill when it counts upcoming fills over the limit (`fill_tick_[k]`), so the group key agrees with `act_within_limit`. Those are the only two places that read `ms_filter_` for a path; the executor greps `ms_filter_` and `has_ms_filter_` in `engine.cpp` and lists any other reader before finishing. `keep_prefix_paths(paths, windows)` keeps paths whose first `windows.size()` activations are the pinned ticks and whose pinned ends match, with the same variant rescue `keep_target_paths` does; `keep_target_paths` stays exact and both call one shared matcher so the activation-and-end check has one owner.

Questions its code answers, and the owner: "may this fill branch under a pinned search" (the fill rule in `Engine::run`); "is this activation's fill pinned" (the new helper, read by `pinned_end`, `act_within_limit` and the lookahead); "does this path start with the pinned windows" (`keep_prefix_paths`).

Tests, in `test_search_target.cpp`:
- `free after: with nothing pinned the search is the plain search` (every corpus chart: `run_search` with `free_after` and empty `target_act_ticks` equals `run_search` without, path strings and totals).
- `free after: fills past the last pin branch, fills before it follow the pins` (a corpus chart with at least three activations on its best path: pin the first, assert the returned paths all start with it and at least one activates elsewhere later).
- `free after: a pinned window over the Path limit survives a soft and a hard limit` (pin a window the unlimited best uses that fails a 0 ms hard limit; with `free_after` and `hard_ms_filter` the path still comes back; without the pin it does not).
- `keep_prefix_paths keeps a prefix match and drops a path that activates earlier`.

Run: `build-cpp\Release\hydra_tests.exe -sf=*test_search_target.cpp -sf=*test_engine_sweeps.cpp -sf=*test_tied_variants.cpp -sf=*test_fills_and_skips.cpp -sf=*test_sp_end.cpp -sf=*test_sp_end_clamp.cpp -sf=*test_sp_meter_curve.cpp -sf=*test_replay.cpp -sf=*test_preview_scene.cpp`. These are the test files that set `target_act_ticks`, call `search_target` or `keep_target_paths`, or set `ms_filter` on a search (found by grep on 2026-10-28); the executor greps again and adds any new one.

Done when the four new tests pass and every existing test in `test_search_target.cpp` still passes. Blockers: none. earlyStart: none; starts at the user's yes.

#### PV-1. The two helpers move to a neutral home

**Replaced (user's answer to Q6).** The library plan's T2 does this move, with the wording unchanged, and also moves `job_base.h` to `src/app/`. This text is kept as the record; do not build it.

Owns `src/app/display_format.h`, `src/app/display_format.cpp`, `src/ui/library_parts.h`, `src/ui/library_dialogs.cpp`, `src/ui/widgets.h`, `src/ui/preview_load_job.cpp` (its two include lines only), `src/ui/library_jobs.cpp` (if it calls either helper), `tests/test_batch_text.cpp`, `tests/single_owner/rules/ui.json` (the `gated_time_left_text` row's owner lines, if the paths move).

What it builds. `format_duration`, `time_left_text` and `progress_fraction` move into `app/display_format` (already the home of `format_ms_whole` and `format_percent`, per the phase-3 plan). `library_parts.h` and `widgets.h` stop declaring them and include `app/display_format.h` instead; every caller keeps working. `preview_load_job.cpp` includes `app/display_format.h` instead of `ui/library_parts.h` and `ui/widgets.h`. No wording changes.

Questions: "how long is left, in words" (`time_left_text`, one home); "what fraction of a job is done" (`progress_fraction`, one home).

Tests: the existing `time_left_text` cases in `test_batch_text.cpp` (lines 44 and 45) and `test_preview_load_progress.cpp` (lines 240 to 248) pass unchanged; the single-owner scan passes.

Run: `hydra_tests.exe -sf=*test_batch_text.cpp -sf=*test_preview_load_progress.cpp "-sf=*single_owner*"`.

Done when the three runs pass and `preview_load_job.cpp` includes nothing under `ui/` but `ui/preview_load_job.h` and `ui/job_base.h`. Blockers: none.

#### RD-1. A mouse position becomes a chart time, and a time becomes a fill

Owns `src/render/overlay_layout.h`, `src/render/overlay_layout.cpp`, `src/render/track_state.h`, `src/render/track_state.cpp`, `tests/test_overlay_layout.cpp`, `tests/test_track_state.cpp`.

What it builds. `image_to_floor_time(cfg, width, height, point, now_s, speed)` in `overlay_layout`: the inverse of `project_to_image`. It unprojects the image point through the same `make_camera` and `track_rect`, intersects the ray with the floor plane `y = cfg.track.y`, and hands the depth to `z_to_time`. It returns no value when the ray misses the floor or lands above the far end or behind the strike line. `TrackState::fill_at(t)` in `track_state`: the index of the fill (offered or taken) whose drawn span holds `t`, from the same `fill` and `fill_taken` sweeps the highway draws with, so a click lands exactly where the strip is lit. The track state records each fill interval's scene index when `set_overlay_intervals` builds them.

Questions: "where on the floor is this image pixel" (`image_to_floor_time`, the one inverse); "which fill is lit at time t" (`TrackState::fill_at`, reading the sweeps that light the lanes).

Tests:
- `image_to_floor_time inverts project_to_image on the floor` (several world points on the floor between `z_now` and the far end, at two image sizes, round-trip within 1 ms).
- `image_to_floor_time refuses a point above the far end` and one below the strike line.
- `fill_at names the fill lit at t and none between fills` (a scene with two fills, one offered and one taken, pinning the index at each span's first note, at its end `kSpanEndTicks` past the last note, and none one tick later).

Run: `hydra_tests.exe -sf=*test_overlay_layout.cpp -sf=*test_track_state.cpp`.

Done when the tests pass. Blockers: none.

#### HY-1. Hydra's "Open in Path Builder" button and the launcher

Owns `src/app/launch.h` (new), `src/app/launch.cpp` (new), `src/app/report_files.cpp` (its `launch_html_viewer` only), `src/ui/details_panel.cpp`, `src/ui/details_parts.h`, `tests/test_launch.cpp` (new), `tests/ui/uitest_details.cpp`, `installer/hydra.iss` (the Q2 shortcut), and one line in `CMakeLists.txt`: `src/app/launch.cpp` in `hydra_app`'s source list, next to `src/app/library_query.cpp`. That list exists only after the library plan's T1c has split today's `hydra_core` list, so HY-1 forks from T1c's merge. A task that adds a `.cpp` to a library adds only its own line there, beside its neighbours and not at the end of the list, so two tasks in one wave do not touch the same line. (`tests/test_*.cpp` files are picked up by the glob at CMakeLists.txt line 557 and need no line.)

What it builds. `app::sibling_exe_path(name)` (the exe beside `exe_dir()`) and `app::launch_sibling(name, args)`, a `CreateProcessW` wrapper that quotes each argument and reports a failure through `plain_error` (the launch is the one new reader of `CreateProcessW` beside `launch_html_viewer`, `src/app/report_files.cpp` lines 40 to 59, which today writes the quoting inline as `"\"" + exe + "\" \"" + page + "\""`; the two share the quoting rule, so `launch.cpp` owns the quoting and `launch_html_viewer` calls it). A test seam beside `set_installed_for_test` (`src/app/config.h`) lets a test point `sibling_exe_path` at a scratch folder. The button from Q3, which runs `HydraPathBuilder.exe <notespath>` for `app.selected->notespath`, greyed with a hint when `sibling_exe_path` finds no file, and a status-line error from `plain_error` when the launch fails. The Start Menu shortcut from Q2: one new line in `installer/hydra.iss`'s `[Icons]` section, named `Hydra Deluxe Path Builder` (the spec's window title), pointing at `{app}\HydraPathBuilder.exe`, with no `AppUserModelID` (the user's answer of 2026-10-29: the builder gets its own taskbar entry, not Hydra's). PB-1's `main.cpp` sets no id either, so the shortcut and the running process agree.

Questions: "where is the builder exe" (`sibling_exe_path`); "how is a command line quoted" (`launch.cpp`); "may the button be clicked" (the one `exists` check in the button's draw); "does the installer give the builder a shortcut" (the `[Icons]` section of `installer/hydra.iss`).

Tests: `test_launch.cpp` pins the quoting of a path with spaces and the missing-exe answer, and has one case, `installer: the builder has a Start Menu shortcut`, that reads `installer/hydra.iss` through `sourcetree::root()` and checks that an `[Icons]` line names `{app}\HydraPathBuilder.exe`. `uitest_details.cpp` gains `open-in-path-builder-greyed` and `open-in-path-builder-enabled`: both set the seam to a scratch folder, the first with no builder exe in it (the button is drawn disabled with its hint) and the second with an empty file named `HydraPathBuilder.exe` (the button is enabled); each asserts the label and the disabled flag through `widget_dump`, and that the button's rect ends left of `Hide library`'s rect (Q3). The seam is needed because `build-cpp\Release`, where `hydra_uitest.exe` runs, holds the real builder exe once PB-1 builds, so a test that relied on its absence would start failing. Launching is not tested headless; RA-1 covers it.

Run: `hydra_tests.exe -sf=*test_launch.cpp -sf=*test_library_jobs.cpp` (the one existing test file that reaches `open_in_browser`); `hydra_uitest.exe --test open-in-path-builder-greyed --test open-in-path-builder-enabled --test panel-open-close --test panel-hide-library --test layout-sweep --test panel-headline`.

Done when both pass. Blockers: the library plan's T1c. earlyStart: fork from T1c's merge. The button is a Hydra layout change, so it is also seen at RA-1 before merge.

### Wave 2

#### EN-2. The best-finish function

Owns `src/search/pather.h`, `src/search/pather.cpp` (except the two functions EN-1 owns, which it merges in), `tests/test_best_finish.cpp` (new).

What it builds. `TargetResult best_finish(const ScoreGraph&, const SearchSettings&, std::vector<PinnedWindow> windows)` and a `Song` overload that builds the graph through `search_graph`, which becomes public so the builder builds one graph per chart. It validates the pins as `search_target` does (same checks, same `std::invalid_argument` texts, shared with it in one validator), runs `run_search` with `free_after`, the pins, the user's `depth_mode`, `depth_value` and `ms_filter` (soft, as `analyze_chart`'s `read` passes them; `hard_ms_filter` off), then `keep_prefix_paths`. With no window pinned it returns what `read` returns. When nothing comes back it calls `search_target` over the same windows and copies its `realized_prefix`, `failed_tick` and `failed_reason`, so the failure is the existing one. `paths.front()` is the best finish; `paths.front().totalscore()` is BF(k).

Questions: "what is the best path that starts with these windows" (`best_finish`); "why can these windows not happen" (`search_target`'s failure, reused, not re-derived); "which options does a best-finish run use" (one function in pather.cpp, `best_finish_options`, read by nothing else).

Tests, in `test_best_finish.cpp`, over `testdata/input`:
- `best finish: with nothing pinned it is Hydra's best path on every corpus chart` (identity 1: equals `analyze_chart(...).best_path()` string and total).
- `best finish: every window of a corpus best path pinned in full, with no free fill left, equals search_target and the replay` (identity 2: for each corpus chart's best path, `pinned_windows(path)`, then `best_finish` and `search_target` give the same total, and `replay_path`'s total agrees). A path whose best finish adds an activation after its last window is not that identity; the test pins full windows and compares the path that activates nowhere else, by filtering the returned paths to the pinned count.
- `best finish: pinning the best path's first window keeps the best total` (BF(1) == BF(0) on a chart where the best path's first window is inside the limit; pinned from one run).
- `best finish: an impossible window reports search_target's reason` (a tick that is not a fill node gives `failed_reason == "activation"`).
- `best finish: a window tighter than the Path limit can beat the limited best` (the negative-cost case from decision 6: the executor searches the corpus with a script capped at 10 minutes for a chart whose unlimited best total exceeds its 10 ms best; if one exists, pins BF(1) > BF(0) with its first unlimited window; if none, the test is left out and the report says which charts were tried).

Run: `hydra_tests.exe -sf=*test_best_finish.cpp -sf=*test_search_target.cpp`.

Done when the tests pass. Blockers: EN-1. earlyStart: fork from EN-1's executor commit as soon as `free_after` and `keep_prefix_paths` exist; do not wait for its review.

#### PV-2. The Preview becomes its own library

**Replaced (user's answer to Q6).** The library plan's T4 builds `hydra_preview` with the files left in `src/ui/` (no `src/preview/` folder, as the user approved on 2026-10-29). This text is kept as the record; do not build it.

Owns `CMakeLists.txt`, `src/preview/` (new folder: `preview_controller.{h,cpp}`, `preview_load_job.{h,cpp}`, `preview_transport.{h,cpp}`, `job_base.h` moved from `src/ui/`), the include lines of `src/ui/preview_tab.cpp`, `src/ui/app_state.h`, `src/ui/app_state.cpp`, `src/ui/library_jobs.h`, `src/ui/library_jobs.cpp`, `src/ui/dm_jobs.h`, `src/ui/update_job.h`, `src/ui/report_window.h` and every other includer of the moved headers (the executor greps `ui/preview_` and `ui/job_base` across `src/`, `tests/` and `tools/`), `tests/test_preview_controller.cpp`, `tests/test_preview_load_progress.cpp`, `tests/test_preview_transport.cpp`, `tests/single_owner/rules/*.json` rows that name the moved paths, `CONTEXT.md` (the Preview paragraph's paths only).

What it builds. `add_library(hydra_preview STATIC ...)` with the six files, `PUBLIC hydra_render hydra_audio`, `PRIVATE hydra_warnings`; `hydra_ui` links `hydra_preview PUBLIC` and drops the three files. The namespace stays `hydra::ui` so no call site changes except the include path. Hydra's Preview tab, tests and behaviour are untouched: this is a move.

Questions: none new. The library split is a build fact; `CMakeLists.txt` is its one home.

Tests: the existing Preview unit tests and UI scripts, as the spec requires.

Run: `hydra_tests.exe -sf=*test_preview_controller.cpp -sf=*test_preview_load_progress.cpp -sf=*test_preview_transport.cpp -sf=*test_preview_boxes.cpp "-sf=*single_owner*"`; `hydra_uitest.exe --test preview --test preview-path-overlay --test preview-controls --test preview-mode-reload --test preview-activation-jumps`.

Done when all pass with no test edited beyond include lines. Blockers: PV-1. earlyStart: fork from PV-1's executor commit.

#### SH-1. The window set-up without AppState (if Q6 says yes)

**Replaced (user's answer to Q6).** The library plan's T2 does this split as `hydra_uibase`, with one hook field the text below did not foresee. This text is kept as the record; do not build it.

Owns `src/ui/app_shell.h`, `src/ui/app_shell.cpp`, `src/ui/imgui_setup.h` (new), `src/ui/imgui_setup.cpp` (new), `src/ui/theme.h`, `src/ui/theme.cpp`, `src/ui/win32_dialogs.h`, `src/ui/win32_dialogs.cpp`, every includer of a moved declaration (the executor greps them), `tests/single_owner/rules/*.json` rows that name the moved functions, and the `CMakeLists.txt` library lines.

What it builds. `add_library(hydra_uibase STATIC ...)` holding `imgui_setup.cpp`, `theme.cpp` and `win32_dialogs.cpp`, linked `PUBLIC` by `hydra_ui`. `imgui_setup` takes from `app_shell` everything that needs no `AppState`: `ImGuiSetupOptions`, `setup_imgui`, `shutdown_imgui`, `kClearColor`, `ui_scale_for_dpi` and the `FrameText` capture that the headless runner reads. `app_shell` keeps `run_frame(AppState&)` and the layout notes (`library_share`, `library_hidden`, report placements). The functions move as they are; no wording or number changes. Hydra's look and behaviour are untouched.

Questions: none new; this is a move. `ImGuiSetupOptions::ini_file` already lets a program choose its own ini file.

Tests: the existing theme, text-role and GUI-shell tests, and the GUI scripts below, pass with nothing edited beyond include lines.

Run: `hydra_tests.exe -sf=*test_theme.cpp -sf=*test_text_role.cpp "-sf=*single_owner*"`; `hydra_uitest.exe --test scan --test panel-open-close --test startup-screen`.

Done when all pass. Blockers: PV-2 (the library lines in `CMakeLists.txt`). earlyStart: fork from PV-2's executor commit.

#### UT-1. The app-free half of the headless runner

**Replaced (user's answer to Q6).** The library plan's T5 does this split as `hydra_uitest_engine`, with five callbacks instead of three. This text is kept as the record; do not build it.

Owns `tests/ui/uitest_engine.h`, `tests/ui/uitest_engine.cpp` (new), `tests/ui/uitest_harness.h`, `tests/ui/uitest_harness.cpp`, `tests/ui/uitest_commands.h`, `tests/ui/uitest_commands.cpp`, `tests/ui/uitest_script.cpp`, `tests/ui/uitest_live.cpp`, `docs/agents/ui-testing.md` (the Layout list only), and the `hydra_uitest_harness` source list in `CMakeLists.txt`.

What it builds. `uitest_engine` holds what needs no `AppState`: the WARP device and offscreen target, the engine setup and `frame()`, `FrameText` capture, `visible_text`, `on_screen`, `widget_dump`, `item_line`, `screenshot`, `wait_until`, `idle_frame`, `escape_ref`, `text_width`, and the verb interpreter with two callbacks the app supplies: `draw_frame()` and `jobs_busy()`, plus a `state_text()` callback for `state`. `uitest_harness` keeps `Harness` as a thin layer over it with the `AppState`, `reset_app`, the gates and every Hydra helper, so the existing tests compile unchanged. The builder's runner (PB-1) links `uitest_engine` and supplies its own three callbacks.

Questions: "what does a verb mean" stays in `uitest_commands.cpp` (ADR 0030); "what is on screen" stays `visible_text`.

Tests: the existing runner tests prove nothing moved.

Run: `hydra_uitest.exe --test scan --test panel-open-close --test preview --test startup-screen --test report-window-open-path`; `hydra_uitest.exe --script tests/ui/scripts/smoke.txt`; `hydra_tests.exe -sf=*test_live_protocol.cpp`.

Done when all pass. Blockers: SH-1 (it moves the `FrameText` capture that `uitest_engine` reads, and both edit the `CMakeLists.txt` library lines; if Q6 says no, the blocker is PV-2). earlyStart: fork from SH-1's executor commit.

#### EN-3. The cost list

Owns `src/app/path_costs.h` (new), `src/app/path_costs.cpp` (new), `tests/test_path_costs.cpp` (new), and one line in `CMakeLists.txt`: `src/app/path_costs.cpp` in `hydra_app`'s source list (the library plan's T1c makes it), next to `src/app/path_view.cpp`.

What it builds. `struct BestFinishLadder { std::vector<int64_t> bf; int64_t player_total; }` and `cost_list(ladder)`: cost k is `bf[k-1] - bf[k]` for k in 1..n, and the last line "after your last window" is `bf[n] - player_total`. `cost_tier(cost, bf0)`: `Green` at 0, `Gain` below 0, `Red` when `100 * cost >= bf0`, else `Yellow` (Q5). `build_ladder(graph, settings, windows, progress)`: calls `best_finish` for each prefix length 0..n and `search_target` for the player's total, in order, reporting each value as it lands so the UI fills as answers arrive; it stops at the first impossible prefix and carries that failure. Device-free; the builder's worker thread calls it.

Questions: "what did window k cost" (`cost_list`, the only subtraction); "which colour is a cost" (`cost_tier`); "what is BF(k)" (`best_finish`, EN-2; never re-priced here).

Tests:
- `cost list: the costs sum exactly to the gap` on three hand-built corpus paths (the best path with one window moved, pinned from one run; the all-0 path; a two-window path), asserting `sum == bf[0] - player_total` and the pinned numbers.
- `cost tier: the 1% boundary` (Q5's four cases) and `cost tier: a negative cost is a gain`.
- `build ladder: stops at the first impossible prefix and names it`.

Run: `hydra_tests.exe -sf=*test_path_costs.cpp`.

Done when the tests pass. Blockers: EN-2 and the library plan's T1c (the source list). earlyStart: fork from the merge of both.

### Wave 3

#### PB-1. The builder's main window

Owns `src/pathbuilder/` (new: `main.cpp`, `pb_state.{h,cpp}`, `pb_window.{h,cpp}`, `pb_paths.{h,cpp}`, `pb_open.{h,cpp}`, `pb_menu.{h,cpp}`, `pathbuilder.rc`), `src/ui/win32_dialogs.{h,cpp}` (one new function, a file-open dialog beside `browse_for_folder`), `CMakeLists.txt` (the new exe, its runner, and `HydraPathBuilder` added to the `install(TARGETS ...)` line at line 660), `tests/pb_ui/` (new: `pbtest_main.cpp`, `pbtest_harness.{h,cpp}`, `pbtest_window.cpp`), `tests/test_pb_state.cpp` (new).

What it builds. `HydraPathBuilder.exe`: a Win32 window and swapchain copied in shape from `src/ui/main.cpp` (the executor reads lines 228 to 330 and reuses `setup_imgui`, `apply_theme`, `ui_scale_for_dpi` and `kClearColor` from `hydra_uibase`, which the library plan's T2 makes; the window class name and title are the spec's, `Hydra Deluxe Path Builder`; it passes `ImGuiSetupOptions::ini_file` = its own `hydra_pathbuilder_ui.ini`, never `hydra_ui.ini`, and leaves T2's `before_ini_load` field null; unlike `src/ui/main.cpp` line 221, it sets no `AppUserModelID`, so it gets its own taskbar entry and agrees with HY-1's shortcut, which names none), linking `hydra_preview`, `hydra_uibase`, `hydra_app`, `hydra_replay_json`, `imgui` and `d3d11`, the list the library plan gives it (the test engine comes in through the runner only). `pb_state` holds the open chart: `Settings::load()` is read on every open (so a settings change in Hydra applies to the next chart; nothing reads it per frame), the chart parsed through `app::resolve_preview_song` with `settings.reading()` and `fill_rule_for(settings.legacy_fills)`, `require_notes` checked first, the `HydraRecord` from `analyze_chart` on a worker, the `ScoreGraph` from `search_graph` (public after EN-2) kept for EN-3's ladder, and a `ChartLibraryEntry` filled from the file for `PreviewController::open`. It also holds undo and redo as a vector of player states with no cap. `pb_paths` is the left list: Hydra's best path, the best all-0 path, the near-best paths from the record, and the player's saved branches (none until PB-4), each with its total and gap from the best, plus "Edit a copy". `pb_open`: File > Open for a chart and `WM_DROPFILES`. `pb_menu` draws the menu bar, with File > Open and Edit > Undo and Redo; PB-4 adds its items there. Errors: a failed load shows `plain_error` and `plain_error_detail`; a chart with no SP phrases opens and says there is nothing to build. The player's path is drawn on the highway as the Preview draws any path (`PreviewController::open` with the player's `Path`).

The headless runner `hydra_pathbuilder_uitest` links `hydra_uitest_engine` (the library plan's T5) and supplies that engine's five callbacks for the builder: `draw_frame`, `jobs_busy`, `state_text`, `reset` (reopens the chart) and `is_library_row_title` (always false, because the builder has no library). It registers its tests. It takes `--test`, `--list`, `--script`, `--keep-temp` and `--shots` with the same meanings; the executor copies the flag parsing shape from `uitest_main.cpp` and leaves out `--live`, `--db` and `--ini`.

Questions: "which settings does the builder run under" (`Settings::load()` through `batch_run()` and `reading()`, Hydra's owners); "what are the paths on the left" (the record, `HydraRecord::paths`, `allzero_paths`); "what does an error say" (`plain_error`).

Tests: `test_pb_state.cpp` pins open-from-path on a corpus chart (record present, path list count = paths + all-0 + 0 branches), the no-SP chart's message, a failed path's `plain_error` sentence, and undo and redo over three edits. `pbtest_window.cpp` has `open-and-list` (opens a corpus chart from the command line, reads the left list's totals through `text`), `no-star-power` and `file-open-error`.

Run: `hydra_tests.exe -sf=*test_pb_state.cpp`; `hydra_pathbuilder_uitest.exe --test open-and-list --test no-star-power --test file-open-error`.

Done when all pass and `HydraPathBuilder.exe <corpus chart>` opens and plays. Blockers: the library plan's T2, T4 and T5 (T4 also waits on T1c, so all of them are in by then). earlyStart: fork from the merge of the three.

#### RD-2. The best path's faint outlines on the highway

Owns `src/app/preview_view.{h,cpp}` (a list on `PreviewScene` of the fills a second, "ghost" path activates, and an optional second path for `apply_preview_overlay`), `src/render/track_state.{h,cpp}` (a ghost sweep built in `set_overlay_intervals` beside `fill` and `fill_taken`), `src/render/highway_draw.{h,cpp}` (the outline), `src/ui/preview_controller.{h,cpp}` (`set_ghost_path(const Path*)`, which rebuilds only the overlay as a mode change does), `tests/test_highway_draw.cpp`, `tests/test_track_state.cpp`, `tests/test_preview_scene.cpp`.

What it builds. The spec says the best path's activations are drawn as faint outlines. `highway_draw` has no outline routine today, so the outline is drawn as the offered-fill strip's rectangle, edge only, at `fill_offered_alpha`. Its line width is a layout size like the others in Q4; the executor stops and asks for any other new number. With no ghost path set, nothing changes in Hydra.

Questions: "which fills does the ghost path take" (`apply_preview_overlay`, the owner of Offered, Taken and now ghost; the ghost reads the same rule as Taken, not a copy of it).

Tests: a scene with a ghost path lists exactly that path's taken fills, pinned from one run (`test_preview_scene.cpp`); the track state's ghost sweep holds those spans and none when no ghost is set (`test_track_state.cpp`); `highway_draw` emits the outline only for ghost spans (`test_highway_draw.cpp`).

Run: `hydra_tests.exe -sf=*test_highway_draw.cpp -sf=*test_track_state.cpp -sf=*test_preview_scene.cpp -sf=*test_preview_controller.cpp`; `hydra_uitest.exe --test preview --test preview-path-overlay`.

Done when all pass with Hydra's Preview unchanged. Blockers: RD-1 (same files), the library plan's T4 (the controller's library; the file stays in `src/ui/`). earlyStart: fork from T4's merge, merging RD-1.

#### PB-3. Reading Hydra's path text back

Owns `src/search/pathstring_read.h` (new), `src/search/pathstring_read.cpp` (new), `tests/test_pathstring_read.cpp` (new), and one line in `CMakeLists.txt`: `src/search/pathstring_read.cpp` in `hydra_search`'s source list (the library plan's T1c makes it), next to `src/search/pather.cpp`.

What it builds, and why it is required. Decision 4 says path text copies both ways between the programs, and the spec's Errors section covers a pasted path, so the builder must read `Path::pathstring` back. Today only the writers exist: I searched `src/` and `tools/` for `pathstring`, `parse_path`, `from_pathstring` and `notation`, and `hydra_replay` reads paths as `act:deact` tick lists or JSON (`tools/replay.cpp` lines 94 to 112). A token is `Activation::notationstr` (`src/core/model.cpp` line 547): an optional `E` when the early fill was critical, then the number of fills passed over before the activation (`Activation::skips()`, the fills the search charged a skip for), then one `+` for each SqIn and `-` for each SqOut. It is not CHOpt's phrase count. The text does not carry ticks, so the reader finds them with the engine: `paths_from_pathstring(song, settings, text)` splits the text into tokens, and for each token tries the song's remaining fills in order as the next activation, running `search_target` on the windows found so far (each pinned in full from the returned path, `pinned_windows(path)`) plus the candidate pinned by activation only. It accepts the candidate whose last window's `notationstr()` equals the token and takes that window's end from the returned path. It returns the matching `Path`, or throws `std::invalid_argument` naming the token (by its position) that no fill could place. It lives in `search/` and not beside `Path::pathstring` in `core/model`, because it needs `Song` and `SearchSettings`, which `core/model.h` cannot include; the round-trip test below is what keeps it in step with `notationstr`.

Questions: "which fill does this token mean" (the reader, one owner; the engine answers whether a fill can be activated and how many it passed, not the reader); "what does the token mean" (`notationstr`, the writer).

Tests: `pathstring read: every corpus best path round-trips` (for every corpus chart and every path in its record, the path read back from `pathstring(p)` has the same activation ticks and the same `pathstring()` as `p`); `pathstring read: a token no fill can place is refused with its position`; `pathstring read: "(No activations.)" gives a path with no activations`.

Run: `hydra_tests.exe -sf=*test_pathstring_read.cpp`.

Done when the tests pass. Blockers: the library plan's T1c (the source list); it uses `search_target` as it is today. earlyStart: fork from T1c's merge.

### Wave 4

#### PB-2a. The decision list, end choices and "Finish for me"

Owns `src/pathbuilder/pb_coach.{h,cpp}` (new), `src/pathbuilder/pb_state.{h,cpp}` (from PB-1), `src/pathbuilder/pb_window.cpp` (the right column only), `tests/test_pb_coach.cpp` (new), `tests/pb_ui/pbtest_coach.cpp` (new).

What it builds. The worker that runs `build_ladder` (EN-3) whenever the player's path changes, superseding an older run through the progress callback (the callback throws when a newer request exists, the way Hydra's `AnalysisCancelled` unwinds). The decision list: one row per window with `Path::pathstring`'s token for it, its tier colour (Q1 for the gain), its cost, and the timing it needs (the activation's own `within_ms_limit` inputs, read off the returned path). The gain row's note names the window's timing and the limit from `settings.ms_filter`. Clicking a row calls `PreviewController::seek_activation`. Selecting a row lists its end choices: the window's distinct ends from `search_target` with that activation pinned alone (every squeeze variant comes back), each with its timing and the best finish with that end pinned (`best_finish` with the `deact_tick` set), run on the worker when selected. "Finish for me" replaces everything after the last window with the path behind BF(n). Clicking an offered fill to start a window pins only the new activation, runs `best_finish`, and keeps the end it chose; an impossible click shows the `failed_reason` in plain words through `plain_error`'s owners and changes nothing.

Questions: "what does this row say" (`cost_list` and `cost_tier`, EN-3); "what ends can this window have" (`search_target` with the activation pinned alone; variants are the ends); "what does Finish for me do" (BF(n)'s path, nothing recomputed).

Tests: `test_pb_coach.cpp` pins, on one corpus chart, the ladder for a two-click path, the row texts, the end-choice list for the first window, that "Finish for me" yields BF(n)'s path string, and that an impossible click leaves the path unchanged with the reason text. `pbtest_coach.cpp`: `decision-list-fills` (reads the rows through `text` after `wait-idle`), `finish-for-me`, `impossible-click-message`.

Run: `hydra_tests.exe -sf=*test_pb_coach.cpp`; `hydra_pathbuilder_uitest.exe --test decision-list-fills --test finish-for-me --test impossible-click-message`.

Done when all pass. Blockers: PB-1, EN-3. earlyStart: fork from PB-1's executor commit, merging EN-3's branch.

#### PB-2b. The strip and clicks on the highway

Owns `src/pathbuilder/pb_strip.{h,cpp}` (new), `src/pathbuilder/pb_window.cpp` (the highway and strip area only), `tests/test_pb_strip.cpp` (new), `tests/pb_ui/pbtest_strip.cpp` (new).

What it builds. The strip under the highway: a marker per fill (hidden ones left out, as the highway leaves them out), and the step line from BF(0) on the left down by each window's cost to the player's total on the right, read from EN-3's ladder and nothing else. A click on a marker does what a click on the highway does. The highway click: the mouse position inside `##highway_image` goes through RD-1's `image_to_floor_time` with the transport's position and the renderer's speed, then `TrackState::fill_at`; an offered fill starts a window (PB-2a's click handler), a taken one removes it. Marker sizes per Q4. It also hands Hydra's best path to `PreviewController::set_ghost_path` (RD-2), so the best path's activations show as faint outlines.

Questions: "where did the points go" (the step line reads `cost_list`); "which fill was clicked" (`image_to_floor_time` then `fill_at`, RD-1).

Tests: `test_pb_strip.cpp` pins the step line's points for the PB-2a ladder (first point BF(0), last the player's total, each step one cost). `pbtest_strip.cpp`: `highway-click-starts-window` (reads `rect ##highway_image`, projects a taken fill's centre through `project_to_image`, clicks there, and expects the decision list to gain a row) and `strip-click-removes-window`.

Run: `hydra_tests.exe -sf=*test_pb_strip.cpp`; `hydra_pathbuilder_uitest.exe --test highway-click-starts-window --test strip-click-removes-window`.

Done when all pass. Blockers: PB-2a, RD-1, RD-2. earlyStart: fork from PB-2a's executor commit, merging RD-2's branch.

#### PB-4. Saved path files, copy and paste

Owns `src/pathbuilder/pb_files.{h,cpp}` (new), `src/pathbuilder/pb_menu.{h,cpp}` (PB-1's file; PB-4 adds the items only), `tests/test_pb_files.cpp` (new), `tests/pb_ui/pbtest_files.cpp` (new).

What it builds. Save and open of the builder's own path files through `paths_json` and `windows_from_json` (`tools/replay_json.h`, the `hydra_replay_json` library), the saved branches those files add to the left list, and copy and paste of a path as `Path::pathstring` text. Paste calls PB-3's reader and, when the text does not fit the chart, refuses with that reason and leaves the player's path as it was (the spec's Errors section). Undo and redo (PB-1) cover paste and open.

Questions: "what text is a path" (`Path::pathstring`); "what is in a saved file" (`paths_json`, `windows_from_json`); "which path does this text mean" (PB-3).

Tests: `test_pb_files.cpp` pins a save-then-open round trip, a copy-then-paste round trip on a corpus path, and a refused paste that leaves the path unchanged. `pbtest_files.cpp`: `paste-refused-keeps-path`.

Run: `hydra_tests.exe -sf=*test_pb_files.cpp`; `hydra_pathbuilder_uitest.exe --test paste-refused-keeps-path`.

Done when all pass. Blockers: PB-1, PB-3. earlyStart: fork from PB-1's executor commit, merging PB-3's branch.

### Wave 5

#### DOC-1. The docs and the record

Owns `docs/UserGuide.md` (a new "Path Builder" section after "Preview tab"), `README.md` (one line beside line 16's Preview sentence), `docs/audit/2026-10-03-fix-decisions.md` (one new D-record at the next free number after D263; the executor checks the file), `CONTEXT.md` (a Path Builder paragraph), `docs/agents/ui-testing.md` (a short paragraph on `hydra_pathbuilder_uitest`).

The User Guide section says, in second person and with every label checked against `pb_window.cpp`, how to open a chart, click fills, read the colours (green, yellow under 1%, red at 1% or more, the gain colour), the end choices, "Finish for me", the strip, save and copy. The README line says the installer ships the builder. The D-record copies the spec's decisions in the user's words. The User Guide and README are public pages: public doc style applies and the doc-review gate fires at the merge to `main`.

Run: `hydra_tests.exe "-tc=docs match code*"`.

Done when it passes and the reviewer has signed off the two public pages. Blockers: PB-2b (labels final). earlyStart: fork from PB-2b's executor commit.

#### RA-1. The real-app check (the main session, before the builder's GUI and HY-1 merge)

A check, not a build, capped at 20 minutes, then report what was seen. First headless on the corpus: `hydra_pathbuilder_uitest.exe --script` a short file that opens a corpus chart, clicks two fills, and dumps the decision list and the strip's text. Then the real window on a snapshot copy of the user's library: `hydra_start("window")` through the `hydra` MCP server to see the song panel's new button on a real song, then the button launches the real builder on that chart (the user watches; one screenshot of each for the user). The gain colour (Q1) and the sizes (Q4) are judged here. Rendering is checked on the corpus charts; the library copy is read only for the real song's look (memory rule: render checks use the corpus). Nothing runs over the whole library.

## Waves, in one line each

The library plan replaces four of this plan's tasks (user's answer to Q6, 2026-10-28). That plan is `docs/superpowers/plans/2026-10-28-hydra-libraries.md` (on `main`, and on branch `claude/hydra-libraries-plan`). Its T2 replaces PV-1 and SH-1, its T4 replaces PV-2 (with no folder move, so RD-2 edits `src/ui/preview_controller.*` where it is today), and its T5 replaces UT-1. So PB-1 waits on that plan's T2, T4 and T5, and RD-2 waits on its T4. EN-1, EN-2 and RD-1 can run beside that plan's first waves (EN-1 and EN-2 share one `search/pather.h` include line with its T1a, which git merges). EN-3, HY-1 and PB-3 each add a source line to today's `hydra_core` list, which the library plan's T1c rewrites, so all three fork from T1c's merge. PB-1 links `hydra_app` and implements the runner's five callbacks, as that plan's T5 defines them.

The order, with the library plan's tasks in place of PV-1, PV-2, SH-1 and UT-1. Start now, beside the library plan's first waves: EN-1 and RD-1. EN-2 forks from EN-1. After the library plan's T1c merges: HY-1 and PB-3 fork from it, and EN-3 forks from it and from EN-2. After its T2, T4 and T5 merge: PB-1 forks from the three, and RD-2 forks from T4 and merges RD-1. Then PB-2a after PB-1 and EN-3, then PB-2b (after RD-2 too); PB-4 after PB-1 and PB-3, beside PB-2a. Last: DOC-1, then RA-1, then the merges to `main`. The wave numbers in the task headings above keep their order, not their members. Each merge runs what `hydra_precheck.exe --tests` prints plus the three repo-wide checks (rule 1).

## What a different answer changes

All seven are answered, so this lists what changing an answer later would touch. Q1 the teal instead of the violet: one constant in `pb_coach.cpp` and a new contrast row in `test_theme.cpp`. Q2 no shortcut: HY-1's `[Icons]` line and its test case. A shared taskbar entry instead of the builder's own: the Hydra line's `AppUserModelID` on HY-1's `[Icons]` line and the matching call in PB-1's `main.cpp`. Q3 the Paths tab: HY-1 edits `paths_tab.cpp` instead of `details_panel.cpp`. Q4 other sizes: numbers in PB-2b and RD-2 only. Q5 rounding: `cost_tier` and its boundary test only. Q6 (the library plan) is approved; dropping it would bring back PV-1, PV-2, SH-1 and UT-1 as written above. Q7 is dropped; if the corpus round-trip test ever finds a chart that reads two ways, that chart goes to the user and the pick rule in PB-3 is the only thing that changes.

## Out of scope (for the user to confirm)

- Charts longer than the corpus are untimed (the spec's open item). The worker thread and the superseding callback keep the window live; no number backs a cap, and none is invented here.
- The "Why here?" button, the category diff, practice mode, the browser version and CHOpt-style counts (left out by the spec).

## Promise map

| Promise | Where it is written | Task | Check that proves it |
|---|---|---|---|
| With nothing pinned the free-after search is Hydra's best path exactly | spec "Two rules for the free part"; plan EN-1, EN-2 | EN-1, EN-2 | `free after: with nothing pinned the search is the plain search`; `best finish: with nothing pinned it is Hydra's best path on every corpus chart` |
| Free-after applies only past the last pinned activation; before it the pins are followed | spec "The engine change"; plan EN-1 | EN-1 | `free after: fills past the last pin branch, fills before it follow the pins` |
| Pinned window ends and squeeze-out checks work as today | spec "The engine change" | EN-1, EN-2 | existing `search_target: every corpus path pinned in full comes back alone, the same` still passes in EN-1's and EN-2's runs |
| The Path limit never drops a pinned window | spec "Two rules"; plan "The Path limit" | EN-1 | `free after: a pinned window over the Path limit survives a soft and a hard limit` |
| Best finish returns search_target's existing failure | spec "best finish"; plan EN-2 | EN-2 | `best finish: an impossible window reports search_target's reason` |
| Identity 1 | spec Testing 1 | EN-2 | the corpus test above |
| Identity 2 (full pin equals search_target and the replay) | spec Testing 2 | EN-2 | `best finish: every window of a corpus best path pinned in full ... equals search_target and the replay` |
| Identity 3 (costs add up exactly to the gap) | spec Testing 3, "Costs" | EN-3 | `cost list: the costs sum exactly to the gap` |
| Every cost is read from BF values; nothing re-prices | spec "Costs" | EN-3 | `cost_list` is the only subtraction; the reviewer greps `pb_*` for `replay_path` and `totalscore` outside the ladder |
| Green at 0, yellow under 1%, red at 1% or more, exactly at 1% red | spec decision 3, "Colours" | EN-3 | `cost tier: the 1% boundary` |
| A negative cost shows as a gain in a fourth colour with its note | spec decision 6 | EN-3, PB-2a | `cost tier: a negative cost is a gain`; PB-2a's row text pin for a gain row (when the corpus has one; else the unit test on a hand-built ladder) |
| A negative cost is pinned if a corpus chart has one | spec Testing, last line | EN-2 | `best finish: a window tighter than the Path limit can beat the limited best`, or the report names the charts tried |
| A new window gets the end that gives the best finish; the player can change it | spec "Costs" last paragraphs | PB-2a | `test_pb_coach.cpp` end-choice pin; `decision-list-fills` |
| "Finish for me" is BF(n)'s path | spec "Costs" | PB-2a | `finish-for-me` and the unit pin |
| The list fills on a worker; a long chart never freezes the window | spec "Speed" | PB-2a | `decision-list-fills` reads rows after `wait-idle`; the superseding callback has a unit test in `test_pb_coach.cpp` (a newer request cancels the older) |
| Hydra looks and behaves the same after the Preview move | spec "How it is built" | library plan T2, T4 | the named Preview unit tests and uitest scripts in T2's and T4's runs pass with no test edited beyond include lines (that plan's promise map owns the check) |
| `time_left_text` leaves `AppState` behind | spec "How it is built" | library plan T2 | T2's "Done when": `preview_load_job.cpp` includes nothing from `src/ui/` but its own header (the layering test shows it); the single-owner scan |
| The builder reads Hydra's settings through the same owner | spec "How it is built" | PB-1 | `test_pb_state.cpp` opens under a scratch `hydra_settings.ini` with Hard difficulty and pins the reading |
| It neither reads nor writes Hydra's database | spec | PB-1 | the builder links no `open_store` call (reviewer grep of `src/pathbuilder/` for `open_store` and `RecordStore`) |
| Both ways of opening a chart | spec decision 2 | HY-1, PB-1 | `open-in-path-builder-greyed` and `open-in-path-builder-enabled`; `open-and-list` (command line); `file-open-error`; drag-drop seen at RA-1 |
| The button sits left of `Hide library`, greyed with a hint when the exe is missing (Q3) | Q3 answer | HY-1 | the same two uitest cases assert the disabled flag and that the button's rect ends left of `Hide library`'s |
| The installer gives the builder a Start Menu shortcut (Q2) | Q2 answer | HY-1 | `installer: the builder has a Start Menu shortcut` in `test_launch.cpp` reads `installer/hydra.iss` |
| Saved files use `hydra_replay`'s JSON shape | spec "Saving" | PB-4 | the save-then-open round trip in `test_pb_files.cpp` goes through `paths_json` and `windows_from_json` |
| Copy and paste use Hydra's path text, both ways | spec decision 4 | PB-3, PB-4 | `pathstring read: every corpus best path round-trips`; the copy-then-paste round trip in `test_pb_files.cpp` |
| A pasted path that does not fit is refused with the reason; the path stays | spec Errors | PB-3, PB-4 | `pathstring read: a token no fill can place is refused with its position`; `paste-refused-keeps-path` and the unchanged-path pin in `test_pb_files.cpp` |
| An impossible click shows search_target's reason and changes nothing | spec Errors | PB-2a | `impossible-click-message` and the unit pin |
| A chart that fails to load shows Hydra's message | spec Errors | PB-1 | `file-open-error` pins the `plain_error` sentence |
| A chart with no SP phrases opens and says there is nothing to build | spec Errors | PB-1 | `no-star-power` |
| Every error line comes from `plain_error` / `plain_error_detail` | spec Errors | PB-1, PB-2a | reviewer grep of `src/pathbuilder/` for `e.what()` (memory rule: none at a display site) |
| Fills clickable on the highway and the strip | spec decision 7 | RD-1, PB-2b | RD-1's three tests; `highway-click-starts-window`; `strip-click-removes-window` |
| The strip's step line starts at BF(0), steps by each cost, ends at the player's total | spec "The screen" | PB-2b | `test_pb_strip.cpp` |
| Best path's activations drawn as faint outlines | spec "The screen" | RD-2, PB-2b | RD-2's three pins (scene list, track-state sweep, highway draw); PB-2b hands the best path to `set_ghost_path`; seen at RA-1 |
| The left list: best, all-0, near-best under the depth setting, saved branches, each with total and gap | spec "The screen" | PB-1 | `open-and-list` and the count pin in `test_pb_state.cpp` |
| Undo and redo cover every change | spec "The screen" | PB-1, PB-4 | the undo/redo pin in `test_pb_state.cpp`; PB-4 puts paste and open under it |
| Window title and exe name | spec decision 5 | PB-1 | `pbtest_window.cpp` reads the window name; CMake target name `HydraPathBuilder` |
| The installer ships the builder | spec "Docs and installer"; Q2 | PB-1 | `install(TARGETS ...)` lists it; the reviewer builds the stage with `cmake --install` into scratch and lists the exe |
| The builder has a headless runner driven by label | spec Testing | library plan T5, PB-1 | `hydra_pathbuilder_uitest.exe --test open-and-list` |
| The user sees it in the real app before the GUI merges | spec Testing; CLAUDE.md top rule | RA-1 | the main session's screenshots and the user's yes in chat before the merge |
| User Guide section and README line, doc-reviewed | spec "Docs and installer" | DOC-1 | `-tc=docs match code*` and the reviewer's sign-off on the two public pages |
| One fresh reviewer per change | CLAUDE.md rule 5 | every task | the ref gate refuses a merge without the key's verdict |
| No whole-library run anywhere | brief preamble | every task | every test names `testdata/input`; RA-1 reads one song from a snapshot copy |
| No new number without the user's decision | brief | Q4, Q5 | the questions above; the executor stops and asks for any other |
