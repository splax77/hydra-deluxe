// Chord::code spells a chord one character per lane. Every chord a chart can
// express must get a code of its own, because the replay's rows name chords
// by these codes.

#include "doctest.h"

#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/model.h"

using hydra::Chord;
using hydra::ChordNote;
using hydra::NoteColor;
using hydra::NoteCymbalType;
using hydra::NoteDynamicType;

namespace {

// Every shape one lane can take, empty included: every dynamic, plus the
// lane's flag where it has one (hydra::lane_allows_flag, set by
// hydra::set_lane_flag).
std::vector<std::optional<ChordNote>> lane_shapes(NoteColor color) {
    std::vector<std::optional<ChordNote>> out{std::nullopt};
    for (NoteDynamicType dyn :
         {NoteDynamicType::Normal, NoteDynamicType::Ghost, NoteDynamicType::Accent}) {
        const ChordNote plain{color, dyn};
        out.push_back(plain);
        if (hydra::lane_allows_flag(color)) {
            ChordNote flagged = plain;
            hydra::set_lane_flag(flagged);
            out.push_back(flagged);
        }
    }
    return out;
}

}  // namespace

TEST_CASE("every chord a chart can express has a code of its own") {
    const auto kick = lane_shapes(NoteColor::Kick);
    const auto red = lane_shapes(NoteColor::Red);
    const auto yellow = lane_shapes(NoteColor::Yellow);
    const auto blue = lane_shapes(NoteColor::Blue);
    const auto green = lane_shapes(NoteColor::Green);

    std::set<std::string> codes;
    for (const auto& k : kick)
        for (const auto& r : red)
            for (const auto& y : yellow)
                for (const auto& b : blue)
                    for (const auto& g : green) {
                        Chord chord;
                        chord.at(NoteColor::Kick) = k;
                        chord.at(NoteColor::Red) = r;
                        chord.at(NoteColor::Yellow) = y;
                        chord.at(NoteColor::Blue) = b;
                        chord.at(NoteColor::Green) = g;
                        codes.insert(chord.code());
                    }
    // No two chords share a code. Kick 7 shapes x red 4 x yellow, blue, green 7 each, the empty chord included.
    CHECK(codes.size() == 7u * 4 * 7 * 7 * 7);
}

TEST_CASE("a chord's code spells its lanes") {
    // Joe Sibol's Hot Sexy Girls (RBN) hits red, yellow cymbal and green
    // cymbal together. Before codes were spelled out, a lookup table stopped
    // at two pads and its re-analysis threw "chord has no encode-table entry".
    Chord three;
    three.at(NoteColor::Red) = ChordNote{NoteColor::Red};
    three.at(NoteColor::Yellow) =
        ChordNote{NoteColor::Yellow, NoteDynamicType::Normal, NoteCymbalType::Cymbal};
    three.at(NoteColor::Green) =
        ChordNote{NoteColor::Green, NoteDynamicType::Normal, NoteCymbalType::Cymbal};
    CHECK(three.code() == ".nN.N");

    // Every lane at once: accent 2x kick, ghost red, yellow tom, accent blue
    // cymbal, ghost green tom.
    Chord five;
    ChordNote kick{NoteColor::Kick, NoteDynamicType::Accent};
    kick.is2x = true;
    five.at(NoteColor::Kick) = kick;
    five.at(NoteColor::Red) = ChordNote{NoteColor::Red, NoteDynamicType::Ghost};
    five.at(NoteColor::Yellow) = ChordNote{NoteColor::Yellow};
    five.at(NoteColor::Blue) =
        ChordNote{NoteColor::Blue, NoteDynamicType::Accent, NoteCymbalType::Cymbal};
    five.at(NoteColor::Green) = ChordNote{NoteColor::Green, NoteDynamicType::Ghost};
    CHECK(five.code() == "AgnAg");
}
