# Note builder, `#8265.ʹʹʾʴʳʶʼʵʳʼʼ`, RVA `0x20D2EA0` (build 6142)

Listing: `note_builder_20D2EA0.asm`, 440 lines, starts at the function's first instruction. Made with `tools/ch_mcp/chdis.py`, 2026-10-30. This is the function that builds a player's note list and decides how fills are made. Its arguments: the player settings (`rdx`, instrument byte at +0, difficulty byte at +1, modifier bits at +4), the chart (`r15`), and a flag.

Lines 1-35: prologue and one-time metadata set-up. Nothing about fills.

Lines 36-47: read the instrument byte; instrument 9 (ProDrums) is mapped to 6 (Drums) for the track lookup; a null chart ends in an exception stub at line 436.

Lines 48-62: ask 0x20CFD40 for the track of this instrument and difficulty (`rbp`). Then `0x20D4BE0` builds the note list (`rdi`).

Lines 63-130: make a new measure-grid object (`r13`, later `[rsp+0xb8]`), give it three empty lists, then `0x20DACA0` fills it (the grid restart at a meter change; scout R2's row).

Lines 131-146: stop if the note list is empty (line 137-138); set `r13d` to 0 (line 134). Nothing writes `r13` again before line 285.

Lines 147-152: read the modifier int (`[settings+4]`) into `ebx`; call `0x20D12C0` for a list in `r14`.

Lines 153-156: instrument 6 or 9 jumps to line 228 (the drum path). Any other instrument goes on to lines 157-189, where each set modifier bit calls its own transform (Note Shuffle is the `0x40` bit, line 157-162), and all of those paths end at line 207 with a jump to line 302, past every fill line. So other instruments never reach the fill test.

Lines 228-277: the drum path: its own modifier calls (lines 228-277, including the drum Note Shuffle call at line 259) and then flip/2x kick/other transforms.

Lines 278-282: the fill test begins. Null track or a null fills list (`[track+0x88]`) ends in the exception stub at line 436.

Lines 283-289: compare the fills list's Count (`[rax+0x18]`) with `r13d`, which is 0. If Count is not 0, call `0x20CFF60(notes, chart, track)` (written fills to activation notes; the landing rule is scout R2's) and store its list in `r13` and `[rsp+0xc0]`; then jump to line 189/190.

Lines 293-299: Count is 0: call the generator `0x20D2B30(notes, track, grid)` and store its list in `r13` and `[rsp+0xc0]`; jump to line 190.

Lines 190-207: for either path, the third argument (a bool that defaults to True, the byte at `[rsp+0x178]`) decides one more step. If it is set and the track's Star Power phrase list (`[track+0x70]`) has Count above 0, `0x20D2440(notes, phrases)` runs (it hands the phrases to notes; its signature is at `dump.cs` line 528343). Then `0x20D18F0(notes, track)` runs and the code jumps to line 302.

Lines 302-346: number the notes, build the other lists.

Lines 358-365: if `r13` is still null (every non-drum instrument), make an empty fills list so the result always has one.

Lines 366-424: call `0x20D1ED0` and `0x20D4440`, gather the result fields, and call the result-struct constructor `0x20E8D30`. The fills list is its fourth argument (line 393, `r9`).

Lines 425-440: epilogue, return, and the two exception stubs.
