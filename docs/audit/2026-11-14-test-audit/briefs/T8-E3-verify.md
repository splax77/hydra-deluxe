# Brief: T8 E3, verify two scouts' label files

Session f3bbb32b-5f4d-4400-8cda-86f04d67f2a4. Your task id is `T8-E3-V-<first batch>-<second batch>`, from your dispatch. AUD is `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\t8-m1\docs\audit\2026-11-14-test-audit`.

Read first, in this order:

1. `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md`. It binds you. Its status rule matters most: append a status line every 5 minutes with `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`, alone in its own PowerShell call.
2. `AUD/briefs/T8-E3-labels.md`, the brief the scouts worked to.
3. `AUD/exits/keys.md`, the closed key list, including "Revision 1".
4. For each of your two batches: `AUD/exits/labels/batches/<batch>.tsv` and the scout's file `AUD/exits/labels/<batch>.md`.

You labelled neither file. If your agent id is the one on the first line of either label file, stop and report it.

## What you do

For every row of both batches, check the scout's keys against the row's evidence: the counter's reason for a `count` row, the record's own text for a `record` row (open it at the line in `row`, under AUD), and the test where those are not enough. A row's keys are right when each one fits the definition in `keys.md` and no fault the evidence names is left without a key. Each fault mark on a count row (X9, X10, X11, X12, snapshot) needs at least one key of its family.

Check also that every batch row has exactly one line in the label file, and that the ends table names, for each key, rows that really are in the batch and carry that key.

## What you write

One file, `AUD/exits/labels/V-<first batch>-<second batch>.md`, written with the Write tool and nothing else in AUD. Do not commit. Its parts:

- A first line: `Agent id: <your id> (task T8-E3-V-..., session f3bbb32b)`.
- For each batch, a count line: rows in the batch, lines in the label file, rows you agree with, rows you change.
- A table of every row you change: `| batch | row | scout's keys | your keys | why, one sentence |`. Only rows you change go here.
- "Side notes": anything outside the job, with file and line, including any split you see. A side note is a finding.

## Rules

- You check labels. You pick no exit. There are four exits (fixed, a question to the user, covered by a ruling, not a problem), and only the main session picks one.
- Use only keys from `keys.md`. A row no key fits is `no key`, with the reason.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own.
- Read only. You run nothing: no test, no build, no `hydra_*` tool, never a whole-library run.
- The user's words may be cited only from lines under `## Confirmed` in `docs/agents/rulings/` and from `AUD/questions.md`.
- Never label from the `run` column. It is a mutation run's fact with limits (`AUD/mutation/README.md`).

## Caps

The progress hook warns at 100 calls or 20 minutes and stops you at 150 calls or 30 minutes. At the warning, write what you have and list, by exact row, what you did not check under "Not checked".

## Your final report

Return the file path, the two count lines, the number of rows you changed by batch, and your side notes in one line each.
