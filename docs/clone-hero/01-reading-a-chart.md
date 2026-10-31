# Reading a chart [CR]

This page covers how Clone Hero finds and reads a chart: the notes file, the song id, tracks, song.ini keys, sections, solos and the song end.
It describes build 1.1 (v1.1.0.6142).
Last write-back: 2026-10-30.

## Finding the notes file

### CR-01 When a folder or .sng holds both notes files, notes.mid wins

Rule: Hydra takes the first `notes.mid` it finds, else the last `notes.chart`; the game's own choice has never been read.
Label: Hydra assumes it, never checked
Evidence: Hydra's rule is `pick_notes_file` in src/parse/chart_files.cpp line 28, the same order public Hydra used (3b49012c, hydra/hysong.py lines 174 to 189). The derivation ledger (docs/audit/2026-10-03-derivation-ledger.md lines 4064 and 4713) says the old tools/ch_probe player preferred `.chart`, the opposite; that was not re-checked.
Game code (6142): not found: no session read the game's folder scan or its .sng reader.
Hydra: `pick_notes_file` in src/parse/chart_files.cpp line 28; `notes_file_format` line 21.
Ruling: none found.
Found: this plan's wave 1 scout H1.

### CR-02 The notes files and song.ini are found in any letter case

Rule: Hydra finds `notes.mid`, `notes.chart` and `song.ini` whatever their letter case; whether the game does the same has not been read.
Label: Hydra assumes it, never checked
Evidence: `notes_file_format` and `is_song_ini` in src/parse/chart_files.cpp lines 21 and 38. Public Hydra compared the names exactly (3b49012c, hydra/hyutil.py lines 153 to 157), so this is a Hydra Deluxe change.
Game code (6142): not found: the game's file-name test was never read.
Hydra: `notes_file_format` and `is_song_ini` in src/parse/chart_files.cpp.
Ruling: none found.
Found: this plan's wave 1 scout H1.

### CR-03 A .srb is a 16-byte header, four DEFLATE streams, then encrypted audio

Rule: a .srb (the container of the songs that ship with the game) is a 16-byte header, then four raw-DEFLATE streams back to back (song info, the chart, two JPEG images), then an encrypted audio section to the end of the file; the song-info stream holds the notes file name, eight text fields, 12 difficulty bytes, the preview start, the song length in ms and a 16-byte checksum. The byte layout lives in docs/srb-format.md, which this row does not copy.
Label: Unknown
Evidence: decoded from the bytes of all 30 bundles in the game's install folder on 2026-10-30 (every stream inflates in order, streams 3 and 4 start FF D8, Biology's length field reads 196,905 ms); no result file is saved. The game's own .srb opener was read in session 7c71d4dd on 2026-09-29, but no address or listing was recorded, so the game's code for it is lost.
Game code (6142): not found: listing lost; the game's .srb opener was read 2026-09-29 in session 7c71d4dd with no address recorded.
Hydra: `srb_inflate_stream` in src/parse/srb.cpp line 100, `srb_parse_metadata` line 136, `srb_read_metadata` line 188, `kSrbHeaderSize` in src/parse/srb.h line 43.
Ruling: none needed: same rule as far as the bytes show.
Found: 2026-09-29, session 7c71d4dd; re-derived by this plan's wave 1 scout H1.

### CR-04 The game has a header scrambler and its exact inverse

Rule: the game holds a decoder that maps two 64-bit numbers a and b to mix(((a xor b) times 0x308DBC3) and 0x1FFFFFFFFF) minus 1, and an encoder that undoes it for every n below 2 to the 30th; "mix" xors back a 7-bit-shifted copy under the mask 0x550055.
Label: Proven in the game's code
Evidence: `code/6142/header_decode_214EFD0.asm` lines 1 to 13 and `code/6142/header_encode_214F0A0.asm` lines 1 to 14, no calls and no globals in either; the scout re-ran both in Python over 200,000 random pairs with 0 mismatches. The decoder is also in the 2026-09-17 Ghidra decompile, lines 37889 to 37899.
Game code (6142): two static methods of class #8599 (obfuscated names), RVAs `0x214EFD0` and `0x214F0A0`, listings above.
Hydra: no owner: Hydra never decodes the header, it skips it (`kSrbHeaderSize` in src/parse/srb.h line 43).
Ruling: none needed: Hydra reads no field the header holds.
Found: 2026-09-29, session 7c71d4dd; listed by wave 2 scout R9.

### CR-05 What the 16 header bytes of a .srb hold

