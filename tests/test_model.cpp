// Unit tests for core/model (chord encoding, squeezes). The chord-code
// round-trip lives in test_chord_code.cpp and the transfer scales in
// test_squeeze_rating.cpp; the cases here pin the string/number forms
// directly.

#include "doctest.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
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
    for (int combo : {0, 8, 9, 29, 45}) {
        CAPTURE(combo);
        // first_note (the default): only note 0 loses its SP doubling.
        std::vector<CategoryScores> per_note;
        const CategoryScores cs = category_scores(c, combo, &per_note);
        const int first_cut = notes[0].basescore() * to_multiplier(combo + 1);
        CHECK(cs.sqout_reduction == first_cut);
        REQUIRE(per_note.size() == 2);
        CHECK(per_note[0].sqout_reduction == first_cut);
        CHECK(per_note[1].sqout_reduction == 0);
        CHECK(cs.sqout_sp() == cs.sp - cs.sqout_reduction);

        // whole_chord: every note loses it, each at its own multiplier.
        std::vector<CategoryScores> whole_per_note;
        const CategoryScores whole =
            category_scores(c, combo, &whole_per_note, core::SqOutRule::WholeChord);
        int whole_cut = 0;
        for (size_t i = 0; i < notes.size(); ++i) {
            const int note_cut =
                notes[i].basescore() * to_multiplier(combo + 1 + static_cast<int>(i));
            REQUIRE(whole_per_note.size() == 2);
            CHECK(whole_per_note[i].sqout_reduction == note_cut);
            whole_cut += note_cut;
        }
        CHECK(whole.sqout_reduction == whole_cut);
        CHECK(whole.sqout_sp() == whole.sp - whole.sqout_reduction);
    }
}

TEST_CASE("group_thousands matches Python {:,}") {
    CHECK(group_thousands(0) == "0");
    CHECK(group_thousands(999) == "999");
    CHECK(group_thousands(1000) == "1,000");
    CHECK(group_thousands(228710) == "228,710");
    CHECK(group_thousands(1234567) == "1,234,567");
    CHECK(group_thousands(-1234567) == "-1,234,567");
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

    CHECK(is_e0(kEarlyFillWindowMs - 0.1, 0));
    CHECK_FALSE(is_e0(kEarlyFillWindowMs, 0));
    CHECK_FALSE(is_e0(10.0, 1));
    CHECK(early_fill_difficulty(-4.0) == 4.0);
    CHECK_FALSE(std::signbit(early_fill_difficulty(0.0)));

    Activation a;
    a.e_offset = 10.0;
    test::set_skips(a, 0);
    CHECK(a.is_E0() == is_e0(10.0, 0));
    REQUIRE(a.e_difficulty().has_value());
    CHECK(*a.e_difficulty() == early_fill_difficulty(10.0));
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
}

TEST_CASE("Activation: each end's anchor and each squeeze's end, from the steps") {
    using K = SpEndKind;
    Activation a;
    a.timecode = Timecode::raw(2304);

    // Case 3 of the SqIn-plus-clamp table: clamp at C1, SqIn, clamp at C2.
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

    // Case 2: SqIn, then a clamp. The SqIn's end was the activation's.
    a.sp_end_steps = {{2304, 5376, K::Activation},
                      {5400, 6912, K::SqIn},        // late SqIn: X = 5376
                      {6144, 12288, K::Clamped}};
    CHECK(a.squeeze_end_tick(0) == std::optional<int64_t>(5376));
    CHECK(a.squeeze_anchor_tick(0) == std::optional<int64_t>(2304));
    CHECK(a.deact_anchor_tick() == std::optional<int64_t>(6144));

    // Case 1: clamp at C, then a SqIn. X was pinned to C, so both anchors
    // are C.
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
    a.e_offset = 300.0;  // not e-critical (>= kEarlyFillWindowMs)
    CHECK(a.notationstr() == "2");

    a.e_offset = kEarlyFillWindowMs;  // boundary: not e-critical
    CHECK(a.notationstr() == "2");

    a.e_offset = 50.0;  // e-critical
    CHECK(a.notationstr() == "E2");

    a.e_offset = kEarlyFillWindowMs - 0.1;  // boundary: e-critical
    CHECK(a.notationstr() == "E2");

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
    p.score_base = 100000;  // totalscore == 100000

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

    // Nothing on either side.
    Path none;
    const ActivationWalk nothing = none.walk_activations();
    CHECK(nothing.empty());
    CHECK(nothing.begin() == nothing.end());
}

