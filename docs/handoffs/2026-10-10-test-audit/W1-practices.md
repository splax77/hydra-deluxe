# W1: Test-design standards and what applies to Hydra Deluxe

Task TA-W1. Read-only web research. Used 17 searches and about 25 page fetches.

## Source quality: read this first

- Fetched in full and primary: Beck's testdesiderata.com; Fowler's "Eradicating Non-Determinism" and "Practical Test Pyramid"; Fowler's TestCoverage and "Mocks Aren't Stubs"; the Software Engineering at Google (SWE@Google) chapters 11, 12 and 14 on abseil.io; the Hypothesis article; the ArchUnit user guide; the tsDetect smell list at testsmells.org; Google's research abstracts on mutation testing and coverage.
- Fetched but thin: the Google Testing Blog posts on change-detector tests and on flaky tests came back as comments only, with no post body. I use them only for what the comments and the SWE@Google book say. The "Just Say No to More End-to-End Tests" page was also mostly comments.
- Not fetched. These sites refused or returned 404: xunitpatterns.com (Meszaros), enterprisecraftsmanship.com (Khorikov), the Luo et al. PDF (unreadable binary, and no PDF reader on this machine). I cite Meszaros, Khorikov, Luo, van Deursen and Feathers only through search-result summaries. Those are marked "secondary" below. Treat them as pointers, not quotes.
- Not covered at all: ISTQB/ISO 29119 (skipped, as allowed), and the exact flaky percentages from Luo et al. (the table was not readable).
- No source I found says anything specific about C++, doctest or Dear ImGui Test Engine beyond its README summary. The C++ spotting rules in the smell catalogue are my translation of the language-neutral definitions.

---

## 1. Checklist for auditing a test file

Each item: the check, why it matters, source.

**A. What a test claims**

