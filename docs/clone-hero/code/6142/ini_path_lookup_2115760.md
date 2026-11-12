# ini_path_lookup_2115760.asm (build 6142, RVA `0x2115760`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 2115760 C0` (46 lines), the whole method. It is a private static method of the ini class #8513 (`dump.cs` line 541104) that takes an ini name and returns a string.

Plain version: it looks the ini name up in the class's name-to-path dictionary (static +0x8) and returns the path. It returns null when the name is not there, or when its path is the text "%internal%".

- Lines 1 to 19: set-up and the one-time loads.
- Lines 20 to 23: the static +0x8 dictionary; null throws (line 44).
- Lines 24 to 29: `TryGetValue(name)`; not found goes to line 41 (return null).
- Lines 30 to 35: a found path equal to "%internal%" also returns null.
- Lines 36 to 40: return the path.

Who calls it, direct callers only (byte scan with `docs/audit/2026-11-08-clone-hero-unknowns/W11b/callscan.py`): 5 sites, all in the ini class, 0x211310D and 0x2113297 (the save method `0x2112DB0`, `ini_save_2112DB0.asm` lines 183 and 276), 0x211357A, 0x21136DD and 0x211507B. Each site at 0x211357A, 0x21136DD and 0x211507B is followed by a call to the document writer `0x2165C50` (0x211358D, 0x21136F0, 0x211508E), in the methods `0x21134B0`, `0x21135B0` and `0x2114A80`; those three were not listed.
