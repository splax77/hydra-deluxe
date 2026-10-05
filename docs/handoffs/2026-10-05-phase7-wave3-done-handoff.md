# Handoff: phase 7 after wave 3 (2026-10-05)

Written by session 6fc5f64c ("Phase 7 Wave 2 handoff"), which continued `docs/handoffs/2026-10-04-phase7-wave2-handoff.md`. Nothing from this session is still running. Nothing is pushed.

## Where things stand

Waves 1 to 3 of phase 7 are on main. Only wave 4 (ER, typed errors) is left. The plan says ER runs last and alone, after every phase 6 fold has landed (M6-J3 and M6-J4), because it touches throw sites in nearly every folder. Phase 6 is still running those waves in its own session ("Phase 6 execution handoff"). So phase 7 now waits on phase 6, and ER has no brief yet.

## What merged in this session

| Merge | Main commit | What it is |
|---|---|---|
| M7-2a | 199fec4 | E3: one owner for "does this need timing", `kSpActivationBars`, the depth-mode int |
| M7-2b | 6e980ed | ST2: per-difficulty song lengths, duplicate copies named once (D63), batch dedupe, stamps |
| M7-2c | c8c4196 | PV, SE2, TM: Preview fixes, settings boxes clamp through `Settings::clamp`, the meter-change tick |
| M7-3a | 3ecc9b8 | RP: report pages and the two report CLIs (D65) |
| M7-3b | f4e352b | DOC: guide, CONTEXT.md, ADRs 0014 and 0019; the Path limit tooltip (D62) and the all-0 hint (D66) |
| M7-3c | 2c1c41e | LB1: batch counts from `run_batch`, `plan_batch`, a decimal hit window (D67) |
| M7-3d | 4868f44 | LB2: library screen, toolbar and app state read one owner each |

Every merge had a CLEAN derive-once review under D61 (one exchange), and the full suite passed on the merged tree each time. The last one ran 1054/1054. `hydra_uitest --all` passed 63/63 on LB2's tip on the first run. The triage JSON marks wave 2 (a1a61dc) and wave 3 (80e2b4f).

The 1.09 GB Endless Setlist III .sng Preview load was timed. It took 0.80 s before M7-2c and 0.76 s after, with the fingerprint shortcut taking effect. With the shortcut forced off it takes about 2.3 s. The CPU was rarely quiet, so trust the comparison more than the exact figures.

## Decisions made in this session

All are at the end of `docs/audit/2026-10-03-fix-decisions.md`. The user took the recommended answer each time.
- **D63:** only a chart with more than one copy in the library takes its names from the library table. Every other chart keeps its newest analysis's names.
- **D65:** hydra_fillcompare says nothing for an unstamped database, as ADR 0010 says. RP's brief question 4 is withdrawn.
- **D66:** the all-0 hint reads "It needs no timing."
- **D67:** a hit window with junk after the number (`90abc`) reads the default.
Phase 6 took D64 (the "+N over" text stays for off-speed rows).

## How the merges were run, and what to reuse

Wave 3 forked from phase 6's branch `claude/p6-j2-wave3`, not from main, because phase 6's J2 tasks were not merged yet. That made every merge to main conflict in `tests/test_single_owner.cpp`, where every task appends rows. This session handled it one way every time. It took main's copy of the file and re-added the task's own rows with Edit. Then it checked that the changed lines of `git diff main <tip>` equal the reviewed diff, ignoring hunk offsets. Then the same reviewer re-signed the merge tip. Phase 6 asked us to hold merges while its J2-4 landed, and that saved one round of this.

## Loose ends

- **Merged worktrees still on disk:** p7-w2a, -w2b, -w2c, -rp, -doc, -lb1, -lb2, plus the wave 1 ones the earlier handoff listed. Their branches are merged, so the worktrees can go. Branches that phase 6 forks from (p7-t1, -au1, -se2) must stay.
- **Notes for phase 6:**
  - J3-2 folds the engine's three bare `sp < 2` literals onto `kSpActivationBars` and adds scan row 3 from the M7-2a review.
  - The two-argument `RecordStore::set_song_length` still has one caller, `save_analysis`, so J4-2 can't simply delete it.
- **Notes the reviewers left, not blocking:**
  - hydra_fillcompare and hydra_batch read `engine_mode()` only to print the raw stamp.
  - The guide's "its figure shows in orange" holds at the default 10 ms Path limit.
  - A `model.h` comment still says "hardest squeeze or E0 fill".
- **The scratchpad** holds about 574 MB of hydra.db copies from the timing run (session 6fc5f64c's `scratchpad\db-before` and `db-after`). They can be deleted.

## Next

1. Wait for phase 6's M6-J3 and M6-J4 merges.
2. Write the ER brief (finding 193) against main at that point, then run ER alone as M7-4.
