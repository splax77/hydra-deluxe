# Handoff: continue the player audit (2026-11-08, session 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e)

## Goal

The user asked: "do a full audit of the playing code before making any changes. i dont trust any of this anymore." The playing code is everything between Hydra's top path and a press landing in Clone Hero, plus everything that judges a run afterwards. The audit is read-only. No code has changed. Fixes come in a fix plan after the user rules on every finding.

This session continued from `docs/handoffs/2026-11-08-session-6341c618-findings-handoff.md` (on branch `claude/handoff-6341c618-findings`). That handoff said LS-M2 missed 14 notes because the game ran a stale built `kit.js`.

## What was done

The audit plan is `docs/superpowers/plans/2026-11-08-player-audit.md` on branch `claude/player-audit-plan`. A fresh reviewer signed it off clean at `3404e324d630e286c7848164e6e110c8880c3fec`. The user approved it with "Yes, recommended scope". That scope leaves out Hydra's path search and the probe scripts no play run imports.

The audit ran the plan's stages:

- **Calibration.** One scout was given `play_chart.py` and no hint. It found the known silent `groups` fallback at `play_chart.py:1166`, so the method passed.
- **Finding.** Four finders ran first. The user then approved a second wave ("Yes, second wave"), which sent two more finders at the files the first four had not read.
- **Verifying.** Five fresh verifiers checked every candidate against the code and the run folders.
- **The ledger.** A compiler merged everything into `ledger.md`: 85 rows, built from 131 candidates.
- **Exits.** The main session picked an exit for every row, in `exits.md`.
- **Closure check.** A fresh checker passed four closures and reopened two, L07 and L47. Both became Q9. It also showed the room margin was already ruled, so that closed as exit 3.

Everything is in `docs/audit/2026-11-08-player-audit/` on `claude/player-audit-plan`, tip `98a03d773755e49c0c372b32de4614a829aa181d`. It is not merged and has had no doc review.

The handoff's main claim holds. Verifier 2 matched all 14 LS-M2 misses to the second parts of 12 split chords, which the stale `kit.js` (built 2026-11-06 21:31) never pressed. Verifier 4 corrected one detail: chord 618 was hit. The Star Power phrase was lost at chord 609, the activation chord, whose kick was never pressed. Freak run 1 had 8 misses. Only 4 are the stale agent. The other 4 are stalls and a late press at chords 153, 154, 803 and 1144 (ledger row L02).

The user's answers so far are recorded under "## Answers" in `questions.md`:

- **Q1:** stale built agents and `hydra_replay` builds are refused by a build stamp.
- **Q2:** the run stops on a reply or file missing what it needs.
- **Q3 and Q4:** one rule, Hydra's `Chord::activation_note()` (`src/core/model.cpp:307`). It names the gem a skipped fill leaves out and the gem an activation chord presses first. The Python modes `kick_only` and `nothing`, and `_TOP_PAD_ORDER`, go.
- **Q5:** the run checks it is playing the right song, path and settings, through all four tools.
- **Q6:** a run is judged by source facts: per-chord score steps against Hydra's points, plus the game's own counts.
- **Q7:** the Star Power checks judge against Hydra.
- **Q8:** a press may be judged one frame late (SP-47), as one constant both sides read.
- **Q9:** Hydra's squeeze need is split equally between the two presses. This is the equal room that TC2 built. -45/+45 was only the user's example.
- **Q12:** every schedule skip is flagged and tested before TP1.
- **Q13:** no change for focus, and the 43.9 s window event is not pursued.

Q10 folded into earlier rulings; `questions.md` says how.

**Q11 was not settled when this handoff was written; the branch answered it later the same day (commit `fb1be3cd`, see the state row in Claims).** The user asked whether the facts were really unknown. The main session then found that most of them are documented in `docs/audit/drum-lanes/FINDINGS-play-time.md` and its committed listings. Two items are left:

1. Which note a press goes to when two unhit same-pad notes are both in their windows. This is real: Freak run 1's chord 154 missed this way. The committed press handler `docs/audit/drum-lanes/play/f20deaa0.asm` can settle it.
2. Whether a press is processed at its own timestamp. It is optional, because the measured one-frame model already covers it in practice.

The main session asked whether to read the listing. The user has not answered.

## What went wrong this session

- The first draft of the plan's scope missed about a dozen files that a play run loads. Its calibration brief would have given away its answer. The reviewer fixed both.
- The main session carried rules forward as if the user had made them, and the user corrected it each time:
  - It offered `kick_only` and `nothing` as live options. Planning agents had invented them, and no user words named them.
  - It cited a session card about multiplier squeezes as a rule for activation order.
  - It listed the user's "-45/+45" example as an option.
  - It called game facts Unknown after reading only the Clone Hero pages, not the drum-lanes study already in the repo.

The rule the user stated: read what Hydra and the docs already have before proposing a rule.

## What is next

