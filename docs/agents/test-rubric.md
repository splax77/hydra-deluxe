# Test rubric

A passing test is not proof that Hydra works. It proves Hydra does what the test says it should do. So two things have to be true. The test must say the right thing. And each check must say it directly, in a form a reader can follow without knowing what a number means.

This page holds the questions every test must be able to answer. A reviewer asks them of each new or changed test (`docs/agents/derive-once-review.md`, step 5). A test audit asks them of every existing test, and runs by `docs/agents/audit-method.md`. Real tests from this repo, quoted line by line, are in `docs/agents/test-rubric-examples.md`. Read the bad ones before you give your first verdict.

## Why this page exists

Three failures made this page. Each one got past people who were looking.

**A snapshot.** A 98th chart was added to `testdata/input`, and tests still expected 97. One of them pinned totals over the whole test library, such as 374 saved rows (`tests/test_engine_sweeps.cpp`, example B1). Nobody could say why 374 was right. It was what the code printed once.

**A label taken for a reason.** An earlier version of this page asked where each number comes from, then accepted a citation as the answer. It kept `CHECK(acts > 1000);  // a floor the user approved (D43)`. That comment does not say what the check is for, where 1000 comes from, or why the test needs it. It also kept tests because the sums in their comments added up. Correct sums were never the question. The question is why the test needs the number at all, instead of checking the behaviour directly.

**A wrong claim taken for the behaviour.** The earlier version then judged `CHECK(act.refill_tick(1) == 13440);` and decided its only fault was the raw number. It proposed the same check with the number given a name. The claim under the check is that a late squeeze-in refills the Star Power gauge at the old SP end. `Activation::refill_tick` in `src/core/model.cpp` works that out, and one place in `src/` calls it: the Preview's gauge, in `src/app/preview_view.cpp`. The search never asks it. Meanwhile the engine's own record already names the chord the player squeezes in. On Pitch Dark (`testdata/input/test_pitch_dark_ratio`) the record's squeeze-in step sits at tick 128520, and `refill_tick` moves the gauge's refill to tick 128400, the end that was in force before it. The gauge draws a moment the engine never named. Two tests pinned that moment, and every audit accepted them, because each audit asked whether the test was well written and never asked whether it was right.

So the questions below start with what the test claims, and who owns that claim. How the check is written comes after.

## Two kinds of number

A **rule number** is a number that is the rule. Clone Hero's 85 ms hit window is one. So is "one Star Power bar lasts two measures". The game or the user chose it, a page or a ruling states it, and the test would be wrong with any other value. Rule numbers are fine. Only the rule itself counts: 85 is a rule number, but 7285 (a deadline on a made-up chart plus 85) is a raw result built from one, and fails like any other raw result. The same holds for small numbers: 10 is the combo where the multiplier first doubles, and 9 is a result built from it. Write the working, `x2_from - 1`, so the line shows which rule the 9 comes from.

A **snapshot number** is what the code printed on the day the test was written. A total over `testdata/input` is always one, because adding a chart changes it. A number "pinned from one run" is one. A number an agent picked and someone approved is one too, until somebody writes down why that value and no other. Snapshot numbers are never fine.

A raw result sits between them, and it fails too. `CHECK(act.refill_tick(1) == 13440)` holds a number that follows from the chart. But a reader cannot tell what 13440 is, or what behaviour the line protects. Its meaning lives in a comment, in another file. A reviewer should not have to just know.

## The questions

Answer each one in writing, in a sentence or two. Name files, lines and functions. A general answer such as "needs more edge cases" counts as no answer.

Questions 1 to 3 judge the claim. Do them first, and do them before you read the test's own comments as fact. A comment in a test is a claim made by whoever wrote the test (agent rule 9).

**1. What does the test say should happen?** Write it as one sentence in the game's words, as a player would see it: "when the player squeezes a late phrase in, the gauge refills at the moment Star Power would have ended". Take it from the test's name, its checks and its comments together. If you cannot write the sentence, the verdict is investigate. Do not go on to judge how well the test checks a claim you cannot state.

**2. Which layer owns that rule, and who uses it?** Find the function that works the rule out. Then grep `src/` for every caller, and write the callers down. Three answers are possible.

- The engine owns it: the search uses the rule to choose or score a path, and stamps the answer on the record. Go on.
- A display owns it, and it is the display's to own: how wide a bar is drawn, what colour a badge is, how a number is rounded for the screen. Go on.
- A display works out a game fact for itself: when a bar arrives, which chord ends Star Power, what a squeeze is worth. This is the bug. A game rule that only display code uses is a sign the display invented it. Report it as a derive-once finding (kind A in `docs/agents/derive-once-review.md`) against the production code. It is not a question about what Clone Hero does: the engine already has its answer, and the display is meant to draw that answer.

