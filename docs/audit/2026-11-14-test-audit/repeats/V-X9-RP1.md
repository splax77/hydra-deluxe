agent a45ef3a60f32ca1df (verifier T5-V-X9-RP1, checking repeats/X9-RP1.md against the tests)

Plain version. I agree with the reader on the made-up records and made-up path text. I dispute the "typed number into a formatter or filter" X9 verdicts, because the test states the rule and the input beside the expected text, which is the reader's own definition of sound. The big gap is snapshots: about 20 places state a number that comes from one engine run and not from a rule, and the reader marked only 4. I did not open every one of the ~70 reader X9 places; the ones I opened are listed. Limits are at the end.

## The four places the reader did not trace

- `test_app_library.cpp:253` (`best_label().find("2,000")`): X9, same as 355 and 538. The "2,000" is the score of the record `gated_scores` (line 204-215) invents: `scored_record(1000 * turn)`, handed back as the analyzer's result. The test is about where rows sit during a batch, but the number on screen comes from a made-up record. Agree with row 13's X9.
- `test_activation_rows.cpp:558,560` (`rating == "Normal"`): sound. The comment at 522-524 states the rule ("every counted plain row reads Normal", D276 item 2), and the rows' offsets (2.5, 20.0 and so on) are typed beside it, so the expected text follows from the rule and the named inputs.
- `test_report_view.cpp:339`: sound. The rows carry the tier names as song names so the sort order can be read back; the expected order is the ladder (D284 comment, line 328). Caveat: the rows are `path_row(...)` hand-built rows, the same shape as row 10, so if the coordinator keeps row 10 as X9 this one is the same family.
- `test_preview_boxes.cpp:775` (`score_solo == 800`): sound. `two_solo_song()` (line 71) has two solos of 4 notes each (the test checks `notes == 4` twice at 742-743) and the same file's line 767 shows the engine's "+400" per solo, so 800 is the two named solos added up. Not a snapshot.

## Answer to the format_ms(163.0) question

Burnout's figure is typed by hand there, so it is an X1 copy of a real result. The check itself (`format_ms` gives "163.0 ms") is sound, since any number would do. The same 163.0 sits in `test_display_format.cpp:62` (`format_ms_whole(163.0)`), `test_model.cpp:973` (`shown_timing(163.0)`), `test_rules.cpp:197` (`backend_row_timing(-163.0,...)`) and `test_activation_rows.cpp:579` (inside the `format_measure` test). In all four the choice of 163.0 is arbitrary and copies Burnout; each is X1, not X9 and not snapshot.

## X9 places: agreed

- Row 1: `ready()` (`test_library_model.cpp:52`) builds a `RecordLookup` with a typed path text and score, and `gated_scores` (`test_app_library.cpp:204`) invents a stored record per chart. Agree.
- Rows 11, 16, 20 (the `ready(378315,"3- 1 2",7,163.0)` parts): agree. They are Burnout's real values typed into a made-up summary, and the path text is the same kind of string as the user's example.
- Row 24 `test_overlay_layout.cpp:342` (`wrap_words("  3- 1 2  (optimal)", ...)`): agree. This is the user's own example.
- Row 25 `put()` with notecount 1234 and "old-path-A": agree on the reader's description; I did not re-read `test_fill_report` (see limits).

## X9 places: disputed (I call them sound)

The reader's line was: a scalar into a formatter whose rule the expected text states is sound, but a struct holding a scalar is X9. I do not see the difference when the test states the rule and the input in the same lines. These tests check a rule on a number the test names.

- Row 2, `test_dynamics_breakdown.cpp:270-281`: Kick 1700 plus Kick2x 300 gives "300 of 2,000 kick notes (15%)". The comment at 270-271 states the rule (the noun comes from the total), and 300 of 2000 is 15 percent. Sound.
- Rows 30, 52, 54 on `test_activation_rows.cpp:743-757` and `:965-984`, and `test_preview_boxes.cpp:965-974`: hand-set `SPSqueeze` offsets and expected text "squeeze in 12.6 ms", "20.0 ms late", "0+ (100.0 ms)". The comments state the rule (one decimal, D265; SqIn late/early wording). The 307.7 in row 52 is 400 * 2 / (1.6 + 1), a rule applied to the named inputs. Sound.
- Rows 4, 54, 69, `test_library_query.cpp:343-362,421-427`: `analyzed(7, 20.0)` against `squeeze>20` and so on. The number is the boundary of the filter the test names. Sound for the boundary cases. The 163.0 in `:393-397` stays X1 (its own comment says "Burnout's real 163.0 ms"). A judgement call: if the coordinator wants no hand-built RecordLookup anywhere, these stay X9.
- Row 10 `path_row(...,500.0,...)` at `test_report_view.cpp:450-451`: same family; a boundary of the filter. Sound on the same reasoning, same caveat.

## Snapshots: agreed

- `test_preview_boxes.cpp:520`: `score_solo == 200`. The number is two solo notes of 100, so it is derivable, but the comment at 515-516 admits "pinned from one run" and no rule is stated. Snapshot. Line 706 is the same number with a comment that names the two one-note chords and a `+200` check at 730; I call 706 derivable, not snapshot.
- `test_audio_player.cpp:139` (`frames_of_ms(1.0, 44100) == 44  // pinned from one run`): agree, with the note that 44.1 rounded is the rule.

