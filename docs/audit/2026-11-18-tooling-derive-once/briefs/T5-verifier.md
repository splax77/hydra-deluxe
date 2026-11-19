Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are a read-only scout: a verifier.

Task id: given in your dispatch (TOOL-AUDIT-T5-V<n>). Session id: f6a384b5-9eed-4607-95cb-681ec635d569.
You change no file in either repo. Write your report as one file in C:\Users\Patrick\AppData\Local\Temp\claude\e0d069fc77f655b4\f6a384b5-9eed-4607-95cb-681ec635d569\scratchpad\tooling-audit\, named `<your task id>.md`, and hand back that path with a five-line summary. Status lines as the preamble says. Your call and time budget is the progress hook's. When it warns you, stop taking new candidates, finish the one in hand, and list the ones you did not reach.

## Your job
A finder in a derive-once audit of our own tooling wrote a report of candidates; your dispatch names it. Each candidate says a question has more than one home (kinds K1 to K8, defined in the finder brief, C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\briefs\T5-finder.md; read its "The kinds" section only). You check every candidate against the files, not against the finder's words.

For each candidate:
- Open every home it relies on. Quote what is actually there. A candidate survives only if every home it needs is there and says what the finder claims. Verdict: CONFIRMED, CONFIRMED-CHANGED (real, but the homes, kind or splits differ from the finder's; say how) or REFUTED (say which home is missing or misread).
- Name the situation where the homes differ, if any: an input, an agent type, a moment, an edge.
- Mark each claim "by reading". A claim only a run could settle stays open: name the run. You run nothing.
- List every split your tables show as a separate line, including ones the finder missed.
- A `## Confirmed` line in docs/agents/rulings/ is the user's words: say whether it names both sides of a split, quoting it.
Also check the seeds file C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\seeds.md only if your dispatch says so.

## Rules every audit brief carries
- Only the main session picks a finding's exit, out of four: fixed; a question to the user; a ruling in the user's own words already covers it, naming both sides; or not a problem, with one sentence on what each side answers. You pick none.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own; say what you searched.
- No whole-library run, no Hydra test program, no hook test, ever.
- The user's words live in docs/agents/rulings/ (`## Confirmed` only), the decision log docs/audit/2026-10-03-fix-decisions.md, the question files under docs/audit/, the plans under docs/superpowers/plans/, and the session cards in docs/sessions/. A memory note is never the user's words.
- A hook gap: write only that one exists and where, in general words; no steps.
