// Tests for audio/song_audio: a song's stems mapped, opened and mixed by the
// real functions the Preview load calls (map_song_stems, stems_total_bytes,
// open_song_stems, mix_song_stems). The audio is the testdata/audio sine
// fixtures plus junk bytes typed here. The Preview load's own case with a stem
// that will not open lives in test_preview_load_progress.cpp.

#include "doctest.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/preview_source.h"
#include "audio/frames.h"
#include "audio/song_audio.h"
#include "audio/stem_reader.h"
#include "audio_util.h"  // fixture_path, read_fixture

using namespace hydra::audio;
using hydra::app::PreviewAudioStem;
using testaudio::fixture_path;
using testaudio::read_fixture;

namespace {

// The byte size of a fixture under testdata/audio, read from the file system.
uint64_t fixture_bytes(const std::string& name) {
    return static_cast<uint64_t>(std::filesystem::file_size(std::filesystem::u8path(fixture_path(name))));
}

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

// The mp3 sine fixture held in memory, as one opened reader.
std::unique_ptr<StemReader> mp3_bytes_reader() {
    return open_stem_reader(bytes_stem("drums", read_fixture("sine220.mp3")));
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
    const uint64_t ogg_bytes = fixture_bytes("sine220.ogg");
    const uint64_t mp3_bytes = read_fixture("sine220.mp3").size();
    CHECK(mapped[0]->size() == ogg_bytes);

    // The missing file: empty, so open_song_stems skips it.
    CHECK_FALSE(mapped[1].has_value());

    // The bytes stem: its bytes owned, no mapping.
    REQUIRE(mapped[2].has_value());
    CHECK(mapped[2]->mapped == nullptr);
    CHECK(mapped[2]->owned.size() == mp3_bytes);

    CHECK(stems_total_bytes(mapped) == ogg_bytes + mp3_bytes);
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
        // The two that opened are the ogg file and the mp3 bytes, in stem
        // order: each matches the same stem opened on its own.
        REQUIRE(readers.size() == 2);
        const std::unique_ptr<StemReader> ogg = sine_reader();
        const std::unique_ptr<StemReader> mp3 = mp3_bytes_reader();
        CHECK(readers[0]->length_frames() == ogg->length_frames());
        CHECK(readers[0]->sample_rate() == ogg->sample_rate());
        CHECK(readers[0]->channels() == ogg->channels());
        CHECK(readers[1]->length_frames() == mp3->length_frames());
        CHECK(readers[1]->sample_rate() == mp3->sample_rate());
        CHECK(readers[1]->channels() == mp3->channels());
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

TEST_CASE("mix_song_stems pads a negative offset in front") {
    // The same stem mixed with no offset gives the plain length.
    std::vector<std::unique_ptr<StemReader>> plain_readers;
    plain_readers.push_back(sine_reader());
    const SongMix plain = mix_song_stems(std::move(plain_readers), 0.0);
    REQUIRE(plain.mix != nullptr);

    // A -250 ms offset pads 250 ms of silence at the mix's output rate.
    const double offset_ms = -250.0;
    const int64_t pad_frames = frames_of_ms(-offset_ms, kOutRate);
    REQUIRE(pad_frames > 0);
    std::vector<std::unique_ptr<StemReader>> one;
    one.push_back(sine_reader());
    const SongMix padded = mix_song_stems(std::move(one), offset_ms);
    CHECK(padded.audio_offset_ms == 0.0);
    REQUIRE(padded.mix != nullptr);
    CHECK(padded.mix->length_frames() == plain.mix->length_frames() + pad_frames);

    // No readers: the front pad alone is all there is.
    const SongMix none = mix_song_stems({}, offset_ms);
    REQUIRE(none.mix != nullptr);
    CHECK(none.mix->length_frames() == pad_frames);
    const SongMix none_unpadded = mix_song_stems({}, 0.0);
    REQUIRE(none_unpadded.mix != nullptr);
    CHECK(none_unpadded.mix->length_frames() == 0);
}