Rule: session 7c71d4dd read bytes 0 to 7 as a per-file key and bytes 8 to 15 as the song-info size scrambled with it (by CR-04's functions); nobody has seen which bytes the game passes in.
Label: Unknown
Evidence: unchecked by the scout: it would need a caller of `0x214EFD0` or `0x214F0A0` read to show which bytes feed them. The card's "all 30 stored header values match" has no script or result file. docs/srb-format.md lines 45 to 49 still call the header unknown.
Game code (6142): not found: no caller of the two functions was read.
Hydra: `kSrbHeaderSize` in src/parse/srb.h line 43 skips the 16 bytes.
Ruling: none needed.
Found: 2026-09-29, session 7c71d4dd; this plan's wave 2 scout R9.

### CR-06 The .srb audio key is built at run time and equals Hydra's

Rule: the game builds the 16-byte AES key for bundled audio at run time with a loop that runs 41 times over two 64-bit constants; the two results, little-endian, are exactly Hydra's key.
Label: Proven in the game's code
Evidence: `code/6142/aes_key_build_215AFE0.asm` constants at lines 6 and 7, the loop at lines 10 to 31, the two key writes at lines 44 and 51; the scout ran the loop in Python and got the bytes of `kSrbAesKey`. Also decompile lines 46814 to 46850.
Game code (6142): a static method of class #8616, RVA `0x215AFE0`, `code/6142/aes_key_build_215AFE0.asm` lines 6 to 51.
Hydra: `kSrbAesKey` in src/app/preview_source.cpp line 53.
Ruling: none needed: same rule. The user asked for it: "implement the decryptor into hydra's preview so srb songs can play audio normally" (card 2026-09-15-450488b5, 03:24 UTC).
Found: 2026-09-29, session 7c71d4dd; listed by wave 2 scout R9.

### CR-07 The .srb audio cipher mode and IV

Rule: each audio blob is AES-128-CFB with that key, and its IV is the blob's 16-byte header with its two halves swapped.
Label: Unknown
Evidence: unchecked by the scout: it would need the game's decryptor listed. Session 450488b5 reached the decryptor by hooking BASS and working back from known plaintext; no address, listing or result file is in the repo.
Game code (6142): not found: listing lost; read 2026-09-15 in session 450488b5, address not recorded.
Hydra: the decryptor beside `kSrbAesKey` in src/app/preview_source.cpp.
Ruling: none needed.
Found: 2026-09-15, session 450488b5.

### CR-08 A .srb whose notes entry is neither .mid nor .chart

Rule: Hydra reads such an entry by sniffing the chart bytes for `MThd` rather than refusing it; the game's handling is unread, and none of the 30 shipped bundles has such a name.
Label: Hydra assumes it, never checked
Evidence: `load_songpath_srb` in src/parse/song.cpp line 2038 holds the sniff; docs/audit/2026-10-29-audit-of-audits/questions.md lines 63 to 75 found no real container with an odd name. D27 says .sng and .srb find their notes file "the same way", and the audit's Q5 shows they differ.
Game code (6142): not found: not read.
Hydra: `load_songpath_srb` in src/parse/song.cpp line 2038.
Ruling: none found.
Found: this plan's wave 1 scout H1.

### CR-09 A .sng stores each file XORed with a 16-byte mask

Rule: a .sng is a 10-byte prefix, a 16-byte XOR mask, a metadata block and a file table; byte i of each file is stored XORed with mask[i % 16] ^ (i & 0xff).
Label: Hydra assumes it, never checked
Evidence: Hydra's reader, and public Hydra's (3b49012c, hydra/hysong.py lines 163 to 196), read it this way. The game's .sng reader class is #1068; its static constructor uses the literal "SNGPKG" (stringliteral.json), and none of its methods was listed or read.
Game code (6142): not found: the .sng class's static constructor `0x266340` was located, not read.
Hydra: `kSngXorMaskOffset` in src/parse/sng.h, `sng_read_notes` in src/parse/sng.cpp line 148.
Ruling: none found.
Found: this plan's wave 1 scout H1 and wave 2 scout R9.

## The song id

### CR-10 A song's id is the MD5 of its notes file, or the stored checksum for a .srb

Rule: Clone Hero's song id (the key of the player's saved scores) is the MD5 of the notes file for a folder chart, the MD5 of the unmasked notes file for a .sng, and the 16-byte checksum stored in the .srb's song info for a .srb; a revised chart gets a new id.
Label: Proven in a live run
Evidence: on 2026-10-30 the game's own `songcache.bin` (8,221,087 bytes, written 2026-10-28 20:35 by the game) held the stored checksum of 30 of 30 bundles and the whole-file MD5 of none of them. The .sng half (54 of 54) and the folder half rest on ADR 0028 lines 12 to 45 and were not re-run (CR-12).
Game code (6142): not found: the checksum route through `SongEntry` is page 7's; no listing.
Hydra: `song_id_read` in src/app/analysis.cpp line 285; `hash_chart_file` in src/app/analysis.h line 83.
Ruling: D170: "yes, go ahead, let the 84 results drop" (session bc0144a4, 03:59 UTC).
Found: 2026-10-10, session bc0144a4; re-checked by this plan's wave 1 scouts H1 and HB.

### CR-11 A .srb's stored checksum is the chart stream's MD5 in 26 of 30 bundles

Rule: for 26 of the 30 shipped bundles the stored checksum equals the MD5 of the chart stream; Good Grief Retreat, No Known Suspects, Stigma and Troopers of the Stars differ, for a reason nobody has found.
Label: Proven in a live run
Evidence: re-run on 2026-10-30 over the 30 real files against the game's `songcache.bin`; the four names match ADR 0028 lines 41 to 45. Card 7c71d4dd ruled out line endings, byte-order marks and dropped MIDI tracks as the cause.
Game code (6142): not found: data, not code.
Hydra: `song_id_read` in src/app/analysis.cpp line 285 reads the stored checksum and never recomputes it.
Ruling: D170 (as CR-10).
Found: 2026-09-29, session 7c71d4dd; re-run by this plan's wave 1 scout H1.

### CR-12 The .sng and folder halves of the id rule

