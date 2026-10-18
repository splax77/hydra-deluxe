// Tests for app/song_length: a song's one length (D255, amending D75), from
// the stated length its chart's metadata gives, the chart's end event and its
// last note over every instrument, plus the pad. The scan reads the stated
// length and the delay with the names. No audio is opened for any of it.
// Every expected length is a literal pinned from one run.

#include "doctest.h"

#include <optional>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/song_length.h"
#include "audio_chart_fixtures.h"
#include "chart_text.h"
#include "core/winstr.h"
#include "load_util.h"
#include "parse/chart_files.h"
#include "parse/song.h"
#include "sng_util.h"
#include "srb_util.h"
#include "store/record_store.h"
#include "temp_util.h"

using hydra::Difficulty;
using hydra::app::ScanItem;
using hydra::store::ChartTimingMeta;

namespace {

using testtemp::write_bytes;

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
    const hydra::ChartReading reading{Difficulty::Expert, true, true, false};
    return hydra::app::song_length_ms(*item.timing, hydra::load_songpath(notespath, reading));
}

// short_chart_with_long_audio's chart with `ini` as its song.ini.
std::string chart_with_ini(const std::string& tag, const std::string& ini) {
    const std::string notes = audiochart::short_chart_with_long_audio(tag);
    audiochart::write_text_file(hydra::parent_folder(notes) + "\\song.ini", ini);
    return notes;
}

// The tempo of short_chart_with_long_audio: 600 BPM at resolution 192, so a
// beat is 192 ticks and 100 ms. Its drum notes sit at ticks 0 and 192.
const std::string kSync600 = testchart::line(0, "TS 4") + testchart::line(0, "B 600000");
const std::string kTwoDrumNotes =
    testchart::section("ExpertDrums", testchart::line(0, "N 0 0") + testchart::line(192, "N 1 0"));

// The bytes of a chart with those two drum notes, then `more` sections.
std::vector<uint8_t> drums_and(const std::string& more) {
    return testchart::chart_bytes(kTwoDrumNotes + more, 192, "", kSync600);
}

// An [Events] section holding one end event at `tick`.
std::string end_event_at(int64_t tick) {
    return testchart::section("Events", testchart::line(tick, "E \"end\""));
}

// song_length_ms of `chart` (bytes) parsed at Expert, with `meta` as what the
// scan read.
std::optional<double> length_of(const ChartTimingMeta& meta, const std::vector<uint8_t>& chart) {
    const hydra::ChartReading reading{Difficulty::Expert, true, true, false};
    return hydra::app::song_length_ms(meta, hydra::load_songbytes_chart(chart, reading));
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
                   192, "", kSync600));
    audiochart::write_text_file(d + "\\song.ini", "[song]\nname = Two Kick\n");
    return d + "\\notes.chart";
}

}  // namespace

TEST_CASE("song length: song.ini's song_length is the length, moved by the delay") {
    std::optional<double> length = length_after_scan(chart_with_ini("sl_stated", "[song]\nsong_length = 200000\n"));
    REQUIRE(length.has_value());
    CHECK(*length == 201000.0);

    // A positive delay puts chart time 0 that far into the audio: the song
    // ends that much sooner in chart time. A negative one, later.
    length = length_after_scan(
        chart_with_ini("sl_delay_pos", "[song]\nsong_length = 200000\ndelay = 250\n"));
    REQUIRE(length.has_value());
    CHECK(*length == 200750.0);
    length = length_after_scan(
        chart_with_ini("sl_delay_neg", "[song]\nsong_length = 200000\ndelay = -250\n"));
    REQUIRE(length.has_value());
    CHECK(*length == 201250.0);
}

