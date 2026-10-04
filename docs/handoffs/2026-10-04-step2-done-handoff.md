# Handoff: step 2 is built and integrated; what's left

This picks up from `docs/handoffs/2026-10-03-step2-parser-rules-handoff.md`. Session bbf41dc4 did that handoff's whole job. It measured each rule, got the user's decisions, wrote the plan and executed it. Step 2 now sits on its own integration branch, fully checked, waiting for step 1 and the review gate. This note says where everything is, what is still open, and what the next session should do.

## Where things stand

All ten step-2 tasks (T0 to T9) are merged on `claude/s2-int`, at commit a03731c. Its worktree is `.claude/worktrees/s2-int2`. The branch starts from `claude/s1-t3` (1abd188), as the old handoff asked. Nothing is on `main`; `main` is still at 02976c1.

Every check on the integrated branch passed. All 717 `hydra_tests` cases pass. `hydra_uitest --all --jobs 4` passed 61 of 61. `hydra_replay selfcheck` gives 382 PASS. The 175 probe-tool tests pass. On the 97 test charts, Expert is unchanged. At Hard, Medium and Easy, exactly the charts the plan predicted changed, to the predicted numbers. Every library spot check matched too. A second agent rebuilt from clean and confirmed all of this on its own.

The user's decisions are D19 to D31 in `docs/audit/2026-10-03-fix-decisions.md`, under "## Step 2: parser and Clone Hero rules". D32 and D33 are step 1's. D33 matters here: steps 1 and 2 ship together in release 2.1.0, and `kResultsStamp` becomes "2.1.0" in step 1's Task 22. Step 2 does not bump it a second time.

The plan is `docs/superpowers/plans/2026-10-03-step2-parser-rules.md`, with its `.tasks.json`. Its "Expected score changes, for the integrator" section is the table every check above was held to.

## What each task did, in one line each

- **T0** puts each difficulty's kick pitch, 2x-kick pitch and disco digit in one table.
- **T1** makes each difficulty read only its own disco markers (D19).
- **T2** makes each difficulty read its own 2x kick, lets the 2x Bass box work at every difficulty, and gives the Dynamics tab one kick total (D20).
- **T3** gives both chart formats one phrase-end rule (D21).
- **T4** widens the fill landing window by one tick and places each fill on its own (D22, D30).
- **T5** accepts the dynamics tag only in Clone Hero's two spellings, in file order, and adds the "from m:ss on" line (D24).
- **T6** gives six small parser rules one owner each (D27).
- **T7** gives off-speed leaderboard scores their own "other speed" status (D25).
- **T8** makes the probe tools use the measured 171.43 ms cap (D27, D28).
- **T9** bumps the Dynamics count and blob stamps and writes the records: ADRs, CONTEXT.md and the differences page.

## What is waiting, and on what

**Merging `claude/s2-int` to `main`.** Three things must hold first. Step 1 must merge to `main`, with its Task 22 stamp. The derive-once review gate must be live (`docs/handoffs/2026-10-03-derive-once-gate-and-round7-handoff.md`). And the user must say yes. The plan's "Gates before `main`" paragraph covers the one expected conflict, `src/store/stored_versions.h`. T9 Step 6 also checks that no release tag sits between step 1's Task 22 and this merge. If one does, it stops and asks the user before bumping the results stamp again.

**T10 (finding 37) and T11 (finding 304).** These branch from `main` once step 1 has merged. They touch engine files that step 1 is rewriting, which is why they wait. T10 must show the user its corpus and library count before it lands (plan T10 Step 7). T11 is one test pinning the strict 3 ms edge.

To check on step 1, use `ListAgents` to find session ade9655b, or read `git log claude/s1-int`. Don't touch `s1-*` worktrees or branches.

## Follow-ups the reviews found (the user hasn't decided these yet)

None of these is visible on screen. Ask the user before doing any of them.

