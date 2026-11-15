# Brief: judge clean records by reading the code

Task id: test-audit-T5c-by-reading. Session id: f9b4f64e-df75-4ee2-b5f3-9f2bc5dfc7aa. Your batch name and its record file are in your prompt.

Read `docs/agents/brief-preamble.md` first; every rule there binds you. Then read `docs/agents/audit-method.md`, "The four exits" and "Verify". You are a read-only scout: change nothing in any repo, and write only your one output file. Write a status line every 5 minutes with `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`, alone in its own call.

The checkout is `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records` (branch `claude/test-audit-records`); its `src/`, `tests/` and `tools/` match main's code. The hook code is in `C:\Users\Patrick\.claude\gates` (read it, never change it). The audit folder is `docs/audit/2026-11-14-test-audit/` (AUD).

## What this is

Every test has a record in `AUD/records/<group>.md` (find a record by its item, such as `16`, `T3` or `#4`). The records in your batch broke none of the user's test rulings in the recount (`AUD/counts/TOTALS.md`). Each is open for one reason: its test claims Hydra does something, and nobody checked that claim. Real runs could not settle most of them. The user's instruction now: "just proceed without captures for the remaining and use your best judgement with static code analysis".

So for each record you read the test, then the production code it is about, and say whether the code does what the test claims.

## How to judge one record

1. Open the record and the test it names (column 3; short names like `click.cpp` or `rep.cpp` are test files under `tests/`, find them with Glob). Write down the test's claim in one plain sentence: the input it gives and what it says must happen.
2. Find the production function the claim is about, in `src/`, `tools/` or the gates folder. Check the test calls that real function. If it calls a stand-in or a copy, say so in a side note.
3. Trace the test's own input through the production code, by reading, to its result. Name the function, file and lines you followed.
4. If a capture run in `AUD/capture/runs/` served the record (column 5, and `AUD/capture/runs/index.md`), read its `output.txt` too. Output beats reading: a line in it that shows or contradicts the claim decides the verdict. A `notes.md` there is a prediction written before the run, never evidence.

## The verdicts

Give each record exactly one:

- **SHOWN BY RUN:** a capture output holds the claim. Quote the output file and line.
- **HOLDS BY READING:** the production code, traced with the test's input, gives what the test claims. Name the function and the lines. Say "by reading" in the row.
- **CONTRADICTED:** the code or a run gives something other than the claim. Quote the test line and the code or output line. This is the most important verdict: look hard for it. A test can pass while claiming the wrong thing, for example when its input never reaches the branch it names.
- **CANNOT TELL:** the claim depends on something the code alone does not settle, such as the live game, a file outside the repo, or a timing race. Say exactly what would settle it.

A verdict is about the claim, not how the test is written. A fault in how it is written (a typed number it pins, a fake, a citation of something that is not code, a made-up string) goes in a side note, with the line. The user's rulings on those are in `docs/agents/rulings/tests-and-runs.md` on main (`git -C C:\Users\Patrick\Downloads\Hydra\hydra-test show main:docs/agents/rulings/tests-and-runs.md`).

Your verdict is not an exit. Only the main session picks an exit (`audit-method.md`), so never write "keep", "close", "fix" or "delete". "On purpose", "by design", "deliberately" and "kept" in a comment are claims, not evidence. Never trust a comment's account of what the code does; read the code. A search that finds nothing proves nothing on its own.

## What to write

Write `C:\Users\Patrick\AppData\Local\Temp\claude\f39aa8e9e045be0e\f9b4f64e-df75-4ee2-b5f3-9f2bc5dfc7aa\scratchpad\byreading\<batch>-verdicts.md`. Start with one line naming your batch and agent id. Then one table: record (group and item), test (file:line), the claim in a few words, verdict, evidence (function, file and lines, or output line). Then "Side notes": every fault, misfiled record, duplicate or wrong line number you saw, with file and line. Then a count line per verdict.

Write the file once you have judged about half your records, then update it at the end, so a cap never loses your work.

## Caps

At most 100 tool calls and 20 minutes. Spend about two calls per record: Grep to find the lines, then Read just those lines. If you reach a cap, write every row you have, mark each record you did not reach "not reached", and report. Never skip a record silently.

## Report

Return the file path, the count line, every CONTRADICTED row in full, and the side notes. Keep it under 500 words.
