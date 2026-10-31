# Handoff: session cards into the docs, and finished plans and handoffs out of the repo (2026-10-31, session 933c3f22)

This is a planning handoff. Nothing here is built yet. A new session plans two pieces of work, takes the plan to the user, and builds only after the user's yes (CLAUDE.md agent rule 3).

## The user's rulings, in their words

On 2026-10-31, after finding that the Clone Hero reference cited files in a scratch folder:

- "permanent docs should never reference or be held in scratch folders"
- "the session cards are fine but they should never reference or depend on anything in a scratch folder. they should live in the docs folder as part of the project's permanent documentation"
- "old handoffs and plans can be skipped, they are not permanent documentation. they dont need to be kept in the repo"

Then, asked two questions:

- Asked what goes into `docs/`, given that the GitHub repo `splax77/hydra-deluxe` is public, the user chose **"Cards only, published"**. The 421 cards move into `docs/` and are pushed publicly. The condensed transcripts stay outside the repo.
- Asked which plans and handoffs leave the repo, the user answered **"finished only. create a new handoff for another session to plan"**. This file is that handoff.

The memory note `permanent-docs-never-cite-scratch` records the first ruling as a top rule.

## Part A: the session cards move into `docs/`

### What exists today

The card system lives outside the repo, in `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\session-cards\`. That folder holds:

- `cards\`: 421 files, 1.6 MB, one card per past session.
- `index.md`: one line per card.
- `condensed\`: 1,284 files, 15 MB, the condensed transcripts. These stay outside the repo.
- Working files of the card pipeline: `batches\`, `rulings-parts\`, `manifest.json` (and a `.bak`), `numbering.json`, `prune-list.txt`, several `rulings-*.tsv` files, `tags.txt` and `writer-brief.md`.

The tools that read and write the cards are in the hooks repo, under `C:\Users\Patrick\.claude\tools\session_cards\`: `batch.py`, `check.py`, `common.py`, `index.py`, `link_memory.py`, `number_rulings.py`, `prepass.py`, `prune_list.py`, and their tests in `test_session_cards.py`. The agent definition `C:\Users\Patrick\.claude\agents\card-writer.md` and a start-of-session hook also name the folder. Memory notes end with a `Sessions:` line that `link_memory.py` reads.

In the Hydra repo, these name the card folder: `CLAUDE.md` ("Past sessions"), `docs/agents/audit-method.md`, five blocks under `docs/audit/2026-10-29-audit-of-audits/blocks/`, and `docs/audit/2026-10-29-audit-of-audits/briefs/checkquotes.py` and `common.md`.

### What the plan must settle

1. **Where in `docs/` the cards go,** and whether `index.md` goes with them. Keep the card file names, because memory notes, the reference pages and the audit blocks name cards by session id.
2. **No card may reference or depend on a scratch folder.** A grep of the cards for "scratch" (any spelling) matches 98 of them today. Each such line is rewritten to say the file was not kept, or points at a repo copy. The guard test below then checks the cards too, because they will live under `docs/`.
3. **Cards are published.** The repo is public, and cards quote the user's words and name local paths. Before the first push, read every card for anything that should not be public (credentials, tokens, private names or addresses beyond the user's own folders) and put any doubt to the user. The user chose to publish the cards; this check is for anything that was never meant to be in a card.
4. **The condensed transcripts stay outside the repo,** so every permanent doc line that cites one needs a new source. On the Clone Hero reference, 18 lines cite a condensed transcript and 35 cite a card (counted on `main` at the time of this handoff). Outside the reference, 11 permanent docs cite a transcript (excluding plans and handoffs). Each transcript citation is re-sourced to a repo file, or to the card when the card holds the fact, or it says the source is not kept.
5. **The card pipeline's working files** (manifest, rulings tables, batches, numbering) are working state, not documentation. Decide whether they stay outside the repo; the ruling says permanent docs move, and these are not docs.
6. **The tools and hooks** read the new place: every path in `tools/session_cards/*.py`, the card-writer agent, the start-of-session hook and `CLAUDE.md`. That is a hooks-repo change, so it goes through a worktree of `C:\Users\Patrick\.claude` and one fresh reviewer, as `C:\Users\Patrick\.claude\gates\README.md` says. How a new card reaches the repo (who commits it, and whether a doc-review gate applies) is part of the plan.

## Part B: finished plans and handoffs leave the repo

### What exists today

514 files are tracked under `docs/handoffs/` and `docs/superpowers/` (plans, specs and their sub-folders). 49 permanent docs outside those two folders cite a file inside them, including `CLAUDE.md`, pages under `docs/agents/`, the ADRs, and 10 lines on the Clone Hero reference (for example IN-03 and IN-36 cite `docs/superpowers/plans/2026-10-28-ch-mcp-server.md` lines 304 to 316 as evidence). Git keeps every removed file in history either way.

### What the plan must settle

1. **Which are finished.** A plan or handoff is finished when its work is merged and no session is still working from it. Some are current: for example, sessions on 2026-10-31 were still running the ch MCP kit live steps from `docs/handoffs/2026-10-31-ch-mcp-kit-live-steps-handoff.md` and its `-2` and `-3` follow-ups. Those three sit untracked in the main checkout, as do dozens of other handoffs and plans (`git status` there lists them); the plan says what happens to untracked ones too. Current ones stay until their work ends. Say how each one was judged.
2. **Move what permanent docs depend on, first.** For each citation in the 49 docs, the fact moves to its permanent home (a reference page, an ADR, the decision log, a doc under `docs/agents/`), or the citation is rewritten. Only then is the plan or handoff removed. Never leave a permanent doc pointing at a removed file.
3. **The derive-once and doc-review gates** count `docs/` changes, so the removal and the moves are reviewed like any change.

## The guard test, in progress

Branch `claude/chref-guard` (worktree `.claude\worktrees\chref-guard`) adds a check to `tests/test_docs_match_code.cpp`. It fails any tracked `.md` file that names a path into a scratch folder: a Temp path, a `scratchpad` path segment, `chref-scratch`, or a `scratch_ms` path. It skips `docs/handoffs/` and `docs/superpowers/`, per the user's ruling that those are not permanent documentation. Once the cards are under `docs/`, the check covers them, so Part A step 2 must land with or before the move. At the same time, branches `claude/chref-nsa` and `claude/chref-nsb` remove every scout and scratch citation from the Clone Hero reference pages. Check where these stand (`git branch --list 'claude/chref-*'`, `git log main`) before planning; they may have merged already.

## Where to look

- `CLAUDE.md`, "Past sessions" and "Doc review".
- `C:\Users\Patrick\.claude\gates\README.md` for the gates and the hooks-repo review flow.
- The memory notes `permanent-docs-never-cite-scratch` and `session-cards-backfill-2026-10`.
- `docs/clone-hero/README.md` for how the reference cites evidence.
