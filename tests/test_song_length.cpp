// Tests for app/song_length: a song's one length (D255, amending D75), from
// the stated length its chart's metadata gives, the chart's end event and its
// last note over every instrument, plus the pad. The scan reads the stated
// length and the delay with the names. No audio is opened for any of it.
// Every expected length is the candidate the rule picks, read from the song's
// own tempo map, plus app::kSongEndPadMs through the song speed. The chart
// sync rule (app/preview_source.h) is tested here too, as the step that moves
// the stated length into chart time.

#include "doctest.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/preview_source.h"  // preview_audio_offset_ms, chart_ms_of_audio_ms
#include "app/song_length.h"
#include "audio_chart_fixtures.h"
#include "chart_text.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "load_util.h"
#include "parse/chart_files.h"
#include "parse/song.h"
#include "sng_util.h"
#include "srb_util.h"
#include "store/record_store.h"
#include "temp_util.h"

using hydra::Difficulty;
using hydra::app::kSongEndPadMs;
using hydra::app::ScanItem;
using hydra::store::ChartTimingMeta;

namespace {

using testtemp::write_bytes;

// The reading an analysis at Expert with 2x kick parses with.
const hydra::ChartReading kExpert2x{Difficulty::Expert, true, true, false};

// The one chart the scan finds under the folder of `notespath`.
ScanItem scan_one(const std::string& notespath) {
    auto [items, errors] = hydra::app::discover_charts({hydra::parent_folder(notespath)});
    REQUIRE(errors.empty());
    REQUIRE(items.size() == 1);
    REQUIRE(items[0].timing.has_value());
    return items[0];
}

// The chart at `notespath`, or the chart `chart` (bytes), parsed at Expert
// with 2x kick.
hydra::Song song_at(const std::string& notespath) {
    return hydra::load_songpath(notespath, kExpert2x);
}
hydra::Song song_of(const std::vector<uint8_t>& chart) {
    return hydra::load_songbytes_chart(chart, kExpert2x);
}
// The chart `chart` (bytes) parsed at Expert with 2x kick, timed at the song
// speed `speed_percent` (ChartReading::speed_percent).
hydra::Song song_of_at_speed(const std::vector<uint8_t>& chart, int speed_percent) {
    hydra::ChartReading reading = kExpert2x;
    reading.speed_percent = speed_percent;
    return hydra::load_songbytes_chart(chart, reading);
}

// The song's length the way an analysis at Expert with 2x kick works it out:
// the scan's timing and the chart parsed once.
std::optional<double> length_after_scan(const std::string& notespath) {
    const ScanItem item = scan_one(notespath);
    return hydra::app::song_length_ms(*item.timing, song_at(notespath));
}

// song_length_ms of `chart` (bytes) parsed at Expert, with `meta` as what the
// scan read.
std::optional<double> length_of(const ChartTimingMeta& meta, const std::vector<uint8_t>& chart) {
    return hydra::app::song_length_ms(meta, song_of(chart));
}

// The three candidates of the length rule, each read from `song`'s own tempo
// map: a tick in chart ms, the last note's end, and the end event.
double ms_at(const hydra::Song& song, int64_t tick) { return song.timecode(tick).ms(); }
double last_note_ms(const hydra::Song& song) {
    REQUIRE(song.last_note_end_tick.has_value());
    return ms_at(song, *song.last_note_end_tick);
}
double end_event_ms(const hydra::Song& song) {
    REQUIRE(song.end_event_tick.has_value());
    return ms_at(song, *song.end_event_tick);
}

// `meta`'s stated length moved into `song`'s chart time by the delay rule's
// owners (preview_audio_offset_ms, chart_ms_of_audio_ms), at the speed
// `song` is timed at.
double stated_chart_ms(const ChartTimingMeta& meta, const hydra::Song& song) {
    const std::optional<double> raw = hydra::app::stated_length_ms(meta.length_ms);
    REQUIRE(raw.has_value());
    return hydra::app::chart_ms_of_audio_ms(
        *raw, hydra::app::preview_audio_offset_ms(meta.delay_ms, song.chart_offset_s),
        song.timing().speed_ratio());
}

// short_chart_with_long_audio's chart with `ini` as its song.ini.
std::string chart_with_ini(const std::string& tag, const std::string& ini) {
    const std::string notes = audiochart::short_chart_with_long_audio(tag);
    audiochart::write_text_file(hydra::parent_folder(notes) + "\\song.ini", ini);
    return notes;
}

// The tempo of short_chart_with_long_audio: 600 BPM at resolution 192, so a
// beat is 192 ticks. Its drum notes sit at ticks 0 and 192.
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

// The tick of the 2x kick that is chart_ending_on_2x_kick's last note.
constexpr int64_t kLastKickTick = 384;

// A chart whose last note is a 2x kick at kLastKickTick, with Hard charting
// that ends at tick 0.
std::string chart_ending_on_2x_kick(const std::string& tag) {
    const std::string d = testtemp::temp_dir(tag);
    using namespace testchart;
    audiochart::write_text_file(
        d + "\\notes.chart",
        chart_text(section("ExpertDrums", line(0, "N 0 0") + line(192, "N 1 0") +
                                              line(kLastKickTick, "N 32 0")) +
                       section("HardDrums", line(0, "N 0 0")),
                   192, "", kSync600));
    audiochart::write_text_file(d + "\\song.ini", "[song]\nname = Two Kick\n");
    return d + "\\notes.chart";
}

// A corpus chart whose stated length stands (it does not end before the last
// note) and whose end event counts, with its song length and those two
// candidates in chart ms.
struct EndCase {
    std::string notespath;
    std::optional<double> length;
    double stated_ms;
    double end_event_ms;
};

// Every such chart the scan of testdata/input finds.
std::vector<EndCase> corpus_end_cases() {
    auto [items, errors] = hydra::app::discover_charts({corpus::root()});
    REQUIRE(errors.empty());
    std::vector<EndCase> out;
    for (const ScanItem& item : items) {
        CAPTURE(item.notespath);
        REQUIRE(item.timing.has_value());
        if (!hydra::app::stated_length_ms(item.timing->length_ms)) continue;
        const hydra::Song& song = corpus::song(item.notespath, kExpert2x);
        if (!song.last_note_end_tick || !hydra::app::end_event_counts(song)) continue;
        const double stated = stated_chart_ms(*item.timing, song);
        if (stated < last_note_ms(song)) continue;
        out.push_back(EndCase{item.notespath, hydra::app::song_length_ms(*item.timing, song),
                              stated, end_event_ms(song)});
    }
    return out;
}

}  // namespace

