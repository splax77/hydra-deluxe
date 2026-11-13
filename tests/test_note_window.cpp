// The game's per-note hit window and the Dynamic Hit Window row
// (core/note_window.h). The plan is
// docs/superpowers/plans/2026-11-12-dynamic-hit-window.md, task T1.

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "core/model.h"
#include "core/note_window.h"
#include "core/song.h"
#include "source_tree.h"  // for_each_source_file, the shared walk of the repo

using namespace hydra;
namespace fs = std::filesystem;

namespace {

// A plain chart at 120 BPM, 192 ticks a beat (2.604 ms a tick), with one Red
// chord on each tick of `ticks`.
Song plain_song(const std::vector<int64_t>& ticks) {
    Song song(192);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t tick : ticks) {
        SongTimestamp& ts = song.sequence.emplace_back();
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
    }
    return song;
}

// A side at the floor and at the cap: half of TM-21's whole window at a
// summed gap of 75 and of 170 (TM-22 prints them as 48.15 and 85.72).
const doctest::Approx kFloorSide = doctest::Approx(96.2931672 / 2);
const doctest::Approx kCapSide = doctest::Approx(171.4313075 / 2);

// A literal pinned to more places than the plan prints.
doctest::Approx pinned(double v) { return doctest::Approx(v).epsilon(1e-9); }

// Chords 768 ticks (2,000 ms) apart, so every side is the cap. The
// activation is on 768; a SqIn set the step on 2304 and a SqOut squeezed out
// the chord on 3840. Each scale's two sides differ, so reading the wrong one
// shows.
struct TwoSqueezes {
    Song song;
    Activation act;
};
TwoSqueezes two_squeezes() {
    TwoSqueezes t{plain_song({0, 768, 1536, 2304, 3072, 3840}), Activation{}};
    Activation& a = t.act;
    a.timecode = t.song.timecode(768);
    a.bank_rise_ticks = {0, 768};  // two bars, so the ready point is known
    a.sp_end_steps = {{768, 3000, SpEndKind::Activation}, {2304, 3900, SpEndKind::SqIn}};
    SPSqueeze in{};
    in.kind = SqueezeKind::SqIn;
    in.offset_ms = 20.0;
    in.transfer = TransferScale{3.0, 1.5};
    SPSqueeze out{};
    out.kind = SqueezeKind::SqOut;
    out.offset_ms = -100.0;
    a.sqinouts = {in, out};
    a.sqout_tick = 3840;
    a.transfer_post = TransferScale{2.0, 4.0};
    return t;
}

}  // namespace

TEST_CASE("note window: the window formula and clamps are pinned") {
    // TM-21's two end values, as the plan pins them.
    CHECK(whole_window_ms(170.0) == pinned(171.4313075));
    CHECK(whole_window_ms(75.0) == pinned(96.2931672));
    // TM-21's three worked values.
    CHECK(whole_window_ms(170.0) == doctest::Approx(171.4313).epsilon(1e-6));
    CHECK(whole_window_ms(75.0) == doctest::Approx(96.2932).epsilon(1e-6));
    CHECK(whole_window_ms(80.0) == doctest::Approx(102.2586).epsilon(1e-6));
    // TM-22: the floor and the cap of a side.
    CHECK(window_side_ms(1.0, 1.0) == kFloorSide);
    CHECK(window_side_ms(10000.0, 10000.0) == kCapSide);
    // TM-19: a missing neighbour, or one at the same time, gives the back.
    CHECK(half_gap_ms(std::nullopt) == 85.0);
    CHECK(half_gap_ms(0.0) == 85.0);
    CHECK(half_gap_ms(10.0) == 37.5);
    CHECK(half_gap_ms(100.0) == 50.0);
    // Freak's chord 263: 56.18 ms from both neighbours.
    CHECK(window_side_ms(56.18, 56.18) == kFloorSide);
}

