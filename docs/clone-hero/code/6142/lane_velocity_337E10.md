Listing: `lane_velocity_337E10.asm`, build 6142, made 2026-11-06 with `tools/ch_mcp/chdis.py`. Function: the helper at RVA 0x337E10, which returns one lane's signed velocity byte. The dump has no line for this address, so it has no name and no facts-table row. Its argument is the Rewired action id. The drums input method calls it once per lane (`drums_input_source.asm` lines 145 to 187).

Plain version: the game looks at a hit's strength only for a player on a MIDI kit, or a player holding a custom controller with id 6. For anyone else, and for any lane not pressed this frame, the answer is -1.

The first check asks whether MIDIInputManager exists, and if so whether the player's slot is in its list of registered kits. The second is ContainsController with type 20 (Custom) and controller id 6. If GetButtonDown is true for the action and either check holds, the value is round((GetAxis - 0.5) * 2 * 127). Otherwise it is 0xFF, which is -1 as a signed byte.

What the listing does not show: why the game tests id 6, and how Rewired numbers a custom controller.
