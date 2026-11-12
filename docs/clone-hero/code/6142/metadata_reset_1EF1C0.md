# metadata_reset_1EF1C0.asm  (build 6142, RVA 0x1EF1C0, a #828 song metadata method with a scrambled name)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2c). Method start per dump.cs line 45174.
Lines 1-11: zero the metadata fields, the song length at +0x30 among them (line 7), clear the SongEntry reference at +0x48 and the byte at +0x50.
Its only direct caller is SongEntry method 0x35B5D0, by a byte scan of rel32 calls and jumps.