Rule: ADR 0028 says the .sng id matched for 54 of 54 songs and the folder id for a 300-chart sample.
Label: Unknown
Evidence: unchecked by the scout: it would need the two checks re-run; no tool or result file for either is in the repo (a grep of tools/ for `songcache` finds nothing).
Game code (6142): not found: data, not code.
Hydra: `song_id_read` in src/app/analysis.cpp line 285.
Ruling: D170 (as CR-10).
Found: 2026-10-10, session bc0144a4 (ADR 0028).

## Track names and track 0

### CR-13 A .mid drum track is named "part drums" or "part drum", in any case

Rule: the game lowercases each track name and compares it exactly with "part drums" and "part drum"; nothing is trimmed.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm line 218 (the lowercase call at 0x2155FDF), lines 297 to 302 (the "part drums" compare at 0x2156134) and lines 503 to 508 (the "part drum" compare at 0x21564A8).
Game code (6142): a method of the .mid reader class #8626, RVA `0x2155C00`, the listing above.
Hydra: `is_drums_track_name` in src/parse/midi.cpp line 446, names in `kDrumsTrackNames` in src/parse/midi.h line 97.
Ruling: D252: chose "Yes, build it" (card 2026-10-17-77c0ebab, 00:31 UTC).
Found: 2026-10-16, session 77c0ebab.

### CR-14 When two tracks carry a drum name, the first in file order wins

Rule: the game adds each instrument it reads to a set, and a later track of an instrument already in the set is skipped.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm lines 509 to 516: the set's Add call at 0x21564D7, and a false return jumps to the next track at 0x2156665.
Game code (6142): RVA `0x2155C00`, lines 509 to 516.
Hydra: `MidiFile::drums_track` in src/parse/midi.cpp line 489.
Ruling: D252 (as CR-13; the record states the rule, "the first drum-named track in file order wins, as in the game").
Found: 2026-10-16, session 77c0ebab.

### CR-15 Track 0 is never read as an instrument

Rule: the instrument loop starts at track index 1, so a drum track in track 0 (a format-0 single-track file) is not drums in the game.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm lines 197 to 201 (the loop counter set to 1 at 0x2155F8D).
Game code (6142): RVA `0x2155C00`, line 198.
Hydra: `MidiFile::kFirstInstrumentTrack` in src/parse/midi.h line 200, used by `MidiFile::drums_track`.
Ruling: D253 item 1: "Match the game (Recommended)".
Found: 2026-10-16, session cfb159b5.

### CR-16 A track named "notes" then "PART DRUMS" at tick 0 is the drum track

Rule: the game plays Hormone's Echo with drums, whose drum track carries two names at tick 0, "notes" then "PART DRUMS"; Hydra's rule is that a track's name is its first recognised name.
Label: Hydra assumes it, never checked
Evidence: the user reported the game reads it; no agent checked. The .mid reader listing only consumes each track's finished name (midi_reader_2155C00.asm line 170); the code that builds that name from several name events was not read.
Game code (6142): not found: the name-building step was not read (mtrk_read_214F6D0.asm exists and was not traced for it).
Hydra: `TrackNamePick` in src/parse/midi.cpp line 273; `recognized_track_name` line 25.
Ruling: D78: "clone hero is able to read hormone's echo correctly, hydra should also support it." (card 2026-10-06-d3bdb873, 00:16 UTC). It sets the target, not which name the game picks.
Found: 2026-10-05, session d3bdb873.

### CR-17 The .mid reader has a second path behind a thread-local switch

Rule: when a static thread-local flag on the .mid reader class reads true, the drum reader calls another function instead of reading the drum track; what sets the flag and what that function does are not known.
Label: Unknown
Evidence: `code/6142/mid_drum_reader_2155050.asm` lines 77 to 79 (a call to `0x21581E0`, which returns the flag, and a jump to lines 562 to 570 when true). Searched only these functions.
Game code (6142): RVAs `0x2155050`, `0x21581E0` and `0x2154180`; the switch's setter and `0x2154180` were not read.
Hydra: no owner: not searched.
Ruling: none found.
Found: 2026-10-30, wave 2 scout R1.

### CR-18 Where the two track flags at +0xA1 and +0xA2 come from

Rule: the .mid reader starts the two flags from two bytes of its second argument, raises the first for a note on pitch 65, 77, 89 or 101 and the second for a tom marker, and writes both to every track it fills; the .chart reader raises them only from N 5 and from N 66 to 68. What the flags do downstream (the names five-lane and pro-drums are guesses from what sets them) is unread.
Label: Unknown
Evidence: `code/6142/mid_drum_reader_2155050.asm` lines 87 to 99 (starting bytes), 136 to 147 (the call to `0x210D780`, which tests those four pitches), 270 (the tom marker) and 552 to 557 (the stores).
Game code (6142): RVAs `0x2155050` and `0x210D780`.
Hydra: no owner: Hydra takes Pro Drums from the player's setting (`mode_pro_` in src/parse/song.cpp); the flags were not searched for in Hydra.
Ruling: none found.
Found: 2026-10-30, wave 2 scout R1.

## The events track

### CR-19 The events track is the first track named "events", in any case

Rule: the game searches from track 0 for a name equal to "events" ignoring case, reads the first match and stops, so only one events track counts.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm lines 154 to 181: the loop starts at 0, the compare with comparison type 5 (ignore case) is at 0x2155F36, and a match leaves the loop at 0x2155F3D.
Game code (6142): RVA `0x2155C00`, lines 154 to 181.
Hydra: `MidiFile::events_track` in src/parse/midi.cpp line 495; `is_events_track_name` line 451.
Ruling: D253 item 2: "Match the game (Recommended)".
Found: 2026-10-16, session cfb159b5.

