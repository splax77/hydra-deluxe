# ini_register_caller_235340: the one ini-name registration whose name had not been read (build 6142)

Listing: `ini_register_caller_235340.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis 235340 140`. It is a slice of a method that starts before 0x235340 and was not read from its start.

Plain version: this call registers an ini name with a path, and the name always starts with the text 'highway_', so it is never the empty name.

Line 42 takes the file name without its extension (the path helper) and line 49 loads 'highway_'. Line 50 joins the two with the string join. Line 55 joins the folder with 'config.ini'. Line 64 calls the registration `0x2112BF0` with the joined name in `rcx` and the path in `rdx`. `0x2112BF0` stores that pair in the ini class's name-to-path dictionary (static +8) through `set_Item`.

This is the registration caller `ini_save_2112DB0.md` left unread. With it read, all nine direct callers of `0x2112BF0` name a non-empty string: 'GameColors_' plus a value (four sites), 'Rhythm Guitar' plus a value, 'setlistErrorDialog' plus a value, 'profiles', 'settings' and 'highway_' plus a name.
