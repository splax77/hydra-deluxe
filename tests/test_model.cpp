// Unit tests for core/model (chord encoding, squeezes). The chord-code
// round-trip lives in test_chord_code.cpp and the transfer scales in
// test_squeeze_rating.cpp; the cases here pin the string/number forms
// directly.

#include "doctest.h"

#include <cmath>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "core/model.h"
#include "core/scoring.h"
#include "core/timing.h"
#include "core/backend_value.h"
#include "core/rules.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/graph.h"
#include "search/pather.h"

using namespace hydra;

TEST_CASE("basescore matches ChordNote.basescore") {
    CHECK(ChordNote{NoteColor::Red}.basescore() == 50);
    CHECK(ChordNote{NoteColor::Yellow, NoteDynamicType::Normal,
                    NoteCymbalType::Cymbal, false}
              .basescore() == 65);
    CHECK(ChordNote{NoteColor::Red, NoteDynamicType::Accent}.basescore() == 100);
    CHECK(ChordNote{NoteColor::Yellow, NoteDynamicType::Ghost,
                    NoteCymbalType::Cymbal, false}
              .basescore() == 130);
}

// The kick and the green tie at 50, so the sort must keep their lane order:
// the order category_scores prices a chord's notes in. note_list owns it and
// notes(true) copies it, so both give these lanes.
TEST_CASE("note_list(true) orders a tied chord as notes(true) always has") {
    Chord c;
    c.at(NoteColor::Kick) = ChordNote{NoteColor::Kick};
    c.at(NoteColor::Yellow) = ChordNote{NoteColor::Yellow, NoteDynamicType::Normal,
                                        NoteCymbalType::Cymbal, false};
    c.at(NoteColor::Blue) = ChordNote{NoteColor::Blue, NoteDynamicType::Accent};
    c.at(NoteColor::Green) = ChordNote{NoteColor::Green};
    const std::vector<NoteColor> sorted = {NoteColor::Kick, NoteColor::Green,
                                           NoteColor::Yellow, NoteColor::Blue};

    const auto listed = c.note_list(true);
    REQUIRE(listed.size() == sorted.size());
    for (size_t i = 0; i < sorted.size(); ++i) CHECK(listed[i].colortype == sorted[i]);

    const std::vector<ChordNote> copied = c.notes(true);
    REQUIRE(copied.size() == sorted.size());
    for (size_t i = 0; i < sorted.size(); ++i) CHECK(copied[i].colortype == sorted[i]);

    // Unsorted, the notes come in lane order.
    const std::vector<NoteColor> lanes = {NoteColor::Kick, NoteColor::Yellow, NoteColor::Blue,
                                          NoteColor::Green};
    const auto unsorted = c.note_list(false);
    REQUIRE(unsorted.size() == lanes.size());
    size_t i = 0;
    for (const ChordNote& note : unsorted) CHECK(note.colortype == lanes[i++]);
}

TEST_CASE("note value: one owner for base, cymbal and dynamic points") {
    CHECK(kNoteBasePoints == 50);
    CHECK(kCymbalBonusPoints == 15);
    CHECK(kSoloBonusPerNote == 100);
    const ChordNote accent_cymbal{NoteColor::Yellow, NoteDynamicType::Accent,
                                  NoteCymbalType::Cymbal, false};
    CHECK(accent_cymbal.basescore() == (kNoteBasePoints + kCymbalBonusPoints) * 2);
    CHECK(ChordNote{NoteColor::Red}.basescore() == kNoteBasePoints);
}

TEST_CASE("category_scores: the squeeze-out cut is basescore at each note's multiplier") {
    Chord c;
    c.at(NoteColor::Red) = ChordNote{NoteColor::Red};
    c.at(NoteColor::Yellow) = ChordNote{NoteColor::Yellow, NoteDynamicType::Accent,
                                        NoteCymbalType::Cymbal, false};
    const std::vector<ChordNote> notes = c.notes(true);
    REQUIRE(notes.size() == 2);
    // The cuts for this chord (a Red tom worth 50 and an accented Yellow
    // cymbal worth 130), pinned from one run of category_scores, one row per
    // combo before the chord. The combos straddle the multiplier steps, so
    // the two notes sometimes pay at different ones.
    struct Row {
        int combo;
        int first_cut;  // first_note: only the Red tom loses its doubling
        int whole_cut;  // whole_chord: both notes lose it
    };
    const Row rows[] = {
        {0, 50, 180}, {8, 50, 310}, {9, 100, 360}, {29, 200, 720}, {45, 200, 720},
    };
    for (const Row& row : rows) {
        const int combo = row.combo;
        CAPTURE(combo);
        // first_note (the default): only note 0 loses its SP doubling.
        std::vector<CategoryScores> per_note;
        const CategoryScores cs = category_scores(c, combo, &per_note);
        CHECK(cs.sqout_reduction == row.first_cut);
        REQUIRE(per_note.size() == 2);
        CHECK(per_note[0].sqout_reduction == row.first_cut);
        CHECK(per_note[1].sqout_reduction == 0);
        CHECK(cs.sqout_sp() == cs.sp - cs.sqout_reduction);

        // whole_chord: every note loses it, each at its own multiplier. The
        // Red tom's share is the same as under first_note.
        std::vector<CategoryScores> whole_per_note;
        const CategoryScores whole =
            category_scores(c, combo, &whole_per_note, core::SqOutRule::WholeChord);
        REQUIRE(whole_per_note.size() == 2);
        CHECK(whole_per_note[0].sqout_reduction == row.first_cut);
        CHECK(whole_per_note[0].sqout_reduction + whole_per_note[1].sqout_reduction ==
              row.whole_cut);
        CHECK(whole.sqout_reduction == row.whole_cut);
        CHECK(whole.sqout_sp() == whole.sp - whole.sqout_reduction);
    }
}

// The copied path writes a needed timing with one decimal, as every screen
// does (D265).
TEST_CASE("notationstr_verbose: a 12.6 ms squeeze copies as 12.6 ms") {
    Activation a;
    test::set_skips(a, 0);
    a.e_offset = 300.0;  // not e-critical
    a.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 12.6});
    CHECK(a.notationstr_verbose() == "0+ (12.6 ms)");
}

TEST_CASE("squeeze symbols, timing, difficulty") {
    SPSqueeze sqin{SqueezeKind::SqIn, -5.0};
    SPSqueeze sqout{SqueezeKind::SqOut, 3.0};
    CHECK(std::string(sqin.symbol()) == "+");
    CHECK(std::string(sqout.symbol()) == "-");
    CHECK(sqin.difficulty() == -5.0);
    CHECK(sqout.difficulty() == -3.0);
    CHECK(sqin.timing() == 5.0);
    CHECK(std::string(sqin.type_name()) == "SqIn");
    CHECK(std::string(sqout.type_name()) == "SqOut");
}

TEST_CASE("squeeze_difficulty and is_e0: one owner for the engine and the model") {
    CHECK(squeeze_difficulty(/*is_sqin=*/true, 3.5) == 3.5);
    CHECK(squeeze_difficulty(/*is_sqin=*/false, -3.5) == 3.5);
    // A SqOut at exactly 0 is +0.0, never -0.0: the "-x + 0.0" idiom stays.
    CHECK_FALSE(std::signbit(squeeze_difficulty(false, 0.0)));
    CHECK(SPSqueeze{SqueezeKind::SqOut, -3.5}.difficulty() == squeeze_difficulty(false, -3.5));

    const double w = kDefaultEarlyFillWindowMs;
    CHECK(is_e0(w - 0.1, 0, w));
    CHECK_FALSE(is_e0(w, 0, w));
    CHECK_FALSE(is_e0(10.0, 1, w));
    CHECK(early_fill_difficulty(-4.0) == 4.0);
    CHECK_FALSE(std::signbit(early_fill_difficulty(0.0)));

    Activation a;
    a.e_offset = 10.0;
    test::set_skips(a, 0);
    CHECK(a.is_E0() == is_e0(10.0, 0, a.e_window_ms));
    REQUIRE(a.e_difficulty().has_value());
    CHECK(*a.e_difficulty() == early_fill_difficulty(10.0));
}

