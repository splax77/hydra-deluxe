Listing: `score_store_write_2F57C0.asm`, build 6142, made 2026-11-11 with `tools/ch_mcp/chdis.py` (`py -m tools.ch_mcp.chdis 2F57C0`). Function: the static method at RVA 0x2F57C0 of the score store class TypeDefIndex 1205 (`dump.cs` line 64526), which takes a `List` of chart records (class TypeDefIndex 907).

Plain version: this writes `scoredata.bin`. It opens the file, writes the version number, then the number of chart records, then each record in turn.

Lines 30 to 38 read the class's static field 0x0, which 0x2F4300 set to the `scoredata.bin` path (`score_store_paths_2F4300.asm` line 41), and open it with `File.OpenWrite`. Lines 40 to 46 wrap it in a `BinaryWriter`. Lines 55 to 57 write the 32-bit integer 0x1346541, which is 20211009, the version constant at `dump.cs` line 64478. Lines 63 to 66 write the list's count (the `List` size field at +0x18) as a 32-bit integer. Lines 67 to 90 walk the list and call 0x2167B0 for each record (`score_chart_write_2167B0.asm`). Lines 91 to 114 dispose the enumerator and the writer.

The class has four more methods of the same shape (0x2F4510, 0x2F5EF0, 0x2F7250, 0x2F7830). Each writes a different first integer (-81, 0x84, 0x49 and -79) and then calls the same record writer. Which file each one writes, and when, was not read.

Who calls it, direct callers only (byte scan with `docs/audit/2026-11-08-clone-hero-unknowns/W8c/xref.py`, output in `xref_IN-55.out.txt` there): 0x2F3BD0, 0x2F5A40 and 0x2F6A60, all in class 1205. 0x2F5A40 is the one the score add path calls (`song_entry_add_score_358560.asm` line 40).
