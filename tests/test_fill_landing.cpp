// Step 2, T4 (finding 315, D22 and D30): an authored fill lands on the next
// chord up to trunc(resolution x slop) + 1 ticks after the fill ends, as Clone
// Hero 1.1 computes it (cvttsd2si at 0x20D0088, inc at 0x20D008D). A chord
// exactly at the window's edge still lands. Each fill is placed on its own
// once every chord is read (0x20CFF60 calling 0x5DE030).

#include "doctest.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "chart_text.h"
#include "core/rules.h"
#include "load_util.h"
#include "midi_util.h"
#include "parse/song.h"

using namespace hydra;

namespace {

int64_t activation_tick(const Song& song) {
    for (const SongTimestamp& ts : song.sequence)
        if (ts.has_activation()) return ts.timecode.ticks();
    return -1;
}

// A .chart at `res` ticks per beat: chords at 0 and 3*res, a fill from 2*res
// to 4*res, and one more chord `after` ticks past the fill end. Returns the
// tick of the chord the fill landed on.
int64_t chart_landing(int64_t res, int64_t after,
                      const core::Rules& rules = core::default_rules()) {
    using testchart::line;
    const std::vector<uint8_t> data = testchart::chart_bytes(
        testchart::section("ExpertDrums",
                           line(0, "N 1 0") + line(2 * res, "S 64 " + std::to_string(2 * res)) +
                               line(3 * res, "N 1 0") + line(4 * res + after, "N 1 0")),
        res);
    return activation_tick(load_songbytes_chart(data, ChartReading{Difficulty::Expert}, rules));
}

// The same shape as a .mid at 480 ticks per beat: chords at 0 and 1440, the
// fill (pitch 120) from 960 to 1920, and one more chord `after` ticks later.
int64_t mid_landing(uint8_t after) {
    REQUIRE(after < 0x80);  // one delta byte
    const std::vector<uint8_t> track = testmidi::concat({
        testmidi::track_name("PART DRUMS"),
        {0x00, 0x90, 97, 100},          // tick 0: red
        {0x87, 0x40, 0x90, 120, 100},   // tick 960: fill starts
        {0x83, 0x60, 0x90, 97, 100},    // tick 1440: red, inside the fill
        {0x83, 0x60, 0x80, 120, 0},     // tick 1920: fill ends
        {after, 0x90, 97, 100},         // tick 1920 + after: red
        testmidi::end_of_track(),
    });
    return activation_tick(load_songbytes_mid(testmidi::smf_drums(track), ChartReading{}));
}

// Whether the chord at `tick` carries a fill.
bool chord_has_fill(const Song& song, int64_t tick) {
    for (const SongTimestamp& ts : song.sequence)
        if (ts.timecode.ticks() == tick) return ts.has_activation();
    return false;
}

// An EVENTS track holding one text meta per (tick, text), in tick order.
std::vector<uint8_t> events_track(const std::vector<std::pair<uint32_t, std::string>>& texts) {
    std::vector<uint8_t> t = testmidi::track_name("EVENTS");
    uint32_t at = 0;
    for (const auto& [tick, text] : texts) {
        const std::vector<uint8_t> ev = testmidi::after(tick - at, testmidi::text_event(text));
        t.insert(t.end(), ev.begin(), ev.end());
        at = tick;
    }
    const std::vector<uint8_t> end = testmidi::end_of_track();
    t.insert(t.end(), end.begin(), end.end());
    return t;
}

// The coda shape as a .mid at 480 ticks per beat. Fill A (240 to 480) lands
// on the red at 360 and is never near the coda, so the chart always keeps a
// written fill and no generated fill can take a chord. Fill B (960 to 1920)
// lands on the blue at 1935 unless the coda skips it. The events track comes
// after the drum track, the order that fails if the coda is read too late.
Song mid_coda(const std::vector<std::pair<uint32_t, std::string>>& texts,
              FillDeadlineRule rule = kTestFillRule) {
    const std::vector<uint8_t> drums = testmidi::concat({
        testmidi::track_name("PART DRUMS"),
        {0x81, 0x70, 0x90, 120, 100},  // tick 240: fill A on
        {0x78, 0x90, 97, 100},         // tick 360: red
        {0x78, 0x90, 120, 0},          // tick 480: fill A off
        {0x83, 0x60, 0x90, 120, 100},  // tick 960: fill B on
        {0x83, 0x60, 0x90, 98, 100},   // tick 1440: yellow, inside B
        {0x83, 0x60, 0x90, 120, 0},    // tick 1920: fill B off
        {0x0F, 0x90, 99, 100},         // tick 1935: blue
        testmidi::end_of_track(),
    });
    const std::vector<uint8_t> timing =
        testmidi::concat({testmidi::set_tempo(), testmidi::end_of_track()});
    const Song song =
        load_songbytes_mid(testmidi::smf_tracks({timing, drums, events_track(texts)}),
                           ChartReading{}, core::default_rules(), rule);
    REQUIRE(chord_has_fill(song, 360));
    return song;
}

// The same shape as a .chart at 192 ticks per beat: fill A (48 to 96) lands
// on 72, fill B (384 to 768) on 774 unless the coda skips it. `before` and
// `after` are whole sections written before and after the drum section, and
// `in_drums` is extra drum-section lines at 384.
Song chart_coda(const std::string& before, const std::string& after,
                const std::string& in_drums = "") {
    using testchart::line;
    const std::string drums = testchart::section(
        "ExpertDrums", line(48, "S 64 48") + line(72, "N 1 0") + line(384, "S 64 384") +
                           in_drums + line(576, "N 2 0") + line(774, "N 3 0"));
    const Song song = load_songbytes_chart(testchart::chart_bytes(before + drums + after),
                                           ChartReading{Difficulty::Expert});
    REQUIRE(chord_has_fill(song, 72));
    return song;
}

std::string chart_events(const std::string& lines) {
    return testchart::section("Events", lines);
}

// mid_coda without fill A: fill B (960 to 1920) is the chart's only written
// fill, or there is none when `written_fill` is false. The blue at 1935 is 15
// ticks after the measure line at 1920, so the generator, when it runs, puts
// a fill on it.
Song mid_lone_fill(const std::vector<std::pair<uint32_t, std::string>>& texts,
                   FillDeadlineRule rule, bool written_fill = true) {
    const std::vector<uint8_t> drums =
        written_fill ? testmidi::concat({
                           testmidi::track_name("PART DRUMS"),
                           {0x87, 0x40, 0x90, 120, 100},  // tick 960: fill B on
                           {0x83, 0x60, 0x90, 98, 100},   // tick 1440: yellow, inside B
                           {0x83, 0x60, 0x90, 120, 0},    // tick 1920: fill B off
                           {0x0F, 0x90, 99, 100},         // tick 1935: blue
                           testmidi::end_of_track(),
                       })
                     : testmidi::concat({
                           testmidi::track_name("PART DRUMS"),
                           {0x8B, 0x20, 0x90, 98, 100},  // tick 1440: yellow
                           {0x83, 0x6F, 0x90, 99, 100},  // tick 1935: blue
                           testmidi::end_of_track(),
                       });
    const std::vector<uint8_t> timing =
        testmidi::concat({testmidi::set_tempo(), testmidi::end_of_track()});
    return load_songbytes_mid(testmidi::smf_tracks({timing, drums, events_track(texts)}),
                              ChartReading{}, core::default_rules(), rule);
}

// chart_coda without fill A: fill B (384 to 768) is the only written fill, or
// there is none when `written_fill` is false. The chord at 774 is 6 ticks
// after the measure line at 768, so the generator, when it runs, fills it.
Song chart_lone_fill(const std::string& before, FillDeadlineRule rule,
                     bool written_fill = true) {
    using testchart::line;
    const std::string drums = testchart::section(
        "ExpertDrums", (written_fill ? line(384, "S 64 384") : std::string()) +
                           line(576, "N 2 0") + line(774, "N 3 0"));
    return load_songbytes_chart(testchart::chart_bytes(before + drums),
                                ChartReading{Difficulty::Expert}, core::default_rules(), rule);
}

bool no_fill_at_all(const Song& song) {
    return activation_tick(song) == -1 && !song.generated_fills.has_value();
}

}  // namespace

