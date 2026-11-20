// Unit tests for core/stars: Clone Hero's star cutoffs, as the game computes
// them (float multiply, then round up), and where their inputs come from.

#include "doctest.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "core/backend_value.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/scoring.h"
#include "core/stars.h"
#include "core/timing.h"
#include "corpus_util.h"
#include "scratch_settings.h"

using namespace hydra;

namespace {

std::array<int64_t, kMaxStars> cutoffs_for(int64_t chart_base_score) {
    std::array<int64_t, kMaxStars> out{};
    for (int stars = 1; stars <= kMaxStars; ++stars)
        out[stars - 1] = star_cutoff(chart_base_score, stars);
    return out;
}

// A chord holding these notes, each on its own pad.
Chord chord_of(std::initializer_list<ChordNote> notes) {
    Chord c;
    for (const ChordNote& n : notes) c.at(n.colortype) = n;
    return c;
}

// Every combo from 0 up to the first one at the top multiplier, so a walk over
// it meets every band of to_multiplier. The top is the multiplier at the
// largest combo an int holds, so the walk ends there at the latest.
std::vector<int> combos_through_top_multiplier() {
    const int last = (std::numeric_limits<int>::max)();
    const int top = to_multiplier(last);
    std::vector<int> out;
    for (int combo = 0;; ++combo) {
        out.push_back(combo);
        if (to_multiplier(combo) == top || combo == last) break;
    }
    return out;
}

// The first combo above `combo` whose multiplier is higher than combo's own:
// the next step of to_multiplier. Fails the test when there is none.
int next_multiplier_step(int combo) {
    const int last = (std::numeric_limits<int>::max)();
    int step = combo + 1;
    while (step < last && to_multiplier(step) <= to_multiplier(combo)) ++step;
    REQUIRE_MESSAGE(to_multiplier(step) > to_multiplier(combo),
                    "to_multiplier has no step above combo " << combo);
    return step;
}

// A path with a share in every one of the six score categories, so each star
// test can see which categories its answer reads and which it leaves out.
Path path_with_every_category() {
    Path path;
    path.notes_points = 1000;
    path.score_ghosts = 100;
    path.score_accents = 200;
    path.score_solo = 800;
    path.score_combo = 5000;
    path.score_sp = 3000;
    return path;
}

// The exact product of `base` and star `stars`'s multiplier, in tenths of a
// point. Every entry of the game's table is a whole number of tenths, which
// this checks, so the product is exact in integers.
int64_t exact_product_in_tenths(int64_t base, int stars) {
    const float multiplier = kStarMultipliers.at(static_cast<size_t>(stars - 1));
    const long tenths = std::lround(multiplier * 10.0f);
    REQUIRE(std::fabs(multiplier * 10.0f - static_cast<float>(tenths)) < 1e-4f);
    return base * tenths;
}

// Each cutoff for `base` is its exact product rounded up: at or above the
// product, and less than one point above it.
void check_cutoffs_round_the_exact_product_up(int64_t base) {
    for (int stars = 1; stars <= kMaxStars; ++stars) {
        CAPTURE(stars);
        const int64_t product_tenths = exact_product_in_tenths(base, stars);
        const int64_t cutoff = star_cutoff(base, stars);
        CHECK(cutoff * 10 >= product_tenths);
        CHECK((cutoff - 1) * 10 < product_tenths);
    }
}

}  // namespace

