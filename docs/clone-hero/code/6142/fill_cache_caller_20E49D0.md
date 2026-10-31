# Fills reader that calls the twin, `#8251.ReadFills`, RVA `0x20E49D0` (build 6142)

Listing: `fill_cache_caller_20E49D0.asm`, 126 lines, starts at the first instruction (`script.json` lists it as `TrackStreamReader$$ReadFills`, in our readable names, class `#8251`). Made with `tools/ch_mcp/chdis.py`, 2026-10-30. Arguments: the reader object (`rcx`, its stream at `[reader+0x10]`) and the track (`rdx`, kept in `r15`).

Lines 1-14: call 0x20CF940 on the stream to read a count into `r12d`. If the count is 1 or less (line 13-14) the function returns at once.

Lines 15-26: keep registers; loop index `r13d` starts at 0; one-time set-up of a metadata slot.

Lines 27-51: first read in the loop. A helper makes an 8-byte buffer (0x1801BD810), the stream's reader (0x1805F18B0) fills it, and the first 8 bytes become the first value, kept in `rbp` (line 53). That value is passed as the start tick.

Lines 52-100 (listing addresses 0x20E4AAD-0x20E4B2F): the second read the same way. Its 8 bytes are read at 0x20E4B4E into `rbx` and passed as the length.

Addresses 0x20E4B34-0x20E4B62: a virtual call on the stream object (`[rax+0x388]`) returns a number; `cmp eax,1 / sete r9b` turns it into the boolean flag.

Line at 0x20E4B79 (`call 0x18215b7c0`): add a fill to the track `r15` with (tick `rdx = rbp`, length `r8 = rbx`, flag `r9b`). Then the loop adds 1 to its index and goes round while the index is below the count (`0x20E4B7E-0x20E4B87`).

This is a deserialiser: the count, then per fill a tick, a length and a flag. What it deserialises (a saved copy of a parsed chart) is the survey's reading and not something I read here; the instructions show only a stream reader that adds fills to a track.
