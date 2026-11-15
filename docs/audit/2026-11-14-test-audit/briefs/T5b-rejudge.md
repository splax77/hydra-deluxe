# Brief: re-judge clean records against the capture runs

Task id: test-audit-T5b-rejudge. Session id: f9b4f64e-df75-4ee2-b5f3-9f2bc5dfc7aa. Your slice (J1 to J4) and its records are in your prompt.

Read `docs/agents/brief-preamble.md` first; every rule there binds you. Then read `docs/agents/audit-method.md`, "The four exits" and "Verify". You are a read-only scout: change nothing in any repo. Write only your one output file, named below. Write a status line every 5 minutes with `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`, alone in its own call.

The checkout is `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records` (branch `claude/test-audit-records`). Every path below is relative to it. The audit folder is `docs/audit/2026-11-14-test-audit/` (call it AUD).

## What this is

The audit gave every test a record (`AUD/records/<group>.md`, one file per group id like `A17` or `B7`; a record is found by its item, such as `16` or `T3` or `#4`). The user's rulings on what a test may rest on were applied in the recount (`AUD/counts/TOTALS.md`). Your records passed that recount: none holds a ruled fault. Each is "Investigate" for one reason only. Its test makes a claim about what Hydra does, and nobody had checked that claim against a real run.

The main session has now run real Hydra on real data (`testdata/input`, `testdata/store`, the real reflog). The outputs are in `AUD/capture/runs/<run>/`, and `AUD/capture/runs/index.md` says what each run did and what it could not show. `AUD/capture/CLEAN.md` says which run serves each record. `CLEAN.md`'s commands say `capture/out/`; the outputs are really in `capture/runs/`.

Your job: for each record in your slice, read the record, read the test it names, and read the run output that serves it. Then say whether the real run shows the test's claim.

## The verdicts

Give each record exactly one:

- **SHOWN:** the run output shows what the test claims. Quote the output line or lines (file and line number), and say which assertion of the test they match. For a screenshot, say which picture and what in it.
- **CONTRADICTED:** the run output shows something other than what the test claims. Quote both: the test's line and the output's line.
- **NOT SHOWN:** the output holds nothing that settles the claim. Say why in one sentence (the run never reached that state, the claim is internal with no visible effect, the record was filed under the wrong run), and name the run that would settle it, if one could.

A verdict is about the claim, not about how the test is written. If a capture shows the claim, but the test still pins a typed number, uses a fake, or cites something that is not code, write SHOWN and add a side note naming the fault. Your verdict is not an exit. Only the main session picks an exit (`audit-method.md`), so never write "keep", "close", "fix" or "delete".

Never write SHOWN by reading the code. The output has to hold it. "The code does X, so the run would show X" is NOT SHOWN.

Never write SHOWN from a run's notes (`notes.md` in a capture folder). A scout wrote each `notes.md` before the run, as a prediction. Only `output.txt`, the dumps, the `.txt` outputs, the `.db` files and the screenshots are evidence.

A `.db` in `capture/runs/R5/` is kept out of git but sits on disk; read it with `py -I` and the `sqlite3` module, read-only (`file:<path>?mode=ro` with `uri=True`).

## What to write

Write `C:\Users\Patrick\AppData\Local\Temp\claude\f39aa8e9e045be0e\f9b4f64e-df75-4ee2-b5f3-9f2bc5dfc7aa\scratchpad\rejudge\<your slice>.md`. Start with one line naming your slice and agent id. Then one table: record (group and item), test (file:line), run, verdict, evidence. After it, a "Side notes" section: every fault, misfiled record or wrong line number you saw, with file and line. A side note is a finding too.

Then a count line: how many SHOWN, CONTRADICTED and NOT SHOWN.

## Caps

At most 100 tool calls and 20 minutes. If you reach a cap, write the rows you have, mark every record you did not reach as "not reached", and report. Never skip a record silently.

## Report

Return the file path, the count line, every CONTRADICTED row in full, and the side notes. Keep it under 500 words.
