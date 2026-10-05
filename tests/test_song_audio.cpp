// Tests for audio/song_audio: a song's length is how long its audio runs, in
// chart time (D69). The folders are real charts with the test sine beside
// them (audio_chart_fixtures.h), opened the way the Preview opens them.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "app/analysis.h"  // hash_chart_file
#include "audio/device.h"
#include "audio/song_audio.h"
#include "audio_chart_fixtures.h"
#include "core/winstr.h"
#include "parse/song.h"
#include "store/record_store.h"
#include "ui/preview_controller.h"

namespace {

// The length of the chart at `notespath`, parsed the way analysis parses it.
std::optional<double> length_of(const std::string& notespath) {
    return hydra::audio::song_length_ms(notespath, hydra::load_songpath(notespath, true, true));
}

}  // namespace

TEST_CASE("song length: a loose chart's length is its audio's end in chart time") {
    // The chart's last note is at 100 ms; its song.ogg is the 5 s sine.
    const std::optional<double> length =
        length_of(audiochart::short_chart_with_long_audio("prevctl_len"));
    REQUIRE(length.has_value());
    CHECK(*length == 5000.0);
}

TEST_CASE("song length: the chart's offset moves the end") {
    const std::string chart = audiochart::short_chart_with_long_audio("prevctl_len_delay");
    const std::string ini = hydra::parent_folder(chart) + "\\song.ini";

    // A positive delay puts chart time 0 that far into the audio, so the
    // audio ends that much sooner in chart time.
    audiochart::write_text_file(ini, "[song]\ndelay = 250\n");
    std::optional<double> length = length_of(chart);
    REQUIRE(length.has_value());
    CHECK(*length == 4750.0);

    // A negative delay starts the chart before the audio: the end comes later.
    audiochart::write_text_file(ini, "[song]\ndelay = -250\n");
    length = length_of(chart);
    REQUIRE(length.has_value());
    CHECK(*length == 5250.0);
}

TEST_CASE("song length: a chart with no readable audio has none") {
    const std::string chart = audiochart::short_chart_with_long_audio("prevctl_len_none");
    const std::string ogg = hydra::parent_folder(chart) + "\\song.ogg";

    // Junk bytes under an audio name: the stem will not open.
    audiochart::write_text_file(ogg, "not audio at all");
    CHECK_FALSE(length_of(chart).has_value());

    // No audio file at all.
    REQUIRE(DeleteFileW(hydra::utf8_to_wide(ogg).c_str()));
    CHECK_FALSE(length_of(chart).has_value());
}

TEST_CASE("song length: the Preview's audio end and the song length agree") {
    const std::string chart = audiochart::short_chart_with_long_audio("prevctl_len_preview");
    const std::optional<double> length = length_of(chart);
    REQUIRE(length.has_value());

    hydra::ui::PreviewController pc(nullptr, nullptr);
    pc.set_audio_device_factory(
        [](int, int, hydra::ui::PreviewController::AudioSource)
            -> std::unique_ptr<hydra::audio::PreviewAudioDevice> {
            throw std::runtime_error("no device in tests");
        });
    hydra::store::ChartLibraryEntry entry;
    entry.md5 = hydra::app::hash_chart_file(chart);
    entry.notespath = chart;
    pc.open(entry, true, true, hydra::Difficulty::Expert, nullptr, "", 4);
    for (int i = 0; i < 1200 && pc.loading(); ++i) {
        pc.poll();
        Sleep(50);
    }
    REQUIRE_FALSE(pc.loading());
    REQUIRE(pc.has_audio());
    CHECK(pc.scrub_end_ms() == *length);
}
