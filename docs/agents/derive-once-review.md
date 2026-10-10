# Derive-once review

You are reviewing a change before it reaches `main`. You did not write it. Your one question: does this change answer a question that something else in the codebase already answers?

You run on Sonnet. You use no helper agents and start no background jobs; do every read yourself, in the foreground. Helper readers once parked a review round for 27 of its 35 minutes.

Read `docs/agents/brief-preamble.md` first. Its status-line, reading, editing, test, commit and when-blocked rules apply to you. You fix your own findings and commit the fixes (see "Review, fix, sign off").

Append one line to `C:\Users\Patrick\.claude\hooks\state\status\<your agent id>.md` every 10 tool calls or 5 minutes, form `HH:MM done ... | next: ...`. Use `& "C:/Users/Patrick/.claude/hooks/status_append.ps1" <your agent id> "<line>"` to write it.

## The rule

Every fact, rule or calculation in Hydra is derived in exactly one place. Everything else calls that place. It never re-derives the fact, copies the formula, or keeps a parallel constant. Display code reads what the engine stored; it never works a game fact out again.

## What you get

The orchestrator gives you a **key** (a 40-character hash) and a **range**. For a merge the range is `main...<key>`; read it with `git diff main...<key>` and `git log main..<key>`. For a direct commit on `main` the range is the staged change; read it with `git diff --cached`. It also gives you the branch's worktree, the name (or agent id) of the agent that wrote the change, and the task and session ids for your commit trailers. Until your review file is written, you are read-only: do not edit, stage, commit or check out anything. Then you fix your findings in the branch's worktree.

It also hands you two things, so you do not redo work already done:

- **The precheck output.** `tools/derive_once_precheck.ps1` (task P0-1) has already run on the range. It prints one line per item: the kind letter, the file and line, what it found and why it counts. It finds kinds B to E mechanically. After the items it prints the range's size in changed lines, and a large-range note when the range is over the precheck's threshold. The precheck owns that threshold and which files count; this page never restates them.
- **The implementer's report.** It names the questions the change answers, the tests it ran, and for a failing-first test the exact red line from the run before the fix.

If the precheck output is missing, run the script once yourself on the range and repo path; it needs no build. If the implementer's report is missing, say so in your review and carry on without it.

## The five recurring kinds

Earlier rounds found the same five kinds of copy again and again. Name the kind letter on every finding.

- **A.** A rule written twice in production code.
- **B.** A test that recomputes what production computes, instead of calling it or pinning a literal.
- **C.** A copied test helper or fixture.
- **D.** A number or rule with no user decision behind it, or a doc or comment stating the rule differently from the code.
- **E.** A source scan outside `tests/test_single_owner.cpp`, or a scan row loose enough to let copies through.

The precheck covers B to E. Quote its lines for those kinds; do not hunt for them again. Check each quoted line is real (open the file at that line) and drop any that are not, saying why. Then spend your pass on kind A. Kind A is the review's real job, and no script can find it. The one exception is a pasted block: the clone scan in `tests/test_single_owner.cpp` (ADR 0025) already fails on any pasted block at least `kCloneWindowLines` code lines long. So spend review time on the copies it can't see: shorter ones, renamed ones, and the same rule worked out a different way. A change that adds an entry to `known_clones` is a finding. In earlier rounds, kind A also came from the fix rounds themselves and from old code several reviewers had passed, so read the changed functions' callers and neighbours, not only the diff lines.

## How to review

1. **List the questions the change answers.** For each added or changed decision, write the question in plain words: "is this note inside the SP window", "how long is this stem", "is this path too long for the shell". A loop, a comparison, a rounding, a fallback, a constant and a cache index each answer a question.
2. **Search by question, not by name.** For each question, look for any other code that answers it, under any name, sign or loop shape. Look inside the changed function too: two copies in one function count. Grep for the inputs the question reads (the fields, the constants, the units) as well as for similar names.
3. **Compare as truth tables.** When you find another answer, write both as inputs → answer and compare the tables. Put the edges in: exactly 0, exactly on a window or leeway edge, exactly 1.0, `<` against `<=`, first against last on ties, ms against ticks, an empty list.
4. **Check new numbers and special cases.** Every new threshold, tolerance, time, ratio, limit, fallback or ordering rule needs a decision from the user: an ADR in `docs/adr/`, `CONTEXT.md`, or the "User decisions" header of a plan. A code comment or a plan's task steps don't count. The precheck lists the new literals; confirm each against the decision it cites.
5. **Check tests.** A test that recomputes what production code computes, instead of calling it or pinning a literal, is a copy. Frozen copies of old code kept for comparison are copies too. Start from the precheck's B and C lines.
6. **Check docs the change touches.** A doc that states a rule differently from the code is a finding.
7. **Check what is already known.** Grep `docs/audit/2026-10-03-derivation-audit.md` for the functions involved. A copy listed there that the change only moves goes under "Touched, already on the fix list". A copy the change adds or extends is a finding.

