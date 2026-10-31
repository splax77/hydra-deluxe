# Plan: session cards into `docs/`, finished plans and handoffs out (2026-10-31)

This plan does two things. First, the session cards move from a local folder into the repo, at `docs/sessions/`, and are published with the repo. Second, the plans and handoffs whose work is finished leave the repo, after every fact that permanent docs lean on has moved somewhere permanent. Nothing here is built yet. The build waits for the user's yes (CLAUDE.md agent rule 3).

The plan comes from the handoff `docs/handoffs/2026-10-31-session-cards-into-docs-and-handoff-cleanup-handoff.md`. Four read-only scouts mapped the ground on 2026-10-31. Every count below comes from their greps or from greps the main session re-ran on `main` at `99c732cd`. Counts drift as other sessions merge, so each build task re-runs its grep before it edits.

## The user's rulings, in their words

From the handoff (2026-10-31):

- "permanent docs should never reference or be held in scratch folders"
- "the session cards are fine but they should never reference or depend on anything in a scratch folder. they should live in the docs folder as part of the project's permanent documentation"
- "old handoffs and plans can be skipped, they are not permanent documentation. they dont need to be kept in the repo"
- On what goes public, the user chose "Cards only, published". On which plans and handoffs leave, the user chose "finished only".

Answers given in this planning session (2026-10-31):

- On the `## Sources` block every card ends with: "the transcipts eat up a lot of storage space and arent kept long term. they shouldnt be expected as a reference". So the cards drop the block. No permanent doc cites a condensed transcript afterwards.
- On the four cards that quote the user's rulings with strong language: "Keep exact (Recommended)".
- On the 23 release-note files in `docs/handoffs/`: "Move to docs/releases/ (Recommended)".
- On the 15 finished plans and handoffs that exist only as untracked files with no commit anywhere: "Archive branch, then delete (Recommended)".

## What the scouts found

**The card tools have one root.** In the hooks repo, `tools/session_cards/common.py` sets one folder (`DEFAULT_OUT`) and builds `cards_dir` and `condensed_dir` from it. The cards, `index.md`, the condensed transcripts and the working files (manifest, rulings tables, batches, numbering, tags, writer brief) all hang off it. Seven tools read or write the cards: `check.py`, `index.py`, `link_memory.py`, `number_rulings.py`, `prune_list.py`, `prepass.py` and the tests in `test_session_cards.py`. The start-of-session hook `gates/session_cards_start.ps1` takes its folder from `common.py`, so only its printed instructions change. Memory notes name cards by id, not path, so they keep working.

**Nothing commits cards today.** The hook runs `prepass.py` and `batch.py`, then tells the main session to launch Haiku card-writer agents. The main session then runs `check.py`, `index.py` and `link_memory.py`. No step commits.

**A docs-only commit on `main` needs no reviewer.** `explain_main_move` in `gates/review_rules.py` asks for a code review only when a commit touches the code paths, and a doc review only for the four public pages and the marker file. So committing new cards needs no reviewer. Every commit still needs the `Task:`, `Agent:` and `Session:` trailers. The handoff's line saying docs changes are gated is wrong for these files.

**The `## Sources` block is written by hand.** The card-writer writes it from `writer-brief.md`. `check.py` requires its first two keys, and `common.HEADINGS` requires the heading. Nothing opens the paths.

**The cards hold almost nothing private.** A grep of all 428 cards found no key, token value, email address, phone number or street address. The long strings are session ids. Three cards (31f7ae4e, 7a3a21a3, c1c32c35) say a GitHub token was pasted into chat; the token string is not in them. The update-signing key's path and the name of the rival player in the "Splax vs Boddy" sheet are already in public repo files. A grep cannot catch secrets written as prose, so a full read is still part of the build.

**Scratch lines in cards.** 99 cards hold the word "scratch" on 153 lines. Six lines name a path into a scratch folder, which the docs check would fail. About 23 more say the card's own evidence was a scratch file. Six lines say "temp folder" without the word.

**Condensed-transcript citations.** 78 lines in 19 permanent docs mention "condensed". On the Clone Hero pages, 21 lines cite a transcript. In the audit-of-audits blocks, 50 do. Two are in the decision log (D274 and D275). Four describe the card system: CLAUDE.md twice, `docs/agents/audit-method.md` and `docs/clone-hero/README.md`. One is unrelated (the word in a quote). A card keeps the user's typed words, the chosen label and the start of each question. It does not keep line-level detail, full option text or per-chart numbers. So some citations move to the card and the rest say the source is not kept.

