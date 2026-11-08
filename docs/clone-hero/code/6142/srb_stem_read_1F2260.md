# srb_stem_read_1F2260 (build 6142, RVA 0x1F2260)
A read method of the .srb audio stem class #811 (interface slot 17): fills a caller's buffer with decoded audio.
Lines 53 to 64: makes a 16-byte block on the stack and fills it through another method of the class (0x1F1D50, not listed).
Lines 66 to 89: the byte loop. Each output byte is the stored byte at the stream position (+0x20 plus +0x40) xored with one byte of the 16-byte block; the position at +0x40 then moves on by one.
Lines 72 to 78: when the index into the block wraps, it refills the block by calling `srb_stem_iv_1F2060`.
Caution: several immediates and displacements in this method decode to odd values (for example `and rbx, 0xffffffffffffffa7` at line 67 and `cmp rbx, -0x2d` at line 72), so the exact block size and wrap test are not read from this listing.
