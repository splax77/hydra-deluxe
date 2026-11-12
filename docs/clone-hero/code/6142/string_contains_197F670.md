# string_contains_197F670: String.Contains(string) (build 6142)

Listing: `string_contains_197F670.asm` beside this file, made 2026-11-11 (session 428cf079, CR-24 plan task T0) with `py -m tools.ch_mcp.chdis 197F670 1D0` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. The method is `public bool Contains(string value)` of System.String, `dump.cs` lines 77989 and 77990. The next method, the two-argument Contains, starts at RVA 0x197F840 (`dump.cs` line 77992), so the byte count stops the listing before it.

Plain words: Contains asks the shared CompareInfo for an ordinal search, byte for byte, with case counting. A text holding "_endofsong" in lower case does not match "_ENDOFSONG".

Lines 34 to 39 set up the call to `CompareInfo.IndexOfOrdinal` (RVA 0x1A72950, `dump.cs` line 195058, signature `(string source, string value, int startIndex, int count, bool ignoreCase)`). The source is the string itself (`rdx` at line 37, from `rbx`, which line 8 copies from `rcx`), the value is the argument (`r8` at line 36, from `rsi`, which line 6 copies from `rdx`), the start index is 0 (`r9d`, line 34), the count is the string's length (line 38, from `[rcx+0x10]` at line 7), and ignoreCase is 0 (the byte at `[rsp+0x28]`, line 35). Lines 42 and 43 turn the index into the result: true when it is not negative.

Who calls it: CR-24's .mid events reader, `docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm` line 134, with the text _ENDOFSONG.