TEST_CASE("song length: with no stated length and no end event, the last note ends the song") {
    // short_chart_with_long_audio's last note is at 100 ms; its audio runs 5 s.
    for (const char* ini : {"[song]\nsong_length =\n", "[song]\nname = X\n",
                            "[song]\nsong_length = 0\n", "[song]\nsong_length = -5\n",
                            "[song]\nsong_length = abc\n"}) {
        CAPTURE(ini);
        const std::optional<double> length = length_after_scan(chart_with_ini("sl_fallback", ini));
        REQUIRE(length.has_value());
        CHECK(*length == 1100.0);
    }
}

TEST_CASE("song length: a stated length that ends before the last note is dropped") {
    // short_chart_with_long_audio's last note is at 100 ms.
    std::optional<double> length =
        length_after_scan(chart_with_ini("sl_too_short", "[song]\nsong_length = 50\n"));
    REQUIRE(length.has_value());
    CHECK(*length == 1100.0);

    // The check is in chart time: 150 ms of audio less a 60 ms delay ends at
    // 90 ms of chart time, before the note.
    length = length_after_scan(
        chart_with_ini("sl_too_short_delay", "[song]\nsong_length = 150\ndelay = 60\n"));
    REQUIRE(length.has_value());
    CHECK(*length == 1100.0);

    // Ending on the note itself is not too short.
    length = length_after_scan(chart_with_ini("sl_on_note", "[song]\nsong_length = 100\n"));
    REQUIRE(length.has_value());
    CHECK(*length == 1100.0);
    length = length_after_scan(chart_with_ini("sl_past_note", "[song]\nsong_length = 120\n"));
    REQUIRE(length.has_value());
    CHECK(*length == 1120.0);
}

TEST_CASE("song length: the last note is read from every note in the file, whatever was analyzed") {
    const std::string notes = chart_ending_on_2x_kick("sl_2x");
    const ChartTimingMeta none = scan_one(notes).timing.value();

    // Analyzed with 2x off: the 2x kick still ends the song.
    const hydra::ChartReading expert_no_2x{Difficulty::Expert, true, false, false};
    std::optional<double> length =
        hydra::app::song_length_ms(none, hydra::load_songpath(notes, expert_no_2x));
    REQUIRE(length.has_value());
    CHECK(*length == 1200.0);

    // Analyzed at Hard: the Expert chart still ends the song.
    const hydra::ChartReading hard_2x{Difficulty::Hard, true, true, false};
    length = hydra::app::song_length_ms(none, hydra::load_songpath(notes, hard_2x));
    REQUIRE(length.has_value());
    CHECK(*length == 1200.0);
}

TEST_CASE("song length: the last note is the latest note end over every instrument") {
    // A guitar note at tick 576 held 192 ticks ends at tick 768 (400 ms), past
    // the drums' last note at 100 ms.
    const std::string guitar =
        testchart::section("ExpertSingle", testchart::line(576, "N 0 192"));
    std::optional<double> length = length_of({}, drums_and(guitar));
    REQUIRE(length.has_value());
    CHECK(*length == 1400.0);

    // A stated 300 ms ends before that note's end, so it is dropped.
    length = length_of({300.0, std::nullopt}, drums_and(guitar));
    REQUIRE(length.has_value());
    CHECK(*length == 1400.0);

    // The end event's one-beat margin is measured from that note's end too.
    length = length_of({}, drums_and(guitar + end_event_at(768 + 191)));
    REQUIRE(length.has_value());
    CHECK(*length == 1400.0);
    length = length_of({}, drums_and(guitar + end_event_at(768 + 192)));
    REQUIRE(length.has_value());
    CHECK(*length == 1500.0);
}

TEST_CASE("song length: an end event before the stated length caps it") {
    // Allister - Overrated's shape: the end event (150,000 ms, tick 288,000)
    // comes before the stated 200,000 ms.
    const std::optional<double> length =
        length_of({200000.0, std::nullopt}, drums_and(end_event_at(288000)));
    REQUIRE(length.has_value());
    CHECK(*length == 151000.0);
}

