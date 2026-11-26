# ini_class_cctor_21159A0: the ini class's static constructor (build 6142)

Listing: `ini_class_cctor_21159A0.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis 21159A0 600`.

Plain version: it builds the ini class's static tables, and one static key whose two strings are the empty string and "tmp". A setting built under that key belongs to no named ini file.

Lines 21 to 24 load the two strings, '' and 'tmp'. Lines 106 to 112 build a two-string key from them, the empty string first (in `rdx`) and 'tmp' second (in `r8`). Lines 113 to 117 store that key at the class's static +0x30. Line 126 clears the static byte at +0x40.

The settings loader builds `song_speed` under this +0x30 key (`settings_load_374D50.asm` line 2015 reads the key, line 2026 hands it to the setting's constructor). The saver skips every key whose first string is empty (`ini_save_2112DB0.asm` lines 100 to 105), and the file loader takes only the keys filed under the ini name it was given (`ini_file_load_2114A80.md`).
