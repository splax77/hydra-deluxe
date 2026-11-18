# Cross-finding rows: one per finding, and one per split or dispute inside it

T8 step E4 (`docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md`). Written by the main session, session f3bbb32b, 2026-11-16.

`cross-findings.md` holds twelve findings, X1 to X12, that cross many tests, plus "The Keep recount" and "What the repeated-values pass left out". Each ends "Exit: not chosen". This file gives each finding, and each split, dispute and open point inside it, a row of its own, so E7 can give every one an exit and T9 can check every closure.

There are two parts. Part 1 is the main session's rows: one per finding and one per item the plan's E4 list names. Part 2 is every item a read-only scout found in the verifier files the findings cite (`exits/cross-candidates.md`, agent a68b829f4e9b0643b), carried over row for row. The scout found 111 items. 44 of them `cross-findings.md` does not name at all, and 17 it names only in part.

## How a row's route is set

The route says which exit a row can take. It is not the exit; E7 picks that from the rule table.

- **2** (a question to the user). Every split, dispute and open point. A split could leave by exit 3 only with a quote from a `## Confirmed` line that names both of its sides. None of the rulings in `docs/agents/rulings/tests-and-runs.md` names both sides of any split below, so none is routed to 3. An open point needs a read or a run that nobody has done, so it cannot close either.
- **closure?** (a closure candidate). A row where a verifier called something sound, or set copies aside. It leaves by exit 4 only if the main session writes one sentence at E7 saying what each side answers, and a fresh agent agrees at T9. "Sound", "set aside", "kept" and "weak" are the verifier's words, not evidence. Without the sentence, it is `no rule`, which becomes a question (`rules.md`, rule Xc).
- **prod** (a production-code finding). The fault is in `src/` or `tools/`, not in a test. It is exit 2, and its question shows every screen that reads the value (agent rule 11).

## Part 1: the findings, and the items the plan names

