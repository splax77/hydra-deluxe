# Reading a chart [CR]

This page covers how Clone Hero finds and reads a chart: the notes file, the song id, tracks, song.ini keys, sections, solos and the song end.
It describes build 1.1 (v1.1.0.6142).
Last write-back: 2026-11-23.

## Finding the notes file

### CR-01 When a folder or .sng holds both notes files, notes.mid wins

Rule: Hydra takes the first `notes.mid` it finds, else the last `notes.chart`; the game's own choice has never been read.
Label: Hydra assumes it, never checked
Evidence: Hydra's rule is `pick_notes_file` in src/parse/chart_files.cpp line 36, the same order public Hydra used (3b49012c, hydra/hysong.py lines 174 to 189). The derivation ledger (docs/audit/2026-10-03-derivation-ledger.md lines 4064 and 4713) says the old tools/ch_probe player preferred `.chart`, the opposite; that was not re-checked.
Game code (6142): not found: no session read the game's folder scan or its .sng reader.
Hydra: `pick_notes_file` in src/parse/chart_files.cpp line 36; `notes_file_format` line 29.
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### CR-02 The notes files and song.ini are found in any letter case

Rule: Hydra finds `notes.mid`, `notes.chart` and `song.ini` whatever their letter case; whether the game does the same has not been read.
Label: Hydra assumes it, never checked
Evidence: `notes_file_format` and `is_song_ini` in src/parse/chart_files.cpp lines 29 and 46. Public Hydra compared the names exactly (3b49012c, hydra/hyutil.py lines 153 to 157), so this is a Hydra Deluxe change.
Game code (6142): not found: the game's file-name test was never read.
Hydra: `notes_file_format` and `is_song_ini` in src/parse/chart_files.cpp.
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### CR-03 A .srb is a 16-byte header, two DEFLATE streams, two image blobs, then encrypted audio

Rule: a .srb (the container of the songs that ship with the game) is a 16-byte header, then the song-info stream and the chart stream as raw DEFLATE, then the two image blobs, then an encrypted audio section cut into one stem after another. The game's reader skips the header, inflates the song info and then the chart, steps over the image bytes by sizes it read from the song info, and cuts the rest into stems. Two more facts are not shown in the code; they were seen in the bytes, and the result of that decode is not kept. The first is the song-info field list: the notes file name, eight text fields, 12 difficulty bytes, the preview start, the song length in ms and a 16-byte checksum. The game reads those fields in the call 0x1CF270, which is unread (CR-51). The second is that the two image blobs (streams 3 and 4) are DEFLATE streams with JPEG inside; the game's reader steps over them without inflating them. The byte layout lives in docs/srb-format.md, which this row does not copy.
Label: Proven in the game's code
Evidence: `code/6142/srb_reader_2B84A0.asm` lines 88 to 116 read the 16 header bytes and decode the song-info size from them (CR-05). Lines 130 to 182 inflate the song info from offset 16 with a raw DeflateStream through the helper `code/6142/srb_inflate_2E0A10.asm`, and lines 350 to 404 inflate the chart stream after it. Lines 405 to 417 add up the image sizes to step over the images, and lines 418 to 478 cut the audio section into stems, each opening with the 32-byte entry header that `code/6142/srb_stem_ctor_1F2550.asm` reads. The bytes of all 30 bundles in the game's install folder, decoded on 2026-10-30, show the same order (every stream inflates in order, streams 3 and 4 start FF D8, Biology's length field reads 196,905 ms); no result file of that decode is saved.
Game code (6142): the .srb reader, RVA `0x2B84A0`, `code/6142/srb_reader_2B84A0.asm` lines 88 to 478; the inflate helper RVA `0x2E0A10`; the stem constructor RVA `0x1F2550`.
Hydra: `srb_inflate_stream` in src/parse/srb.cpp line 100, `srb_parse_metadata` line 136, `srb_read_metadata` line 188, `kSrbHeaderSize` in src/parse/srb.h line 43. The game takes the song-info stream's length from the header; Hydra inflates from offset 16 until the stream ends. On a valid file both read the same bytes.
Ruling: none needed: same rule.
Found: 2026-09-29, session 7c71d4dd; re-derived 2026-10-30 in session b02ab3e6 (the reference build); the reader listed 2026-11-08 in session 77057df2 (the Clone Hero unknowns check).

### CR-04 The game has a header scrambler and its exact inverse

Rule: the game holds a decoder that maps two 64-bit numbers a and b to mix(((a xor b) times 0x308DBC3) and 0x1FFFFFFFFF) minus 1, and an encoder that undoes it for every n below 2 to the 30th; "mix" xors back a 7-bit-shifted copy under the mask 0x550055.
Label: Proven in the game's code
Evidence: `code/6142/header_decode_214EFD0.asm` lines 1 to 13 and `code/6142/header_encode_214F0A0.asm` lines 1 to 14, no calls and no globals in either. A Python re-run of both over 200,000 random pairs on 2026-10-30 gave 0 mismatches; its script was not kept. The decoder is also in the 2026-09-17 Ghidra decompile, lines 37889 to 37899.
Game code (6142): two static methods of class #8599 (obfuscated names), RVAs `0x214EFD0` and `0x214F0A0`, listings above.
Hydra: no owner: Hydra never decodes the header, it skips it (`kSrbHeaderSize` in src/parse/srb.h line 43).
Ruling: none needed: Hydra reads no field the header holds.
Found: 2026-09-29, session 7c71d4dd; listed 2026-10-30 in session b02ab3e6 (the reference build).

### CR-05 What the 16 header bytes of a .srb hold

Rule: the game's .srb reader reads header bytes 0 to 7 as one 64-bit number in big-endian order and bytes 8 to 15 as one in little-endian order. It passes bytes 8 to 15 first and bytes 0 to 7 second to CR-04's decoder, and the result is the stored length of the song-info stream that follows the header. A length larger than the file makes the reader return without reading the song. The decoder's result depends only on the two numbers xored together, so "key" for bytes 0 to 7 and "scrambled size" for bytes 8 to 15 are names for the roles, not two different kinds of field.
Label: Proven in the game's code
Evidence: `code/6142/srb_reader_2B84A0.asm` lines 88 to 95 read bytes 0 to 7 and byte-swap each 32-bit half, and lines 107 to 110 put the halves back together swapped, which is a big-endian read. Lines 99 to 102 read bytes 8 to 15. Line 113 calls the decoder with bytes 8 to 15 as its first argument. Lines 114 to 116 compare the result with the file's length and leave when it is larger, and lines 130 and 156 to 160 use it as the song-info stream's byte count. The encoder `0x214F0A0` has no direct caller found (byte scan of il2cpp and .text). The older observation in docs/srb-format.md ("The header") that bytes 12 to 15 hold a small number from 1 to 29 fits this byte order.
Game code (6142): the .srb reader, RVA `0x2B84A0`, `code/6142/srb_reader_2B84A0.asm` lines 88 to 160; the decoder RVA `0x214EFD0` (CR-04).
Hydra: no owner: `kSrbHeaderSize` in src/parse/srb.h line 43 skips the 16 bytes, and Hydra never decodes them.
Ruling: none needed.
Found: 2026-09-29, session 7c71d4dd; re-read 2026-10-30 in session b02ab3e6 (the reference build); the reader listed 2026-11-08 in session 77057df2 (the Clone Hero unknowns check).

### CR-06 The .srb audio key is built at run time and equals Hydra's

Rule: the game builds the 16-byte AES key for bundled audio at run time with a loop that runs 41 times over two 64-bit constants; the two results, little-endian, are exactly Hydra's key.
Label: Proven in the game's code
Evidence: `code/6142/aes_key_build_215AFE0.asm` constants at lines 6 and 7, the loop at lines 10 to 31, the two key writes at lines 44 and 51. A Python run of the loop on 2026-10-30 gave the bytes of `kSrbAesKey`; its script was not kept. Also decompile lines 46814 to 46850.
Game code (6142): a static method of class #8616, RVA `0x215AFE0`, `code/6142/aes_key_build_215AFE0.asm` lines 6 to 51.
Hydra: `kSrbAesKey` in src/app/preview_source.cpp line 53.
Ruling: none needed: same rule. The user asked for it: "implement the decryptor into hydra's preview so srb songs can play audio normally" (card 2026-09-15-450488b5, 03:24 UTC).
Found: 2026-09-29, session 7c71d4dd; listed 2026-10-30 in session b02ab3e6 (the reference build).

### CR-07 The .srb audio cipher mode and IV

Rule: each audio blob is AES-128-CFB with that key, and its IV is the blob's 16-byte header with its two halves swapped. One part of this is proven in the game's code: the block cipher the game's managed code holds has the shape of AES-128 encryption (ten rounds on a 16-byte block, driven by lookup tables), and it only encrypts, which is what CFB needs in both directions. The CFB feedback and where each IV half lands are still unknown, and the table contents were not compared with AES's.
Label: Unknown
Evidence: the proven part is `code/6142/srb_cipher_managed_32AA80.asm`: lines 12 to 21 xor the four input words with the first round key, lines 24 to 283 loop four times with the counter stepping by two, lines 285 to 402 run one more round and lines 403 to 491 run the last round from byte lookups, ten rounds in all. `code/6142/srb_cipher_rounds_336E50.asm` line 121 falls back to that managed copy when Unity Burst is off; with Burst on, line 110 jumps to compiled code outside GameAssembly.dll, which was not read. For the mode: the stem entry header holds the two 64-bit IV fields at bytes 8 and 16 (`code/6142/srb_stem_ctor_1F2550.asm`), and the stem's read method xors each stored byte with a byte of a 16-byte block it refills through the cipher (`code/6142/srb_stem_read_1F2260.asm`, `code/6142/srb_stem_iv_1F2060.asm` and `code/6142/srb_stem_iv_setup_1F1D50.asm`). The first refill is built from the field at +0x60 and then +0x58, which fits "halves swapped", and later refills take stored bytes, which fits CFB. Several values in those three listings decode to numbers a 16-byte block would not need, so where each half lands cannot be read from them. Session 450488b5 reached the decryptor by hooking BASS and working back from known plaintext; no address, listing or result file of that is in the repo.
Game code (6142): the managed block cipher RVA `0x32AA80` and its Burst dispatch RVA `0x336E50`; the stem methods RVAs `0x1F2550`, `0x1F2260`, `0x1F2060` and `0x1F1D50`; the listings above.
Hydra: the decryptor beside `kSrbAesKey` in src/app/preview_source.cpp (AES-128-CFB). Nothing found in the game's code contradicts it.
Ruling: none needed.
Found: 2026-09-15, session 450488b5; the cipher listed 2026-11-08 in session 77057df2 (the Clone Hero unknowns check).

