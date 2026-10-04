# Fix round

You fix the findings a derive-once review raised on a branch, so the next review comes back CLEAN. Read `docs/agents/brief-preamble.md` first; its rules apply to you in full. The orchestrator gives you the review file, the branch, its worktree, the range, and which findings are yours.

## Why this brief exists

One branch once took seven review rounds to reach CLEAN. Each round fixed only the copies the reviewer had listed. The next reviewer then found the copies nobody had listed, or a third copy of a finding marked fixed, or a new copy the fix itself had written. Three findings came back a round later as "still in N places". This brief ends that pattern: fix every copy of each kind, prove none is left, and leave committed work behind even if you run out of calls.

## Who does what

Production findings and test findings go to two fresh agents that run in parallel. Production findings are kind A, plus kind D where the copy is in `src/`. Test findings are kinds B and C, kind E, and kind D where the copy is in `tests/`. Your brief says which half is yours. Touch only the files on your side. If a finding needs a file on the other side, stop and report it; do not edit it.

You are capped at 100 tool calls. Commit after each finding, so an agent that runs out leaves its finished findings behind.

## The kinds, in one line each

- **A.** A rule written twice in production code.
- **B.** A test that recomputes what production computes, instead of calling it or pinning a literal.
- **C.** A copied test helper or fixture.
- **D.** A number or rule with no user decision, or a doc or comment stating the rule differently from the code.
- **E.** A source scan outside `tests/test_single_owner.cpp`, or a scan row loose enough to let copies through.

## How to fix a finding

1. **Restate the question.** Write the question the finding is about in plain words, and name its owner: the one function that answers it after your fix.
2. **Sweep the whole range for that question.** Do not stop at the copies the reviewer listed. Grep the whole branch (every file the range touches, and their callers) for the inputs the question reads, the fields, constants and units, not just for the names in the finding. Look inside the changed functions too; two copies in one function count. For a kind C finding, grep `tests/` for every other definition of the helper, by name and by its body.
3. **Fold every copy into the owner.** Each copy calls the owner, or for a test, calls production code or pins a literal from one run. Never compute an expected value in a test.
4. **Prove nothing is left.** Run the grep again after the fix. It must find only the owner, or only lines the scan already lists as known.
5. **Run the tests your brief names for that finding**, with `-tc=` or `-sf=` filters. Never the full suite.
6. **Commit that finding alone.** The commit body carries the grep proof: the exact grep you ran and its one-line result, for example "grep for X under src/ and tests/: one hit, the owner in Y". Then the trailers from the preamble.

A sweep for one finding often turns up copies of another kind. Fold those too if they are on your side, and name them in the commit body. If they are on the other side, list them in your report.

## Things you never do

Never add a new number (a threshold, floor, depth, tolerance or band) to settle a finding. If a fix needs one, stop and report it as a question for the user, in game terms. Never change a displayed text, a score or a stored record unless the review file quotes a user decision for it. Never write a new copy while removing an old one: if your fix needs a helper, check the owner or `tests/` for one first.

If a finding is wrong (the two copies are not the same question, or the reviewer misread the code), do not change the code. Say why in your report, with the truth table that shows the difference.

## After the sweep: run the precheck

When every finding on your side is committed, run `tools/derive_once_precheck.ps1` on the range and repo path. It needs no build. It must print nothing new for your side: no duplicate helper, no recompute spelling, no undecided number, no stray scan. If it does, fix those the same way and commit them. Quote its final output in your report.

## Your report

For each finding: its kind letter, the question, the owner, every copy you folded (including ones the reviewer did not list), the commit hash, and the grep proof. Then the precheck's final output, the exact test commands you ran with their pass counts, and anything you stopped on or left for the other agent.
