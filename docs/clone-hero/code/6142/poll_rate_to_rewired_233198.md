# poll_rate_to_rewired_233198.asm (build 6142, a stretch of GlobalVariables.Start, RVA `0x232E90`)

Made 2026-11-11, session 428cf079 (W10a), with `py -m tools.ch_mcp.chdis 233198 3A` (14 lines). It starts mid-method, at the instruction 0x233198 inside `GlobalVariables.Start` (`dump.cs` line 55902, `private void Start()`), and is not the whole method.

Plain words: when the game starts, it hands the `poll_rate` setting's whole-number value to Rewired as the joystick refresh rate. Rewired is the input library the game uses.

- Line 1: `Rewired.ReInput.get_configuration`, the object that holds Rewired's settings.
- Lines 2 to 4: load the settings statics of class 193 (`dump.cs` line 12599) and read the field at +0xC0. The settings loader stores the `poll_rate` setting object there (`settings_load_374D50.asm` line 1671).
- Lines 5 to 8: skip ahead when either object is null.
- Line 9: read the dword at +0x10 of the setting object, its value as a whole number.
- Line 12: `Rewired.ReInput.ConfigHelper.set_joystickRefreshRate(int value)` (`dump.cs` line 307817) with that value.
- Lines 13 to 14: the next setting, the one at +0x1E8, goes on to `Cursor.set_visible`; outside this note.

Who reads the +0xC0 field: `docs/audit/2026-11-08-clone-hero-unknowns/W10a/static_poll_rate_C0.txt`, a pattern scan (load of the class's TypeInfo slot, then +0xB8, then +0xC0) by `static_field_scan.py` beside it. It found this read, two in `GameplaySettingsMenu` (0x1AEE40 in 0x1AE1F0 and 0x1AFD51 in 0x1AF540, not read here), the loader's store, and one false hit on a stack slot in `SongEntry` (0x35C3C8). Who calls `GlobalVariables.Start`: Unity, by name; the byte scan in `xref_settings_methods.txt` found only its method-table pointer (direct callers only, byte scan).
