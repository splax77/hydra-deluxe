# Handoff: step 2, the parser and Clone Hero rule fixes (runs in parallel with step 1)

This is the second of two handoffs from the planning session (cebf3006). The first, `docs/handoffs/2026-10-03-step1-plan-handoff.md`, is being executed now by another session (its scratch folders carry the id `ade9655b`). It works in worktrees named `.claude/worktrees/s1-*` on branches `claude/s1-*`, integrating on `claude/s1-int`. This session takes step 2 of the audit's fix order, the parser drifts, plus the Clone Hero rule questions the evidence now answers. Plan it, settle its calls with the user, then execute what doesn't collide with step 1.

## Why this can run in parallel, and where it can't

Step 1 touches `src/parse` in exactly one task, Task 3, the load guard: timing that can't measure time refuses the chart (D6). Every later step-1 branch carries that change and nothing more under `src/parse`. Task 3 is on `claude/s1-t3` (1abd188) and is integrated into `claude/s1-int`. Step 1 does not touch `src/app/dynamics_breakdown.cpp`, `src/core/timing.*`, `tools/ch_probe` or `src/net`.

So branch step 2 from `claude/s1-t3`, not from `main`. That gives the parser fixes Task 3's code, so the eventual merge doesn't conflict in `song.cpp`. Re-check with `git diff --name-only main...claude/s1-int -- src/parse` before starting. If step 1 has touched `src/parse` again since then, rebase onto the newer commit.

Three things in step 2 do collide with step 1, so plan them now and execute them only after step 1 merges:

- **Finding 37**: `src/search/graph.cpp`, which step 1's Tasks 11, 12 and 17 rewrite.
- **Finding 304**: `src/core/backend_value.h`, step 1's Task 15.
- **Any results-stamp bump.** Step 1's Task 22 bumps `kResultsStamp` to the release version. If step 2 ships in the same release, it reuses that value and doesn't bump twice. Step 2 owns `kDynamicsCountStamp` alone, because the dynamics findings change what the Dynamics tab counts.

Don't touch the `s1-*` worktrees, `claude/s1-*` branches or `.superpowers/sdd/2026-10-03-step1-engine-facts/` beyond reading. If you need to coordinate, find the step-1 session with `ListAgents` and message it.

## Shared files: re-read before writing

`docs/audit/2026-10-03-fix-decisions.md` is written by both sessions. Step 1 has already added D10 to D14. Re-read it before each append, and number new decisions after the highest one present. Put step 2's calls under a "## Step 2: parser and Clone Hero rules" heading. `docs/audit/2026-10-03-fix-triage.md` and its `.json` list every finding's fix group, owner and open question. Don't rewrite them; record outcomes in the decisions file and the plan.

## The evidence you start with

