# cli_loader_3968D0: the command-line player loader, and the one instruction that makes a bot (build 6142)

Listing: `cli_loader_3968D0.asm` beside this file, made 2026-10-31 (session b15bb484) with `tools/ch_mcp/chdis.py` (its `Image` disassembler, printing a range) from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. The listing runs from the function's first instruction (RVA `0x3968D0`) to the last byte before the next method (0x3970D0). The function is the `CliGameLauncher` method declared `public bool ...(int ...)` at `dump.cs` line 17992 (class `CliGameLauncher`, TypeDefIndex 282, `dump.cs` line 17909). Its one argument is the player slot's index (`edi`); `rsi` is the launcher.

Plain version: when the game is started with `--song` and `--player`, this function sets up one player slot. It takes the instrument and difficulty from the parsed `--player` words, writes them into a profile, and, when the launcher holds no replay, switches the profile's "is_bot" setting on with the value 1. On Drums and ProDrums it also sets the profile's controller type to Drums and its modifiers to DoubleKick. Then it puts the profile into the slot. The `1` that makes the player a bot is the immediate of one instruction, `mov edx, 1` at 0x396EC5, so the byte that holds it is 0x396EC6.

The objects. The launcher's fields are in `dump.cs` lines 17912 to 17932. The profile (`rdi` from line 254 on) is class #451, whose base class #8526 (`dump.cs` lines 541668 to 541698) holds the fields written here: the instrument (enum #8326) at +0x10, the difficulty (enum #8617) at +0x11, the modifiers (enum #8576) at +0x14, a string setting at +0x18, and int settings (class #8308) at +0x48, +0x50 and +0x60. The profile constructor names +0x48 "is_bot" and +0x60 "controller_type" (0x2123CE3 and 0x2123B0F in `0x2123890`, read 2026-10-31; no listing of it is committed). The player slot (`rbp` from line 210 on) is class #1435 (`dump.cs` lines 73337 to 73345): the profile at +0x10, the Rewired `Player` at +0x18, the index at +0x28.

Lines of the listing:

Lines 1 to 45 are the usual start: save registers and the one-time IL2CPP set-up of the string literals and types the function uses.

Lines 46 to 64: read entry `edi` of the launcher's list at +0x20 (the parsed `--player` pairs; the call is the shared two-byte list getter). `bl` is the instrument and the byte above it, saved at `[rsp+0xe9]` and read into `r12b` on line 76, is the difficulty. Lines 60 to 64 keep a second copy in `ebp` with ProDrums (9) turned into Drums (6); that copy is only used to look up the chart.

Lines 65 to 118: load the song entry at +0x40 and its chart for that instrument and difficulty. When a replay path is set (+0x50), check it and compare the replay's chart hash with the song's (Blake3's hash inequality, line 116).

Lines 119 to 194: the error path, which builds "Replay file ... does not match the song file ..." and shows it with an OK button; the function then returns false (lines 184 to 194).

Lines 195 to 213: fetch `GlobalVariables.instance`, then entry `edi` of `GlobalVariables.playerList` (+0x38), and write the index into the slot's +0x28 (line 213).

Lines 214 to 248: choose the slot's Rewired player. When the launcher's bool at +0x58 is false (line 215 jumps to line 235), call Rewired's GetSystemPlayer (line 244; `dump.cs` line 310436). When it is true, take an entry of Rewired's player list through the IList of Player interface instead (lines 217 to 233; the helper at 0x2C421D0 was not read, so which entry is not checked). Either way the result is stored in the slot's +0x18 (line 246). What sets +0x58 was not traced.

Lines 249 to 292: get the profile from the launcher's object at +0x38 (the call at 0xD8470, line 253), set its int setting at +0x50 to 0 and its string setting at +0x18 to the empty string, and set its +0xA8 from the colour manager.

Lines 293 to 319: only when the launcher's bool at +0x48 is set: set the string at +0x18 from a lookup on the instrument and set the +0x50 setting to 1. Neither field's meaning was traced.

Lines 320 and 321: write the instrument byte to profile +0x10 and the difficulty byte to +0x11.

Lines 322 and 323: compare the launcher's replay field at +0x60 with null. When a replay is set, jump to line 358.

Lines 324 to 330, the bot write: load the "is_bot" setting from profile +0x48, put 1 in `edx` (line 328, `mov edx, 1` at 0x396EC5; its bytes are `BA 01 00 00 00`, so the `1` is the byte at 0x396EC6) and call the int setting's setter at 0xFE5F50 (line 329), which stores `edx` in the setting's value at +0x10. Then write 1 to the modifiers at +0x14 (line 330); 1 is None in enum #8576 (`dump.cs` line 547680).

Lines 331 to 334: compare the instrument with 6 (Drums) and with 9 (ProDrums). Either jumps to line 350, so ProDrums takes the drums branch.

Lines 335 to 349, the other instruments: when the instrument is at most 14 and its bit is set in the mask `0x4C30` (bits 4, 5, 10, 11 and 14: SixFretGuitar, SixFretBass, SixFretRhythm, SixFretGuitarCoop and SixFretKeys in enum #8326), the controller type gets 1 (SixFretGuitar); otherwise it gets 0 (Guitar). Both go to the setter call on lines 406 and 407 with the "controller_type" setting at +0x60.

Lines 350 to 357, the drums branch: call the same setter with 2 (Drums in enum #8309) on "controller_type" (+0x60), then write 2 to the modifiers at +0x14. In enum #8576, 2 is DoubleKick (`dump.cs` line 547703; AllStrums has the same value).

Lines 358 to 405, the replay branch: skip the bot write. Take the controller type from byte +0x6C of the replay's object at +0x20 (line 367), the modifiers from its +0x70 (lines 376 and 377), the string setting at +0x18 from its +0x50, keep the replay in profile +0xE0, and set the profile's +0x38 setting from its byte +0x8D.

Lines 408 to 413: every successful branch ends here. Store the profile in the slot's +0x10 (line 409) and return true.

Lines 414 to 452: the "Unable to load chart file" error path and the two throw stubs.

What this proves and what it does not. It proves that a command-line start with no replay always switches "is_bot" on for the slot it loads, with the value 1, and that the value comes from the instruction at 0x396EC5 and nowhere else in this function. It proves ProDrums takes the drums branch. It does not show what reads "is_bot" later; that is the kind builder in `player_kind_11676A0.md`. It does not show whether the System Player stored at +0x18 is the player the drums input reads.
