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

// FL-03, the game's rule: a written fill may land on a chord up to
// trunc(resolution x slop) + 1 ticks after its end, and the game's slop is a
// fixed 1/32 of a beat. Hydra's place_authored_fills keeps its window private,
// so the tests state the game's rule here and check Hydra against it.
constexpr double kGameSlopBeats = 1.0 / 32;

int64_t game_landing_window(int64_t res, double slop_beats) {
    // static_cast truncates toward zero, as the game's cvttsd2si does.
    return static_cast<int64_t>(static_cast<double>(res) * slop_beats) + 1;
}

// The window shape both readers' tests build, in beats of `res` ticks: a chord
// at 0, a fill from beat 2 to beat 4, a chord inside the fill at beat 3, and
// one more chord some ticks past the fill end.
int64_t window_fill_start(int64_t res) { return 2 * res; }
int64_t window_inner_chord(int64_t res) { return 3 * res; }
int64_t window_fill_end(int64_t res) { return 4 * res; }

// The window shape as a .chart at `res` ticks per beat, with the last chord
// `past_end` ticks past the fill end. Returns the tick of the chord the fill
// landed on.
int64_t chart_landing(int64_t res, int64_t past_end,
                      const core::Rules& rules = core::default_rules()) {
    using testchart::line;
    const int64_t fill_start = window_fill_start(res);
    const std::vector<uint8_t> data = testchart::chart_bytes(
        testchart::section(
            "ExpertDrums",
            line(0, "N 1 0") +
                line(fill_start,
                     "S 64 " + std::to_string(window_fill_end(res) - fill_start)) +
                line(window_inner_chord(res), "N 1 0") +
                line(window_fill_end(res) + past_end, "N 1 0")),
        res);
    return activation_tick(load_songbytes_chart(data, ChartReading{Difficulty::Expert}, rules));
}

// Delta-0 events placed at absolute ticks, in tick order: each event's zero
// delta becomes the distance from the event before it.
std::vector<uint8_t> timed_events(
    const std::vector<std::pair<uint32_t, std::vector<uint8_t>>>& events) {
    std::vector<uint8_t> t;
    uint32_t at = 0;
    for (const auto& [tick, event] : events) {
        const std::vector<uint8_t> ev = testmidi::after(tick - at, event);
        t.insert(t.end(), ev.begin(), ev.end());
        at = tick;
    }
    return t;
}

// The .mid note numbers these tests write: a fill marker and three pads.
constexpr uint8_t kMidFill = 120;
constexpr uint8_t kMidKick = 96;
constexpr uint8_t kMidRed = 97;
constexpr uint8_t kMidYellow = 98;
constexpr uint8_t kMidBlue = 99;

std::vector<uint8_t> hit(uint8_t note) { return testmidi::note_on(note, 100); }
// A zero-velocity note-on, the note-off most .mid files write.
std::vector<uint8_t> release(uint8_t note) { return testmidi::note_on(note, 0); }

// The .mid division testmidi::smf_tracks writes. Each .mid test REQUIREs the
// parsed song reports it, so the positions below follow the real file.
constexpr int64_t kMidRes = 480;

// The window shape as a .mid, with the last red `past_end` ticks past the
// fill end. Returns the tick of the chord the fill landed on.
int64_t mid_landing(int64_t past_end) {
    const auto at = [](int64_t tick) { return static_cast<uint32_t>(tick); };
    const std::vector<uint8_t> track = testmidi::concat({
        testmidi::track_name("PART DRUMS"),
        timed_events({
            {0, hit(kMidRed)},
            {at(window_fill_start(kMidRes)), hit(kMidFill)},
            {at(window_inner_chord(kMidRes)), hit(kMidRed)},
            {at(window_fill_end(kMidRes)), testmidi::note_off(kMidFill)},
            {at(window_fill_end(kMidRes) + past_end), hit(kMidRed)},
        }),
        testmidi::end_of_track(),
    });
    const Song song = load_songbytes_mid(testmidi::smf_drums(track), ChartReading{});
    REQUIRE(song.tick_resolution() == kMidRes);
    return activation_tick(song);
}

// Lands the window shape's fill with its last chord one tick inside the
// window, on its edge and one tick past it, and checks each against FL-03:
// inside or on the edge the last chord takes the fill (it is closer than the
// inner chord); past the edge the inner chord, at or before the end, takes it.
template <class Landing>
void check_window_edges(Landing landing, int64_t res, int64_t window) {
    REQUIRE(window <= window_fill_end(res) - window_inner_chord(res));
    for (int64_t past_end = window - 1; past_end <= window + 1; ++past_end) {
        const int64_t expected = past_end <= window ? window_fill_end(res) + past_end
                                                    : window_inner_chord(res);
        const std::string msg = "last chord " + std::to_string(past_end) +
                                " ticks past the fill end, window " + std::to_string(window);
        CHECK_MESSAGE(landing(past_end) == expected, msg);
    }
}

// A written fill and the chord a test expects it to land on.
struct ExpectedLanding {
    int64_t fill_start;
    int64_t chord;
};

