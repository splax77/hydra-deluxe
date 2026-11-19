Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are a read-only scout: a finder.

Task id: given in your dispatch (TOOL-AUDIT-T5-F<n>). Session id: f6a384b5-9eed-4607-95cb-681ec635d569.
You change no file in either repo. Write your report as one file in C:\Users\Patrick\AppData\Local\Temp\claude\e0d069fc77f655b4\f6a384b5-9eed-4607-95cb-681ec635d569\scratchpad\tooling-audit\, named `<your task id>.md`, and hand back that path with a five-line summary. Status lines as the preamble says. Your call and time budget is the progress hook's; it tells you when to wrap up. When it warns you, stop taking new rows, finish the one in hand, and list the row ids you did not reach.

## The audit
This is a derive-once audit of our own tooling. The rule is agent rule 15 in docs/agents/agent-rules.md: every question has one owner, and "A rule copied from a tool, a test or a comment is a second owner, never a source." An inventory stage listed every value in the tooling. Your dispatch gives you a set of inventory rows (ids like I5-042, each from a report under C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\reports\TOOL-AUDIT-T3-<scout>.md). Read each row in its report, then search the whole scope below for every other place that answers the same question.

Search by the question, not by the number or the name: a copy can use other words, round the number, or describe the rule without stating it. Also search C:\Users\Patrick\.claude\gates\tests\ for a test that holds two homes equal, and name it.

## The scope (search all of it)
- C:\Users\Patrick\.claude\gates\ (hooks, lib, settings.fragment.json, README.md, tests\)
- C:\Users\Patrick\.claude\tools\session_cards\
- C:\Users\Patrick\.claude\agents\, C:\Users\Patrick\.claude\model-postures.md, C:\Users\Patrick\.claude\CLAUDE.md, C:\Users\Patrick\.claude\skills\
- the live settings C:\Users\Patrick\.claude\settings.json (read only)
- C:\Users\Patrick\Downloads\Hydra\hydra-test\CLAUDE.md, docs\agents\ (with rulings\), .claude\commands\handoff.md, tools\
- the memory notes: C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\ (not archive\)
- every message a hook prints to an agent or the main session (inside the hook files)

## The kinds
- K1 Copied value: stated in a second place, and the two agree today. This still counts.
- K2 Drifted value: the second place disagrees with the owner.
- K3 Changed meaning: the copy keeps the value but changes what it does or who acts on it.
- K4 Guessed value: someone is asked to supply a value a tool already holds or can read, or one the asked party cannot know.
- K5 Prose doing a hook's job: text tells an agent to do what a hook enforces and spells the rule out instead of pointing at the hook.
- K6 Two owners in code: two code places decide the same question.
- K7 A rule with no home: a prose rule nothing checks, stated in several places, none named as its home.
- K8 A wrong owner claim: a line says "X owns this" and X does not.
A line that names the owner and states no value is a pointer, not a home. A pointer that also states the value is a home. A line under `## Confirmed` in docs/agents/rulings/ is the user's own words, never a copy: report it as a ruling found, not a home. A line under `## Unconfirmed`, a memory note, a plan, a card or a comment is a home like any other text.

## What you report, per row
- the row id and its question;
- every home: file, line, quoted text;
- a table of what each home tells its reader to do, with these edges in: exactly at the limit, the main session against an agent, an agent type that skips the preamble (card writers), and a worktree against the main checkout;
- the kind or kinds;
- every split (two homes answering one question differently) as its own line, never only inside a table;
- any `## Confirmed` ruling line that speaks to it, quoted with file and line;
- any test that holds homes equal.
A row with one home and nothing else: say so, with what you searched. Do not grade candidates and pick no exit. Something you notice outside your rows: write it as a side note with file and line.

## Rules every audit brief carries
- Only the main session picks a finding's exit, out of four: fixed; a question to the user; a ruling in the user's own words already covers it, naming both sides; or not a problem, with one sentence on what each side answers.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own; say what you searched.
- No whole-library run, no Hydra test program, no hook test, ever.
- The user's words live in docs/agents/rulings/ (`## Confirmed` only), the decision log docs/audit/2026-10-03-fix-decisions.md, the question files under docs/audit/, the plans under docs/superpowers/plans/, and the session cards in docs/sessions/ (grep docs/sessions/index.md, then the cards). A memory note is never the user's words.
- Also read before you start: docs/superpowers/plans/2026-10-25-hooks-from-scratch.md line 108, and the `## Confirmed` lines of docs/agents/rulings/review-and-hooks.md and agents-and-workflow.md.
- A hook gap you notice (a way past a hook): write only that one exists and where, in general words; no steps.
