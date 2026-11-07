# Activate Star Power, RVA 0x20F5640 (build 6142), vtable slot 14

Class `#8288`. Listing: `sp_activate_20F5640.asm` (chdis, 2026-10-30). Decompiled lines: `ghidra_decompile_all.txt` 52711 to 52748. The listing starts at the method start the dump gives (dump.cs for class 8288, `Slot: 14` per il2cpp-fields.md line 29).

In plain words: it turns Star Power on. It sets the active flag, remembers when it started, works out when it will end from the meter as it stands, counts the activation and doubles the multiplier. It does not look at how full the meter is: whatever rule says "at least half a bar" lives in the caller, not here.

Lines 1 to 8: save a register and run the one-time init check.
Line 9: load the tempo segment at engine +0x148 (the segment the song clock is in).
Line 10: `[+0x42] = 1`, the Star Power active flag.
Lines 11 to 12: copy the song clock (double at +0x100) to +0x110, the activation time.
Lines 13 to 17: if the segment or the chart record (+0x230) is null, go to line 51 (null throw).
Lines 18 to 23: clock minus segment start, converted to ticks by 0x1820D1D20 with the chart's double at +0xB0.
Lines 24 to 33: add the segment's start tick (segment +0x38), truncate to a whole number and store it at +0x118. That is the activation tick; the drain code later measures ticks since this.
Lines 28 to 34: call 0x20F37E0 (the end-time function) with the meter (+0x68) and the activation time (+0x110).
Lines 35 to 40: activation counter at +0xB4 goes up by one (line 36); the multiplier at +0x48 is doubled (lines 35, 37, 40); the end time goes to +0x120 (line 39).
Lines 41 to 47: if a listener object (+0x1A0) exists, tail-call the event sender with event number 0xB (11).
Lines 48 to 50: otherwise return.
Lines 51 to 52: null-reference throw.
