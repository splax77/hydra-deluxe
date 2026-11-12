# sng_meta_reader_1EFF90.asm  (build 6142, RVA 0x1EFF90, a #828 method with a scrambled name, the .sng metadata reader)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2). Method start per dump.cs (line 45187).
Lines 8-30: first call only: touch every key-name string literal. No rule here.
Lines 33-38: key "delay", with r8d and r9d zeroed (lines 34-35), through the getter 0x262940 (sng_int_getter_262940.md); line 38 stores the int at metadata+0x24, the same field the song.ini reader writes (ini_song_reader_1EFC60.md, lines 38 to 47).
Lines 39-101: the other keys (hopo_frequency, five_lane_drums, pro_drums, eighthnote_hopo, multiplier_note, video_start_time, sustain_cutoff_threshold, chord_snap_threshold, loading_phrase). Not read line by line.
