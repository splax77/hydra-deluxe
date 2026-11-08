# drums_note_renderer_update_1370A0: DrumsNoteRenderer.Update (build 6142)

Listing: `drums_note_renderer_update_1370A0.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. `DrumsNoteRenderer.Update`, `dump.cs` line 33398.

Line 98 reads the double at +0x78 of the object at +0x28 (loaded at line 94). The base class `BaseNoteRenderer` (`dump.cs` line 7203) holds a `GameManager` field, but the dump prints its offsets as 0x0 because the class is generic, so that +0x28 is that field is not shown. The method makes no call to `0x16C490`.