`docs/audit/ch-evidence.md` answers most of the Clone Hero fact questions. An agent read Clone Hero 1.1's code statically through an earlier Il2CppDumper run. Every answer cites a code address, and the disassembly listings are in that session's scratch `re\` folder. It is strong evidence, not a test. Before you tell the user a finding is "proven", quote the address and say it was read, not run.

## Scope, grouped by the fix each needs

Finding numbers are from `docs/audit/2026-10-03-derivation-audit.md`. Read each finding in full before planning it.

**Score-changing rules the evidence settles.** These change analysis results, so they need the user's yes even though the evidence is clear.

- **Disco flip per difficulty (findings 11 and 250).** Hydra applies Expert's `mix 3` markers at every difficulty, so Hard-and-below Pro Drums results are wrong on charts with disco sections. The game's rule now lives in docs/clone-hero/02-note-list.md (NL-17, and NL-21 for the dnoflip bug). The fix must keep the dnoflip rule: Hydra treats `drums0dnoflip` as off, and the Clone Hero bug that ignores it must not be copied (memory note `ch-disco-noflip-rule`).
- **2x kicks below Expert (findings 10, 12 and 255).** Hydra's Dynamics tab reads Expert's 2x kicks at Hard and below. What the game reads at each difficulty now lives in docs/clone-hero/02-note-list.md (NL-24 to NL-26). The evidence suggested the audit's proposed fix, an Expert-only gate on pitch 95, may be the wrong shape. Re-read the evidence's "Things that surprised me" section, and decide with the user which pitch each difficulty reads.
- **SP phrase edges (finding 21).** The game's phrase edges now live in docs/clone-hero/05-star-power.md (SP-01 to SP-03, SP-07). Hydra's `.mid` side matched them, but its `.chart` side got the zero-length and past-the-last-note cases wrong. The fix is one phrase-end helper both parsers feed.
- **Fill landing window (finding 315).** The game's landing window, its tie rule and what it does with a fill that has no chord now live in docs/clone-hero/04-fills.md (FL-03, FL-05 to FL-07). The evidence said Hydra's window was one tick narrower and that Hydra matched the other two cases. Whether one authored fill stops fill generation is FL-27 and FL-28 on the same page.

**Rules the evidence confirms Hydra already gets right.** These need only a recorded decision (an ADR line or CONTEXT.md), not a code change, unless the finding also names a display problem.

- **Finding 64.** The evidence confirmed that Hydra's per-note dynamics gate matches the game's (NL-13 in docs/clone-hero/02-note-list.md). What's left is the Dynamics tab saying "dynamics on" while early notes are plain. That's a wording decision.
- **Finding 141.** The evidence confirmed that Hydra's handling of a quoted `E "solo"` matches the game's (CR-33 in docs/clone-hero/01-reading-a-chart.md). The leftover is section names stripping quotes, which is a separate small owner (the game's rule is CR-32).
- **Finding 320.** The evidence confirmed Hydra's assumption about `.chart` ghosts and accents with no enabling tag (NL-06 in docs/clone-hero/02-note-list.md).

**Still unknown; need a live check or the user's call.**

- **Findings 53 and 314**: which meter sets a generated fill's length, and whether beats are quarter notes in 6/8 and 7/8. Also the open half of 315. What is now known on both lives in docs/clone-hero/04-fills.md (FL-16, FL-20, FL-26). The evidence's cheapest check was one breakpoint on Clone Hero's fill-adding method (FL-02 on the same page) with `tools/ch_probe`, while playing a chart with no authored fills. Running Clone Hero needs the user. Ask before planning a live run, and keep to one CPU-heavy tool at a time.
- **Finding 222**: the score's lack of a speed factor now lives in docs/clone-hero/06-scoring.md (SC-03) and 03-timing.md (TM-42). One fact here has no row on the pages, so it stays: the evidence said Clone Hero keeps a separate leaderboard per speed. Whether off-speed runs count as "above optimal" on the leaderboard page is the user's call.

**Small parser owners with no Clone Hero question.** These are low-weight. Each needs a recommendation and the user's yes for anything visible.

- **Finding 60**: blank artist or charter shows blank for `.ini`/`.sng` but a placeholder for `.srb`.
- **Finding 61**: a container's notes entry is recognised by name in `.sng` but by extension in `.srb`.
- **Findings 258 and 319**: the ticks-per-measure rule is restated in the constructor and fixtures, and the default 4/4 is written three ways.
- **Finding 331**: a `.chart` modifier with no note under it fails the whole chart.
- **R7.5**: `.chart` Offset "500ms" is read as 500 s, song.ini drops it, and both accept `nan`.
- **R7.7**: practice sections are sorted from `.chart` but kept in track order from `.mid`.

**Already done by step 1, Task 3 (D6):** R7.6 (negative tempo) and finding 100 (zero time signature). Don't redo them; check that their tests exist on `claude/s1-t3`.

**Tool-only, no collision:** finding 77. Clone Hero's whole hit-window cap now lives in docs/clone-hero/03-timing.md (TM-22, and TM-23 for the live values); `poll_windows.csv` measured the same, and the `tools/ch_probe` scripts disagreed. This also answers steps 1-3 of `docs/superpowers/plans/2026-09-25-hit-window-testing.md`. Tell the user that, and ask whether that plan's live run is still wanted.

**After step 1 merges:** finding 37 (the game's meter clamp during SP and its end recompute, now SP-24, SP-34 and IN-27 in docs/clone-hero/05-star-power.md and 07-internals.md; the evidence matched them to `extend_deacts`, not `add_deact_edge`) and finding 304 (the 3 ms leeway edge, Hydra's own rule and the user's call; SP-42).

## How to run it

1. Read this handoff, the evidence file, the triage rows for the findings above, and the decisions file.
2. **Decisions round.** Bring the user the weighty calls one at a time, as the previous session did. For each, say how it works today, what each answer changes (including which charts' scores move), what the evidence says with its strength, and a recommendation. Expect these: disco per difficulty, the 2x-kick pitch per difficulty, the phrase edges, the fill landing window, the finding-64 tab wording, the speed question (222), and whether to do a live Clone Hero check for 53, 314 and 315. Give the low-weight ones recommendations in one batch.
3. **Measure before planning.** For each score-changing rule, count how many corpus charts change score at each difficulty. Use a scratch copy of the binaries and a scratch database. The step-1 baseline scripts in `.superpowers/sdd/2026-10-03-step1-engine-facts/partD/scores.ps1` show the pattern, so copy, don't edit. Give the user the numbers with the decision.
4. **Write the plan** at `docs/superpowers/plans/2026-10-03-step2-parser-rules.md`, in plain English, tests first, real code, HEAD line numbers on the `claude/s1-t3` base. The previous session learned one lesson: when parallel agents draft parts of one record or one parser, write integration rulings before joining their drafts. Otherwise each adds its own copy of a fact. Mark the step-1-blocked tasks (37, 304) as waiting on the step-1 merge.
5. Ask the user how to execute it.

## Rules

The user's rule for this phase: "run important things by me before changing anything". Explain how a thing works today and what would change, and get a yes before editing code. Any visible change is a blocking question up front. Everything is derived once: one owner per rule, and display code reads stored or parsed facts. Before saying a change needs no re-analysis, check `src/store/stored_versions.h`. Parser changes that alter chart output bump `kDynamicsCountStamp` where counts change, and need the results-stamp decision above (finding 344 says the parser rule is unwritten, so say so when it applies). Plain English per CLAUDE.md. Stage by file name, never `git add -A`. Every agent brief carries the status-line sentence with `hooks\state\status` and the session id for commit trailers. Keep fan-out lean: three or four agents. Fix hooks that block wrongly; don't route around them.

The review gate from `docs/handoffs/2026-10-03-derive-once-gate-and-round7-handoff.md` is still unfinished and is not this session's job. No step-2 fix merges to `main` before the gate is live and the user says yes.

## Journal state at handoff

The handoff hook supplied no journal lines. The planning session (cebf3006) launched nothing for this handoff and has nothing in flight. The step-1 execution session (ade9655b) was active at the time of writing. By 21:09, its integration branch `claude/s1-int` held Tasks 1-5, 11, 12 and 14-16, going by its merge commits, with Task 3's review fixes (d6b5640). Tasks 6, 8, 9, 17, 18 and 20 were still on their own branches.