// The all-0 rule (D102): no skips, or one skip that was the early fill. Run
// at the default window and at the widest one the setting allows.
TEST_CASE("allzero_activation: no skips, or an E1") {
    for (const double w : {kDefaultEarlyFillWindowMs, kHitWindowMs}) {
        CAPTURE(w);
        CHECK(allzero_activation(500.0, 0, w));               // a plain 0
        CHECK(allzero_activation(w - 0.1, 0, w));             // an E0
        CHECK(allzero_activation(w - 0.1, 1, w));             // an E1
        CHECK(allzero_activation(-w, 1, w));                  // an E1, as late as a fill spawns
        CHECK_FALSE(allzero_activation(w, 1, w));             // a plain 1
        CHECK_FALSE(allzero_activation(10.0, 2, w));          // an E2 skips a real fill
    }
    // One offset, two windows: 70 ms to spare is an E1 at 85 and a plain 1 at 60.
    CHECK(allzero_activation(70.0, 1, kHitWindowMs));
    CHECK_FALSE(allzero_activation(70.0, 1, kDefaultEarlyFillWindowMs));

    Path p;
    Activation e1;
    e1.e_offset = 10.0;
    test::set_skips(e1, 1);
    p.activations.push_back(e1);
    CHECK(p.is_allzero());
    p.activations.front().e_offset = e1.e_window_ms;
    CHECK_FALSE(p.is_allzero());
    // Path::is_allzero reads each activation's own window.
    p.activations.front().e_offset = 70.0;
    p.activations.front().e_window_ms = kHitWindowMs;
    CHECK(p.is_allzero());
    p.activations.front().e_window_ms = kDefaultEarlyFillWindowMs;
    CHECK_FALSE(p.is_allzero());
}

// The early-fill window has one owner: model.h states both sides of it, the
// fill that refuses (fill_refuses) and the E0 (is_e0), both read from the
// window the caller passes. Each side's edge is checked at the default and at
// the widest window the setting allows, so a side that read a constant
// instead of its argument fails at one of them.
TEST_CASE("early-fill window: the refusal and the E0 edges") {
    CHECK(fill_e_offset(10000.0, 10050.0) == -50.0);
    for (const double w : {kDefaultEarlyFillWindowMs, kHitWindowMs}) {
        CAPTURE(w);
        CHECK_FALSE(fill_refuses(-w, w));  // ready exactly the window late still spawns
        CHECK(fill_refuses(-w - 0.1, w));  // a hair later refuses
        CHECK_FALSE(is_e0(w, 0, w));       // exactly the window to spare: no E
        CHECK(is_e0(w - 0.1, 0, w));       // a hair less: an E

        Activation a;
        a.e_window_ms = w;
        a.e_offset = w - 0.1;  // SP ready just under the window before the deadline
        test::set_skips(a, 1);
        CHECK(a.is_e_critical());
        CHECK(a.is_e_critical() == is_e0(a.e_offset, 0, w));
        a.e_offset = w;  // exactly the window to spare: no E
        CHECK_FALSE(a.is_e_critical());
        CHECK(a.is_e_critical() == is_e0(a.e_offset, 0, w));
    }
    // Between the two windows the answers differ, so neither side can be a
    // fixed number: 70 ms late spawns at 85 and refuses at 60, and 70 ms to
    // spare is an E at 85 and not at 60.
    CHECK_FALSE(fill_refuses(-70.0, kHitWindowMs));
    CHECK(fill_refuses(-70.0, kDefaultEarlyFillWindowMs));
    CHECK(is_e0(70.0, 0, kHitWindowMs));
    CHECK_FALSE(is_e0(70.0, 0, kDefaultEarlyFillWindowMs));
    CHECK_FALSE(fill_within_limit(70.0, 0, -80.0, kHitWindowMs));
    CHECK(fill_within_limit(70.0, 0, -80.0, kDefaultEarlyFillWindowMs));
}

// Both sides read one number (O13): every offset at which a fill refuses is
// an E0 at the same window, which Engine::ready_class leans on. Walked over
// windows from 0 to the widest, and offsets on both sides of each edge.
TEST_CASE("early-fill window: every fill that refuses is an E0 at the same window") {
    for (const double w : {0.0, 1.0, 30.0, kDefaultEarlyFillWindowMs, 70.0, kHitWindowMs}) {
        CAPTURE(w);
        int refusing = 0;
        for (double e = -200.0; e <= 200.0; e += 0.5) {
            CAPTURE(e);
            if (!fill_refuses(e, w)) continue;
            ++refusing;
            CHECK(is_e0(e, 0, w));
        }
        CHECK(refusing > 0);
    }
}

// The 0 ms window (O19): a fill spawns only when SP was ready by its
// deadline, and no activation that spawns is an early fill.
TEST_CASE("early-fill window: at 0 ms no spawning activation is an E") {
    CHECK(fill_refuses(-0.1, 0.0));        // a hair after the deadline refuses
    CHECK_FALSE(fill_refuses(0.0, 0.0));   // exactly on it spawns
    for (double e = 0.0; e <= 200.0; e += 0.5) {
        CAPTURE(e);
        CHECK_FALSE(fill_refuses(e, 0.0));
        CHECK_FALSE(is_e0(e, 0, 0.0));
        CHECK_FALSE(allzero_activation(e, 1, 0.0));
        CHECK(fill_within_limit(e, 0, 0.0, 0.0));
    }
    Activation a;
    a.e_window_ms = 0.0;
    a.e_offset = 0.0;
    test::set_skips(a, 0);
    CHECK_FALSE(a.is_e_critical());
    CHECK_FALSE(a.e_difficulty().has_value());
}

// The default window has one home, kDefaultEarlyFillWindowMs, and every
// carrier starts from it.
TEST_CASE("early-fill window: every default is kDefaultEarlyFillWindowMs") {
    CHECK(SearchSettings{}.early_fill_window_ms == kDefaultEarlyFillWindowMs);
    CHECK(Activation{}.e_window_ms == kDefaultEarlyFillWindowMs);
    CHECK(HydraRecord{}.early_fill_window_ms == kDefaultEarlyFillWindowMs);
    const Song song = test::ready_time_fold_song();
    const ScoreGraph graph(song, 2);
    CHECK(graph.early_fill_window_ms() == kDefaultEarlyFillWindowMs);
    // The default is inside the setting's range, 0 to the hit window.
    CHECK(kDefaultEarlyFillWindowMs >= 0.0);
    CHECK(kDefaultEarlyFillWindowMs <= kHitWindowMs);
}

TEST_CASE("Path::is_difficult: past the difficult floor, not at it") {
    Path empty;
    CHECK_FALSE(empty.is_difficult());

    Activation a;
    test::set_skips(a, 0);
    a.e_offset = 300.0;  // not e-critical
    a.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -(kDifficultMs + 0.5)});
    Path hard;
    hard.activations.push_back(a);
    CHECK(hard.is_difficult());

    Path edge = hard;
    edge.activations[0].sqinouts[0].offset_ms = -kDifficultMs;
    CHECK_FALSE(edge.is_difficult());  // exactly at the floor is not past it

    SUBCASE("past_difficult_floor: 2.0 ms is not past it") {
        // D48 Q3: a timing exactly on an edge is inside it.
        CHECK_FALSE(past_difficult_floor(2.0));
        CHECK(past_difficult_floor(2.1));
        CHECK_FALSE(SPSqueeze{SqueezeKind::SqIn, 2.0}.is_difficult());
        CHECK(SPSqueeze{SqueezeKind::SqIn, 2.1}.is_difficult());
        CHECK_FALSE(edge.activations[0].is_difficult());
        CHECK(hard.activations[0].is_difficult());
    }
}

TEST_CASE("HydraRecord::is_optimal: every path tied at the top score") {
    const HydraRecord rec = test::tied_variant_record();
    const Path& root = rec.paths.at(0);
    CHECK(rec.is_optimal(root));
    CHECK(rec.is_optimal(root.variants.at(0)));  // the tied variant
    CHECK_FALSE(rec.is_optimal(rec.paths.at(1)));
}

TEST_CASE("HydraRecord::optimal_score: the best path's total, empty with no paths") {
    const HydraRecord rec = test::tied_variant_record();
    REQUIRE(rec.optimal_score().has_value());
    CHECK(*rec.optimal_score() == rec.paths.at(0).totalscore());

    const HydraRecord empty{};
    CHECK_FALSE(empty.optimal_score().has_value());
    CHECK_FALSE(empty.is_optimal(rec.paths.at(0)));
}

TEST_CASE("multiplier_step: the multipliers at a chord's two ends") {
    struct Case {
        int combo;
        int notes;
        int first;
        int last;
    };
    // Literal answers; to_multiplier owns where the steps sit.
    const Case cases[] = {
        // No notes: the chord pays nothing, so both ends stay at the combo's.
        {9, 0, 1, 1}, {10, 0, 2, 2}, {29, 0, 3, 3},
        {7, 1, 1, 1}, {7, 2, 1, 1}, {7, 3, 1, 2}, {7, 4, 1, 2}, {7, 5, 1, 2},
        {8, 1, 1, 1}, {8, 2, 1, 2}, {8, 3, 1, 2}, {8, 4, 1, 2}, {8, 5, 1, 2},
        {9, 1, 2, 2}, {9, 2, 2, 2}, {9, 3, 2, 2}, {9, 4, 2, 2}, {9, 5, 2, 2},
        {10, 1, 2, 2}, {10, 2, 2, 2}, {10, 3, 2, 2}, {10, 4, 2, 2}, {10, 5, 2, 2},
        {18, 1, 2, 2}, {18, 2, 2, 3}, {18, 3, 2, 3}, {18, 4, 2, 3}, {18, 5, 2, 3},
        {19, 1, 3, 3}, {19, 2, 3, 3}, {19, 3, 3, 3}, {19, 4, 3, 3}, {19, 5, 3, 3},
        {28, 1, 3, 3}, {28, 2, 3, 4}, {28, 3, 3, 4}, {28, 4, 3, 4}, {28, 5, 3, 4},
        {29, 1, 4, 4}, {29, 2, 4, 4}, {29, 3, 4, 4}, {29, 4, 4, 4}, {29, 5, 4, 4},
    };
    for (const Case& c : cases) {
        const MultiplierStep step = multiplier_step(c.combo, c.notes);
        CHECK_MESSAGE(step.first == c.first, "combo=" << c.combo << " notes=" << c.notes);
        CHECK_MESSAGE(step.last == c.last, "combo=" << c.combo << " notes=" << c.notes);
        CHECK_MESSAGE(crosses_multiplier_step(c.combo, c.notes) == (c.first != c.last),
                      "combo=" << c.combo << " notes=" << c.notes);
    }
}