### CR-08 A .srb whose notes entry is neither .mid nor .chart

Rule: Hydra reads such an entry by sniffing the chart bytes for `MThd` rather than refusing it; the game's handling is unread, and none of the 30 shipped bundles has such a name.
Label: Hydra assumes it, never checked
Evidence: `load_songpath_srb` in src/parse/song.cpp line 2101 holds the sniff; docs/audit/2026-10-29-audit-of-audits/questions.md lines 63 to 75 found no real container with an odd name. The audit's Q5 quotes the chat batch behind D27 as saying ".sng and .srb would find their notes file the same way" (D27 itself says "one container-entry rule (61)") and shows the two differ today.
Game code (6142): not found: not read.
Hydra: `load_songpath_srb` in src/parse/song.cpp line 2101.
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### CR-09 A .sng stores each file XORed with a 16-byte mask

Rule: a .sng is a 10-byte prefix, a 16-byte XOR mask, a metadata block and a file table; byte i of each file is stored XORed with mask[i % 16] ^ (i & 0xff).
Label: Hydra assumes it, never checked
Evidence: Hydra's reader, and public Hydra's (3b49012c, hydra/hysong.py lines 163 to 196), read it this way. The game's .sng reader class is #1068; its static constructor uses the literal "SNGPKG" (stringliteral.json), and none of its methods was listed or read.
Game code (6142): not found: the .sng class's static constructor `0x266340` was located, not read.
Hydra: `kSngXorMaskOffset` in src/parse/sng.h, `sng_read_notes` in src/parse/sng.cpp line 148.
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build).

## The song id

### CR-10 A song's id is the MD5 of its notes file, or the stored checksum for a .srb

Rule: Clone Hero's song id (the key of the player's saved scores) is the MD5 of the notes file for a folder chart, the MD5 of the unmasked notes file for a .sng, and the 16-byte checksum stored in the .srb's song info for a .srb; a revised chart gets a new id.
Label: Proven in a live run
Evidence: on 2026-10-30 the game's own `songcache.bin` (8,221,087 bytes, written 2026-10-28 20:35 by the game) held the stored checksum of 30 of 30 bundles and the whole-file MD5 of none of them. The folder half is also seen in a run of 2026-11-08: the game's score file C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1\scorestats-The_Who_-_Won_t_Get_Fooled_Again_O-20261108-122918.json has `checksum` AF44D5C8D941BB33444D18E7871CDC77, and that is the MD5 of the chart it played, The Who - Won't Get Fooled Again (O), notes.mid (hashed 2026-11-08). The .sng half (54 of 54) rests on the 2026-10-09 check written out in CR-12 and was not re-run.
Game code (6142): not found: the checksum route through `SongEntry` is page 7's; no listing.
Hydra: `song_id_read` in src/app/analysis.cpp line 274; `hash_chart_file` in src/app/analysis.h line 103.
Ruling: D170: "yes, go ahead, let the 84 results drop" (session bc0144a4, 03:59 UTC).
Found: 2026-10-10, session bc0144a4; re-checked 2026-10-30 in session b02ab3e6 (the reference build); 2026-11-08, session 77057df2 (docs-gaps search).

### CR-11 A .srb's stored checksum is the chart stream's MD5 in 26 of 30 bundles

Rule: for 26 of the 30 shipped bundles the stored checksum equals the MD5 of the chart stream; Good Grief Retreat, No Known Suspects, Stigma and Troopers of the Stars differ, for a reason nobody has found.
Label: Proven in a live run
Evidence: re-run on 2026-10-30 over the 30 real files against the game's `songcache.bin`; the four names match the 2026-10-10 record (ADR 0028 as it stood at commit 5fbe71ae, lines 41 to 45). Card 7c71d4dd ruled out line endings, byte-order marks and dropped MIDI tracks as the cause.
Game code (6142): not found: data, not code.
Hydra: `song_id_read` in src/app/analysis.cpp line 274 reads the stored checksum and never recomputes it.
Ruling: D170 (as CR-10).
Found: 2026-09-29, session 7c71d4dd; re-run 2026-10-30 in session b02ab3e6 (the reference build).

### CR-12 The .sng and folder halves of the id rule

