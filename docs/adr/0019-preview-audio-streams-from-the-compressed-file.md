# Preview audio streams from the compressed file

_Supersedes the "decode every stem and sum" sentence of ADR 0004. Extends ADR
0006 (the hand-written Opus decoder stays; it gained a page index, seeks, the
header gain and chained streams)._

The Preview used to unpack every stem into one big buffer before it would play
a note. That breaks on long charts. The blink-182 Discography song.opus holds
8.6 hours of audio in 625 MB, and 8.6 hours unpacked is about 12 GB. Building
that buffer peaked near three times as much, so the machine swapped for
minutes. A Vorbis file over about 3.1 hours didn't play at all, because
stb_vorbis's whole-file decode overflowed its buffer size.

So the Preview now plays straight from the compressed bytes. Each stem gets a
small reader that can unpack a stretch of audio from any point in the file on
demand. The audio device asks for a few milliseconds at a time, and only that
much is unpacked. This is what YARG, Moonscraper and Onyx do.

How it works:

A loose audio file is memory-mapped. That means Windows treats the file as if
it were in memory and reads pages from disk only when they are touched. A stem
inside a `.sng` or `.srb` keeps its bytes in memory, read from the container
once.

One mixer (`StreamMix`) owns every stem's reader. It reads all of them at one
shared position, converts each to 48 kHz stereo, and adds them. Because there
is only one position, the stems can never drift apart. A negative chart offset
is still silence in front of the audio, as before.

A seek moves that one position. Each reader then jumps to the new spot.

Opus: the reader first walks the Ogg page headers once, without decoding, and
records where each page starts and how many samples come before it. A seek
binary-searches that list and decodes forward from a page at least 400 ms
before the target, throwing away the warm-up. The Opus spec (RFC 7845 section
4.6) asks for at least 80 ms. We measured 80 ms on the test fixture and it was
not exact: errors of 0.03 remained 20 ms past the target. With 400 ms the
samples after a seek match a straight read. A seek costs a few milliseconds of
decoding.

Vorbis: stb_vorbis's own pull decoder and seek. It never builds a whole-file
buffer, so the 3.1-hour overflow is gone.

WAV and FLAC: miniaudio's `ma_decoder`, whose seeks are exact.

MP3: Hydra builds its own seek points. miniaudio 0.11.25's MP3 seek ignores
the LAME encoder delay (the silent frames an encoder adds at the start), and
its built-in seek table lands thousands of frames off. The reader walks the
frame headers at open, works out exactly where the decoder will land, and
checks every seek against that.

Resampling: miniaudio 0.11.25's `ma_data_converter_reset` leaves the resampler
filter broken (it zeroes a filter coefficient instead of the filter history).
So at each seek `StreamMix` re-creates the converter in a memory block it
allocated up front. That gives a fresh converter and allocates nothing in the
audio callback.

What stays the same:

A straight read with no seek gives exactly the old samples. The Opus decode is
pinned bit for bit by a test, and that test is unchanged.

After a seek, samples may differ slightly from a straight read. Opus and
Vorbis need a short warm-up, and the converter restarts on its own grid. Tests
compare post-seek audio with a tolerance (1e-3 per sample after the first 20
ms), never bit for bit.

The Opus header's output gain is now applied, as the spec requires. A scan of
the library found 0 of 3,722 Opus files with a non-zero gain, so nothing
audible changes today. Chained Opus files (several streams glued end to end)
now play every link.

Opus end trimming (stopping at the last page's sample count instead of playing
the encoder's padding at the very end) is built but switched off. The Preview
has always played that padding, so playback is unchanged.

The audio callback never throws, never allocates after its first call, and
never reads a file directly; it only touches mapped or in-memory bytes.

A stem with a decode error in the middle plays up to the damage, then goes
silent while the other stems carry on. The old full decode threw on that
error, and the mixer dropped the whole stem. A file that is simply cut short
behaved the same before and after (it plays to where it ends). The user
approved this as the intended design on 2026-10-03. A scan of the whole
library that day found no file that hits this case (details in the audit's
Status section).

Trade-off: Hydra now owns seeking code that opusfile and miniaudio would
otherwise hide: the Opus page index, the MP3 seek points, and the converter
restart. Each one is covered by tests against a straight read. Do not go back
to unpacking a whole stem "because it's simpler"; long charts are real, and
they need the file to stay compressed.
