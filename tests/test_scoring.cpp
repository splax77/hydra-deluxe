// The squeeze-out rule's one body (core/scoring.h): sqout_late_notes names
// the notes a squeezed-out chord pays outside Star Power, and category_scores
// takes exactly their doubling away.

#include "doctest.h"

#include <cstddef>
#include <vector>

#include "core/model.h"
#include "core/rules.h"
#include "core/scoring.h"
#include "core/timing.h"

using namespace hydra;

namespace {

// The first combo the game pays at x2, read from to_multiplier, the owner of
// the multiplier steps.
int first_x2_combo() {
    int combo = 0;
    while (to_multiplier(combo) < 2) ++combo;
    return combo;
}

}  // namespace

TEST_CASE("scoring: a squeezed-out chord pays its first note outside Star Power, or every note under the whole-chord rule") {
    // A kick, a red and a blue: three plain notes of one price, so the
    // base-sorted order keeps lane order and the kick is the first note.
    hydra::Chord three;  // qualified: windows.h declares a Chord too
    three.add_note(NoteColor::Kick);
    three.add_note(NoteColor::Red);
    three.add_note(NoteColor::Blue);
    const hydra::Chord::NoteList order = three.note_list(true);
    REQUIRE(order.size() == 3);
    const size_t kick_place = 0;
    CHECK(order[kick_place].colortype == NoteColor::Kick);

    CHECK(sqout_late_notes(three, core::SqOutRule::FirstNote) == std::vector<size_t>{kick_place});
    std::vector<size_t> every_place;
    for (size_t place = 0; place < order.size(); ++place) every_place.push_back(place);
    CHECK(sqout_late_notes(three, core::SqOutRule::WholeChord) == every_place);
    CHECK(sqout_late_notes(hydra::Chord{}, core::SqOutRule::WholeChord).empty());
}

TEST_CASE("scoring: a chord crossing the x2 step pays each note at its own multiplier and a squeeze-out loses the late notes' doubling") {
    // A red and a kick hit two short of the x2 combo: the first note lands
    // one short of it and pays at x1, the second lands on it and pays at x2.
    const int x2_from = first_x2_combo();
    const int combo_before = x2_from - 2;
    REQUIRE(to_multiplier(combo_before + 1) == 1);
    REQUIRE(to_multiplier(combo_before + 2) == 2);

    hydra::Chord chord;
    chord.add_note(NoteColor::Red);
    chord.add_note(NoteColor::Kick);
    // Both are plain pads, each worth kNoteBasePoints at 1x (model.h).
    const int first_paid = kNoteBasePoints * 1;
    const int second_paid = kNoteBasePoints * 2;
    // Star Power pays kStarPowerMultiplier copies of each paid value; its
    // share is the copies beyond the one already paid.
    const int sp_share = (kStarPowerMultiplier - 1) * (first_paid + second_paid);

    const CategoryScores first = category_scores(chord, combo_before, nullptr, core::SqOutRule::FirstNote);
    const CategoryScores whole = category_scores(chord, combo_before, nullptr, core::SqOutRule::WholeChord);
    for (const CategoryScores* s : {&first, &whole}) {
        CHECK(s->notes == kNoteBasePoints + kNoteBasePoints);
        CHECK(s->combo == second_paid - kNoteBasePoints);
        CHECK(s->sp == sp_share);
        CHECK(s->accent == 0);
        CHECK(s->ghost == 0);
        CHECK(s->multiplier == 1);
        CHECK(s->multiplier_after == 2);
        CHECK(s->combo_after == x2_from);
    }
    // A lost doubling is the note's paid value: FirstNote loses the first
    // note's, WholeChord both.
    CHECK(first.sqout_reduction == first_paid);
    CHECK(whole.sqout_reduction == first_paid + second_paid);
    CHECK(first.sqout_sp() == sp_share - first_paid);
    CHECK(whole.sqout_sp() == sp_share - first_paid - second_paid);
}
