# R4e notes: the Dynamics tab

The tests are in `tests/test_dynamics_breakdown.cpp` (the brief's `dyn.cpp`). The screen is `render_dynamics_panel` in `src/ui/dynamics_tab.cpp`, drawn under tab `Dynamics` of `##DetailsTabs` (`src/ui/details_panel.cpp:324,343`).

Labels checked in `src/ui`: `###analysissettings` (`settings_bar.cpp:305`), `##bass2x` (117), `##prodrums` (111), `##difficulty` (100), `##DetailsTabs` and `Dynamics` (`details_panel.cpp`). Titles checked in `song.ini`: `Acid Romance` (Alpha Wolf, T3) and `When The Sunrise Breaks The Darkness` (Pathfinder, T6). Defaults: Expert, Pro Drums on, 2x Bass on (`config.h:149-151`). Escape closes the settings popup (the harness's `close_settings_panel` does the same).

Hard cannot be picked from a command file. The script verbs have no combo pick (the verb list has no such verb; `uitest_commands.cpp` has no combo code); only the C++ helper `settings_combo_pick` does. A Hard run needs `--ini` to seed a settings file with `view_difficulty=Hard` (`reset` copies the seed, `uitest_harness.cpp:248`). Unresolved: I did not write that ini, and it also needs the chart folder line. The script therefore runs Pathfinder at Expert only; the Hard facts (0 of 972, 3,426 total) are not captured.

**B1 22 (line 82), hand-built song row counts.** The test builds its own Song and checks `count_dynamics` per row. The real chart shows the same function: the script's first `text` on Acid Romance. The real test pins Acid Romance (line 155): Red snare 5/5/137, Yellow cymbal 3 accent/88 normal, Yellow tom 1/25, Blue cymbal 2/133, Blue tom 32, Green cymbal 1/60, Green tom 15, Kick 418, 2x kick 37, total 962. Matching numbers on the tab confirm; any shifted count contradicts.

**B1 27 (line 249), every row's own note counts in that row.** Covers `dynamics_row_for` and `dynamics_row_note`, a round trip with no direct screen. On screen, the nine rows each hold their own pad's counts (the `text | <row name>` lines). If a note landed in another row, the Acid Romance numbers above would differ. Partly shown: it shows the mapping for real notes, not the table's own sample notes.

**B1 36 (line 365), which kick rows count.** Covers `kick_row_counts`. Shown by `text | kick` with 2x Bass on (2x kick row plain, All kicks 455) and off (row reads `2x kick (not counted)`, All kicks 418, heading and status lines say `not counted (2x Bass off)`). The faded colour shows only in the two screenshots.

**B1 37 (line 373), one share rule for every percent.** Covers `dynamics_share`. Shown by the `2x kicks: 37 of 455 kick notes (8%)` line and `Dynamic notes: N of 962 (P%)` (925 with 2x Bass off). The empty-total case (0%) needs a chart with no kicks or no notes; none was found in the corpus, so that case is not shown.

**B1 38 (line 381), flags a row does not have are ignored.** Covers `dynamics_row_for` with notes carrying impossible flags (red cymbal, kick cymbal, blue 2x). No screen shows this: a real chart's parser does not produce such notes (unchecked), so there is nothing to see on the tab.

Not part of the records: Pro Drums off is captured too (cymbal rows disappear), as extra context.
