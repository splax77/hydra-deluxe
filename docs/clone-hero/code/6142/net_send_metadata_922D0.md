# net_send_metadata_922D0.asm  (build 6142, RVA 0x922D0, a method of class #374 with a scrambled name)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2c). Method start per dump.cs line 22397; its one argument is a song metadata object (class #828).
Lines 7-10: call the metadata method 0x1F0320 (metadata_to_msg_1F0320.md) with the object held at +0x78 of this class, which copies the metadata fields into that object.
Lines 11-16: call the base-class method 0x322B0 with the same +0x78 object and the byte 2. What 0x322B0 does with it was not read.
The load coroutine calls this method only when CHNetManager's state is Connected (gm_load_coroutine_D8D80.asm lines 391 to 410).
