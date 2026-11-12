# globals_any_bot_235A40.asm (build 6142, RVA `0x235A40`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 235A40` (48 lines). Starts at the method's first instruction: `GlobalVariables`' public bool ʺʳʾʶʿʳʻˁʶˁʿ() (`dump.cs` line 55963).

Plain words: it answers "does any player in the player list play as a bot". It walks `GlobalVariables.playerList` (+0x38, `dump.cs` line 55807) and returns true at the first player that passes two tests: `0x337540`, then the "is_bot" test `0x13F8B0`.

- Lines 1 to 9: set-up; line 5 reads `playerList` into `rdi`, and a null list goes to the null-reference exit (line 44).
- Lines 11 to 18: the loop over the list, reading entry `ebx` into `rsi` (line 16).
- Lines 19 to 23: call `0x337540` on the player; false goes to the next entry.
- Lines 24 to 28: call `0x13F8B0` on the player; true returns 1 (line 32).
- Lines 29 to 31: next entry. Lines 38 to 43: the end of the list returns 0.

The two tests, read with `py -m tools.ch_mcp.chdis 337540 100` and `13F8B0 100` and not kept as listings. `0x337540` (`dump.cs` line 73394, a method of player class #1435 at line 73337) returns true when the player's +0x18 is not null, and otherwise returns the byte at +0xB1 of the player's +0x10. Field +0x18 is a Rewired `Player` (`dump.cs` line 73341); +0xB1 is the byte page 7's IN-64 reads as the remote mark. `0x13F8B0` is the "is_bot" test that IN-64 already reads: the player's +0x10, its +0x48 setting, then the bool conversion `0x210BF30`.

Who calls it: the GameManager load coroutine at 0xD974F (`gm_load_coroutine_D8D80.asm` line 547) and `EndOfSong.Start` at 0x15E997 (`end_of_song_start_15DC20.asm` line 729). Those two were found in the field-write scan, not by a caller search; no caller search was run for this method.