### CR-20 A .chart E line is trimmed, then tested for [mix, mix, solo and soloend in that order

Rule: the .chart E handler reads the rest of the line up to LF or CRLF, trims both ends, then tests in order: starts with `[mix`, starts with `mix` (both case-sensitive), equals `solo`, equals `soloend`.
Label: Proven in the game's code
Evidence: `code/6142/chart_e_handler_213CAF0.asm` lines 65 to 112 (end of line and the trim), 116 to 168 (the two starts-with tests), 169 to 230 (the solo compares); decompile lines 24441 to 24700 give the same four literals in the same order.
Game code (6142): a method of the .chart line class #8615, RVA `0x213CAF0`, the listing above.
Hydra: `classify_chart_line` in src/parse/song.cpp lines 1237 to 1259, with `trim_view`.
Ruling: none needed: same rule.
Found: 2026-10-03, session cebf3006; listed by wave 2 scout R1.

### CR-21 A .chart line is dispatched by its first letter, E, N or S

Rule: the .chart line dispatcher sends a line to the E, N or S handler by its first letter, upper case only; any other line is skipped to its end.
Label: Proven in the game's code
Evidence: decompile lines 24907 to 24950 (FUN_18213d2d0); a listing of the same function is committed as docs/audit/note-shuffle/step0/open1/trackbuilder_dispatch_213D2D0.asm, not re-read for this row.
Game code (6142): RVA `0x213D2D0`, decompile lines 24844 to 24968.
Hydra: `classify_chart_line` in src/parse/song.cpp lines 1239 to 1278 compares the first whitespace-split word.
Ruling: none needed: same rule for normal files.
Found: 2026-10-03, session cebf3006; re-read by wave 2 scout R1.

### CR-22 The .chart reader has a flag-only pass

Rule: when the line state's scanning byte is set, the E handler returns before recording anything and the N handler only raises the two track flags (CR-18) and stores no note.
Label: Proven in the game's code
Evidence: `code/6142/chart_e_handler_213CAF0.asm` lines 113 to 115; `code/6142/chart_n_handler_213D620.asm` lines 27 to 59. The flag-only branch runs only when the track's note list already holds more than one item and a test at `0x215FE80` returns true. What sets the scanning byte was not read.
Game code (6142): RVAs `0x213CAF0` and `0x213D620`, the listings above.
Hydra: no owner: Hydra has no flag-only pass (the `CPhase` passes in src/parse/song.cpp lines 1329 to 1340).
Ruling: none found.
Found: 2026-10-30, wave 2 scout R1.

### CR-23 A .chart with notes out of tick order is read in tick order

Rule: Hydra settles chord order by tick once, before the phrase and fill rules; how the game orders such a file is unread.
Label: Hydra assumes it, never checked
Evidence: D47 says "Clone Hero's own ordering is not in the evidence file." None of the library's 4,338 loose .chart files is out of order (D47).
Game code (6142): not found: looked in ch-evidence.md, ADR 0023 and the open1 listings by name.
Hydra: the .chart reader in src/parse/song.cpp line 366 emits chords in tick order.
Ruling: D47 (record text, no quoted words): "A .chart whose drum notes are written out of tick order is read in tick order".
Found: 2026-10-04, D47.

### CR-24 The .mid events reader also tests [coda], coda and _ENDOFSONG

Rule: the game's .mid events handler tests the texts `coda`, `[coda]` and any text containing _ENDOFSONG; what each branch stores is unread, and Hydra reads none of them.
Label: Unknown
Evidence: the three literals sit in the events-track function (decompile lines 41771 to 42120); docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm lines 130 to 135 show a text holding _ENDOFSONG is sent to the `[prc` test instead of the section test. The effect of the coda branches was not traced.
Game code (6142): RVA `0x21544E0`, the listing above.
Hydra: no owner: a grep of src/ for `coda` and `ENDOFSONG` finds nothing.
Ruling: none found.
Found: 2026-10-30, wave 1 scout H1 and wave 2 scout R9.

### CR-25 Ticks are the file's own ticks; no reader rescales them

Rule: both readers store each note's own 64-bit file tick unchanged, whatever the file's resolution.
Label: Proven in the game's code
Evidence: the chain of stores read by wave 1 scout H2a: docs/audit/note-shuffle/step0/open1/trackbuilder_addnote_213D620.asm line 141, mtrk_read_214F6D0.asm lines 175 and 208, songnote_init_214D240.asm line 18, build_gamenote_20D4BE0.asm lines 253 and 261, gamenote_ctor_214BF10.asm line 19, ending at docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm lines 84 and 85, which multiplies by that tick. docs/audit/note-shuffle/step0/FINDINGS.md lines 11 to 15 say the same.
Game code (6142): RVAs `0x213D620`, `0x214F6D0`, `0x214D240`, `0x20D4BE0`, `0x214BF10`, `0x20F0C30`, the listings above.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp line 127 reads the chord's own ticks.
Ruling: none needed: same rule.
Found: 2026-10-09, session 30beeb6e.

## song.ini keys the game reads

### CR-26 The game reads ten chart keys from song.ini's [song] section

