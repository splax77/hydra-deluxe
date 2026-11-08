# Handoff: find documentation missing from the Clone Hero pages (2026-11-08, session 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e)

## Goal

The user asked for "one to search for other documentation missing from our central docs". The central docs are the Clone Hero reference in `docs/clone-hero/`. CLAUDE.md calls it the one home of what the project knows about the game. Every agent is told to read it first.

The problem: facts proven elsewhere in the repo never reached those pages. Agents that trust the pages then call those facts Unknown, invent rules, or ask the user questions that were already settled. In the player audit (session 31e4f22f), every agent and the main session did exactly that. The user's words: "are the facts actually unknown or did you never check what we already have documented?"

The task is a read-only search of the whole repo for game facts the pages lack, or that the pages label weaker than their evidence. It ends with a list of write-backs, each giving the page block, the source file and line, and the label that evidence earns. Writing the pages is a later step, after the user's yes.

## What was found so far

All of the below is by reading, in session 31e4f22f.

`docs/audit/drum-lanes/FINDINGS-play-time.md` and its listings in `docs/audit/drum-lanes/play/` are committed. They document things the pages still mark Unknown or "not listed":

- **SP-29 and SP-30** (`docs/clone-hero/05-star-power.md`) are labelled Unknown. They say `0x20F4950` and `0x20F6FF0` are "not listed". Both listings are committed, as `play/f20f4950_step_loop.asm` and `play/f20f6ff0_next_event.asm`. FINDINGS (the paragraph under "When does a granted hit happen?") says the engine queues its own times and steps its clock to each one in order: each note's window start and end, and the Star Power times.
- **SP-36** has the Star Power test in the hit routine. FINDINGS line 98 says the same thing in plain words: a note counts in Star Power by the clock it is hit at.
- **TM-25** has the window edge as a miss test, and its Rule line already says a press is valid up to and including the window end (page 03 line 295). What FINDINGS adds is that a granted hit lands at the window end within one rounding step, and the rounding-tie case (FINDINGS line 104: a grant that lands one scheduled time later). That is the only part TM-25 lacks.
- **TM-50** is proven only in live runs on main. The press handler listing `play/f20deaa0.asm` is committed, and FINDINGS lines 52 and 56 describe it. Later the same day, commit `fb1be3cd` on `claude/player-audit-plan` (session 274bd9dd, not merged) rewrote TM-50 as Proven in the game's code from that listing and the base update listing, so the rule is written down on that branch; it still needs a check against the listings. This is a real problem, not an edge case: Freak run 1's chord 154 missed this way.
- **FINDINGS line 108** names one thing it did not trace: the caller of `0x100E0D0`, which would show whether a press is processed at its own timestamp. SP-47's one-frame model is measured live and works in practice, so tracing it is optional.
- **Run results the pages lack.** FL-31 cites one run, and LS-M2 (`C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1`) adds nine granted fill chords. TM-50 does not name Freak chord 803.

## What is next

