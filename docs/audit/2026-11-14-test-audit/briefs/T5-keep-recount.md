# Brief: re-read every Keep record by the user's three test rulings

Read `docs/agents/brief-preamble.md` first, in the worktree `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records` (branch `claude/test-audit-records`). Its rules bind you. Session id: 63c70014-4328-49dc-9205-742235732752. Your task id and your record files are in your prompt.

## Why

The audit judged each test case by `docs/agents/test-rubric.md`, and 421 records ended "Keep". The rubric has no question for three faults the user ruled on today, filed in `docs/agents/rulings/tests-and-runs.md` under `## Confirmed`:

- "tests should not be pinned to typed constants. they should be pinned to the behavior of the code."
- "tests exist to verify the behavior of the code. a test citing something that isn't code is always wrong"
- "tests should test their intended behavior directly, not make up an arbitrary meaningless string"

On fakes the user added, in chat: "fake data will never appear in the real hydra and we should be testing the behavior of the real code".

Read X9 to X12 in `docs/audit/2026-11-14-test-audit/cross-findings.md` for worked examples of each fault. Your job is to count how many Keep records hold one of them.

## Your job

For every record in your files whose verdict is `Keep`, open the test case it names, in the code, and read every check. Mark each fault you find:

- **X9, made-up input.** The test feeds made-up engine output or made-up outside data to the code under test: a hand-built Activation, summary row, path text, score or record; a hand-written reply from a service; a fake analyzer's invented result. This holds even when the expected value follows correctly from the made-up input. Real input is the engine's output on a real chart, or data saved from the real source. A chart the test builds is input to the engine, not X9; what the engine returns from it is not made up.
- **X10, constant pin.** An expected value that is a typed product constant, table or text (a window, a cap, a tier edge, a message, a label, a key name) instead of what the code does with it. Reading the constant by name in order to check behaviour is not X10. Checking that the constant equals a typed number is.
- **X11, echo.** A value typed in and required back unchanged, so the check passes whatever the code does.
- **X12, cited non-code.** A check whose expected value names its source as something that is not code: "from one run", "pinned", "the engine's record", a saved run, a plan, a mockup, a doc, a person's chart.
- **Snapshot.** An expected number pinned from one run, with no rule and named inputs it follows from, whether or not a comment says so (`docs/agents/test-rubric.md:23`).
- **None.** The test checks the code's behaviour on real or rule-built input.

For hook and tool tests (Python, PowerShell), the same faults apply: a hand-written stand-in for a real transcript, state file, tool call or game reply is X9; a test that checks a typed constant or message text is X10.

For each faulty record, also say whether the record itself already named that fault anywhere in its text.

Never trust the record's reasons or the test's comments: read the code under test to see what it receives and what it does.

## File

Write `docs/audit/2026-11-14-test-audit/counts/<your task id>.md`, with your agent id on its first line. First a table, one line per Keep record: record file, record number, test `file:line`, faults found (X9, X10, X11, X12, snapshot or none), the `file:line` of each faulty check, one-line reason, and whether the record named it. Then counts: Keep records read, records with at least one fault, records per fault, records whose fault the record already named. Stage only your file by name, check `git diff --cached --name-only` shows only it right before you commit, and retry on `index.lock` or a moved HEAD; other agents commit to this branch at the same time. Commit with the preamble's trailers, every 25 records or so.

## Rules

You run no hydra_* program, no hook test and nothing that builds. You choose no exit and fix nothing. A search that finds nothing proves nothing on its own; widen it and say where you looked. Status line every 5 minutes with `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`, alone in its own call. At the progress hook's warning (100 calls or 20 minutes), commit and list the records you did not reach.

## Report

Return the commit hash, the counts above, the five records with the clearest faults (one line each), and the records you did not reach.
