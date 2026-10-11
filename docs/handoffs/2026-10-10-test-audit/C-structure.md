# TA-C audit: suite structure, source-scan tests, GUI suite, fixtures, tool tests

Scout: agent a780a21e845b786c9 (task TA-C). Read-only. I ran no tests and built nothing. The only things I ran were a Python script over the saved durations file and a Python script that counts which src files a test includes. Both live in the scratchpad.

Short version: the suite is fast and healthy (1,250 cases, all green, about 42 s of test time). The weak spots are not the engine tests. They are (1) the one giant "single owner" file that nearly every refactor has to edit, (2) a handful of waits with no cap, in the unit tests and in the GUI runner, (3) the tool tests, which nothing runs, and (4) a few baselines that were meant to shrink and have not.

---

## (a) Working well

**The source-scan tests check themselves.** `tests/test_single_owner.cpp:5812-5838` runs every rule against its own good and bad example lines, checks that every "owner line" still matches, and checks that no two rules ask the same question. Lines 5905-5919 make sure a rule scoped to one function really finds that function once, so a rename cannot leave a rule checking nothing. The clone scanner has its own unit tests at 6474-6536. This is the right way to build a text scan: it cannot go silently vacuous. (Severity of any issue here: none.)

**The failure message names the file, the line, the question and the owner.** At 5892-5899 the scan prints `file:line: answers "<question>", which belongs to <owner>: <line>`. A developer can fix it without reading the test. Stale baseline entries print a "remove it" instruction (5742, 5745). Same quality in the clone scan (6550-6555: both places, line ranges, "move them into one place both can call").

**The docs test has a real floor and shrinks its own allow-list.** `tests/test_docs_match_code.cpp:279, 336` require that the walk found more than 1,000 words and more than 100 names, so an empty walk cannot pass. Lines 330-335 fail on an allow-list entry no doc needs. The marker check has a self-test for a wrong number, a wrong "on/off" word and an unknown marker (414-428).

**The GUI suite waits on app state, not on frame counts or sleeps.** `tests/ui/uitest_harness.cpp:371-387` polls a predicate with a deadline and returns at once if the test has already failed, so one missed click does not sit out a 300 s timeout. Batch and click analyses are held open with gate objects (`BatchGate`, `ViewGate`, `OpenGate`, described in `docs/agents/ui-testing.md:34, 161-163`) rather than hoping a fast job is still running. Only five `sleep_for` calls exist in all of `tests/ui/` and each is a 1-2 ms poll nap.

**`--jobs` gives each GUI test its own process.** `tests/ui/uitest_main.cpp:95-205` starts one child per test, prints a crashed child's exit code when it printed no result (195-199) and always ends with "N of M passed". A crash in one test cannot corrupt the next.

**A renamed GUI test cannot disappear silently.** `tests/ui/uitest_tests.cpp:44-49` aborts the run if a name in `kRunOrder` has no registered test.

**CI is simple and honest.** `.github/workflows/ci.yml:38-51` builds with the same `build_cpp.ps1` developers use (warnings fail the build), runs the whole doctest binary and the whole GUI suite. The one exclusion, `-tce=PreviewAudioDevice opens and starts the default output device`, matches the real test name (`tests/test_audio_device.cpp:16`), so it does exclude what it claims to.

**One shared corpus cache.** `tests/corpus_util.h:173-228` caches the parsed `Song` and the analysis per chart and settings, even caching a thrown exception. The 115-chart corpus is analyzed once per run, not once per test.

---

## (b) Broken or misleading

**1. Two baselines that are meant to "only shrink" have not shrunk (medium).**
`tests/test_single_owner.cpp:5687-5695` lists three known copies of the "right-align an item" rule, each described as "a follow-up to RW-T5's review". RW-T5 merged long ago (commit bf270da5 is its merge), so these are three open violations parked in a test, not follow-ups in progress. The clone baseline (`6426-6444`) still holds all 16 entries from 2026-10-05, 14 of them in test files, many keyed on lines like `using namespace hydra;` or `namespace {` that mark only the start of an 8 to 11 line block of test setup. The file header promises the list "only shrinks" (lines 5-6); today it has not shrunk in five days.

