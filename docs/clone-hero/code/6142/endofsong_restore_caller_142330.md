# endofsong_restore_caller_142330: a method that calls an end-of-song restore on the EndOfSong object, when one exists (build 6142)

Listing: `endofsong_restore_caller_142330.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis 142330 120`. It stops inside the method (66 lines, to the start of the branch for argument 1 at 0x142446).

Plain version: it takes a one-byte code. For code 5 it looks up the `EndOfSong` object in the scene and, when one exists, calls the restore method `0x160650` on it.

Lines 18 to 21 branch on the code (1, 3 or 5; any other code returns). For 5, line 33 calls the scene lookup for an EndOfSong object (the metadata name at line 10), line 35 skips the call when no object came back, and line 38 calls `0x160650` on the object with 0. So this caller cannot reach a restore method while no `EndOfSong` object exists. It is a virtual method (`Slot: 12`, `dump.cs` line 33256) with no direct caller, so what sends it code 5 was not read.
