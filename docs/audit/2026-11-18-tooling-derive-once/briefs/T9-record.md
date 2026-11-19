Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are an executor writing an audit's records. You write documents only: no code, no hook, no test.

Task id: given in your dispatch (TOOL-AUDIT-T9-Q or TOOL-AUDIT-T9-S). Session id: 0e741144-a798-4d71-b737-1b6149f84c1a.
Your worktree is given in your dispatch. Work and commit only there, on its branch. Never touch main, never push, never merge. Commit with the Task:, Agent: and Session: trailers the preamble asks for. Status lines as the preamble says. When the hook warns you, commit what you have and report what is left.

## The audit

A derive-once audit of our own tooling (Hydra's docs and code, and the hooks repo C:\Users\Patrick\.claude) is finished except for its records. The plan is C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-plan\docs\superpowers\plans\2026-11-18-tooling-derive-once-audit.md; read its sections "T6" to "T9", "Questions and the user's answers" and "Promise map". Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\audit-method.md, stages 5 to 8. The audit folder in your worktree is docs\audit\2026-11-18-tooling-derive-once\. Its records:

- `log.md`: every stage's numbers and every answer the user gave, with the question text. Sections "T6: the user's four answers", "T8: the user's answers, batch 1" to "batch 5", and the T8 re-check and fold sections. This is the source for every answer. The user's words are quoted there exactly; copy them exactly, typos included.
- `exits.md`: the 660 ledger rows with their T6 category and exit.
- `reports\TOOL-AUDIT-T7-C01.md` to `C23.md`: T7's closure verdicts (PASS, REOPEN, GONE) per row. There is no closures.md; these reports are the record.
- `reports\TOOL-AUDIT-T8-G1.md`: the 238 question rows grouped into 20 themes.
- `fold-T8.md`: the 115 rows re-checked in T8 (themes 11 to 15 and 17 to 20, plus the 19 rows T7 reopened), each with its final exit, after the T8 closure check (`reports\TOOL-AUDIT-T8-K1.md` to `K7.md`).
- `ledger\TOOL-AUDIT-T5-L*.md`: each row's homes, for any row you need to describe.

## Task TOOL-AUDIT-T9-Q: questions.md and the D-records

Write `questions.md` in the audit folder: every question the user was asked in this audit, in the order asked, numbered Q1, Q2 and so on. For each: the question as log.md gives it, the two sides it named, the ledger rows or themes it covers (theme numbers from G1, row ids where log.md or fold-T8.md names them), the user's answer in their exact words or the label they chose, what the answer does to those rows (which exit), and the D-record item that holds it. Where the user asked a question back instead of choosing, record that, what the main session checked, and how the rows left (log.md says each time). Every exit 2 row must point at a question number, so check fold-T8.md and G1 against your list.

Then add six D-records at the end of C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\<your worktree>\docs\audit\2026-10-03-fix-decisions.md, after D344, in the format of D342 to D344 (bold first line with number, session and date, then numbered items):
- D345 (session f6a384b5-9eed-4607-95cb-681ec635d569), 2026-11-18: the four T6 answers.
- D346 (session f6a384b5-9eed-4607-95cb-681ec635d569), 2026-11-18: T8 batch 1.
- D347 (session f6a384b5-9eed-4607-95cb-681ec635d569), 2026-11-18: T8 batch 2.
- D348 (session 57045e74-0b59-432e-94f2-0890be921616), 2026-11-18: T8 batch 3.
- D349 (session 0e741144-a798-4d71-b737-1b6149f84c1a), 2026-11-19: T8 batch 4.
- D350 (session 0e741144-a798-4d71-b737-1b6149f84c1a), 2026-11-19: T8 batch 5.
Check the session id of each batch against log.md and the session cards in docs\sessions\ (grep the answer's words); if log.md or a card says otherwise, use what they say and note it in your report. Each item names both sides and quotes the question's text and the answer, so a later audit can test "does this ruling name both sides?" from one entry. Each record says these are answers in chat, not filed rulings under docs\agents\rulings\, unless the item rests on a Confirmed line, which it then cites with file and line. Make no D-record claim the log does not support.

Owned files: questions.md, and the decision log's new D345 to D350 only. Change nothing else.

## Task TOOL-AUDIT-T9-S: summary.md

Write `summary.md` in the audit folder. It holds:
1. One paragraph on what the audit read and what it was for (the plan's "The short version").
2. The counts, each with where it came from: ledger rows by verdict and by kind; T6 categories; T7 verdicts; the T8 re-check and closure-check numbers (fold-T8.md's "Counts"); and the final exit of every row, by exit (2, 3, 4, Fix pending, GONE, refuted, still open). Compute the final exits by script from the records: start from exits.md, apply T7's verdicts, then the answers by theme (log.md and G1), then fold-T8.md for its 115 rows, which win. Keep the script in `scripts\` and name it in the summary. Every row ends with one final exit; a row your script cannot place is listed by id as a gap, never guessed.
3. The rows not audited, and why, from the plan's Scope section and coverage.md.
4. The four rows still NOT VERIFIED after both verifier rounds: L-F12-17, L-F19-20, L-F20-13, L-F25-16 (check each against exits.md and fold-T8.md and say where each stands now).
5. The Fix pending list: every row whose exit is "Fix pending", plus every exit 3 row whose fix is still to be made (the doc fixed to point at the code, the note fixed or pruned, the pointer moved, the pick listed for the user's yes, the home given, the claim dropped), grouped by the answer that sent it there. The rows fixed already (L-F13-08, L-F16-08, L-F13-17, by merge 6acfbed4) are listed as fixed, not pending. Add the fix-plan items log.md names outside the rows: the time-stamp fix, status_append.ps1 appending to any .md path, tools/build_slot.ps1's state under the removed C:\Users\Patrick\.claude\hooks\ folder, the hook trims, the build-count change to agent-rules.md rule 3, and the hook gaps the T8 checkers recorded (log.md, "T8: the fold and its closure check"). End by saying the fixes are a separate plan that needs the user's yes, and that the audit is not closed while the Fix pending list has rows.

Owned files: summary.md and one script in scripts\. Change nothing else.

## Rules for both
- Plain English, short sentences, one idea per sentence, as C:\Users\Patrick\.claude\CLAUDE.md says. No walls of file:line bullets with no sentences.
- Every number says where it came from.
- A memory note is never the user's words. Only lines under `## Confirmed` in docs\agents\rulings\ are filed rulings; the answers in log.md are chat answers.
- No run of any kind besides your own script over the audit folder's files.
