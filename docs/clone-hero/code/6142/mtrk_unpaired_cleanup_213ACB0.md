# mtrk_unpaired_cleanup_213ACB0 (build 6142, RVA 0x213ACB0)

The end-of-track step of the MIDI track reader `0x214F6D0` (`docs/audit/note-shuffle/step0/open1/mtrk_read_214F6D0.asm`), which calls it three times on its exit paths (0x214FE6F, 0x214FE7D, 0x214FE8B) with a block of pointers to its own locals at rbp+0xE8. script.json gives this address no name of its own.

It drops every note that was switched on and never switched off. Lines 5 to 10 copy the track's event list (track +0x18) into a local. Lines 19 to 62 walk the reader's first table of open note-ons (one stack of list indexes per channel and pitch, filled at 0x214FBC6) and, for each index still on a stack, write null into that list slot with `set_Item` (line 46). Lines 63 to 114 do the same for the reader's second table (filled at 0x214FE06). Lines 115 to 150 then walk the list from its last slot down to slot 0 and call `RemoveAt` on each null slot (line 143). Lines 151 to 203 dispose of both tables.

So it removes events but never moves one: the events left keep the order the reader added them in. Direct callers only (byte scan of call instructions): the three calls in 0x214F6D0 above, plus one call at 0x2F00DFD, in code with no script.json name near it, which was not read.
