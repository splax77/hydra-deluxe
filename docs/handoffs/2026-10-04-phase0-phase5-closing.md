# Phase 0 and phase 5: closing note (2026-10-04)

Both phases are merged to main and nothing is pushed. C2 (one owner for the shell path limit, `fits_shell`) went in as c86ab6a. M0 (the derive-once precheck, `tools/new_worktree.ps1` and `tools/build_slot.ps1`) went in as 9d67d20. P0-2's briefs landed as 876b507, and P0-3's hooks live outside the repo. The plan records all of this at the top of `docs/superpowers/plans/2026-10-04-phases-3-5.md`.

A few small things are left open. None blocks anything.

The precheck finds 20 or 21 of the 23 audit copies, not all of them. The two it misses need wider rows in `tests/test_single_owner.cpp`, such as a tied-root row that matches more than `collect_tied`. `tasks/P0-1.md` has the details. Widening those rows is a scan change, so it needs its own task and review.

The precheck's self-test plants a C++ example for check 4 but no Python one, so the Python half of that check (os.walk, glob, listdir over src) has no fixture. Round 10's reviewer noted it.

The precheck's header still quotes a dated count: the whole-tree run listed 80 items at 23a5d97. It lists 111 today. The sentence names its commit, so it is not wrong, but it reads as current.

I messaged the phase 3, 6 and 7 sessions to point their preambles at `tools\build_slot.ps1` instead of the scratch `cold_build.ps1`; the files are theirs to change. Phase 3 has done it, using the main checkout's copy because its worktrees predate 9d67d20. Phases 6 and 7 had not answered when this note was written.
