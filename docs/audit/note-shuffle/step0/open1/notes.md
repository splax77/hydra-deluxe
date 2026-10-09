# Open 1: are ticks rescaled before the seed is taken? (scout notes, 2026-10-09)

Answer: no. The tick the shuffle seed reads (game note + 0x40) is the file's own integer tick, as a 64-bit integer. Neither the .chart reader nor the .mid reader multiplies or divides it. The resolution is stored in the song object as a double and is used only for HOPO thresholds and tick-to-seconds conversion.

## Chain of custody for one drum note's tick

1. .chart: `0x213AFC0` (chart_line_parse_213AFC0.asm) parses "N tick lane len" as decimal into a 64-bit integer (rsi = rsi*10 + digit). `0x213D2D0` (trackbuilder_dispatch_213D2D0.asm) keeps it in rbp, checks ordering, and for 'N' lines calls `0x213D620` with r8 = that tick. `0x213D620` (trackbuilder_addnote_213D620.asm, 0x213d845) stores it unchanged at song-note + 0x10.
2. .mid: `0x214F6D0` (mtrk_read_214F6D0.asm, 0x214fa4b..0x214fada) reads each delta as a VLQ, adds it to the track's running 64-bit tick (track + 0x20) and stamps every event with it (event + 0x10). `0x2156990` (midi_track_handler_2156990.asm, 0x2156d55 and 0x2156ffc) takes the note-on event's tick into r12 and calls `0x20CEC60` with rdx = r12. `0x20CEC60` (midi_sink_20CEC60.asm) is a copy of the chart's add-note; it calls the note init `0x214D240` with the same rdx. `0x214D240` (songnote_init_214D240.asm, 0x214d27b) stores rdx unchanged at song-note + 0x10.
3. Game note: `0x20D4BE0` (build_gamenote_20D4BE0.asm, 0x20d4fa0 and 0x20d4fc8) loads song-note + 0x10 into the 5th constructor argument. `0x214BF10` (gamenote_ctor_214BF10.asm, 0x214bf57) stores that argument at game-note + 0x40. The seed loop in the shuffle (scout-flag/shuffle_20f0c30.asm line 85) does `imul rcx, qword ptr [rax + 0x40]` on that field.

## Where the resolution goes instead

- .chart `Resolution = N`: `0x2164100` (chart_resolution_2164100.asm, 0x2164580) converts N to a double and calls the song setter `0x212C0B0`.
- .mid: `0x2155C00` (midi_reader_2155C00.asm, 0x2155dd4..0x2155ded) takes the MThd ushort and calls the same setter.
- Setter `0x212C0B0` (song_set_resolution_212C0B0.asm) stores the double at song + 0xB0 and + 0xB8, then 0xB0 = resolution * (settings float at + 0x14). Default in the song constructor is 192.0 for both (song_ctor_212E990.asm, 0x212ed9c).
- Song + 0xB8 feeds the HOPO threshold (song + 0x118) and sustain cutoff (song + 0x114). Nothing in the parse path multiplies a tick by it.

## Not verified

- Which MThd ushort (+0x12) is the division field; inferred from use.
- The settings float at + 0x14 (passed by SongEntry) and what song + 0xB0 is used for.
- The Stream-based reader `ʿˀʻʳʺʻʼʲʷʻʷ` (0x20E36F0 etc.) also creates notes via `0x214D240`; its role (cache?) and its writer were not traced.
