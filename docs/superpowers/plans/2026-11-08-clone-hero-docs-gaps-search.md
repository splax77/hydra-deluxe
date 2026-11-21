# Search for Clone Hero facts the reference pages lack

Date: 2026-11-08. Planner: Fable, task CHDG-plan, session 77057df2. Comes from the handoff `docs/handoffs/2026-11-08-clone-hero-docs-gaps-handoff.md` (removed; `git show 13738279889ceb4840285f1eba78ba46c01c2fa5:docs/handoffs/2026-11-08-clone-hero-docs-gaps-handoff.md`) (on branch `claude/handoff-31e4f22f`, not on main). The user's words behind it: "are the facts actually unknown or did you never check what we already have documented?" and "create two handoffs. one to continue this work, and one to search for other documentation missing from our central docs. are either of those real problems, or edge cases you invented?" Today the user said "Yes, plan it" to a plan that settles TM-50's two-note rule first, because the player audit (`docs/handoffs/2026-11-08-player-audit-continue-handoff.md` (removed; `git show 13738279889ceb4840285f1eba78ba46c01c2fa5:docs/handoffs/2026-11-08-player-audit-continue-handoff.md`), lines 43 and 61) waits on that rule.

## The plain version

The Clone Hero reference under `docs/clone-hero/` is meant to be the one home of what we know about the game. It is not complete. Facts proven elsewhere in the repo never reached it, so its pages call things Unknown that a committed listing already settles. The clearest case: page 05 says the step-loop listings `0x20F4950` and `0x20F6FF0` are "not listed", and both have sat committed under `docs/audit/drum-lanes/play/` since October.

This plan is a read-only search for every such gap. It reads nothing in the game and runs nothing; it only reads files we already have. Read-only scouts on Sonnet read each source that holds game facts and ask three things of every fact: does a page already hold it, does the page's label say less than the evidence earns, and does the page contradict the source. A second set of scouts works from the other side: they take every page block labelled Unknown or "Hydra assumes it" and look for proof anywhere in the repo. Fresh agents check every candidate. The main session then gives each finding one of the audit method's four exits, a fresh agent checks every closure, and the user gets one list of write-backs. Nothing on the pages changes until the user says yes to that list.

One step runs first and alone, before any fan-out: TM-50's two-note rule. While this plan was being written, the player audit's own session already read the press handler and rewrote TM-50 as Proven in the game's code, in commit `fb1be3cd` on branch `claude/player-audit-plan` (not merged). So the first step is no longer a fresh read. It is a check of that text against the listings, because the player audit waits on it.

## Before anything runs

No scout or verifier starts until the user approves this plan and answers its three questions. "Yes, plan it" was a yes to planning, not to launching. The user approved on 2026-11-08 ("approve but every fact must be checked against the real code"), with the three questions answered as recommended; "The finding row" below says what that rule means for each row. Once the user approves, T0 runs alone and the main session reads its result before T1 starts. Scouts read this plan's worktree (the Clone Hero pages and listings there match main), write only to their report and their scratch folder, and never build or run the game.

## What was checked, and how

Every handoff claim this plan builds on was reopened today. The Premise map at the end names each source. Five things needed sharpening:

- SP-29 and SP-30 are still Unknown, and still say `0x20F4950` is "not listed" (page 05 lines 304 and 313 to 314). Both listings are committed: `docs/audit/drum-lanes/play/f20f4950_step_loop.asm` (115 lines) and `f20f6ff0_next_event.asm` (131 lines). So the handoff's first claim holds.
- TM-50's Game code line on main still says "which note it takes when two match was not read" (page 03 line 328), so the handoff holds against main. But commit `fb1be3cd` on `claude/player-audit-plan` (14:50 today, session 274bd9dd, eight minutes before this plan was committed) rewrites TM-50 as Proven in the game's code. It cites the press handler and the base update `f20f5810_base_update.asm`, and says the window array is filled in chart order. Neither handoff knows this: the docs-gaps handoff says nobody has written the rule down, and the player-audit handoff lists Q11 as unanswered. T0 below checks that text instead of redoing the read.
- FL-31's Evidence line cites only LS-O3 run 1 (page 04 line 326). LS-M2 run 1 (`hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1`) has nine chords the player declined (239, 800, 827, 1004, 1029, 1328, 1617, 1662, 1989, `replies.txt` line 3) and each one's `press-2.csv` row shows the score rising 200 or 400 with `hit` reading `declined`. The handoff holds.
- The handoff's TM-25 claim is partly stale. It says FINDINGS "adds that a press is valid up to and including the window end"; TM-25's Rule line already says that (page 03 line 295). What TM-25 lacks is FINDINGS line 104's rounding-tie case (a grant that lands one scheduled time later). That becomes one row for the scouts, not a settled gap.

