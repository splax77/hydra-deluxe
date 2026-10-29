# Plan: every part of Hydra becomes a library its own programs can link

Task id `hydra-libraries-plan`, session `d9bea2e5-9589-4187-a0d6-c231e61d8921`. Planner: Fable. Status: plan written; the build waits for the user's yes and for the two answers below. Nothing is built.

The user's request (2026-10-28): "plan out how to make each component of hydra a reusable library". Scope, in the user's words: reusable by Hydra's own programs only (Hydra.exe, the new HydraPathBuilder.exe, the command-line tools, the tests and test runners). No public API, no DLLs, no promises to outside projects. Every path and line number below was read in the worktree on 2026-10-28; the include graph comes from the `#include` lines themselves, not from folder names.

## The plain version

Hydra today is one big static library (`hydra_core`: core, parse, search, store and app, 46 files) with five smaller ones on top (audio, render, dm, ui, image) and one test harness. The folders already read like components, but the include lines do not: the scoring code in `core/` reads the `Song` class out of `parse/`, `parse/` reads the model out of `core/`, the replay in `core/` reaches up into `app/` for a text formatter, `app/` reaches up into `audio/` for the chart sync rule, and the GUI's window set-up sits in the same file as the code that draws Hydra's whole window. So a program that wants only the Preview, or only the window set-up, gets all of Hydra.

The plan makes twelve libraries with a one-way order and keeps every file where it is today. `CMakeLists.txt` becomes the one place that says which file is in which library (headers get listed beside their `.cpp` files), and a new test reads those lists and every `#include` line and fails when a file reaches into a library its own library does not link. Four small moves fix the includes that point the wrong way. The Preview and the window set-up become libraries of their own, and the headless GUI runner splits into an app-free engine and Hydra's harness, so the Path Builder links the engine and brings its own window. Hydra.exe is built from the same files and looks and behaves exactly the same; the proof is that the existing tests for every moved piece pass with nothing edited beyond include lines.

Six executor tasks in four waves. T3 (the layering test) goes first and alone, because it adds the header lines to `CMakeLists.txt` that every later task carries into its new blocks, and because its known-gaps list is each later task's red-then-green proof. T1 and T2 then run together, T4 and T5 together, and T6 closes the record.

## Open questions for the user

Each quotes what the code really does and gives my recommendation. Say "go with the recommendations" to approve both.

**Q1. Files stay in their folders; `CMakeLists.txt` says which library each one is in.** The Path Builder plan moved the Preview into a new folder `src/preview/` (its PV-2) and left the window set-up in `src/ui/`. I checked what a folder move costs. `ui/theme.h` alone is included by 40 files; the headers that would move to a "ui base" folder are included from 60 files in `src/`, `tests/` and `tools/`, and 151 lines in 22 single-owner rule files name paths under `src/ui/`. An executor edits include lines one call at a time, so a folder move for the GUI base does not fit one task's 100 calls, and it changes nothing a program can see. The alternative keeps every file where it is and makes the `add_library` source lists in `CMakeLists.txt` the one owner of "which library is this file in", with headers listed beside their `.cpp` files (CMake accepts headers in a source list and compiles nothing for them). The layering test (T3) reads those lists, so membership is written once. The cost is that `src/ui/` holds files of three libraries (`hydra_uibase`, `hydra_preview`, `hydra_ui`) and `src/app/` holds files of two (`hydra_app`, `hydra_dm`), told apart by the CMake lists and nothing else. Recommendation: no folder moves. If you want the folders to match the libraries, say so, and T2 and T4 each grow a second task for the include-line edits, and the single-owner rows that name the moved paths move with them.

**Q2. The `Song` class moves down from `parse/` to `core/`.** `core/scoring.h`, `core/replay.h` and `core/sqout_chord.h` include `parse/song.h` for the `Song` class (`src/parse/song.h` line 226), and `parse/song.h` includes `core/model.h`, `core/rules.h`, `core/timing.h` and `core/winstr.h`. That is a cycle between the two folders, and two libraries cannot each link the other. The class itself uses nothing from `parse/`: its body (lines 226 to 335) names no `Difficulty` or `ChartReading`, and its three methods in `song.cpp` (`sp_phrase_count`, `note_count`, `check_activations`, lines 644 to 678) plus `apply_timesig`, `check_timing_maps` and `cymbal_counts` read only the model. The loaders (`load_songpath_*`, lines 367 to 416) and the name helpers (`difficulty_name`, `display_title`, `NoNotesError`) stay in `parse/`. Recommendation: a new `core/song.{h,cpp}` holding the class, its small record types (`SongTimestamp`, `SongSection`, `SoloSection`, `DrumLane`, `FillDeadlineRule`), the two default-meter constants and those six functions; `parse/song.h` includes it, so every includer above `parse/` keeps working unchanged. The alternative is one library for `core/` and `parse/` together (`hydra_core` keeps both folders), which costs nothing now and leaves the cycle in the include lines. The executor has a stop rule: if moving the class drags any loader or `Difficulty`-typed function with it, it stops and reports, and the fallback is the one-library answer.

