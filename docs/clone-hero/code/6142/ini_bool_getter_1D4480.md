# ini_bool_getter_1D4480.asm  (build 6142, RVA 0x1D4480, ini object method that returns a bool for a key)
Made with tools/ch_mcp/chdis.py (py -m tools.ch_mcp.chdis 1D4480 600) on 2026-11-12, session c61c1986 (task T1-MID-NO-PRO). Method start per dump.cs line 551389 (class #8646, the song.ini object; the same class holds the int getter, ini_int_getter_21647A0.md).
Plain words: the key's text counts as true when it is a whole number that is not zero, or the word "true" in any letter case. The word "false" in any case is false. A missing key, and any other text, give the caller's default.
Lines 1-16: prologue; first-call string setup for "false" and "true". r9b, the caller's default, is kept in ebx (line 7).
Lines 17-25: ask the ini for the key's text through the text lookup 0x1801BD150 (ini_text_lookup_1BD150.md). A missing key (al = 0, lines 26-27) jumps to line 74 and returns the default.
Lines 28-31: Int32.TryParse(text, out n), at 0x1D44F9 (int32_tryparse_1B4F710.md). Success jumps to line 66.
Lines 66-67: the number's answer is "n is above 0" as an unsigned compare (cmp then seta), so every n that is not 0 is true, a negative one too.
Lines 34-42: a null text throws (line 81). Otherwise text.Equals("true", 5), comparison type 5 being OrdinalIgnoreCase (line 40). Equal jumps to line 59, which returns true.
Lines 43-51: text.Equals("false", 5) (line 49). Not equal jumps to line 74, the default.
Line 52: "false" returns false.
The three return paths that end the method: lines 52-58 (false), 59-65 (true) and 66-73 (the number). Line 74 is the default's return, and line 81 is the null-text throw.
The text arrives already trimmed at both ends by the ini parser (ini_parse_21695E0.md, line 418), so blanks around the value change nothing.
Callers that matter here: the song.ini [song] reader reads five_lane_drums and pro_drums through it with a default of false (ini_song_reader_1EFC60.asm lines 76 to 89). The .sng metadata reader does not: it uses its own getter 0x2642C0, which is not this one.