Where the function sits does not settle this. `refill_tick` sits in `src/core`, on the engine's own record type, and it is still a display rule, because only the Preview calls it. Judge by the callers. Also search for the engine's answer to the same question before you call a function the owner (agent rule 15): the stored step's own tick already said where the squeeze-in is.

**3. Does the owner's answer on a real chart agree with the claim?** Take one real chart where the case happens. Read what the engine recorded for it, chord by chord. Then say what the test's claim would put on that chart, and whether the two agree.

The engine's record is readable as text. `hydra_replay.exe dump` then `hydra_replay.exe schedule` print, for each activation, its `sp_end_steps` and `squeezes` with the chord and tick of each, and for each chord its `paid_by` and `ends_sp`. Agents may not run `hydra_replay` (agent rule 14), so an auditor asks the main session for the output and names the chart. Write the answer as two lines:

```
Engine:  Pitch Dark, first activation: squeeze-in step on chord 243, tick 128520.
Claim:   the gauge refills at the end in force before it, tick 128400.
         They differ. The test pins the display's own rule.
```

If they differ, the test's framing is the bug, and no amount of tidying its checks fixes it. If you have not seen the engine's record for a real chart, write "claim not checked". A test with an unchecked claim cannot get a verdict of keep.

A decision record does not answer this question. The squeeze-in claim cites one: D9 in `docs/audit/2026-10-03-fix-decisions.md` says the gauge refills at the old SP end, "matching the engine". The record on Pitch Dark shows it does not match. When a decision and the engine's record disagree, say so and send both to the user. Never settle it by citing the decision.

**4. Does each check state the expected behaviour directly?** Go through the test one assertion at a time. For each, say in one plain sentence what it checks, then look at what it compares against. These pass:

- A rule number, typed because the test is about that rule, with the page and rule id that state it written where the number is typed. Calling a number a rule number does not make it one: open the rule and find that exact value in it.
- A property: two things that must agree, or a condition every item must meet. "No row differs." "Every stored fill is a fill on the screen."
- The owner's answer, compared item by item (question 6).
- The rule applied to the chart's named inputs, so the check reads as the rule.

These fail: a snapshot number, a floor with no reason, and a raw result whose meaning lives in a comment. The test for a raw result is to cover the comments and ask whether a reader can still tell what behaviour the line protects.

**5. Can each number be traced to a reason?** A reason says why this value and not another, and why the test needs it. An approval is not a reason. A citation is not a reason until you open it. So open every D-number, ruling and page the test cites, and write down what it said. Four things give a reason: a Clone Hero rule (cite the page and rule id under `docs/clone-hero/`, and copy its `Label:` line); a line under `## Confirmed` in `docs/agents/rulings/` that names the value; the test's own input, which the test is free to choose; and a property, which needs no number. An input is free only when its name or its working says what it stands for. `to_multiplier(1000)` holds an input nobody can explain: why 1000, and not 31 or a million? If the value matters, write it from the rule. If it does not, say what range it stands for, in code. Zero is the one literal a property may use freely.

Three things never give one. A snapshot. A copy, where the test repeats the production code's steps to make its expected value, so it agrees with any bug in them. And a saved expected-output file: the ruling in `docs/agents/rulings/code-and-engine.md` reads "we don't need golden anymore, remove it all", so never propose one.

**6. For a test of a display: is the display checked against what the engine already knows?** A display is anything a person reads: the Preview, the gauge, a report, a table, a label. Its test takes the expected side from the engine's record, and compares item by item: for each chord, step or row the record names, the screen shows that one, at that place. The test never types the value the display should show, and never calls a function only the display uses to get it. A display test with a typed expected value can only prove the display does what the display does. "Not a display test" is an answer only with question 2's caller list beside it, showing that nothing a person reads calls the function directly.

This is not the fault question 9 describes. Comparing a screen with the record is the right shape for a display test, because the record is the screen's owner. The record itself is tied to the game by the engine's own tests.

**7. What failure is the test for, and can it still happen?** Find the commit that added the test (`git log -S` on its name) and read what broke. Say the failure in one sentence. Then read today's code and say whether it is still possible. A test nobody can name a failure for was written to make a number green. A commit that names no failure, such as a bulk checkpoint, leaves the question unanswered; it does not answer it. Then name the failure yourself, as a wrong result a player would see, or give the verdict investigate.

