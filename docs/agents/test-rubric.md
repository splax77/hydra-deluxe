# Test rubric

A test passing is not proof that Hydra works. It only proves Hydra meets the checks inside that test. So each check has to say what Hydra is supposed to do, in words a reader can follow back to a reason.

This page holds the questions every test must be able to answer. A reviewer asks them of each new or changed test (`docs/agents/derive-once-review.md`, step 5). A test audit asks them of every existing test, and runs by `docs/agents/audit-method.md`. Real good and bad tests from this repo, quoted line by line, are in `docs/agents/test-rubric-examples.md`. Read at least the bad ones before you give your first verdict.

## Why this page exists

On 2026-11-13 three CI runs on `main` failed for one reason. A 98th chart, Pitch Dark, was added to `testdata/input`, and tests across four files still expected 97. One of them, "corpus: squeeze-ins at exactly 0 and what they change" in `tests/test_engine_sweeps.cpp`, pinned nine totals over the whole test library, such as 374 saved rows. Nobody could say why 374 was right. It was what the code printed once.

The first version of this page asked where each expected value comes from. It then accepted a label as the answer. A trial on `tests/test_sp_end.cpp` kept a check that read `acts > 1000` because its comment named a decision, D43. D43 says the number was approved. It does not say why 1000. The trial also kept fifteen tests because the arithmetic in their comments added up. Correct arithmetic was never the question. The questions are where the numbers come from, and why the test needs them instead of checking the behaviour directly.

## Two kinds of number

Every question below leans on this split, so learn it first.

A **rule number** is a number that is the rule. Clone Hero's 85 ms hit window is one. So is "one Star Power bar lasts two measures". The game or the user chose it, a page or a ruling states it, and the test would be wrong with any other value.

A **snapshot number** is what the code printed on the day the test was written. A total over `testdata/input` is always one, because adding a chart changes it. A number pinned "from one run" is one. A number an agent picked and a batch answer approved is one too, until someone writes down why that value and no other.

Rule numbers are fine. Snapshot numbers are not.

A third kind sits between them: a number that is right, but hides the rule. `CHECK(act.deact_tick() == 5376)` on a made-up chart is correct. But 5376 means nothing to a reader. The rule, "Star Power lasts two measures per bar from the activation", is in a comment, and the check holds only its result. Question 6 deals with these.

## The questions

Answer each one in writing, in a sentence or two. Name files, lines and functions. An answer someone can check against the code is the point. A general answer such as "needs more edge cases" counts as no answer.

**1. Does each check state a rule or a number?** Go through the test one assertion at a time. For each, say the rule it checks in one plain sentence. Then look at what the assertion compares against. A rule number passes. A property passes: two things that must agree, or a condition every item must meet. A named input with the rule applied to it passes. A bare number whose meaning lives only in a comment, or nowhere, does not pass here; send it to question 2 and question 6.

**2. Can each number be traced to a reason?** A reason says why this value and not another. An approval is not a reason. A citation is not a reason until you open it. So open every D-number, ruling and page the test cites, and read whether it gives a reason for that exact value. Write down what you opened and what it said. The `acts > 1000` checks cite D43; D43 calls such numbers approved test limits and gives no reason for 1000. A number with no reason fails this question, however many people approved it.

These sources give a reason:

- **A Clone Hero rule.** Cite the page and rule id in `docs/clone-hero/`, and copy its `Label:` line. If the label says Hydra assumes the rule, go to question 7.
- **A user ruling** under `## Confirmed` in `docs/agents/rulings/`. Quote it, and check the quote names this value.
- **The test's own input.** The chart the test built has a resolution, a tempo and a time signature. Those are the test's to choose, and a value that follows from them by the cited rule has a reason. Question 6 says how it must be written.
- **A property.** No number is needed, so none needs a reason. Zero is the one literal a property may use freely: "no row differs", "the loop checked something".

These do not:

- **A snapshot.** See "Two kinds of number".
- **A copy.** The expected value is worked out by repeating the production code's steps in the test, or by a frozen copy of old code. A copy agrees with any bug in the code it copies.
- **A saved expected-output file.** The user ruled these out on 2026-08-18: "we don't need golden anymore, remove it all" (`docs/agents/rulings/code-and-engine.md`). Never propose one as a rewrite.

