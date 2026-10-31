Listing: `note_hit_dynamics_20DE770.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. Function: drum engine (#8255) virtual slot 11, RVA `0x20DE770`: the handler for one hit note. Arguments: the engine and the note.

Lines 1 to 7 and 136 to 137: prologue; a null note goes to the error call.
Lines 8 to 22: if the note is not a kick-type note (bit 0 of `note+0x80` clear), a streak-style counter at `engine+0x2C4` is non-zero and the note is not flagged 0xC000, count up at `engine+0x2EC`; at 3 it clears `+0x2EC`, `+0x2C4` and two flag bytes (lane-pair bookkeeping for the pad; I did not trace it further).
Lines 23 to 28: if the note carries flag 0x10000, set `engine+0x2F1` and copy the clock field `engine+0x100` into `engine+0x168`... (the copy is `param_1[0x5b] = param_1[0x20]`, the song clock at `+0x100` into `+0x2D8`).
Lines 29 to 61: pick one signed byte for the note's lane: the note's lane mask `note+0x80` is compared with 2, 4, 8, 0x10, 1, 0x20, 0x40 in turn, and the byte at `engine+0x30A`, `+0x30B`, `+0x30C`, `+0x30D`, `+0x30E`, `+0x30F` or `+0x310` (or `+0x309` as the default) is loaded into `sil`. That byte is the lane's value.
Lines 62 to 71: ask the note for its accent flag (`note+0x7C` bit 0x40000). If not an accent, ask for the ghost flag (bit 0x80000); if neither, jump to line 105 (a plain note).
Lines 72 to 83 (ghost): load the threshold `engine+0x2C0`. If the lane value is below it, or negative, jump to line 84. Otherwise set the "performed" mark (bit 4 of `note+0x84`, through `0x214B740`), add 1 to the counter at `engine+0xDC`, and go on to line 105.
Lines 84 to 90 and 103 to 104: if bit 0x10 of the flags word at `engine+0x198` is set, return now (the note is not scored and the streak does not step). Otherwise go on to line 105.
Lines 91 to 102 (accent): load the threshold. If the lane value is at or above it, or negative, jump to line 103. Otherwise set the "performed" mark, add 1 to `engine+0xD8`, and go on to line 105.
Lines 105 to 117: if the note has a ghost or accent flag (`note+0x7C` & 0xC0000) but not the performed mark, call `0x20F5FF0`, which only raises a listener event with code 4.
Lines 118 to 121: set bit 0 of `note+0x84` (`0x214BB60`; the note is hit).
Lines 122 to 125: call `0x20F63F0` (the streak, hit count and multiplier update).
Lines 126 to 135: load the engine's virtual table, put 0x32 (50, the base points) in `r8d`, and jump through slot 6 (`+0x198` of the class, RVA `0x20DEEC0`) with the note and 50.