// Checks the song's chords are exactly `chords`, in order, that each expected
// landing chord carries a fill running from its fill's start to the chord,
// and that no other chord carries one.
void check_landings(const Song& song, const std::vector<int64_t>& chords,
                    const std::vector<ExpectedLanding>& landings) {
    REQUIRE(song.sequence.size() == chords.size());
    for (size_t i = 0; i < chords.size(); ++i) {
        const SongTimestamp& ts = song.sequence[i];
        REQUIRE(ts.timecode.ticks() == chords[i]);
        const std::string msg = "chord at tick " + std::to_string(chords[i]);
        INFO(msg);
        const ExpectedLanding* want = nullptr;
        for (const ExpectedLanding& l : landings)
            if (l.chord == chords[i]) want = &l;
        if (want == nullptr) {
            CHECK_FALSE(ts.has_activation());
            continue;
        }
        REQUIRE(ts.has_activation());
        CHECK(*ts.activation_length == want->chord - want->fill_start);
    }
}

// Whether the chord at `tick` carries a fill.
bool chord_has_fill(const Song& song, int64_t tick) {
    for (const SongTimestamp& ts : song.sequence)
        if (ts.timecode.ticks() == tick) return ts.has_activation();
    return false;
}

// An EVENTS track holding one text meta per (tick, text), in tick order.
std::vector<uint8_t> events_track(const std::vector<std::pair<uint32_t, std::string>>& texts) {
    std::vector<std::pair<uint32_t, std::vector<uint8_t>>> events;
    for (const auto& [tick, text] : texts) events.emplace_back(tick, testmidi::text_event(text));
    return testmidi::concat(
        {testmidi::track_name("EVENTS"), timed_events(events), testmidi::end_of_track()});
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
    const int64_t res = 192;
    check_window_edges([res](int64_t past_end) { return chart_landing(res, past_end); }, res,
                       game_landing_window(res, kGameSlopBeats));
}

TEST_CASE("fill landing: .mid window at resolution 480 is 16 ticks (the Thrice case)") {
    check_window_edges(mid_landing, kMidRes, game_landing_window(kMidRes, kGameSlopBeats));
}

TEST_CASE("fill landing: the extra tick applies on top of a user's slop of 0") {
    const int64_t res = 192;
    core::Rules rules = core::default_rules();
    rules.fill_land_slop_beats = 0.0;
    check_window_edges(
        [res, &rules](int64_t past_end) { return chart_landing(res, past_end, rules); }, res,
        game_landing_window(res, rules.fill_land_slop_beats));
}

// Fill B (D30): fills are placed after every chord is read. A fill whose next
// fill starts before any chord reaches its end still lands, and so does the
// next one.
TEST_CASE("fill landing: a fill is placed even when the next fill starts before its end chord (.chart)") {
    using testchart::line;
    const int64_t fill_length = 100;
    const int64_t a_start = 100, a_chord = 150;  // A's only chord, inside A
    const int64_t b_start = 180, b_chord = 250;  // B's only chord, inside B
    const int64_t last_chord = 400;
    // B starts before A ends, and before any chord past A's end.
    REQUIRE(b_start < a_start + fill_length);
    REQUIRE(a_chord < b_start);
    const std::string fill = "S 64 " + std::to_string(fill_length);
    const std::vector<uint8_t> data = testchart::chart_bytes(testchart::section(
        "ExpertDrums", line(0, "N 0 0") + line(a_start, fill) + line(a_chord, "N 1 0") +
                           line(b_start, fill) + line(b_chord, "N 2 0") +
                           line(last_chord, "N 3 0")));
    check_landings(load_songbytes_chart(data, ChartReading{}), {0, a_chord, b_chord, last_chord},
                   {{a_start, a_chord}, {b_start, b_chord}});
}

TEST_CASE("fill landing: a fill is placed even when the next fill starts before its end chord (.mid)") {
    const uint32_t a_start = 240, a_chord = 360, a_end = 480;
    const uint32_t b_start = 500, b_chord = 600, b_end = 720;
    const uint32_t last_chord = 1200;
    // B starts before any chord past A's end.
    REQUIRE(a_chord < a_end);
    REQUIRE(a_end < b_start);
    REQUIRE(b_start < b_chord);
    const std::vector<uint8_t> track = testmidi::concat({
        testmidi::track_name("PART DRUMS"),
        timed_events({
            {0, hit(kMidKick)},
            {a_start, hit(kMidFill)},
            {a_chord, hit(kMidRed)},
            {a_end, release(kMidFill)},
            {b_start, hit(kMidFill)},
            {b_chord, hit(kMidYellow)},
            {b_end, release(kMidFill)},
            {last_chord, hit(kMidBlue)},
        }),
        testmidi::end_of_track(),
    });
    const Song song = load_songbytes_mid(testmidi::smf_drums(track), ChartReading{});
    REQUIRE(song.tick_resolution() == kMidRes);
    check_landings(song, {0, a_chord, b_chord, last_chord},
                   {{a_start, a_chord}, {b_start, b_chord}});
}

TEST_CASE("fill landing: an earlier chord before the fill start is no candidate") {
    using testchart::line;
    const int64_t res = 192;
    const int64_t fill_start = 100, fill_length = 4;
    const int64_t fill_end = fill_start + fill_length;
    const int64_t early_chord = fill_start - 1;
    const int64_t late_chord = fill_end + game_landing_window(res, kGameSlopBeats) - 1;
    // The early chord is the closer one, so only its place before the fill
    // start keeps it from the fill.
    REQUIRE(fill_end - early_chord < late_chord - fill_end);
    const std::vector<uint8_t> data = testchart::chart_bytes(
        testchart::section("ExpertDrums",
                           line(early_chord, "N 1 0") +
                               line(fill_start, "S 64 " + std::to_string(fill_length)) +
                               line(late_chord, "N 2 0")),
        res);
    check_landings(load_songbytes_chart(data, ChartReading{}), {early_chord, late_chord},
                   {{fill_start, late_chord}});
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
