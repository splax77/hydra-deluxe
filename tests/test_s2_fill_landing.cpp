// Step 2, T4 (finding 315, D22 and D30): an authored fill lands on the next
// chord up to trunc(resolution x slop) + 1 ticks after the fill ends, as Clone
// Hero 1.1 computes it (cvttsd2si at 0x20D0088, inc at 0x20D008D). A chord
// exactly at the window's edge still lands. Each fill is placed on its own
// once every chord is read (0x20CFF60 calling 0x5DE030).

#include "doctest.h"

#include <cstdint>
#include <string>
#include <vector>

#include "core/rules.h"
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
    const auto line = [](int64_t tick, const std::string& what) {
        return "  " + std::to_string(tick) + " = " + what + "\n";
    };
    const std::string text =
        "[Song]\n{\n  Resolution = " + std::to_string(res) + "\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n" +
        line(0, "N 1 0") + line(2 * res, "S 64 " + std::to_string(2 * res)) +
        line(3 * res, "N 1 0") + line(4 * res + after, "N 1 0") + "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    return activation_tick(load_songbytes_chart(data, true, true, Difficulty::Expert, rules));
}

// The same shape as a .mid at 480 ticks per beat: chords at 0 and 1440, the
// fill (pitch 120) from 960 to 1920, and one more chord `after` ticks later.
int64_t mid_landing(uint8_t after) {
    REQUIRE(after < 0x80);  // one delta byte
    const std::vector<uint8_t> track = testmidi::concat({
        testmidi::track_name("PART DRUMS"), testmidi::set_tempo(),
        {0x00, 0x90, 97, 100},          // tick 0: red
        {0x87, 0x40, 0x90, 120, 100},   // tick 960: fill starts
        {0x83, 0x60, 0x90, 97, 100},    // tick 1440: red, inside the fill
        {0x83, 0x60, 0x80, 120, 0},     // tick 1920: fill ends
        {after, 0x90, 97, 100},         // tick 1920 + after: red
        testmidi::end_of_track(),
    });
    return activation_tick(load_songbytes_mid(testmidi::smf(track), true, true));
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
// fill starts before any chord reaches its end used to be forgotten; now both
// land. Resolution 192: the window is 6 + 1 = 7 ticks.
TEST_CASE("fill landing: a fill is placed even when the next fill starts before its end chord (.chart)") {
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n"
        "  0 = N 0 0\n"
        "  100 = S 64 100\n"   // fill A: 100-200
        "  150 = N 1 0\n"      // A's only chord, inside A
        "  180 = S 64 100\n"   // fill B: 180-280, starts before a chord reaches 200
        "  250 = N 2 0\n"      // B's only chord, inside B
        "  400 = N 3 0\n"
        "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    const Song song = load_songbytes_chart(data, true, true);
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
        track_name("PART DRUMS"), set_tempo(),
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
    const Song song = load_songbytes_mid(smf(track), true, true);
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
    // can take it. Before D30 the fill was dropped.
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n"
        "  99 = N 1 0\n"
        "  100 = S 64 4\n"
        "  110 = N 2 0\n"
        "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    const Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.sequence.size() == 2);
    CHECK_FALSE(song.sequence[0].has_activation());
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 10);
}

TEST_CASE("fill landing: a fill before the first note keeps the window's bound (B2 not adopted)") {
    // Fill 0-50 has no chord before its end, and the first chord after it
    // (192) is far outside the window, so it is dropped. Clone Hero's 0x5DE030
    // would take the 192 chord; D30 keeps the bound.
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n"
        "  0 = S 64 50\n"
        "  192 = N 0 0\n"
        "  300 = S 64 84\n"    // fill 300-384, lands on 384
        "  384 = N 1 0\n"
        "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    const Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.sequence.size() == 2);
    CHECK_FALSE(song.sequence[0].has_activation());
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 84);
    CHECK(song.features.empty());  // an authored fill landed: no generated ones
}

TEST_CASE("fill landing: a fill past the last note keeps its start bound (B2 not adopted)") {
    // Fill 500-600 has no chord after it, and the last chord before its end
    // (384) is before its start, so it is dropped. Clone Hero's 0x5DE030 would
    // move 384's fill to it; D30 keeps 384's own fill, 300-384.
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n"
        "  0 = N 0 0\n"
        "  300 = S 64 84\n"
        "  384 = N 1 0\n"
        "  500 = S 64 100\n"
        "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    const Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.sequence.size() == 2);
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 84);
}
