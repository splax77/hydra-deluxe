# header_decode_214EFD0 (build 6142, RVA 0x214EFD0)
Made with tools/ch_mcp/chdis.py on 2026-10-30. Static method of class #8599, two 64-bit arguments (rcx = a, rdx = b), no calls, no globals.
Lines 1 to 5: x = ((a xor b) times 0x308DBC3) and 0x1FFFFFFFFF (keeps 37 bits).
Lines 6 to 11: mix(x): t = ((x shifted right 7) xor x) and 0x550055; result = (t shifted left 7) xor t xor x.
Lines 12 to 13: subtract 1 and return. Inverse of header_encode_214F0A0 (checked on 200,000 random pairs in Python, 0 mismatches).
