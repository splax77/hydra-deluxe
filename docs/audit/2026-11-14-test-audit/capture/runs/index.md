# The capture runs taken for the clean records (session 63c70014, 2026-11-15)

The user asked to capture real runs only for the Investigate records that hold no fault under their test rulings ("capture the real ones then stop and create handoff"). `../CLEAN.md` lists those records, 397 of them, and names the run each needs. The commands come from `../RUNS.md`. The main session ran every run below on `testdata/input`, the `testdata/store` files and the real reflog. Nothing touched the user's own library or real database. No record has been re-judged against these outputs yet.

The tools were built from main. `hydra_replay.exe` and `hydra_uitest.exe` were built after the last code commit. Their build stamp is "Nov 15 2026 11:27:58". `hydra_batch.exe` was rebuilt for R5 at 16:22 with a build pass, because `src/app` had changed since it was last built.

## What ran

| Run | What | Output | Result |
|---|---|---|---|
| R1 | `hydra_replay dump`, `schedule` and `score` for all 116 corpus charts, `--cap 4 --ms 10 --depth 2 --efill 60` | `R1/` | 116 charts, no errors, 10 s. Only the dumps are committed. The schedule and score files (183 MB) are written again by the R1 command in `../RUNS.md` in about 10 s, and `R1/.gitignore` keeps them out. |
| R2 | the same dump at `--efill 0`, `--efill 85` and `--legacy-fills` | `R2/` | 116 charts times 3, no errors. CLEAN.md asked for the two windows. Legacy fills was added because A26 RU20 and RU21 may mean it. |
| R3 | `hydra_replay target` on Believer paths 0 to 4 and Allies (path 0, and `--ticks "22272,49920"`) | `R3/` | All ran with exit 0. Believer's depth-2 dump holds four paths, so path 4 was targeted from a depth-4 dump (`believer.depth4.dump.json`), as the old `records-screen` capture was. The Allies `--ticks` run finished inside its 10-minute cap. |
| R4a | the eight `records-screen` captures retaken with the fixed `moments.py` | `R4a/` | All eight ran with exit 0 in 13 s, 93 screenshots, and no error line in any text output. None of the old command files had lines after its last screenshot, so nothing was lost. |
| R5 | `hydra_batch` over `testdata/input`, plain and with `--legacy-fills`, plus the settings file it read | `R5/` | 98 charts analyzed, 0 failed, both times. The two `.db` files are kept out of git by `R5/.gitignore`. The R5 command writes them again. |
| R6 | `hydra_uitest` opening `v2.1.0-schema4.db` and `v2.12.1_library.db` from `testdata/store` | `R6/` | Both ran with exit 0. The startup text and `state` line are in the two `.txt` files. |
| R8 | the ref gate's `ReflogReplay` test over the 294 recorded real branch moves, run from `C:\Users\Patrick\.claude` with `GATES_REFLOG_REPLAY=1` | `R8/reflog_replay.txt` | **The test fails.** Today's gate refuses 65 of the historical main moves, but the test expects exactly one known exception (`f7f64cb18c..fa19f5c942`). The refusals name a missing real-app check after a `src/ui` change, or a missing reviewer verdict. This is a real result, not a run error. |

The documented way to run a hook test file is `python -I -m unittest discover -t . -s gates/tests -p <file>` from `C:\Users\Patrick\.claude` (`gates/README.md` line 135). `RUNS.md`'s R8 command, `python -I -m unittest tests.test_review_rules.ReflogReplay` from the gates folder, cannot import `tests` in isolated mode.

## What did not run

- **R4b to R4h** (Library view, batch strip, Settings panel, Dynamics tab, Path report, Path Builder). They serve 61 clean records by CLEAN.md's count. No script files exist yet. `RUNS.md` gives only the verbs. Each script has to be written from `docs/agents/ui-testing.md` first, and R4h's Path Builder opening lines are unchecked.
- **R7 and R9** serve no clean record.
- CLEAN.md marks 171 records "unplaced": no run in RUNS.md fits them. These are mostly the D14 Python tool tests, the precheck tests and the MIDI tests. It also marks 54 "no run" (the live game, or a test with no assertion), 16 "unavailable input" and 1 "needs the user's library".
