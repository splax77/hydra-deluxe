# Findings that cross slices

A record judges one test. Some faults are one pattern repeated across many tests in many slices, and each record sees only its own copy. This file holds each such pattern as one finding, so T8 gives the pattern one exit and the closure check sees it whole. Each test's own record still carries its own verdict.

Counts below are from `git grep` over `tests/` at the audit branch's HEAD (code of main `5ec682c7`), run by the main session on 2026-11-15.

## X1. Burnout's best path and score are typed by hand in many tests

The best path of Burnout (`corpus::burnout()`, the corpus chart the GUI and unit tests use most) is the engine's answer. Many tests type that answer as a literal instead of reading it from the engine's record:

- The score `378315` or `378,315` appears 25 times in 10 test files: `tests/test_library_model.cpp` (10), `pb_ui/pbtest_build.cpp` (3), `pb_ui/pbtest_window.cpp` (2), `test_stars.cpp` (2), `ui/uitest_details.cpp` (2), `ui/uitest_paths.cpp` (2), and once each in `pb_ui/pbtest_coach.cpp`, `test_activation_rows.cpp`, `test_path_list.cpp` and `test_pb_coach.cpp`.
- The path text `"3- 1 2"` appears 23 times in 10 test files: `test_library_model.cpp` (7), `ui/uitest_paths.cpp` (4), and two or one each in `test_activation_rows.cpp`, `test_overlay_layout.cpp`, `test_path_list.cpp`, `test_store_records.cpp`, `test_preview_scene.cpp`, `test_sp_meter_curve.cpp`, `ui/uitest_details.cpp` and `ui/uitest_preview.cpp`.
- Other Burnout figures from the same run are typed beside them: "378,175", "375,955", "0 4 1", "163.0 ms", "2,360 below optimal", m20 → m24, m32, and the activation count 3.

Why it is a bad test: the rubric's question 5 (an expected value with no named source) and the derive-once rule. Each copy is a second answer to "what is Burnout's best path", with no owner. A change to the engine that moves Burnout's result turns all of them red at once, whether or not the change is a bug. A real bug that moves it gets the same flood. So the tests cannot tell the two apart, and the one test that should own the fact is lost among the copies.

The window change from 85 to 60 ms did not move Burnout's result. `records-engine/burnout.efill60.dump.json` and `burnout.efill85.dump.json` (`hydra_replay dump --cap 4 --ms 10 --depth 2`, the GUI tests' settings, built from main `5ec682c7`'s code) are identical apart from the window: "3- 1 2" 378315, "0 4 1" 378175, "2 1 2" 378075 at both. So the copies are not wrong today. The finding is the design: the day an engine change does move Burnout, every copy fails at once.

The repeated-values pass (`repeats/`, session 63c70014) changes this list in two ways. First, `test_sp_meter_curve.cpp:948` uses "3- 1 2" only as a key to look up the engine's path, which is sound. Second, it adds more Burnout figures typed from the same run:
- "0 4 1" and 378,175, the second path (RP2-1);
- 364675 twice and the cost 13,640 at `test_pb_coach.cpp:66`, under the comment "From one run" (V-RP1);
- the cost −140, in 3 files (RP2-1, confirmed by V-RP2);
- the song-length clock 2:10.088, in 3 files (RP2-1, V-RP3);
- the activation measures "m32.1.0" and "m58.1.0", "3 bars", and `kBurnoutSqOutRating` "Insane+ (eff. 163.5 ms)" (RP3-2). The owner of the measure text is `format_measure`, `src/core/timing.cpp:358` (V-RP3);
- 71040, Burnout's first activation's stored end, with 70800 and 48783.075 ms, at `test_sp_meter_curve.cpp:954-956` (V-RP3-B).

Every copy's `file:line` is in those candidates. Where a verifier removed or added copies, its V file holds the final list.

V-RP1 and V-RP2 removed `test_overlay_layout.cpp` and `test_path_list.cpp:434` from these copies, because the test makes up "3- 1 2  (optimal)" and feeds it to the word-wrap and trim functions. That removal is overruled. The user, in session 63c70014: "that doesnt make it any better. tests should test their intended behavior directly, not make up an arbitrary meaningless string". Those lines stay in X1's count, so the path text is still in 10 files, and they are also an instance of X9.

