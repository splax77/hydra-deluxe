# Handoff: the rest of the Clone Hero unknowns work, after FL-08 and FL-10

Session 7b39835b-f911-4885-91b2-d4cbf4fe2285, 2026-11-08. The user said "create handoff for the rest". This session continued `docs/handoffs/2026-11-08-clone-hero-unknowns-handoff.md` (on branch `claude/ch-unk-handoff`), which session 77057df2-e94e-4ae2-9526-82deb22eb27c wrote. Read that handoff first. It still holds the goal, the workstream branches and what each one covers. This one says what changed since and what is left.

## The goal

The goal has not changed. The user asked that "every claim labeled unknown should be checked", and chose "Unknown only". Every `Label: Unknown` block on `docs/clone-hero/0*.md` gets checked against the game's code, and each difference from Hydra goes to the user. The approved plan is `docs/superpowers/plans/2026-11-08-clone-hero-unknowns.md`, on branch `claude/ch-unknowns-plan`, and it is not merged.

## What this session did

The earlier handoff left two open questions, FL-08 and FL-10. The user asked: "get me real numbers on the open questions before i decide anything". The session measured both with `hydra_replay dump`. For each chart, it scored the chart as it is and a scratch copy edited so Hydra sees only the fills the game would use. An untouched copy, scored the same way, matched the original every time.

FL-08: fills on or after the coda. 330 library charts have a coda. 327 have a written fill at or after it. On 323 of those, Hydra's best score does not change. Four drop: Nirvana Endless, Nameless Setlist goes from 39,273,950 to 34,193,190; Nirvana Bleach (Full Album) from 5,629,865 to 4,692,885; Ghoul Bringer of War from 574,110 to 572,930; John Wasson Caravan from 2,361,350 to 2,360,430. A scout traced the `.chart` side, which the earlier checkers had left unread. The game skips there too, from the first `coda` in `[Events]`. A coda written inside a drum section does not count. The user ruled "Match the game".

A planner wrote the plan, a fresh reviewer read it, and the user approved it with two answers: "Bump the stamp" and "1.0's own rule (Recommended)". Two executors wrote the code and the page update. The main session found a visible change no one had asked for. 33 charts lose every written fill to the coda, and Hydra then generated fills for them. The user chose "Trace the game first (Recommended)". A second scout found that the game keeps coda-skipped fills in the list its generation test counts, so the game generates none. The user ruled "Match the game", and a third executor made Hydra do the same. Each change had one fresh reviewer, and both signed off clean.

The main session ran the whole `hydra_tests` suite on the final tip: 2,027 cases, 0 failed. The branch's `hydra_replay` scored all 327 coda charts on their original files, and every score matched the edited-copy measurement. None of the 33 charts has a fill chord any more. The work merged on `main` as commit 5fe26cb80c335a26b27634ad4aa29cb1abb59628. It is not pushed.

So, on `main` now: `place_authored_fills()` skips a written fill whose start is at or after the first coda, or exactly on it in `FillDeadlineRule::Ch10` mode. `Song::check_activations()` generates no fills when a coda-skipped fill exists (`Song::coda_skipped_fill`). `kResultsStamp` is `2.14.1+coda`, so every saved result reads Stale. Page 04's FL-08 reads Proven in the game's code, Q-40 is closed and retired, and FL-27 and FL-39 are updated. The seven coda listings are under `docs/clone-hero/code/6142/`. The plan for this change was `docs/superpowers/plans/2026-11-08-fl08-coda-fills.md` (removed; `git show c62c8d12b0c5c2a83d2095c2fd7c9e507dd57180:docs/superpowers/plans/2026-11-08-fl08-coda-fills.md`).

FL-10: two written fills on one chord. A scan with Hydra's own landing rule found exactly two charts. Pangaea has 76 such chords and RAT KING has 6. Keeping only the earlier fill, which gives Hydra the game's deadline, changes neither best score: Pangaea stays at 722,900 and RAT KING at 260,970. The user ruled "Record it only".

Both sets of numbers and all these rulings are written into `docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md` on this handoff's branch. Two of the new rulings ("1.0's own rule (Recommended)" with "Bump the stamp", and the second "Match the game" on generation) are lines in `docs/agents/rulings/clone-hero.md` on `main`.

## What is next

The earlier handoff's "What is next" list still holds, with steps 1 and 7's FL-08 part done. In order:

