# audio_load_36B700.asm  (build 6142, RVA 0x36B700, a BassAudioManager method with a scrambled name that takes a SongEntry; dump.cs line 12284)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2). Method start per dump.cs.
Line 57: calls the BassAudioManager method 0x369810 (audio_reset_369810.md), which zeroes audioLength (+0x40) and originalAudioLength (+0x48).
Lines 207 and 470: r12d is the stem index; it starts at 0 and goes up by one per stem.
Lines 349-366: each opened stem is stored in the stem array at BassAudioManager+0x20, at index r12d. The .sng branch (lines 478 to 544) rejoins this path at line 368 (the jump at line 544).
Lines 404-416: the stem's handle goes to 0x36E100 (stem_length_36E100.md), which returns its length in seconds. Line 413 compares it with audioLength (+0x40), and only a larger value is stored (line 416). So audioLength ends as the longest stem.
Lines 669-670: at the end, audioLength is copied to originalAudioLength (+0x48).
Field names from dump.cs lines 11876 to 11885. Lines 586 to 657 (the encrypted-song branch) were not read line by line.
