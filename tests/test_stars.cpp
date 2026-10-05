// Unit tests for core/stars: Clone Hero's star cutoffs, as the game computes
// them (float multiply, then round up), and where their inputs come from.

#include "doctest.h"

#include <array>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "core/model.h"
#include "core/scoring.h"
#include "core/stars.h"
#include "core/timing.h"
#include "corpus_util.h"
#include "scratch_settings.h"

using namespace hydra;

namespace {

std::array<int64_t, kMaxStars> cutoffs_for(int64_t base) {
    std::array<int64_t, kMaxStars> out{};
    for (int stars = 1; stars <= kMaxStars; ++stars) out[stars - 1] = star_cutoff(base, stars);
    return out;
}

// A chord holding these notes, each on its own pad.
Chord chord_of(std::initializer_list<ChordNote> notes) {
    Chord c;
    for (const ChordNote& n : notes) c.at(n.colortype) = n;
    return c;
}

}  // namespace

TEST_CASE("scoring: one note's value at combos 0, 9 and 29, by category") {
    // Each note lands at combo 1, 10 and 30: multipliers 1, 2 and 4 (D51
    // call 13). The values are the audit's table (finding 160): a tom is 50,
    // a cymbal 65, a ghost tom 100 and an accent cymbal 130 at 1x.
    struct Row {
        const char* name;
        ChordNote note;
        int base, accent, ghost;
        std::array<int, 3> combo, sp, dynamics_bonus;
    };
    const Row rows[] = {
        {"tom", ChordNote{NoteColor::Red}, 50, 0, 0, {0, 50, 150}, {50, 100, 200}, {0, 0, 0}},
        {"cymbal",
         ChordNote{NoteColor::Yellow, NoteDynamicType::Normal, NoteCymbalType::Cymbal, false},
         65, 0, 0, {0, 65, 195}, {65, 130, 260}, {0, 0, 0}},
        {"ghost tom", ChordNote{NoteColor::Red, NoteDynamicType::Ghost}, 50, 0, 50,
         {0, 100, 300}, {100, 200, 400}, {50, 100, 200}},
        {"accent cymbal",
         ChordNote{NoteColor::Yellow, NoteDynamicType::Accent, NoteCymbalType::Cymbal, false},
         80, 50, 0, {0, 130, 390}, {130, 260, 520}, {65, 130, 260}},
    };
    const std::array<int, 3> combos{0, 9, 29};
    const std::array<int, 3> multipliers{1, 2, 4};
    for (const Row& row : rows) {
        const Chord chord = chord_of({row.note});
        for (size_t k = 0; k < combos.size(); ++k) {
            CAPTURE(row.name);
            CAPTURE(combos[k]);
            std::vector<CategoryScores> per_note;
            const CategoryScores total = category_scores(chord, combos[k], &per_note);
            REQUIRE(per_note.size() == 1);
            const CategoryScores& n = per_note[0];
            CHECK(n.base == row.base);
            CHECK(n.combo == row.combo[k]);
            CHECK(n.sp == row.sp[k]);
            CHECK(n.accent == row.accent);
            CHECK(n.ghost == row.ghost);
            CHECK(n.dynamics_bonus == row.dynamics_bonus[k]);
            CHECK(n.multiplier == multipliers[k]);

            // The chord total of a one-note chord is that note.
            CHECK(total.base == n.base);
            CHECK(total.combo == n.combo);
            CHECK(total.sp == n.sp);
            CHECK(total.accent == n.accent);
            CHECK(total.ghost == n.ghost);
            CHECK(total.sqout_reduction == n.sqout_reduction);

            // 160: the 1x shares add up to the note's basescore.
            CHECK(n.base + n.accent + n.ghost == row.note.basescore());
            // The squeeze-out cut is the note's value.
            CHECK(n.sqout_reduction == n.sp);
        }
    }
}

TEST_CASE("scoring: combo_after is the combo plus the chord's notes") {
    const Chord chord = chord_of(
        {ChordNote{NoteColor::Red},
         ChordNote{NoteColor::Yellow, NoteDynamicType::Normal, NoteCymbalType::Cymbal, false},
         ChordNote{NoteColor::Kick}});
    std::vector<CategoryScores> per_note;
    const CategoryScores total = category_scores(chord, 7, &per_note);
    CHECK(total.combo_after == 10);
    CHECK(total.multiplier == 1);        // the first note lands at combo 8
    CHECK(total.multiplier_after == 2);  // the third at combo 10
    REQUIRE(per_note.size() == 3);
    CHECK(per_note[0].combo_after == 8);
    CHECK(per_note[1].combo_after == 9);
    CHECK(per_note[2].combo_after == 10);
}