## What the code does today (checked)

**The libraries.** `CMakeLists.txt` builds `hydra_image` (line 291), `hydra_core` (298, forty-six files from `core/`, `parse/`, `search/`, `store/` and `app/`), `hydra_audio` (381), `hydra_render` (408), `hydra_dm` (427: `app/dm_report*`, `net/*`, `app/update_release.cpp`), `hydra_ui` (441, thirty files), `hydra_uitest_harness` (493), `hydra_precheck_lib` (714) and `hydra_replay_json` (755). The programs are `Hydra` (521), `hydra_batch` and `hydra_fillcompare` (548 and 549, linking `hydra_core` only), `hydra_tests` (558), `hydra_uitest` (612), `hydra_bench` (681), `hydra_replay` (692), `hydra_precheck` (743) and the fuzz exes (775). The link lines are one-way already: `hydra_audio`, `hydra_render` and `hydra_dm` link `hydra_core`; `hydra_ui` links all three plus `imgui` and `imgui_test_engine`; the harness links `hydra_ui`.

**The include graph, folder to folder.** I counted every `#include` line under `src/`, `tools/` and `tests/ui/` (the script is in the planner's scratch folder; the counts are what it printed). `parse/` includes `core/` 22 times and `core/` includes `parse/` 3 times (the `Song` cycle, Q2). `core/replay.cpp` includes `app/path_view.h` for `format_measure`. `app/song_length.cpp` includes `audio/frames.h` for `chart_ms_of_audio_ms`, while `audio/song_audio.h` and `audio/stem_reader.cpp` include `app/preview_source.h`: a cycle between `app/` and `audio/`. `render/` includes `app/preview_view.h` from three files (`track_state.h`, `preview_renderer.h`, `highway_draw.cpp`), so the renderer sits above `app/`, as its link line already says. `net/dmbot_client.cpp` includes `app/analysis.h` for `normalize_chart_hash`; it is in `hydra_dm`, which links `hydra_core`, so that is downward. `store/record_store.h` includes `search/graph.h` and `parse/song.h`, and the header uses no name from either (I grepped it for every type they declare); `record_store.cpp` includes neither. `search/graph.h` and `search/pather.h` include `parse/song.h` for `Song` only. `ui/` includes `imgui` from 40 lines; nothing in `src/ui/` includes an `imgui_te_*` header except `main.cpp` under `HYDRA_UITEST_ATTACHED`, yet `hydra_ui` links `imgui_test_engine` PUBLIC (line 476), so every program that links the GUI carries the test engine. `ui/icons.h` includes `d3d11.h` and `hydra_ui` does not link `d3d11`; the exes do. `ui/widgets.h` includes `render/overlay_layout.h` for `ellipsize`, and `ui/app_shell.cpp` includes `audio/mapped_file.h` to map font files, so the GUI base will sit above `render` and `audio`; both are downward includes and stay.

**What is free of `AppState`.** Reading the includes transitively, these `src/ui/` files never reach `ui/app_state.h`: `app_shell.h`, `theme.{h,cpp}`, `win32_dialogs.{h,cpp}`, `imgui_files.cpp`, `fonts.h`, `fallback_fonts.h`, `text_role.h`, `column_widths.{h,cpp}`, `widgets.h`, `badge_pill.h`, `label_layout.h`, `list_scroll.{h,cpp}`, `activation_row_layout.h`, `icons.{h,cpp}`, `generation.h`, `report_state.h`, `job_base.h`, `preview_controller.{h,cpp}`, `preview_transport.{h,cpp}`, `preview_load_job.h`, `dm_jobs.{h,cpp}`, `library_jobs.{h,cpp}`, `library_model.{h,cpp}`, `update_job.{h,cpp}`, `report_window.{h,cpp}`. `preview_load_job.cpp` reaches it through `ui/library_parts.h` (`time_left_text`) and `ui/widgets.h` (`progress_fraction`). `app_shell.cpp` reaches it because one file holds both `setup_imgui` (line 395) and `run_frame(AppState&)` (line 601). Everything else in `src/ui/` draws Hydra's window and needs `AppState`.

**Does `src/app` need splitting?** No file under `src/app/` includes `imgui`, `d3d11` or anything from `src/ui/` (checked transitively). The folder is GUI-free logic and the text that the screens and pages show (`report_view`, `path_view`, `preview_view`, `display_format`), and the lower libraries that need those view types (`render` for `PreviewScene`, `audio` for `PreviewSong`) already link it. It stays one library, `hydra_app`, minus the three DMBot files and `update_release.cpp` that `hydra_dm` already owns.

**The headless runner.** `tests/ui/uitest_harness.cpp` names `app` 39 times and `uitest_commands.cpp` 4 times (`state`, `wait-idle`'s `jobs_busy` and the reset). The device, the engine, `frame()`, `visible_text`, `widget_dump`, `screenshot` and the verb interpreter need no `AppState` once those four readers go through callbacks.

**What a merge runs.** `hydra_precheck` treats any change to `CMakeLists.txt` as shared code and prints `whole suite` (`tools/precheck/test_pick.cpp` line 175; `tests/test_precheck_pick.cpp` line 422 pins it). Five of the six tasks edit `CMakeLists.txt`, so the main session runs the whole suite at five merges. That is the rule working as designed, not a cost to plan around; the tasks are batched so no library block is rewritten twice.

## The target layout

Twelve libraries, lowest first. A library links only what is listed after "links", and the layering test (T3) enforces that its files include only headers from those libraries, their PUBLIC dependencies, and the externals they declare.

`hydra_core` (`src/core/*`, plus `core/song.{h,cpp}` from Q2): the chart model, `Song`, paths, records, scoring, replay, stars, timing, rules, strings and Win32 paths. Links nothing of ours.

`hydra_parse` (`src/parse/*`): the chart, MIDI, `.sng` and `.srb` readers, section names, Note Shuffle, text decoding, `ChartReading`. Links `hydra_core`, `miniz`.

`hydra_search` (`src/search/*`): the graph, the engine, the pather. Links `hydra_core`.

`hydra_store` (`src/store/*`): the database, the chart index, the library file, upgrades, the version stamps. Links `hydra_core`, `sqlite3`. (T1 drops the two unused includes in `record_store.h`; if the compiler then names a `search` type the header really uses, `hydra_store` links `hydra_search` too, and the plan's line here changes.)

`hydra_app` (`src/app/*` except `dm_report.*`, `dm_report_view.*`, `update_release.*`; plus `app/job_base.h`, moved from `src/ui/` by T2): analysis, settings, rules file, reports and their pages, the view-model text, the Preview's source and scene, song length, user messages, update verification, the allocator and the work pool. Links `hydra_parse`, `hydra_search`, `hydra_store`.

`hydra_audio` (`src/audio/*`): decode, mix, stream, player, device. Links `hydra_app`, the codec libraries.

`hydra_image` (`src/image/*`): unchanged.

`hydra_render` (`src/render/*`): unchanged in content. Links `hydra_app`, `hydra_image`, `d3d11`, `dxgi`, `d3dcompiler`.

`hydra_preview` (`src/ui/preview_controller.*`, `preview_load_job.*`, `preview_transport.*`): the Preview's controller, loader and transport. Links `hydra_app`, `hydra_audio`, `hydra_render`. No ImGui.

`hydra_uibase` (`src/ui/imgui_setup.{h,cpp}` new, `theme.*`, `win32_dialogs.*`, `imgui_files.cpp`, `fonts.h`, `fallback_fonts.h`, `text_role.h`, `column_widths.*`, `widgets.h`, `badge_pill.h`, `label_layout.h`, `list_scroll.*`, `activation_row_layout.h`, `icons.*`, `generation.h`): ImGui start-up and shutdown, the DPI scale, the theme, the fonts, the shared widgets, the folder dialog, the frame-text capture. Links `hydra_app`, `hydra_audio` (`mapped_file` for the fonts), `hydra_render` (`ellipsize`), `hydra_image`, `imgui`, `d3d11`, `ole32`.

`hydra_dm` (`src/app/dm_report.*`, `app/dm_report_view.*`, `app/update_release.*`, `src/net/*`): unchanged in content. Links `hydra_app`, `winhttp`.

`hydra_ui` (the rest of `src/ui/`: `app_shell.*` with `run_frame` and the placements, `app_state.*`, the tabs, the panels, the library screens, the jobs, the report windows, `report_state.h`, `resource.h`): Hydra's window. Links `hydra_uibase`, `hydra_preview`, `hydra_dm`. It no longer links `imgui_test_engine`.

Two test libraries: `hydra_uitest_engine` (`tests/ui/uitest_engine.{h,cpp}` new, `uitest_commands.{h,cpp}`, `uitest_script.cpp`): the WARP device, the engine, the verbs, driven through three callbacks; links `hydra_uibase`, `imgui_test_engine`, `d3d11`, `dxgi`. `hydra_uitest_harness` (the rest of `tests/ui/` and `tests/report_samples.cpp`): Hydra's `Harness`, its tests and the live mode; links `hydra_uitest_engine`, `hydra_ui`.

The programs: `Hydra` links `hydra_ui` (and the harness in dev builds) plus `d3d11 dxgi d3dcompiler`. `hydra_batch`, `hydra_fillcompare`, `hydra_bench`, `hydra_replay`, `hydra_replay_json` and the fuzz exes link `hydra_app` where they link `hydra_core` today (`tools/replay_json.cpp` reads `store/record_store.h`; `hydra_bench` and `hydra_replay` include `app/` headers; the fuzz harness links `hydra_audio` already). `hydra_precheck_lib` keeps `hydra_core`: its sixteen project includes are all `core/`. `hydra_tests` links `hydra_ui`, `hydra_dm`, `hydra_audio`, `hydra_render`, `hydra_image` as today (the new libraries arrive through `hydra_ui`'s PUBLIC links). `hydra_uitest` links `hydra_uitest_harness`. `HydraPathBuilder` (the Path Builder's PB-1) links `hydra_preview`, `hydra_uibase`, `hydra_app`, `hydra_replay_json`, `imgui`, `d3d11`; its runner links `hydra_uitest_engine`.

The mimalloc rule holds throughout: `hydra_use_mimalloc` links it first on every exe, and no library links it (the comment at `CMakeLists.txt` lines 360 to 366 says why). The existing redirect checks (`tests/test_library_jobs.cpp` lines 280 to 288 and the GUI runner's start check) are the proof after every task.

## How the layering stays true: the test (T3)

A new test file `tests/test_layering.cpp`, globbed into `hydra_tests` like every `test_*.cpp`, reads `CMakeLists.txt` through `sourcetree::root()` and walks the tree with `sourcetree::for_each_source_file` (`tests/source_tree.h`, the one walker). It builds two maps from the CMake text with its `#` comments blanked: file to target, from every `add_library(` and `add_executable(` source list (the `${hydra_test_sources}` glob stands for `tests/test_*.cpp`; `foreach(... IN ITEMS a b)` loops are read as plain lists); and target to the targets it may include from, from every `target_link_libraries(`, following PUBLIC links transitively and PRIVATE links one step. Then, for every `.h` and `.cpp` under `src/`, `tools/` and `tests/ui/` that a target lists, it reads each `#include "..."` or `<...>` line, resolves it to a project file (by the `src`, `tools` and `tests/ui` include roots) or to an external by `tests/layering/externals.json`, and fails with `file: include "x" reaches target T, which L does not link` for each include outside the allowed set. A `src/` file in no target fails with `src/x: in no library's source list`; a file under `tests/` or `tools/` that no target lists is skipped (test helper headers belong to the exe that includes them).

`externals.json` is a list of `{"header": "imgui.h", "target": "imgui"}` rows for the third-party and system headers that stand for a link line: `imgui*.h` and `imstb*` to `imgui`; `imgui_te_*.h` and `imgui_capture_tool.h` to `imgui_test_engine`; `d3d11.h`, `dxgi.h`, `d3dcompiler.h` to those three; `sqlite3.h`, `miniz.h`, `miniaudio.h`, `stb_vorbis.c`, `stb_image.h`, `opus*.h`, `ogg/*.h`, `mimalloc.h`, `winhttp.h`, `json.hpp` (whose "target" is the include directory `third_party/json`, read from `target_include_directories`). Standard and Windows SDK headers match no row and are free. Rows have the single-owner shape (`must_match` examples the test checks against itself), so a mistyped row fails the test, not the tree.

`known_gaps.json` is the baseline, in the shape of `tests/single_owner/baseline.json`: one entry per include that points the wrong way today, each with the file, the include text, who removes it (the task id) and a `remove_by` date; an entry past its date fails, and an entry no include matches fails too, so the list cannot go stale in either direction. T3 writes the entries for exactly the wrong-way includes listed above; T1, T2 and T4 each delete theirs, which is each task's red-then-green proof. After T6 the file holds `[]` and the test's "every entry names a real include" case keeps it empty for good.

The reader is pinned on a made-up `CMakeLists.txt` string in the same file: three libraries, one PUBLIC chain, one PRIVATE link, one `foreach`, one `${...}` glob name, one `if()` block, with cases for each. No number is involved. A new library needs no edit to the test; a new external header needs one row in `externals.json`. An ADR records the decision (T6).

## Tasks

Each task fits one Opus executor inside 100 tool calls and 20 minutes, in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`. Each runs only the tests it names, with `build-cpp\Release\hydra_tests.exe` and `build-cpp\Release\hydra_uitest.exe`. After each task one fresh reviewer (Sonnet) reads the change and its documents, fixes what it finds, commits on the same branch and signs off; then it merges (D107). The merge's own test list comes from `hydra_precheck.exe --tests`, never typed by hand. No task changes a number, a label or a stored record. No task runs anything over the user's library.

### Wave 1

#### T3. The layering test and the header lines

Owns `tests/test_layering.cpp` (new), `tests/layering/externals.json` (new), `tests/layering/known_gaps.json` (new), `tests/shards.txt` (nothing, unless the new file's time unbalances a slice; the executor leaves it alone and says so), and in `CMakeLists.txt` only the header lines it adds inside today's source lists: each header-only file (`widgets.h`, `text_role.h`, `job_base.h`, `badge_pill.h`, `label_layout.h`, `activation_row_layout.h`, `fonts.h`, `fallback_fonts.h`, `generation.h`, `report_state.h`, `details_parts.h`, `details_view.h`, `library_parts.h`, `library_view.h`, `resource.h`, `app/preview_clock.h`, `app/work_pool.h`, `app/update_key.h`, `audio/frames.h`, `core/*.h` with no `.cpp`, `parse/chart_reading.h`, `parse/timesig.h`, `store/stored_versions.h`, `cli/batch_scope.h`, `tests/ui/live_protocol.h`, `tools/replay_json.h`; the executor greps for the rest) goes on its own line beside its folder's `.cpp` lines in the library that owns it today. Headers with a `.cpp` twin are not listed; the reader pairs them by name.

What it builds: the test described above, green on today's tree because `known_gaps.json` lists today's wrong-way includes (the `Song` cycle's three, `core/replay.cpp` to `app/path_view.h`, `app/song_length.cpp` to `audio/frames.h`, `store/record_store.h`'s two, `ui/preview_load_job.cpp`'s two, `ui/app_shell.cpp` reaching `ui/app_state.h` from the set-up half is not an include gap and is not listed; and `hydra_ui`'s `d3d11.h` reach from `icons.h`, which T2 fixes on the link line). Nothing else changes.

Questions its code answers: "which library is this file in" (the CMake source lists, one owner; the test reads them); "may this file include that header" (the link closure, one owner; the test reads it). The test computes nothing twice: the externals table is data, the gaps list is data.

Tests: `layering: the CMake reader, pinned on a made-up file`; `layering: externals rows match their own examples`; `layering: every known gap names a real include and is not past its date`; `layering: every src file is in one library`; `layering: every include stays inside its library's links`. Run: `hydra_tests.exe -sf=*test_layering.cpp "-sf=*single_owner*" "-tc=docs match code*"` (the last two because `CMakeLists.txt` changes).

Done when all pass and `known_gaps.json` lists exactly the includes named above, each with the task that removes it. Blockers: none. earlyStart: at the user's yes.

### Wave 2 (T1 and T2 start together; both fork from T3's executor commit, not its review)

#### T1. The five bottom libraries

Owns `src/core/**`, `src/parse/**`, `src/search/**`, `src/store/**`, `src/app/**` except `display_format.{h,cpp}` and the new `job_base.h` (T2's), `src/audio/frames.h`, the `format_measure` call lines in `src/ui/paths_tab.cpp` and `src/ui/details_panel.cpp`, the tests of the moved code (`tests/test_song_parse.cpp`, `test_timing.cpp`, `test_replay.cpp`, `test_path_list.cpp`, `test_song_length.cpp`, `test_preview_source.cpp`, `test_audio_player.cpp`, `test_store_open.cpp`, and the fixtures `tests/load_util.h`, `corpus_util.h`, `preview_song_fixtures.h` if an include changes), the single-owner rows whose owner lines move (`rules/parse.json`, `parse-fills.json`, `app/concept-F5.json` for `Song`; `rules/audio.json` for the chart sync rule), `tests/layering/known_gaps.json` (its own entries, deleted), and in `CMakeLists.txt` the `hydra_core` block (today lines 297 to 371, from the `# ---- hydra_core` comment to `endif()`), which it rewrites as five blocks (`hydra_core`, `hydra_parse`, `hydra_search`, `hydra_store`, `hydra_app`, in that order, keeping every source and header line T3 added), and the six link lines that name `hydra_core` for `hydra_batch`/`hydra_fillcompare` (551), `hydra_bench` (682), `hydra_replay` (693), `hydra_replay_json` (757) and the fuzz exes (777), which become `hydra_app`; `hydra_precheck_lib` (739) keeps `hydra_core`.

What it moves. Q2's `Song` move into `core/song.{h,cpp}`; `core/scoring.h`, `core/replay.h`, `core/sqout_chord.h`, `search/graph.h` and `search/pather.h` include `core/song.h` instead of `parse/song.h`; `parse/song.h` includes `core/song.h`. `format_measure` and `measure_label` (`app/path_view.h` lines 37 to 43) move to `core/timing.{h,cpp}`, the home of `Timecode` and `SongTiming`; `path_view.h` includes `core/timing.h` and the callers in `app/`, `core/replay.cpp`, `ui/paths_tab.cpp`, `ui/details_panel.cpp` and the tests name the new namespace. `audio_ms_of_chart_ms` and `chart_ms_of_audio_ms` (`audio/frames.h` lines 35 to 41) move to `app/preview_source.h`, beside `chart_audio_offset_ms` which makes the offset they take; `frames.h` includes `app/preview_source.h` for `audio_end_chart_ms`, which stays. `store/record_store.h` drops `search/graph.h` and `parse/song.h` (the stop rule: if a compile error names a type from either, the include comes back and the plan's `hydra_store` line gains the link; the executor reports which type). `HYDRA_VERSION`, `HYDRA_APP_NAME` and `HYDRA_APP_USER_MODEL_ID` stay PUBLIC on `hydra_core` so every library above sees them, as today. The `mimalloc` include-directory lines (367 to 371) stay on `hydra_core`, because `app/allocator.cpp` moves to `hydra_app` and needs the header there: the `if(HYDRA_MIMALLOC)` block moves to the `hydra_app` block with it.

Questions: "where is the one `Song` class" (`core/song.h`); "how is a chart position printed" (`core/timing`, one owner, the row scan keeps it so); "where does chart time 0 sit in the audio" (`app/preview_source.h`, now beside the offset). No rule changes; the functions move as they are.

Tests, in the files above, pass unchanged beyond include lines and the namespace of two calls. Run: `hydra_tests.exe -sf=*test_song_parse.cpp -sf=*test_midi.cpp -sf=*test_timing.cpp -sf=*test_replay.cpp -sf=*test_path_list.cpp -sf=*test_song_length.cpp -sf=*test_preview_source.cpp -sf=*test_audio_player.cpp -sf=*test_store_open.cpp -sf=*test_store_records.cpp -sf=*test_engine_sweeps.cpp -sf=*test_cli.cpp -sf=*test_layering.cpp "-sf=*single_owner*" "-tc=docs match code*"`.

Done when all pass, `known_gaps.json` has lost the seven entries T1 owns, and `hydra_batch.exe` and `hydra_fillcompare.exe` still build (`test_cli.cpp` runs them). Blockers: T3. earlyStart: fork from T3's executor commit.

#### T2. The GUI base: window set-up, theme, widgets and dialogs without `AppState`

Owns `src/ui/app_shell.{h,cpp}`, `src/ui/imgui_setup.{h,cpp}` (new), `src/app/display_format.{h,cpp}`, `src/app/job_base.h` (new, moved from `src/ui/job_base.h`, which is deleted), `src/ui/library_parts.h`, `src/ui/library_dialogs.cpp`, `src/ui/widgets.h`, `src/ui/preview_load_job.cpp` (include lines only), `src/ui/preview_load_job.h`, `src/ui/library_jobs.{h,cpp}`, `src/ui/dm_jobs.h`, `src/ui/update_job.h` (the `job_base` include line in each), `src/ui/main.cpp`, `tests/ui/uitest_harness.{h,cpp}` (include lines and the `FrameText` capture call only), `tests/test_app_shell.cpp`, `tests/test_batch_text.cpp`, `tests/headless_imgui_fixture.h`, `tests/single_owner/rules/ui.json` (the `gated_time_left_text` row's owner lines, and any row naming `src/ui/job_base.h`), `tests/layering/known_gaps.json` (its own entries, deleted), and in `CMakeLists.txt` the `hydra_ui` block (today lines 438 to 480, from the `# ---- hydra_ui` comment to its last `target_link_libraries`), which it rewrites as two blocks, `hydra_uibase` then `hydra_ui`, carrying T3's header lines.

What it moves. From `app_shell` into `imgui_setup`: `ImGuiSetupOptions`, `setup_imgui`, `shutdown_imgui`, `kClearColor`, `ui_scale_for_dpi`, `scaled_style`, `set_ui_scale`, `FrameText` and the capture that fills it (today inside `run_frame`, `app_shell.cpp` line 601 on, reading `g.LogBuffer`), as a pair of functions `run_frame` calls around Hydra's draw. `app_shell` keeps `run_frame(AppState&)`, the window and report placements and the layout notes. This is the Path Builder plan's SH-1, the files unchanged. From `ui/library_parts.h` and `ui/widgets.h` into `app/display_format`: `format_duration`, `time_left_text` (declared at `library_parts.h` lines 66 to 72, defined in `library_dialogs.cpp`) and `progress_fraction` (`widgets.h` line 86); `preview_load_job.cpp` includes `app/display_format.h` instead. This is the Path Builder plan's PV-1, the wording unchanged. `ui/job_base.h` becomes `app/job_base.h` (it includes only `app/user_messages.h` and `core/error_kind.h`). On the CMake side, `hydra_ui` stops linking `imgui_test_engine` (nothing in it includes the engine; `main.cpp` is the exe's and the harness brings it), and `hydra_uibase` links `d3d11` PUBLIC because `icons.h` exposes `ID3D11` types.

Questions: "how is ImGui set up" (`imgui_setup`); "how long is left, in words" and "what fraction is done" (`display_format`, one home each); "what is a job" (`app/job_base.h`). None new.

Tests pass with nothing edited beyond include lines: `hydra_tests.exe -sf=*test_app_shell.cpp -sf=*test_theme.cpp -sf=*test_text_role.cpp -sf=*test_batch_text.cpp -sf=*test_preview_load_progress.cpp -sf=*test_library_jobs.cpp -sf=*test_icons.cpp -sf=*test_layering.cpp "-sf=*single_owner*" "-tc=docs match code*"`; `hydra_uitest.exe --test scan --test panel-open-close --test startup-screen --test preview-load-bar --test layout-sweep`.

Done when all pass, `preview_load_job.cpp` includes nothing from `src/ui/` but its own header, and `known_gaps.json` has lost T2's two entries. Blockers: T3. earlyStart: fork from T3's executor commit, beside T1 (disjoint files: T1 never opens `app_shell`, `display_format`, `job_base`, the `*_jobs` headers or the `hydra_ui` block; T2 never opens `src/core`, `src/parse`, `src/search`, `src/store`, `path_view`, `preview_source`, `frames.h` or the `hydra_core` block).

### Wave 3 (T4 and T5 start together; both fork from a merge of T1 and T2)

#### T4. The Preview library

Owns `src/ui/preview_controller.{h,cpp}`, `src/ui/preview_load_job.{h,cpp}`, `src/ui/preview_transport.{h,cpp}` (no edits expected beyond what the compiler asks for), `tests/test_preview_controller.cpp`, `test_preview_load_progress.cpp`, `test_preview_transport.cpp`, `test_preview_boxes.cpp` (the same), `tests/layering/known_gaps.json` (nothing left to delete; it checks), `CONTEXT.md` (nothing; T6), and in `CMakeLists.txt`: a new `# ---- hydra_preview` block placed between the `hydra_render` block and the `hydra_uibase` block, the three `preview_*` source lines removed from `hydra_ui`'s list, and `hydra_ui`'s `target_link_libraries ... PUBLIC` line, which gains `hydra_preview`.

What it builds. `add_library(hydra_preview STATIC ...)` with the six files, `PUBLIC hydra_app hydra_audio hydra_render`, `PRIVATE hydra_warnings`. The namespace stays `hydra::ui`; no call site changes. This is the Path Builder plan's PV-2 without the folder move (Q1).

Questions: none new; the library split is a build fact and `CMakeLists.txt` is its one home.

Run: `hydra_tests.exe -sf=*test_preview_controller.cpp -sf=*test_preview_load_progress.cpp -sf=*test_preview_transport.cpp -sf=*test_preview_boxes.cpp -sf=*test_layering.cpp "-sf=*single_owner*" "-tc=docs match code*"`; `hydra_uitest.exe --test preview --test preview-path-overlay --test preview-controls --test preview-mode-reload --test preview-activation-jumps`.

Done when all pass with no test edited beyond include lines and the layering test shows `hydra_preview` includes nothing from `hydra_uibase` or `hydra_ui`. Blockers: T1, T2. earlyStart: fork from the main session's merge of T1 and T2 (or from a local merge of both executor commits).

#### T5. The app-free half of the headless runner

Owns `tests/ui/uitest_engine.{h,cpp}` (new), `tests/ui/uitest_harness.{h,cpp}`, `tests/ui/uitest_commands.{h,cpp}`, `tests/ui/uitest_script.cpp`, `tests/ui/uitest_live.cpp`, `tests/ui/uitest_main.cpp`, `docs/agents/ui-testing.md` (the Layout list at its line 271 only), and in `CMakeLists.txt` the `hydra_uitest_harness` block (today lines 489 to 516), rewritten as `hydra_uitest_engine` then `hydra_uitest_harness`, and the `hydra_uitest` link line (613 to 614), which names the harness as before.

What it builds. This is the Path Builder plan's UT-1 as written: `uitest_engine` holds the WARP device and offscreen target, the engine set-up, `frame()`, the `FrameText` capture (from T2's `imgui_setup`), `visible_text`, `on_screen`, `widget_dump`, `item_line`, `screenshot`, `wait_until`, `idle_frame`, `escape_ref`, `text_width`, the verb interpreter and the script runner, with a `Callbacks` struct the program supplies: `draw_frame()`, `jobs_busy()` and `state_text()`. `Harness` stays a thin layer over it with the `AppState`, `reset_app`, the gates, the live mode and every Hydra helper, so the test files compile unchanged. `hydra_uitest_engine` links `hydra_uibase`, `imgui_test_engine`, `d3d11`, `dxgi`; the harness links the engine and `hydra_ui`.

Questions: "what does a verb mean" stays in `uitest_commands.cpp` (ADR 0030); "what is on screen" stays `visible_text`.

Run: `hydra_uitest.exe --test scan --test panel-open-close --test preview --test startup-screen --test report-window-open-path --test command-open`; `hydra_uitest.exe --script tests/ui/scripts/smoke.txt` (the executor checks the file exists; if not, any one script under `tests/ui/scripts/`); `hydra_tests.exe -sf=*test_live_protocol.cpp -sf=*test_layering.cpp "-sf=*single_owner*" "-tc=docs match code*"`.

Done when all pass and the layering test shows `hydra_uitest_engine` includes nothing from `hydra_ui` or `ui/app_state.h`. Blockers: T2 (the capture it reads). earlyStart: fork from the merge of T1 and T2, beside T4 (disjoint: T4 edits `src/ui/preview_*`, the `hydra_preview` block and `hydra_ui`'s lines; T5 edits `tests/ui/` and the harness block; the two CMake regions are more than three lines apart).

### Wave 4

#### T6. The record

Owns `CONTEXT.md` (the paragraphs that name `src/ui/job_base.h`, `format_measure`, the chart sync rule and the libraries; the executor greps for every moved name), `docs/development.md` (a short "Libraries" paragraph naming the twelve and their order, pointing at `CMakeLists.txt` as the owner), `docs/adr/0032-cmake-is-the-one-owner-of-library-membership.md` (new), `tests/layering/known_gaps.json` (set to `[]`), and the `# ----` comment lines of the library blocks in `CMakeLists.txt` where an older comment still says `hydra_core` holds everything below the GUI.

Run: `hydra_tests.exe -sf=*test_layering.cpp "-tc=docs match code*" "-sf=*single_owner*"`.

Done when they pass and the gaps file is empty. Blockers: T4, T5. earlyStart: fork from the merge of T4 and T5.

## What this plan replaces in the Path Builder plan

PV-1 is T2's `display_format` move. PV-2 is T4 (without the folder: the Preview stays in `src/ui/`, so the Path Builder's RD-2 edits `src/ui/preview_controller.{h,cpp}`, not `src/preview/`). SH-1 is T2's `imgui_setup` split. UT-1 is T5. The Path Builder's later tasks wait as follows: PB-1 (the builder's window and runner) waits on T2, T4 and T5; PB-2a and PB-2b wait on PB-1 and so on all of them; RD-2 waits on T4 (the controller's library). The Path Builder's EN-1, EN-2, EN-3, RD-1, HY-1 and PB-3 touch none of this plan's files and can run beside wave 1 or 2 of this plan, except that HY-1 and PB-3 each add one source line to what is today `hydra_core`'s list: they must fork from T1 (`hydra_app` for `app/launch.cpp`; `hydra_search` for `search/pathstring_read.cpp`) or add their line after T1 merges.

## Out of scope (for the user to confirm)

- Folder moves to match the libraries (Q1 says no).
- Splitting `hydra_dm` into a plain HTTP library and the DMBot feature: no program needs one without the other.
- Moving `ellipsize` out of `render/overlay_layout` and `mapped_file` out of `audio/`: both are downward includes after the split; they only make `hydra_uibase` link two libraries it would not otherwise need.
- DLLs, install rules for the libraries, or any header meant for projects outside this repo.

## Promise map

| Promise | Where it is written | Task | Check that proves it |
|---|---|---|---|
| Hydra.exe looks and behaves exactly the same | the brief; "The plain version" | T1, T2, T4, T5 | the named existing unit tests and uitest scripts pass with nothing edited beyond include lines (reviewer check of each diff); the uitest scripts `scan`, `panel-open-close`, `startup-screen`, `preview`, `layout-sweep` compare screen text to pinned values |
| Moves change no behaviour, no number, no label | Tasks preamble | every task | each task's diff moves declarations as they are; the single-owner scan and the uitest text pins pass |
| Every `src/` file is in exactly one library | "The test" | T3 | `layering: every src file is in one library` |
| No include points the wrong way after the split | brief item 3 | T3, T6 | `layering: every include stays inside its library's links` with `known_gaps.json` empty after T6 |
| The gaps list cannot go stale | "The test" | T3 | `layering: every known gap names a real include and is not past its date` |
| A wrong-way include is caught by the test (the test can fail) | brief item 3 | T3 | the made-up CMake case includes one wrong-way include and asserts the failure line |
| `CMakeLists.txt` is the one owner of membership; the test computes nothing twice | "The test"; preamble rule | T3, T6 | the test has no library list of its own (reviewer check); ADR 0032 |
| `hydra_preview` links no ImGui and no `AppState` | target layout | T4 | `layering` case: `hydra_preview`'s allowed set has neither `imgui` nor `hydra_ui` |
| `hydra_uibase` needs no `AppState`; `preview_load_job.cpp` includes nothing from `ui/` but its header | target layout; T2 | T2 | `layering` case after T2; reviewer grep of the file |
| `hydra_uitest_engine` runs without `AppState` | T5 | T5 | `layering` case: engine's allowed set has no `hydra_ui`; the scripts named in T5 pass through the callbacks |
| `hydra_ui` no longer carries `imgui_test_engine` | "What the code does today" | T2 | `hydra_ui`'s link line (reviewer check); Hydra still builds with the harness attached (`scan` passes) |
| The CLI tools still link only what they need (no codecs, no WinHTTP) | CMake comments 376 to 378, 424 to 426 | T1 | `test_cli.cpp` runs both exes; `hydra_batch`/`hydra_fillcompare` link `hydra_app` only (reviewer check of line 551) |
| mimalloc stays redirected on every exe | memory rule; CMake 360 to 366 | T1, T2, T4, T5 | `test_library_jobs.cpp`'s redirect checks in T2's run; the uitest runner's start check in every uitest run |
| `Song` move drags no loader (stop rule) | Q2 | T1 | the executor's report names what moved; `core/song.h` includes nothing from `parse/` (layering case) |
| `record_store.h`'s dropped includes were unused | "What the code does today" | T1 | the build; if a type is named, the report says which and the link line changes |
| No whole-library run anywhere | brief preamble | every task | every test names `testdata/input` or scratch files |
| One fresh reviewer per change | CLAUDE.md rule 5 | every task | the ref gate refuses a merge without the key's verdict |
| The merge's test list is never typed by hand | brief item 5 | every task | the main session runs what `hydra_precheck.exe --tests` prints (whole suite, since `CMakeLists.txt` changes) |
| Tasks in one wave own disjoint files and CMake lines | brief item 4 | T1/T2, T4/T5 | the ownership lists above; the reviewer checks the two diffs touch no common file line |
| No new number without the user | brief item 3 | every task | the layering test holds no threshold; the executor stops and asks for any other |