TEST_CASE("song length: song.ini's song_length is the length, moved by the delay") {
    const std::string plain = chart_with_ini("sl_stated", "[song]\nsong_length = 200000\n");
    const std::optional<double> length = length_after_scan(plain);
    REQUIRE(length.has_value());
    CHECK(*length == stated_chart_ms({200000.0, std::nullopt}, song_at(plain)) + kSongEndPadMs);

    // A positive delay puts chart time 0 that far into the audio: the song
    // ends sooner in chart time. A negative one, later.
    const std::string pos_notes =
        chart_with_ini("sl_delay_pos", "[song]\nsong_length = 200000\ndelay = 250\n");
    const std::optional<double> pos = length_after_scan(pos_notes);
    REQUIRE(pos.has_value());
    CHECK(*pos == stated_chart_ms({200000.0, 250.0}, song_at(pos_notes)) + kSongEndPadMs);
    CHECK(*pos < *length);

    const std::string neg_notes =
        chart_with_ini("sl_delay_neg", "[song]\nsong_length = 200000\ndelay = -250\n");
    const std::optional<double> neg = length_after_scan(neg_notes);
    REQUIRE(neg.has_value());
    CHECK(*neg == stated_chart_ms({200000.0, -250.0}, song_at(neg_notes)) + kSongEndPadMs);
    CHECK(*neg > *length);
}

