# Scout brief for the concept duplication audit (phases 2 and 3)

Every scout in this audit reads `docs/agents/brief-preamble.md` first, then this page, then the few lines its own dispatch adds (task id, group, files, scratch folder). Where they disagree, stop and report it.

## What you are looking for

The audit looks for one idea with two homes in `src/`. An idea is a question in plain words with named inputs: "does Hydra have a result for this chart", "which settings does this page read", "how is an empty cell sorted". A home is a type, an enum, a set of status words, a helper family or a pipeline step that answers it. When two modules each hold a home for one idea, the next change has to be made twice, and one day it will be made once.

Read these parts of the plan before you start: "What is wrong and what this audit looks for", "What counts as a finding" and your phase (2 or 3) in `docs/superpowers/plans/2026-10-11-concept-duplication-audit.md` (removed; `git show 444631e6:docs/superpowers/plans/2026-10-11-concept-duplication-audit.md`). They define the kinds:

- K1: same idea, same rule, two homes. The two tables agree on every input.
- K2: same word, different rules. The tables differ, and one home is a word the user sees, a stored value or a score.
- K3: parallel on purpose. Two modules specialise one shared framework, and every parallel part holds only per-module facts. List what each parallel part knows. If any entry is a rule (how to sort, filter, measure, draw, decide), that rule is its own K1 or K2 candidate.
- K0: looks alike, answers different questions. Drop it with one sentence.

`CONTEXT.md` at the repo root holds the domain words. `docs/audit/2026-10-03-derivation-audit.md` already lists line-level copies. Before you write a candidate, grep that file for its symbols; a candidate that only repeats one of those is marked "known, line-level" in one line and not written up again. Don't read that file whole.

## How you work

You are read-only. You change nothing in the repo except your one report file, copied in at the end. No builds, no tests.

Read every file your dispatch lists, in full, with Read and `offset`/`limit`. Take the leads for your group in `docs/audit/2026-10-11-concept-audit/leads.md` (section "Leads by group") as your first stops. You may grep outside your group to find the other home of an idea your group holds; read only the lines you need there.

Don't grade candidates as findings. That is phase 4's job. For each candidate, write both homes and your first draft of both tables, and say which kind it looks like.

Check every claim against the code, including the plan's and `leads.md`'s. They were written by other agents.

## Your report

Write one markdown file in your scratch folder, then copy it into the audit folder with PowerShell `Copy-Item` to the path your dispatch gives. Don't commit; the main session commits the reports.

The report has four sections.

**Candidates.** One block per idea with two or more homes, in this shape:

```
### <group id>-<n>. <the idea as a plain question>
Looks like: K1 | K2 | K3-with-a-rule-inside
Homes: <symbol>, <file> <first>-<last line> (<its inputs>);
       <symbol>, <file> <first>-<last line> (<its inputs>).
Table, home 1: <input> -> <answer>, one row per case, edges included.
Table, home 2: the same inputs.
Differ on: <the input where they differ>, or "agree today; <the change that would split them>".
Note: anything the verifier should check, such as a claim you made by reading only.
```

Put the edges in the tables where they apply: an empty list, exactly 0, a Stale row, a chart in no mode, a value at a window edge, a missing optional.

**Parallel on purpose.** Each K3 pair you cleared, with the list of what each side knows and one sentence on why every entry is a per-module fact.

**Dropped leads.** One sentence per lead from `leads.md` you found to be K0, or "known, line-level".

**Not read.** Any file in your list you did not finish, or "none".

Write plain English: one idea per sentence, sentences around file and line references.

## Caps

100 tool calls or 20 minutes, whichever comes first. At the cap, write what you have, copy it in, and list the unread files under "Not read". The main session sends at most one more scout for those, once.

## Your final reply

Keep it under 15 lines: the report's path, how many candidates, the K-kinds they look like, and the "Not read" list. The report holds the detail.