TEST_CASE("fill landing: .chart window is trunc(res/32) + 1 ticks") {
    // Resolution 192: trunc(6.0) + 1 = 7. The fill ends at 768.
    CHECK(chart_landing(192, 6) == 774);
    CHECK(chart_landing(192, 7) == 775);  // the edge still lands
    CHECK(chart_landing(192, 8) == 576);  // too far: the earlier chord takes it
}

TEST_CASE("fill landing: .mid window at resolution 480 is 16 ticks (the Thrice case)") {
    CHECK(mid_landing(15) == 1935);
    CHECK(mid_landing(16) == 1936);
    CHECK(mid_landing(17) == 1440);
}

TEST_CASE("fill landing: the extra tick applies on top of a user's slop of 0") {
    core::Rules rules = core::default_rules();
    rules.fill_land_slop_beats = 0.0;
    CHECK(chart_landing(192, 1, rules) == 769);
    CHECK(chart_landing(192, 2, rules) == 576);
}

// Fill B (D30): fills are placed after every chord is read. A fill whose next
// fill starts before any chord reaches its end still lands, and so does the
// next one. Resolution 192: the window is 6 + 1 = 7 ticks.
TEST_CASE("fill landing: a fill is placed even when the next fill starts before its end chord (.chart)") {
    const std::vector<uint8_t> data = testchart::chart_bytes(testchart::section(
        "ExpertDrums",
        "  0 = N 0 0\n"
        "  100 = S 64 100\n"   // fill A: 100-200
        "  150 = N 1 0\n"      // A's only chord, inside A
        "  180 = S 64 100\n"   // fill B: 180-280, starts before a chord reaches 200
        "  250 = N 2 0\n"      // B's only chord, inside B
        "  400 = N 3 0\n"));
    const Song song = load_songbytes_chart(data, ChartReading{});
    REQUIRE(song.sequence.size() == 4);
    CHECK_FALSE(song.sequence[0].has_activation());
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 50);
    REQUIRE(song.sequence[2].has_activation());
    CHECK(*song.sequence[2].activation_length == 70);
    CHECK_FALSE(song.sequence[3].has_activation());
}

