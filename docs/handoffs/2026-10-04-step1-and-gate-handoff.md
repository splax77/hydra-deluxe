# Handoff: step 1 finished on its branch, review gate installed (2026-10-04)

This picks up from `2026-10-03-step1-plan-handoff.md`. Session `ade9655b` ran the 22-task step-1 plan in parallel worktrees, plus several fixes the reviews turned up. It also rebuilt the hooks and installed the git-side review gate. The user asked to stop new work here, so everything below is either done, finishing, or waiting on a decision.

Nothing has been merged to `main` and nothing has been pushed. `main` is still `02976c1`.

## Where step 1 stands

All of step 1 lives on `claude/s1-int`, head `a412cb9`. It holds all 22 tasks. It also holds three extra fixes the user approved along the way: the early squeeze-out fix (D18), the extreme-tempo crash fixes (D32), and the flaky GUI test fix. Every proof on that head is green:

| Check | Result |
|---|---|
| `hydra_tests` | 744 passed, 0 failed, 4 skipped |
| `hydra_uitest --all --jobs 4` | 60/60, twice |
| `hydra_replay selfcheck` | 382/382 paths |
| Scores vs `baseline\scores-HEAD.txt` | 0 differences (run with pwsh) |
| Part B | only the 4 variant facts Task 17 names, all oracle OK |
| Part C | identical except the doctest skipped-count line |
| Stale check | records from today's build read Stale in the new build |

The stamps are set once for the whole plan. `kResultsStamp` is "2.1.0", because steps 1 and 2 ship in release 2.1.0 (D33). `kPathFormatStamp` is 7. The app version in `CMakeLists.txt` still says 2.0.0. It changes at release time, not before.

The ledger with every lane's history is `.superpowers/sdd/2026-10-03-step1-engine-facts/progress.md`. The decisions are in `docs/audit/2026-10-03-fix-decisions.md`. Step 1 owns D1 to D18 and D32 to D34. D19 to D31 belong to the step-2 plan, which another session runs in this same repo. Both sessions append to that file, so check the last number before adding one.

## Work still finishing when this was written

Two agents were still running when the user said stop. Both have since finished. Their results sit on their own branches, not on `claude/s1-int`.

**D34, on `claude/s1-d34` (finished, `24666ec`).** A phrase can be squeezed in only once. When the first phrase in an SP end's window was already squeezed in or banked, the next phrase in that window is offered instead. The worktree is `.claude\worktrees\s1-d34`, branched from `2ffc766`. `d6a6936` holds the red tests and three hand-made charts. `24666ec` holds the fix, plus ADR 0014 and CONTEXT.md updates.

The rule lives in one function, `core::offered_phrase` in `src/core/sqout_chord.h`. The graph, the engine and `hydra_replay` all ask it. Each deactivation edge now lists every phrase in its window, and the engine remembers which phrase each window squeezed out. A `hydra_replay --acts` window typed by hand carries no squeeze-in history, so it refuses a squeeze-out on the phrase after one squeezed in. Stored paths and dump files work.

Its proofs: `hydra_tests` passes all 747 test cases. `hydra_replay selfcheck` passes 382/382 and passes on the three new charts. Scores show 0 differences with pwsh. The library run at caps 2 to 4 shows exactly the 7 expected path changes and no score changes. Thrice - Deadbolt cap 4 goes from "1 5 4++" to "1 5 4+", still 283,480. SoundHaven - Triad cap 2 goes from "0+ 0- 0 1++++" to "0+ 0- 0 1+++", still 1,575,170. The next-phrase offer itself never fires on the library, only on the 4,000 BPM test charts, where it can raise a score (`banked_then_next` goes from 2,150 to 2,250). There's no stamp bump, because 2.1.0 already covers step 1. Its library files are in the scratchpad's `s1-d34` folder.

It found one new gap, listed as decision 3 below. Next step: review the branch, then merge it into `claude/s1-int` and rerun the proofs.

