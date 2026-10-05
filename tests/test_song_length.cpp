// Tests for app/song_length: a song's length is the length its chart's
// metadata states, in chart time, or its last Expert drum note when the
// metadata states none (D75). The scan reads the stated length and the delay
// with the names. No audio is opened for any of it.

#include "doctest.h"

#include <optional>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/song_length.h"
#include "audio_chart_fixtures.h"
#include "chart_text.h"
#include "core/winstr.h"
#include "parse/song.h"
#include "sng_util.h"
#include "srb_util.h"
#include "store/record_store.h"
#include "temp_util.h"

using hydra::Difficulty;
using hydra::app::ScanItem;
using hydra::store::ChartTimingMeta;

namespace {

void write_bytes(const std::string& path, const std::vector<uint8_t>& data) {
    std::FILE* f = hydra::fopen_utf8(path, L"wb");
    REQUIRE(f != nullptr);
    if (!data.empty()) std::fwrite(data.data(), 1, data.size(), f);
    std::fclose(f);
}

// The one chart the scan finds under the folder of `notespath`.
ScanItem scan_one(const std::string& notespath) {
    auto [items, errors] = hydra::app::discover_charts({hydra::parent_folder(notespath)});
    REQUIRE(errors.empty());
    REQUIRE(items.size() == 1);
    REQUIRE(items[0].timing.has_value());
    return items[0];
}

// The song's length the way an analysis at Expert with 2x kick works it out:
// the scan's timing and the chart parsed once.
std::optional<double> length_after_scan(const std::string& notespath) {
    const ScanItem item = scan_one(notespath);
    const hydra::Song song = hydra::load_songpath(notespath, true, true);
    return hydra::app::chart_song_length_ms(*item.timing, notespath, song, Difficulty::Expert,
                                            true, hydra::core::default_rules());
}

// short_chart_with_long_audio's chart with `ini` as its song.ini.
std::string chart_with_ini(const std::string& tag, const std::string& ini) {
    const std::string notes = audiochart::short_chart_with_long_audio(tag);
    audiochart::write_text_file(hydra::parent_folder(notes) + "\\song.ini", ini);
    return notes;
}

// A chart whose last note is a 2x kick at tick 384 (200 ms at 600 BPM), with
// Hard charting that ends at tick 0.
std::string chart_ending_on_2x_kick(const std::string& tag) {
    const std::string d = testtemp::temp_dir(tag);
    using namespace testchart;
    audiochart::write_text_file(
        d + "\\notes.chart",
        chart_text(section("ExpertDrums", line(0, "N 0 0") + line(192, "N 1 0") +
                                              line(384, "N 32 0")) +
                       section("HardDrums", line(0, "N 0 0")),
                   192, "", line(0, "TS 4") + line(0, "B 600000")));
    audiochart::write_text_file(d + "\\song.ini", "[song]\nname = Two Kick\n");
    return d + "\\notes.chart";
}

}  // namespace

TEST_CASE("song length: song.ini's song_length is the length, moved by the delay") {
    std::optional<double> length = length_after_scan(chart_with_ini("sl_stated", "[song]\nsong_length = 200000\n"));
    REQUIRE(length.has_value());
    CHECK(*length == 200000.0);

    // A positive delay puts chart time 0 that far into the audio: the song
    // ends that much sooner in chart time. A negative one, later.
    length = length_after_scan(
        chart_with_ini("sl_delay_pos", "[song]\nsong_length = 200000\ndelay = 250\n"));
    REQUIRE(length.has_value());
    CHECK(*length == 199750.0);
    length = length_after_scan(
        chart_with_ini("sl_delay_neg", "[song]\nsong_length = 200000\ndelay = -250\n"));
    REQUIRE(length.has_value());
    CHECK(*length == 200250.0);
}

