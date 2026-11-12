# note_is_kick_214C8A0.asm  (build 6142, RVA 0x214C8A0, a note method that answers "is this note a kick")
Made with tools/ch_mcp/chdis.py (py -m tools.ch_mcp.chdis 214C8A0 400) on 2026-11-12, session c61c1986 (task T1-MID-NO-PRO). Method start per dump.cs line 547077.
Lines 1-3: the answer is true when the note's type, its +0x20 int, is 13. Type 13 is the kick (five_lane_convert_215D450.md gives the type numbers).
The all-toms tail of the five-lane conversion calls it once per note (five_lane_convert_215D450.asm line 215) and leaves a note alone when it answers true.
