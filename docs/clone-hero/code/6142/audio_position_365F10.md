# audio_position_365F10: the audio manager's position getter (build 6142)

Listing: `audio_position_365F10.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. It runs from RVA `0x365F10` to the last byte before the next method (0x3660A0). The function is a `BassAudioManager` method declared `public double ...()` at `dump.cs` line 12068 (class `BassAudioManager`, TypeDefIndex 190, `dump.cs` line 11865).

Plain version: the getter returns 0 when `isSongLoaded` (+0x28) is false (lines 17 to 22). Otherwise it takes the int handle at +0x58, calls an extern that takes a handle and a mode of 0 and returns a long (line 32, RVA `0x247CB0`, `dump.cs` line 55050), then passes the handle and that long to an extern that returns a double (line 45, RVA `0x24DC30`, `dump.cs` line 55451). That double is the result. When the first call returns -1 or the second returns -1.0, it logs "Failed to get audio position {0}" or "Failed to convert audio position {0}" through the error-code extern at `0x24EDF0`.

The two extern shapes match the BASS library's `BASS_ChannelGetPosition(handle, BASS_POS_BYTE)` and `BASS_ChannelBytes2Seconds(handle, pos)`, and the two error strings say the same. The entry-point names were not read from the metadata, so that match is from the shapes and the strings only.

What this does not prove: which BASS channel the handle at +0x58 is (the plain decoded stream or a tempo stream made over it). Its writer was not read. The class keeps both `audioLength` (+0x40) and `originalAudioLength` (+0x48).