TEST_CASE("song length: with no stated length and no end event, the last note ends the song") {
    // short_chart_with_long_audio's audio runs past its last note; the audio
    // is never read.
    for (const char* ini : {"[song]\nsong_length =\n", "[song]\nname = X\n",
                            "[song]\nsong_length = 0\n", "[song]\nsong_length = -5\n",
                            "[song]\nsong_length = abc\n"}) {
        CAPTURE(ini);
        const std::string notes = chart_with_ini("sl_fallback", ini);
        const std::optional<double> length = length_after_scan(notes);
        REQUIRE(length.has_value());
        CHECK(*length == last_note_ms(song_at(notes)) + kSongEndPadMs);
    }
}

TEST_CASE("song length: a stated length that ends before the last note is dropped") {
    const std::string short_notes = chart_with_ini("sl_too_short", "[song]\nsong_length = 50\n");
    const hydra::Song song = song_at(short_notes);
    const double last_note = last_note_ms(song);
    REQUIRE(50.0 < last_note);
    std::optional<double> length = length_after_scan(short_notes);
    REQUIRE(length.has_value());
    CHECK(*length == last_note + kSongEndPadMs);

    // The check is in chart time: 150 ms of audio less a 60 ms delay ends
    // before the note, though 150 ms of audio alone would not.
    REQUIRE(150.0 > last_note);
    REQUIRE(stated_chart_ms({150.0, 60.0}, song) < last_note);
    length = length_after_scan(
        chart_with_ini("sl_too_short_delay", "[song]\nsong_length = 150\ndelay = 60\n"));
    REQUIRE(length.has_value());
    CHECK(*length == last_note + kSongEndPadMs);

    // Ending on the note itself is not too short. An end event well past the
    // note tells kept from dropped: a kept stated length caps the end event,
    // a dropped one leaves the end event to stand in.
    const std::vector<uint8_t> with_end = drums_and(end_event_at(3840));
    const hydra::Song ended = song_of(with_end);
    const double on_note = last_note_ms(ended);
    REQUIRE(stated_chart_ms({on_note, std::nullopt}, ended) == on_note);
    REQUIRE(end_event_ms(ended) > on_note);
    length = length_of({on_note, std::nullopt}, with_end);
    REQUIRE(length.has_value());
    CHECK(*length == on_note + kSongEndPadMs);
    // One millisecond short of the note, it is dropped.
    length = length_of({on_note - 1.0, std::nullopt}, with_end);
    REQUIRE(length.has_value());
    CHECK(*length == end_event_ms(ended) + kSongEndPadMs);

    const std::string past_notes = chart_with_ini("sl_past_note", "[song]\nsong_length = 120\n");
    REQUIRE(120.0 > last_note);
    length = length_after_scan(past_notes);
    REQUIRE(length.has_value());
    CHECK(*length == stated_chart_ms({120.0, std::nullopt}, song_at(past_notes)) + kSongEndPadMs);
}

TEST_CASE("song length: the last note is read from every note in the file, whatever was analyzed") {
    const std::string notes = chart_ending_on_2x_kick("sl_2x");
    const ChartTimingMeta none = scan_one(notes).timing.value();

    // Analyzed with 2x off: the 2x kick still ends the song.
    const hydra::ChartReading expert_no_2x{Difficulty::Expert, true, false, false};
    const hydra::Song no_2x = hydra::load_songpath(notes, expert_no_2x);
    std::optional<double> length = hydra::app::song_length_ms(none, no_2x);
    REQUIRE(length.has_value());
    CHECK(*length == ms_at(no_2x, kLastKickTick) + kSongEndPadMs);

    // Analyzed at Hard: the Expert chart still ends the song.
    const hydra::ChartReading hard_2x{Difficulty::Hard, true, true, false};
    const hydra::Song hard = hydra::load_songpath(notes, hard_2x);
    length = hydra::app::song_length_ms(none, hard);
    REQUIRE(length.has_value());
    CHECK(*length == ms_at(hard, kLastKickTick) + kSongEndPadMs);
}

