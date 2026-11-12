# metadata_from_entry_1F0550.asm  (build 6142, RVA 0x1F0550, a #828 song metadata method with a scrambled name)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2c). Method start per dump.cs line 45183.
Lines 5-13: zero the delay and the other small fields, as the encrypted-song branch of 0x1F0160 does (metadata_load_1F0160.asm lines 89-95).
Lines 16-19: store the argument's +0x48 reference at metadata+0x40.
Lines 20-22: store the argument's int at +0x78, as a double, in the song length at metadata+0x30.
Its only direct caller is SongEntry method 0x35B030, by a byte scan of rel32 calls and jumps. What the argument type is was not traced.