1. Each test checks one behaviour, and its name says the behaviour and outcome. Why: a failure then points at one cause; a name needing "and" signals two behaviours. https://abseil.io/resources/swe-book/html/ch12.html
2. Tests are written per behaviour, not per function. Why: one test per method forces unrelated checks together and splits one behaviour across tests. https://abseil.io/resources/swe-book/html/ch12.html
3. The test goes through the public interface, the way a user or caller would. Why: it then fails only when a user would also be hurt, and survives refactors. https://abseil.io/resources/swe-book/html/ch12.html and https://martinfowler.com/articles/practical-test-pyramid.html
4. The test checks the resulting state or output, not which internal calls were made. Why: call-checking tests break on harmless refactors (Fowler's main objection to mock-style tests). https://martinfowler.com/articles/mocksArentStubs.html
5. If the test would still pass after the behaviour it names is broken, it is rejected. Why: Beck calls this "Behavioral" (tests sensitive to behaviour changes) and "Predictive" (all pass means fit for production). https://testdesiderata.com/
6. If the test would fail after a pure refactor that keeps behaviour, it is rejected. Why: Beck's "Structure-insensitive" property. https://testdesiderata.com/
7. Test doubles are used only where a real collaborator is slow, nondeterministic or out of process. Why: SWE@Google prefers real objects when they are fast and deterministic; heavy mocking makes brittle tests. https://abseil.io/resources/swe-book/html/ch12.html (Khorikov agrees: mock only unmanaged dependencies; secondary, via search summary of his book, https://www.oreilly.com/library/view/unit-testing-principles/9781617296277/Text/kindle_split_021.html)

**B. How expected values are written**

8. Expected values are literals written in the test, not recomputed with the production formula. Why: a test that recomputes the answer repeats the production bug and proves nothing. SWE@Google says do not put logic in tests; Rainsberger-style pinned outputs are the same idea. https://abseil.io/resources/swe-book/html/ch12.html
9. A test has no loops, branches or string-building around its assertions. Why: logic in a test can hide a bug in the test itself, and an `if` can skip the check. https://abseil.io/resources/swe-book/html/ch12.html and https://testsmells.org/pages/testsmells.html (Conditional Test Logic)
10. Where a literal is not obvious (a magic number), a nearby name or a one-line comment says where it came from. Why: tsDetect flags unexplained numeric literals; this is the one place where "literal, not formula" and "explain your numbers" must be balanced. https://testsmells.org/pages/testsmells.html (Magic Number Test)
11. A pinned value taken from one run is labelled as such, and the test says what a deliberate change to it means. Why: a golden value proves "same as before", not "correct" (see section 3, golden tests). https://docs.deno.com/runtime/test/snapshots/ (via search summary)

**C. Readability and shared code**

12. A reader can see the setup, the action and the expected result in the test body, in that order, without opening other files. Why: SWE@Google calls this "complete and concise" and wants given/when/then kept apart. https://abseil.io/resources/swe-book/html/ch12.html
13. Shared helpers have sensible defaults, and any value a test depends on is stated in the test. Why: this is "DAMP, not DRY": some duplication is fine if it makes the test clearer. https://abseil.io/resources/swe-book/html/ch12.html
14. Fixtures create only what the test uses. Why: General Fixture / Excessive Setup slows tests and hides what matters. https://testsmells.org/pages/testsmells.html (General Fixture)
15. Failure messages say expected, actual and the key inputs. Why: Beck's "Specific": when a test fails the cause should be obvious. https://abseil.io/resources/swe-book/html/ch12.html and https://testdesiderata.com/

**D. Determinism and isolation**

16. No fixed sleeps to wait for something. Use a completion signal, or poll with a generous timeout and a clear timeout error. Why: sleeps are too long or occasionally too short; this is the leading flaky cause in the Luo et al. study (secondary). https://www.martinfowler.com/articles/nonDeterminism.html and https://abseil.io/resources/swe-book/html/ch14.html
17. Time, randomness and the clock come in through a seam the test controls (a fixed seed, a frozen clock). Why: unfixed inputs make results vary run to run. https://www.martinfowler.com/articles/nonDeterminism.html
18. Each test builds its own starting state, and the result does not depend on test order or on files left by another test. Why: shared state is Fowler's first cause of intermittent failure; Beck's "Isolated". https://www.martinfowler.com/articles/nonDeterminism.html and https://testdesiderata.com/
19. Teardown failures fail loudly. Why: a swallowed cleanup error poisons later tests. https://www.martinfowler.com/articles/nonDeterminism.html
20. Tests that read files, databases or the real machine say so in the test, and use checked-in or generated fixtures rather than whatever is on the developer's disk. Why: "Mystery Guest" and "Resource Optimism" smells; also Hydra's own rule against touching the user's library. https://testsmells.org/pages/testsmells.html
21. A quarantined or skipped test has an owner, a reason and an expiry. Why: Fowler says quarantine is only temporary and the list needs a cap; a disabled test is a smell. https://www.martinfowler.com/articles/nonDeterminism.html and https://testsmells.org/pages/testsmells.html (Ignored Test)

**E. Tests that read source text (architecture / "source scan" tests)**

22. A source-scan rule fails if it finds nothing to check. Why: ArchUnit fails a rule whose subject set is empty by default, so renaming a folder cannot make the rule pass silently. https://www.archunit.org/userguide/html/000_Index.html
23. Each rule states why it exists, and its failure message names the rule and each offending file and line. Why: ArchUnit's `because(...)` and its failure format; a bare "false" teaches nobody. https://www.archunit.org/userguide/html/000_Index.html
24. Allow-lists and ignore patterns are as narrow as possible, and the list of known exceptions can only shrink in CI. Why: a broad ignore pattern hides real violations; ArchUnit's freeze store is locked from silent updates in CI. https://www.archunit.org/userguide/html/000_Index.html
25. The scan matches code structure where it can (what is called, what is included) and does not just grep for names. Why: ArchUnit's own warning that patterns match the wrong thing (package vs class name); a grep that misses X does not prove X is absent. https://www.archunit.org/userguide/html/000_Index.html

**F. Placement and cost**

26. A behaviour is checked at the lowest level that can see it. If a high-level test fails and no low-level one does, add a low-level one; delete high-level tests that duplicate lower ones. Why: Fowler's "push tests down"; cheaper, faster, easier to locate. https://martinfowler.com/articles/practical-test-pyramid.html
27. Large or whole-system tests are few, each as small as its needed fidelity allows, and each has a named owner. Why: SWE@Google notes larger tests are slower, flakier and rot without owners. https://abseil.io/resources/swe-book/html/ch14.html
28. Golden/diff tests keep the baseline small enough to review, strip noise (timestamps, ordering, ids) and have a documented, deliberate way to regenerate. Why: reviewers stop reading large diffs; noise creates false diffs. https://abseil.io/resources/swe-book/html/ch14.html and https://blog.thecodewhisperer.com/permalink/how-not-to-write-golden-master-tests
29. GUI tests find controls by label or role, assert on state that the user can read, and use screenshots only where appearance is the thing under test. Why: image matching breaks on fonts, layout and colour changes. https://www.ministryoftesting.com/insights/how-to-write-visual-ui-automation-tests-using-graphics-instead-of-complex-locator-strings and https://martinfowler.com/articles/practical-test-pyramid.html
30. The test file is justified by a named risk. If you cannot say what bug it would catch that no other test catches, it is a candidate for deletion. Why: Khorikov's four pillars (regression protection, refactor resistance, fast feedback, maintainability) say a test that scores near zero on one is not worth keeping; secondary source: https://notesbylex.com/four-pillars-of-good-unit-tests

---

## 2. Test-smell catalogue

Names are from the tsDetect list (Peruma et al., ESEC/FSE 2020, https://testsmells.org/pages/testsmells.html), Meszaros (secondary) and the open catalogue at https://test-smell-catalog.readthedocs.io/en/latest/ (maintained by the Easy Lab at UFAL; built from a literature review of 127 sources). The origin of the idea is van Deursen et al., "Refactoring Test Code", XP 2001 (https://www.simula.no/research/refactoring-test-code; I could confirm the paper exists but not its smell list). "How to spot it" is my C++/doctest translation.

| Smell | What it means | How to spot it in C++/doctest |
|---|---|---|
| Assertion Roulette | Many assertions with no message; hard to tell which one failed. https://testsmells.org/pages/testsmells.html | A `TEST_CASE` with 10+ bare `CHECK(...)` lines, no `CAPTURE`, no `INFO`, no message string. |
| Eager Test | One test calls many methods of the thing under test. https://testsmells.org/pages/testsmells.html | A test name like "works" or "basic"; one test covering parse, analyse and report. |
| Lazy Test | Several tests call the same method for the same behaviour. https://testsmells.org/pages/testsmells.html | Near-identical `TEST_CASE`s differing by one constant; candidate for `SUBCASE` or a table. |
| Conditional Test Logic | `if`, `switch` or loops in the test body. https://testsmells.org/pages/testsmells.html | `for` or `if` around `CHECK`; a check that is skipped when the condition is false. |
| Magic Number Test | Unexplained numeric literal in an assertion. https://testsmells.org/pages/testsmells.html | `CHECK(x == 4172)` with no note on origin. Balance against the "literal not formula" rule: keep the literal, add a source comment. |
| Sleepy Test | Test sleeps to wait. https://testsmells.org/pages/testsmells.html | `std::this_thread::sleep_for`, `Sleep(`, `ImGui` frame-count waits tuned by trial. |
| Mystery Guest | Test depends on an external file or database the reader cannot see. https://testsmells.org/pages/testsmells.html | Reads `C:\...`, the user's library, `hydra.db` in the home folder, or a fixture path built far from the test. |
| Resource Optimism | Assumes a file exists without checking. https://testsmells.org/pages/testsmells.html | `fopen`/`ifstream` on a corpus path with no `REQUIRE(exists)`. |
| General Fixture | Setup builds things most tests never use. https://testsmells.org/pages/testsmells.html | A large shared fixture struct, or a long file-top `static` builder used by a few tests. |
| Ignored Test | Disabled test. https://testsmells.org/pages/testsmells.html | `doctest::skip()`, `* doctest::may_fail()`, `#if 0`, `DOCTEST_...SKIP`, commented-out `TEST_CASE`. |
| Unknown Test | No assertion at all. https://testsmells.org/pages/testsmells.html | A `TEST_CASE` with no `CHECK`/`REQUIRE`/`CHECK_THROWS`; or one that only runs code and relies on "did not crash". |
| Empty Test | No statements. https://testsmells.org/pages/testsmells.html | `TEST_CASE("x") {}`. |
| Redundant Assertion | Compares a value to itself, always true. https://testsmells.org/pages/testsmells.html | `CHECK(a == a)`, `CHECK(true)`. |
| Duplicate Assert | Same assertion repeated with the same arguments. https://testsmells.org/pages/testsmells.html | Pasted `CHECK` lines in a row. |
| Redundant Print | Debug output left in the test. https://testsmells.org/pages/testsmells.html | `printf`, `std::cout`, `OutputDebugString` inside tests. |
| Sensitive Equality | Compares against a text rendering that changes for unrelated reasons. https://testsmells.org/pages/testsmells.html | `CHECK(to_string(x) == "...")` where the string is formatting, not a rule. |
| Test Logic Reimplements Production | Expected value recomputed with production formula. SWE@Google "logic in tests" https://abseil.io/resources/swe-book/html/ch12.html | The test includes the production header and calls the same helper to compute the "expected" side. |
| Change-detector test | Mirrors the implementation; breaks on every change, catches nothing. Google Testing Blog (only comments retrieved) https://testing.googleblog.com/2015/01/testing-on-toilet-change-detector-tests.html | Mock-heavy test that asserts call order or call counts; a test that copies a table out of the source file. |
| Overmocking / Mock Everything | Test checks only the mocks. https://test-smell-catalog.readthedocs.io/en/latest/ | Hand-written fakes with more lines than the code under test. |
| Test Pollution / Order Dependency | Result depends on what ran before. https://test-smell-catalog.readthedocs.io/en/latest/ and Fowler https://www.martinfowler.com/articles/nonDeterminism.html | Tests share `static` or global state, a singleton, a database or temp path with no reset. |
| Rotten Green Test | Assertion that never runs. https://test-smell-catalog.readthedocs.io/en/latest/ | `CHECK` inside a loop over an empty container, or after an early `return`. |
| Excessive Setup / The Giant | Test or setup is huge. https://test-smell-catalog.readthedocs.io/en/latest/ | `TEST_CASE` over ~100 lines; a doctest file of thousands of lines with one giant fixture. |
| Slow Test / Hardcoded Sleep | Suite slow enough that people skip it. https://test-smell-catalog.readthedocs.io/en/latest/ | A test that runs the full analysis on a big chart to check a small rule. |
| Empty-subject scan | Source-scan rule that matches nothing and passes. ArchUnit's guard https://www.archunit.org/userguide/html/000_Index.html | A scan whose file glob or regex has stopped matching after a rename, with no `REQUIRE(count > 0)`. |
| Rubber-stamp golden | Golden file regenerated without reading the diff. https://docs.deno.com/runtime/test/snapshots/ (via search summary) | A golden update commit with thousands of changed lines and no explanation. |

Meszaros's three headline groups (secondary summaries only): Obscure Test and others under code smells; Erratic Test, Fragile Test, Slow Tests under behaviour smells. Source for the grouping: search-result summaries of the book's contents (https://www.oreilly.com/library/view/xunit-test-patterns/9780131495050); I could not open xunitpatterns.com.

---

## 3. Topic notes

### Test properties and the general frame

- Beck lists 12 properties: Isolated, Composable, Deterministic, Fast, Writable, Readable, Behavioral, Structure-insensitive, Automated, Specific, Predictive, Inspiring. He says some properties pull against each other (for example, more predictive usually means slower), and that some conflicts are only apparent. https://testdesiderata.com/
- Khorikov (secondary) uses four pillars: regression protection, resistance to refactoring, fast feedback, maintainability. Notes on his book say the first three trade off against each other and that resistance to refactoring should always be kept high. That "binary" claim is one reader's reading, not a quote. https://notesbylex.com/four-pillars-of-good-unit-tests
- Agreement: tests should be sensitive to behaviour and blind to structure (Beck, SWE@Google, Khorikov, Fowler). https://testdesiderata.com/ , https://abseil.io/resources/swe-book/html/ch12.html
- Disagreement: how to treat test doubles. Fowler is a "classicist" (real objects, check state) and says mock-style tests couple to implementation; many good developers disagree. https://martinfowler.com/articles/mocksArentStubs.html SWE@Google prefers real dependencies when they are fast. https://abseil.io/resources/swe-book/html/ch12.html
- Disagreement on the shape of the suite: SWE@Google suggests roughly 80% unit, 15% integration, 5% end-to-end by count, and says it is a rough guide (https://abseil.io/resources/swe-book/html/ch11.html). Commenters on the Google blog cite 70/20/10 and caution against fixed targets (https://testing.googleblog.com/2015/04/just-say-no-to-more-end-to-end-tests.html, comments only). Fowler says "do not get attached to the layer names" (https://martinfowler.com/articles/practical-test-pyramid.html).
- Fit for Hydra: the suite is one huge doctest exe plus golden tests. The pyramid advice says to push each check down to the cheapest level that can see it, and to delete a higher-level test when a lower-level one covers it.

### Flaky tests and non-determinism

- Fowler's causes and fixes: lack of isolation (rebuild state per test, make teardown fail loudly), async behaviour (no fixed sleeps; poll with timeout; use the humble-object pattern to move logic into sync code), remote services (use doubles plus periodic contract tests), time (wrap the clock), resource leaks (small pool that throws when empty). Quarantine is temporary and should be capped. https://www.martinfowler.com/articles/nonDeterminism.html
- SWE@Google: at 0.1% flake rate across 10,000 daily tests you get about 10 investigations a day; near 1% tests start losing value because people stop trusting failures; Google's rate is about 0.15%. Rerunning only delays the real fix. https://abseil.io/resources/swe-book/html/ch11.html
- Larger tests are flakier; shrink the system under test; replace sleeps with polling or notifications; sleeps fail more as the fleet gets busy. https://abseil.io/resources/swe-book/html/ch14.html
- Luo et al. (FSE 2014) studied 201 fix commits in 51 projects. Async wait, concurrency and test-order dependency were the top causes. I could not read the percentages. Secondary: https://huang.isis.vanderbilt.edu/cs8395/paper/flakytest.pdf (binary PDF, unread) and the summaries returned by search (https://arxiv.org/pdf/2203.00483).
- Google's own flaky-test post came back as comments only. Its quoted stance: "a test that fails reliably is far better than a test that is flaky". https://testing.googleblog.com/2016/05/flaky-tests-at-google-and-how-we.html
- Fit for Hydra: the project has a rule against timing tests chasing reruns and a uitest suite. Check for frame-count waits and wall-clock thresholds in tests.

### Golden, snapshot and approval tests

- Sources agree: a golden test proves "same as the baseline", not "correct". Wrong baselines accepted once look like coverage. https://docs.deno.com/runtime/test/snapshots/ (via search summary of pitfalls)
- Pitfalls named: huge snapshots get rubber-stamped; blind regeneration captures new bugs; noise (timestamps, ordering, ids) causes false diffs; stale snapshots linger; format-sensitive snapshots break for unrelated reasons.
- Mitigations: small focused snapshots; pair with explicit assertions on critical values; snapshot only what a human can review; review the baseline change together with the code change; make regeneration a deliberate manual step. Rainsberger says regenerating should not be a swapped annotation inside the test runner; make it a separate deliberate program or command. https://blog.thecodewhisperer.com/permalink/how-not-to-write-golden-master-tests
- SWE@Google calls the technique A/B diff testing and lists its costs: someone must approve each difference, noise must be measured, coverage depends on the inputs, and setup is doubled. https://abseil.io/resources/swe-book/html/ch14.html
- Feathers' characterization tests (secondary): record what code actually does before you change it; a later team decides whether that behaviour was right. So a golden test is a safety net for refactoring, not a statement of correctness. https://www.oreilly.com/library/view/the-cucumber-book/9781941222911/f_0105.html and https://adamtornhill.com/reviews/legacycode.htm
- Disagreement: some sources want snapshots tiered by risk with mandatory reviewer sign-off on critical ones; Deno-style advice is just "use them only where a human can review". Both are advice from tool vendors, weaker than the books.
- Fit for Hydra: golden outputs on about 115 charts. Check how each golden is regenerated, what a diff looks like to a reviewer, and whether each golden is paired with at least a few literal assertions about known rule outcomes.

### Property-based testing

- It generates many inputs from a description and then shrinks a failing input to a small readable one. Failed examples are stored and retried first. The typical example is a round trip (encode then decode returns the original). https://hypothesis.works/articles/what-is-hypothesis/
- Limits stated: you still must decide the properties; random search can miss a hard bug; timing or hash-order bugs can still be inconsistent. Same source.
- I did not fetch a C++-specific source (RapidCheck or similar). Gap.
- Fit for Hydra: natural properties for a path optimiser are things like "the optimal score is never below any other valid path's score", "result does not depend on input order of equal notes", "round trip through the database equals the original". Whether these exist is for the audit to check. This is my inference, not a sourced statement.

### Mutation testing

- Mutation testing changes the code in small ways (flip a comparison, delete a line) and checks whether any test fails. A surviving mutant means the tests do not notice that change. Google: mutation is the strongest test criterion for finding faults and subsumes several coverage criteria. https://research.google/pubs/state-of-mutation-testing-at-google/
- Practical form at Google: run it only on changed lines in code review, skip "arid" (uninteresting) lines and lines without coverage, cap mutants per line and per review, and pick mutation operators by past usefulness. This took it from intractable to usable at scale. https://arxiv.org/abs/2102.11378 and https://research.google/pubs/state-of-mutation-testing-at-google/
- The Google paper's motivating example is a test that reaches every line but asserts nothing about the output. Coverage looks complete; mutation catches it. https://arxiv.org/pdf/2102.11378v1 (search summary)
- Fit for Hydra: a full-suite mutation run is not realistic at 43,000 test lines and a ~10 minute CI. A diff-based or sampled run on one rule module is. Note the project rule against full-library runs; mutation over `testdata/input` only would fit. Not a sourced statement, my reading.

### Coverage

- Fowler: coverage is good at finding untested code and poor as a quality number; targets backfire; he would be suspicious of 100% and worried below about half. https://martinfowler.com/bliki/TestCoverage.html
- SWE@Google: coverage measures execution, not verification; targets become ceilings; measure from small tests only. https://abseil.io/resources/swe-book/html/ch11.html
- Google's coverage paper: coverage is most useful at changeset level in code review; 512 survey responses from about 3,000 developers. https://research.google/pubs/code-coverage-at-google/
- All agree. No source supports using a coverage percent as a pass/fail gate for quality. Use it to find holes, then ask whether each hole matters.

### Architecture and fitness-function tests (source scans)

- A fitness function is any automated check that tells you how close the system is to a stated architectural goal. In Ford, Parsons and Kua's book they can be tests or metrics, atomic or holistic, triggered or continuous. Only secondary summaries retrieved: https://www.thoughtworks.com/en-us/insights/books/building-evolutionaryarchitectures-second-edition and https://nealford.com/downloads/Evolutionary_Architectures_by_Neal_Ford.pdf
- ArchUnit (the best documented example of the pattern): tests that read the code structure and fail on a forbidden dependency. Practical points from its guide: fail when a rule matches nothing; give each rule a reason; keep ignore lists narrow; freeze old violations and let only new ones fail; lock the freeze store in CI. https://www.archunit.org/userguide/html/000_Index.html
- ArchUnit works on compiled bytecode, not raw text. Hydra's scans read source text. That makes them closer to grep than to a structural check; the "a grep that misses X is not X absent" risk is higher. This difference is my observation. No source discusses text-scan architecture tests.
- Fowler's Mocks article and SWE@Google both favour tests coupled to behaviour, not structure. A source scan is coupled to structure by design. That is acceptable when the rule is itself structural ("one owner of this rule"), but it makes the scan a change detector for anything else. My reading.

### Speed and test impact analysis

- Regression test selection (run only tests affected by the change) trades safety against precision. Static analysis can miss tests (for example through reflection). The 2023 ISSTA paper suggests the field has hit a "performance wall". https://2023.issta.org/details/issta-2023-technical-papers/46/More-Precise-Regression-Test-Selection-via-Reasoning-about-Semantics-Modifying-Change (via search summary)
- Microsoft's Test Impact Analysis picks the subset needed for a commit and falls back to all tests when it cannot analyse. Docs are for an old product version. https://learn.microsoft.com/en-ie/previous-versions/azure/devops/pipelines/test/test-impact-analysis?view=tfs-2017
- SWE@Google: run larger tests after submit and keep presubmit fast. https://abseil.io/resources/swe-book/html/ch14.html
- Fit for Hydra: the project already uses name and source-file filters (`-tc=`, `-sf=`). A real selection tool for C++ (file-to-test map from coverage data) was not covered by my sources. Gap.

### Pruning and de-duplication

- The standard reference is Yoo and Harman's survey (2012) on minimisation, selection and prioritisation. Minimisation removes tests that add no coverage; classic heuristics are greedy set cover and integer programming. https://www.cse.chalmers.se/~feldt/advice/yoo_2010_regression_testing_survey.pdf (via search summary; the PDF body was not fetched)
- Coverage-based pruning has a known weakness: a test can add no new coverage yet still be the only one that asserts the result. Mutation score is a better guard before deleting (inference from the Google mutation paper above).
- Fowler's practical rule is simpler: delete a high-level test that duplicates lower-level coverage. https://martinfowler.com/articles/practical-test-pyramid.html
- I found no study that measures pruning on suites with source-scan or golden tests. Gap.

### Testing GUIs

- Dear ImGui Test Engine injects mouse, keyboard and gamepad input into ImGui, finds items by label path (`SetRef`, then `ItemClick("label")`), and can run headless in CI. It tries to "find its way", for example scrolling to an item. It works from the end user's point of view. Source: README summary via search (https://github.com/PathogenDavid/imgui_test_engine and https://github-wiki-see.page/m/ocornut/imgui_test_engine/wiki/Setting-Up). I did not read the API headers.
- Sources broadly agree: locate controls by role or label, assert on behaviour, and keep screenshots for the cases where appearance is the requirement. Image matching is brittle against font, layout, colour and resolution changes. https://www.ministryoftesting.com/insights/how-to-write-visual-ui-automation-tests-using-graphics-instead-of-complex-locator-strings
- Fowler: UI testing need not be end-to-end; test behaviour at a lower level where possible; use screenshot comparison or exploratory testing for layout and usability. https://martinfowler.com/articles/practical-test-pyramid.html
- Weak spot: most GUI-testing sources are vendor or blog pages, not studies. For an immediate-mode GUI there is little literature. Label-based handles also couple tests to label text: renaming a button breaks tests. That is the same fragility as Sensitive Equality, but the label is a stable user-facing contract, so it is the lesser evil. My reading.

---

## 4. Tensions the auditor should expect in this project

1. "Expected values are literals" versus "Magic Number Test" smell: both are real. Resolve by keeping the literal and adding a short source note (which run, which chart).
2. Source-scan tests are change detectors of structure by design. Check each has a reason, a non-empty subject guard and a narrow allow-list (ArchUnit practice).
3. Golden tests prove sameness only. Check each golden has a way to be regenerated deliberately, and that known-correct rule outcomes are also pinned as literals.
4. A large single test exe is a suite-organisation issue, not a smell by itself. The sources judge tests by behaviour and cost, not by file size.
5. Hook tests in PowerShell: no source I found covers them. Apply the general rules (isolation, determinism, one behaviour per test).
