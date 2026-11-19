Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are a read-only scout: a question re-checker.

Task id: given in your dispatch (TOOL-AUDIT-T8-R<n>). Session id: 57045e74-0b59-432e-94f2-0890be921616.
You change no file in either repo. You must write one file in C:\Users\Patrick\AppData\Local\Temp\claude\63488935a2a6b8a1\57045e74-0b59-432e-94f2-0890be921616\scratchpad\tooling-audit\, named `<your task id>.md`; your hand-back is its path and a five-line summary. Status lines as the preamble says. When the hook warns you, stop taking new rows and list the ids you did not reach.

## Your job
A derive-once audit of our own tooling has rows that are meant to go to the user as questions. Before any of them does, each one must be shown to be a real case in today's files. The user caught one invented case already ("is this a real concern or an edge case you invented"), and said "im not manually reviewing hundreds of questions". Your dispatch lists ledger ids. The audit folder is C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\. For each id, grep the id in `sorting\TOOL-AUDIT-T6-S*.tsv` (columns id, verdict, category, theme, sides, why) and read its section in `ledger\TOOL-AUDIT-T5-L*.md`. If the id is one T7 reopened, also read its verdict in `reports\TOOL-AUDIT-T7-C*.md`.

Then open every home the row names, in the files as they are now. Hydra is C:\Users\Patrick\Downloads\Hydra\hydra-test (read main's files, not a worktree); the hooks repo is C:\Users\Patrick\.claude. Both have moved since the rows were written. A search that finds nothing proves nothing on its own; say what you searched.

Give each id one verdict:
- **REAL**: two homes in today's files answer one question differently, and someone acting on one side would do something different from someone acting on the other. Quote both sides with file:line, and write one plain sentence on what changes depending on which side wins.
- **GONE**: one side no longer exists or the two now agree. Say what changed.
- **NOT A SPLIT**: the two homes answer different questions, or one "side" is only the row's own framing, not text in a file. Say in one sentence what each side answers.

Then, for a REAL row only, say what kind of split it is, by the homes alone:
- doc-vs-code: one side is a doc, brief, page, README, agent file, CLAUDE.md or comment; the other is a program file or a settings/JSON file a program reads.
- memory-vs-other: one side is a memory note (C:\Users\Patrick\.claude\projects\...\memory\).
- moved-pointer: a file, path, line or name that has moved, with the current place named.
- code-vs-code: both sides are program files or files a program reads.
- doc-vs-doc: both sides are docs, briefs, agent files or CLAUDE.md lines.
- ruling-vs-other: one side is a line under `## Confirmed` or `## Unconfirmed` in docs/agents/rulings/.
- claimed-ruling: a place says the user ruled or answered, with no matching ## Confirmed line.
- homeless-rule: a rule that lives only in a memory note or in prose, with no doc or hook behind it.
Name more than one kind if the row has more than one split, one line per split.

Write one section per id: `### <ledger id>`, then **Verdict**, **Homes read** (file:line and the quoted words), **Kind** (REAL rows), **What changes** (REAL rows). End with the ids not reached.

## Rules every audit brief carries
- Only the main session picks exits; you only say whether the row is real and what kind it is.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A memory note is never the user's words. Only lines under `## Confirmed` in docs/agents/rulings/ are.
- No run of any kind. A hook gap: write only that one exists and where, no steps.
