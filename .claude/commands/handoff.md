---
description: Write a checkable handoff for the next session
argument-hint: <slug>
---

Write a handoff for the next session. The slug for its file name is: $ARGUMENTS

1. Stop launching work. Start no new agent and no new task until the handoff is written.
2. Read `docs/agents/handoff.md`. It owns the shape a handoff takes and how its claims are checked; follow it, not memory.
3. Write `docs/handoffs/<today>-$ARGUMENTS-handoff.md`, with today's date as `YYYY-MM-DD`, in that shape: the prose (goal, what was done, what is next), then the `## Claims` table with one row for every fact the next session will act on. Quote the user's rulings from this session in their own words, each in a `ruling` row. Write every hash in full and name no scratch path.
4. Run the handoff checker on it: `py -I tools/check_handoff.py docs/handoffs/<today>-$ARGUMENTS-handoff.md`. Fix every row it marks `missing`: the hash, branch, path or function is wrong, so correct it in the handoff. A row marked `not-on-main` or `not-pushed` is right only when its Claim says so; make the Claim say so (the run still exits 1 for it). Rows marked `manual` (test, ruling, state) are yours to check: run each test, and read each ruling and each state against what exists. Then run the checker once more. If a row is still `missing` after that second run, leave it, and say which row and why in your report.
5. Paste what the checker printed on that second run, in a code fence, under a `## Checker output` heading at the end of the handoff: every row line and the summary line it ends with.
6. Commit the handoff on its own branch, staged by name, in the commit form the preamble's "Committing" section gives (`docs/agents/brief-preamble.md`).
7. Report the handoff's path, the branch and the commit's full hash.