**2. The doc-name check is weaker than its heading says (medium).**
`tests/test_docs_match_code.cpp:95-123` collects every word in src, tools and tests, comments and string literals included. `:311` then checks each part of `Foo::bar` against that set separately. So `Foo::bar` passes if `Foo` is a word anywhere and `bar` is a word anywhere, even in an unrelated comment. A removed function whose name survives in a comment or in a test string still passes. It also reads only `docs/adr/*`, `CONTEXT.md`, the User Guide, `development.md` and one squeeze note (61-70), so `docs/agents/*`, the README and the handoffs are not checked at all.

**3. Some scan tests pin the exact text of one production line (medium).**
`tests/test_single_owner.cpp:6012-6015` requires the single hit to equal exactly `src/search/pather.cpp: options.ms_filter = 0.0;` and `src/app/config.cpp: return depth_mode == 1 ? DepthMode::Points : DepthMode::Scores;`. Re-wrapping or renaming either line fails the suite although no rule changed. `5950-5980` ties a docs sentence to a code literal by cutting between the first and last double quote of an owner line, which breaks on any escape (it then needs `REQUIRE(sentence.find('\\') ...)` at 5975 to stay honest). These are brittle by construction.

**4. The "only shrinks" idea hides a second reason the file churns (see (d)1):** the rules are regexes for one past duplication each, for example `(\.|->)keep\(` (the Held-row rule, commit e6765d49). A regex like that flags any method called `keep` and misses the same logic spelled another way. The file's own comments admit a case it does not catch (`:92-93`, "A length held in a plain variable is not caught"). It is a guard against the exact copy that was just removed, not proof that the rule lives in one place.

**5. The preview golden test passes silently when its fixture is missing (low).**
`tests/test_preview_golden.cpp:139-143` prints a `MESSAGE` and `return`s if `golden.json` is absent. The fixture is present today (`testdata/preview/`), so it runs now. If it were deleted or the path broke, the case would still be green.

**6. A tool test file is tied to C++ by text (low).**
`tests/test_single_owner.cpp:6145-6157` lists exact lines of `tools/ch_probe/tests/test_probe_songs.py` and `test_probe_chart.py` as "allowed". Editing a Python test can fail a C++ test, and nobody running only the Python tests sees it.

---

## (c) Unnecessary or dead

**Three tests are always skipped (low).** doctest reports 3 skipped on every run:
- `tests/test_path_view.cpp:622` is a manual hunt for a chart with an uncounted squeezed-out row (the comment at 620 says it runs only by hand).
- `tests/test_report.cpp:1028` writes sample fill-comparison HTML for a browser check (needs `HYDRA_PAGE_SAMPLES`). The fill page still exists (`src/app/fill_report.cpp:7` includes `html_page.h`), so this is a live dev aid, but it is a tool wearing a test's clothes.
- `tests/test_replay.cpp:1910-1914` times the replay on `C:\Clone Hero\songs\Misc Downloads\blink-182 - Discography\notes.mid`, a path on the owner's machine that is not in the repo. On any other machine it prints "chart not found, skipped".
They cost nothing at run time. They do put a standing "3 skipped" in every report, which hides a real skip if one appears. The fix is to move them to a `tools/` script or give them a `--no-skip` tag in the docs.

**Two "tests" in the tool tree are live-game experiments (low).** `tools/ch_probe/experiments/key_delivery_test.py` and `pad_flash_test.py` end in `_test.py`, which pytest's default pattern collects. The README tells you to run `python -m pytest tools/ch_probe/tests` (README line 75), which skips them, but `pytest tools/` would try to drive Clone Hero.

**Stray chart folders ride along in the corpus (low).** `testdata/input/test_path_reduction/test0.chart` and three of the four `test_flammarker/*.mid` files are never named by any test. They are used only because corpus discovery (`tests/corpus_util.h:63-73`) walks the whole `testdata/input` tree. That is by design, but it means dropping any file there silently changes every corpus-wide test, and the `test_flammarker` outputs (`expected_prodrums`, `expected_regdrums`, `ingame`) look like reference answers that no test compares against.