TEST_CASE("note window: the formula constants live in one file") {
    // The two formula constants, as the plan's search found them. Only C++
    // under src/ and tools/ counts: tools/ch_probe/SPEC.md and the schedule
    // files under tools/ch_probe/tests/ carry them as prose and data.
    int files = 0;
    std::set<std::string> holders;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
        if (rel.rfind("src/", 0) != 0 && rel.rfind("tools/", 0) != 0) return;
        const std::string ext = path.extension().string();
        if (ext != ".h" && ext != ".cpp") return;
        ++files;
        std::ifstream in(path, std::ios::binary);
        const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        for (const char* needle : {"0110924", "2.628539"})
            if (text.find(needle) != std::string::npos) holders.insert(rel);
    });
    CHECK(files > 0);
    CHECK(holders == std::set<std::string>{"src/core/note_window.h"});
}

TEST_CASE("note window: chord_times lists the song's chords") {
    const Song song = plain_song({0, 20, 40, 2000});
    const ChordTimes chords = chord_times(song);
    REQUIRE(chords.ticks.size() == 4);
    REQUIRE(chords.ms.size() == 4);
    REQUIRE(chords.sides_ms.size() == 4);
    CHECK(chords.ticks[1] == 20);
    CHECK(chords.ms[3] == song.timecode(2000).ms());
    CHECK(chords.sides_ms == chord_sides_ms(chords.ms));
    CHECK(chords.sides_ms[1] == kFloorSide);  // 52.08 ms from both neighbours
    CHECK(chords.at(40) == std::optional<int>(2));
    CHECK_FALSE(chords.at(41).has_value());
    CHECK_FALSE(chords.at(5000).has_value());
}

TEST_CASE("note window: the squeeze-out sums from the mockup") {
    // Dead on Arrival (O), activation 4: room to spare. Prints as 24.1.
    const double doa = squeeze_spare_ms(85.72, 1.033, 79.41, 143.88);
    CHECK(doa == pinned(24.07876));
    CHECK_FALSE(spare_is_short(doa));
    // Tapped Out, activation 4: out of reach. Prints as 65.9 short.
    const double tapped = squeeze_spare_ms(77.85, 1.0014, 71.97, 215.8);
    CHECK(tapped == pinned(-65.87101));
    CHECK(spare_is_short(tapped));
}

TEST_CASE("note window: exactly 0 is spare, anything below is short") {
    CHECK_FALSE(spare_is_short(0.0));
    CHECK(spare_is_short(std::nextafter(0.0, -1.0)));
    WindowRow row;
    row.spare_ms = 0.0;
    CHECK_FALSE(row.is_short());
}

TEST_CASE("note window: each squeeze reads its own side of its own scale") {
    const TwoSqueezes t = two_squeezes();
    const ChordTimes chords = chord_times(t.song);
    CHECK(t.act.squeeze_chord_tick(0) == std::optional<int64_t>(2304));
    CHECK(t.act.squeeze_chord_tick(1) == std::optional<int64_t>(3840));
    CHECK_FALSE(t.act.squeeze_chord_tick(2).has_value());

    // The SqOut: the early side of transfer_post (2.0, not 4.0).
    const std::optional<WindowRow> out = squeeze_out_window(t.act, 1, chords);
    REQUIRE(out.has_value());
    CHECK(out->first_side_ms == kCapSide);
    REQUIRE(out->second_side_ms.has_value());
    CHECK(*out->second_side_ms == kCapSide);
    CHECK(out->spare_ms == pinned(157.14696125214215));

    // The SqIn: the late side of its own transfer (1.5, not 3.0), never
    // transfer_post's.
    const std::optional<WindowRow> in = squeeze_in_window(t.act, 0, chords);
    REQUIRE(in.has_value());
    CHECK(in->first_side_ms == kCapSide);
    REQUIRE(in->second_side_ms.has_value());
    CHECK(in->spare_ms == pinned(194.2891343767851));

    // Each function answers only for its own kind.
    CHECK_FALSE(squeeze_out_window(t.act, 0, chords).has_value());
    CHECK_FALSE(squeeze_in_window(t.act, 1, chords).has_value());
    CHECK_FALSE(squeeze_out_window(t.act, 2, chords).has_value());
}

