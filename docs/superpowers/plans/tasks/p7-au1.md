Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task AU1: stem length and damage (findings R7.8, R7.9)

Task id: AU1. Base: main at 6a1bb49. Branch: claude/p7-au1 (worktree `.claude\worktrees\p7-au1`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 1 table, "AU1 Stem length and damage". Decisions: D51 Q19 (a FLAC whose header length is 0 is counted by decoding it once on open) and Q20 (a damaged Opus stem comes back after a scrub to before the damage, like every other format) in `docs/audit/2026-10-04-phase-7-questions.md`. Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### R7.8` and `#### R7.9`.

## Goal

Two questions about a Preview stem get one written answer in the `StemReader` contract, and every reader keeps it. How long is a stem whose file header says 0? As long as it decodes. Does scrubbing back before a decode error bring the stem back? Yes, and `failed()` still remembers the error. Today each reader answers on its own, and two of them answer wrong.

## What the code does today

**R7.8.** `StreamMix::StreamMix` in `src/audio/stream_mix.cpp` takes each reader's `length_frames()` as the stem's length, caps every read at it and builds the mix length from it. `MaReader` in `src/audio/ma_reader.cpp` gets that length from `ma_decoder_get_length_in_pcm_frames`, which for FLAC is the STREAMINFO total as written in the file. RFC 9639 says a total of 0 means "unknown", not empty; any encoder writing to a pipe leaves it 0. Such a stem plays silent for the whole song, and if it is the only stem the Preview has no sound. `VorbisReader` in `src/audio/vorbis_reader.cpp` gets its length from `stb_vorbis_stream_length_in_samples`, which returns 0 when no end page is found or the last granule is -1; no library file hits that today, by the audit's scan. `read_all` in `src/audio/decode.cpp` already reads past the promised length when the decoder gives more, so the whole-file path is unaffected.

**R7.9.** `OpusReader::decode_next` in `src/audio/opus_reader.cpp` sets `failed_` and `at_end_` on a libopus decode error. `OpusReader::seek` returns early while `failed_` is set, before `start_link` would clear `at_end_`, so the stem stays silent after any scrub until the chart is reopened. `MaReader::seek` and `VorbisReader::seek` reset `at_end_` and play again, and `StreamMix::Stem::seek_to` clears its own `ended` flag, so the mixer is ready to replay the stem; only Opus refuses. `MaReader` keeps `failed_` sticky while playing again, so there `failed()` means "ever failed"; `read_all` relies on that meaning to throw on a damaged stream.

## What changes

**The contract.** `src/audio/stem_reader.h` gets two sentences on `StemReader`, beside `length_frames()`, `seek()` and `failed()`. One: `length_frames()` is the number of frames the stem decodes; a header that says 0 (unknown) is replaced on open by a count from decoding the stem once, and every other header keeps today's fast open. Two: `failed()` is true once any decode error has ended the stem early and stays true for the reader's life, while `seek()` to a frame before the damage plays again from there (D51 Q19 and Q20). Every reader is held to both sentences.

**`MaReader`** (owner of R7.8 for FLAC and WAV): when the length miniaudio reports is 0, read the stream through once on open, count the frames, then seek back to 0. Only the zero case pays for this. A WAV cannot say 0 with data (dr_wav clamps to the chunk), so in practice this is the FLAC case; say so in a comment.

**`VorbisReader`**: give it the same zero-length branch, so the contract holds for every reader and not just the one with a fixture. It is not pinned (see "Decided at launch").

**`OpusReader::seek`** (owner of R7.9): drop the early return on `failed_`. `start_link` already resets the decoder state and clears `at_end_`; a seek to exactly `length_` still sets `at_end_`. Keep `failed_` sticky, so `read_all` keeps throwing on a damaged stream and the `decode_audio` error message is unchanged.

`StreamMix` itself does not change: it already trusts `length_frames()`, which now tells the truth.

No scan row: the contract is prose and the tests below guard both readers.

## Owned files (only these may change)

- `src/audio/stem_reader.h`
- `src/audio/opus_reader.cpp`, `src/audio/ma_reader.cpp`, `src/audio/vorbis_reader.cpp`
- `tests/test_stem_reader.cpp`, `tests/test_stream_mix.cpp`
- `tests/test_single_owner.cpp`: add your own scan rows at the end of the file only; the main session joins every task's rows at M7-1.

Not yours: `stream_mix.cpp`/`.h`, `decode.cpp`, `preview_transport.cpp`, `docs/adr/0019-*.md`, `testdata/audio/*`.

## Fixtures

The audio fixtures live in `testdata/audio/`: `sine220.wav`, `sine220.mp3`, `sine220.flac` (stereo, 1 s, 44.1 kHz), `sine220.ogg`, `sine220.opus`. `tests/test_stem_reader.cpp` reads them with `fixture_bytes` and holds the old whole-file decoders as the reference (`old_full_decode`), plus `read_to_end`, `opus_packets` (every packet of the Opus fixture) and `page_opus` (re-pages packets with fresh CRCs). `tests/test_stream_mix.cpp` has `fixture`, `open_bytes`, `make_mix`, `reference`, `read_frames`, `max_diff` and a `FailingReader` wrapper. Do not add a second copy of any of these; add no file under `testdata/`. Both failing-first fixtures are built in test code from the existing bytes:

- **FLAC with its total zeroed.** Copy `sine220.flac` and zero the 36-bit total-samples field of STREAMINFO (RFC 9639: it follows the 20-bit sample rate, 3-bit channels and 5-bit bits-per-sample fields, so it is the low 4 bits of STREAMINFO byte 13 and all of bytes 14 to 17, where STREAMINFO starts 8 bytes into the file). Check the unmodified fixture reads 44100 frames first, so a wrong offset shows up as a changed rate or channel count rather than a quiet pass.
- **Opus with one corrupted packet.** Take `opus_packets` of `sine220.opus`, replace one audio packet near the middle with bytes libopus rejects as an invalid packet (a code-3 TOC byte with a zero frame count does it, RFC 6716 section 3.2.5; a random byte change is not reliable, since libopus decodes most garbage without an error), and `page_opus` the list back into one stream.

## Test cases to add

In `tests/test_stem_reader.cpp`:
1. `StemReader: a FLAC whose header says 0 frames is counted on open`. Pins: the zeroed copy opens; `length_frames()` equals the unmodified fixture's `length_frames()` (44100) and `old_full_decode(original).frames()`; `read_to_end` equals the original's samples exactly; `failed()` is false; after `read_to_end`, `seek(0)` and a second `read_to_end` give the same samples again. Red line before the fix: `length_frames()` reads 0.
2. `StemReader: a damaged Opus stem plays again after a seek to before the damage`. Pins: `read_to_end` of the corrupted stream returns fewer frames than `length_frames()` and `failed()` is true; `seek(0)` then `read_to_end` returns the same number of frames as the first read (the stem plays up to the damage again) and its first 20 ms match the original's within 1e-3 after the existing 960-frame warm-up the Opus seek cases use; `failed()` is still true after the seek. Also pin that `decode_audio` on the corrupted bytes throws (the `read_all` path keeps failing loudly). Red line before the fix: the read after `seek(0)` returns 0 frames.

In `tests/test_stream_mix.cpp`:
3. `StreamMix: a FLAC stem whose header says 0 plays for its real length`. Pins: a mix of the zeroed FLAC copy alone, at 48 kHz stereo with no pad, has `length_frames()` equal to the mix of the unmodified fixture, and `read_frames` to the end equals the unmodified mix within 1e-6. Red line before the fix: `length_frames()` is 0.

Existing cases that must pass unchanged: every `StemReader:` case in `-sf=*stem_reader*`, in particular "straight-through read equals the full decode", "seeking past the end reads nothing; seeking to 0 restarts", "a chained Opus file plays as one stream", "Opus end trimming stops at the last granule" (its 240000 and 240648 pins) and "unrecognized bytes throw a decode_audio error"; and in `-sf=*stream_mix*`, "StreamMix: a stem that stops early goes silent, the others keep playing" (it stays as it is) and "StreamMix: real fixtures mix and seek like the decoded mix".

The only numbers you may type are the ones above: 44100 (the fixture's own frame count, which the unmodified reader already prints), the existing 1e-3 and 1e-6 tolerances and the 960-frame warm-up, and the two RFC byte positions.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*stem_reader*`
- `build-cpp\Release\hydra_tests.exe -sf=*stream_mix*`

Nothing else. Never the full suite.

## Not in this task

- ADR 0019's damaged-stem paragraph (R7.37) is the wave 3 DOC task's; do not edit `docs/adr/`.
- `read_all` in `decode.cpp` and `StreamMix` do not change; both already follow the contract.
- The Preview's "audio end" (`PreviewTransport::load`) follows for free from `length_frames()` and is PV's file.
- An MP3's length (`Mp3Reader`) is a frame count, never 0 with data; leave it.

## Done when

- `stem_reader.h` states both rules; `MaReader` and `VorbisReader` count a zero-length header on open; `OpusReader::seek` has no `failed_` early return and `failed_` stays sticky.
- The three new cases are green with their red lines recorded, and every existing case in both filters passes with no pin edited.
- `git diff --stat 6a1bb49..HEAD` lists only the owned files. No score, path or stored record changes; the results stamp stays "2.1.0".

## Decided at launch (D51 addendum)

The user answered "go with recommended answers to everything". These replace the open questions:

- Vorbis zero length: add the branch for the contract's sake and pin only the FLAC case. No Vorbis fixture.
- If libopus decodes the chosen bad packet without an error, stop and report. Do not search for another byte pattern.

## Commits

One commit, trailers `Task: AU1` plus the preamble's others. Report as the preamble says: the two contract sentences, one line per reader, each case's red line and green result, the diff file list.
