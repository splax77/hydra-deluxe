# Audit of the past audits: handoff (2026-10-29, session b4409ae9)

The user asked for a full audit of every past audit. The goal is to find each place where an audit found a split and closed it without the user ever ruling on it. A split here means two parts of Hydra answering one question differently. The user approved the method below on 2026-10-29 ("yes both"). They then asked for this work to run in its own session.

## Why: what went wrong with M39

The Backend timings table on the Paths tab rates each note by how far it lands from the Star Power end. Its two kinds of row use different middle bands. A plain row reads "Standard" from -10 ms to under 3 ms. The squeezed-out row reads "Standard SqOut" from -10 ms to under 10 ms. Take a note 5 ms after the end: as a plain row it reads "Hard (uncounted)", and as a squeezed-out row it reads "Standard SqOut (uncounted)". The code lives in `BackendSqueeze::summarystr` (`src/core/model.cpp`). The 3 is the default of the leeway setting, through `core::counted_without_squeeze`. The 10 is `kBackendInnerBandMs`.

The split came from public Hydra's author in commit c9c00cf7 (2025-08-12, "New ratings for the backends table"). That was a year before the user's first commit. Four steps then passed it by without the user ever ruling on it:

1. A 2026-08-21 session asked about "two squeeze rating ladders" (card `2026-08-21-266e74ad`). The user chose "Behaviour-preserving only". That question was about the report tiers against the row labels, not the 3 vs 10 split.
2. Derivation audit finding 306 (2026-10-03) shows the split in its own table, at offsets 3 and 10. But the finding was titled "fixed 10 ms nobody chose". So its question, D48 question 4 in `docs/audit/2026-10-04-phase-3-5-questions.md`, asked only whether the 10 should stay fixed. The user answered "all recommended" to 33 questions at once. The triage then marked 306 "fixed", because the 10 got a constant name.
3. D50 item 1 settled a different pair: the report tiers against the row labels. It only reworded the report footer.
4. The concept audit (2026-10-11, M39) put both row tables in `findings/V5.md`. It then matched the finding to D50 by topic ("two ladders") and listed it under "Already decided, no question" in `questions.md`.

The finding step worked: every audit saw the split. Splits got lost at the triage step, in three ways. A question got written about a neighbouring thing. A closure got matched to a decision by topic. And nobody but the agent that found a split ever checked how it was closed.

## The rules every verdict follows

Read `docs/agents/audit-method.md` first. It is the method this audit runs by: the four exits, the closure check and how to write questions. It also holds the record of what went wrong in earlier audits. The five rules the user approved are below, and the method page spells each one out. Apply them to every closure you check.

1. **A split is a question by default.** Only a user ruling closes it. "On purpose", "by design", "deliberately" and "kept" close nothing unless a ruling backs them.
2. **"Already decided" needs the user's words, naming both sides.** The closure must quote a ruling that names both values or behaviours. When the user answered "all recommended", the words that count are the question's own text and its Recommended line, so that text must name both sides. A match by topic fails.
3. **Questions show the real rule.** A reopened item goes to the user with both values side by side. It gives one example from each end and one from the gap where they disagree, in game terms.
4. **A fresh agent checks every closure.** The finder never closes its own finding.
5. **Splits are not asked in big batches.** Each split question goes to the user alone or in a small group.

"Fixed" in `docs/audit/2026-10-03-fix-triage.json` does not mean the split is gone. Finding 306 is "fixed" because the 10 got a name, yet both edges are still there. So a fixed finding that carries a `decision` field gets the same check as an open one.

## What to check

Most of the work is in two audits.

**The 2026-10-03 derivation audit.** `docs/audit/2026-10-03-fix-triage.json` lists 396 findings. 133 of them carry a non-empty `decision` field (the question put to the user): 64 drifts, 45 assumptions, 20 duplicates, 3 double readings and 1 doc. Of those 133, 126 are `fixed` and seven are `settled_by_decision`: ids 53, 141, 307, 314, 317, 320 and 321. One more row, id 341, is `closed` and has an empty `decision`. Those are the only eight rows that are not `fixed`. Read each finding in `2026-10-03-derivation-audit.md` (headings look like `#### 306.`). Then find the question the user saw: the phase 3-5, 6 and 7 question files in `docs/audit/`. Then find the decision that answered it in `2026-10-03-fix-decisions.md`; D48, D50 and D51 were "all recommended" batches. Last, check whether the code today still has the split.

