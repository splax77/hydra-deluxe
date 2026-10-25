# Plan: a tool that picks the merge's tests from the diff

Task TESTPICK-PLAN. Session e14bc305-6203-43d5-8b60-af52a9881f06. Planner: Fable. Status: approved by the user on 2026-10-25.

## User decisions

The user's words: "Approved with the recommended answers: Q1 30, Q2 40, Q3 20, Q4 one level with selector values, Q5 C++ only now. Build it, and hold Task 5 until the hooks build's doc edits are on main."

Q1: a name mentioned by more than 30 test files picks nothing by itself. Q2: more than 40 picked test files runs the whole suite. Q3: a changed header with more than 20 direct includers runs the whole suite. Q4: one level of callers, plus the `A::B` selector values on the call line or its nearest `if`, `else if` or `case` line. Q5: C++ tests only in this build; the Python follow-up is not approved. Task 5 starts only after the hooks-from-scratch build's doc edits (its task T11) are on main.

Later ruling, after TESTPICK-3's review found the plan's Step 3 and worked example disagree, in the user's words: "Keep it: a comment-only change picks the tests that name that function, but doesn't follow its callers." This overrides Step 4 for comment-only changes.

Later ruling, after the tool picked 58 files for its own merge because every function named `run` in src/ matched, in the user's words: "a call site counts only if it's in the same file as the changed function or in a file that includes the header declaring it." This narrows Step 4's caller search.

Later ruling on Step 5's search of test files, the user answering "go with recommend" to this option: "A test file counts for a changed name only if it includes the header that declares it, directly or through a chain. Exception: a name in a file that builds an exe's main (like tools/precheck/main.cpp) keeps today's plain-name search, since tests reach those by running the exe."

Later ruling, the user answering "go with recommended": a changed name in a file that builds an exe's main picks only test files that name that exe's `HYDRA_<TARGET>_EXE` constant, directly or through a test helper header. An exe that no test names that way keeps the plain-name search. In the same answer, the main session changes the posture line in `C:\Users\Patrick\.claude\model-postures.md` in a worktree of that repo with a fresh reviewer, after checking with the hooks session that it is not editing that file.

## What the user asked for

"Plan a small tool that takes a merge range, finds every test file that names a function, type or constant the diff changed, and prints the -sf= filters. Briefs and merges use its output instead of a hand list."

## The incident in one paragraph

Merge 18a2c64f (CH10-SHARED-MEASURES) rewrote `Song::place_fills_ch10` in `src/parse/song.cpp` and deleted the class `Ch10Measures`. The main session wrote the test list by hand: six test files plus the three repo-wide checks. It never searched the tests. `tests/test_preview_load_progress.cpp` loads a chart with `FillDeadlineRule::Ch10`, which runs the rewritten code, and it broke. CI caught it on two pushes. The hand list missed it because no test names `place_fills_ch10` at all. Tests reach that function through the enum value that selects it. A file-name grep ("song") cannot see that either. The tool below reproduces the right answer for this merge mechanically (14 test files, the broken one among them), and the worked example at the end shows every step.

## Questions for the user

Each question quotes the rule the tool would apply and gives an example from both ends. The numbers are mine; every one is a new threshold, so each needs your decision.

**Q1. Common names: a name that more than 30 test files mention does not pick tests by itself.**
The rule: the tool counts how many test .cpp files mention each name (whole word). Over 30, the name is printed under `common:` and picks nothing. If such a name is the head of a changed definition (you changed the function or type that carries it), the tool falls back to the whole suite, because the change really does reach that many files.
Low end: `check_activations` is named by 4 test files, so it picks those 4. High end: `ticks` is named by 46 files and `Song` by 77; neither picks anything on its own, and a change to `Song`'s own declaration runs the whole suite. The biggest real key in the CH10 merge was `FillDeadlineRule::Ch10` at 11 files; `load_songpath` at 20 would still count. I recommend 30. Lower (20) would drop `load_songpath`; higher (50) would let `ticks` pick 46 files, which is nearly the whole suite anyway.