TEST_CASE("scoring: one note's value at every combo band, by category") {
    // A note's 1x price: a pad's kNoteBasePoints, plus kCymbalBonusPoints on a
    // cymbal, doubled for a ghost or an accent.
    struct Row {
        const char* name;
        ChordNote note;
        int plain_price;  // the same note with no ghost or accent
    };
    const int tom = kNoteBasePoints;
    const int cymbal = kNoteBasePoints + kCymbalBonusPoints;
    const Row rows[] = {
        {"tom", ChordNote{NoteColor::Red}, tom},
        {"cymbal",
         ChordNote{NoteColor::Yellow, NoteDynamicType::Normal, NoteCymbalType::Cymbal, false},
         cymbal},
        {"ghost tom", ChordNote{NoteColor::Red, NoteDynamicType::Ghost}, tom},
        {"accent cymbal",
         ChordNote{NoteColor::Yellow, NoteDynamicType::Accent, NoteCymbalType::Cymbal, false},
         cymbal},
    };
    for (const Row& row : rows) {
        const Chord chord = chord_of({row.note});
        const int price = row.note.is_dynamic() ? 2 * row.plain_price : row.plain_price;
        CAPTURE(std::string(row.name));
        CHECK(row.note.basescore() == price);
        for (int combo : combos_through_top_multiplier()) {
            CAPTURE(combo);
            // The note is hit from `combo`, so it lands one combo later.
            const int multiplier = to_multiplier(combo + 1);
            const int paid = price * multiplier;
            std::vector<CategoryScores> per_note;
            const CategoryScores total = category_scores(chord, combo, &per_note);
            REQUIRE(per_note.size() == 1);
            const CategoryScores& n = per_note[0];
            CHECK(n.multiplier == multiplier);
            // A ghost or accent's own category holds the pad's base points;
            // the notes category holds the rest of the 1x price.
            CHECK(n.accent == (row.note.is_accent() ? kNoteBasePoints : 0));
            CHECK(n.ghost == (row.note.is_ghost() ? kNoteBasePoints : 0));
            CHECK(n.notes + n.accent + n.ghost == price);
            // The combo share is what the multiplier adds on top of the price.
            CHECK(n.combo == paid - price);
            // Star Power pays the paid value kStarPowerMultiplier times.
            CHECK(n.sp == (kStarPowerMultiplier - 1) * paid);
            // A ghost or accent earns its paid value less the plain note's.
            CHECK(n.dynamics_bonus == paid - row.plain_price * multiplier);
            // Squeezing the note out loses its paid value.
            CHECK(n.sqout_reduction == paid);

            // The chord total of a one-note chord is that note.
            CHECK(total.notes == n.notes);
            CHECK(total.combo == n.combo);
            CHECK(total.sp == n.sp);
            CHECK(total.accent == n.accent);
            CHECK(total.ghost == n.ghost);
            CHECK(total.sqout_reduction == n.sqout_reduction);
        }
    }
}

TEST_CASE("scoring: squeezing out any chord always costs points") {
    // A counted row's cost is core::sqout_cost, which comes to the chord's
    // sqout_reduction, so it can never be 0 (path_view's squeeze sentence
    // relies on that). Every pad, every ghost/accent, the flag each lane can
    // carry (lane_allows_flag, set_lane_flag), every combo band, both
    // squeeze-out rules; one-note chords and the chord on all five pads.
    const NoteColor pads[] = {NoteColor::Kick, NoteColor::Red, NoteColor::Yellow,
                              NoteColor::Blue, NoteColor::Green};
    const NoteDynamicType dynamics[] = {NoteDynamicType::Normal, NoteDynamicType::Ghost,
                                        NoteDynamicType::Accent};
    const core::SqOutRule rules[] = {core::SqOutRule::FirstNote, core::SqOutRule::WholeChord};
    std::vector<Chord> chords;
    Chord all_pads;
    for (NoteColor pad : pads) {
        all_pads.at(pad) = ChordNote{pad};
        for (NoteDynamicType dyn : dynamics)
            for (bool flagged : {false, true}) {
                if (flagged && !lane_allows_flag(pad)) continue;
                ChordNote note{pad, dyn};
                if (flagged) set_lane_flag(note);
                chords.push_back(chord_of({note}));
            }
    }
    chords.push_back(all_pads);
    for (const Chord& chord : chords)
        for (core::SqOutRule rule : rules)
            for (int combo : combos_through_top_multiplier()) {
                CAPTURE(chord.rowstr());
                CAPTURE(combo);
                std::vector<CategoryScores> per_note;
                const CategoryScores sg = category_scores(chord, combo, &per_note, rule);
                // The squeeze-out loses the Star Power share of every note
                // the rule pays outside Star Power, and nothing else.
                int lost = 0;
                for (size_t place = 0; place < per_note.size(); ++place)
                    if (sqout_pays_outside(place, rule)) lost += per_note[place].sp;
                CHECK(sg.sqout_reduction == lost);
                // Never 0: the first note is paid outside Star Power under
                // either rule, and every note is worth points.
                REQUIRE(!per_note.empty());
                CHECK(sqout_pays_outside(0, rule));
                CHECK(per_note[0].sp > 0);
                CHECK(sg.sqout_reduction > 0);
                // A row squeezed out at 0 ms is counted whatever the leeway,
                // so its cost is exactly the reduction.
                CHECK(core::sqout_cost(0.0, sg.sp, sg.sqout_sp(), 0.0) == sg.sqout_reduction);
            }
}

