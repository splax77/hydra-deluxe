# Drum engine fill clear, RVA 0x20DF3E0 (`#8255`)

Listing: `fill_clear_20DF3E0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. `dump.cs` line 527577 gives this RVA, a public method of the drum engine with no arguments. Its loop is the same as the gain override's (`fill_deadline_20DDB70.asm` lines 24-59), with no meter test before it.

- Lines 13-17: the deadline is +0x300 (four times the resolution) plus the current tick at +0x108; the start index is +0x2f8.
- Lines 18-48: for each wrapper from that index, stop when its +0x28 (fill start tick) is past the deadline (lines 35-36); otherwise clear its +0x38 byte and flag 0x800 on its note's +0x7c (lines 37-41), and step the index (line 44).
- Line 49: store the index back in +0x2f8.

No direct call or jump to this RVA was found (byte scan, 2026-11-08), and it has no `ScriptMetadataMethod` entry in `script.json`. Its only 8-byte pointer in the DLL is in `.data`. How it is reached was not found.