Rule: the checks of 2026-10-09 and 2026-10-10 looked for a chart's 16 id bytes anywhere in the game's song cache, not in a decoded id field. The cache listed 54 .sng songs: the MD5 of the notes file inside was in the cache for all 54, and the whole-file MD5 for none of them. For a sample of 300 folder charts (the first 300 the probe met when walking the user's song folder), the MD5 of the notes file was in the cache for all 300. In the game's code, one part is proven: `SongEntry.checksum` is the MD5 of the first count bytes of a byte array, and that array comes from one function for a folder chart and another for a .sng. Which bytes those two functions return was not read, and neither was the cache writer.
Label: Unknown
Evidence: the cache record is ADR 0028 as it stood at commit 5fbe71ae, lines 12 to 45, now cut down to a pointer here; it was not re-run, and no tool or result file for either check is in the repo (a grep of tools/ for `songcache` finds nothing). The code part: `code/6142/song_hash_md5_20FA8F0.asm` line 15 makes an MD5 and lines 24 to 27 call ComputeHash(array, 0, count). `code/6142/songentry_hash_chart_35F6E0.asm` line 21 branches on the song's isSng byte; line 32 calls `0x1313F0` for a folder chart and line 53 calls `0x262100` for a .sng, and line 61 hashes the result. `0x1313F0` works through an ArrayPool and a cache; what it fills was not read. Direct callers of the hash method found (byte scan of il2cpp and .text): `SongEntry` methods at 0x35C110, 0x35F6E0 and 0x35F8B0, the song cache's 0x27EE20, and three tail jumps at 0x282C04, 0x285783 and 0x2EFD42; the tail jumps were not read.
Game code (6142): the checksum's hash method RVA `0x20FA8F0`, `code/6142/song_hash_md5_20FA8F0.asm` lines 15 to 27; `SongEntry`'s chart hash RVA `0x35F6E0`, `code/6142/songentry_hash_chart_35F6E0.asm` lines 21 to 61; the two byte sources RVAs `0x1313F0` and `0x262100`, not read.
Hydra: `song_id_read` in src/app/analysis.cpp line 274 hashes a folder chart's notes file whole and, for a .sng, the bytes of the notes file `sng_read_notes` picks. The game hashes the array its two byte sources return. Whether those are the same bytes, and whether Hydra's .sng file pick is the file the game hashes, were not compared. The user chose to record this, and Hydra is unchanged (session 428cf079, 2026-11-11).
Ruling: D170 (as CR-10).
Found: 2026-10-10, session bc0144a4 (ADR 0028); the hash method listed 2026-11-08 in session 77057df2 (the Clone Hero unknowns check).

## Track names and track 0

### CR-13 A .mid drum track is named "part drums" or "part drum", in any case

Rule: the game lowercases each track name and compares it exactly with "part drums" and "part drum"; nothing is trimmed.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm line 218 (the lowercase call at 0x2155FDF), lines 297 to 302 (the "part drums" compare at 0x2156134) and lines 503 to 508 (the "part drum" compare at 0x21564A8).
Game code (6142): a method of the .mid reader class #8626, RVA `0x2155C00`, the listing above.
Hydra: `is_drums_track_name` in src/parse/midi.cpp line 447, names in `kDrumsTrackNames` in src/parse/midi.h line 98.
Ruling: D252 states the rule and the addresses; the user's quoted call on it is only the build approval, chose "Yes, build it" (card 2026-10-17-77c0ebab, 00:31 UTC).
Found: 2026-10-16, session 77c0ebab.

### CR-14 When two tracks carry a drum name, the first in file order wins

Rule: the game adds each instrument it reads to a set, and a later track of an instrument already in the set is skipped.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm lines 509 to 516: the set's Add call at 0x21564D7, and a false return jumps to the next track at 0x2156665.
Game code (6142): RVA `0x2155C00`, lines 509 to 516.
Hydra: `MidiFile::drums_track` in src/parse/midi.cpp line 490.
Ruling: D252 (as CR-13; the record states the rule, "the first drum-named track in file order wins, as in the game").
Found: 2026-10-16, session 77c0ebab.

### CR-15 Track 0 is never read as an instrument

Rule: the instrument loop starts at track index 1, so a drum track in track 0 (a format-0 single-track file) is not drums in the game.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm lines 197 to 201 (the loop counter set to 1 at 0x2155F8D).
Game code (6142): RVA `0x2155C00`, line 198.
Hydra: `MidiFile::kFirstInstrumentTrack` in src/parse/midi.h line 199, used by `MidiFile::drums_track`.
Ruling: D253 item 1: "Match the game (Recommended)".
Found: 2026-10-16, session cfb159b5.

### CR-16 A track named "notes" then "PART DRUMS" at tick 0 is the drum track

Rule: the game plays Hormone's Echo with drums, whose drum track carries two names at tick 0, "notes" then "PART DRUMS"; Hydra's rule is that a track's name is its first recognised name.
Label: Hydra assumes it, never checked
Evidence: the user reported the game reads it; no agent checked. The .mid reader listing only consumes each track's finished name (midi_reader_2155C00.asm line 170); the code that builds that name from several name events was not read.
Game code (6142): not found: the name-building step was not read (mtrk_read_214F6D0.asm exists and was not traced for it).
Hydra: `TrackNamePick` in src/parse/midi.cpp line 274; `recognized_track_name` line 25.
Ruling: D78: "clone hero is able to read hormone's echo correctly, hydra should also support it." (card 2026-10-06-d3bdb873, 00:16 UTC). It sets the target, not which name the game picks.
Found: 2026-10-05, session d3bdb873.

### CR-17 The .mid reader has a second path behind a thread-local switch

Rule: the .mid reader class keeps a static thread-local switch. The main .mid reader sets it at the start of every read from a byte of the chart-read settings, and `SongEntry`'s chart loader fills that byte from its own first argument. When the switch is true, the drum reader runs a light scan in place of a full drum read. The light scan walks the drum track's events, raises the two track flags (CR-18), and adds one placeholder note with every value zero to each difficulty the first time that difficulty has a gem. It then writes the two flags to every track of the instrument. The game's play load passes false and reads the notes. Of the loader's 12 direct callers, six pass true, five pass false and one passes a variable. Five of the seven calls in the song cache pass true.
Label: Proven in the game's code
Evidence: `code/6142/mid_drum_reader_2155050.asm` lines 77 to 79 call the getter `0x21581E0` and jump to the other path when it is true. `code/6142/mid_reader_cctor_2158260.asm` builds the switch, and docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm lines 74 to 76 set it from settings byte +0x2C, the ninth argument of the settings constructor (`code/6142/chart_settings_ctor_20E73E0.asm`). `code/6142/songentry_load_35C110.asm` lines 163 to 174 pass the loader's first bool argument there. The loader has 12 direct callers (byte scan of il2cpp and .text). Five pass false: 0xD92BA in a `GameManager` coroutine, 0x235BCD in `GlobalVariables`, 0x285045 and 0x2894CF in the song cache (class #1136), and 0x396A4F in the command-line `--song` loader `code/6142/cli_loader_3968D0.asm`. Six pass true: 0x27E86D, 0x28A8EF, 0x2848FF, 0x2858A2 and 0x28821F in the song cache, and 0x397A79 in the `CliGameLauncher` method at 0x3978E0, which has no listing. The call at 0x397261, in the `CliGameLauncher` method at 0x3970D0 (no listing), passes a variable. The argument at each site was read on 2026-11-11 by docs/audit/2026-11-08-clone-hero-unknowns/main/songentry_load_callers.py; its output, songentry_load_callers_output.txt beside it, shows the instructions that set the bool (edx) before each call. Which method holds each site was looked up in dump.cs. In `code/6142/mid_alt_path_2154180.asm`, lines 60 to 100 raise the flags, lines 113 to 124 call the note adder 0x20CEC60 with zero arguments for a newly seen difficulty, and lines 158 to 182 write the flags. docs/audit/note-shuffle/step0/open1/midi_sink_20CEC60.asm lines 30 to 41 show that adder builds a note from those arguments. The note beside the alt-path listing says it stores no notes; the listing shows the placeholder note.
Game code (6142): the drum reader RVA `0x2155050`, the switch getter RVA `0x21581E0`, the main reader RVA `0x2155C00`, the settings constructor RVA `0x20E73E0`, `SongEntry`'s loader RVA `0x35C110`, the light scan RVA `0x2154180`; the listings above.
Hydra: no owner: Hydra has no light scan, and its .mid reader always reads the drum notes (`MidiFile::drums_track` in src/parse/midi.cpp line 490). The game's play load reads the notes too, so play shows no difference.
Ruling: none needed.
Found: 2026-10-30, session b02ab3e6 (the reference build); the switch traced 2026-11-08 in session 77057df2 (the Clone Hero unknowns check).

### CR-18 Where the two track flags at +0xA1 and +0xA2 come from

Rule: the .mid reader starts the two flags from the song's settings object (class #8267): +0xA1 from its +0x10 byte, which is song.ini's five_lane_drums, and +0xA2 from its +0x11 byte, which is song.ini's `pro_drums`. It raises the first for a note on pitch 65, 77, 89 or 101 and the second for a tom marker, and writes both to every track it fills. The .chart loader starts both at zero, takes each from the first drum track whose N handler raised it (N 5 for the first, N 66 to 68 for the second), and writes both to every Drums and ProDrums track; it does not read song.ini for them. The five-lane conversion reads both flags and the same five_lane_drums byte (NL-74). One more reader is the song cache, for a track whose instrument value is 2, treats "either flag up" the same as the song's two force switches, forceProDrums and forceFiveLane (dump.cs names), and then updates the song's instrument-and-difficulty mask. Which entry that update adds, and so what the flags mean (the names five-lane and pro-drums are guesses from what sets them), is unread.
Label: Unknown
Evidence: the starting bytes (read 2026-11-12, session c61c1986): `code/6142/mid_drum_reader_2155050.asm` lines 80 to 99 load the settings object from the reader's `in` argument and copy its +0x10 and +0x11 bytes into the two flag slots. `code/6142/chart_settings_ctor_20E73E0.asm` lines 33 and 34 store the constructor's first two bools there, and `code/6142/songentry_load_35C110.asm` lines 128 to 174 pass `GlobalVariables.currentSongMetadata` +0x38 and +0x39 as those bools, which `code/6142/ini_song_reader_1EFC60.asm` fills from five_lane_drums (lines 77 to 82) and `pro_drums` (lines 84 to 89). That the metadata object read at load is the one filled from this song's song.ini is not traced in the code. For five_lane_drums a live run now shows it: the Five Lane C bot run of 2026-11-12 took the five-lane branch from that key alone (NL-74, `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\five-lane\scorestats-Five_Lane_C-20261112-104841.json`). That was a .chart, so it shows the key reaching the settings object, not the .mid flag. For `pro_drums` a live run shows it too: probe G (`C:\Clone Hero\songs\Hydra Probe\Mid No Pro G`) is probe F's .mid, with no tom marker and no fifth pad, plus song.ini `pro_drums = 1`. Its bot run of 2026-11-12 (Clone Hero v1.1.0.6142-final, ProDrums Expert) scored 6,470 (note_score 2,330, combo_score 4,140, 43 of 43), the total with the 12 yellow, blue and green notes kept as cymbals, where probe F with no key scored 5,750 (NL-74): `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\five-lane\scorestats-Mid_No_Pro_G-20261112-164123.json`, summed up in `probes_G_J.txt` in the same folder. So the key raised the .mid's Pro flag, and the game read "1" as true (CR-54). The Five Lane E bot run of the same day fits this block's .chart reading: a .chart with no N 5, no N 66 to 68 and five_lane_drums = True paid yellow and the fourth pad as cymbals (6,490, NL-75), because the conversion's settings byte picks the five-lane branch and that branch returns before the all-toms tail (`code/6142/five_lane_convert_215D450.asm` line 122). The run cannot tell this reading from one where the .chart loader also starts +0xA1 from five_lane_drums, because both send E down the five-lane branch. The .chart side: `code/6142/chart_loader_2161a80.asm` lines 158 and 159 (both start at zero), 160 to 185 (the first drum track that has each flag up) and 209 to 214 (the stores to tracks whose instrument byte is 6 or 9). A byte scan of the DLL for stores to +0xA1 and +0xA2 in the chart classes, 2026-11-12, finds only these readers, the .chart N handler 0x213D620, the .chart method 0x213DD40 and the .mid light scan 0x2154180. The label stays Unknown for the song cache part below. The writers: `code/6142/mid_drum_reader_2155050.asm` lines 87 to 99 (starting bytes), 136 to 147 (the call to `0x210D780`, which tests those four pitches), 270 (the tom marker) and 552 to 557 (the stores); the light scan of CR-17 tests the tom marker as pitch 110 to 112 (`code/6142/mid_alt_path_2154180.asm` lines 60 to 100). The reader: the track's method `0x215E000` returns "+0xA1 or +0xA2" (`code/6142/track_drum_flags_get_215E000.asm`), and the class has no getter for either flag alone. Its three direct callers (byte scan of il2cpp and .text) are song cache methods 0x27E730, 0x2847C0 and 0x2880E0. In `code/6142/songcache_drum_flags_2847C0.asm`, line 113 tests the track's instrument byte for 2, which the instrument enum (#8326, not checked) names Rhythm, where Drums is 6 and ProDrums 9. Lines 117 to 123 call `0x215E000` and test the song's bytes +0x92 and +0x93, and when any is set, lines 124 to 128 call `0x1F1250` on the mask at +0x20. The other two cache methods use other instrument values, and the mask bit was not worked out. No scan for raw +0xA1 or +0xA2 reads was run, because every class with a field there would match.
Game code (6142): the drum reader RVA `0x2155050` and the pitch test RVA `0x210D780`; the settings constructor RVA `0x20E73E0`, `SongEntry` RVA `0x35C110`, the song.ini reader RVA `0x1EFC60` and the .chart loader RVA `0x2161A80`; the conversion RVA `0x215D450`; the flags getter RVA `0x215E000`; the song cache method RVA `0x2847C0` and the mask update RVA `0x1F1250`; the listings above.
Hydra: the five-lane conversion reads both flags and both song.ini keys: `midi_drum_chart_flags` (a .mid starts the flags from the keys), `ChartParser::load_sections` and `ChartParser::parse` (a .chart starts them at zero and takes only five_lane_drums) and `convert_drum_pads`, all in src/parse/song.cpp. On probe G the branch's `hydra_replay score --prodrums 1` gives 6,470 (notes 2,330, combo 4,140, 12 cymbal notes), the game's total. Hydra still takes Pro Drums for scoring from the player's setting (`mode_pro_`), and it keeps no per-song list of drum modes for the song cache's reader.
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build); the cache reader found 2026-11-08 in session 77057df2 (the Clone Hero unknowns check); the starting bytes, the .chart side, the Five Lane C and E runs and the probe G run 2026-11-12, session c61c1986.

## The events track

### CR-19 The events track is the first track named "events", in any case

Rule: the game searches from track 0 for a name equal to "events" ignoring case, reads the first match and stops, so only one events track counts.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm lines 154 to 181: the loop starts at 0, the compare with comparison type 5 (ignore case) is at 0x2155F36, and a match leaves the loop at 0x2155F3D.
Game code (6142): RVA `0x2155C00`, lines 154 to 181.
Hydra: `MidiFile::events_track` in src/parse/midi.cpp line 496; `is_events_track_name` line 452.
Ruling: D253 item 2: "Match the game (Recommended)".
Found: 2026-10-16, session cfb159b5.

### CR-20 A .chart E line is trimmed, then tested for [mix, mix, solo and soloend in that order

