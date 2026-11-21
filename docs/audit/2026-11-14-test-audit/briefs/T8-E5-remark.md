# Brief: T8 E5, re-mark one batch of side notes from their files

Session f3bbb32b-5f4d-4400-8cda-86f04d67f2a4. Your task id is `T8-E5-<batch>`, where the batch is the one your dispatch names (such as `R01`). AUD is `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\t8-m1\docs\audit\2026-11-14-test-audit`.

Read first, in this order:

1. `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md`. It binds you. Its status rule matters most: append a status line every 5 minutes with `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`, alone in its own PowerShell call.
2. Your batch: `AUD/exits/notes/remark/<batch>.tsv`. Each row is one side note a script pulled from an audit file. The `note` column is `file:line` under AUD. `old_mark` and `old_note` are a first scout's mark, made without opening the file, so they settle nothing.

## Why this step exists

The audit's agents wrote side notes, splits and production-code findings into their record and verify files. Audit-method says a side note is a finding too, and it needs an exit like any other: in a past audit five parked side notes turned out to be four real bugs. A first round of scouts marked these notes from the batch text alone, without opening the record, and a spot check found five of six such marks wrong. Your job is to mark each note again, from the file.

## What you do

For each row, open the note in its file with the Read tool, by range, with enough of the record around it to see its verdict. Rows in one file sit close together, so one Read by range covers many of them; read each file's stretch once, in row order. Then give the note one mark:

- `repeat`: it repeats a fault the same record's verdict already names (the fake, pin, echo or citation the record judged). The record's own verdict covers it. Check the verdict before you use this mark.
- `split`: two parts of Hydra, or a test and Hydra, answer one question differently. Name both parts.
- `prod`: a fault in production code (`src/` or `tools/`), not in a test. Name the file and line the note gives.
- `finding`: any other problem with a test, a record, a doc or a citation that is not the record's verdict (a stale line, a wrong citation, a name that disagrees with the body, a missing test, an open point).
- `empty`: not a finding at all: a heading with no body, a note that says "none", or a duplicate row of the same note (name the row it duplicates).

A note that mixes several items takes the mark of its most serious item (prod, then split, then finding), and its sentence names the others.

## What you write

One file, `AUD/exits/notes/remark/<batch>.md`, with the Write tool, and nothing else in AUD. Do not commit. Its parts:

- A first line: `Agent id: <your id> (task T8-E5-<batch>, session f3bbb32b)`.
- A table with one line per batch row, in batch order: `| note | mark | what it says, one sentence |`. Copy `note` exactly from the batch. The sentence names the file and line of any production code or second owner the note gives.
- A list "Changed from the first mark": each row whose mark differs from `old_mark`, with both marks.

Every batch row must have a line in the table; check by script that the table's `note` column equals the batch's, in order, before you finish. A script may do that check and may copy the `note` column, but every mark is your judgement from reading the note in its file. A row you did not open is `not read`, never a mark guessed from the batch text.

## Rules

- You mark facts. You pick no exit. There are four exits (fixed, a question to the user, covered by a ruling, not a problem), and only the main session picks one. Your file has no exit column.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence. A note that says something was kept on purpose is still a finding if it names a problem.
- A search that finds nothing proves nothing on its own.
- Read only. You run nothing: no test, no build, no `hydra_*` tool, never a whole-library run. You may open a `src/` or `tools/` file to read the line a `prod` note names, but you do not have to.
- The user's words may be cited only from lines under `## Confirmed` in `docs/agents/rulings/` and from `AUD/questions.md`.
- Take exactly the rows of your batch. Looking for notes the script missed is not part of this step.
- Keep any scratch scripts in your own folder, not a shared one.

## Caps

The progress hook warns at 100 calls or 20 minutes and stops you at 150 calls or 30 minutes. At the warning, write what you have, mark unreached rows `not read`, and finish.

## Your final report

Return the file path, the count of rows by mark, the count of `not read` rows, and the count of rows whose mark changed.