TEST_CASE("fill landing: a fill is placed even when the next fill starts before its end chord (.mid)") {
    // 480 ticks per beat: the window is 15 + 1 = 16 ticks.
    using namespace testmidi;
    const std::vector<uint8_t> track = concat({
        track_name("PART DRUMS"),
        note_on(96, 100),                // tick 0: kick
        {0x81, 0x70, 0x90, 120, 100},    // tick 240: fill A on
        {0x78, 0x90, 97, 100},           // tick 360: red, inside A
        {0x78, 0x90, 120, 0},            // tick 480: fill A off
        {0x14, 0x90, 120, 100},          // tick 500: fill B on, no chord since A's end
        {0x64, 0x90, 98, 100},           // tick 600: yellow, inside B
        {0x78, 0x90, 120, 0},            // tick 720: fill B off
        {0x83, 0x60, 0x90, 99, 100},     // tick 1200: blue
        end_of_track(),
    });
    const Song song = load_songbytes_mid(smf_drums(track), ChartReading{});
    REQUIRE(song.sequence.size() == 4);
    CHECK_FALSE(song.sequence[0].has_activation());
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 120);
    REQUIRE(song.sequence[2].has_activation());
    CHECK(*song.sequence[2].activation_length == 100);
    CHECK_FALSE(song.sequence[3].has_activation());
}

TEST_CASE("fill landing: an earlier chord before the fill start is no candidate") {
    // Fill 100-104. The chord at 99 is closer (5 ticks) but before the fill
    // start, so only the chord at 110 (6 ticks, inside the 7-tick window)
    // can take it.
    const std::vector<uint8_t> data = testchart::chart_bytes(testchart::section(
        "ExpertDrums",
        "  99 = N 1 0\n"
        "  100 = S 64 4\n"
        "  110 = N 2 0\n"));
    const Song song = load_songbytes_chart(data, ChartReading{});
    REQUIRE(song.sequence.size() == 2);
    CHECK_FALSE(song.sequence[0].has_activation());
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 10);
}

