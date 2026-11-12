Listing: `song_entry_add_score_358560.asm`, build 6142, made 2026-11-11 with `tools/ch_mcp/chdis.py` (`py -m tools.ch_mcp.chdis 358560`). Function: a `SongEntry` method at RVA 0x358560 that takes a score entry (struct TypeDefIndex 1150) and returns a bool.

Plain version: at the end of a song it adds the run to the song's chart record, saves the score file, and says whether the run beat the old best.

Lines 11 to 20 read the song's chart record at `SongEntry` +0x60 and fetch the stored entry for the run's instrument through 0x2165A0 (not read). Lines 28 to 34 add the run with 0x2161E0 (`score_chart_add_2161E0.asm`). Line 40 calls the score store's 0x2F5A40, which calls the `scoredata.bin` writer 0x2F57C0 (byte scan, reference at 0x2f5d5d). Lines 41 to 50 compare the run's value from the getter 0x2BA7A0 (the larger of entry +0x4 and +0x18) with the old entry's, and return true when the run's is greater.

Who calls it, direct callers only (byte scan with `docs/audit/2026-11-08-clone-hero-unknowns/W8c/xref.py`, output in `xref_IN-55.out.txt` there): `EndOfSong.Start` (0x15DC20, call at 0x15eeb7, right after building the entry with the struct's constructor at 0x15ee47) and one more `EndOfSong` method at 0x161C70 (call at 0x162f42). Whether `EndOfSong.Start` calls it once per player was not read.
