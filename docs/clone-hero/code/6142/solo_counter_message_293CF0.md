Listing: `solo_counter_message_293CF0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: `SoloCounter` (`dump.cs` line 7999) method, RVA `0x293CF0`, the whole method. A byte scan for code that loads the `Awesome Choke!` literal found it here (and in `CliGameLauncher`). It fills the class's static string array once, with ten entries, each passed through the method `0x12EE30` (a text lookup, not traced; the same method takes the result-screen texts in `scorestats_writer_15DC20.md`).

Lines 29 to 32: if the static array is already set, return.
Lines 34 to 36: make a string array of 10.
Lines 42 to 179: fill slots 0 to 9 in this order: `Perfect Solo!`, `Awesome Solo!`, `Great Solo!`, `Good Solo!`, `Solid Solo!`, `Okay Solo...`, `Messy Solo...`, `Awesome Choke!`, `soloPointsText`, `nice` (the literal loads at lines 42, 58, 72, 86, 100, 114, 128, 142, 156 and 170).
Lines 180 to 188: store the array in the class's static field.

This listing holds the messages only. The percent thresholds that pick a slot are in another `SoloCounter` method, not listed here.