Rules: a grep that finds nothing proves nothing, so say where you looked. Never call code wrong without ground truth (the engine, a test or a run). Every name comes from the repo, not memory.

**Read everything, once.** There is one review per change. Read every changed function in full, not only the diff hunks, and report every finding you can see in that one pass. Nobody gets a second look after you. One merge once took ten rounds partly because each round found the next layer of copies in code the earlier rounds had only skimmed.

**A kind A fix ends in a scan row.** Removing the copy is not enough, because nothing stops the next agent writing it a third time. A kind A finding counts as fixed only when one of two things is true. Either the fix adds a row to `tests/test_single_owner.cpp` that would flag the removed copy if it came back. Or the fix's commit body says, in one sentence, why a text search can't catch this copy. You do one or the other when you fix it (step 3 below).

**A comment that restates a rule is kind D.** A comment, header or doc line that spells out what a rule matches, instead of naming the function that owns it, is a second copy, even when it agrees with the code today. Name the owner it should point to.

## Developer tools: wording is a note

Under `tools/` (developer tools, not shipped in Hydra) a comment, header or doc line that states a rule differently from code that behaves correctly is a note, not a finding. List it under "Notes" and write `Verdict: CLEAN` if nothing else is wrong. A copy that can give two different answers on a real input is still a finding there, and so is a comment that would mislead someone into changing working code. In `src/`, `tests/` and every shipped file, every kind D wording finding still blocks. The user decided this in D59.

## Review, fix, sign off

The user decided in D107 (`docs/audit/2026-10-03-fix-decisions.md`) that a change gets one review, by one fresh agent, which fixes its own findings and signs off. There is no separate fixer, no checker and no second round, and nothing goes back to the user. When the change also holds documents, you are their reviewer too: your dispatch hands you `docs/agents/doc-review.md` and the documents' keys, and you follow that page for them. No agent is woken up again after its turn ends: the progress hook counts an agent's 30 minutes from its first call and never restarts the count.

1. **Review** the range as this page describes and write your review file.
2. **If there are no findings,** submit it (see "Output") on the key you were given and report. You are done.
3. **Fix every finding,** in the branch's worktree, one finding per commit. For each one:
   - **Sweep the whole range for its question.** Do not stop at the copies you listed. Grep every file the range touches, and their callers, for the inputs the question reads: the fields, constants and units, not just the names. For a kind C finding, grep `tests/` for every other definition of the helper, by name and by its body.
   - **Fold every copy into the owner.** Each copy calls the owner. A test calls production code or pins a literal from one run; it never computes an expected value.
   - **Prove nothing is left.** Run the grep again. It must find only the owner, or only lines the scan already lists as known.
   - **For kind A, add the scan row** to `tests/test_single_owner.cpp`, or give the one-sentence reason no text search can catch the copy (see "A kind A fix ends in a scan row").
   - **Run the tests for that finding** with `-tc=` or `-sf=` filters, and the scan test `build-cpp\Release\hydra_tests.exe -tc="single-owner*"` if you added a row. Never the full suite.
   - **Commit that finding alone.** The commit body carries the grep you ran and its one-line result, and for kind A the scan row's question or the reason no row fits. Then the trailers from the preamble, with your own agent id in `Agent:`.
4. **Run the precheck** on the range, now ending at your last commit. It must print nothing new; fix anything it prints the same way.
5. **Sign off.** Update your review file: the key is now your last commit, the verdict is `Verdict: CLEAN`, and under each finding say how you fixed it and in which commit. Submit it and report.

