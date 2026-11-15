# Test rubric

A test passing is not proof that Hydra works. It only proves Hydra meets the checks inside that test. So the checks have to say what Hydra is supposed to do, not just what it happened to do on the day someone wrote the test.

This page holds the questions every test must be able to answer. A reviewer asks them of each new test (`docs/agents/derive-once-review.md`, step 5). A test audit asks them of every existing test, and runs by `docs/agents/audit-method.md`.

## Why this page exists

On 2026-11-13 three CI runs on `main` failed for one reason. A 98th chart, Pitch Dark, was added to `testdata/input`, and tests across four files still expected 97. One of them, "corpus: squeeze-ins at exactly 0 and what they change" in `tests/test_engine_sweeps.cpp`, pinned nine totals over the whole test library, such as 374 saved rows. Nobody could say why 374 was right. It was what the code printed once. So the test went red for a new chart, and it would have stayed green for a bug that happened to leave the totals alone.

## The five questions

Answer each one in writing, in a sentence or two. Name files and functions. An answer someone can check against the code is the point. A general answer such as "needs more edge cases" counts as no answer.

**1. What does this test promise?** Say it as a Clone Hero rule, a user ruling, or something the player sees or relies on. "It runs without crashing" counts only when a crash is the bug it guards against.

**2. Where does each expected value come from?** Every number, string and list the test compares against has one of these four sources:

- **A Clone Hero rule.** Cite the page in `docs/clone-hero/` that states it.
- **Worked by hand from the test's own small input.** The arithmetic is written next to it, as in "scoring: category_scores is unchanged by sqout_pays_outside" in `tests/test_scoring.cpp`. That comment works out Red and Kick at combo 8 to 100 note points, 50 combo points and 150 Star Power points, and the checks compare against those.
- **A user ruling** under `## Confirmed` in `docs/agents/rulings/`. Quote it.
- **A property.** Two things Hydra computes must agree, or every item must meet a condition. The `shown_differs` check in the sweep test above is one: every saved row must show the same timing as `Path::difficulty()`.

Two sources are not allowed:

- **A snapshot.** The value is what the code printed when the test was written. It freezes today's behaviour, bugs included. A total over `testdata/input` is always one, because adding a chart changes it.
- **A copy.** The expected value is worked out by repeating the production code's steps in the test. A copy agrees with any bug in the code it copies.

A count on its own is weak even when its source is fine. "Three rows came back" passes with the wrong three rows. Check which items came back, or say why the count alone is the promise.

When you cannot find where a value comes from, do not guess and do not assume the current code is right. The verdict is **investigate**.

**3. Name a bug it catches, and one it misses.** The bug is a specific small change in a named function. Pick from these kinds:

- a comparison turned around or made inclusive (`<` to `<=`)
- a boundary moved by one tick, note or millisecond
- a condition dropped from an `if`
- a step skipped (a note not counted, a value not saved)
- a wrong value returned (zero, the previous item, the other side's value)
- an error path skipped

Then name one plausible bug of the same kinds that the test would let through, if it should catch it. Mark each claim **reasoned** (you read the code and believe it) or **confirmed** (the main session planted the bug, built, and saw the test go red or stay green). Agents cannot build (agent rule 14), so every agent claim starts as reasoned.

**4. What does it lean on that it does not own, and does a failure say where?** Things a test can lean on without meaning to: how many charts sit in `testdata/input`, the order files or rows come back in, timing, the machine, or another test running first. Then ask what a failure tells you. A test over the whole library that fails with one total says something moved somewhere. A good failure names the chart, chord or row.

**5. Does another test already make the same promise?** Name it, with file and test name.

## The verdict

One line, one of four:

- **Keep.** All five answers hold up.
- **Rewrite.** The promise is worth keeping but a check is weak, a snapshot or a copy. Say what the test should check instead.
- **Remove.** Another test already makes the same promise (name it), or the promise no longer applies to Hydra (say why).
- **Investigate.** You cannot tell what the test should promise, or where a value comes from. This goes to the user as a question (exit 2 in `docs/agents/audit-method.md`).

A rewrite or remove also says what the test catches today, so nothing quietly stops being checked.

## A filled-in record

```
corpus: squeeze-ins at exactly 0 and what they change   (tests/test_engine_sweeps.cpp)
1 Promise:  A saved row shows the timing the engine worked out.
2 Values:   shown_differs.size() == 0 is a property. charts == 98,
            saved_rows == 374 and seven other totals are snapshots over
            testdata/input.
3 Catches:  shown_timing() reading a saved 0.0 as "none" would fill
            shown_differs (reasoned).
  Misses:   The totals catch nothing the property misses; they only say
            something moved.
4 Leans on: Every total leans on which charts sit in testdata/input. A
            failed total names no chart.
5 Overlap:  None for the property.
Verdict:    Rewrite. Keep the shown_differs check. Drop the totals. Add a
            small built chart with a squeeze-in at 0.0 and check its row.
            Today it catches: a display/store mismatch (kept) and any change
            in the totals (dropped, since a chart change trips it too).
```

## Running it as an audit

An audit with this rubric is read-only. It changes no test and no code; each verdict leaves by one of the four exits in `docs/agents/audit-method.md`, and a fresh agent checks every closure.

Files that loop over `testdata/input` and compare totals against literals go first. That is where snapshots live. Every other file still gets all five questions.

The main session plants a sample of the agents' named bugs, builds once per batch, and records which tests went red. A claim that turns out wrong is reported, and the rest of that agent's claims get a closer read.

Missing coverage is a separate pass after the audit: list the rules on the `docs/clone-hero/` pages and the filed rulings, and flag each one no test checks.
