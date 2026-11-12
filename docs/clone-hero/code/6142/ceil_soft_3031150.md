Listing: `ceil_soft_3031150.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: the software route of the rounding helper 0x3031130 (`ceil_helper_3031130.md`), RVA 0x3031150, the whole routine. It works on the double's bits.

Lines 4 to 11: `r9` is 1 when the number is negative; `r8` is the exponent field.
Lines 12 to 18: an exponent of 0x433 or more means the number is already whole (2 to the 52 or more). It is returned as it is, unless it is infinite or not a number, which goes to 0x304B9C0 (not listed).
Lines 19 to 29: below 1 in size (exponent under 0x3FF): zero is returned as it is, a negative number gives minus zero, and a positive one gives 1.0.
Lines 30 to 37: otherwise clear the fraction bits, which cuts the number toward zero.
Lines 38 to 42: if the number is positive and the cut changed it, add 1.0.

So every route returns the smallest whole number at or above the input: this is ceiling.