Rule: one function reads `delay` (whole number, default 0), hopo_frequency (limited to 0 to 65535), five_lane_drums, `pro_drums`, eighthnote_hopo, multiplier_note, video_start_time, sustain_cutoff_threshold, chord_snap_threshold and loading_phrase into a settings object. Hydra reads only `song_length` and `delay`. How the game lets `delay` and the chart Offset combine is page 3's.
Label: Proven in the game's code
Evidence: `code/6142/ini_song_reader_1EFC60.asm` lines 15 to 33 (the key literals), 38 to 47 (`delay`), 49 onward (hopo_frequency), 77 (five_lane_drums), 84 (`pro_drums`) and 91 (eighthnote_hopo).
Game code (6142): a method of class #828, RVA `0x1EFC60`, the listing above.
Hydra: `ini_delay_ms` in src/app/preview_source.cpp line 301 and the `song_length` read in src/app/analysis.cpp line 153; no owner for the other keys (a grep of src/ for each key name finds none).
Ruling: none found.
Found: 2026-10-30, wave 2 scouts R9 and R7.

### CR-27 The game writes missing keys back into song.ini

Rule: for each metadata key a song.ini lacks, `0x358850` appends a line such as song_length = {0} or `delay = {0}` (and name, artist, album, genre, year, charter, the difficulties, `pro_drums` and the rest) and logs "Creating metadata file for"; it writes, it does not read.
Label: Proven in the game's code
Evidence: `code/6142/song_ini_loader_358850.asm` line 88 (song_length = {0}), lines 316 to 345 (`delay`) and line 674 (`song.ini`).
Game code (6142): a method of `SongEntry` (TypeDefIndex 176), RVA `0x358850`, the listing above.
Hydra: no owner: Hydra never writes song.ini.
Ruling: none needed: Hydra reads the file only.
Found: 2026-10-18, session b71e9156 (the address); listed by wave 2 scout R9.

## Sections and their display names

### CR-28 A .mid section is [section X] or [prc_X], shown by the game's name table

Rule: the events reader takes a text that starts with `[section` or `[prc`, cuts the name, looks it up in an exact-case name table, and falls back to the name with each underscore turned into a space.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm lines 33 to 41 (the literals), 100 to 125 (the starts-with test), 146 to 158 (the name cut from character 9), 169 to 186 (the table lookup) and 175 to 182 (the underscore fallback), with the `[prc` branch from line 190. Decompile lines 41771 to 42125 agree.
Game code (6142): a method of the .mid reader class #8626, RVA `0x21544E0`, the listing above.
Hydra: `section_display_name` in src/parse/section_names.cpp line 25; `marker_name` line 13; `is_section_marker` line 21.
Ruling: D224: chose "As Clone Hero shows them (Recommended)" (session 28fad19c, 02:27 UTC).
Found: 2026-10-12, session 28fad19c; re-read by wave 2 scout R9.

### CR-29 "[sectionX]" with no space is a section to the game, not to Hydra

Rule: the game's test is starts-with `[section`, then the name is cut from character 9, so a text like `[sectionX]` would be a section with a short or empty name; Hydra needs `section ` with the space.
Label: Unknown
Evidence: the starts-with test is midi_events_track_21544E0.asm lines 100 to 125 and the cut lines 146 to 158; unchecked by the scout: it would need a .mid with such an event loaded in the game.
Game code (6142): RVA `0x21544E0`, lines 100 to 158.
Hydra: `marker_name` in src/parse/section_names.cpp line 13.
Ruling: open split: see questions page Q-15.
Found: 2026-10-30, wave 2 scout R9.

### CR-30 The game's section-name table holds 2,718 entries, and Hydra ships a copy

Rule: the game maps practice-section keys to shown names through a table of 2,718 entries; Hydra ships a copy as data/clone-hero-1.1.0.6142-section-names.tsv.
Label: Unknown
Evidence: the TSV has 2,718 rows. It was copied from hydra-data/ch-fill-evidence-2026-10-11/section_table.tsv, built by a script from a listing of `0x2095040`; that listing sits only in hydra-data, outside the repo, and was not re-extracted.
Game code (6142): RVA `0x2095040`; listing uncommitted (hydra-data/ch-fill-evidence-2026-10-11/f2095040.asm).
Hydra: `section_name_table` in src/parse/section_names.h line 35; generator tools/gen_section_names.py.
Ruling: D228: chose "Ship the list" (session 28fad19c, 02:44 UTC).
Found: 2026-10-12, session 28fad19c.

### CR-31 A section key matches exactly, case included

Rule: Hydra looks a key up with its exact case, so `Intro` typed by hand misses where `intro` hits; the game's key compare was never read.
Label: Hydra assumes it, never checked
Evidence: the comment in src/parse/section_names.cpp lines 32 to 34; the game's lookup comparer (FUN_1813ff9d0, decompile lines 42110 to 42125) was not read.
Game code (6142): not found: the lookup's comparer was not read.
Hydra: `section_display_name` in src/parse/section_names.cpp lines 32 to 39.
Ruling: none found.
Found: this plan's wave 1 scout H1.

### CR-32 A .chart section name drops one pair of quotes

Rule: in a .chart, a practice-section name loses one pair of quotes, as the format writes it.
Label: Hydra assumes it, never checked
Evidence: ADR 0023 lines 65 and 66; the game's .chart events reader `0x2163170` was not traced for this.
Game code (6142): not found: not traced.
Hydra: `section_name_of` in src/parse/song.cpp near line 30, which calls `section_display_name`.
Ruling: none found.
Found: this plan's wave 1 scout H1.

