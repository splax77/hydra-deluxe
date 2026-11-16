# A small mutation tool for Hydra: one build, every planted bug

Date: 2026-11-15. Session 88ea48bb. Status: waiting for the user's yes. Nothing here is built yet.

## The short version

A mutation tool plants small bugs in the real code and runs the tests. A test that goes red on a planted bug has shown it can fail. Hydra already has such a tool, `tools/mutation_probe.ps1` (removed; `git show ed0aa857:tools/mutation_probe.ps1`), but it rebuilds once per bug. The user ruled that out: "one build per test is a massive waste of time. if we're doing this it's one build and run to test everything".

The new tool, `hydra_mutate`, writes every bug into the code at once. Each bug sits behind its own numbered switch, and all switches are off unless one is named. So there is one build. One run with every switch off records which test reaches which bug spot. Then each bug is switched on alone, and only the test files that reach it are run. No rebuild happens between bugs.

No agent picks a bug and no agent judges a result. A fixed list of text rules picks the spots. A test counts as "caught the bug" only when doctest itself reports it failed. This is the point of the tool: the user asked "how can I trust any of these answers?" about agents reading tests, and a run is the answer.

The work is nine small tasks (M1 to M8, with the runner and its report split in two), then seven timed steps by the main session on a small sample, then the first real use on the audit's 62 disputed rows. The sample steps can stop the plan; "What stops the plan" says on what.

One thing departs from "one build". If the compiler refuses some planted bugs, the plan allows up to three small repeat builds of the changed files. That needs your yes, so it is question 2. Nothing is built until you answer.

This plan changes nothing Hydra decides or shows. The normal build never carries a switch. So no Owner map applies (`docs/agents/owner-map.md` covers only plans that change what Hydra decides or shows).

## What exists already, and what is reused

Agent rule 15 asks for the existing owner first. I searched `tools`, `src`, `tests`, `docs` and `CMakeLists.txt` for `mutation`, `mutant`, `codelex`, `code-mask`, `lexer_language` and `t7_bug`. Five pieces exist.