Rule: the .chart E handler reads the rest of the line up to LF or CRLF, trims both ends, then tests in order: starts with `[mix`, starts with `mix` (both case-sensitive), equals `solo`, equals `soloend`.
Label: Proven in the game's code
Evidence: `code/6142/chart_e_handler_213CAF0.asm` lines 65 to 112 (end of line and the trim), 116 to 168 (the two starts-with tests), 169 to 230 (the solo compares); decompile lines 24441 to 24700 give the same four literals in the same order.
Game code (6142): a method of the .chart line class #8615, RVA `0x213CAF0`, the listing above.
Hydra: `classify_chart_line` in src/parse/song.cpp lines 1286 to 1318, with `trim_view`.
Ruling: none needed: same rule.
Found: 2026-10-03, session cebf3006; listed 2026-10-30 in session b02ab3e6 (the reference build).

### CR-21 A .chart line is dispatched by its first letter, E, N or S

Rule: the .chart line dispatcher sends a line to the E, N or S handler by its first letter, upper case only; any other line is skipped to its end.
Label: Proven in the game's code
Evidence: decompile lines 24907 to 24950 (FUN_18213d2d0); a listing of the same function is committed as docs/audit/note-shuffle/step0/open1/trackbuilder_dispatch_213D2D0.asm, not re-read for this row.
Game code (6142): RVA `0x213D2D0`, decompile lines 24844 to 24968.
Hydra: `classify_chart_line` in src/parse/song.cpp lines 1286 to 1328 compares the first whitespace-split word.
Ruling: none needed: same rule for normal files.
Found: 2026-10-03, session cebf3006; re-read 2026-10-30 in session b02ab3e6 (the reference build).

### CR-22 The .chart reader has a flag-only pass

Rule: when the line state's scanning byte is set, the E handler returns before recording anything and the N handler only raises the two track flags (CR-18) and stores no note.
Label: Proven in the game's code
Evidence: `code/6142/chart_e_handler_213CAF0.asm` lines 113 to 115; `code/6142/chart_n_handler_213D620.asm` lines 27 to 59. The flag-only branch runs only when the track's note list already holds more than one item and a test at `0x215FE80` returns true. What sets the scanning byte was not read.
Game code (6142): RVAs `0x213CAF0` and `0x213D620`, the listings above.
Hydra: no owner: Hydra has no flag-only pass (the `CPhase` passes in src/parse/song.cpp lines 1373 to 1390).
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### CR-23 A .chart with notes out of tick order is read in tick order

Rule: Hydra settles chord order by tick once, before the phrase and fill rules; how the game orders such a file is unread.
Label: Hydra assumes it, never checked
Evidence: D47 says "Clone Hero's own ordering is not in the evidence file." None of the library's 4,338 loose .chart files is out of order (D47).
Game code (6142): not found: looked in ch-evidence.md, ADR 0023 and the open1 listings by name.
Hydra: the .chart reader in src/parse/song.cpp line 386 emits chords in tick order.
Ruling: D47 (record text, no quoted words): "A .chart whose drum notes are written out of tick order is read in tick order".
Found: 2026-10-04, D47.

### CR-24 The .mid events reader also tests [coda], coda and _ENDOFSONG

