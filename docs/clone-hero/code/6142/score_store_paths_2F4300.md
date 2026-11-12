Listing: `score_store_paths_2F4300.asm`, build 6142, made 2026-11-11 with `tools/ch_mcp/chdis.py` (`py -m tools.ch_mcp.chdis 2F4300`). Function: the static method at RVA 0x2F4300 of the score store class TypeDefIndex 1205 (`dump.cs` lines 64475 to 64556), which takes no argument. The class holds the constants `scores.bin`, `scoredata.bin`, `scoredata_backup.bin`, `scoredata_corrupted_{0}.bin` and the version 20211009 (`dump.cs` lines 64478 to 64482), and three static string fields at 0x0, 0x8 and 0x10.

Plain version: it works out the full paths of the three score files once and keeps them in the class's static fields.

Lines 22 to 41 get a base folder from the method at 0x307590 (not read) and join `scoredata.bin` to it with `Path.Combine`; line 41 stores the result in static field 0x0. Lines 45 to 53 do the same for `scores.bin` into field 0x8, and lines 59 to 67 for `scoredata_backup.bin` into field 0x10. Line 76 then jumps on to 0x1564A0 (not read).

Who calls it, direct callers only (byte scan with `docs/audit/2026-11-08-clone-hero-unknowns/W8c/xref.py`, output in `xref_IN-55.out.txt` there): one, `SongScan` iterator MoveNext at 0x1BB600 (reference at 0x1bbc0b).

What the listing does not show: which folder 0x307590 returns. The tools take it to be the LocalLow folder (`LOCALLOW_DIR` in tools/ch_mcp/game_files.py).