The records that already see a copy (each judges only its own): `records/A2.md`, `A3.md`, `A19.md`, `A26.md`, `A33.md`, `A39.md`, `A44.md`, `A45.md`, `A48.md`, `A49.md`, `C5.md`, `C6.md`, `C7.md`, `C12.md`, `delta-DL4.md`, `window-WN2.md`, `window-WN4b.md`, `window-WN4c.md`, `window-WN5.md`, `window-WN6.md`, `window-WN7.md`, and their verify files. WN5, V-WN6 and V-WN7 name it as a split (one fact, several homes).

Exit: not chosen. Only the main session picks it, in T8.

## X2. The Path Builder tests click fill ticks typed from one run

`tests/pb_ui/pbtest_harness.h:60-61` types `kFirstFillTick = 36480` and `kSecondFillTick = 74880`, the fills the player can activate on, "from one run under the GUI tests' settings". The same ticks are typed again as `kEarliestTick` (`pb_ui/pbtest_window.cpp:178`) and appear in `test_pb_coach.cpp`, `test_pb_state.cpp` and `test_fills_and_skips.cpp`. Related ticks from the same kind of run: `kLongSongFillTick` 133920 (`pbtest_build.cpp:63`) and `kLateFillTick` 162240 (`pbtest_window.cpp:390`).

Why it is a bad test: which fills are offered is decided by the engine's early-fill rule (`fill_refuses`, `src/core/model.h:331`). The tests type its answer instead of asking the engine for the offered fills. A rule change that stops offering one of these fills makes every test that clicks it fail at the click, with no bug, and the failure names the click, not the rule. The ticks are Burnout's (`pbtest_harness.h:58`). Since Burnout's dump does not change between 60 and 85 ms (X1), and both ticks appear in it at both windows, the window change most likely left both fills offered. The dump does not list "offered fills" as such, so that last step is by reading, not proven.

The repeated-values pass adds the same fill in other forms: its clock text "0:25.123" and the tick 21120, in the Path Builder tests (RP3-1, agreed by V-RP3). Their owners are the same early-fill rule and `clock_str`, `src/app/display_format.cpp:64`.

The records that see it: `records/window-WN5.md`, `window-WN6.md`, `A48.md`, `A49.md`, and their verify files.

Exit: not chosen. Only the main session picks it, in T8.

## Findings from the repeated-values pass

`repeats/repeats.py` listed every literal typed on a test assertion line or test constant in three or more test files: 207 rows in `repeats/repeats.tsv`. Three sorters sorted them (`RP1.md` to `RP3.md`), and one fresh verifier read each sorter's file against the code (`V-RP1.md` to `V-RP3.md`). Below are the candidates a verifier agreed with that X1 and X2 do not already hold. Each names its candidates. Every copy's `file:line` is in those candidates, and where a verifier changed the list, its V file holds the final one. As with X1, a change to the owner turns every copy red at once.

## X3. Allister's offered fills are typed by hand in two tests

Which fills the player is offered on Allister - Overrated is the engine's answer. Two tests type it: the list {55680, 71040, 90240, 105600, 120960, 136320, 151680, 167040} appears at `test_fills_and_skips.cpp:448` and `test_pb_text.cpp:118`. The comment on the second line says "the engine lists these fills", and its cost tiers and order are typed beside it (`:121`, `:126`). The owner is the same early-fill rule as X2 (`fill_refuses`, `src/core/model.h:331`), with the search's trailing fills. Same kind as X2, a different chart. Source: V-RP3-A.

Exit: not chosen.

## X4. The squeeze timing tiers, their names and edges, are typed in many tests

