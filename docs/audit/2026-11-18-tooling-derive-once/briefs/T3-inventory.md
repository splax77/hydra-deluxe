Read C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md first; it binds you. You are a read-only scout.

Task id: TOOL-AUDIT-T3-<scout>. Session id: f6a384b5-9eed-4607-95cb-681ec635d569. <scout> is the inventory scout name given in your dispatch (I1 to I10).
You change no file in either repo. Write your report as one file: the session's scratch folder (not kept; each report was copied to this audit's reports folder), and hand back that path with a five-line summary. Status lines as the preamble says. Your call and time budget is the progress hook's; it tells you when to wrap up. When it warns you, stop taking new files, finish the one in hand, and list the files you did not reach.

## The audit
This is a derive-once audit of our own tooling: the hooks, helper tools, agent files, memory notes and agent docs. The rule is agent rule 15 in docs/agents/agent-rules.md: every question has one owner, and "A rule copied from a tool, a test or a comment is a second owner, never a source." Later stages search for copies. Your stage is a census: list every value your files decide or state as a rule.

A value is a number, a limit, a time, a path, a list, a name, a word list, a pattern, a format, or a rule (who does what, when, and what is refused or allowed). Include values a text asks someone to type, fill in or report, and say who is asked.

## Your files
Your dispatch names them. Read every one in full. Name the commit of each repo you read (`git -C <repo> log -1 --format=%H`); for a file outside git (memory notes), say so.

## What each row holds
One row per value, numbered `<scout>-001` onwards:
- the question it answers, in plain words ("how long may an agent run before it is warned");
- file and line;
- the value, quoted exactly;
- whether the text claims to be the owner ("the one home of", "owns", "nowhere else"), quoting that claim;
- who reads or acts on it: a hook (name it), the main session, a named agent type, every agent, or the user;
- if a text asks someone to supply the value: who is asked.
Do not judge rows, search for copies, or pick exits. If you happen to see that a value is stated in two of your own files, put both rows in and note the other row's id; that is all.

## Rules every audit brief carries
- Only the main session picks a finding's exit, out of four: fixed; a question to the user; a ruling in the user's own words already covers it, naming both sides; or not a problem, with one sentence on what each side answers.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own; say what you searched.
- No whole-library run, and no Hydra test program, ever. Run no hook test.
- The user's words live in docs/agents/rulings/ (only lines under `## Confirmed`), the decision log docs/audit/2026-10-03-fix-decisions.md, the question files under docs/audit/, the plans under docs/superpowers/plans/, and the session cards in docs/sessions/. A memory note is never the user's words; it is a home like any other text.
- A hook gap you notice (a way past a hook): write only that one exists and where, in general words; put no steps in your report.