Rule: the game's .mid events handler trims each events-track text and tests it in this order: starts with `[section`, starts with `[prc`, equals `[end]`, equals `[coda]`, equals `coda`. A `[section` text whose untrimmed text contains _ENDOFSONG is not made a section; it falls through to the later tests. _ENDOFSONG has no branch of its own: that veto is all it does. `[end]` calls the song's method `0x212E310` with the event's tick (what that stores is CR-38's). A `coda` or `[coda]` text stores a coda marker with its tick on the song, the first one only; Hydra reads that (FL-08 on page 4). Hydra applies the same veto to a .mid `section ` text. The game's .chart reader has no such test, so a .chart `section _ENDOFSONG` line is a section in both. Hydra also tests `section ` with the space, where the game tests `[section` without one (CR-29).
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm lines 99 to 128 (the `[section` test), 129 to 136 (String.Contains with _ENDOFSONG on the untrimmed text, which skips the section step), 190 to 216 (the `[prc` test), 217 to 243 and 319 to 324 (`[end]` and the call), and 244 to 314 (the two coda compares, the empty test on the song's field at +0x100 at lines 300 and 301, and the store of a "coda" record at line 312). Any other text goes to the loop's next step. Who reads the coda record later was not searched. `code/6142/mid_events_coda_21544E0.asm` lines 245 to 312 hold the same coda branches. The Contains test counts case: `code/6142/string_contains_197F670.asm` lines 34 to 39 call an ordinal search with ignoreCase 0 (line 35), and lines 42 and 43 turn its index into the result (`code/6142/string_contains_197F670.md`). So "[section _endofsong]" in lower case stays a section. The .chart reader has no veto: in `code/6142/events_section_2151830.asm`, its text literals at lines 41 to 59 hold no _ENDOFSONG, and its section path at lines 325 to 528 makes no Contains call.
Game code (6142): the events reader RVA `0x21544E0`, the listings above; the end setter RVA `0x212E310`; String.Contains RVA `0x197F670`; the .chart `[Events]` line reader, `code/6142/events_section_2151830.asm`.
Hydra: `marker_name` in src/parse/section_names.cpp lines 19 to 27 holds the veto, for a .mid `section ` body only (lines 21 and 22, with the text `kMidSectionVeto` at line 13); `is_section_marker` (line 31) asks it, and `section_name_of` in src/parse/song.cpp (line 34) tells it which reader is asking, from the .mid reader at line 1185 and the .chart reader at line 1635. `MidiParser::parse` in src/parse/song.cpp reads the first `coda` or `[coda]` text of the events track before the drum pass and hands its tick to `place_authored_fills`.
Ruling: D334: the user chose "Match the game" for the _ENDOFSONG veto (2026-11-08, session 77057df2; `docs/sessions/2026-11-08-77057df2.md`). Hydra matches since this change. Two splits found on the way stay open: see questions page Q-83 (two sections on one tick) and Q-84 (a section text with white space round it).
Found: 2026-10-30, session b02ab3e6 (the reference build); the branches traced 2026-11-08 in session 77057df2 (the Clone Hero unknowns check); Hydra's veto added 2026-11-11, session 428cf079.

### CR-25 Ticks are the file's own ticks; no reader rescales them

Rule: both readers store each note's own 64-bit file tick unchanged, whatever the file's resolution.
Label: Proven in the game's code
Evidence: the chain of stores: docs/audit/note-shuffle/step0/open1/trackbuilder_addnote_213D620.asm line 141, mtrk_read_214F6D0.asm lines 175 and 208, songnote_init_214D240.asm line 18, build_gamenote_20D4BE0.asm lines 253 and 261, gamenote_ctor_214BF10.asm line 19, ending at docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm lines 84 and 85, which multiplies by that tick. docs/audit/note-shuffle/step0/FINDINGS.md lines 11 to 15 say the same.
Game code (6142): RVAs `0x213D620`, `0x214F6D0`, `0x214D240`, `0x20D4BE0`, `0x214BF10`, `0x20F0C30`, the listings above.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp line 121 reads the chord's own ticks.
Ruling: none needed: same rule.
Found: 2026-10-09, session 30beeb6e.

## song.ini keys the game reads

### CR-26 The game reads ten chart keys from song.ini's [song] section

Rule: one function reads `delay` (whole number, default 0), hopo_frequency (limited to 0 to 65535), five_lane_drums, `pro_drums`, eighthnote_hopo, multiplier_note, video_start_time, sustain_cutoff_threshold, chord_snap_threshold and loading_phrase into a settings object. Hydra reads `song_length`, `delay`, `five_lane_drums` and `pro_drums`. How the game lets `delay` and the chart Offset combine is page 3's.
Label: Proven in the game's code
Evidence: `code/6142/ini_song_reader_1EFC60.asm` lines 15 to 33 (the key literals), 38 to 47 (`delay`), 49 onward (hopo_frequency), 77 (five_lane_drums), 84 (`pro_drums`) and 91 (eighthnote_hopo).
Game code (6142): a method of class #828, RVA `0x1EFC60`, the listing above.
Hydra: `ini_delay_ms` in src/app/preview_source.cpp line 301 and the `song_length` read in src/app/analysis.cpp line 141; `ini_drum_key_is_true` in src/parse/song.cpp for the two drum keys (CR-18); no owner for the other keys (a grep of src/ for each key name finds none).
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### CR-27 The game writes missing keys back into song.ini

Rule: for each metadata key a song.ini lacks, `0x358850` appends a line such as song_length = {0} or `delay = {0}` (and name, artist, album, genre, year, charter, the difficulties, `pro_drums` and the rest) and logs "Creating metadata file for"; it writes, it does not read.
Label: Proven in the game's code
Evidence: `code/6142/song_ini_loader_358850.asm` line 88 (song_length = {0}), lines 316 to 345 (`delay`) and line 674 (`song.ini`).
Game code (6142): a method of `SongEntry` (TypeDefIndex 176), RVA `0x358850`, the listing above.
Hydra: no owner: Hydra never writes song.ini.
Ruling: none needed: Hydra reads the file only.
Found: 2026-10-18, session b71e9156 (the address); listed 2026-10-30 in session b02ab3e6 (the reference build).

### CR-54 How the game reads five_lane_drums and pro_drums as yes or no

Rule: from song.ini, both keys go through one bool getter. A value that parses as a whole number counts as true when the number is not 0, a negative number too. Otherwise the word "true" in any letter case is true and "false" in any case is false. A missing key and any other text, such as "yes", give the default, false. Blanks around the value change nothing, because the ini parser trims every value before a getter sees it. From a .sng, the same two keys go through a different getter, which takes only .NET's Boolean.TryParse: "true" or "false" in any case, and every other text, "1" included, gives the default, false.
Label: Proven in the game's code
Evidence: the song.ini getter `code/6142/ini_bool_getter_1D4480.asm`: lines 17 to 27 look the text up and return the default for a missing key, lines 28 to 31 call Int32.TryParse, lines 66 and 67 turn a number into "not 0" by an unsigned compare, lines 34 to 42 and 43 to 52 compare with "true" and "false" with comparison type 5 (OrdinalIgnoreCase), and line 74 returns the default. Which blanks and signs the number test takes is .NET's style 7 (`code/6142/int32_tryparse_1B4F710.asm` lines 56 to 62). The trim: `code/6142/ini_parse_21695E0.asm` line 418 trims the value. The song.ini reader calls this getter for five_lane_drums and `pro_drums` with a default of false (`code/6142/ini_song_reader_1EFC60.asm` lines 76 to 89). The .sng getter `code/6142/sng_bool_getter_2642C0.asm`: lines 20 to 25 look the key up in the metadata's Dictionary, a miss returns the default (lines 43 and 44), line 34 calls Boolean.TryParse, and a failed parse returns the default. The .sng metadata reader calls it for five_lane_drums and `pro_drums` with a default of false (`code/6142/sng_meta_reader_1EFF90.asm` lines 46 to 56). A live run backs the song.ini number case: probe G, with `pro_drums = 1` and no tom marker, kept its cymbals and scored 6,470 (CR-18). No run has played a .sng with a number for either key.
Game code (6142): the song.ini bool getter RVA `0x1D4480`, Int32.TryParse RVA `0x1B4F710`, the ini parser RVA `0x21695E0`, the song.ini reader RVA `0x1EFC60`, the .sng bool getter RVA `0x2642C0` and the .sng metadata reader RVA `0x1EFF90`; the listings above.
Hydra: `ini_drum_key_is_true` in src/parse/song.cpp reads a song.ini's two keys by the first getter's rule, and `sng_drum_key_is_true` reads a .sng's by the second's; `chart_drum_keys` picks the reader by the source. The tests are the getter cases among the "five-lane all-toms" tests in tests/test_five_lane.cpp.
Ruling: D340 (docs/audit/2026-10-03-fix-decisions.md), item 1: `five_lane_drums = 1` counts as yes, as in the game.
Found: 2026-11-12, session c61c1986 (the all-toms plan's T1 and the probe G run).

## Sections and their display names

### CR-28 A .mid section is [section X] or [prc_X], shown by the game's name table

Rule: the events reader takes a text that starts with `[section` or `[prc`, cuts the name, looks it up in an exact-case name table, and falls back to the name with each underscore turned into a space.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm lines 33 to 41 (the literals), 100 to 125 (the starts-with test), 146 to 158 (the name cut from character 9), 169 to 186 (the table lookup) and 175 to 182 (the underscore fallback), with the `[prc` branch from line 190. Decompile lines 41771 to 42125 agree.
Game code (6142): a method of the .mid reader class #8626, RVA `0x21544E0`, the listing above.
Hydra: `section_display_name` in src/parse/section_names.cpp line 35; `marker_name` line 19; `is_section_marker` line 31.
Ruling: D224: chose "As Clone Hero shows them (Recommended)" (session 28fad19c, 02:27 UTC).
Found: 2026-10-12, session 28fad19c; re-read 2026-10-30 in session b02ab3e6 (the reference build).

### CR-29 "[sectionX]" with no space is a section to the game, not to Hydra

Rule: the game's test is starts-with `[section`, then the name is cut from character 9, so a text like `[sectionX]` would be a section with a short or empty name; Hydra needs `section ` with the space.
Label: Unknown
Evidence: the starts-with test is midi_events_track_21544E0.asm lines 100 to 125 and the cut lines 146 to 158; not checked: it would need a .mid with such an event loaded in the game.
Game code (6142): RVA `0x21544E0`, lines 100 to 158.
Hydra: `marker_name` in src/parse/section_names.cpp line 19.
Ruling: open split: see questions page Q-15.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### CR-30 The game's section-name table holds 2,718 entries, and Hydra ships a copy

Rule: the game maps practice-section keys to shown names through a table of 2,718 entries, filled in its static constructor. Hydra ships a copy as data/clone-hero-1.1.0.6142-section-names.tsv, and its 2,718 rows equal the game's pairs, row for row and in the same order.
Label: Proven in the game's code
Evidence: `code/6142/section_table_cctor_2095040.asm` holds the whole static constructor; from line 10876 to its end it makes 2,718 calls to the dictionary Add at 0x182C503E0, each with a key literal and a shown-name literal. Those pairs were read in order and compared with the TSV's 2,718 data rows on 2026-11-08, once by the agent that made the listing and once by a fresh checker, and every row matched. The fresh checker's script is kept as docs/audit/2026-11-08-clone-hero-unknowns/main/cr30_section_table_compare.py; its output, cr30_section_table_compare_output.txt beside it (rerun 2026-11-11), reads 2,718 listing pairs, 2,718 TSV rows, 0 mismatches and 0 duplicate keys. The name "Breaking 'n' Entering" (listing line 25024) is printed in double quotes because it holds a single quote, and its text matches. Hydra's compiled table, src/parse/section_names_table.cpp, has 2,718 rows with the same keys; its shown names were compared through the TSV, not one by one.
Game code (6142): the section table's static constructor, RVA `0x2095040`, `code/6142/section_table_cctor_2095040.asm` from line 10876.
Hydra: `section_name_table` in src/parse/section_names.h line 41, defined in src/parse/section_names_table.cpp; generator tools/gen_section_names.py.
Ruling: D228: chose "Ship the list" (session 28fad19c, 02:44 UTC).
Found: 2026-10-12, session 28fad19c; the table listed and compared 2026-11-08 in session 77057df2 (the Clone Hero unknowns check).

### CR-31 A section key matches exactly, case included

Rule: Hydra looks a key up with its exact case, so `Intro` typed by hand misses where `intro` hits; the game's key compare was never read.
Label: Hydra assumes it, never checked
Evidence: the comment in src/parse/section_names.cpp lines 40 and 41; the game's lookup comparer (FUN_1813ff9d0, decompile lines 42110 to 42125) was not read.
Game code (6142): not found: the lookup's comparer was not read.
Hydra: `section_display_name` in src/parse/section_names.cpp lines 35 to 52.
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### CR-32 A .chart event's text is what lies between its first and last double quote

Rule: the game's .chart [Events] reader takes an event's text from just after the first double quote on the line to just before the last one, and tests that text for a section. Hydra trims the text after `E` and takes one pair of quotes off only when the text both starts and ends with one. The two agree on a line written `E "section intro"`. They can differ when text sits outside the quotes on the line; no chart with such a line has been tried in the game.
Label: Proven in the game's code
Evidence: the Ghidra decompile C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-28\ch-dynamics-2\out26.c (outside the repo), function FUN_182163170 from line 412. Lines 685 to 689 scan forward to the first double quote (0x22, line 687). Lines 722 to 727 search back from the end of the line for the last one (line 727), and lines 728 to 745 cut the text between them. The text then goes to the starts-with tests at lines 797 and 826 and the section table lookup at line 847. What the game does with a line that has only one quote was not worked out. ADR 0023 lines 65 and 66 state Hydra's side.
Game code (6142): the .chart events reader, RVA `0x2163170`, out26.c lines 685 to 745; no listing is committed.
Hydra: `classify_chart_line` in src/parse/song.cpp lines 1314 to 1316 takes the quotes off; `section_name_of` line 34 then calls `section_display_name`.
Ruling: none found. The user ruled that out26.c counts as the game's code: "Yes, cite them (Recommended)" (2026-11-08, session 77057df2; no card yet). The difference outside the quotes is open: see questions page Q-16.
Found: 2026-10-30, session b02ab3e6 (the reference build); 2026-11-08, session 77057df2 (docs-gaps search).

## Solos

### CR-33 A .chart solo and soloend compare exactly, case included

Rule: `solo` and `soloend` count only when the whole trimmed E text equals them exactly, so `E "solo"` (quoted), `E Solo` and `E solo extra` do nothing.
Label: Proven in the game's code
Evidence: `code/6142/chart_e_handler_213CAF0.asm` lines 169 to 230 call the compare `0x20FE510` with ignore-case 0; decompile lines 58780 to 58830 show that compare checks the lengths and then every 16-bit character, and the handler has no quote-stripping step.
Game code (6142): RVA `0x213CAF0` lines 169 to 230; compare RVA `0x20FE510`, decompile lines 58780 to 58845.
Hydra: `classify_chart_line` in src/parse/song.cpp lines 1301 to 1304.
Ruling: none needed: same rule.
Found: 2026-10-03, session cebf3006; listed 2026-10-30 in session b02ab3e6 (the reference build).

### CR-34 .chart solos are a stack in the game and one flag in Hydra

Rule: in the game `solo` pushes a new solo record onto the track's solo list and `soloend` pops the most recent one, writing its length as end tick minus start tick plus 1; a `soloend` with nothing open is ignored. Hydra keeps one on-off flag.
Label: Proven in the game's code
Evidence: `code/6142/chart_e_handler_213CAF0.asm` lines 231 to 250 (the pop, and the length at lines 247 to 249) and lines 251 to 306 (the push and the list Add); decompile lines 24641 to 24680.
Game code (6142): RVA `0x213CAF0`, lines 231 to 306.
Hydra: `flag_solo_` in src/parse/song.cpp lines 1466, 1706 and 1710.
Ruling: open split: see questions page Q-11.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### CR-35 How the score code reads a solo record

Rule: the score code found so far never reads the chart's solo list by tick. A builder step makes a second list of solo records from the note flags of CR-37: it counts from the first note flagged SoloBegin up to and including the first note flagged SoloEnd. The engine copies that count when a solo starts and pays the bonus from it when the solo ends. If that is the only route, a note on the `soloend` tick is in the solo because the plus-1 length of CR-34 puts it before start plus length, and the score side adds no tick test of its own. That it is the only route is not settled: three more methods that refer to the solo types were found and not read.
Label: Unknown
Evidence: a scan for every reference to the metadata slots that name the chart solo type and the built solo type (byte scan of il2cpp and .text). Outside the chart loaders and the track's own methods, its readers are the flag step `0x20D18F0` (CR-37) and the builder `0x20D1ED0`; the rescan of 2026-11-11 also found 0x215E6E0, 0x215C320 and 0x20E8B60, which were not read. In `code/6142/solo_records_20D1ED0.asm`, lines 99 to 124 walk the track's solo list, lines 165 to 176 start the count at the first note whose SoloBegin test `0x214B630` is true, lines 177 to 184 count each note unless `0x214BCC0` is true for it, and lines 195 to 200 stop after the first note whose SoloEnd test `0x214B7D0` is true; lines 185 to 194, a branch taken only when a byte argument is set, were not read. `code/6142/engine_solo_reader_20F62C0.asm` lines 17 to 23 copy the record's count to engine +0x16C; its direct callers are 0x20F36AB and 0x20F6633, the second behind the SoloBegin test. `code/6142/engine_solo_count_20F7610.asm` lines 25 to 50 pay the bonus when engine +0x164 is at least 50, from hit +0x168 and total +0x16C; its direct callers are 0x20F36C9, 0x20F3A59 and 0x20F6738, the last behind the SoloEnd test. The hit counter +0x168 is raised at 0x20F666E to 0x20F685, and +0x164 is 100 times hit over total. No other reader of the built list was found by the metadata-slot scan.
Game code (6142): the builder RVA `0x20D1ED0`, the solo start RVA `0x20F62C0`, the solo end RVA `0x20F7610`, the note tests RVAs `0x214B630` and `0x214B7D0`; the listings above. Not read: RVAs `0x215E6E0`, `0x215C320` and `0x20E8B60`.
Hydra: `E soloend` is applied after the tick's notes (src/parse/song.cpp line 1710).
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build); the solo readers traced 2026-11-08 in session 77057df2, and the rescan 2026-11-11 in session 428cf079 (the Clone Hero unknowns check).

### CR-36 MIDI note 103 on the drum track is the solo marker

Rule: the .mid drum reader sends note 103 to a sink that appends a (start tick, length) entry to the track's solo list, and the note-list builder later turns that list into solo flags.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2/f2155050.asm lines 225 to 233 (the compare with 0x67, then a call to 0x20CF1B0 (a function the dump does not list as a method)); docs/audit/note-shuffle/step0/open1/midi_sink_20CF1B0.asm lines 18 to 66 (the Add to the list at +0x78); `code/6142/solo_flags_20D18F0.asm` lines 45 to 157 read that list. dump.cs lines 551830 and 551859 give the list's type.
Game code (6142): sink RVA 0x20CF1B0 (a function the dump does not list as a method), builder step RVA `0x20D18F0`, the listings above.
Hydra: `kSoloMarkerPitch` in src/parse/song.cpp line 706; `find_solo_sections` line 74.
Ruling: none needed: same rule.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### CR-37 A solo covers notes from its start up to, not including, start plus length

Rule: a note is in a solo when its tick is at or after the solo's start and strictly before start plus length; the first covered note gets SoloBegin (0x200) and the last SoloEnd (0x400), and a solo that covers no note sets no flag.
Label: Proven in the game's code
Evidence: `code/6142/solo_flags_20D18F0.asm` lines 72 to 74 (the start compare), 82 to 85 (the end compare leaves at start plus length), 91 and 92 (no covered note, no flag), 120 (SoloBegin) and 157 (SoloEnd); flag values from dump.cs lines 548206 and 548207. Lines 34 to 39 limit it to instrument 6 or 9; what the test at `0x214BCC0` checks was not read.
Game code (6142): a static method of class #8265, RVA `0x20D18F0`, called from the builder `0x20D2EA0`.
Hydra: `find_solo_sections` in src/parse/song.cpp line 74, with the comment at lines 981 to 984 that the note-off tick is outside the solo.
Ruling: none needed: same rule.
Found: 2026-10-30, session b02ab3e6 (the reference build).

## The end event and song length

### CR-38 A .mid end event is [end] on the events track, and the first one wins

Rule: the .mid events reader reads the song's end event from the text `[end]` on the events track; the setter stores a tick only while the chart's end field is still empty, so the first end event wins.
Label: Proven in the game's code
Evidence: the literal `[end]` in the events-track function (decompile lines 41771 to 42120, the call to the setter at line 42051); the setter's own decompile, lines 15845 to 15867, stores only when the field at +0x108 is empty.
Game code (6142): events reader RVA `0x21544E0` (docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm); setter RVA `0x212E310`, decompile lines 15845 to 15867.
Hydra: `is_end_event` in src/parse/song.cpp line 46 (`kEndEventBody` line 41); the .mid pass at lines 1174 to 1183 keeps the first.
Ruling: none needed: same rule.
Found: 2026-10-18, session c433b17f; re-read 2026-10-30 in session b02ab3e6 (the reference build).

### CR-39 The .mid [end] test is exact and case-sensitive, after the game trims the text

Rule: the game trims the white space off both ends of a .mid events-track text, then tests it for equality with `[end]`, character by character. So `[END]`, `[end] x` and `[endx]` are not end events, and `[end]` with spaces or tabs round it is. Hydra's .mid pass trims nothing, so the padded form is the one difference.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm line 98 trims the text (the MemoryExtensions Trim call) into registers rbx, r15d and edi (lines 99 to 101). Line 218 loads the `[end]` literal; lines 234 to 243 pass the SequenceEqual metamethod to the compare shim at 0x182C61850 and, when it returns true, jump to line 319 and the end setter of CR-38. The two tests before it, at lines 100 to 126 and 191 to 214, are the starts-with tests for `[section` and `[prc`; those use the StartsWith metamethod (0x3D09CF0) with the shim at 0x182C61990. The decompile has the same order: lines 41924 and 41953 are those two starts-with tests, and line 41981 is the `[end]` equality test (FUN_182c61850 with DAT_183d09b78 and the literal DAT_183d008e8). SequenceEqual on two character spans is an ordinal compare, which is .NET's definition of it; the shim's own body was not read.
Game code (6142): the events reader, RVA `0x21544E0`, the listing above; the end setter, RVA `0x212E310`.
Hydra: `is_end_event` in src/parse/song.cpp line 46 needs the bracket body to equal "end"; the .mid pass at lines 1174 to 1183 first requires the whole text to start with "[" and end with "]", with no trim.
Ruling: open split: see questions page Q-75.
Found: 2026-10-30, session b02ab3e6 (the reference build); the compare re-read in the decompile and the listing by the review of this page on 2026-10-31.

### CR-40 A .chart event drops a leading "[", and then a closing "]" if there is one

Rule: when a .chart event's text (CR-32) starts with "[", the game drops that "[" always, and drops the last character only when it is "]". So `[end]` is cut down to `end`, the same text a bare `end` gives (which text the end-event compare wants is not named in the decompile; see Evidence). Hydra takes the brackets off only when both are there, so a malformed event such as `[end`, with no closing bracket, is `end` to the game and stays `[end` to Hydra. A well-formed chart never shows the difference. The first end event the setter gets wins (CR-38).
Label: Proven in the game's code
Evidence: the Ghidra decompile C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-28\ch-dynamics-2\out26.c (outside the repo), function FUN_182163170. Line 750 tests for a leading "[" (0x5b). Lines 751 to 757 keep the length less one when the last character is "]" (0x5d) and the full length otherwise, then take one off, and line 761 steps past the "[". The call to the end setter `0x212E310` is at line 1054, behind a compare at line 1014 whose literal the decompile does not name, so which text counts as the end event is not shown here. An older listing of this function, hydra-data/ch-fill-evidence-2026-10-11/f2163170.asm, is not committed and was not re-read.
Game code (6142): the .chart events reader, RVA `0x2163170`, out26.c lines 750 to 761 and 1014 to 1054; no listing is committed.
Hydra: the .chart events pass in src/parse/song.cpp lines 1640 and 1641 takes the brackets off only when both are there, then asks `is_end_event` at line 1642.
Ruling: asked as question 3 of the docs-gaps search (2026-11-08, session 77057df2; no card yet), the user chose "Record it, no fix (Recommended)", so the `[end` case is a recorded malformed-chart edge case and Hydra keeps its rule.
Found: 2026-10-18, session c433b17f; 2026-11-08, session 77057df2 (docs-gaps search).

### CR-41 An end event counts only at or past the last note end plus one beat

Rule: an end event counts only when its tick is at least the last note end plus one beat (the double at chart +0xB0, cut to a whole number); a tick exactly on the margin counts, and with no end event the answer is no.
Label: Proven in the game's code
Evidence: `code/6142/end_event_usable_212C7E0.asm` lines 2 to 6 (no end event gives false) and 7 to 13 (end tick greater than or equal to the length); `code/6142/chart_length_212E130.asm` lines 88 to 91 (the flag adds the double at +0xB0, cut to an integer). Decompile lines 14779 to 14791 agree.
Game code (6142): methods of the chart class #8538, RVAs `0x212C7E0` and `0x212E130`, the listings above.
Hydra: `end_event_counts` in src/app/song_length.cpp line 22, whose beat is `SongTiming::working_resolution` cut to a whole number (line 27), the game's +0xB0 (CR-42).
Ruling: D255 item 4: "Copy the margin (Recommended)".
Found: 2026-10-18, session c433b17f; listed 2026-10-30 in session b02ab3e6 (the reference build).

### CR-42 Chart field +0xB0 is the resolution times a factor, not the Offset

Rule: the setter for chart field +0xB0 accepts only a positive finite number and stores it times a float read from another object; the caller passes the .mid division (ticks per beat). The song-end handoff's line 97 ("the chart's offset") is wrong. The float is the song speed ratio, so +0xB0 is the file's resolution times the speed, and +0xB8 keeps the file's own value (page 3, TM-56).
Label: Proven in the game's code
Evidence: decompile lines 14456 to 14483 (FUN_18212c0b0: the guard at line 14465, the stores at lines 14466 to 14470) and its caller at line 42845, read on 2026-10-30. A listing is committed as docs/audit/note-shuffle/step0/open1/song_set_resolution_212C0B0.asm, not re-read for this row. The float's source was traced on 2026-11-23: `code/6142/songentry_load_35C110.asm` lines 141 to 174 pass the speed ratio to the chart-read settings, which `code/6142/chart_settings_ctor_20E73E0.asm` stores at +0x14 (line 32), and `code/6142/song_resolution_set_212C0B0.asm` lines 12 to 19 multiply by that +0x14.
Game code (6142): RVA `0x212C0B0`, decompile lines 14456 to 14483.
Hydra: `SongTiming::working_resolution` in src/core/timing.h line 300 is +0xB0, the file's resolution times `SongTiming::speed_ratio`, and `SongTiming::tick_resolution` (line 292) is +0xB8; `end_event_counts` (src/app/song_length.cpp line 27) cuts the working resolution to a whole number for its margin, as CR-41's flag does.
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build); the float traced 2026-11-23, session 46353d99.

