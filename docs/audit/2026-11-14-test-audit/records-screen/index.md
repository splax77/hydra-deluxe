# What the screens show on real charts

Question 3 of the rubric puts what a display shows on a real chart beside what the chart and the engine say there. `records-engine/` holds the engine's side. This folder holds the screen's side, captured by the main session on 2026-11-15.

## How it was captured

Each song has a command file (`<name>.cmd.txt`) run by `hydra_uitest.exe --script`, built from main at 4c6272e2 (rebuilt that day; the code under `src/` is the same as at a19b9cb2). The run's stdout is `<name>.txt`: every command, then Hydra's reply, as text. It is headless Hydra on the `testdata/input` corpus, with an empty database and the corpus's default analysis settings (Pro Drums and 2x Bass on, "Score range 2 scores"). To capture again, run the same command file the same way.

Each capture holds:

- the Paths tab with every activation row expanded (the notation, the chord, the badges, the Dynamic Hit Window lines and each Backend timings table), in `<name>.txt` after the first `> text`;
- the Stars tab, after the second `> text`;
- screenshots of the Preview at the song's start and after each "Act >" jump, so at each activation of the optimal path: `<name>_preview_0_start.png`, then `<name>_preview_<n>_act.png`. Each shows the time box, the score box (score, stars, multiplier, combo, next star), the SP drain box, the next-activation box and the SP gauge. These boxes are drawn on the highway, not built as text widgets, so a screenshot is the only way to read them. An auditor reads a PNG with the Read tool.

## The songs

| Name | Song | Activations captured | Its engine record |
|---|---|---|---|
| `pitch_dark` | Pitch Dark (CHON) | 3, at m57.1.0, m93.1.0 and m123.1.0 | `records-engine/pitch_dark_ratio.*.json` |
| `believer` | I'm A Believer (The Monkees cover) (Smash Mouth) | 4, at m36.1.0, m70.1.0, m98.1.0 and m116.1.0 | `records-engine/believer.*.json` |

The path on screen is the engine dump's path 0 in both. Pitch Dark shows "3+ 2+ 1" at 393,885, and its dump's path 0 is "3+ 2+ 1" at 393885, activating at ticks 109440, 178560 and 237120. Believer shows "1 0+ 0 0", and its dump's path 0 is "1 0+ 0 0" at 354615, activating at ticks 26880, 52992, 74496 and 88320.

## What is not captured

- **The other four charts in `records-engine/`.** `folded_sqin`, `late_sqin_twice`, `flammarker_ingame` and `path_reduction_test0` are loose fixture files in folders with no `song.ini`, so the library does not list them and no screen shows them. A search for each in the scanned corpus found none.
- **Moments other than an activation's start.** No screenshot sits at an SP end, a squeeze or a leeway chord. The Preview can step 10 ms at a time from an activation, so a later capture can reach those moments by adding steps to a command file.
- **Believer's squeeze-out path (dump path 4).** The screen shows the optimal path only. Its "Showing" list can pick another path; this capture did not.
- **Other tabs and windows**: the Dynamics tab, the library's other columns, the path report and the Path Builder.

An auditor who needs a moment or screen not here names it, writes "claim not checked", and the main session captures it after the find round, as T1 does for charts.
