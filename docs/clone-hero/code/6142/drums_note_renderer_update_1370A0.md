# drums_note_renderer_update_1370A0: DrumsNoteRenderer.Update (build 6142)

Listing: `drums_note_renderer_update_1370A0.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. `DrumsNoteRenderer.Update`, `dump.cs` line 33398.

Line 98 reads the double at +0x78 of the object at +0x28 (loaded at line 94). The base class `BaseNoteRenderer` (`dump.cs` line 7203) holds a `GameManager` field, but the dump prints its offsets as 0x0 because the class is generic, so that +0x28 is that field is not shown. The method makes no call to `0x16C490`.

From ch-unk-w5:
DrumsNoteRenderer's Update, named in script.json (class DrumsNoteRenderer, dump.cs line 33333). Line 267 calls the lane draw 0x1166890 (lane_draw_1166890.asm). Line 342 calls the same sprite loop 0x41BE70 for a second list, of class 8322, which is not the lane list. Page 2's NL-57 cites this listing.