### CR-43 The last note end is the largest gem end over every track

Rule: the chart's length is the largest note start plus sustain over every track in the chart's track list, every instrument and difficulty; the per-track helper reads only the track's gem list, never the instrument, and an empty list gives 0.
Label: Proven in the game's code
Evidence: `code/6142/chart_length_212E130.asm` lines 33 to 63 (the loop over the track list and the running maximum); the helper's decompile, lines 50458 to 50507, reads only the list at +0x60.
Game code (6142): RVA `0x212E130`, the listing above; helper RVA `0x215FED0`, decompile lines 50474 to 50507.
Hydra: `Song::last_note_end_tick` in src/core/song.h line 147, set in src/parse/song.cpp lines 1190 (.mid) and 1947 (.chart).
Ruling: D255 item 3: "All instruments".
Found: 2026-10-18, session c433b17f; listed 2026-10-30 in session b02ab3e6 (the reference build).

### CR-44 Vocal notes never reach the gem lists, so they do not move the song end

Rule: the .mid vocals method writes its pitches and phrase markers only to the chart-wide lyric list and never calls the gem adder, so a vocal note never counts toward the last note end. Hydra counts a vocals track's low notes.
Label: Proven in the game's code
Evidence: decompile lines 43955 to 44246 (FUN_1821579b0): its calls to 0x2152CC0 (a function the dump does not list as a method) are at lines 44067 to 44231, and no direct call to the gem adder (0x20CEC60, the "midi sink" function, listed in docs/audit/note-shuffle/step0/open1/midi_sink_20CEC60.asm) appears in lines 43900 to 44300; that function writes the lyric list (decompile lines 40632 to 40655). Two other callees, the track lookup at 0x20CFD40 (line 44151) and FUN_18212d1e0 (lines 44227 and 44239, not in the decompile), were not read. The .chart vocals and the binary note-stream loaders were not checked. No listing was made.
Game code (6142): .mid vocals method RVA `0x21579B0`, decompile lines 43955 to 44246; lyric adder RVA 0x2152CC0 (a function the dump does not list as a method).
Hydra: `MidiFile::last_note_end_tick` in src/parse/midi.cpp lines 465 to 488 counts vocals pitches 36 to 57 (`kVocalsTrackName` in src/parse/midi.h line 123).
Ruling: open split: see questions page Q-74.
Found: 2026-10-18, session b71e9156 (inferred); read in the decompile 2026-10-30 in session b02ab3e6 (the reference build).

