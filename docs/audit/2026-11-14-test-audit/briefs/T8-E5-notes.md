# Brief: T8 E5, mark one batch of side notes, splits and production-code findings

Session f3bbb32b-5f4d-4400-8cda-86f04d67f2a4. Your task id is `T8-E5-<batch>`, where the batch is the one your dispatch names (such as `N01`). AUD is `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\t8-m1\docs\audit\2026-11-14-test-audit`.

Read first, in this order:

1. `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md`. It binds you. Its status rule matters most: append a status line every 5 minutes with `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`, alone in its own PowerShell call.
2. Your batch: `AUD/exits/notes/batches/<batch>.tsv` (the notes a script pulled) and `AUD/exits/notes/batches/<batch>-files.txt` (the files they came from, AUD-relative).

## Why this step exists

The audit's agents wrote side notes, splits and production-code findings into their record and verify files. Audit-method says a side note is a finding too, and it needs an exit like any other: in a past audit five parked side notes turned out to be four real bugs. A script (`AUD/exits/ledger.py`) pulled every block it could find into `AUD/exits/notes.tsv`. It matched a few line shapes, and a search that misses a shape proves nothing. Your job is to mark each pulled note, and to find the notes the script missed in your files.

## What you do

**Mark each note in the batch.** Open the note in its file (the `note` column is `file:line`, under AUD) and read it with enough of the record around it to understand it. Give it one mark:

- `repeat`: it repeats a fault the same record's verdict already names (the fake, pin, echo or citation the record judged). The record's own verdict covers it.
- `split`: two parts of Hydra, or a test and Hydra, answer one question differently. Name both parts in your note.
- `prod`: a fault in production code (`src/` or `tools/`), not in a test. Name the file and line the note gives.
- `finding`: any other problem with a test, a record, a doc or a citation that is not the record's verdict (a stale line, a wrong citation, a name that disagrees with the body, a missing test).
- `empty`: not a finding at all: a heading with no body, a note that says "none", or a duplicate row of the same note (name the row it duplicates).

**Find what the script missed.** Read every file in your `-files.txt` list once, whole or by range, with the Read tool. A search for note shapes by script is what the ledger script already did, so a file you only searched is `not reached`. Keep any scratch scripts in your own folder, not a shared one. Look for any side note, split, production-code finding, open point or "not checked" item written in a shape the script did not catch: a different heading, a bullet under "Notes", a sentence inside a verdict. Each one you find becomes a new row. Rows already in your batch need no new row.

## What you write

One file, `AUD/exits/notes/<batch>.md`, with the Write tool, and nothing else in AUD. Do not commit. Its parts:

- A first line: `Agent id: <your id> (task T8-E5-<batch>, session f3bbb32b)`.
- A table with one line per batch row, in batch order: `| note | mark | what it says, one sentence |`. Copy `note` exactly from the batch.
- A table of new rows: `| file:line | mark | what it says, one sentence |`.
- A list "Files read": every file in your `-files.txt`, each marked `read` or `not reached`.

Every batch row must have a line in the first table; the main session checks by script.

## Rules

- You mark facts. You pick no exit. There are four exits (fixed, a question to the user, covered by a ruling, not a problem), and only the main session picks one. Your file has no exit column.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence. A note that says something was kept on purpose is still a finding if it names a problem.
- A search that finds nothing proves nothing on its own.
- Read only. You run nothing: no test, no build, no `hydra_*` tool, never a whole-library run.
- The user's words may be cited only from lines under `## Confirmed` in `docs/agents/rulings/` and from `AUD/questions.md`.
- Take exactly the rows and files of your batch.

## Caps

The progress hook warns at 100 calls or 20 minutes and stops you at 150 calls or 30 minutes. At the warning, write what you have, mark unreached rows `not read` and unreached files `not reached`, and finish. The main session sends the rest to a fresh scout once, by exact row and file.

## Your final report

Return the file path, the count of batch rows by mark, the count of new rows by mark, and the files not reached.
