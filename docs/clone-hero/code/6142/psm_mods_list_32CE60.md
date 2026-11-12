# psm_mods_list_32CE60: the modifier list the player menu offers (build 6142)

Listing: `psm_mods_list_32CE60.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. The method is a `PlayerSelectionMenu` method (`dump.cs` line 72916, class TypeDefIndex 1429).

Plain version: it picks one of three arrays of modifiers held by the menu and returns it, filtered when the online leaderboards are on. All three arrays hold `PrecisionMode` (4096).

Lines 21 to 30: the array is the menu's +0x258 when the byte at +0x242 is set, else +0x250 when +0x243 is set, else +0x248. Lines 33 to 35: when the leaderboards manager's test `0x139AD0` is false, that array is returned as is. Lines 36 to 82: otherwise a list of one value (8, `AllTaps`) or of three values (a static array) is made and the array is filtered through `Where` and `ToArray`; the filter's own test (in the nested class #1428) was not read.

Where the three arrays come from: the `PlayerSelectionMenu` method `0x331630` (`dump.cs` line 72955) fills +0x248, +0x250 and +0x258 from three static array blobs (0x33174F, 0x33178B and 0x3317C7). Their bytes are in `global-metadata.dat` at the offsets `dump.cs` lines 27250, 27238 and 27240 give, read 2026-11-08 as little-endian 32-bit values:

- +0x248, offset 0x8607E0, 21 values: 1, 4194304, 4096, 8192, 32768, 65536, 131072, 262144, 524288, 2, 4, 8, 16, 128, 32, 64, 16384, 256, 512, 1024, 2048.
- +0x250, offset 0x8592C8, 21 values: the same with 2097152 in place of 4194304.
- +0x258, offset 0x859338, 13 values: 1, 2, 4096, 32768, 16, 4, 8, 32, 64, 256, 512, 1024, 2048.

Which array a drums player sees (which flag +0x242 or +0x243 means) was not traced. 4096 is the third entry of each.
