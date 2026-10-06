# Fixing your review findings

You wrote a change, and the derive-once reviewer has sent you its findings in a message. This page says how to fix them. Read `docs/agents/brief-preamble.md` first if you haven't; its rules apply to you in full.

You get one chance. The user decided this in D61: the reviewer sends findings once, you fix them once and reply, and then the reviewer checks your fix and fixes anything left itself. There is no second round. So fix every copy now, not only the ones the reviewer listed.

## Why this page exists

One branch once took seven review rounds, and another took ten. Each round fixed only the copies the reviewer had listed. The next review then found copies nobody had listed, a third copy of a finding marked fixed, or a new copy the fix itself had written, often a comment restating a rule. Fix the whole question, prove nothing is left, and write no new copies.

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
5. **For a kind A finding, guard it with a scan row.** Removing the copy is not enough, because nothing stops the next agent writing it again. Add a row to `tests/test_single_owner.cpp` that would flag the copy you removed if it came back: its must-match examples include that copy's line, and its owner is the function from step 1. Follow the rows already in that file for the form. If no text search can catch this copy, say why in one sentence in the commit body instead. The reviewer checks for one or the other when it signs off.
6. **Run the tests your brief names for that finding**, with `-tc=` or `-sf=` filters. Never the full suite. If you added a scan row, also run the scan test, `build-cpp\Release\hydra_tests.exe -tc="single-owner*"`.
7. **Commit that finding alone.** The commit body carries the grep proof: the exact grep you ran and its one-line result, for example "grep for X under src/ and tests/: one hit, the owner in Y". For a kind A finding it also names the scan row's question, or gives the one-sentence reason no row can catch it. Then the trailers from the preamble.

A sweep for one finding often turns up copies of another kind. Fold those too, and name them in the commit body.

## Things you never do

Never add a new number (a threshold, floor, depth, tolerance or band) to settle a finding. If a fix needs one, say so in your reply as a question for the user, in game terms, and leave it unfixed. Never change a displayed text, a score or a stored record unless the review file quotes a user decision for it. Never write a new copy while removing an old one: if your fix needs a helper, check the owner or `tests/` for one first. A comment that describes what a rule matches is a new copy too; name the owner instead (see "If you write code" in the preamble).

If a finding is wrong (the two copies are not the same question, or the reviewer misread the code), do not change the code. Say why in your reply, with the truth table that shows the difference.

## Slow proofs run once

Grep proofs and named tests run after each finding. Anything slower runs once, after the last finding: a corpus comparison, an old-against-new output comparison, a self-test over a whole tool. If that one run shows a change, rerun it at each of your commits to find which one.

## When you are done: run the precheck, then reply

Run `tools/derive_once_precheck.ps1` on the range and repo path. It needs no build. It must print nothing new. If it does, fix those the same way and commit them.

Then reply to the reviewer with SendMessage, to the agent id its message gave (load SendMessage first with ToolSearch, query `select:SendMessage`). Your reply gives:
- the new branch tip hash,
- for each finding: the owner, every copy you folded, and the commit hash, and for a kind A finding the scan row you added or why no row can catch it,
- every comment, header line and doc sentence you added or changed,
- the precheck's final output and the test commands you ran with their pass counts,
- anything you did not fix, and why.

Then end your turn with the same text as your report. Do not wait for an answer; the reviewer finishes the job.

## For the orchestrator

These rules are for the session that runs the merge.

Before the review, run the precheck on the range and fix what it prints. Keep merges small enough to review in one read. The precheck prints the range's size, and a large-range note when it is over the threshold; `tools/derive_once_precheck.ps1` owns that threshold. A range with that note should go in as two or more merges. The note is a warning, not a gate. When you plan, size each task so its diff stays under the precheck's threshold.

Give the reviewer the key, the range, the branch's worktree, the precheck output, the author's report, and the author's name or agent id (from its spawn result). Then wait for the reviewer's final report. The author's own completion notice arrives in between; it is not the end of the review. When the reviewer reports CLEAN, run the full suite once on the joined tree and merge. When it reports FINDINGS, the finding needs the user: take it to them. Never start another review round.
