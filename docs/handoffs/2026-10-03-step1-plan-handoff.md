# Handoff: the step-1 fix plan is written; execution, the review gate and step 2 are next

This picks up from `docs/handoffs/2026-10-03-derive-once-gate-and-round7-handoff.md`. That session asked whether to run audit round 8 or start planning. The user chose planning. This session sorted every audit finding into fix groups, settled the step-1 decisions with the user, gathered Clone Hero evidence for the fact questions, and wrote the step-1 plan. No code was edited, built or committed.

## Where to start

Three jobs are open. They don't depend on each other, except that no fix should merge to `main` before the review gate is live.

1. **Execute the step-1 plan.** It is `docs/superpowers/plans/2026-10-03-step1-engine-stores-what-displays-guess.md`, with its task file next to it. The user asked for a handoff instead of choosing an execution mode, so ask them: subagent-driven in the new session, or a separate parallel session in a worktree.
2. **Finish the review gate.** Nothing changed there since the previous handoff. Its "Review gate: state and next step" section is still accurate: Task 3's fix round 2, then registering the hooks, then the user's yes on the merge.
3. **Step 2's decisions.** These are the parser drifts. The Clone Hero evidence below answers most of the fact questions.

## What this session produced

All of these are untracked. Stage by name if the user wants them committed.

- `docs/audit/2026-10-03-fix-triage.md` and `.json`. Every one of the 396 findings (352 plus R7.1-R7.44) has a row: its fix step, fix group, owner, status at HEAD, whether it needs a user call, and whether stored results change. 133 findings need a call. 33 of those are weighty.
- `docs/audit/2026-10-03-fix-decisions.md`, decisions D1 to D9. D1-D5 are the step-1 engine calls. D6-D9 are the plan's Q1-Q4.
- The step-1 plan and its `.tasks.json`.
- `.superpowers/sdd/2026-10-03-step1-engine-facts/` holds the baseline scripts (`partA`, `partB`, `partD`), the Clone Hero evidence (`ch-evidence.md`) and the rulings used to join the plan (`integration-rulings.md`). `.superpowers/` is not git-ignored in this repo, so never stage it.

## The step-1 plan in brief

The engine knows every way a Star Power window's end moved, but the record keeps only the final end. So the transfer scales, the Preview gauge, the replay and the tied variants each guess the rest, and get corners wrong. The plan stores one SP-end history per activation. The stored `deact_tick`, `clamp_tick` and `collected_phrase_ticks` become read-only views of it with the same names. Three stored counts become the lengths of lists that hold the actual ticks. The squeeze-out, the 500 ms window and "did SP pay this chord" each get one owner.

There are 22 tasks. Tasks 1 to 3 change nothing in the engine. Task 1 runs the existing one-squeeze-rating plan unchanged. Task 4 captures a baseline at HEAD on all 97 corpus charts. Every later task must reproduce it, except for differences it names. Task 22 bumps `kPathFormatStamp` to 7 and `kResultsStamp` once, writes ADRs 0021 and 0022 and the doc amendments, and runs the final proof. Until Task 22 lands, no development build may open a real `hydra.db`. Scratch databases only.

Scores, path strings and path lists must not change. The one allowed exception is D7: a tied variant may gain a + or − in its path string.

How the plan was made: four Opus agents each drafted one part. Joined as they were, the drafts stored several facts twice. Examples: a new history list whose last entry repeated `deact_tick`, and lists whose sizes repeated stored counts. The rulings in `integration-rulings.md` fixed that, and the agents revised against them. One accessor name was reconciled by hand. Part A's `anchor_of_step` is Task 5's `end_anchor_tick`, and Task 8 now builds on Task 5's accessors. The drafters did not build or run anything, and their fixture ticks were worked out by hand. Expect some test values to need correcting on the first red-green run. Treat such a fix as normal TDD, and name it in the commit.

**Q5 to Q10 in the plan header use their recommended answers unless the user says otherwise.** Confirm that with the user before execution. Tasks 14, 16 and 21 are conditional on Q7, Q8 and Q9. Q8's recommendation asks for one FC video check first.

## Clone Hero evidence for step 2

An agent read Clone Hero 1.1's code statically, through an earlier session's Il2CppDumper output. Every answer gives its code address. Full write-up: `docs/audit/ch-evidence.md`. It is strong evidence, but not a test.

- **Finding 11.** Disco flip is set per difficulty in both formats. `[mix 2 ...]` applies to Hard. Hydra applies Expert's markers at every difficulty, so Hard-and-below Pro Drums scores are wrong on charts with disco sections.
- **Finding 21.** A phrase covers start ≤ tick < end. A chord on the end tick is outside it. A zero-length phrase awards nothing. A phrase running past the last note is awarded on that note. Hydra's `.mid` side matches, but its `.chart` side gets the last two cases wrong.
- **Finding 37.** During SP the meter is clamped to full and the end is recomputed. That matches `extend_deacts`, not `add_deact_edge`. This fix can change scores, so it was taken out of step 1.
- **Finding 64.** Notes before the dynamics tag stay plain. Clone Hero gates per note, as Hydra does.
- **Finding 77.** The whole-window cap is 171.43 ms, from clamping the formula's input at 170 ms. This also answers steps 1-3 of the 2026-09-25 hit-window plan without a live run.
- **Finding 141.** A quoted `E "solo"` is not recognised. Hydra already matches.
- **Finding 222.** Speed has no factor in the score. Clone Hero keeps a separate leaderboard per speed. Whether off-speed runs count as "above optimal" is the user's call.
- **Finding 304.** The 3 ms leeway is Hydra's own rule, so it's the user's call.
- **Finding 315.** Ties go to the later chord, as Clone Hero does. Clone Hero's landing window is one tick wider than Hydra's (floor(res/32)+1). A fill with no chord is dropped, as in Clone Hero.
- **Still unknown:** findings 53 and 314, and whether one authored fill stops fill generation. The cheapest check is a breakpoint with `tools/ch_probe` on `0x215BD50` while playing a chart with no authored fills.
- **Also surfaced:** Clone Hero accepts 2x-kick MIDI notes 59, 71, 83 and 95 at every difficulty, not just Expert+. Its per-side hit window is about 85.7 ms at the widest gaps.

## Rules for the next session

The user's rule for this phase: "run important things by me before changing anything". Explain how a thing works today and what would change, and get a yes before editing code. Any visible change is a blocking question up front. Write plans and reports in plain English per CLAUDE.md. Stage by file name, never `git add -A`. Every agent brief carries the status-line sentence with `hooks\state\status` and the session id for commit trailers. Keep fan-out lean. When parallel agents each design part of one record, write integration rulings before joining their drafts.

## Session state at handoff

No workflows ran in this session. Every subagent finished and reported: three triage agents, one Clone Hero evidence agent and four plan drafters, each revised once. Nothing is in flight. This session's id is cebf3006-2e4a-46ed-85a2-1902d8175f82. Its scratchpad holds the drafts (`triage\draft_*.md`), but everything the plan needs was copied into the repo or the `.superpowers` folder.
