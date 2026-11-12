# ini_text_lookup_1BD150.asm  (build 6142, RVA 0x1BD150, ini object method that returns a key's text)
Made with tools/ch_mcp/chdis.py (py -m tools.ch_mcp.chdis 1BD150 800) on 2026-11-12, session c61c1986 (task T1-MID-NO-PRO). Neither dump.cs nor script.json names a method at 0x1BD150; it is the address the bool and int getters call. Its first-call setup (line 25) names the class #8646 method that dump.cs line 551477 lists at RVA 0x2164910, with the arguments (section, key, out string). That suggests this is the same method, but it was not proved.
Plain words: it finds the section, then the key, and hands back the stored text as it is. It does not trim the text; the parser trimmed it when the file was read (ini_parse_21695E0.md).
Lines 1-29: prologue; first-call metadata setup. Arguments: the ini object (rcx), the section name (rdx), the key (r8), the text out (r9).
Lines 38-42: a test on the key (0x216BB00, not read). True skips to line 84 with the key unchanged.
Lines 43-83: otherwise copy the key into a stack buffer of its own length and pass it through 0x1BD9A0 (line 79, not read), then make a string of the result (line 82). This is the key's normalising step; what it changes was not read.
Lines 84-90: find the section's dictionary through 0x1BD350 (line 88, not read). Not found jumps to line 106.
Lines 91-105: Dictionary<string, string>.TryGetValue(key) on that section (line 97). Found: store the text in the out argument untouched (line 102) and return true (line 104).
Lines 106-112: not found: store String.Empty and return false.
Line 123: a null section dictionary throws.
