# aes_key_user_333790 (build 6142, RVA 0x333790)
A method of class #1448, one of four in that class that call the AES key builder `aes_key_build_215AFE0` (the others are at 0x333AB0, 0x333E80 and 0x334750; a byte scan of every E8 call in the DLL, 2026-11-08, found no other caller).
Line 40: calls the key builder on a 16-byte stack buffer.
Lines 45 to 47: allocates a uint array.
Lines 53 to 64: copies words of the key into the array.
Lines 66 to 164: a loop that rotates the last word by 8 bits, looks up each of its four bytes in a static byte table on class #1448, xors in a value from a second static table indexed by the pass, and xors the words forward. That is the shape of an AES key expansion; the loop's bound (`cmp r14d, 0x50`) and the array size (0x6F) decode to odd values, so the round count is not read from this listing.
