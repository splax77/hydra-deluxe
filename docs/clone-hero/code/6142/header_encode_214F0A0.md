# header_encode_214F0A0 (build 6142, RVA 0x214F0A0)
Static method of class #8599, rcx = n, rdx = key, no calls.
Lines 1 to 8: y = n + 1, then mix(y) with the same mix as the decoder (xor-shift under 0x550055).
Lines 9 to 14: multiply by 0x2B86CEB, xor with the key, keep 37 bits (0x1FFFFFFFFF), return.
