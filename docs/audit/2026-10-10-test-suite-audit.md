# Test suite audit (2026-10-10)

Six read-only scouts did this audit. Two searched the web: one for test-design practice, one for testing tools. Four read the suite by area: the engine tests, the store and app tests, the preview, report and audio tests, and the suite's structure (the single-owner scan, the GUI suite, the tool tests and which source has no test). A fifth area, the Claude Code hook tests in `~/.claude/hooks/tests`, went to the fourth audit scout. The main session ran the full C++ suite once for timings and checked each high finding against the code before writing it here. The scouts' full reports, with file:line evidence for every item, are in the session scratchpad under `audit\` (A-engine, B1-store-app, B2-preview-report, C-structure, D-hooks, W1-practices, W2-tools).

## The short version

The C++ suite is in good health. One run on 2026-10-10 had 1,250 test cases and 381,345 checks, all passing, with 3 skipped, in 42 seconds. CI has been green on its last 10 runs. Across the engine, store and preview areas the scouts found the same good habits: tests pin literal values, run against the real parsers and real databases, use fake clocks rather than real time, and never touch the user's settings or song library.

The real problems sit in a few places. The worst is in the hook suite, not the app: the hook dispatchers let a call through when one of their gates is broken, and no test covers that case. Next, many tests and the GUI runner wait with no time limit, so a hung job would freeze CI instead of failing. A handful of tests prove less than their names claim. And the single-owner scan file keeps growing and never shrinks.

## How the time is spent

One test takes 43% of the run. "rebuild_overlay_fields: equals a full build on every corpus chart and stored path" ([test_track_state.cpp:871](../../tests/test_track_state.cpp)) took 18.2 of the 42 seconds. The single-owner scan took 4.2 seconds, and the random-overlay rebuild test took 1.5. No other test took more than 0.7 seconds. The source is `--duration=true` output from the one full run (`scratchpad\suite-durations.txt`).

## What works and should be kept

The engine tests are the strongest part of the suite. The Note Shuffle tests, the difficulty literals in `difficulty_literals.h`, and the test where the replay must agree with the engine on every corpus path are good models for new tests. The randomised "property" tests in `test_replay.cpp` check that a rule holds over many generated inputs rather than a few hand-picked ones. That catches bugs nobody thought to write a case for.

The store tests break things on purpose. They test a locked database file, a junk file, dropped tables, and an upgrade stopped partway through. That is exactly where data-loss bugs hide.

The Preview tests check that base plus overlay equals a full build on every corpus chart. The clock and transport tests run on fake clocks, so they never flake on a busy machine. Only one test opens a real audio device, and CI already leaves it out.

The newer hook tests (h15 to h38) are well built. They copy the hooks into a temporary folder and use throwaway git repos. test_h27 runs the dispatcher against stub gates, and test_h19 checks that a missing helper makes the hook deny.

## Broken or misleading

These tests would not catch the bug they exist for, or a gap that leaves real risk.

**Hook dispatchers fail open (high).** If a gate script is missing, the dispatcher skips it (`~/.claude/hooks/dispatch_bash_ps.ps1`, line 68). If it crashes, the dispatcher catches the error and skips it too (line 75). If it prints garbage, the verdict merger counts that as "no opinion" (`~/.claude/hooks/lib/verdict_merge.ps1`, line 38). `dispatch_star.ps1` does the same. So a broken gate silently lets every call through, which breaks the "fail loudly" rule. No test feeds a dispatcher a crashing gate. test_h21 checks only that the result is "not ask", so it passes even when the gate crashes (lines 99 and 112).

**Old-database upgrade tests share production's mistakes (medium-high).** The schema 2 and schema 3 upgrade fixtures are built from column lists that production also exports and uses ([test_store.cpp:1168-1202](../../tests/test_store.cpp)). No frozen copy of a real old database is checked in. If production had the old layout wrong, the test would build the same wrong layout and pass.

**The golden-image check is loose (medium-high).** The test passes when the average colour error over the whole frame is under 10 out of 255 ([golden.json:10](../../testdata/preview/golden.json)). ADR 0008 records 5.7 at the time it was written. A wrong-coloured lane or slightly shifted gems would barely move a whole-frame average. The test also passes silently when its fixture file is missing ([test_preview_golden.cpp:140](../../tests/test_preview_golden.cpp)). ADR 0008 says to re-capture the golden when the look changes, but never says how.

**The stem-skipping rule is tested against a copy (medium-high).** Stems are the separate audio tracks a song ships as. The rule "skip a stem that won't open" is checked against a copy of the loop written inside the test ([test_audio_mixer.cpp:124](../../tests/test_audio_mixer.cpp)). Nothing in the tests calls `open_song_stems`, `map_song_stems` or `mix_song_stems`. No load-job test feeds the loader a corrupt stem.

**Tests that recompute their expected answer (medium).** Your rule is that a test pins a literal or calls the production function, and never recomputes the answer. `test_song.cpp:604-727` builds its expected values with regex copies of an old rule. `test_fill_deadline.cpp:63-64` copies the formula.

**Tests that prove less than their names (medium).** "Every corpus chord" in [test_replay.cpp:1371](../../tests/test_replay.cpp) replays one chart. The 300 MB Opus cancel test ([test_preview_load_progress.cpp:259](../../tests/test_preview_load_progress.cpp)) can pass without the load ever starting. `leak_checked` does nothing in a Release build, so the store test that uses it checks no leaks. The "upgrade timing" test has no assertions and cannot fail. A loop in `test_rules.cpp` breaks after the first chart.

## Can hang instead of failing

About 25 loops in the store and app tests wait for a background job with no time cap. Examples are [test_app_state.cpp:982](../../tests/test_app_state.cpp) and its neighbours, `test_song_panel_state.cpp:153` and `test_library_jobs.cpp:90`. [test_cli.cpp:80](../../tests/test_cli.cpp) waits on a child process forever. The GUI runner does the same at [uitest_main.cpp:168](../../tests/ui/uitest_main.cpp). If a job hangs, CI sits until its 60-minute job limit instead of failing one test with a name. This breaks the "every step has a stop condition" rule.

The GUI suite also behaves differently in its two modes. Run serially with `--all`, tests share one GUI context and carry state between them. CI runs `--jobs 4` with a fresh process per test. A test can pass in one mode and fail in the other.

Two tests fail if a step takes 20 ms or more of wall-clock time ([test_library_model.cpp:393](../../tests/test_library_model.cpp), [test_library_query.cpp:379](../../tests/test_library_query.cpp)). A busy machine can trip them. `test_path_view.cpp:691` compares two wall-clock timings, and a counting test already covers the same thing.

## Unnecessary

These can go or be merged without losing protection.

The 18-second overlay test checks every stored path on every corpus chart. The random-overlay test beside it already covers the same property. Checking three paths per chart would keep the corpus check and cut it to about 1 to 2 seconds.

Five developer tools live in the test program as test cases. These are the three always-skipped cases, the always-running "preview dump" case, and the no-assertion dev aid at `test_s2_dynamics_tag.cpp:154`. One of the skipped cases is a benchmark that points at the real song library ([test_replay.cpp:1910](../../tests/test_replay.cpp)). Tools belong in `tools/`, not in the suite.

Smaller duplicates: three chartmode_key tests repeat rows of the 16-row table test. The same database error sentences are pinned in four files. The "(unknown) / tags removed" block is pasted into three. Four SP-overfill cases are near-copies that want one table.

`RecordStore::delete_results_without_chart` has no caller and no test, so it is dead production code.

In the hook folder, 193 `.bak-*` and 2 `.retired-*` files (2.7 MB) sit beside the hooks, in `lib` and in `tests`. Nothing loads them. `hooks\state` holds 6,524 files (40 MB) and nothing prunes it.

## Poorly designed

**test_single_owner.cpp (high).** It is 6,558 lines, and lines 85 to 5682 are a 341-row rules table written as C++. Each row is a regex that guards against one duplicate found in the past. 352 of the 1,335 commits since October 3 touch it. Rows are added and never retired. Its "only shrink" baselines haven't shrunk: three known copies still point at an RW-T5 follow-up, and all 16 clone entries from October 5 are still there. Its failure messages are clear, and it tests its own scanner, which is good. The fix is to move the rules into a data file, give each row an owner and a reason, and review stale rows on a schedule. Practice for this kind of architecture test (ArchUnit's user guide) is that each rule should fail if its pattern matches nothing at all. That way a rename can't quietly turn it into a no-op.

**test_docs_match_code is weak.** It accepts any backticked name that appears anywhere in `src`, `tools` or `tests`, including inside comments. So a doc can name something that survives only in a comment.

**Older hook tests use live state.** Tests h1 to h13, h29 and parts of h2, h3, h21 and h30 run the live hooks against the live `hooks\state` folder. h8, h9 and h10 depend on a real transcript that is 35 days old. The test helpers Check, Get-Decision and Invoke-Hook are pasted into 20 to 33 files. The newer tests show the right pattern: a temporary copy and a shared helper.

**Names and placement.** Several files are named after past plan steps (`test_s2_*`, "finding 37") rather than what they test. `test_s2_parser_owners` is a grab bag. `test_search.cpp` holds engine tests under a library-sounding name. A few files run past 1,000 lines. Some tests pin internal details that break on harmless changes. `test_report.cpp:892` and `:1128` read JavaScript and CSS source as text. Tooltip text is pinned word for word in `test_report_view.cpp:276` and `:391`. Two tests pin a vector's capacity.

## Gaps

**Hooks.** Six hooks have no test: `no_recursive_delegation_gate`, `no_background_in_subagent_gate`, `fanout_mark_delegated`, `fanout_reset`, `posture_inject` and `transcript_lib`. The watchdog's ffmpeg kill is destructive and untested. The D107 reviewer wording on the hard stop (`agent_progress_gate.ps1:254`) is untested. No test checks that every gate is wired into `settings.json` or a dispatcher's list. `suite_run_gate.ps1:153` blocks harmless read-only searches that mention `test_h*`.

**Tool tests nobody runs.** CI runs only `hydra_tests` and `hydra_uitest`. The 12 `ch_probe` pytest files, `test_compare_db.py` and the two `.ps1` self-tests run only when someone runs them by hand. They still import cleanly today.

**Source no test reaches.** 22 of the 88 `.cpp` files under `src` are not included by any test. Most are exercised through the exe-driven CLI test or the GUI suite. Five have no test mention at all: icons, win32_dialogs, imgui_files, dynamics_tab and stars_tab.

**Engine digest.** The engine's output digest in `test_perf_digest.cpp` is pinned only at default settings.

**Not reached.** These were read only partly or not at all: the second half of `test_search.cpp`, the second halves of `test_preview_view`, `test_path_view` and `test_highway_draw`, parts of `test_report.cpp`, `test_timing`, `test_song_length`, `test_sng`, most `s2` tests, the individual GUI tests, and the bodies of about 14 hook test files. Each scout report lists exactly what it missed.

## Tools worth adopting

The tools scout recommends keeping doctest. Catch2, GoogleTest, Boost.Test and ut would cost a migration and buy nothing that matters here. Its ranked list of what would help:

1. **Run the suite in parallel slices.** Register `hydra_tests` with CTest (CMake's test runner) in slices using doctest's `--first`/`--last` or `-sf` filters, then run them with `ctest -j`. First check for tests that share temp files, a database or the audio device.
2. **An AddressSanitizer build in CI.** AddressSanitizer is a compiler mode that catches memory errors while tests run, and MSVC supports it. It may clash with mimalloc, so that build should turn mimalloc off.
3. **Fuzz the parsers.** A fuzzer feeds random malformed files to the chart, MIDI, SNG, SRB, audio and image readers and looks for crashes. MSVC's `/fsanitize=fuzzer` exists but is still marked experimental.
4. **Coverage via Microsoft.CodeCoverage.Console**, uploaded to Codecov. Coverage shows which lines no test runs. OpenCppCoverage, the older tool, has been archived since 2020. Whether Microsoft's output converts cleanly for native C++ is unconfirmed.
5. **Pester for new hook tests.** Pester is PowerShell's standard test framework, and it would replace the pasted helpers. RapidCheck (C++) and Hypothesis (Python) are worth trying on one invariant each.

Two things to skip. Mutation testing grades a suite by planting small bugs and checking that a test fails, but no mutation tool works with MSVC (Mull has no Windows build). ApprovalTests.cpp has had no release since March 2024. The Dear ImGui Test Engine's licence is free for individuals and open-source projects, so Hydra's current use looks fine.
