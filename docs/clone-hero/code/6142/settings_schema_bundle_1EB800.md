# settings_schema_bundle_1EB800.asm (build 6142, RVA `0x1EB800`, start of the method)

Made 2026-11-11, session 428cf079 (W10a), with `py -m tools.ch_mcp.chdis 1EB800 D0` (44 lines). It is the start of an instance method of class 819 (ʲʴʹʶʻʵʴʸʿʲʶ, `dump.cs` line 44529, `public string`, no arguments) and stops at 0x1EB8C9, before the method ends.

Plain words: the method reads every byte of a file called `Threshold` in a folder from `0x307590`, then hands those bytes, with the name `settings_schema.json`, to another method of the same class. What that method does with them, and what `Threshold` is, was not read.

- Lines 1 to 17: one-time loads of `System.IO.Path`, class 819, class ʵˁʼʾʽʶʵʹʴʸʻ and the literals `settings_schema.json` and `Threshold`.
- Lines 18 to 23: call `0x307590`, a static of class ʵˁʼʾʽʶʵʹʴʸʻ (not the folder method `0x307B40` that the ini path uses).
- Lines 24 to 32: `System.IO.Path.Combine(that folder, "Threshold")`.
- Line 36: `System.IO.File.ReadAllBytes` on it.
- Lines 37 to 41: call `0x1ECC50` of class 819 with (this, the bytes, `"settings_schema.json"`, 0).
- Lines 42 to 44: the result goes on; not read.

Who calls it: none found by the byte scan for `call`/`jmp` rel32 and absolute pointers (`docs/audit/2026-11-08-clone-hero-unknowns/W10a/xref_settings_methods.txt`); only its method-table pointer. A call through a vtable or a delegate would not show.
