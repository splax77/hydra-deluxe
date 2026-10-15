# Handoff: session cards, after round 4 (2026-10-14)

This replaces `2026-10-14-session-cards-writing-handoff.md`. The plan is `docs/superpowers/plans/2026-10-14-session-cards-backfill.md`. The user stopped the work after round 4 and asked for this handoff. Nothing is running except what the "Script fixes" section says.

## Where things stand

The scripts are live on `.claude` main in `C:\Users\Patrick\.claude\tools\session_cards\`. Since the last handoff, the card cap went from 45 to 60 lines at the user's request (4f18f31, with the test that pins it now reading the constant).

The data folder is `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\session-cards\`. After round 4, `check.py` reports 126 cards: 120 checked and 6 failed. `prepass.py` then found 312 sessions condensed and 7 still inside the two-hour cutoff, and `batch.py` gives 186 sessions without a card in 21 batches, so 3 more rounds.

The writer brief is `session-cards\writer-brief.md` (copied out of the session scratch folder, which the cleanup sweep empties after 7 days). Read it before launching writers; every change below is already in it.

## Decisions the user made in this stretch

The cap is 60 lines, and cards that had dropped rulings under 45 were redone (08a569fc, a9a461da, 8157f2dc). Card writers run on Haiku, not Sonnet (memory file `haiku-for-simple-readers.md`). The user approved the two script fixes below.

## How the rounds were run

Writers never run `check.py`. The manifest is saved through one shared temp name (`manifest.json.tmp` in `common.save_json`), so parallel runs would collide. The main session runs `check.py` once after each round, in one process, then `batch.py` to rebuild the batches. `batch.py` renumbers from `wave-1` every time it runs, so the next round always uses `wave-1-batch-1` to `wave-1-batch-10`.

Haiku writers kept calling tools the brief forbids. Four of them made stray calls in rounds 3 and 4: three `spawn_task` chips (each withdrawn by the writer) and two browser calls (both did nothing). The brief's tool rule did not stop them. Before the next round, define a card-writer agent in `C:\Users\Patrick\.claude\agents\` whose tools are limited to Read, Glob, Grep, Write and PowerShell, and dispatch writers with that type. That is a change in the `.claude` repo, so it goes through a worktree like any other.

Cost per writer, from the agents' usage reports: about 150,000 tokens for Sonnet in round 1 (one big session each), and 200,000 to 265,000 for Haiku in rounds 3 and 4 (three to six sessions each). Haiku saves on price per token, not on token count.

## Failed cards (for Task 5's one retry)

Four fail on parentheses inside the note after a ruling, which the checker cannot parse: a918c71b, 35360d51, and the chose lines in fb6cd471 and c4589620 (c4589620 has ten such lines). One fails on a roadblock line without its ` -> ` parts (1c03b11e). One cites a mistyped hash, `037adcab4` (557b3904). Run `py -I batch.py --failed` after the last round and send those to writers with the checker's messages.

## Script fixes (SC-5)

Built, not yet reviewed or merged. The branch is `sc-5-fixes` in the worktree `C:\Users\Patrick\.claude\worktrees\sc-5-fixes`, with three commits: 719b4f8, a037018 and 95d2c38. The main session reran its tests at handoff: 41 pass. `.claude` main has since moved to 3ad67ed (another session's merge), so merge main into the branch, with the three trailers on the merge commit, before the fast-forward the hooks README describes.

The first fix lets a chose line hold a label with quote marks: the label now runs to the first `" to "`. The ruling patterns live once in `common.py`, and `check.py` and `number_rulings.py` both use them. The second fix counts agents that workflow runs launched. It groups launches by run id and counts the `agent-*.jsonl` files in each run's folder under `<session>\subagents\workflows\<runId>\`, falling back to the largest `<agent_count>` in a completion notice. Counting folders matters because a stopped run sends no notice and a resumed run keeps its run id (the executor found both in real sessions). `meta.json` gains `subagents_direct`, `workflow_runs`, `workflow_agents`, `workflow_runs_uncounted` and `workflows`, and `subagents_launched` becomes the total. `prepass.FORMAT` is unchanged, so checked cards stay removable.

On a scratch copy of the data, the executor's trial refreshed all 312 sessions with no failures and left every `user_text.json`, condensed body and card byte-identical. Agent totals across all sessions went from 1,395 to 2,342 over 117 workflow runs. 32 sessions changed; b5aa7c31, for example, went from 0 to 253. Rewriting the cards' `agents:` field changed 29 of 125 cards and nothing else in them.

After the merge, with no writers running, run these two commands. The second rewrites the `agents:` fields and rechecks in one go:

```
py -I C:\Users\Patrick\.claude\tools\session_cards\prepass.py --meta-only
py -I C:\Users\Patrick\.claude\tools\session_cards\check.py --refresh-agents
```

The reviewer should also look at one gap the executor left alone: the note in parentheses after a ruling cannot contain parentheses. That is what four of the six failed cards trip on. Either the pattern allows one level of nesting, or the brief keeps forbidding it and the retries fix the cards.

## Next steps, in order

1. Finish SC-5: one fresh Sonnet reviewer, then fast-forward `.claude` main, then run the commands SC-5 lists (meta refresh, card `agents:` rewrite, `check.py`).
2. Add the restricted card-writer agent type.
3. Run `prepass.py` and `batch.py`, then rounds 5 to 7 with that agent type, checking after each round.
4. Tasks 5 to 9 of the plan: retries, `link_memory.py`, the 12-card spot-check, numbering (the counts go to the user before the decision log is touched), and the transcript removal list, which the user runs.

## Small follow-ups found during the writing

The condensing step finds a commit only when its hash shows up in tool output, so quiet commits and release scripts leave no trace. The 2026-08-18 session made nine commits by its own summary and the metadata holds 2. Commits since the trailer rule carry `Session: <8 hex>`, so `git log --grep "Session: <id>"` can rebuild the list; three doubted commits checked this way all matched (7227894, 95bab12, 4ce4504). Older commits can be matched by date. Nothing is lost when transcripts go, because git keeps both.

Three typed messages never reached Claude and are stored as `unsent` (sessions 4f777343, d3bdb873, b2623752). No card may quote them.

## Things the cards surfaced for the user

These are facts about old sessions, written on their cards under Open threads. In the 2026-08-15 and 2026-08-18 sessions, Claude closed running Hydra windows without asking. In the 2026-08-10 rewrite, a download approved at about 120 MB was 274 MB and went ahead without a new question. A 2026-10-08 session found the installed Hydra stopped running after a hands-on check, cause unknown. Several sessions left questions unanswered: whether to run a whole-library batch for the memory peak (88fa5e4c), five performance choices (6dce3c94), the `--decoder nvdec` question (11668887), and the Scope-line question (ead503d1). The card for e40fb55d was corrected by the main session: the decision log no longer records a library-run approval, so the dispute it describes is settled.