TEST_CASE("song length: the last note is the latest note end over every instrument") {
    // A guitar note held past the drums' last note.
    constexpr int64_t kGuitarTick = 576;
    constexpr int64_t kGuitarHold = 192;
    constexpr int64_t kGuitarEnd = kGuitarTick + kGuitarHold;
    const std::string guitar =
        testchart::section("ExpertSingle", testchart::line(kGuitarTick, "N 0 " + std::to_string(kGuitarHold)));
    const hydra::Song song = song_of(drums_and(guitar));
    const double guitar_end_ms = ms_at(song, kGuitarEnd);
    std::optional<double> length = length_of({}, drums_and(guitar));
    REQUIRE(length.has_value());
    CHECK(*length == guitar_end_ms + kSongEndPadMs);

    // A stated 300 ms ends before that note's end, so it is dropped.
    REQUIRE(300.0 < guitar_end_ms);
    length = length_of({300.0, std::nullopt}, drums_and(guitar));
    REQUIRE(length.has_value());
    CHECK(*length == guitar_end_ms + kSongEndPadMs);

    // The end event's one-beat margin is measured from that note's end too.
    const int64_t beat = song.tick_resolution();
    length = length_of({}, drums_and(guitar + end_event_at(kGuitarEnd + beat - 1)));
    REQUIRE(length.has_value());
    CHECK(*length == guitar_end_ms + kSongEndPadMs);
    length = length_of({}, drums_and(guitar + end_event_at(kGuitarEnd + beat)));
    REQUIRE(length.has_value());
    CHECK(*length == ms_at(song, kGuitarEnd + beat) + kSongEndPadMs);
}

TEST_CASE("song length: an end event before the stated length caps it") {
    int capped = 0;
    for (const EndCase& c : corpus_end_cases()) {
        if (c.end_event_ms >= c.stated_ms) continue;
        CAPTURE(c.notespath);
        REQUIRE(c.length.has_value());
        CHECK(*c.length == c.end_event_ms + kSongEndPadMs);
        ++capped;
    }
    CHECK(capped > 0);
}

TEST_CASE("song length: an end event past the stated length leaves it") {
    int left = 0;
    for (const EndCase& c : corpus_end_cases()) {
        if (c.end_event_ms < c.stated_ms) continue;
        CAPTURE(c.notespath);
        REQUIRE(c.length.has_value());
        CHECK(*c.length == c.stated_ms + kSongEndPadMs);
        ++left;
    }
    CHECK(left > 0);
}

TEST_CASE("song length: with the stated length dropped, the end event stands in") {
    // The stated 50 ms ends before the last note; the end event sits well
    // past it.
    const std::vector<uint8_t> chart = drums_and(end_event_at(3840));
    const hydra::Song song = song_of(chart);
    REQUIRE(50.0 < last_note_ms(song));
    const std::optional<double> length = length_of({50.0, std::nullopt}, chart);
    REQUIRE(length.has_value());
    CHECK(*length == end_event_ms(song) + kSongEndPadMs);
}

TEST_CASE("song length: with no stated length, the end event stands in") {
    const std::vector<uint8_t> chart = drums_and(end_event_at(3840));
    std::optional<double> length = length_of({}, chart);
    REQUIRE(length.has_value());
    CHECK(*length == end_event_ms(song_of(chart)) + kSongEndPadMs);

    // With no end event either, the last note.
    const std::vector<uint8_t> bare = drums_and("");
    length = length_of({}, bare);
    REQUIRE(length.has_value());
    CHECK(*length == last_note_ms(song_of(bare)) + kSongEndPadMs);
}