// D30: Clone Hero's one-side fallback (option B2) is not adopted.
TEST_CASE("fill landing: a fill before the first note keeps the window's bound, not Clone Hero's fallback") {
    // Fill 0-50 has no chord before its end, and the first chord after it
    // (192) is far outside the window, so it is dropped. Clone Hero's 0x5DE030
    // would take the 192 chord; D30 keeps the bound.
    const std::vector<uint8_t> data = testchart::chart_bytes(testchart::section(
        "ExpertDrums",
        "  0 = S 64 50\n"
        "  192 = N 0 0\n"
        "  300 = S 64 84\n"    // fill 300-384, lands on 384
        "  384 = N 1 0\n"));
    const Song song = load_songbytes_chart(data, ChartReading{});
    REQUIRE(song.sequence.size() == 2);
    CHECK_FALSE(song.sequence[0].has_activation());
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 84);
    CHECK(song.features.empty());  // an authored fill landed: no generated ones
}

// D30: Clone Hero's one-side fallback (option B2) is not adopted.
TEST_CASE("fill landing: a fill past the last note keeps its start bound, not Clone Hero's fallback") {
    // Fill 500-600 has no chord after it, and the last chord before its end
    // (384) is before its start, so it is dropped. Clone Hero's 0x5DE030 would
    // move 384's fill to it; D30 keeps 384's own fill, 300-384.
    const std::vector<uint8_t> data = testchart::chart_bytes(testchart::section(
        "ExpertDrums",
        "  0 = N 0 0\n"
        "  300 = S 64 84\n"
        "  384 = N 1 0\n"
        "  500 = S 64 100\n"));
    const Song song = load_songbytes_chart(data, ChartReading{});
    REQUIRE(song.sequence.size() == 2);
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 84);
}

// FL-08: a written fill that starts on or after the chart's first coda event
// never lands, as Clone Hero 1.1 marks it (place_authored_fills). Fill B
// starts at 960 (.mid) or 384 (.chart); the coda sits on it or one tick after.
TEST_CASE("fill landing: a .mid fill that starts on the coda tick never lands") {
    CHECK_FALSE(chord_has_fill(mid_coda({{960, "coda"}}), 1935));
    CHECK_FALSE(chord_has_fill(mid_coda({{960, "[coda]"}}), 1935));
}

TEST_CASE("fill landing: a .mid fill that starts one tick before the coda lands") {
    CHECK(chord_has_fill(mid_coda({{961, "coda"}}), 1935));
    CHECK(chord_has_fill(mid_coda({}), 1935));  // no coda: nothing is skipped
}

TEST_CASE("fill landing: a .mid coda is trimmed, then compared whole and case-sensitively") {
    CHECK_FALSE(chord_has_fill(mid_coda({{960, " coda "}}), 1935));
    CHECK(chord_has_fill(mid_coda({{960, "[CODA]"}}), 1935));
    CHECK(chord_has_fill(mid_coda({{960, "[coda"}}), 1935));  // 0x21544E0 compares whole
}

TEST_CASE("fill landing: only the first .mid coda counts") {
    CHECK_FALSE(chord_has_fill(mid_coda({{960, "coda"}, {2400, "coda"}}), 1935));
}

// Q-40: Clone Hero 1.0 skips only the fill on the coda tick.
TEST_CASE("fill landing: in 1.0 mode only a fill on the coda tick is skipped") {
    CHECK_FALSE(chord_has_fill(mid_coda({{959, "coda"}}, FillDeadlineRule::Ch11), 1935));
    CHECK(chord_has_fill(mid_coda({{959, "coda"}}, FillDeadlineRule::Ch10), 1935));
    CHECK_FALSE(chord_has_fill(mid_coda({{960, "coda"}}, FillDeadlineRule::Ch11), 1935));
    CHECK_FALSE(chord_has_fill(mid_coda({{960, "coda"}}, FillDeadlineRule::Ch10), 1935));
}

