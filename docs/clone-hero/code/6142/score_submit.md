# score_submit: where a player's stats record goes after a song (build 6142)

Listing: `score_submit.asm` beside this file, made 2026-10-31 (session b15bb484) with `tools/ch_mcp/chdis.py` (its `Image` disassembler, printing a range) from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. The listing is the whole method at RVA `0x1144C0`, from its first instruction to the last byte before the next method (0x114E40). It is a method of class #561 (`dump.cs` line 30475) that takes a `Utf8JsonWriter` and writes one player's stats as JSON. Despite the file name, nothing in it posts anything; the name is the question it was made to answer.

Plain version: this method writes one player's block of `scorestats.json`. Its text goes to a file on disk, through File.WriteAllText. A scan for direct calls reported nothing else that it reaches, but that scan's result was not kept. "is_bot" is one of the values it writes into the block, not a test that skips it. Whether anything is sent online after a song was not settled by this trace.

Lines of the listing:

Lines 1 to 130: save registers and the one-time IL2CPP set-up, naming every key string the method writes (lines 31 to 125: "instrument", "difficulty", "profile_name", "score", "is_bot", "remote_network_player" and the rest).

Lines 144 to 164: write "instrument", "difficulty" and "profile_name" as strings (the calls at lines 146, 159 and 164).

Lines 166 to 212: write the "modifiers" array.

Lines 215 to 237: write "remote_network_player", "lefty_flip" and "no_fail" as bools and the two calibrations as numbers.

Lines 248 to 406: write "controller_type", the score parts, the counts and the Star Power numbers.

Lines 407 to 410: read the byte at the record's +0xB6 and write it as the bool "is_bot" (IN-38).

Lines 411 to 498 write the keys that follow and end with the throw stubs; they were not annotated line by line.

Where the text goes: the route the listings show. The stats object's text writer `0x252630` calls this method at line 230 of `scorestats_json_252630.asm`. `EndOfSong.Start` `0x15DC20` calls that text writer at line 266 of `scorestats_writer_15DC20.asm`, and writes the text with System.IO File.WriteAllText (line 270) to "scorestats.json" in the folder from `0x307B40`, which is `Documents\Clone Hero` (`profile_save.md`).

Where the text goes: what the scan reported. A scan of the DLL's code for direct calls (`call` and `jmp` with a 32-bit offset) was run outside these listings. Its result was not kept, and no listing covers the methods named here, so a reader cannot check this paragraph from the repo. The scan reported two callers of this method, and `EndOfSong.Start` as the one direct caller of `0x252630`. It reported the second caller as `0x253EF0` (`dump.cs` line 54708), another JSON builder of the same stats class, whose one direct caller is the `EndOfSong` method `0x161C70` (`dump.cs` line 38788). It reported that method passing the text to File.WriteAllText (0x1621B1 to 0x1621BF), to a path joined from `0x307B40`'s folder and a string slot the dump labels "No". The string labels in those two methods did not fit what the code does (they name menu and log texts), so that file's name is not known.

What this proves and what it does not. It proves the stats record is written into a local JSON file and that "is_bot" is a value inside it. It does not prove that no copy is sent online: the scan followed direct calls only, not calls through method tables, delegates or tasks, and no network code was read. The dump's string list (stringliteral.json) holds the text "Making session request to gateway: https://gateway-service.clonehero.net/api/session/request", so the game does have an online service. The scan placed that text in the method `0x285AD0` and reported no direct caller for it.
