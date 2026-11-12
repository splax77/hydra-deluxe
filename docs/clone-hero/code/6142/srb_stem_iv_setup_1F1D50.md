# srb_stem_iv_setup_1F1D50 (build 6142, RVA 0x1F1D50)
A method of the .srb audio stem class #811, called once, from the stem read method 0x1F2260 at 0x1F2357 (direct callers only, byte scan of call instructions).
Lines 4 to 21: it rounds the stream position (+0x40) down, multiplies it by four and, when the result is at least 1, passes a pointer into the stored bytes (+0x20 plus that offset) to `0x334A30` of class #1448.
Lines 22 to 41: otherwise it builds a stack buffer from the two stem fields, +0x60 first and then +0x58, and passes it to `0x334340` of class #1448.
Caution: the mask 0xFFFFFFFFFFFFFFE0, the multiply by four and the second store at offset 2 (line 32) are not the values a 16-byte block would need, so this listing does not show where each half lands.
