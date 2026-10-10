// Tests for audio/song_audio: a song's stems mapped, opened and mixed by the
// real functions the Preview load calls (map_song_stems, stems_total_bytes,
// open_song_stems, mix_song_stems). The audio is the testdata/audio sine
// fixtures plus junk bytes typed here. The Preview load's own case with a stem
// that will not open lives in test_preview_load_progress.cpp.

#include "doctest.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/preview_source.h"
#include "audio/song_audio.h"
#include "audio/stem_reader.h"
#include "audio_util.h"  // fixture_path, read_fixture

using namespace hydra::audio;
using hydra::app::PreviewAudioStem;
using testaudio::fixture_path;
using testaudio::read_fixture;

namespace {

// Byte sizes of the two fixtures the cases below map, pinned from one run
// (Get-Item) on 2026-10-10 at 013f4c66.
constexpr uint64_t kSine220OggBytes = 11402;  // testdata/audio/sine220.ogg
constexpr uint64_t kSine220Mp3Bytes = 26441;  // testdata/audio/sine220.mp3

// Bytes no audio opener recognises: a stem that will not open.
const std::string kJunk = "not audio";

PreviewAudioStem file_stem(const std::string& label, const std::string& path) {
    PreviewAudioStem s;
    s.label = label;
    s.path = path;
    return s;
}

PreviewAudioStem bytes_stem(const std::string& label, std::vector<uint8_t> bytes) {
    PreviewAudioStem s;
    s.label = label;
    s.bytes = std::move(bytes);
    return s;
}

std::vector<uint8_t> junk_bytes() { return std::vector<uint8_t>(kJunk.begin(), kJunk.end()); }

// The sine fixture as one opened reader.
std::unique_ptr<StemReader> sine_reader() {
    return open_stem_reader(file_stem("song", fixture_path("sine220.ogg")));
}

}  // namespace

TEST_CASE("map_song_stems maps a file, leaves a missing file empty and moves bytes in") {
    const std::string missing = fixture_path("no_such_stem.ogg");
    const std::vector<std::optional<StemBytes>> mapped =
        map_song_stems({file_stem("song", fixture_path("sine220.ogg")),
                        file_stem("guitar", missing), bytes_stem("drums", read_fixture("sine220.mp3"))});
    REQUIRE(mapped.size() == 3);

    // The file stem: mapped read-only, nothing copied.
    REQUIRE(mapped[0].has_value());
    CHECK(mapped[0]->mapped != nullptr);
    CHECK(mapped[0]->owned.empty());
    CHECK(mapped[0]->size() == kSine220OggBytes);

    // The missing file: empty, so open_song_stems skips it.
    CHECK_FALSE(mapped[1].has_value());

    // The bytes stem: its bytes owned, no mapping.
    REQUIRE(mapped[2].has_value());
    CHECK(mapped[2]->mapped == nullptr);
    CHECK(mapped[2]->owned.size() == kSine220Mp3Bytes);

    CHECK(stems_total_bytes(mapped) == kSine220OggBytes + kSine220Mp3Bytes);
}

TEST_CASE("open_song_stems skips a stem that will not open and reports its progress") {
    const auto stems = [] {
        return map_song_stems({file_stem("song", fixture_path("sine220.ogg")),
                               bytes_stem("broken", junk_bytes()),
                               bytes_stem("drums", read_fixture("sine220.mp3"))});
    };

    SUBCASE("the junk stem is skipped and the two real ones open") {
        std::vector<std::optional<StemBytes>> bytes = stems();
        const uint64_t total = stems_total_bytes(bytes);
        std::vector<std::pair<uint64_t, uint64_t>> calls;
        const std::vector<std::unique_ptr<StemReader>> readers =
            open_song_stems(std::move(bytes), [&calls](uint64_t done, uint64_t of) {
                calls.emplace_back(done, of);
                return true;
            });
        CHECK(readers.size() == 2);
        REQUIRE_FALSE(calls.empty());
        CHECK(calls.back().first == total);
        CHECK(calls.back().second == total);
    }

    SUBCASE("a progress callback that says stop on its first call cancels the open") {
        int heard = 0;
        CHECK_THROWS_AS(open_song_stems(stems(),
                                        [&heard](uint64_t, uint64_t) {
                                            ++heard;
                                            return false;
                                        }),
                        OpenCancelled);
        CHECK(heard == 1);
    }
}

TEST_CASE("mix_song_stems pads a negative offset in front and finds where the audio ends") {
    // The same stem mixed with no offset gives the plain length.
    std::vector<std::unique_ptr<StemReader>> plain_readers;
    plain_readers.push_back(sine_reader());
    const SongMix plain = mix_song_stems(std::move(plain_readers), 0.0);
    REQUIRE(plain.mix != nullptr);

    // The 12000 frames are 250 ms at 48 kHz, the same figure the Preview load's
    // own front-silence case pins (test_preview_load_progress.cpp).
    std::vector<std::unique_ptr<StemReader>> one;
    one.push_back(sine_reader());
    const SongMix padded = mix_song_stems(std::move(one), -250.0);
    CHECK(padded.audio_offset_ms == 0.0);
    REQUIRE(padded.mix != nullptr);
    CHECK(padded.mix->length_frames() == plain.mix->length_frames() + 12000);
    CHECK(padded.end_chart_ms.has_value());

    // No readers: the front pad alone is not audio, so there is no end.
    const SongMix none = mix_song_stems({}, -250.0);
    REQUIRE(none.mix != nullptr);
    CHECK_FALSE(none.end_chart_ms.has_value());
    const SongMix none_unpadded = mix_song_stems({}, 0.0);
    CHECK_FALSE(none_unpadded.end_chart_ms.has_value());
}