TEST_CASE("song length: an end event inside one beat of the last note is ignored") {
    // The end event counts from one beat past the last note's end on.
    const hydra::Song bare = song_of(drums_and(""));
    REQUIRE(bare.last_note_end_tick.has_value());
    const int64_t edge = *bare.last_note_end_tick + bare.tick_resolution();
    const hydra::Song counts = song_of(drums_and(end_event_at(edge)));
    const hydra::Song ignored = song_of(drums_and(end_event_at(edge - 1)));
    CHECK(hydra::app::end_event_counts(counts));
    CHECK_FALSE(hydra::app::end_event_counts(ignored));

    // With no stated length: the event at the edge, else the last note.
    std::optional<double> length = length_of({}, drums_and(end_event_at(edge)));
    REQUIRE(length.has_value());
    CHECK(*length == end_event_ms(counts) + kSongEndPadMs);
    length = length_of({}, drums_and(end_event_at(edge - 1)));
    REQUIRE(length.has_value());
    CHECK(*length == last_note_ms(ignored) + kSongEndPadMs);

    // With a stated 300 ms: the event at the edge caps it, else it stands.
    REQUIRE(end_event_ms(counts) < stated_chart_ms({300.0, std::nullopt}, counts));
    length = length_of({300.0, std::nullopt}, drums_and(end_event_at(edge)));
    REQUIRE(length.has_value());
    CHECK(*length == end_event_ms(counts) + kSongEndPadMs);
    length = length_of({300.0, std::nullopt}, drums_and(end_event_at(edge - 1)));
    REQUIRE(length.has_value());
    CHECK(*length == stated_chart_ms({300.0, std::nullopt}, ignored) + kSongEndPadMs);
}

TEST_CASE("song length: the end event is chart time, the stated length is audio time") {
    // A 60 ms delay moves the stated 400 ms of audio to an earlier chart time,
    // before the end event; the delay does not move the end event.
    const std::vector<uint8_t> chart = drums_and(end_event_at(672));
    const hydra::Song song = song_of(chart);
    const double stated = stated_chart_ms({400.0, 60.0}, song);
    REQUIRE(stated < end_event_ms(song));
    std::optional<double> length = length_of({400.0, 60.0}, chart);
    REQUIRE(length.has_value());
    CHECK(*length == stated + kSongEndPadMs);

    length = length_of({std::nullopt, 60.0}, chart);
    REQUIRE(length.has_value());
    CHECK(*length == end_event_ms(song) + kSongEndPadMs);
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
    CHECK(*length == end_event_ms(song_at(dir + "\\a.sng")) + kSongEndPadMs);
}

TEST_CASE("song length: a .sng's song_length key and a .srb's song_length_ms field") {
    const std::string dir = testtemp::temp_dir("sl_containers");
    const std::vector<uint8_t> notes =
        hydra::read_file_bytes(audiochart::short_chart_with_long_audio("sl_container_notes"));

    // The .sng states 150000 ms with a 100 ms delay; the .srb states
    // 196905 ms and no delay. Each length must come out of the scan as the
    // song's length.
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
        const hydra::Song song = song_at(item.notespath);
        const ChartTimingMeta stated =
            hydra::chart_format_of(item.notespath) == hydra::ChartFormat::Sng
                ? ChartTimingMeta{150000.0, 100.0}
                : ChartTimingMeta{196905.0, std::nullopt};
        const std::optional<double> length = hydra::app::song_length_ms(*item.timing, song);
        REQUIRE(length.has_value());
        CHECK(*length == stated_chart_ms(stated, song) + kSongEndPadMs);
    }

    // A .srb that stores 0 states no length: the last note stands in.
    const std::string zero = testtemp::temp_dir("sl_srb_zero") + "\\z.srb";
    write_bytes(zero, testsrb::make_srb(testsrb::make_metadata_with_length("notes.chart", 0),
                                        notes));
    const std::optional<double> length = length_after_scan(zero);
    REQUIRE(length.has_value());
    CHECK(*length == last_note_ms(song_at(zero)) + kSongEndPadMs);
}