## Solos

### CR-33 A .chart solo and soloend compare exactly, case included

Rule: `solo` and `soloend` count only when the whole trimmed E text equals them exactly, so `E "solo"` (quoted), `E Solo` and `E solo extra` do nothing.
Label: Proven in the game's code
Evidence: `code/6142/chart_e_handler_213CAF0.asm` lines 169 to 230 call the compare `0x20FE510` with ignore-case 0; decompile lines 58780 to 58830 show that compare checks the lengths and then every 16-bit character, and the handler has no quote-stripping step.
Game code (6142): RVA `0x213CAF0` lines 169 to 230; compare RVA `0x20FE510`, decompile lines 58780 to 58845.
Hydra: `classify_chart_line` in src/parse/song.cpp lines 1252 to 1255.
Ruling: none needed: same rule.
Found: 2026-10-03, session cebf3006; listed by wave 2 scout R1.

### CR-34 .chart solos are a stack in the game and one flag in Hydra

Rule: in the game `solo` pushes a new solo record onto the track's solo list and `soloend` pops the most recent one, writing its length as end tick minus start tick plus 1; a `soloend` with nothing open is ignored. Hydra keeps one on-off flag.
Label: Proven in the game's code
Evidence: `code/6142/chart_e_handler_213CAF0.asm` lines 231 to 250 (the pop, and the length at lines 247 to 249) and lines 251 to 306 (the push and the list Add); decompile lines 24641 to 24680.
Game code (6142): RVA `0x213CAF0`, lines 231 to 306.
Hydra: `flag_solo_` in src/parse/song.cpp lines 1417, 1643 and 1647.
Ruling: open split: see questions page Q-11.
Found: 2026-10-30, wave 2 scout R1.

### CR-35 How the score code reads a solo record

Rule: whether a note on the `soloend` tick is inside the solo, and whether the plus-1 length is the whole reason, is not known from the .chart side.
Label: Unknown
Evidence: unchecked by the scout: it would need the solo list's readers in the score code read. CR-37 reads the note-list side of the same list.
Game code (6142): not found: looked only in the eight functions of wave 2 scout R1's slice.
Hydra: `E soloend` is applied after the tick's notes (src/parse/song.cpp line 1647).
Ruling: none found.
Found: 2026-10-30, wave 2 scout R1.

### CR-36 MIDI note 103 on the drum track is the solo marker

Rule: the .mid drum reader sends note 103 to a sink that appends a (start tick, length) entry to the track's solo list, and the note-list builder later turns that list into solo flags.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2/f2155050.asm lines 225 to 233 (the compare with 0x67, then a call to 0x20CF1B0 (a function the dump does not list as a method)); docs/audit/note-shuffle/step0/open1/midi_sink_20CF1B0.asm lines 18 to 66 (the Add to the list at +0x78); `code/6142/solo_flags_20D18F0.asm` lines 45 to 157 read that list. dump.cs lines 551830 and 551859 give the list's type.
Game code (6142): sink RVA 0x20CF1B0 (a function the dump does not list as a method), builder step RVA `0x20D18F0`, the listings above.
Hydra: `kSoloMarkerPitch` in src/parse/song.cpp line 678; `find_solo_sections` line 63.
Ruling: none needed: same rule.
Found: 2026-10-30, wave 2 scout R9.

### CR-37 A solo covers notes from its start up to, not including, start plus length

Rule: a note is in a solo when its tick is at or after the solo's start and strictly before start plus length; the first covered note gets SoloBegin (0x200) and the last SoloEnd (0x400), and a solo that covers no note sets no flag.
Label: Proven in the game's code
Evidence: `code/6142/solo_flags_20D18F0.asm` lines 72 to 74 (the start compare), 82 to 85 (the end compare leaves at start plus length), 91 and 92 (no covered note, no flag), 120 (SoloBegin) and 157 (SoloEnd); flag values from dump.cs lines 548203 and 548204. Lines 34 to 39 limit it to instrument 6 or 9; what the test at `0x214BCC0` checks was not read.
Game code (6142): a static method of class #8265, RVA `0x20D18F0`, called from the builder `0x20D2EA0`.
Hydra: `find_solo_sections` in src/parse/song.cpp line 63, with the comment at lines 953 to 956 that the note-off tick is outside the solo.
Ruling: none needed: same rule.
Found: 2026-10-30, wave 2 scout R9.

## The end event and song length

### CR-38 A .mid end event is [end] on the events track, and the first one wins

Rule: the .mid events reader reads the song's end event from the text `[end]` on the events track; the setter stores a tick only while the chart's end field is still empty, so the first end event wins.
Label: Proven in the game's code
Evidence: the literal `[end]` in the events-track function (decompile lines 41771 to 42120, the call to the setter at line 42051); the setter's own decompile, lines 15845 to 15867, stores only when the field at +0x108 is empty.
Game code (6142): events reader RVA `0x21544E0` (docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm); setter RVA `0x212E310`, decompile lines 15845 to 15867.
Hydra: `is_end_event` in src/parse/song.cpp line 46 (`kEndEventBody` line 41); the .mid pass at lines 1126 to 1135 keeps the first.
Ruling: none needed: same rule.
Found: 2026-10-18, session c433b17f; re-read by wave 1 scout H1 and wave 2 scout R3.

