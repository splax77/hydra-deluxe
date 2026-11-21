# Test fix plan follow-ups

Date: 2026-11-21. Branch: `claude/plan-tf-followups`. This plan changes no code itself; it says what each executor does.

## What this is

The test fix plan left a list of open follow-ups. A scout wrote the list, so it is a summary, not proof. You can read it with `git show claude/test-fix-plan-close-handoff:docs/handoffs/2026-11-21-test-fix-plan-close/followups.md`. It has 70 rows. The scout says 45 are open, but its table has 44 rows marked OPEN (39 numbered rows and X1 to X5). The scout's own "40 numbered rows" is the miscount.

Four open rows are not this plan's: row 1 and row 17 (both settled by the user on 2026-11-21), row X5 (it has its own plan, on branch `claude/plan-ch10-fills`) and row 64 (it waits on the test fix plan's other work). That leaves 40 rows. Row 61 is marked GONE by the scout, so it is not one of the 44; the leftover branch `claude/tf-C183` is part of row 63.

I read every one of the 40 against the code on main (commit `38304a9c9`). Twenty-eight are planned below. Twelve are not: five need an owner map or the user, and seven need no work. The section "Not planned" says why for each. One half of row 33 is already fixed, and task P1 says so.

Almost every planned row is the same kind of fix. A test types a sentence, a number or a small rule again because the code keeps it private. The fix gives it a name in `src` (or in the tool), makes the code use that name, and makes the test read it. No task changes a word a player sees, a number Hydra uses, or what any tool prints.

## Global constraints

These hold for every task. Each one has a row in the Promise map.

1. No text on a screen changes. A task that names a sentence copies it byte for byte into the constant.
2. No number changes value. A task that names a number keeps the value the code holds today. No task adds a new threshold, tolerance or wait time. If a task cannot be done without one, the executor stops and reports.
3. A named value has one home. After a task, the old literal is gone from `src` and from the tests; both read the name.
4. Each executor works only in the files its task owns. If it needs another file, it stops and reports which one and why.
5. Executors never build and never run a test that needs a build. The main session builds each wave once and runs the tests.
6. A task whose first step finds its row already fixed, or finds no test that types the value, stops and reports that instead of adding a name nobody reads.

## The tasks

Each task is one Opus executor in its own worktree, cut from main by hand. Tasks A1 and A3 to A14 run together as wave 1 (there is no A2: row 24 was dropped, see "Not planned"). Their files do not overlap. Task A15 is wave 2, because it edits a test file A1 also edits. P1, P2 and D1 need no build of their own and can run at any time beside wave 1.