TEST_CASE("category_scores: a chord with no notes keeps the combo's multiplier at both ends") {
    const Chord empty{};
    const CategoryScores at9 = category_scores(empty, 9);
    CHECK(at9.multiplier == 1);
    CHECK(at9.multiplier_after == 1);
    CHECK(at9.combo_after == 9);
    const CategoryScores at10 = category_scores(empty, 10);
    CHECK(at10.multiplier == 2);
    CHECK(at10.multiplier_after == 2);
    CHECK(at10.combo_after == 10);
}

TEST_CASE("Activation::hardest: the part with its ms, a tie names the squeeze") {
    using P = TimingPart;
    auto plain = [] {
        Activation a;
        test::set_skips(a, 0);
        a.e_offset = 300.0;  // not e-critical
        return a;
    };

    // The larger of two SqIns, with its raw value.
    Activation two = plain();
    two.sqinouts = {SPSqueeze{SqueezeKind::SqIn, 12.4}, SPSqueeze{SqueezeKind::SqIn, 12.6}};
    CHECK(two.hardest() == std::optional<HardestTiming>(HardestTiming{P::SqueezeIn, 12.6}));
    two.sqinouts = {SPSqueeze{SqueezeKind::SqIn, 12.4}};
    CHECK(two.hardest() == std::optional<HardestTiming>(HardestTiming{P::SqueezeIn, 12.4}));

    // A required (E0) fill and a SqOut at the same ms: the squeeze.
    Activation tie = plain();
    tie.e_offset = -12.6;  // early fill 12.6 ms
    tie.sqinouts = {SPSqueeze{SqueezeKind::SqOut, -12.6}};
    REQUIRE(tie.is_E0());
    CHECK(tie.hardest() == std::optional<HardestTiming>(HardestTiming{P::SqueezeOut, 12.6}));

    // The fill alone, when it is harder.
    tie.sqinouts = {SPSqueeze{SqueezeKind::SqOut, -12.4}};
    CHECK(tie.hardest() == std::optional<HardestTiming>(HardestTiming{P::EarlyFill, 12.6}));

    // An E activation that skips one fill, with no squeeze: its optional
    // early fill (D48 Q10).
    Activation skip = plain();
    test::set_skips(skip, 1);
    skip.e_offset = -12.6;
    REQUIRE(skip.is_e_critical());
    REQUIRE_FALSE(skip.is_E0());
    CHECK(skip.hardest() == std::optional<HardestTiming>(HardestTiming{P::EarlyFill, 12.6}));
    // ...which difficulty() still leaves out.
    CHECK_FALSE(skip.difficulty().has_value());

    // Nothing to time: no badge.
    CHECK_FALSE(plain().hardest().has_value());
}

// D51 Q4; D302.
TEST_CASE("Activation::needs_timing: squeezes inside and slack fills need none") {
    auto plain = [] {
        Activation a;
        test::set_skips(a, 0);
        a.e_offset = 300.0;  // not e-critical
        return a;
    };

    // A squeeze-in 163 ms before the SP end is already in: nothing to time,
    // so nothing is hardest and there is no difficulty.
    Activation free_in = plain();
    free_in.sqinouts = {SPSqueeze{SqueezeKind::SqIn, -163.0}};
    CHECK_FALSE(free_in.needs_timing());
    CHECK_FALSE(free_in.hardest().has_value());
    CHECK_FALSE(free_in.difficulty().has_value());

    // A squeeze-in dead on the SP end still needs 0.0 ms, like the
    // squeeze-out on the same note (D302).
    Activation on_end_in = plain();
    on_end_in.sqinouts = {SPSqueeze{SqueezeKind::SqIn, 0.0}};
    CHECK(on_end_in.needs_timing());
    CHECK(on_end_in.difficulty() == std::optional<double>(0.0));
    CHECK(on_end_in.hardest() ==
          std::optional<HardestTiming>(HardestTiming{TimingPart::SqueezeIn, 0.0}));

    // A squeeze-out dead on the SP end still has to be hit late.
    Activation on_end_out = plain();
    on_end_out.sqinouts = {SPSqueeze{SqueezeKind::SqOut, 0.0}};
    CHECK(on_end_out.needs_timing());
    CHECK(on_end_out.difficulty() == std::optional<double>(0.0));

    Activation late_out = plain();
    late_out.sqinouts = {SPSqueeze{SqueezeKind::SqOut, -12.5}};
    CHECK(late_out.needs_timing());

    // A required (E0) fill with 10 ms to spare needs none; with none to
    // spare it does, like the squeeze-out on the SP end.
    Activation slack_fill = plain();
    slack_fill.e_offset = 10.0;
    REQUIRE(slack_fill.is_E0());
    CHECK_FALSE(slack_fill.needs_timing());
    CHECK_FALSE(slack_fill.hardest().has_value());
    Activation tight_fill = plain();
    tight_fill.e_offset = 0.0;
    REQUIRE(tight_fill.is_E0());
    CHECK(tight_fill.needs_timing());
    CHECK(tight_fill.difficulty() == std::optional<double>(0.0));

    // A path needs timing once any of its activations does.
    Path p;
    p.activations = {free_in, slack_fill};
    CHECK_FALSE(p.needs_timing());
    p.activations.push_back(on_end_in);
    CHECK(p.needs_timing());
    Path q;
    q.activations = {free_in, slack_fill, late_out};
    CHECK(q.needs_timing());
}

TEST_CASE("Activation::badge_timings: every timing in notation order; hardest() picks from it") {
    using P = TimingPart;
    // The user's E3+ row: an optional fill with no time to spare, then a free
    // squeeze-in. Both listed, neither weighed as required, neither difficult.
    Activation row;
    test::set_skips(row, 3);
    row.e_offset = 0.0;
    row.sqinouts = {SPSqueeze{SqueezeKind::SqIn, -12.0}};
    const std::vector<ActivationTiming> t = row.badge_timings();
    REQUIRE(t.size() == 2);
    CHECK(t[0].part == P::EarlyFill);
    CHECK(t[0].ms == 0.0);
    CHECK_FALSE(t[0].required);
    CHECK_FALSE(t[0].difficult);
    CHECK(t[1].part == P::SqueezeIn);
    CHECK(t[1].ms == -12.0);
    CHECK_FALSE(t[1].required);
    CHECK_FALSE(t[1].difficult);
    // The free squeeze never names the hardest part; the optional fill does,
    // as nothing else needs timing (D48 Q10), and is still no difficulty.
    CHECK(row.hardest() == std::optional<HardestTiming>(HardestTiming{P::EarlyFill, 0.0}));
    CHECK_FALSE(row.difficulty().has_value());

    // A required fill tied with a squeeze: both listed, fill first, both
    // required and difficult; hardest() still names the squeeze.
    Activation tie;
    test::set_skips(tie, 0);
    tie.e_offset = -12.6;
    tie.sqinouts = {SPSqueeze{SqueezeKind::SqOut, -12.6}};
    const std::vector<ActivationTiming> tt = tie.badge_timings();
    REQUIRE(tt.size() == 2);
    CHECK(tt[0].part == P::EarlyFill);
    CHECK(tt[1].part == P::SqueezeOut);
    CHECK((tt[0].required && tt[1].required && tt[0].difficult && tt[1].difficult));
    CHECK(tie.hardest() == std::optional<HardestTiming>(HardestTiming{P::SqueezeOut, 12.6}));

    // A required fill with time to spare is listed, not difficult, and never
    // named hardest (D261, D51 call 4).
    Activation slack;
    test::set_skips(slack, 0);
    slack.e_offset = 10.0;
    const std::vector<ActivationTiming> st = slack.badge_timings();
    REQUIRE(st.size() == 1);
    CHECK(st[0].part == P::EarlyFill);
    CHECK(st[0].ms == -10.0);
    CHECK_FALSE(st[0].difficult);
    CHECK_FALSE(slack.hardest().has_value());
    CHECK_FALSE(slack.difficulty().has_value());
    CHECK(slack.within_ms_limit(0.0));

    // An optional fill with time to spare beside a free squeeze: both listed,
    // and nothing is hardest, as nothing needs timing.
    Activation spare;
    test::set_skips(spare, 2);
    spare.e_offset = 56.2;
    spare.sqinouts = {SPSqueeze{SqueezeKind::SqIn, -56.0}};
    const std::vector<ActivationTiming> sp = spare.badge_timings();
    REQUIRE(sp.size() == 2);
    CHECK(sp[0].part == P::EarlyFill);
    CHECK(sp[0].ms == -56.2);
    CHECK_FALSE(sp[0].difficult);
    CHECK(sp[1].part == P::SqueezeIn);
    CHECK_FALSE(spare.hardest().has_value());
    CHECK_FALSE(spare.difficulty().has_value());
}

