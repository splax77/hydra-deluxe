# T1: the engine's record for real charts

Each chart below was run through `hydra_replay dump`, then `hydra_replay schedule` on the dump's path 0, and on Believer's path 4 as well. Each chart has `<name>.dump.json` (the paths the search keeps) and `<name>.schedule.json` (one path's activations, SP-end steps, squeezes and presses). Both are the engine's own output, not edited.

The records were first made with `hydra_replay` built at a19b9cb2. On 2026-11-15 main merged eef6b1d6, which moved the early-fill E mark from a 60 ms slack to the 85 ms hit window. So every record was made again with `hydra_replay` built from main at 326cd4e0. Every dump came out the same as before. Every schedule changed only in its `hydra_replay_build` stamp and in the path of the dump file it read (`path_source.file`). The screen records in `records-screen/` come from the same code.

The plan's five starting folders hold 19 chart files, and T1's cap is ten charts. So one file per folder was dumped, the one named for its case, and two corpus charts were added for cases the five do not show. Seven charts in all.

| Name | Chart |
|---|---|
| `pitch_dark_ratio` | `testdata/input/test_pitch_dark_ratio/notes.mid` |
| `folded_sqin` | `testdata/input/test_folded_sqin/folded_sqin.chart` |
| `late_sqin_twice` | `testdata/input/test_fast_tempo/late_sqin_twice.chart` |
| `flammarker_ingame` | `testdata/input/test_flammarker/flammarker_ingame.mid` |
| `path_reduction_test0` | `testdata/input/test_path_reduction/test0.chart` |
| `believer` | `testdata/input/common/IB24/T1/Smash Mouth - I_m A Believer (The Monkees cover) [highfine]/notes.chart` |
| `beg` | `testdata/input/common/IB24/T1/Evans Blue - Beg [highfine]/notes.chart` |

`believer` was added for the squeeze-out case. `tests/testdata_dirs.h` names it ("its best path's second activation can end two ways, with a squeeze-in or a squeeze-out"). `beg` was added for the early-fill case; its best path's notation, "1- 0 0 E0", carries an E.

## The cases

**A squeeze-in step.** `pitch_dark_ratio.schedule.json`, activation 0 (act tick 109440, chord 144): its `sp_end_steps` end with `squeeze_in` on chord 243 at tick 128520, and its `squeezes` hold one `SqIn` on chord 243 with `end_tick` 128400. This is the rubric's question 3 example. Activation 1 has another, on chord 644 at tick 202480. `late_sqin_twice.schedule.json`, activation 0 (act tick 5952, chord 35), has four `squeeze_in` steps, on chords 40, 48, 50 and 54.

**A squeeze-out.** `believer.dump.json`, path 4 (pathstring "1 0- 1", total 351935), activation 1: act tick 52992, `sqout_tick` 57600, `sqinouts` one `SqOut` at offset 0 ms. Its schedule is `believer.path4.schedule.json`. Path 0 of the same dump ("1 0+ 0 0", 354615) is the other way that activation can end: there, activation 1 squeezes in at tick 57600 (`sqin_ticks`); its schedule is `believer.schedule.json`. `beg.dump.json` path 0 has a second squeeze-out, on activation 0 (act tick 23040, "1-").

**A skipped fill.** `folded_sqin.schedule.json`, activation 0 (act tick 13824, chord 37): `skipped_fill_ticks` 4608 and 10752. `pitch_dark_ratio` activation 0 skips fills at 86400, 94080 and 101760; `path_reduction_test0` activation 0 skips one at 3072; `believer` path 0 activation 0 skips one at 19200.

**An early fill (E0).** `beg.dump.json` path 0, "1- 0 0 E0" at 328150, activation 3 (the fourth): act tick 62976. Its schedule gives `sp_ready_ms` 199079.9 and `deact_ms` 214079.9. The E in the pathstring is the engine's own: `Activation::notationstr` (`src/core/model.cpp`) writes it when `is_e_critical` holds, which is `is_e0(e_offset, 0)`. On screen, `records-screen/beg.txt` shows that row as "4 E0 m83.1.0 2 bars early fill 0.0 ms" and "Early fill: 0.0 ms (required)". The 0.0 ms is `early_fill_difficulty`, the negative of `e_offset`, so here SP became ready exactly at the fill's deadline. `folded_sqin.dump.json` also keeps a path with an E, "0 E0" at 6750, which is not its path 0.

The code that decides an E, at 326cd4e0, is in `src/core/model.h`, "the early-fill rule". One window, `kHitWindowMs` (85 ms), sits on both sides of the fill's deadline, measured on `e_offset` (the deadline minus the moment SP became ready). After the deadline it is the reach: `fill_refuses` is true when `e_offset` is below -85, and that decides which activations the search can find. Before the deadline it marks the activation E-critical: `is_e0(e_offset, skips)` is true when `e_offset` is under 85 and `skips` is 0. Whether the fill needs timing is a separate test, `early_fill_needs_timing`. The game's own fill deadline is an exact tick compare with no millisecond term (FL-24, `docs/clone-hero/04-fills.md`), so the 85 ms is Hydra's model, not a game rule. Neither `dump` nor `schedule` writes `e_offset` itself (a grep of `tools/replay*.cpp` for `e_offset` finds nothing), so a claim about its exact value reads it from the screen's "early fill N ms" line, which shows its negative.

This code is about to change again. Main at c10c7f54 filed two rulings (session a3ba79d1, `docs/agents/rulings/code-and-engine.md`): "make the early fill threshold an adjustable setting pinned to one number in both directions" and "analysis settings panel, default 60ms, supports 0ms to 85ms". Beg's E0 has an `e_offset` of 0, inside the window at any value from 0 to 85 ms. Any other E record is retaken once the setting lands.

**No activation.** `flammarker_ingame.dump.json` keeps one path, "(No activations.)" at 155295, and its schedule's `activations` is empty. It shows none of the cases above and stays as the record of a chart with no activation. Its screen shows a multiplier squeeze (+15).
