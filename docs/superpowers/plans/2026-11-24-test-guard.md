# Test guard: stopping new bad tests

Written 2026-11-24 by a planning agent, for the user's question: "how do i encourage claude to write good, meaningful tests? we had a whole audit around bad tests but that only cleaned up the existing tests. it doesnt stop new bad tests from being written".

## The short version

The 2026-11-14 test audit found five kinds of bad test. The pages agents read today name only one of them (the snapshot). So an agent writing a test has never been told what a fake stand-in, a constant pin or an echo is. This plan fixes that in four layers.

1. **The rule.** The test rubric names all five kinds, each with a real bad test quoted beside it. It catches nothing by itself; it is what the other layers point at.
2. **The brief.** The preamble every code-writing agent reads gets five short lines, one per kind, and a pointer to the rubric. This stops a bad test before it is written.
3. **The reviewer.** The reviewer's test step names the kinds and tells the reviewer what to write for each new test. This is the only layer that catches a fake stand-in reliably, and the only one that catches a wrong claim.
4. **The machine checks.** The precheck lists likely constant pins, echoes and fakes on the changed lines, so the reviewer starts from them. A standing scan fails a corpus total. A mutation gate plants small bugs in the real code and refuses a new or changed test that catches none of the bugs it reaches.

No layer catches everything. This table says which layer catches which kind. "Lists" means the tool prints a candidate line and a person judges it.

| Fault kind | Brief | Reviewer | Precheck | Standing scan | Mutation gate |
|---|---|---|---|---|---|
| Fake stand-in (X9) | names it | catches | lists some, by name only | no | no |
| Constant pin (X10) | names it | catches | lists the line-level shape | no | catches some |
| Echo (X11) | names it | catches | lists some | no | catches |
| Cited non-code (X12) | names it | catches | no | no | no |
| Snapshot | names it | catches | no | catches corpus totals | no |
| Wrong claim (rubric questions 1 to 3) | no | catches | no | no | no |

Two limits are worth saying plainly. A test built on a fake can still catch a planted bug, so the mutation gate passes it. And a test that pins the wrong behaviour catches planted bugs very well, so the gate passes that too. Only a reader catches those two.

## What the plan does not need

This plan changes docs and developer tools. It does not change what Hydra decides or shows, so it needs no Owner map (premise P12). It has no live step, because nothing here drives Clone Hero. It proposes no change to the hooks in `C:\Users\Patrick\.claude\gates`; question 1 says where one would be needed.

## Existing owners (agent rule 15)

Each new check goes into the place that already answers its question.

"Which test shapes does the precheck flag?" is answered by check 2 of `hydra_precheck` (`tools/precheck/check_2.cpp`), which prints kind B lines for the reviewer. The new line-level checks are new entries there, not a new check and not a new tool.

"Which rules does the whole repo get scanned for?" is answered by the single-owner scan under `tests/single_owner/`. The rubric already says the corpus-totals scan belongs there as its own case beside `special.cpp`, because a scan row is one pattern on one line and cannot see a loop (premise P6).

"Did a test catch a planted bug?" is answered by `bug_result` in `tools/mutate/run.py`, and the per-test count is `tests.txt` from `tools/mutate/report.py`. The user chose that count as the one "caught" count on 2026-11-16 (premise P10). The gate reads that count. It never works out a verdict of its own.

One existing owner is wrong today and gets fixed first. Check 2 prints advice with each line it flags. Two of those advice texts tell the author to do what the user ruled against: "a test pins literals from one run; it never computes the expected value" and "the test restates the module's formula; pin the value" (premise P5). The ruling says "tests should not be pinned to typed constants. they should be pinned to the behavior of the code." (premise P2). Task S1 rewrites the advice to point at the rubric.

## Prior art

This section is background. No task depends on it.

