# Handoff: phase 6 execution, waves J1 and J2 (2026-10-04)

Written by session 4f777343 when the user said "stop all work now and create a handoff". Every agent, workflow and monitor this session started has been stopped. Nothing is running for phase 6. Nothing has been pushed.

Phase 6 is the audit's step 6, the duplicate folds. Wherever one fact is worked out in two places, it gets one owner and the copies call it. The plan is `docs/superpowers/plans/2026-10-04-phase-6.md`. The running ledger is `docs/superpowers/plans/2026-10-04-phase-6-progress.md`; read its "Notes for later waves" before writing or launching any J3 or J4 task. The user answered the plan's four questions "use recommended answers to everything", recorded as D53 (the questions) and D54 (the code-only calls).

## The user's rules for this work

These seven rules come from the user and apply to every agent.

1. Agents never run the full test suite. The main session runs it once when merging; agents run only the tests their change touches.
2. Planning agents run on Fable, code executors on Opus, code reviewers on Sonnet.
3. Anything that changes a display in the app or a stored record goes to the user first. Code-only changes go ahead on the recommendation.
4. Agents never stall or go quiet. They fail loudly and report the exact error.
5. A reviewer must clear a derive-once audit before any change merges.
6. Agents run in parallel workflows, and the main session merges. Waves are fine. The user's last instruction before the stop was "do the remaining work in parallel. you can merge when it's done."
7. Agents write a status line every 10 tool calls and every 5 minutes. The soft cap is 100 tool calls, at which point an agent wraps up and hands the rest to a fresh agent. The hard cap is 150.

The phase 6 preamble, `docs/superpowers/plans/tasks/_phase6-preamble.md`, carries all of these into every brief. First builds go through `tools\build_slot.ps1` on main.

**The review flow changed while this session ran (D61, commit 03bdcfb, a user decision).** A derive-once review is now one exchange, not rounds. The reviewer reviews once, sends its findings by SendMessage to the agent that wrote the change, and ends its turn. That author fixes them once, following `docs/agents/fix-round.md`, and replies. The reviewer then checks the fix, fixes anything left itself, and submits CLEAN on the new tip. Give the reviewer the author's agent id and the worktree. Never dispatch a second review. See "One exchange, then you finish it" in `docs/agents/derive-once-review.md`. Everything this session merged used the old rounds flow; everything from here on uses the new one.

## What is on main

Three phase 6 merges and the decisions are on main.

- **M6-J1a (7a4e59e)** holds J1-2 (app shell numbers and GUI harness plumbing), J1-3 (track state owners), J1-5 (chart files, containers and path helpers) and J1-6 (the scan reads the shared owners). It also holds a folder-and-name join owner, `join_folder` in winstr, which came out of the review.
- **M6-J1b (8fe0356)** holds J1-1: the replay reads the Star Power edge owner, and every hand-built phrase end goes through one fixture helper.
- **D53 and D54** are recorded (b1d596e). Ten already-fixed findings are marked fixed in the triage. Finding 153 gets marked when phase 3's O1 is confirmed on main; M_D is now merged, so check it.

Each merge passed the full suite once, the GUI suite, and a byte-identical corpus score check on all 97 charts.

## M6-J1c: J1-4, ready except one line

J1-4 (audio owners and test oracles) is on branch `claude/p6-j1c` at 0f8c70c, in worktree `.claude\worktrees\p6-j1c`. It was ported onto phase 3's `frames.h`, because phase 3's FX-P added the same frame-conversion header independently. Both sessions agreed that phase 3's `frames.h` and `player.cpp` win.

Its round 1 review found five things. A sweep round fixed four of them. The fifth, `fixture_bytes` in `test_stem_reader.cpp`, went into J2-6's brief. The round 2 review (key 0f8c70c) found one more line. In `tests/srb_util.h` around line 72, `make_srb` still writes a little-endian u32 by hand. It should call `testbytes::put_u32(out, 17)` from the new `tests/bytes_util.h`. The written bytes are the same either way.

On 0f8c70c the full suite passed 991 of 991 and the GUI suite 63 of 63. Corpus scores were byte-identical to main at 3210839, which includes M_D.

Next for J1c:

