# cli_start_398F00: the command-line start, from the player loader to the scene change (build 6142)

Listing: `cli_start_398F00.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis 398F00 400`.

Plain version: it runs the command-line player loader once per player, and only then asks for the "Gameplay" scene. So the chart is loaded before the gameplay scene, and its `GameManager`, exist.

Lines 44 to 58 are the loop over the players: line 49 calls the loader `0x3968D0` (`cli_loader_3968D0.asm`), and line 52 counts each player that loaded. Lines 59 to 88 show the "No players were loaded" error when none did. Otherwise lines 97 to 104 call the int setter `0xFE5F50` with 0 on the settings static at +0x150 (which setting that is was not read; it is not `song_speed`, which is +0x10). Lines 115 to 122 then call the scene change with the string 'Gameplay'.

The loader loads the chart at its line 72 (`cli_loader_3968D0.asm`, the call to `0x35C110`), and that load reads the speed ratio (`songentry_load_35C110.asm` line 152). `GameManager.Awake` belongs to the gameplay scene, so it runs after line 122 here.
