# chnet_test_146A90.asm  (build 6142, RVA 0x146A90, a CHNetManager method with a scrambled name)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2c). Method start per dump.cs.
Lines 9-14: when the CHNetManager connection state (+0x38) is not 2 (Connected, see chnet_connected_145FE0.md), return false.
Lines 15-23: otherwise take the static instance of class #374 (dump.cs line 22143) and tail-jump to its method 0x90610 (net_int_compare_90610.md), so the result is that method's result.