TEST_CASE("scoring: combo_after is the combo plus the chord's notes") {
    const Chord chord = chord_of(
        {ChordNote{NoteColor::Red},
         ChordNote{NoteColor::Yellow, NoteDynamicType::Normal, NoteCymbalType::Cymbal, false},
         ChordNote{NoteColor::Kick}});
    const int notes = chord.count();
    REQUIRE(notes > 1);
    // Hit from here, the chord's last note lands on the first multiplier step
    // and its first note below it.
    const int start = next_multiplier_step(0) - notes;
    REQUIRE(start >= 0);
    std::vector<CategoryScores> per_note;
    const CategoryScores total = category_scores(chord, start, &per_note);
    CHECK(total.combo_after == start + notes);
    CHECK(total.multiplier == to_multiplier(start + 1));
    CHECK(total.multiplier_after == to_multiplier(start + notes));
    CHECK(total.multiplier != total.multiplier_after);
    REQUIRE(per_note.size() == static_cast<size_t>(notes));
    for (int i = 0; i < notes; ++i) {
        CAPTURE(i);
        CHECK(per_note[static_cast<size_t>(i)].combo_after == start + 1 + i);
        CHECK(per_note[static_cast<size_t>(i)].multiplier == to_multiplier(start + 1 + i));
    }
}

TEST_CASE("scoring: solo_note_count is every note in a solo, else 0") {
    const Chord two = chord_of({ChordNote{NoteColor::Red}, ChordNote{NoteColor::Kick}});
    CHECK(solo_note_count(two, true) == 2);
    CHECK(solo_note_count(two, false) == 0);
}

TEST_CASE("scoring: solo_bonus pays kSoloBonusPerNote for each note in a solo, else 0") {
    const Chord one = chord_of({ChordNote{NoteColor::Red}});
    const Chord three = chord_of(
        {ChordNote{NoteColor::Red},
         ChordNote{NoteColor::Yellow, NoteDynamicType::Normal, NoteCymbalType::Cymbal, false},
         ChordNote{NoteColor::Kick}});
    REQUIRE(three.count() > one.count());
    CHECK(solo_bonus(one, true) == kSoloBonusPerNote);
    // Per note, not per chord.
    CHECK(solo_bonus(three, true) == three.count() * solo_bonus(one, true));
    CHECK(solo_bonus(three, false) == 0);
    CHECK(solo_bonus(one, false) == 0);
}

TEST_CASE("stars: with_solo is each cutoff plus the solo bonus") {
    Path path = path_with_every_category();
    REQUIRE(path.score_solo > 0);
    const StarCutoffs sc = star_cutoffs(path);
    for (size_t i = 0; i < sc.cutoffs.size(); ++i) {
        CAPTURE(i);
        CHECK(sc.with_solo[i] == sc.cutoffs[i] + path.score_solo);
    }

    // No solo bonus: the two columns agree.
    path.score_solo = 0;
    const StarCutoffs no_solo = star_cutoffs(path);
    CHECK(no_solo.with_solo == no_solo.cutoffs);
}