1. Merge main into `claude/p6-j1c`, because main has moved past 3210839 (it was 4472d59 at this handoff).
2. Fix the one `srb_util.h` line.
3. Run one review under D61 (one exchange).
4. Rerun the full suite once and the score check.
5. Merge.

The review files are `review-M6-J1c.md` and `review-M6-J1c-r2.md` in this session's scratchpad (path at the end).

## Wave J2: where each task stands

Phase 7 launched its wave 2 from phase 3's join branch, `claude/p3-d2` at 8b5a99d, without waiting for M_D, and phase 6 did the same. That commit holds M7-1, M6-J1a, J1-1 and M0. The phase 3 merge M_D has since landed on main (57dcf23). Its final fix round changed two things J2 must take in when main is merged into the J2 branches. First, `tests/display_fixtures.h` now reads the ms limit from the key. That fixes three `test_fill_report.cpp` cases that throw on 8b5a99d: an answer checker saw them fail and correctly said J2-2 did not cause it. Second, the Dynamics tab's "Dynamic notes" line now calls `dynamics_share`, and J2-5 also changes that line. It's a one-line conflict.

The J2 workflow was wf_82ef1b6e-5e5, and J2-4's was wf_df11b2ec-bcb. Both journals are under this session's `subagents\workflows\` folder, with each agent's full report as a "result" line.

**J2-1 (settings file, text helpers and build identity)** is at 3f12d19 on `claude/p6-j2-1`. Its answer check came back CLEAN after one fix. The fix turned a file-wide scan exemption for `tools/replay.cpp` into a known copy of its line 554, which picks the depth word from `DepthMode`. That known copy needs an owner: give it to J4-1, which owns `tools/replay.cpp` for the depth copy. J2-1 also ran the installer build, and the shipped exe carries both product strings.

**J2-2 (report pages and their fixtures)** is at 3e6dcab on `claude/p6-j2-2`, and it is blocked on a user question. On the leaderboard page, a score at a different scroll speed that beats the optimal currently reads "+N over". The fold would key that text on the row's status, which for any off-speed row is just "other speed". That would change what such a row shows, so it needs the user's answer. The recommended answer is to keep "+N over": `collect_dm_rows` carries an "above optimal" field into the payload, and the script reads that. Once answered, change `dm_report.cpp` line 104 and delete its known-copy row. Then extend "build_dm_html colours the delta from the status" to pin that `r.delta < 0 ?` appears nowhere in the script. Everything else in J2-2 passed its checks.

**J2-3 (song parser and its tests)** is at beac57b on `claude/p6-j2-3`. The implementer committed just before the stop and reported green runs (song 37, midi 12, s2 9, sng 12, srb 10, cli 12, single-owner 7). Its answer check never ran, so run it.

**J2-4 (app state, jobs and the toolbar)** is at 8f7bc54 on `claude/p6-j2-4`. It forks from phase 7's SE2 commit 156d162, at phase 7's request, because SE2 also edits `test_app_state.cpp`. The only open item is scope. The task changed one line in `tests/ui/uitest_library.cpp`, which its brief did not list. Deleting `library_total` forced that edit, and no J2 task owns the file. Accept it, and record that J4-3, which owns that file in J4, starts from it. Because J2-4 sits on SE2, it cannot reach main before phase 7's M7-2.

**J2-5 (library query, Dynamics blob and the Preview source)** is at f8c4c08 on `claude/p6-j2-5`. It committed just before the stop, its filters green, with 6 scan rows added and 10 known copies removed. Its answer check never ran. Expect the "Dynamic notes" conflict above when main is merged.

**J2-6 (tool text and the stem readers)** has no commit; `claude/p6-j2-6` is still at J2-1's 3f12d19. Merging the J1-4 port (2477c80) conflicted in `tests/test_single_owner.cpp`, where each side had appended its own rows. The implementer stopped as told. Restart it with the instruction to keep both sides of both blocks, which is mechanical. J2-6 could also fork from `claude/p6-j1c` once that branch takes J2-1's commit. Its brief already carries two folds from the reviews: the eight max-difference loops, and `fixture_bytes` in `test_stem_reader.cpp`.

**J2-7 (one note value in the scorer, and the stars)** is at 16916e7 on `claude/p6-j2-7`, with a CLEAN answer check. It folds the hottest function in analysis, so at the merge, check that corpus scores are identical and that analysis time is within noise.

**J2-8 (render callers and the Onyx numbers)** is at 5b7b358 on `claude/p6-j2-8`, with a CLEAN answer check. It is where D53 item 3 lands: each taken fill lights its own lane colour.

**Next for J2:**

1. Answer or ask the J2-2 question.
2. Run the answer checks for J2-3 and J2-5.
3. Restart J2-6.
4. Join everything on a `claude/p6-j2` branch from main and merge main in.
5. Run one D61 review, the full suite once, and the score check, then merge.

J2-4 makes M6-J2 wait for M7-2. Alternatively, merge J2-4 separately after M7-2, the way J1-1 was handled.

## The other sessions

Phase 7 ("Phase 7 handoff execution") asked, just before the stop, for the name of the J2 join branch. It wants to fork its wave 3 tasks RP and LB1 from J2-1, J2-2 and J2-4's code instead of waiting for M6-J2. That join branch does not exist yet, and nobody has replied. Phase 7 suggested forking RP from J2-2's tip and LB1 from J2-1 plus J2-4. The tips above are what it would use; J2-2's open question is the catch. Phase 7's wave 3 files (D62) are these: RP owns the report pages and `cli/report.cpp`, LB1 owns `analysis.*`, `library_jobs.*`, `cli/batch.cpp` and `config.*`, LB2 owns `app_state.*` and the `library_*` UI files, and DOC owns UserGuide, CONTEXT and two ADRs.

Phase 7 has also stopped. Its handoff is `docs/handoffs/2026-10-04-phase7-wave2-handoff.md` (4472d59). M7-2 is joined but stopped mid-review, so J2-4 and every J3 task still wait on it. Read that handoff before forking anything from a phase 7 branch.

Phase 3 ("Phase 3 continuation") is finished with M_D. Phase 0 and phase 5 ("Phase 5 continuation") merged M0 (build slots, precheck) and D61.

## J3 and J4 briefs

Three briefs were written, but their writers were stopped before their final check: `p6-j3-1.md`, `p6-j4-1.md` and `p6-j4-2.md` in `docs/superpowers/plans/tasks/`. Each has every section. Re-verify each one against main before launching. The other eleven J3 and J4 briefs are not written.

While reading, the writers found that several plan items are already done:

- Phases 3 and 7 already folded findings 158, 159, 191, 182 and 347, which J3-4 and J3-5 would otherwise do.
- Phase 7's E3 closed the engine halves of 151 and 152.
- `fill_rule_for` is already on main.
- `hydra_replay --depth` is already optional.
- Phase 7's PV takes J3-5's transport gain-floor line.
- `app_state.cpp` line 73 collides with J4-3.

The calendar has moved too. J3 waits for M7-2, and J4 waits for M7-3.

## Worktrees and scratch

Finished phase 6 worktrees are still on disk with their build folders: p6-j1-2, j1-3, j1-5, j1-6, j1-fixprod, j1-fixtest, j1, j1b, p6-base. The user has not yet said yes to removing them. Live ones for the next session: p6-j1c (plus j1c-fixprod and j1c-fixtest, already merged into it), p6-j1-4c, and p6-j2-1 to p6-j2-8.

This session's scratchpad is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\4f777343-4b5a-4c3a-a586-89b5f49d24d5\scratchpad`. It holds the review files, the precheck outputs and the score lists. The newest baseline is `scores-main-3210839.txt`, from main with M_D. Scores are written with phase 3's `scores.ps1` (`C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\06ef47ec-7105-4611-b41f-4db00557e4a1\scratchpad\p3\scores.ps1 -Out <file> -Repo <worktree> -Work <fresh folder>`). Give it a fresh `-Work` folder for each build.

## Loose ends (not phase 6)

The earlier handoffs' loose ends still stand. Ask the user before pushing anything. The `s2-proto-*` and `s1-flake` branches need the user's yes before deletion. The `.bak-d46` hook backups are still there, and `blink-both.json` stays untracked on purpose.
