# srb_stem_iv_1F2060 (build 6142, RVA 0x1F2060)
A method of the .srb audio stem class #811 that refills the 16-byte block `srb_stem_read_1F2260` xors with. It calls one method of class #1448 (0x334620, not listed) in both branches; class #1448's other methods call the AES key builder `aes_key_build_215AFE0` (see `aes_key_user_333790`).
Lines 11 to 27: when the stream position (rax, doubled, minus 0x20) is not negative, it passes a pointer into the stored bytes at +0x20 plus that offset to 0x334620, so the input is stored data from earlier in the stem.
Lines 28 to 52: otherwise it builds a stack buffer from the two stem fields, +0x60 first and then +0x58 (lines 36 to 39), and passes that to 0x334620.
Caution: both stores at lines 37 and 39 decode as `[rdi]` with no offset, and the doubled position at line 11 is odd, so this listing does not show where in the buffer each half lands.