The tier names ("Normal", "Extreme", "Insane", "Insane+", "Beyond", "Hard (uncounted)") and the edges at 85 ms and 170 ms are typed in about ten test files outside the tier table's own tests. Owners: `timing_tiers()`, `src/core/squeeze_rating.cpp:241-251`; `kHitWindowMs`, `src/core/model.h:106`; " (uncounted)", `src/core/model.cpp:354`; `kFormulaScale` and `kWindowBackMs`, `src/core/note_window.h`. Renaming a tier or moving an edge turns all of them red at once. `test_squeeze_rating.cpp` and `test_note_window.cpp` are the tier table's own tests, but re-typing the table there is no better: it pins the constant, not what the rating does (X10). Sources: RP1-4, RP2-2 and RP3-3, all agreed. V-RP2 removed `test_config.cpp:285,302` (the test sets its own early-fill window to 85, a different constant) and `test_report_view.cpp:339,342` (song names that equal tier names).

Exit: not chosen.

## X5. Product messages are typed in the tests that check them

Each of these messages is written once in `src/` and typed again in three or more test files:
- "No Expert Pro Drums notes in this chart." and its Hard form. Owner `no_notes_message`, `src/parse/song.cpp:572`. 7 files for the Expert form (RP1-2), 3 more for Hard (RP3-7).
- "cancelled", owner `src/core/error_kind.h:158` (RP3-7).
- "sqlite exec failed: ...", owner `src/store/record_store.cpp:1058` (RP3-7).
- "cannot open file: ", in 4 test files (RP2-6). This one has no single owner: `src/` types the phrase in at least six places (`src/audio/mapped_file.cpp:28,33,46,49`, `src/core/winstr.cpp:290`, `src/app/analysis.cpp:113`, `src/app/update_verify.cpp:54`), none a shared constant (V-RP2). So it is a split on the product side as well as a copy on the test side.
- "/all-users", owner `src/net/dmbot_client.cpp:122` (RP3-7). The copies are fakes that answer that path. A renamed path would make them quietly return the wrong canned answer rather than fail. The answers themselves are written by hand: `canned_dm_users()` and `canned_dm_scores` in `ui/uitest_harness.cpp:200-206`, and `kDmUsersJson` and `kDmScoresJson` in `test_app_reports.cpp`. They stand in for the dmleaderboards reply the same way X9's made-up summaries stand in for the engine's, and no saved real reply exists on this machine (`capture/RUNS.md`). The user, in session 63c70014: "fake data will never appear in the real hydra and we should be testing the behavior of the real code". So the hand-written replies are X9 too, with the typed DMBot payload in `test_dm_report.cpp:118-130` (marked unclear by V-X9-RP2).

Exit: not chosen.

## X6. The unknown-title, artist and charter placeholders are typed in many tests

"(unknown)" (owner `kUnknownTitle`, `src/parse/song.h:101`) is typed 20 times across 6 files (RP1-3). The unknown-artist and unknown-charter texts (owners `kUnknownArtist` and `kUnknownCharter`, `src/parse/song.h:142-143`) are typed in 3 files (RP3-4). Only `test_sng.cpp:294` reads them from the product's output; the other copies feed the string in and then check it. Sources: RP1-3 and RP3-4, both agreed.

Exit: not chosen.

## X7. Settings defaults and screen words are typed in the GUI tests

These are constants or texts that `src/` owns, typed in three or more test files:
- "Expert", the default difficulty (`difficulty_name`, `src/parse/song.cpp:553`; `Settings::view_difficulty`, `src/app/config.h:149`) (RP1-5).
- "Expert Pro Drums, 2x Bass", the chart-mode key (`chartmode_key`, `src/app/config.cpp:331`). Only the three copies in `uitest_library.cpp` count. The four in `test_store_open.cpp` are the stored value in the old fixture databases (`testdata/store/README.md:77`), which `config.cpp` says must keep that spelling (RP2-3 as narrowed by V-RP2).
- "Analysis settings: defaults" (built by `settings_button_label`, `src/ui/library_dialogs.cpp:132`, from "Analysis settings: " and `settings_changes_summary`), "Stop the batch to change these." (`kSettingsLockedText`, `src/ui/library_parts.h:97`), "10 ms" (`mslimit_value`, `src/app/config.h:165`), and "Burnout — Green Day" (`chart_title_line`, `src/pathbuilder/pb_state.cpp:43`) (RP3-5).
- "SP drain (if activated)" (`src/app/preview_view.cpp:680`) and "early fill 0.0 ms" (`src/app/path_view.cpp:58`) (RP3-6). V-RP3 disputed RP3-6's third part: the separator `kDot` has no product constant to copy, because `src/app/display_format.h:13` is only a comment.
- The widget labels "**/< Act##prevact" and "##timeline", which the GUI tests need in order to click (RP3-9, weak by both the sorter's and the verifier's reading).

