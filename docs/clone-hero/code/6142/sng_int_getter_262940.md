# sng_int_getter_262940.asm  (build 6142, RVA 0x262940, an int getter of the .sng metadata class)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2). Method start per dump.cs.
Line 24: looks the key up in a Dictionary (TryGetValue); a miss returns the default (lines 70-75). The dictionary's key comparer was not read.
Lines 30-46: text with no "." goes through Int32.TryParse and is returned as is.
Lines 47-69: text with a "." goes through Single.TryParse with the invariant culture, is multiplied by 1000.0 (line 63) and truncated to an int (line 64).
A failed parse returns the default (line 70). This is the same shape as the song.ini getter, ini_int_getter_21647A0.md.