**Q2. Whole suite when the pick is bigger than 40 test files.**
The rule: after expansion, if more than 40 of the 142 test .cpp files are picked, the tool prints `tests: whole suite` and the reason, instead of a 40-name filter. Low end: the CH10 merge picks 14, so it prints 14 filters. High end: a change to `tests/temp_util.h` (included by 44 test files) picks at least 44, so it prints the whole suite. I recommend 40 because the hand lists at merges have been 3 to 10 files, and past 40 the run time is most of the suite's.

**Q3. Widely included header: a changed header that more than 20 files include runs the whole suite.**
The rule: the tool counts the files under src/, tools/ and tests/ that `#include` the changed header directly. Over 20, whole suite, printed with the count. This is CLAUDE.md rule 1's "a header many files include" made exact. Low end: `src/store/stored_versions.h` is included by 3 files, so a change to it picks by name (2 test files in CH10). High end: `src/parse/song.h` is included by 92 files and `src/core/model.h` by 94; a change to either runs the whole suite. `src/core/rules.h` sits at 28 and would also run the whole suite. I recommend 20; if you would rather a rules.h change pick by name, say 30.

**Q4. One level of callers, with the value that selects the changed function.**
The rule: for each changed function, the tool finds its call sites in src/. The calling function's name becomes a key. So does every `A::B` value on the call line, or on the nearest `if`, `else if` or `case` line above it in the same block. It goes one level up, never two. Low end (what one level buys): `place_fills_ch10` is called once, in `check_activations`, under `if (fills == FillDeadlineRule::Ch10)`; the keys `check_activations` (4 files) and `FillDeadlineRule::Ch10` (11 files) come out, and the second one is the only link to the test that broke. High end (why not two levels): `check_activations` is called from two song loaders, and the loaders are named by 20 test files; two levels would pull those in on every fill-generator change, and three would reach nearly everything. I recommend one level with the selector values. Without the selector values, one level alone would have missed the CH10 case; that is the real trade-off, and I would rather you saw it than I hid it.

**Q5. Scope now: C++ tests only. uitest scripts and Python tool tests later.**
The tool prints `-sf=` filters for `hydra_tests.exe` only. `tests/ui/scripts` (1 script today) are driven by widget labels, not by names in the code, so a name grep cannot link them; the brief keeps naming them by hand. `tools/test_*.py` (2 files) test Python tools; the same name grep works on Python through the precheck's existing mask, but it is a second output line and a second set of fixtures, so I put it in a follow-up task the user can approve separately. Do you want the Python half in this build?

## Decision: a new mode of hydra_precheck, not a new exe

The tool is `hydra_precheck --tests <range>`. The precheck already owns the four questions the picker needs: what commits a range names and what lines it changes (`range.cpp`), how to read file text at a revision through one git process (`git_text.cpp`), how to blank comments and strings so a name in a comment never counts (`code_mask.cpp`), and where the braces are (`braces.cpp`). A second parser for any of those would be a second owner of the same question, which the derive-once rule forbids. A mode, rather than a sibling exe, also costs nothing at build time: `hydra_tests` already builds `hydra_precheck.exe` and already knows its path (`HYDRA_PRECHECK_EXE`), so the fixture tests run the real exe with no new CMake target, and the exe is already in the saved CI build folder. The precheck's existing `--code-mask` mode is the precedent for a second job in the same exe.

The picking logic lives in `hydra_precheck_lib` as two new files, so the unit tests call it directly and the exe only prints. Nothing in the precheck's existing checks changes; the mode runs instead of them.

## What the tool does, in order

**Step 1. Read the range and the changed lines, both sides.** `resolve_range` already turns `main...HEAD`, `A..B` or a commit into a base and a tip. `AddedLines` already knows the added lines in the tip's numbering. The picker also needs the deleted lines in the base's numbering, because a deleted class is a changed name that no tip line carries. Task 1 extends the one diff parser (`parse_diff`) to record deleted lines too, under a new `ChangedLines` type with `added` and `deleted` sides. A merge commit is resolved as `first-parent...second-parent`, which is the range that was reviewed.