1. **Get the user's answer to the Q11 gap.** The question was whether to read `docs/audit/drum-lanes/play/f20deaa0.asm` for the two-note press rule.
2. **Write and send Q14 and Q15 as `questions.md` names them.** Q14 is report and launch lines that can say the wrong thing (rows L31, L32, L34, L35, L74, L41, L42, L43). Q15 is the smaller items as one list of fixes (rows L13, L15, L18, L33, L75, L76, L78, L80, L81, L82, L85, plus L12). Before writing either, read what Hydra and the docs already have for each.
3. **Send this round's new closures to a fresh closure checker.** They are L06 and L46, recorded in `questions.md` under Q10. `exits.md` must be updated for them, and for L04 and L05 folding into Q8.
4. **Record each answer as a D-record** in `docs/audit/2026-10-03-fix-decisions.md`, naming both sides of each split.
5. **Get one fresh doc review of the audit folder**, as a committed range on `claude/player-audit-plan`. Then merge it, then run the docs check on main.
6. **Write the fix plan** from the rulings. It goes past a fresh reviewer, then waits for the user's yes. The previous handoff's "What is next" items wait for that plan: rebuild `kit.js`, rerun LS-M2, merge TD (`claude/replay-td`), the kick plan's T4, and the replay plan's TP1 to TP4.

The search for documentation missing from `docs/clone-hero/` is its own task, in `docs/handoffs/2026-11-08-clone-hero-docs-gaps-handoff.md`.

## The user's rulings this session

- "do a full audit of the playing code before making any changes. i dont trust any of this anymore."
- "Yes, recommended scope"
- "Yes, second wave"
- "Refuse stale (Recommended)" (Q1)
- "Stop the run (Recommended)" (Q2)
- "why are there three modes? who came up with this rule? what is the exact text i was presented with? there is only one way to correctly handle a skip and hydra's engine already knows it" (Q3)
- "that was about multiplier squeezes. hydra's engine already knows how to handle activations. you keep trying to reinvent new rules instead of reading what we already have" (Q4)
- "Yes, that rule" (Chord::activation_note for skips and activations)
- "All of it (Recommended)" (Q5)
- "Source facts (Recommended)" (Q6)
- "Judge vs Hydra (Recommended)" (Q7)
- "One frame (Recommended)" (Q8)
- "i provided 45 as an example. why do you keep carrying it forward as if it was a rule? hydra already knows what timing you need and it can be split equally" (Q9)
- "are the facts actually unknown or did you never check what we already have documented?" (Q11)
- "Flag and test all (Recommended)" (Q12)
- "i care about the runs working, not the window coming into focus" (Q13)
- "you said that "most" is already documented. what do you need that isnt documented? that looks like a lot we already have that you didnt check"
- "create two handoffs. one to continue this work, and one to search for other documentation missing from our central docs. are either of those real problems, or edge cases you invented?"

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| branch | `claude/player-audit-plan` | holds the audit plan and the audit folder; not merged | `git log main..claude/player-audit-plan` |
| commit | `3404e324d630e286c7848164e6e110c8880c3fec` | the reviewer's fix to the audit plan, signed off clean; not on main | `git branch --contains` |
| commit | `98a03d773755e49c0c372b32de4614a829aa181d` | the audit branch's tip: answers to Q5 to Q13 and the Q11 follow-up; not on main | `git rev-parse claude/player-audit-plan` |
| state | the approved audit plan is `docs/superpowers/plans/2026-11-08-player-audit.md`, on `claude/player-audit-plan` only | as written | `git show claude/player-audit-plan:docs/superpowers/plans/2026-11-08-player-audit.md` |
| state | the audit folder `docs/audit/2026-11-08-player-audit/` holds `ledger.md`, `exits.md`, `questions.md` and `reports/` | on `claude/player-audit-plan` only | `git ls-tree -r --name-only claude/player-audit-plan docs/audit/2026-11-08-player-audit` |
| state | the ledger has 85 rows, L01 to L85, and every row has an exit in `exits.md` | as written | grep the two files on the branch |
| state | as written when the handoff was made: Q11's two-note press rule and Q14 and Q15 were not answered. Later the same day the branch answered Q11 (commit `fb1be3cd`, TM-50 rewritten as Proven in the game's code, which still needs a check against the listings) and Q14 and Q15 (commit `ccb32ac9`), and the branch tip moved to `1fd85238` | as written | `questions.md` "## Answers" on the branch |
| path | `C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\ch_mcp\agent\kit.js` | the stale built kit agent, 2026-11-06 21:31, with no `after_ms` or `groupsOf` | `Get-Item` and `Select-String` |
| function | `Chord::activation_note` | Hydra's one rule for the gem that activates: first of Green, Blue, Yellow, Red, Kick, in `src/core/model.cpp` | `git grep -n "Chord::activation_note"` |
| function | `_kept_gems()` | the Python decline modes the Q3 ruling removes, in `tools/ch_probe/experiments/play_chart.py` | `git grep -n "def _kept_gems"` |
| path | `docs/audit/drum-lanes/FINDINGS-play-time.md` | documents the step loop, the hit clock and the window edge that SP-29, SP-30 and TM-25 lack | open it |
| path | `docs/audit/drum-lanes/play/f20deaa0.asm` | the press handler listing that can settle which note a press goes to | open it |
| path | `docs/handoffs/2026-11-08-clone-hero-docs-gaps-handoff.md` | the separate task to find documentation missing from `docs/clone-hero/` | open it |
| branch | `claude/handoff-6341c618-findings` | the previous handoff this session continued; not merged | `git log main..claude/handoff-6341c618-findings` |
| branch | `claude/replay-td` | TD docs, reviewed clean; not merged; waits for the fix plan | `git log main..claude/replay-td` |
| state | nothing is pushed, and no code changed this session | as written | `git log origin/main..main` |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "do a full audit of the playing code before making any changes. i dont trust any of this anymore." | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "Yes, recommended scope" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "Yes, second wave" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "Refuse stale (Recommended)" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "Stop the run (Recommended)" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "why are there three modes? who came up with this rule? what is the exact text i was presented with? there is only one way to correctly handle a skip and hydra's engine already knows it" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "that was about multiplier squeezes. hydra's engine already knows how to handle activations. you keep trying to reinvent new rules instead of reading what we already have" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "Yes, that rule" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "All of it (Recommended)" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "Source facts (Recommended)" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "Judge vs Hydra (Recommended)" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "One frame (Recommended)" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "i provided 45 as an example. why do you keep carrying it forward as if it was a rule? hydra already knows what timing you need and it can be split equally" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "are the facts actually unknown or did you never check what we already have documented?" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "Flag and test all (Recommended)" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "i care about the runs working, not the window coming into focus" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "you said that "most" is already documented. what do you need that isnt documented? that looks like a lot we already have that you didnt check" | the transcript |
| ruling | `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e` | "create two handoffs. one to continue this work, and one to search for other documentation missing from our central docs. are either of those real problems, or edge cases you invented?" | the transcript |

