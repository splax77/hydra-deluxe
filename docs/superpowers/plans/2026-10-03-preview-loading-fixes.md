# Preview Loading Fixes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers-extended-cc:subagent-driven-development (recommended) or superpowers-extended-cc:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the Preview open any chart in about a second and with little memory, by playing audio straight from the compressed file instead of unpacking it all first, and fix everything else the 2026-10-03 audit found on the load path.

**Architecture:** Each audio stem gets a small reader that can unpack a stretch of audio from any point in the file on demand. One mixer reads every stem at the same position and adds them, so the stems can never drift apart. The playhead asks the mixer for the next few milliseconds whenever the audio device wants more. The rest of the load (chart parse, scene, highway timeline) moves fully onto the worker thread, and the slow parts get linear algorithms.

**Tech Stack:** C++17, MSVC, CMake, libogg + libopus (hand-driven, per ADR 0006), stb_vorbis pull API, miniaudio `ma_decoder` and `ma_data_converter`, doctest, Dear ImGui Test Engine (`hydra_uitest`).

**Spec:** `docs/handoffs/2026-10-03-preview-loading-audit.md`

## Global Constraints

- Nothing the user sees or hears may change except load time, memory use, and the loading bar. Same audio, same sync, same offsets, same highway, same scores. A behavior change found during a task is a blocking question for the user, never a silent fix.
- Keep Hydra's hand-written Opus decoding (ADR 0006). Do not add opusfile.
- The chart sync rule stays exactly as today: `audio_ms = chart_ms + audio_offset_ms`, with a negative offset turned into silence in front of the audio (today's `pad_front_ms`).
- A straight-through read of an Opus or Vorbis stem (no seek) must give the same samples as today's full decode. The existing bit-for-bit Opus test (`tests/test_audio_decode.cpp`, "Ogg-Opus output is pinned bit for bit") must keep passing unchanged.
- After a seek, samples may differ slightly from a straight-through decode (Opus and Vorbis warm-up). Tests compare post-seek audio with a tolerance of 1e-3 absolute per sample after the first 20 ms, never bit for bit.
- The audio device callback must never throw, never allocate after the first call, and never read from disk directly. Loose audio files are memory-mapped; container stems (.sng/.srb) keep their bytes in memory.
- Engine results (replay, scores, SP meter, paths) must be identical before and after every algorithm change. Each such task proves it with an old-versus-new equality test.
- Every task ends green: `.\build_cpp.ps1 -Target hydra_tests` then `.\build-cpp\Release\hydra_tests.exe`, and for UI-touching tasks also `.\build_cpp.ps1 -Target hydra_uitest` then `.\build-cpp\Release\hydra_uitest.exe --all --jobs 4`.
- Work happens in git worktrees branched from `main` (other sessions leave edits in the main checkout). Stage files by name, never `git add -A`.
- Our code builds with `/W4 /WX`; no new warnings.

**User decisions (already made):**
- "make all of those fixes" (2026-10-03): streaming audio, cancel inside the load, honest progress bar, linear timeline off the UI thread with path-only rebuilds, the smaller algorithm fixes, and the correctness bugs from the audit.
- Opus header gain gets applied as the Opus spec requires. No ask needed: a scan of the library found 0 of 3,722 Opus files with a non-zero gain, so nothing audible changes today.

---

## How the pieces fit

Today the load job unpacks every stem into one giant buffer, then playback copies out of it. After this plan:

1. `resolve_preview_source` finds the stems as today. A loose file becomes a memory-mapped view instead of a path. A .sng/.srb stem keeps its bytes, read from the container once.
2. `open_stem_reader` turns each stem into a `StemReader`. For Opus it builds a page index: one fast pass over the page headers that records where each page starts and how many samples come before it. For Vorbis it opens stb_vorbis's pull decoder. For WAV/MP3/FLAC it opens a miniaudio `ma_decoder`.
3. `StreamMix` owns the readers, converts each one to 48 kHz stereo as it reads, adds them, and handles the front silence for a negative offset.
4. `Playhead` now pulls from a `MixSource` instead of indexing a buffer. A seek just records the new position; the next pull does the real seek. Tests still build a `Playhead` from a plain buffer through `BufferSource`.
5. The load job reports progress in bytes with real units and checks for cancel inside every long loop. It also builds the highway timeline itself, so the UI thread only uploads it.

## File map

| File | Change | Responsibility |
|---|---|---|
| `src/core/winstr.cpp/.h` | modify | `read_file_bytes` handles files over 2 GB; new `file_size_bytes` |
| `src/audio/mapped_file.h/.cpp` | create | read-only memory map of a file, with size |
| `src/audio/stem_reader.h/.cpp` | create | `StemReader` interface + `open_stem_reader` dispatcher |
| `src/audio/opus_reader.cpp` | create | Opus page index, seek, gain, chained streams |
| `src/audio/vorbis_reader.cpp` | create | stb_vorbis pull decoder (removes the 3-hour overflow) |
| `src/audio/ma_reader.cpp` | create | WAV/MP3/FLAC through `ma_decoder` (MP3 seek table) |
| `src/audio/decode.cpp/.h` | modify | `decode_audio` becomes "open a reader, read it all" |
| `src/audio/stream_mix.h/.cpp` | create | `MixSource`, `BufferSource`, `StreamMix` |
| `src/audio/mixer.cpp/.h` | modify | keep `mix_stems` (test reference); drop `decode_and_mix` once unused |
| `src/audio/player.cpp/.h` | modify | `Playhead` pulls from a `MixSource` |
| `src/app/preview_source.cpp/.h` | modify | loose stems carry a path only; container read once |
| `src/parse/song.cpp`, `src/parse/sng.cpp` | modify | note loaders accept bytes already read |
| `src/render/track_state.cpp/.h` | modify | linear sweep build, vector merge |
| `src/render/preview_renderer.cpp/.h` | modify | `set_scene` accepts a prebuilt `TrackState` |
| `src/ui/preview_load_job.cpp/.h` | modify | parallel open, byte progress, cancel, timeline in job |
| `src/ui/preview_controller.cpp/.h` | modify | hands the job's timeline to the renderer; caches |
| `src/ui/preview_tab.cpp` | modify | new bar label and time left; per-frame caching |
| `src/app/preview_view.cpp/.h` | modify | path-only overlay rebuild; linear scans |
| `src/core/replay.cpp/.h` | modify | active-window list; scores-only mode |
| `src/parse/midi.cpp/.h` | modify | smaller messages, reserve |
| `CMakeLists.txt` | modify | new audio sources; new tests |
| `docs/adr/0019-preview-audio-streams-from-the-compressed-file.md` | create | the decision |
| `docs/adr/0006-...md`, `docs/adr/0004-...md` | modify | point to 0019 |

## Waves (what can run in parallel)

Tasks in one wave touch different files and can run in separate worktrees at the same time. Merge and build the whole wave before starting the next.

- Wave 1: Task 1, Task 2, Task 6, Task 9, Task 11
- Wave 2: Task 3, Task 7, Task 12
- Wave 3: Task 4
- Wave 4: Task 8
- Wave 5: Task 10
- Wave 6: Task 13

---

### Task 1: Files over 2 GB read correctly

**Goal:** `read_file_bytes` returns the whole file for files over 2 GB, and a new `file_size_bytes` gives a file's size as 64 bits without reading it.

The bug: `ftell` returns a 32-bit `long` on Windows, which fails past 2 GB, so the function returns an empty buffer. The fix is `_ftelli64` / `_fseeki64`, plus a size helper built on `GetFileAttributesExW` that later tasks use for progress totals.

**Files:**
- Modify: `src/core/winstr.cpp:43-53`, `src/core/winstr.h:27`
- Test: `tests/test_winstr.cpp` (create if absent; add to the `hydra_tests` source list in `CMakeLists.txt`)

**Acceptance Criteria:**
- [ ] `file_size_bytes(path)` returns the exact size of a 2.5 GB sparse file (created in the test with `FSCTL_SET_SPARSE` + `SetEndOfFile`, so it uses no disk space).
- [ ] `read_file_bytes` uses `_fseeki64`/`_ftelli64` and a `size_t` size; it throws `std::runtime_error` when the size can't be read, instead of returning empty.
- [ ] Existing tests pass.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*file_size*"` → the sparse-file test passes.

**Steps:**

- [ ] **Step 1: Write the failing test**

```cpp
#include "doctest.h"
#include <windows.h>
#include <winioctl.h>
#include "core/winstr.h"

TEST_CASE("file_size_bytes reports sizes past 2 GB") {
    wchar_t dir[MAX_PATH];
    GetTempPathW(MAX_PATH, dir);
    std::wstring path = std::wstring(dir) + L"hydra_sparse_test.bin";
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    DWORD ret = 0;
    DeviceIoControl(h, FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0, &ret, nullptr);
    LARGE_INTEGER size;
    size.QuadPart = 2'500'000'000LL;
    SetFilePointerEx(h, size, nullptr, FILE_BEGIN);
    SetEndOfFile(h);
    CloseHandle(h);
    CHECK(hydra::file_size_bytes(hydra::wide_to_utf8(path)) == 2'500'000'000ULL);
    DeleteFileW(path.c_str());
}
```

- [ ] **Step 2: Run it and see it fail** (`file_size_bytes` doesn't exist yet).

- [ ] **Step 3: Implement**

```cpp
// winstr.h
// The size of a file in bytes, read from the file system (no open, no read).
// Throws std::runtime_error when the file can't be found.
uint64_t file_size_bytes(const std::string& utf8_path);

// winstr.cpp
uint64_t file_size_bytes(const std::string& utf8_path) {
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (!GetFileAttributesExW(utf8_to_wide(utf8_path).c_str(), GetFileExInfoStandard, &fa))
        throw std::runtime_error("cannot read file size: " + utf8_path);
    return (static_cast<uint64_t>(fa.nFileSizeHigh) << 32) | fa.nFileSizeLow;
}

std::vector<uint8_t> read_file_bytes(const std::string& utf8_path) {
    std::FILE* f = fopen_utf8(utf8_path, L"rb");
    if (f == nullptr) throw std::runtime_error("cannot open file: " + utf8_path);
    _fseeki64(f, 0, SEEK_END);
    const long long size = _ftelli64(f);
    _fseeki64(f, 0, SEEK_SET);
    if (size < 0) {
        std::fclose(f);
        throw std::runtime_error("cannot read file size: " + utf8_path);
    }
    std::vector<uint8_t> buf(static_cast<size_t>(size));
    if (size > 0) buf.resize(std::fread(buf.data(), 1, buf.size(), f));
    std::fclose(f);
    return buf;
}
```

- [ ] **Step 4: Run the test and the whole suite; both pass.**

- [ ] **Step 5: Commit** `src/core/winstr.cpp src/core/winstr.h tests/test_winstr.cpp CMakeLists.txt` — "Read files over 2 GB; add file_size_bytes".

---

### Task 2: Stem readers that unpack audio on demand

**Goal:** A `StemReader` for each format that reports its length up front, reads the next N frames, and seeks to any frame, plus `decode_audio` rebuilt on top of it.

This is the heart of the fix. Today's decoders unpack a whole file in one call. A reader unpacks only what it's asked for. Think of it as a bookmark in the file, plus a way to move the bookmark.

**Files:**
- Create: `src/audio/mapped_file.h`, `src/audio/mapped_file.cpp`, `src/audio/stem_reader.h`, `src/audio/stem_reader.cpp`, `src/audio/opus_reader.cpp`, `src/audio/vorbis_reader.cpp`, `src/audio/ma_reader.cpp`
- Modify: `src/audio/decode.cpp` (decoder bodies move into the readers; `sniff_format` stays), `src/audio/decode.h`, `CMakeLists.txt` (add sources to `hydra_audio`; set `STB_VORBIS_NO_PUSHDATA_API` only if already set — do not change stb flags otherwise)
- Test: `tests/test_stem_reader.cpp` (create, add to `hydra_tests`), existing `tests/test_audio_decode.cpp` unchanged

**Acceptance Criteria:**
- [ ] `open_stem_reader` returns a reader for each of WAV, MP3, FLAC, Ogg Vorbis and Ogg Opus, from bytes in memory or from a memory-mapped file.
- [ ] For every fixture in `testdata/audio/`: `length_frames()` equals the frame count today's full decode produces, and reading start to end equals today's full decode bit for bit.
- [ ] Seeking to frame F then reading N frames matches the full decode's frames [F, F+N) within 1e-3 absolute, ignoring the first 20 ms after the seek, for F at 0, 1, the middle, and length − 100.
- [ ] Seeking past the end reads 0 frames; seeking to 0 after reading to the end restarts cleanly.
- [ ] The Opus reader applies the OpusHead output gain (bytes 16–17, signed Q7.8 dB): scale = 10^(gain/(20·256)). A test builds a copy of the fixture with gain +6.02 dB (1541 in Q7.8) and checks the samples are ×2.0 within 1e-3.
- [ ] The Opus reader plays a chained file (two Ogg streams back to back, different serial numbers) as one continuous stream, each link with its own pre-skip. A test builds one from the fixture with libogg and checks length = 2× the fixture's.
- [ ] Opus length comes from the last page's granule position minus pre-skip, and reading stops exactly there (end trimming, RFC 7845 §4.4).
- [ ] The Opus page index build reports progress through a callback `(bytes_done, bytes_total)` at least every 4 MB, and stops by throwing `ui`-agnostic `audio::OpenCancelled` when the callback returns false.
- [ ] The Vorbis reader uses `stb_vorbis_open_memory` + `stb_vorbis_get_samples_float_interleaved` + `stb_vorbis_seek` + `stb_vorbis_stream_length_in_samples`. `stb_vorbis_decode_memory` is no longer called anywhere.
- [ ] The miniaudio reader sets `ma_decoder_config.seekPointCount = 1024` so long MP3s seek fast.
- [ ] Timing on the real file: opening `C:\Clone Hero\songs\Misc Downloads\blink-182 - Discography\song.opus` (index build) takes under 0.5 s, and seeking to 4 hours then reading 1 s takes under 10 ms. Record both numbers in the commit message.
- [ ] `decode_audio(bytes)` is reimplemented as: open a reader, reserve exactly `length_frames × channels`, read to the end. All existing decode tests pass unchanged.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*StemReader*,*decode_audio*,*sniff*"` → all pass.

**Steps:**

- [ ] **Step 1: Write the interface** (`src/audio/stem_reader.h`)

```cpp
// A seekable, pull-style decoder for one stem: it unpacks only the frames it
// is asked for, so an 8-hour stem costs no more to open than a short one.
// Frames are interleaved float in [-1, 1] at the stem's own rate and channel
// count. Not thread-safe: one thread drives a reader (the audio device thread,
// through StreamMix and the Transport's lock).
class StemReader {
public:
    virtual ~StemReader() = default;
    virtual int channels() const = 0;
    virtual int sample_rate() const = 0;
    virtual int64_t length_frames() const = 0;
    // Reads up to `frames` frames into `out`; returns frames read (0 at the
    // end). Never throws: a decode error ends the stem early.
    virtual int64_t read(float* out, int64_t frames) = 0;
    // Moves to `frame`, clamped to [0, length_frames()]. Never throws.
    virtual void seek(int64_t frame) = 0;
};

// The compressed bytes a reader decodes from. Either owned (a container stem's
// extracted bytes) or a read-only map of a loose file; the reader keeps it alive.
struct StemBytes {
    std::vector<uint8_t> owned;
    std::shared_ptr<const MappedFile> mapped;
    const uint8_t* data() const;
    size_t size() const;
};

struct OpenCancelled : std::exception {
    const char* what() const noexcept override { return "cancelled"; }
};
// bytes_done / bytes_total; return false to cancel (open throws OpenCancelled).
using OpenProgress = std::function<bool(uint64_t bytes_done, uint64_t bytes_total)>;

// Throws std::runtime_error ("decode_audio: ...") for an unrecognized or
// unreadable stream, so user_messages.cpp keeps classifying it as an audio
// decode error.
std::unique_ptr<StemReader> open_stem_reader(StemBytes bytes,
                                             const OpenProgress& progress = nullptr);
std::unique_ptr<StemReader> open_stem_reader(const app::PreviewAudioStem& stem,
                                             const OpenProgress& progress = nullptr);
```

`MappedFile` (`src/audio/mapped_file.h`): `static std::shared_ptr<const MappedFile> open(const std::string& utf8_path)` using `CreateFileW` (share read), `CreateFileMappingW(PAGE_READONLY)`, `MapViewOfFile(FILE_MAP_READ)`; exposes `data()` and `size()`; unmaps in the destructor. A zero-length file maps as empty without calling `CreateFileMappingW`.

- [ ] **Step 2: Write the failing tests** (`tests/test_stem_reader.cpp`). Use the fixtures and helpers already in `tests/test_audio_decode.cpp` (sine220 in each format). For each fixture:

```cpp
TEST_CASE("StemReader: straight-through read equals the full decode") {
    for (const char* name : {"sine220.wav", "sine220.mp3", "sine220.flac",
                             "sine220.ogg", "sine220.opus"}) {
        std::vector<uint8_t> bytes = fixture_bytes(name);
        audio::DecodedAudio full = old_full_decode(bytes);  // copy of today's decoder, test-only
        auto r = audio::open_stem_reader(audio::StemBytes{bytes, nullptr});
        CHECK(r->length_frames() == full.frames());
        std::vector<float> got(static_cast<size_t>(r->length_frames()) * r->channels());
        int64_t n = 0;
        while (int64_t k = r->read(got.data() + n * r->channels(), 997)) n += k;
        CHECK(n == full.frames());
        CHECK(got == full.samples);
    }
}

TEST_CASE("StemReader: a seek lands on the same audio within tolerance") {
    // for each fixture and F in {0, 1, len/2, len-100}: seek(F), read 4800 frames,
    // compare to full.samples[F*ch ..] skipping the first 20 ms (rate/50 frames)
    // with |a-b| <= 1e-3
}

TEST_CASE("StemReader: Opus header gain is applied") { /* patch bytes 16-17 of the
   OpusHead packet in a re-paged copy (libogg recomputes CRC); expect x2.0 */ }

TEST_CASE("StemReader: a chained Opus file plays as one stream") { /* re-page the
   fixture's packets twice with serials 1 and 2 via ogg_stream_packetin/pageout */ }

TEST_CASE("StemReader: Opus open reports progress and can be cancelled") {
    // progress callback returns false on first call -> CHECK_THROWS_AS(..., audio::OpenCancelled)
}
```

`old_full_decode` is today's `decode_ogg_opus` / `decode_ogg_vorbis` / `decode_with_miniaudio`, copied into the test file as the reference. That's deliberate: it pins the new readers to the old output.

- [ ] **Step 3: Run them and see them fail.**

- [ ] **Step 4: Implement the Opus reader** (`src/audio/opus_reader.cpp`). How it works:

The page index: walk the bytes by hand. Each page starts with `OggS`, then version, header type (bit 0x02 = first page of a stream, BOS), 8-byte granule, 4-byte serial, 4-byte sequence, 4-byte CRC, segment count, segment table. Page length = 27 + segment count + sum of the segment table. Record `{offset, granule, serial, bos}` per page. If the capture pattern is missing at the expected offset, search forward for the next `OggS` (damaged file). Skip CRC checks in the index (libogg still checks every page when it decodes). Call `progress` every 4 MB.

Links: a BOS page starts a new link. For each link, read its OpusHead (channels, pre-skip, gain) from its first packet with libogg. Mapping family must be 0 and channels 1–2, as today. If a later link's channel count differs from the first, stop at the end of the first link (rare; document it). A link's sample count = its last page's granule − pre-skip. The stream's total length is the sum over links.

Read: keep libogg `ogg_stream_state` + `ogg_sync_state` and an `OpusDecoder`. Feed pages from the current byte offset. Decode packets into a 5760×channels scratch buffer allocated at open, then copy out. Drop pre-skip at link start. Apply gain if non-zero. Stop each link at its granule end.

Seek(F): find the link holding F. Target granule = F_in_link + pre-skip. Binary-search the link's pages for the last page whose granule is ≤ target − 3840 (80 ms pre-roll, RFC 7845 §4.6). Reset the decoder (`opus_decoder_ctl(dec, OPUS_RESET_STATE)`) and `ogg_stream_reset_serialno`. Start feeding from the next page. Throw away decoded samples until the output position reaches F. The first packet on that page may be the tail of a packet started on the previous page (continued-packet flag 0x01); libogg drops it after the reset, which is correct. The output position after the reset comes from that page's granule (the granule counts samples up to the end of the last packet completed on the page).

- [ ] **Step 5: Implement the Vorbis and miniaudio readers.** Vorbis: `stb_vorbis_open_memory(data, (int)size, &err, nullptr)`; reject sizes over `INT_MAX` with a clear error; `length = stb_vorbis_stream_length_in_samples`; read via `stb_vorbis_get_samples_float_interleaved(v, channels, out, frames*channels)`; seek via `stb_vorbis_seek(v, (unsigned)frame)`. Keep the old int16 conversion's output for straight-through reads: today's path decodes to int16 then divides by 32768. To stay bit-identical, read with `stb_vorbis_get_samples_short_interleaved` into a scratch buffer and divide by 32768.0f exactly as today. miniaudio: `ma_decoder_init_memory` with `ma_format_f32`, native channels and rate, `seekPointCount = 1024`; `ma_decoder_get_length_in_pcm_frames`; `ma_decoder_read_pcm_frames`; `ma_decoder_seek_to_pcm_frame`.

- [ ] **Step 6: Rebuild `decode_audio`** as open + exact reserve + read to the end, and `decode_stem` through `open_stem_reader(stem)`.

- [ ] **Step 7: Time the real file** with a scratch program in the session scratchpad (not the repo), and put the numbers in the commit message.

- [ ] **Step 8: Run all tests; commit** the files above — "Seekable stem readers; decode_audio on top of them".

---

### Task 3: Stream the mix during playback

**Goal:** The Playhead pulls audio from a `StreamMix` that reads every stem at one shared position, converts each to 48 kHz stereo, and adds them, so playback needs no decoded buffer at all.

**Files:**
- Create: `src/audio/stream_mix.h`, `src/audio/stream_mix.cpp`
- Modify: `src/audio/player.h`, `src/audio/player.cpp`, `src/audio/mixer.h`, `src/audio/mixer.cpp`, `CMakeLists.txt`
- Test: `tests/test_stream_mix.cpp` (create), `tests/test_audio_player.cpp` and `tests/test_preview_transport.cpp` (must pass unchanged)

**Acceptance Criteria:**
- [ ] `MixSource` interface: `channels()`, `sample_rate()`, `length_frames()`, `read(float* out, int64_t frames) -> int64_t`, `seek(int64_t frame)`. `BufferSource` wraps a `DecodedAudio`. `StreamMix` wraps N `StemReader`s.
- [ ] `Playhead(DecodedAudio)` still exists (builds a `BufferSource`), plus `Playhead(std::unique_ptr<MixSource>)`. Every existing player and transport test passes without edits.
- [ ] `Playhead::seek_frames` only records the position; the next `read_frames` while playing calls `source->seek(position)` once before reading. While paused, `read_frames` writes silence and never touches the source.
- [ ] Reading a `StreamMix` start to end equals today's `mix_stems(decode every stem)` output within 1e-6 (resampling in chunks versus one shot), for: two same-rate stems of different lengths; a 44.1 kHz mono stem plus a 48 kHz stereo stem; a front pad of 250 ms.
- [ ] `StreamMix` length = front pad + the longest converted stem. Shorter stems read as silence past their end.
- [ ] After `seek(F)`, reading matches the reference mix from F within 1e-3, skipping the first 20 ms.
- [ ] `StreamMix::read` does no heap allocation after construction (scratch buffers sized at construction for a 4096-frame block; larger requests are served in 4096-frame chunks).
- [ ] A stem whose reader returns a short read (decode error) goes silent; the others keep playing.
- [ ] `decode_and_mix` and `pad_front_ms` are deleted once Task 4 stops calling them; if Task 4 hasn't merged yet, leave them.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*StreamMix*,*Playhead*,*transport*"` → all pass.

**Steps:**

- [ ] **Step 1: Write failing tests** (`tests/test_stream_mix.cpp`). Build stems from synthetic WAV bytes (the WAV writer helper already in `tests/test_audio_mixer.cpp`), open them with `open_stem_reader`, wrap in `StreamMix(readers, 48000, 2, front_pad_frames)`, read in odd-sized blocks (e.g. 511 frames), compare to `mix_stems` of `decode_audio` of the same bytes with the same front pad prepended.

- [ ] **Step 2: Implement.**

```cpp
class MixSource {
public:
    virtual ~MixSource() = default;
    virtual int channels() const = 0;
    virtual int sample_rate() const = 0;
    virtual int64_t length_frames() const = 0;
    virtual int64_t read(float* out, int64_t frames) = 0;  // never throws
    virtual void seek(int64_t frame) = 0;                  // never throws
};

class StreamMix : public MixSource {
public:
    // `front_pad_frames` of silence come before every stem (a negative chart
    // offset). Each stem converts to out_rate / out_channels as it is read.
    StreamMix(std::vector<std::unique_ptr<StemReader>> stems, int out_rate,
              int out_channels, int64_t front_pad_frames);
    ...
private:
    struct Stem {
        std::unique_ptr<StemReader> reader;
        ma_data_converter conv;  // null when already out_rate/out_channels
        bool passthrough;
        std::vector<float> in_buf;   // reader-format scratch
        int64_t converted_length;    // ceil(len * out_rate / in_rate)
        bool ended;
    };
    ...
};
```

Per read: for each stem, pull enough input frames to produce the requested output frames (`ma_data_converter_get_required_input_frame_count`), convert into a scratch block, add into `out`. On seek: compute each stem's input frame as `floor(F_out_after_pad * in_rate / out_rate)`, call `reader->seek`, and reset the converter (`ma_data_converter_uninit` + `init`, done at seek time, not in the read path). Passthrough stems read straight into scratch and add.

`Playhead` keeps its position, gain and auto-pause logic; only the copy from `audio_.samples` becomes `source_->read`. Keep `std::memset` silence for frames past the end.

- [ ] **Step 3: Run all tests; commit** — "Stream the Preview mix from seekable readers".

---

### Task 4: Load job: open audio in parallel, honest progress, cancel anywhere

**Goal:** The Preview load opens the stems instead of unpacking them, does it beside the chart parse, reports progress in real units weighted by measured cost, and stops within a moment when the window closes.

**Files:**
- Modify: `src/ui/preview_load_job.h`, `src/ui/preview_load_job.cpp`, `src/ui/preview_controller.cpp`, `src/ui/preview_controller.h`, `src/ui/preview_tab.cpp` (the loading-bar drawing only), `src/audio/mixer.cpp/.h` (delete `decode_and_mix` and `pad_front_ms`), `tests/test_audio_mixer.cpp`, `tests/test_preview_transport.cpp` (move the `pad_front_ms` case to a `StreamMix` front-pad case)
- Test: `tests/test_preview_load_progress.cpp` (create), `tests/ui/uitest_details.cpp` (add one case)

**Acceptance Criteria:**
- [ ] The job runs two branches at once: (a) parse the chart and build the scene; (b) open every stem with `open_stem_reader`. The audio offset needs the parsed chart's `chart_offset_s`, so branch (b) builds the `StreamMix` only after both finish. A stem that fails to open is skipped, as today.
- [ ] `Result` carries a `std::unique_ptr<audio::MixSource>` instead of `DecodedAudio mixed`; the controller builds the `Playhead` from it. `audio_offset_ms` keeps its meaning; a negative offset becomes `front_pad_frames` in `StreamMix`.
- [ ] Steps are `Reading chart`, `Opening audio`, `Building scene`, `Building highway` (the last only after Task 7; before then the three earlier steps). Each step's share of the bar comes from costs measured in Task 4's Step 1, written as named constants with a comment giving the measured times.
- [ ] Within `Opening audio`, the bar moves by compressed bytes processed out of the total bytes of all stems (from `file_size_bytes` for loose files, `bytes.size()` for container stems). The bar never moves backwards, and never moves faster than the work.
- [ ] The label uses real units: `Opening audio: 312 of 625 MB` (MB = 10^6 bytes, whole numbers). Other steps show their name.
- [ ] Time left appears only when the load has run ≥ 3 s and the current byte rate has been measured over ≥ 1 s. It reads `about N s left` under a minute and `about N min left` after, and updates at most once a second.
- [ ] Any zero total (no stems, empty file) gives a finite fraction (no NaN reaches `ImGui::ProgressBar`, ImGui issue #7451).
- [ ] Cancel: the open progress callback returns false when the job is cancelled, so the Opus index stops within 4 MB; container extraction checks between entries. Closing the details window during `Opening audio` returns within 100 ms. Unit test: a job on a large synthetic Opus stem (build 300 MB in the test by re-paging the fixture's packets repeatedly, written to the test's temp folder) is cancelled 50 ms after start and `finished()` is true within 200 ms.
- [ ] GUI test in `uitest_details.cpp`: open the Preview, wait for load, check the label text passed through `load_progress()` was one of the four step names at each poll, and check no fraction value decreased between polls.
- [ ] On the Discography chart in the real app: Preview ready in under 2 s; the Hydra process stays under 1.5 GB working set (Task Manager or `GetProcessMemoryInfo` in a scratch harness). Record the numbers.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe; .\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4` → all pass.

**Steps:**

- [ ] **Step 1: Measure step costs** on the Discography chart (and one ordinary chart from `testdata/`) with a scratch harness: parse, index build, scene, timeline. Turn them into the weights.

- [ ] **Step 2: Write the progress tests** (`tests/test_preview_load_progress.cpp`): `Progress::fraction()` is non-decreasing across a scripted sequence of steps and byte counts; `label()` formats `Opening audio: 312 of 625 MB`; a zero total gives a finite fraction; `time_left_text()` is empty before 3 s and formats minutes and seconds after.

- [ ] **Step 3: Implement the job.**

```cpp
// run(): two branches. The parse is not cancellable mid-way (about 0.1 s on
// the largest chart); the audio branch checks the cancel flag every 4 MB.
auto audio_future = std::async(std::launch::async, [this, stems = ...] {
    std::vector<std::unique_ptr<audio::StemReader>> readers;
    for (const app::PreviewAudioStem& s : stems) {
        try {
            readers.push_back(audio::open_stem_reader(s, [this](uint64_t done, uint64_t) {
                bytes_done_.store(bytes_before_this_stem + done);
                return !is_cancelled();
            }));
        } catch (const audio::OpenCancelled&) { throw JobCancelled{}; }
          catch (const std::exception&) { /* skip, as today */ }
    }
    return readers;
});
```

`resolve_preview_source` must hand back the stem list without opening audio, so split it: `resolve_preview_song` (parse + offset) and `resolve_preview_stems` (find stems; Task 12 makes a .sng read once and shared). Run them on the two branches.

Progress state: atomics for `step_`, `bytes_done_`, `bytes_total_`, plus the start time; `Progress` gains `bytes_done`, `bytes_total`, `elapsed_s` and `time_left_text()`.

- [ ] **Step 4: Update the controller and the tab.** `PreviewController::LoadProgress` gets a `detail` string (time left). `preview_tab.cpp` keeps today's bar and `%` overlay; it shows `Loading preview: <label>` as today, plus the time left after it when not empty.

- [ ] **Step 5: Delete `decode_and_mix` and `pad_front_ms`**, move their tests to `StreamMix` equivalents.

- [ ] **Step 6: Run the full unit and GUI suites; time the real chart; commit** — "Preview load opens audio in parallel with honest progress and prompt cancel".

---

### Task 6: Build the highway timeline in one pass

**Goal:** `build_track_state` gives exactly the same timeline as today, in linear time, without a `std::map`.

Today, for each of the 214,000 moments on the Discography highway, `toggle_at` loops over every span. The fix walks the moments and the span edges together in time order, like merging two sorted lists.

**Files:**
- Modify: `src/render/track_state.cpp:68-171`, `src/render/track_state.h` (private helpers only; `toggle_at` stays public as the reference)
- Test: `tests/test_track_state.cpp` (add cases)

**Acceptance Criteria:**
- [ ] For every fixture scene the existing tests use, plus a randomized test (fixed seed, 2,000 instants, overlapping, touching, zero-length and nested spans in every span field), each instant's seven span fields and `fill_lane_pad` equal what the old per-instant `toggle_at` / `synthesize` computes.
- [ ] Instants come from merging the already-sorted notes, beats and span edges into a vector (sort + unique on time, then group), not a `std::map`.
- [ ] `synthesize` (still used by `window()` for the empty-window fallback) no longer rebuilds the lane interval list per call: keep a `fill_lane_ivs_` vector built once.
- [ ] On the Discography scene, `build_track_state` takes under 150 ms (was 608–741 ms). Record it.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*track*,*Track*"` → all pass.

**Steps:**

- [ ] **Step 1: Write the randomized equivalence test** using the old algorithm as a test-local reference (copy today's `build_track_state` body into the test as `reference_build`).

- [ ] **Step 2: Implement the sweep.** For one span field with intervals I and a sorted list of instant times T, the state at t needs three facts: some interval starts at t, some ends at t, and some interval has start < t < end. Precompute sorted `starts` and `ends` vectors. Walk T with two pointers: `n_start_before` = count of starts < t, `n_end_at_or_before` = count of ends ≤ t, `n_zero_at` = count of intervals with start == end == t. Then inside = `n_start_before − n_end_at_or_before + n_zero_at > 0`. Drop malformed intervals (end < start) before counting, because the old code could never mark them inside but would count them. They can still mark Start or End: keep a separate check for those, so behavior stays identical. The randomized test includes them. The lane pad: walk the lane intervals in start order with a small active set (fills rarely overlap), applying the old rule (an interval starting at or containing t wins; otherwise one ending at t).

- [ ] **Step 3: Replace the map** with a vector of `(time, kind, index)` events, sorted, then grouped by equal time.

- [ ] **Step 4: Time it on the Discography scene; run the tests; commit** — "Build the highway timeline in one pass".

---

### Task 7: Timeline built on the worker, uploaded on the UI thread

**Goal:** The load job and the path-overlay job build the `TrackState` themselves, and `PreviewRenderer::set_scene` takes it ready-made, so the UI thread does no timeline work.

**Files:**
- Modify: `src/render/preview_renderer.h:43`, `src/render/preview_renderer.cpp:391-393`, `src/ui/preview_load_job.h/.cpp` (both jobs), `src/ui/preview_controller.h/.cpp` (`render`, `poll`)
- Test: `tests/test_preview_controller.cpp` if present, else `tests/ui/uitest_details.cpp` (existing Preview cases must pass)

**Acceptance Criteria:**
- [ ] New `PreviewRenderer::set_scene(const PreviewScene&, TrackState state)`. The old overload stays and builds the state itself (used by the golden renderer test).
- [ ] `PreviewLoadJob::Result` and `PreviewSceneJob` carry a `render::TrackState` built with the controller's `pro_` option; the controller passes it through when `scene_dirty_`.
- [ ] Changing pro drums while a chart is open still rebuilds correctly: if the job's `pro` doesn't match the controller's at upload time, fall back to the building overload.
- [ ] All unit and GUI tests pass; the Onyx golden screenshot test passes unchanged.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe; .\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4` → all pass.

**Steps:**

- [ ] **Step 1:** Add the overload and plumb `TrackStateOptions` into both jobs (the controller already knows `pro_` when it starts them).
- [ ] **Step 2:** Build the state at the end of `PreviewLoadJob::run` (step `Building highway`) and `PreviewSceneJob::run`.
- [ ] **Step 3:** In `PreviewController::render`, use the job's state when present; clear it after upload.
- [ ] **Step 4:** Run all suites; commit — "Build the Preview timeline off the UI thread".

---

### Task 8: A path click rebuilds only what the path changes

**Goal:** Selecting a different path reuses the parsed song and every path-free part of the scene and timeline, and rebuilds only the overlay.

**Files:**
- Modify: `src/app/preview_view.h`, `src/app/preview_view.cpp` (split `build_preview_scene` into `build_preview_base` + `apply_preview_overlay`), `src/render/track_state.h/.cpp` (`rebuild_overlay_fields(TrackState&, const PreviewScene&)`), `src/ui/preview_load_job.cpp` (`PreviewSceneJob`), `src/ui/preview_controller.cpp`
- Test: `tests/test_preview_view.cpp`, `tests/test_track_state.cpp`

**Acceptance Criteria:**
- [ ] `build_preview_scene(song, path, cap, rules)` equals `apply_preview_overlay(build_preview_base(song, rules), song, path, cap, rules)` field by field for every fixture chart, with and without a path (an equality helper in the test compares every `PreviewScene` member).
- [ ] The implementer lists, in the header comment, which scene fields are path-free (expected: notes, beats, tempos, sections, spans, timing, song length) and which the overlay owns (expected: activations, fill states, score timeline, SP meter). Confirm the split by reading `build_preview_scene`; if a "path-free" field actually depends on the path, it belongs to the overlay.
- [ ] `rebuild_overlay_fields` gives the same `TrackState` as a full build for the new scene (equality test over instants).
- [ ] `PreviewSceneJob` holds the base scene (shared, read-only, like the song) and only runs the overlay + `rebuild_overlay_fields`.
- [ ] On the Discography chart, a path change costs under 100 ms on the worker plus the upload. Record it.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*preview*,*track*"; .\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4` → all pass.

**Steps:**

- [ ] **Step 1:** Write the equality tests against today's `build_preview_scene`.
- [ ] **Step 2:** Split the function; keep `build_preview_scene` as the composition so existing callers keep working.
- [ ] **Step 3:** Add `rebuild_overlay_fields`. Overlay span fields (fill, fill_taken, sp_active, fill_lane) depend on the path. Instants created only by old overlay edges must be removed, and new overlay edges inserted. The simple correct version: keep the base instants (notes, beats, path-free span edges) cached, merge in the new overlay edges, and recompute only the overlay fields with the Task 6 sweep.
- [ ] **Step 4:** Rewire `PreviewSceneJob` and the controller; run all suites; commit — "Rebuild only the path overlay on a path change".

---

### Task 9: Replay walks only the open activation windows

**Goal:** `replay_path` keeps a short list of activation windows that can still pay the current chord, instead of checking every window for every chord, and offers a scores-only mode for the Preview.

**Files:**
- Modify: `src/core/replay.cpp:61-160`, `src/core/replay.h`
- Test: `tests/test_replay.cpp`

**Acceptance Criteria:**
- [ ] Windows enter the open list when `row.tick >= act_tick` (they are already sorted by `act_tick`). A window leaves the list for good when the current chord is past its deact node (`row.tick > deact_tick`) and it no longer counts the chord (`sqout_position(...) == After` or `!counted_without_squeeze(offset, leeway)`). The implementer must confirm from `core/squeeze_rating` / `core/scoring` that both exclusions only grow as time moves forward. If either doesn't, the window may only leave by the condition that does.
- [ ] A new test replays every fixture chart with every path the existing tests use, plus a synthetic path with 2,000 activations on a long fixture, through both the old loop (copied into the test as the reference) and the new one: every `ReplayChord` field is identical.
- [ ] `ReplayOptions{.scores_only = true}` skips `chord_code` and `notes` per row (left empty). Only the Preview uses it; `build_preview_scene` reads no other row field. The implementer checks this with a grep and lists the fields it reads in a comment.
- [ ] With 2,000 synthetic activations on the Discography chart, replay drops from about 243–345 ms to under 100 ms. Record it.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*replay*,*Replay*"` → all pass.

**Steps:**

- [ ] **Step 1:** Write the equivalence test with the old loop as reference.
- [ ] **Step 2:** Implement the open list (`std::vector<size_t>` of window indexes; append on entry, erase-remove on exit) and the option.
- [ ] **Step 3:** Point `build_preview_scene`'s replay call at `scores_only`.
- [ ] **Step 4:** Run tests; commit — "Replay walks only open activation windows".

---

### Task 10: Stop recomputing Preview boxes every frame

**Goal:** The Preview tab stops rebuilding unchanged things every frame, and the remaining per-frame lookups use binary search.

**Files:**
- Modify: `src/ui/preview_tab.cpp:250,306,430`, `src/ui/preview_controller.h/.cpp` (`next_act_boxes`, `scrub_marks` caches), `src/app/preview_view.cpp:142-149,339-352,464-484,591-604`
- Test: `tests/test_preview_view.cpp`

**Acceptance Criteria:**
- [ ] `next_act_boxes()` and `scrub_marks()` are computed once per scene change (and per length change for scrub marks) and cached in the controller; `preview_tab.cpp` calls `scrub_marks()` once per frame.
- [ ] `build_next_act_box` finds the next activation by binary search over the sorted activation times instead of scanning from the first.
- [ ] The time box's BPM, time-signature and section lookups use `std::upper_bound` over their time-sorted vectors.
- [ ] Fill classification (`preview_view.cpp:339-352`) and the SP meter build (`142-149`) walk their sorted inputs with a moving index instead of nested scans.
- [ ] Every existing test passes. New tests check binary-search results equal a linear scan for randomized times on a fixture scene (box text, BPM, time signature, section, fill states, SP meter segments).

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe; .\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4` → all pass.

**Steps:**

- [ ] **Step 1:** Write the old-versus-new equality tests (old scans copied into the test).
- [ ] **Step 2:** Implement the caches (invalidate on `scene_dirty_` and on length change) and the searches.
- [ ] **Step 3:** Run all suites; commit — "Cache Preview boxes per scene; binary-search per-frame lookups".

---

### Task 11: Leaner chart parsing

**Goal:** The MIDI and .chart readers stop doing per-event allocations that cost time and memory on huge charts, with identical parse results.

**Files:**
- Modify: `src/parse/midi.h:40,52`, `src/parse/midi.cpp:243-285`, `src/parse/song.cpp:533-547,741-749,813-854`
- Test: `tests/test_midi.cpp`, `tests/test_song.cpp`, plus a parse-equality check across `testdata/`

**Acceptance Criteria:**
- [ ] `midi::Message` stores its type as an enum and keeps a string only for text and meta-text events. Every caller that compared the type string compares the enum. Track message vectors `reserve` from a cheap first-pass estimate (bytes / 3).
- [ ] The MIDI note-on-tick handling in `song.cpp:533-547` stops building `std::function` vectors per tick: use a fixed array of handlers or a switch.
- [ ] The .chart reader walks the file text with `std::string_view` lines instead of copying it into a vector of strings. It reads `E` events with a hand parser instead of `std::regex`, and stops copying each entry into a hash map where a view or a move does.
- [ ] Parse equality: a test (or a scratch run of `hydra_batch` over `testdata/` before and after, diffing the outputs per the repo-cleanup verification story) shows every `Song` field identical for every fixture chart, .mid and .chart.
- [ ] The Discography `notes.mid` parse drops from about 93–105 ms. Record the new number. Also time the largest .chart in `testdata/` before and after.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*midi*,*MIDI*,*song*,*chart*"` → all pass; parse-equality diff empty.

**Steps:**

- [ ] **Step 1:** Capture the "before" parse output of every fixture (scratch program serializing each `Song` to text) and timings.
- [ ] **Step 2:** Make the MIDI changes; re-run the equality check.
- [ ] **Step 3:** Make the .chart changes; re-run the equality check. Keep the exact same handling of malformed lines (the existing tests pin it; add a test for any malformed-line case you find that isn't pinned).
- [ ] **Step 4:** Run tests; commit — "Leaner MIDI and .chart parsing, identical results".

---

### Task 12: Read a .sng or .srb chart once

**Goal:** Opening a container chart's Preview reads the file from disk once and shares the bytes between the note loader and the audio extractor, and unmasking a .sng audio entry doesn't copy it twice.

**Files:**
- Modify: `src/parse/song.cpp:1093,1123` (bytes-taking loaders), `src/parse/song.h`, `src/parse/sng.cpp:81-91`, `src/parse/sng.h`, `src/app/preview_source.cpp:337-363`, `src/app/preview_source.h`, `src/app/analysis.cpp/.h` (`load_songpath` gets a bytes overload)
- Test: `tests/test_preview_source.cpp`, `tests/test_sng.cpp`

**Acceptance Criteria:**
- [ ] `load_songpath_from_bytes(path, bytes, ...)` exists for .sng and .srb; `load_songpath` calls it after one read.
- [ ] `resolve_preview_source` for .sng/.srb reads the file once and passes the same buffer to both. A test with a counting hook (a test-only read counter in `read_file_bytes`, or a wrapper passed in) shows one read.
- [ ] `sng_decode_file` unmasks into the output buffer directly (one allocation per entry).
- [ ] Every .sng/.srb fixture gives the same `Song`, stems and offset as before.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*sng*,*srb*,*preview_source*"` → all pass.

**Steps:**

- [ ] **Step 1:** Write the equality and read-count tests.
- [ ] **Step 2:** Add the bytes overloads and rewire.
- [ ] **Step 3:** Run tests; commit — "Read container charts once for the Preview".

---

### Task 13: Docs, ADR and the end-to-end check

**Goal:** The decision is recorded, the docs describe the new load, and the whole change is measured on the real library charts.

**Files:**
- Create: `docs/adr/0019-preview-audio-streams-from-the-compressed-file.md`
- Modify: `docs/adr/0004-audio-via-miniaudio-and-libopus.md` (note: the "decode every stem and sum" sentence is superseded by 0019), `docs/adr/0006-ogg-opus-decoded-by-hand-with-libogg-and-libopus.md` (add the page index, seek, gain and chains, and point to 0019), `CONTEXT.md` (Transport/Preview entries if they mention decoded buffers), `docs/handoffs/2026-10-03-preview-loading-audit.md` (add a "Status" section listing what shipped with commit hashes)

**Acceptance Criteria:**
- [ ] ADR 0019 says, in plain sentences: the Preview streams from the compressed bytes; why (an 8.6-hour stem is 12 GB unpacked); one mixer reads every stem at one position; seeks pre-roll 80 ms; post-seek samples may differ slightly; loose files are memory-mapped.
- [ ] Measured and written into the audit's Status section: Preview-ready time and peak working set for blink-182 Discography (opus), Rise Against Discography (opus), and Nirvana "Endless, Nameless Setlist (discog)" song.ogg (Vorbis, 715 MB). The Nirvana chart must now play audio. Check whether it was silent before by running the old build on it, and say which.
- [ ] Full unit suite and full GUI suite pass on the merged result; counts recorded.
- [ ] A short manual check list for the user: open the Discography chart's Preview, play, seek to the middle, step by ticks, change path, close mid-load. The executor runs the GUI-test equivalents; the user's look in the real app is listed as pending.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe; .\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4` → all pass; the Status section exists with the numbers.

**Steps:**

- [ ] **Step 1:** Write ADR 0019 and the ADR/CONTEXT edits.
- [ ] **Step 2:** Run the measurements one at a time on a quiet machine (check `tasklist` first; nothing else CPU-heavy).
- [ ] **Step 3:** Write the Status section; commit — "Docs: Preview audio streams (ADR 0019); audit status".

---

## Self-review notes

Spec coverage, checked against the audit: streaming (Tasks 2, 3, 4), cancel (Task 4, with the Opus index hook from Task 2), progress bar (Task 4), timeline linear (Task 6), off the UI thread (Task 7), path-only rebuild (Task 8), replay (Task 9), per-frame boxes, scrub marks, time box, fill classification, SP meter (Task 10), MIDI and .chart readers (Task 11), Vorbis overflow (Task 2), files over 2 GB (Task 1), Opus gain and chains (Task 2), .sng/.srb read once (Task 12), parse beside audio (Task 4). The audit's "parallel decode of one stem" is deliberately not built: streaming makes it unnecessary, and the audit's own seam test showed it needs extra warm-up. There is no Task 5: the audit's cancel fix folded into Task 4.
