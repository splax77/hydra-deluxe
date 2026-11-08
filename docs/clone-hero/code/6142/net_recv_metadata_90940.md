# net_recv_metadata_90940.asm  (build 6142, RVA 0x90940, a method of class #374 with a scrambled name)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2c). Method start per dump.cs line 22339.
Lines 11-16: call a generic method of another class (0x5D7EB0) on the object at +0x18, passing the object at +0x78 and the method slot for its instance over the message type that 0x1F0320 fills. That reads like filling the +0x78 object from a received message; the generic method was not read.
Lines 17-29: load GlobalVariables.instance (static field at +0x8, dump.cs line 55829), then its currentSongMetadata (+0x30, dump.cs line 55806).
Lines 30-34: tail-jump to the metadata method 0x1F07D0 (metadata_from_msg_1F07D0.md) with the +0x78 object, which copies its fields into currentSongMetadata, the song length at +0x30 among them.
This is the only direct caller of 0x1F07D0 (byte scan of rel32 calls and jumps).
