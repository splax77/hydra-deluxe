# online_settings_26410: the builder of the [online] settings (build 6142)

Listing: `online_settings_26410.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis 26410 500`. The listing stops at 0x500 bytes, after the last setting it names here.

Plain version: `maxsongspeed` and `lowsongspeed` are two settings of their own, in the online section, kept on a different class from `song_speed`.

Lines 37 and 46 load the section name 'online'. Lines 93 to 103 build `maxsongspeed` with the int setting constructor `0x210C470`: default 250 (line 96), minimum 5 (line 101), maximum 5000 (line 99), step argument 5 (line 97). Lines 104 to 107 store it at static +0x10 of the class whose TypeInfo slot is 0x3C9DF58. Lines 114 to 124 build `lowsongspeed` the same way with default 50 (line 117), and lines 125 to 127 store it at static +0x18 of that class.

`song_speed` is static +0x10 of another class, TypeInfo slot 0x3C98320 (`settings_load_374D50.asm` lines 2028 to 2030). Who reads the two online settings was not listed here.