1. **The 2x-kick pitch is stored twice.** T0's table holds it as its own column, but it is always the kick pitch minus one. The plan's own code asked for the column, and a test pins the two together, so they can't drift. The derive-once fix is to work it out as kick − 1 where it's read, and drop the column.
2. **The Clone Hero evidence file isn't tracked.** `tools/ch_probe/constants.py` and the hit-window plan cite `docs/audit/ch-evidence.md`, which git ignores. A fresh clone won't have it. The fix is to copy it, or at least section 77, under `docs/audit/` and point the citations there.
3. **Two leftovers from the fill rework.** Nothing puts a step in `MPhase::PreTimestamp` any more, so it is dead code. The comment above the marker tests in `tests/test_song.cpp` still says "These cases keep the old regexes as the oracle", but the dynamics half no longer uses a regex.
4. **Two flaky tests on a busy machine.** `hydra_uitest`'s `batch-strip-drift` fails now and then when about ten builds run at once. It failed at `uitest_batch_reports.cpp` lines 75 and 95, and passes when re-run. One unnamed `hydra_tests` case failed once in several worktrees and never came back. Neither comes from step 2; both look like timing under load.
5. **A hook bug, already filed.** `deletion_shrink_gate.ps1` refused an edit saying the brief didn't name the file, when it did. A task chip was raised for it ("Fix deletion_shrink_gate brief-name check"). Fix it with a test; don't route around it.
6. **A scratch-tool quirk.** `s2-scores.ps1` merges `hydra_replay`'s stdout and stderr with `*>`, so a FAILED line's two parts sometimes swap order. When they do, `s2-compare.ps1` flags a chart whose score didn't change. A re-run clears it. It only matters if the scripts are used again.

## Open questions recorded but not acted on

These are in the decisions and the plan's Appendix B. Nothing needs doing unless the user asks.

- **Live Clone Hero check: not now (D26).** Generated-fill length and 6/8 beats keep today's rule, written down as unverified against Clone Hero.
- **D24's caveat.** It assumes Clone Hero doesn't trim the dynamics tag's text before comparing it. One play of Programmed for Battle would settle it: do accents pay double?
- **D30's caveat.** Clone Hero's fill search takes a starting index, so it may carry its place from one fill to the next. That was not traced.
- **2x kicks.** The step where Clone Hero turns a note's 2x mark into the bit its Double Kick check reads was inferred, not traced.

## Where the files are

**Branches and worktrees.**
- `claude/s2-t0` to `claude/s2-t9` (`.claude/worktrees/s2-t0` to `s2-t9`) are the task branches.
- `claude/s2-int` (`.claude/worktrees/s2-int2`) is the integration.
- `claude/s2-proto-disco`, `-kick2x`, `-phrasefill`, `-fillonly` and `-fillb` (`.claude/worktrees/s2-disco`, `s2-kick2x`, `s2-phrasefill`, `s2-fillb`) are measurement prototypes only. Never merge them. They can be removed once the user is happy, but ask first.

**Scratch** (session bbf41dc4's scratchpad, `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\bbf41dc4-b650-434b-a047-ce3abe26e342\scratchpad\`):
- `s2-plan\` holds the integration rulings, the briefs and their verifier corrections (`briefs\*.json`), the drafts and the plan review.
- `s2-baseline\` holds the score scripts and the 1abd188 baseline files.
- `s2-disco\`, `s2-kick2x\`, `s2-phrasefill\`, `s2-fillb\` and `s2-small\` hold the measurement scripts and raw dumps behind every number in D19 to D30.
- `s2-int2\` and `s2-int-verify\` hold the integration's dumps and logs.

**Untracked repo files** (never staged by this session): the decisions file, the plan and its tasks file, this handoff, and the earlier audit and handoff files listed in `git status`. Stage them by name only when the user asks.

## Rules that still apply

- Run important things by the user before changing anything. Any visible change is a blocking question asked up front.
- When a score moves, say *why* in game terms before asking: what a player can or can't earn, and which side is unreachable. The user asked "why is a score dropping? you need to explain before changing anything" when this was missed.
- Derive once: one owner per rule, and display code reads stored or parsed facts.
- Run work in parallel as workflows. The user said so plainly: "all work must be run in parallel as workflows". Only merges wait on gates.
- Every agent brief carries the status-line rule (`hooks\state\status`) and the session id for commit trailers. Stage by file name, never `git add -A`. Fix hooks that block wrongly, with tests.
- Nothing merges to `main` before the review gate is live and the user says yes.
