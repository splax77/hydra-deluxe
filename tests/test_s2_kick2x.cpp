// Step 2, T2 (D20, findings 10, 12 and 255): each difficulty reads its own 2x
// kick, and the Dynamics tab counts kicks by one rule.

#include "doctest.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "core/model.h"
#include "midi_util.h"
#include "parse/song.h"

#ifndef HYDRA_INPUT_DIR
#error "HYDRA_INPUT_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

TEST_CASE(".mid: each difficulty reads its own 2x kick pitch, as Clone Hero does") {
    // Clone Hero (0x2155050 at 0x21555CD) flags 59, 71, 83 and 95 as
    // DoubleKick, each in its own difficulty: one below that difficulty's kick.
    using namespace testmidi;
    std::vector<std::vector<uint8_t>> ev = {track_name("PART DRUMS"), set_tempo(),
                                            note_on(95, 100)};  // tick 0: Expert
    ev.push_back({0x40, 0x90, 83, 100});                       // tick 64: Hard
    ev.push_back({0x40, 0x90, 71, 100});                       // tick 128: Medium
    ev.push_back({0x40, 0x90, 59, 100});                       // tick 192: Easy
    ev.push_back(end_of_track());
    const std::vector<uint8_t> mid = smf(concat(ev));
    const std::pair<Difficulty, int64_t> cases[] = {{Difficulty::Expert, 0},
                                                    {Difficulty::Hard, 64},
                                                    {Difficulty::Medium, 128},
                                                    {Difficulty::Easy, 192}};
    for (const auto& [d, tick] : cases) {
        const std::string name = difficulty_name(d);
        CAPTURE(name);
        const Song on = load_songbytes_mid(mid, true, true, d);
        REQUIRE(on.sequence.size() == 1);
        CHECK(on.sequence[0].timecode.ticks() == tick);
        CHECK(on.sequence[0].chord.at(NoteColor::Kick)->is2x);
        CHECK(load_songbytes_mid(mid, true, false, d).sequence.empty());
    }
}

TEST_CASE(".mid: Expert's 2x kick never swallows a Hard kick on the same tick") {
    // Finding 255: two kicks on one tick keep whichever comes first in the
    // file. With the 95 first, Hard used to lose its real kick to it.
    using namespace testmidi;
    const std::vector<uint8_t> mid = smf(concat(
        {track_name("PART DRUMS"), set_tempo(), note_on(95, 100), note_on(84, 100),
         end_of_track()}));
    const Song hard = load_songbytes_mid(mid, true, /*bass2x=*/true, Difficulty::Hard);
    REQUIRE(hard.sequence.size() == 1);
    REQUIRE(hard.sequence[0].chord.at(NoteColor::Kick).has_value());
    CHECK_FALSE(hard.sequence[0].chord.at(NoteColor::Kick)->is2x);
}

TEST_CASE(".chart: N 32 is a 2x kick only in the section being read") {
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n  0 = N 32 0\n}\n"
        "[HardDrums]\n{\n  192 = N 1 0\n  384 = N 32 0\n}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    const Song on = load_songbytes_chart(data, true, true, Difficulty::Hard);
    REQUIRE(on.sequence.size() == 2);  // Expert's tick-0 N 32 is not Hard's
    CHECK(on.sequence[1].timecode.ticks() == 384);
    CHECK(on.sequence[1].chord.at(NoteColor::Kick)->is2x);
    CHECK(load_songbytes_chart(data, true, false, Difficulty::Hard).sequence.size() == 1);
}

TEST_CASE("Dynamics: one kick total follows the 2x Bass setting") {
    app::DynamicsBreakdown bd;
    bd.rows[static_cast<size_t>(app::DynamicsRow::RedSnare)] = {1, 2, 3};
    bd.rows[static_cast<size_t>(app::DynamicsRow::Kick)] = {0, 1, 40};
    bd.rows[static_cast<size_t>(app::DynamicsRow::Kick2x)] = {0, 0, 12};
    CHECK(bd.kicks_total(false).all() == 41);
    CHECK(bd.kicks_total(true).all() == 53);
    for (bool bass2x : {false, true}) {
        CAPTURE(bass2x);
        CHECK(bd.played_total(bass2x).all() ==
              bd.pads_total().all() + bd.kicks_total(bass2x).all());
        CHECK(bd.played_total(bass2x).accent ==
              bd.pads_total().accent + bd.kicks_total(bass2x).accent);
    }
}

TEST_CASE("Dynamics: Car Bomb - The Sentinel at Hard counts Hard's own kicks") {
    // Finding 10's chart. Hard has 1,147 kicks (pitch 84) and no 2x kicks of
    // its own; Expert's 95 shares a tick with 409 of them. Before D20 the
    // Dynamics parse at Hard showed 828 kicks plus 319 "2x kicks" (the 95 came
    // first on those ticks), and Totals said 1,719 instead of 2,038.
    using app::DynamicsRow;
    const std::string path =
        std::string(HYDRA_INPUT_DIR) + "/common/IB24/T7/Car Bomb - The Sentinel/notes.mid";
    const app::DynamicsBreakdown bd = app::count_dynamics(
        load_songpath(path, /*pro=*/true, app::kDynamicsParseBass2x, Difficulty::Hard));
    CHECK(bd.row(DynamicsRow::Kick).all() == 1147);
    CHECK(bd.row(DynamicsRow::Kick2x).all() == 0);
    CHECK(bd.played_total(true).all() == 2038);
    CHECK(bd.played_total(false).all() == 2038);
}
