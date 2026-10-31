Listing: `star_table_cctor_214C320.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. Function: the static constructor of #8611, RVA `0x214C320`. It builds the star table.

Lines 1 to 11: prologue and one-time type setup.
Lines 12 to 14: make a new float array of 9 elements.
Lines 15 to 19: fill it with RuntimeHelpers.InitializeArray from the field `<PrivateImplementationDetails>.6C5DB40D...`. The dump says that field is 36 bytes at metadata offset 0x891AC0 (`dump.cs` line 541526); the bytes there are the floats 0.1, 0.5, 1.0, 2.0, 2.8, 3.6, 4.4, 5.2, 6.0 (read directly from `global-metadata.dat`, one occurrence).
Lines 20 to 22: store the array into the static field `#8611.ʼʲʺʹʹʽʹʽˁʴʽ` and return.