### CR-45 Outside practice mode, the song ends when the clock reaches the game's song length

Rule: GameManager's Update compares songTime (+0x70) with songLength (+0x50), and its end block runs once songTime is at or past songLength. Outside practice mode, that block calls the method that sets isSongOver (+0x91), unless a FadeBehaviour check returns true. In practice mode (GlobalVariables isPracticeEnabled, +0x72) it calls another method, which was not read. Two cases skip the compare: a GameManager byte at +0x158 being 0, and isPaused (+0x90) being set while `CHNetManager` does not report a connection (when connected, a paused game still runs the compare). song.ini `song_length` does not reach songLength offline (CR-48).
Label: Proven in the game's code
Evidence: `code/6142/gm_update_C05A0.asm` lines 178 to 180 compare songTime with songLength and jump past the end block only when songTime is below it, so an equal time runs the end block. Lines 199 to 204 run the FadeBehaviour check and then call `0xBD640`; line 209 calls `0xBF510` in practice mode. `code/6142/gm_song_over_BD640.asm` lines 126 to 128 set isSongOver when it is not already set. Lines 17 and 18 and lines 68 to 79 of the Update listing are the two skips. dump.cs lines 25298 to 25311 name the GameManager fields, and line 55818 names GlobalVariables +0x72 isPracticeEnabled.
Game code (6142): GameManager's Update, RVA `0xC05A0`, `code/6142/gm_update_C05A0.asm` lines 178 to 209; the song-over method RVA `0xBD640`, `code/6142/gm_song_over_BD640.asm` lines 126 to 128; the practice-mode method RVA `0xBF510`, not read.
Hydra: `app::song_length_ms` in src/app/song_length.cpp line 31 is the one end.
Ruling: D255 item 5: "the problem is that we have two different rules. there should one rule that every function uses. we already know how clone hero decides the end of the song, we don't need different places deriving different rules."
Found: 2026-10-18, session b71e9156; 2026-11-08, session 77057df2 (docs-gaps search, and the Update listed in the Clone Hero unknowns check).

### CR-46 The length is Min(end event, Max(audio length, last note end)) plus 1 second

Rule: the load coroutine builds the song length as the earlier of the end event and the later of the audio length and the last note end, plus 1 second; an unusable end event drops the Min. Both chart times are first multiplied by the speed ratio. The game's chart times are the file's times divided by the speed (page 3, TM-56), so this puts them back in the audio file's seconds to be compared with the audio length. The clock then divides the whole length, the 1 second included, by the ratio, so at 50% the song ends twice as late in real time and the pad is 2 seconds. This is one branch of the coroutine; on the other branch it reuses a length it already stored.
Label: Proven in the game's code
Evidence: `code/6142/gm_load_coroutine_D8D80.asm`, re-read 2026-11-08. Line 316 calls `0x210C090`, the ratio getter of page 3's TM-39 (`code/6142/speed_ratio_210C090.asm` lines 10 to 13), on a setting loaded at lines 310 to 312; that this setting is `song_speed` is what `code/6142/speed_ratio_210C090.md` line 3 and `code/6142/settings_load_374D50.md` line 6 say, and it was not re-traced. Lines 324 to 342 take the last note end (`0x212E130`, CR-43), turn it into seconds with `0x212B9C0` and multiply it by the ratio. Line 344 reads the audio length from the audio manager, and line 351 takes System.Math.Max of the two. Line 357 asks whether the end event counts (`0x212C7E0`, CR-41); when it does, lines 363 to 377 turn the end event into seconds times the ratio, and line 380 takes System.Math.Min. Line 388 adds the double 1.0 at RVA 0x3063DC0 in the DLL's read-only data (a constant, not a method; it has no `constants.py` name yet), and line 389 stores the result. Lines 300 to 309 are the other branch: when a byte flag is clear, it loads the stored length and jumps past the build. What sets that flag was not read. Both branches hand the length to the clock constructor, which divides it by the ratio (`code/6142/clock_ctor_16C9A0.asm` line 29, read 2026-11-23, session 46353d99).
Game code (6142): load coroutine RVA `0xD8D80`, `code/6142/gm_load_coroutine_D8D80.asm` lines 300 to 389; first read 2026-10-18 in session b71e9156.
Hydra: `app::song_length_ms` in src/app/song_length.cpp line 31; `kSongEndPadMs` in src/app/song_length.h line 38. Its chart times already divide by the speed (TM-56). It turns the stated audio length into chart time with `chart_ms_of_audio_ms` (src/app/preview_source.h line 127), which divides by `SongTiming::speed_ratio` (song_length.cpp lines 45 and 46), and adds the pad divided by the same ratio last (line 63), so at 50% the pad is 2 seconds.
Ruling: D255 item 2: "Copy the pad (Recommended)".
Found: 2026-10-18, session b71e9156; 2026-11-08, session 77057df2 (docs-gaps search).

### CR-47 The game's audio length is the longest stem; Hydra uses the stated length

Rule: within the game's audio load, audioLength is zeroed, then raised to each loaded stem's length when that is larger. Other writers of audioLength (+0x40) were not searched. Hydra never opens audio, so it puts the song's stated length (song.ini or the .srb field, moved into chart time by the delay or Offset and divided by the song speed, CR-46) in that seat.
Label: Proven in the game's code
Evidence: the load coroutine reads the audio length from BassAudioManager+0x40 (`code/6142/gm_load_coroutine_D8D80.asm` line 343), which dump.cs names audioLength. In `code/6142/audio_load_36B700.asm`, line 57 calls `0x369810`, which zeroes audioLength (`code/6142/audio_reset_369810.asm` lines 75 and 76). Lines 82 and 87 call `0x36DB60`, which was not read. For each loaded stem, lines 404 to 412 get its length in seconds from `0x36E100`, and lines 413 to 416 store it in audioLength only when it is larger. The .sng and encrypted branches rejoin that same step. Other writers of +0x40 were not searched. Hydra's side is src/app/song_length.h lines 48 to 69.
Game code (6142): BassAudioManager's audio load, RVA `0x36B700`, `code/6142/audio_load_36B700.asm` lines 57 to 416; the reset RVA `0x369810`; the stem length RVA `0x36E100`.
Hydra: `stated_length_ms` in src/app/song_length.cpp line 13, used in `app::song_length_ms` line 44.
Ruling: D255 item 1: "Option C (Recommended)". The record says: "It matches the game whenever the stated length is the game's own record of the audio."
Found: 2026-10-18, session b71e9156; the audio load listed 2026-11-08 in session 77057df2 (the Clone Hero unknowns check).

### CR-48 Offline, song.ini song_length and [music_end] are not read on the song-end path

Rule: in a game that is not connected online, the load coroutine always builds the playing length itself, from the chart, the audio and the end event (CR-46), with no song.ini term, and hands it through the clock to GameManager's songLength. The coroutine's own listing and the game's string-literal list show no `song_length` or music_end literal. The coroutine's callees were not all read. When the game is connected, the branch flag at +0x30 is the result of a `CHNetManager` method, 0x146A90, which was not read.
Label: Proven in the game's code
Evidence: `code/6142/gm_load_coroutine_D8D80.asm`, read whole. Lines 101 to 113 set the branch flag at +0x30 to 1 when `0x145FE0` returns false, and to the result of the call at line 112 otherwise, and `code/6142/chnet_connected_145FE0.asm` lines 1 to 3 return true only when CHNetManager's connection state is 2, Connected. When the flag is 1, lines 310 to 389 build the length. Lines 454 to 478 hand it to the clock constructor, which divides it by the speed ratio and stores it at clock +0x10 (`code/6142/clock_ctor_16C9A0.asm` lines 23, 24 and 29), and line 478 copies clock +0x10 to GameManager.songLength (+0x50), the value CR-45's compare reads. Lines 487 to 700 write neither songLength nor a song.ini key. The song.ini [song] reader (`code/6142/ini_song_reader_1EFC60.asm`, CR-26) loads no `song_length` literal; the only listing that holds it is the song.ini writer `0x358850` (CR-27), which writes song_length from `SongEntry` +0x80, or from audioLength when that is missing. music_end and [music_end] are not among the 23,972 string literals of `stringliteral.json` (searched 2026-10-30). The callees on the path were not all read whole.
Game code (6142): the load coroutine RVA `0xD8D80`, `code/6142/gm_load_coroutine_D8D80.asm` lines 101 to 478; the connection test RVA `0x145FE0`; the clock constructor RVA `0x16C9A0`.
Hydra: `is_end_event` in src/parse/song.cpp line 46 accepts only `end`, so Hydra reads no [music_end] either. Hydra does read song.ini `song_length`, as its stand-in for the audio length it never measures (`stated_length_ms` in src/app/song_length.cpp line 13; CR-47). Hydra has no online play.
Ruling: D255 item 1 (as CR-47) covers Hydra's use of the stated length; none needed for [music_end].
Found: 2026-10-18, session b71e9156; string search 2026-10-30 in session b02ab3e6 (the reference build); the coroutine read whole 2026-11-08 in session 77057df2 (the Clone Hero unknowns check).

