# int_setting_clamp_FE5B10: the int setting's clamp, the step the setter ends in (build 6142)

Listing: `int_setting_clamp_FE5B10.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis FE5B10 200`.

Plain version: it pulls the setting's value back inside the setting's own minimum and maximum, and does nothing else.

The setter `0xFE5F50` stores the new value at +0x10 and tail-calls the setting's virtual method at vtable +0x2C8 (`int_setting_set_FE5F50.asm` lines 2 to 4). That +0x2C8 is slot 25 of the generic setting class (`dump.cs` lines 533834 to 533840, `protected virtual void` with `Slot: 25`; the vtable starts at +0x138 and each slot takes 16 bytes, which matches the setter's own reads of +0x2C8 and +0x2D0, so 0x138 + 25 x 16 = 0x2C8). The int instance of that slot is this function at RVA 0xFE5B10, and neither subclass of it (TypeDefIndex 8354 and 8308, the class of the `song_speed` object) lists a Slot 25 override.

Lines 15 to 17 read the value (+0x10), the minimum (+0x14) and the maximum (+0x18). Lines 30 to 35 compare the value with the minimum through the integer compare call; when it is below, line 45 stores the minimum. Lines 37 to 43 compare it with the maximum and store the smaller of the two. The function then returns (line 52). It calls no event, writes no file and reads no other setting, so no other setting's value can cap this one here.