## Snapshots: added (the reader missed them)

The test's own comment says "pinned from one run", or the number is a real-chart result with no rule beside it. File:line, value, verdict.

- `test_preview_boxes.cpp:294` `steps[1].total == 180` (comment 291-292: "pinned from one run as a fixed oracle"): snapshot. Also 494-495 `"star 1 in 20"`, 507-509 `end.stars == 7` on a corpus chart, and 516-533 the literals `"180"`, `"star 2 in 10"` and cutoff 380 (comment at 515 says so): snapshot.
- `test_replay.cpp:190-200` chord totals 150, 50, 300, 350, 500 (comment "Pinned from one run"): snapshot. `:107` `min_paths` floors ("a floor pinned from one run"): snapshot. `:312-326` `skipped` 0, 0, 4 and `min_variants` 10, 200, 200 ("pinned exactly"): snapshot. `test_replay_schedule.cpp:1436` Pitch Dark chord 243's end 100949.4546 ("pinned from one run of this test"): snapshot.
- `test_model.cpp:88-99` the `category_scores` rows `{0,50,180},{8,50,310},...` (comment "pinned from one run of category_scores"), and the chord-score table literals near `:1070` ("Every value is a literal pinned from one run"): snapshot.
- `test_tied_variants.cpp:347` `totalscore() == 2650  // pinned from one run`: snapshot.
- `test_fills_and_skips.cpp:350-352` (chart "Missed Injections", activation at 123838, two fills) and `:446-448` (eight of nine fills at 55680, 71040, ...): snapshot (both say "pinned from one run").
- `test_dynamics_breakdown.cpp:434-` every row's counts on The Decade Of Statues, "pinned from one run": snapshot. Also `:303` Acid Romance's kick counts: snapshot of a real chart.
- `test_generated_fills.cpp:191,245` the per-chart hash and count table ("Pinned from one run of the code"): snapshot.
- `test_replay_schedule.cpp:965` the overfill shape's literals ("pinned before F1"): snapshot.
- `test_sp_meter_curve.cpp:950-990` Burnout and Pitch Dark expected ticks 59520, 71040, 70800, 48783.075, 48620.0315 and the Pitch Dark sp_end steps: snapshot of the record "as PS0b saved it" (an earlier hydra_replay run). The user's words behind it name behaviour, not these numbers.
- `test_activation_rows.cpp:585-635` Burnout's three activations: best path "3- 1 2" and 378315, `m32.1.0`, `m58.1.0`, `m88.1.0`, "3 bars", `[Kick - Green cymbal]`, "260 fewer points", "3 / 6 / 7 notes near the SP end", `squeezed_out == 1`: snapshot (real-chart results typed, no rule beside them). The reader marked only 586/587/601.

## Sound verdicts I moved or confirmed

- `test_sp_end_clamp.cpp`: confirmed sound. The numbers 5376, 6528, 3456 and -250 come from stated arithmetic in the comments at 28-48 and 86-99. Not a snapshot.
- `test_note_window.cpp:72-75,143-147,175-183,325-393` `pinned(...)` literals: the comments say they come from "TM-21's worked values" and a mockup, not an engine run. I did not open that plan or mockup, so this stays sound but unclear. Not moved.
- `test_stars.cpp:250-260` (130 to 5720 are the percent steps of 1300) and `test_cli.cpp:704-707` (ticks of the chart): confirmed sound.
- Moved: `test_report_view.cpp:339`, `test_activation_rows.cpp:558,560` and `test_preview_boxes.cpp:775` from unclear or probable snapshot to sound; `test_preview_boxes.cpp:706` from snapshot to derivable.

## Counts

- Reader X9 places agreed: those in rows 1, 11, 13, 16, 20, 24, 25 that I opened (about 14 of about 70). Disputed as sound: rows 2, 4, 10, 30, 52, 54, 69 (about 25 places). Not opened: the rest of the X9 list (`test_app_click.cpp:1069`, `test_chart_index.cpp:100,127`, `test_path_list.cpp:55-61`, `test_fill_report.cpp:62-70,139`, `test_library_model.cpp:176,426,720-859`, `test_activation_rows.cpp:364-402,500-515`).
- Reader snapshots: 520 and 139 agreed; 706 and 775 moved away.
- Snapshots added: 12 groups, about 45 literals.
- Four untraced places settled: 253 X9, 558/560 sound, 339 sound, 775 sound.

## Limits

I did not open every place of the other rows the reader called sound (rows 3, 5-9, 12, 14, 15, 17-19, 21-23, 26-29, 31-36, 38-40, 42-44, 46-49, 53, 55-60, 62-68). I looked only at sp_end_clamp, sp_meter_curve, test_stars, test_cli and the note_window comments, plus a grep for "pinned", "oracle" and "from one run" over the engine-on-hand-chart files (preview_boxes, sp_meter_curve, replay, sp_end_clamp, stars, model, note_window, replay_schedule, generated_fills, tied_variants, fills_and_skips, dynamics_breakdown, activation_rows, cli). A snapshot with no such comment, or in a file outside that list, would not be found. I did not re-derive expected numbers by running the engine, since I run nothing.