I did not find a never-failing test or a duplicated test in the files I read. I did not check the 341 scan rows one by one for rows whose owner no longer exists (see (g)).

---

## (d) Poorly designed (finding, then the fix in one sentence)

**1. `tests/test_single_owner.cpp` is a 5,600-line data table living in C++ (high).** The rules table is one function, `rules()`, from line 85 to 5682, with 341 rows (`^        {"[A-Z]` count). The engine that uses it is under 900 lines. By git history, 352 of the 1,335 commits since 2026-10-03 touch this file (26%), with 8,470 lines added against 1,807 removed. Spot-checked diffs (9f4861bc, e6765d49, c087ee25, 66450594) are all "add one row, 17 to 43 lines". Every refactor therefore edits the largest test file, forces a recompile of it, and is a merge-conflict magnet for parallel agents. *Fix: move the rows to a plain data file (or one `.inc` per area) read at run time, and keep the engine in the .cpp.*

**2. The ratchet grows forever (medium).** Each fix adds a regex row and nothing retires one. The scan's run time (one case, 4.16 s, 10% of the whole suite, see (f)) grows with rows times lines because it uses `std::regex`. *Fix: give every row an expiry or review date in its `decided_by` text, and retire rows whose old copy cannot be written again because the helper is now the only way (for example, make the helper the only public entry).*

**3. The GUI runner can wait forever (medium).** `tests/ui/uitest_main.cpp:167-168` calls `WaitForMultipleObjects(..., INFINITE)` on the child processes. One hung child hangs the whole run until CI's 60-minute job limit (`ci.yml:28`). The inner `wait_until` has caps, but gate loops do not: `tests/ui/uitest_report_windows.cpp:428-429` spins until a release flag with no deadline and no cancel check (the dm fetcher at 457-458 at least checks `cancel`). Your own rule is "every wait has a cap". *Fix: give each child a wall-clock limit (say 5 minutes), kill it and print "[FAIL] timed out".*

**4. Serial and parallel GUI runs can disagree (medium).** `tests/ui/uitest_tests.cpp:21-33` says outright that under `--all` "the ImGui context carries over from one test to the next", and keeps a hand-written run order for that reason. CI uses `--jobs 4`, where each test starts fresh. The `ctest` entry (`CMakeLists.txt:577`) and a plain `--all` run use the carried-over state. So a test that only passes because of leftover state would pass locally and fail in CI, or the reverse, and there is no run mode that checks both. *Fix: make `--all` also run each test on a fresh context, then delete `kRunOrder`.*

**5. Several unit-test waits have no cap (medium).** Loops like `while (!release.load()) sleep` or `while (!app.report_job->finished()) sleep` appear at `tests/test_app_state.cpp:242, 982, 1000, 1037` and `tests/test_library_jobs.cpp:90, 433, 445`. If the thing they wait for breaks, the failure is a frozen suite, not a red test. A few others do have caps (`test_app_state.cpp:254` loops at most 12,000 times; `test_preview_controller.cpp:54` at most 1,200). *Fix: one shared `wait_until(pred, timeout)` in a test header, and replace the bare loops with it.*

**6. "Nothing happened" checks use fixed sleeps (low).** `tests/test_library_jobs.cpp:137` and `:161` sleep 200 ms and then assert nothing started; `tests/test_dm_report.cpp:586` sleeps 300 ms. These pass if the bug takes longer than the sleep to show. They are also fixed costs on every run. *Fix: pause the work behind a gate and assert on the gate's counter instead of on elapsed time.*

**7. Mtime tests assume a fast clock (low).** `tests/test_app_state.cpp:352` (and 378, 402, 430) `Sleep(20)` so a rewritten file's modified time moves. On a filesystem with coarse timestamps this would not move. *Fix: set the file time explicitly with `last_write_time` instead of sleeping.*

