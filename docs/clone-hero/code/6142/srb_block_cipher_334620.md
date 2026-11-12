# srb_block_cipher_334620 (build 6142, RVA 0x334620)
A method of class #1448 (two byte-pointer arguments, returns an int). It does no cipher work itself. Lines 13 to 52 pick three arrays: one from the object's +0x10 (from element +0x24 on) and two from the class's static fields +0x10 and +0. Lines 53 to 59 pass them, with the two pointers, to `0x336E50`, a method of a class nested in #1448 (not listed). Line 61 returns 0x7E.
So the rounds are in 0x336E50, and this listing cannot show the block size or round count.