TEST_CASE("song length: an end event past the stated length leaves it") {
    // Thrice - Deadbolt's shape: the end event (210,000 ms, tick 403,200) sits
    // past the stated 200,000 ms.
    const std::optional<double> length =
        length_of({200000.0, std::nullopt}, drums_and(end_event_at(403200)));
    REQUIRE(length.has_value());
    CHECK(*length == 201000.0);
}

TEST_CASE("song length: with the stated length dropped, the end event stands in") {
    // Rime of the Ancient Mariner's shape: the stated 50 ms ends before the
    // last note (100 ms), and the end event sits at 2,000 ms (tick 3,840).
    const std::optional<double> length =
        length_of({50.0, std::nullopt}, drums_and(end_event_at(3840)));
    REQUIRE(length.has_value());
    CHECK(*length == 3000.0);
}

TEST_CASE("song length: with no stated length, the end event stands in") {
    std::optional<double> length = length_of({}, drums_and(end_event_at(3840)));
    REQUIRE(length.has_value());
    CHECK(*length == 3000.0);

    // With no end event either, the last note.
    length = length_of({}, drums_and(""));
    REQUIRE(length.has_value());
    CHECK(*length == 1100.0);
}

TEST_CASE("song length: an end event inside one beat of the last note is ignored") {
    // The last note ends at tick 192 and a beat is 192 ticks, so the end event
    // counts from tick 384 (200 ms) on.
    const hydra::Song counts =
        hydra::load_songbytes_chart(drums_and(end_event_at(384)), hydra::ChartReading{});
    const hydra::Song ignored =
        hydra::load_songbytes_chart(drums_and(end_event_at(383)), hydra::ChartReading{});
    CHECK(hydra::app::end_event_counts(counts));
    CHECK_FALSE(hydra::app::end_event_counts(ignored));

    // With no stated length: the event at the edge, else the last note.
    std::optional<double> length = length_of({}, drums_and(end_event_at(384)));
    REQUIRE(length.has_value());
    CHECK(*length == 1200.0);
    length = length_of({}, drums_and(end_event_at(383)));
    REQUIRE(length.has_value());
    CHECK(*length == 1100.0);

    // With a stated 300 ms: the event at the edge caps it, else it stands.
    length = length_of({300.0, std::nullopt}, drums_and(end_event_at(384)));
    REQUIRE(length.has_value());
    CHECK(*length == 1200.0);
    length = length_of({300.0, std::nullopt}, drums_and(end_event_at(383)));
    REQUIRE(length.has_value());
    CHECK(*length == 1300.0);
}

TEST_CASE("song length: the end event is chart time, the stated length is audio time") {
    // A 60 ms delay: the stated 400 ms of audio ends at 340 ms of chart time.
    // The end event at tick 672 is 350 ms of chart time, and the delay does
    // not move it.
    std::optional<double> length = length_of({400.0, 60.0}, drums_and(end_event_at(672)));
    REQUIRE(length.has_value());
    CHECK(*length == 1340.0);

    length = length_of({std::nullopt, 60.0}, drums_and(end_event_at(672)));
    REQUIRE(length.has_value());
    CHECK(*length == 1350.0);
}

TEST_CASE("song length: a chart with no stated length, no end event and no notes has none") {
    CHECK_FALSE(hydra::app::song_length_ms({}, hydra::Song(192)).has_value());
}

TEST_CASE("song length: a .sng carries its inner file's end event") {
    const std::string dir = testtemp::temp_dir("sl_sng_end");
    write_bytes(dir + "\\a.sng",
                testsng::make_sng({{"name", "End"}},
                                  {{"notes.chart", drums_and(end_event_at(3840))}}));
    const std::optional<double> length = length_after_scan(dir + "\\a.sng");
    REQUIRE(length.has_value());
    CHECK(*length == 3000.0);
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
        // The backfill's own read agrees with the scan's.
        CHECK(hydra::app::read_chart_timing_meta(item.notespath) == *item.timing);
        if (hydra::chart_format_of(item.notespath) == hydra::ChartFormat::Sng) {
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
    CHECK(*length == 1100.0);
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
