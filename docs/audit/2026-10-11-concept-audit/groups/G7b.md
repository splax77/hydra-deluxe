# G7b report: the file readers (midi, chart_files, sng, srb, timing, audio_sniff, little_endian, image/decode)

Scout id: G7b (the progress hook never named an id, so I used `G7b` for the status file).
I read all 15 files in full. I also read the callers I needed: `song.cpp` 1930-2074 and 360-400, `analysis.cpp` 100-520 and 660-735, `preview_source.cpp` 1-390, `preview_view.cpp` 110-140 and 425-550, `graph.cpp` 58-91, `winstr.cpp` 200-300, `stem_reader.cpp`, `user_messages.cpp`.

The short answer to the group question: the four readers do share one idea of a chart file, its name and its audio magic. The extension and name rules live in `chart_files.cpp` only. The audio magic rule lives in `audio_sniff.cpp` only. Little-endian reading lives in `little_endian.h` only (sng, srb, preview_source and opus_reader all call it; I grepped for hand-written copies and found none). Big-endian reading has one home, `midi.cpp`. Timing maths has one home, `timing.cpp`, with two outside lookups of the same kind (G7b-5). The real splits are small and are written up below.

No file in the group reads album art. `image/decode.cpp` is used only for authored textures and icons (`preview_renderer.cpp` 166, `icons.cpp` 31). `srb.h` says streams 3 and 4 are JPEG art, "ignored here", and `preview_source.cpp` 223 skips them because they fail the audio sniff. So the question "how does each reader find the album art" has the answer: none does.

## Candidates

