# globals_set_song_235AD0.asm (build 6142, RVA `0x235AD0`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 235AD0` (263 lines). Starts at the method's first instruction: `GlobalVariables`' `public bool ʺʵʸʸʽʶʾˀʶʺʺ(SongEntry, ˁʻʽʷʽʻʾʵʷˀʶ, out string, out string, out string)` (`dump.cs` line 55969; class header at line 55801).

Plain words: this is the method that loads a song for play. The first thing it does after clearing its three out strings is store its SongEntry argument in `GlobalVariables.currentSongEntry` (+0x28, `dump.cs` line 55805). Only then does it load the song's data, and on a failure it logs "Failed to load song object for" with the artist and name, or "Error loading song into memory:".

- Lines 1 to 18: stack set-up. `rsi` is `this` (the GlobalVariables object), `rbx` is the SongEntry argument (`rdx`), `rdi` is the player argument, `r15` the first out string.
- Lines 19 to 30: one-time loads of the type infos and the two error strings.
- Lines 31 to 45: set the three out strings to null.
- Lines 47 to 50: `this.currentSongEntry = song`, with no test in front of it, then the GC write barrier (`0x2F5FC00`).
- Lines 51 onward: read `currentSongMetadata` (+0x30), call into SongEntry (`0x35C110`, line 64), the player (line 78) and the chart builder `0x20D2EA0` (line 110), then the error paths (lines 173 to 232). Return at line 247.

Who calls it, direct callers only (byte scan with `docs/audit/2026-11-08-clone-hero-unknowns/W9a/callscan.py`, output in `callers_235AD0.txt` beside it): `SongOptions` method `0x297240` at 0x2983C3 (`dump.cs` line 7083) and the coroutine `ʲʻʳʶʲʼʸʴʾʲʳ.ˁʸˁʵʽʵʾʹʲʶʼ.MoveNext` `0x3BD710` at 0x3BD88C (`dump.cs` line 19032). Neither caller was read.
