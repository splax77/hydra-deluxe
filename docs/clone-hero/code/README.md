# Annotated game code: the index

This page lists every annotated listing of Clone Hero's code that the reference pages cite, so a row's evidence can be found by its address.
A listing is the game's machine code for one function, turned back into readable instructions, one instruction per line.

New listings made for the reference live under `code/6142/` (build 1.1.0.6142) and `code/4045/` (build 1.0.0.4045), and `facts-6142.tsv` and `facts-4045.tsv` beside them are the single home of the game facts the pages cite.

## Listings that stay where they are

The 210 listings below were committed before the reference existed, by the Note Shuffle work under `docs/audit/note-shuffle/` and the drum-lane work under `docs/audit/drum-lanes/`. They stay in those folders, so the links in their own FINDINGS and plans keep working, and this table is the one place that says which file holds which address.

Every one is from build 1.1.0.6142 (`GameAssembly.dll`). They were made with a capstone wrapper (capstone is a disassembler library) whose first copy lived in a scratch folder and is gone. The same wrapper is now committed as tools/ch_mcp/chdis.py, and it reproduces them: run on 0x20F0B30, it printed the 69 lines of `lanedraw_20f0b30.asm` exactly (checked 2026-10-30). The date is the day git first added the file.

The first column is the function start, read from the listing's own first line. A file that holds several functions lists each start, in the order the file holds them. The role is the name the scout gave the file; where the file name carries no role, the folder's README or FINDINGS says what the function does. Several functions were listed twice, once by the scout that read them and once by the scout that tried to refute the reading (the `-refute` folders), so one address can have two or three rows. The table was generated from the files, not typed, and its 210 rows match the 210 `.asm` files in the two folders.

