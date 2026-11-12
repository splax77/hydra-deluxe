# ini_parse_21695E0.asm  (build 6142, RVA 0x21695E0, the song.ini parser that fills the ini object)
Made with tools/ch_mcp/chdis.py (py -m tools.ch_mcp.chdis 21695E0 1200) on 2026-11-12, session c61c1986 (task T1-MID-NO-PRO). Class #8646, the song.ini object. The song.ini load hands it the file's text right after File.ReadAllText (metadata_load_1F0160.asm lines 36 and 62).
Plain words: it reads the text one line at a time. It trims each line, skips blank lines and comments, starts a section on a [name] line, and splits any other line at its first "=". The key and the value are each trimmed at both ends before they are stored, so a value's leading and trailing blanks never reach a getter.
Lines 1-108: prologue, first-call setup and the loop head over the text.
Lines 166-207: find the next line break, character 0x0A, with SpanHelpers.IndexOf (lines 178, 190 and 203). No break means the line runs to the end of the text (line 207).
Line 227: MemoryExtensions.Trim(ReadOnlySpan<char>) on the whole line. That overload trims white space at both ends by .NET's char.IsWhiteSpace rule, so a carriage return at a line's end goes too.
Lines 262-266: a line that is empty after the trim is skipped.
Lines 270-275: a line that starts with "#" (0x23) or ";" (0x3B) is skipped as a comment.
Lines 276-333: a line that starts with "[" (0x5B) and ends with "]" (0x5D) names a section. Its name, at most 0x400 characters (line 302), goes through 0x2152890 (line 328, not read) and becomes the current section.
Lines 334-342: any other line is split at its first "=" (0x3D). A line with no "=" is skipped.
Line 362: Trim on the text before the "=", the key. Lines 367-392: the key, at most 0x400 characters, goes through 0x2152890 (line 392, not read).
Line 418: Trim on the text after the "=", the value.
Lines 421-445: find the current section's dictionary, or make one and add it.
Lines 446-460: store value under key with Dictionary<string, string>.set_Item (line 460), so a later line with the same key replaces an earlier one.