TEST_CASE("song length: at 50% a stated length is twice its 100% length, pad included") {
    // CR-46: the game builds the length in the audio file's time, adds the
    // 1 s pad, and its clock divides the whole by the speed. The stated
    // length stands here: it ends well past the two drum notes.
    const std::vector<uint8_t> chart = drums_and("");
    const ChartTimingMeta meta{200000.0, 250.0};
    const hydra::Song normal = song_of(chart);
    const hydra::Song half = song_of_at_speed(chart, 50);
    REQUIRE(half.timing().speed_ratio() == 0.5);
    const std::optional<double> at_100 = hydra::app::song_length_ms(meta, normal);
    const std::optional<double> at_50 = hydra::app::song_length_ms(meta, half);
    REQUIRE(at_100.has_value());
    REQUIRE(at_50.has_value());
    CHECK(*at_50 == 2.0 * *at_100);
    // At 50% the pad is 2 s of chart time.
    CHECK(*at_50 - stated_chart_ms(meta, half) == 2.0 * kSongEndPadMs);
    // At 100% the length is today's: the stated 200000 ms, less the 250 ms
    // delay, plus the 1 s pad.
    CHECK(*at_100 == 200000.0 - 250.0 + kSongEndPadMs);
}

TEST_CASE("song length: at 50% an end event or last note length is twice its 100% length") {
    // The chart times come from the song's own tempo map at its speed, and
    // the pad is divided by the same speed.
    for (const std::string& more : {end_event_at(3840), std::string()}) {
        const std::vector<uint8_t> chart = drums_and(more);
        const std::optional<double> at_100 = hydra::app::song_length_ms({}, song_of(chart));
        const hydra::Song half = song_of_at_speed(chart, 50);
        const std::optional<double> at_50 = hydra::app::song_length_ms({}, half);
        REQUIRE(at_100.has_value());
        REQUIRE(at_50.has_value());
        CHECK(*at_50 == doctest::Approx(2.0 * *at_100));
        const double end_ms = more.empty() ? last_note_ms(half) : end_event_ms(half);
        CHECK(*at_50 - end_ms == doctest::Approx(2.0 * kSongEndPadMs));
    }
}

TEST_CASE("song length: at 200% the end event counts only two file beats past the last note") {
    // CR-41: the game's margin is its working resolution, the file's
    // resolution times the speed, cut to a whole number. At 200% that is two
    // of the file's beats.
    const hydra::Song bare = song_of(drums_and(""));
    REQUIRE(bare.last_note_end_tick.has_value());
    const int64_t last = *bare.last_note_end_tick;
    const int64_t file_beat = bare.tick_resolution();
    const auto counts_at = [&](int speed_percent, int64_t end_tick) {
        return hydra::app::end_event_counts(
            song_of_at_speed(drums_and(end_event_at(end_tick)), speed_percent));
    };
    // At 100%, one file beat, as today.
    CHECK(counts_at(100, last + file_beat));
    CHECK_FALSE(counts_at(100, last + file_beat - 1));
    // At 200%, two file beats: one beat past the last note no longer counts.
    CHECK(counts_at(200, last + 2 * file_beat));
    CHECK_FALSE(counts_at(200, last + 2 * file_beat - 1));
    CHECK_FALSE(counts_at(200, last + file_beat));

    // The length follows: at 200% an end event one file beat past the last
    // note is ignored, so the song is as long as the same chart with no end
    // event.
    const std::optional<double> ignored = hydra::app::song_length_ms(
        {}, song_of_at_speed(drums_and(end_event_at(last + file_beat)), 200));
    const std::optional<double> no_event =
        hydra::app::song_length_ms({}, song_of_at_speed(drums_and(""), 200));
    REQUIRE(ignored.has_value());
    REQUIRE(no_event.has_value());
    CHECK(*ignored == *no_event);
}