1. Finish W8c on `claude/ch-unk-w8b` (IN-55 and IN-45), then run round 3 (W9, W10, W11) from the plan's Workstreams section, at about four blocks per agent.
2. Send fresh checkers for W6's second pass, W7, W8, W8c and round 3, with the round 1 brief: open every cited line, then PASS, REOPEN or GONE.
3. Put SP-06 and SP-08 (overlapping Star Power phrases) to the user once their checker has passed them. Measure first, as this session did: score the chart as it is against an edited copy, and quote real chart numbers. Check FL-23 against the user's earlier "85 ms both sides" fill ruling before asking anything about it.
4. Run the page executors (sets 01+03, 02, 04+05, 06+07) with the wording in `checker-verdicts.md`. Three things are new. Page 04 on `main` already holds FL-08, so its executor must start from `main` and skip FL-08. FL-10 needs a Q entry for "Record it only", like CR-51 and NL-58. And `claude/ch-unk-w6` holds older copies of `fill_skip_from_coda_215DA70.md` and `mid_events_coda_21544E0.md`, which `main` has since corrected. Take `main`'s copy if they conflict.
5. Live steps last, one at a time, as the earlier handoff lists them. The FL-08 plan's own live check was never run: open Caravan in the real Hydra window, read its Library "Best path" cell (2,360,430 after re-analysis) and its Paths tab.
6. Two code plans the user ruled for are still to write: CR-24's `_ENDOFSONG` rule in `marker_name()`, and NL-55's lane pad.

Housekeeping: the worktrees `.claude/worktrees/fl08-plan`, `fl08-t1` and `fl08-t2` hold merged or finished work and can be removed. Ask before deleting any branch.

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| commit | `5fe26cb80c335a26b27634ad4aa29cb1abb59628` | the FL-08 merge (coda skip, no generation after a coda skip, stamp bump, page 04); on main, not pushed | `git branch --contains`; `git log origin/main --oneline -1` |
| commit | `9bbd3250a9336e17bc3fdb9dbd6510b4381efee1` | the last reviewed tip of `claude/fl08-t1`, merged into main, not pushed | `git merge-base --is-ancestor 9bbd3250a9336e17bc3fdb9dbd6510b4381efee1 main` |
| commit | `c62c8d12b0c5c2a83d2095c2fd7c9e507dd57180` | holds the removed FL-08 plan file; on main through the FL-08 merge, not pushed | `git show c62c8d12b0c5c2a83d2095c2fd7c9e507dd57180:docs/superpowers/plans/2026-11-08-fl08-coda-fills.md` |
| commit | `e8aa6e7285a88b1a885008f965206c96b619de7a` | tip of `claude/ch-unk-handoff` with the FL-08 and FL-10 numbers and rulings in user-answers.md; not on main, not pushed | `git log -1` on the branch |
| branch | `claude/ch-unk-handoff` | the earlier handoff, checker-verdicts.md and user-answers.md; not merged | `git log main..claude/ch-unk-handoff` |
| branch | `claude/ch-unknowns-plan` | the approved unknowns plan, tip c9b45492d16588f4162ddd148e310f97f586df03; not merged, not pushed | `git log main..claude/ch-unknowns-plan` |
| branch | `claude/ch-unk-w6` | W6 results; second pass (SP-06, SP-07, SP-08, FL-22, FL-32) not checked; tip 3fa81d86030985f6456852bf5d0b2abb282e2894; not on main | `git log main..claude/ch-unk-w6` |
| branch | `claude/ch-unk-w7` | W7 results, not checked; tip 703fa92e4af8f28ab18ef84119717efd33168e97; not on main | `git log main..claude/ch-unk-w7` |
| branch | `claude/ch-unk-w8` | W8 results, not checked; tip 439f6107783982ed80524f44b7147d80667924ea; not on main | `git log main..claude/ch-unk-w8` |
| branch | `claude/ch-unk-w8b` | W8c, IN-44 written, IN-55 and IN-45 not; tip 104fe3d75b5cda0912dbbc15cdd5bbe9fae8532a; not on main | `git log main..claude/ch-unk-w8b` |
| path | `docs/handoffs/2026-11-08-clone-hero-unknowns-handoff.md` | the earlier handoff; on `claude/ch-unk-handoff` and this branch | open it |
| path | `docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md` | every user answer, including FL-08's and FL-10's numbers | open it |
| path | `docs/audit/2026-11-08-clone-hero-unknowns/checker-verdicts.md` | the checkers' verdicts and narrowed wording per block | open it |
| path | `docs/clone-hero/04-fills.md` | FL-08 reads Proven in the game's code; FL-10 still open for a Q entry | read FL-08 and FL-10 on main |
| path | `docs/clone-hero/code/6142/events_section_2151830.asm` | the `.chart` [Events] reader that stores the coda; on main | open it |
| path | `docs/agents/rulings/clone-hero.md` | holds this session's three 2026-11-08 lines on main | read its top lines |
| function | `place_authored_fills()` | the one owner of the coda skip (`>=`, `==` under Ch10) | read src/parse/song.cpp |
| function | `Song::check_activations()` | returns without generating when `coda_skipped_fill` is set | read src/core/song.cpp |
| function | `marker_name()` | CR-24's ruled `_ENDOFSONG` change goes here; not started | read src/parse/section_names.cpp |
| test | `hydra_tests.exe` | the whole suite passed on 9bbd3250a9336e17bc3fdb9dbd6510b4381efee1 (2,027 cases) | rebuild main and run it |
| ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | "get me real numbers on the open questions before i decide anything" | the session transcript |
| ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | FL-08: "Match the game" | the session transcript; rulings file |
| ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | FL-08 plan: "Bump the stamp", "1.0's own rule (Recommended)", "Approve" | the session transcript; rulings file |
| ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | the 33 coda-skipped charts: "Trace the game first (Recommended)", then "Match the game" | the session transcript; rulings file |
| ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | FL-10: "Record it only" | the session transcript; user-answers.md |
| ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | "create handoff for the rest" | the session transcript |
| state | main is 927 commits ahead of origin/main | nothing from this run is pushed | `git rev-list --count origin/main..main` |
| state | `.claude/worktrees/fl08-plan`, `fl08-t1`, `fl08-t2` | finished work, safe to remove after a look | `git worktree list` |

