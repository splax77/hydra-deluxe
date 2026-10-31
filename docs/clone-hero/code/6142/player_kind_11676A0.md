# player_kind_11676A0: how the drums player picks its engine's player kind (build 6142)

Listing: `player_kind_11676A0.asm` beside this file, made 2026-10-31 (session b15bb484) with `tools/ch_mcp/chdis.py` (its `Image` disassembler, printing a range) from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. The listing is the whole method at RVA `0x11676A0`, from its first instruction to the last byte before the next method (0x1167950); the kind is decided in lines 110 to 132. That method is a method of `BasePlayer<TInputs>` (TypeDefIndex 996, `dump.cs` line 53175) in the instantiation that `DrumsPlayer` (TypeDefIndex 156, `dump.cs` line 10041) derives from. It is not a constructor: it fills a record whose address comes in `rcx` (kept in `rbx`) for the player in `rdx` (kept in `rdi`). Its one direct caller found is the `DrumsPlayer` method at `0x33C560`, at 0x33C8D5.

Plain version: the method asks three questions about the player's slot, in a fixed order, and writes the answer as the engine's player kind. A remote slot gets RemotePlayer (1). Otherwise a slot whose profile has "is_bot" on gets Bot (2). Otherwise a slot whose profile holds a replay gets Replay (3). Everything else gets Player (0). The values are enum #8553 (`dump.cs` line 546294).

The three tests, all on the player's slot field `#901.player` (+0x88 of the non-generic BasePlayer, TypeDefIndex 901, `dump.cs` line 48740; the slot is class #1435 and its profile is at its +0x10):

- The remote test `0x337720` returns the profile's bool at +0xB1 (`dump.cs` line 541694). That the bool means "remote" is the reading of this rule, not a name from the dump.
- The bot test `0x13F8B0` reads the profile's "is_bot" setting at +0x48 and returns the setting's bool conversion `0x210BF30`, which is true when the setting's int at +0x10 is above 0.
- The replay test `0x13F8E0` returns true when the profile's +0xE0 (the replay object, class #8516, `dump.cs` line 25618) is not null.

Lines of the listing:

Lines 1 to 25 are the usual start: save registers, the one-time IL2CPP set-up, clear the record and store the player in its +0.

Lines 26 to 33: copy the profile's modifiers (+0x14, enum #8576) into the record's +8.

Lines 34 to 90 fill the record's bytes at +0xC and +0xD from game-wide state (`GlobalVariables.instance` +0x72, a static setting and the network manager). What those two bytes mean was not traced.

Lines 91 to 109 fill the record's byte at +0xE: 0 when the replay test is true (lines 92 to 96), else 1 when the bot test is true (lines 98 to 103), else the remote test's answer (lines 104 to 108). Line 109 stores it.

Lines 110 to 116: call the remote test. When it is true, jump to line 132.

Lines 117 to 121: call the bot test. When it is true, jump to line 130.

Lines 122 to 127: call the replay test, then turn its answer into 3 or 0 (`neg al`, `sbb ecx, ecx`, `and ecx, 3`).

Line 128 stores that kind in the record's +0x10. Line 130 stores 2 (Bot) and line 132 stores 1 (RemotePlayer) in the same field.

Lines 133 to 156 build the engine's name from the profile's instrument with the text "{0}Engine" and return. Lines 157 to 164 are the throw stubs.

Where the record goes. The base engine constructor `0x20F79E0` reads its third argument's +0x10 and stores it as the engine's player kind at +0x190 (line 93 of `engine_ctor_20F79E0.asm`), and its +0xE into +0x195 (line 91). The record this method fills has the same shape: the player at +0, the profile's modifiers at +8, bytes at +0xC, +0xD and +0xE, the kind at +0x10. The caller at 0x33C560 copies the record into a larger one before it builds the engine; the path from there to the constructor's third argument was not read. So that the engine's kind is this record's kind rests on the matching shape, not on a traced call.

What this proves and what it does not. It proves the order of the three tests and the four values the kind can take. With the loader in `cli_loader_3968D0.md` switching "is_bot" on and the profile holding no replay, a command-line drums player that is not remote gets Bot. With "is_bot" left at 0 it gets Player. It does not prove which part of the game calls this method for every drums player, or the hand-off to the engine named above.
