# Preview loading audit (2026-10-03)

The stress case is the blink-182 Discography chart. Its song.opus holds 8.6
hours of audio in 625 MB (Explorer shows that as 596 MB). The Preview sits at
48% for minutes on it. Every number below was measured on this machine
(Ryzen 7 9800X3D, 8 cores / 16 threads, 31 GB RAM, 16.7 GB free at the time)
unless it says "reasoned from code".

## The short version

The Preview unpacks the whole song into memory before it will play a single
note. Opus is about 20 times smaller than raw audio, so 625 MB on disk becomes
about 12 GB in memory. The way the buffer is built roughly triples that at the
peak, to about 36 GB. That is more than this machine has, so Windows starts
swapping to disk. The swapping is the "minutes".

The bar sits at 48% because it only moves when a whole audio file finishes.
This chart has two: a 3-second crowd track, then the 8.6-hour song. The crowd
track finishes at once and the bar jumps to 47.5%. Then nothing happens until
the song is done.

The fix that every comparable tool uses is to stop unpacking up front. They
unpack a few seconds ahead of the playhead while it plays. YARG, Moonscraper and
Onyx all do this. Onyx is the tool Hydra's Preview visuals are ported from.
With that change, this chart's Preview would be ready in well under a second.

## Where the time and memory go

| Step | Time | Memory | Thread |
|---|---|---|---|
| Read and parse notes.mid | 0.09 s | small | worker |
| Build the scene (no path) | 0.03 s | small | worker |
| Unpack song.opus, one core, nothing stored | 51 s | none | worker |
| Same, stored the way Hydra does it (1/10 of the file, scaled ×10) | ~58 s + swapping | ~36 GB peak | worker |
| Build the highway timeline (`build_track_state`) | 0.6–0.74 s | small | **UI** |

The 1/10 run made 1.25 GB of audio but reserved 3.63 GB of memory at its peak.
I didn't run the full file, because it would have pushed this machine into
heavy swapping for minutes. 1,542,479 packets × 20 ms = 30,850 s, which matches
song.ini's length. That cross-checks the 12 GB figure.

### Why memory peaks at three times the audio

The Opus decoder appends each packet to a growing vector (`decode.cpp`,
`decode_ogg_opus`). When the vector fills, MSVC allocates one 1.5 times bigger
and copies everything across. For a moment both copies exist, and the final
buffer keeps up to 50% spare room. Then the mixer copies the result again
(`mixer.cpp`, `add_into`). It starts from an empty mix, so it allocates a
second 12 GB buffer to add the song into, while the first one is still alive.
The finished 12 GB buffer then stays in memory for as long as the Preview is
open.

The decoder itself is not slow. libopus is built correctly here: the float
decoder, SIMD on, CPU detection at runtime. It ran at 604 times real time on one
core. That is in line with published numbers (opus-pure measured about 530x on
an M1 Pro). The problem is the 8.6 hours, not the codec.

## What to change, ranked by payoff

### 1. Stream the audio instead of unpacking it all (the real fix)

