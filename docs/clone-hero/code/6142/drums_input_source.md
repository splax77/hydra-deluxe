# drums_input_source: where the drums player reads its pads (build 6142)

Listing: `drums_input_source.asm` beside this file, made 2026-10-31 (session b15bb484) with `tools/ch_mcp/chdis.py` (its `Image` disassembler, printing a range) from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. The listing is the whole method at RVA `0x33A880`, from its first instruction to the last byte before the next method (0x33B040). It is a `DrumsPlayer` method (TypeDefIndex 156, `dump.cs` line 10158), a `protected override` that takes the song time and a `ref` to the drums inputs record, and fills that record.

Plain version: each time this method runs, it goes to the player's slot, takes the Rewired player stored in the slot, and asks that Rewired player which buttons are down. It does not keep the Rewired player anywhere between reads: before nearly every button read it loads the slot and the slot's Rewired player again. So the Rewired player that counts is whatever the slot holds at that moment, not a copy taken when the drums player was built.

The fields it reads. The slot is the drums player's field `#901.player` (+0x88; the class is #1435, `dump.cs` line 73337). The slot's +0x10 is the profile and its +0x18 is a Rewired `Player` (`dump.cs` line 73341). The loader stores the System Player in that +0x18 (`cli_loader_3968D0.md`, line 246 of its listing).

Lines of the listing:

Lines 1 to 10 save registers and keep the drums player in `r13` and the inputs record in `rbx`.

Line 11: when the drums player's byte at +0xC0 is 0, jump to line 210 and return with no read. What that byte means was not traced.

Lines 19 to 26: load the slot from +0x88 and its profile from +0x10; when the profile's byte at +0xB1 is set (the remote test of IN-64 reads the same byte), jump to line 204 and return with no read.

Lines 27 to 32: load the slot's Rewired player from +0x18 and call Rewired's Player GetButtonDown (RVA `0x1F39540`, `dump.cs` line 306997) with action 4.

Lines 33 to 37 load the slot from +0x88 and its +0x18 again for the next read. The same pattern repeats down to line 292: the listing holds 17 calls into Rewired's `Player` (`GetButtonDown` and `GetButton`, RVA `0x1F3B730`, `dump.cs` line 306991) and 16 loads of the slot from +0x88.

Lines 293 to 496 turn the button states into the inputs record (calls into the record's class), and line 200 to 203 build and store the record's last part. Lines 497 to 501 are the throw stubs.

What this proves and what it does not. It proves the drums pads are read from the slot's Rewired player, through the slot, at the time of each read. It does not prove how often the method runs: it is an override and has no direct caller, so it is reached through the class's method table. `BasePlayer.Update` (RVA `0x20A1B0`) makes a call through the method table in its input loop (at 0x20A397), which may be this one; that was not checked. It also does not say whether Rewired lets a controller added to the System Player be read there; that is the live question LS-D1 of the direct-start plan.
