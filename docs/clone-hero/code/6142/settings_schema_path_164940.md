# settings_schema_path_164940.asm (build 6142, a stretch of GameInit method `0x164810`)

Made 2026-11-11, session 428cf079 (W10a), with `py -m tools.ch_mcp.chdis 164940 C0` (38 lines). It starts mid-method, inside the static `GameInit` method at 0x164810 (`dump.cs` line 38594), and is not the whole method.

Plain words: at start-up the game builds the path of `settings_schema.json` in the same folder as `settings.ini` and keeps it in a static field. This stretch only stores the path; it does not write or read the file.

- Lines 1 to 3: the end of the store before (static +0x20 of the class loaded just above; not read).
- Lines 4 to 9: call `0x307B40`, the same folder method that `settings_ini_path_377FC0.asm` line 17 uses for `settings.ini`.
- Lines 10 to 18: `System.IO.Path.Combine(folder, "settings_schema.json")`.
- Lines 19 to 38: store that path in static +0x28 of class ʲʷʽʹʺʴˀʵʽˁʻ (TypeInfo slot 0x3D0DF68); the store itself is line 34, and line 38 is the write barrier call.
- Right after this stretch (0x1649FD, not in the listing) the method loads the `version` text asset with `Resources.Load<TextAsset>`; outside this note.

Who reads static +0x28 of that class, and so who writes the schema file, was not found here. Who calls 0x164810, direct callers only (byte scan, `docs/audit/2026-11-08-clone-hero-unknowns/W10a/xref_settings_methods.txt`): `GameInit.PreUnityInit` at 0x1644FC and the `UnpackingScene` coroutine 0x361C60 at 0x36201C.
