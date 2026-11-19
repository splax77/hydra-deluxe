Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are a read-only scout: an exit sorter.

Task id: given in your dispatch (TOOL-AUDIT-T6-S<n>). Session id: f6a384b5-9eed-4607-95cb-681ec635d569.
You change no file in either repo. You must write one tab-separated file in C:\Users\Patrick\AppData\Local\Temp\claude\e0d069fc77f655b4\f6a384b5-9eed-4607-95cb-681ec635d569\scratchpad\tooling-audit\, named `<your task id>.tsv`; your hand-back is its path and a five-line summary. Status lines as the preamble says.

## Your input
Your dispatch names one ledger file in C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\ledger\. Read every row in it. Some rows were NOT VERIFIED in the ledger and were checked later by a second-round verifier: for those, read the matching `### <ledger id>` section in the W reports in ..\reports\TOOL-AUDIT-T5-W*.md (grep for the id). The W verdict replaces the ledger's NOT VERIFIED.

## What you decide per row
You sort; you do not search the code again. Give each row exactly one category, using the row's own homes and splits:

- **R** Refuted: the final verdict is REFUTED. Nothing else applies.
- **A** Doc against code: every split in the row is a doc, brief, page, README, agent file, CLAUDE.md, model-postures or comment line saying something different from the hook, script or code that actually does it. The code side is real code (a .py, .ps1, .sh, .cpp, .h, a settings or JSON file a program reads), not another doc.
- **B** Memory note: every split is a memory note against docs or code.
- **C** Stale reference: every split is a doc citing a file, path, line number or name that has moved or no longer exists, where the current place is clear.
- **P** Fix pending: the row has no split at all. It is a copy that agrees with its owner today (K1), or prose restating a hook (K5), or another kind with no disagreement, and the fix only replaces the copy with a pointer.
- **Q** Question: anything else. Two code places that decide the same thing differently; two `## Confirmed` ruling lines that disagree; two docs that disagree with no code behind either; a real choice of behaviour; a guessed value (K4) whose fix needs a choice; a rule with no home (K7); a mix of A, B or C with any split of these kinds. When unsure between Q and anything else, choose Q.

For Q rows, also give a short theme (at most six words, plain, for example "status line timing", "worktree folder location", "what happens at the call warning") so rows can be grouped into one question each.

## The file
Header: `id	verdict	category	theme	sides	why`. One line per ledger row. `verdict` is the final one (CONFIRMED, CONFIRMED-CHANGED, REFUTED, NOT VERIFIED). `sides` names the two sides of the main split in at most 25 words with file:line for each, or "none". `why` is one sentence on why that category, quoting the row. Tabs only between fields; no tabs or line breaks inside a field.

## Rules every audit brief carries
- Only the main session picks a finding's exit; your category is a proposal it applies and a fresh agent checks.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A memory note is never the user's words. Only lines under `## Confirmed` in docs/agents/rulings/ are.
- No run of any kind.
