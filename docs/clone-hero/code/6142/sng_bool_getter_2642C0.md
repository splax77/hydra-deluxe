# sng_bool_getter_2642C0.asm  (build 6142, RVA 0x2642C0, a bool getter of the .sng metadata class)
Made with tools/ch_mcp/chdis.py on 2026-11-12, session c61c1986 (task T1-MID-NO-PRO); committed by task mid-no-pro T5. Called from the .sng metadata reader (sng_meta_reader_1EFF90.asm lines 46 to 56) for five_lane_drums and pro_drums, each with a default of false (r8d zeroed, lines 47 and 53 there).
Plain words: the key's text counts only through .NET's Boolean.TryParse. So "true" or "false" in any letter case is read as that word, and every other text, "1" included, gives the caller's default, false. A missing key gives the default too. Unlike the song.ini getter (ini_bool_getter_1D4480.md), it has no number test.
Lines 1-14: prologue; first-call setup. r8b, the caller's default, is kept in edi (line 6).
Lines 15-19: load the metadata's key-to-text Dictionary from +0x30; a null one jumps to line 49, a throw.
Lines 20-25: Dictionary.TryGetValue(key, out text). A missing key jumps to line 43, which returns the default (line 44).
Lines 31-36: Boolean.TryParse(text, out b), at 0x264355. A failed parse jumps to line 43, the default.
Lines 37-42: a parsed text returns b.
Which blanks Boolean.TryParse accepts around the word is .NET's rule, not read in this listing.