Exit: not chosen.

## X8. Names and keys the product writes are typed in the tests

- `max_tied_paths`, the rules key named in an error message (owner `src/core/rules.cpp:78`), typed in 4 files (RP2-4).
- "activation", the pather's failure reason (owner `src/search/pather.cpp:366`), typed in 3 files (RP2-5).
- " ENDOFSONG", a section-name rule (owner `src/parse/section_names.cpp:13`), typed in 3 files (RP3-8). V-RP3 notes that the product's string is "_ENDOFSONG", with an underscore. Nobody has checked which form reaches the tests' input.
- The replay tool's output keys (`activations`, `deact_ms`, `act_tick`, `steady_ms_before_end`). The owners are in `tools/`, not `src/`: `tools/replay.cpp:484`, `tools/replay_json.cpp:187` and `tools/replay_schedule.cpp:1063`. They are read in 4 test files and in two Python tools (RP2-7, added by V-RP2).

Exit: not chosen.

## X9. Tests feed made-up stand-ins for real engine output

A test that checks how Hydra handles an engine value (wraps it, trims it, lists it, stores it) should run on a value the engine made from a real chart. Many tests instead type a string or number that looks like a real result but is tied to nothing. The user, in session 63c70014: "tests should test their intended behavior directly, not make up an arbitrary meaningless string".

Why it is a bad test: the input has no link to what the engine produces, so the test proves the code works on the input someone imagined. When the engine's real output changes shape (a longer label, a new mark, a different separator), the made-up input does not, and the test stays green while the screen breaks. Where the made-up value copies a real one, it also carries X1's harm, a second answer with no owner.

The repeated-values pass first sorted these as K3 ("the test's own input, typing it is not a copy"). That reading is wrong for this kind of input. The instances found so far, each from the named file:
- `test_overlay_layout.cpp` wraps and trims "3- 1 2  (optimal)" (lines 340 to 344, 472 to 480, 505 to 510) and lays out "at m58.1.0" and "at m32..." (324, 331, 397, 398). Sources: V-RP1, V-RP2 and V-RP3, which all set these aside as input.
- `test_path_list.cpp:434` feeds the same "(optimal)" label (V-RP2).
- `test_library_model.cpp` builds made-up summaries such as `ready(378315,"3- 1 2",7,163.0)`, and `test_library_query.cpp` uses `analyzed(7, 163.0)`. Line 393 there carries the comment "Burnout's real 163.0 ms" (RP1-1, V-RP1).
- `test_model.cpp:1286` builds `one_step(59520, 71040)`, reusing Burnout's activation numbers on a hand-built step (V-RP3-B).

The same holds for outside data. A hand-written reply from a service Hydra talks to (dmleaderboards, GitHub) is a made-up stand-in too. The user: "fake data will never appear in the real hydra and we should be testing the behavior of the real code". The DMBot fakes are listed under X5.

**The K3 re-read.** Three readers re-read every K3 row of the repeated-values pass for this question (`repeats/X9-RP1.md` to `X9-RP3.md`). Then a fresh verifier checked each reader's file (`V-X9-RP1.md` and `V-X9-RP1b.md`, `V-X9-RP2.md`, `V-X9-RP3.md`), judging by the three rulings filed in `docs/agents/rulings/tests-and-runs.md`. Where a verifier and its reader differ, the verifier's file holds the final list.

Some verifiers first disputed made-up inputs because the expected text followed from the typed input. The rulings overrule that, since the user's own overlay-layout example was such a case. The verifiers withdrew those disputes, or a second verifier checked the inputs again. The one dispute left is `test_app_click.cpp:1069`. It feeds a count of shown charts to a label builder, which is app state, not engine output, so it is sound.

