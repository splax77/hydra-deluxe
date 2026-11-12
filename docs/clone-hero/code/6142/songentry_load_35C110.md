# songentry_load_35C110 (build 6142, RVA 0x35C110)
A method of `SongEntry` (class #176) that loads its chart and returns the song object; it takes three bools, all defaulting to False. Line 2 spills the first bool (dl) to the stack slot that line 163 reads back as `[rbp + 0x67]`, and lines 163 to 174 pass it as the switch argument of `chart_settings_ctor_20E73E0`. The other settings come from the entry's fields (+0x10 to +0x20, +0x38, +0x39).
The listing stops at 0x1000 bytes; the method is longer.