The first is `tools/mutation_probe.ps1` (removed; `git show ed0aa857:tools/mutation_probe.ps1`). It already owns the list of bug kinds (`Find-MutationCandidates`), the rule that edits go only into code, and the time limit for a stuck run (`Get-TestTimeoutMs`, the user's decision D82 item 3). It plants one bug, rebuilds, tests, restores, and repeats. The new tool takes over its list of kinds and its time limit, so each rule keeps one home. The time limit's rule is now the user's own for this tool: a bug run's own clean time plus 5 seconds (the answers of session 60c3a576). Task M8 removes the probe, if the user agrees (question 1).

The second is the repo's one lexer, codelex, in `tests/code_lexer.h`. A lexer splits a file into code, comments and strings. `hydra_precheck --code-mask <file>` prints a file with everything that is not code blanked out, same length, same line numbers. The probe already calls it. The new tool calls it the same way, for C++ and for Python files. It does not write a second lexer.

The third is how T7 did this by hand. On branch `claude/test-audit-t7`, a header `src/core/t7_bug.h` reads one environment variable, and each plant is written `t7_bug("A4") ? <bug> : <original>`. One build held every plant. That is the same shape this plan uses, and T7 showed it builds with MSVC and that tests pass with every switch off. T7 also showed two traps, which this plan handles. Plants in `src/` made two text-reading tests fail (the single-owner scan and the layering check). And three plants could not sit behind a runtime switch, because the compiler needed a constant there.

The fourth is doctest, the test framework (`third_party/doctest/doctest.h`, version 2.4.11). It lets a program register a listener that is told when each test starts, ends, and when a check fails. `tests/test_main.cpp` already has its own `main`, so the mutation build can add a listener there.

The fifth is the build path: `build_cpp.ps1` takes `-Preset` and `-Target`, `CMakePresets.json` already holds test-only presets (asan, coverage, fuzz), and `CMakeLists.txt` already has options of the form "never ships" (`HYDRA_COVERAGE`, `HYDRA_ASAN`, `HYDRA_FUZZ`).

I found no existing code that records which test reaches which line at run time. The search was for `test_case_start`, `REGISTER_LISTENER` and `IReporter` under `tests/` and `tools/`; `HYDRA_COVERAGE` exists but produces a whole-run report, not a per-test one.

Prior art, from the research report (`reports/Mutation testing tools for Hydra.md`). Compiling all bugs in once behind switches is called "mutant schemata". Dredd does it with one environment variable naming the live bug, and dextool and Mull's new rewriter do it too; none documents Windows or MSVC, which is why Hydra writes its own. Vercammen et al. 2022 measured schemata plus "run only the tests that reach the bug" at 2x to 30x faster than rebuilding, and found the limits this plan also has: templates and constant expressions cannot be switched at run time, and a dense switched build can itself run slower (0.81x in one project). Google's "arid node" idea is to not plant bugs in lines where a survivor would tell nobody anything, such as logging. This plan does not copy that, because choosing such lines is a judgment; see "Out of scope".

## What the tool does, step by step

**Step 1: find the spots.** The tool reads each chosen source file through `hydra_precheck --code-mask`, so comments and strings can never match. It then applies a fixed list of kinds. Four come from the brief: flip a comparison (`<` and `<=`, `>` and `>=`, `==` and `!=`), swap `&&` and `||`, drop a `!`, and flip a returned `true` or `false`. One more comes from the existing probe's list: a binary `+ 1` becomes `- 1`. The reason to keep it is that off-by-one is the bug T7's auditors named most, and the probe already makes it; it costs no new rule.

**Step 2: wrap each spot.** A spot is wrapped as `HYDRA_MUT(17, <original text>, <text with the one change>)`. The macro reads "if switch 17 is on, use the changed text, otherwise the original". The tool does not parse C++. Instead it wraps the whole smallest expression the lexer can bound: the text between the nearest enclosing bracket pair, comma, semicolon, `return` or assignment `=`. Because that text is a complete expression, putting it inside a bracketed choice cannot change what binds to what. The original text is copied byte for byte into the "off" side.

**Step 3: skip what cannot be bounded, and list it.** A spot is skipped, never guessed, when the bounding walk meets anything it cannot classify. That means: a preprocessor line or macro body; a `<` or `>` with no space on both sides (a likely template bracket; this is the probe's existing rule); a lambda or a brace inside the expression; and any expression that follows `template`, `constexpr`, `consteval`, `static_assert`, `case`, `requires`, `noexcept`, `enum`, or sits in an array size. Every skipped spot goes into `skipped.txt` with its file, line and the reason.

**Step 4: build once.** The main session builds `hydra_tests` with the new `mutation` preset. If the compiler rejects a wrapped spot, the tool reads the compiler's own error lines, drops every spot on those lines, lists them as "skipped: did not compile", and the main session rebuilds only the changed files. This is the one departure from "one build": a few top-up rebuilds while the tool learns which spots MSVC refuses. It is capped at three rounds. A fourth is a stop. It needs the user's yes (question 2). If the answer is no, the first failed build is a stop: the main session reports the refused spots and the user decides.

**Step 5: the mapping run.** The suite runs once with every switch off. Each switch, when its line executes, notes its number. The doctest listener writes, for each test, the numbers noted while it ran. That gives the map: which test reaches which bug. A test that fails in this run is listed as "red with no bug" and takes no further part; the two text-reading tests T7 hit cannot fail here, because the rewritten files are copies (see below).

**Step 6: one bug at a time.** For each bug at least one test reaches, the tool starts a fresh `hydra_tests.exe` with `HYDRA_MUTANT=<number>` and a doctest source-file filter naming the test files that hold a reaching test. Several such processes run side by side. The listener writes each test's pass or fail and each failed check's file and line. A run that outlives its time limit is stopped and recorded as "timed out". The limit is that run's own clean time plus 5 seconds. Its clean time is the sum of the mapping run's own times for the tests in the files it runs, which doctest's `--duration=true` prints in the mapping run's console output; a run of every file uses the whole mapping run's time.

**Step 7: the report.** Written as text files in the run's folder. Per bug: file, line, the change in words, and every test that went red, with the check lines that failed. Per test: how many bugs it reaches and how many it caught, with the tests that caught none listed first. Three more lists: bugs no test reaches, bugs reached but caught by nobody (survivors), and spots skipped with reasons. Timed-out bugs are counted as caught but listed apart, as the probe does today.

The report states its own limit in its first lines. A test that caught nothing has shown no way to fail against these bugs; that is a question to look at, not proof the test is empty. And a check that sits after an earlier failing `REQUIRE` in the same test may never have run.

## Where the switches live

The switch header is `tools/mutate/hydra_mutant.h`. It is not under `src/`, so the layering check and the single-owner scan never see it. It holds the `HYDRA_MUT` macro, one `inline` integer read once from the `HYDRA_MUTANT` environment variable (the way `t7_bug.h` does), and the "reached" notes. C++17 allows an `inline` variable in a header, and Hydra builds as C++17.

The rewritten sources are copies. `hydra_mutate` writes them into `build-mutation/mutated/...`, never into `src/`. A new CMake option, `HYDRA_MUTATION` (off by default, marked "never ships" like `HYDRA_COVERAGE`), makes the libraries compile the copy of a file when one exists, adds `tools/mutate` to the include path, and defines `HYDRA_MUTATION` for `tests/test_main.cpp` so it registers the listener. A new `mutation` preset, both a configure preset and a build preset (`build_cpp.ps1` stops without a build preset that names a configuration), turns the option on and uses its own build folder, `build-mutation` (a `.gitignore` line covers it, as for the other test-only folders). The rewritten copies sit in `build-mutation/mutated/`. With the option off, nothing in the build changes: no include path, no define, no macro. The option refuses to configure together with the `ship` preset's `HYDRA_LTCG`.

## What stops the plan

The sample is the code first use needs anyway: the source files reached by the tests that hold the disputed lines (task F1 names them mechanically). The main session measures, in the live steps below, and stops and reports to the user if any of these is true.

1. The switched build fails after three top-up rounds, or after the first failure if question 2 is a no (LS1).
2. With every switch off, a test fails that passes in the normal build (LS2). That would mean a wrap changed behaviour, which breaks the tool's base promise. The two text-reading tests T7 hit are checked in the normal build after M5 merges, because new files under `tools/mutate` could trip them.
3. The projected time for the first use is not shorter than doing it the rejected way. The projection is: measured seconds per bug in LS3b, times the number of reached bugs, divided by the number of side-by-side processes. The rejected way is: one measured warm rebuild of `hydra_tests` per bug, from LS1's top-up timing. If the new tool does not beat a rebuild per bug, it has no reason to exist.
4. Two runs of the same sample disagree on any verdict (LS3 against LS3b). A tool whose answers move cannot settle a dispute.

No absolute hour limit is set here, because none has a source. The main session shows the user the projected hours for the first use, and for a whole-codebase run, before either starts (agent rule 3).

## The tasks

Each task is one executor in its own worktree, then one fresh reviewer, then merge (`docs/agents/derive-once-review.md`). Executors never build and never run `hydra_tests.exe`. M1, M2, M4, M5 and M6 can run in parallel. M3 needs M1, M2 and M5 merged (it reads M5's result files). M3b waits for LS3 (it tests against real results). M7 needs M3b and M5. M8 is last. The live steps need these merged first: LS0 needs M1 and M2; LS1 needs M2, M4 and M5; LS2 needs LS1; LS3, LS3b and LS4 need M3 and LS2; LS5 needs M6.

### M1: the spot finder (Python, no build needed)

Owned files: `tools/mutate/find.py`, `tools/mutate/test_find.py`. The finder takes a source path, calls `hydra_precheck.exe --code-mask` from the main checkout, and returns numbered spots: offset, length, kind, the enclosing expression's bounds, or a skip reason. The kinds are one table in this file. Tests the executor may run: `py -I -m unittest tools/mutate/test_find.py`.

Acceptance criteria. The tests run the finder on real Hydra files from `src/`, not on made-up snippets (the user's ruling: "tests should never be based on anything fake"). They check properties the code must hold, not typed counts: every spot's offset lies in code according to the mask; every spot's enclosing expression has balanced brackets; no spot lies on a preprocessor line; every skipped spot carries a reason from the finder's own reason table; and the same file read twice gives the same numbered list. One test per skip rule finds a real line in `src/` that the rule skips by searching for it, and fails if the finder returns a spot there.

### M2: the rewriter (Python, no build needed)

Owned files: `tools/mutate/rewrite.py`, `tools/mutate/test_rewrite.py`. It takes the finder's spots and writes the wrapped copy and a `mutants.json` (number, file, line, kind, change in words). Nested and overlapping spots in one expression are handled by wrapping the outer expression once per spot, innermost first. Tests the executor may run: `py -I -m unittest tools/mutate/test_rewrite.py`.

Acceptance criteria. For real files from `src/`: taking the "off" side of every `HYDRA_MUT` in the copy gives back the original file byte for byte. Taking the "on" side of exactly one switch gives a file that differs from the original only inside that spot's bounds, and by exactly the kind's change. The copy is never written under `src/`; the test fails if the output path is inside the source tree. Numbers are unique and stable across two runs.

### M3: the runner (Python, no build needed to write; main session runs it)

Owned files: `tools/mutate/run.py`. `run.py` drives steps 4 to 6: reads compiler errors and drops spots, starts the mapping run, starts one process per bug with the source-file filter (`--processes N` sets how many run side by side), applies the time limit, and collects the listener's result files. `--one-process` is the LS4 comparison. The time limit moves here from `mutation_probe.ps1` (removed; `git show ed0aa857:tools/mutation_probe.ps1`), so it has one home once M8 lands; `test_timeout_seconds` holds it, with the rule the user chose in session 60c3a576. `test_run.py` tests the parts that need no build (the time sums, the limit, the re-score); LS3 is the test of the whole.

Acceptance criteria. The runner never rebuilds between bugs; its only build-related act is to print which spots it dropped. It refuses to start bug runs when the mapping run had a test fail that is not on the "red with no bug" list it just wrote. It reports as an error, not a catch, any test that went red on a bug the map says it does not reach. A fourth compile round exits with a message and no report.

### M3b: the report

Owned files: `tools/mutate/report.py`, `tools/mutate/test_report.py`. `report.py` turns the collected result files into the report of step 7. Tests the executor may run: `py -I -m unittest tools/mutate/test_report.py`. That test needs real result files, so it waits for LS3's result folder.

Waits for: LS0, LS1, LS2, LS3b.

Acceptance criteria. The report test reads LS3's real result folder and checks: every red test in the result files appears under its bug; every test with zero catches is in the zero list; the per-test and per-bug counts add up to the same total (the user's rule that two counts on a screen must agree).

### M4: the build option and preset

Owned files: `CMakeLists.txt` (the `HYDRA_MUTATION` block only), `CMakePresets.json` (the `mutation` configure preset and the `mutation` build preset only), `.gitignore` (one line, `build-mutation/`). Tests: none an executor can run; the main session proves it in LS1.

Acceptance criteria. With the option off, the configure output and the list of compiled sources are the same as today; the reviewer checks by reading that every added line sits inside `if(HYDRA_MUTATION)`. With it on, a file with a copy under the mutation folder is compiled from the copy and a file without one from `src/`. Configuring with both `HYDRA_MUTATION` and `HYDRA_LTCG` fails with a message. The configure preset inherits `default`, sets `binaryDir` to `${sourceDir}/build-mutation` and the option on; the build preset names `configuration` `Release` and `hydra_tests` as its target, as the `fuzz` build preset names its target. Premise P14 says why both presets and the folder line are needed.

### M5: the switch header and the listener

Owned files: `tools/mutate/hydra_mutant.h`, `tools/mutate/listener.cpp`, `tests/test_main.cpp` (one `#ifdef HYDRA_MUTATION` block). The header is described above. After M5 merges, the main session runs the two text-reading tests ("single-owner rules hold across src/, tools/ and tests/" and "layering: every src file is in one library") in the normal build, because new files under `tools/` could trip them. The listener writes one line per test (name, file, line, passed or failed, the switch numbers reached) and one line per failed check, to the file named by `HYDRA_MUTANT_OUT`. Tests: none an executor can run; LS2 proves it.

Acceptance criteria. With `HYDRA_MUTANT` unset, `HYDRA_MUT(n, a, b)` evaluates `a` and only `a`. With it set to n, it evaluates `b` and only `b`, and every other switch evaluates its `a`. Without `HYDRA_MUTATION` defined, `tests/test_main.cpp` compiles to exactly what it does today; the reviewer checks every added line is inside the `#ifdef`. Reach notes made while no test is running (static start-up) are written under the name "before any test".

### M6: the Python half

Owned files: `tools/mutate/py_mutate.py`, `tools/mutate/test_py_mutate.py`. It does the same job for Python with Python's own `ast` module (the parser Python ships): the same kinds, each rewritten as `(changed if switch(n) else original)`. It copies the target folder (`tools/`, or the hooks repo `C:\Users\Patrick\.claude\gates`) into the run folder, rewrites the copy, and runs the folder's own unittest files against the copy, with the same mapping run and the same one-bug-per-process runs. The real folders are never edited. Tests the executor may run: `py -I -m unittest tools/mutate/test_py_mutate.py`.

Acceptance criteria. For real files from `tools/`: the rewritten module with every switch off produces the same `ast` as the original once the wrappers' "off" sides are taken. The test fails if any file under the real `tools/` or the gates folder has a changed modification time after a rewrite. A test file that imports its target by absolute path, and so never reaches the copy, shows up as "reaches no bug" in the map rather than as a pass; the acceptance test finds one real case if one exists and otherwise says none was found.

### M7: the docs

Owned files: `docs/development.md` (the mutation section), `docs/agents/this-machine.md` (one line, if a path is new). Says how the main session runs the tool, what the report's lists mean, and its limits. Needs M3b and M5 merged. Tests: the main session runs `hydra_tests.exe "-tc=docs match code*"`.

Acceptance criteria. Every command and file name in the section exists in the merged code. The docs check passes.

### M8: retire the old probe (only if question 1 is a yes)

Owned files: `tools/mutation_probe.ps1`, `tools/test_mutation_probe.ps1` (both removed), `docs/development.md` (the three probe mentions). The probe's finder and time limit already moved in M1 and M3, so after M8 each rule has one home. If the answer is no, this task is dropped and the probe is changed instead to read its kinds and time limit from `tools/mutate`, so there are still not two copies.

Acceptance criteria. `git grep mutation_probe` finds only lines that say how to read it now, in the form the docs check asks for. The docs check passes.

### F1: first use, the 62 disputed rows

Run by the main session after the live steps pass.

Waits for: LS0, LS1, LS2, LS3b, M3b.

The eight re-read files (`A3.md`, `A30.md`, `A9.md`, `C7.md`, `WN3.md`, `WN4a.md`, `WN4b.md`, `WN8.md` under `docs/audit/2026-11-14-test-audit/mechanical/reread/` on branch `claude/test-audit-t8-questions-handoff`) hold 62 rows whose verdict cell starts "does not hold" or "DNH" or contains "cannot tell" (premise P16 gives the count per file). Each row names a test file and a line, not a test name. An executor copies, row by row, the file and line into `tools/mutate/first_use_lines.txt` (the file comes from the line cell, or from the slice file's opening sentence when the cell holds a bare line). That is copying, not judging. The runner turns each line into a test mechanically: the test is the doctest test case in that file whose registered line is the nearest one at or before the named line, read from doctest's own registration. It never names a test for a line that sits outside every TEST_CASE (a helper function): that row reads "outside every test case" and still reports whether a check on that exact line failed. The code under test is then chosen by the run, not by anyone: it is every source file holding a spot that one of those tests reaches in a mapping run.

Acceptance criteria. The output is the tool's report, unedited, plus one table: each disputed row, its test, the bugs that test caught, and the failed check lines. No row holds an agent's verdict. The tool plants its own kinds of bug, not the change a row names, so a row whose test caught nothing is marked "not shown to fail by this run" and goes to the user as it stands; the table says this in its opening line.

## The live steps

These are runs only the main session makes, each through a pass from `hydra_build_pass` where it builds. Each step holds one new piece, so a failure names it.

### LS0: rewrite the sample

The main session runs the finder and rewriter on the sample files, with no build. It records the count of spots, the count skipped and the reasons, and checks that M2's byte-for-byte round trip holds on every sample file. Settles P17a.

Calls: `py -I tools/mutate/rewrite.py` (never tried)

### LS1: build the sample with every switch in

The main session builds `hydra_tests` with the `mutation` preset on the rewritten sample. It records the build time, each top-up round and its time, and the count of spots dropped. Stop rule 1 applies. Settles P17b. Does not start before question 2 is answered.

Calls: `build_cpp.ps1 -Preset mutation -Target hydra_tests` (never tried)

### LS2: the mapping run, every switch off

Runs the switched `hydra_tests.exe` once with `HYDRA_MUTANT` unset and the listener on, and the normal build's `hydra_tests.exe` once, and records both run times. Stop rule 2 applies. Settles P18.

Calls: `hydra_tests.exe` of the normal build (seen: the main session's own check at every merge), `hydra_tests.exe` of the mutation build with `HYDRA_MUTANT_OUT` set (never tried)

### LS3: one bug per fresh process, one at a time

Runs `run.py` on the sample with `--processes 1`. Records seconds per bug and keeps every verdict. This run is also M3's test. Stop rule 3 starts here.

Calls: `py -I tools/mutate/run.py --processes 1` (never tried)

### LS3b: the same, side by side

Runs `run.py` on the same sample with `--processes N`, where N is the number of physical cores Windows reports, read at run time with `(Get-CimInstance Win32_Processor).NumberOfCores`. It compares every verdict with LS3 and records seconds per bug. Stop rules 3 and 4 apply. Settles P19.

Calls: `py -I tools/mutate/run.py --processes N` (seen: LS3's passing run, same sample, only the process count changes)

### LS4: many bugs inside one process, as a comparison

The brief asks to measure this before choosing. `--one-process` runs the same sample with one long-lived process that changes the live bug between tests. If every verdict matches LS3 and it is faster, the main session reports both numbers to the user. The default stays a fresh process per bug unless the verdicts match exactly. Settles P20.

Calls: `py -I tools/mutate/run.py --one-process` (never tried)

### LS5: the Python half on one hooks module

Runs `py_mutate.py` on one module of the gates folder and its own test file, chosen as the first in name order so nobody picks it. Records seconds per bug. Settles P21.

Calls: `py -I tools/mutate/py_mutate.py` (never tried)

## Out of scope

These need the user's yes to stay out.

- The UI tests (`hydra_uitest`). They drive a window and the research report marks them the likeliest to be flaky. First version covers `hydra_tests` and the Python suites.
- A whole-codebase run. The brief puts it after the first use, with its own go.
- Skipping "uninteresting" lines such as logging (Google's arid nodes). Choosing them is a judgment, and the brief keeps judgment out of which bugs are planted.
- Deciding whether a survivor is an "equivalent" bug, one that changes nothing a user could see. No tool can decide that; the report lists survivors and says so.
- Templates, `constexpr` code, macros and anything else in the skip list. They are listed, not planted.
- Running on CI.

## Questions

1. May task M8 delete `tools/mutation_probe.ps1` and its self-test? It is the one-build-per-bug tool, built under your decision D82. The new tool replaces what it does. If you say no, it stays and reads its rules from the new tool.
### The answers (session 88ea48bb, 2026-11-15)

The user answered in question boxes: the plan, "Yes, go ahead"; question 2, "Stop on first fail", so there are no top-up rounds and the first failed build stops the plan; leaving out the UI tests, "Leave them out"; question 1, after asking what the old probe was, when it was made, who made it and how it was presented, "yes delete". So M8 runs, and it also removes the probe's self-test step from `.github/workflows/ci.yml` and the probe mention in `tools/test_build_pass.ps1`.

### The answers (session 872cc6f3, 2026-11-16)

Three answers change the live steps.

1. A survivor is not run again. The user answered "Yes, drop it" to dropping the whole-suite confirmation run, so a bug's verdict comes from its own filtered run alone.
2. There is no LS3b and no rerun of the sample. The user answered "Yes, re-score": the fixed runner scores last session's two one-at-a-time runs again from their files. That re-scoring found the two runs disagree on bugs 7, 37 and 38. Bug 7 makes `relpath` read past the end of a string, and bugs 37 and 38 change how the folder walk's threads share folders, so whether a test catches them depends on memory contents or thread timing.
3. Stop rule 4 becomes a line in the report, not a stop. Asked how the tool treats a bug that can be caught only by luck, the user chose "One run, say so". Each verdict says it comes from one run ("caught in its run", "survived its run"), and the report says that a bug in threaded code or in a read past the end of memory may land either way.

2. The plan builds once. But if the compiler refuses a few of the planted bugs, the tool drops those and rebuilds only the changed files, up to three repeat builds in all. You said "one build per test is a massive waste of time. if we're doing this it's one build and run to test everything". Do you allow up to three repeat builds? If you say no, the first build that fails stops the plan, I report which planted bugs the compiler refused, and you decide.

### The answers (session 60c3a576, 2026-11-16)

One answer changes the time limit. Each bug runs only its own test files, which take 3 to 30 seconds clean, but every bug run had the whole suite's limit of about 130 seconds. Asked what a bug run's limit should be, the user answered "own time plus 5s". So a bug run may take the clean time of the tests it runs, plus 5 seconds, and then counts as timed out. The mapping run records each test's own time for this.

A second answer keeps the runs side by side. The clean times are measured with one process alone, and on the 81 bugs of `src/app/analysis.cpp` and `src/app/allocator.cpp` the bug runs that finished took a median 14% and at most 62% longer with 8 side by side (6% and 30% with 4). Bug 14 is caught in 31.9 s alone but timed out at its limit of about 34 s with 4 or 8 side by side, so a long run can pass its limit only because of the load, and a survivor can be counted as caught that way. Asked how full runs should handle this, the user chose "Keep 8, accept it". The default stays one process per physical core, 8 on this machine, and `docs/development.md` says the limit.

## Promise map

| Promise | Plan section | Task | The check that proves it |
|---|---|---|---|
| One build holds every bug; no rebuild between bugs | Short version; step 4; M3 | M3, LS3 | LS3's log shows N bug runs and no build; M3's reviewer reads that `run.py` holds no build call |
| Top-up rebuilds are capped at three rounds, and only with the user's yes | Step 4; stop rule 1; question 2 | M3, LS1 | M3: a fourth round exits with no report; LS1 records each round and does not start before question 2 is answered |
| With every switch off the code behaves as today | Short version; M2; M5 | M2, M5, LS2 | M2's byte-for-byte round trip on real files; LS2 fails on any test that passes in the normal build |
| The normal build never carries a switch | Where the switches live; M4; M5 | M4, M5 | Reviewer reads that every added line is inside `if(HYDRA_MUTATION)` or `#ifdef HYDRA_MUTATION`; the LTCG refusal in M4 |
| Rewritten files are copies, never under `src/` | Where the switches live; M2 | M2 | M2's test fails on an output path inside the source tree |
| No agent picks a bug | Short version; step 1; F1 | M1, F1 | M1: kinds are one table, same list on two reads; F1: sample files come from the mapping run |
| No agent judges a verdict | Short version; step 6; F1 | M5, F1 | Verdicts are the listener's lines from doctest; F1's table holds no agent column |
| A spot that cannot be bounded is skipped and listed, never guessed | Step 3; M1 | M1 | One test per skip rule on a real line; every skip carries a reason |
| Bugs go only into code, never comments or strings | Step 1; M1 | M1 | Every spot offset lies in code by the mask |
| Only the tests that reach a bug are run for it | Step 6; M3 | M3, LS3 | LS3's logs show the filter per bug; a red test that does not reach its bug is an error |
| A test red with no bug takes no further part | Step 5; M3 | M3 | Runner refuses to start bug runs on an unlisted red test |
| The report lists per bug and per test, zero-catch tests first, plus unreached, survivors, skipped, timed out | Step 7; M3b | M3b | `test_report.py` on LS3's real results; counts agree |
| The report states its own limits | Step 7 | M3b | `test_report.py` checks the opening lines are present |
| A stuck run is stopped at its own clean time plus 5 s | Step 6; M3 | M3 | Reviewer reads that the rule has one home, `test_timeout_seconds`; `test_run.py` checks the sum and the limit; M8 removed the old home |
| The tool's own tests use real code, no fakes, no typed counts | M1, M2, M3b, M6 | M1, M2, M3b, M6 | Reviewer reads each test file against the rulings in P11 |
| The Python half never edits the real folders | M6 | M6 | Modification-time test |
| The Python half uses the same kinds, mapping run and per-bug runs | M6 | M6, LS5 | M6's `ast` round trip; LS5's result folder |
| Executors never build; the main session builds one at a time | The tasks; live steps | all | Each task names the tests an executor may run; builds appear only in LS steps |
| The plan stops on the four stop rules | What stops the plan | LS1, LS2, LS3, LS3b | Each step names its rule and its record |
| The user sees projected hours before any wider run | What stops the plan | F1 | The main session's go request quotes LS3's numbers |
| Fresh process per bug stays the default unless one-process verdicts match exactly | LS4 | LS4 | LS4 compares every verdict |
| Each rule keeps one home (kinds, time limit) | What exists already; M8 | M1, M3, M8 | `git grep` for the kinds table and the time rule finds one home after M8 |
| First-use lines are copied, not judged | F1 | F1 | Runner names no test for a line outside every doctest test case, and marks it so |
| Verdicts do not move between runs or process counts | What stops the plan, rule 4 | LS3, LS3b | LS3b compares every verdict with LS3 |
| Docs name only what exists | M7 | M7 | The docs check |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The user wants one build and one run, not a build per bug. | ruling | The user's message as quoted in the brief, `mutation-plan-brief.md` line 12 | checked 2026-11-15; the reviewer's brief confirms the quotes are the user's own words typed in this session |
| P2: An existing probe plants one bug at a time and rebuilds per bug. | code | `tools/mutation_probe.ps1` (removed; `git show ed0aa857:tools/mutation_probe.ps1`) lines 319 to 336 | checked 2026-11-15 |
| P3: The probe owns a list of bug kinds: comparison flips, `&&` to `\|\|`, `+ 1`, returned bool. | code | `tools/mutation_probe.ps1` (removed; `git show ed0aa857:tools/mutation_probe.ps1`) lines 122 to 145 | checked 2026-11-15 |
| P4: The probe's time limit was baseline times 1.25 plus 4 s, decision D82 item 3. For this tool the user replaced it with "own time plus 5s" (session 60c3a576). | code | `tools/mutation_probe.ps1` (removed; `git show ed0aa857:tools/mutation_probe.ps1`) lines 176 to 183; `docs/audit/2026-10-03-fix-decisions.md` line 413 | checked 2026-11-15 |
| P5: `hydra_precheck --code-mask <file>` blanks comments and strings, keeps length, and reads C++ and Python. | code | `tools/precheck/main.cpp` lines 7 and 92; `tools/precheck/code_mask.h` lines 1 to 20 | checked 2026-11-15 |
| P6: The repo's lexer is codelex in `tests/code_lexer.h`, with Cpp and Python among its languages. | code | `tests/code_lexer.h` lines 19 to 22 | checked 2026-11-15 |
| P7: T7 put every plant behind one environment variable in one build, written as `t7_bug(id) ? bug : original`. | code | `git show claude/test-audit-t7:src/core/t7_bug.h`; `git grep t7_bug claude/test-audit-t7` (for example `src/app/analysis.cpp` line 778) | checked 2026-11-15 |
| P8: In T7, plants in `src/` made two text-reading tests fail, and three plants needed a compile-time constant. | code | `git show claude/test-audit-records:docs/audit/2026-11-14-test-audit/mechanical/planted.md`, "How it ran" | checked 2026-11-15, the "How it ran" paragraphs; that this plan avoids both traps is checked by LS2 |
| P9: doctest 2.4.11 has a listener interface with test start and failed-check callbacks, and a source-file filter. | code | `third_party/doctest/doctest.h` lines 2064, 2079, 2951, 6614; `third_party/doctest/VERSION.txt` | checked 2026-11-15 |
| P10: `tests/test_main.cpp` defines its own `main` for doctest. | code | `tests/test_main.cpp` lines 4 and 44 | checked 2026-11-15 |
| P11: The user's test rulings: no fakes, no typed constants, test the behaviour directly, use the test corpus. | ruling | `docs/agents/rulings/tests-and-runs.md`, `## Confirmed`, the five lines dated 2026-11-15 | checked 2026-11-15 |
| P12: Hydra builds as C++17, so an `inline` variable in a header is allowed. | code | `CMakeLists.txt` line 30 | checked 2026-11-15 |
| P13: `CMakeLists.txt` already has test-only options and `CMakePresets.json` test-only presets. | code | `CMakeLists.txt` lines 95, 108, 133; `CMakePresets.json` preset names `asan`, `coverage`, `fuzz` | checked 2026-11-15 |
| P14: `build_cpp.ps1 -Preset X` takes the build folder from the configure preset's `binaryDir` (or a parent's), takes the configuration from a build preset of the same name and throws if there is none, configures only when the folder has no `CMakeCache.txt` (or with `-Configure`), and builds with `cmake --build --preset X`. So `mutation` needs both a configure and a build preset, and the new folder needs a `.gitignore` line. | code | `tools/cmake_presets.ps1` lines 12 to 46; `build_cpp.ps1` lines 45, 46, 80, 81, 98; `CMakePresets.json` `buildPresets`; `.gitignore` lines 22 to 26 and 45 | checked 2026-11-15 |
| P15: Agents never build or run `hydra_tests.exe`; they may run `hydra_precheck.exe` from main. | code | `docs/agents/brief-preamble.md`, "Tests" and "Builds" | checked 2026-11-15 |
| P16: The eight re-read files hold 62 rows whose verdict cell starts "does not hold" or "DNH" or contains "cannot tell": A3 8, A9 8, A30 10, C7 8, WN3 5, WN4a 5, WN4b 16, WN8 2. Each row names a line, not a doctest test name. | code | `git show claude/test-audit-t8-questions-handoff:docs/audit/2026-11-14-test-audit/mechanical/reread/A3.md` and the seven others in that folder | checked 2026-11-15, counted by script over each file's verdict column |
| P17a: The finder's and rewriter's text rules bound real Hydra expressions so the round trip holds. | unchecked | `tools/mutate/rewrite.py` does not exist yet | LS0, blocks M3b, F1 |
| P17b: MSVC builds the wrapped copies. | unchecked | `tools/mutate/rewrite.py` does not exist yet; T7 built only `t7_bug(...) ? a : b` forms | LS1, blocks M3b, F1 |
| P18: The switched build with every switch off passes the same tests as the normal build, at a usable speed. | unchecked | `reports/Mutation testing tools for Hydra.md` reports Vercammen's 0.81x case only | LS2, blocks M3b, F1 |
| P19: Several `hydra_tests.exe` processes can run side by side without changing each other's results. | unchecked | searched `tests/test_main.cpp` for a lock or shared file and read only lines 40 to 75 | LS3b, blocks M3b, F1 |
| P20: Hydra keeps state between tests, so many bugs in one process may give different verdicts. | unchecked | `mutation-plan-brief.md` line 23 states it; no code was read for it | LS4, blocks nothing |
| P21: The Python tests of `tools/` and the gates folder run against a copied folder. | unchecked | `reports/Mutation testing tools for Hydra.md` says collection is unverified | LS5, blocks nothing |
| P22: Schemata with test selection ran 2x to 30x faster than rebuilding, and cannot switch templates or constant expressions. | unchecked | `reports/Mutation testing tools for Hydra.md` cites Vercammen et al. 2022; the paper was not opened | LS3, blocks nothing (the plan's own stop rule 3 measures it on Hydra) |
| P23: The `ship` preset turns `HYDRA_LTCG` on, so a mutation build must refuse it. | code | `CMakePresets.json` lines 20 to 27; `CMakeLists.txt` line 47 | checked 2026-11-15 |
| P24: `py` is the project's Python for scripts. | code | `docs/agents/this-machine.md` lines 5 to 9 | checked 2026-11-15 |
