# cli_args_parse_395C10: the command-line option switch (build 6142)

Listing: `cli_args_parse_395C10.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. It runs from the first instruction (RVA `0x395C10`) to the last byte before the next method in `script.json` (0x396070). The function is the `CliGameLauncher` method declared `private bool ...(string[] ...)` at `dump.cs` line 18002.

Plain version: this is the function that reads the game's command-line words. It hashes each word and compares it with a fixed list of option names. The list is twelve names and nothing else: `--versus`, `--instrumentNames`, `--song`, `-i`, `--player`, `--profile`, `-r`, `-s`, `-v`, `--analysis`, `-p` and `--replay` (the string loads on lines 12 to 35, the compares on lines 61 to 245). No option names a modifier, a song speed or a precision setting.

What this proves and what it does not. It proves the option names this switch accepts. The class has two more methods that take the argument array (`0x398440` and `0x3964F0`, `dump.cs` lines 17977 and 17983); they were not read, so this listing alone does not prove that no other word is read anywhere. The value after `--player` goes to the method at `0x396110` (line 231), which returns an instrument and difficulty pair, as `cli_loader_3968D0.md` describes.
