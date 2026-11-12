# tempo_create_366FC0: the tempo stream (build 6142)

Listing: `tempo_create_366FC0.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. It is a `BassAudioManager` method that takes an int and a float and returns an int (`dump.cs` lines 12043 and 12044).

Plain version: it calls the extern `0x3A8520` (`dump.cs` line 17800) with the source handle and flags 0x10000 (lines 24 to 27). The error string for a zero result is "Failed to create tempo stream" (line 34). When a bool from the settings class (`0x210C2F0`, line 68) is true, it sets attribute 0x10001 ("Failed to set BassFX pitch") to 12 times log2 of a setting's value divided by 100 (lines 79 to 106). It then sets attribute 0x10000 ("Failed to set BassFX starting tempo") to the float argument (lines 112 to 117), and attribute 0x10016 to 1 ("Failed to set BassFX quality attribute", lines 118 to 123). It returns the new handle (line 125).

The strings name the BASS_FX add-on. The entry-point names were not read from the metadata, so the match to BASS_FX's tempo-create call and its tempo and pitch attributes rests on the shapes and the strings.
