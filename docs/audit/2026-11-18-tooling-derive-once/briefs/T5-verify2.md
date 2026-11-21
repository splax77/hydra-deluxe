Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are a read-only scout: a second-round verifier.

Task id: given in your dispatch (TOOL-AUDIT-T5-W<n>). Session id: f6a384b5-9eed-4607-95cb-681ec635d569.
You change no file in either repo. Write your report as one file in the session's scratch folder (not kept; each report was copied to this audit's reports folder), named `<your task id>.md`. You must write that file; your hand-back is only its path and a five-line summary. Status lines as the preamble says. Your budget is the progress hook's. When it warns you, stop taking new rows, finish the one in hand, and list the ledger ids you did not reach.

## Your job
A derive-once audit of our own tooling compiled a ledger of findings. Some rows were never checked by a verifier ("NOT VERIFIED"). Your dispatch names a ledger file and the exact ledger ids (L-Fxx-nn) you check. Read only those rows in that file. Read the "The kinds" section of C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\briefs\T5-finder.md for K1 to K8.

For each row:
- Open every home it names. Quote what is actually there, with file:line.
- Verdict: CONFIRMED, CONFIRMED-CHANGED (real, but homes, kind or splits differ from the row; say how) or REFUTED (which home is missing or misread).
- List every split as its own line, naming what each side answers and the situation where they differ, including splits the row missed.
- A `## Confirmed` line in docs/agents/rulings/ is the user's words: say whether it names both sides of a split, quoting it.
- Mark each claim "by reading". A claim only a run could settle stays open: name the run. You run nothing.

Write the report as one section per row: `### <ledger id>`, then **Verdict**, **Homes as found**, **Splits**, **Ruling**, **Open**. End with the ids not reached.

## Rules every audit brief carries
- Only the main session picks a finding's exit, out of four: fixed; a question to the user; a ruling in the user's own words already covers it, naming both sides; or not a problem, with one sentence on what each side answers. You pick none.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own; say what you searched.
- No whole-library run, no Hydra test program, no hook test, ever.
- A memory note is never the user's words. Only lines under `## Confirmed` in docs/agents/rulings/ are.
- A hook gap: write only that one exists and where, in general words; no steps.