Calling the owner is not a copy. "graph: every deactivation edge lists the chords squeeze_window_phrases names" in `tests/test_sp_end.cpp` compares the graph's choices with `squeeze_window_phrases`, the owner the graph is meant to use. That proves the wiring: the graph passes the owner's answer on. It proves nothing about whether the owner is right. So name the test that checks the owner itself against a rule. With none, the verdict is rewrite or investigate.

**3. What failure is the test for, and can it still happen?** Find the commit that added the test (`git log -S` on the test's name) and read what broke. Say the failure in one sentence. Then read today's code and say whether that failure is still possible. A test for a bug in code that no longer exists guards nothing. A test nobody can name a failure for was written to make a number green.

**4. Line by line: if this check were deleted, what bug would go unnoticed?** Answer for every assertion, not for the test as a whole. Name the bug as a small change in a named function: a comparison turned around or made inclusive, a boundary moved by one tick or millisecond, a condition dropped, a step skipped, a wrong value returned, an error path skipped. If the answer is "none, another line already catches it", the line goes. If the answer is "any change at all", the line is a snapshot. Mark each claim **reasoned** (you read the code and believe it) or **confirmed** (the main session planted the bug, built, and saw the test go red). Agents cannot build (agent rule 14), so every agent claim starts as reasoned.

**5. Is the check aimed at the real risk, or at a stand-in for it?** Say what could really go wrong, then ask whether the check tests that or something near it. Three stand-ins come up again and again.

A **floor** such as `acts > 1000` stands in for "this loop checked something, and skipped nothing". Check those two things directly. Count what the loop skipped and require the count to be zero, or name each chart that may be skipped and say why. Then require that the loop ran its checks at least once (`> 0`, which needs no source).

A **silent skip** such as `if (song.is_empty()) continue;` stands in for "this chart has nothing to check". It also hides a chart that failed to load. Every chart could take that branch and the test would stay green. A skip must be counted and explained, or be a failure.

A **count** stands in for "the right items came back". "Three rows came back" passes with the wrong three rows. Check which items came back, or say why the count alone is the promise.

Also ask what the test leans on without meaning to: how many charts sit in `testdata/input`, the order files come back in, timing, the machine, another test running first. And ask what a failure tells you. A good failure names the chart, chord or row. A failed total says only that something moved.

**6. Are inputs named for what they mean, and is the expected result written as the rule applied to them?** A raw tick is fine as the chart's own input: the test built a note at tick 2304 and may say so. A raw tick is never fine as an expected result. The result is written as the rule applied to named inputs, so the check itself reads as the rule:

```
activation = measure(3)                  // the chart's own input
CHECK(end == activation + bars * measures_per_bar * one_measure)
```

and not `CHECK(end == 5376)` under a comment that does the sum. Correct arithmetic in a comment does not pass this question. The test is this: cover the comments, and ask whether a reader can still tell which rule the check enforces. "Named inputs" below says what the names may and may not be built from.

**7. Is the test only checking Hydra against Hydra?** Three shapes count. Two Hydra functions are compared and neither is checked against a rule. The expected value comes out of a Hydra function. Or the rule is one Hydra made up: a page labels it as Hydra's assumption (SP-33 on `docs/clone-hero/05-star-power.md` reads "Label: Hydra assumes it, never checked"), or no page states it at all. For each rule the test rests on, say whether any test or live run ties it to what Clone Hero does, and name it. A Hydra-only test can stay as a wiring check, but the record says so in those words, and a rule with no tie to the game goes to the user as a question.

**8. Do two tests state the same rule differently?** Search for the rule, not the test's name. A rule is stated once, in one owner, and tests use it. Two tests that each type the rule in their own comment will drift, and two audits will judge them differently. That happened: "the bar arrives at the old end" was kept in one test of `tests/test_sp_end.cpp` and sent to investigate in another. One rule gets one verdict. If another test already makes the same promise, name it with file and test name.

**9. Can the scenario happen in a real chart?** A fixture at 4000 BPM, or a search at Hydra's what-if Star Power cap of 2, may be the only way to reach an edge. That is fine when the test says why the odd input is needed and what real case it stands for. It is a smell when it does not: the test may be guarding a state no player can reach, and its expected value may be whatever Hydra did there.
**10. For the auditor, not the test: what did you actually read?** Every verdict lists what was opened: the test's lines, the fixture, the production function, each cited page, ruling or D-number, and the commit that added the test. Every claim is marked reasoned or confirmed. Something you did not open is written as "not read", never guessed. And a verdict of keep is impossible while any value in the test lacks a reason. "Keep, with a flag" and "keep, with a note" are not verdicts. A flag means a question is open, and an open question is rewrite or investigate.

## Named inputs

Question 6 needs tests to say "measure 3" and "250 ms after this end" instead of typing ticks. No helper does this today. The search: `tests/record_fixtures.h`, `tests/corpus_util.h` and every `.cpp` and `.h` under `tests/` for "measure", "resolution", `plusmeasure` and `768`. The fixtures state their grid only in comments ("192 ticks per beat, so a measure is 768 ticks and 2000 ms", `tests/test_sp_end.cpp` line 50). The nearest code is `m * 768` typed inline in `tests/test_rules.cpp` line 51. In `src/`, the owner of "a tick so many measures later" is `Timing::plusmeasure`, and the owner of "a tick's time" is the song's own tempo map, which `test::tick_ms` in `tests/record_fixtures.h` reads.

Those two production owners are the trap. A helper that turns measures into ticks by calling `Timing::plusmeasure` makes every expected value a copy: the test would ask Hydra's timing code what the answer is and then check Hydra against it. The same goes for `test::tick_ms` when it is used to build an expected value. Both are fine for finding an input. Neither may produce a result.

So the helper has to describe the chart the test itself built, and nothing else. Its home is `tests/record_fixtures.h`, beside `build_fixture_song`, which already takes the resolution and the tempo. It must be this:

- It holds the fixture's own facts: ticks per beat, beats per measure, and one flat tempo. The test chose them, so they are inputs.
- From those it answers "the tick where measure N starts", "the length of one measure in ticks" and "the tick this many ms after that tick" by plain multiplication, and it calls nothing in `src/`.
- It refuses a chart with a tempo change or a second time signature. On those charts a tick's time is no longer one multiplication, and writing the sum again in a helper would be a second timing engine. A test on such a chart names its inputs by hand, each with the sum from the chart's own tempo map beside it.
- The song builder and the helper read the same description, so the chart and its names cannot disagree.

This page does not write the helper. That is a code change, with its own plan and review.

A helper names inputs. It does not own game rules. "Two measures per bar" is Clone Hero's rule, and its number comes from the rule's page, or from the production constant when the test is about something else. A test of that rule itself types the number, so that a changed constant fails the test.

## The verdict

One line, one of four:

- **Keep.** Every question holds, for every assertion. Every value has a reason the auditor opened and read.
- **Rewrite.** The promise is worth keeping, but a check is a snapshot, a copy, a stand-in, a raw result or a Hydra-only comparison. Say what the test should check instead, as the behaviour stated directly.
- **Remove.** Another test already makes the same promise (name it), or the failure the test was for can no longer happen (say why).
- **Investigate.** You cannot tell what the test should promise, or a value or rule has no reason you can find. This goes to the user as a question (exit 2 in `docs/agents/audit-method.md`). Do not guess, and do not assume the current code is right.

A rewrite or remove also says what the test catches today, so nothing quietly stops being checked.

One test can hold good and bad lines. Give the verdict per test, and list the lines that drive it. A rule shared by several tests gets its verdict once (question 8), and every test that rests on it points at that one answer.

## A filled-in record

```
SP end history: every corpus activation is consistent   (tests/test_sp_end.cpp:761)
Read:       lines 761-800; tests/bank_check.h not read; D43 addenda in
            docs/audit/2026-10-03-fix-decisions.md lines 109 and 112;
            commit ad3aab5c (git log -S"acts > 1000").
1 Rule/number: Lines 777-791 are properties (history never empty, steps in
            order, each step inside the end before it). Line 799,
            acts > 1000, is a bare number.
2 Reason:   The properties need none. 1000: the comment cites D43. The
            addenda read say "Approved as a test limit" and give no reason
            for 1000. No reason found.
3 Failure:  Added in ad3aab5c with the stored history. Commit message read,
            diff not read: the failure it was for is not established.
4 Deleted:  777: an activation stored with no history (reasoned). 790: a
            step that takes effect after SP ran dry (reasoned). 799: nothing
            the properties miss, unless the loop ran under 1001 times.
5 Risk:     799 stands in for "the loop checked every chart". Line 766
            skips an empty song without counting it, so all 98 charts could
            be skipped and only 799 would notice, for no stated reason.
6 Named:    No raw results; the checks compare fields with fields.
7 Hydra:    Every line compares Hydra's stored history with itself. Which
            test ties "a step takes effect at or before the end in force"
            to the game: not found, not searched beyond this file.
8 Same rule: "Bank: every corpus path banks in order" (line 827) has the
            same floor and the same skip. One verdict covers both.
9 Real:     Real charts.
Verdict:    Rewrite. Keep the properties. Replace 799 with: zero charts
            skipped (or each skipped chart named with its reason), and
            acts > 0. Question for the user under 7.
            Today it catches: a malformed history on any corpus chart
            (kept), and a corpus that shrank below 1001 activations
            (dropped; the skip count replaces it).
```

## Running it as an audit

An audit with this rubric is read-only. It changes no test and no code. Each verdict leaves by one of the four exits in `docs/agents/audit-method.md`, and a fresh agent checks every closure. That page's calibration stage applies too: before the auditors start, one agent gets a file whose bad tests are known, with no hint of what they are. Do not use `tests/test_sp_end.cpp`: this page and the examples page discuss its tests, so any agent that read them already knows the answers. Pick a file neither page quotes, and have the main session write its answer key before the calibration run.

Files that loop over `testdata/input` and compare against literals go first. That is where snapshots live. Every other file still gets every question.

Three mechanical checks back the reading up. None replaces it.

**Change the corpus and watch what turns red.** In a throwaway worktree the main session adds one chart to `testdata/input` and runs the tests, then removes one chart and runs them again. The corpus is read at run time (`corpus::chart_paths` in `tests/corpus_util.h`), so one build serves both runs. Every test that changes colour leans on which charts are in the corpus, and goes to the top of the audit list. The check has two limits. A test that stays green is not proven independent: its total may not have moved for that one chart. And a test that names the removed chart fails for an honest reason, so remove a chart no test names. Only the main session can run this (agent rule 14).

**Rebuild the answers blind.** An agent gets one test's input and the rule it cites, with the expected values left out, and works out the answers. Use it on tests with hand-built charts. A mismatch is not a verdict: the test, the rule's page or the agent may be wrong, so it becomes investigate. A match proves less than it seems when the rule is Hydra's own assumption (question 7). The check is most useful for question 6: when the agent cannot work the answer out from the cited rule and the named inputs, the test does not state its rule. The hiding is only as good as the brief. The agent can open the test file, so the brief gives the input and the rule in full and tells the agent to open neither the test nor its fixture comments.

**A standing scan for totals over the corpus.** The idea is a permanent check that fails any assertion comparing a total gathered over `corpus::chart_paths()` with a literal other than 0. It holds, with two changes. First, it cannot be a row in `tests/single_owner/rules/`. A row is one regular expression tried on one line (`flags_line` in `tests/single_owner/engine.h`), and a line cannot know whether it sits inside a test that loops over the corpus. The right home is a case of its own beside the rows, the way `tests/single_owner/special.cpp` holds "the single-owner cases that a row cannot express"; source scans live nowhere else (kind E in `docs/agents/derive-once-review.md`). Second, "no literal but 0" is too wide. A corpus property may rightly use a tolerance such as `1e-9` or a rule number inside the loop. The scan should flag only a counter that is added to inside the loop and compared with a literal other than 0 after it. The scan is a code change: it needs its own plan, a baseline for the totals that exist today, and the user's yes. Until it exists, the audit finds these by reading.

The main session also plants a sample of the agents' named bugs from question 4, builds once per batch, and records which tests went red. A claim that turns out wrong is reported, and the rest of that agent's claims get a closer read.

Missing coverage is a separate pass after the audit: list the rules on the `docs/clone-hero/` pages and the filed rulings, and flag each one no test checks.
