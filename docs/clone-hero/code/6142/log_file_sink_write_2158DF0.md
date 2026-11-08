Listing: `log_file_sink_write_2158DF0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py` (`py -m tools.ch_mcp.chdis 2158DF0 600`). Function: the file sink's write method (class TypeDefIndex 8639, `dump.cs` lines 551032 and 551033, interface slot 5), which takes the formatted text and the log entry (class TypeDefIndex 8248, `dump.cs` lines 527038 to 527050).

Plain version: the sink writes an entry to its file only when the sink has no tag, or when the entry's tag is exactly the sink's tag.

Lines 21 to 24 return at once if the sink has no FileStream (+0x10) or no StreamWriter (+0x18). Lines 27 to 31 test the sink's tag (+0x30): when it is empty, the write goes ahead (line 31 jumps to line 43). Otherwise lines 32 to 42 read the entry's string at +0x40 and call String.Equals with comparison 5, which is StringComparison.Ordinal; when they differ, line 42 jumps past the write to the return. Lines 43 to 68 write the text to the StreamWriter, and lines 69 to 74 make a second virtual call on it (by its position, a flush; the slot was not named).

The two HitDetection sinks carry the tags `GuitarEngine` and `GuitarEngineValidator` (`fast_logger_setup_213040.asm` lines 248 and 268). The entry's +0x40 string is the fifth argument of the logger's writer (`log_writer_2128F10.asm` lines 93 and 94).
