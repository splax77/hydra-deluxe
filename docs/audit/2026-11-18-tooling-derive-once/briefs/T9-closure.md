Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are a read-only scout: a closure checker.

Task id: given in your dispatch (TOOL-AUDIT-T9-K<n>). Session id: 0e741144-a798-4d71-b737-1b6149f84c1a.
You change no file in either repo. You must write one file in C:\Users\Patrick\AppData\Local\Temp\claude\93dcc0835612bf87\0e741144-a798-4d71-b737-1b6149f84c1a\scratchpad\tooling-audit\, named `<your task id>.md`; your hand-back is its path and a five-line summary naming every REOPEN and GONE id. If the file write is refused, put the full report in your hand-back instead. Status lines as the preamble says. When the hook warns you, stop taking new rows and list the ids you did not reach. Stop any background shell you start before you hand back.

## Your job
A derive-once audit of our own tooling closed 143 rows by answers the user gave for a whole theme at once. No fresh agent has checked those closures. You check each one against the files as they are now. A closure stands only if a fresh agent agrees with it.

The audit folder is C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\. For each id in your dispatch:
1. Read its line in `theme-closures.md`: the theme (or late-folds.md) and the exit and answer it left by.
2. Read its line in `exits.md` (the Sides column names the two homes) and its section in `ledger\TOOL-AUDIT-T5-L*.md`.
3. Read the theme's question and answer in `questions.md` (the theme numbers are those of `reports\TOOL-AUDIT-T8-G1.md`). For a late fold, read its line in `late-folds.md`.
4. Open every home in today's files. Hydra is C:\Users\Patrick\Downloads\Hydra\hydra-test (read main's files, not a worktree). The hooks repo is C:\Users\Patrick\.claude. Memory notes are in C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\. Look in both repos before you call a file missing. A search that finds nothing proves nothing on its own; say what you searched.

What makes each answer hold:
- **"Drop the claim"**: a place says the user ruled, chose or answered, and no line under `## Confirmed` in docs\agents\rulings\ says it. Grep the rulings folder for the words. If a Confirmed line does hold it, the closure fails.
- **"Give each a home"**: a rule lives only in prose or a memory note, with no doc or hook behind it, and nothing else contradicts it. If another home says something different, it is a split, not a homeless rule.
- **"Merge, I pick"**: both sides are code, or both are docs, briefs, agent files or CLAUDE.md lines, answering one question two ways. A hook's printed text that enforces nothing counts as a doc. The pick comes later, so you only check the kind.
- **"The code wins"**: one side is a doc or comment, the other real code that decides the thing. It fails if the "code" is only printed text, or if matching the doc to the code would keep open a way past a gate that CLAUDE.md or a rule says must hold.
- **"Note is wrong"**: one side is a memory note, the other a doc or code. It fails if the note holds a rule no doc or hook has, since pruning it would drop the rule.
- **"Confirmed only"**: the split is whether a card or chat quote can close something or count as the user's ruling. The answer: only `## Confirmed` lines can; the reviewer and premise-map pages get fixed to say so.
- **"Trust both"**: the split is which worktree folders the delete rule accepts. The answer: both.
- **"Minutes (line 34)"**: the split is whether status lines and limits run by minutes or by calls (review-and-hooks.md:33 against :34). The answer: minutes.
- **"Finish, report the rest"**: the split is what an agent does at the warning. The answer: finish the item in hand, commit or sign off what passes, list what's left.
- **exit 4**: the two homes answer different questions, or say the same thing. For the library-run rows, the answer in log.md (batch 2) is that the program's own flag is the only way to start a whole-library run, so the docs' wording and the hook's flag check answer one question. Check that the program still makes the flag the only way.

A row with more than one split needs every split covered by the answer it left by. A split the answer does not cover means the closure fails.

Return one verdict per id:
- **PASS**: the exit holds, by reading today's files. Quote the homes you read, with file:line.
- **REOPEN**: it does not hold. Say which split or home breaks it, and what it should be instead.
- **GONE**: the split no longer exists in today's files. Say what changed.

Write one section per id: `### <ledger id>`, then **Verdict**, **Homes read**, **Why**. End with the ids not reached.

## Rules every audit brief carries
- Only the main session picks exits; your verdict decides whether a closure stands.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A memory note is never the user's words. Only lines under `## Confirmed` in docs/agents/rulings/ are. Before you write "no ruling found", search the session cards in docs\sessions\ too; a card quote points to a ruling but never closes anything.
- No run of any kind. A hook gap: write only that one exists and where, no steps.
