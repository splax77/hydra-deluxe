# track_drum_flags_get_215E000 (build 6142, RVA 0x215E000)
A bool getter of the track class #8661 (the class whose bools sit at +0xA1 and +0xA2). It returns true when the byte at +0xA1 is set, and otherwise returns the byte at +0xA2 (lines 1 to 5). So it asks "is either drum flag up".
Direct callers only (byte scan of call instructions): three methods of the song cache, class #1136, at 0x27E730, 0x2847C0 (songcache_drum_flags_2847C0.md) and 0x2880E0.