What the re-read found, by the code under test:
- Made-up summary rows (`ready(...)`, `analyzed(...)`) fed to the Library model and search. That is 27 uses of `ready` in `test_library_model.cpp` alone, several copying Burnout's real result.
- Hand-built Activations, squeezes and paths fed to the activation badges, the path buttons, the preview boxes and the preview scene builder (`test_activation_rows.cpp`, `test_path_list.cpp`, `test_preview_boxes.cpp`, `test_sp_meter_curve.cpp`).
- Made-up records written to the store and read back by the reports (`test_store_results.cpp`, `test_report_rows.cpp`, `test_chart_index.cpp`, and `test_fill_report.cpp`, which overwrites a real record's score, path and counts).
- Hand-built activations and path JSON fed to the replay tool's writers and readers (`test_replay.cpp`, `test_replay_schedule.cpp`).
- Made-up scores from the fake analyzer `gated_scores` (`test_app_library.cpp`).
- Hand-written DMBot replies (`test_dm_report.cpp:118-130`, and the fakes under X5).

The same files list the snapshots, X10 pins and X12 citations each verifier found along the way.

Exit: not chosen.

## X10. Tests pin a typed constant instead of what the code does

The user, in session 63c70014: "tests should not be pinned to typed constants. they should be pinned to the behavior of the code. i have repeatedly told you this is bad and you never listen."

A check such as `kDefaultEarlyFillWindowMs == 60.0` (`test_config.cpp:255`), or a search of `model.h`'s source text for `kHitWindowMs = 85.0` and `kSqueezeWindowMs = 500.0` (`test_replay_schedule.cpp:355,356`), proves only that the number is still the number. An intended change of the constant turns it red with no bug. A broken rule that never reads the constant leaves it green. The check that belongs there states the behaviour: a squeeze 1 ms past an edge rates in the next tier, or a fill 1 ms inside the window is refused.

So these are all X10, not just the repeats across files:
- The single-file pins listed below under "Pins in one file only".
- The tier table re-typed in its own tests (X4).
- The product texts and keys typed into the tests that check them (X5 to X8). For a message, the behaviour is when it is shown and what it names, not its spelling.

`docs/agents/test-rubric.md` has no question that catches this, and the per-test records were judged by that rubric. So records that keep a constant pin may exist in every slice. They have not been counted.

Exit: not chosen.

## X11. Echo checks

A check that types a value in and requires the same value back tests nothing. `test_user_messages.cpp:120-121` types "No Expert Pro Drums notes in this chart." into `ChartFileError(AlreadyPlain, msg)` and requires `plain_error` to return it. The next line already runs the real `NoNotesError` through `plain_error`, so the made-up line adds nothing. The placeholder tests in X6 mostly feed "(unknown)" in and check for "(unknown)". Sources: V-RP1, X9-RP1.

Exit: not chosen.

## X12. Tests that cite something that is not code

The user, in session 63c70014: "tests exist to verify the behavior of the code. a test citing something that isn't code is always wrong".

Many expected values name their source in a comment instead of in code: "From one run" (`test_pb_coach.cpp:66`, `:161`), "pinned from one run" (`test_replay.cpp:1516`, `test_preview_boxes.cpp:512-516`, `test_audio_player.cpp:139`), "The engine's record" (`test_sp_meter_curve.cpp:461`, `:464`), "as PS0b saved it" (`test_sp_meter_curve.cpp:951`), and "Burnout's real 163.0 ms" (`test_library_query.cpp:393`). Each comment admits the value belongs to the engine, and none ties it there. They are the snapshot instances in `repeats/X9-RP1.md` to `X9-RP3.md`.

Exit: not chosen.

## The Keep recount

Five counters re-read every record the audit called Keep, by the three filed rulings and X9 to X12 (`briefs/T5-keep-recount.md`, `counts/T5-KR1.md` to `T5-KR5.md`). They read 421 records, and 240 hold at least one fault: 59 of 91 (KR1), 68 of 97 (KR2), 54 of 84 (KR3), 34 of 66 (KR4) and 25 of 83 (KR5). Of the 240, the record itself already named the fault, wholly or partly, in at least 103. It was kept anyway, because the rubric had no question for it.

The counters split on one case. KR4 counted hand-built library rows (`fake_charts`, `library_entry`, `timed_library`) as X9, and KR5 did not. The main session applied the user's words ("fake data will never appear in the real hydra") to both: a library row Hydra's scan did not make is made-up data. KR5's uncounted rows are listed in its file and still have to be added to its 25.

A fake analyzer is X9 too, including one that returns a real corpus result. The user, in session 63c70014: "why do we have a fake analyzer? what the fuck is the point of a fake analyzer? who came up with this shit?" The seam is in the product: `BatchJob::set_analyzer_for_test` (`src/ui/library_jobs.h:285`) and `set_app_batch_analyzer_for_test` (`:419`). Sessions added them to get around timing races in the GUI batch tests. Commit e6d81592 (2026-09-27, session 159155c8) added the job seam, and a33c5d71 (2026-10-03, session dadb77f5) added the app-wide one, which "holds the batch open". `gated_scores` (`test_app_library.cpp:204`), which makes up a score of 1000 per chart in turn, followed on 2026-11-05 (e87fa19d, session 08dcb02a). No ruling of the user asked for any of them.

**Both rounds, totalled.** Twenty-one more counters then re-read every Investigate record (`counts/T5-IR1.md` to `T5-IR21.md`), by the four rulings filed in `docs/agents/rulings/tests-and-runs.md`. They also followed the user's later choice that hand-built input fed to the real code is not fake. A fresh tallier read all 26 files under that one reading and corrected 55 marks (`counts/TOTALS.md`):

| Group | Records read | With a fault | X9 | X10 | X11 | X12 | Snapshot |
|---|---|---|---|---|---|---|---|
| Keep | 421 | 239 | 153 | 137 | 48 | 6 | 4 |
| Investigate | 2,028 | 1,622 | 1,299 | 826 | 201 | 129 | 42 |
| Both | 2,449 | 1,861 | 1,452 | 963 | 249 | 135 | 46 |

So 76% of the records the audit had not marked Rewrite or Remove hold at least one fault the user has ruled on. Made-up input (X9) is the most common.

The Investigate count is 2,028, not 1,970, because some verdicts sit inline on a check line. The tallier left about 40 records unsettled and lists them in `TOTALS.md`:
- hand-written database files in an old layout;
- a scanner that lives in its own test file;
- hand-written hook payloads.

They do not move the totals by more than about 2%.

This changes what the capture pass is for. It was meant to check each "claim not checked" record against a real run, then move it to Keep. But 1,622 of those records stand on fakes, typed constants or echoes. A real run would confirm a claim that the test itself still does not check. Only the 406 Investigate records with no fault can be moved to Keep by a capture.

## What the repeated-values pass left out of X3 to X12

- **Disputed:** RP3-10 ("chart" and "charts" are arguments passed to the product's `counted()`, not copies of its output) and RP3-6's `kDot`.
- **Not settled:** RP1-7, 48 kHz. Only `test_audio_decode.cpp:155` is clearly the product's rate (`kRate`, `src/audio/opus_reader.cpp:52`). Three places were not traced (V-RP1).
- **Pins in one file only, not repeats.** Each sits in a single test file. Each is a fault by X10, even in one file. RP1-6 lists six of them, among them `kReportConfirmAbove` 500, `kDefaultEarlyFillWindowMs` 60.0 and the source text of `kSqueezeWindowMs`. V-RP1 adds the backend cap 500 (`uitest_paths.cpp:408,410`) and `wal_autocheckpoint` 1000 (`test_store_open.cpp:1284,1292`; the owner is a bare literal at `src/store/record_store.cpp:1115`). V-RP2 adds four more:
  - the MIDI message-length limit text (`test_midi.cpp:524`);
  - the star cutoffs (`test_stars.cpp:222,259,341`);
  - the Library column widths (`test_column_widths.cpp:146,214`);
  - a default highway setting (`test_preview_config.cpp:62`).
- **Engine answers on charts the tests built.** 12960 is the engine's squeezed-out chord on a hand-built chart (`test_sp_meter_curve.cpp:464`, `test_sp_end.cpp:220`). Allies' best-path ticks are typed as pins in `test_path_costs.cpp` only (V-RP3, rows 164 and 144). Neither is a repeat across files.
- **Rows the verifiers did not re-open** are named in each V file. They stand on the sorter's reading.
