# Handoff: checking every Unknown block of the Clone Hero reference

Session 77057df2-e94e-4ae2-9526-82deb22eb27c, 2026-11-08. The user said "stop and create a handoff" while round 2 was finishing.

## The goal

The user asked: "every claim labeled unknown should be checked". Asked whether the 30 "Hydra assumes it, never checked" blocks were in too, they chose "Unknown only". So the job is every block whose `Label:` line is `Unknown` on `docs/clone-hero/0*.md` (96 when the plan was written). The plan is `docs/superpowers/plans/2026-11-08-clone-hero-unknowns.md` on branch `claude/ch-unknowns-plan`. It was reviewed and the user approved it (they clicked "Approve, my answers", then wrote "i clicked on the approve option you presented to me"). The plan is not merged.

Before this, the same session finished the docs-gaps write-backs: the four page branches and a follow-ups branch, each reviewed, are merged on local `main` (the follow-ups merge is 356e48230590739a1decd4598eac3c8623e35434). That work is on `main` but not pushed.

## What was done

Code-reading agents worked in worktrees cut from the plan branch, one branch per workstream. Each wrote its results to `docs/audit/2026-11-08-clone-hero-unknowns/<W>.md` on its own branch, plus new listings under `docs/clone-hero/code/6142/` with a `.md` beside each. No agent edited an area page, the README, `facts-6142.tsv`, `questions.md` or `live-checks.md`. Nothing on these branches is merged, and nothing is pushed.

Round 1 (W1 to W4, pages 1 to 3) is finished, and a fresh checker read every proposed label move against the listings. Round 2 (W5 to W8) is finished except W8c, which was stopped mid-task: it committed IN-44 and had not written IN-55 or IN-45. Fresh checkers have read W5 and W6's first three blocks (FL-08, FL-10, FL-29). They have not read W6's second pass, W7 or W8. Round 3 (W9 to W11) has not started.

The checkers' verdicts, with the exact narrowed wording each block needs, are committed in `docs/audit/2026-11-08-clone-hero-unknowns/checker-verdicts.md`. The user's answers to the plan's questions and to each difference from Hydra are in `docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md`. Both are on this handoff's branch. Read them before writing any page: the agents' result files often claim more than the checkers passed.

What the code showed, in short. About 40 blocks now have a proposed move to "Proven in the game's code", in whole or in part. SC-29 is settled: the game counts stars on the score without the solo bonus and the clean-play bonus, as Hydra does (`score_without_solo()`), so User Guide line 382 stays as it is once that move is checked. Several blocks stay Unknown but are narrowed, each with its next step written down.

Differences from Hydra that came to the user, and their answers:

- CR-51: the game reads a `.srb` only when its version is 20210228; Hydra never checks. The user said "Record it only".
- CR-24: the game does not make a section from a `[section` text that holds `_ENDOFSONG`; Hydra does. 31 of the user's `.mid` charts have one. On The Cure "Fascination Street" Hydra shows a last section " ENDOFSONG" at 311,982 ms. The user first said "Search, then ask", then "Match the game". That is a separate code plan for `marker_name()` in `src/parse/section_names.cpp`. The no-space form (`[section_intro]`, 9 files) belongs to CR-29 and its live step.
- NL-58: the game drops a drum lane whose first pad chord lands more than resolution/16 ticks after the lane starts; Hydra lights it (`light_drum_lanes()`). 75 lanes in 50 charts. The user said "Record it only".
- FL-08: the game skips every written fill that starts on or after the first coda; Hydra has no coda rule (`place_authored_fills()`). 327 charts. The user asked "what is a coda event? does this change any scores?". The session checked "Cherub Rock": none of Hydra's top five paths uses the fill after the coda, so its score does not change. The session then asked whether to run the path finder on all 327 charts. That question is unanswered.
- FL-10: when two written fills land on one chord, the game's deadline comes from the earlier fill and Hydra's from the later (`apply_fill_end()`). 2 charts. The user asked "does this change any scores". Pangaea's chart uses no Star Power on its best path. RAT KING "Open Hands to the Void" activates on such a chord, with fills 31 ticks apart at resolution 960. Whether Star Power becomes ready inside that 31-tick window is not checked. No answer yet.

Also found but not yet put to the user: SP-06 and SP-08, overlapping Star Power phrases (the game dedupes, trims and stretches them; Hydra keeps one open phrase). They need a fresh checker first. FL-23: Hydra's fill boundary rests on the user's earlier fill ruling ("85 ms both sides", memory note fill-spawn-comparison); check that ruling before asking anything.