**8. No tool test runs anywhere automatic (medium).** `.github/workflows/ci.yml` runs only `hydra_tests` and `hydra_uitest`. The 12 `tools/ch_probe/tests/test_*.py` files, `tools/test_compare_db.py`, `tools/test_derive_once_precheck.ps1` and `tools/test_mutation_probe.ps1` are run by hand. Only `development.md:297` mentions one of the ps1 self-tests, `development.md:255` mentions `compare_db.py` but not its test, and the ch_probe README line 75 mentions the pytest command. `test_compare_db.py` needs pytest (`tmp_path` fixture) while the probe tests are plain `unittest`, and no doc says pytest must be installed. By directory listing their imports still resolve (every module they import exists: `active_probe`, `passive_probe`, `live`, `play_chart`, `walk_edges`, `watch_window`, `analysis`, `debugger`, `engine_finder`, `input_driver`, `ocr`, `probe_chart`, `probe_songs`, `process`, `constants`, `engine`, plus `tests/fakes.py` and `tests/chart_reader.py`), and the last edits were 2026-10-05 and 2026-10-07, so they are young, not rotted. But "nobody runs them" is exactly how they will rot. *Fix: add one CI step (`py -m pytest tools -q --ignore=tools/ch_probe/experiments`) or write in `docs/development.md` which command runs them and when.*

**9. Test time is concentrated (low).** One case, `tests/test_track_state.cpp:871` ("equals a full build on every corpus chart and stored path"), takes 18.2 s, which is 43% of all test time; the whole file is 47%. That case builds a Preview scene for every stored path of every corpus chart and compares full against partial rebuilds. Some of that is probably the one-time analysis the shared cache then makes free for later tests, so its true cost is mixed with a cache warm-up. I did not verify which share is which. Running a single later test alone (`-sf=`) would pay that warm-up itself. *Fix: have the case use a fixed sample of charts (the ones with the most paths) instead of all 115.*

---

## (e) Coverage map (by src module)

88 `.cpp` files under `src/`. Method: a script checked whether any file under `tests/` includes the matching header. 66 are included by name. The 22 that are not:

- **Covered some other way (fine):** `cli/batch.cpp` and `cli/fillcompare.cpp` are run as real executables by `tests/test_cli.cpp` (11 cases, 1.2 s; `CMakeLists.txt:625` makes the test depend on both). `audio/ma_reader.cpp`, `opus_reader.cpp`, `vorbis_reader.cpp`, `song_audio.cpp` and `core/audio_sniff.cpp` are exercised through `test_stem_reader.cpp` and `test_audio_decode.cpp` via other headers (`sniff_format` is named in 4 test files). The big UI files (`preview_tab`, `library_dialogs`, `paths_tab`, `library_table`, `details_panel`, `settings_bar`, `library_toolbar`, `path_report_window`, `dm_report_window`) are driven by the 88 GUI tests (about 1,000 `IM_CHECK`s) and several are named in unit tests (`settings_bar`, `library_table` and `details_panel` appear in 2 test files each).
- **No test names them at all:** `ui/icons.cpp`, `ui/win32_dialogs.cpp`, `ui/imgui_files.cpp`, `ui/dynamics_tab.cpp`, `ui/stars_tab.cpp`. The first three are thin platform glue (file pickers, icon drawing); the two tabs are reached only through the `dynamics` and `stars` GUI tests, which check on-screen text. That is a thin spot, not a hole.
- **Engine and data layers** (`search`, `parse`, `store`, `core`, `app`) are all covered by name, with heavy tests: `test_search` 57 cases, `test_replay` 45, `test_store` 69, `test_app_state` 67, plus nine `test_s2_*` rule files.

Nothing important is untested. The GUI layer is covered by label-driven text checks, so layout and drawing bugs (position, overlap, colour) are covered only by the layout unit tests and the one WARP golden image (`test_preview_golden`), which compares to an Onyx screenshot at tolerance 20.

---

## (f) Timing summary (from suite-durations.txt; file complete: "1250 test cases, 1250 passed, 0 failed, 3 skipped, 381,345 assertions")

Total of per-test times: **41.9 s** across 1,250 cases. Only 3 cases take over 1 s, together 23.9 s (57%).