Builds: two. One build tree holds all of wave 1 (A1 and A3 to A14, plus D1's one comment in a header). A second holds A15. No task needs `earlyStart`: A15 simply starts after A1 is merged.

For every C++ task, the main session gets the merge's test list from `build-cpp\Release\hydra_precheck.exe --tests` on the branch. The tests named below are the ones I found by reading; the precheck list is the final word.

### A1: Name the report windows' waiting sentences (row 10)

The report windows show a few fixed sentences while they build, and the screen test types each of them again. The path report sets "Building the report from your library..." and the "Analyzing " line inline. The comparison window sets "Fetching scores and building the report..." inline. The shared frame draws "Report cancelled." inline. The test also types the path report's search box hint, "Search song, artist, charter, or path notation", twice (lines 324 and 1551). `src` does not hold that sentence as a literal: `path_report_window.cpp` builds it by calling `app::html::search_hint` with the list {"song", "artist", "charter", "path notation"}, and the comparison window reads `charts_search_hint()` from `src/app/html_page.h` (premise P5).

The executor gives each sentence a named constant beside the ones that already exist for these windows (`kPathReportFailed` and `kLeaderboardWaking` in `src/app/user_messages.h`). The three windows then read the names, and the test reads them too. For the search hint, the executor adds `path_report_search_hint()` to `src/app/html_page.h`, beside `charts_search_hint()`, with the list the window passes today. The window calls it and the test calls it, so the field list has one home.

Owns: `src/ui/path_report_window.cpp`, `src/ui/dm_report_window.cpp`, `src/ui/report_window.cpp`, `src/app/user_messages.h`, `src/app/html_page.h`, `tests/ui/uitest_report_windows.cpp`. Needs a C++ build. Covered by `hydra_uitest` (the report windows cases, `test_filters` and `test_states`).

Acceptance: no sentence above, and no search hint, is typed in `tests/ui/uitest_report_windows.cpp`; each appears once in `src`; `test_states` and `test_filters` pass on the build. The reviewer diffs each constant against the old literal, byte for byte.

### A3: Give the batch strip's progress words one function (row 44)

The strip that shows a running batch builds its line inline: a lead word, two spaces, then "N of M". The screen test has a helper, `strip_progress`, that builds the same line again, and its own comment says there is no function to call.

The executor moves the lead-and-count part of that line into one function in `src/ui/library_parts.h`, which the tests already include. The strip calls it, and the test's `strip_progress` is deleted in favour of it. The function builds exactly the text the strip builds today.

Owns: `src/ui/library_dialogs.cpp`, `src/ui/library_parts.h` (and `library_parts.cpp` if the header needs a body there), `tests/ui/uitest_batch_reports.cpp`. Needs a C++ build. Covered by `hydra_uitest` (the batch report cases).

Acceptance: `strip_progress` no longer builds text itself; the strip and the test call the same function; the batch cases pass.

### A4: Share the Path Builder's ending-line name and best-fill helpers (rows 15, 16)

The steps panel names each ending line "##end_" plus a tick or a notation, built inline. The Path Builder screen test builds that name again in `end_ref`. The same test has `card_numbers`, which finds the best next fill by walking the candidates itself, because the window's own helpers `best_candidate` and `best_fill_name` are private to `pb_window.cpp`.

The executor moves three things into a pathbuilder header: a function that returns an ending line's widget name, `best_candidate`, and `best_fill_name`. The header is the existing one that declares `fill_name`. The window calls them. In the test, `end_ref` calls the name function, and `card_numbers` calls `best_candidate` and `best_fill_name` instead of its own loop.

Owns: `src/pathbuilder/pb_window.cpp`, the pathbuilder header that declares `fill_name` (and its `.cpp` if bodies move there), `tests/pb_ui/pbtest_coach.cpp`. Needs a C++ build. Covered by the Path Builder screen tests in `tests/pb_ui/` (the coach cases).

Acceptance: "##end_" appears once in `src` and not in `tests/`; `card_numbers` holds no loop over the candidates; the coach cases pass.

### A5: Name the Preview panel's layout numbers and button hints (rows 30, 32)

The Preview panel's text layout uses numbers that live inside `preview_panel.cpp`: the notation line's scale, how many header words stay together, the gap between panels, the large type's factors and the right column's margins. `tests/test_overlay_layout.cpp` types all of them again in a struct, and its comment says a test cannot read them. The two step buttons' hover hints, "Previous activation ([)" and "Next activation (])", are literals in the same file.

The executor moves the named numbers into a header beside the panel, and gives the unnamed ones (the gap, the factors, the margins) names there with the values the code uses today. The panel reads the header. The test reads the same names and drops its typed copies. The two hints become constants in the same header; the executor greps `tests/` and switches any test that types them.

Owns: `src/ui/preview_panel.cpp`, a new `src/ui/preview_panel_layout.h`, `tests/test_overlay_layout.cpp`, and any screen test its grep finds that types the two hints, if no other task owns it. Needs a C++ build. Covered by `tests/test_overlay_layout.cpp` and the Preview cases of `hydra_uitest`.

Acceptance: the `PanelText` struct in the test is filled from the header's names with no typed factor; every number has the value it had; the layout cases pass.

### A6: Make the drawn span's end edge readable (row 29)

`span_interval` works out where a drawn span starts and ends in seconds. It is private to `track_state.cpp`. `tests/test_highway_draw.cpp` restates its end edge as "1.5 plus `kSpanEndTicks` over 1000", which is the function's rule typed again for a scene with one tick per millisecond.

The executor declares `span_interval` in `src/render/track_state.h`, beside `kSpanEndTicks`, and the test calls it for the expected edge.

Owns: `src/render/track_state.cpp`, `src/render/track_state.h`, `tests/test_highway_draw.cpp`. Needs a C++ build. Covered by `tests/test_highway_draw.cpp`.

Acceptance: the two cases in `test_highway_draw.cpp` that use `kSpanEndTicks / 1000.0` get their expected edge from `span_interval`; they pass.

### A7: Name the old title placeholder and the generated-fills feature (rows 31, X2)

`title_or_unknown` treats "<unknown title>" as an old placeholder, but keeps that word in a constant local to the function, so `tests/test_analysis.cpp` types it. Separately, `Song::check_activations` adds the feature name "Auto-Generated Fills" as a literal, and `tests/test_generated_fills.cpp` types it twice.

The executor makes the placeholder a public constant in `src/parse/song.h` beside `kUnknownTitle`, and makes the feature name a public constant in `src/core/song.h`. The two functions and the two tests read the names.

Owns: `src/parse/song.cpp`, `src/parse/song.h`, `src/core/song.cpp`, `src/core/song.h`, `tests/test_analysis.cpp`, `tests/test_generated_fills.cpp`. Needs a C++ build. Covered by those two test files.

Acceptance: each of the two strings appears once in `src` code and not in `tests/`; both test files pass.

### A8: Share the Opus gain formula; guard two macro-prone calls (rows 22, 35)

The Opus reader turns the file's gain field into a multiplier with a small formula inside a private method. `tests/test_stem_reader.cpp` types the same formula in `opus_head_gain` and says the owner is private. The executor moves the formula into a function in the reader's `detail` header, calls it from the reader, and calls it from the test.

The second fix is in `src/app/ini_errors.h`. It writes `std::numeric_limits<int>::min()` and `max()` bare. When a file includes windows.h first, its `min` and `max` macros break those calls. The executor wraps each name in parentheses, the way `tests/test_stars.cpp` already does.

Owns: `src/audio/opus_reader.cpp`, the header that declares `open_opus_reader`, `tests/test_stem_reader.cpp`, `src/app/ini_errors.h`. Needs a C++ build. Covered by `tests/test_stem_reader.cpp` and the ini tests the precheck list names.

Acceptance: `opus_head_gain` in the test holds no `pow`; `ini_errors.h` has no bare `::min()` or `::max()`; both test sets pass.

### A9: Name the 1.0 fill lead's clamp and the activation row's top padding (rows 38, X1)

`activation_fill_deadline_ms` clamps the Clone Hero 1.0 lead between 250 and 10000 as bare numbers. `tests/test_fill_deadline.cpp` types both again and says the code exposes no constant. The executor names the two in `src/search/graph.h`, with the values the code has, and the test reads them.

`activation_row_layout` works out a row's top padding as a local. `tests/test_activation_row_layout.cpp` types the same sum in `top_pad`. The executor makes the padding a small inline function in `src/ui/activation_row_layout.h`, which the layout calls and the test calls.

Owns: `src/search/graph.cpp`, `src/search/graph.h`, `tests/test_fill_deadline.cpp`, `src/ui/activation_row_layout.h`, `tests/test_activation_row_layout.cpp`. Needs a C++ build. Covered by those two test files.

Acceptance: 250 and 10000 are not typed in `test_fill_deadline.cpp`; `top_pad` in the row test holds no arithmetic of its own; both pass. The clamp's values are unchanged (constraint 2).

### A10: Make the installer log's path readable (row 47)

`installer_log_path` joins the update folder and "setup.log" (`src/ui/app_state.cpp:1116`). It is private to `app_state.cpp`. The leftover-folder test in `tests/test_update_state.cpp` types "setup.log" at line 752, and a comment at line 745 says the function is not public (premise P11). The executor declares `installer_log_path` in `ui/update_job.h`, the header that holds the update job's sentences. The test calls it, and the comment's excuse goes.

Owns: `src/ui/app_state.cpp`, `src/ui/update_job.h`, `tests/test_update_state.cpp`. Needs a C++ build. Covered by `tests/test_update_state.cpp`.

Acceptance: "setup.log" appears once in `src` and not in `tests/`; the test passes.

### A11: Share two test helpers and the contrast floors (rows 11, 40)

Three copies live in test files. First, `sweep_settings` and its depth are private to `tests/test_engine_sweeps.cpp`, and one case in `tests/test_rules.cpp` builds the same settings by hand so it can reuse the sweep's cached records. Second, `listed_again` in `tests/test_report_page.cpp` does the same copying as `scanned_copy` in `tests/test_report_rows.cpp`. Third, the contrast floors 4.5 and 3.0 are typed in both `tests/test_theme.cpp` and `tests/test_report_page.cpp`, and the first file's comment says so.

The executor moves `sweep_settings` and `kSweepDepthScores` into a shared test header, and the `test_rules.cpp` case calls it. It moves `scanned_copy` into a shared test header and `listed_again` calls it. It puts the two floors, once, in the header that already holds the `testwcag` helpers, and both test files read them. The other hand-built settings in `test_rules.cpp` (the 1.0 fills case) are a different search and stay.

Owns: `tests/test_engine_sweeps.cpp`, `tests/test_rules.cpp`, `tests/test_report_page.cpp`, `tests/test_report_rows.cpp`, `tests/test_theme.cpp`, the `testwcag` header, and one new shared test header. Needs a C++ build. Covered by those five test files.

Acceptance: `test_rules.cpp` holds no `depth_value = 200`; one function copies a chart to a second folder; 4.5 and 3.0 appear once in `tests/`; all five files pass.

### A12: Stop pinning the Trivium ticks (row 19)

One case in `tests/test_pb_text.cpp` checks that a fill on a window's end tick is not a choice row. It names a chart and types two ticks, 180000 and 195360. The second is confirmed by a `REQUIRE` against the engine, but both were picked by hand from one run.

The executor keeps the chart and the starting tick as the test's input, and reads the end tick from the engine: `one.activations[0].sp_end_tick`. Every later use of 195360 becomes that value. The case then requires that a fill sits on that tick, so it fails loudly if the chart ever stops giving the case.

Owns: `tests/test_pb_text.cpp`. Needs a C++ build. Covered by that file.

Acceptance: 195360 is not typed in the file; the case still requires a fill on the window's end tick and passes.

### A13: Let the precheck test read check 1's wording (row 49)

The precheck's first check keeps its sentences in strings private to `check_1.cpp`, such as `kHelperWhy`. `tests/test_precheck_check1.cpp` says so and checks only names and places.

The executor moves check 1's fixed sentences into a header in `tools/precheck/`, and the test checks each item's reason against them. The tool's output does not change.

Owns: `tools/precheck/check_1.cpp`, a new `tools/precheck/check_1_text.h`, `tests/test_precheck_check1.cpp`. Needs a C++ build. Covered by `tests/test_precheck_check1.cpp`.

Acceptance: the test compares at least `kHelperWhy` with the item's printed reason; the comment about file-local strings is gone; the test passes.

### A14: Prove a save holds the store's lock all the way through (row 6)

One store test runs lookups and saves side by side. Its own comment says it cannot prove the lock covers the whole save, only that it would probably catch a lost lock. The store already has a test seam of this kind, `set_between_lookup_reads_for_test`.

The store's own rule for its lock (`src/store/record_store.h:502`) says it covers sqlite calls and nothing else. The executor reads that rule first and checks only the steps inside a save that make sqlite calls. The lock is a `std::recursive_mutex` (line 505), so a `try_lock` from a second thread fails while the saving thread holds it (premise P30). The executor adds one more seam: a function the store calls at each step inside a save, set only by a test. The new test sets it to check, at every step, that the store's lock is held by the saving thread (a `try_lock` from a second thread fails). That needs no wait time, so it adds no new number. If the executor finds it cannot prove the lock without a timed wait, it stops and reports (constraint 2).

Owns: `src/store/record_store.h`, `src/store/record_store.cpp`, `tests/test_store_results.cpp`. Needs a C++ build. Covered by `tests/test_store_results.cpp`.

Acceptance: a new case fails if the save releases the lock before its last step; the old case's comment no longer says the lock is unproven; the file passes. The reviewer says which line, removed, turns the new case red.

### A15: Check on screen that Clear filters lets a dragged column go (row 52, wave 2)

A unit test already checks that Clear filters moves the table's reset key. No screen test drags a column and then clicks Clear filters. The audit asked for one.

The executor adds a case to `tests/ui/uitest_report_windows.cpp`: read a column's width, drag its edge, check the width changed, click "Clear filters", and check the width is back to the first reading. Both readings come from the table itself, so the test types no width.

Starts after A1 is merged, because A1 edits the same file. Owns: `tests/ui/uitest_report_windows.cpp`. Needs a C++ build (the second). Covered by `hydra_uitest`.

Acceptance: the new case fails if Clear filters leaves the dragged width in place; it passes on the build.

### P1: Three Python tool tests (rows 18, 33, X3)

No build. All three are in the Clone Hero tools.

The quiet check calls the machine quiet when the CPU share is at or under `QUIET_CPU_PCT`. The tests only check that the limit's number appears in a line. The executor adds a boundary test: a reading equal to `QUIET_CPU_PCT` is quiet, and the next reading above it is not. It reads the constant; it types no 30.

`mono_facts.run_reader` builds its failure message inline, and the test matches it with its own pattern. The executor moves the message into one small function in `mono_facts.py` and the test builds its expectation from that function's parts. The scout's other half, "the reader cap has no constant", is already fixed: `READER_CAP_S` exists and the test reads it.

`play_chart.load_schedule` refuses a schedule with no chords with a sentence built inline. The executor names the sentence and adds or tightens the test so it checks the refusal by that name.

Owns: `tools/ch_mcp/tests/test_quiet.py`, `tools/ch_mcp/mono_facts.py`, `tools/ch_mcp/tests/test_mono_facts.py`, `tools/ch_probe/experiments/play_chart.py`, `tools/ch_probe/tests/test_play_chart.py`. The executor runs those three test files itself.

Acceptance: the boundary test fails if `<=` becomes `<`; the two messages each have one home; the three files pass.

### P2: Name two hooks' messages (row 23, hooks repo)

No build. This task is in the hooks repo, `C:\Users\Patrick\.claude`, in a worktree under its `worktrees` folder.

`review_dispatch._key_problem` builds three refusal sentences inline. `progress.py` builds its two wrap-up notes inline. The executor gives each sentence a named template at the top of its file, makes the code fill it, and makes the hook tests that check those messages read the names. No message changes.

Owns: `gates/review_dispatch.py`, `gates/progress.py`, and the test files under `gates/tests/` that check these two hooks' messages. The executor runs only those test files.

Acceptance: each sentence appears once in `gates/`; the named tests pass; the reviewer diffs the messages against the old text.

### D1: Four stale lines (rows 26, 37, 51, X4)

No build of its own. One of the four is a comment in a C++ header, so this branch joins the wave 1 build tree.

The comment on `relative_slash_path` in `src/core/strutil.h` says "as scan_snapshot.json keys it". The executor greps for what still writes or reads that file. If nothing does, the comment says what the function returns without naming the file.

`docs/agents/test-rubric-examples.md` cites `tests/test_stars.cpp:124` as a `to_multiplier` call. That line is now inside a table of rows; the calls are at lines 47 and 51. The executor fixes the citation, and checks the neighbouring citation of `tests/test_replay.cpp:1613` the same way.

A comment in `.github/workflows/ci.yml` says the precheck self-test skips a part when two commits are missing. The scout says it now fails instead. The executor reads `tests/test_precheck.cpp` and makes the comment say what the test does.

ADR 0019 says the Opus decode "is pinned bit for bit by a test, and that test is unchanged". The test that exists checks that reading one frame at a time equals the whole decode, bit for bit, and that the fixture is a 220 Hz sine. It compares with no saved copy of the old output. The executor rewrites that one sentence to say what the test checks.

Owns: `src/core/strutil.h`, `docs/agents/test-rubric-examples.md`, `.github/workflows/ci.yml`, `docs/adr/0019-preview-audio-streams-from-the-compressed-file.md`.

Acceptance: each of the four lines matches the code it describes, and the reviewer opens each cited place to confirm it.

## Not planned

Twelve rows get no task. Five of them need an owner map or the user. Seven need no work.

### Needs an owner map or the user

- Row 12 (two checks in the engine sweeps test). I read one of the two: the test turns a backend row's offset back into a tick with its own rounding and its own special case for zero. Replacing that needs the engine's own answer to "which tick does this row's offset point at". That is an owner-map question. I could not identify the other check (the "deadline check") from the scout's one line.
- Row 27 (the Preview's meter used as referee for the engine's list of passed fills). The test checks an engine list against a display value. Turning that round needs the engine's own record of how much Star Power a path held at a fill. That is an owner-map question.
- Row 28 (one reader for the all-0 answer). The record states the answer in two ways, and the paths screen reads it at `src/app/path_view.cpp`. A single reader would change what that screen is built from, so it needs an owner map.
- Row 50 (a name function beside `ViewedSong::State`). The names would come from the screen test harness, which is not a source, and the audit says one of them ("rules-broken") does not match the state it stands for. Fixing it changes what the test tool's `state` report prints, so it needs an owner map.
- Row 63 (the handoff branches). Eighteen `claude/test-fix-*handoff*` branches are unmerged (none is an ancestor of main), and `claude/tf-C183` is left over. Merging or deleting branches is the user's call, not an executor's.

### Needs no work

- Row 2 (a process id of 2^32 or more). Windows process ids are 32 bits, and the scout's own verdict is "a no-op risk". No caller can pass such a number, so a rule for it would be a made-up one (agent rule 15). Closed.
- Row 20 (audit rows E4:591 and E4:675 and the branch `claude/tf-C298`). The branch is in the hooks repo, not this one, and it is already merged into the hooks repo's `main` (commit `12a44f1`, "Cleanup-sweep hook test: report checks read what was removed, not typed counts"). The scout's "not on either main" is wrong. The record `records/E4.md` is not on main in this repo: it lives only on the unmerged branch `claude/test-fix-plan` (`docs/audit/2026-11-14-test-audit/records/E4.md`). Rows 591 and 675 there are audit history about a hook test that has since been reworked. Editing a record that is not on main is not this plan's work. Closed.
- Row 21 (a function for the song length before the end pad). The pad is already a public constant and the tests read it. I found no test that types the length before the pad, so a new function would have no reader. Closed.
- Row 24 (the report filters' first choices). The three sentences are literals in `src/app/path_report_view.cpp:49,55` and `src/app/dm_report_view.cpp:181`, but a grep of `tests/` and `src/` for "All timing tiers", "No timing needed" and "All charts" finds them only there (and "All charts" once more in `src/app/fill_report.cpp:52`, a web page option). No test types them, so a name would have no reader. Closed (constraint 6).
- Row 41 (untested capture code). The row itself says no work is possible without a recorded capture, and the user ruled the capture out. Closed.
- Row 42 (nothing formats the game's accuracy numbers). Hydra's tool only parses these numbers. The game formats them. A formatter in the tool would exist only for the test, and its rule would be made up. Closed.
- Row 48 (`tools/check_pointers.py` builds its two hook replies itself). `tools/test_check_pointers.py` already loads the hooks repo's `gates/common.py` and checks each reply against `common.context` and `common.system_message` (lines 40-60), so a drift in the shape fails that test today. Making the tool itself import `common.py` would tie a Hydra tool to a file in another repo. The tool runs as a SessionStart hook under `py -I`, and its failure reply must print even when the gates folder cannot load, so that reply could not come from the helper. Closed: the existing test is the owner of the shape.

## Out of scope

Rows 1, 17, X5 and 64, and the test fix plan's other work (B1 to B9, H1, H2, G1, V0 to V2, O1), as the brief says. The user's decisions on these rows are in premise P51.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| No screen text changes | Global constraint 1 | A1, A3, A4, A5, A7 | The screen tests compare the screen with the new constants and pass; the reviewer diffs each constant against the old literal |
| No number changes value; no new number | Global constraint 2 | A5, A9, A11, A14 | `test_overlay_layout.cpp`, `test_fill_deadline.cpp` and `test_theme.cpp` pass with the named values; A14's case uses no wait |
| A named value has one home | Global constraint 3 | every A and P task | Each task's acceptance line: the literal appears once in `src` and not in `tests/` (a grep the reviewer runs) |
| Executors stay in their own files | Global constraint 4 | every task | The reviewer checks the branch's changed files against the task's Owns line |
| Executors never build | Global constraint 5 | every task | The main session's two build passes are the only builds; the stall hook's call list shows none |
| A row with no reader is reported, not built | Global constraint 6 | A5 (the two hints) | The executor's report shows the grep; the reviewer reruns it |
| The strip and its test call one function | A3 | A3 | `strip_progress` holds no text building; the batch cases pass |
| `card_numbers` holds no loop of its own | A4 | A4 | The reviewer reads the function; the coach cases pass |
| The Trivium case fails loudly if the chart stops giving the case | A12 | A12 | The case's `REQUIRE` that a fill sits on `sp_end_tick` |
| The lock is proven held through the whole save | A14 | A14 | The new case; the reviewer names the line whose removal turns it red |
| A dragged column lets go after Clear filters | A15 | A15 | The new screen case compares the two width readings |
| The quiet check's limit is tested at its edge | P1 | P1 | The boundary test fails when `<=` becomes `<` |
| Hook messages do not change | P2 | P2 | The hook tests pass reading the names; the reviewer diffs the text |
| Each of the four stale lines matches the code | D1 | D1 | The reviewer opens each cited place |
| Two builds cover all the C++ work | The tasks, "Builds" | main session | Wave 1 is one build tree; A15 is the second |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The path report sets its building sentence and its "Analyzing " line inline | code | `src/ui/path_report_window.cpp:197-205` | checked 2026-11-21 |
| P2: The comparison window sets "Fetching scores and building the report..." inline | code | `src/ui/dm_report_window.cpp:125-126` | checked 2026-11-21 |
| P3: The shared frame draws "Report cancelled." inline | code | `src/ui/report_window.cpp:190-192` | checked 2026-11-21 |
| P4: The report windows test types those sentences and the search hint | code | `tests/ui/uitest_report_windows.cpp:324-400` | checked 2026-11-21 |
| P5: `src` builds the path report's search hint from `app::html::search_hint` with four field names; no literal of the sentence exists in `src`, and the test types it twice | code | `src/ui/path_report_window.cpp:157-158`, `src/app/html_page.h:52-67`, `tests/ui/uitest_report_windows.cpp:324`, `tests/ui/uitest_report_windows.cpp:1551` | checked 2026-11-21 |
| P6: Named constants for these windows already live in one header | code | `src/app/user_messages.h:177-185` | checked 2026-11-21 (a grep hit, lines 177 and 185) |
| P7: The three filter labels are literals in the two view files | code | `src/app/path_report_view.cpp:45-58`, `src/app/dm_report_view.cpp:180-185` | checked 2026-11-21 |
| P8: No test or other code types the three filter labels | code | `src/app/path_report_view.cpp:49`, `src/app/path_report_view.cpp:55`, `src/app/dm_report_view.cpp:181`, `src/app/fill_report.cpp:52` (a web page option); a search of all of `src` and `tests` for the three sentences found them nowhere else | checked 2026-11-21 (row 24 is not planned) |
| P9: The batch strip builds its progress line inline and the test rebuilds it | code | `src/ui/library_dialogs.cpp:443-444`, `tests/ui/uitest_batch_reports.cpp:52-58` | checked 2026-11-21 |
| P10: The ending line's name and the best-fill helpers are private to the window, and the test repeats them | code | `src/pathbuilder/pb_window.cpp:112-128`, `src/pathbuilder/pb_window.cpp:866-868`, `tests/pb_ui/pbtest_coach.cpp:172-178`, `tests/pb_ui/pbtest_coach.cpp:738-767` | checked 2026-11-21 |
| P11: A test in `tests/test_update_state.cpp` types "setup.log" | code | `tests/test_update_state.cpp:745-752` | checked 2026-11-21 |
| P12: `installer_log_path` is private to `app_state.cpp` | code | `src/ui/app_state.cpp:1115-1118` | checked 2026-11-21 |
| P13: The Preview panel's layout numbers and two hints are private to the panel file, and the layout test types them | code | `src/ui/preview_panel.cpp:150-156`, `src/ui/preview_panel.cpp:343-350`, `src/ui/preview_panel.cpp:545-547`, `tests/test_overlay_layout.cpp:177-194` | checked 2026-11-21 |
| P14: `span_interval` is private and adds `kSpanEndTicks` to the end tick | code | `src/render/track_state.cpp:141-161` | checked 2026-11-21 |
| P15: `tests/test_highway_draw.cpp` restates the span's end edge as 1.5 plus `kSpanEndTicks` over 1000 | code | `tests/test_highway_draw.cpp:472`, `tests/test_highway_draw.cpp:796` | checked 2026-11-21 |
| P17: `check_pointers.py` builds two hook replies by hand, and its test already checks both against `common.py`'s helpers | code | `tools/check_pointers.py:291-305`, `tools/test_check_pointers.py:40-60`, `C:\Users\Patrick\.claude\gates\common.py:28-33` | checked 2026-11-21 (row 48 is not planned) |
| P18: The old placeholder is local to `title_or_unknown`, and the test types it | code | `src/parse/song.cpp:600-604`, `src/parse/song.h:100-108`, `tests/test_analysis.cpp:1209-1215` | checked 2026-11-21 |
| P19: "Auto-Generated Fills" is a literal in `check_activations`, and a test types it | code | `src/core/song.cpp:107-113`, `tests/test_generated_fills.cpp:297-299` | checked 2026-11-21 |
| P20: The Opus gain formula is inside a private method and the test types it | code | `src/audio/opus_reader.cpp:308-317`, `tests/test_stem_reader.cpp:271-279` | checked 2026-11-21 |
| P21: `ini_errors.h` calls `numeric_limits<int>::min()` and `max()` bare | code | `src/app/ini_errors.h:44-49` | checked 2026-11-21 |
| P22: The 1.0 lead's clamp is two bare numbers and the test types them | code | `src/search/graph.cpp:89-90`, `tests/test_fill_deadline.cpp:34-38` | checked 2026-11-21 |
| P23: The activation row's top padding is a local, and the test types the same sum | code | `src/ui/activation_row_layout.h:61-68`, `tests/test_activation_row_layout.cpp:83-86` | checked 2026-11-21 |
| P24: `sweep_settings` is private to the sweeps test and `test_rules.cpp` builds the same settings by hand | code | `tests/test_engine_sweeps.cpp:84-103`, `tests/test_rules.cpp:617-629` | checked 2026-11-21 |
| P25: Two test files each copy a chart to a second folder, and two each type the contrast floors | code | `tests/test_report_page.cpp:54-65`, `tests/test_report_rows.cpp:96-109`, `tests/test_report_page.cpp:332-335`, `tests/test_theme.cpp:28-32` | checked 2026-11-21 |
| P26: The Trivium case types 180000 and 195360 and requires the second from the engine | code | `tests/test_pb_text.cpp:523-540` | checked 2026-11-21 |
| P27: Check 1 keeps its wording in file-local strings and the test says so | code | `tests/test_precheck_check1.cpp:243-244` | checked 2026-11-21 |
| P28: `check_1.cpp` holds `kHelperWhy` as a file-local string | code | `tools/precheck/check_1.cpp:40`, `tools/precheck/check_1.cpp:151` | checked 2026-11-21 |
| P29: The store test says it cannot prove the lock covers the whole save, and the store already has a test seam | code | `tests/test_store_results.cpp:516-524`, `src/store/record_store.h:773-780` | checked 2026-11-21 |
| P30: The store's lock is a recursive mutex whose rule is "sqlite calls and nothing else", so a second thread can try it inside a save without a timed wait | code | `src/store/record_store.h:502-505` | checked 2026-11-21 |
| P31: The report windows test clicks Clear filters and has no drag step | code | `tests/ui/uitest_report_windows.cpp:317-341` | checked 2026-11-21 |
| P32: The harness can drag with the mouse (move, button down, move, button up), and the report tables' column room comes from `table_room` | code | `tests/ui/uitest_details.cpp:838-844`, `src/ui/column_widths.h:193-198` | checked 2026-11-21 |
| P33: The quiet check compares with `<=` against `QUIET_CPU_PCT`, and the test checks only the limit's text | code | `tools/ch_mcp/quiet.py:205`, `tools/ch_mcp/tests/test_quiet.py:163` | checked 2026-11-21 |
| P34: `QUIET_CPU_PCT` is 30 and is a named constant | code | `tools/ch_mcp/quiet.py:26-31` | checked 2026-11-21 |
| P35: `run_reader` builds its failure message inline, the cap is already `READER_CAP_S`, and the test matches with its own pattern | code | `tools/ch_mcp/mono_facts.py:64-81`, `tools/ch_mcp/tests/test_mono_facts.py:268-283` | checked 2026-11-21 |
| P36: `load_schedule` builds the "no chords to play" refusal inline | code | `tools/ch_probe/experiments/play_chart.py:262-269` | checked 2026-11-21 |
| P37: The two hooks build their messages inline | code | `C:\Users\Patrick\.claude\gates\review_dispatch.py:50-62`, `C:\Users\Patrick\.claude\gates\progress.py:92-100` | checked 2026-11-21 |
| P38: Hook tests under `gates/tests/` check those messages | code | `C:\Users\Patrick\.claude\gates\tests\test_review_dispatch.py:226-235` (the key refusals, by fragment), `C:\Users\Patrick\.claude\gates\tests\test_progress.py:192-196` (the wrap-up note) | checked 2026-11-21 |
| P39: The `relative_slash_path` comment names `scan_snapshot.json` | code | `src/core/strutil.h:59-67` | checked 2026-11-21 |
| P40: Nothing writes or reads `scan_snapshot.json` any more | code | `src/core/strutil.h:59` is the only hit of a search of the whole repo, apart from old session and audit notes | checked 2026-11-21 |
| P41: The rubric examples page cites `tests/test_stars.cpp:124`, which is now a table row; the calls are at 47 and 51 | code | `docs/agents/test-rubric-examples.md:319-324`, `tests/test_stars.cpp:118-125`, `tests/test_stars.cpp:45-52` | checked 2026-11-21 |
| P42: The CI comment says the self-test skips a part without the two commits | code | `.github/workflows/ci.yml:61-64` | checked 2026-11-21 |
| P43: The precheck self-test now fails, not skips, without the two commits | code | `tests/test_precheck.cpp:479-481` | checked 2026-11-21 |
| P44: ADR 0019 says a test pins the Opus decode bit for bit, and the test compares two reads of one file with no saved output | code | `docs/adr/0019-preview-audio-streams-from-the-compressed-file.md:67-68`, `tests/test_audio_decode.cpp:169-195` | checked 2026-11-21 |
| P45: The song-end pad is a public constant | code | `src/app/song_length.h:35-37`, `src/app/song_length.cpp:54-55` | checked 2026-11-21 |
| P46: The sweeps test turns a backend row's offset back into a tick with its own rounding | code | `tests/test_engine_sweeps.cpp:430-443` | checked 2026-11-21 |
| P47: `pid_running` passes the pid straight to `OpenProcess` with no range check | code | `tools/ch_mcp/game_tools.py:84-100` | checked 2026-11-21 |
| P48: The trailing-fills test judges the engine's list with the Preview's meter, and the record states the all-0 answer two ways | code | `tests/test_fills_and_skips.cpp:322-325`, `tests/test_fills_and_skips.cpp:405-444` | checked 2026-11-21 |
| P49: The accuracy test has its own filler and the tool only parses | code | `tools/ch_probe/tests/test_ocr.py:54-66` | checked 2026-11-21 |
| P50: `ViewedSong::State` has seven values and no name function beside it | code | `src/ui/app_state.h:170-180` | checked 2026-11-21 |
| P51: Rows 1, 17, X5 and 64 are out of scope | user | the user's choices as the main session reported them for this review: row 1, "Close it (Recommended)" (transfer hint closed as done, D17's log line updated, main commit `38304a9c9`); row X5, "Match the old game" (own plan, branch `claude/plan-ch10-fills`, which exists); row 17, asked whether to add lib, cvtres, mspdbsrv and Tracker, the user typed "is this a real concern or an edge case you invented", and the main session found no recorded failure and closed it with the list unchanged; row 64 waits on V0 and V1. Not a filed ruling | checked 2026-11-21 (the branch and the commit exist; the words are the main session's report) |
| P52: The scout's table has 44 rows marked OPEN, not 45 | count | the scout's table: 39 numbered rows (1, 2, 6, 10, 11, 12, 15 to 24, 26 to 33, 35, 37, 38, 40 to 42, 44, 47 to 52, 63, 64) and X1 to X5 | checked 2026-11-21 |
| P53: `claude/tf-C298` is merged into the hooks repo's main; `records/E4.md` is only on `claude/test-fix-plan` | code | a git ancestry check run in the hooks repo, and a listing of that branch's files in this repo | checked 2026-11-21 |
| P54: Eighteen handoff branches named like `claude/test-fix-wave18-handoff` exist and none is an ancestor of main | code | a branch listing and an ancestry check for each, run in this repo | checked 2026-11-21 |

## Questions

None. Every task reads a value or a sentence the code already holds, and the closures of rows 1, 17, X5 and 64 are the user's own (premise P51).