// Backend rows past a squeezed-out note are impossible in game: the sqout note
// is hit after SP ends, so every note after it is hit outside SP too. The
// engine trims them at record build; this pins the display-layer guard that
// keeps records stored before that fix from showing them.
TEST_CASE("display_backends drops rows beyond a squeeze out") {
    Activation a;
    const std::vector<double> offsets = {-368.1, -184.0, 0.0, 184.0, 368.1};
    int64_t tick = 100;  // chart order, one row per 100 ticks
    for (double off : offsets) {
        BackendSqueeze bsq;
        bsq.timecode = Timecode::raw(tick);
        tick += 100;
        bsq.points = 50;
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
    // Green is cymbal-capable, so a normal green renders as "GreenTom"; red is
    // not, so it is just "Red".
    CHECK(c.rowstr() == "[Red - GreenTom]");

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
    CHECK(kick(NoteDynamicType::Normal, true) == "Kick (2x)");
    CHECK(kick(NoteDynamicType::Ghost, true) == "Kick (Ghost, 2x)");
    CHECK(kick(NoteDynamicType::Accent, true) == "Kick (Accent, 2x)");

    // Pads read exactly as they always did.
    CHECK(ChordNote{NoteColor::Red}.str() == "Red");
    CHECK(ChordNote{NoteColor::Red, NoteDynamicType::Ghost}.str() ==
          "Red (Ghost)");
    CHECK(ChordNote{NoteColor::Yellow, NoteDynamicType::Accent,
                    NoteCymbalType::Cymbal, false}
              .str() == "YellowCym (Accent)");
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
    CHECK(Chord::from_code("gn...") == ghost);

    Chord accent;
    accent.add_note(NoteColor::Kick).dynamictype = NoteDynamicType::Accent;
    accent.add_note(NoteColor::Red);
    CHECK(accent.code() == "an...");
    CHECK(Chord::from_code("an...") == accent);

    CHECK(ghost.rowstr() == "[Kick (Ghost) - Red]");
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
// the leeway after it does not. No such constant was found in the engine
// methods read (no 0.003 in the 126 Clone Hero engine methods read); the
// 3 ms is Hydra's own setting (Rules::backend_leeway_ms), so the edge is
// Hydra's call, and this is it.
TEST_CASE("backend leeway: +2.999 ms is counted, exactly +3.0 ms is not (D29)") {
    const double lw = core::default_rules().backend_leeway_ms;
    REQUIRE(lw == 3.0);
    CHECK(core::counted_without_squeeze(2.999, lw));
    CHECK_FALSE(core::counted_without_squeeze(3.0, lw));

    // The details table labels a plain (not squeezed-out) row by the same
    // rule.
    BackendSqueeze row;
    row.offset_ms = 2.999;
    CHECK(row.summarystr(false, 85.0, lw) == "Standard");
    row.offset_ms = 3.0;
    CHECK(row.summarystr(false, 85.0, lw) == "Hard (uncounted)");
}

TEST_CASE("SPSqueeze::is_free: a note on the SP end is inside SP (D13)") {
    // A SqIn is free once its note is inside SP: at the end or before it.
    CHECK(SPSqueeze{SqueezeKind::SqIn, 0.0}.is_free());
    CHECK(SPSqueeze{SqueezeKind::SqIn, -0.0}.is_free());
    CHECK(SPSqueeze{SqueezeKind::SqIn, -0.001}.is_free());
    CHECK_FALSE(SPSqueeze{SqueezeKind::SqIn, 0.001}.is_free());
    // A SqOut is free once its note is already outside: past the end only.
    CHECK_FALSE(SPSqueeze{SqueezeKind::SqOut, 0.0}.is_free());
    CHECK_FALSE(SPSqueeze{SqueezeKind::SqOut, -0.001}.is_free());
    CHECK(SPSqueeze{SqueezeKind::SqOut, 0.001}.is_free());
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

TEST_CASE("title_or_unknown: one fallback for a song with no usable name") {
    CHECK(std::string(kUnknownTitle) == "(unknown)");
    CHECK(title_or_unknown("") == "(unknown)");
    // What the metadata readers wrote before this fallback existed.
    CHECK(title_or_unknown("<unknown title>") == "(unknown)");
    CHECK(title_or_unknown("Some Song") == "Some Song");
}

// A multiplier squeeze is a chord whose notes straddle a to_multiplier step.
// For 2- and 3-note chords, MultSqueeze accepts exactly the straddling
// combos. For 4-note chords it accepts only 7, 17 and 27; this pins that
// as-is (see the comment on MultSqueeze::validate).
TEST_CASE("MultSqueeze accepts exactly the 2- and 3-note chords that straddle a multiplier step") {
    const NoteColor order[] = {NoteColor::Red, NoteColor::Yellow, NoteColor::Kick,
                               NoteColor::Blue, NoteColor::Green};
    auto chord_of = [&](int n) {
        Chord c;
        for (int i = 0; i < n; ++i) c.add_note(order[i]);
        c.apply_cymbal(NoteColor::Yellow);  // a cymbal among pads: something to squeeze
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
    for (int n = 2; n <= 3; ++n)
        for (int combo = 0; combo < 40; ++combo) {
            const bool straddles = to_multiplier(combo + 1) < to_multiplier(combo + n);
            CHECK_MESSAGE(accepted(chord_of(n), combo) == straddles,
                          "n=" << n << " combo=" << combo);
        }
    for (int combo = 0; combo < 40; ++combo)
        CHECK_MESSAGE(accepted(chord_of(4), combo) == (combo == 7 || combo == 17 || combo == 27),
                      "n=4 combo=" << combo);
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
        CHECK_MESSAGE(MultSqueeze(kry, combo).howto() == "Hit [YellowCym] last.",
                      "combo=" << combo);

    // Kick (50) + cymbal (65) + accented Red (100): three values, so the
    // direction decides. Combo 17 carries one note over; 18 leaves one behind.
    Chord kya;
    kya.add_note(NoteColor::Kick);
    kya.add_note(NoteColor::Red);
    kya.add_note(NoteColor::Yellow);
    kya.apply_cymbal(NoteColor::Yellow);
    kya.apply_accent(NoteColor::Red);
    CHECK(MultSqueeze(kya, 17).howto() == "Hit [Red (Accent)] last.");
    CHECK(MultSqueeze(kya, 18).howto() == "Hit [Kick] first.");
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

// The one-writer rule, checked: in src/ and tools/, only model.cpp (set_sqout)
// writes Activation::sqout_tick. Tests may build odd shapes by hand. The
// replay keeps its own windows (ReplayWindow, and replay.cpp's Window), which
// have a field of the same name; those are always named w or win there.
TEST_CASE("only set_sqout writes an activation's sqout_tick") {
    namespace fs = std::filesystem;
    const fs::path root = fs::u8path(HYDRA_SOURCE_DIR);
    // An assignment, reset or emplace, with the object it's on (if any).
    const std::regex write(
        R"((?:(\w+)\s*(?:\.|->)\s*)?\bsqout_tick\s*(?:=(?!=)|\.\s*(?:reset|emplace)\s*\())");
    std::smatch m;
    // The pattern itself: writes are seen, reads are not.
    CHECK(std::regex_search(std::string("    act.sqout_tick = 200;"), write));
    CHECK(std::regex_search(std::string("a->sqout_tick.reset();"), write));
    CHECK(std::regex_search(std::string("sqout_tick.emplace(5);"), write));
    CHECK_FALSE(std::regex_search(std::string("if (act.sqout_tick == t)"), write));
    CHECK_FALSE(std::regex_search(std::string("w.opt_i64(act.sqout_tick);"), write));
    std::string probe = "        win.sqout_tick = w.sqout_tick;";
    REQUIRE(std::regex_search(probe, m, write));
    CHECK(m[1].str() == "win");
    probe = "    const std::optional<int64_t> sqout_tick =";
    REQUIRE(std::regex_search(probe, m, write));
    CHECK(m[1].str().empty());  // a local: no object it's on

    const auto replay_window_file = [](const std::string& rel) {
        return rel == "src/core/replay.cpp" || rel == "tools/replay.cpp" ||
               rel == "tools/replay_json.cpp";
    };
    std::vector<std::string> problems;
    int files = 0, model_writes = 0;
    for (const char* sub : {"src", "tools"}) {
        for (const fs::directory_entry& e : fs::recursive_directory_iterator(root / sub)) {
            const fs::path ext = e.path().extension();
            if (ext != ".cpp" && ext != ".h") continue;
            ++files;
            const std::string rel = fs::relative(e.path(), root).generic_u8string();
            std::ifstream in(e.path());
            std::string line;
            int lineno = 0;
            while (std::getline(in, line)) {
                ++lineno;
                const std::string code = line.substr(0, line.find("//"));
                if (!std::regex_search(code, m, write)) continue;
                if (rel == "src/core/model.cpp") {
                    ++model_writes;
                    continue;
                }
                const std::string on = m[1].str();
                // A bare name is a local of the same name (engine.cpp has
                // one) unless it's inside Activation, whose members live in
                // model.cpp and model.h.
                if (on.empty() && rel != "src/core/model.h") continue;
                if (replay_window_file(rel) && (on == "w" || on == "win")) continue;
                problems.push_back(rel + ":" + std::to_string(lineno) + ": " + line);
            }
        }
    }
    CHECK(files > 50);
    CHECK(model_writes > 0);  // the scan does see the one writer
    INFO(problems.size() << " problem lines; first: "
                         << (problems.empty() ? std::string() : problems.front()));
    CHECK(problems.empty());
}

TEST_CASE("refill_tick: a late squeeze-in's bar arrives at the old end") {
    Activation a;
    a.timecode = Timecode::raw(5760);
    a.sp_end_steps = {{5760, 13440, SpEndKind::Activation},
                      {12000, 15360, SpEndKind::Collected},
                      {15840, 19200, SpEndKind::SqIn}};
    CHECK(a.refill_tick(0) == 5760);
    CHECK(a.refill_tick(1) == 12000);
    CHECK(a.refill_tick(2) == 15360);  // past the end in force: the old end
}
