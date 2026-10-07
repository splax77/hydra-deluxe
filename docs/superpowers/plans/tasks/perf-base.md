Read docs/superpowers/plans/tasks/_perf-preamble.md first; it holds the rules.

# Task BASE: build the baseline exe set and its outputs, once

Task id: BASE. You write no code and commit nothing. You build the baseline that every wave-1 task compares against, so five agents don't each cold-build one. The plan's "Proving identical results" section is the recipe; you run its baseline half.

Your prompt gives the base commit (H1's tip) and its short hash. Your folder is `C:\Users\Patrick\.claude\hooks\state\bench\baseline-<short hash>\` (the preamble's "baseline folder"). Write `README.md` there as you go, listing every file, the exact command that made it, and its hash or row counts. Create `READY` (empty) only when every step below is done.

## Steps

1. **Check the base is bc58282's engine.** `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test diff --stat bc58282 <base> -- src` must print nothing. If it does, stop and report: then the plan's bc58282 baseline needs its own build. If it is empty, write in the README that this set is also the bc58282 baseline for the join check and T1, because H1 changed no source under `src/`.
2. **Build.** `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test worktree add --detach C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\perf-base <base>`. Then `pwsh -NoProfile -File <that worktree>\tools\build_slot.ps1 -Repo <that worktree> -Target hydra_batch`, then `.\build_cpp.ps1 -Target hydra_bench` there. Copy `hydra_batch.exe` and `hydra_bench.exe` (and any DLL beside them in `build-cpp\Release\` that they need) to the folder.
3. **Settings.** Copy `C:\Program Files\Hydra\hydra_settings.ini` beside the exes with its `dm_last_user` line removed. Read it against the key table in `src/app/config.cpp` and confirm in the README, key by key, that it says Expert, Pro Drums on, 2x Bass on, depth 4 scores, Path limit 10 ms, SP cap 4 and the folder `C:\Clone Hero`. Never guess a key name. If any value differs, stop and report. Then make a `hard\` subfolder holding copies of both exes and an ini that differs only in difficulty Hard, Pro Drums off and 2x Bass off.
4. **Real database.** Take the pristine copy with SQLite's backup API (Hydra is running; this is safe alongside it): `py -c "import sqlite3; s = sqlite3.connect(r'file:C:/Program Files/Hydra/hydra.db?mode=ro', uri=True); d = sqlite3.connect(r'<folder>\real.db'); s.backup(d); d.close(); s.close()"`. Record its size and its `results` row count.
5. **Baseline outputs**, each run through the lock (`tools\bench_run.ps1` in your worktree, label `base-<what>`), one at a time:
   - `hydra_batch.exe --db <folder>\fresh.db` with no folder argument (the fresh-database run).
   - `hydra_bench.exe --engine "C:\Clone Hero" --out <folder>\engine_rows.txt`; save the printed hash.
   - `hydra_bench.exe --parse "C:\Clone Hero" --out <folder>\parse_expert.tsv`, and the same from `hard\` into `parse_hard.tsv`; save both hashes and failure counts.
   - `hydra_bench.exe --scan "C:\Clone Hero" --db <folder>\scan.db`, copy it to `scan_first.db`, run the same command again on `scan.db` (the rescan) and keep that as `scan_rescan.db`.
   - Then `py <worktree>\tools\compare_db.py <folder>\fresh.db <folder>\fresh.db` once, to show the tool runs, and put the per-table row counts in the README.
6. Remove your worktree: `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test worktree remove --force <that worktree>`. Then create `READY`.

## Owned files

Only the baseline folder and your scratchpad. Nothing in the repo.

Owned-file check: every deliverable is a file in the baseline folder outside the repo; the detached worktree is yours and is removed at the end.

## Preflight

Command: `tasklist /FI "IMAGENAME eq Hydra.exe"` and `Test-Path "C:\Program Files\Hydra\hydra_settings.ini"`, run by the orchestrator on 2026-10-06.
Output: Hydra.exe is running (pid 36184), so the real database must be copied through the backup API, never by file copy; the ini path is the one the plan names.

## Return

`complete`, `folder`, `engine_hash`, `parse_expert_hash`, `parse_hard_hash`, `report` (the README's content in short), `questions`, `handoff`.
