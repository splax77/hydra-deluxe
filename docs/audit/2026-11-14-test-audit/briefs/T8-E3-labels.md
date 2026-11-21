# Brief: T8 E3, label one batch of faulty records with keys

Session f3bbb32b-5f4d-4400-8cda-86f04d67f2a4. Your task id is `T8-E3-<batch>`, where the batch is the one your dispatch names (such as `L-C01`). AUD is `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\t8-m1\docs\audit\2026-11-14-test-audit`.

Read first, in this order:

1. `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md`. It binds you. Its status rule matters most: append a status line every 5 minutes with `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`, alone in its own PowerShell call.
2. `AUD/exits/keys.md`, the closed list of keys. You use only its keys.
3. Your batch, `AUD/exits/labels/batches/<batch>.tsv`.

## What you do

The audit judged every test in Hydra's suite. Many tests break one of the user's rulings about tests. The next step groups them, so that tests with the same fault share one question to the user. Your job is to label each row of your batch with the key or keys that say what its test fakes, pins, echoes or cites.

Each batch row is one ledger row. Its columns are: kind, row, record, test_or_title, faults_or_verdict, counter_reason, run.

A `count` row is a recount counter's verdict on one test. `faults_or_verdict` holds its fault marks (X9 made-up stand-in, X10 typed constant, X11 echo, X12 citation, snapshot). `counter_reason` says what the counter saw. Label from the reason when it names the thing plainly ("hand-built `ready(...)` rows; 378315 typed" is F-row and P-burnout). When the reason is not enough, open the record (`record` names its file and the heading's line, under AUD) and, if you must, the test. Each fault mark needs at least one key: an X9 mark gets an F, O or C key; an X10 mark gets a P key; X11 is E-echo; X12 is Z-cite; snapshot is S-snapshot.

A `record` row is a Rewrite or Remove record. Open the record at the line in `row` (under AUD) and read its verdict and reasons. Give it the K key for the rubric kind it describes, and any other key its text plainly shows.

The `run` column is what a mutation run saw for that test, which planted small bugs and counted the tests that went red. It is a fact beside the record, with limits (`AUD/mutation/README.md`). Never label from it. "Caught nothing" is not proof the test is empty.

## What you write

One file, `AUD/exits/labels/<batch>.md`. Write it with the Write tool, and nothing else in AUD. Do not commit; the main session commits. Its parts:

- A first line: `Agent id: <your id> (task T8-E3-<batch>, session f3bbb32b)`.
- A table with one line per batch row, in batch order: `| row | keys | note |`. `row` is copied exactly from the batch. `keys` is a comma-separated list of keys from `keys.md`, or `no key`. `note` is one short sentence: what the test fakes, pins, echoes or cites, in your words. For `no key`, say what the fault is and why no key fits.
- A table of ends: `| key | plainest row | least-fitting row |`, one line per key you used. The plainest row is the clearest case of that key in your batch. The least-fitting row is the one that fits it worst while still fitting. Where a key has one row, both cells name it.
- A short section "Side notes", only if you saw something outside your job: a record whose verdict and test disagree, a test line that has moved, a fault the counter missed. Give the file and line. A side note is a finding, so write it down even if it seems small.

Every row of the batch must have a line in your table. The main session's check counts them.

## Rules

- You label facts. You pick no exit. There are four exits (fixed, a question to the user, covered by a ruling, not a problem), and only the main session picks one, at E7. Your file has no exit column and no recommendation.
- Use only keys from `keys.md`. Never invent one. When none fits, write `no key`. The key list is revised from those rows.
- "On purpose", "by design", "deliberately" and "kept", in a record or a comment, are claims, not evidence. They change no label.
- If you see two parts of Hydra answer one question differently (a split), write it as its own side-note line.
- A search that finds nothing proves nothing on its own.
- Read only. You run nothing: no test, no build, no `hydra_*` tool, and never a whole-library run (that needs the user's `allow library run` in chat).
- Where the user's words may be cited: only lines under `## Confirmed` in `docs/agents/rulings/`, and the answers in `AUD/questions.md`. A session card in `docs/sessions/` keeps the user's typed words but is not a filed ruling.
- Take exactly the rows of your batch. Do not skip rows that look already handled, and do not take rows from another batch.

## Caps

The progress hook warns at 100 calls or 20 minutes and stops you at 150 calls or 30 minutes. At the warning, write what you have, put `not read` in the keys cell of every row you did not reach, and finish. The main session sends the rest to a fresh scout once, by exact row.

## Your final report

Return the file path, the number of rows labelled, the number marked `no key` and `not read`, the count of rows per key, and your side notes in one line each.
