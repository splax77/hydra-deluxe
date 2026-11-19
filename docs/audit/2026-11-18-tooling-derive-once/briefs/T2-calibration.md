Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are a read-only scout.

Task id: TOOL-AUDIT-T2. Session id: f6a384b5-9eed-4607-95cb-681ec635d569.
You change no file in either repo. Write your report as one file: the session's scratch folder (not kept; each report was copied to this audit's reports folder), and hand back that path with a five-line summary. Status lines as the preamble says. Your call and time budget is the progress hook's; it tells you when to wrap up. When it warns you, stop taking new values, finish the one in hand, and report what you did not reach.

## The audit
This is a derive-once audit of our own tooling: the hooks, helper tools, agent files and agent docs. The rule is agent rule 15 in docs/agents/agent-rules.md: every question has one owner, and "A rule copied from a tool, a test or a comment is a second owner, never a source." For every value in your files (a number, a limit, a path, a list, a name or a rule), ask: who owns it, and does any other place restate it, recompute it, or ask someone to supply it?

The kinds of problem, in general words:
- K1 Copied value: a value is stated in a second place, and the two agree today. This still counts.
- K2 Drifted value: the second place disagrees with the owner.
- K3 Changed meaning: the copy keeps the value but changes what it does or who acts on it.
- K4 Guessed value: a person or agent is asked to supply a value that a tool already holds or can read.
- K5 Prose doing a hook's job: text tells an agent to do what a hook enforces, and spells the rule out instead of pointing at the hook.
- K6 Two owners in code: two code places decide the same question.
- K7 A rule with no home: a prose rule nothing checks, stated in several places, none named as its home.
- K8 A wrong owner claim: a line says "X owns this" and X does not.
An example from Hydra's game code, not from the tooling: the Paths tab once showed a Dynamic Hit Window figure that it computed itself from chart gaps, while the engine already held the per-note window; the display was a second owner of an engine value (rule 15's last sentences). A line that names the owner and states no value is a pointer, not a home. A pointer that also states the value is a home.

## Your files
Read these seven files, all of them, and trace every value in them across the seven:
1. C:\Users\Patrick\.claude\gates\progress.py
2. C:\Users\Patrick\.claude\gates\status_append.ps1
3. C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md
4. C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md
5. C:\Users\Patrick\.claude\agents\card-writer.md
6. C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\agent-rules.md
7. C:\Users\Patrick\.claude\model-postures.md
Name the commit of each repo you read (`git -C <repo> log -1 --format=%H`).

## What you report
For each candidate: the question it answers in plain words, every home with file, line and quoted text, a table of what each home tells its reader to do (include the edges: exactly at a limit, the main session against an agent, an agent type that skips the preamble), and the kind. Every split (two homes answering one question differently) is its own line. Do not grade candidates and pick no exit.
Also report one number: how many distinct values you traced across the seven files before the hook warned you (or in all, if it never did).

## Rules every audit brief carries
- A finding leaves the audit by one of four exits, and only the main session picks the exit: (1) fixed, (2) a question to the user, (3) a ruling quoted in the user's own words already covers it, naming both sides, (4) not a problem, with one sentence saying what each side answers.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own; say what you searched.
- No whole-library run, and no Hydra test program, ever.
- The user's words live in docs/agents/rulings/ (only lines under `## Confirmed` are the user's), the decision log docs/audit/2026-10-03-fix-decisions.md, the question files under docs/audit/, the plans under docs/superpowers/plans/, and the session cards in docs/sessions/ (grep docs/sessions/index.md). A memory note is never the user's words. A card keeps the user's typed words and the start of each question, not the full question.
- Also read, for context: docs/agents/rulings/review-and-hooks.md and agents-and-workflow.md (`## Confirmed` lines), and docs/superpowers/plans/2026-10-25-hooks-from-scratch.md line 108.
