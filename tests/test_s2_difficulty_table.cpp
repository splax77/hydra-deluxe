// Step 2, T0: the one table of how each difficulty is spelled in a chart
// file. Every parser rule that depends on the difficulty reads its row.

#include "doctest.h"

#include <cstdint>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "difficulty_literals.h"
#include "midi_util.h"
#include "parse/song.h"

using namespace hydra;

TEST_CASE("difficulty table: name, section, kick, 2x kick and disco digit for every difficulty") {
    // The literals are Clone Hero's (tests/difficulty_literals.h).
    REQUIRE(std::size(testdiff::kLiterals) == std::size(kAllDifficulties));
    for (const testdiff::Literals& w : testdiff::kLiterals) {
        CAPTURE(w.name);
        const DifficultyChartCodes& c = difficulty_chart_codes(w.d);
        CHECK(std::string(difficulty_name(w.d)) == w.name);
        CHECK(c.chart_section() == w.section);
        CHECK(c.kick_pitch == w.kick);
        CHECK(c.mix_digit == w.mix);
        // The 2x kick is not stored: it is worked out as kick - 1, and that
        // must land on Clone Hero's own 2x pitch at every difficulty.
        CHECK(c.kick2x_pitch() == w.kick2x);
    }
    // An out-of-range value reads as Expert, through the table's one fallback.
    const auto bad = static_cast<Difficulty>(7);
    CHECK(std::string(difficulty_name(bad)) == "Expert");
    CHECK(difficulty_chart_codes(bad).kick_pitch == 96);
}

TEST_CASE("difficulty table: the .mid parser reads each difficulty's kick from it") {
    // One kick per difficulty, a beat apart, in kLiterals' order: Expert's at
    // tick 0, Hard's at 480, Medium's at 960, Easy's at 1440. A difficulty that
    // read another's kick pitch would load the wrong tick.
    using namespace testmidi;
    constexpr uint32_t kBeat = 480;
    std::vector<std::vector<uint8_t>> ev = {track_name("PART DRUMS"), set_tempo()};
    for (size_t i = 0; i < std::size(testdiff::kLiterals); ++i) {
        const std::vector<uint8_t> kick =
            note_on(static_cast<uint8_t>(testdiff::kLiterals[i].kick), 100);
        ev.push_back(i == 0 ? kick : after(kBeat, kick));
    }
    ev.push_back(end_of_track());
    const std::vector<uint8_t> mid = smf(concat(ev));

    for (size_t i = 0; i < std::size(testdiff::kLiterals); ++i) {
        const testdiff::Literals& w = testdiff::kLiterals[i];
        CAPTURE(w.name);
        const Song song = load_songbytes_mid(mid, true, true, w.d);
        REQUIRE(song.sequence.size() == 1);
        CHECK(song.sequence[0].timecode.ticks() == static_cast<int64_t>(i * kBeat));
        CHECK(song.sequence[0].chord.at(NoteColor::Kick).has_value());
    }
}
