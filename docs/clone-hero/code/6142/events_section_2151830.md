# events_section_2151830.asm (build 6142, RVA `0x2151830`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (868 lines). Starts at the method's first instruction, the `.chart` reader for one line of an `[Events]` section.

Plain words: for an `E "..."` line, take the text between the first quote and the last one, trim it, and take off a leading `[` (and the closing `]` when there is one). Then compare the result, exactly and case-sensitively, with a list of event names. When it is `coda` and the chart has no coda yet, build a coda object and store it on the chart. So only the first coda in the file counts, whichever `[Events]` section holds it.

- Lines 253 to 279: the quote scan. It walks back from the end of the value to the last `"` (`cmp word ptr [r14 + rcx*2 + 2], 0x22`, line 258) and slices the text after the opening quote up to it.
- Line 291: `call System.MemoryExtensions$$Trim` on that text. The trimmed span is left in `r14` (start), `edi` (length) and `ebx`, which every compare below reads.
- Lines 295 to 320: the bracket strip. If the first character is `[` (`cmp word ptr [r14], 0x5b`, line 297), the start moves on one character (line 313). The length loses one more character only when the last one is `]` (`cmp word ptr [r14 + rax*2], 0x5d`, line 304, then the `cmovne` at line 306). So `[coda` reads as `coda`, while `coda]` keeps its bracket.
- Lines 612 to 637: the `end` compare (`ReadOnlySpan<char>.op_Equality`), on the same span.
- Lines 639 to 664: the `coda` compare (`MemoryExtensions.SequenceEqual<char>`, line 662, an ordinal compare). A mismatch jumps on (line 664).
- Lines 665 to 679: the first-only store. `cmp qword ptr [rsi + 0x100], 0` (line 668, 0x21523A7) skips the store when the chart already holds a coda. Otherwise line 677 calls the coda object's constructor `0x211BA30` with the string `coda`, and line 679 (0x21523E0) stores it at `[chart + 0x100]`.

Callers (direct callers only, byte scan on 2026-11-08 with a `call rel32`/`jmp rel32` scan of the `.text` and `il2cpp` sections): `0x2150DFB` in the section dispatcher `0x2150C00` (`section_loop_2150c00.asm` line 131), and `0x216195B` in a twin dispatcher that was not read here.
