# ini_file_load_2114A80: the ini class's load of one named file (build 6142)

Listing: `ini_file_load_2114A80.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis 2114A80 500`. The listing stops at 0x500 bytes, inside the function's clean-up code; the part that picks which keys to load is whole.

Plain version: given an ini name, it loads only the settings filed under that name.

Line 9 keeps the ini name it was given in `r12`. Lines 61 to 73 look that name up in the class's name-to-path table (static +8). Lines 75 to 104: when the path is not '%internal%' and no file exists there, it calls the saver `0x2112DB0` and returns. Otherwise lines 119 to 123 read the file.

Lines 145 to 163 are the pick. The function walks a table of ini name to list of keys. For each entry, line 161 compares the entry's name with the name in `r12`, and line 163 goes to the next entry when they differ. Only the matching entry's keys go on to lines 166 to 241, which set each setting from the file.

Who calls it was not read. It does not need to be: a call with the empty name looks up a path in the name-to-path dictionary (lines 61 to 73), and every registration of a path (`0x2112BF0`, nine direct callers) uses a non-empty name (`ini_save_2112DB0.md`, `ini_register_caller_235340.md`). So no path is ever found for the empty name. If the lookup throws on a missing name, nothing is loaded; if it gives null, the call takes the branch at lines 75 to 104, where `File.Exists` of no path is false, calls the saver (which skips an empty-named key) and returns. Either way a key filed under the empty name is not loaded from any file.