TEST_CASE("stars: score_without_solo is the total minus the solo bonus") {
    const Path path = path_with_every_category();
    REQUIRE(path.score_solo > 0);
    // Stars count every category but the solo bonus.
    const int64_t counted = path.notes_points + path.score_ghosts + path.score_accents +
                            path.score_combo + path.score_sp;
    CHECK(score_without_solo(path) == counted);

    // A replay's running score is the same six categories, so the ReplayScore
    // form gives the same answer for the same numbers.
    ReplayScore running;
    running.notes = path.notes_points;
    running.ghost = path.score_ghosts;
    running.accent = path.score_accents;
    running.combo = path.score_combo;
    running.sp = path.score_sp;
    running.solo = path.score_solo;
    CHECK(score_without_solo(running) == counted);
}

TEST_CASE("stars: each star needs a higher multiplier than the one before") {
    CHECK(kStarMultipliers[0] > 0.0f);
    for (size_t i = 1; i < kStarMultipliers.size(); ++i) {
        CAPTURE(i);
        CHECK(kStarMultipliers[i] > kStarMultipliers[i - 1]);
    }
}

TEST_CASE("stars: whole-number products come out exact") {
    // A chart of 2000 plain toms: a base whose every product is whole.
    const int64_t base = 2000 * static_cast<int64_t>(ChordNote{NoteColor::Red}.basescore());
    for (int stars = 1; stars <= kMaxStars; ++stars) {
        CAPTURE(stars);
        REQUIRE(exact_product_in_tenths(base, stars) % 10 == 0);
    }
    check_cutoffs_round_the_exact_product_up(base);
}

TEST_CASE("stars: fractional products round up") {
    // A chart of one tom and one cymbal: a base a real chart can have, whose
    // 1-star product ends in half a point.
    const int64_t base =
        ChordNote{NoteColor::Red}.basescore() +
        ChordNote{NoteColor::Yellow, NoteDynamicType::Normal, NoteCymbalType::Cymbal, false}
            .basescore();
    REQUIRE(exact_product_in_tenths(base, 1) % 10 != 0);
    check_cutoffs_round_the_exact_product_up(base);
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
    Path path = path_with_every_category();
    // The base is the notes, ghosts and accents categories.
    const int64_t base = path.notes_points + path.score_ghosts + path.score_accents;
    CHECK(path.chart_base_score() == base);

    const StarCutoffs sc = star_cutoffs(path);
    CHECK(sc.chart_base_score == base);
    CHECK(sc.solo_bonus == path.score_solo);
    for (int stars = 1; stars <= kMaxStars; ++stars) {
        CAPTURE(stars);
        CHECK(sc.cutoffs[static_cast<size_t>(stars - 1)] == star_cutoff(base, stars));
    }

    // The combo and Star Power shares are never part of the base.
    REQUIRE(path.score_combo > 0);
    REQUIRE(path.score_sp > 0);
    path.score_combo = 0;
    path.score_sp = 0;
    CHECK(path.chart_base_score() == base);
    CHECK(star_cutoffs(path).cutoffs == sc.cutoffs);
}

TEST_CASE("stars: avg_mult divides the score without the solo bonus by the base score") {
    const Path path = path_with_every_category();
    REQUIRE(path.chart_base_score() > 0);
    CHECK(path.avg_mult() == doctest::Approx(static_cast<double>(score_without_solo(path)) /
                                             static_cast<double>(path.chart_base_score())));
}

