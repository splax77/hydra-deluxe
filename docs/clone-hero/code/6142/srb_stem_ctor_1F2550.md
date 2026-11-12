# srb_stem_ctor_1F2550 (build 6142, RVA 0x1F2550)
The constructor of the .srb audio stem class #811. Arguments: the mapped file (rdx), the stem's offset (r8) and its size (r9). Called from `srb_reader_2B84A0` line 450.
Lines 22 to 31: makes a view of the mapped file over that offset and size, kept at +0x10.
Lines 32 to 42: builds a BASS file-callback object (class #248) on the stem, kept at +0x28.
Lines 54 to 57: reads a u32 at byte 0 of the stem and stores it at +0x30.
Lines 58 to 61: reads the u64 at byte 8 into +0x58 and the u64 at byte 16 into +0x60 (the two fields the IV helpers read, `srb_stem_iv_1F2060`). Bytes 4 to 7 are not read.
Lines 62 to 65: reads the u64 at byte 24, clears its top bit, and stores it at +0x38.
Line 63: moves the data pointer past the 32-byte entry header.
Lines 66 to 68 and 75 to 94: when that size plus 32 is larger than the stem's size it throws ArgumentException("size too small").