TEST_CASE("fill landing: a .chart fill that starts on the coda tick never lands") {
    using testchart::line;
    CHECK_FALSE(chord_has_fill(chart_coda(chart_events(line(384, "E \"coda\"")), ""), 774));
    CHECK_FALSE(chord_has_fill(chart_coda(chart_events(line(384, "E \"[coda]\"")), ""), 774));
    // [Events] after the drum section in the file.
    CHECK_FALSE(chord_has_fill(chart_coda("", chart_events(line(384, "E \"coda\""))), 774));
}

TEST_CASE("fill landing: a .chart fill that starts one tick before the coda lands") {
    using testchart::line;
    CHECK(chord_has_fill(chart_coda(chart_events(line(385, "E \"coda\"")), ""), 774));
    CHECK(chord_has_fill(chart_coda("", ""), 774));  // no coda: nothing is skipped
}

TEST_CASE("fill landing: a .chart coda is trimmed and loses a leading bracket, then compared exactly") {
    using testchart::line;
    CHECK_FALSE(chord_has_fill(chart_coda(chart_events(line(384, "E \" coda \"")), ""), 774));
    CHECK(chord_has_fill(chart_coda(chart_events(line(384, "E \"[CODA]\"")), ""), 774));
    // 0x2151830 takes off a leading '[' even with no ']' after it.
    CHECK_FALSE(chord_has_fill(chart_coda(chart_events(line(384, "E \"[coda\"")), ""), 774));
}

TEST_CASE("fill landing: only the first .chart coda counts, across [Events] sections") {
    using testchart::line;
    CHECK_FALSE(chord_has_fill(
        chart_coda(chart_events(line(384, "E \"coda\"") + line(2400, "E \"coda\"")), ""), 774));
    CHECK_FALSE(chord_has_fill(chart_coda(chart_events(line(384, "E \"coda\"")),
                                   chart_events(line(2400, "E \"coda\""))),
                        774));
}

TEST_CASE("fill landing: a coda event inside the drum section skips nothing") {
    CHECK(chord_has_fill(chart_coda("", "", testchart::line(384, "E coda")), 774));
}

// FL-27 and FL-08: a fill the coda skipped stays in Clone Hero's fill list, so
// the generator does not run (0x20D3375). A chart whose only written fill the
// coda skipped gets no fill at all, under both fill rules.
TEST_CASE("fill landing: a chart whose only written fill the coda skipped gets no generated fill") {
    for (const FillDeadlineRule rule : {FillDeadlineRule::Ch11, FillDeadlineRule::Ch10}) {
        CAPTURE(static_cast<int>(rule));
        CHECK(no_fill_at_all(mid_lone_fill({{960, "coda"}}, rule)));
        CHECK(no_fill_at_all(
            chart_lone_fill(chart_events(testchart::line(384, "E \"coda\"")), rule)));
    }
}

TEST_CASE("fill landing: a lone written fill one tick before the coda still lands") {
    for (const FillDeadlineRule rule : {FillDeadlineRule::Ch11, FillDeadlineRule::Ch10}) {
        CAPTURE(static_cast<int>(rule));
        const Song mid = mid_lone_fill({{961, "coda"}}, rule);
        CHECK(chord_has_fill(mid, 1935));
        CHECK_FALSE(mid.generated_fills.has_value());
        const Song chart =
            chart_lone_fill(chart_events(testchart::line(385, "E \"coda\"")), rule);
        CHECK(chord_has_fill(chart, 774));
        CHECK_FALSE(chart.generated_fills.has_value());
    }
}

// The same shapes with no written fill still generate one, so the case above
// tests the coda skip and not a chart the generator would leave empty.
TEST_CASE("fill landing: the lone-fill shapes with no written fill still generate") {
    for (const FillDeadlineRule rule : {FillDeadlineRule::Ch11, FillDeadlineRule::Ch10}) {
        CAPTURE(static_cast<int>(rule));
        const Song mid = mid_lone_fill({{960, "coda"}}, rule, false);
        CHECK(chord_has_fill(mid, 1935));
        CHECK(mid.generated_fills == rule);
        const Song chart =
            chart_lone_fill(chart_events(testchart::line(384, "E \"coda\"")), rule, false);
        CHECK(chord_has_fill(chart, 774));
        CHECK(chart.generated_fills == rule);
    }
}