TEST_CASE("song length: the end event's margin is the working resolution cut to a whole number") {
    // At 33% the file's 192 ticks a beat make a working resolution of
    // 63.36 ticks; the game cuts it to 63 (CR-41), never rounds it up to 64.
    const hydra::Song bare = song_of_at_speed(drums_and(""), 33);
    REQUIRE(bare.tick_resolution() == 192);
    REQUIRE(bare.last_note_end_tick.has_value());
    const int64_t last = *bare.last_note_end_tick;
    CHECK(hydra::app::end_event_counts(song_of_at_speed(drums_and(end_event_at(last + 63)), 33)));
    CHECK_FALSE(
        hydra::app::end_event_counts(song_of_at_speed(drums_and(end_event_at(last + 62)), 33)));
}

TEST_CASE("chart sync rule: song_length_ms's audio-to-chart step runs both ways at 50%, 100% and 200%") {
    // TM-05: the game's clock is (audio position - delay) / speed. The delay
    // is in the file's time and is never scaled.
    const double offset_ms = 250.0;
    for (const double ratio : {0.5, 1.0, 2.0}) {
        CAPTURE(ratio);
        for (const double chart_ms : {0.0, 1234.5, 200000.0}) {
            CAPTURE(chart_ms);
            const double audio_ms = hydra::app::audio_ms_of_chart_ms(chart_ms, offset_ms, ratio);
            CHECK(hydra::app::chart_ms_of_audio_ms(audio_ms, offset_ms, ratio) ==
                  doctest::Approx(chart_ms));
        }
        // Chart time 0 is the delay's place in the audio at every speed.
        CHECK(hydra::app::audio_ms_of_chart_ms(0.0, offset_ms, ratio) == offset_ms);
    }
    // At 200% one chart second covers two seconds of the file.
    CHECK(hydra::app::audio_ms_of_chart_ms(1000.0, offset_ms, 2.0) == 2000.0 + offset_ms);
    // At 100% both are the one-offset rule.
    CHECK(hydra::app::audio_ms_of_chart_ms(1234.5, offset_ms, 1.0) == 1234.5 + offset_ms);
    CHECK(hydra::app::chart_ms_of_audio_ms(1234.5, offset_ms, 1.0) == 1234.5 - offset_ms);
}

TEST_CASE("song length: the library keeps what the scan read, so a stored row gives the same length") {
    // The scan of the corpus, saved the way the app saves it. An older
    // scan's rows reading no timing is tested in test_store_library.cpp.
    auto [items, errors] = hydra::app::discover_charts({corpus::root()});
    REQUIRE(errors.empty());
    hydra::store::RecordStore store(":memory:");
    REQUIRE_FALSE(hydra::app::save_scan_as_library(store, items).has_value());

    std::map<std::string, ChartTimingMeta> scanned;
    for (const ScanItem& item : items) {
        REQUIRE(item.timing.has_value());
        scanned[item.notespath] = *item.timing;
    }
    const hydra::store::ChartLibraryCache cache = store.chart_library_cache();
    const std::vector<hydra::store::ChartLibraryEntry> listed = store.list_chart_library(0, -1);
    REQUIRE(listed.size() == items.size());
    int stated = 0;
    for (const hydra::store::ChartLibraryEntry& row : listed) {
        CAPTURE(row.notespath);
        REQUIRE(scanned.count(row.notespath) == 1);
        REQUIRE(row.timing.has_value());
        REQUIRE(cache.count(row.notespath) == 1);
        REQUIRE(cache.at(row.notespath).timing.has_value());
        const ChartTimingMeta& read = scanned.at(row.notespath);
        const hydra::Song& song = corpus::song(row.notespath, kExpert2x);
        const std::optional<double> want = hydra::app::song_length_ms(read, song);
        CHECK(hydra::app::song_length_ms(*row.timing, song) == want);
        CHECK(hydra::app::song_length_ms(*cache.at(row.notespath).timing, song) == want);
        if (hydra::app::stated_length_ms(read.length_ms)) ++stated;
    }
    CHECK(stated > 0);
}
