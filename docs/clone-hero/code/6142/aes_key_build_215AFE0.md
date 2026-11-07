# aes_key_build_215AFE0 (build 6142, RVA 0x215AFE0)
Static method of class #8616; rcx points at a 16-byte buffer.
Lines 6 to 7: two constants, 0x60749CF3495CDCBB (rbx) and 0x14AC2DCA74A88953 (rdi).
Lines 10 to 31: a loop. Each pass takes a 4-bit mask (0xF shifted left by the pass number) and mixes it into both constants with and, or, xor. It stops when the pass count is above ((low 32 bits of one value) xor (low 32 bits of the other)) and 0x3F. Run in Python this gives 41 passes.
Lines 32 to 36: one-time setup of a generic method pointer.
Lines 37 to 51: writes the first value, then the second, into the buffer through a generic writer (0x5D94B0, lines 44 and 51). The values are 0x60749EF7CB5FFEBF and 0x14AC2FCEF6ABAB57; their little-endian bytes equal kSrbAesKey in src/app/preview_source.cpp:53.
