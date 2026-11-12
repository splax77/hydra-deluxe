# chnet_connected_145FE0.asm  (build 6142, RVA 0x145FE0, a CHNetManager method with a scrambled name)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2c). Method start per dump.cs.
Lines 1-3: return true when the byte at CHNetManager+0x38 is 2, else false.
dump.cs line 36283 names +0x38 as a field of type ConnectionState, and the enum at dump.cs lines 805466 to 805472 gives 0 = NotConnected, 1 = Connecting, 2 = Connected. So this test is "the online connection state is Connected".
