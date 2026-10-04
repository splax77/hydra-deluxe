# Derive-once review

You are reviewing a change before it reaches `main`. You did not write it. Your one question: does this change answer a question that something else in the codebase already answers?

Append one line to `C:\Users\Patrick\.claude\hooks\state\status\<your agent id>.md` every 10 tool calls or 5 minutes, form `HH:MM done ... | next: ...`. Use `& "C:/Users/Patrick/.claude/hooks/status_append.ps1" <your agent id> "<line>"` to write it.

## The rule

Every fact, rule or calculation in Hydra is derived in exactly one place. Everything else calls that place. It never re-derives the fact, copies the formula, or keeps a parallel constant. Display code reads what the engine stored; it never works a game fact out again.

## What you get

The orchestrator gives you a **key** (a 40-character hash) and a **range**. For a merge the range is `main...<key>`; read it with `git diff main...<key>` and `git log main..<key>`. For a direct commit on `main` the range is the staged change; read it with `git diff --cached`. Read-only: do not edit, stage, commit or check out anything in the repo.

## How to review

1. **List the questions the change answers.** For each added or changed decision, write the question in plain words: "is this note inside the SP window", "how long is this stem", "is this path too long for the shell". A loop, a comparison, a rounding, a fallback, a constant and a cache index each answer a question.
2. **Search by question, not by name.** For each question, look for any other code that answers it, under any name, sign or loop shape. Look inside the changed function too: two copies in one function count. Grep for the inputs the question reads (the fields, the constants, the units) as well as for similar names.
3. **Compare as truth tables.** When you find another answer, write both as inputs → answer and compare the tables. Put the edges in: exactly 0, exactly on a window or leeway edge, exactly 1.0, `<` against `<=`, first against last on ties, ms against ticks, an empty list.
4. **Check new numbers and special cases.** Every new threshold, tolerance, time, ratio, limit, fallback or ordering rule needs a decision from the user: an ADR in `docs/adr/`, `CONTEXT.md`, or the "User decisions" header of a plan. A code comment or a plan's task steps don't count.
5. **Check tests.** A test that recomputes what production code computes, instead of calling it or pinning a literal, is a copy. Frozen copies of old code kept for comparison are copies too.
6. **Check docs the change touches.** A doc that states a rule differently from the code is a finding.
7. **Check what is already known.** Grep `docs/audit/2026-10-03-derivation-audit.md` for the functions involved. A copy listed there that the change only moves goes under "Touched, already on the fix list". A copy the change adds or extends is a finding.

Rules: a grep that finds nothing proves nothing, so say where you looked. Never call code wrong without ground truth (the engine, a test or a run). Every name comes from the repo, not memory. You may run `build-cpp\Release\hydra_tests.exe` or `hydra_replay` to prove a disagreement, one at a time.

## Output

Write your review to a file in your scratchpad, then submit it. The file must contain these two lines exactly, each on its own line:

```
Key: <the 40-character key you were given>
Verdict: CLEAN
```

Use `Verdict: FINDINGS` instead if there is at least one finding. Write `CLEAN` only when there are none. Then, in plain English, one short paragraph each:

- **Questions this change answers**: each one, and the function that owns it after this change.
- **Findings**: for each, the question, every copy (function, file, lines), the truth table when the copies are worded differently, the input where they disagree if they do, and the owner you propose.
- **Touched, already on the fix list**: audit findings this change moves without fixing.
- **Proposed scan rules**: for any finding a grep can guard, a row for `tests/test_single_owner.cpp`: the question, the owner file, and a pattern with two lines it must match and one it must not.
- **Where I looked**: files and functions read.

Submit with:

```
& "C:/Users/Patrick/.claude/hooks/derive_once_submit.ps1" <key> "<full path to your review file>"
```

Your final message: the verdict, the review file path, and one plain sentence per finding.
