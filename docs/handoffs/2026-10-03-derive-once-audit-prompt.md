Audit the whole Hydra project for duplicated derivations, drift between them, and assumptions
nobody decided. This is a read-only audit. Do not change any source, test or build file. The
deliverable is a findings document. Every fix waits for my yes.

## Why

My rule: every fact, rule or calculation is derived in exactly one place in the project. Every
other module calls that one place. It never re-derives the fact, copies the formula, or keeps a
parallel constant.

The first audit (`docs/audit/2026-09-24-derivation-audit.md`, from
`docs/handoffs/2026-09-24-audit-prompt.md`) had the right rule and still missed things. On
2026-10-03 I found that `rate_activation` in `src/core/squeeze_rating.cpp` decided "which
multiplier governs this note" twice, in different words, and rated the squeezed-out note through
both copies against two different Star Power ends. That audit had read that very function. It
failed for four reasons, and this prompt exists to close each one:

1. It searched by function name and formula, not by question. Both copies called the same
   formula, so the formula looked single-owned. The duplicate was the decision feeding it.
2. It fanned out by module, so agents hunted for copies across files. Two copies inside one
   function never looked like a cross-module duplicate.
3. A code comment counted as justification. Claude-written comments justified Claude-invented
   thresholds (the 1 ms "impact" cutoff), so they passed as documented.
4. "Done" meant "found nothing more". Nothing proved what was examined, and nothing tested
   whether the method could find a duplicate at all.

## What counts as a finding

**Duplicate derivation.** The same question is answered in two or more places by separate code.
The copies count even if they agree today, even if they use different names, signs or loop
shapes, and even if they sit in the same function. Examples: two pieces of code that each decide
whether a note is inside an SP window, a formula copied between the engine and a view, two
constants for the same epsilon, a test helper that recomputes what production code computes
instead of calling it. A wrapper that only calls the single owner is fine.

**Double reading of one stored fact.** One fact stored in two places (for example a SqOut entry
in `sqinouts` and the same note as a backend row), where code reads both copies and derives
something from each.

**Drift.** Two copies that give different answers for some input. Show the input.

**Undecided assumption.** Any threshold, cutoff, epsilon, tolerance, magic number, ordering
rule, fallback or special case that I did not decide. Only three things count as my decision:
an ADR in `docs/adr/`, `CONTEXT.md`, or a quote from me (a commit message, plan header or
session transcript that quotes me). A code comment explaining the value does not count, however
good it is. For each one, say who introduced it (`git log -S`) and whether I asked for it.

**Display re-derivation.** Any UI, report or CLI output that computes a game fact itself instead
of reading what the engine stored or calling the engine's own rule. The engine is the single
source of truth for scoring facts.

## Scope

Everything under `src/` (app, audio, cli, core, image, net, parse, render, search, store, ui),
`tests/` and `tools/`. Also the docs that make claims about behavior: `CONTEXT.md`, `docs/adr/`,
`docs/UserGuide.md`, `docs/cap-clamped-squeeze-frontend-anchor.md`. Skip `third_party/`.

## Method (mandatory, every step)

**Step 0: calibration run, before anything else.** One agent applies methods 1 to 3 below to
`src/core/squeeze_rating.cpp` as it stood at commit `707c285` (read it with
`git show 707c285:src/core/squeeze_rating.cpp`, plus whatever it calls at that commit). Its brief
must not say what is wrong there. The run passes only if it reports both of these:

- `rate_activation` decides which multiplier governs a note twice: four branches for backend
  rows, and a separate loop for SqIn/SqOut notes that reasons from squeeze kind and difficulty
  sign. Reduced to a truth table, both are "inside the SP end → early side, outside → late side".
- The squeezed-out note is rated twice: as its backend row against the final SP end
  (`transfer_post`), and as its SqOut entry against the pre-SqIn end (`transfer_pre`). Both
  copies hold the same stored number, the deact edge's `sqinout_timing`.