Keep the compressed file (or just its path). A feeder thread unpacks a few
seconds ahead of the playhead into a small ring buffer: a fixed-size buffer that
gets refilled as it is drained. One mixer reads every stem at the same sample
position and adds them together. Because there is one position for all stems,
nothing can drift out of sync. That is how YARG does it
([BassStemMixer.cs](https://github.com/YARC-Official/YARG/blob/master/Assets/Script/Audio/Bass/BassStemMixer.cs)).
It avoids the sync trouble someone hit with miniaudio sound groups
([miniaudio #917](https://github.com/mackron/miniaudio/discussions/917)).

Seeking uses each format's own seek. For Opus that is opusfile's `op_pcm_seek`
([docs](https://opus-codec.org/docs/opusfile_api-0.7/group__stream__seeking.html)).
It usually needs one or two jumps in the file, plus 80 ms of warm-up decoding.
At 604x, that warm-up costs about 0.13 ms. opusfile is BSD-licensed and sits on
the libogg and libopus Hydra already vendors. For Vorbis, stb_vorbis has its own
pull-and-seek interface. For MP3, WAV and FLAC, miniaudio's `ma_decoder`
already streams and seeks.

Payoff on this chart: audio becomes ready in milliseconds instead of minutes.
Memory drops from about 36 GB to the compressed bytes plus a few seconds of
audio. Playback costs about 1/600 of a core per stem. The whole Preview load
becomes parse + scene + timeline, about 0.1 s on the worker plus the UI-thread
timeline in item 4.

Cost: this is the biggest change. `audio::Playhead` (pure and unit-tested)
becomes a streaming source. The Transport's seek, tick-stepping and scrubbing
must work with a seek that finishes a moment later; the callback plays silence
until the new spot is ready. A seek through opusfile doesn't produce
bit-identical samples to a straight-through decode, but the difference is
smaller than Opus's own compression loss. Exact-sample tests would need a
tolerance. Nothing the user sees should change except the loading time.

Prior art: YARG and Moonscraper stream with BASS. Onyx streams each stem in
small chunks through OpenAL
([Audio.hs](https://github.com/mtolly/onyx/blob/master/haskell/packages/onyx-lib-game/src/Onyx/Game/Audio.hs)).
Editor on Fire keeps only the compressed OGG in memory and decodes while
playing. The miniaudio manual says to stream large sounds rather than hold them
in memory.

### 2. Make closing or switching charts cancel immediately (a bug, needed either way)

The worker checks for "cancel" only between stems
(`preview_load_job.cpp`, the progress callback). Closing the details window, or
pressing `<` / `>`, makes the UI thread wait for the worker
(`job_base.h`, `shutdown()` joins). Close the window during the song stem and
the whole app freezes until that stem finishes. The fix is a cancel check
inside the decode loop every few thousand packets. Streaming would remove the
long decode, but a cancellable loop is still the right shape.

### 3. Fix the progress bar

The bar breaks the published rules in four ways. It doesn't move while work is
happening, which breaks Microsoft's "the progress bar must advance if progress
is being made"
([Win32 UX guide](https://learn.microsoft.com/en-us/windows/win32/uxguide/progress-bars))
and Apple's "keep indicators moving"
([HIG](https://developer.apple.com/design/human-interface-guidelines/progress-indicators)).
It splits the decode slice evenly per file, not by size. Microsoft says to size
each step by how long it takes. It shoots to 48% and then sits there. Microsoft
names this exact pattern, and Apple calls it potentially "deceptive". Its label,
"2/2", has no real units. GNOME's HIG asks for units ("12.1 of 30 MB") and a
time left. Harrison et al. (UIST 2007) found that pauses make the whole wait
feel longer, and late pauses feel worst.

The fix: weight each stem by its compressed size. Report progress inside a stem
from bytes consumed. Label it in real units: "Decoding song: 1:23:45 of
8:34:07". Show time remaining only after the rate settles, in minutes rather
than seconds. Dear ImGui (1.90.6 and later) has an indeterminate mode: pass a
negative fraction for any step whose size is unknown. Guard against dividing by
a zero total, because a NaN fraction triggers a huge allocation in ImGui
([#7451](https://github.com/ocornut/imgui/issues/7451)). With streaming, the
decode step disappears and the bar only covers the sub-second steps, but they
should still be weighted honestly.

### 4. The highway timeline freezes the UI for 0.6–0.74 s, on load and on every path click

`build_track_state` (`src/render/track_state.cpp`) runs on the UI thread inside
`PreviewRenderer::set_scene`. It makes one entry for each moment anything
happens: 214,266 on this chart. For each entry, `toggle_at` loops over every SP
phrase, solo, fill and activation window. `synthesize` also rebuilds the
fill-lane list from scratch for each entry. That is about 214,000 × 4,000
comparisons. With the spans removed, the same build takes 88 ms, so the
repeated scans are nearly all the cost.

The fix: sort every span's start and end once, then walk them alongside the
moments, which are already in time order. That turns it into one pass. The
remaining 88 ms comes from a `std::map` with one heap allocation per moment.
The notes and beats are already sorted, so merging them straight into a vector
removes that. The function never touches the GPU, so the load job can build it
and hand it over with the scene. A path click should only rebuild what depends
on the path: activations, fill states, score, the SP meter, and the timeline's
fill / active-SP / lane fields. Today a click rebuilds the whole scene plus the
whole timeline.

### 5. Smaller algorithm fixes on the same path

The score replay (`core/replay.cpp`) checks every activation window for every
chord. With 2,000 made-up windows it went from 70 ms to 243–345 ms. A "first
window still open" index moved forward as chords pass makes it one pass. Each
chord also builds strings and vectors that the Preview throws away; it keeps
four numbers per chord.

`next_act_boxes()` runs every frame and is quadratic in activations, meaning
the work grows with the square of their count. `scrub_marks()` runs twice per
frame. Both only change when the scene changes, so they can be cached. The time
box scans about 5,700 tempo changes per frame where a binary search would do.
That is cheap today. The MIDI reader stores two `std::string`s on each of
758,000 messages, with no `reserve`. The `.chart` reader copies the file into a
vector of lines and runs a regex on every event, so a `.chart` discography
would parse much slower than this MIDI one. These are all reasoned from code
except the replay numbers.

## Where parallel computing helps (and where it doesn't)

The bottleneck is memory, not CPU. Spreading the work over more cores doesn't
fix a buffer that is three times bigger than RAM. Streaming removes the need for
most parallel work, because a few seconds at a time costs almost nothing.

If the full decode were kept, parallel work would help a lot. Splitting
song.opus across cores, each starting from its own point in the file, measured:

| Threads | Time | Speed |
|---|---|---|
| 1 | 51.1 s | 604x real time |
| 4 | 14.9 s | 2,078x |
| 8 | 8.1 s | 3,796x |
| 16 | 6.9 s | 4,506x |

It scales almost perfectly up to the 8 real cores, then gains little from the
extra hyper-threads. There's a catch, though. Each worker has to warm up the
decoder before its chunk, because Opus predicts each frame from the one before
it (RFC 6716 §4.3.2.1). The spec's standard warm-up is 80 ms (RFC 7845 §4.6).
With exactly 80 ms, the first samples of a chunk still differed from a
straight-through decode by up to −21 dBFS. That is loud enough to risk a click
at each seam. A split decode would need a longer warm-up or a short crossfade.
I found no player or library that decodes one Opus stream in parallel. fre:ac's
chunked parallel encoder is the nearest example.

Two other places parallelism fits. Separate stems can decode on separate
threads; that is trivial and has no seam problem. The chart parse and scene
build could run beside the audio instead of before it, which hides about 0.1 s.
For the timeline in item 4, a better algorithm beats extra threads.

## Correctness bugs found along the way

These aren't speed problems, but they turned up during the audit and are
verified in the code.

A Vorbis file longer than about 3.1 hours (stereo, 48 kHz) plays silent.
`stb_vorbis_decode_memory` keeps its buffer size in a 32-bit `int` that doubles
each time the buffer fills (`third_party/stb/stb_vorbis.c`, line 5410). Past
2^29 frames it overflows and the resize fails. Hydra's mixer catches the error
and quietly skips the stem. Streaming with stb_vorbis's pull interface avoids
this call entirely.

A file over 2 GB reads back empty. `read_file_bytes` (`src/core/winstr.cpp:47`)
stores the size from `ftell` in a `long`, which is 32 bits on Windows.
`_ftelli64` or a Win32 file-size call fixes it.

The hand-written Opus decoder reads only the channel count and the pre-skip
from the Opus header. It ignores the header's output gain, which opusfile
applies by default. It also follows only the first stream of a chained file,
meaning several Opus files glued end to end. Most chart audio has zero gain and
no chaining, so this rarely matters. Switching to opusfile fixes both.

A `.sng` or `.srb` chart is read from disk twice: once for the notes, once for
the audio (`parse/song.cpp` and `app/preview_source.cpp`). A `.sng` with 625 MB
of audio inside would cost 1.25 GB of reading. Read it once and pass the buffer
to both.

## Sources

Progress UI: [NN/g response times](https://www.nngroup.com/articles/response-times-3-important-limits/),
[NN/g progress indicators](https://www.nngroup.com/articles/progress-indicators/),
[Microsoft Win32 progress bars](https://learn.microsoft.com/en-us/windows/win32/uxguide/progress-bars),
[Apple HIG](https://developer.apple.com/design/human-interface-guidelines/progress-indicators),
[GNOME HIG](https://developer.gnome.org/hig/patterns/feedback/progress-bars.html),
[Harrison et al. 2007](https://chrisharrison.net/projects/progressbars/ProgBarHarrison.pdf),
[ImGui 1.90.6 indeterminate bar](https://github.com/ocornut/imgui/releases/tag/v1.90.6).

Audio: [miniaudio manual](https://miniaud.io/docs/manual/index.html),
[miniaudio libopus extra](https://github.com/mackron/miniaudio/tree/master/extras/decoders),
[opusfile seeking](https://opus-codec.org/docs/opusfile_api-0.7/group__stream__seeking.html),
[RFC 7845](https://datatracker.ietf.org/doc/html/rfc7845),
[RFC 6716](https://www.rfc-editor.org/rfc/rfc6716),
[YARG audio](https://github.com/YARC-Official/YARG/blob/master/Assets/Script/Audio/Bass/BassAudioManager.cs),
[Moonscraper audio](https://github.com/FireFox2000000/Moonscraper-Chart-Editor/blob/master/Moonscraper%20Chart%20Editor/Assets/Scripts/Engine/Audio/AudioManager/AudioManager.cs),
[Onyx audio](https://github.com/mtolly/onyx/blob/master/haskell/packages/onyx-lib-game/src/Onyx/Game/Audio.hs),
[opus-pure speed](https://github.com/stephenberry/opus-pure),
[fre:ac SuperFast](https://www.freac.org/developer-blog-mainmenu-9/14-freac/257-introducing-superfast-conversions).

Timing programs: the audio bench and the parse/scene bench live in this
session's scratchpad (`bench\preview_bench.cpp`, `pbench\bench.cpp`). They link
against the 9/29 `build-ship` Release libraries.
