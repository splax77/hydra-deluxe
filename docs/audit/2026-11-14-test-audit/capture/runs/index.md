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

## The screen runs R4b to R4h (session f9b4f64e, 2026-11-15)

The user said "Yes, run them". One read-only scout per screen drafted each command file from the records' tests and `docs/agents/ui-testing.md`. The main session ran each file with `build-cpp\Release\hydra_uitest.exe --script <folder>\script.txt --shots <folder>`, built from main (stamp 15:08, after main's last code commit). Each folder holds `script.txt`, the scout's `notes.md` (one paragraph per record: what is shown, what is not, and why), `output.txt` and the screenshots. Where the main session changed a scout's line to make it run, a `# Main session fix` comment sits above it. The notes were written before any run, so where a note predicts a label or a count, `output.txt` is what the app drew.

| Run | Records | Result |
|---|---|---|
| R4b Library view | A17 16, 17; A34 T3, T7; A41 1, 6, 13, 15, 18 | PASS on the third run. Two fixes: Burnout already had a result, so the confirm box needed its re-analyze tick, and a chip is found only by its whole label (`Not analyzed (97)##chipnew`). No verb holds a hover, so A34 T7's hover teal and T3's hover and press shades are not shown. A41 15's scroll half is not shown, because the library is never scrolled. |
| R4c click and batch | DL3 1, 5, 6, 7, 8, 9 | PASS on the first run. **The six DL3 records are not batch-screen tests.** They are in `tests/test_app_click.cpp` and test a single song click's save racing a cancel, close or replace. CLEAN.md and RUNS.md filed them under the batch screens. A script cannot hold a click at its save, so only the end states are shown (the row reads its score after the click is replaced or closed); DL3 1 and 7 are not shown. |
| R4d settings panel | DL4 6 | PASS on the first run. The button reads `Analysis settings: Score range 2 scores` before any change, not `defaults`, because the harness's settings file sets search depth 2. Early fill window 85 and 0 show in the label, 60 drops it, `Reset to defaults` gives `defaults`. |
| R4e Dynamics tab | B1 22, 27, 36, 37, 38 | PASS on the first run, on Acid Romance with 2x Bass on and off and Pro Drums off, and on When The Sunrise Breaks The Darkness. A command file cannot pick a difficulty combo, so no Hard capture. B1 38 (flags a row does not have) is not shown. |
| R4f Path report | A7 #1 to #11, #43, #47; A47 2, 6, 7; A25 P1, R25, R29, R30; DL3 32, 38, 39 | PASS on the sixth run. Fixes: a combo item is picked by clicking the combo, then the item in its popup; a typed setting needs `key Enter` before the Escape that closes the panel, or Escape undoes it; the out-of-date strip's button needs `**/`; the re-analyze tick needs its whole label; the Cancel click was dropped, because the corpus report builds between two frames. A7 #43 and #47 point at `share_is_valid` and font loading in `test_app_shell.cpp`, not the Path report, so their line numbers may be wrong. A7 #3, #7, #10, #11, A25 P1, R25, R29 and DL3 32, 38 are internal or need a held build, and are not shown. |
| R4h-paths Paths tab | A19 PL13 | PASS on the first run. A19 PL13 tests `PathsTabUi` (`src/ui/paths_tab.cpp`), Hydra's Paths tab, so it ran in `hydra_uitest` on Burnout, not in the Path Builder runner. The dump shows rows opening one at a time and Expand all and Collapse all. The footer fold's open state does not show in a dump, so the screenshot is its evidence. |
| R4h Path Builder | WN7 4; A49 5, 26; A38 1 to 13 | **Not run.** `hydra_pathbuilder_uitest` takes only `--test`, `--script` and `--list` (`tests/pb_ui/pbtest_main.cpp`), and nothing in a script can open a chart: `open` has no song hook in that runner and File > Open is a native dialog. `R4h-paths/pathbuilder-scout-notes.md` has the scout's reading. A capture needs a chart flag in the runner, which is a code change. The runner was rebuilt from main for this session (`hydra_pathbuilder_uitest.exe`, build pass, 2026-11-15). |

R4g has no clean record in CLEAN.md, so it was not written.

## What did not run

- **R4h's Path Builder records** (16 of them), as the table above says.
- **R7 and R9** serve no clean record.
- CLEAN.md marks 171 records "unplaced": no run in RUNS.md fits them. These are mostly the D14 Python tool tests, the precheck tests and the MIDI tests. It also marks 54 "no run" (the live game, or a test with no assertion), 16 "unavailable input" and 1 "needs the user's library".