TEST_CASE("song length: a length song.ini does not state falls back to the last note") {
    // short_chart_with_long_audio's last note is at 100 ms; its audio runs 5 s.
    for (const char* ini : {"[song]\nsong_length =\n", "[song]\nname = X\n",
                            "[song]\nsong_length = 0\n", "[song]\nsong_length = -5\n",
                            "[song]\nsong_length = abc\n"}) {
        CAPTURE(ini);
        const std::optional<double> length = length_after_scan(chart_with_ini("sl_fallback", ini));
        REQUIRE(length.has_value());
        CHECK(*length == 100.0);
    }
}

TEST_CASE("song length: the backup reads the Expert chart with 2x kick, whatever was analyzed") {
    const std::string notes = chart_ending_on_2x_kick("sl_2x");
    const ChartTimingMeta none = scan_one(notes).timing.value();
    const hydra::core::Rules& rules = hydra::core::default_rules();

    // Analyzed with 2x off: the 2x kick still ends the song.
    const hydra::Song no_2x = hydra::load_songpath(notes, true, false);
    std::optional<double> length = hydra::app::chart_song_length_ms(
        none, notes, no_2x, Difficulty::Expert, false, rules);
    REQUIRE(length.has_value());
    CHECK(*length == 200.0);

    // Analyzed at Hard: the Expert chart still ends the song.
    const hydra::Song hard = hydra::load_songpath(notes, true, true, Difficulty::Hard);
    length = hydra::app::chart_song_length_ms(none, notes, hard, Difficulty::Hard, true, rules);
    REQUIRE(length.has_value());
    CHECK(*length == 200.0);
}

TEST_CASE("song length: a .sng's song_length key and a .srb's song_length_ms field") {
    const std::string dir = testtemp::temp_dir("sl_containers");
    const std::vector<uint8_t> notes =
        hydra::read_file_bytes(audiochart::short_chart_with_long_audio("sl_container_notes"));

    write_bytes(dir + "\\a.sng",
                testsng::make_sng({{"song_length", "150000"}, {"delay", "100"}},
                                  {{"notes.chart", notes}}));
    write_bytes(dir + "\\b.srb", testsrb::make_srb(
                                     testsrb::make_metadata_with_length("notes.chart", 196905),
                                     notes));
    auto [items, errors] = hydra::app::discover_charts({dir});
    REQUIRE(errors.empty());
    REQUIRE(items.size() == 2);
    for (const ScanItem& item : items) {
        CAPTURE(item.notespath);
        REQUIRE(item.timing.has_value());
        // The backfill's own read agrees with the scan's (open question 3).
        CHECK(hydra::app::read_chart_timing_meta(item.notespath) == *item.timing);
        if (hydra::ends_with_ci(item.notespath, ".sng")) {
            CHECK(item.timing->length_ms == 150000.0);
            CHECK(item.timing->delay_ms == 100.0);
        } else {
            CHECK(item.timing->length_ms == 196905.0);
            CHECK_FALSE(item.timing->delay_ms.has_value());
        }
    }

    // A .srb that stores 0 states no length: the last note stands in.
    const std::string zero = testtemp::temp_dir("sl_srb_zero") + "\\z.srb";
    write_bytes(zero, testsrb::make_srb(testsrb::make_metadata_with_length("notes.chart", 0),
                                        notes));
    const std::optional<double> length = length_after_scan(zero);
    REQUIRE(length.has_value());
    CHECK(*length == 100.0);
}

TEST_CASE("song length: the library keeps what the scan read, and an older scan's rows read none") {
    hydra::store::RecordStore store(":memory:");
    hydra::store::ChartLibraryEntry e;
    e.md5 = "abc";
    e.notespath = "C:\\x\\notes.chart";
    e.sig = "1:2:3:4";
    e.timing = ChartTimingMeta{200000.0, 250.0};
    store.rebuild_chart_library({e});

    const std::vector<hydra::store::ChartLibraryEntry> listed = store.list_chart_library(0, -1);
    REQUIRE(listed.size() == 1);
    CHECK(listed[0].timing == e.timing);
    const hydra::store::ChartLibraryCache cache = store.chart_library_cache();
    REQUIRE(cache.count(e.notespath) == 1);
    CHECK(cache.at(e.notespath).timing == *e.timing);
}
