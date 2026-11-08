# metadata_from_msg_1F07D0.asm  (build 6142, RVA 0x1F07D0, a #828 song metadata method with a scrambled name)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2c). Method start per dump.cs line 45162.
Lines 4-21: the reverse of 0x1F0320 (metadata_to_msg_1F0320.md). It copies the passed object's fields back into the metadata: +0x18 to the song length at +0x30 (lines 16-17), +0x20 to the delay at +0x24 (lines 14-15), and the rest to +0x10, +0x14, +0x18, +0x1C, +0x20, +0x38 and +0x39.
Line 22: set the metadata byte at +0x50 to 1.
Its only direct caller is the tail-jump in 0x90940 (net_recv_metadata_90940.md), by a byte scan of rel32 calls and jumps.