**8. Line by line: can this check fail, and if it were deleted, what bug would go unnoticed?** Answer for every assertion. First read the production code and ask whether the line can fail at all. `CHECK(act.refill_tick(k) <= prev.end_tick)` in `tests/test_sp_end.cpp` sits under the comment "SP never runs dry inside a window". For a squeeze-in step, `refill_tick` returns the smaller of the step's tick and `prev.end_tick`. So the line is true by construction there, and it proves nothing about Star Power running dry. Then name the bug the line catches, as a small change in a named function: a comparison turned around, a boundary moved by one tick, a step skipped. If the answer is "none, another line catches it", the line goes. If the answer is "any change at all", the line is a snapshot. Mark each claim **reasoned** (you read the code) or **confirmed** (the main session planted the bug, built, and saw red). Agents cannot build, so every agent claim starts as reasoned.

**9. Is the check aimed at the real risk, or at a stand-in for it?** Three stand-ins come up again and again. A **floor** such as `acts > 1000` stands in for "the loop checked something and skipped nothing". Check those directly: count the skips and require zero, or name each chart that may be skipped and why; then require the loop ran at least once. A **silent skip** such as `if (song.is_empty()) continue;` stands in for "nothing to check here", and also hides a chart that failed to load. A **count** stands in for "the right items came back", and passes with the wrong items. A fourth is two Hydra functions compared with each other when neither is the other's owner and neither is checked against a rule. Also ask what the test leans on by accident: how many charts are in `testdata/input`, file order, timing, another test running first. And ask what a failure would tell you. A good failure names the chart, chord or row.

**10. Do other tests state the same rule?** Search for the rule, not the test's name: grep `tests/` for the function, the field and the words. One rule gets one verdict, and every test that rests on it points at that verdict. Take care that it really is one rule. "A late squeeze-in's bar arrives at the old end" (`Activation::refill_tick`) and "a squeezed-out bar arrives at the deact node" (`bank_rise_ticks`) sound alike and are two rules with two owners.

**11. Can the scenario happen in a real chart?** A fixture at 4000 BPM may be the only way to reach an edge. That is fine when the test says why the odd input is needed and names the real case it stands for. Without that, the test may guard a state no player can reach, and its expected value may be whatever Hydra did there.

**12. For the auditor: what did you actually read?** Every verdict lists what was opened: the test's lines, the fixture, the production function and its callers, the engine's record for the real chart in question 3, each cited page, ruling or D-number, and the commit that added the test. Something you did not open is written as "not read", never guessed.

## The verdict

One line, one of four.

- **Keep.** The claim agrees with the owner's answer on a real chart, and every other question holds for every assertion. One answer of "no", "partly", "not read" or "claim not checked", on any question or any line, rules Keep out. A fault that the record admits is still a fault.
- **Rewrite.** The promise is worth keeping, but the claim is the display's own rule, or a check is a snapshot, a copy, a stand-in or a raw result. Say what the test should check instead, as the behaviour stated directly. A rewrite never keeps a value that question 3 showed to be wrong, under a new name or any other way.
- **Remove.** Another test already makes the same promise (name it), or the failure can no longer happen (say why).
- **Investigate.** You cannot say what the test should promise, or the engine's record and a decision disagree, or a value has no reason you can find. This goes to the user as a question (exit 2 in `docs/agents/audit-method.md`). Do not assume the current code is right.

"Keep, with a flag" is not a verdict. A flag is an open question, and an open question is rewrite or investigate.

A verdict about a test can carry a second finding about production code. When question 2 or 3 finds a display working out a game fact, write that finding on its own line, against the display's function. Rewriting the test does not close it.

A rewrite or remove also says what the test catches today, so nothing quietly stops being checked. One test can hold good and bad lines; give the verdict per test and list the lines that drive it.

## A filled-in record