**The 2026-10-11 concept audit** (`docs/audit/2026-10-11-concept-audit/`). Check every way a finding left without a question:
- the "Already decided, no question" section of `questions.md` (M04, M09, M27, M39, M60, M61);
- its "Dropped from the candidate set as code-only" and "Code-only findings" sections;
- the "Left out, one line each" section of `fix-wave.md`;
- every verdict in `findings.md` and `findings/*.md` that says kept, by design, on purpose or no fix.

Two items already look wrong. M04 says "no decision covers that" about the empty-state sentences and is still listed as decided. M60 says the charter "deliberately differs" without naming a ruling.

**The smaller audits.** These need one quick pass for closures of the same kind:
- `2026-09-24-derivation-audit.md` and `2026-09-25-codebase-audit.md`;
- `2026-10-04-phase6-recheck/` (a.md to d.md);
- `2026-10-10-test-suite-audit.md`, and `docs/handoffs/2026-10-25-test-audit/report.md` (not committed yet; it sits untracked in the main checkout);
- `2026-10-10-hook-audit.md` and `2026-10-25-hook-design-audit.md`. These are about hooks, so a split there is a split in hook behaviour.

**Out of scope.** The session-time audits (`2026-10-04-step1-2-session-audit.md`, `2026-10-04-session-audit/`) measure where time went and close no product choices. `drum-lanes/`, `note-shuffle/` and `ch-evidence.md` are reverse-engineering investigations. Look at those only when a scout follows a reference into them.

The decision log's D1 to D47 records were never reached by any decision audit (`docs/handoffs/2026-10-29-path-builder-build-handoff.md`, open items; that file is not committed yet and sits untracked in the main checkout). Where a closure cites one of them, check it the same way.

## What each verdict looks like

Each scout writes one block per item. The block names the audit, the item id, and what the split is, with both sides' values. It quotes the closure's claim and names the decision it cites. It quotes the cited words verbatim, with their file and line. Then it gives one of three verdicts:

- **PASS:** the quoted words name both sides.
- **REOPEN:** they don't, or no ruling exists. Add an example from each end and one from the gap, in game terms.
- **GONE:** the code no longer has the split. Name the commit that removed it.

A scout reads the code to confirm the split still exists. It never takes an audit's word for that (agent rule 9).

The main session checks each PASS itself, by grepping its quote in the cited file. A quote that isn't there turns the PASS into REOPEN. The main session also reads each REOPEN against the code before it goes to the user.

## How to run it

All of this is read-only. Show the user the fan-out before launching anything (agent rule 3). Scouts run on Sonnet (`subagent_type: scout`) and in parallel (rule 6). Each brief starts with `docs/agents/brief-preamble.md` and carries the status rule.

A good first wave is four scouts:
1. the concept audit's closures;
2. the derivation audit's 45 assumptions that carry a decision;
3. the derivation audit's 67 drifts and double readings that carry a decision;
4. its 21 duplicate and doc findings that carry a decision, the eight rows that are `settled_by_decision` or `closed` (most also sit in the lists above, so this scout checks them once more), and the smaller audits.

Each scout has a budget of 100 tool calls and 20 minutes (rule 7). A scout that can't finish lists the ids it didn't reach, and a fresh scout takes those. Expect a second wave.

## What to deliver

1. **Write the results** to `docs/audit/2026-10-29-audit-of-audits/`: the briefs, each scout's blocks, and a summary. The summary counts PASS, REOPEN and GONE per audit, and says which closure paths leaked most.
2. **Extend the method doc.** `docs/agents/audit-method.md` is already on `main`. Add whatever this audit teaches about where splits get lost, especially to "What went wrong before". Do it on its own branch in a worktree under `.claude\worktrees\`, with one fresh reviewer, and show the change to the user before it merges.
3. **Write a question list** for the user, one split per question or in small groups (rule 5). Each question follows rule 3, and each is about something the user sees or that changes a score.
4. **Correct the record without rewriting old audits.** List each wrong closure in the summary. M39 is the first one: it's a REOPEN, and the user hasn't answered how the two Standard bands should relate. That question can go to the user right away.

## Not this session's work

The Path Builder build continues in its own handoff, `docs/handoffs/2026-10-29-path-builder-build-handoff.md`. It has its own questions for the user (PB-2a's wording, re-picking later windows' ends, the negative-limit note) and the real-app check. Leave those to that session.