### CR-39 Whether the .mid [end] test is exact, a prefix test, or ignores case

Rule: the game compares the text with `[end]` through a string function that the .chart handler also uses for its `[mix` starts-with test, so `[end] x` or `[END]` might count in the game; Hydra counts only the exact text.
Label: Unknown
Evidence: the compare is FUN_182c61990 in the decompile (lines 41940 to 41990, and 24549 for the `[mix` use); that function is not in the decompile and has no name in script.json, so its meaning is unread.
Game code (6142): RVA `0x21544E0`; the compare function at 0x2C61990 was not read.
Hydra: `is_end_event` in src/parse/song.cpp line 46; its comment at line 43 calls the game's compare exact and case-sensitive.
Ruling: open split: see questions page Q-75.
Found: 2026-10-30, wave 1 scout H1 and wave 2 scout R3.

### CR-40 A .chart end event strips one pair of brackets

Rule: the .chart [Events] reader strips one pair of brackets, so `end` and `[end]` both count, and the first wins.
Label: Unknown
Evidence: the bracket compares and the call to the end setter sit in a listing of `0x2163170` (hydra-data/ch-fill-evidence-2026-10-11/f2163170.asm lines 297 to 304 and 692) that is not committed; the `end` compare itself and a settings flag the handoff says skips it were not opened.
Game code (6142): RVA `0x2163170`; listing uncommitted (hydra-data/ch-fill-evidence-2026-10-11/f2163170.asm).
Hydra: the .chart events pass in src/parse/song.cpp lines 1580 to 1591 strips one bracket pair, then asks `is_end_event`.
Ruling: none needed: same rule as far as read.
Found: 2026-10-18, session c433b17f.

### CR-41 An end event counts only at or past the last note end plus one beat

Rule: an end event counts only when its tick is at least the last note end plus one beat (the double at chart +0xB0, cut to a whole number); a tick exactly on the margin counts, and with no end event the answer is no.
Label: Proven in the game's code
Evidence: `code/6142/end_event_usable_212C7E0.asm` lines 2 to 6 (no end event gives false) and 7 to 13 (end tick greater than or equal to the length); `code/6142/chart_length_212E130.asm` lines 88 to 91 (the flag adds the double at +0xB0, cut to an integer). Decompile lines 14779 to 14791 agree.
Game code (6142): methods of the chart class #8538, RVAs `0x212C7E0` and `0x212E130`, the listings above.
Hydra: `end_event_counts` in src/app/song_length.cpp line 22.
Ruling: D255 item 4: "Copy the margin (Recommended)".
Found: 2026-10-18, session c433b17f; listed by wave 2 scout R3.

### CR-42 Chart field +0xB0 is the resolution times a factor, not the Offset

Rule: the setter for chart field +0xB0 accepts only a positive finite number and stores it times a float read from another object; the caller passes the .mid division (ticks per beat). The song-end handoff's line 97 ("the chart's offset") is wrong. That the float is the song speed ratio, so the field is one beat at the current speed, has not been verified.
Label: Proven in the game's code
Evidence: decompile lines 14456 to 14483 (FUN_18212c0b0: the guard at line 14465, the stores at lines 14466 to 14470) and its caller at line 42845, read by wave 1 scout H1 and wave 2 scouts R3 and R7. A listing is committed as docs/audit/note-shuffle/step0/open1/song_set_resolution_212C0B0.asm, not re-read for this row.
Game code (6142): RVA `0x212C0B0`, decompile lines 14456 to 14483.
Hydra: none: Hydra keeps no copy of this field; `end_event_counts` uses `Song::tick_resolution` because Hydra works at 100% speed.
Ruling: none needed.
Found: 2026-10-30, wave 1 scout H1 and wave 2 scouts R3 and R7.

### CR-43 The last note end is the largest gem end over every track

Rule: the chart's length is the largest note start plus sustain over every track in the chart's track list, every instrument and difficulty; the per-track helper reads only the track's gem list, never the instrument, and an empty list gives 0.
Label: Proven in the game's code
Evidence: `code/6142/chart_length_212E130.asm` lines 33 to 63 (the loop over the track list and the running maximum); the helper's decompile, lines 50458 to 50507, reads only the list at +0x60.
Game code (6142): RVA `0x212E130`, the listing above; helper RVA `0x215FED0`, decompile lines 50474 to 50507.
Hydra: `Song::last_note_end_tick` in src/core/song.h line 147, set in src/parse/song.cpp lines 1141 (.mid) and 1884 (.chart).
Ruling: D255 item 3: "All instruments".
Found: 2026-10-18, session c433b17f; listed by wave 2 scout R3.

### CR-44 Vocal notes never reach the gem lists, so they do not move the song end

Rule: the .mid vocals method writes its pitches and phrase markers only to the chart-wide lyric list and never calls the gem adder, so a vocal note never counts toward the last note end. Hydra counts a vocals track's low notes.
Label: Proven in the game's code
Evidence: decompile lines 43955 to 44246 (FUN_1821579b0): its calls to 0x2152CC0 (a function the dump does not list as a method) are at lines 44067 to 44231, and no call to the gem adder appears in lines 43900 to 44300; that function writes the lyric list (decompile lines 40632 to 40655). The .chart vocals and the binary note-stream loaders were not checked. No listing was made.
Game code (6142): .mid vocals method RVA `0x21579B0`, decompile lines 43955 to 44246; lyric adder RVA 0x2152CC0 (a function the dump does not list as a method).
Hydra: `MidiFile::last_note_end_tick` in src/parse/midi.cpp lines 464 to 487 counts vocals pitches 36 to 57 (`kVocalsTrackName` in src/parse/midi.h line 122).
Ruling: open split: see questions page Q-74.
Found: 2026-10-18, session b71e9156 (inferred); read in the decompile by wave 2 scout R3.

