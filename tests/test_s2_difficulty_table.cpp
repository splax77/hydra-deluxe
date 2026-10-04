// Step 2, T0: the one table of how each difficulty is spelled in a chart
// file. Every parser rule that depends on the difficulty reads its row.

#include "doctest.h"

#include <cstdint>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "midi_util.h"
#include "parse/song.h"

using namespace hydra;

TEST_CASE("difficulty table: kick, 2x kick and disco digit for every difficulty") {
    // Clone Hero 1.1: 0x210D9C0 maps pitches 58-66 / 70-78 / 82-90 / 94-102 to
    // Easy / Medium / Hard / Expert; 0x21555CD flags 59, 71, 83 and 95 as
    // DoubleKick; 0x210D990 maps a mix digit 0-3 to Easy..Expert.
    struct Want {
        Difficulty d;
        int kick;
        int kick2x;
        char mix;
    };
    const Want want[] = {{Difficulty::Expert, 96, 95, '3'},
                         {Difficulty::Hard, 84, 83, '2'},
                         {Difficulty::Medium, 72, 71, '1'},
                         {Difficulty::Easy, 60, 59, '0'}};
    REQUIRE(std::size(want) == std::size(kAllDifficulties));
    for (const Want& w : want) {
        const std::string name = difficulty_name(w.d);
        CAPTURE(name);
        const DifficultyChartCodes& c = difficulty_chart_codes(w.d);
        CHECK(c.kick_pitch == w.kick);
        CHECK(c.kick2x_pitch == w.kick2x);
        CHECK(c.mix_digit == w.mix);
        // The 2x kick sits one pitch below the kick at every difficulty.
        CHECK(c.kick2x_pitch == c.kick_pitch - 1);
    }
}

TEST_CASE("difficulty table: the .mid parser reads each difficulty's kick from it") {
    // One kick per difficulty, a beat apart: Expert's 96 at tick 0, Hard's 84
    // at 480, Medium's 72 at 960, Easy's 60 at 1440. A difficulty that read
    // another's kick pitch would load the wrong tick.
    using namespace testmidi;
    std::vector<std::vector<uint8_t>> ev = {track_name("PART DRUMS"), set_tempo(),
                                            note_on(96, 100)};
    // Each later kick: a delta of 480 ticks (0x83 0x60), then a note-on.
    for (uint8_t pitch : {uint8_t{84}, uint8_t{72}, uint8_t{60}})
        ev.push_back({0x83, 0x60, 0x90, pitch, 100});
    ev.push_back(end_of_track());
    const std::vector<uint8_t> mid = smf(concat(ev));

    const std::pair<Difficulty, int64_t> cases[] = {{Difficulty::Expert, 0},
                                                    {Difficulty::Hard, 480},
                                                    {Difficulty::Medium, 960},
                                                    {Difficulty::Easy, 1440}};
    for (const auto& [d, tick] : cases) {
        const std::string name = difficulty_name(d);
        CAPTURE(name);
        const Song song = load_songbytes_mid(mid, true, true, d);
        REQUIRE(song.sequence.size() == 1);
        CHECK(song.sequence[0].timecode.ticks() == tick);
        CHECK(song.sequence[0].chord.at(NoteColor::Kick).has_value());
    }
}