**Plans and handoffs.** 277 files are tracked under `docs/handoffs/`, 238 under `docs/superpowers/`, and 43 are untracked in the main checkout. 31 are current and stay (the keep list below). Outside those folders, 153 markdown lines and 97 lines in code and test files cite about 80 of them. Of the cited files, about 25 hold a fact or ruling that already has a permanent home (an ADR or a D-record), so only the citation changes. About 22 hold a fact found nowhere else. About 24 are cited as history. Four are permanent material filed in the wrong place.

**One tool reads plans.** `tools/precheck/decisions.cpp` (`decision_paras`, line 307) reads the "User decisions" and "Test limits" blocks of every top-level plan in `docs/superpowers/plans/`. Precheck's check 3 uses them to accept a new number that a user decision backs. 24 plans that would leave hold such blocks. Removing them fails nothing today. But a later change that moves or re-adds a number backed only by a removed block would fail check 3, with no record left to point at. The same file reads top-level `docs/audit/*decisions*.md` files, where a paragraph counts only under a `**D<number>` record.

**Eight citations already point at nothing.** Three cited files exist only on unmerged branches (the D269/D270 plan, the D271 plan, the D271 timing handoff). Five were never committed.

**Cards cite plans too.** 90 lines in 80 cards name a path under `docs/handoffs/` or `docs/superpowers/`.

## Decisions this plan makes

These are the plan's own choices. The user's yes to the plan approves them.

**Where the cards go.** The cards go to `docs/sessions/`, one file per session with its current name (`<date>-<id8>.md`), and `index.md` goes beside them. The audit blocks, reference pages and memory notes name cards by session id, so keeping the names keeps those links.

**What stays outside.** The condensed transcripts and the pipeline's working files stay in `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\session-cards\`. They are inputs to card writing, not documentation. The card tools still read the transcripts to write new cards; no permanent doc cites one.

**How a new card reaches the repo.** The card tools write new cards and `index.md` straight into the main checkout's `docs/sessions/`. After `check.py`, `index.py` and `link_memory.py`, the main session runs the docs check (`hydra_tests.exe "-tc=docs match code*"`). Then it stages the new cards and `index.md` by name and commits on `main` with the usual trailers. The start hook's printed steps say this. No reviewer is needed, as above. A `.gitattributes` line keeps `docs/sessions/*.md` byte for byte (`-text`). Without it, line-ending changes in a checkout would change a card's sha. `prune_list.py` would then hold that session back until `check.py` ran again.

**Cards are public from now on.** `writer-brief.md` gets one short section. It says cards are published, so a card holds no secret, token or key, and names no file outside the repo except as "kept locally" or "not kept". A file that lived in a scratch folder is described as "a scratch file, not kept", never by its path.

**Plan decision blocks get a permanent home.** Before a plan with a "User decisions" or "Test limits" block leaves, the block is copied word for word into the decision log `docs/audit/2026-10-03-fix-decisions.md`. If the block belongs to an existing D-record, it goes under that record. Otherwise it goes under a new D-record that names the plan and says the block was copied when the plan left the repo. Check 3 then reads it from the log. This keeps the user's rulings, which are permanent, and needs no precheck code change.

**Citations of removed files keep their words.** A permanent doc that cites a removed plan or handoff as history keeps the line. The path is rewritten to say the file left the repo and how to read it: "`<path>` (removed; `git show <commit>:<path>`)", where the commit is the last one on `main` that holds it. Audit records keep their quoted words and line numbers, so no recorded finding or verdict changes. Where the citation backs a fact, the fact moves first and the citation points at its new home.

**Cards keep their plan and handoff paths.** The 90 lines in cards that name a plan or handoff are left as written. A card records what a past session did, at that time, the way a commit message does. The new dangling-path check (task B2) skips `docs/sessions/` for that reason. This is the one place the plan narrows "never leave a permanent doc pointing at a removed file", so it is called out for the user.

**A guard for the future.** A new docs check fails any permanent markdown file, and any code comment under `src/`, `tools/`, `tests/` or `installer/`, that names a `docs/` path that does not exist in the tree. It skips `docs/handoffs/`, `docs/superpowers/` and `docs/sessions/`, and a path written with its removal commit. It lives beside the scratch-path check in `tests/test_docs_match_code.cpp`. With it in place, a later session can remove a finished plan in the same merge that finishes its work, and the check proves nothing still points at it. CLAUDE.md gets one line saying so.

## The keep list (31 files)

A file on this list stays until its work ends. The build re-checks each one before removal.

