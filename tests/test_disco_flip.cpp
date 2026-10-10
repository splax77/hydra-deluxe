// Step 2, T1 (D19, findings 11 and 250): each difficulty reads only its own
// disco-flip markers, in both formats, and the Pro Drums gate lives in one
// place.

#include "doctest.h"

#include <cstdint>
#include <string>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "chart_text.h"
#include "difficulty_literals.h"
#include "midi_util.h"
#include "parse/song.h"

#ifndef HYDRA_INPUT_DIR
#error "HYDRA_INPUT_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

namespace {

// Each difficulty's red-pad pitch, mix digit and .chart section, from the one
// pinned literal table.
using Diff = testdiff::Literals;
constexpr const auto& kDiffs = testdiff::kLiterals;

// A .mid whose PART DRUMS holds `markers` at tick 0, then a red note for
// every difficulty on that same tick.
std::vector<uint8_t> mid_with(const std::vector<std::string>& markers) {
    using namespace testmidi;
    std::vector<std::vector<uint8_t>> ev = {track_name("PART DRUMS"), set_tempo()};
    for (const std::string& m : markers) ev.push_back(text_event(m));
    for (const Diff& x : kDiffs) ev.push_back(note_on(x.red, 100));
    ev.push_back(end_of_track());
    return smf(concat(ev));
}

// A .chart with one difficulty section: `markers` at tick 0, then a red note.
std::vector<uint8_t> chart_with(const char* section, const std::vector<std::string>& markers) {
    std::string lines;
    for (const std::string& m : markers) lines += "  0 = E " + m + "\n";
    lines += "  0 = N 1 0\n";
    return testchart::chart_bytes(testchart::section(section, lines));
}

// The one chord's red note was flipped: a flipped red reads as a yellow
// cymbal (Chord::apply_disco_flip).
bool flipped(const Song& song) {
    REQUIRE(song.sequence.size() == 1);
    return song.sequence[0].chord.at(NoteColor::Yellow).has_value();
}

std::string mid_on(char digit) { return std::string("[mix ") + digit + " drums0d]"; }
std::string chart_on(char digit) { return std::string("mix_") + digit + "_drums0d"; }

}  // namespace

TEST_CASE(".mid: a disco marker flips only the difficulty it names") {
    for (const Diff& marked : kDiffs) {
        const std::vector<uint8_t> mid = mid_with({mid_on(marked.mix)});
        for (const Diff& parsed : kDiffs) {
            const std::string at = std::string(difficulty_name(parsed.d)) + ", marker " +
                                   mid_on(marked.mix);
            CAPTURE(at);
            CHECK(flipped(load_songbytes_mid(mid, true, true, parsed.d)) ==
                  (parsed.d == marked.d));
        }
    }
}

TEST_CASE(".chart: a section obeys only the disco marker naming its own difficulty") {
    for (const Diff& parsed : kDiffs) {
        for (const Diff& marked : kDiffs) {
            const std::string at = std::string(parsed.section) + ", marker " +
                                   chart_on(marked.mix);
            CAPTURE(at);
            const Song song = load_songbytes_chart(
                chart_with(parsed.section, {chart_on(marked.mix)}), true, true, parsed.d);
            CHECK(flipped(song) == (parsed.d == marked.d));
        }
    }
}

TEST_CASE("disco flip needs Pro Drums, in both formats and at every difficulty") {
    for (const Diff& x : kDiffs) {
        const std::string name = difficulty_name(x.d);
        CAPTURE(name);
        const std::vector<uint8_t> mid = mid_with({mid_on(x.mix)});
        const std::vector<uint8_t> chart = chart_with(x.section, {chart_on(x.mix)});
        CHECK(flipped(load_songbytes_mid(mid, true, true, x.d)));
        CHECK(flipped(load_songbytes_chart(chart, true, true, x.d)));
        CHECK_FALSE(flipped(load_songbytes_mid(mid, false, true, x.d)));
        CHECK_FALSE(flipped(load_songbytes_chart(chart, false, true, x.d)));
    }
}

TEST_CASE("disco: dnoflip still closes the section (Hydra's rule, not Clone Hero's)") {
    // Both markers on one tick run in file order: on, then off.
    for (const Diff& x : kDiffs) {
        const std::string name = difficulty_name(x.d);
        CAPTURE(name);
        const std::string mid_off = std::string("[mix ") + x.mix + " drums0dnoflip]";
        const std::string chart_off = std::string("mix_") + x.mix + "_drums0dnoflip";
        CHECK_FALSE(flipped(load_songbytes_mid(mid_with({mid_on(x.mix), mid_off}), true,
                                               true, x.d)));
        CHECK_FALSE(flipped(load_songbytes_chart(
            chart_with(x.section, {chart_on(x.mix), chart_off}), true, true, x.d)));
    }
}

TEST_CASE("disco: Band Like That keeps its lower difficulties' hi-hat on yellow") {
    // This chart marks disco only for Expert ([mix 3 drums0d], 13 sections);
    // Hard, Medium and Easy carry only `[mix N drums0]` at tick 960. So below
    // Expert, Pro Drums must not move a single red or yellow note: the counts
    // equal a parse with Pro Drums off, where no flip ever applies. Before D19,
    // Hard followed Expert's sections and swapped 111 notes each way (finding
    // 11: 151 red and 40 yellow inside the spans, where Hard has 40 red and 151
    // yellow as authored).
    using app::DynamicsRow;
    const std::string path = std::string(HYDRA_INPUT_DIR) +
                             "/common/IB24/T2/fanclubwallet - Band Like That/notes.mid";
    for (Difficulty d : {Difficulty::Hard, Difficulty::Medium, Difficulty::Easy}) {
        const std::string name = difficulty_name(d);
        CAPTURE(name);
        const app::DynamicsBreakdown pro = app::count_dynamics(load_songpath(path, true, true, d));
        const app::DynamicsBreakdown plain =
            app::count_dynamics(load_songpath(path, false, true, d));
        CHECK(pro.row(DynamicsRow::RedSnare).all() == plain.row(DynamicsRow::RedSnare).all());
        CHECK(pro.row(DynamicsRow::YellowCymbal).all() + pro.row(DynamicsRow::YellowTom).all() ==
              plain.row(DynamicsRow::YellowTom).all());
    }
    // Expert still flips inside its own sections.
    const app::DynamicsBreakdown pro =
        app::count_dynamics(load_songpath(path, true, true, Difficulty::Expert));
    const app::DynamicsBreakdown plain =
        app::count_dynamics(load_songpath(path, false, true, Difficulty::Expert));
    CHECK(pro.row(DynamicsRow::RedSnare).all() != plain.row(DynamicsRow::RedSnare).all());
}
