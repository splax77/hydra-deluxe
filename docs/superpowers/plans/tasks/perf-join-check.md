Read docs/superpowers/plans/tasks/_perf-preamble.md first; it holds the rules.

# Task JC: the identical-results check on the joined build

Task id: JC. You write no code and commit nothing. Your prompt gives the joined branch's worktree and its signed-off tip, and the baseline folder. You run the plan's recipe once on the joined build against the baseline, the step the plan's "Merge M1" paragraph names. The baseline was built from H1's tip, which has bc58282's engine code (its README says so), so it stands for bc58282.

## Steps

1. In the joined worktree, check `git rev-parse HEAD` equals the tip you were given; if not, stop and report. Build `hydra_batch` and `hydra_bench` there by `-Target` (the integrator already made the cold build). Copy both exes, and the baseline's two inis (the top one and `hard\`), into `<your scratchpad>\joined\` and `<your scratchpad>\joined\hard\`.
2. Through the lock, one at a time, run every baseline output's twin from the joined exes, the same way the baseline README says each was made: the fresh-database `hydra_batch` run; `--engine` with `--out`; `--parse` at both settings; `--scan` fresh then rescan on the same database.
3. Compare: `py tools\compare_db.py` for `fresh.db`, `scan_first.db` and `scan_rescan.db` against the baseline's; the printed `--engine` and both `--parse` hashes against the README's; and the per-chart files byte for byte (`fc.exe /b` or a Python compare).
4. The real database: two fresh copies of the baseline's `real.db`, then `hydra_batch --db <copy> --redo` with the baseline exe on one and the joined exe on the other, through the lock, and `compare_db.py` the two.

Every line must read 0 differ and every hash must match. Any difference is a finding: stop, report the table, the first differing keys and which run, and do not dig further.

## Owned files

Only your scratchpad.

Owned-file check: this task writes nothing in the repo; every output lives in the scratchpad.

## Preflight

Command: `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test diff --stat bc58282 7403685 -- src`, run by the orchestrator on 2026-10-06.
Output: empty. The two commits after bc58282 on main are docs only, so a baseline built on them has bc58282's engine.

## Return

`complete`, `all_identical` (true or false), `report` (every compare line and hash pair), `questions`, `handoff`.
