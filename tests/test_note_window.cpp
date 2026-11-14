// The game's per-note hit window and the Dynamic Hit Window row
// (core/note_window.h).

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
#include "record_fixtures.h"  // test::build_fixture_song
#include "source_tree.h"  // for_each_source_file, the shared walk of the repo

using namespace hydra;
namespace fs = std::filesystem;

namespace {

// A chart at 120 BPM, 192 ticks a beat (2.604 ms a tick), with one Red chord
// on each tick of `ticks` (test::build_fixture_song).
Song red_chords(const std::vector<int64_t>& ticks) {
    std::vector<test::FixtureNote> notes;
    for (int64_t tick : ticks) notes.push_back({tick});
    return test::build_fixture_song(192, 120.0, notes);
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
    TwoSqueezes t{red_chords({0, 768, 1536, 2304, 3072, 3840}), Activation{}};
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
    const Song song = red_chords({0, 20, 40, 2000});
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

TEST_CASE("note window: chord_times carries the chord at each place") {
    Song song = red_chords({0, 20, 40});
    song.sequence[1].chord.add_note(NoteColor::Kick);
    const ChordTimes chords = chord_times(song);
    REQUIRE(chords.chords.size() == 3);
    CHECK(chords.chords[0].notationstr() == "[ R   ]");
    CHECK(chords.chords[1].notationstr() == "[KR   ]");
    // One lookup by tick gives both the side and the chord.
    const std::optional<int> c = chords.at(20);
    REQUIRE(c.has_value());
    CHECK(chords.chords[static_cast<size_t>(*c)].notationstr() == "[KR   ]");
}

TEST_CASE("note window: the squeeze-out sums from the mockup") {
    // Dead on Arrival (O), activation 4: room to spare. Prints as 24.1.
    const double doa = squeeze_safe_ms(85.72, 1.033, 79.41, 143.88);
    CHECK(doa == pinned(24.07876));
    CHECK_FALSE(safe_is_short(doa));
    // Tapped Out, activation 4: out of reach. Prints as 65.9 short.
    const double tapped = squeeze_safe_ms(77.85, 1.0014, 71.97, 215.8);
    CHECK(tapped == pinned(-65.87101));
    CHECK(safe_is_short(tapped));
}

TEST_CASE("note window: exactly 0 is safe, anything below is short") {
    CHECK_FALSE(safe_is_short(0.0));
    CHECK(safe_is_short(std::nextafter(0.0, -1.0)));
    WindowEnds row;
    row.safe_ms = 0.0;
    CHECK_FALSE(row.is_short());
    row.safe_ms = -0.04;
    CHECK(row.is_short());
}

TEST_CASE("note window: each squeeze reads its own side of its own scale") {
    const TwoSqueezes t = two_squeezes();
    const ChordTimes chords = chord_times(t.song);
    CHECK(t.act.squeeze_chord_tick(0) == std::optional<int64_t>(2304));
    CHECK(t.act.squeeze_chord_tick(1) == std::optional<int64_t>(3840));
    CHECK_FALSE(t.act.squeeze_chord_tick(2).has_value());

    // The SqOut, anchor on the frontend: the early side of transfer_post
    // (2.0, not 4.0).
    const std::optional<WindowEnds> out = squeeze_window_ends(t.act, 1, chords);
    REQUIRE(out.has_value());
    REQUIRE(out->ends.size() == 2);
    CHECK(out->ends[0].side_ms == kCapSide);
    CHECK(out->ends[1].side_ms == kCapSide);
    CHECK(out->safe_ms == pinned(157.14696125214215));

    // The SqIn, anchor on the backend: the late side of its own transfer
    // (1.5, not 3.0), never transfer_post's.
    const std::optional<WindowEnds> in = squeeze_window_ends(t.act, 0, chords);
    REQUIRE(in.has_value());
    REQUIRE(in->ends.size() == 2);
    CHECK(in->ends[1].side_ms == kCapSide);
    CHECK(in->safe_ms == pinned(194.2891343767851));

    // A place past the activation's squeezes has no row.
    CHECK_FALSE(squeeze_window_ends(t.act, 2, chords).has_value());
}

TEST_CASE("note window: a cap-clamped squeeze is anchored on the overfill chord") {
    // The chord on 1536 has a neighbour 20 ticks (52.08 ms) after it, so its
    // side is not the activation chord's.
    const Song song = red_chords({0, 768, 1536, 1556, 3072});
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
    // The squeeze out has a timing, so its anchor is the frontend end.
    const std::optional<WindowEnds> row = squeeze_window_ends(act, 0, chords);
    REQUIRE(row.has_value());
    REQUIRE(row->ends.size() == 2);
    CHECK(row->ends[0].end == NoteEnd::Frontend);
    CHECK(row->ends[0].tick == 1536);
    CHECK(row->ends[0].side_ms == overfill_side);
}

TEST_CASE("note window: a row the record cannot name is left out") {
    const Song song = red_chords({0, 768, 1536, 2304, 3072, 3840});
    const ChordTimes chords = chord_times(song);

    SUBCASE("an unset scale") {
        TwoSqueezes t = two_squeezes();
        t.act.transfer_post.reset();
        CHECK_FALSE(squeeze_window_ends(t.act, 1, chords).has_value());
        t.act.sqinouts[0].transfer.reset();
        CHECK_FALSE(squeeze_window_ends(t.act, 0, chords).has_value());
    }
    SUBCASE("a squeeze that names no chord") {
        TwoSqueezes t = two_squeezes();
        t.act.sqout_tick.reset();
        CHECK_FALSE(squeeze_window_ends(t.act, 1, chords).has_value());
    }
    SUBCASE("a squeeze with no end in the record") {
        TwoSqueezes t = two_squeezes();
        t.act.sp_end_steps.clear();
        CHECK_FALSE(squeeze_window_ends(t.act, 1, chords).has_value());
        CHECK_FALSE(squeeze_window_ends(t.act, 0, chords).has_value());
    }
    SUBCASE("a tick with no chord on it") {
        TwoSqueezes t = two_squeezes();
        t.act.sqout_tick = 3000;
        CHECK_FALSE(squeeze_window_ends(t.act, 1, chords).has_value());
        t.act.bank_rise_ticks = {0, 3000};  // the bar arrived where no chord sits
        CHECK_FALSE(early_fill_window_end(t.act, chords).has_value());
    }
    SUBCASE("fewer stored bar arrivals than the activation level") {
        TwoSqueezes t = two_squeezes();
        t.act.bank_rise_ticks = {0};
        CHECK_FALSE(early_fill_window_end(t.act, chords).has_value());
    }
}

// ---- the one owner of which end each note is on ---------------------------

namespace {

SPSqueeze squeeze_at(SqueezeKind kind, double offset_ms) {
    SPSqueeze q{};
    q.kind = kind;
    q.offset_ms = offset_ms;
    return q;
}

// The early and late sides of Dead on Arrival (O) activation 3's scale, as
// the plan's figures give them.
const TransferScale kDoaScale{1.0044, 0.9970};

}  // namespace

TEST_CASE("note window: squeeze_ends follows the sentence, not the kind") {
    // Each line is (kind, offset_ms) -> the squeezed note's end, the end
    // whose word the squeeze sentence prints ("late" is the backend). Each
    // kind lands on both ends, so a pick by kind alone fails.
    struct Case {
        SqueezeKind kind;
        double offset_ms;
        NoteEnd squeezed;
    };
    const Case cases[] = {
        // A squeeze in: "more than ... early" while its figure is 0 or more.
        {SqueezeKind::SqIn, 0.001, NoteEnd::Frontend},
        {SqueezeKind::SqIn, 0.0, NoteEnd::Frontend},
        {SqueezeKind::SqIn, -0.001, NoteEnd::Backend},
        // A squeeze out: its figure is the offset negated.
        {SqueezeKind::SqOut, 0.001, NoteEnd::Frontend},
        {SqueezeKind::SqOut, 0.0, NoteEnd::Backend},
        {SqueezeKind::SqOut, -0.001, NoteEnd::Backend},
    };
    for (const Case& c : cases) {
        CAPTURE(c.offset_ms);
        CAPTURE(static_cast<int>(c.kind));
        const SqueezeEnds e = squeeze_ends(squeeze_at(c.kind, c.offset_ms));
        CHECK(e.squeezed == c.squeezed);
        CHECK(e.anchor != e.squeezed);
    }
}

TEST_CASE("note window: early_fill_end follows the Early fill line's sign") {
    // e_offset is the figure negated: +0.001 prints -0.001 (backend).
    CHECK(early_fill_end(0.001) == NoteEnd::Backend);
    CHECK(early_fill_end(0.0) == NoteEnd::Frontend);
    CHECK(early_fill_end(-0.001) == NoteEnd::Frontend);
}

TEST_CASE("note window: transfer_side reads the early side for the frontend") {
    CHECK(transfer_side(kDoaScale, NoteEnd::Frontend) == 1.0044);
    CHECK(transfer_side(kDoaScale, NoteEnd::Backend) == 0.9970);
}

TEST_CASE("note window: squeeze_window_ends lists each end, frontend first") {
    // two_squeezes' activation, on a song whose squeezed-out chord (3840) has
    // a neighbour 20 ticks (52.08 ms) after it, so its side is not the cap.
    const TwoSqueezes t = two_squeezes();
    const ChordTimes chords = chord_times(red_chords({0, 768, 1536, 2304, 3072, 3840, 3860}));

    // The squeeze out has a timing: the anchor (768) is on the frontend and
    // the squeezed chord (3840) on the backend.
    const std::optional<WindowEnds> out = squeeze_window_ends(t.act, 1, chords);
    REQUIRE(out.has_value());
    REQUIRE(out->ends.size() == 2);
    CHECK(out->ends[0].end == NoteEnd::Frontend);
    CHECK(out->ends[0].tick == 768);
    CHECK(out->ends[0].side_ms == kCapSide);
    CHECK(out->ends[1].end == NoteEnd::Backend);
    CHECK(out->ends[1].tick == 3840);
    CHECK(out->ends[1].side_ms == pinned(71.97216363656233));

    // The squeeze in has a timing: its squeezed chord (2304) is on the
    // frontend, so it comes first and the anchor (768) second.
    const std::optional<WindowEnds> in = squeeze_window_ends(t.act, 0, chords);
    REQUIRE(in.has_value());
    REQUIRE(in->ends.size() == 2);
    CHECK(in->ends[0].end == NoteEnd::Frontend);
    CHECK(in->ends[0].tick == 2304);
    CHECK(in->ends[1].end == NoteEnd::Backend);
    CHECK(in->ends[1].tick == 768);

    CHECK_FALSE(squeeze_window_ends(t.act, 2, chords).has_value());
}

TEST_CASE("note window: the anchor's scale side follows its end") {
    // Every side here is the cap; only the scale side and the figure move
    // the sum. A pick by kind gives the other side in the two cases without
    // a timing.
    TwoSqueezes t = two_squeezes();
    const ChordTimes chords = chord_times(t.song);
    t.act.transfer_post = kDoaScale;
    t.act.sqinouts[0].transfer = kDoaScale;

    SUBCASE("with a timing: early for a squeeze out, late for a squeeze in") {
        const std::optional<WindowEnds> out = squeeze_window_ends(t.act, 1, chords);  // offset -100
        REQUIRE(out.has_value());
        CHECK(out->safe_ms == pinned(71.80845637793124));
        const std::optional<WindowEnds> in = squeeze_window_ends(t.act, 0, chords);  // offset 20
        REQUIRE(in.has_value());
        CHECK(in->safe_ms == pinned(151.17416054017596));
    }
    SUBCASE("needing no timing: early for a squeeze in, late for a squeeze out") {
        t.act.sqinouts[0].offset_ms = -20.0;
        t.act.sqinouts[1].offset_ms = 10.0;
        const std::optional<WindowEnds> in = squeeze_window_ends(t.act, 0, chords);
        REQUIRE(in.has_value());
        CHECK(in->ends[0].end == NoteEnd::Frontend);
        CHECK(in->ends[0].tick == 768);  // the anchor
        CHECK(in->safe_ms == pinned(191.80845637793124));
        const std::optional<WindowEnds> out = squeeze_window_ends(t.act, 1, chords);
        REQUIRE(out.has_value());
        CHECK(out->ends[1].end == NoteEnd::Backend);
        CHECK(out->ends[1].tick == 768);  // the anchor
        CHECK(out->safe_ms == pinned(181.17416054017596));
    }
}

TEST_CASE("note window: the early fill end reads the SP-ready note") {
    TwoSqueezes t = two_squeezes();
    const ChordTimes chords = chord_times(t.song);

    t.act.e_offset = -10.0;  // ready 10 ms after the deadline: figure +10.0
    std::optional<WindowEnds> row = early_fill_window_end(t.act, chords);
    REQUIRE(row.has_value());
    REQUIRE(row->ends.size() == 1);
    CHECK(row->ends[0].end == NoteEnd::Frontend);
    CHECK(row->ends[0].tick == 768);
    CHECK(row->ends[0].side_ms == kCapSide);
    CHECK(row->safe_ms == pinned(75.71565375071405));

    t.act.e_offset = 10.0;  // time to spare: figure -10.0
    row = early_fill_window_end(t.act, chords);
    REQUIRE(row.has_value());
    REQUIRE(row->ends.size() == 1);
    CHECK(row->ends[0].end == NoteEnd::Backend);
    CHECK(row->safe_ms == pinned(95.71565375071405));
}