**The cheaper ready-time key, on `claude/s1-rk` (finished, `3083a5d`).** Tied paths were folded without checking when each one's SP became ready. So Hydra could keep the path that misses a fill's early window, and drop the one that reaches it. The simple fix costs about 2x library time and about 30x on discography charts. The cheaper key counts how many upcoming fills fall on the far side of each path's ready-time cut-offs. Two paths with equal counts face exactly the same fills, so folding them is safe. The early-fill rule now lives in one place, shared by the activation code and the key.

The measured result, against an untouched `claude/s1-int` build over 19,343 library charts:

| | Cap 2 | Cap 4 (default) |
|---|---|---|
| Best scores higher | 7 | 0 |
| Best scores lower | 0 | 0 |
| Search size | +0.15% | identical |
| Speed | no measurable change | no measurable change |

The 7 gains are Kittie - The Truth +5,900 (522,585 to 528,485, confirmed with `hydra_replay score`), Ne Obliviscaris - Xenoflux +4,300, Xane60 & SoundHaven - Flamingo +1,380, Ne Obliviscaris - And Plague Flowers the Kaleidoscope +1,220, Endless Setlist I +940, Nine Inch Nails - The Perfect Drug +780 and The Faceless - Planetary Duality +420. At cap 2, 24 charts list higher scores lower down their path lists, and none list lower. blink-182's discography stays at about 0.3 s. The 97-chart corpus scores are identical, and `hydra_replay selfcheck` passes 382/382.

One display note: when two paths with slightly different ready times fold together, the second still shows the leader's "Early fill" ms figure. Both are always on the same side of every limit, so only that number can differ. Today's code has the same issue, more widely. The change ships under the same 2.1.0 stamp if it goes out in the same release. Measurement files are in the scratchpad's `rk` folder.

The user said "build the cheaper key, measure, then decide". So the next step is to show these numbers, get a yes, then review it and merge it into `claude/s1-int`. Its new decision number is the next free one in the decisions file.

## Decisions waiting on the user

1. **Adopt the ready-time key?** The numbers are in the section above. The recommendation is yes: it recovers all 7 better best paths at no speed cost.
2. **A gap at extreme tempo, found while closing a test gap.** At 2,000 BPM and up, the 500 ms squeeze window can hold two phrases. When the engine squeezes out the first, it also drops the second phrase's SP-end step. The record then stores an SP end one bar earlier than where SP really ended. The example is `node_before_phrase.chart` at cap 3, path "0-". No library chart can reach it, because the shortest SP bar in the library is 598.7 ms. Fixing it means keeping the second phrase's step, which is an engine change, so it needs a yes. Until then the full bank-order check is skipped on the fast-tempo charts, with a comment in `tests/test_fast_tempo.cpp` saying why.
3. **A tied-path gap at extreme tempo, found by D34.** When the search folds tied paths together, it doesn't track which phrases each one already squeezed in. So at 2,000 BPM and up, a folded path can copy its leader's squeeze of the next phrase, where on its own it would squeeze the first. `early_sqin_twice.chart` shows it, so D34 left that chart out of the "prices the same as alone" check, with a note in the test and in ADR 0014. Normal-tempo charts never reach it. Fixing it means changing how paths are grouped, which could change library listings, so it needs a yes. It sits next to decision 2, and one fix might cover both.
4. **Switch the review rule on.** Covered in the gate section below.
5. **Merge step 1 to `main`.** That needs D34 and the ready-time key decided and merged into `claude/s1-int`, the review rule live, and the user's yes.

## Still to do on step 1

The Task 6 speed check never ran, because other work kept the CPU busy. It compares `bench_head_02976c1.exe` (in `.superpowers/sdd/2026-10-03-step1-engine-facts/`) with a fresh `hydra_bench` built from `claude/s1-int`. The new total must be within 5% of the old one. Run it while nothing else builds, or alternate the two binaries several times and compare medians.

