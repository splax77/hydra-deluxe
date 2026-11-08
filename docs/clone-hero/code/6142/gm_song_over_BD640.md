# gm_song_over_BD640.asm  (build 6142, RVA 0xBD640, a GameManager method with a scrambled name)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2). Method start per dump.cs.
Lines 126-128: if isSongOver (GameManager+0x91, dump.cs line 25311) is already set, the method jumps away (to 0xBE982); otherwise it sets isSongOver to 1. The rest of the method was not read.
