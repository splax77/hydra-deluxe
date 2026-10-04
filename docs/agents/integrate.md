# Integrate

You join a wave's task branches onto the wave branch, so the derive-once review and the main session's one suite run see a single tree that builds and holds together. Read `docs/agents/brief-preamble.md` first; its rules apply to you in full. Your brief names the wave branch, its worktree, the task branches in the order to join them, and the fork point (the `main` commit the wave branch started from).

You write no new feature code. You only fix what the join itself breaks: a conflict, a renamed function, a scan row or doc line that one task's change made stale for another's.

## Why this brief exists

`main` keeps moving while a wave runs, because other sessions merge to it too. A task branch forked before `main` gained an owner function (the one place a rule is worked out) can carry its own copy of that rule. Nothing in the task's own tests notices. The review then finds it as a kind A copy, and a whole round goes to a copy the join could have caught in a minute.

## Step 1: list what `main` gained since the fork

Run `git log --first-parent <fork>..main --oneline` to see every commit `main` gained since the wave forked. For each one that touches code, note the owner functions it added or moved, from its subject, its diff and its ADR or `CONTEXT.md` lines.

Then run `git diff <fork>..main -- tests/test_single_owner.cpp`. Each scan row those commits added names a question and its owner file. Together these are the owners the wave branch must call.

If `main` gained nothing since the fork, say so and skip to step 2.

## Step 2: check each task branch calls those owners

For each owner from step 1, grep the task branch's diff (`git diff <fork>...<task branch>`) for the inputs that owner reads: its fields, constants and units, not only its name. A task line that answers the same question without calling the owner is a copy. Fold it into the owner in the join, and name it in the merge commit body.

If a fold would change a displayed text, a score or a stored record, stop and report it. Do not pick.

## Step 3: join the branches

Merge each task branch onto the wave branch in the order your brief gives, one at a time, with `git merge --no-ff --no-commit <task branch>`. Resolve the join, then commit it with separate `-m` arguments and the preamble's trailers, never on stdin. Hold the last task's merge uncommitted until step 4 passes, so the join fixes land in that merge commit.

When two tasks both append rows to `tests/test_single_owner.cpp`, git merges the appended blocks badly. Rebuild the file as the wave branch's version plus the other task's added rows, relative to the fork. Do the same for any other file two tasks both appended to.

Resolve every conflict inside the merge commit. Never rebase, amend, reset or force-move a branch. If a conflict is between two different answers to one question, not just two edits to nearby lines, stop and report both sides.

## Step 4: check the joined branch

Build the joined branch once with the target your brief names.

Run the precheck: `tools/derive_once_precheck.ps1` on the range `main...<wave branch>` and the worktree path. It needs no build. Fix every item it prints that the join caused, and list the rest for the reviewer.

Run the scan test and the docs test with their filters, and nothing else:

- the scan test: `build-cpp\Release\hydra_tests.exe -tc="single-owner*"`;
- the docs test, once it exists: `build-cpp\Release\hydra_tests.exe -sf=*docs_match_code*`.

Also run any test case your brief names. Never run the full suite. The main session runs it once, after the review.

When the scan or docs test fails because one task renamed or moved a name another task's rows or docs still use, fix it in the join, stage the fix by name, and rerun only the failing test. Then commit the last merge, with a body saying which task's change broke which line and how you fixed it. Never loosen a scan row to make it pass.

If a join break shows up only after an earlier merge is already committed, fix it in the next merge commit instead. Never amend.

## Step 5: hand over to the review

Stop changing the tree once step 4 passes. The gate keys a review on the exact tree, so any later commit voids it.

Report the wave branch's head commit hash and the range `main...<head>`. The main session takes the exact key from the gate's message and hands the key, the range and your precheck output to a fresh Sonnet reviewer with `docs/agents/derive-once-review.md`. You are an author of the merge commits, so you cannot review this range yourself.

## Your report

Give the wave branch, its worktree, and its head commit hash. List each task branch you joined with its merge commit hash. Then, in plain sentences: the owners `main` gained since the fork, any task copy you folded into one of them, each conflict and how you resolved it, the precheck's output, and the exact scan and docs test commands with their pass counts. End with anything you stopped on.