The 20 slowest:
1. 18.20 s, `rebuild_overlay_fields: equals a full build on every corpus chart and stored path` (test_track_state.cpp:871)
2. 4.16 s, `single-owner rules hold across src/, tools/ and tests/` (test_single_owner.cpp:5848)
3. 1.53 s, `rebuild_overlay_fields: equals a full build on random overlays` (test_track_state.cpp:823)
4. 0.70 s, `a Preview load cancelled while opening a 300 MB Opus stem stops promptly` (test_preview_load_progress.cpp:259)
5. 0.64 s, `a saved result reads back its best path and summary across the corpus and config matrix` (test_store.cpp:100)
6. 0.51 s, `search invariants hold across the corpus and config knobs` (test_search.cpp:41)
7. 0.50 s, `StemReader: MP3 seeks through the seek points on a long stream` (test_stem_reader.cpp:496)
8. 0.37 s, `the Preview trusts the scan's fingerprint and does not re-hash an unchanged chart` (test_preview_controller.cpp:727)
9. 0.37 s, `http transport: cancel aborts a request the server never answers` (test_dm_report.cpp:571)
10. 0.37 s, `docs match code: every backticked code name exists...` (test_docs_match_code.cpp:277)
11. 0.35 s, `dynamics_breakdown: the rules never change a chart's Dynamics count` (test_dynamics_breakdown.cpp:396)
12. 0.33 s, `srb: the note loader reads the metadata and notes streams, not the rest` (test_srb.cpp:159)
13. 0.33 s, `closing the Preview frees its render targets; reopening draws again (WARP)` (test_preview_controller.cpp:404)
14. 0.31 s, `the Preview overlay job builds the highway timeline on its worker` (test_preview_controller.cpp:207)
15. 0.31 s, `the Preview base job builds the path-free scene and timeline` (test_preview_controller.cpp:252)
16. 0.31 s, `a mode change on the open chart reloads only the notes and keeps the playhead` (test_preview_controller.cpp:566)
17. 0.30 s, `note total: the engine, the Song and the Dynamics tab agree on every corpus chart` (test_search.cpp:150)
18. 0.27 s, `the graph finds the chart's multiplier squeezes once, in chart order` (test_search.cpp:192)
19. 0.27 s, `run_work_pool: a cancel mid-run never strands the consumer` (test_analysis.cpp:264)
20. 0.27 s, `replay: on every corpus path the disc follows SP's payment and windows add up` (test_replay.cpp:1852)

**test_single_owner.cpp:** 10 cases, 4.43 s, **10.6%** of the total. One case holds 4.16 s of that. **test_docs_match_code.cpp:** 5 cases, 0.38 s (0.9%). **test_track_state.cpp:** 18 cases, 19.7 s (47.1%).

Other files by time: test_preview_controller 3.2 s (7.6%), test_app_state 2.35 s (5.6%), test_store 1.44 s, test_search 1.27 s, test_cli 1.24 s, test_replay 1.03 s.

The suite is fast enough that speed is not the problem. The 42 s is about the cost of one cold compile of a mid-sized file. CI wall time is dominated by the build (`build_cpp.ps1` twice, no cache step in `ci.yml`), not by the tests. The GUI doc claims about 14 s for the GUI suite at `--jobs 4` (`ui-testing.md:32`); that figure is five days old and the doc itself says it has not been re-measured.

---

## (g) Not reached

- I did not check the 341 scan rows one by one for rows that guard something that no longer exists (only sampled four commits and the first, last and probe rows). A script that counts each row's matches over the tree would find vacuous rows. I did not write it because the regexes are C++ raw strings.
- I did not read the individual GUI tests beyond the harness, runner and run-order list, so I did not judge each test's design or overlap with the unit tests. The only overlap evidence is structural: the GUI tests check on-screen text for panels that the unit tests (`test_path_view`, `test_preview_view`, `test_report_view`, `test_song_panel_state`) already cover at the model level.
- I did not confirm the exact GUI test count: doc says 85 (`ui-testing.md:32`), my count of `void test_*` in `tests/ui/` is 88 (not checked against `hydra_uitest --list`; some may be helpers).
- I did not run the tool tests, so "still pass" is judged only by imports resolving and by recent edit dates.
- I did not verify what share of the 18.2 s case is analysis warm-up versus its own work.
- `test_preview_golden.cpp` was read only around its skip path; I did not judge the tolerance of 20 or whether WARP output is stable across CI images.
- Hook tests and the individual unit-test areas belong to other scouts and were not touched.