| Start | Role | File | Build | Date |
|---|---|---|---|---|
| 0x100EAE0 | generic ctor | [docs/audit/note-shuffle/step0/open34-refute/generic_ctor_100eae0.asm](../../audit/note-shuffle/step0/open34-refute/generic_ctor_100eae0.asm) | 6142 | 2026-10-09 |
| 0x100EAE0 | engine generic ctor | [docs/audit/note-shuffle/step0/open34/engine_generic_ctor_100eae0.asm](../../audit/note-shuffle/step0/open34/engine_generic_ctor_100eae0.asm) | 6142 | 2026-10-09 |
| 0x208870 | baseplayer | [docs/audit/note-shuffle/step0/open34-refute/baseplayer_208870.asm](../../audit/note-shuffle/step0/open34-refute/baseplayer_208870.asm) | 6142 | 2026-10-09 |
| 0x208870 | baseplayer | [docs/audit/note-shuffle/step0/open34/baseplayer_208870.asm](../../audit/note-shuffle/step0/open34/baseplayer_208870.asm) | 6142 | 2026-10-09 |
| 0x20CEB90 | midi sink | [docs/audit/note-shuffle/step0/open1/midi_sink_20CEB90.asm](../../audit/note-shuffle/step0/open1/midi_sink_20CEB90.asm) | 6142 | 2026-10-09 |
| 0x20CEC60 | midi sink | [docs/audit/note-shuffle/step0/open1-refute/midi_sink_20CEC60.asm](../../audit/note-shuffle/step0/open1-refute/midi_sink_20CEC60.asm) | 6142 | 2026-10-09 |
| 0x20CEC60 | midi sink | [docs/audit/note-shuffle/step0/open1/midi_sink_20CEC60.asm](../../audit/note-shuffle/step0/open1/midi_sink_20CEC60.asm) | 6142 | 2026-10-09 |
| 0x20CEC60 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f20CEC60.asm](../../audit/note-shuffle/step0/open2-refute/f20CEC60.asm) | 6142 | 2026-10-09 |
| 0x20CEC60 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f20cec60.asm](../../audit/note-shuffle/step0/open2/f20cec60.asm) | 6142 | 2026-10-09 |
| 0x20CEDC0 | midi sink | [docs/audit/note-shuffle/step0/open1/midi_sink_20CEDC0.asm](../../audit/note-shuffle/step0/open1/midi_sink_20CEDC0.asm) | 6142 | 2026-10-09 |
| 0x20CEEC0 | midi sink | [docs/audit/note-shuffle/step0/open1/midi_sink_20CEEC0.asm](../../audit/note-shuffle/step0/open1/midi_sink_20CEEC0.asm) | 6142 | 2026-10-09 |
| 0x20CF0B0 | midi sink | [docs/audit/note-shuffle/step0/open1/midi_sink_20CF0B0.asm](../../audit/note-shuffle/step0/open1/midi_sink_20CF0B0.asm) | 6142 | 2026-10-09 |
| 0x20CF1B0 | midi sink | [docs/audit/note-shuffle/step0/open1/midi_sink_20CF1B0.asm](../../audit/note-shuffle/step0/open1/midi_sink_20CF1B0.asm) | 6142 | 2026-10-09 |
| 0x20CFB80 | sink | [docs/audit/note-shuffle/step0/open1/sink_20CFB80.asm](../../audit/note-shuffle/step0/open1/sink_20CFB80.asm) | 6142 | 2026-10-09 |
| 0x20CFBC0 | midi sink | [docs/audit/note-shuffle/step0/open1/midi_sink_20CFBC0.asm](../../audit/note-shuffle/step0/open1/midi_sink_20CFBC0.asm) | 6142 | 2026-10-09 |
| 0x20CFD40 | sink | [docs/audit/note-shuffle/step0/open1/sink_20CFD40.asm](../../audit/note-shuffle/step0/open1/sink_20CFD40.asm) | 6142 | 2026-10-09 |
| 0x20D12C0 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/f20d12c0.asm](../../audit/drum-lanes/f20d12c0.asm) | 6142 | 2026-10-10 |
| 0x20D2EA0 | build | [docs/audit/note-shuffle/scout-flag/build_20d2ea0.asm](../../audit/note-shuffle/scout-flag/build_20d2ea0.asm) | 6142 | 2026-10-09 |
| 0x20D2EA0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/scout-strings/f20D2EA0.asm](../../audit/note-shuffle/scout-strings/f20D2EA0.asm) | 6142 | 2026-10-09 |
| 0x20D2EA0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f20D2EA0.asm](../../audit/note-shuffle/step0/open2-refute/f20D2EA0.asm) | 6142 | 2026-10-09 |
| 0x20D2EA0 | build | [docs/audit/note-shuffle/step0/open2/f20d2ea0_build.asm](../../audit/note-shuffle/step0/open2/f20d2ea0_build.asm) | 6142 | 2026-10-09 |
| 0x20D2EA0 | builder | [docs/audit/note-shuffle/step0/open34-refute/builder_20d2ea0.asm](../../audit/note-shuffle/step0/open34-refute/builder_20d2ea0.asm) | 6142 | 2026-10-09 |
| 0x20D2EA0 | builder | [docs/audit/note-shuffle/step0/open34/builder_20d2ea0.asm](../../audit/note-shuffle/step0/open34/builder_20d2ea0.asm) | 6142 | 2026-10-09 |
| 0x20D36A0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/scout-strings/f20D36A0.asm](../../audit/note-shuffle/scout-strings/f20D36A0.asm) | 6142 | 2026-10-09 |
| 0x20D36A0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f20D36A0.asm](../../audit/note-shuffle/step0/open2-refute/f20D36A0.asm) | 6142 | 2026-10-09 |
| 0x20D36A0 | lanemap | [docs/audit/note-shuffle/step0/open2/f20d36a0_lanemap.asm](../../audit/note-shuffle/step0/open2/f20d36a0_lanemap.asm) | 6142 | 2026-10-09 |
| 0x20D36A0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f20d36a0.asm](../../audit/note-shuffle/step0/open2/f20d36a0.asm) | 6142 | 2026-10-09 |
| 0x20D4BE0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/scout-strings/f20D4BE0.asm](../../audit/note-shuffle/scout-strings/f20D4BE0.asm) | 6142 | 2026-10-09 |
| 0x20D4BE0 | build gamenote | [docs/audit/note-shuffle/step0/open1-refute/build_gamenote_20D4BE0.asm](../../audit/note-shuffle/step0/open1-refute/build_gamenote_20D4BE0.asm) | 6142 | 2026-10-09 |
| 0x20D4BE0 | build gamenote | [docs/audit/note-shuffle/step0/open1/build_gamenote_20D4BE0.asm](../../audit/note-shuffle/step0/open1/build_gamenote_20D4BE0.asm) | 6142 | 2026-10-09 |
| 0x20D4BE0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f20D4BE0.asm](../../audit/note-shuffle/step0/open2-refute/f20D4BE0.asm) | 6142 | 2026-10-09 |
| 0x20D4BE0 | notelist | [docs/audit/note-shuffle/step0/open2/f20d4be0_notelist.asm](../../audit/note-shuffle/step0/open2/f20d4be0_notelist.asm) | 6142 | 2026-10-09 |
| 0x20D51D0 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/f20d51d0.asm](../../audit/drum-lanes/f20d51d0.asm) | 6142 | 2026-10-10 |
| 0x20D5310 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/f20d5310.asm](../../audit/drum-lanes/f20d5310.asm) | 6142 | 2026-10-10 |
| 0x20DCAD0 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/f20dcad0.asm](../../audit/drum-lanes/f20dcad0.asm) | 6142 | 2026-10-10 |
| 0x20DDC80 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/play/f20ddc80.asm](../../audit/drum-lanes/play/f20ddc80.asm) | 6142 | 2026-10-10 |
| 0x20DDEC0 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/play/f20ddec0.asm](../../audit/drum-lanes/play/f20ddec0.asm) | 6142 | 2026-10-10 |
| 0x20DE070 | poll input | [docs/audit/drum-lanes/play/f20de070_poll_input.asm](../../audit/drum-lanes/play/f20de070_poll_input.asm) | 6142 | 2026-10-10 |
| 0x20DE420 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/f20de420.asm](../../audit/drum-lanes/f20de420.asm) | 6142 | 2026-10-10 |
| 0x20DE5B0 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/play/f20de5b0.asm](../../audit/drum-lanes/play/f20de5b0.asm) | 6142 | 2026-10-10 |
| 0x20DE770 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/f20de770.asm](../../audit/drum-lanes/f20de770.asm) | 6142 | 2026-10-10 |
| 0x20DE9A0 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/play/f20de9a0.asm](../../audit/drum-lanes/play/f20de9a0.asm) | 6142 | 2026-10-10 |
| 0x20DEAA0 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/play/f20deaa0.asm](../../audit/drum-lanes/play/f20deaa0.asm) | 6142 | 2026-10-10 |
| 0x20DEEC0 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/play/f20deec0.asm](../../audit/drum-lanes/play/f20deec0.asm) | 6142 | 2026-10-10 |
| 0x20DEEC0 | drum score | [docs/audit/note-shuffle/scout-flag/drum_score_20deec0.asm](../../audit/note-shuffle/scout-flag/drum_score_20deec0.asm) | 6142 | 2026-10-09 |
| 0x20DF680 | drums engine ctor | [docs/audit/note-shuffle/step0/open34-refute/drums_engine_ctor_20df680.asm](../../audit/note-shuffle/step0/open34-refute/drums_engine_ctor_20df680.asm) | 6142 | 2026-10-09 |
| 0x20E1DA0 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/f20e1da0.asm](../../audit/drum-lanes/f20e1da0.asm) | 6142 | 2026-10-10 |
| 0x20E34F0 | stream class | [docs/audit/note-shuffle/step0/open1-refute/stream_class_20E34F0.asm](../../audit/note-shuffle/step0/open1-refute/stream_class_20E34F0.asm) | 6142 | 2026-10-09 |
| 0x20E34F0, 0x20E3510, 0x20E36F0, 0x20E39A0, 0x20E3C20, 0x20E4040, 0x20E4200, 0x20E45C0, 0x20E49D0, 0x20E4BF0, 0x20E4E90, 0x20E5130, 0x20E53D0, 0x20E5940, 0x20E5BA0, 0x20E5D60, 0x20E5E70, 0x20E6160, 0x20E6680, 0x20E69A0, 0x20E6DD0, 0x20E6EA0 | stream class all | [docs/audit/note-shuffle/step0/open1-refute/stream_class_all.asm](../../audit/note-shuffle/step0/open1-refute/stream_class_all.asm) | 6142 | 2026-10-09 |
| 0x20E45C0 | stream note | [docs/audit/note-shuffle/step0/open1-refute/stream_note_20E45C0.asm](../../audit/note-shuffle/step0/open1-refute/stream_note_20E45C0.asm) | 6142 | 2026-10-09 |
| 0x20E45C0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f20e45c0.asm](../../audit/note-shuffle/step0/open2/f20e45c0.asm) | 6142 | 2026-10-09 |
| 0x20E4E90 | stream res | [docs/audit/note-shuffle/step0/open1-refute/stream_res_20E4E90.asm](../../audit/note-shuffle/step0/open1-refute/stream_res_20E4E90.asm) | 6142 | 2026-10-09 |
| 0x20E4E90 | loader | [docs/audit/note-shuffle/step0/open1/loader_20E4E90.asm](../../audit/note-shuffle/step0/open1/loader_20E4E90.asm) | 6142 | 2026-10-09 |
| 0x20E5130 | loader | [docs/audit/note-shuffle/step0/open1/loader_20E5130.asm](../../audit/note-shuffle/step0/open1/loader_20E5130.asm) | 6142 | 2026-10-09 |
| 0x20E53D0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f20e53d0.asm](../../audit/note-shuffle/step0/open2/f20e53d0.asm) | 6142 | 2026-10-09 |
| 0x20E6160 | loader | [docs/audit/note-shuffle/step0/open1/loader_20E6160.asm](../../audit/note-shuffle/step0/open1/loader_20E6160.asm) | 6142 | 2026-10-09 |
| 0x20E69A0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f20e69a0.asm](../../audit/note-shuffle/step0/open2/f20e69a0.asm) | 6142 | 2026-10-09 |
| 0x20E8D30 | struct ctor | [docs/audit/note-shuffle/step0/open34-refute/struct_ctor_20e8d30.asm](../../audit/note-shuffle/step0/open34-refute/struct_ctor_20e8d30.asm) | 6142 | 2026-10-09 |
| 0x20E8D30 | result struct ctor | [docs/audit/note-shuffle/step0/open34/result_struct_ctor_20e8d30.asm](../../audit/note-shuffle/step0/open34/result_struct_ctor_20e8d30.asm) | 6142 | 2026-10-09 |
| 0x20EF450, 0x20EFF60, 0x20F0320, 0x20F1420, 0x20F16E0, 0x20EFEE0 | drum pretransforms | [docs/audit/note-shuffle/scout-flag/drum_pretransforms.asm](../../audit/note-shuffle/scout-flag/drum_pretransforms.asm) | 6142 | 2026-10-09 |
| 0x20EF450, 0x20EFF60, 0x20F16E0, 0x20EFEE0, 0x20F1420 | drum pre | [docs/audit/note-shuffle/scout-strings/drum_pre.asm](../../audit/note-shuffle/scout-strings/drum_pre.asm) | 6142 | 2026-10-09 |
| 0x20EF450 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f20EF450.asm](../../audit/note-shuffle/step0/open2-refute/f20EF450.asm) | 6142 | 2026-10-09 |
| 0x20EF450, 0x20EFF60, 0x20F0320, 0x20F1420, 0x20F16E0, 0x20EFEE0 | drum pretransforms | [docs/audit/note-shuffle/step0/open2/drum_pretransforms_20ef450_20eff60_20f0320_20f1420_20f16e0_20efee0.asm](../../audit/note-shuffle/step0/open2/drum_pretransforms_20ef450_20eff60_20f0320_20f1420_20f16e0_20efee0.asm) | 6142 | 2026-10-09 |
| 0x20EFEE0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f20EFEE0.asm](../../audit/note-shuffle/step0/open2-refute/f20EFEE0.asm) | 6142 | 2026-10-09 |
| 0x20EFF60 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f20EFF60.asm](../../audit/note-shuffle/step0/open2-refute/f20EFF60.asm) | 6142 | 2026-10-09 |
| 0x20F0320 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/scout-strings/f20F0320.asm](../../audit/note-shuffle/scout-strings/f20F0320.asm) | 6142 | 2026-10-09 |
| 0x20F0B30 | lambda | [docs/audit/note-shuffle/scout-flag/lambda_20f0b30.asm](../../audit/note-shuffle/scout-flag/lambda_20f0b30.asm) | 6142 | 2026-10-09 |
| 0x20F0B30 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/scout-strings/f20F0B30.asm](../../audit/note-shuffle/scout-strings/f20F0B30.asm) | 6142 | 2026-10-09 |
| 0x20F0B30 | lanedraw | [docs/audit/note-shuffle/step0/open34-refute/lanedraw_20f0b30.asm](../../audit/note-shuffle/step0/open34-refute/lanedraw_20f0b30.asm) | 6142 | 2026-10-09 |
| 0x20F0B30 | lanedraw | [docs/audit/note-shuffle/step0/open34/lanedraw_20f0b30.asm](../../audit/note-shuffle/step0/open34/lanedraw_20f0b30.asm) | 6142 | 2026-10-09 |
| 0x20F0C30 | shuffle | [docs/audit/note-shuffle/scout-flag/shuffle_20f0c30.asm](../../audit/note-shuffle/scout-flag/shuffle_20f0c30.asm) | 6142 | 2026-10-09 |
| 0x20F0C30 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/scout-strings/f20F0C30.asm](../../audit/note-shuffle/scout-strings/f20F0C30.asm) | 6142 | 2026-10-09 |
| 0x20F0C30 | shuffle | [docs/audit/note-shuffle/step0/open1-refute/shuffle_20F0C30.asm](../../audit/note-shuffle/step0/open1-refute/shuffle_20F0C30.asm) | 6142 | 2026-10-09 |
| 0x20F0C30 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f20F0C30.asm](../../audit/note-shuffle/step0/open2-refute/f20F0C30.asm) | 6142 | 2026-10-09 |
| 0x20F0C30 | shuffle | [docs/audit/note-shuffle/step0/open2/f20f0c30_shuffle.asm](../../audit/note-shuffle/step0/open2/f20f0c30_shuffle.asm) | 6142 | 2026-10-09 |
| 0x20F0C30 | shuffle | [docs/audit/note-shuffle/step0/open34-refute/shuffle_20f0c30.asm](../../audit/note-shuffle/step0/open34-refute/shuffle_20f0c30.asm) | 6142 | 2026-10-09 |
| 0x20F0C30 | shuffle | [docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm](../../audit/note-shuffle/step0/open34/shuffle_20f0c30.asm) | 6142 | 2026-10-09 |
| 0x20F1420 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f20F1420.asm](../../audit/note-shuffle/step0/open2-refute/f20F1420.asm) | 6142 | 2026-10-09 |
| 0x20F16E0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f20F16E0.asm](../../audit/note-shuffle/step0/open2-refute/f20F16E0.asm) | 6142 | 2026-10-09 |
| 0x20F2720 | gm writer | [docs/audit/note-shuffle/step0/open1-refute/gm_writer_20F2720.asm](../../audit/note-shuffle/step0/open1-refute/gm_writer_20F2720.asm) | 6142 | 2026-10-09 |
| 0x20F3610 | base miss | [docs/audit/drum-lanes/play/f20f3610_base_miss.asm](../../audit/drum-lanes/play/f20f3610_base_miss.asm) | 6142 | 2026-10-10 |
| 0x20F43A0 | add event | [docs/audit/drum-lanes/play/f20f43a0_add_event.asm](../../audit/drum-lanes/play/f20f43a0_add_event.asm) | 6142 | 2026-10-10 |
| 0x20F4680 | starbase | [docs/audit/note-shuffle/step0/open34-refute/starbase_20f4680.asm](../../audit/note-shuffle/step0/open34-refute/starbase_20f4680.asm) | 6142 | 2026-10-09 |
| 0x20F4680 | starbase | [docs/audit/note-shuffle/step0/open34/starbase_20f4680.asm](../../audit/note-shuffle/step0/open34/starbase_20f4680.asm) | 6142 | 2026-10-09 |
| 0x20F4950 | step loop | [docs/audit/drum-lanes/play/f20f4950_step_loop.asm](../../audit/drum-lanes/play/f20f4950_step_loop.asm) | 6142 | 2026-10-10 |
| 0x20F4B60 | combo break | [docs/audit/drum-lanes/play/f20f4b60_combo_break.asm](../../audit/drum-lanes/play/f20f4b60_combo_break.asm) | 6142 | 2026-10-10 |
| 0x20F5490 | deactivate sp | [docs/audit/drum-lanes/play/f20f5490_deactivate_sp.asm](../../audit/drum-lanes/play/f20f5490_deactivate_sp.asm) | 6142 | 2026-10-10 |
| 0x20F5540 | slot9 base and activate sp | [docs/audit/drum-lanes/play/f20f5540_slot9_base_and_activate_sp.asm](../../audit/drum-lanes/play/f20f5540_slot9_base_and_activate_sp.asm) | 6142 | 2026-10-10 |
| 0x20F5810 | base update | [docs/audit/drum-lanes/play/f20f5810_base_update.asm](../../audit/drum-lanes/play/f20f5810_base_update.asm) | 6142 | 2026-10-10 |
| 0x20F63F0 | base hit | [docs/audit/drum-lanes/play/f20f63f0_base_hit.asm](../../audit/drum-lanes/play/f20f63f0_base_hit.asm) | 6142 | 2026-10-10 |
| 0x20F6A50 | advance schedule | [docs/audit/drum-lanes/play/f20f6a50_advance_schedule.asm](../../audit/drum-lanes/play/f20f6a50_advance_schedule.asm) | 6142 | 2026-10-10 |
| 0x20F6FF0 | next event | [docs/audit/drum-lanes/play/f20f6ff0_next_event.asm](../../audit/drum-lanes/play/f20f6ff0_next_event.asm) | 6142 | 2026-10-10 |
| 0x20F79E0 | engine ctor | [docs/audit/note-shuffle/step0/open34-refute/engine_ctor_20f79e0.asm](../../audit/note-shuffle/step0/open34-refute/engine_ctor_20f79e0.asm) | 6142 | 2026-10-09 |
| 0x20F79E0 | engine ctor | [docs/audit/note-shuffle/step0/open34/engine_ctor_20f79e0.asm](../../audit/note-shuffle/step0/open34/engine_ctor_20f79e0.asm) | 6142 | 2026-10-09 |
| 0x210D7B0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f210D7B0.asm](../../audit/note-shuffle/step0/open2-refute/f210D7B0.asm) | 6142 | 2026-10-09 |
| 0x210D7B0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f210d7b0.asm](../../audit/note-shuffle/step0/open2/f210d7b0.asm) | 6142 | 2026-10-09 |
| 0x210D9C0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f210d9c0.asm](../../audit/note-shuffle/step0/open2/f210d9c0.asm) | 6142 | 2026-10-09 |
| 0x2116390 | lane ctor | [docs/audit/drum-lanes/lane_ctor_2116390.asm](../../audit/drum-lanes/lane_ctor_2116390.asm) | 6142 | 2026-10-10 |
| 0x211AF60 | mthd parse | [docs/audit/note-shuffle/step0/open1-refute/mthd_parse_211AF60.asm](../../audit/note-shuffle/step0/open1-refute/mthd_parse_211AF60.asm) | 6142 | 2026-10-09 |
| 0x212B9C0 | tick to sec | [docs/audit/note-shuffle/step0/open1-refute/tick_to_sec_212B9C0.asm](../../audit/note-shuffle/step0/open1-refute/tick_to_sec_212B9C0.asm) | 6142 | 2026-10-09 |
| 0x212C0B0 | song set res | [docs/audit/note-shuffle/step0/open1-refute/song_set_res_212C0B0.asm](../../audit/note-shuffle/step0/open1-refute/song_set_res_212C0B0.asm) | 6142 | 2026-10-09 |
| 0x212C0B0 | song set resolution | [docs/audit/note-shuffle/step0/open1/song_set_resolution_212C0B0.asm](../../audit/note-shuffle/step0/open1/song_set_resolution_212C0B0.asm) | 6142 | 2026-10-09 |
| 0x212E3D0 | song cctor | [docs/audit/note-shuffle/step0/open1/song_cctor_212E3D0.asm](../../audit/note-shuffle/step0/open1/song_cctor_212E3D0.asm) | 6142 | 2026-10-09 |
| 0x212E990 | song ctor | [docs/audit/note-shuffle/step0/open1/song_ctor_212E990.asm](../../audit/note-shuffle/step0/open1/song_ctor_212E990.asm) | 6142 | 2026-10-09 |
| 0x213AFC0 | chart line parse | [docs/audit/note-shuffle/step0/open1-refute/chart_line_parse_213AFC0.asm](../../audit/note-shuffle/step0/open1-refute/chart_line_parse_213AFC0.asm) | 6142 | 2026-10-09 |
| 0x213AFC0 | chart line parse | [docs/audit/note-shuffle/step0/open1/chart_line_parse_213AFC0.asm](../../audit/note-shuffle/step0/open1/chart_line_parse_213AFC0.asm) | 6142 | 2026-10-09 |
| 0x213D2D0 | trackbuilder dispatch | [docs/audit/note-shuffle/step0/open1-refute/trackbuilder_dispatch_213D2D0.asm](../../audit/note-shuffle/step0/open1-refute/trackbuilder_dispatch_213D2D0.asm) | 6142 | 2026-10-09 |
| 0x213D2D0 | trackbuilder dispatch | [docs/audit/note-shuffle/step0/open1/trackbuilder_dispatch_213D2D0.asm](../../audit/note-shuffle/step0/open1/trackbuilder_dispatch_213D2D0.asm) | 6142 | 2026-10-09 |
| 0x213D2D0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f213d2d0.asm](../../audit/note-shuffle/step0/open2/f213d2d0.asm) | 6142 | 2026-10-09 |
| 0x213D620 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/scout-strings/f213D620.asm](../../audit/note-shuffle/scout-strings/f213D620.asm) | 6142 | 2026-10-09 |
| 0x213D620 | trackbuilder addnote | [docs/audit/note-shuffle/step0/open1-refute/trackbuilder_addnote_213D620.asm](../../audit/note-shuffle/step0/open1-refute/trackbuilder_addnote_213D620.asm) | 6142 | 2026-10-09 |
| 0x213D620 | trackbuilder addnote | [docs/audit/note-shuffle/step0/open1/trackbuilder_addnote_213D620.asm](../../audit/note-shuffle/step0/open1/trackbuilder_addnote_213D620.asm) | 6142 | 2026-10-09 |
| 0x213D620 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f213D620.asm](../../audit/note-shuffle/step0/open2-refute/f213D620.asm) | 6142 | 2026-10-09 |
| 0x213D620 | chart N line | [docs/audit/note-shuffle/step0/open2/f213d620_chart_N_line.asm](../../audit/note-shuffle/step0/open2/f213d620_chart_N_line.asm) | 6142 | 2026-10-09 |
| 0x213DD40 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f213dd40.asm](../../audit/note-shuffle/step0/open2/f213dd40.asm) | 6142 | 2026-10-09 |
| 0x213E0C0 | spec | [docs/audit/drum-lanes/spec_213e0c0.asm](../../audit/drum-lanes/spec_213e0c0.asm) | 6142 | 2026-10-10 |
| 0x214B640 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f214B640.asm](../../audit/note-shuffle/step0/open2-refute/f214B640.asm) | 6142 | 2026-10-09 |
| 0x214B800 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f214B800.asm](../../audit/note-shuffle/step0/open2-refute/f214B800.asm) | 6142 | 2026-10-09 |
| 0x214B850 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f214B850.asm](../../audit/note-shuffle/step0/open2-refute/f214B850.asm) | 6142 | 2026-10-09 |
| 0x214BCD0, 0x20FE7E0, 0x20FC610, 0x20FC5F0, 0x20EA810 | helpers | [docs/audit/note-shuffle/scout-flag/helpers.asm](../../audit/note-shuffle/scout-flag/helpers.asm) | 6142 | 2026-10-09 |
| 0x214BCD0, 0x214B840, 0x20FE7E0, 0x20FC610, 0x20FC5F0 | helpers | [docs/audit/note-shuffle/scout-strings/helpers.asm](../../audit/note-shuffle/scout-strings/helpers.asm) | 6142 | 2026-10-09 |
| 0x214BCD0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f214BCD0.asm](../../audit/note-shuffle/step0/open2-refute/f214BCD0.asm) | 6142 | 2026-10-09 |
| 0x214BCD0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f214bcd0.asm](../../audit/note-shuffle/step0/open2/f214bcd0.asm) | 6142 | 2026-10-09 |
| 0x214BE30 | cymbal check | [docs/audit/note-shuffle/step0/open34-refute/cymbal_check_214be30.asm](../../audit/note-shuffle/step0/open34-refute/cymbal_check_214be30.asm) | 6142 | 2026-10-09 |
| 0x214BE70 | gamenote clone | [docs/audit/note-shuffle/step0/open1-refute/gamenote_clone_214BE70.asm](../../audit/note-shuffle/step0/open1-refute/gamenote_clone_214BE70.asm) | 6142 | 2026-10-09 |
| 0x214BE70 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f214BE70.asm](../../audit/note-shuffle/step0/open2-refute/f214BE70.asm) | 6142 | 2026-10-09 |
| 0x214BE70 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f214be70.asm](../../audit/note-shuffle/step0/open2/f214be70.asm) | 6142 | 2026-10-09 |
| 0x214BF10 | gamenote ctor | [docs/audit/note-shuffle/step0/open1-refute/gamenote_ctor_214BF10.asm](../../audit/note-shuffle/step0/open1-refute/gamenote_ctor_214BF10.asm) | 6142 | 2026-10-09 |
| 0x214BF10 | gamenote ctor | [docs/audit/note-shuffle/step0/open1/gamenote_ctor_214BF10.asm](../../audit/note-shuffle/step0/open1/gamenote_ctor_214BF10.asm) | 6142 | 2026-10-09 |
| 0x214BF10 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f214bf10.asm](../../audit/note-shuffle/step0/open2/f214bf10.asm) | 6142 | 2026-10-09 |
| 0x214C9E0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f214C9E0.asm](../../audit/note-shuffle/step0/open2-refute/f214C9E0.asm) | 6142 | 2026-10-09 |
| 0x214C9E0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f214c9e0.asm](../../audit/note-shuffle/step0/open2/f214c9e0.asm) | 6142 | 2026-10-09 |
| 0x214CEE0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f214cee0.asm](../../audit/note-shuffle/step0/open2/f214cee0.asm) | 6142 | 2026-10-09 |
| 0x214D240 | songnote init | [docs/audit/note-shuffle/step0/open1-refute/songnote_init_214D240.asm](../../audit/note-shuffle/step0/open1-refute/songnote_init_214D240.asm) | 6142 | 2026-10-09 |
| 0x214D240 | songnote init | [docs/audit/note-shuffle/step0/open1/songnote_init_214D240.asm](../../audit/note-shuffle/step0/open1/songnote_init_214D240.asm) | 6142 | 2026-10-09 |
| 0x214D2C0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f214D2C0.asm](../../audit/note-shuffle/step0/open2-refute/f214D2C0.asm) | 6142 | 2026-10-09 |
| 0x214D2C0, 0x214DCA0, 0x214D580, 0x214DC90, 0x214C8B0, 0x214C5D0, 0x214CDC0, 0x214C880, 0x214CDB0, 0x20E7880, 0x20E7900, 0x20E78F0 | getters | [docs/audit/note-shuffle/step0/open2/getters.asm](../../audit/note-shuffle/step0/open2/getters.asm) | 6142 | 2026-10-09 |
| 0x214F6D0 | mtrk read | [docs/audit/note-shuffle/step0/open1-refute/mtrk_read_214F6D0.asm](../../audit/note-shuffle/step0/open1-refute/mtrk_read_214F6D0.asm) | 6142 | 2026-10-09 |
| 0x214F6D0 | mtrk read | [docs/audit/note-shuffle/step0/open1/mtrk_read_214F6D0.asm](../../audit/note-shuffle/step0/open1/mtrk_read_214F6D0.asm) | 6142 | 2026-10-09 |
| 0x2150010 | mtrk read event | [docs/audit/note-shuffle/step0/open1/mtrk_read_event_2150010.asm](../../audit/note-shuffle/step0/open1/mtrk_read_event_2150010.asm) | 6142 | 2026-10-09 |
| 0x2150C00 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f2150c00.asm](../../audit/note-shuffle/step0/open2/f2150c00.asm) | 6142 | 2026-10-09 |
| 0x2150E60 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f2150e60.asm](../../audit/note-shuffle/step0/open2/f2150e60.asm) | 6142 | 2026-10-09 |
| 0x2151650 | chart section dispatch | [docs/audit/note-shuffle/step0/open1/chart_section_dispatch_2151650.asm](../../audit/note-shuffle/step0/open1/chart_section_dispatch_2151650.asm) | 6142 | 2026-10-09 |
| 0x2151650 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f2151650.asm](../../audit/note-shuffle/step0/open2/f2151650.asm) | 6142 | 2026-10-09 |
| 0x2151830 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f2151830.asm](../../audit/note-shuffle/step0/open2/f2151830.asm) | 6142 | 2026-10-09 |
| 0x2152970 | midi helper | [docs/audit/note-shuffle/step0/open1/midi_helper_2152970.asm](../../audit/note-shuffle/step0/open1/midi_helper_2152970.asm) | 6142 | 2026-10-09 |
| 0x2152B20 | midi helper | [docs/audit/note-shuffle/step0/open1/midi_helper_2152B20.asm](../../audit/note-shuffle/step0/open1/midi_helper_2152B20.asm) | 6142 | 2026-10-09 |
| 0x21544E0 | midi events track | [docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm](../../audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm) | 6142 | 2026-10-09 |
| 0x2155050 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f2155050.asm](../../audit/note-shuffle/step0/open2-refute/f2155050.asm) | 6142 | 2026-10-09 |
| 0x2155050 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f2155050.asm](../../audit/note-shuffle/step0/open2/f2155050.asm) | 6142 | 2026-10-09 |
| 0x2155200 | mid reader lane case | [docs/audit/drum-lanes/play/f2155200_mid_reader_lane_case.asm](../../audit/drum-lanes/play/f2155200_mid_reader_lane_case.asm) | 6142 | 2026-10-10 |
| 0x2155C00 | midi reader | [docs/audit/note-shuffle/step0/open1-refute/midi_reader_2155C00.asm](../../audit/note-shuffle/step0/open1-refute/midi_reader_2155C00.asm) | 6142 | 2026-10-09 |
| 0x2155C00 | midi reader | [docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm](../../audit/note-shuffle/step0/open1/midi_reader_2155C00.asm) | 6142 | 2026-10-09 |
| 0x2156990 | midi track handler | [docs/audit/note-shuffle/step0/open1-refute/midi_track_handler_2156990.asm](../../audit/note-shuffle/step0/open1-refute/midi_track_handler_2156990.asm) | 6142 | 2026-10-09 |
| 0x2156990 | midi track handler | [docs/audit/note-shuffle/step0/open1/midi_track_handler_2156990.asm](../../audit/note-shuffle/step0/open1/midi_track_handler_2156990.asm) | 6142 | 2026-10-09 |
| 0x2156990 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f2156990.asm](../../audit/note-shuffle/step0/open2/f2156990.asm) | 6142 | 2026-10-09 |
| 0x215B8C0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f215B8C0.asm](../../audit/note-shuffle/step0/open2-refute/f215B8C0.asm) | 6142 | 2026-10-09 |
| 0x215B8C0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f215b8c0.asm](../../audit/note-shuffle/step0/open2/f215b8c0.asm) | 6142 | 2026-10-09 |
| 0x215BBF0 | chart addnote | [docs/audit/note-shuffle/step0/open1-refute/chart_addnote_215BBF0.asm](../../audit/note-shuffle/step0/open1-refute/chart_addnote_215BBF0.asm) | 6142 | 2026-10-09 |
| 0x215BBF0 | chart addnote | [docs/audit/note-shuffle/step0/open1/chart_addnote_215BBF0.asm](../../audit/note-shuffle/step0/open1/chart_addnote_215BBF0.asm) | 6142 | 2026-10-09 |
| 0x215BBF0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f215BBF0.asm](../../audit/note-shuffle/step0/open2-refute/f215BBF0.asm) | 6142 | 2026-10-09 |
| 0x215BBF0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f215bbf0.asm](../../audit/note-shuffle/step0/open2/f215bbf0.asm) | 6142 | 2026-10-09 |
| 0x215BE50 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f215BE50.asm](../../audit/note-shuffle/step0/open2-refute/f215BE50.asm) | 6142 | 2026-10-09 |
| 0x215BE50 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f215be50.asm](../../audit/note-shuffle/step0/open2/f215be50.asm) | 6142 | 2026-10-09 |
| 0x215C040 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f215C040.asm](../../audit/note-shuffle/step0/open2-refute/f215C040.asm) | 6142 | 2026-10-09 |
| 0x215C040 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f215c040.asm](../../audit/note-shuffle/step0/open2/f215c040.asm) | 6142 | 2026-10-09 |
| 0x215C320 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/scout-strings/f215C320.asm](../../audit/note-shuffle/scout-strings/f215C320.asm) | 6142 | 2026-10-09 |
| 0x215C320 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f215C320.asm](../../audit/note-shuffle/step0/open2-refute/f215C320.asm) | 6142 | 2026-10-09 |
| 0x215C320 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f215c320.asm](../../audit/note-shuffle/step0/open2/f215c320.asm) | 6142 | 2026-10-09 |
| 0x215C560 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f215c560.asm](../../audit/note-shuffle/step0/open2/f215c560.asm) | 6142 | 2026-10-09 |
| 0x215CD50 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f215CD50.asm](../../audit/note-shuffle/step0/open2-refute/f215CD50.asm) | 6142 | 2026-10-09 |
| 0x215CD50 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f215cd50.asm](../../audit/note-shuffle/step0/open2/f215cd50.asm) | 6142 | 2026-10-09 |
| 0x215D450 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f215D450.asm](../../audit/note-shuffle/step0/open2-refute/f215D450.asm) | 6142 | 2026-10-09 |
| 0x215D450 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f215d450.asm](../../audit/note-shuffle/step0/open2/f215d450.asm) | 6142 | 2026-10-09 |
| 0x215DA70 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/scout-strings/f215DA70.asm](../../audit/note-shuffle/scout-strings/f215DA70.asm) | 6142 | 2026-10-09 |
| 0x215DA70 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f215DA70.asm](../../audit/note-shuffle/step0/open2-refute/f215DA70.asm) | 6142 | 2026-10-09 |
| 0x215DBD0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f215dbd0.asm](../../audit/note-shuffle/step0/open2/f215dbd0.asm) | 6142 | 2026-10-09 |
| 0x215E020 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f215E020.asm](../../audit/note-shuffle/step0/open2-refute/f215E020.asm) | 6142 | 2026-10-09 |
| 0x215E020 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f215e020.asm](../../audit/note-shuffle/step0/open2/f215e020.asm) | 6142 | 2026-10-09 |
| 0x215E120 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f215E120.asm](../../audit/note-shuffle/step0/open2-refute/f215E120.asm) | 6142 | 2026-10-09 |
| 0x215E280 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f215E280.asm](../../audit/note-shuffle/step0/open2-refute/f215E280.asm) | 6142 | 2026-10-09 |
| 0x215E280 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f215e280.asm](../../audit/note-shuffle/step0/open2/f215e280.asm) | 6142 | 2026-10-09 |
| 0x215E4F0 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/f215e4f0.asm](../../audit/drum-lanes/f215e4f0.asm) | 6142 | 2026-10-10 |
| 0x215ED20, 0x215E120, 0x215C040, 0x215B8C0 | finish steps | [docs/audit/note-shuffle/scout-strings/finish_steps.asm](../../audit/note-shuffle/scout-strings/finish_steps.asm) | 6142 | 2026-10-09 |
| 0x215ED20, 0x215E120, 0x215C040, 0x215B8C0 | finish steps | [docs/audit/note-shuffle/step0/open2/finish_steps_215ed20_215e120_215c040_215b8c0.asm](../../audit/note-shuffle/step0/open2/finish_steps_215ed20_215e120_215c040_215b8c0.asm) | 6142 | 2026-10-09 |
| 0x215F860 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/f215f860.asm](../../audit/drum-lanes/f215f860.asm) | 6142 | 2026-10-10 |
| 0x215FE80 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/scout-strings/f215FE80.asm](../../audit/note-shuffle/scout-strings/f215FE80.asm) | 6142 | 2026-10-09 |
| 0x2161760 | chart track parser | [docs/audit/note-shuffle/step0/open1/chart_track_parser_2161760.asm](../../audit/note-shuffle/step0/open1/chart_track_parser_2161760.asm) | 6142 | 2026-10-09 |
| 0x2161A80 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f2161A80.asm](../../audit/note-shuffle/step0/open2-refute/f2161A80.asm) | 6142 | 2026-10-09 |
| 0x2161A80 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f2161a80.asm](../../audit/note-shuffle/step0/open2/f2161a80.asm) | 6142 | 2026-10-09 |
| 0x2163100 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f2163100.asm](../../audit/note-shuffle/step0/open2/f2163100.asm) | 6142 | 2026-10-09 |
| 0x2164100 | chart resolution | [docs/audit/note-shuffle/step0/open1-refute/chart_resolution_2164100.asm](../../audit/note-shuffle/step0/open1-refute/chart_resolution_2164100.asm) | 6142 | 2026-10-09 |
| 0x2164100 | chart resolution | [docs/audit/note-shuffle/step0/open1/chart_resolution_2164100.asm](../../audit/note-shuffle/step0/open1/chart_resolution_2164100.asm) | 6142 | 2026-10-09 |
| 0x2164780 | settings ctor | [docs/audit/note-shuffle/step0/open34/settings_ctor_2164780.asm](../../audit/note-shuffle/step0/open34/settings_ctor_2164780.asm) | 6142 | 2026-10-09 |
| 0x235AD0 | globalvars | [docs/audit/note-shuffle/step0/open34/globalvars_235ad0.asm](../../audit/note-shuffle/step0/open34/globalvars_235ad0.asm) | 6142 | 2026-10-09 |
| 0x2D4D0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f2D4D0.asm](../../audit/note-shuffle/step0/open2-refute/f2D4D0.asm) | 6142 | 2026-10-09 |
| 0x2D4D0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f2d4d0.asm](../../audit/note-shuffle/step0/open2/f2d4d0.asm) | 6142 | 2026-10-09 |
| 0x33C560 | drumsplayer | [docs/audit/note-shuffle/step0/open34-refute/drumsplayer_33c560.asm](../../audit/note-shuffle/step0/open34-refute/drumsplayer_33c560.asm) | 6142 | 2026-10-09 |
| 0x35C110 | songentry settings | [docs/audit/note-shuffle/step0/open1/songentry_settings_35C110.asm](../../audit/note-shuffle/step0/open1/songentry_settings_35C110.asm) | 6142 | 2026-10-09 |
| 0x3BD710 | online movenext | [docs/audit/note-shuffle/step0/open34/online_movenext_3bd710.asm](../../audit/note-shuffle/step0/open34/online_movenext_3bd710.asm) | 6142 | 2026-10-09 |
| 0x5DB0C0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2-refute/f5DB0C0.asm](../../audit/note-shuffle/step0/open2-refute/f5DB0C0.asm) | 6142 | 2026-10-09 |
| 0x5DB0C0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f5db0c0.asm](../../audit/note-shuffle/step0/open2/f5db0c0.asm) | 6142 | 2026-10-09 |
| 0x5DB4C0 | not named in the file name; see the folder notes | [docs/audit/note-shuffle/step0/open2/f5db4c0.asm](../../audit/note-shuffle/step0/open2/f5db4c0.asm) | 6142 | 2026-10-09 |
| 0x5DE790 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/f5de790.asm](../../audit/drum-lanes/f5de790.asm) | 6142 | 2026-10-10 |
| 0x5DEB40 | not named in the file name; see the folder notes | [docs/audit/drum-lanes/f5deb40.asm](../../audit/drum-lanes/f5deb40.asm) | 6142 | 2026-10-10 |
| 0x6D60 | ctor | [docs/audit/note-shuffle/step0/open34-refute/ctor_6d60.asm](../../audit/note-shuffle/step0/open34-refute/ctor_6d60.asm) | 6142 | 2026-10-09 |
| 0xB5220 | gamemanager | [docs/audit/note-shuffle/step0/open34/gamemanager_b5220.asm](../../audit/note-shuffle/step0/open34/gamemanager_b5220.asm) | 6142 | 2026-10-09 |
| 0xBA8B0 | gamemanager | [docs/audit/note-shuffle/step0/open34/gamemanager_ba8b0.asm](../../audit/note-shuffle/step0/open34/gamemanager_ba8b0.asm) | 6142 | 2026-10-09 |
| 0xBD640 | gamemanager | [docs/audit/note-shuffle/step0/open34/gamemanager_bd640.asm](../../audit/note-shuffle/step0/open34/gamemanager_bd640.asm) | 6142 | 2026-10-09 |
| 0xD9EF0 | gamemanager movenext | [docs/audit/note-shuffle/step0/open34/gamemanager_movenext_d9ef0.asm](../../audit/note-shuffle/step0/open34/gamemanager_movenext_d9ef0.asm) | 6142 | 2026-10-09 |

## New listings

This section is filled when the reference pages merge.
