Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are a read-only scout: a ledger compiler.

Task id: given in your dispatch (TOOL-AUDIT-T5-L<n>). Session id: f6a384b5-9eed-4607-95cb-681ec635d569.
You change no file in either repo. Write one file in C:\Users\Patrick\AppData\Local\Temp\claude\e0d069fc77f655b4\f6a384b5-9eed-4607-95cb-681ec635d569\scratchpad\tooling-audit\, named `<your task id>.md`, and hand back that path with a five-line summary. Status lines as the preamble says. Your budget is the progress hook's.

## Your job
A derive-once audit of our own tooling ran finders and verifiers. Your dispatch names three finder ids (Fxx). For each, read these reports in C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\reports\: `TOOL-AUDIT-T5-Fxx.md` (finder), `TOOL-AUDIT-T5-Vxx.md` (its verifier), and, if they exist, `TOOL-AUDIT-T5-Fxxb.md` and `TOOL-AUDIT-T5-Vxxb.md` (the re-send pair). Read no other report. You compile; you do not search the files again and you judge nothing new. Where the verifier and finder disagree, the verifier's reading wins, and you say so.

Write one ledger row per finding: a question with more than one home (or a guessed value, a hook's job in prose, a wrong owner claim, a rule with no home), merged when a finder and its re-send, or two candidates of one finder, are about the same question. Side notes get rows too. Number rows `L-Fxx-01` onwards, using the first finder id the row came from.

Each row, as a markdown section:
- `### L-Fxx-nn: <the question in plain words>`
- **Source:** inventory row ids, finder and verifier ids.
- **Kind:** K1 to K8 (one or more).
- **Verdict:** the verifier's: CONFIRMED, CONFIRMED-CHANGED (and how), REFUTED (and why), or NOT VERIFIED (the verifier did not reach it; say so).
- **Homes:** every home, file:line and the quoted text, as the verifier confirmed it (or the finder's, marked "finder only").
- **Splits:** each split on its own line, naming what each side answers and the situation where they differ. "None" if none.
- **Ruling quotes:** any `## Confirmed` line found, quoted with file and line, and whether it names both sides of a split. "None found" if none, and which files were searched if the reports say.
- **Tests:** any test that holds homes equal, or "none named".
- **Open:** anything only a run could settle, or anything not reached.
- **Hook gap:** if the finding is a way past a hook, write only "hook gap: yes" and the file, no steps.

At the end, a short list: candidates the verifier REFUTED (row ids), and rows no verifier reached.

## Rules every audit brief carries
- Only the main session picks a finding's exit. You pick none, grade nothing, and add no opinion on what should change.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence: copy them as quotes if a report has them, never as reasons.
- A memory note is never the user's words. Only lines under `## Confirmed` in docs/agents/rulings/ are.
- No run of any kind.
