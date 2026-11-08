// The squeeze-out rule's one body (core/scoring.h): sqout_late_notes names
// exactly the notes category_scores pays outside, and category_scores pays
// what it paid before the rule moved into sqout_pays_outside. Task F1 of
// docs/superpowers/plans/2026-11-08-player-audit-fixes.md.

#include "doctest.h"

#include <cstddef>
#include <string>
#include <vector>

#include "app/config.h"
#include "core/model.h"
#include "core/rules.h"
#include "core/scoring.h"
#include "corpus_util.h"
#include "parse/song.h"

using namespace hydra;

namespace {

const core::SqOutRule kBothRules[] = {core::SqOutRule::FirstNote, core::SqOutRule::WholeChord};

// The places whose per-note share carries a lost doubling: the notes
// category_scores pays outside.
std::vector<size_t> paid_outside(const hydra::Chord& chord, int combo, core::SqOutRule rule) {
    std::vector<CategoryScores> per_note;
    category_scores(chord, combo, &per_note, rule);
    std::vector<size_t> out;
    for (size_t i = 0; i < per_note.size(); ++i)
        if (per_note[i].sqout_reduction != 0) out.push_back(i);
    return out;
}

}  // namespace

TEST_CASE("scoring: sqout_late_notes is what category_scores pays outside") {
    // Hand pins: a three-note chord.
    hydra::Chord three;  // qualified: windows.h declares a Chord too
    three.add_note(NoteColor::Kick);
    three.add_note(NoteColor::Red);
    three.add_note(NoteColor::Blue);
    CHECK(sqout_late_notes(three, core::SqOutRule::FirstNote) == std::vector<size_t>{0});
    CHECK(sqout_late_notes(three, core::SqOutRule::WholeChord) == std::vector<size_t>{0, 1, 2});
    CHECK(sqout_late_notes(hydra::Chord{}, core::SqOutRule::WholeChord).empty());

    // Every corpus chord, under both rules, at its running combo.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int chords = 0;
    std::string first_bad;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        if (song.is_empty()) continue;
        // The running combo before each chord is the table's (chord_score_table).
        const ChordScoreTable table = chord_score_table(song, core::SqOutRule::FirstNote);
        for (size_t i = 0; i < song.sequence.size(); ++i) {
            const hydra::Chord& chord = song.sequence[i].chord;
            const int combo = table.rows[i].combo_before;
            ++chords;
            for (core::SqOutRule rule : kBothRules) {
                if (sqout_late_notes(chord, rule) != paid_outside(chord, combo, rule) && first_bad.empty())
                    first_bad = path + ": chord " + std::to_string(i) + " under " + core::sqout_rule_name(rule);
            }
        }
    }
    CHECK(chords > 0);
    CHECK_MESSAGE(first_bad.empty(), first_bad);
}

TEST_CASE("scoring: category_scores is unchanged by sqout_pays_outside") {
    // Worked by hand: Red and Kick at combo 8. The first note lands on combo
    // 9 (x1) and pays 50, the second on combo 10 (x2) and pays 100. Notes 100
    // (two 50s at 1x), combo 50 (the second note's extra 50), SP 150 (one more
    // copy of each paid value). FirstNote loses note 0's 50; WholeChord loses
    // all 150.
    hydra::Chord chord;
    chord.add_note(NoteColor::Red);
    chord.add_note(NoteColor::Kick);
    const CategoryScores first = category_scores(chord, 8, nullptr, core::SqOutRule::FirstNote);
    const CategoryScores whole = category_scores(chord, 8, nullptr, core::SqOutRule::WholeChord);
    for (const CategoryScores* s : {&first, &whole}) {
        CHECK(s->notes == 100);
        CHECK(s->combo == 50);
        CHECK(s->sp == 150);
        CHECK(s->accent == 0);
        CHECK(s->ghost == 0);
        CHECK(s->multiplier == 1);
        CHECK(s->multiplier_after == 2);
        CHECK(s->combo_after == 10);
    }
    CHECK(first.sqout_reduction == 50);
    CHECK(whole.sqout_reduction == 150);
    CHECK(first.sqout_sp() == 100);
    CHECK(whole.sqout_sp() == 0);
}
