# menu_drums_test_D7F20.asm (build 6142, RVA `0xD7F20`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis D7F20` (30 lines), session 428cf079 (task CH-UNK-W9b). A public bool method of the profile class #451 (`dump.cs` line 25655), taking no argument.

Plain words: it says yes when the profile's controller type is a drum kit and one global setting is 1.

- Lines 9 to 11: read the profile's controller type through the getter `0x2123320` (IN-57), subtract 2 and test for 0 or 1. So only Drums (2) and FiveLaneDrums (3) go on; any other type returns false (lines 12 to 15).
- Lines 16 to 23: for a drum type, return the result of `0x210C2F0` on the object in static field +0x140 of class #193 (`dump.cs` line 12642). That method returns whether its int at +0x10 equals 1 (read with chdis, not kept: `cmp dword ptr [rbx + 0x10], 1; sete al` at 0x210C315). Which setting that object holds was not found.

Who calls it: `menu_buttons_1D4BC0.asm` line 84, which uses the answer to pick the Yellow and Blue slots. Not byte-scanned for other callers.