TEST_CASE("stars: the base score is the sum of every note's basescore, on every path") {
    // "87" by Polyphia: cymbals and a drum solo. The engine adds the base
    // score up by category (notes + ghosts + accents); ChordNote::basescore
    // prices one note. This ties the two together on a real chart, so they
    // can't drift apart, and checks the Stars tab's premise that every path
    // of a record has the same base score and solo bonus.
    const std::string chart =
        corpus::root() + "/common/Summer Blast _25 Setlist/Tier 6/Polyphia - 87/notes.chart";
    app::AnalysisSettings settings;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 10;
    settings.ms_filter = 10.0;

    const Song& song = corpus::song(chart, settings.reading(), settings.rules);
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
    CHECK(best.score_solo > 0);

    const std::vector<const Path*> paths = record.all_paths();
    const std::vector<const Path*> allzero_paths = record.all_allzero_paths();
    // The premise compares the best path with the record's other paths, so
    // the record must hold at least one besides it.
    REQUIRE(paths.size() + allzero_paths.size() > 1);
    for (const std::vector<const Path*>* list : {&paths, &allzero_paths}) {
        for (const Path* p : *list) {
            CHECK(p->chart_base_score() == best.chart_base_score());
            CHECK(p->score_solo == best.score_solo);
        }
    }
}

TEST_CASE("stars: path_stars counts cutoffs reached without the solo bonus") {
    Path path = path_with_every_category();
    path.score_combo = 0;
    path.score_sp = 0;
    path.score_solo = 0;
    const StarCutoffs sc = star_cutoffs(path);
    REQUIRE(sc.cutoffs[0] > 0);

    // A cutoff counts once it is reached, and not one point before.
    CHECK(stars_for_score(sc, 0) == 0);
    for (int stars = 1; stars <= kMaxStars; ++stars) {
        CAPTURE(stars);
        const int64_t cutoff = sc.cutoffs[static_cast<size_t>(stars - 1)];
        CHECK(stars_for_score(sc, cutoff) == stars);
        CHECK(stars_for_score(sc, cutoff - 1) == stars - 1);
    }
    // The game stops counting at kMaxStars, whatever the score.
    CHECK(stars_for_score(sc, (std::numeric_limits<int64_t>::max)()) == kMaxStars);

    // A solo bonus that alone passes the top cutoff adds no star: path_stars
    // counts only what the path scores without it.
    const int64_t base = path.chart_base_score();
    const int64_t top_cutoff = sc.cutoffs[kMaxStars - 1];
    path.score_solo = top_cutoff;
    for (int stars = 1; stars <= kMaxStars; ++stars) {
        CAPTURE(stars);
        const int64_t cutoff = sc.cutoffs[static_cast<size_t>(stars - 1)];
        // A path hits every note, so it scores at least its base; a cutoff
        // below the base has no path one point short of it.
        if (cutoff <= base) continue;
        path.score_combo = cutoff - base;
        CHECK(path.totalscore() > top_cutoff);
        CHECK(path_stars(path) == stars);
        path.score_combo = cutoff - base - 1;
        CHECK(path_stars(path) == stars - 1);
    }
}

TEST_CASE("stars: Burnout's optimal path earns every star") {
    // Green Day - Burnout at Expert, Pro Drums, 2x Bass, cap 4, under the
    // settings the reference cases share (scratch_settings).
    const std::string chart = corpus::burnout_notes();
    const app::AnalysisSettings settings = scratch_settings().to_analysis_settings();
    const HydraRecord& record = corpus::analyzed(chart, settings);
    REQUIRE(!record.paths.empty());
    const Path& best = record.best_path();
    const StarCutoffs sc = star_cutoffs(best);
    const int64_t counted = score_without_solo(best);
    // The star count is the number of cutoffs the score reaches without the
    // solo bonus, read straight from the table.
    int reached = 0;
    for (int64_t cutoff : sc.cutoffs)
        if (counted >= cutoff) ++reached;
    CHECK(path_stars(best) == reached);
    // The optimal path clears the top cutoff without its solo bonus.
    CHECK(counted >= sc.cutoffs[kMaxStars - 1]);
    CHECK(path_stars(best) == kMaxStars);
}
