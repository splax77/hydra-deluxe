Listing: `solo_counter_percent_294C60.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: `SoloCounter` (`dump.cs` line 7999) method, RVA `0x294C60` (`dump.cs` line 8057), the whole method. It takes one int and writes the running percent text. Its direct callers are the `BasePlayer` methods `0x2086C0` and `0x2096F0` (a byte scan for call instructions).

Lines 11 and 12: do nothing once the done byte `+0x50` is set (the end message is up).
Lines 13 and 14: do nothing if the int equals the last one shown, kept at `+0x84`.
Lines 16 to 30: set the text object `+0x28` to the int followed by `%`.
Line 33: remember the int at `+0x84`.
Lines 34 to 36: set the byte `+0x51`, and the two floats at `+0x54` and `+0x58` to 1.15 (`0x3F933333`), which reads as the start of a size pulse.

The int arrives already worked out; this method does no division.
