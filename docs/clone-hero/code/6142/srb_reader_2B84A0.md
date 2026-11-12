# srb_reader_2B84A0 (build 6142, RVA 0x2B84A0)
The constructor of the game's .srb reader (an obfuscated class; its static-init list names BassAudioManager, the header class #8599 and the stem class #811). rdx is the file path. Found as the only direct caller of the header decoder `header_decode_214EFD0` (a byte scan of every E8 call in the DLL, 2026-11-08); the scan misses calls made through a pointer.
Lines 41 to 66: opens the file with File.OpenRead and maps it with MemoryMappedFile; r12 is the mapped length.
Lines 88 to 95: reads bytes 0 to 7 as a little-endian u64, then byte-swaps each 32-bit half and swaps the halves, so the value passed on is bytes 0 to 7 read big-endian.
Lines 99 to 102: reads bytes 8 to 15 as a little-endian u64.
Lines 107 to 113: calls the header decoder with bytes 8 to 15 as its first argument and the big-endian bytes 0 to 7 as its second.
Lines 114 to 116: the result is a byte count; a count above the file length leaves the reader.
Lines 130 to 160: copies that many bytes from file offset 16 into a new array.
Lines 169 to 172: inflates them with `srb_inflate_2E0A10` (a raw DeflateStream).
Lines 176 to 182: reads a u32 at offset 0 of the inflated song info, stores it at +0x40, and leaves the reader when it is not 0x1346234 (20210228).
Line 189: a call (0x1CF270, not listed) that reads on from offset 4 with a position held by reference.
Lines 190 to 198: reads a u64 (the chart's stored size); above the file length leaves.
Lines 201 to 207: reads a u32 count, compared with the length of an array on BassAudioManager's statics; larger leaves.
Lines 208 to 214: reads one byte and stores it at +0x44.
Lines 217 to 223: reads a u32 count, stored at +0x5C; above 0x10000 leaves.
Lines 224 to 241: when the byte has bit 2 set reads a u32 into +0x50; when it has bit 1 set reads a u32 into +0x54.
Lines 242 to 270: reads +0x5C u64 values into +0x48; any above the file length leaves.
Lines 279 to 349: reads two arrays of u64 values, one per counted item of line 206, into +0x28 and then +0x30; any above the file length leaves.
Lines 350 to 404: copies the chart's stored bytes from file offset 16 plus the song-info size, inflates them with `srb_inflate_2E0A10`, and keeps the result at +0x20.
Lines 405 to 417: +0x58 is the offset right after the chart; the loop adds every +0x48 value to it, so the bytes those values count are passed over here.
Lines 418 to 478: for each item of line 206 builds a stem object (`srb_stem_ctor_1F2550`) on the mapped file at the running offset with that item's +0x30 value as its size, then moves the offset on by that size.
Line 511 onward: every "leaves" above disposes the view and jumps to the shared exit at 0x2B8D8B, which this listing does not follow.
