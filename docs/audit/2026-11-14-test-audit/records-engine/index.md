# T1: the engine's record for real charts

Each chart below was run through `hydra_replay dump`, then `hydra_replay schedule` on the dump's path 0 (Believer's path 4 as well), with `hydra_replay` built from main at a19b9cb2 on 2026-11-15. Each chart has `<name>.dump.json` (the paths the search keeps) and `<name>.schedule.json` (one path's activations, SP-end steps, squeezes and presses). Both are the engine's own output, not edited.

The plan's five starting folders hold 19 chart files, and T1's cap is ten charts. So one file per folder was dumped, the one named for its case.

| Name | Chart |
|---|---|
| `pitch_dark_ratio` | `testdata/input/test_pitch_dark_ratio/notes.mid` |
| `folded_sqin` | `testdata/input/test_folded_sqin/folded_sqin.chart` |
| `late_sqin_twice` | `testdata/input/test_fast_tempo/late_sqin_twice.chart` |
| `flammarker_ingame` | `testdata/input/test_flammarker/flammarker_ingame.mid` |
| `path_reduction_test0` | `testdata/input/test_path_reduction/test0.chart` |
| `believer` | `testdata/input/common/IB24/T1/Smash Mouth - I_m A Believer (The Monkees cover) [highfine]/notes.chart` |

`believer` was added for the squeeze-out case. `tests/testdata_dirs.h` names it ("its best path's second activation can end two ways, with a squeeze-in or a squeeze-out"). Six charts were dumped in all, under the cap of ten.

## The cases

**A squeeze-in step.** `pitch_dark_ratio.schedule.json`, activation 0 (act tick 109440, chord 144): its `sp_end_steps` end with `squeeze_in` on chord 243 at tick 128520, and its `squeezes` hold one `SqIn` on chord 243 with `end_tick` 128400. This is the rubric's question 3 example. Activation 1 has another, on chord 644 at tick 202480. `late_sqin_twice.schedule.json`, activation 0 (act tick 5952, chord 35), has four `squeeze_in` steps, on chords 40, 48, 50 and 54.

**A squeeze-out.** `believer.dump.json`, path 4 (pathstring "1 0- 1", total 351935), activation 1: act tick 52992, `sqout_tick` 57600, `sqinouts` one `SqOut` at offset 0 ms. Its schedule is `believer.path4.schedule.json`. Path 0 of the same dump is the other way that activation can end: there, activation 1 squeezes in at tick 57600 (`sqin_ticks`). Path 0 of Believer was dumped but not scheduled. No starting chart showed a squeeze-out.

**A skipped fill.** `folded_sqin.schedule.json`, activation 0 (act tick 13824, chord 37): `skipped_fill_ticks` 4608 and 10752. `pitch_dark_ratio` activation 0 skips fills at 86400, 94080 and 101760; `path_reduction_test0` activation 0 skips one at 3072; `believer` path 0 activation 0 skips one at 19200.

**An early fill at the SP-ready tick: no corpus chart found.** The engine calls this case E-critical: `is_e0(e_offset, 0)` in `src/core/model.h` lines 316 to 323, where `e_offset` is the fill's deadline minus the moment SP became ready, and the fill is E-critical when that is under `kEarlyFillSlackMs` with no fill skipped. Neither number is a game rule. The game's fill deadline is an exact tick compare with no millisecond term (FL-24, `docs/clone-hero/04-fills.md` line 244). The 85 ms reach is Hydra's own model (D250, D270 item 8): `fill_refuses` reads it, so it decides which activations the search can find. The 60 ms (`kEarlyFillSlackMs`, `src/core/model.h` line 136) is the other side of the window. It was split from a single 60 ms window by commit efd1457f (D250 call 1, `docs/audit/2026-10-03-fix-decisions.md` line 1022). Today `is_e0` reads it, and through it: the `E` in an activation's notation (`Activation::notationstr`, `src/core/model.cpp`); the early-fill badge and "Early fill" row on the Paths tab (`badge_timings`, `early_fill_window_view` in `src/app/path_view.cpp`); whether a passed-over fill counts as an E1 for the best all-0 path (`allzero_activation`, D102); the ms-limit check (`fill_within_limit`); and the search's group key (`Engine::ready_class`, `src/search/engine.cpp`). Whether such a fill needs timing is a separate test, `early_fill_needs_timing`, which is true only when Star Power is ready at or after the deadline. The older D51 note (line 168) described the single window before that split and does not describe this code. Neither `dump` nor `schedule` writes `e_offset` or the E-critical flag (a grep of `tools/replay*.cpp` for `e_critical`, `e_offset` and `early_fill` finds nothing). Working it out from the dump's `sp_ready_ms` and the fill's deadline would be a value made outside the engine, so it is not done, and none of the six charts is known to show the case or known not to. A test that rests on this case reads "claim not checked". That the replay record does not carry the flag is itself a side note for the audit.

`flammarker_ingame` has no Star Power path (its dump's `paths` is empty). It shows none of the four cases and stays only as the record of a chart with no activation.