I did not take the handoff's "Freak chord 803" line on trust: TM-50 names chords 119, 120 and 152 to 154 and never 803, but I did not find which run holds 803. It is an `unchecked` premise, settled by scout S3.

## Rulings that bind this plan

- "are the facts actually unknown or did you never check what we already have documented?" (the user, session 31e4f22f, quoted in the handoff's rulings list and in this plan's brief). The search exists because of this.
- "Yes, plan it" (the user, 2026-11-08, quoted in this plan's brief): TM-50's two-note rule is dealt with first.
- The audit-scope rulings on `docs/agents/audit-method.md` line 36: "the first pass should cover everything" and "Leave nothing in a "pre-existing, left alone" pile." So every source family below is in, and the only things left out sit under "Out of scope" for the user to approve.
- Transcripts are not a source: "arent kept long term. they shouldnt be expected as a reference" (`docs/clone-hero/README.md` line 32). A fact that lives only in a transcript gets no row.

## The source inventory

These are the places in the repo, and the two places beside it, that hold game facts outside `docs/clone-hero/`. I sized each one today with directory listings and greps, so the scout split below is honest. "Cited" means an area page (01 to 07) names the file.

The drum-lanes audit, `docs/audit/drum-lanes/` (48 files, 852 KB), is the richest source. Its `FINDINGS-play-time.md` is about 190 lines and holds the press handler, the grant rule, the double-lane timer, the three Star Power times, the step loop and the rounding-tie case. It has 31 listings; 13 are cited by a page and 18 are not, among them the step loop, the next-event reader, the advance schedule, the add-event routine, the base miss, the combo breaker, the input poll and the overhit path.

The Note Shuffle audit, `docs/audit/note-shuffle/` (294 files, 2.7 MB), has 179 listings and five notes files: `step0/FINDINGS.md` (10 KB), `step0/open1/notes.md`, two `open2` READMEs, and `game-tests/README.md` (18 KB, ten tiny songs played in the real game, with one result JSON beside it). Its `probes/` folder holds three scripts.

The reference's own code folder, `docs/clone-hero/code/`: 91 listings under `6142/`, each with a `.md` beside it, plus four `4045/` files. One listing, `two_x_kick_drop_20F0320.asm`, is cited by `code/README.md` only and by no area page. `code/README.md` (335 lines) indexes 210 older listings and says which address each holds, so it is the map a scout uses to find a listing for an Unknown block.

Live-run folders under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\`: 41 run folders across five dates; the area pages cite 18 of them by path and name 19 distinct `ls-*` run ids. Each folder holds `replies.txt`, press CSVs, a Star Power watch CSV and score stats. The 23 uncited folders are where proven-in-a-run facts with no block are most likely.

Plans under `docs/superpowers/plans/`: 8 of the 10 committed plans name game addresses or Clone Hero facts (the harness, ch-run-timing, player-limits, player-refinements, shell-output, kick-axis-bind, player-freak-fixes and replay-press-schedule plans; 36 to 172 KB each). Their "What was checked" sections and Premise maps record game reads. 237 plan files were removed from the tree and live only in git history; they are out of scope below.

Session cards: 80 cards carry the `clone-hero` tag (314 KB in all). Three carry `[write-back]` in `docs/sessions/index.md` (lines 460, 469 and 475): card 70bddae3 says SP-17 lacks the dynamics handler `0x20DE770` (and page 05's SP-17 still does not name it); card b132faae names the frame-rate drop under builds, which TM-49 now holds but without that session's id; card d0a3868c names the frame-read fact, which SP-47 now holds.

The user's rulings file `docs/agents/rulings/clone-hero.md` (19 lines): seven of its rows end "backs: no page yet", so seven rulings that touch game behaviour are not tied to any block.

The decision log `docs/audit/2026-10-03-fix-decisions.md` (402 KB): 76 lines name Clone Hero or CH. Six ADRs under `docs/adr/` name it. `docs/audit/ch-evidence.md` is a pointer file to the pages and needs no read.

Hydra's own code and tests: 253 lines in 68 files under `src/`, 283 lines in 57 files under `tests/`, and 902 lines in 105 files under `tools/`, match "Clone Hero", "CH", "the game" or a seven-digit address. A comment that asserts what the game does is a claim with no block, or an "assumes" block's home.

The tools' own docs: `tools/ch_probe/constants.py` (359 lines, the one home of method addresses and field offsets), `tools/ch_probe/SPEC.md` (16 KB), `tools/ch_probe/README.md`, `tools/ch_mcp/README.md` (18 KB) and `tools/ch_probe/experiments/results/overlay-spike/il2cpp-fields.md` (33 KB of game fields).

Agent pages: `docs/agents/clone-hero-mcp.md` (74 lines) and `docs/agents/this-machine.md` (79 lines, the only agents page that names addresses).

Public pages: `docs/UserGuide.md` (31 lines name the game) and `docs/differences-from-public-hydra.md` (22 lines). `docs/srb-format.md` (11 KB) and `docs/research/2026-10-28-clone-hero-mcp-feasibility.md` (25 KB).

The three handoffs under `docs/handoffs/` and the untracked ones in the main checkout's status.

Memory files under `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\`: 21 name the game. CLAUDE.md says a game fact never lives in a memory note, so any fact still only there is itself a write-back.

What I searched to widen the handoff's list: every folder under `docs/`, by name and by grep for addresses and the game's name; `src/`, `tests/` and `tools/` by the same grep; `.claude/agents` in both repos; the memory folder; and git history for removed plans. A search that finds nothing proves nothing, so the inventory names where each count came from, and scout S8 re-greps the whole tree with wider terms (the obfuscated-name forms `#dddd.` and `#dddd+0x`, `RVA_`, `OFF_`, `GameAssembly`, `dump.cs`) before the search closes.

## How the search runs

It runs as an audit by `docs/agents/audit-method.md`: plan, calibrate, find, verify, choose exits, check every closure, ask, record. Here a "finding" is one game fact whose page state is wrong: missing, under-labelled or contradicted. A "split" is a page block and a source that state different rules.

The four exits, as this audit uses them. **Fixed** means the write-back is written on the page, later, after the user's yes; until then no finding is fixed. **A question to the user** is the exit for every split, and for any write-back that would change a block's Rule line to say something different (not just more). **Covered by a ruling** needs the user's exact words, file and line, naming both the page's wording and the source's. **Not a problem** needs one sentence saying why the page and the source answer different questions, which the closure checker agrees with. "Already known", "obvious" and "the page implies it" are not exits.

Only the main session picks an exit. Scouts and verifiers never do. Scouts write their findings only to their report and to the running session's scratch folder; they touch no repo page and no other file.

### The finding row

Every candidate, from any scout, is one row with these seven cells. Page and block id, or "new" with the page it belongs on. Source file and line (or run folder and result file and row). The label the evidence earns, one of the README's four. What the page says today, quoted, or "no block". One sentence of what to write. The kind: missing, under-labelled, contradicted, or stale (the page cites a file or session that the source shows changed). The exit is blank; the main session fills it.

The user approved this plan with one rule (2026-11-08, this session): "plans are another agent's summary and should not be treated as authoritative facts. the code is the source of truth. approve but every fact must be checked against the real code." So every row's source cell names the real thing: a line of a committed game listing (`.asm`), a row of a run's own data file (a press CSV, a Star Power watch CSV, a score-stats JSON), or a line of Hydra's code. Prose is never the source, whoever wrote it. That covers plans, FINDINGS files, notes files, `code/` `.md` write-ups, session cards, handoffs, memory notes, the decision log, a run's `replies.txt` summary lines, the other branch's TM-50 text, and the Clone Hero pages themselves. Prose is a lead: the scout follows it to the listing or data it names and quotes that line. A fact whose lead names no listing or data, or whose listing or data does not show it, earns Unknown, and its sentence says where we looked. A row whose source is a transcript is dropped. Verifiers in T4 reopen the named listing or data line, never the lead.

### Caps and the stop rule

Every scout and verifier gets a cap of 90 tool calls or 20 minutes, whichever comes first, below the hook's own 100-call and 20-minute warning. At the cap it stops, and its report lists the jobs it finished, the jobs it did not start, every file it opened, and no guess for the rest. A job it did not finish goes to the next round by name, never as "whatever was left". The search has two find rounds and one verify round, and then it stops. A third find round happens only if the user asks for one after seeing the list; the plan does not promise completeness, it promises that every named source was read or is named as unread.

## Questions

1. The 237 plans removed from the tree hold game reads only in git history. Leave them out (recommended: the facts they found were written to pages or cards at the time, and `code/README.md` indexes their listings), or send one scout through `git log --diff-filter=D` to grep each one for addresses? The case for reading them: the Note Shuffle plan's Part 4 lives only there (`game-tests/README.md` line 3 cites it by `git show`).
2. The 1.3 MB derivation audit and ledger (`2026-10-03-derivation-audit.md`, `-ledger.md`) are grepped for the game's name only, not read. Agree?
3. Where the final list lives: this plan commits it as `docs/audit/2026-11-08-clone-hero-docs-gaps/findings.md` so later write-backs can cite a permanent record, and shows it in chat in batches of five or fewer splits per message. Agree?

## Tasks

### T0: check TM-50's two-note rule against the listings (one scout, Sonnet; runs first and alone)

TM-50 on main (page 03 line 323) is Proven in a live run and says the press goes to the earlier of two unhit same-pad notes; its Game code line says the handler's choice "was not read". Commit `fb1be3cd` on `claude/player-audit-plan` already rewrites it as Proven in the game's code (premise P35). The text says: the base update fills the window array in chart order; the press handler walks that array from its first entry; a note takes the press when one of its pads is pressed and no earlier note has used that pad. So each pressed pad hits at most one note, the earliest unjudged one in its window.

I read the press handler myself (premise P36), and my reading agrees. In `docs/audit/drum-lanes/play/f20deaa0.asm` an outer loop runs from line 72 to line 162 and an inner loop from line 99 to line 157. Lines 104 to 106 call `0x18214b6d0` and skip the note when it returns true. Lines 109 to 113 mask the note's pads (`[rsi+0x80]`) against the pressed pads (`[rdi+0x152]`) less the pads already used (`bx`). Line 114 adds the note's pads to the used set, and line 152 hits the note through the slot at `+0x1E8`. What I did not read is the window array's fill order (premise P9) and what `0x18214b6d0` tests (premise P8). The other branch's text claims both: it cites `docs/audit/note-shuffle/scout-flag/note_getters.txt` line 9 for the test, and `f20f5810_base_update.asm` lines 57 to 102 for the fill.

The scout's job, in order. Read `git show fb1be3cd` and TM-50's new text. Check every line it cites against the listings, quoting each line: the press handler lines above, the getter at `note_getters.txt` line 9, and the base update's cursor and window-fill lines. Then say whether each sentence of the new Rule line holds, including "on every pad", "an early press as well as a late one" and the sentence about two presses in one update. Last, say whether the live runs the text cites fit it.

What each result means. If every sentence holds, TM-50 earns **Proven in the game's code** and the rows P8 and P9 are checked; the player audit's Q11 is answered from that text with that label; and the page change is already written on the other branch, so this search adds no TM-50 row of its own. If a sentence does not hold, the scout quotes both sides, and it goes to the user as a question before the player audit continues (it is not settled by reading). If a cited listing does not show what the text says, the label drops to **Proven in a live run** and the open piece is a code read still owed. The main session opens the scout's named lines itself before repeating any of this (rule 9).

Acceptance: the scout's report quotes each line it rests on; the main session opens those lines and sees the same instructions; the verdict carries exactly one of the four labels and cites only committed files. Cap: 60 calls or 15 minutes. At the cap the scout reports which sentences it checked, which it did not, and the last line it read.

### T1: calibrate the finder brief (one scout, Sonnet; before T2)

One scout gets the finder brief and the drum-lanes FINDINGS file with pages 03 and 05 as its target, and no hint of what is wrong. It must report the SP-29 and SP-30 "not listed" gap. If it does, its other rows are kept and its unfinished sections go to S1 by name. If it misses that gap, the main session stops, reports the miss to the user, fixes the brief, and only then launches the rest. The brief must not name the gap or an example near it.

Acceptance: the calibration report names `f20f4950_step_loop.asm` or `f20f6ff0_next_event.asm` against SP-29 or SP-30, with the page line and the file. Cap: 90 calls or 20 minutes.

### T2: find round 1, the source side (four scouts in parallel, Sonnet)

Each scout owns sources, reads them, and for each game fact greps all seven area pages, the questions page and the live-checks page for the fact (by address, by field, by the rule's words). It writes one finding row per gap. No two scouts own the same source. About eight jobs each.

S1 owns the drum-lanes audit. Its jobs: the lane marking and fields section against NL-56 and TM-45; the miss-becomes-hit section against TM-45 and SP-20; the flag 0x800 and press-handler sections against FL-31, SP-20 and TM-50 (T0's result is given to it, so it does not redo T0); the combo breaker (slot 20, role field) against pages 06 and 07; the double-lane timer against page 03; the three Star Power times and the hit-clock sentence at line 98 against SP-36 and SP-28; the step loop and rounding tie (lines 100 to 108) against SP-29, SP-30 and TM-25, including the untraced caller at line 108; and the 18 uncited listings, each one's role from FINDINGS or `code/README.md` and whether any block should cite it.

S2 owns the Note Shuffle audit. Its jobs: `step0/FINDINGS.md` against page 02's shuffle blocks; the `open1` and `open2` notes; `game-tests/README.md` and its result JSON (ten songs played in the real game; each predicted row is a proven-in-a-run candidate if the JSON holds the result) against pages 02 and 06; the three probe scripts and `verify/`; and a sample of the 179 listings: every listing `code/README.md` gives a role, checked for a citing block, with the rest counted and named unread.

S3 owns live runs, plans and the decision log. Its jobs: the 23 run folders no page cites, each one's `replies.txt` read, with one grep across all the folders, for a result line that states a game rule or number (such as LS-M2's nine granted chords for FL-31, and the run that holds Freak chord 803, premise P12); the 8 plans' "What was checked" sections and Premise maps, each `run` or `data` row checked for a block; the 76 decision-log lines; and the six ADRs.

S4 owns the reference's code folder and the tools. Its jobs: `two_x_kick_drop_20F0320.asm` and the uncited `4045/fill_deadline.md`; `code/README.md`'s facts tables against page 07 (every method and field in `facts-6142.tsv` that no block names); `constants.py` names against page 07; `il2cpp-fields.md` against page 07; `tools/ch_probe/SPEC.md` and the two tool READMEs; `docs/agents/clone-hero-mcp.md` and `this-machine.md`; and the 253 `src/` comment lines.

Acceptance: each scout's report holds its rows in the seven-cell shape, a list of every file it opened, and the jobs it did not finish by name. The main session spot-checks three rows per scout against the source before T4 starts, and a row that does not match is dropped with a note. Cap per scout: 90 calls or 20 minutes.

### T3: find round 2, the page side and the human records (four scouts in parallel, Sonnet; after T2's reports are in)

S5 and S6 work from the pages. Together they take every block labelled Unknown (103) or "Hydra assumes it, never checked" (34), as the README's count table gives them. S5 takes pages 01, 02 and 04 (66 blocks); S6 takes pages 03, 05, 06 and 07 (71 blocks). For each block they read its Evidence line's "where we looked", then grep the whole repo and `code/README.md`'s address index for the block's address, field or key words, and open any hit. A block whose proof exists is one row with the label it earns. A block whose evidence line names a file that now exists (as SP-30's does) is one row of kind stale. About eight jobs each means about eight to nine blocks a job, grouped by `##` heading. Sixty-odd blocks cannot each get their own search inside 90 calls, so each job runs one scripted grep over all of its blocks' addresses and key words in a single call, and the scout opens only the hits.

S7 owns the human records: the 80 `clone-hero` cards (one grep across the cards for each card's "open threads" and `write-back:` lines and its quoted rulings, then open only the hits, because reading 80 cards one by one would use the whole cap); the three `[write-back]` cards against the pages they name; `rulings/clone-hero.md`'s seven "no page yet" rows, each one matched to the block that should carry it in its Ruling line; the 21 memory files; and the three handoffs. A card is a lead: the row names the file or run the card points at.

S8 owns the rest of the tree: the 283 `tests/` lines and the 902 `tools/` lines (less what S4 read, named by file); the UserGuide and differences pages (a public sentence about the game with no block is a row); `srb-format.md` and the feasibility research; and the whole-tree re-grep with the wider terms named in the inventory, reporting any file the inventory missed as its own row.

Acceptance: as T2, plus S8's report ends with the list of files its wider grep found that no scout owned, or the sentence that it found none and the terms it used. Cap per scout: 90 calls or 20 minutes.

### T4: verify every candidate (four scouts in parallel, Sonnet; fresh agents)

Each verifier takes the rows of two finders it did not scout (V1 takes S1 and S5; V2 takes S2 and S6; V3 takes S3 and S7; V4 takes S4 and S8). For every row it opens the source at the named line and the page at the named block and marks the row "holds", "wrong line" (with the right one), or "does not hold" (with why). For a row of kind contradicted it quotes both sides' words. Every split a row's source shows gets its own line, even when the finder wrote the row as something else. Side notes (a gap seen outside the rows) are rows too. A verifier that cannot settle a row by reading says what run or read would, and the row goes on the open list rather than quietly becoming "by reading".

Acceptance: every row from T2 and T3 has a verifier mark; no row is marked by the agent that wrote it; each contradicted row quotes both sides. Cap per verifier: 90 calls or 20 minutes; at the cap the unmarked rows are listed by id and go to one more verifier.

### T5: choose each finding's exit, then check every closure (the main session, then one fresh scout)

The main session gives every row that holds one exit and writes its evidence beside it: for a question, the question's number in the list; for covered by a ruling, the quote, its file and line, and the two sides it names; for not a problem, the one sentence. Nothing is marked fixed in this task. A row that would change a block's Rule line to a different rule, or that contradicts a block, is always a question. A row that only raises a label or adds a citation and keeps the Rule line's meaning is a plain write-back and needs no question; it still waits for the user's yes to the list as a whole.

Then one fresh scout, who wrote and verified no row, checks every closure (every exit 3 and exit 4) and returns PASS, REOPEN or GONE per row. Then the main session greps every exit 3 quote in its cited file, with `docs/audit/2026-10-29-audit-of-audits/briefs/checkquotes.py` or by hand; a quote not found turns PASS into REOPEN and the row into a question. Before any "no ruling found", the checker searches the 80 cards, not only the decision log.

Acceptance: every held row has an exit and evidence; every exit 3 and 4 row has a checker verdict from an agent that did not write or verify it; every exit 3 quote is found by grep in its file. Cap for the checker: 90 calls or 20 minutes.

### T6: the list the user sees (the main session)

The user gets one list, grouped by page and then by block id, with "new" rows at each page's end. Each row shows the seven cells, the verifier's mark and the exit. The splits come first, in messages of five or fewer, each written as the audit method's "How to write a question" says: both sides quoted as the page and the source have them, the user's own words if a ruling exists, and no purpose nobody wrote down. The plain write-backs follow as one table. The list also carries three closing lines: the sources read in full, the sources read in part with what was skipped, and the sources not read. The list is committed as question 3 proposes, on a branch, and goes past a fresh reviewer before the user sees it (the plans-reviewed rule), because it is the document the write-back step will be built from.

No page under `docs/clone-hero/` changes in this plan. The write-back step is a later plan, after the user's yes, and it carries one review per page change.

Acceptance: the committed list exists with every held row; each split sits in a chat message with four or fewer other splits; `git log -- docs/clone-hero` on the search's branches shows no commit from this plan.

## Out of scope (needs the user's yes to leave out)

- The 237 removed plans in git history (question 1).
- A full read of the 2026-10-03 derivation audit and ledger (question 2).
- Reading the game: no listing is made, no run is started, and no Ghidra decompile is opened for a fact a committed file lacks. A gap that needs a new read goes on the list as Unknown with the read named.
- Writing the pages, the README's count table and the live-checks page: the later plan.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| TM-50's rule is checked first, before any fan-out, and the player audit gets its answer with a label | the plain version; T0 | T0 | T0's report exists before T1 launches; the main session's go message for T1 names T0's result |
| No scout starts before the user approves this plan and answers its questions | before anything runs | all | the main session's launch message quotes the user's yes |
| T0 cites only committed files and carries exactly one of the four labels; a contradiction with the live runs becomes a question, not a reading | T0 | T0 | T0 acceptance: the proposed block text; the main session opens each line named |
| The main session checks T0's named lines itself before repeating them (rule 9) | T0 | T0 | the go message for T1 quotes the lines it opened |
| The calibration scout gets no hint and must find the SP-29/SP-30 gap, or the briefs are fixed before anyone else starts | T1 | T1 | T1 acceptance; the brief file holds no "0x20F4950", "step loop" or "not listed" |
| No two scouts own the same source; each owns about eight jobs; three to four scouts per round | T2, T3 | T2, T3 | the brief files' job lists, read side by side by the reviewer of this plan |
| Scouts write only to their report and the scratch folder; no repo page changes in this plan | how the search runs; T6 | T2, T3, T4, T6 | `git status` in each scout's worktree is clean; `git log -- docs/clone-hero` shows no commit from this plan |
| Every scout and verifier has a cap of 90 calls or 20 minutes and a line saying what to report at the cap; unfinished jobs are passed on by name | caps and the stop rule | T1 to T5 | each brief's cap line; each report's "not finished" list |
| The search has two find rounds and one verify round, then stops; a third round needs the user's word | caps and the stop rule | T3, T4 | the plan's task list has no T7; the list's closing lines name what was not read |
| Every finding row has the seven cells and the exit cell starts blank | the finding row | T2, T3 | the main session's spot-check of three rows per scout |
| A lead (card, handoff, plan, memory) is never a source; the row names the file it points at or earns Unknown; a transcript-only fact is dropped | the finding row; rulings | T3, T4 | V3's marks on S7's rows; the reviewer greps the list for "transcript" |
| Every candidate is verified by an agent that did not scout its group; each contradicted row quotes both sides; every split is its own line | T4 | T4 | T4 acceptance; the verifier assignment table |
| A row that needs a run or an unmade read stays open, never "by reading" | T4 | T4 | the open list in T6's closing lines |
| Only the main session picks an exit; nothing is marked fixed in this plan | how the search runs; T5 | T5 | the list's exit column holds no "fixed" |
| Every split and every Rule-changing row is a question; a ruling closure quotes the user's words naming both sides; "not a problem" has one agreed sentence | the four exits; T5 | T5 | the closure checker's verdicts; the quote grep |
| A fresh agent checks every closure, and the main session greps every exit 3 quote | T5 | T5 | PASS/REOPEN/GONE per exit 3 and 4 row; a grep hit per PASS |
| "No ruling found" is said only after the 80 cards are searched | T5 | T5 | the checker's report names the card search terms |
| Splits reach the user five or fewer per message, each with both sides quoted and the user's own words where a ruling exists | T6 | T6 | the chat messages; the committed list's split rows |
| The list is committed and reviewed by a fresh agent before the user sees it | T6; question 3 | T6 | the branch's review commit precedes the chat message |
| Page write-backs wait for the user's yes to the list | the plain version; T6 | T6 | no page commit in this plan's branches |
| The wider re-grep names any file the inventory missed, or says none and names its terms | the source inventory; T3 | T3 (S8) | S8's report's last section |

There are no live steps in this plan, so it has no `## The live steps` section and no `Calls:` lines; `docs/agents/premise-map.md` asks for them only under `### LS` headings.

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: SP-29 and SP-30 are labelled Unknown and say `0x20F4950` and `0x20F6FF0` are not listed | code | `docs/clone-hero/05-star-power.md` lines 299 to 316 (the Label lines at 302 and 312, "not listed" at 304 and 313 to 314) | checked 2026-11-08 |
| P2: the step-loop and next-event listings are committed | code | `docs/audit/drum-lanes/play/f20f4950_step_loop.asm` (115 lines) and `f20f6ff0_next_event.asm` (131 lines), listed today | checked 2026-11-08 |
| P3: TM-50 on main is Proven in a live run, names chords 119, 120 and 152 to 154, and its Game code line says the two-match choice "was not read" | code | `docs/clone-hero/03-timing.md` lines 323 to 331 | checked 2026-11-08 |
| P4: FL-31 cites LS-O3 run 1 and no other run | code | `docs/clone-hero/04-fills.md` line 326 | checked 2026-11-08 |
| P5: LS-M2 run 1 declined chords 239, 800, 827, 1004, 1029, 1328, 1617, 1662 and 1989 | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1\replies.txt` line 3 | checked 2026-11-08 |
| P6: each of those nine rows shows `hit` as `declined` and the score rising 200 (239, 800, 827, 1328, 1617, 1662, 1989) or 400 (1004, 1029) | run | `ls-m2-run1\press-2.csv` rows with those `index` values (`score_before`, `score_after`, `hit`) | checked 2026-11-08 |
| P7: the press handler walks window notes in a loop (lines 72 to 162), masks a note's pads against the pressed pads less the pads already used (`bx`, lines 109 to 114) and hits through the slot at `+0x1E8` (line 152) | code | `docs/audit/drum-lanes/play/f20deaa0.asm` lines 72 to 162 | checked 2026-11-08 |
| P8: the call at line 104 (`0x18214b6d0`) is the note's "already hit or missed" test | unchecked | `docs/audit/drum-lanes/play/f20deaa0.asm` lines 104 to 106; FINDINGS does not name it; the `fb1be3cd` text cites `docs/audit/note-shuffle/scout-flag/note_getters.txt` line 9 | T0 |
| P9: the window array is filled in chart order by the base update, not in the press handler's listing | unchecked | `docs/audit/drum-lanes/play/f20f5810_base_update.asm` lines 57 to 102, as the `fb1be3cd` text cites them; FINDINGS line 102 names the scheduling listings | T0 |
| P10: FINDINGS describes the press handler at lines 52 and 56, the hit-clock rule at line 98, the step loop at line 102, the rounding tie at line 104 and the untraced caller `0x100E0D0` at line 108 | code | `docs/audit/drum-lanes/FINDINGS-play-time.md` those lines | checked 2026-11-08 |
| P11: TM-25's Rule line already says a press is valid up to and including the window end | code | `docs/clone-hero/03-timing.md` line 295 | checked 2026-11-08 |
| P12: the handoff's "Freak chord 803" is in some run's result file (scout S3 looks) | unchecked | `docs/handoffs/2026-11-08-clone-hero-docs-gaps-handoff.md` (removed; `git show 13738279889ceb4840285f1eba78ba46c01c2fa5:docs/handoffs/2026-11-08-clone-hero-docs-gaps-handoff.md`) line 22 only; TM-50 does not name 803 | T2 |
| P13: the reference has four labels, the seven-line block shape, and 392 blocks of which 103 are Unknown and 34 "Hydra assumes" | code | `docs/clone-hero/README.md` lines 19 to 26, 53 to 64, 110 to 124 | checked 2026-11-08 |
| P14: transcripts are not a source ("arent kept long term...") | ruling | `docs/clone-hero/README.md` line 32 | checked 2026-11-08 |
| P15: the audit method has four exits, a calibration stage, a closure check by a fresh agent and a quote grep; its scope rulings are quoted there | code | `docs/agents/audit-method.md` lines 19 to 28, 36, 44 to 46, 79 to 89 | checked 2026-11-08 |
| P16: `checkquotes.py` exists | code | `docs/audit/2026-10-29-audit-of-audits/briefs/checkquotes.py`, present today | checked 2026-11-08 |
| P17: a plan with no `### LS` heading needs no `Calls:` line | code | `docs/agents/premise-map.md` lines 50 to 52 | checked 2026-11-08 |
| P18: the `scout` agent type runs on Sonnet, is read-only by instruction and cannot launch agents | code | `C:\Users\Patrick\.claude\agents\scout.md` lines 2 to 9 | checked 2026-11-08 |
| P19: read-only scouts run on Sonnet or Haiku | code | `docs/agents/agent-rules.md` line 8 | checked 2026-11-08 |
| P20: the hook warns at 100 calls or 20 minutes and stops at 150 or 30 | code | `docs/agents/brief-preamble.md` line 69 | checked 2026-11-08 |
| P21: the drum-lanes folder has 31 listings, 18 of them cited by no area page | code | `docs/audit/drum-lanes/` and its `play/` folder, listed today, against the joined text of pages 01 to 07 | checked 2026-11-08 |
| P22: `code/6142/` holds 91 listings; `two_x_kick_drop_20F0320.asm` is cited by `code/README.md` only; `4045/fill_deadline.md` is cited by no area page | code | `docs/clone-hero/code/6142/` and `code/4045/`, listed today, against the pages' joined text | checked 2026-11-08 |
| P23: `code/README.md` indexes 210 older listings by address, which is 31 drum-lanes plus 179 note-shuffle | code | `docs/clone-hero/code/README.md` lines 10 to 14; the two folders' `.asm` counts today | checked 2026-11-08 |
| P24: 41 run folders exist across five dates; the area pages cite 18 by path | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\`, listed today; the pages' `ch-mcp-runs\` paths | checked 2026-11-08 |
| P25: 80 cards carry the `clone-hero` tag; three carry `[write-back]` at index lines 460, 469 and 475 | code | `docs/sessions/index.md`; the cards' `write-back:` lines in `2026-11-07-70bddae3.md` line 41, `2026-11-07-b132faae.md` line 46, `2026-11-08-d0a3868c.md` line 58 | checked 2026-11-08 |
| P26: SP-17 does not name `0x20DE770`; TM-49 holds the frame-rate drop but not session b132faae's id; SP-47 holds the frame-read fact | code | `docs/clone-hero/05-star-power.md` SP-17 block and SP-47 (line 349); `docs/clone-hero/03-timing.md` TM-49 (line 151), grepped today | checked 2026-11-08 |
| P27: seven rows of the Clone Hero rulings file end "backs: no page yet" | code | `docs/agents/rulings/clone-hero.md` lines 9 and 14 to 19 | checked 2026-11-08 |
| P28: the Note Shuffle audit has 179 listings, five notes files and a game-tests README with one result JSON | code | `docs/audit/note-shuffle/`, listed today; `game-tests/README.md` lines 1 to 11 | checked 2026-11-08 |
| P29: the line counts for `src/`, `tests/`, `tools/`, the decision log, the ADRs, the public pages, the plans and the memory folder | code | the greps named in "The source inventory", run today over `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\ch-docs-gaps-plan` and the memory folder | checked 2026-11-08 |
| P30: 237 plan files were removed from `docs/superpowers/plans/` over the repo's history | code | `git log --diff-filter=D --name-only -- docs/superpowers/plans` in the worktree today | checked 2026-11-08 |
| P31: the player audit waits on TM-50's two-note rule (its Q11) | code | `docs/handoffs/2026-11-08-player-audit-continue-handoff.md` (removed; `git show 13738279889ceb4840285f1eba78ba46c01c2fa5:docs/handoffs/2026-11-08-player-audit-continue-handoff.md`) lines 43, 61 and 101 | checked 2026-11-08 |
| P32: the user said "are the facts actually unknown or did you never check what we already have documented?" and asked for this search | ruling | `docs/handoffs/2026-11-08-clone-hero-docs-gaps-handoff.md` (removed; `git show 13738279889ceb4840285f1eba78ba46c01c2fa5:docs/handoffs/2026-11-08-clone-hero-docs-gaps-handoff.md`) lines 46 to 48, and the same words quoted in this plan's brief by the main session | checked 2026-11-08 |
| P33: the user said "Yes, plan it" to settling TM-50 first | ruling | this plan's line 3, which quotes the session's chat (`docs/superpowers/plans/2026-11-08-clone-hero-docs-gaps-search.md`) | checked 2026-11-08 |
| P34: a game fact never lives in a memory note | code | `CLAUDE.md` "Clone Hero reference"; `docs/clone-hero/README.md` line 80 | checked 2026-11-08 |
| P35: commit `fb1be3cd` on `claude/player-audit-plan` (not merged) rewrites TM-50 as Proven in the game's code, with the window array filled in chart order and each pressed pad hitting the earliest unjudged note | code | `git show fb1be3cd -- docs/clone-hero/03-timing.md` and `docs/audit/2026-11-08-player-audit/questions.md` in that commit, read today | checked 2026-11-08 |
| P36: the press handler's loop shape, skip test, pad mask, used-pad update and hit slot are as T0 describes | code | `docs/audit/drum-lanes/play/f20deaa0.asm` lines 72 to 162, 104 to 106, 109 to 114 and 152, read today | checked 2026-11-08 |