| Id | Finding | Item | Kind | Source | Route |
|---|---|---|---|---|---|
| X1 | X1 | Burnout's best path, score and figures typed by hand in many tests, with no owner | finding | cross-findings line 7 | 2 |
| X1.a | X1 | One fact, several homes: WN5, V-WN6 and V-WN7 each name the copies a split | split | cross-findings line 31; Part 2 C19 to 23 | 2 |
| X1.b | X1 | The copies at `test_overlay_layout.cpp` and `test_path_list.cpp:434` that V-RP1 and V-RP2 removed as input, and the user's filed words put back: "tests should test their intended behavior directly, not make up an arbitrary meaningless string" (`tests-and-runs.md` line 13) | dispute | cross-findings line 29; Part 2 C7, 8 | 2 |
| X1.c | X1 | `test_sp_meter_curve.cpp:948` uses "3- 1 2" only as a key to look up the engine's path; a verifier called it sound | sound | cross-findings line 19; Part 2 C9 | closure? |
| X2 | X2 | The Path Builder tests click fill ticks typed from one run | finding | cross-findings line 35 | 2 |
| X2.a | X2 | Both fills still being offered at 60 ms rests on a reading of Burnout's dump, which does not list offered fills | open | cross-findings line 39; Part 2 C46 | 2 |
| X3 | X3 | Allister's offered fills typed by hand in two tests | finding | cross-findings line 51 | 2 |
| X4 | X4 | The squeeze timing tiers, their names and edges, typed in many tests | finding | cross-findings line 57 | 2 |
| X4.a | X4 | The four lines V-RP2 removed (`test_config.cpp:285,302`, `test_report_view.cpp:339,342`) | dispute | cross-findings line 59; Part 2 C51 | 2 |
| X5 | X5 | Product messages typed in the tests that check them | finding | cross-findings line 63 | 2 |
| X5.a | X5 | "No Expert Pro Drums notes in this chart." and its Hard form, owner `no_notes_message` | copy | cross-findings line 66 | 2 |
| X5.b | X5 | "cancelled", owner `src/core/error_kind.h:158` | copy | cross-findings line 67 | 2 |
| X5.c | X5 | "sqlite exec failed: ...", owner `src/store/record_store.cpp:1058` | copy | cross-findings line 68 | 2 |
| X5.d | X5 | "cannot open file: " typed in four test files | copy | cross-findings line 69 | 2 |
| X5.e | X5 | "cannot open file: " has no single owner: `src/` types it in at least six places with no shared constant | split | cross-findings line 69; Part 2 C57 | prod |
| X5.f | X5 | "/all-users", owner `src/net/dmbot_client.cpp:122`; the copies are fakes that answer that path | copy | cross-findings line 70 | 2 |
| X5.g | X5 | The hand-written dmleaderboards replies (`canned_dm_users()`, `canned_dm_scores`, `kDmUsersJson`, `kDmScoresJson`) | stand-in | cross-findings line 70 | 2 |
| X5.h | X5 | The typed DMBot payload at `test_dm_report.cpp:118-130`, marked unclear by V-X9-RP2 | open | cross-findings line 70; Part 2 C58 | 2 |
| X6 | X6 | The unknown-title placeholder "(unknown)" typed 20 times in 6 files | finding | cross-findings line 76 | 2 |
| X6.a | X6 | The unknown-artist and unknown-charter placeholders typed in 3 files | copy | cross-findings line 76 | 2 |
| X7 | X7 | Settings defaults and screen words typed in the GUI tests | finding | cross-findings line 80 | 2 |
| X7.a | X7 | "Expert", the default difficulty | copy | cross-findings line 83 | 2 |
| X7.b | X7 | "Expert Pro Drums, 2x Bass" in `uitest_library.cpp` (three copies) | copy | cross-findings line 84 | 2 |
| X7.c | X7 | The four `test_store_open.cpp` copies of that key, set aside as the stored value in old fixture databases | set aside | cross-findings line 84; Part 2 C66 | closure? |
| X7.d | X7 | "Analysis settings: defaults", "Stop the batch to change these.", "10 ms", "Burnout — Green Day" | copy | cross-findings line 85 | 2 |
| X7.e | X7 | "SP drain (if activated)" and "early fill 0.0 ms" | copy | cross-findings line 86 | 2 |
| X7.f | X7 | The disputed separator `kDot`, which has no product constant to copy | dispute | cross-findings line 86; Part 2 C64 | 2 |
| X7.g | X7 | The widget labels "**/< Act##prevact" and "##timeline", weak by both readers | set aside | cross-findings line 87; Part 2 C65 | closure? |
| X8 | X8 | Names and keys the product writes typed in the tests | finding | cross-findings line 91 | 2 |
| X8.a | X8 | `max_tied_paths`, owner `src/core/rules.cpp:78` | copy | cross-findings line 93 | 2 |
| X8.b | X8 | "activation", owner `src/search/pather.cpp:366` | copy | cross-findings line 94 | 2 |
| X8.c | X8 | " ENDOFSONG" typed in 3 files, where the product's string is "_ENDOFSONG"; nobody has checked which form reaches the tests' input | open | cross-findings line 95; Part 2 C69 | 2 |
| X8.d | X8 | The replay tool's output keys, owned in `tools/` and read by 4 test files and two Python tools | copy | cross-findings line 96; Part 2 C68 | 2 |
| X9 | X9 | Tests feed made-up stand-ins for real engine output | finding | cross-findings line 100 | 2 |
| X9.a | X9 | Made-up summary rows (`ready(...)`, `analyzed(...)`) fed to the Library model and search | stand-in | cross-findings line 119 | 2 |
| X9.b | X9 | Hand-built Activations, squeezes and paths fed to the badges, path buttons, preview boxes and scene builder | stand-in | cross-findings line 120 | 2 |
| X9.c | X9 | Made-up records written to the store and read back by the reports | stand-in | cross-findings line 121 | 2 |
| X9.d | X9 | Hand-built activations and path JSON fed to the replay tool's writers and readers | stand-in | cross-findings line 122 | 2 |
| X9.e | X9 | Made-up scores from the fake analyzer `gated_scores` | stand-in | cross-findings line 123 | 2 |
| X9.f | X9 | Hand-written DMBot replies (with X5.g) | stand-in | cross-findings line 124 | 2 |
| X9.g | X9 | `test_app_click.cpp:1069`, the one dispute left: a count of shown charts fed to a label builder, which a verifier called sound as app state | dispute | cross-findings line 116; Part 2 C73 | closure? |
| X10 | X10 | Tests pin a typed constant instead of what the code does | finding | cross-findings line 130 | 2 |
| X10.a | X10 | `kReportConfirmAbove` 500, pinned in one file | pin | cross-findings line 190 | 2 |
| X10.b | X10 | `kDefaultEarlyFillWindowMs` 60.0, pinned in one file (`test_config.cpp:255`) | pin | cross-findings lines 134, 190 | 2 |
| X10.c | X10 | The source text of `kSqueezeWindowMs` and `kHitWindowMs` searched in `model.h` (`test_replay_schedule.cpp:355,356`) | pin | cross-findings lines 134, 190 | 2 |
| X10.d | X10 | The rest of RP1-6's six single-file pins, as `repeats/RP1.md` lists them | pin | cross-findings line 190 | 2 |
| X10.e | X10 | The backend cap 500 (`uitest_paths.cpp:408,410`) | pin | cross-findings line 190 | 2 |
| X10.f | X10 | `wal_autocheckpoint` 1000 (`test_store_open.cpp:1284,1292`), whose owner is itself a bare literal at `src/store/record_store.cpp:1115` | pin | cross-findings line 190; Part 2 C91 | 2 |
| X10.g | X10 | The owner of `wal_autocheckpoint` is an unnamed literal | prod | cross-findings line 190 | prod |
| X10.h | X10 | The MIDI message-length limit text (`test_midi.cpp:524`) | pin | cross-findings line 191 | 2 |
| X10.i | X10 | The star cutoffs (`test_stars.cpp:222,259,341`) | pin | cross-findings line 192 | 2 |
| X10.j | X10 | The Library column widths (`test_column_widths.cpp:146,214`) | pin | cross-findings line 193 | 2 |
| X10.k | X10 | A default highway setting (`test_preview_config.cpp:62`) | pin | cross-findings line 194 | 2 |
| X10.l | X10 | Constant pins never counted across the slices: "records that keep a constant pin may exist in every slice" | open | cross-findings line 141; Part 2 C86 to 88 | 2 |
| X11 | X11 | Echo checks: a value typed in and required back | finding | cross-findings line 145 | 2 |
| X12 | X12 | Tests that cite something that is not code | finding | cross-findings line 151 | 2 |
| KR | Keep recount | 239 of 421 Keep records hold a fault the user has ruled on | finding | cross-findings line 159 | 2 |
| KR.a | Keep recount | The counters split on hand-built library rows: KR4 counted them as X9, KR5 did not; KR5's uncounted rows still have to be added to its 25 | split | cross-findings line 163; Part 2 C106 | 2 |
| KR.b | Keep recount | The two fake-analyzer seams in the product, `BatchJob::set_analyzer_for_test` and `set_app_batch_analyzer_for_test` (`src/ui/library_jobs.h:285`, `:419`); no ruling of the user asked for them | prod | cross-findings line 165; Part 2 C107 | prod |
| LO.a | Left out | RP3-10, "chart" and "charts" passed to the product's `counted()`, disputed | dispute | cross-findings line 188; Part 2 C111 | 2 |
| LO.b | Left out | RP1-7, 48 kHz: three places not traced | open | cross-findings line 189; Part 2 C112 | 2 |
| LO.c | Left out | 12960, the engine's squeezed-out chord on a hand-built chart (`test_sp_meter_curve.cpp:464`, `test_sp_end.cpp:220`) | pin | cross-findings line 195; Part 2 C95, 96 | 2 |
| LO.d | Left out | Allies' best-path ticks typed as pins in `test_path_costs.cpp` only | pin | cross-findings line 195; Part 2 C114 | 2 |
| LO.e | Left out | Rows the verifiers did not re-open, which stand on the sorter's reading alone | open | cross-findings line 196; Part 2 C115 | 2 |