TEST_CASE("note window: a cap-clamped squeeze is anchored on the overfill chord") {
    // The chord on 1536 has a neighbour 20 ticks (52.08 ms) after it, so its
    // side is not the activation chord's.
    const Song song = plain_song({0, 768, 1536, 1556, 3072});
    const ChordTimes chords = chord_times(song);
    Activation act;
    act.timecode = song.timecode(768);
    act.sp_end_steps = {{768, 3000, SpEndKind::Activation}, {1536, 3082, SpEndKind::Clamped}};
    SPSqueeze out{};
    out.kind = SqueezeKind::SqOut;
    out.offset_ms = -26.04;
    act.sqinouts.push_back(out);
    act.sqout_tick = 3072;
    act.transfer_post = TransferScale{1.0, 1.0};

    REQUIRE(act.squeeze_anchor_tick(0) == std::optional<int64_t>(1536));
    const double overfill_side = chords.sides_ms[static_cast<size_t>(*chords.at(1536))];
    const double activation_side = chords.sides_ms[static_cast<size_t>(*chords.at(768))];
    REQUIRE(overfill_side != activation_side);
    const std::optional<WindowRow> row = squeeze_out_window(act, 0, chords);
    REQUIRE(row.has_value());
    CHECK(row->first_side_ms == overfill_side);
}

TEST_CASE("note window: the early fill row reads the SP-ready note") {
    TwoSqueezes t = two_squeezes();
    const ChordTimes chords = chord_times(t.song);
    t.act.e_offset = -10.0;  // Star Power ready 10 ms after the deadline
    const std::optional<WindowRow> row = early_fill_window(t.act, chords);
    REQUIRE(row.has_value());
    CHECK(row->first_side_ms == kCapSide);
    CHECK_FALSE(row->second_side_ms.has_value());
    CHECK(row->spare_ms == pinned(75.71565375071405));
}

TEST_CASE("note window: a row the record cannot name is left out") {
    const Song song = plain_song({0, 768, 1536, 2304, 3072, 3840});
    const ChordTimes chords = chord_times(song);

    SUBCASE("an unset scale") {
        TwoSqueezes t = two_squeezes();
        t.act.transfer_post.reset();
        CHECK_FALSE(squeeze_out_window(t.act, 1, chords).has_value());
        t.act.sqinouts[0].transfer.reset();
        CHECK_FALSE(squeeze_in_window(t.act, 0, chords).has_value());
    }
    SUBCASE("a squeeze that names no chord") {
        TwoSqueezes t = two_squeezes();
        t.act.sqout_tick.reset();
        CHECK_FALSE(squeeze_out_window(t.act, 1, chords).has_value());
    }
    SUBCASE("a squeeze with no end in the record") {
        TwoSqueezes t = two_squeezes();
        t.act.sp_end_steps.clear();
        CHECK_FALSE(squeeze_out_window(t.act, 1, chords).has_value());
        CHECK_FALSE(squeeze_in_window(t.act, 0, chords).has_value());
    }
    SUBCASE("a tick with no chord on it") {
        TwoSqueezes t = two_squeezes();
        t.act.sqout_tick = 3000;
        CHECK_FALSE(squeeze_out_window(t.act, 1, chords).has_value());
        t.act.bank_rise_ticks = {0, 3000};  // the bar arrived where no chord sits
        CHECK_FALSE(early_fill_window(t.act, chords).has_value());
    }
    SUBCASE("fewer stored bar arrivals than the activation level") {
        TwoSqueezes t = two_squeezes();
        t.act.bank_rise_ticks = {0};
        CHECK_FALSE(early_fill_window(t.act, chords).has_value());
    }
}