### CR-45 The song ends when the clock reaches the game's song length

Rule: the game ends the song when its song clock reaches GameManager's songLength field (+0x50); song.ini `song_length` is not read on that path.
Label: Unknown
Evidence: the frame update at `0xC05A0` is not in the decompile and no listing survives; dump.cs line 25298 confirms the field at +0x50.
Game code (6142): GameManager's Update, RVA `0xC05A0`; listing lost; read 2026-10-18 in session b71e9156.
Hydra: `app::song_length_ms` in src/app/song_length.cpp line 28 is the one end.
Ruling: D255 item 5: "the problem is that we have two different rules. there should one rule that every function uses. we already know how clone hero decides the end of the song, we don't need different places deriving different rules."
Found: 2026-10-18, session b71e9156.

### CR-46 The length is Min(end event, Max(audio length, last note end)) plus 1 second

Rule: the load coroutine builds the song length as the earlier of the end event and the later of the audio length and the last note end, plus 1 second; an unusable end event drops the Min.
Label: Unknown
Evidence: wave 2 scout R3 confirmed on 2026-10-30 that the 1 second is a double 1.0 in the DLL's read-only data (it has no `constants.py` name yet); the coroutine itself, `0xD8D80`, and its Min and Max helpers were not opened, and the 2026-10-18 disassembly survives only as about ten lines quoted in docs/handoffs/2026-10-18-song-length-end-event-handoff.md lines 164 to 186.
Game code (6142): load coroutine RVA `0xD8D80`; listing lost; read 2026-10-18 in session b71e9156.
Hydra: `app::song_length_ms` in src/app/song_length.cpp line 28; `kSongEndPadMs` in src/app/song_length.h line 37.
Ruling: D255 item 2: "Copy the pad (Recommended)".
Found: 2026-10-18, session b71e9156.

### CR-47 The game's audio length is the longest stem; Hydra uses the stated length

Rule: the game's audio length is the longest loaded audio stem; Hydra never opens audio, so it puts the song's stated length (song.ini or the .srb field, moved into chart time by the delay or Offset) in that seat.
Label: Unknown
Evidence: the audio manager method `0x36B700` is not in the decompile and has no listing; Hydra's side is src/app/song_length.h lines 40 to 50.
Game code (6142): RVA `0x36B700`; listing lost; read 2026-10-18 in session b71e9156.
Hydra: `stated_length_ms` in src/app/song_length.cpp line 13, used in `app::song_length_ms` line 38.
Ruling: D255 item 1: "Option C (Recommended)". The record says: "It matches the game whenever the stated length is the game's own record of the audio."
Found: 2026-10-18, session b71e9156.

### CR-48 song_length and [music_end] are not read on the song-end path

Rule: song.ini `song_length` is never read when the game builds the playing length, and [music_end] is not read at all.
Label: Unknown
Evidence: music_end and [music_end] are not among the 23,972 string literals of `stringliteral.json` (searched 2026-10-30), which shows only that no code holds it as a literal; the "never read" part rests on the lost reading of `0xD8D80`.
Game code (6142): RVA `0xD8D80`; listing lost; read 2026-10-18 in session b71e9156.
Hydra: `is_end_event` in src/parse/song.cpp line 46 accepts only `end`.
Ruling: none found.
Found: 2026-10-18, session b71e9156; string search by wave 2 scout R3.

### CR-49 A .srb's stated length can differ from its audio

Rule: a bundle's stated song length can be longer than its real audio (Pressure states 273 s and holds 223 s of audio).
Label: Unknown
Evidence: unchecked by the scout: it would need the audio decoded and measured. The stored length fields of all 30 bundles were re-read on 2026-10-30 and match docs/srb-format.md.
Game code (6142): not found: data, not code.
Hydra: `srb_parse_metadata` in src/parse/srb.cpp line 136 (the length at lines 178 to 180).
Ruling: none needed.
Found: 2026-09-29, session 7c71d4dd.

### CR-50 Timing that makes a beat zero, negative or infinite refuses the chart

Rule: Hydra refuses a chart whose timing makes a measure or beat last zero, negative or infinite time, and ignores a time signature with a top number of 0; what the game does there is unread.
Label: Hydra assumes it, never checked
Evidence: D6 in docs/audit/2026-10-03-fix-decisions.md line 17.
Game code (6142): not found: not read.
Hydra: no owner located for this page; the refusal is D6's error sentence.
Ruling: D6: chose "Refuse with error (Recommended)" (card cebf3006, 21:45 UTC).
Found: 2026-10-03, session cebf3006.

## Rows waiting on a live run

CR-01, CR-02, CR-08, CR-16, CR-23, CR-24, CR-29, CR-31, CR-34, CR-39, CR-44, CR-46, CR-47. Each is listed on the live-checks page.

## Open splits

CR-29 (Q-15), CR-34 (Q-11), CR-39 (Q-75), CR-44 (Q-74). Each is written up on the questions page.
