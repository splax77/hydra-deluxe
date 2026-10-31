# ini_song_reader_1EFC60.asm  (build 6142, RVA 0x1EFC60, #828.ReadIniSongSection, the song.ini [song] reader)
Made with tools/ch_mcp/chdis.py (capstone) on 2026-10-30, session b02ab3e6. The function starts at 0x1EFC60 (script.json names it; it is a method start).
Lines 2-10: prologue. rdi = the ini object (second argument), rsi = the metadata object (class 828, "song metadata cache"). Lines 7-10 test a once-only flag.
Lines 11-35: first call only: touch every key-name string literal (video_start_time, delay, ..., hopo_frequency) so the runtime loads them. No rule here.
Lines 36-37: a null ini object jumps to a null-reference throw (194).
Lines 38-47: the delay rule. It asks the ini object for section "song", key "delay", default 0 (r9d = 0, line 39), through the int getter at 0x21647A0 (see ini_int_getter_21647A0.md), and stores the int at metadata+0x24 (line 47). The value is whole milliseconds. There is no clamp and no sign test.
Lines 48-75: hopo_frequency, read as text then Int32.TryParse, kept in 0..65535, stored at +0x18. Not a timing rule.
Lines 76-96: five_lane_drums (+0x38), pro_drums (+0x39), eighthnote_hopo (+0x10) as booleans; multiplier_note (+0x14), video_start_time (+0x28), sustain_cutoff_threshold (+0x1C, default -1), chord_snap_threshold (+0x20), loading_phrase text (+0x40). None touch the clock.
Lines 97-196: epilogue and the null-throw stub.