## Checker output

```
ok | branch | claude/player-audit-plan | holds the audit plan and the audit folder; not merged
not-on-main | commit | 3404e324d630e286c7848164e6e110c8880c3fec | the reviewer's fix to the audit plan, signed off clean; not on main
not-on-main | commit | 98a03d773755e49c0c372b32de4614a829aa181d | the audit branch's tip: answers to Q5 to Q13 and the Q11 follow-up; not on main
manual | state | docs/superpowers/plans/2026-11-08-player-audit.md | as written
manual | state | docs/audit/2026-11-08-player-audit/ | on `claude/player-audit-plan` only
manual | state | exits.md | as written
manual | state | Q11's two-note press rule and Q14 and Q15 are not answered; L06 and L46 closures are not yet checked by a fresh agent | as written
ok | path | C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\ch_mcp\agent\kit.js | the stale built kit agent, 2026-11-06 21:31, with no `after_ms` or `groupsOf`
ok | function | Chord::activation_note | Hydra's one rule for the gem that activates: first of Green, Blue, Yellow, Red, Kick, in `src/core/model.cpp`
ok | function | _kept_gems() | the Python decline modes the Q3 ruling removes, in `tools/ch_probe/experiments/play_chart.py`
ok | path | docs/audit/drum-lanes/FINDINGS-play-time.md | documents the step loop, the hit clock and the window edge that SP-29, SP-30 and TM-25 lack
ok | path | docs/audit/drum-lanes/play/f20deaa0.asm | the press handler listing that can settle which note a press goes to
ok | path | docs/handoffs/2026-11-08-clone-hero-docs-gaps-handoff.md | the separate task to find documentation missing from `docs/clone-hero/`
ok | branch | claude/handoff-6341c618-findings | the previous handoff this session continued; not merged
ok | branch | claude/replay-td | TD docs, reviewed clean; not merged; waits for the fix plan
manual | state | nothing is pushed, and no code changed this session | as written
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "do a full audit of the playing code before making any changes. i dont trust any of this anymore."
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "Yes, recommended scope"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "Yes, second wave"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "Refuse stale (Recommended)"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "Stop the run (Recommended)"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "why are there three modes? who came up with this rule? what is the exact text i was presented with? there is only one way to correctly handle a skip and hydra's engine already knows it"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "that was about multiplier squeezes. hydra's engine already knows how to handle activations. you keep trying to reinvent new rules instead of reading what we already have"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "Yes, that rule"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "All of it (Recommended)"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "Source facts (Recommended)"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "Judge vs Hydra (Recommended)"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "One frame (Recommended)"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "i provided 45 as an example. why do you keep carrying it forward as if it was a rule? hydra already knows what timing you need and it can be split equally"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "are the facts actually unknown or did you never check what we already have documented?"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "Flag and test all (Recommended)"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "i care about the runs working, not the window coming into focus"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "you said that "most" is already documented. what do you need that isnt documented? that looks like a lot we already have that you didnt check"
manual | ruling | 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e | "create two handoffs. one to continue this work, and one to search for other documentation missing from our central docs. are either of those real problems, or edge cases you invented?"
checked 34 rows: 9 ok, 0 missing, 2 not-on-main, 0 not-pushed, 23 manual
```