### CR-49 A .srb's stated length can differ from its audio

Rule: a bundle's stated song length can be longer than its real audio. Pressure states 273,467 ms, and each of its five audio stems ends at 223.130 s, so the stated length is 50.337 s longer than every stem.
Label: Proven in the game's code
Evidence: counted from the game's own file C:\Clone Hero\1.1 june\Clone Hero\Clone Hero_Data\StreamingAssets\songs\Pressure.srb on 2026-11-11 by the kept script docs/audit/2026-11-08-clone-hero-unknowns/W11b/srb_audio_length.py; its output is docs/audit/2026-11-08-clone-hero-unknowns/W11b/srb_audio_length_Pressure.txt. The script reads the stated length by the same field walk as `srb_parse_metadata`, decrypts each audio blob with Hydra's key and IV rule into a scratch folder, and takes each Ogg Opus stem's length as its last granule position less the pre-skip, over 48,000. All five blobs end at granule 10,710,551 with a pre-skip of 312, and the audio section ends exactly at the end of the file, so no blob was missed. The stored length fields of all 30 bundles were re-read on 2026-10-30 and match docs/srb-format.md. A list of every shipped bundle's stated length beside its audio length is docs/audit/2026-11-08-clone-hero-unknowns/main/srb_lengths_all.txt; two bundles, Challenger (feat. Krank_it Critch) and Set You Free, state 0. The user ruled that a fact counted from the game's own files takes this label: "Proven in code" (2026-11-08, docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md line 4).
Game code (6142): not found: data, not code; the count above is from the game's own file.
Hydra: `srb_parse_metadata` in src/parse/srb.cpp line 136 takes the stated value as the song length (line 180), and `app::song_length_ms` in src/app/song_length.cpp lines 37 to 55 uses it. So Hydra's length for Pressure rests on 273,467 ms, while its audio stops at 223,130 ms. The game's song end is Min(end event, Max(audio, last note end)) plus 1 second (CR-46). Whether the game takes that route for a bundle, and where Pressure's last note falls, were not read, so this is not yet a known difference.
Ruling: none needed yet; the open part belongs with D255's one song end.
Found: 2026-09-29, session 7c71d4dd; measured 2026-11-11 in session 428cf079 (the Clone Hero unknowns check).

### CR-50 Timing that makes a beat zero, negative or infinite refuses the chart

Rule: Hydra refuses a chart whose timing makes a measure or beat last zero, negative or infinite time, and ignores a time signature with a top number of 0; what the game does there is unread.
Label: Hydra assumes it, never checked
Evidence: D6 in docs/audit/2026-10-03-fix-decisions.md line 17.
Game code (6142): not found: not read.
Hydra: no owner located for this page; the refusal is D6's error sentence.
Ruling: D6: chose "Refuse with error (Recommended)" (card cebf3006, 21:45 UTC).
Found: 2026-10-03, session cebf3006.

## More on the .srb container

### CR-51 The rest of a .srb's song info, and its audio table

Rule: the song-info stream's first four bytes ("4b4" and a byte of 1) are a 32-bit version number, 20210228 (0x01346234), which matches a constant in the game's reader class. After the eight text fields come 12 one-byte difficulties in this order: band, guitar, guitar co-op, bass, rhythm, drums, Pro Drums, keys, then the four GHL parts (guitar, bass, rhythm, co-op). Then the preview start in ms, an icon name, the playlist track, the album track, the song length in ms and the 16-byte checksum. Then a table: the chart size, the stem count, an image flag byte (2 means album art, 1 a background), the image count and indices, the image sizes, and each stem's offset and size inside the audio section. Each audio entry is a 32-bit stem type, 4 unused bytes, the two IV halves and a 64-bit size. The stem types are 0 guitar, 1 bass, 2 rhythm, 3 vocals, 4 and 5 the two extra vocals, 6 drums, 7 to 10 the four drum stems, 11 keys, 12 song and 13 crowd. In all 30 bundles the stems are stored in alphabetical order. Part of this is proven in the game's code: the reader compares the version with 0x1346234 and stops reading the bundle when it differs, and from the chart size to the audio entry it reads the fields in the order above. Hydra never checks the version, so a bundle with another version is read by Hydra and dropped by the game's reader; the user chose to record this difference only.
Label: Unknown
Evidence: the proven part is `code/6142/srb_reader_2B84A0.asm`. Lines 176 to 182 compare the first 32-bit word of the song info with 0x1346234 and leave when it differs; where that exit leads was not followed. Lines 190 to 198 read the chart size, lines 201 to 207 the stem count (capped by the length of a BassAudioManager array), lines 208 to 214 the flag byte, and lines 217 to 223 the image count (at most 0x10000). Lines 224 to 241 read one 32-bit word when the flag has bit 2 and another when it has bit 1, lines 242 to 270 one 64-bit size per image, and lines 279 to 349 two arrays of one 64-bit value per stem. The second array is each stem's size (lines 418 to 478); the first array's use was not read. `code/6142/srb_stem_ctor_1F2550.asm` reads each 32-byte audio entry: a 32-bit type at byte 0, four unread bytes, 64-bit values at bytes 8 and 16, and a 64-bit size at byte 24 with its top bit cleared. Still unknown: that the first per-stem array holds offsets, that the two flag-gated words are image indices, that flag 2 means album art and 1 a background, the stem-type names, that the values at bytes 8 and 16 are the IV halves, and the fields from the text fields to the checksum, which the unread call `0x1CF270` reads. The rest of this block comes from session 7c71d4dd (2026-09-29), which read the game's reader class and checked the layout on all 30 bundles. Its card (docs/sessions/2026-09-29-7c71d4dd.md) says the session read the game's .srb reader and that the spec's readers and examples ran against the 30 shipped bundles. The field list above, and the match between the version and a constant in the reader class, come from the session's own record, which is not kept. No listing, script or result file from that session is in the repo. docs/srb-format.md describes only the part Hydra reads.
Game code (6142): the .srb reader, RVA `0x2B84A0`, `code/6142/srb_reader_2B84A0.asm` lines 176 to 478; the stem constructor RVA `0x1F2550`; the song-info field reader RVA `0x1CF270`, not read.
Hydra: `srb_parse_metadata` in src/parse/srb.cpp line 136 reads up to the checksum and stops. It never tests the version: src/parse/srb.h line 18 names it only in a comment, and a grep of src/ for 20210228 and 0x1346234 finds nothing else.
Ruling: the version check: the user chose "Record it only" (2026-11-08, session 77057df2; docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md line 18), so Hydra keeps reading every version; questions page Q-80 records it. None of the 30 shipped bundles is known to have another version. None needed for the fields past the checksum, which Hydra does not read.
Found: 2026-09-29, session 7c71d4dd; the reader listed 2026-11-08 in session 77057df2 (the Clone Hero unknowns check).

### CR-52 A stem typed -1 stops a bundle's audio, and the song freezes the game

Rule: the game's stem-type getter throws when the type is below 0, and the audio loader calls it on every stem. Saurian King ships with two stems typed -1. Highlighting it in the song list loads no preview audio, because the audio thread catches the throw. Starting it freezes the game on a black screen, because the song-load coroutine does not catch it. Saurian King cannot be played in 1.1.0.6142.
Label: Proven in a live run
Evidence: session 7c71d4dd (2026-09-29). Its card (docs/sessions/2026-09-29-7c71d4dd.md, the Saurian King roadblock) records the run: no preview sound, a freeze on a black screen in gameplay, two audio pieces labelled -1 that the type reader refuses, the error caught on the audio thread and not in gameplay setup. The session's own record also quoted `Player.log`: after the user highlighted the song it held "Audio resource thread error! System.InvalidOperationException"; after the user started it, the log held "Loading Song: Shadow of Intent - Saurian King" and "Uploading Crash Report" with the same exception. It also gave the code path that session read (the getter at RVA 0x1ED840, the loader at 0x36B700, the coroutine's MoveNext at 0xD8D80 with the call at 0xD915D). That record is not kept, and no listing or log copy is in the repo.
Game code (6142): not found: listings lost; addresses as recorded in session 7c71d4dd, given above as plain text.
Hydra: none: Hydra's Preview names stems by position and never reads the type.
Ruling: none needed.
Found: 2026-09-29, session 7c71d4dd.

## Duplicate charts

### CR-53 The game hides a chart whose notes file matches one it already has

Rule: when two song folders hold byte-identical charts, the game shows only one and lists the other in `badsongs.txt` in the Clone Hero documents folder. Changing one line of the copy's chart (its Name) makes it show.
Label: Proven in a live run
Evidence: session 2a88c065 (2026-09-25). Its card (docs/sessions/2026-09-25-2a88c065.md, the first roadblock) records that a test copy of Lunaris with the same chart name and hash did not show, that the game hid it as a duplicate, and that it showed once the copy's Name line was renamed and the user rescanned. That `badsongs.txt` named it comes from the session's own record, which is not kept. That the test compares the whole notes file (rather than, say, the song id of CR-10) is the session's reading, not shown by code.
Game code (6142): not found: not read.
Hydra: none: Hydra lists every chart it finds.
Ruling: none needed.
Found: 2026-09-25, session 2a88c065.

## Rows waiting on a live run

CR-01, CR-02, CR-08, CR-16, CR-23, CR-24, CR-29, CR-31, CR-34, CR-44. Each is listed on the live-checks page.

## Open splits

CR-29 (Q-15), CR-32 (Q-16), CR-34 (Q-11), CR-39 (Q-75), CR-44 (Q-74). Each is written up on the questions page.
