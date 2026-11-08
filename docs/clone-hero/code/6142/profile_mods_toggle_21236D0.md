# profile_mods_toggle_21236D0: the profile's modifier toggle (build 6142)

Listing: `profile_mods_toggle_21236D0.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. The method is `public void ...(modifier)` of the profile base class #8526 (`dump.cs` line 541753). The profile's modifiers word, enum #8576, is at +0x14 (`dump.cs` line 541673; the enum's values from line 547679, `PrecisionMode = 4096` at line 547692).

Plain version: the method flips one modifier in the profile's modifiers word. If the word is `None` (1) it is cleared first. The modifiers that clash with the new one are taken out. If the modifier was already on it is turned off, otherwise it is turned on. A word left at 0 becomes `None` again, or `GamepadMode` (0x100000).

Lines 5 to 13: a request for `GamepadMode` itself, or for 0x20000 while `GamepadMode` is on, does nothing. Lines 14 and 15: a request for `None` jumps to line 38, which resets the word. Lines 18 to 20: a word equal to 1 is set to 0. Lines 21 to 23: `ebx` is the word, `edi` is the word AND the request (is it already on). Lines 25 to 27: `0x2122EF0` (not read) returns a mask for the request, and the word keeps only the bits outside it. Line 30 stores that. Lines 29 and 32 to 34: when the modifier was not already on, it is ORed in and stored. Lines 35 to 43: a word of 0 becomes `GamepadMode` when `0x2123150` returns true, else `None`.

Direct callers (byte scan): `0x32D3A2` in `0x32D360`, `0x32E241` in `0x32E190` and `0x330AFA` in `0x330AB0`, all `PlayerSelectionMenu` methods (`dump.cs` lines 72991, 72943 and 72946). Each passes the entry of the list `0x32CE60` returns at the cursor's index, for the profile of the menu's slot (`+0x20`, then `+0x10`).

What it does not show: what `0x2122EF0` counts as a clash for `PrecisionMode`, so whether it can turn DoubleKick off is not read.
