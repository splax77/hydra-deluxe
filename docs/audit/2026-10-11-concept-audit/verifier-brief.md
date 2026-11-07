# Verifier brief for the concept duplication audit (phase 4)

Every verifier reads `docs/agents/brief-preamble.md` first, then this page, then its dispatch (task id, batch, scratch folder). Where they disagree, stop and report it.

## Your job

Phases 2 and 3 found candidates: ideas that may have two or more homes in `src/`. The scouts wrote first drafts and did not grade them. You grade them. Your batch is one section of `docs/audit/2026-10-11-concept-audit/candidates.md` (V1 to V9), about nine candidates.

Read these parts of `docs/superpowers/plans/2026-10-11-concept-duplication-audit.md` (removed; `git show 444631e6:docs/superpowers/plans/2026-10-11-concept-duplication-audit.md`) first: "What counts as a finding", "Phase 4", the `findings.md` shape under "Where the outputs live", and "How findings become decisions". They define the kinds (K1, K2, K3, K0) and the fix classes.

For each candidate:

1. Read every scout block its "From" line names. They are in `groups/<group>.md` or `inventory-C1.md` / `inventory-C2.md` in the audit folder; the "also seen" blocks sit in the named group's report. Treat everything in them as a claim, not a fact.
2. Find every home in the code and read it. A candidate survives only if you find every home it relies on. A scout's line numbers may be off; find the symbol.
3. Write both tables (or every home's table) with the edges in: an empty list, exactly 0, a Stale row, a chart in no mode, a value at a window edge, a missing optional, a case-variant hash. Then either name the input where they differ, or say they agree today and what change would split them.
4. Assign the kind: K1, K2, K3 or K0. A K3 holds only per-module facts; if any entry is a rule, that rule is a K1 or K2 finding on its own.
5. Propose the owner: which home stays, and what the others call. Say why in one sentence.
6. Class the fix. It is "code-only" when no displayed word changes, no stored field changes, no score or count moves, and the merged rule gives the answer the kept home gave. Otherwise it is "needs the user", and you say which of those four it touches.
7. Say "by reading" where a claim needs a run to settle, and name the run that would prove it (a uitest script, a test case, a one-chart `hydra_batch` on a chart in `testdata/input`). Don't run it. No builds, no tests.
8. Say whether a scan row under `tests/single_owner/rules/` could flag the removed home if it came back (name the spelling it would match), or give the one sentence why no text search can catch it.

A candidate the scouts marked "known, line-level" (already in `docs/audit/2026-10-03-derivation-audit.md`) gets a short block: confirm it is still open in the code, give the finding number, and skip the tables.

## Your output

One file, `findings/V<n>.md`, written in your scratch folder and copied with PowerShell `Copy-Item` into `docs/audit/2026-10-11-concept-audit/findings/`. Don't commit. One block per candidate, in this shape:

```
### M07. <the idea as a plain question>
Kind: K1 | K2 | K3 | K0 (one line on why). Fix: code-only | needs the user (display text / stored record / score or count / a choice between behaviours).
Homes: <symbol>, <file> <lines> (<inputs>);
       <symbol>, <file> <lines> (<inputs>).
Tables: <inputs -> answers for each home, edges included; a small markdown table is fine>
Disagree on: <the input>, or "none today; <the change that would split them>".
Owner proposed: <which home stays and what the others call>.
Evidence: by reading | by reading, proven by <named run>.
Found by: <the From line>; verified by V<n>.
Scan row or reason: <a row on <spelling> in rules/<area>.json> | <why no text search can catch it>.
```

A candidate that turns out to be K0 still gets a block, with Kind: K0 and one sentence why; skip the other lines.

At the end, add a "Side notes" section for anything you noticed that is a bug but not a duplication question, with file and line. Keep it short.

## Caps

100 tool calls or 20 minutes, whichever comes first. A candidate you can't finish goes in as "unverified, looked in <files>" with no second verifier. Write and copy your file before the cap, not after it.

## Your final reply

Under 15 lines: the file's path, then one line per candidate with its id, kind and fix class, then anything you stopped on.