Google runs mutation testing at code review and plants bugs only on changed, covered lines. Per changelist, the median number of mutants is 820 for the plain method and 7 once one-per-line and arid-node selection are applied ([Petrovic et al.](https://ar5iv.labs.arxiv.org/html/2102.11378), read 2026-11-24). Google's findings "do not need to be resolved by the author before submission, unless a human reviewer marks them as mandatory" (same paper), so Google does not block on a surviving mutant by default; this plan's gate does, which is a choice this plan makes. One study found that 81% of the mutants relevant to a commit sit outside the changed code ([arXiv 2112.14566](https://arxiv.org/abs/2112.14566), abstract, read 2026-11-24). That is why this gate picks bugs by what the changed test reaches, not by which source lines changed. The R packages `mutator` and `muttest` show the plain CI form: the job exits non-zero below a score and that blocks the pull request ([mutator, continuous integration](https://ftp.fau.de/cran/web/packages/mutator/vignettes/continuous-integration.html)). The untracked report `reports/Mutation testing tools for Hydra.md` in the main checkout adds one warning this plan keeps: a test that kills nothing "has shown no way to fail against that mutant set", which is evidence, not proof.

## Plan run

Slug: test-guard
Repo: C:\Users\Patrick\Downloads\Hydra\hydra-test

| Phase | Builds |
|---|---|
| docs | 0 |
| scans | 1 |
| gate | 0 |

## The tasks

| Task | Branch | Phase | Owned files |
|---|---|---|---|
| T1 | `claude/test-guard-rubric` | docs | `docs/agents/test-rubric.md`, `docs/agents/test-rubric-examples.md` |
| T2 | `claude/test-guard-preamble` | docs | `docs/agents/brief-preamble.md` |
| T3 | `claude/test-guard-reviewer` | docs | `docs/agents/derive-once-review.md` |
| S1 | `claude/test-guard-precheck` | scans | `tools/precheck/check_2.cpp`, `tests/test_precheck_check24.cpp` |
| S2 | `claude/test-guard-totals` | scans | `tests/single_owner/corpus_totals.cpp`, `CMakeLists.txt` |
| G1 | `claude/test-guard-run-filter` | gate | `tools/mutate/run.py`, `tools/mutate/test_run.py` |
| G2 | `claude/test-guard-gate` | gate | `tools/mutate/gate.py`, `tools/mutate/test_gate.py` |
| G3 | `claude/test-guard-gate-docs` | gate | `docs/development.md`, `docs/agents/derive-once-review.md` |

The docs phase lands first and costs no build. The scans phase costs one build. The gate phase changes only Python tools and two docs pages, so it needs no compile and has a build count of 0 (`docs/agents/plan-runs.md`: 0 is for a phase that needs no compiling). The mutation build is asked for like any other build, and the build gate never refuses it (see question 1).

### T1: The rubric names every fault kind

The rubric keeps its 12 questions. The four kinds it does not name are folded into the questions they belong to, so there is still one list.

Question 5 already lists what never gives a number its reason: a snapshot, a copy and a saved expected-output file. T1 adds the constant pin and the cited non-code there, each with the user's ruling quoted from `docs/agents/rulings/tests-and-runs.md` (premise P2). Question 8 asks whether a check can fail; T1 adds the echo there, as the plainest check that cannot fail. Question 9 lists stand-ins for the real risk; T1 adds the fake stand-in there, with the ruling "tests should never be based on anything fake". Question 9 also carries the audit's own boundary: a chart, ini or malformed file the test writes and the real code reads is input, not a fake (premise P3).

The examples page gets one bad example per new kind, quoted line by line from a real test. The audit's bad tests have since been fixed or deleted, so T1 quotes each from the commit that still holds it and says so, as the page already does for its filled-in record. Three starting points come from the audit's count files at b2287e56: an echo at the record marked "SV :213" in `counts/T5-KR1.md`, a constant pin at `tests/test_precheck_pick.cpp:315`, and a fake stand-in at "RC:323" in `counts/T5-IR6.md`. T1 opens each test before quoting it, and finds its own cited-non-code example in the same count files (premises P1 and P4).

`docs/agents/audit-method.md` needs no change. Its one line on test faults, at line 5, already points at the rubric (premise P13).

Acceptance criteria:

- Each of the five kinds (fake stand-in, constant pin, echo, cited non-code, snapshot) has a bold name in exactly one question of the rubric. Proof: the reviewer greps the rubric for each name and finds its one defining line.
- Each of the four new kinds has an example on the examples page whose quoted lines the reviewer finds, character for character, with `git show <commit>:<test file>`.
- Each ruling quoted matches its line under `## Confirmed` exactly. Proof: the reviewer opens `docs/agents/rulings/tests-and-runs.md` lines 10 to 13.
- The rubric states the limit table from this plan's short version, or points at one home for it, so a reader knows mutation does not catch fakes or wrong claims.
- The rubric's "Running it as an audit" section still describes the corpus-totals scan, and now says S2 built it once S2 has landed. T1 writes the sentence so it is true before and after S2: it names the file and says "when it exists".

### T2: The preamble says how to write a test

The "If you write code" section gets one short paragraph, under the paragraph that already says a test calls the production function. It has five sentences, one per kind, each in plain words, and one pointer to the rubric. It adds no detail the rubric holds.

Acceptance criteria:

- The paragraph names all five kinds with the same names T1 uses. Proof: the reviewer greps both pages for each name.
- The paragraph is five sentences, one per kind, plus one pointer to the rubric, and no more. The rubric is the one home of the detail; the reviewer checks no definition is restated beyond one sentence.
- T2's branch is written after T1's names are fixed. T2's brief quotes the five names from T1's merged commit on the plan branch.

### T3: The reviewer's step 5 names the kinds

Step 5 of "How to review" gets two additions. It names the five kinds and points at the rubric's questions for them. And for each new or changed test, the reviewer writes one line saying which real input the test feeds the real code, so a fake has to be written down to pass.

The reviewer does not read mutation results for the change it reviews. The change is reviewed uncompiled, and the gate runs after the phase's one build, so no result exists yet (premise P8). The existing sentence stays: "For each new test with no red line in your brief, say in your review what would make it fail." G3 later tells the fix reviewer, who does have a result, to read it.

Acceptance criteria:

- Step 5 names all five kinds by T1's names. Proof: grep.
- Step 5 says the reviewer starts from the precheck's B lines, as it does today, and does not claim the precheck finds fakes reliably.
- Kind B's one-line definition under "The recurring kinds" still points at the rubric and is not widened into a second definition.

### S1: The precheck lists pins, echoes and named fakes

S1 adds entries to check 2's two lists, `cpp_spellings` and `py_spellings`. Each entry prints a kind B candidate line on an added line of a test file, the same way the four C++ and two Python entries there do today (premise P5).

The constant pin entry matches an assertion that compares a product constant, read by name, with a typed literal. That is the audit's own definition: "Checking that the constant equals a typed number is" a constant pin (premise P3). The named-fake entry matches a test that defines or imports something whose name starts with Fake, Stub or Mock, or uses Python's `unittest.mock`. The echo entry matches only what one statement shows: the same literal typed as the input and as the expected side of one assertion. An echo spread over several statements is left to the reviewer and the gate, and S1's commit body says so.

S1 adds no threshold, length or count. If an entry cannot be written without one, S1 stops and reports it (the preamble's rule on new numbers).

S1 also rewrites the two advice texts that tell authors to pin literals, so each points at the rubric instead.

Acceptance criteria:

- For each new entry, `tests/test_precheck_check24.cpp` holds a case that feeds the real `run_check_2` a test file with the bad shape and requires a B line at that line, and a case with the good shape that requires none. The inputs are test source text the real check reads, which the audit counts as input, not a fake.
- No advice text in `check_2.cpp` contains "pins literals from one run" (line 72) or "pin the value" (line 92). Proof: grep, and the test case that reads the advice from the check's own output.
- Check 2 still prints candidates only. Nothing in S1 makes the precheck exit non-zero for a B line.
- The diff adds no numeric literal to `check_2.cpp` outside a pattern. Proof: the precheck's own kind D lines on S1's range are empty.

### S2: A standing scan for totals over the corpus

S2 writes the scan the rubric describes: it fails when a test adds to a counter inside a loop over `corpus::chart_paths()` and then compares that counter with a literal other than 0 (premise P6). It lives in `tests/single_owner/` as its own case, the way `special.cpp` does, so it runs at every merge with the single-owner scan (premise P9).

Acceptance criteria:

- The scan reads every file under `tests/`. On today's tree it passes. If it finds an existing total, S2 stops and reports the test; it does not add a baseline entry.
- A doctest case feeds the scan's function three source texts: a loop with a counter compared to a typed total (must be flagged), the same compared to 0 (must pass), and a property check inside the loop (must pass).
- The root `CMakeLists.txt` lists the new file next to `tests/single_owner/special.cpp` (the list at lines 772 to 775; there is no `tests/CMakeLists.txt`). Proof: the scans phase build links it and the `*single_owner*` filter runs it.

### G1: The runner can be pointed at named tests

Today `run.py` maps every test and runs every reached bug (premise P7). The gate needs the same run for a few tests only. G1 adds one option that names test files. The mapping run then takes only those files, and the bug runs cover only the switches those tests reach. The rules for a verdict do not change: `bug_result` still decides, and the time limit is still `test_timeout_seconds`.

`run.py` today has the options `--build`, `--processes`, `--out`, `--map-only`, `--rescore` and `--resume`, and none names test files (premise P14). G1 adds the one new option. The file filter itself is the existing `-sf` argument that `filter_arg` already builds.

Acceptance criteria:

- `tools/mutate/test_run.py` has a case showing the mapping command and each bug command carry only the named files. It calls the real command builder; it starts no process and fakes none.
- With the option absent, every command is unchanged. Proof: the existing cases in `test_run.py` pass untouched.
- The docstring's usage line shows the option, and says nothing the code does not do.

### G2: The gate

`gate.py` answers one question: of the tests a change added or changed, which caught no planted bug? It takes a run folder and a list of test names. For a C++ run it reads each test's catches and reaches from the run's own results, through `report.py`'s reader, and prints one line per test.

A test that reaches at least one planted bug and catches none is refused, and the gate exits non-zero. A test that catches at least one passes. A test that reaches no planted bug is printed as "not judged", with the reason, and does not fail the gate: the tool has nothing to say about it, and saying "passed" would be a lie. Tests in `hydra_uitest` are never judged, because the mutation build builds only `hydra_tests` (premise P7).

Which tests a change added or changed has no owner at test level today. The search found two near answers. `hydra_precheck --tests` prints whole test files to run, and `changed_names.cpp` lists changed names such as functions and constants, where a doctest `TEST_CASE(...)` head reads as one function called `TEST_CASE` (premise P15). So the gate takes the changed test files as its input and judges only the tests whose own line, in the run's test list, falls on an added line of `git diff -U0 <range>`. A C++ test id in that list is `file:line name`, and a test runs from its line to the next test's line in the same file. This one small step is the gate's own, and it is the only new owner in the plan.

The Python side reads `py_mutate.py`'s own run folder, not `report.py`, because the two tools write different files. Its `summary.txt` lists every test as "reaches N, caught M", and `bugs.json` holds the catches (premise P16). Its test ids are dotted names with no line, so the Python gate judges every test of a test module the range changed, and says so in its output. `py_mutate.py` already takes `--tests` (test modules) and `--mutate` (source files), so the Python side needs no change to the runner (premise P14). It needs no build (premise P11).

Acceptance criteria:

- The changed-test step is tested on a real `git diff -U0` made in a throwaway git repository the test creates, against a test list written by the real run writers. One case has the added line inside one test and one case has it between two tests; each requires the right test, or none.
- `test_gate.py` runs the gate on a run folder written by the real `run.py` writers from three recorded outcomes (catches some, reaches some and catches none, reaches none) and requires pass, refused and not judged, in that order, and exit codes 0, 1, 0 for the three alone. G2's brief asks the main session for a small real run folder if the writers cannot produce one without a build; it never hand-types one.
- The gate holds no rule about what "caught" means. Proof: the reviewer greps `gate.py` for a verdict word from `run.py` and finds only reads.
- A refused line names the test, the number of bugs it reached, and the path of the report file that lists them.
- The gate never prints "passed" for a test it did not judge.

### G3: The pages say when the gate runs and who runs it

`docs/development.md`, "Mutation testing", gets a short part on the gate: the commands, in order, and that every step is the main session's. `docs/agents/derive-once-review.md`, "For the orchestrator", gets one step after the phase's tests: run the gate on the phase's new and changed tests. A refused test is a failed test. It goes with every other failure to the one fresh executor, and the fresh fix reviewer is given the gate's output for that test (premise P8).

G3 writes the build step as: the main session asks `hydra_build_pass` for a pass with `preset` set to `mutation`, as for any build, and the build gate does not refuse it (`build_rules.py` in `C:\Users\Patrick\.claude\gates\` owns that rule). Waits for: nothing.

Acceptance criteria:

- The orchestrator step names the gate command and says a refusal is handled as a test failure under agent rule 5. It gives no cost figure that M1 has not measured.
- `development.md` says which tests the gate cannot judge, in the same words G2 prints.
- Neither page restates what "caught" means; both point at `bug_result`.

### M1: Measure what the gate costs

No one has measured a gate run. The only figures are from the one full run, and they come from a session card, which is a pointer and not a source (premise P17). So this plan gives no per-change cost.

M1 is the main session's step, after the gate phase lands. It runs the gate once on the scans phase's own new tests (S1's and S2's cases), and reports to the user: the mutation build's wall time, the mapping run's wall time, the number of bugs reached, and the bug runs' wall time. Each figure is read from the build's own printed output and the run folder's `summary.json`. The stop condition is one run: it ends when the run folder's `summary.json` exists, or when the user says stop, and M1 then reports how far it got. M1's mutation build is handed a pass like any other build: the build gate does not refuse it, and `build_rules.py` in `C:\Users\Patrick\.claude\gates\` owns that rule.

Acceptance criteria:

- The report to the user holds the four figures, each with the file it was read from.
- The gate is not described to anyone as cheap, fast or free before M1's figures exist.

## Global constraints

- Agents never build and never run `hydra_tests.exe` (agent rule 14). Every task above hands back uncompiled. The Python tests named in G1 and G2 need no build and are run one file at a time.
- No task adds a threshold, floor or tolerance. A task that needs one stops and reports.
- No task hand-types a result, a run folder or a stand-in program for its own tests. This plan would otherwise add the fault it is about.
- Every fault-kind name is defined once, in the rubric. Other pages and tools point there.

## Questions for the user

**Question 1. May a phase compile a second time for the mutation gate?**

No longer open. The gate this question was about was scrapped on 2026-11-24, and the build gate that replaced it never refuses a mutation build (`build_rules.py` in `C:\Users\Patrick\.claude\gates\` owns that rule). The text below is kept as it was asked.

The real rule today, in your words from 2026-11-07, filed under Confirmed in `docs/agents/rulings/agents-and-workflow.md`: "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." The build gate applies that per plan phase: one build, plus one rebuild only after a new reviewed commit. It counts every recorded build of the phase's branch and does not look at which preset was built (`C:\Users\Patrick\.claude\gates\plan_run_judge.py`, lines 110 to 123 and 182 to 192).

The mutation gate for C++ tests needs its own compile. The planted bugs live in rewritten copies that only the `mutation` preset builds, into `build-mutation\` (premise P7). So after the phase's normal build, a mutation build of the same commit is refused today: "has no new reviewed commit since".

Two real cases show both ends. A phase that changes only Python tests, such as this plan's G1 and G2, needs no compile at all for the gate, because the Python half copies the folder and runs its tests against the copy. A phase that changes one C++ test, such as S1's new cases in `tests/test_precheck_check24.cpp`, needs the full mutation build of `hydra_tests` before the gate can say anything about that one test.

The choices the existing rules allow:

- Allow it: a phase that adds or changes a C++ test may take one extra build, of the `mutation` preset only, on the commit its normal build passed. This needs a change to the build gate in the hooks repo, planned and reviewed there on its own.
- Do not allow it: the gate runs on Python tests in every phase, and on C++ tests only when you type `allow build` for a mutation build. New C++ tests are then guarded by layers 1 to 3 and the precheck, not by planted bugs, between those runs.

You will see: nothing in Hydra changes either way. This only changes how many compiles a phase costs.

I have no measured build time to put beside either choice. M1 measures it.

## Out of scope, for the user to approve

- A hook that refuses a move of `main` when the gate has not run. In this plan the gate is a command the main session runs and a step on the orchestrator's page; nothing enforces it.
- Running the gate on `hydra_uitest` tests. The mutation build does not build that program.
- Re-reading the survivors and the tests that caught nothing in the 2026-11-16 full run. That is the audit's open thread, not this plan's.
- A machine check for cited non-code, or for a wrong claim. No scan can find either honestly; the reviewer owns both.

## Promise map

| Promise | Plan line | Task | The check that proves it |
|---|---|---|---|
| The rubric names every audit fault kind, once each | T1, first criterion | T1 | Reviewer greps the rubric for each of the five names and finds one defining line |
| Each new kind has a real example | T1, second criterion | T1 | Quoted lines found character for character with `git show` |
| Quoted rulings are exact | T1, third criterion | T1 | Reviewer opens the rulings file at lines 10 to 13 |
| The pages say which layer catches which kind, and that mutation misses fakes and wrong claims | T1, fourth criterion | T1 | Reviewer finds the limit table or its one pointer in the rubric |
| The new kinds are folded into the 12 questions, not a parallel list | T1, opening paragraph | T1 | The rubric still has questions 1 to 12 and no new numbered list; reviewer reads the headings |
| The preamble names the kinds and stays short | T2, criteria 1 and 2 | T2 | Grep for the five names; the paragraph is five sentences and one pointer |
| The preamble uses the rubric's names | T2, criterion 3 | T2 | T2's brief quotes the names from T1's merged commit |
| The reviewer's step names the kinds and asks for the real input of each test | T3, criterion 1 and second paragraph | T3 | Grep; reviewer reads step 5 |
| The reviewer is not told the precheck finds fakes reliably | T3, criterion 2 | T3 | Reviewer reads step 5 |
| The reviewer does not read mutation results before the build | T3, third paragraph | T3 | Step 5 holds no sentence asking for a result; G3's fix-reviewer step does |
| The precheck lists constant pins, single-statement echoes and named fakes | S1, criterion 1 | S1 | A red case and a green case per entry in `tests/test_precheck_check24.cpp` |
| The precheck stops advising authors to pin literals | S1, criterion 2 | S1 | Grep for the two old texts finds nothing; a test reads the advice from the check's output |
| The precheck still only lists candidates | S1, criterion 3 | S1 | Reviewer reads `run_check_2`: no exit code change |
| No task adds a threshold | S1, criterion 4; Global constraints | S1, S2, G1, G2 | The precheck's kind D lines on each range are empty |
| A new corpus total fails before it merges | S2, criteria 1 and 2 | S2 | The flagged case fails the scan; the scan runs at every merge with `*single_owner*` |
| An existing total is reported, never baselined | S2, criterion 1 | S2 | S2's diff touches no baseline file |
| The runner can be limited to named tests, with verdict rules unchanged | G1, criteria 1 and 2 | G1 | The new case in `test_run.py`; the old cases pass untouched |
| A test that reaches bugs and catches none is refused | G2, criterion 1 | G2 | `test_gate.py`: refused, exit code 1 |
| A test the tool cannot judge is never called passed | G2, criteria 1 and 4 | G2 | `test_gate.py`: "not judged", exit code 0, and no "passed" line |
| The gate judges only tests the range added or changed | G2, changed-test criterion | G2 | `test_gate.py`: a real `git diff -U0` in a throwaway repo; the added line inside one test picks that test, between two tests picks none |
| The gate works out no verdict of its own | G2, criterion 2 | G2 | Reviewer's grep of `gate.py` |
| The gate's own tests use no hand-typed run folder | G2, criterion 1; Global constraints | G2 | Reviewer reads `test_gate.py`'s setup |
| A refusal is handled as a test failure, by the main session | G3, criterion 1 | G3 | Reviewer reads the orchestrator step |
| No cost figure without a source | G3, criterion 1; M1 | M1 | M1's report names the file behind each figure |
| The gate's build step is an ordinary build pass | G3 | G3 | Reviewer reads the build step for a typed line or a wait |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The pages agents read name the snapshot but not the fake stand-in, the constant pin, the echo or cited non-code as kinds of bad test | code | `docs/agents/test-rubric.md:1-205`, `docs/agents/brief-preamble.md:113-119`, `docs/agents/derive-once-review.md:46` | checked 2026-11-24; a wider grep found "made-up" once in the rubric and once in the preamble and "hand-built" once on the examples page, none defining a kind |
| P2: The user ruled against fake-based tests, typed-constant pins, cited non-code and arbitrary strings | ruling | `docs/agents/rulings/tests-and-runs.md` lines 10 to 13 under Confirmed: "tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation"; "tests should not be pinned to typed constants. they should be pinned to the behavior of the code." | checked 2026-11-24 |
| P3: The audit defined X9 to X12 as this plan words them, and counted a file the test writes and the real code reads as input | unchecked | `docs/audit/2026-11-14-test-audit/briefs/T5-keep-recount.md`, removed from today's tree; the review read it with git show at b2287e56 and found lines 27 to 31 define X9 to X12 and the snapshot, and line 17 holds "Hand-built input is not fake" | T1 |
| P4: The audit read 2,449 tests and found a fault in 1,861: X9 1,452, X10 963, X11 249, X12 135, snapshot 46 | unchecked | `docs/audit/2026-11-14-test-audit/counts/TOTALS.md`, removed from today's tree; the review read it with git show at b2287e56 and the row "Both, mine" holds exactly these figures; an audit's report, not the tests | T1 |
| P5: Check 2 of the precheck prints kind B candidate lines from two lists of line patterns, and two of its advice texts tell authors to pin literals | code | `tools/precheck/checks.cpp:7-13`, `tools/precheck/check_2.cpp:50-96`, `tools/precheck/check_2.cpp:106-152` | checked 2026-11-24 |
| P6: The rubric says the corpus-totals scan cannot be a scan row and lives as its own case beside `special.cpp` | code | `docs/agents/test-rubric.md:203` | checked 2026-11-24 |
| P7: The mutation tool plants every bug in one build of `hydra_tests` under the `mutation` preset, then does one mapping run and one run per reached bug, and every step is the main session's | code | `tools/mutate/run.py:1-53`, `CMakeLists.txt:1024-1039`, `docs/development.md:635-685` | checked 2026-11-24 |
| P8: A change is reviewed uncompiled; the phase builds once after every task is merged; a failure goes to one fresh executor and a fresh reviewer of that fix | code | `docs/agents/plan-runs.md:68-70`, `docs/agents/agent-rules.md:20` | checked 2026-11-24 |
| P9: The single-owner scan runs at every merge to main | code | `docs/agents/agent-rules.md:7` | checked 2026-11-24 |
| P10: The user chose the tests.txt count as the one "caught" count | ruling | `docs/sessions/2026-11-16-899dc0dd.md`: chose "Count 1; fix F1 (Recommended)" to "The 'caught' count: tests.txt (count 1) includes failures the map can't see ..." | checked 2026-11-24 |
| P11: The development page says the Python half needs no build and rewrites only a copy | code | `docs/development.md:724-739` | checked 2026-11-24 |
| P12: A plan that only edits docs, tools or hooks needs no Owner map | code | `docs/agents/owner-map.md:13` | checked 2026-11-24 |
| P13: `docs/agents/audit-method.md` says nothing about fault kinds that the new rubric text would contradict | code | `docs/agents/audit-method.md:1-190` | checked 2026-11-24; read whole; line 5 points an audit of tests at the rubric and no line names a fault kind |
| P14: `run.py` has no option today that limits a run to named test files | code | `tools/mutate/run.py:861-880` | checked 2026-11-24; the options are `--build`, `--processes`, `--out`, `--map-only`, `--rescore`, `--resume`; `py_mutate.py:638-643` already has `--tests` and `--mutate` |
| P15: The precheck does not print changed test names; it prints test files (`--tests`) and changed function or constant names | code | `tools/precheck/main.cpp:4-12`, `tools/precheck/test_pick.h:1-8`, `tools/precheck/changed_names.cpp:176-212` | checked 2026-11-24; no code in `tools/precheck/` handles `TEST_CASE`, so G2 owns the line-to-test step |
| P16: `py_mutate.py` records catches per test, in `summary.txt` and `bugs.json`, with test ids that carry no line | code | `tools/mutate/py_mutate.py:596-626` | checked 2026-11-24 |
| P17: The one full run planted 4,583 bugs in 112 files, tests reached 3,819, and the whole suite's clean time was 100.8 s | unchecked | `docs/sessions/2026-11-16-60c3a576.md`, a session card, which is a pointer | M1 |
| P18: The build gate counts every recorded build of a phase's branch whatever its preset, and allows one build plus one rebuild that needs a new reviewed commit | code | `C:\Users\Patrick\.claude\gates\plan_run_judge.py:110-123`, `C:\Users\Patrick\.claude\gates\plan_run_judge.py:166-193` | checked 2026-11-24 |
| P19: The user ruled "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." | ruling | `docs/agents/rulings/agents-and-workflow.md:10`, under `## Confirmed` (session e3e9069c) | checked 2026-11-24 |
| P20: `allow build` lets one build through only for a session with no open run | code | `docs/agents/plan-runs.md:80` | checked 2026-11-24 |