1. **Read `docs/clone-hero/README.md`** for the four labels and how a block is shaped.
2. **List every place in the repo that holds game facts outside `docs/clone-hero/`.** Known candidates:
   - `docs/audit/drum-lanes/`, `docs/audit/note-shuffle/` and the other folders under `docs/audit/`;
   - `docs/clone-hero/code/` listings that no block cites;
   - `docs/research/`;
   - `docs/srb-format.md`;
   - plans under `docs/superpowers/plans/` whose findings sections record game reads;
   - session cards in `docs/sessions/` marked `[write-back]` in `docs/sessions/index.md`;
   - the user's rulings in `docs/agents/rulings/clone-hero.md`;
   - run folders under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\` that a plan cites as proof.

   A search that finds nothing proves nothing on its own: say what was searched.
3. **Check each page block against those sources, family by family.** For each block, report three things: whether a source in the repo proves more than the block's label says; whether a source contradicts the block; and whether a source states a game fact that has no block.
4. **Give the result as one list.** Each row gives the page and block id (or "new"), the source file and line, the label the evidence earns, and one sentence of what to write. Splits between a page and a source go to the user as questions.
5. **Plan the work and wait for the user's yes before any agent starts.** The plan names its caps, and it follows `docs/agents/audit-method.md` if it is run as an audit. Agents are read-only scouts on Sonnet.

The player audit (`docs/handoffs/2026-11-08-player-audit-continue-handoff.md`) needs the TM-50 two-note rule from this search's sources. If this task starts first, settle that row early.

## The user's rulings this session

- "are the facts actually unknown or did you never check what we already have documented?"
- "you said that "most" is already documented. what do you need that isnt documented? that looks like a lot we already have that you didnt check"
- "create two handoffs. one to continue this work, and one to search for other documentation missing from our central docs. are either of those real problems, or edge cases you invented?"

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| path | `docs/clone-hero/README.md` | the index of the Clone Hero reference and its four labels | open it |
| path | `docs/clone-hero/05-star-power.md` | SP-29 and SP-30 labelled Unknown, saying `0x20F4950` and `0x20F6FF0` are not listed | grep `SP-29`, `SP-30` |
| path | `docs/clone-hero/03-timing.md` | TM-25 (window edge) and TM-50 (two-note press, live runs only) | grep `TM-25`, `TM-50` |
| path | `docs/audit/drum-lanes/FINDINGS-play-time.md` | the step loop, the hit clock, the window edge and the press handler, with line 108's untraced caller | open it |
| path | `docs/audit/drum-lanes/play/f20f4950_step_loop.asm` | the listing SP-29 and SP-30 call not listed | open it |
| path | `docs/audit/drum-lanes/play/f20f6ff0_next_event.asm` | the listing SP-30 calls not listed | open it |
| path | `docs/audit/drum-lanes/play/f20deaa0.asm` | the press handler that can settle TM-50's two-note rule | open it |
| path | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1` | LS-M2: nine granted fill chords that FL-31 lacks | list it |
| path | `docs/handoffs/2026-11-08-player-audit-continue-handoff.md` | the player audit's continuation, which needs TM-50's rule | open it |
| state | no page has been changed for this; the search has not started | as written | `git log -- docs/clone-hero` |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "are the facts actually unknown or did you never check what we already have documented?" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "you said that "most" is already documented. what do you need that isnt documented? that looks like a lot we already have that you didnt check" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "create two handoffs. one to continue this work, and one to search for other documentation missing from our central docs. are either of those real problems, or edge cases you invented?" | the transcript |

## Checker output

```
ok | path | docs/clone-hero/README.md | the index of the Clone Hero reference and its four labels
ok | path | docs/clone-hero/05-star-power.md | SP-29 and SP-30 labelled Unknown, saying `0x20F4950` and `0x20F6FF0` are not listed
ok | path | docs/clone-hero/03-timing.md | TM-25 (window edge) and TM-50 (two-note press, live runs only)
ok | path | docs/audit/drum-lanes/FINDINGS-play-time.md | the step loop, the hit clock, the window edge and the press handler, with line 108's untraced caller
ok | path | docs/audit/drum-lanes/play/f20f4950_step_loop.asm | the listing SP-29 and SP-30 call not listed
ok | path | docs/audit/drum-lanes/play/f20f6ff0_next_event.asm | the listing SP-30 calls not listed
ok | path | docs/audit/drum-lanes/play/f20deaa0.asm | the press handler that can settle TM-50's two-note rule
ok | path | C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1 | LS-M2: nine granted fill chords that FL-31 lacks
ok | path | docs/handoffs/2026-11-08-player-audit-continue-handoff.md | the player audit's continuation, which needs TM-50's rule
manual | state | no page has been changed for this; the search has not started | as written
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "are the facts actually unknown or did you never check what we already have documented?"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "you said that "most" is already documented. what do you need that isnt documented? that looks like a lot we already have that you didnt check"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "create two handoffs. one to continue this work, and one to search for other documentation missing from our central docs. are either of those real problems, or edge cases you invented?"
checked 13 rows: 9 ok, 0 missing, 0 not-on-main, 0 not-pushed, 4 manual
```