TEST_CASE("scoring: solo_bonus is 100 per note in a solo, else 0") {
    const Chord three = chord_of(
        {ChordNote{NoteColor::Red},
         ChordNote{NoteColor::Yellow, NoteDynamicType::Normal, NoteCymbalType::Cymbal, false},
         ChordNote{NoteColor::Kick}});
    CHECK(solo_bonus(three, true) == 300);
    CHECK(solo_bonus(three, false) == 0);
    CHECK(solo_bonus(chord_of({ChordNote{NoteColor::Red}}), true) == 100);
}

TEST_CASE("stars: with_solo is each cutoff plus the solo bonus") {
    // The path of "stars: star_cutoffs reads the path's base score and solo
    // bonus": base 1300, solo 800, cutoffs 130, 650, 1300, 2600, 3640, 4680,
    // 5720.
    Path path;
    path.score_base = 1000;
    path.score_ghosts = 100;
    path.score_accents = 200;
    path.score_solo = 800;
    path.score_combo = 5000;
    path.score_sp = 3000;
    const std::array<int64_t, 7> expected{930, 1450, 2100, 3400, 4440, 5480, 6520};
    CHECK(star_cutoffs(path).with_solo == expected);

    // No solo bonus: the two columns agree.
    path.score_solo = 0;
    const StarCutoffs no_solo = star_cutoffs(path);
    CHECK(no_solo.with_solo == no_solo.cutoffs);
}

TEST_CASE("stars: score_without_solo is the total minus the solo bonus") {
    // The path of "stars: path_stars counts cutoffs reached without the solo
    // bonus", at its 6680 total with a 2000 solo bonus.
    Path path;
    path.score_base = 1000;
    path.score_ghosts = 100;
    path.score_accents = 200;
    path.score_combo = 3380;
    path.score_solo = 2000;
    CHECK(path.totalscore() == 6680);
    CHECK(score_without_solo(path) == 4680);
    CHECK(path_stars(path) == 6);
}

TEST_CASE("stars: the table is the game's first seven multipliers") {
    CHECK(kMaxStars == 7);
    const std::array<float, 7> expected{0.1f, 0.5f, 1.0f, 2.0f, 2.8f, 3.6f, 4.4f};
    CHECK(kStarMultipliers == expected);
}

TEST_CASE("stars: whole-number products come out exact") {
    const std::array<int64_t, 7> expected{10000, 50000, 100000, 200000, 280000, 360000, 440000};
    CHECK(cutoffs_for(100000) == expected);
}

TEST_CASE("stars: fractional products round up") {
    // 12345.6 -> 12346, 345676.8 -> 345677, 444441.6 -> 444442, 543206.4 -> 543207.
    const std::array<int64_t, 7> expected{12346, 61728, 123456, 246912, 345677, 444442, 543207};
    CHECK(cutoffs_for(123456) == expected);
}

TEST_CASE("stars: the multiply is 32-bit float, as in the game") {
    // Exact: 786437 * 3.6 = 2831173.2, which rounds up to 2831174. The game
    // multiplies in float, where the product lands on 2831173.0, so its
    // cutoff is 2831173. The other six agree with exact math.
    const std::array<int64_t, 7> expected{78644, 393219, 786437, 1572874, 2202024, 2831173, 3460323};
    CHECK(cutoffs_for(786437) == expected);
}

TEST_CASE("stars: a zero base gives zero cutoffs") {
    const std::array<int64_t, 7> expected{};
    CHECK(cutoffs_for(0) == expected);
}

TEST_CASE("stars: star_cutoffs reads the path's base score and solo bonus") {
    Path path;
    path.score_base = 1000;
    path.score_ghosts = 100;
    path.score_accents = 200;
    path.score_solo = 800;
    path.score_combo = 5000;  // the multiplier's share: never part of the base
    path.score_sp = 3000;

    CHECK(path.chart_base_score() == 1300);

    const StarCutoffs sc = star_cutoffs(path);
    CHECK(sc.base == 1300);
    CHECK(sc.solo_bonus == 800);
    const std::array<int64_t, 7> expected{130, 650, 1300, 2600, 3640, 4680, 5720};
    CHECK(sc.cutoffs == expected);
}

