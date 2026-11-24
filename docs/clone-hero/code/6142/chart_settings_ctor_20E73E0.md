# chart_settings_ctor_20E73E0 (build 6142, RVA 0x20E73E0)
The constructor of class #8267, the settings a chart read is given (the `in` argument of every .mid reader method). Lines 20 to 34 store the nine arguments in order: two bools at +0x10 and +0x11 (the two track flags' starting values), a float at +0x14, an int at +0x18, a bool at +0x1C, three ints at +0x20 to +0x28, and the bool at +0x2C that the .mid reader copies into its thread-local switch. Lines 35 to 52 make a new empty song object and store it at +0x30.
Direct callers (byte scan of call instructions): `songentry_load_35C110` at 0x35C42E, and four methods of class #8251 (0x20E36F0, 0x20E4E90, 0x20E5130, 0x20E6160).

The float at +0x14 is the song speed ratio: `songentry_load_35C110` passes it in xmm3 (its lines 152 to 164), this constructor moves xmm3 to xmm6 (line 9) and stores it at +0x14 (line 32). The song's resolution setter multiplies the resolution by it (`song_resolution_set_212C0B0.md`). Checked 2026-11-23.