// D85: the all-0 list's 0 ms limit lets a timing of exactly 0 ms through,
// though such a timing needs hitting (Activation::needs_timing). A free
// squeeze counts too, so a negative limit can still refuse one.
TEST_CASE("Activation::within_ms_limit: 0 ms timings pass a 0 ms limit") {
    auto plain = [] {
        Activation a;
        test::set_skips(a, 0);
        a.e_offset = 300.0;  // not e-critical
        return a;
    };

    Activation on_end_out = plain();
    on_end_out.sqinouts = {SPSqueeze{SqueezeKind::SqOut, 0.0}};
    REQUIRE(on_end_out.needs_timing());
    CHECK(on_end_out.within_ms_limit(0.0));

    Activation tight_fill = plain();
    tight_fill.e_offset = 0.0;
    REQUIRE(tight_fill.is_E0());
    REQUIRE(tight_fill.needs_timing());
    CHECK(tight_fill.within_ms_limit(0.0));

    Activation late_out = plain();
    late_out.sqinouts = {SPSqueeze{SqueezeKind::SqOut, -0.5}};
    CHECK_FALSE(late_out.within_ms_limit(0.0));
    CHECK(late_out.within_ms_limit(0.5));

    Activation late_fill = plain();
    late_fill.e_offset = -0.5;
    REQUIRE(late_fill.is_E0());
    CHECK_FALSE(late_fill.within_ms_limit(0.0));

    // An optional fill (not E0) is never a limit's business.
    Activation optional_fill = plain();
    test::set_skips(optional_fill, 1);
    test::set_sp_meter(optional_fill, 2);
    optional_fill.e_offset = -30.0;
    REQUIRE_FALSE(optional_fill.is_E0());
    CHECK(optional_fill.within_ms_limit(0.0));

    // A free squeeze-in 163 ms inside SP is -163 ms: inside 0, outside -200.
    Activation free_in = plain();
    free_in.sqinouts = {SPSqueeze{SqueezeKind::SqIn, -163.0}};
    REQUIRE_FALSE(free_in.needs_timing());
    CHECK(free_in.within_ms_limit(0.0));
    CHECK_FALSE(free_in.within_ms_limit(-200.0));

    // A path is inside when every activation is; no activations is inside.
    Path p;
    CHECK(p.within_ms_limit(0.0));
    p.activations = {on_end_out, tight_fill, free_in};
    CHECK(p.within_ms_limit(0.0));
    p.activations.push_back(late_out);
    CHECK_FALSE(p.within_ms_limit(0.0));
}

TEST_CASE("Activation: each end's anchor and each squeeze's end, from the steps") {
    using K = SpEndKind;
    Activation a;
    a.timecode = Timecode::raw(2304);

    // Clamp at C1, SqIn, clamp at C2.
    a.sp_end_steps = {{2304, 5376, K::Activation},
                      {3072, 6144, K::Clamped},     // C1 pins X
                      {6100, 7680, K::SqIn},        // early SqIn: X = 6144
                      {6912, 9984, K::Clamped}};    // C2 pins D
    a.sqinouts = {SPSqueeze{SqueezeKind::SqIn, -50.0}};
    CHECK(a.end_anchor_tick(0) == 2304);
    CHECK(a.end_anchor_tick(1) == 3072);
    CHECK(a.end_anchor_tick(3) == 6912);
    CHECK(a.squeeze_end_tick(0) == std::optional<int64_t>(6144));
    CHECK(a.squeeze_anchor_tick(0) == std::optional<int64_t>(3072));  // C1
    CHECK(a.deact_anchor_tick() == std::optional<int64_t>(6912));     // C2

    // SqIn, then a clamp. The SqIn's end was the activation's.
    a.sp_end_steps = {{2304, 5376, K::Activation},
                      {5400, 6912, K::SqIn},        // late SqIn: X = 5376
                      {6144, 12288, K::Clamped}};
    CHECK(a.squeeze_end_tick(0) == std::optional<int64_t>(5376));
    CHECK(a.squeeze_anchor_tick(0) == std::optional<int64_t>(2304));
    CHECK(a.deact_anchor_tick() == std::optional<int64_t>(6144));

    // Clamp at C, then a SqIn. X was pinned to C, so both anchors are C.
    a.sp_end_steps = {{2304, 5376, K::Activation},
                      {3072, 6144, K::Clamped},
                      {6100, 7680, K::SqIn}};
    CHECK(a.squeeze_anchor_tick(0) == std::optional<int64_t>(3072));
    CHECK(a.deact_anchor_tick() == std::optional<int64_t>(3072));

    // A SqOut is measured from D, with D's anchor.
    a.sp_end_steps = {{2304, 5376, K::Activation},
                      {5400, 6912, K::SqIn},
                      {6144, 12288, K::Clamped}};
    a.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -20.0});
    CHECK(a.squeeze_end_tick(1) == a.deact_tick());
    CHECK(a.squeeze_anchor_tick(1) == a.deact_anchor_tick());

    // Two SqIns map to the two SqIn steps in order.
    a.sp_end_steps = {{2304, 5376, K::Activation},
                      {5400, 6912, K::SqIn},
                      {6950, 8448, K::SqIn}};
    a.sqinouts = {SPSqueeze{SqueezeKind::SqIn, 24.0}, SPSqueeze{SqueezeKind::SqIn, 38.0}};
    CHECK(a.squeeze_end_tick(0) == std::optional<int64_t>(5376));
    CHECK(a.squeeze_end_tick(1) == std::optional<int64_t>(6912));

    // An old record (no steps) answers nothing.
    Activation old;
    old.sqinouts = {SPSqueeze{SqueezeKind::SqIn, 5.0}};
    CHECK_FALSE(old.squeeze_end_tick(0).has_value());
    CHECK_FALSE(old.squeeze_anchor_tick(0).has_value());
    CHECK_FALSE(old.deact_anchor_tick().has_value());
}

TEST_CASE("Activation notationstr: E prefix, skips, symbols") {
    Activation a;
    test::set_skips(a, 2);
    a.e_offset = 300.0;  // not e-critical (at or past the window)
    CHECK(a.notationstr() == "2");

    a.e_offset = a.e_window_ms;  // boundary: not e-critical
    CHECK(a.notationstr() == "2");

    a.e_offset = 50.0;  // e-critical
    CHECK(a.notationstr() == "E2");

    a.e_offset = a.e_window_ms - 0.1;  // boundary: e-critical
    CHECK(a.notationstr() == "E2");

    // The E prefix follows the activation's own window, not a constant.
    a.e_window_ms = kHitWindowMs;
    a.e_offset = kHitWindowMs - 0.1;
    CHECK(a.notationstr() == "E2");
    a.e_window_ms = kDefaultEarlyFillWindowMs;
    CHECK(a.notationstr() == "2");
    a.e_offset = 50.0;

    a.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -1.0});
    a.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, 1.0});
    CHECK(a.notationstr() == "E2+-");
}

TEST_CASE("Path pathstring and pathstring_verbose") {
    Path p;
    Activation a;
    test::set_skips(a, 1);
    a.e_offset = 400.0;  // not e-critical, no sqinouts -> verbose == notationstr
    p.activations.push_back(a);
    p.notes_points = 100000;  // totalscore == 100000

    CHECK(p.pathstring() == "1");
    CHECK(p.pathstring_verbose({}) ==
          "(No mult squeezes.) | 1 | Score: 100,000");
    Chord c;
    c.add_note(NoteColor::Red);
    c.add_note(NoteColor::Yellow);
    c.apply_cymbal(NoteColor::Yellow);
    CHECK(p.pathstring_verbose({MultSqueeze(c, 8)}) == "2x | 1 | Score: 100,000");

    Path empty;
    CHECK(empty.pathstring() == "(No activations.)");
    CHECK(empty.pathstring_verbose({}) ==
          "(No mult squeezes.) | (No activations.) | Score: 0");
}