Every "Exit: not chosen" line in `cross-findings.md` (X1 to X12, twelve in all) maps to its finding's first row above. The Keep recount and "What the repeated-values pass left out" carry no such line, and get rows KR and LO.

## Part 2: every item in the verifier files

These are the scout's rows, carried over with a row id `C<n>`, where n is the line the row sits on in `cross-candidates.md` (C7 to C117) and a route set by the rule above. A row Part 1 already holds is marked in its last column. The scout checked no claim against the code. Its line numbers come from the files it cites.

| Row | Finding | Kind | What it is | Source | In cross-findings? | Route | Part 1 |
|---|---|---|---|---|---|---|---|
| C7 | X1 | dispute | V-RP1 removed `test_overlay_layout.cpp:342,510` from Burnout copies ("The test makes the text up"); the user overruled it, so they stay in X1. | V-RP1:15 | yes (CF 29) | 2 | X1.b |
| C8 | X1 | dispute | V-RP2 removed `test_overlay_layout:344,472,505` and `test_path_list:434` from the "(optimal)" copies as K3 inputs, and said `test_path_list:434` is a pass-through into `path_item_id`; real copies are `test_preview_scene:998` and `uitest_preview:700`. | V-RP2:52, 80, 98 | yes (CF 29) | 2 | X1.b |
| C9 | X1 | sound | `test_sp_meter_curve.cpp:948` uses "3- 1 2" only as a lookup key into the engine's paths ("Keep it, marked as a key"). | V-RP1:17 | yes (CF 19) | closure? | X1.c |
| C10 | X1 | dispute | V-RP1 added `test_pb_coach.cpp:66` (364675 twice, "13,640", "one run") to the copies. | V-RP1:19 | yes (CF 21) | 2 | none |
| C11 | X1 | set aside | `test_activation_rows.cpp:579,755` (`format_ms(163.0)`, made-up SPSqueeze -163.0) are called K3 builds of their own value. | V-RP1:19 | no | closure? | none |
| C12 | X1 | set aside | `format_ms(163.0)` also appears at `test_display_format.cpp:62`, `test_model.cpp:973`, `test_rules.cpp:197`, `test_activation_rows.cpp:579`; verifier says the choice of 163.0 is arbitrary and copies Burnout: X1, not X9, not snapshot. | V-X9-RP1:14 | partly (CF 13 lists 163.0 ms; not these four) | closure? | none |
| C13 | X1 | open | `test_library_query.cpp:393` has the comment "Burnout's real 163.0 ms" on a made-up summary; the verifier says the author "meant it as the real value" while the sorter read K3. | V-RP1:53 | yes (CF 109, 155) | 2 | none |
| C14 | X1 | open | Burnout "m32.1.0" owner corrected to `format_measure` (`timing.cpp:358`); weak copies in `test_overlay_layout:324,331,397,398` are "layout tests that choose Burnout-sized strings", real ones in uitest_paths:293, uitest_preview:783,1040. | V-RP3:37 | yes (CF 24, 107) | 2 | none |
| C15 | X1 | dispute | 71040 in `test_model.cpp:1286` is K3 per V-RP3 (hand-built `one_step(59520, 71040)`) but CF lists it under X9 as reusing Burnout numbers. | V-RP3:50 | yes (CF 110) | 2 | none |
| C16 | X1 | sound | Row "Burnout" is the song title read from the corpus chart (K4); "Bloodline" and "Acid Romance" split into built-by-hand (K3) and read-from-the-real-chart (K4, `uitest_details`, `uitest_library:663-669`). | V-RP1:45; V-RP2:27, 67 | no | closure? | none |
| C17 | X1 | sound | `test_pb_coach`/`test_fills_and_skips` hold X2's tick; `song_length` 130087.87999999993 at `uitest_paths:925,935` is parse output, not window-dependent. | V-RP2:80; WN5:33 | partly (CF 41 names the clock, not this) | closure? | none |
| C18 | X1 | open | `test_activation_rows.cpp:1367` (85.1, 61.9, 48.1) and 1340-1341 ("frontend 84.5 ms", "5.4 ms safe") are real-chart results typed with no rule beside them (snapshot). | V-X9-RP3:31 | partly (CF 155 names snapshots, not these) | 2 | none |
| C19 | X1 | split | `test_pb_text.cpp`, `pbtest_build`, `pbtest_coach` (fill-card), `pbtest_window` each type "378,315", "378,175", m32, "m20 to m24", "-140": "four copies of one answer"; "one 60 ms change moves them in step". | vWN6:40 | yes (CF 31 names WN6 as a split) | 2 | X1.a |
| C20 | X1 | split | Three tests (`test_panel_headline`, `test_paths_list`, `open_burnout`) and two helpers each pin Burnout's `3- 1 2` and `378,315` "with no one owner of Burnout's optimal path". | WN5:168, 442; vWN5:62 | yes (CF 31) | 2 | X1.a |
| C21 | X1 | split | `scratch_settings.h` comment says its settings are those "the Burnout reference results in the unit tests were made with", yet the window default is 60 and `tests/song_digest.h` pins 85; so unit digests and GUI tests describe Burnout at different windows. The verifier did not open `song_digest.h`. | WN5:442; vWN5:63, 72 | no | 2 | X1.a |
| C22 | X1 | split | `test_path_list.cpp:272-299` and `test_activation_rows.cpp:582` type the same Burnout values ("3- 1 2", 163.0 ms): "two typed copies of one run". | vWN7:46 | yes (CF 11-13, files listed) | 2 | X1.a |
| C23 | X1 | split | `test_pb_files:56,88` and `test_path_list:406` each type Burnout's activation count: "three copies". | vWN7:47 | yes (CF 13, "activation count 3") | 2 | X1.a |
| C24 | X1 | split | `test_preview_boxes:509` types `stars == 7` beside :505 which compares with `path_stars(best)`: "a typed second answer to a question the owner answers". | vWN7:48 | no | 2 | none |
| C25 | X1 | split | Tapped Out's activation 4 text is typed in `uitest_paths.cpp:941-942` and again in `test_activation_rows.cpp` (the test's comment names it); also `test_paths_hit_window` comment names the same row pinned there. | vWN5:64; WN5:411 | no | 2 | none |
| C26 | X1 | dispute | vWN7 corrects WN7 record 2: the button counts at `test_path_list:426-430` depend on record generation and score range, not row count; only :406 and :407 can move. | vWN7:42 | no | 2 | none |
| C27 | X1 | open | No engine record at 60 ms existed for Burnout, Tapped Out, Tapestry or chart 0, so all 17 WN5 C records have "claim not checked"; CF 17 now has Burnout's dump at 60 and 85 but not the other three charts. | WN5:17, 438; vWN5:3 | partly (CF 17 covers Burnout) | 2 | none |
| C28 | X1 | open | WN5 `test_paths_uncounted`: Tapestry was picked by a corpus search at the old window; whether it still has an uncounted squeezed-out row at 60 is not checked. | WN5:325-338; vWN5:44 | no | 2 | none |
| C29 | X1 | open | WN5 `test_paths_hit_window`: whether Tapped Out's activation 4 stays "optional" depends on skips and e_offset "which no record shows". | WN5:400 | no | 2 | none |
| C30 | X1 | open | WN5 `test_preview_path_overlay`: which chart is "chart 0" is open; needs the first library row read. | WN5:23, 176; vWN5:34 | no | 2 | none |
| C31 | X1 | dispute | vWN5 says WN5 record 3's check cite is wrong: the code has `IM_CHECK(active_ms >= 0.0)` at `uitest_preview.cpp:505`, not 516. | vWN5:36, 54 | no | 2 | none |
| C32 | X1 | open | vWN5 notes `records-engine/beg.dump.json` (at 85) was not used for Beg's row 4 ("likely stable"). | vWN5:41 | no | 2 | none |
| C33 | X1 | dispute | `test_note_window` expected values `pinned(...)`: V-X9-RP1 left them "sound but unclear" (sources "TM-21's worked values", a mockup, a plan); V-X9-RP1b ruled them snapshot and X12. | V-X9-RP1:55; V-X9-RP1b:36, 38 | no | 2 | none |
| C34 | X1 | dispute | WN5 had no row for the registered test "script" (`uitest_script.cpp:57`); vWN5 says it needs one. | vWN5:10 | no | 2 | none |
| C35 | X1 | open | Files that run the search through the GUI harness sat in no window group (`uitest_batch_reports`, `uitest_tests`, `pbtest_build`, `pbtest_coach`, `pbtest_files`, `pbtest_window`): "a gap for the main session". | vWN5:12 | no | 2 | none |
| C36 | X1 | open | WN5 B rows rest on a typed-number grep only for unread regions (`uitest_report_windows` 1051-1105 etc., `uitest_library` 541-704, 805-1012, `uitest_live` 1-375); vWN5 also read some by assertion grep only. | WN5:440; vWN5:69-71 | no | 2 | none |
| C37 | X1 | open | vWN7: B case bodies in WN7's "Not read whole" list (many `test_preview_boxes` cases, `test_app_library` 810-1180) not read by either; grep "supports B but does not prove it". | vWN7:28, 56 | no | 2 | none |
| C38 | X1 | sound | vWN7 B rows: `test_pb_files:123`, `:170,179`, `test_path_list:94,143,169,197,229` relational to the same record; a silent skip at `test_path_list:254` (`if (all.size() > 1)`) "is confirmed". | vWN7:23-25 | no | closure? | none |
| C39 | X1 | sound | vWN5 and vWN6 called 104 + 25 B rows sound ("Disputed: 0"), among them `test_stars` "4.4" (a constant, not a search output), `test_preview_solo_box` (no SP phrase in the chart), `test_paths_long_path`. | vWN5:18-28; vWN6:17-25 | no | closure? | none |
| C40 | X1 | open | vWN6 could not fully follow `preview_view.cpp:466-468` (null path means every fill Offered) against `cant-activate-reason`; "the auditor's section on offered states rests on code I could not fully follow either". | vWN6:24, 45 | no | 2 | none |
| C41 | X1 | open | vWN6 record 6: `believer.schedule.json` third activation (74496) not checked; proven at 85 only. | vWN6:33, 45 | no | 2 | none |
| C42 | X1 | sound | vWN6 B rows: `settings-and-reports` settled by arithmetic (optimal cannot be under 100,000), `batch-chip-count`, `drag-moves-the-clock` (hit-box centre depends on neighbouring fills, chart data). | vWN6:19-22 | no | closure? | none |
| C43 | X1 | open | vWN6 records 13 and 14: red "with no bug in what it claims to test" only through the setup click `click_fill(kFirstFillTick)`. | vWN6:36 | no | 2 | none |
| C44 | X2 | split | `kEarliestTick = 36480` (`pbtest_window:178`) retypes `kFirstFillTick`; `kUnderTwoTick = 21120` typed twice (`pbtest_window:161`, `pbtest_coach:126`). | vWN6:41 | yes (CF 37, 41) | 2 | none |
| C45 | X2 | split | `kFirstFillTick`/`kSecondFillTick` are read by five files in `tests/pb_ui`, only the harness that defines them is in a window group. | vWN5:65 | yes (CF 37) | 2 | none |
| C46 | X2 | open | "The dump does not list offered fills as such, so that last step is by reading, not proven." | CF 39 (in CF itself) | yes (CF 39) | 2 | X2.a |
| C47 | X2 | dispute | V-RP3 added the song-length "2:10.088" at `pbtest_build:199`, `pbtest_coach:309`, `pbtest_window:204` as RP3-1 copies; V-RP3 says the length is a chart fact and the clock text is `clock_str`; RP3 row 149 "5:59.957" moved from K1 to K4. | V-RP3:12, 36, 62 | partly (CF 24, 41 name 2:10.088 and 0:25.123; the K4 move is not named) | 2 | none |
| C48 | X2 | sound | `test_pb_coach:108,218` pass "activation" in as input (RP2-5). | V-RP2:84 | no | closure? | none |
| C49 | X3 | split | Allister's fill list typed in `test_fills_and_skips:448` and `test_pb_text:118`, with cost tiers and order typed beside it; owner `fill_refuses`. V-RP3 resolved the sorter's "71040 in test_pb_text:118" as Allister's, a different fact from Burnout's 71040. | V-RP3:16, 49 | yes (CF 53) | 2 | none |
| C50 | X3 | dispute | V-X9-RP3 calls `test_pb_text:118-126` X9 (made-up cost list fed to `choice_order`) with the fill list a typed copy of engine output (X3). | V-X9-RP3:17 | yes (CF 53) | 2 | none |
| C51 | X4 | dispute | V-RP2 removed `test_config:285,302` (test sets its own window to 85) and `test_report_view:339,342` (song names equal tier names) from X4's copies. | V-RP2:26, 81 | yes (CF 59) | 2 | X4.a |
| C52 | X4 | open | V-RP1: `test_report_view.cpp:339` "still unclear, I did not trace it"; V-RP1 counted :446 as a K2 copy; V-X9-RP1 later called :339 sound with the caveat "same family as row 10". | V-RP1:36, 53; V-X9-RP1:9 | partly (CF 59 records the removal only) | 2 | none |
| C53 | X4 | dispute | V-RP1 added `test_rules.cpp:208,217,227,241`, `test_squeeze_rating.cpp:917`, `test_report_view.cpp:446` as "Normal" copies. | V-RP1:36 | partly (CF 59 says "about ten test files") | 2 | none |
| C54 | X4 | sound | `test_activation_rows.cpp:558,560` (`rating == "Normal"`): "The comment states the rule (D276 item 2)". | V-X9-RP1:8 | no | closure? | none |
| C55 | X4 | set aside | `test_replay_schedule:355` greps `model.h` for `kHitWindowMs = 85.0`; V-RP2 added it as a source-text pin. | V-RP2:26, 81 | yes (CF 134) | closure? | none |
| C56 | X4 | open | X10 candidates in the tier rows: `test_report_rows:468` ("Beyond means past the 170 ms window"), `test_report_view:423,447` ("Beyond 170 ms") typed as product text. | V-X9-RP2:73 | partly (CF 59, 139 name X4 and X5-X8 pins; not these lines) | 2 | none |
| C57 | X5 | split | "cannot open file: " typed at `mapped_file.cpp:28,33,46,49`, `winstr.cpp:290`, `analysis.cpp:113`, `update_verify.cpp:54`; "Four owners for one phrase" / six places, none a shared constant. | V-RP2:65, 85 | yes (CF 69) | 2 | X5.e |
| C58 | X5 | open | DMBot payload `test_dm_report:118-130` stands in for outside data; "I mark it unclear". | V-X9-RP2:38 | yes (CF 70) | 2 | X5.h |
| C59 | X5 | sound | `test_user_messages:100-107` "Hydra can't analyze..." messages typed as expected text (X10), and `test_dm_report:822` definition text; V-X9-RP3 notes these are X10, not echo. | V-X9-RP3:42 | no | closure? | none |
| C60 | X5 | open | `test_batch_text:22,32,33` type "12,345 charts" and "12,345 of these can't be analyzed..."; "The behaviour there is the thousands grouping". | V-X9-RP2:74 | no | 2 | none |
| C61 | X5 | open | `test_path_list:303` types "Within 5,000 points" as product text. | V-X9-RP2:78 | no | 2 | none |
| C62 | X6 | sound | `test_model.cpp:1060` `kUnknownTitle == "(unknown)"` and the other "(unknown)" literals compare a function's output with a typed copy (X10 soft); the sort lists at `test_library_model:346,348,975,1031` use it only as a sort key (sound). | V-X9-RP1b:44 | partly (CF 76) | closure? | none |
| C63 | X6 | sound | `test_sng.cpp:294` is the only copy that reads the placeholder from product output. | CF 76 | yes (CF 76) | closure? | none |
| C64 | X7 | dispute | RP3-6's third part: separator `kDot` has no product constant; `display_format.h:13` is only a comment; tests type " · " in many places. | V-RP3:20, 41 | yes (CF 86) | 2 | X7.f |
| C65 | X7 | set aside | Widget labels "**/< Act##prevact" and "##timeline" (RP3-9) "agree, weak". | V-RP3:23, 44 | yes (CF 87) | closure? | X7.g |
| C66 | X7 | set aside | `test_store_open` four "Expert Pro Drums, 2x Bass" copies are the stored value in old fixture databases (K4), not copies of `chartmode_key`; `dm_fixture.h:26`, `test_report_rows:891,894` K3 inputs. Copies are `uitest_library:164,217,241` only. | V-RP2:13, 98 | yes (CF 84) for store_open; no for dm_fixture/report_rows | closure? | X7.c |
| C67 | X7 | open | "10 ms" owner `config.h:165`; `test_config.cpp:1021` and `test_batch_text.cpp:55` "are two checks of one fact"; `uitest_batch_reports:393` finds it only as a substring. | V-RP3:26, 40 | yes (CF 85) | 2 | none |
| C68 | X8 | split | The replay tool's keys (`activations`, `deact_ms`, `act_tick`, `steady_ms_before_end`) are written in `tools/` (`replay.cpp:484`, `replay_json.cpp:187`, `replay_schedule.cpp:1063`) and read by 4 test files and two Python tools; the tests are the schema's own pin; `deact_ms` value 12000.0 at `test_cli:707` is the test's own chart (K3). | V-RP2:11, 86, 94 | yes (CF 96) | 2 | X8.d |
| C69 | X8 | open | " ENDOFSONG" vs product's "_ENDOFSONG": "Nobody has checked which form reaches the tests' input"; "check which one the product shows before calling it a copy". | V-RP3:29, 43 | yes (CF 95) | 2 | X8.c |
| C70 | X8 | sound | `max_tied_paths` (7 places, 4 files) are the error message naming the key; `pather.cpp:366` "activation" read and confirmed. | V-RP2:83-84 | yes (CF 93-94) | closure? | none |
| C71 | X9 | dispute | First verifiers called rows 2, 4, 10, 30, 52, 54, 69 sound ("a struct holding a scalar is X9; I do not see the difference"); `test_dynamics_breakdown:270-281`, `test_library_query:343-362,421-427`, `test_report_view:450-451`, `test_activation_rows:743-757,965-984`, `test_preview_boxes:965-974`. Later withdrawn under the rulings. | V-X9-RP1:25-30; V-X9-RP1b:18; V-X9-RP3:3 | yes (CF 116) | 2 | none |
| C72 | X9 | dispute | V-X9-RP2 first called 12.6 (`test_activation_rows:749,910,912,921`) and -12.0 (`test_library_query:353`) "a judgement call" and "a split, not mine to choose", then withdrew it (addendum): all typed numbers there are X9. | V-X9-RP2:15, 68 | partly (CF 116 covers withdrawn disputes in general) | 2 | none |
| C73 | X9 | dispute | `test_app_click.cpp:1069` (`analyze_shown_label(1234)`): reader X9, verifier sound (app state, not engine output). | V-X9-RP1b:7 | yes (CF 116) | 2 | X9.g |
| C74 | X9 | sound | `test_activation_rows:558,560`, `test_report_view:339`, `test_preview_boxes:775` moved to sound; `test_preview_boxes:706` snapshot moved to derivable. | V-X9-RP1:57 | no | closure? | none |
| C75 | X9 | sound | `test_sp_end_clamp.cpp` sound (arithmetic in comments), `test_stars:250-260`, `test_cli:704-707`, `test_sp_end:221,512` (240 BPM), `test_model:597-621` ("a judgement call"), `test_preview_boxes:608,633-635,766,767,818`, `test_replay:1306`. | V-X9-RP1:54, 56; V-X9-RP1b:51 | no | closure? | none |
| C76 | X9 | sound | V-X9-RP2 kept sound rows 74, 76, 77, 84, 90... 135 (arithmetic from the typed chart or a stated rule); `test_tied_variants:522,575` expected values follow rule plus chart, chart hand-built so not X9; row 207 "script" and row 205 places 2-3 sound. | V-X9-RP2:42; V-X9-RP3:47-49 | no | closure? | none |
| C77 | X9 | dispute | `test_sp_meter_curve` hand-built `Path`/`Activation` fed to `build_preview_scene` (rows 78, 79, 82, 130, 136, 137): reader said sound, V-X9-RP2 moved to X9 for the input; "Which cases truly need a hand-built record is for the user." | V-X9-RP2:37, 49 | partly (CF 120 names test_sp_meter_curve; "which cases need it" not named) | 2 | none |
| C78 | X9 | open | `test_replay.cpp:518-528` typed path JSON to `windows_from_json`: X9 "with low confidence" (old dump cannot be regenerated); `test_replay_schedule:130-193,866` "Moderate confidence"; `test_sp_meter_curve:763,764` "soft". | V-X9-RP1b:23-30 | yes (CF 122, 120) without the confidence caveats | 2 | none |
| C79 | X9 | open | `test_library_model.cpp:1081` ("the weakest case, since size is what the test measures"), `test_library_query:494` (value never read), `test_column_widths:66` ("nothing depends on it", "weakest") are made-up values by the ruling but filler by effect. | V-X9-RP3:11, 13, 46 | partly (CF 119 names library_model, not the filler caveat) | 2 | none |
| C80 | X9 | dispute | V-X9-RP1b: `test_library_model.cpp:426,720,723,754,783,796` `ready(1000,"1",3,0.0)`: "Weak in kind (the score is a marker)". | V-X9-RP1b:12 | yes (CF 119) | 2 | none |
| C81 | X9 | dispute | KR4 counted hand-built library rows (`fake_charts`, `library_entry`, `timed_library`) as X9, KR5 did not; main session applied the user's words to both. | CF 163 | yes (CF 163) | 2 | none |
| C82 | X9 | set aside | `test_replay_schedule.cpp:476-497` real squeeze numbers go in as input; expected aims typed to 0.01 ms (snapshot). `test_dm_report:86-90` `pct_h == 9956` "read from one run". | V-X9-RP3:39; V-X9-RP2:30 | partly (CF 155) | closure? | none |
| C83 | X9 | sound | Reader's "K3 = the test's own input" sort was rejected for made-up input; the rows keep their K3 label in the sorters' files. | CF 106 | yes (CF 106) | closure? | none |
| C84 | X9 | set aside | `test_dm_report:118-130` and X5's fakes: "the reader's 'sound' holds under that definition" but "unclear" under the user's words. | V-X9-RP2:38 | yes (CF 70, 124) | closure? | none |
| C85 | X10 | dispute | Four `test_stars`/`test_midi`/`test_column_widths`/`test_preview_config` pins added by V-RP2 (`test_midi:524`, `test_stars:222,259,341`, `test_column_widths:146,214`, `test_preview_config:62`) belong to the owners' own tests. | V-RP2:22, 37, 46, 62, 90 | yes (CF 191-194) | 2 | none |
| C86 | X10 | open | X10 candidates found "from what I read", not scanned: colours in `test_theme:91,134-145,185-187` and `test_badge_pill:148-152`, `uitest_library:951,1338` headers, `test_stars:215` multiplier table, `test_model:37-45` note score table, `test_config:407-410`. | V-X9-RP1b:42-45; V-X9-RP2:77-79 | no (CF 141 says they "have not been counted") | 2 | X10.l |
| C87 | X10 | open | `test_stars:219-223`, `test_dm_report:822`, `test_user_messages:100-107` X10 found in files opened for other reasons; "X10 was not hunted across the folder". | V-X9-RP3:42, 62 | partly (CF 141) | 2 | X10.l |
| C88 | X10 | open | Records that keep a constant pin may exist in every slice; "They have not been counted." | CF 141 | yes (CF 141) | 2 | X10.l |
| C89 | X10 | open | Row 87: `test_report_rows:468`, `test_report_view:423,447`; rows 72, 83, 86, 97, 110 badge texts (`test_activation_rows:745-837,877-921`); row 100 `test_pb_coach:163,174`, `test_pb_text:159,177`. | V-X9-RP2:73-76 | no | 2 | none |
| C90 | X10 | open | `test_docs_match_code.cpp:2024` `kDocImageMaxBytes = 500 * 1024` typed in a test and in `tools/doc_shots/shrink.py:20`, each pointing at the other (D300). "Not a repeat of 3 or more." | V-RP1:25 | no | 2 | none |
| C91 | X10 | open | `wal_autocheckpoint` 1000: the owner at `record_store.cpp:1115` "is itself an unnamed literal". | V-RP1:23 | yes (CF 190) | 2 | X10.f |
| C92 | X10 | set aside | `test_config:414` `clamp(early_fill_window_ms,200)==85` and `:410` `clamp(preview_volume,150)==100` are clamp pins; 200 and 150 are test inputs (K3). | V-RP1:27 | no | closure? | none |
| C93 | X11 | open | `test_store_results:641,645` type -315.8 in and read it back through the store; "the nearest" echo; `test_library_query:556` "close to an echo". | V-X9-RP2:81; V-X9-RP3:14, 56 | partly (CF 147 names only test_user_messages and X6) | 2 | none |
| C94 | X11 | sound | `test_user_messages:121` uses the text as both input and expected output ("pass-through"); next line already runs the real error. | V-RP1:34 | yes (CF 147) | closure? | none |
| C95 | X12 | open | `test_sp_meter_curve:464` (12960): V-RP3 moved it to K1 (an engine answer on a hand-built chart); V-X9-RP3 says `make_early_sqout_song` (`record_fixtures.h:97`) states 12960 as the chart's own tick so it is a chart input, but the comment "The engine's record" makes it X12. | V-RP3:51; V-X9-RP3:29 | partly (CF 155, 195) | 2 | LO.c |
| C96 | X12 | dispute | `test_sp_end.cpp:220,689` (12960): V-RP3 called it an engine answer on a hand-built song ("pinned"); V-X9-RP3 disputes: "chart input... Sound". | V-RP3:51; V-X9-RP3:30 | no | 2 | LO.c |
| C97 | X12 | open | `test_sp_meter_curve` 14400 and `{480,1920}` (`test_sp_end:812,813`) have no rule beside them; 12960 and 13440 follow from stated arithmetic. | V-X9-RP2:22 | no | 2 | none |
| C98 | X12 | open | `test_sp_end.cpp:134` `extended_tick = 6912`: the comment near :150 "gives no arithmetic, so it may be an unmarked snapshot". | V-X9-RP2:62 | no | 2 | none |
| C99 | X12 | open | Snapshot/X12 sets added that CF 155 does not name: `test_fill_deadline:116-138` (`want[2][3][3]`), `test_tied_variants:347,583`, `test_path_costs` totals 407610 and 451830, `test_replay:190-200,312-326,1513-1590`, `test_generated_fills:191,245`, `test_dynamics_breakdown:303,434`, `test_preview_boxes:294,494-535`, `test_model:88-110,1070`, `pbtest_*` "from one run" headers, `test_replay_schedule:965,1436`. | V-X9-RP1:41-49; V-X9-RP2:27-31; V-X9-RP3:26, 38-40 | partly (CF 155 lists a sample) | 2 | none |
| C100 | X12 | set aside | `pbtest_window:92` ("0:00.000") is a rule checked at `test_display_format:36`, but the comment cites a run; `test_preview_timeline:556` value derivable but "Pinned from one run"; `test_song_length` and `test_timeline_view` headers say "pinned from one run" though read values follow stated arithmetic. | V-X9-RP3:32-33, 41 | no | closure? | none |
| C101 | X12 | open | `test_note_window.cpp:73-80,140-147,265` cite "TM-21", "the plan", "the mockup" as source; V-X9-RP1 left it "unclear", V-X9-RP1b ruled X12. | V-X9-RP1:55; V-X9-RP1b:38 | no | 2 | none |
| C102 | X12 | open | "111 'from one run' comments" in the test folder: `test_pb_coach` (14), `pbtest_coach` (16), `pbtest_window` (11), `test_preview_timeline` (16), `test_activation_row_layout` (8), `pbtest_build` (7), `test_badge_pill` (6) and 25 more files not read; "All are X12 candidates." | V-X9-RP3:61 | no | 2 | none |
| C103 | X12 | open | Other files with "pinned" comments found by grep outside the K3 rows, not opened: `test_audio_decode:166,167`, `test_label_layout:56`, `test_best_finish:92`, `test_engine_sweeps:579,742`, `test_note_shuffle:3`, `test_app_shell:344`, `test_store_results:606` and others. | V-X9-RP2:33, 62 | no | 2 | none |
| C104 | X12 | open | V-X9-RP1 limits: a snapshot with no "pinned/oracle/from one run" comment, or in files outside its grep list, "would not be found". | V-X9-RP1:68 | no | 2 | none |
| C105 | X12 | open | V-X9-RP1b and V-X9-RP2 not-reached lists: `test_replay_schedule:504-1400`, `test_replay:756-860`, hand curves in `test_sp_meter_curve`, `test_stars`, `test_scoring:85,93`, `test_dm_report`, rows 78/79/130/136/137 in preview files; sound-row places listed unopened by V-X9-RP3. | V-X9-RP1b:66; V-X9-RP2:62; V-X9-RP3:63 | no | 2 | none |
| C106 | Keep recount | split | KR4 counted hand-built library rows as X9, KR5 did not; "KR5's uncounted rows are listed in its file and still have to be added to its 25". | CF 163 | yes (CF 163) | 2 | KR.a |
| C107 | Keep recount | split | The test-only seams `BatchJob::set_analyzer_for_test` (`library_jobs.h:285`) and `set_app_batch_analyzer_for_test` (`:419`) were added to dodge timing races; "No ruling of the user asked for any of them." | CF 165 | yes (CF 165) | prod | KR.b |
| C108 | Keep recount | open | The tallier "left about 40 records unsettled" (hand-written old-layout databases, a scanner in its own test file, hand-written hook payloads). | CF 177-182 | yes (CF 177-182) | 2 | none |
| C109 | Keep recount | open | First round says 240 Keep records hold a fault (59+68+54+34+25); the table after the tallier says 239. Investigate count 2,028 vs 1,970 is explained, the 240 vs 239 gap is not. | CF 161, 171 | no | 2 | none |
| C110 | Keep recount | open | 1,622 Investigate records with a fault stand on fakes/pins/echoes; only 406 with no fault can move to Keep by a capture. | CF 184 | yes (CF 184) | 2 | none |
| C111 | Left out | dispute | RP3-10 ("chart"/"charts" passed to `counted()`) and RP3-6's `kDot`. | V-RP3:10, 20, 45 | yes (CF 188) | 2 | LO.a |
| C112 | Left out | open | RP1-7, 48 kHz: only `test_audio_decode:155` is the product's rate; `test_preview_load_progress:341`, `test_preview_transport:62`, `test_stream_mix:168` "not traced"; `test_stream_mix:349,196` fixture inputs. | V-RP1:39, 53 | yes (CF 189) | 2 | LO.b |
| C113 | Left out | open | Single-file pins: backend cap 500 (`uitest_paths:408,410`), WAL 1000, midi limit text, star cutoffs, column widths, highway default, `kReportConfirmAbove` 500, `kDefaultEarlyFillWindowMs` 60.0, source text of `kSqueezeWindowMs`. | V-RP1:21-23; V-RP2:90; V-X9-RP1b:41 | yes (CF 190-194) | 2 | none |
| C114 | Left out | open | Allies' best-path ticks 9984, 22272, 37632, 49920 typed as pins in `test_path_costs` only; V-RP3 notes the other 9984 places (`test_replay:739`, `test_tied_variants:522,575`) are hand-built K3. | V-RP3:52 | yes (CF 195) | 2 | LO.d |
| C115 | Left out | open | Rows the verifiers did not re-open: V-RP1 rows 8-11 etc.; V-RP3 rows 139, 151, 155, 157, 171, 172, 177, 190, 194, 197, 199, 200, 203, 204, 206 (row 203's `rules.h:51` default not checked); V-RP2 did not read `song_length_ms`'s formatter or `stars_for_score` owner claims. | V-RP1:53; V-RP2:110; V-RP3:58 | yes (CF 196) in general | 2 | LO.e |
| C116 | Left out | open | V-RP3: "Rows not opened" includes 12960's twin at `test_sp_end:220` ("same chart tick... also an engine answer on a hand-built song"). | V-RP3:51 | yes (CF 195) | 2 | none |
| C117 | Left out | sound | V-RP2 found no K3/K4 row that hides a K1/K2 part across files; V-RP1 moved none of its checked K3/K4 rows to K1. | V-RP2:5, 90; V-RP1:43 | no | closure? | none |

111 rows: dispute 22, open 48, set aside 9, sound 18, split 14.