A few small review notes were left for later. None blocks a merge:

- Task 17's lone-pricing corpus test and Task 19's guard overlap a lot. One of them could go.
- Task 20's variant gauge test calls `corpus::song` without `cfg.rules`, unlike its siblings.
- Task 7 asked for an exact-tick test under the 1.1 fill rule, and for `hydra_replay dump` to show the fill list.
- The GUI batch gate runs every batch on one worker, so a strip with several workers isn't tested.
- The search sometimes builds two SP-end points one tick apart. D34 makes that harmless. The twin points themselves may deserve a look.

## The hooks and the review gate

All the hook work lives in `C:\Users\Patrick\.claude\hooks`, which is not a git repo.

**What is live now.** Every command-line gate reads commands through one shared scanner and one shared git-word reader. The rules about what may reach `main` live in one shared file, `lib\derive_once_rules.ps1`. These protections are on:

- A subagent can't rewrite history or force-push.
- A subagent can't commit under another agent's name.
- Nobody in Claude Code can point git at another hooks folder, unset the Claude-session marker, change where origin points, or skip the push check with `--no-verify`.
- The deletion guard lets through any text git can bring back, text the agent wrote itself, and anything in the session's own scratch folder. It only stops someone else's uncommitted text past 200 characters.

All 18 hook test suites pass on the live folder.

**What is installed in the repo.** `hydra-test\.git\hooks` now has `reference-transaction` and `pre-push`. They only act when `CLAUDECODE=1`, so the user's own terminal is unaffected. The trailer rule is on: every new commit from Claude Code needs `Task:`, `Agent:` and `Session:` lines. Commits already on any GitHub branch are exempt, and the hook checks GitHub to confirm. Pushes get the same judgement as local moves of `main`.

**What is not on yet.** The review rule ("`main` may only move to reviewed code") stays off until the gate's own branch, `claude/derive-once-gate` (`3551b5e`), merges and brings the marker file. Before that merge, two entries must be registered in `C:\Users\Patrick\.claude\settings.json`: `derive_once_review_gate.ps1` in the Bash/PowerShell dispatcher's gate list, and `derive_once_waiver.ps1` as a UserPromptSubmit hook. Then a fresh reviewer agent reviews `claude/derive-once-gate` and records its verdict, and that branch merges into `main` locally. That merge is the gate's first live test. After it come the gate plan's Task 5 dry-run probe, a sync of the plan text, and a final review. The user was asked about this and chose to stop for now, so it needs a fresh yes.

**Rollback, if needed.** Delete the two files in `hydra-test\.git\hooks` by hand; agents are blocked from doing it. Then copy the files in the scratchpad's `gate-install\live-backup` folder back into the hooks folder, and delete `lib\derive_once_rules.ps1`. The file list is in `gate-install\swap-log.txt`. The scratchpad is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\ade9655b-b3ed-451c-8dfd-0a9077ee2da1\scratchpad`. It's temporary, so copy anything important out of it before it's cleaned up.

The gate's ledger is `.superpowers/sdd/2026-10-03-derive-once-review-gate/progress.md`. Its plan, `docs/superpowers/plans/2026-10-03-derive-once-review-gate.md`, has a Known limits section, including what was added at install time.

## Things to know before picking this up

- The other session (step 2, parser rules) works in this same checkout and its worktrees. Stage files by name, and don't touch its `s2-*` worktrees.
- Run `scores.ps1` with `pwsh`, never Windows PowerShell 5. The older shell turns the full-width slash in "Sugar／Tzu" into a plain one and shows a false difference.
- Never open a real `hydra.db` with a build from these branches until a release ships the new stamps. Tests and checks use scratch databases only.
- Worktree paths under `testdata` can exceed Windows' path limit. Keep new worktrees short.
- The memory notes `derive-once-gate-2026-10` and `derivation-audit-2026-10-03` describe this state.
