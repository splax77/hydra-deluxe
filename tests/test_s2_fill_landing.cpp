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