**Step 2. Decide the fallbacks before any name work.** If any changed file is a build file (`CMakeLists.txt`, `CMakePresets.json`, anything under `cmake/`, `build_cpp.ps1`, anything under `.github/workflows/`), or part of the test harness (`tests/test_main.cpp`, `tests/shards.txt`, `third_party/doctest/`), or a header with more direct includers than the Q3 limit, the tool prints `tests: whole suite` and one `why:` line naming the file and the rule. It still prints the `always:` line (Step 7). These files are decided once, in one function the output names, so the docs can point at it instead of listing them again.

**Step 3. Extract the changed names.** For each changed production file (not a test file, decided by the precheck's existing `test_file`), read the masked text of the tip (for added lines) and of the base (for deleted lines), build the braces, and for each changed line take the names it defines or sits inside. The rule has four parts, and the new file `changed_names.cpp` owns all four:
A changed line inside a block takes the name from the head of every enclosing block up to the first namespace: a `class`, `struct`, `union` or `enum` head gives the type's name; a function head gives the function's name with any `Class::` qualifier dropped; a namespace head gives nothing and stops the walk. This is how `place_fills_ch10` and the local struct `Cell` come out of one changed line inside `Cell`.
A changed line that itself declares something gives that name: a constant (`constexpr`, `const` or `static` before an identifier followed by `=` or `{`) gives its name, a line inside an `enum` block gives `Enum::Value`, and a line inside a type block that declares a function (an identifier followed by `(`) gives that function's name.
A changed line that masks to blank (a comment) takes the declaration of the next non-blank masked line, so a reworded comment above a function counts as a change to that function. This is how `check_activations` comes out of the CH10 range, whose first hunk changes only the comment above it.
A deleted line is read against the base text with the same rules, so a deleted class gives its name.
Members of a changed or deleted type are not separate keys. A test can only reach them through the type's name, so the type's name is the key. A member function whose own out-of-line body changed is a key under its member name, because tests call it by that name.

What this misses, and why that is accepted: macros (no braces, no head), overloads (one name covers all of them, which over-includes rather than misses), templates (the head rule reads `template<...>` as part of the head and takes the name after it; a specialisation shares the name), and names a test reaches only through `auto` or a function pointer. The tool over-includes by design; running a test file that was not needed costs seconds, and the miss it is built to prevent cost two CI runs.

**Step 4. Follow one level of callers (Q4).** For each changed function name, grep the masked text of every production file under src/ at the tip for `name(` outside the function's own definition. Each call site gives the enclosing function's name (same head rule as Step 3) and every `A::B` token on the call line or on the nearest preceding `if`, `else if` or `case` line in the same block, `std::` excluded. One level only.

**Step 5. Grep the tests.** For each key, one `git grep -l -w` over `tests/*.cpp` and `tests/*.h` at the tip, through the precheck's `Repo::git`. A key found in a test helper header (any `tests/*.h`) expands to every test .cpp that includes that header, because the helper runs inside those files. The CH10 range shows why this matters even when it adds nothing: `tests/record_fixtures.h` mentions `FillDeadlineRule::Ch10` in a comment only, and the mask drops it. Every test file the range itself changes is picked with the reason `changed in range`. Files under `tests/single_owner/` are not picked by name; the scan on the `always:` line runs them.

**Step 6. Apply the common-name and size rules (Q1, Q2).** Count files per key before picking; a key over the Q1 limit is printed under `common:` and picks nothing, and if it heads a changed definition the result is the whole suite. After expansion, more than the Q2 limit of files is the whole suite.

**Step 7. Print.** Exactly this shape, so a brief and a merge command paste the first two lines unchanged:

```
tests: -sf=*test_cli.cpp,*test_config.cpp,*test_fill_deadline.cpp,...
always: "-tc=docs match code*" "-sf=*single_owner*" "-sf=*test_precheck*"
names: place_fills_ch10 (body) Ch10Measures (deleted) Cell (body) check_activations (comment) kCh10FillLengthMeasures (comment) kResultsStamp (value)
callers: check_activations <- place_fills_ch10; FillDeadlineRule::Ch10 selects place_fills_ch10 in check_activations
common: (none)
why: tests/test_preview_load_progress.cpp  FillDeadlineRule::Ch10
why: tests/test_generated_fills.cpp  changed in range; check_activations; FillDeadlineRule::Ch10
...
```

In the whole-suite case the first line is `tests: whole suite` and one `why:` line gives the rule that fired and its count. The `tests:` line uses `*name.cpp` patterns, the same form `tests/shards.txt` uses, so a filter never matches a second file by prefix (`*test_replay*` would also run `test_replay_windows.cpp`). The merge command is `build-cpp\Release\hydra_tests.exe <tests line> ` and then the three `always:` filters each as its own run, as CLAUDE.md rule 1 already gives them. A brief pastes the `tests:` line into its "tests you may run" sentence.

## What the docs say afterwards

CLAUDE.md rule 1 keeps its ruling and its three always-run commands and changes one sentence: "it runs the tests that touch the changed files" becomes "it runs the tests `hydra_precheck.exe --tests <range>` prints (the tool owns which files count as shared code and when the whole suite runs)". The brief preamble's "Tests" section adds one sentence saying the brief's test list comes from that command, run by the main session on the branch, so an executor never guesses. `docs/agents/integrate.md` line 44 and `docs/agents/derive-once-review.md` step 3 add the same pointer where they tell the agent which tests to run. The memory file `merge-tests-touch-changed-files.md` is the main session's to update, not a task here; the plan asks the main session to add one line pointing at the tool. The posture text in `C:\Users\Patrick\.claude\model-postures.md` line 11 gets the same one-sentence change through a worktree of that repo; it is text, not hook code, so it does not collide with the hooks-from-scratch plan (`2026-10-25-hook-redesign.md`), which never mentions how merge tests are picked. None of these pages restate the thresholds or the file lists; they point at the tool.

## Prior art (one web search)

Coverage-based selection (Datadog Test Impact Analysis; the Ratchet C++ prototype on Clang) picks tests from recorded per-test coverage. Rejected: it needs a coverage run of the whole suite to be kept current, and this repo has nothing of the kind. Ekstazi and Bazel `rdeps` pick by file or build-target dependency; rejected, because here one `hydra_tests.exe` links everything, so every test depends on every file, and a file-level answer is "the whole suite" every time. Machine-learned selection (Facebook's predictive selection) needs months of history; rejected. Borrowed: the plain static rule from the practitioner guide, a name-level dependency lookup run as a check, and the universal fallback that when the selection cannot be trusted the whole suite runs. Sources: [Ratchet thesis](https://tdl-ir.tdl.org/handle/2152/ETD-UT-2011-05-3306), [Datadog Test Impact Analysis](https://docs.datadoghq.com/tests/test_impact_analysis/how_it_works/), [Impact-based test selection](https://dzone.com/articles/impact-based-test-selection).

## Worked example: the CH10 range

Range `5f357402...b5c11a75` (the merge's first parent to its second parent). Changed production files: `src/parse/song.cpp` and `src/store/stored_versions.h` (3 includers, under the Q3 limit). Changed test files: `tests/test_generated_fills.cpp`, `tests/test_store_records.cpp`. No build or harness file.

Names from Step 3, checked by hand against the diff: `check_activations` (the first hunk changes only the comment above it), `kCh10FillLengthMeasures` (comment above it), `Ch10Measures` (deleted, from the base text), `place_fills_ch10` (body) and `Cell` (local struct inside it), `kResultsStamp` (value). Git's own hunk headers would not do: they name `int Song::note_count() const` for the comment hunk and `private:` for another, which is why the tool finds enclosing definitions itself.

Step 4: `place_fills_ch10` is called once, in `check_activations`, guarded by `if (fills == FillDeadlineRule::Ch10)`. Keys added: `check_activations` (already present) and `FillDeadlineRule::Ch10`. `check_activations` is also called twice, from the song loaders, but that is level two and is not followed.

Step 5 and 6, counts measured with `git grep -l -w` at 18a2c64f: `check_activations` 4 files, `kResultsStamp` 1 test file (plus the scan's `special.cpp`), `FillDeadlineRule::Ch10` 11 files, the rest 0. Nothing is over the Q1 limit. The printed `tests:` line names 14 files: test_cli, test_config, test_fill_deadline, test_fill_report, test_fills_and_skips, test_generated_fills, test_preview_controller, test_preview_load_progress, test_preview_scene, test_replay, test_report_rows, test_rules, test_song_parse, test_store_records. The merge ran 6; it missed 9 of these (test_timing, which it ran, is not in the list because the diff only uses `MeasureIndex`, it does not change it). `test_preview_load_progress` is picked for the reason `FillDeadlineRule::Ch10`.

## Tasks

Each task is one Opus executor, under 100 tool calls and 20 minutes, in its own worktree on its own branch. All test runs are `build-cpp\Release\hydra_tests.exe "-sf=*test_precheck*"` plus the task's own new test file, never more. Tasks 1 and 2 are independent and run in parallel. Task 3 needs both. Task 4 needs 3. Task 5 needs 3 and runs beside 4.

**Task 1: deleted lines in the diff parser.** Owns `tools/precheck/range.h`, `tools/precheck/range.cpp` and a new `tests/test_precheck_pick_lines.cpp`. It adds `ChangedLines` (an `added` side in tip numbering and a `deleted` side in base numbering, from the same `-U0` parse), keeps `AddedLines` working for the five existing checks, and resolves a merge commit as first-parent...second-parent. Acceptance: a fixture repo where one commit deletes a class and adds a line gives the deleted line numbers against the base and the added ones against the tip; a fixture merge commit resolves to the reviewed range; the existing precheck self-test still passes.

**Task 2: changed names from masked text.** Owns new `tools/precheck/changed_names.h`, `tools/precheck/changed_names.cpp`, the CMake line that adds them to `hydra_precheck_lib`, and a new `tests/test_precheck_pick_names.cpp`. Pure text in, names out: it takes a masked text, its `Braces` and a set of line numbers. Acceptance, each a test case with an inline text fixture: a line inside a member function body gives the member's name without its `Class::`; a line inside a local struct gives both the struct and the function; a changed comment line above a function gives the function; a `constexpr` line gives the constant; a line inside an enum block gives `Enum::Value`; a line inside a class block that declares a function gives the class and the function, and a plain data member line gives only the class; a changed line inside a namespace with no enclosing head gives nothing; a name inside a comment or string never appears (the mask test).

**Task 3: the picker.** Owns new `tools/precheck/test_pick.h`, `tools/precheck/test_pick.cpp`, the CMake line for them, and a new `tests/test_precheck_pick.cpp`. It holds Steps 2, 4, 5 and 6: the shared-code decision (build files, harness, widely included header with the Q3 count), the one-level caller walk with selector values, the test grep with helper-header expansion and `changed in range`, the common-name rule and the size rule, and the result as a struct (picked files with reasons, names, callers, common names, whole-suite reason). The three thresholds are named constants in `test_pick.h`, each with the user's decision beside it, and nowhere else. Acceptance, on fixture repos built with `precheck_fixture.h`: (a) the CH10 miss reproduced: a production file with `void run(Mode m) { if (m == Mode::A) a(); else b(); }`, a commit that changes `a`'s body, and a test file that names only `Mode::A`; the test file is picked with reason `Mode::A`, and a test file naming neither is not; (b) a deleted class is a name and its member names are not keys; (c) a key in a helper header picks every test .cpp that includes the header; (d) a changed test file is picked with `changed in range`; (e) a name mentioned by more test files than the Q1 limit is listed under common and picks nothing, and when it heads a changed definition the result is the whole suite; (f) more picked files than the Q2 limit gives the whole suite; (g) a changed header with more includers than the Q3 limit gives the whole suite with the count; (h) each of the build-file and harness paths gives the whole suite, one test case per path kind, so a path dropped from the list fails a test; (i) `tests/single_owner/` files are never picked by name.

**Task 4: the exe mode and output.** Owns `tools/precheck/main.cpp`, `tools/precheck/test_pick.cpp` (the printing half only; Task 3 is merged first) and `tests/test_precheck_pick_exe.cpp`. It adds `--tests [range]` to the usage, runs the picker instead of the checks, and prints the lines of Step 7. Acceptance: on the Task 3 fixture (a), the exe's first line is a `tests:` line with `*name.cpp` patterns in byte order and the second is the fixed `always:` line; the whole-suite case prints `tests: whole suite` then one `why:` line; `--tests` with a bad range exits 1 with the git error on stderr; a filter pattern never ends in a bare `*` (the prefix-match guard).

**Task 5: the docs.** Owns `CLAUDE.md` rule 1, the "Tests" section of `docs/agents/brief-preamble.md`, line 44 of `docs/agents/integrate.md`, step 3 of `docs/agents/derive-once-review.md`, and a worktree edit of `C:\Users\Patrick\.claude\model-postures.md` line 11. Each page changes one sentence to point at `hydra_precheck.exe --tests` and the tool's ownership of the shared-code rule; none restates a threshold or a file list. Acceptance: the docs-vs-code check passes (`hydra_tests.exe "-tc=docs match code*"`), and a grep of the five pages for the three numbers (30, 40, 20) in this plan finds none of them, which the executor records as a log line in its report.

**Follow-up (not in this build unless Q5 says yes): Python tests.** Extend Step 5 to `tools/test_*.py` when a changed file is Python, printing a `py:` line with the files to run. Same fixtures, one more task.

## Out of scope

uitest scripts under `tests/ui/scripts` stay hand-named in briefs (Q5 explains why). Two or more levels of callers (Q4). Any per-test-case (`-tc=`) selection: the unit is the test file, as `-sf=` already is.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Deleted names count (a deleted class is a key) | Step 1, Step 3 "A deleted line" | 1, 3 | Task 1 deleted-line fixture; Task 3 (b) |
| A merge commit resolves to first-parent...second-parent | Step 1 | 1 | Task 1 merge-commit fixture |
| Existing precheck checks keep working | Decision section | 1, 2, 3 | `-sf=*test_precheck*` passes in each task |
| Build files, harness and widely included headers run the whole suite | Step 2, Q3 | 3 | Task 3 (g), (h), one case per path kind |
| Shared-code files are decided in one function | Step 2 | 3 | Task 3 (h) calls that function; Task 5 grep finds no list in the docs |
| Enclosing head names, local structs, comments above, constants, enum values, class members | Step 3 | 2 | Task 2 cases, one each |
| Members of a changed type are not separate keys | Step 3 | 2, 3 | Task 2 data-member case; Task 3 (b) |
| Names in comments and strings never count | Step 3 | 2 | Task 2 mask case |
| One level of callers with selector values, never two | Step 4, Q4 | 3 | Task 3 (a) picks by `Mode::A`; a level-two caller's test is asserted not picked |
| Helper-header hits expand to their includers | Step 5 | 3 | Task 3 (c) |
| Changed test files are picked | Step 5 | 3 | Task 3 (d) |
| single_owner files never picked by name | Step 5 | 3 | Task 3 (i) |
| Common names pick nothing; common head means whole suite | Step 6, Q1 | 3 | Task 3 (e) |
| More than the size limit means whole suite | Step 6, Q2 | 3 | Task 3 (f) |
| Thresholds live in one header with the decision beside them | Task 3 text | 3, 5 | Task 3 constants; Task 5 grep for 30/40/20 in the docs finds none |
| Output shape: `tests:` then `always:`; `*name.cpp` patterns; whole-suite form | Step 7 | 4 | Task 4 exe cases |
| A filter never matches a second file by prefix | Step 7 | 4 | Task 4 bare-`*` guard case |
| Bad range fails loudly with exit 1 | Step 7 | 4 | Task 4 bad-range case |
| Docs point at the tool and restate nothing | Docs section | 5 | docs-vs-code check; Task 5 grep log line |
| CH10 range picks test_preview_load_progress | Worked example | 3, 4 | Task 3 (a) is the same shape; the main session runs `--tests 5f357402...b5c11a75` once after merge and checks the 14 names (a log line, not a test, because the real repo is not a fixture) |
| Python tests and uitest scripts | Q5, Out of scope | none | User's decision |