TEST_CASE("stars: avg_mult divides by the same base score") {
    Path path;
    path.score_base = 1000;
    path.score_ghosts = 100;
    path.score_accents = 200;
    path.score_combo = 1300;
    path.score_solo = 800;
    // total 3400, minus the 800 solo bonus = 2600, over a base of 1300.
    CHECK(path.avg_mult() == doctest::Approx(2.0));
}

TEST_CASE("stars: the base score is the sum of every note's basescore, on every path") {
    // "87" by Polyphia: cymbals and a drum solo. The engine adds the base
    // score up by category (base + ghosts + accents); ChordNote::basescore
    // prices one note. This ties the two together on a real chart, so they
    // can't drift apart, and checks the Stars tab's premise that every path
    // of a record has the same base score and solo bonus.
    const std::string chart =
        corpus::root() + "/common/Summer Blast _25 Setlist/Tier 6/Polyphia - 87/notes.chart";
    app::AnalysisSettings settings;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 10;
    settings.ms_filter = 10.0;

    const Song& song = corpus::song(chart, settings.prodrums, settings.bass2x,
                                    settings.difficulty, settings.rules);
    const HydraRecord& record = corpus::analyzed(chart, settings);
    REQUIRE(!song.is_empty());
    REQUIRE(!record.paths.empty());

    int64_t note_sum = 0;
    int special_notes = 0;
    for (const SongTimestamp& ts : song.sequence) {
        for (const ChordNote& note : ts.chord.notes()) {
            note_sum += note.basescore();
            if (note.is_cymbal() || note.is_dynamic()) ++special_notes;
        }
    }
    // The chart must exercise more than plain 50-point gems, or the sum
    // proves little.
    REQUIRE(special_notes > 0);

    const Path& best = record.best_path();
    MESSAGE("87: base score " << best.chart_base_score() << ", note sum " << note_sum
                              << ", solo bonus " << best.score_solo);
    CHECK(best.chart_base_score() == note_sum);
    CHECK(best.chart_base_score() == 137950);
    CHECK(best.score_solo > 0);

    int checked = 0;
    for (const Path* p : record.all_paths()) {
        CHECK(p->chart_base_score() == best.chart_base_score());
        CHECK(p->score_solo == best.score_solo);
        ++checked;
    }
    for (const Path* p : record.all_allzero_paths()) {
        CHECK(p->chart_base_score() == best.chart_base_score());
        CHECK(p->score_solo == best.score_solo);
        ++checked;
    }
    MESSAGE("87: paths checked (variants and all-0 paths included): " << checked);
    CHECK(checked > 1);
}

TEST_CASE("stars: path_stars counts cutoffs reached without the solo bonus") {
    // Base 1300, so the cutoffs are 130, 650, 1300, 2600, 3640, 4680, 5720
    // (the star_cutoffs case above).
    Path path;
    path.score_base = 1000;
    path.score_ghosts = 100;
    path.score_accents = 200;
    const StarCutoffs sc = star_cutoffs(path);

    CHECK(stars_for_score(sc, 0) == 0);
    CHECK(stars_for_score(sc, 129) == 0);
    CHECK(stars_for_score(sc, 130) == 1);    // a cutoff counts when reached
    CHECK(stars_for_score(sc, 4679) == 5);
    CHECK(stars_for_score(sc, 4680) == 6);
    CHECK(stars_for_score(sc, 5720) == 7);
    CHECK(stars_for_score(sc, 1000000) == 7);  // the game stops at 7

    // 1300 base + 3380 combo = 4680 without the solo bonus: 6 stars. A solo
    // bonus big enough to pass the 7-star cutoff doesn't count.
    path.score_combo = 3380;
    path.score_solo = 2000;
    CHECK(path.totalscore() == 6680);
    CHECK(path_stars(path) == 6);
    path.score_combo = 3379;
    CHECK(path_stars(path) == 5);
}

TEST_CASE("stars: Burnout's optimal path earns 7 stars") {
    // Green Day - Burnout at Expert, Pro Drums, 2x Bass, cap 4: optimal
    // 378,315 against a 7-star cutoff of 335,500 (the plan's reference data).
    const std::string chart = corpus::root() +
        "/common/Summer Blast _25 Setlist/Tier 4/Green Day - Burnout/notes.mid";
    const app::AnalysisSettings settings = scratch_settings().to_analysis_settings();
    const HydraRecord& record = corpus::analyzed(chart, settings);
    REQUIRE(!record.paths.empty());
    const Path& best = record.best_path();
    CHECK(best.totalscore() == 378315);
    CHECK(star_cutoffs(best).cutoffs[kMaxStars - 1] == 335500);
    CHECK(path_stars(best) == 7);
}