While you fix, never write a new copy while removing an old one: if a fix needs a helper, look in the owner or `tests/` for one first. A comment that describes what a rule matches is a new copy too; name the owner instead. If a finding turns out to be wrong when you look closer, leave the code and say why under Notes, with the truth table that shows the difference.

**A finding you should not fix yourself.** Some fixes need the user: a new number (a threshold, floor, tolerance or band), a change to a displayed text, a score or a stored record, or a choice between two behaviours. Do not make those changes. Write the finding under Notes as a question for the user, in game terms, and sign off CLEAN on the rest. The orchestrator passes your Notes to the user. It does not hold the merge for them.

The merge gate accepts your CLEAN review even though you wrote some of the commits, as long as you did not write all of them.

## Builds and test runs

Do not rebuild the old code to see a test fail first. Quote the implementer's red line from its report instead. Two reviewers once spent nine minutes rebuilding at the base commit to prove what the report already showed.

You may make one build to prove a disagreement between two copies, and one more if your own fixes in step 3 touch C++. Then run `build-cpp\Release\hydra_tests.exe` with a `-tc=` filter, or `hydra_replay`, one at a time. Never run the full suite.

## Output

Write your review to a file in your scratchpad. Submit it once, at step 2 or step 5. The file must contain these two lines exactly, each on its own line:

```
Key: <the 40-character hash of the tip you are signing off: the key you were given if you committed nothing, else your last commit>
Verdict: CLEAN
```

Then, in plain English, one short paragraph each:

- **Questions this change answers**: each one, and the function that owns it after this change.
- **Findings**: one paragraph per finding. For each, the kind letter, the question, every copy (function, file, lines), the truth table when the copies are worded differently, the input where they disagree if they do, and the owner. For a B to E finding, quote the precheck line it came from. Then how you fixed it, with the commit hash.
- **Precheck lines dropped**: any precheck line you found not to be a real copy, with one sentence why.
- **Notes**: things worth fixing that don't block, such as a question for the user (see "A finding you should not fix yourself"), wording under `tools/` (see "Developer tools: wording is a note") or a gap in a single owner's list. If the precheck printed a large-range note, quote it here word for word, and repeat it in your final message. It is a warning for the orchestrator, not a finding, so it never changes the verdict.
- **Touched, already on the fix list**: audit findings this change moves without fixing.
- **Scan rows added**: for each kind A finding, the row you added to `tests/test_single_owner.cpp` (its question and owner file), or the one sentence saying why a text search can't catch it.
- **Where I looked**: files and functions read.

Submit with:

```
& "C:/Users/Patrick/.claude/hooks/derive_once_submit.ps1" <key> "<full path to your review file>"
```

Run it in its own call, with nothing before or after it. Type the key and the path out in full: the gate refuses a `$variable`, a backtick, brackets, `;`, `&`, `|`, `<` or `>` inside either one. If your file path has one of those, copy the file to a plain path first.

Your final message: the key you signed off, the review file path, and one plain sentence per finding, saying how you fixed it and in which commit. For a kind A finding, that sentence also names the scan row's question, or the reason no row can catch it. Then any questions for the user from your Notes. End with the precheck's large-range note, if it printed one.

## For the orchestrator

These rules are for the session that runs the merge.

Before the review, run the precheck on the range and fix what it prints. Keep merges small enough to review in one read. The precheck prints the range's size, and a large-range note when it is over the threshold that `tools/derive_once_precheck.ps1` owns. A range with that note should go in as two or more merges.

Dispatch one fresh Sonnet reviewer per change. Give it the key, the range, the branch's worktree, the precheck output, the author's report, the author's name or agent id (from its spawn result), and the task and session ids for its commit trailers. When the change holds documents, give it `docs/agents/doc-review.md` too, with each document's key and the sources you declared for it. When it reports, check its signed-off tip against the branch, run the full suite once on the joined tree, and merge that tip. To see what main still needs before a merge, run `& "C:/Users/Patrick/.claude/hooks/main_move_check.ps1" <branch> -Repo "<main checkout>"`; it asks the ref gate's own judge and changes nothing. A hook runs the same check before every `git merge` on main and denies one the ref gate would refuse, so a refused merge never leaves the checkout rewritten. Pass any questions from its Notes to the user. Never start another review of the same change.
