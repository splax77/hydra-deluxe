Listing: `log_file_sink_ctor_2158F60.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py` (`py -m tools.ch_mcp.chdis 2158F60 600`). Function: the constructor of the file sink class TypeDefIndex 8639 (`dump.cs` lines 551015 and 551068), which takes a path, a bool with default True, and a second string.

Plain version: the constructor only stores its three arguments. It does not open the file.

Lines 13 to 15 store the path (rdx) at +0x20. Line 17 stores the bool (r8b) at +0x28. Lines 16 and 18 to 25 store the second string (r9) at +0x30. The HitDetection setup passes `true` and the tag `GuitarEngine` or `GuitarEngineValidator` here (`fast_logger_setup_213040.asm` lines 248 to 254 and 268 to 274). The sink's write method compares that tag with each entry (`log_file_sink_write_2158DF0.asm`).

What the listing does not show: where the file is opened. The class has a FileStream at +0x10 and a StreamWriter at +0x18 (`dump.cs` lines 551018 and 551019), filled by a method this listing does not cover.
