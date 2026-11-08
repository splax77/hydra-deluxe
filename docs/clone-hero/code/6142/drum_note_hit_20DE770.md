# Drum engine note hit, RVA 0x20DE770 (`#8255`, slot 11)

Listing: `drum_note_hit_20DE770.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. `dump.cs` line 527547 gives this RVA as the drum engine's override of slot 11; the base class's slot 11 is `0x20F63F0` (`dump.cs` line 530143), which this method calls at line 125. SP-43's scan (`docs/audit/2026-11-08-clone-hero-unknowns/sp43_immscan.py`) flagged its `cmp eax, 3` at line 17.

- Lines 8-13: skip the counter below when the note's +0x80 has bit 1 set, when the engine's +0x2c4 is zero, or when the note's +0x7c has a bit of 0xc000 set.
- Lines 14-23: add one to the counter at +0x2ec. When it reaches 3, set it back to zero and clear +0x2c4, +0x2c8 and +0x2f0. The 3 is a count of hits, not a time.
- Lines 24-28: when the note's +0x7c has bit 0x10000 set, set +0x2f1 and copy the song clock at +0x100 to +0x2d8.
- Lines 29-61: pick one of the bytes +0x309 to +0x310 by the note's lane bit at +0x80.
- Lines 62-104: two note tests; depending on the picked byte against +0x2c0, call `0x214B740` on the note and add one to +0xdc or +0xd8.
- Lines 105-125: more note tests, then the base hit `0x20F63F0`.
- Lines 126-135: jump to virtual slot at offset 0x198 with 0x32 in `r8d`.