## Checker output

```
not-pushed | commit | 5fe26cb80c335a26b27634ad4aa29cb1abb59628 | the FL-08 merge (coda skip, no generation after a coda skip, stamp bump, page 04); on main, not pushed
not-pushed | commit | 9bbd3250a9336e17bc3fdb9dbd6510b4381efee1 | the last reviewed tip of `claude/fl08-t1`, merged into main, not pushed
not-pushed | commit | c62c8d12b0c5c2a83d2095c2fd7c9e507dd57180 | holds the removed FL-08 plan file; on main through the FL-08 merge, not pushed
not-on-main | commit | e8aa6e7285a88b1a885008f965206c96b619de7a | tip of `claude/ch-unk-handoff` with the FL-08 and FL-10 numbers and rulings in user-answers.md; not on main, not pushed
ok | branch | claude/ch-unk-handoff | the earlier handoff, checker-verdicts.md and user-answers.md; not merged
ok | branch | claude/ch-unknowns-plan | the approved unknowns plan, tip c9b45492d16588f4162ddd148e310f97f586df03; not merged, not pushed
ok | branch | claude/ch-unk-w6 | W6 results; second pass (SP-06, SP-07, SP-08, FL-22, FL-32) not checked; tip 3fa81d86030985f6456852bf5d0b2abb282e2894; not on main
ok | branch | claude/ch-unk-w7 | W7 results, not checked; tip 703fa92e4af8f28ab18ef84119717efd33168e97; not on main
ok | branch | claude/ch-unk-w8 | W8 results, not checked; tip 439f6107783982ed80524f44b7147d80667924ea; not on main
ok | branch | claude/ch-unk-w8b | W8c, IN-44 written, IN-55 and IN-45 not; tip 104fe3d75b5cda0912dbbc15cdd5bbe9fae8532a; not on main
ok | path | docs/handoffs/2026-11-08-clone-hero-unknowns-handoff.md | the earlier handoff; on `claude/ch-unk-handoff` and this branch
ok | path | docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md | every user answer, including FL-08's and FL-10's numbers
ok | path | docs/audit/2026-11-08-clone-hero-unknowns/checker-verdicts.md | the checkers' verdicts and narrowed wording per block
ok | path | docs/clone-hero/04-fills.md | FL-08 reads Proven in the game's code; FL-10 still open for a Q entry
ok | path | docs/clone-hero/code/6142/events_section_2151830.asm | the `.chart` [Events] reader that stores the coda; on main
ok | path | docs/agents/rulings/clone-hero.md | holds this session's three 2026-11-08 lines on main
ok | function | place_authored_fills() | the one owner of the coda skip (`>=`, `==` under Ch10)
ok | function | Song::check_activations() | returns without generating when `coda_skipped_fill` is set
ok | function | marker_name() | CR-24's ruled `_ENDOFSONG` change goes here; not started
manual | test | hydra_tests.exe | the whole suite passed on 9bbd3250a9336e17bc3fdb9dbd6510b4381efee1 (2,027 cases)
manual | ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | "get me real numbers on the open questions before i decide anything"
manual | ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | FL-08: "Match the game"
manual | ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | FL-08 plan: "Bump the stamp", "1.0's own rule (Recommended)", "Approve"
manual | ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | the 33 coda-skipped charts: "Trace the game first (Recommended)", then "Match the game"
manual | ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | FL-10: "Record it only"
manual | ruling | 7b39835b-f911-4885-91b2-d4cbf4fe2285 | "create handoff for the rest"
manual | state | main is 927 commits ahead of origin/main | nothing from this run is pushed
manual | state | .claude/worktrees/fl08-plan | finished work, safe to remove after a look
checked 28 rows: 15 ok, 0 missing, 1 not-on-main, 3 not-pushed, 9 manual
```
