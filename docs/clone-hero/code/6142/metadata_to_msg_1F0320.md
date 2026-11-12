# metadata_to_msg_1F0320.asm  (build 6142, RVA 0x1F0320, a #828 song metadata method with a scrambled name)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2c). Method start per dump.cs; dump.cs lists three #828 methods at this one address (their RVA lines are 45132, 45144 and 45156).
Lines 4-21: copy metadata fields into the object passed in rdx. The double at metadata+0x30 (the song length the load coroutine builds) goes to +0x18 (lines 16-17), the delay at +0x24 goes to +0x20 (lines 14-15), and the bytes and ints at +0x10, +0x14, +0x18, +0x1C, +0x20, +0x38 and +0x39 go to the other slots.
Its only direct caller is 0x922D0 (net_send_metadata_922D0.md), by a byte scan of rel32 calls and jumps.
