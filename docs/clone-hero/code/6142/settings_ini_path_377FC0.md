# settings_ini_path_377FC0.asm (build 6142, RVA `0x377FC0`)

Made 2026-11-11, session 428cf079 (W10a), with `py -m tools.ch_mcp.chdis 377FC0 86` (29 lines). It is the whole method: the second method of the settings class 193, `public static string` with no arguments (`dump.cs` line 12692).

Plain words: it returns the full path of `settings.ini`, the folder from `0x307B40` joined with the file name.

- Lines 1 to 11: the one-time load of `System.IO.Path`, class `ʵˁʼʾʽʶʵʹʴʸʻ` and the `settings.ini` literal.
- Lines 12 to 17: make sure class `ʵˁʼʾʽʶʵʹʴʸʻ` is set up, then call its static `0x307B40` (`dump.cs` line 67904, returns a string), the folder. Which folder that is was not read.
- Lines 18 to 28: `System.IO.Path.Combine(folder, "settings.ini")`, by a tail jump.

Who calls it, direct callers only (byte scan, `docs/audit/2026-11-08-clone-hero-unknowns/W10a/xref_settings_methods.txt`): `GlobalVariables` method 0x2344B0 at 0x2346B0. The scan's other hit, at 0x3696FE in `BassAudioManager`, has the bytes `48 83` in front, so it is not a call; it was not read. The same folder method and literal also build the path inside the settings loader (`settings_load_374D50.asm` lines 255 to 272).
