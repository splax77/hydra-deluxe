# Handoff: phase 7 after wave 1, with M7-2 in review (2026-10-04)

Written by session 6c7ec44b ("Phase 7 handoff execution"), which ran `docs/handoffs/2026-10-04-phase7-handoff.md`. The user said "stop everything now", so the M7-2 workflow was stopped mid-run, with M7-2c's integrator still timing a load. Nothing from this session is still running. The only Hydra.exe left running is the installed app in Program Files, which is the user's.

## Where things stand, in one paragraph

Wave 1 is merged to main as 6c65208 (M7-1). Wave 2's five tasks are built and joined on three branches, one per merge, because D59 item 4 splits a change over about 500 lines. Two of the three have a first review with findings, and the third (M7-2c) is joined and prechecked but was stopped before its timing run and its review. Wave 3's four briefs are written and decided (D62) but not launched. Wave 4 (ER) has no brief yet. Nothing from phase 7 is pushed.

## Decisions made in this session

All of these are at the end of `docs/audit/2026-10-03-fix-decisions.md`. The user took the recommended answer to every question.
- **D51 addendum:** the wave 1 launch answers. ST1 gets schema 4 with `rules_fp` in the results key. A `depth_mode` other than 1 reads as scores. The SP cap keeps reading 0 and `auto` as 4. E2's how-to line names every note that crosses.
- **D55:** five wave 1 follow-ups. They cover both-ends how-to wording, `#` kept in `chartfolder` and `dm_last_user`, reindex leaving rows from other rules untouched, the `rules_fp` column having no default, and over-limit paths below the best score staying dropped.
- **D58:** six wave 2 display and record answers (E3, ST2, PV).
- **D60:** the user saw E3's all-0 counts and kept them. Seven test charts lose their all-0 path, and an optional fill with spare time gets no badge.
- **D62:** four wave 3 display answers (RP's no-paths row and empty cell, duplicates count once, "A batch is running." fades). It also says the Path limit tooltip changes to D51 call 5's words at the M7-3 join.

Two rules changed under this session, both from other sessions with the user's yes. **D59** governs review loops: few-line findings are fixed by the orchestrator, slow proofs run once per round, and changes over about 500 lines are split. **D61** governs reviews: a review is ONE exchange. The reviewer sends its findings to the author by SendMessage, the author fixes them once, the reviewer checks and fixes leftovers, and then it submits CLEAN on the new tip. There are no second reviews. Read `docs/agents/derive-once-review.md`, section "One exchange, then you finish it", before running any review.

## M7-2: what to do next

Three join branches, each made from main after M_D (57dcf23) and checked against the corpus:

| Merge | Join branch, tip | Holds | Review so far |
|---|---|---|---|
| M7-2a | `claude/p7-w2a` f726f97 | E3 (0cbe9bd) plus the pather.h comment fix | FINDINGS, 3: `scratchpad/m72/review-w2a-r1.md` |
| M7-2b | `claude/p7-w2b` 6520232 | ST2 (696aa61) plus a scan-baseline join fix | FINDINGS, 5: `scratchpad/m72/review-w2b-r1.md` |
| M7-2c | `claude/p7-w2c` b47d4ba | PV (96d2c31), SE2 (156d162), TM (98e1945), plus the Preview's "ask the scan fingerprint before hashing" join fix (9b64494) and step-1 fixes | Not reviewed. The integrator was stopped while waiting for a quiet CPU to time the 1.09 GB .sng load. Its precheck found 0 items (`m72precheck-w2c.txt` may be missing, so rerun it) |

The scratchpad is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6c7ec44b-dbe0-4806-91ce-84c598754004\scratchpad\`. Its `m72\` folder holds the three implementer report files (`reports-a/b/c.md`), the prechecks, the reviews and the corpus score files.

Those two reviews ran before D61 reached them, so they submitted FINDINGS the old way. Under D61, don't dispatch a second review round. Send each review's findings to an author agent and have the same reviewer check and finish. The reviewer and author agents from this session are workflow agents and may not be reachable by SendMessage. If they aren't, start one fresh reviewer per merge with the D61 brief. Give it the existing review file as its findings, the join worktree (`.claude\worktrees\p7-w2a`, `-w2b`, `-w2c`), and an author: one fresh Opus fixer per merge, with `docs/agents/fix-round.md`.

The findings, in short:
- **M7-2a:**
  1. A rule is written twice. The Paths tab's "a 1-bar cap can never activate" is `== 1` in path_view, while the engine's activation minimum is a bare `2` in three places. The review proposes `kSpActivationBars = 2` in timing.h, with the display test `< kSpActivationBars`. Phase 6's J3-2 already plans that constant; adding it now is fine, and J3-2 then takes the engine's three literals.
  2. The `Activation::needs_timing` comment names readers that don't call it. A comment-only fix.
  3. The new scan rows are too narrow. The review gives wider patterns.
- **M7-2b:**
  1. The `engine_mode` stamp key is read three times. `stamped_fill_rule` should call `engine_mode()`.
  2. The record_store.h comment restates the "ch10"/"ch11" text and is now false about who reads it.
  3. **This one needs the user.** `upsert_song` now takes a saved chart's names from the library table for every listed chart, not only duplicates. So `hydra_batch` run alone after a song.ini fix keeps the old GUI-scan names until the next GUI rescan, against the 2026-09-26 decision "a fixed song.ini reaches the reports on the next analysis". Recommend narrowing the override to charts with more than one copy, so D51 call 10 still holds and fixed names flow as before. Ask before fixing.
  4. A scan row is duplicated and loose. Widen the existing fill-rule row to the bare `"ch1[01]"` literal.
  5. A test copies set-up. Share one small ScanItem helper.
- **M7-2c:** not reviewed yet. First, time the 1.09 GB "Endless Setlist III" .sng Preview load: warm, best of three, on a quiet CPU, at main and at b47d4ba. PV measured about 1.3 s before its change and about 3.5 s with the full re-hash. The join fix 9b64494 should bring it back to about 1.3 s when the scan fingerprint matches. Then run one D61 review: the reviewer is a fresh Sonnet agent, and the author is a fresh Opus fixer briefed with `reports-c.md` and the commits 9b64494 and b47d4ba. Step 1 also added two owners: `drawn_path`, shared by the load job and the controller, and `audio_ms_of_chart_ms` for the chart-to-audio sync.

What each merge already proved:
- All 97 corpus charts are byte-identical to M7-1's tip (`m71\scores-9d6fefd.txt`) on all three joins.
- Each join's named filters pass.
- M7-2c also passed `hydra_uitest settings-and-reports`.
- E3's stored changes are known and approved (D60): the all-0 count goes 68 to 61 on the corpus, and 15 best paths store no hardest timing.
- PV's big .sng load is not yet measured after the join fix (see M7-2c above).

Merge order and steps, once a merge's reviewer submits CLEAN:
- Merge main into the join first if main has moved.
- The main session runs the full suite once, then `git merge --no-ff` into main, staging by name.
- M7-2c first if it is ready: phase 6's J2-4 sits on SE2's 156d162 and waits for M7-2 to land.
- After each merge, message "Phase 6 handoff continuation" with the hash. It asked for the M7-2 hash, and for any SE2 fix that touches `tests/test_app_state.cpp`.
- Mark the wave 2 findings in the triage JSON (E3: 22, 23, 180, 245, 243, 324, 261, plus the E3 halves of 56, 130 and 139. ST2: 62, 63, 345, 54, 88's flag. PV: 73, 126, 72, 139, 130, 109. SE2: 31, 311, 322, 139. TM: 58).

## Wave 3: ready to launch

Briefs: `docs/superpowers/plans/tasks/p7-rp.md`, `p7-doc.md`, `p7-lb1.md` and `p7-lb2.md`, all decided by D62.
- **Order:** RP first. LB1 forks from RP's commit, and LB2 from LB1's. DOC runs alongside.
- **Base:** main after M7-2 and phase 6's M6-J2. Don't wait for the merges themselves (memory "parallel-by-default"). Fork from phase 6's J2 join once it holds J2-1, J2-2 and J2-4, with the M7-2 joins merged in. I asked phase 6 for its J2 join branch name; its reply may be waiting. RP's files are J2-2's this wave, LB1's include J2-1's `config.*` and J2-4's library work, and LB2's are J2-4's.
- **Phase 6 has stopped too**, and it has no J2 join branch. Its handoff is `docs/handoffs/2026-10-04-phase6-execution-handoff.md` (main 0a1e06e). Its task tips all sit on p3-d2 8b5a99d, without main merged in:
  - J2-1 is `claude/p6-j2-1` 3f12d19 (CLEAN).
  - J2-2 is `claude/p6-j2-2` 3e6dcab. It has one open user question: should the "+N over" leaderboard text stay for off-speed rows? The recommended answer is yes.
  - J2-4 is `claude/p6-j2-4` 8f7bc54, on SE2's 156d162.
  - J2-7 (16916e7) and J2-8 (5b7b358) are CLEAN.
  - J2-3 (beac57b) and J2-5 (f8c4c08) are not yet checked.
  - J2-6 has no commit.

  RP can fork from J2-2's tip, apart from that one dm_report line. LB1 forks from J2-1 plus J2-4.
- **At the M7-3 join**, the main session changes the Path limit tooltip in `src/ui/settings_bar.cpp` to "hardest squeeze or required early fill" (D51 call 5, D62). M7-3 also runs `hydra_uitest` once. LB2 names the real uitest scripts, because no script is called `library` or `batch`.

## Wave 4 (ER)

No brief yet. It goes after M7-3 and after every phase 6 fold has landed (M6-J4).

## Things learned this session (also in memory)

- **Measure stored-result changes across corpus × settings before merging.** E1 looked right on its own pins. The cross-settings measurement (97 charts × 12 settings, a temporary doctest case at `scratchpad\m71\measure_case.cpp`) found it brought back over-limit paths below the top, which became D55 item 5.
- **A "wait for another phase's merge" is not a wait.** Fork from that phase's join branch the moment its code exists; only our merge waits. Wave 2 forked from phase 3's `claude/p3-d2` before M_D merged.
- **The precheck and build slots are on main now:** `tools/derive_once_precheck.ps1` and `tools/build_slot.ps1`. The phase 7 preamble points at the slot tool.

## Loose ends

- Merged worktrees still on disk with their build folders: `.claude\worktrees\p7-e1`, `-e2`, `-st1`, `-se1`, `-t1`, `-au1`, `-ps1`, `-pr1`, `-pr2` and `-w1`. Branches for phase 6's forks (t1, au1, se2) must stay; the worktrees can go.
- From the earlier handoff, still open: three empty `.claude\worktrees` folders, unmerged prototype branches (ask first), and the hook backup files.
- Nothing is pushed. Ask the user before pushing.