// walk_activations reads a path's activations in place: its own, then the
// tail it shares with its parent, in that order, and copies nothing.
TEST_CASE("Path::walk_activations: own activations then the variant tail, in place") {
    Path p;
    Activation a1, a2, t1;
    test::set_skips(a1, 0);
    test::set_skips(a2, 1);
    test::set_skips(t1, 2);
    p.activations = {a1, a2};
    p.variant_tail = {t1};

    const ActivationWalk walk = p.walk_activations();
    REQUIRE(walk.size() == 3);
    CHECK_FALSE(walk.empty());
    // In place: each element is the very object in the path, not a copy.
    CHECK(&walk[0] == &p.activations[0]);
    CHECK(&walk[1] == &p.activations[1]);
    CHECK(&walk[2] == &p.variant_tail[0]);
    CHECK(&walk.front() == &p.activations[0]);
    CHECK(&walk.back() == &p.variant_tail[0]);

    // A range-for visits the same objects in the same order.
    std::vector<const Activation*> seen;
    for (const Activation& a : walk) seen.push_back(&a);
    CHECK(seen == std::vector<const Activation*>{&p.activations[0], &p.activations[1],
                                                 &p.variant_tail[0]});

    // The same sequence the copying all_activations() hands out.
    const std::vector<Activation> copied = p.all_activations();
    REQUIRE(copied.size() == walk.size());
    for (size_t i = 0; i < copied.size(); ++i) CHECK(copied[i].skips() == walk[i].skips());
    CHECK(p.has_activations());

    // Nothing on either side.
    Path none;
    const ActivationWalk nothing = none.walk_activations();
    CHECK(nothing.empty());
    CHECK(nothing.begin() == nothing.end());
    CHECK_FALSE(none.has_activations());
    CHECK(none.all_activations().empty());
}

// Backend rows past a squeezed-out note are impossible in game: the sqout note
// is hit after SP ends, so every note after it is hit outside SP too. The
// engine's copy-out trims them (Activation::set_sqout); this pins the
// display-layer guard that keeps any list still holding one from showing it.
TEST_CASE("display_backends drops rows beyond a squeeze out") {
    Activation a;
    const std::vector<double> offsets = {-368.1, -184.0, 0.0, 184.0, 368.1};
    int64_t tick = 100;  // chart order, one row per 100 ticks
    for (double off : offsets) {
        BackendSqueeze bsq;
        bsq.timecode = Timecode::raw(tick);
        tick += 100;
        bsq.sp_points = 50;
        bsq.offset_ms = off;
        a.backends.push_back(bsq);
    }

    // No squeeze out: every row is inside the +/-500 ms display window.
    REQUIRE(a.display_backends().size() == offsets.size());
    for (size_t i = 0; i < offsets.size(); ++i)
        CHECK(a.display_backends()[i].offset_ms.value() == offsets[i]);

    // Squeezing out at -184.0 keeps that row and the one before it, and drops
    // the three that land after it.
    a.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -184.0});
    a.sqout_tick = 200;  // the -184.0 row
    std::vector<BackendSqueeze> shown = a.display_backends();
    REQUIRE(shown.size() == 2);
    CHECK(shown[0].offset_ms.value() == -368.1);
    CHECK(shown[1].offset_ms.value() == -184.0);
}

TEST_CASE("Chord rowstr / notationstr / disco flip") {
    Chord c;
    c.add_note(NoteColor::Red);
    c.add_note(NoteColor::Green);
    CHECK(c.notationstr() == "[ R  G]");
    // The Dynamics tab's words, with Pro Drums on (D48 Q11): a normal green
    // is a tom and red is the snare.
    CHECK(c.rowstr() == "[Red snare - Green tom]");

    // Disco flip swaps red<->yellow (red becomes a yellow cymbal).
    Chord d;
    d.add_note(NoteColor::Red);
    d.apply_disco_flip();
    REQUIRE(d.at(NoteColor::Yellow).has_value());
    CHECK(d.at(NoteColor::Yellow)->cymbaltype == NoteCymbalType::Cymbal);
    CHECK_FALSE(d.at(NoteColor::Red).has_value());
}

TEST_CASE("a ghost or accent kick scores double, like a pad") {
    CHECK(ChordNote{NoteColor::Kick}.basescore() == 50);
    CHECK(ChordNote{NoteColor::Kick, NoteDynamicType::Ghost}.basescore() == 100);
    CHECK(ChordNote{NoteColor::Kick, NoteDynamicType::Accent}.basescore() == 100);
    // A 2x kick is still a kick: the foot, not the value, is what changes.
    CHECK(ChordNote{NoteColor::Kick, NoteDynamicType::Ghost,
                    NoteCymbalType::Normal, true}
              .basescore() == 100);
}

TEST_CASE("ChordNote::str shows the kick's dynamic and its 2x flag") {
    auto kick = [](NoteDynamicType dyn, bool is2x) {
        return ChordNote{NoteColor::Kick, dyn, NoteCymbalType::Normal, is2x}.str();
    };
    CHECK(kick(NoteDynamicType::Normal, false) == "Kick");
    CHECK(kick(NoteDynamicType::Ghost, false) == "Kick (Ghost)");
    CHECK(kick(NoteDynamicType::Accent, false) == "Kick (Accent)");
    CHECK(kick(NoteDynamicType::Normal, true) == "2x kick");
    CHECK(kick(NoteDynamicType::Ghost, true) == "2x kick (Ghost)");
    CHECK(kick(NoteDynamicType::Accent, true) == "2x kick (Accent)");

    // Pads take the Dynamics words too; a ghost or accent keeps its
    // parenthesis.
    CHECK(ChordNote{NoteColor::Red}.str() == "Red snare");
    CHECK(ChordNote{NoteColor::Red, NoteDynamicType::Ghost}.str() ==
          "Red snare (Ghost)");
    CHECK(ChordNote{NoteColor::Yellow, NoteDynamicType::Accent,
                    NoteCymbalType::Cymbal, false}
              .str() == "Yellow cymbal (Accent)");
}

// D48 Q11: a note is called what the Dynamics tab calls it. With Pro Drums
// off a pad has no tom or snare, so the type word goes.
TEST_CASE("note_label: the Dynamics words, plain colours with Pro Drums off") {
    const ChordNote green_tom{NoteColor::Green};
    const ChordNote yellow_cym{NoteColor::Yellow, NoteDynamicType::Normal,
                               NoteCymbalType::Cymbal, false};
    const ChordNote kick2x{NoteColor::Kick, NoteDynamicType::Normal, NoteCymbalType::Normal,
                           true};
    const ChordNote yellow_tom{NoteColor::Yellow};
    const ChordNote red{NoteColor::Red};

    CHECK(note_label(green_tom, true) == "Green tom");
    CHECK(note_label(yellow_cym, true) == "Yellow cymbal");
    CHECK(note_label(kick2x, true) == "2x kick");
    CHECK(note_label(ChordNote{NoteColor::Kick}, true) == "Kick");
    CHECK(note_label(red, true) == "Red snare");

    CHECK(note_label(yellow_tom, false) == "Yellow");
    CHECK(note_label(red, false) == "Red");
    CHECK(note_label(kick2x, false) == "2x kick");

    // A ghost or accent is not part of the name.
    CHECK(note_label(ChordNote{NoteColor::Green, NoteDynamicType::Ghost}, true) == "Green tom");
}

TEST_CASE("Chord::code spells a ghost/accent kick in the kick lane") {
    Chord normal;
    normal.add_note(NoteColor::Kick);
    normal.add_note(NoteColor::Red);
    CHECK(normal.code() == "nn...");

    Chord ghost;
    ghost.add_note(NoteColor::Kick).dynamictype = NoteDynamicType::Ghost;
    ghost.add_note(NoteColor::Red);
    CHECK(ghost.code() == "gn...");

    Chord accent;
    accent.add_note(NoteColor::Kick).dynamictype = NoteDynamicType::Accent;
    accent.add_note(NoteColor::Red);
    CHECK(accent.code() == "an...");

    CHECK(ghost.rowstr() == "[Kick (Ghost) - Red snare]");
}

