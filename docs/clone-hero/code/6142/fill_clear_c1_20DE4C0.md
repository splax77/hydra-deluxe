# Drum engine fill clear with mask 0xC1, RVA 0x20DE4C0 (`#8255`)

Listing: `fill_clear_c1_20DE4C0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. `dump.cs` line 527505 gives this RVA, a public method of the drum engine with no arguments. It has the same deadline as `fill_clear_20DF3E0.asm`, with two differences.

- Lines 13-17: the deadline is +0x300 plus the current tick at +0x108; the start index is +0x2f8.
- Lines 35-41: a wrapper whose +0x28 is at or before the deadline has its +0x38 byte cleared and its note's +0x7c ANDed with 0xC1, which keeps only bits 0x1, 0x40 and 0x80 (line 41).
- Lines 42-47: the loop goes back to line 22 without stepping the index `edi`, so as read it fetches the same wrapper again. Line 48 stores the index only when a wrapper is past the deadline.

No direct call or jump to this RVA was found (byte scan, 2026-11-08), and it has no `ScriptMetadataMethod` entry in `script.json`. Its only 8-byte pointer in the DLL is in `.data`. How it is reached, and whether the loop without a step can run, was not found.