The handoffs that stay are these. This cleanup handoff stays while it is being built. The audit-of-audits "rest" handoff stays, because one live check and an unreached list remain. The song-length handoff stays, because it holds 208 uncommitted lines that some session is still writing. Page 01 line 477 says a disassembly survives only there. The four Clone Hero kit live-steps handoffs stay, because LS-K3 run 3 is next. The hooks-from-scratch build-2 handoff stays, because T13 (deleting the old `hooks\` folder) is not done; the folder still exists. The Path Builder redesign-2 handoff stays, with the real-app-check-2 handoff and its folder. The lighting-app planning handoff stays, because nothing has started; it may belong in its own repo, which is the user's call another time. The test-audit follow-ups handoff and its report stay, because they list leftovers that have not started.

The plans and specs that stay are these. The ch-mcp in-game-input plan stays, because its live steps LS-K3 to LS-K5 have not run. The ch-mcp star-power-and-song-start plan stays, because T8 waits. The Path Builder plan stays with its design and brainstorm specs, because 27 unmerged branches edit them. The Path Builder mockup-2 folder stays. The hydra-libraries plan stays, because the Path Builder plan builds on it. The hooks-from-scratch plan stays, because T13 is open. The font-role plan stays while `claude/pb-redesign` edits it; removing it on `main` would make that branch conflict.

`release-next-notes.md` is not on the keep list. It moves to `docs/releases/` with the other release notes.

## The rule for "finished"

A file in `docs/handoffs/` or `docs/superpowers/` leaves when all of these hold, checked at the moment of removal.

1. It is not on the keep list.
2. It is not a release note. Those move to `docs/releases/` instead.
3. No unmerged branch edits it (`git log --all --not main -- <path>` is empty).
4. No tracked file outside the two folders names its path, its path without `.md`, or its folder. If one does, that citation is handled first (tasks A3, A4 and A5), and then this holds.
5. If it is untracked, it is first committed to the local branch `archive/finished-docs-2026-10`, which is never pushed, and then deleted from the checkout.

Under this rule the scouts counted about 445 files that can go once the release notes move, and about 82 more once their citations are rewritten. The build recounts.

## Tasks

Each task is one executor on Opus in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\` (or `C:\Users\Patrick\.claude\worktrees\` for the hooks repo). Each gets one fresh Sonnet reviewer before it merges (D107). A task owns only the files it names. Where two parts of the work touch one file, one task owns that file for both.

**A1. Card tools take two roots (hooks repo).** `common.py` gets a cards root separate from the data root. The cards root is the Hydra main checkout's `docs/sessions/`; the data root stays where it is. `index.py` writes `index.md` to the cards root. Every tool that reads or writes cards uses the cards root; everything that reads condensed transcripts or working files keeps the data root. The `## Sources` heading leaves `common.HEADINGS`, `check.py` and `writer-brief.md`, and the tests' fixtures and the "sources" bad case change to match. A one-time `migrate.py` copies every card and `index.md` to the new root and strips the `## Sources` block, then `check.py` refreshes the shas. `writer-brief.md` gets the "cards are public" section. `gates/session_cards_start.ps1` prints the new commit step. Tests: `test_session_cards.py`, and the start hook's own tests if any name its printed text. Because `gates/` changes, the reviewer reviews it as `C:\Users\Patrick\.claude\gates\README.md` says.

**A2. Cards land in `docs/sessions/` (Hydra).** This runs after A1 merges. It runs `migrate.py`, adds the `.gitattributes` line, and then edits the cards by hand. The six scratch-path lines and the roughly 23 "evidence was a scratch file" lines are rewritten to say the file was a scratch file and was not kept. The six "temp folder" lines get the same treatment. The 13 lines naming a folder outside the repo (other than Sources) say "kept locally" or are cut. The docs check (`hydra_tests.exe "-tc=docs match code*"`) must pass with the cards in place. Before anything is pushed, three Sonnet scouts each read about a third of the cards in full. Each one lists, for the user, any line that should not be public. The push waits for the user's answer on those lines. Quotes of the user's rulings stay exact.

**A3. Clone Hero pages and Clone Hero tools (Hydra).** This task owns `docs/clone-hero/*.md`, `docs/agents/clone-hero-mcp.md`, `docs/research/2026-10-28-clone-hero-mcp-feasibility.md` and `tools/ch_probe/` and `tools/ch_mcp/` docs and comments. It does both parts' work for these files. It re-sources the 21 transcript citations. It cites the card where the card holds the fact; the scouts found six such cases. It cites a repo file where one holds it; they found three. Otherwise it says the source is not kept. It moves the live results that `07-internals.md` cites from the ch-mcp-server plan (lines 300 to 316) onto page 07 itself. It moves the facts pages 02 and 04 take from the note-shuffle plan and the one-record part-B2 timing plan. It moves the ch-dynamic-input-probe spec beside `tools/ch_probe/`. It fixes `tools/ch_probe/constants.py` line 105, which says an offset awaits a hit-window plan step that D28 shelved; page 03 TM-14 says that offset is not a hit time. It fixes the feasibility doc's line 115, which calls a tracked handoff uncommitted. It checks the claim on page 01 line 53 that card 7c71d4dd says "all 30 stored header values match". The scout could not find it in the card. `README.md` on the reference gets its card-system wording updated.

**A4. Audit records, decision log and agent pages (Hydra).** This task owns `docs/audit/**`, `docs/agents/audit-method.md`, `CLAUDE.md` and `CONTEXT.md`. It re-sources the 50 audit-of-audits transcript lines to the cards, or to "not kept" where the card lacks the detail; two lines there already say so. It copies the question text D274 and D275 cite into those records, since the cards hold only the start. It rewrites the card-system lines in CLAUDE.md ("Past sessions"), `audit-method.md` and `checkquotes.py` for `docs/sessions/` with no transcripts. It copies the 24 plans' decision blocks into the decision log, as "Decisions this plan makes" says. It rewrites every audit-folder citation of a leaving plan or handoff in the removed-path form. It moves the facts that D54, D104, D113, D115 items 6 and 7, and D272 take from plan text into those records.

**A5. Code comments, tests and tool notes (Hydra).** This task owns the 97 code and test lines that cite a plan or handoff, plus `docs/development.md`, `docs/adr/*`, `tests/single_owner/rules/*.json`, `tests/test_menu_tools.py` and tool READMEs. The auto-updater plan's version rule moves to `docs/development.md`, unless an ADR already covers it. The mimalloc trial numbers move there too, and so do the merge-test picker spec and the precheck picker's user-set numbers. The capture scripts in `docs/handoffs/2026-10-10-maximized-report-stretch/` move to `tools/capture/`, and `tools/capture_window.ps1` points there. The perf recipe `tools/compare_db.py` cites moves into that tool's header or README. The about 14 single-owner rule rows that name a plan in their `decision` field point at the D-record instead (from A4). The plan path that `test_menu_tools.py` line 414 asserts changes too. ADR citations point at the ADR's own text or the D-record. This touches `src/` and `tests/`, so its reviewer does a code review too.

**A6. Release notes move (Hydra).** `git mv` the 23 release-note files to `docs/releases/`, and fix every link to them. The memory notes `windows-installer` and `midi-drum-track-names-d252` name `release-next-notes.md`; the main session updates them after the merge.

**B2. The dangling-path check (Hydra).** The new check described under "A guard for the future", with a self-test like the scratch check's. It starts after A3, A4, A5 and A6 merge, because only then does `main` pass it.

**B3. Removal (Hydra).** This runs last, after B2 merges. One executor applies "The rule for finished" afresh, file by file. It `git rm`s the tracked files that qualify. It commits the qualifying untracked files to `archive/finished-docs-2026-10` and then deletes them from the main checkout. The untracked files sit in the main checkout, so this step is the main session's, not an agent's: an agent writes only its own worktree. The executor writes the list, and the main session does the archive and delete. The dangling-path check must pass after the removal. The removal note lists every file removed and the commit to read it from.

**After the merges.** The main session updates the memory notes `session-cards-backfill-2026-10` and `permanent-docs-never-cite-scratch`. It also updates every memory note that names a removed plan or handoff.

## Order and parallel runs

A1, A3, A4, A5 and A6 start together. They own different files. A3 and A4 cite cards at `docs/sessions/<file>`, which exists only after A2. So A2 merges before A3 and A4, and their reviewers check the card citations against the merged `main`. A4 writes the D-records that A5's single-owner rows point at, so A5's earlyStart lists A4: A5 cuts from A4's executor branch once A4 commits, and A4 merges first. B2 waits for A3 to A6. B3 waits for B2. Nothing is pushed until A2's privacy read is answered.

## Tests at merge

Each merge runs what `build-cpp\Release\hydra_precheck.exe --repo <path> --tests <range>` prints on its `tests:` and `always:` lines (CLAUDE.md rule 1). The `always:` lines include the docs-vs-code check, which now covers the cards. A1's merge in the hooks repo runs `test_session_cards.py`, plus the whole hooks suite only if it touches shared code.

## Not in this plan

The condensed transcripts are not deleted. The user said they are not kept long term, but no rule yet says when they go, and a pruning tool is its own task. Plans and handoffs that live only on unmerged branches (the D269/D270 and D271 plans, the Path Builder redesign-2 plan and the harness plan) are not touched. When they merge, the "remove in the finishing merge" line in CLAUDE.md covers them.
