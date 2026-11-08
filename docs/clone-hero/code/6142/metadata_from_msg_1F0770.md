# metadata_from_msg_1F0770.asm  (build 6142, RVA 0x1F0770, a #828 song metadata method with a scrambled name)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2c). Method start per dump.cs; dump.cs lists two #828 methods at this one address (their RVA lines are 45135 and 45180).
The same copy as 0x1F07D0 (metadata_from_msg_1F07D0.md), line for line, including +0x18 to the song length at +0x30 (lines 16-17), except that line 22 sets the byte at +0x50 to 0 instead of 1.
A byte scan of rel32 calls and jumps found no direct caller. Its 64-bit address appears twice in the file, which matches its two slots in CloneHero.dll's method-pointer table (see the TM-08 third pass in docs/audit/2026-11-08-clone-hero-unknowns/W2.md).
