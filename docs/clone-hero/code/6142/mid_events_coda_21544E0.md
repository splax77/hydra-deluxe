# mid_events_coda_21544E0.asm (build 6142, RVA `0x21544E0`, class #8626)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (401 lines). Starts at the method's first instruction, a static method of the `.mid` reader class #8626 (`script.json` names it; the name is scrambled). Read here only for the coda event; the rest of the listing (sections, `[prc`, `[end]`, `_ENDOFSONG`) was not read.

Plain words: when a text event in the `.mid` events track reads `[coda]` or `coda`, and the chart has no coda event yet, the reader makes one at that event's tick and stores it on the chart at `+0x100`. A second coda event is ignored.

- Lines 245 to 270: compare the event's text with `[coda]` (`SequenceEqual`); a match jumps to line 298.
- Lines 272 to 297: the same compare with `coda`; no match moves on (line 297).
- Line 300: `cmp qword ptr [rsi + 0x100], 0`: a coda already stored skips the rest.
- Lines 302 to 310: build a #8500 text event from the string `coda` and the tick `[r14+0x10]` with its constructor `0x211BA30`.
- Line 312: store it at `chart+0x100`.

Whether the text is trimmed or lower-cased before the compare was not read. Its one reader found so far is the fill-skip step `fill_skip_from_coda_215DA70.asm`.