How the work ran. The `rea` tools do not load in agents, and Ghidra times out on the game's DLL even in the main session. Every caller list is a byte scan of the DLL, so each is marked "direct callers only". `tools/ch_mcp/chdis.py` runs only under `C:\Python314\python.exe` without `-I`, because capstone is in the user site-packages. Agents finished about four blocks per 20 minutes, not the eight the plan expected.

## What is next

1. Answer the open questions with the user: FL-08 (run the path finder on all 327 coda charts or record) and FL-10.
2. Finish W8c on `claude/ch-unk-w8b` (IN-55, IN-45), and run round 3 (W9, W10, W11) at about four blocks per agent, from the plan's Workstreams section.
3. Send fresh checkers for W6's second pass, W7, W8, W8c and round 3, with the same brief the round 1 checkers had: open every cited line, then PASS, REOPEN or GONE.
4. Put SP-06 and SP-08 to the user after their checker, with real chart examples, using the same approved-search pattern: ask before reading the chart library.
5. Show the user every label move on a block that names a Hydra owner, then run the page executors (sets 01+03, 02, 04+05, 06+07) using the wording in `checker-verdicts.md`. They also update the README counts, `live-checks.md` and `questions.md` (Q entries for CR-51, NL-58 and any "Record it only"), and run `tools/ch_mcp/dump_facts.py` for the new listings. Then a fresh agent checks every move (audit-method stage 6), the main session builds and runs the docs check, a reviewer reads each branch, and the branches merge.
6. Live steps come last, one at a time: the plan's LS-U1 to LS-U5, plus TM-40 at 50% speed, TM-16, the FL-32 screen capture, the IN-24 breakpoint retry and the IN-73 network capture the user approved, and an NL-71 Note Shuffle rerun through the game's setter.
7. Two code plans the user ruled for: CR-24's `_ENDOFSONG` rule, and NL-55's lane pad (the docs-gaps Q1, "Match the game (Recommended)").

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| branch | `claude/ch-unknowns-plan` | the reviewed, approved unknowns plan; not merged | `git log main..claude/ch-unknowns-plan` |
| commit | `c9b45492d16588f4162ddd148e310f97f586df03` | the plan branch's tip; not on main, not pushed | `git branch --contains c9b45492d16588f4162ddd148e310f97f586df03` |
| branch | `claude/ch-unk-w1` | W1 results (CR-03, CR-05, CR-07, CR-12, CR-17, CR-18, CR-24, CR-51), three passes, checked | `git log main..claude/ch-unk-w1` |
| commit | `15bee3c0a18b1979521da0c7fe90cc531b30e790` | W1 tip; not on main | the checker |
| branch | `claude/ch-unk-w2` | W2 results (CR-30, CR-35, CR-45, CR-47, CR-48, TM-06, TM-08, TM-10), checked | `git log main..claude/ch-unk-w2` |
| commit | `65297ed325f0897096e87eaaf381256eb558a995` | W2 tip; not on main | the checker |
| branch | `claude/ch-unk-w3` | W3 results (NL-12, NL-13, NL-14, NL-36, NL-39, NL-70, NL-73), checked | `git log main..claude/ch-unk-w3` |
| commit | `db3f38fa1ad22a539dfcf6c42bc38e353db9ed42` | W3 tip; not on main | the checker |
| branch | `claude/ch-unk-w4` | W4 results (NL-54, TM-16, TM-32, TM-36, TM-40, TM-43, TM-46, question 4), checked | `git log main..claude/ch-unk-w4` |
| commit | `f3a1ce7733e499d9361a87a67553af2f08320c63` | W4 tip; not on main | the checker |
| branch | `claude/ch-unk-w5` | W5 results (NL-30, NL-42, NL-48, NL-56, NL-57, NL-58, NL-69, NL-71); checked at cb453227, second pass (NL-57, NL-58's resolution) not checked | `git log main..claude/ch-unk-w5` |
| commit | `fb2676170f6caf3eb6ea97e04eb9be89c0cf9006` | W5 tip; not on main | the checker |
| branch | `claude/ch-unk-w6` | W6 results (FL-08, FL-10, FL-29 checked; SP-06, SP-07, SP-08, FL-22, FL-32 not checked) | `git log main..claude/ch-unk-w6` |
| commit | `3fa81d86030985f6456852bf5d0b2abb282e2894` | W6 tip; not on main | the checker |
| branch | `claude/ch-unk-w7` | W7 results (SP-15, SP-29, SP-30, SP-35, SP-43, IN-88, IN-90, FL-23) and the committed SP-43 scan; not checked | `git log main..claude/ch-unk-w7` |
| commit | `703fa92e4af8f28ab18ef84119717efd33168e97` | W7 tip; not on main | the checker |
| branch | `claude/ch-unk-w8` | W8 results (SC-09, SC-23, SC-26, SC-29, SC-37); not checked | `git log main..claude/ch-unk-w8` |
| commit | `439f6107783982ed80524f44b7147d80667924ea` | W8 tip; not on main | the checker |
| branch | `claude/ch-unk-w8b` | W8c, stopped by the user's "stop": IN-44 committed in W8c.md, IN-55 and IN-45 not written | `git log main..claude/ch-unk-w8b` |
| commit | `104fe3d75b5cda0912dbbc15cdd5bbe9fae8532a` | W8c tip; not on main | the checker |
| commit | `356e48230590739a1decd4598eac3c8623e35434` | the docs-gaps follow-ups merge; on main, not pushed | `git merge-base --is-ancestor 356e48230590739a1decd4598eac3c8623e35434 origin/main` exits 1 |
| path | `docs/audit/2026-11-08-clone-hero-unknowns/checker-verdicts.md` | every checker verdict so far, with the narrowed wording per block | the checker |
| path | `docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md` | the user's answers and the open questions, with the chart examples | the checker |
| path | `docs/agents/rulings/clone-hero.md` | holds the 2026-11-08 ruling line "every claim labeled unknown should be checked" with "Unknown only"; that line is on the plan branch only | `git show claude/ch-unknowns-plan:docs/agents/rulings/clone-hero.md` line 5 |
| function | `score_without_solo()` | Hydra counts stars without the solo bonus; SC-29's proposed move says the game does the same | read `src/core/stars.h` |
| function | `marker_name()` | Hydra's section-marker rule; CR-24's ruled code change goes here | read `src/parse/section_names.cpp` |
| function | `light_drum_lanes()` | Hydra lights lanes with no cut-off (NL-58, recorded only) | read `src/parse/song.cpp` |
| function | `place_authored_fills()` | Hydra lands written fills with no coda rule (FL-08, open) | read `src/parse/song.cpp` |
| function | `apply_fill_end()` | a later fill replaces the earlier one's length (FL-10, open) | read `src/parse/song.cpp` |
| ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | "every claim labeled unknown should be checked" | the session transcript |
| ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | scope: "Unknown only" | the session transcript |
| ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | plan approval: "Approve, my answers", then "i clicked on the approve option you presented to me" | the session transcript |
| ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | "plans are another agent's summary and should not be treated as authoritative facts. the code is the source of truth. approve but every fact must be checked against the real code." | the session transcript |
| ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | Q1 "Proven in code"; Q2 "Code first, then ask"; Q3 "Yes, once"; Q7 "Yes, capture"; Q10 "Retry once"; Q12 "Keep Unknown"; Q13 "Capture once" | the session transcript |
| ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | on plan Q5: "why are you presenting this to me? this isnt a fact about the game" (only game behaviour or permissions go to the user) | the session transcript; memory note ask-only-game-or-permission-questions |
| ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | CR-51: "Record it only"; CR-24: "Search, then ask", then "Match the game"; NL-58: "Record it only" | the session transcript |
| ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | chart searches: "Yes, search once" and "Yes, same run" (one read-only search, now used up) | the session transcript |
| ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | FL-08: "what is a coda event? does this change any scores?"; FL-10: "does this change any scores" (both still open) | the session transcript |
| ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | "stop and create a handoff" | the session transcript |
| state | the worktrees `.claude/worktrees/ch-unk-w1` to `ch-unk-w8` and `ch-unk-w8b` exist, each on its branch | the facts above | `git worktree list` |
| state | nothing from this run is pushed, and no unknowns branch is merged | the fact | `git log origin/main..main --oneline`; `git branch --merged main` |

## Checker output

```
ok | branch | claude/ch-unknowns-plan | the reviewed, approved unknowns plan; not merged
not-on-main | commit | c9b45492d16588f4162ddd148e310f97f586df03 | the plan branch's tip; not on main, not pushed
ok | branch | claude/ch-unk-w1 | W1 results (CR-03, CR-05, CR-07, CR-12, CR-17, CR-18, CR-24, CR-51), three passes, checked
not-on-main | commit | 15bee3c0a18b1979521da0c7fe90cc531b30e790 | W1 tip; not on main
ok | branch | claude/ch-unk-w2 | W2 results (CR-30, CR-35, CR-45, CR-47, CR-48, TM-06, TM-08, TM-10), checked
not-on-main | commit | 65297ed325f0897096e87eaaf381256eb558a995 | W2 tip; not on main
ok | branch | claude/ch-unk-w3 | W3 results (NL-12, NL-13, NL-14, NL-36, NL-39, NL-70, NL-73), checked
not-on-main | commit | db3f38fa1ad22a539dfcf6c42bc38e353db9ed42 | W3 tip; not on main
ok | branch | claude/ch-unk-w4 | W4 results (NL-54, TM-16, TM-32, TM-36, TM-40, TM-43, TM-46, question 4), checked
not-on-main | commit | f3a1ce7733e499d9361a87a67553af2f08320c63 | W4 tip; not on main
ok | branch | claude/ch-unk-w5 | W5 results (NL-30, NL-42, NL-48, NL-56, NL-57, NL-58, NL-69, NL-71); checked at cb453227, second pass (NL-57, NL-58's resolution) not checked
not-on-main | commit | fb2676170f6caf3eb6ea97e04eb9be89c0cf9006 | W5 tip; not on main
ok | branch | claude/ch-unk-w6 | W6 results (FL-08, FL-10, FL-29 checked; SP-06, SP-07, SP-08, FL-22, FL-32 not checked)
not-on-main | commit | 3fa81d86030985f6456852bf5d0b2abb282e2894 | W6 tip; not on main
ok | branch | claude/ch-unk-w7 | W7 results (SP-15, SP-29, SP-30, SP-35, SP-43, IN-88, IN-90, FL-23) and the committed SP-43 scan; not checked
not-on-main | commit | 703fa92e4af8f28ab18ef84119717efd33168e97 | W7 tip; not on main
ok | branch | claude/ch-unk-w8 | W8 results (SC-09, SC-23, SC-26, SC-29, SC-37); not checked
not-on-main | commit | 439f6107783982ed80524f44b7147d80667924ea | W8 tip; not on main
ok | branch | claude/ch-unk-w8b | W8c, stopped by the user's "stop": IN-44 committed in W8c.md, IN-55 and IN-45 not written
not-on-main | commit | 104fe3d75b5cda0912dbbc15cdd5bbe9fae8532a | W8c tip; not on main
not-pushed | commit | 356e48230590739a1decd4598eac3c8623e35434 | the docs-gaps follow-ups merge; on main, not pushed
ok | path | docs/audit/2026-11-08-clone-hero-unknowns/checker-verdicts.md | every checker verdict so far, with the narrowed wording per block
ok | path | docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md | the user's answers and the open questions, with the chart examples
ok | path | docs/agents/rulings/clone-hero.md | holds the 2026-11-08 ruling line "every claim labeled unknown should be checked" with "Unknown only"; that line is on the plan branch only
ok | function | score_without_solo() | Hydra counts stars without the solo bonus; SC-29's proposed move says the game does the same
ok | function | marker_name() | Hydra's section-marker rule; CR-24's ruled code change goes here
ok | function | light_drum_lanes() | Hydra lights lanes with no cut-off (NL-58, recorded only)
ok | function | place_authored_fills() | Hydra lands written fills with no coda rule (FL-08, open)
ok | function | apply_fill_end() | a later fill replaces the earlier one's length (FL-10, open)
manual | ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | "every claim labeled unknown should be checked"
manual | ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | scope: "Unknown only"
manual | ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | plan approval: "Approve, my answers", then "i clicked on the approve option you presented to me"
manual | ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | "plans are another agent's summary and should not be treated as authoritative facts. the code is the source of truth. approve but every fact must be checked against the real code."
manual | ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | Q1 "Proven in code"; Q2 "Code first, then ask"; Q3 "Yes, once"; Q7 "Yes, capture"; Q10 "Retry once"; Q12 "Keep Unknown"; Q13 "Capture once"
manual | ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | on plan Q5: "why are you presenting this to me? this isnt a fact about the game" (only game behaviour or permissions go to the user)
manual | ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | CR-51: "Record it only"; CR-24: "Search, then ask", then "Match the game"; NL-58: "Record it only"
manual | ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | chart searches: "Yes, search once" and "Yes, same run" (one read-only search, now used up)
manual | ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | FL-08: "what is a coda event? does this change any scores?"; FL-10: "does this change any scores" (both still open)
manual | ruling | 77057df2-e94e-4ae2-9526-82deb22eb27c | "stop and create a handoff"
manual | state | .claude/worktrees/ch-unk-w1 | the facts above
manual | state | nothing from this run is pushed, and no unknowns branch is merged | the fact
checked 41 rows: 18 ok, 0 missing, 10 not-on-main, 1 not-pushed, 12 manual
```