// The engine's three cases, as values. The deactivation edge in
// create_deactivated_path adds `value - already_paid`, where rows at or
// before the SP end were already paid in full by the SP walk.
TEST_CASE("backend_row_value: every engine case") {
    const double lw = core::default_rules().backend_leeway_ms;  // 3 ms
    using P = core::SqOutPosition;
    using core::backend_row_value;

    // No squeeze-out: full value inside SP or inside the leeway, else 0.
    CHECK(backend_row_value(-50.0, 460, 260, P::NoSqOut, lw) == 460);
    CHECK(backend_row_value(0.0, 460, 260, P::NoSqOut, lw) == 460);
    CHECK(backend_row_value(2.999, 460, 260, P::NoSqOut, lw) == 460);
    CHECK(backend_row_value(3.0, 460, 260, P::NoSqOut, lw) == 0);

    // Before the squeezed-out chord: same as no squeeze-out.
    CHECK(backend_row_value(-50.0, 460, 260, P::Before, lw) == 460);
    CHECK(backend_row_value(1.5, 460, 260, P::Before, lw) == 460);
    CHECK(backend_row_value(10.0, 460, 260, P::Before, lw) == 0);

    // The squeezed-out chord itself keeps only its reduced value, and only
    // where it would have been counted at all.
    CHECK(backend_row_value(-5.0, 460, 260, P::Exact, lw) == 260);
    CHECK(backend_row_value(1.5, 460, 260, P::Exact, lw) == 260);
    // Round and Round (Ratt), second activation: R+Y squeezed out
    // 479.999 ms past the SP end. The engine counts it as 0.
    CHECK(backend_row_value(479.999, 460, 260, P::Exact, lw) == 0);

    // After the squeezed-out chord nothing is under Star Power.
    CHECK(backend_row_value(-50.0, 460, 260, P::After, lw) == 0);
    CHECK(backend_row_value(1.5, 460, 260, P::After, lw) == 0);

    // The leeway is the user's rule (hydra_rules.ini), not a constant.
    CHECK(backend_row_value(5.0, 460, 260, P::NoSqOut, 10.0) == 460);
    CHECK(backend_row_value(5.0, 460, 260, P::NoSqOut, lw) == 0);

    // Position comes from ticks, the way the engine compares them.
    CHECK(core::sqout_position(100, std::nullopt) == P::NoSqOut);
    CHECK(core::sqout_position(99, 100) == P::Before);
    CHECK(core::sqout_position(100, 100) == P::Exact);
    CHECK(core::sqout_position(101, 100) == P::After);

    // Only rows at or before the SP end were paid by the SP walk.
    CHECK(core::paid_by_sp_walk(0.0));
    CHECK(core::paid_by_sp_walk(-0.5));
    CHECK_FALSE(core::paid_by_sp_walk(0.5));
}

// D29 (finding 304): the backend leeway's edge is strict. A note less than
// the leeway after the SP end still scores under Star Power; a note exactly
// the leeway after it does not. No such constant was found in the game's
// engine (SP-43 in docs/clone-hero/05-star-power.md says how far that search
// went). The setting is Hydra's (Rules::backend_leeway_ms) and its 3 ms value
// is DragonDelgar's (ADR 0023), so the edge is Hydra's call, and this is it.
TEST_CASE("backend leeway: +2.999 ms is counted, exactly +3.0 ms is not") {
    const double lw = core::default_rules().backend_leeway_ms;
    REQUIRE(lw == 3.0);
    CHECK(core::counted_without_squeeze(2.999, lw));
    CHECK_FALSE(core::counted_without_squeeze(3.0, lw));

    // The details table labels a plain (not squeezed-out) row by the same
    // rule.
    BackendSqueeze row;
    row.offset_ms = 2.999;
    CHECK(row.summarystr(false, std::nullopt, lw) == "Normal");
    row.offset_ms = 3.0;
    CHECK(row.summarystr(false, std::nullopt, lw) == "Hard (uncounted)");
}

// D302.
TEST_CASE("SPSqueeze::needs_no_timing: a squeeze at exactly 0 needs 0.0 ms, one inside needs none") {
    // A SqIn needs no timing only once its note is already inside SP, before
    // the end; on the end it needs 0.0 ms.
    CHECK_FALSE(SPSqueeze{SqueezeKind::SqIn, 0.0}.needs_no_timing());
    CHECK_FALSE(SPSqueeze{SqueezeKind::SqIn, -0.0}.needs_no_timing());
    CHECK_FALSE(SPSqueeze{SqueezeKind::SqIn, 0.001}.needs_no_timing());
    CHECK(SPSqueeze{SqueezeKind::SqIn, -0.001}.needs_no_timing());
    // A SqOut needs no timing only once its note is already past the end; on
    // the end it needs 0.0 ms.
    CHECK_FALSE(SPSqueeze{SqueezeKind::SqOut, 0.0}.needs_no_timing());
    CHECK_FALSE(SPSqueeze{SqueezeKind::SqOut, -0.001}.needs_no_timing());
    CHECK(SPSqueeze{SqueezeKind::SqOut, 0.001}.needs_no_timing());
}

// D302: a timing of 0.0 ms is still a timing to hit.
TEST_CASE("timing_needs_hitting: 0.0 ms and more need hitting, below 0 does not") {
    CHECK_FALSE(timing_needs_hitting(-0.001));
    CHECK(timing_needs_hitting(0.0));
    CHECK(timing_needs_hitting(0.001));
}

TEST_CASE("early_fill_needs_timing: zero slack needs timing, time to spare does not") {
    CHECK(early_fill_needs_timing(0.0));
    CHECK(early_fill_needs_timing(-0.001));
    CHECK_FALSE(early_fill_needs_timing(0.001));
}

TEST_CASE("shown_timing: a saved figure shows when it needs hitting") {
    CHECK_FALSE(shown_timing(std::nullopt).has_value());
    CHECK_FALSE(shown_timing(-0.001).has_value());
    CHECK_FALSE(shown_timing(-315.8).has_value());
    CHECK(shown_timing(0.0) == std::optional<double>(0.0));
    CHECK(shown_timing(163.0) == std::optional<double>(163.0));
}

TEST_CASE("engine_hardest_ms: the largest figure, sign included") {
    auto plain = [] {
        Activation a;
        test::set_skips(a, 0);
        a.e_offset = 300.0;  // not e-critical
        return a;
    };
    auto path_of = [](std::vector<Activation> acts) {
        Path p;
        p.activations = std::move(acts);
        return p;
    };

    Activation inside_in = plain();
    inside_in.sqinouts = {SPSqueeze{SqueezeKind::SqIn, -315.8}};
    Activation late_out = plain();
    late_out.sqinouts = {SPSqueeze{SqueezeKind::SqOut, -130.4}};  // needs 130.4 ms
    Activation on_end_in = plain();
    on_end_in.sqinouts = {SPSqueeze{SqueezeKind::SqIn, 0.0}};
    Activation slack_fill = plain();
    slack_fill.e_offset = 20.0;
    REQUIRE(slack_fill.is_E0());
    Activation optional_fill = plain();
    test::set_skips(optional_fill, 1);
    test::set_sp_meter(optional_fill, 2);
    optional_fill.e_offset = -30.0;
    REQUIRE(optional_fill.is_e_critical());
    REQUIRE_FALSE(optional_fill.is_E0());

    const Path inside_only = path_of({inside_in});
    const Path with_out = path_of({inside_in, late_out});
    const Path at_zero = path_of({on_end_in});
    const Path nothing = path_of({plain()});
    const Path slack_only = path_of({slack_fill});
    const Path optional_only = path_of({optional_fill});

    CHECK(engine_hardest_ms(inside_only) == std::optional<double>(-315.8));
    CHECK(engine_hardest_ms(with_out) == std::optional<double>(130.4));
    CHECK(engine_hardest_ms(at_zero) == std::optional<double>(0.0));
    CHECK_FALSE(engine_hardest_ms(nothing).has_value());
    CHECK(engine_hardest_ms(slack_only) == std::optional<double>(-20.0));
    CHECK_FALSE(engine_hardest_ms(optional_only).has_value());

    // Through shown_timing, the saved figure shows what Path::difficulty()
    // shows on the live path, so a fresh row reads as it did.
    for (const Path* p : {&inside_only, &with_out, &at_zero, &nothing, &slack_only, &optional_only})
        CHECK(shown_timing(engine_hardest_ms(*p)) == p->difficulty());
}

TEST_CASE("difficulty names: one list, one spelling") {
    // The dropdown indexes this list by the enum's value, so order matters.
    REQUIRE(std::size(kAllDifficulties) == 4);
    for (size_t i = 0; i < std::size(kAllDifficulties); ++i)
        CHECK(static_cast<size_t>(kAllDifficulties[i]) == i);
    CHECK(std::string(difficulty_name(kAllDifficulties[0])) == "Expert");
    CHECK(std::string(difficulty_name(kAllDifficulties[3])) == "Easy");
}

TEST_CASE("difficulty_from_name: any case, nothing else") {
    CHECK(difficulty_from_name("Hard") == Difficulty::Hard);
    CHECK(difficulty_from_name("medium") == Difficulty::Medium);
    CHECK(difficulty_from_name("EASY") == Difficulty::Easy);
    CHECK(difficulty_from_name("eXpErT") == Difficulty::Expert);
    CHECK_FALSE(difficulty_from_name("Legendary").has_value());
    CHECK_FALSE(difficulty_from_name("Har").has_value());
    CHECK_FALSE(difficulty_from_name("").has_value());
}

TEST_CASE("no_notes_message names the difficulty and the drum mode") {
    CHECK(no_notes_message(Difficulty::Hard, true) == "No Hard Pro Drums notes in this chart.");
    CHECK(no_notes_message(Difficulty::Expert, false) == "No Expert Drums notes in this chart.");
}

