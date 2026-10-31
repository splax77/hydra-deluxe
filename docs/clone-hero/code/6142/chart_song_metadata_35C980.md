# chart_song_metadata_35C980.asm  (build 6142, RVA 0x35C980, SongEntry.ReadChartSongSection, scans a .chart's [Song] lines)
Made with chdis.py on 2026-10-30, session b02ab3e6. Listing runs to the next method start (0x35DA90); it starts at a method start.
Lines 8-18, 76-100: prologue. Line 87: r13 = the object at [GlobalVariables.instance + 0x30], the same class-828 song metadata cache that 0x1EFC60 filled. Line 103: xmm8 = 1000.0f (constant at 0x3064904).
Lines 19-72: first-call string setup (Name, Artist, Album, Genre, Year, Charter, Offset, PreviewStart, ...).
Lines 100-560: the line loop. It walks each line of the [Song] section, splits at "=", trims, and compares the key with Name, Artist, Genre, Year, ... in turn. Each compare is a SequenceEqual call; a match jumps to that key's handler.
Lines 562-586: the key "Offset" is compared (562-561 sets up the literal; the test at 585 jumps to 0x35D53F on a match).
Lines 634-653 (handler at 0x35D53F): if r13 is null throw. Line 636: compare dword [r13+0x24] with 0. If it is NOT zero the handler jumps back to the line loop, so the chart's Offset is ignored. Zero means the song.ini gave no delay (or "delay = 0").
Lines 639-653: otherwise Single.TryParse the value text with style 0xA7 and the invariant culture (644-651, the call at 651); a failure goes back to the loop.
Lines 654-663: float value * 1000.0f (659), rounded up with the C runtime helper at 0x3031130 (661), converted to int (662) and stored at [r13+0x24] (663). 0x3031130 jumps to 0x304BA10, whose one instruction is roundsd with immediate 0xA (round toward +infinity), so the result is ceil(Offset x 1000) whole milliseconds. A negative Offset also goes through ceil, toward zero.
Lines 586-632: the PreviewStart handler (stores an int at +0x54 the same way, 622-632); other keys follow to line 925. None touch the clock.
