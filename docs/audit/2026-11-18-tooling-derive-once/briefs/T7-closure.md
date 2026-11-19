Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are a read-only scout: a closure checker.

Task id: given in your dispatch (TOOL-AUDIT-T7-C<n>). Session id: f6a384b5-9eed-4607-95cb-681ec635d569.
You change no file in either repo. You must write one file in C:\Users\Patrick\AppData\Local\Temp\claude\e0d069fc77f655b4\f6a384b5-9eed-4607-95cb-681ec635d569\scratchpad\tooling-audit\, named `<your task id>.md`; your hand-back is its path and a five-line summary. Status lines as the preamble says. When the hook warns you, stop taking new rows and list the ids you did not reach.

## Your job
A derive-once audit of our own tooling closed some findings without a question. You check each closure against the files as they are now (both repos have moved since the findings were written; read the current files). Your dispatch gives a sorting file and the ledger ids to check. For each id, read its line in the sorting file (`category`, `sides`, `why`) and its section in the ledger file named in your dispatch (and, if the ledger row says NOT VERIFIED, the matching section in the W reports under ..\reports\TOOL-AUDIT-T5-W*.md).

The closure categories, and what makes each one hold:
- **A** (the user answered "The code wins"): every split in the row is a doc, brief, page, README, agent file, CLAUDE.md, model-postures or comment line against real code (a program file, or a settings or JSON file a program reads). It fails if any side of any split is another doc, a memory note, or a ruling, or if both sides are code.
- **B** (the user answered "Note is wrong"): every split is a memory note against docs or code.
- **C** (the user answered "Fix without asking"): every split is a moved or missing file, path, line or name, and the current place is clear and you can name it.
- **P** (Fix pending, no question): the row has no disagreement at all: every home says the same thing today, and replacing the copies with a pointer to the owner changes no behaviour and needs no choice of which value is right.

Return one verdict per id:
- **PASS**: the category holds, by reading the current files. Quote the homes you read, with file:line.
- **REOPEN**: it does not hold. Say which split or home breaks it and what category it should be (usually a question).
- **GONE**: the finding no longer exists in the current files (say what changed).

Before any PASS, open every home the row names. A search that finds nothing proves nothing on its own; say what you searched. Write one section per id: `### <ledger id>`, then **Verdict**, **Homes read**, **Why**. End with the ids not reached.

## Rules every audit brief carries
- Only the main session picks exits; your verdict decides whether a closure stands.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A memory note is never the user's words. Only lines under `## Confirmed` in docs/agents/rulings/ are.
- No run of any kind. A hook gap: write only that one exists and where, no steps.
