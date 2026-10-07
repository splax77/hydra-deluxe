#include "core/scoring.h"

#include <vector>

#include "core/timing.h"  // to_multiplier, kStarPowerMultiplier

namespace hydra {

namespace {

// One note's price: what it is worth at 1x (ChordNote::basescore, the one
// rule for that) and what it pays at a combo multiplier. Every share
// category_scores reports, and multsqueeze_gain's two orders, read this.
struct NoteValue {
    int at_1x;
    int paid;
};

NoteValue note_value(const ChordNote& note, int multiplier) {
    const int at_1x = note.basescore();
    return {at_1x, at_1x * multiplier};
}

}  // namespace

CategoryScores category_scores(const Chord& chord, int combo,
                                std::vector<CategoryScores>* per_note,
                                core::SqOutRule sqout_rule) {
    CategoryScores out;
    out.multiplier = to_multiplier(combo);
    out.multiplier_after = out.multiplier;

    const Chord::NoteList ordering = chord.note_list(true);
    if (per_note) {
        per_note->clear();
        per_note->reserve(ordering.size());
    }
    for (size_t i = 0; i < ordering.size(); ++i) {
        const ChordNote& note = ordering[i];
        combo += 1;
        const int combo_multiplier = to_multiplier(combo);
        if (i == 0) out.multiplier = combo_multiplier;
        out.multiplier_after = combo_multiplier;

        const NoteValue value = note_value(note, combo_multiplier);
        // A ghost or accent's own category holds the pad's 50 at 1x; the
        // base category holds the rest of the 1x value, and the combo
        // category everything the multiplier adds on top.
        const int accent = note.is_accent() ? kNoteBasePoints : 0;
        const int ghost = note.is_ghost() ? kNoteBasePoints : 0;
        const int base = value.at_1x - accent - ghost;
        const int combo_share = value.paid - value.at_1x;
        // Star Power multiplies the whole paid value by kStarPowerMultiplier,
        // so its own share is that many copies less the one already paid.
        const int sp = (kStarPowerMultiplier - 1) * value.paid;
        // SqOut: the notes that lose their SP doubling. FirstNote takes note 0
        // only; WholeChord takes every note. A lost doubling is the note's
        // paid value.
        const bool loses_sp = i == 0 || sqout_rule == core::SqOutRule::WholeChord;
        const int note_sqout = loses_sp ? value.paid : 0;

        out.base += base;
        out.combo += combo_share;
        out.sp += sp;
        out.accent += accent;
        out.ghost += ghost;
        out.sqout_reduction += note_sqout;

        if (per_note) {
            CategoryScores note_scores;
            note_scores.base = base;
            note_scores.combo = combo_share;
            note_scores.sp = sp;
            note_scores.accent = accent;
            note_scores.ghost = ghost;
            note_scores.sqout_reduction = note_sqout;
            note_scores.multiplier = combo_multiplier;
            note_scores.multiplier_after = combo_multiplier;
            note_scores.combo_after = combo;
            // What the ghost or accent earns: this note's paid value less a
            // plain note's on the same pad at the same multiplier.
            ChordNote plain = note;
            plain.dynamictype = NoteDynamicType::Normal;
            note_scores.dynamics_bonus = value.paid - note_value(plain, combo_multiplier).paid;
            per_note->push_back(note_scores);
        }
    }
    out.combo_after = combo;
    return out;
}

int solo_bonus(const Chord& chord, bool flag_solo) {
    return flag_solo ? kSoloBonusPerNote * chord.count() : 0;
}

ChordScoreTable chord_score_table(const Song& song, core::SqOutRule sqout_rule,
                                  ChordScoreDetail detail) {
    const bool with_notes = detail == ChordScoreDetail::WithNotes;
    ChordScoreTable out;
    out.rows.reserve(song.sequence.size());
    if (with_notes) out.notes.reserve(static_cast<size_t>(song.note_count()));

    // One scratch list reused for every chord, so keeping the notes costs no
    // allocation per chord.
    std::vector<CategoryScores> per_note;
    int combo = 0;
    for (const SongTimestamp& ts : song.sequence) {
        const CategoryScores sg =
            category_scores(ts.chord, combo, with_notes ? &per_note : nullptr, sqout_rule);
        ChordScoreRow row;
        row.base = sg.base;
        row.combo = sg.combo;
        row.sp = sg.sp;
        row.sqout_sp = sg.sqout_sp();
        row.accent = sg.accent;
        row.ghost = sg.ghost;
        row.solo = solo_bonus(ts.chord, ts.flag_solo);
        row.multiplier = sg.multiplier;
        row.multiplier_after = sg.multiplier_after;
        row.combo_before = combo;
        row.combo_after = sg.combo_after;
        row.note_begin = static_cast<uint32_t>(out.notes.size());
        if (with_notes) out.notes.insert(out.notes.end(), per_note.begin(), per_note.end());
        row.note_end = static_cast<uint32_t>(out.notes.size());
        out.rows.push_back(row);
        combo = sg.combo_after;
    }
    return out;
}

int multsqueeze_gain(const Chord& chord, int combo) {
    // The multiplier each position is paid at, from the payout itself.
    std::vector<CategoryScores> per_note;
    category_scores(chord, combo, &per_note);
    const std::vector<ChordNote> best = chord.notes(true);
    const size_t n = best.size();
    int best_total = 0, worst_total = 0;
    for (size_t i = 0; i < n; ++i) {
        best_total += note_value(best[i], per_note[i].multiplier).paid;
        worst_total += note_value(best[n - 1 - i], per_note[i].multiplier).paid;
    }
    return best_total - worst_total;
}

}  // namespace hydra
