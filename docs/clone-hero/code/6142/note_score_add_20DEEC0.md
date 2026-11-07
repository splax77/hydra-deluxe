Listing: `note_score_add_20DEEC0.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. Function: drum engine (#8255) override of virtual slot 6, RVA `0x20DEEC0`. Arguments: the engine, a note, and the base points. It works out the points of one note and hands them on to the base engine's score add (`0x20F6EB0`).

Lines 1 to 12: save registers, copy the arguments, and stop at the error call (line 32) if the note is null.
Lines 13 to 16: ask the note whether any of its lane bits in 0xE0 is set (a cymbal). If yes, add 15 to the base points; if no, keep them (the `lea` adds 15, the `cmove` undoes it when the answer was no).
Lines 19 to 24: ask the note whether bit 4 of `note+0x84` is set (the mark that the hit handler `0x20DE770` sets when the accent or ghost was performed). If yes, double the points; if no, keep them.
Lines 25 to 31: restore registers and jump to `0x20F6EB0` with the engine, the note and the final points.
Line 32 to 33: the null-note error path.