### G7b-1. How does Hydra decide what format a chart's notes are?
Looks like: K2, low weight (the three homes agree on every real input; they split on one odd input, and the split is between two containers).
Homes: `chart_format_of`, src/parse/chart_files.cpp 9-15 (a path; the extension, any case);
       `notes_file_format`, chart_files.cpp 17-22 (a file or entry name; exact "notes.mid" or "notes.chart", any case; used by `pick_notes_file` for a loose folder and for a .sng, and by `load_container_srb`, song.cpp 1988);
       the byte sniff in `load_container_srb`, song.cpp 1989-1995 (the notes stream's own bytes; only reached for an .srb whose stated notes name is neither of the two), with a second spelling of the same "MThd" magic in `walk_chunks`, midi.cpp 169-172.
Table, home 1 (extension): "a.mid" or "A.MID" -> Mid; "x.chart" -> Chart; "x.sng" -> Sng; "x.srb" -> Srb; "notes.txt" -> None, which `load_songpath_reading` (song.cpp 2053) throws on and `chart_kind_of` (analysis.cpp 259) calls a folder chart; "" -> None.
Table, home 2 (name): "notes.mid", "Notes.MID" -> Mid; "notes.chart" -> Chart; "song.mid" -> None; "x.chart" -> None; "notes.mid.bak" -> None.
Table, home 3 (bytes, .srb only, name outside the rule): 4 or more bytes starting "MThd" -> Mid reader; 3 bytes "MTh" or any other start -> chart reader; empty -> chart reader.
Table, home 3b (the validity check in midi.cpp): fewer than 14 bytes, or not starting "MThd" -> MidiError "not a MIDI file: missing MThd header".
Differ on: (a) "song.mid" in a loose folder: the extension rule says Mid, the name rule says None, so the scan skips it but a path given straight to the loader would load it (Clone Hero reads only notes.*, so this looks deliberate). (b) A notes entry with an unexpected name: an .sng with only "chart.chart" throws "No chart files found in SNG file." (sng.cpp 160), while an .srb whose metadata names "chart.chart" loads, because the bytes decide. (c) 4 to 13 bytes starting "MThd" in an .srb: home 3 picks the Mid reader, which then throws MidiError; home 3b would have thrown the same. So (c) agrees on the outcome.
Note: I did not read any .srb file, so I do not know if any shipped .srb has a notes name outside the rule; the comment at song.cpp 1984-1987 says it can happen, not that it does. The "MThd" literal is the line-level part (two spellings of one magic). Nothing in the group checks the .sng ("SNGPKG") or .srb magic: both containers are picked by extension only and their prefixes are "never needed" (sng.h 6-7, srb.h 8-9). That is a missing check, not a duplicate, so I list it only for the verifier.

### G7b-2. Is this path an archive (.sng or .srb) or a plain chart file?
Looks like: K3-with-a-rule-inside (one enum, one coarser enum, and one hand test).
Homes: `ChartFormat` {None, Mid, Chart, Sng, Srb}, chart_files.h 21 (the extension);
       `ChartKind` {Folder, Sng, Srb} with `chart_kind_of`, analysis.cpp 229 and 259-265 (the scan, `song_id_read`, `pending_chart_of`, `read_chart_timing_meta`, `scan_folder` line 486);
       `is_container_path`, preview_source.cpp 338-343 (the Preview: Sng or Srb from `chart_format_of`).
Table, `ChartKind`: .sng -> Sng; .srb -> Srb; .mid, .chart, no known extension -> Folder.
Table, `ChartFormat` plus the Preview's test: .sng, .srb -> container; .mid, .chart -> not a container; no known extension -> None, not a container, and the loader then throws.
Differ on: a path with no known extension. The scan calls it a folder chart (`hash_chart_file` would MD5 it); the loader refuses it. Real library rows always carry a known extension, so the inputs cannot meet today.
Note: the rule inside the parallel part is `chart_kind_of`'s default ("anything that is not an archive can only be a folder chart's notes file", analysis.cpp 257-258). That is a fact about the scan's walk, which only lists notes.mid / notes.chart / archives, so I think it is per-module. The verifier can confirm by reading `scan_folder` 470-498.

### G7b-3. Is this entry a playable song stem?
Looks like: K2 (two rules for one question, one by name and one by bytes; the name rule is a hand-kept copy of the formats the byte rule knows).
Homes: `is_audio_filename` and `is_song_stem`, src/app/preview_source.cpp 142-151 (a file or entry name; extension in {.ogg, .opus, .mp3, .wav, .flac} and base name not "preview"; used for a loose folder in `find_loose_audio` 157-171 and for a .sng in `sng_audio_from` 118-130);
       `looks_like_audio` over `audio::sniff_format`, preview_source.cpp 153-155 and src/core/audio_sniff.cpp 49-79 (the bytes; used for .srb chain streams at 223 and .srb encrypted blobs at 273, which have no name; also the decoder's own dispatch, `open_stem_reader`, stem_reader.cpp 13-25).
Table, home 1 (name): "guitar.ogg" -> stem; "SONG.MP3" -> stem; "preview.ogg" -> not a stem (folder and .sng alike); "song.m4a" -> not; "song" with no extension -> not; "drums.OPUS" -> stem; a subfolder named "x.ogg" -> not (folder entries skipped, preview_source.cpp 160).
Table, home 2 (bytes): RIFF....WAVE -> Wav; "fLaC" -> Flac; "ID3" tag then "fLaC" -> Flac; "ID3" tag then anything else, or a tag running past the bytes -> Mp3; FF Ex/Fx sync -> Mp3; "OggS" with "OpusHead" in the first 64 bytes -> OggOpus; "OggS" with "vorbis" there -> OggVorbis; "OggS" with neither -> Unknown; fewer than 2 bytes -> Unknown; JPEG bytes -> Unknown.
Differ on: (a) a ".ogg" entry whose bytes are not Vorbis or Opus (for example FLAC-in-Ogg): the name rule keeps it, the byte rule says Unknown, and `open_stem_reader` then throws AudioDecode "unrecognized audio container". (b) A "preview.wav" inside an .srb chain or blob: the byte rule keeps it (no name to test), the name rule would drop it. Real .srb files carry no names for these, so the "preview" exclusion cannot apply there. (c) A file in a folder or .sng that is audio by bytes but not by extension (for example a ".bin"): dropped by name.
Note: `audio_sniff.h` 4-7 calls the sniff "the one are-these-bytes-playable-audio rule" and the Preview "keeps only the streams it accepts, so what is extracted is exactly what the decoder can open". That holds for .srb. For a folder and a .sng the Preview keeps by name, so it does not hold there. I read how `open_stem_reader` throws; I did not trace where the Preview catches it, so whether (a) loses one stem or the whole song's audio is for the verifier (G6b or G8 hold that code). Adding a format means editing both `is_audio_filename` and `sniff_format`; nothing ties the two lists.

### G7b-4. Do n bytes fit at this position in the buffer?
Looks like: K1, line-level (the homes agree on every input a real file can give), with K3 on what each caller does when the answer is no.
Homes: `fits`, src/parse/sng.cpp 15-20 (size, pos, n; overflow-safe: `pos <= size && n <= size - pos`);
       `read_u32`, `read_string` and `skip` lambdas in `srb_parse_metadata`, srb.cpp 143-163 (`n > meta.size() - pos`; safe only because `pos` moves forward by checked amounts from a start already checked at 136);
       `walk_chunks` and `walk_track`, midi.cpp 169-205 and 222-277 (`pos + 4 > size`, `pos + 8 > size`, `pos + nbytes > end`; adds, no subtraction form; chunk and meta lengths are clamped to `end` by `std::min`);
       `srb_audio_from`, preview_source.cpp 201 and 250-268 (`buf.size() <= kSrbHeaderSize`, `offset + 8 > size`, `cursor + 24 > size`, `blob_size > size - cursor`);
       `range_length`, src/core/winstr.cpp 233-236 (a clamp instead of a test: how many of n bytes exist at an offset).
Table (size, pos, n): n = 0 at pos = size -> sng fits; srb lambdas fit; midi n/a; n = 1 at pos = size -> all say no; pos > size -> sng says no, the srb lambdas would underflow (never reached: pos never passes size), `range_length` says 0 bytes; n near 2^64 -> sng says no; srb says no (`n > size - pos`); midi `pos + n` could wrap but the lengths it adds come from 32-bit fields or the 1,000,000 cap (midi.cpp 105), so it cannot.
Table of the answer being no: sng metadata and file table stop early and keep what they have (sng.cpp 83-97, `walk_file_table`); sng notes entry throws "Truncated SNG file."; an audio entry that does not fit is skipped (preview_source.cpp 125); srb metadata keeps the fields read so far and returns true (srb.cpp 164-183); an srb stream throws; midi stops the walk and keeps the messages so far, except a header under 14 bytes or no MTrk chunk, which throw; the Preview's blob walk breaks and returns the stems found.
Differ on: nothing on the yes/no. The answers differ per caller by design (metadata lenient, payload strict). `little_endian.h` 5-6 says as much: "each caller checks its own first, because each one handles short input its own way".
Note: also the same predicate "the file is no bigger than the 16-byte .srb header" is written twice: `srb_read_metadata` throws on it (srb.cpp 187) and `srb_audio_from` returns no stems on it (preview_source.cpp 201), both against `kSrbHeaderSize`. The constant is shared; the test is spelled twice. Line-level.

### G7b-5. Which tempo section and which meter section is in force at this tick?
Looks like: K1 for the tempo side (agree on every input); K2 for the meter side (the homes split at a tick that sits exactly on a meter change); K3 for the two "by measure" lookups.
Homes: `MsIndex::at`, `tps_at`, `ms_at_tick_f`, src/core/timing.cpp 47-73 (three copies of "last tempo key at or before the tick, section 0 if none"; `tick_at_ms` 75-82 is the inverse by ms);
       `MeasureIndex::section_at`, timing.cpp 109-114 (meter section; `lower_bound`, so a tick exactly on a change belongs to the section BEFORE it; used by `Timecode`, `measures_at_tick_f`, `ms_per_measure_at`);
       `ms_per_measure_at`, timing.cpp 197-204 (asks `section_at(ticks + 1)` to get the section that STARTS at the tick);
       `build_time_box`, src/app/preview_view.cpp 521-535 (its own `upper_bound` over `scene.tempos` for the BPM and over `scene.time_sigs` for the "n/d" label; at-or-before, so a tick exactly on a change takes the NEW entry);
       `plusmeasure`, timing.cpp 165-195 (`bisect_left_lo` over measure counts) and `tick_at_measures_f`, 219-234 (a linear walk by entry position): the two measure-to-section lookups.
Table, tempo at tick T with a change at T: `MsIndex::at` and `tps_at` -> new tempo; `ms_at_tick_f(T)` -> new tempo; preview BPM -> new tempo; before the first key -> opening tempo in all four; negative tick -> opening tempo in all four.
Table, meter at tick T with a change at T: `section_at(T)` -> old section (measure position counted in the old meter); `ms_per_measure_at(T)` -> new section (via T+1); preview time-signature label at T -> new entry; before the first change -> section 0 in all three.
Differ on: a tick exactly on a meter change. The measure number is counted in the old meter, the label next to it shows the new signature, and the measure length uses the new meter. On a barline the measure number comes out the same either way; the split shows only for a change in the middle of a measure.
Note: the header (timing.h 166-178) names the section_at rule as the engine's (D51 call 22) and `ms_per_measure_at` carries a comment about the flip, so inside timing.cpp this is documented. The preview label's boundary rule is not mentioned in either place; I read it only, I did not run it. `MeasureIndex` stores ticks per measure only, not the numerator and denominator, so the label cannot come from it today. `plusmeasure` and `tick_at_measures_f` answer "which meter section holds measure position m" with different rules (bisect on whole measures with an integer partial; walk on fractional entry positions). They agree on whole barlines and the header says the second is display-only and must never feed the engine, so I treat that pair as K3 parallel on purpose (see below); the verifier may want to test a mid-measure change.

## Also seen in G7b (wave A candidates)

### also seen in G7b: G7a-5 (which meter is in force before a chord)
New from my files: the owner `MeasureIndex::section_at` has a second outside reader besides `check_activations`, the Preview's time-signature label, which uses the opposite boundary rule (see G7b-5). Wave A named only the song.cpp walk.

### also seen in G7b: G2-4 (which metadata key means which field, .ini / .sng / .srb)
New from my files: the three readers also differ in how they treat blank space around a value. `read_song_ini_keys` (analysis.cpp 345-385) trims leading and trailing spaces and tabs from every value and lower-cases and trims keys. `sng_read_metadata` (sng.cpp 77-99) and `srb_parse_metadata` (srb.cpp 134-184) return the bytes exactly as stored, and `parse_sng_metadata` and `srb_chart_meta` assign them without trimming. The only later test is `name_or_unknown` (song.cpp 452-455), which checks for an empty string, not blank space. So "name = " in a song.ini reads as empty and shows the unknown-title text, but a .sng whose name value is a single space keeps it. Second, the empty-value edge G2-4 flagged for .srb is erased downstream: `srb_chart_meta` skips an empty name (analysis.cpp 198), but the artist and charter are assigned even when empty, and then all three go through the same `*_or_unknown` fallback (analysis.cpp 728-730), so an empty value reads the same from every source. Third, the delay key: `sng_metadata_delay_ms` (preview_source.cpp 181-187) lets a later "delay" pair replace an earlier one even when the later one does not parse, giving nullopt; `ini_delay_ms` reads the last value in a map and then parses it. Same result. `parse_delay_ms` (preview_source.cpp 112-114) only forwards to `parse_finite_number` and adds no rule.

### also seen in G7b: G7a-3 (what a malformed event does)
Nothing new in the rule itself. One fact from my files that the verifier may want: the reader-level failures (`MidiError`, the SNG and SRB throws) are `KindedError(ChartUnreadable)`, not `ChartFileError` (midi.h 31-34, sng.cpp 158-160, srb.cpp, model.h 42-47), so the parsers' per-op catch of `ChartFileError` cannot swallow them. That is consistent across all three readers.

### also seen in G7b: G2-8 (cannot open file)
Nothing new. The readers in my group take a `ByteSource` and never open files themselves.

## Parallel on purpose

- Full MIDI read against lean MIDI read (`parse_track` midi.cpp 448-470; `MidiFile::lean` 398-440 with `scan_track` and `TrackScan` 324-356). Both share `walk_chunks`, `walk_track`, `meta_message`, `note_message`, `TrackNamePick`. The full read knows "keep every message with a known meta and every note". The lean read knows each track's role (timing track 0, drums track, events track) and which message kinds a role keeps. `TrackScan` knows only counts to size a vector, with a comment that a count may run high but never low. Every entry is a per-mode fact. The one near-rule is "which kinds a role keeps", spelled once in the counting (`kept`, 330-333) and once in the keep (421-437), but both call the same helpers (`timing_meta`, `text_meta`, `filter.keeps`) and the role flags are decided once at 413-416, so I did not write it up.
- Whole-buffer against piecewise SNG reads (`sng_decode_file_into` 107-116 against `sng_read_file` 132-142). Both call `fits` and `unmask`. Home 1 knows "bytes are already in memory"; home 2 knows "bytes come from a `ByteSource`, the mask comes from `head`, a short read means the file shrank". Per-source facts.
- Whole-buffer against piecewise SRB inflate (`srb_inflate_stream` against `srb_inflate_stream_reading`). Both feed `inflate_raw`. Each knows how its next piece of input is supplied. Per-source facts.
- The two growing-read loops (`sng_read_head` sng.cpp 118-130; `srb_inflate_stream_reading` srb.cpp 113-132). Both start at `kFirstPieceRead`, grow with `next_piece_read` and treat a short read as "the source ends here" (`head.size() < asked` against `ended = piece.size() < asked`). The sng loop re-reads from byte 0 each round because the file table must be contiguous from the start; the srb loop continues from where the stream stopped. The shared rule is one comparison; the growth policy has one owner (winstr). I count it as per-source.
- `MsIndex` and `MeasureIndex` as twin classes in timing.h. Each knows its own map, its own accumulated arrays and its own boundary rule; the boundary rule difference is G7b-5.
- `plusmeasure` against `tick_at_measures_f` (scoring surface against display-only). The header says the second must never feed the engine. See G7b-5's note for what the verifier should test.
- Constants that are all 2: `kSpActivationBars` (banked bars an activation needs), `kMeasuresPerSpBar` (measures in a bar), `kStarPowerMultiplier` (the SP bonus) in timing.h 27-30 and 132. Three different game facts that happen to be equal. K0.

## Dropped leads

- "Truncated SRB file." in srb.cpp 187 and srb.h 111: K0. It fires when the whole source is no bigger than the 16-byte header. "Truncated SNG file." (sng.cpp 163) fires when one entry's range runs past the file's end, and "SRB stream is truncated." (srb.cpp 83) fires when a deflate stream ends before its end marker. The word is the same; the three conditions differ and sit in three different containers' readers. The player-facing sentence is chosen by the error kind alone (`kind_sentence`, user_messages.cpp 73-101: ChartUnreadable has one sentence), so the wording is only in the detail line. The header comment says `std::runtime_error`; the code throws `KindedError`, a subclass of it, so the comment is still true.
- `image/decode.cpp`: K0. One stb translation unit with PNG and JPEG only, no sniffing of its own, called from two places.
- `little_endian.h`: one owner, used by every little-endian reader I could find (sng, srb, preview_source, opus_reader, record_store). Big-endian reading exists only in `midi.cpp` (`be32`, `be16`, and the hand-written 24-bit tempo at 127-128); no other home in src. K0.
- `to_multiplier`, `shown_multiplier` in core/timing.cpp: the combo thresholds 10, 20, 30 appear only there (grep over src found no other spelling). The two functions are scoring rules kept in the timing header; that is a placement oddity, not a second home.
- `decode_latin1` (midi.cpp 72-86): the only latin-1 decoder in src. `read_song_ini_keys` is the only BOM stripper. K0.
- `parse_delay_ms` (preview_source.cpp 112-114): a forwarding wrapper over `parse_finite_number`. Line-level, no rule of its own.

## Not read

None.
