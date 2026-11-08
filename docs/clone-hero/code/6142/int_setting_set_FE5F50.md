# int_setting_set_FE5F50: the int setting's value setter (build 6142)

Listing: `int_setting_set_FE5F50.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. The method is the `int` instantiation of a `public void ...(T value)` method of the generic setting class #8338 (`dump.cs` line 533732; the instantiation's RVA at line 533735). The int setting class #8308 (`dump.cs` line 532111) derives from #8354 (line 534655), which derives from that generic class with `T = int`.

Plain version: it stores the new value in the setting's +0x10 and then calls the setting's virtual method at +0x2C8, which by its place reads as the "value changed" step (not read).

Line 2 is the store. Lines 3 and 4 tail-call the virtual method. The command-line loader calls this setter to switch "is_bot" on (`cli_loader_3968D0.md`, line 329 of its listing).