TEST_CASE("no_notes_part reads the missing part back out of no_notes_message's sentence") {
    CHECK(no_notes_part(no_notes_message(Difficulty::Expert, true)) ==
          std::optional<std::string>("Expert Pro Drums notes"));
    CHECK(no_notes_part("No Hard Drums notes in this chart.") ==
          std::optional<std::string>("Hard Drums notes"));
    CHECK_FALSE(no_notes_part("Hydra couldn't read this chart file.").has_value());
    CHECK_FALSE(no_notes_part("No  in this chart.").has_value());  // nothing between the ends
    CHECK_FALSE(no_notes_part("").has_value());
}

TEST_CASE("title_or_unknown: one fallback for a song with no usable name") {
    CHECK(std::string(kUnknownTitle) == "(unknown)");
    CHECK(title_or_unknown("") == "(unknown)");
    // An older scan's stored name, which the rescan cache can still hold.
    CHECK(title_or_unknown("<unknown title>") == "(unknown)");
    CHECK(title_or_unknown("Some Song") == "Some Song");
}

// A multiplier squeeze is a chord whose notes straddle a to_multiplier step.
// MultSqueeze accepts exactly the straddling chords of every size, 2 to 5
// notes (D51 call 3: Clone Hero allows 4- and 5-note chords, so Hydra must
// get them right). The combos are pinned as literals: the steps sit at
// combos 10, 20 and 30, so a chord of n notes hit at one of these combos
// pays its first note below a step and its last note at or above it.
TEST_CASE("MultSqueeze accepts exactly the 2- to 5-note chords that straddle a multiplier step") {
    const NoteColor order[] = {NoteColor::Red, NoteColor::Yellow, NoteColor::Kick,
                               NoteColor::Blue, NoteColor::Green};
    auto chord_of = [&](int n, bool cymbal) {
        Chord c;
        for (int i = 0; i < n; ++i) c.add_note(order[i]);
        // A cymbal among pads gives the chord something to squeeze. Without
        // it every note is worth the same.
        if (cymbal) c.apply_cymbal(NoteColor::Yellow);
        return c;
    };
    auto accepted = [](const Chord& c, int combo) {
        try {
            MultSqueeze ms(c, combo);
            return true;
        } catch (const std::invalid_argument&) {
            return false;
        }
    };
    const std::map<int, std::set<int>> straddling = {
        {2, {8, 18, 28}},
        {3, {7, 8, 17, 18, 27, 28}},
        {4, {6, 7, 8, 16, 17, 18, 26, 27, 28}},
        {5, {5, 6, 7, 8, 15, 16, 17, 18, 25, 26, 27, 28}},
    };
    for (const auto& [n, combos] : straddling)
        for (int combo = 0; combo < 40; ++combo) {
            CHECK_MESSAGE(accepted(chord_of(n, true), combo) == (combos.count(combo) == 1),
                          "n=" << n << " combo=" << combo);
            CHECK_MESSAGE(!accepted(chord_of(n, false), combo),
                          "all-same-value n=" << n << " combo=" << combo);
        }
}

// Kick and Red (50 each) with yellow and blue cymbals (65 each), hit at
// combo 7: two notes are paid at 1x and two at 2x. Two notes cross the step,
// so the right order gains two cymbal bonuses.
TEST_CASE("MultSqueeze::gain is the best order minus the worst order") {
    Chord kryb;
    kryb.add_note(NoteColor::Kick);
    kryb.add_note(NoteColor::Red);
    kryb.add_note(NoteColor::Yellow);
    kryb.add_note(NoteColor::Blue);
    kryb.apply_cymbal(NoteColor::Yellow);
    kryb.apply_cymbal(NoteColor::Blue);
    CHECK(MultSqueeze(kryb, 7).gain() == 30);

    Chord ry;
    ry.add_note(NoteColor::Red);
    ry.add_note(NoteColor::Yellow);
    ry.apply_cymbal(NoteColor::Yellow);
    CHECK(MultSqueeze(ry, 8).gain() == 15);
}

// D51 call 3: the advice names every note that has to cross the step.
TEST_CASE("MultSqueeze::howto names every note that crosses the step") {
    // Kick and Red (50 each), yellow and blue cymbals (65 each) at combo 7:
    // both cymbals cross to 2x.
    Chord kryb;
    kryb.add_note(NoteColor::Kick);
    kryb.add_note(NoteColor::Red);
    kryb.add_note(NoteColor::Yellow);
    kryb.add_note(NoteColor::Blue);
    kryb.apply_cymbal(NoteColor::Yellow);
    kryb.apply_cymbal(NoteColor::Blue);
    const MultSqueeze ms(kryb, 7);
    CHECK(ms.howto() == "Hit [Yellow cymbal] and [Blue cymbal] last.");
    CHECK(ms.multiplier() == 2);

    // Accent the Red (100): now the two cymbals tie across the step, so
    // either may cross. The kick must stay before the step and the accented
    // Red must cross it.
    kryb.apply_accent(NoteColor::Red);
    CHECK(MultSqueeze(kryb, 7).howto() == "Hit [Kick] first and [Red snare (Accent)] last.");
}

// A 3-note chord splits two and one across the step. When one end holds a
// single note, the advice names that note whichever side of the step the
// chord falls on; only three different values need direction-specific advice.
TEST_CASE("MultSqueeze::howto names the lone note of a 3-note chord") {
    // Kick (50) + two cymbals (65): the kick is the lone cheap note.
    Chord kyb;
    kyb.add_note(NoteColor::Kick);
    kyb.add_note(NoteColor::Yellow);
    kyb.add_note(NoteColor::Blue);
    kyb.apply_cymbal(NoteColor::Yellow);
    kyb.apply_cymbal(NoteColor::Blue);
    for (int combo : {17, 18, 27, 28})
        CHECK_MESSAGE(MultSqueeze(kyb, combo).howto() == "Hit [Kick] first.",
                      "combo=" << combo);

    // Kick + Red (50) + one cymbal (65): the cymbal is the lone dear note.
    Chord kry;
    kry.add_note(NoteColor::Kick);
    kry.add_note(NoteColor::Red);
    kry.add_note(NoteColor::Yellow);
    kry.apply_cymbal(NoteColor::Yellow);
    for (int combo : {17, 18, 27, 28})
        CHECK_MESSAGE(MultSqueeze(kry, combo).howto() == "Hit [Yellow cymbal] last.",
                      "combo=" << combo);

    // Kick (50) + cymbal (65) + accented Red (100): three values, so the
    // direction decides. Combo 17 carries one note over; 18 leaves one behind.
    Chord kya;
    kya.add_note(NoteColor::Kick);
    kya.add_note(NoteColor::Red);
    kya.add_note(NoteColor::Yellow);
    kya.apply_cymbal(NoteColor::Yellow);
    kya.apply_accent(NoteColor::Red);
    CHECK(MultSqueeze(kya, 17).howto() == "Hit [Red snare (Accent)] last.");
    CHECK(MultSqueeze(kya, 18).howto() == "Hit [Kick] first.");
}

// The advice names its note in the Pro Drums setting's words, as the chord
// rows do (D48 Q11, finding 17): with Pro Drums off a pad has no tom or snare.
TEST_CASE("MultSqueeze::howto names the note in the Pro Drums setting's words") {
    // Kick (50) + accented yellow tom (100) + blue cymbal (65): combo 17
    // carries the dear yellow over.
    Chord kyb;
    kyb.add_note(NoteColor::Kick);
    kyb.add_note(NoteColor::Yellow);
    kyb.add_note(NoteColor::Blue);
    kyb.apply_cymbal(NoteColor::Blue);
    kyb.apply_accent(NoteColor::Yellow);
    CHECK(MultSqueeze(kyb, 17).howto(true) == "Hit [Yellow tom (Accent)] last.");
    CHECK(MultSqueeze(kyb, 17).howto(false) == "Hit [Yellow (Accent)] last.");

    // Kick (50) + accented red (100) + yellow cymbal (65).
    Chord kya;
    kya.add_note(NoteColor::Kick);
    kya.add_note(NoteColor::Red);
    kya.add_note(NoteColor::Yellow);
    kya.apply_cymbal(NoteColor::Yellow);
    kya.apply_accent(NoteColor::Red);
    CHECK(MultSqueeze(kya, 17).howto(false) == "Hit [Red (Accent)] last.");
    // A cymbal says cymbal either way.
    Chord kry;
    kry.add_note(NoteColor::Kick);
    kry.add_note(NoteColor::Red);
    kry.add_note(NoteColor::Yellow);
    kry.apply_cymbal(NoteColor::Yellow);
    CHECK(MultSqueeze(kry, 17).howto(false) == "Hit [Yellow cymbal] last.");
}

