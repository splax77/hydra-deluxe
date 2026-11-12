# stem_length_36E100.asm  (build 6142, RVA 0x36E100, a static BassAudioManager method with a scrambled name)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2). Method start per dump.cs.
Line 25: asks a native-audio wrapper class for the channel's length; -1 means failure (lines 26-27). Line 37: a second method of that class turns it into a double. Lines 38-39: a negative result logs "Failed to convert length {0}" (line 51). That the wrapper is the BASS library's length-in-bytes and bytes-to-seconds pair is a reading of the error text, not checked.
