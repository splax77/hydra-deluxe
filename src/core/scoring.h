// Per-chord score breakdown — the hottest function in chart analysis.
//
// Each note is priced once, as ChordNote::basescore (the owner of a note's
// 1x value) times its combo multiplier, and every share below is read off
// that one value.

#ifndef HYDRA_CORE_SCORING_H
#define HYDRA_CORE_SCORING_H

#include <vector>

#include "core/model.h"
#include "core/rules.h"

namespace hydra {

struct CategoryScores {
    int base = 0;
    int combo = 0;
    int sp = 0;
    int accent = 0;
    int ghost = 0;
    int sqout_reduction = 0;
    // The SP points this chord keeps when it is squeezed out: the chord's SP
    // value minus the lost doubling. The one place that subtraction lives.
    int sqout_sp() const { return sp - sqout_reduction; }
    // The combo multiplier applied to the chord's first note (base-sorted),
    // the same note the SqOut calculation reads. Per note, that note's own.
    int multiplier = 1;
    // The multiplier applied to the chord's last note: what the game's disc
    // shows once the whole chord is hit. Per note, the same as `multiplier`.
    int multiplier_after = 1;
    // The combo once the chord is hit: the combo passed in plus the chord's
    // note count. Per note, the combo once that note is hit, so the last
    // note's equals the chord's.
    int combo_after = 0;
    // Per note only (left 0 on the chord total): the points this note's
    // ghost or accent earns, multiplier included -- its paid value less a
    // plain note's on the same pad. A mis-hit dynamic note loses exactly
    // this, twice over inside Star Power. `accent` and `ghost` above hold
    // only the pad's 50, before the multiplier.
    int dynamics_bonus = 0;
};

// Notes are read base-sorted (Chord::notes(true)); the tie order is
// observable through the SqOut calculation, which reads note 0.
//
// If per_note is given, it is filled with one CategoryScores per note, in
// that same order, holding each note's own share of the six totals below.
// This is only for display/tooling use — it does not change the aggregate
// that gets returned.
// sqout_rule decides which notes' SP doubling sqout_reduction takes: only
// note 0 (FirstNote, Hydra's rule so far) or every note (WholeChord).
CategoryScores category_scores(const Chord& chord, int combo,
                                std::vector<CategoryScores>* per_note = nullptr,
                                core::SqOutRule sqout_rule = core::SqOutRule::FirstNote);

// The solo bonus a chord earns: kSoloBonusPerNote for each of its notes when
// the chord is in a solo section (flag_solo), else 0.
int solo_bonus(const Chord& chord, bool flag_solo);

// What hitting a chord in the right order gains over the wrong order, in
// points, when the chord straddles a multiplier step (a multiplier squeeze).
// Each note is paid at the multiplier category_scores pays its position. The
// best order is Chord::notes(true), cheapest first, so the dearest notes land
// past the step; the worst order is the reverse. MultSqueeze::points calls it.
int multsqueeze_gain(const Chord& chord, int combo);

}  // namespace hydra

#endif  // HYDRA_CORE_SCORING_H
