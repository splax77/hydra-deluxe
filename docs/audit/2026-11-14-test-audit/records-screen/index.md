# What the screens show on real charts

Question 3 of the rubric puts what a display shows on a real chart beside what the chart and the engine say there. `records-engine/` holds the engine's side. This folder holds the screen's side, captured by the main session on 2026-11-15.

## How it was captured

Every capture is a command file (`<name>.cmd.txt`) run by `hydra_uitest.exe --script`. Its stdout is `<name>.txt`: every command, then Hydra's reply, as text. `hydra_uitest` was built from main at c10c7f54, whose code under `src/` is the same as at 326cd4e0, the commit `records-engine/` was made from. It is headless Hydra with an empty database, Pro Drums and 2x Bass on.

Three settings files pick what the library holds:

- No `--ini`: the `testdata/input` corpus at "Score range 2 scores", the harness default. Used by `pitch_dark`, `pitch_dark_moments`, `believer`, `believer_path0_moments` and `beg`.
- `--ini fixtures.ini`: the four fixture charts in `fixture_songs/`, at "Score range 2 scores". Each folder there holds a byte-for-byte copy of the fixture's chart from `testdata/input` (renamed `notes.chart` or `notes.mid`) and a three-line `song.ini` naming it "audit <name>". The fixtures in `testdata/input` have no `song.ini`, so the corpus library does not list them; these copies are how they reach a screen. Used by `folded_sqin`, `late_sqin_twice`, `path_reduction_test0` and `flammarker_ingame`.
- `--ini corpus_depth4.ini`: the corpus at "Score range 4 scores". Believer's squeeze-out path, dump path 4 ("1 0- 1", 351,935), is the fourth score below the best, so the default range does not list it. Used by `believer_path4`, which clicks that path (`**/##path4`) before reading the Paths tab and the Preview. Its Preview reads "Showing { 1 0- 1 }".

The two `.ini` files name absolute folders on this machine. To capture again elsewhere, change the `chartfolder=` line, then run, from the repo root:

```bash
build-cpp/Release/hydra_uitest.exe --script <records-screen>/<name>.cmd.txt [--ini <records-screen>/<file>.ini] > <records-screen>/<name>.txt
```

A rerun overwrites the screenshots in place, at the paths the command file names.

## What each capture holds

- The Paths tab with every activation row expanded: the notation, the chord, the badges, the squeeze and early-fill lines, the Dynamic Hit Window lines and each Backend timings table. It is in `<name>.txt` after the first `> text`. `flammarker_ingame` has no activations, so it has no "Expand all" step.
- The Stars tab, after the second `> text` (not in the `_moments` files or `believer_path4`).
- Screenshots of the Preview's highway. They show the time box, the score box (score, stars, multiplier, combo, next star), the SP drain box, the next-activation box and the SP gauge. These boxes are drawn on the highway, not built as text widgets, so a screenshot is the only way to read them. An auditor reads a PNG with the Read tool.

`pitch_dark.cmd.txt` and `believer.cmd.txt` take their screenshots at the song's start and after each "Act >" jump (`<name>_preview_<n>_*.png`). Every other command file sets the playhead to exact moments the engine record names, with `moments.py` in this folder: `py -I moments.py <schedule.json> <name> <out_dir>` prints the lines. For each activation of the scheduled path it takes:

- `a<n>_start`: the activation chord's time;
- `a<n>_sp_ready`: the schedule's `sp_ready_ms`;
- `a<n>_<kind><k>_end`: each squeeze's `end_ms` (`sqin` or `sqout`);
- `a<n>_end_minus_1ms` and `a<n>_end`: 1 ms before the SP end (`deact_ms`) and the SP end itself;

and `song_start` at 0 ms. Each file name ends with the moment in ms. The playhead box (`##scrub`) takes seconds, so each line writes the ms value divided by 1000.

## The captures

| Capture | Song | Path on screen | Engine record |
|---|---|---|---|
| `pitch_dark`, `pitch_dark_moments` | Pitch Dark (CHON) | "3+ 2+ 1", 393,885 | `pitch_dark_ratio.*.json` |
| `believer`, `believer_path0_moments` | I'm A Believer (The Monkees cover) (Smash Mouth) | "1 0+ 0 0", 354,615 | `believer.dump.json` path 0, `believer.schedule.json` |
| `believer_path4` | the same | "1 0- 1", 351,935 | `believer.dump.json` path 4, `believer.path4.schedule.json` |
| `beg` | Beg (Evans Blue) | "1- 0 0 E0", 328,150 | `beg.*.json` |
| `folded_sqin` | audit folded_sqin | "2", 6,950 | `folded_sqin.*.json` |
| `late_sqin_twice` | audit late_sqin_twice | "0++++", 13,150 | `late_sqin_twice.*.json` |
| `path_reduction_test0` | audit path_reduction_test0 | "1 0 0", 45,450 | `path_reduction_test0.*.json` |
| `flammarker_ingame` | audit flammarker_ingame | "(No activations.)", 155,295 | `flammarker_ingame.*.json` |

Each path on screen is the same path, with the same total, as the engine dump's entry named in the last column.

## Where each case is

- **A squeeze-in**: `pitch_dark` activations 1 and 2; `late_sqin_twice` activation 1, four of them ("squeeze in -90.0 ms", "225.0 ms", "30.0 ms", "195.0 ms"), with a screenshot at each squeeze's end.
- **A squeeze-out**: `believer_path4` activation 2 ("squeeze out 0.0 ms", "<-- squeezed out (-200)"), screenshot `believer_path4_a2_sqout1_end_109255.6ms.png`; and `beg` activation 1 ("1-", "squeeze out 0.0 ms"), screenshot `beg_a1_sqout1_end_89079.9ms.png`.
- **An early fill**: `beg` activation 4, "4 E0 m83.1.0 2 bars early fill 0.0 ms" and "Early fill: 0.0 ms (required)" on the Paths tab. The screenshot at its SP-ready moment, `beg_a4_sp_ready_199079.9ms.png`, shows "Next: activation 4 of 4 / E0 (0.0 ms)". `folded_sqin` also lists a "0 E0" path among its paths within 2 scores, at 6,750, but it is not the path drawn.
- **A skipped fill**: `folded_sqin`, `pitch_dark`, `path_reduction_test0` and `believer` path 0 activation 1 (see `records-engine/index.md`).
- **Leeway**: each Backend timings table lists the notes near an SP end with their offsets and whether they count. The `end_minus_1ms` and `end` screenshots sit on either side of each SP end.
- **No activation**: `flammarker_ingame`, which still shows a multiplier squeeze (+15, "Hit [Kick] first.").

One thing seen while checking the pictures, left for the find rounds to judge: `believer_path4_a2_end_minus_1ms_109254.6ms.png` and `believer_path4_a2_end_109255.6ms.png` show the same score (218,395) and both still draw Star Power as running, with "empties in 0.0 s" in the drain box.

## What is not captured

- **Other tabs and windows**: the Dynamics tab, the library's other columns, the path report and the Path Builder.
- **Paths other than the ones in the table.** Any path a dump keeps can be shown the way `believer_path4` is: widen the score range in an `.ini` if needed, click `**/##path<N>`, and schedule that path for its moments.

An auditor who needs a moment or screen not here names it, writes "claim not checked", and the main session captures it after the find round, as T1 does for charts.
