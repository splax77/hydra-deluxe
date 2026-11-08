# metadata_load_1F0160.asm  (build 6142, RVA 0x1F0160, a #828 song metadata method with a scrambled name, taking a SongEntry)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2c). Method start per dump.cs line 45177.
Lines 11-12: when the metadata already holds this SongEntry (+0x48), return at once, changing nothing.
Lines 13-18: otherwise store the SongEntry at +0x48 and clear the byte at +0x50.
Lines 21-24: pick a reader from two SongEntry bytes. +0xB2 set goes to line 83 (the encrypted-song branch, through SongEntry.get_EncSong). +0xB3 set goes to line 72 (the .sng branch, a tail-jump to 0x1EFF90, sng_meta_reader_1EFF90.md). Neither set reads song.ini: lines 25-66 read the file at SongEntry.get_IniPath and hand it to the [song] reader 0x1EFC60 (ini_song_reader_1EFC60.md).
Lines 83-105: the encrypted-song branch zeroes the delay and the other small fields, then stores the int at +0x78 of the song object under get_EncSong's result (+0x18), as a double, in the song length at +0x30 (lines 102-105). What that int holds was not traced.
The .sng reader and the song.ini reader store nothing at +0x30 (no [reg + 0x30] store in either listing).
Direct callers (byte scan of rel32 calls and jumps): the load coroutine 0xD8D80 (gm_load_coroutine_D8D80.asm line 282), PlayerSelectionCanvas.OnEnable (0xCF000) and GlobalVariables method 0x235AD0. The #828 methods 0x1EF570 and 0x1F05B0 hold the same code; their own callers are PlayerSelectionCanvas methods.