// The graph asks applies() of every chord instead of catching a throw.
// It must answer exactly as the constructor decides, for every shape.
TEST_CASE("MultSqueeze::applies answers exactly when the constructor accepts") {
    const NoteColor order[] = {NoteColor::Red, NoteColor::Yellow, NoteColor::Kick,
                               NoteColor::Blue, NoteColor::Green};
    for (int n = 0; n <= 5; ++n) {
        for (bool cymbal : {false, true}) {
            Chord c;
            for (int i = 0; i < n; ++i) c.add_note(order[i]);
            if (cymbal && n >= 2) c.apply_cymbal(NoteColor::Yellow);
            for (int combo = 0; combo < 40; ++combo) {
                bool constructed = true;
                try {
                    MultSqueeze ms(c, combo);
                } catch (const std::invalid_argument&) {
                    constructed = false;
                }
                CHECK_MESSAGE(MultSqueeze::applies(c, combo) == constructed,
                              "n=" << n << " cymbal=" << cymbal << " combo=" << combo);
            }
        }
    }
}

TEST_CASE("Activation: set_sqout stamps the tick, trims later rows, builds the SqOut from its row") {
    Activation act;
    const std::tuple<int64_t, double> rows[] = {{100, -40.0}, {200, -12.5}, {300, 30.0}};
    for (const auto& [tick, off] : rows) {
        BackendSqueeze b;
        b.timecode = Timecode::raw(tick);
        b.offset_ms = off;
        act.backends.push_back(b);
    }
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 7.0});

    act.set_sqout(200);
    CHECK(act.sqout_tick == std::optional<int64_t>(200));
    REQUIRE(act.backends.size() == 2);  // the row past the squeezed-out chord is gone
    REQUIRE(act.sqinouts.size() == 2);
    CHECK(act.sqinouts[1].kind == SqueezeKind::SqOut);
    CHECK(act.sqinouts[1].offset_ms == -12.5);  // read off the row, not typed twice
    REQUIRE(act.sqout_row() != nullptr);
    CHECK(act.sqout_row()->timecode.ticks() == 200);

    Activation none;
    CHECK(none.sqout_row() == nullptr);
    CHECK_THROWS_AS(none.set_sqout(200), std::logic_error);  // no row on that tick
}

TEST_CASE("Bank arrivals and window_end_tick: a squeezed-out chord ends the window it is not after") {
    // An activation whose only stored step is the activation itself.
    auto one_step = [](int64_t act_tick, int64_t end_tick) {
        Activation a;
        a.timecode = Timecode::raw(act_tick);
        a.sp_end_steps = {{act_tick, end_tick, SpEndKind::Activation}};
        return a;
    };

    // An early squeeze-out: the chord, 64800, is before the stored end, so
    // Star Power is over on the chord. The stored end is unchanged.
    Activation early = one_step(57600, 65280);
    early.sqout_tick = 64800;
    CHECK(early.window_end_tick() == std::optional<int64_t>(64800));
    CHECK(early.deact_tick() == std::optional<int64_t>(65280));

    // A late squeeze-out: the chord, 71200, is after the stored end, so the
    // stored end stands.
    Activation late = one_step(59520, 71040);
    late.sqout_tick = 71200;
    CHECK(late.window_end_tick() == std::optional<int64_t>(71040));

    // The chord exactly on the stored end is not after it: the window ends
    // there, which is both ticks.
    Activation on_end = one_step(59520, 71040);
    on_end.sqout_tick = 71040;
    CHECK(on_end.window_end_tick() == std::optional<int64_t>(71040));

    // No squeeze-out: the stored end.
    Activation plain = one_step(5760, 13440);
    CHECK(plain.window_end_tick() == std::optional<int64_t>(13440));

    // A hand-built activation with no steps has no end at all.
    Activation bare;
    bare.sqout_tick = 64800;
    CHECK_FALSE(bare.window_end_tick().has_value());

    // bank_arrivals: the bars activation 1 spends were stored at 65280 (the
    // squeezed-out bar, stamped by the search), 70000 and 74000. Activation 0
    // squeezed out on 64800, so the first bar arrives there; the rest stand.
    Path path;
    Activation first = one_step(57600, 65280);
    first.sqout_tick = 64800;
    first.bank_rise_ticks = {20000, 30000, 40000, 50000};
    Activation second = one_step(80000, 95360);
    second.bank_rise_ticks = {65280, 70000, 74000};
    path.activations = {first, second};
    path.trailing_bank_ticks = {100000};
    // The first activation has no activation before it: its list as stored.
    CHECK(path.bank_arrivals(0) == std::vector<int64_t>{20000, 30000, 40000, 50000});
    CHECK(path.bank_arrivals(1) == std::vector<int64_t>{64800, 70000, 74000});
    // The trailing bars follow activation 1, which did not squeeze out.
    CHECK(path.bank_arrivals(2) == std::vector<int64_t>{100000});

    // The trailing bars after a squeezed-out last activation: the first moves
    // to the chord. An empty list stays empty.
    path.activations[1].sqout_tick = 95000;
    CHECK(path.bank_arrivals(2) == std::vector<int64_t>{95000});
    path.trailing_bank_ticks.clear();
    CHECK(path.bank_arrivals(2).empty());
}

// The rows of "backend_row_value: every engine case": what the squeezed-out
// chord gives up against its full value.
TEST_CASE("backend_value: sqout_cost is what the squeezed-out chord loses") {
    const double lw = core::default_rules().backend_leeway_ms;  // 3 ms
    CHECK(core::sqout_cost(-5.0, 460, 260, lw) == 200);
    CHECK(core::sqout_cost(1.5, 460, 260, lw) == 200);
    // Round and Round (Ratt): the row the engine counts as 0 loses all of it.
    CHECK(core::sqout_cost(479.999, 460, 260, lw) == 460);
    CHECK(core::sqout_cost(5.0, 460, 260, 10.0) == 200);
}

TEST_CASE("BackendSqueeze::offset: the stored offset, or a loud failure") {
    BackendSqueeze row;
    row.timecode = Timecode::raw(300);
    row.offset_ms = 12.5;
    CHECK(row.offset() == 12.5);

    row.offset_ms.reset();
    bool threw = false;
    try {
        (void)row.offset();
    } catch (const std::logic_error& e) {
        threw = true;
        CHECK(std::string(e.what()).find("300") != std::string::npos);
    }
    CHECK(threw);
}

// Finding 190's table: the kick's flag is 2x, a cymbal pad's is a cymbal,
// red has none.
TEST_CASE("lane flags: the kick carries 2x, the cymbal pads carry a cymbal, red carries nothing") {
    CHECK(lane_allows_flag(NoteColor::Kick));
    CHECK(lane_allows_flag(NoteColor::Yellow));
    CHECK(lane_allows_flag(NoteColor::Blue));
    CHECK(lane_allows_flag(NoteColor::Green));
    CHECK_FALSE(lane_allows_flag(NoteColor::Red));

    ChordNote kick{NoteColor::Kick};
    CHECK_FALSE(lane_flag(kick));
    set_lane_flag(kick);
    CHECK(kick.is2x);
    CHECK(kick.cymbaltype == NoteCymbalType::Normal);
    CHECK(lane_flag(kick));

    ChordNote yellow{NoteColor::Yellow};
    CHECK_FALSE(lane_flag(yellow));
    set_lane_flag(yellow);
    CHECK(yellow.cymbaltype == NoteCymbalType::Cymbal);
    CHECK_FALSE(yellow.is2x);
    CHECK(lane_flag(yellow));

    ChordNote red{NoteColor::Red};
    CHECK_THROWS_AS(set_lane_flag(red), std::logic_error);
}

TEST_CASE("dynamic_label: the display word for each dynamic") {
    CHECK(dynamic_label(NoteDynamicType::Ghost) == "Ghost");
    CHECK(dynamic_label(NoteDynamicType::Accent) == "Accent");
    CHECK(dynamic_label(NoteDynamicType::Normal) == "");
    CHECK(dynamic_str(NoteDynamicType::Ghost) == "ghost");
    CHECK(dynamic_str(NoteDynamicType::Accent) == "accent");
    CHECK(dynamic_str(NoteDynamicType::Normal) == "none");
}

TEST_CASE("squeeze_kind_from_name: type_name's two words and nothing else") {
    CHECK(squeeze_kind_from_name("SqIn") == std::optional<SqueezeKind>(SqueezeKind::SqIn));
    CHECK(squeeze_kind_from_name("SqOut") == std::optional<SqueezeKind>(SqueezeKind::SqOut));
    CHECK_FALSE(squeeze_kind_from_name("sqout").has_value());
    CHECK_FALSE(squeeze_kind_from_name("").has_value());
    for (SqueezeKind kind : {SqueezeKind::SqIn, SqueezeKind::SqOut})
        CHECK(squeeze_kind_from_name(SPSqueeze{kind, 0.0}.type_name()) ==
              std::optional<SqueezeKind>(kind));
}

// The same six values "replay score fields: one list in schema order"
// (tests/test_replay.cpp) assigns.
TEST_CASE("score_total: the six categories in the stored order") {
    CHECK(score_total(1, 2, 3, 4, 5, 6) == 21);

    Path p;
    p.notes_points = 100;
    p.score_combo = 200;
    p.score_sp = 300;
    p.score_solo = 400;
    p.score_accents = 50;
    p.score_ghosts = 60;
    CHECK(p.totalscore() == 1110);
}