If calibration misses either one, stop. Report the miss to me with what the agent did, and fix
the method before running the audit. Only the orchestrator and the final checker see this list.
No finder brief may mention it.

**Method 1: trace from the data.** List every stored field and every input the analysis or
display reads: note offsets, transfer scales, deact, clamp and sqout ticks, collected phrase
ticks, the leeway, the hit window, every `hydra_rules.ini` value, every constant in
`core/model.h` and `core/rules.h`. For each one, list every function that reads it and the
question each read answers. Two readers answering the same question is a finding, including
inside one function.

**Method 2: trace from the screen.** For every number, word and colour on the Paths, Preview,
Stars and Dynamics tabs, in the report, and in CLI output, write the chain back to the one
function that decides it. A chain that forks is a finding. Use `hydra_uitest` and the
`build_activations` probe (see `docs/agents/ui-testing.md`) to confirm what the screen actually
shows. Don't infer it.

**Method 3: compare decisions as truth tables.** For every pair of candidate copies, write each
one as inputs → answer and compare the tables, not the wording. Different names, signs, enums
or loop shapes do not make two rules different. Edge inputs (exactly 0, exactly on a leeway or
window edge, exactly 1.0) go in the table.

**Method 4: coverage ledger.** Build the list of every function in scope mechanically (grep for
definitions), not from memory or reading. Every function gets one row: which agent read it,
which questions it answers, and for each answer whether it owns it or copies it. A function
with no row means the audit is not finished. Save the ledger as
`docs/audit/<date>-derivation-ledger.md`.

**Verification.** A different agent checks each candidate against the code. A finding survives
only if the verifier confirms every copy exists and says whether they agree. For a drift claim,
the verifier produces the concrete input where they disagree. Reading is the default. Running
`hydra_tests`, `hydra_replay`, `hydra_batch` or the probe to prove a disagreement is fine.
Nothing gets edited to do it.

**Completeness pass.** After verification, one fresh agent's only job is to find a duplicate the
audit missed, working from the ledger and methods 1 to 3. If it finds one, that's a finding, and
the pass repeats on the area it exposed. The audit is done only when a completeness pass finds
nothing new.

## How to run it

Use a workflow, under 10 agents at a time. Fan out by question family (SP windows and ends,
backend scoring, squeeze timing and ratings, fills and activation, star and score totals,
tempo and timing, parsing, store and versions, display formatting), not by folder. Every agent
brief carries the status-line rule from the fleet hooks. Run one CPU benchmark at a time.

Rules every agent follows:

- A grep that finds nothing proves nothing. Say "not found where I looked" and name where.
- Never call existing code wrong without ground truth: the engine, a test, or a run.
- Every song, file and function name comes from the repo, not memory.
- Never write "no more duplicates", "all clear" or "complete". The strongest allowed claim is
  "none found in the ledgered functions by methods 1 to 4".

## The deliverable

Write `docs/audit/<date>-derivation-audit.md` and the ledger, then show me. Follow the "How to
explain things" rules in `CLAUDE.md`: plain English, one idea per sentence, no bullet walls of
bare file:line references. Each finding gets a short paragraph covering:

- the question being answered
- every place that answers it
- its truth table, when the copies are worded differently
- whether they agree today, and the input where they don't
- what I would see on screen if they drifted
- which module should own it, and why

Group the findings by kind: drift first, then double readings of one stored fact, then
duplicates that agree today, then undecided assumptions. Within each group, rank by how visible
the effect is to me. Leave nothing in a "pre-existing, left alone" pile. Everything found is a
finding.

End the report with four things, in this order:

1. The calibration result: what the agent reported, and whether it caught both canaries.
2. Coverage: how many functions are in the ledger, and what was not checked.
3. The completeness passes: how many ran, and what each one found.
4. A short proposed order of fixes, presented as a proposal and not a plan in motion.

Wait for my approval before touching code.
