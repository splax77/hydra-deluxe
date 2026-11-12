# mid_alt_path_2154180 (build 6142, RVA 0x2154180)
The other path of the .mid drum reader: a static method of the .mid reader class #8626 that `mid_drum_reader_2155050` calls when the switch `mid_switch_get_21581E0` reads true (one direct caller, at 0x2155A6D).
Lines 40 to 46: it reads the song object from the settings at +0x30 and starts the two track flags from settings bytes +0x10 and +0x11.
Lines 60 to 100: for each note event on the track, it raises the first flag when `0x210D780` returns true (the pitch test of CR-18), raises the second when the pitch is 110 to 112 (0x6E to 0x70), and maps the pitch to a difficulty with `0x210D9C0` (0xFF means none).
Lines 101 to 139: the first time a difficulty is seen, it fetches that difficulty's track (0x20CFD40) and calls 0x20CEC60 on it. It leaves the loop early once every difficulty in the class's static mask (+8, set to 0xF by `mid_reader_cctor_2158260`) is seen and either flag is up.
Lines 158 to 182: it writes the two flags to +0xA1 and +0xA2 of every track the instrument has (from 0x2152B20).
It adds one placeholder note to each difficulty it sees: lines 113 to 124 call the note adder 0x20CEC60 on that difficulty's track with zero in its three register arguments (lines 121 to 123). Past that note, this path only records which difficulties exist and the two flags.