```
sp meter curve: a late squeeze-in refills at the old SP end
            (tests/test_sp_meter_curve.cpp:403)
Read:       lines 403-427; make_late_sqin_song and its comment
            (tests/record_fixtures.h:81-90); Activation::refill_tick
            (src/core/model.cpp:762-768); its callers (grep refill_tick in
            src/: preview_view.cpp:428 only); build_sp_meter_curve
            (src/app/preview_view.cpp:125-220); D9; the schedule output for
            Pitch Dark, from the main session. Adding commit: not read.
1 Claim:    When a phrase sits just past the SP end and the player squeezes
            it in, the gauge refills at the old end, before the phrase.
2 Owner:    refill_tick works it out. Only the Preview calls it. The search
            stamps the step's own tick and never asks for a refill tick. A
            display rule about a game fact: derive-once finding.
3 Real chart: Engine, Pitch Dark, first activation: squeeze-in step on chord
            243, tick 128520. Claim: refill at tick 128400. They differ.
4 Direct:   Lines 418-426 type nine gauge readings in ms. None says which
            behaviour it protects without the fixture's comment.
5 Reason:   7000 ms and 0.875 follow from the display's rule, so their only
            source is the code under test. D9 says "matching the engine";
            the record does not match.
6 Display:  Fails. The expected side is typed, not read from the record.
7 Failure:  "Finding 5" in the comment; not opened.
8 Deleted:  421: the refill moving off the old end (reasoned). That is the
            behaviour question 3 shows to be wrong.
9 Stand-in: None.
10 Same rule: tests/test_sp_end.cpp:485 and tests/test_model.cpp:1175 pin
            the same rule. One verdict covers all three.
11 Real:    Yes; Pitch Dark's first activation has the same case.
Verdict:    Rewrite. For each step of each activation, the gauge changes at
            the tick the record's step names, and nowhere else. No ms is
            typed. D9 against the record goes to the user.
Finding:    Activation::refill_tick is a game rule only the Preview uses
            (derive-once, kind A).
            Today it catches: a gauge that fails to refill at all (kept by
            the rewrite), and a refill that leaves the old end (dropped on
            purpose).
```

## Named inputs

Question 4 lets a check be the rule applied to the chart's named inputs: "measure 3", "a quarter measure after this end". No test helper does this today, and this page does not write one; that is a code change with its own plan and review. A test needs no helper to do it, though: example G1 on the examples page names its three inputs as plain constants in the test. The fixtures state their grid in comments, such as the one above `make_late_sqin_song` in `tests/record_fixtures.h`.

One trap matters now. A helper that names a tick by calling Hydra's timing code, or `test::tick_ms` in `tests/record_fixtures.h`, which reads the song's own tempo map, is fine for finding an input. It may never produce an expected result. An expected value that came out of production code is a copy.

Naming is also never the whole fix. Giving 13440 the name "old_end" leaves the claim untouched. Do questions 1 to 3 first.

## Running it as an audit

An audit with this rubric is read-only. It changes no test and no code. Each verdict leaves by one of the four exits in `docs/agents/audit-method.md`, and a fresh agent checks every closure. That page's calibration stage applies: before the auditors start, one agent gets a file whose bad tests are known, with no hint of what they are. Do not use `tests/test_sp_end.cpp`, `tests/test_sp_meter_curve.cpp` or any other file the examples page quotes, because an agent that read the examples knows the answers. The answer key must hold at least one test whose framing is the bug, so the calibration shows whether an auditor does questions 1 to 3 or skips to the checks.

Display tests go first: every test that builds a scene, a report or a label. Then files that loop over `testdata/input` and compare against literals. Every other file still gets every question.

For question 3 the main session prepares the engine's record for a handful of real charts before the auditors start, and gives each auditor the path. An auditor who needs another chart asks for it and writes "claim not checked" until it arrives.

Four mechanical checks back the reading up. Only the main session can run the first two (agent rule 14), the last two are code changes with their own plan, and none replaces the reading.

**Change the corpus and watch what turns red.** In a throwaway worktree, add one chart to `testdata/input` and run the tests, then remove one no test names and run them again. Every test that changes colour leans on which charts are in the corpus. A test that stays green is not proven independent.

**Plant the bugs from question 8.** The main session plants a sample of the bugs the auditors named, builds once per batch, and records which tests went red. A wrong claim is reported, and the rest of that auditor's claims get a closer read.

**Check every display against the engine, chord by chord, on every corpus chart.** A test that walks every chart in `testdata/input` and, at each chord, compares what a display draws (Star Power on or off, a refill, an end) with what the engine's record says for that chord. One such test would have caught the Preview drawing Pitch Dark's refill one note before the squeezed snare and Blight's Star Power running past the squeezed-out chord. No such test exists today (question 6). Writing it is a code change.

**A standing scan for totals over the corpus.** A permanent check that fails a counter added to inside a loop over `corpus::chart_paths()` and compared with a literal other than 0 after it, so a new snapshot total fails before it merges. A row in `tests/single_owner/rules/` is one regular expression on one line (`flags_line` in `tests/single_owner/engine.h`) and cannot see the loop, so it lives as its own case beside them, the way `tests/single_owner/special.cpp` does. Writing it is a code change.

Missing coverage is a separate pass after the audit: list the rules on the `docs/clone-hero/` pages and the filed rulings, and flag each one no test checks.
