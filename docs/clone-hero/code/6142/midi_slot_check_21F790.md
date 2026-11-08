Listing: `midi_slot_check_21F790.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py` (`py -m tools.ch_mcp.chdis 21F790 b0`). Function: MIDIInputManager's method at RVA 0x21F790 (`dump.cs` line 52813), which takes a player's slot (class #1435) and returns a bool. The velocity helper calls it as its first check (`lane_velocity_337E10.asm` lines 38 to 41).

Plain version: it answers "has a MIDI kit been registered for this slot". It does not look at the MIDI port, the controller or the player's controllers; it only looks for the slot in the manager's own list.

Lines 14 to 18 load the manager's device list (+0x38, `dump.cs` line 52722) and start a count at 0. Lines 20 and 21 leave the loop when the count reaches the list's size. Lines 25 to 27 fetch the record at that place, and line 30, `cmp qword ptr [rax + 0x10], rsi`, compares the record's slot (+0x10, `dump.cs` line 52507) with the argument. A match goes to lines 38 to 43, which return true. Otherwise lines 32 to 37 count up and loop. When the list ends, lines 44 to 49 return false.

The only writer of these records that was read is the register method (`midi_register_21CDE0.asm` lines 127 to 197), which adds one after it has opened a real MIDI port and made the controller.

What the listing does not show: whether any other method adds records to the list, or takes them out.
