Listing: `solo_counter_result_2960B0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: `SoloCounter` (`dump.cs` line 7999) method, RVA `0x2960B0` (`dump.cs` line 8072), the whole method. It takes an int, a bool, an int and a double, and it picks the solo's end message. Its direct callers are the `BasePlayer` methods `0x2085D0` and `0x2096F0` (a byte scan for call instructions).

Lines 7 to 10: keep the first int (`ebx`), the bool (`ebp`) and the second int (`esi`).
Lines 15 to 17: store the double at `+0x48` and set the done byte `+0x50` to 1.
Lines 21 to 82: pick a slot of the message array that `solo_counter_message_293CF0.asm` fills, by the first int. Equal to 100 gives slot 0 (`Perfect Solo!`). 95 or more gives slot 1 (`Awesome Solo!`), 90 or more slot 2 (`Great Solo!`), 80 or more slot 3 (`Good Solo!`) and 70 or more slot 4 (`Solid Solo!`). Exactly 69 gives slot 9 (`nice`), 60 or more slot 5 (`Okay Solo...`), and anything else slot 6 (`Messy Solo...`). Slot n is read at array offset 0x20 + 8n.
Line 40: in the last case only (below 60), the second int is set to 0.
Lines 84 to 86: store the chosen text at `+0x78`.
Lines 87 to 99: if the bool is set, replace the text with slot 7 (`Awesome Choke!`).
Line 101: store the second int at `+0x80`.
Lines 100 to 118: reset the box's scale.
