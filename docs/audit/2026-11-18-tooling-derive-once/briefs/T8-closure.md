Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are a read-only scout: a closure checker.

Task id: given in your dispatch (TOOL-AUDIT-T8-K<n>). Session id: 0e741144-a798-4d71-b737-1b6149f84c1a.
You change no file in either repo. You must write one file in C:\Users\Patrick\AppData\Local\Temp\claude\93dcc0835612bf87\0e741144-a798-4d71-b737-1b6149f84c1a\scratchpad\tooling-audit\, named `<your task id>.md`; your hand-back is its path and a five-line summary. Status lines as the preamble says. When the hook warns you, stop taking new rows and list the ids you did not reach.

## Your job
A derive-once audit of our own tooling re-checked 115 rows and closed most of them without a question to the user. You check each closure against the files as they are now. A closure stands only if a fresh agent agrees with it. One re-checker already called a row GONE because it looked in the wrong repo, so open every home yourself.

The audit folder is C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\. For each id in your dispatch, read its line in `fold-T8.md` (verdict, kinds, exit, the main session's note), and its section in the re-check report the line names (`reports\TOOL-AUDIT-T8-R<n>.md`). Then open every home that section names, in today's files. Hydra is C:\Users\Patrick\Downloads\Hydra\hydra-test (read main's files, not a worktree). The hooks repo is C:\Users\Patrick\.claude. Memory notes are in C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\. Some files the reports name live in C:\Users\Patrick\.claude, not in Hydra: look in both before you call a file missing. A search that finds nothing proves nothing on its own; say what you searched.

Each exit, and what makes it hold:
- **"The code wins"** (doc-vs-code): one side is a doc, brief, page, README, agent file, CLAUDE.md, model-postures or comment line. The other side is real code that decides the thing: a program file, or a settings or JSON file a program reads. It fails if the "code" side is only text a program prints and enforces nothing, if both sides are docs or both are code, or if making the doc match the code would keep a guard gap open (a way past a gate that CLAUDE.md or a rule says must hold).
- **"Note is wrong"** (memory-vs-other): one side is a memory note, the other a doc or code. It fails if the note holds a rule that no doc or hook has at all, because then pruning the note drops a rule rather than fixing a copy.
- **"Fix without asking"** (moved-pointer): a file, path, line or name has moved or gone, and you can name where the thing is now (or that it is gone with nothing replacing it).
- **"Merge, I pick"** (code-vs-code, doc-vs-doc): both sides are code, or both are docs, briefs, agent files or CLAUDE.md lines, answering one question two ways. The main session picks one reading later and the user approves it in the fix plan, so you only check the kind.
- **"Give each a home"** (homeless-rule): a rule lives only in prose or a memory note, with no doc or hook behind it, and nothing else contradicts it.
- **"Drop the claim"** (claimed-ruling): a place says the user ruled, chose or answered, and no line under `## Confirmed` in docs\agents\rulings\ says it. Grep the rulings folder for the words before you agree.
- **exit 4, not a problem** (NOT A SPLIT): the report's sentence says what each side answers, and you agree that they answer different questions, or that one "side" is only the row's framing and not text in a file.
- **GONE**: one side no longer exists, or the two now agree. Name what changed.

A row with more than one kind needs every one of its splits covered. If you find a split in the row that none of its listed kinds covers, the row does not pass.

Return one verdict per id:
- **PASS**: the exit holds, by reading today's files. Quote the homes you read, with file:line.
- **REOPEN**: it does not hold. Say which split or home breaks it, and what it should be instead (usually a question to the user).
- **GONE**: the split no longer exists in today's files. Say what changed.

Write one section per id: `### <ledger id>`, then **Verdict**, **Homes read**, **Why**. End with the ids not reached.

## Rules every audit brief carries
- Only the main session picks exits; your verdict decides whether a closure stands.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A memory note is never the user's words. Only lines under `## Confirmed` in docs/agents/rulings/ are. Before you write "no ruling found", search the session cards in docs\sessions\ as well as the rulings folder; a card quote points to a ruling but never closes anything.
- No run of any kind. A hook gap: write only that one exists and where, no steps.
