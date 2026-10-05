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

// D48 Q12: one rule everywhere. Caps 1 and 1000 are the plan's examples.
TEST_CASE("counted: singular at 1, commas from 1,000") {
    CHECK(counted(1, "bar", "bars") == "1 bar");
    CHECK(counted(2, "bar", "bars") == "2 bars");
    CHECK(counted(1000, "bar", "bars") == "1,000 bars");
    CHECK(std::string(has_have(1)) == "has");
    CHECK(std::string(has_have(2)) == "have");
}

// D48 Q2: whole-ms text rounds to nearest. No exact half is pinned.
TEST_CASE("format_ms_whole: nearest whole millisecond with the unit") {
    CHECK(format_ms_whole(12.4) == "12 ms");
    CHECK(format_ms_whole(12.6) == "13 ms");
    CHECK(format_ms_whole(12.25) == "12 ms");
    CHECK(format_ms_whole(163.0) == "163 ms");
}

// Finding 4: the copied path's timing used to cut 12.6 ms down to "12 ms",
// while the badge said 13.
TEST_CASE("notationstr_verbose: a 12.6 ms squeeze copies as 13 ms") {
    Activation a;
    test::set_skips(a, 0);
    a.e_offset = 300.0;  // not e-critical
    a.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 12.6});
    CHECK(a.notationstr_verbose() == "0+ (13 ms)");
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

// The early-fill window has one owner: model.h states both halves of it, the
// fill that refuses (fill_refuses) and the E0 (is_e0). That only model.h
// reads kEarlyFillWindowMs is a row of the single-owner scan
// (test_single_owner.cpp); this checks the two halves' edges.
TEST_CASE("early-fill window: the refusal and the E0 edges") {
    CHECK(fill_e_offset(10000.0, 10050.0) == -50.0);
    CHECK_FALSE(fill_refuses(-kEarlyFillWindowMs));  // exactly 60 ms late still spawns
    CHECK(fill_refuses(-kEarlyFillWindowMs - 0.1));
    Activation a;
    a.e_offset = kEarlyFillWindowMs - 0.1;
    test::set_skips(a, 1);
    CHECK(a.is_e_critical() == is_e0(a.e_offset, 0));
    a.e_offset = kEarlyFillWindowMs;
    CHECK(a.is_e_critical() == is_e0(a.e_offset, 0));
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

TEST_CASE("Activation::needs_timing: free squeezes and slack fills need none (D51 Q4, D13)") {
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

    // A squeeze-in dead on the SP end is inside SP too (D13).
    Activation on_end_in = plain();
    on_end_in.sqinouts = {SPSqueeze{SqueezeKind::SqIn, 0.0}};
    CHECK_FALSE(on_end_in.needs_timing());

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
    p.activations = {free_in, on_end_in, slack_fill};
    CHECK_FALSE(p.needs_timing());
    p.activations.push_back(late_out);
    CHECK(p.needs_timing());
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
    CHECK(Chord::from_code("gn...") == ghost);

    Chord accent;
    accent.add_note(NoteColor::Kick).dynamictype = NoteDynamicType::Accent;
    accent.add_note(NoteColor::Red);
    CHECK(accent.code() == "an...");
    CHECK(Chord::from_code("an...") == accent);

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
TEST_CASE("MultSqueeze::points is the best order minus the worst order") {
    Chord kryb;
    kryb.add_note(NoteColor::Kick);
    kryb.add_note(NoteColor::Red);
    kryb.add_note(NoteColor::Yellow);
    kryb.add_note(NoteColor::Blue);
    kryb.apply_cymbal(NoteColor::Yellow);
    kryb.apply_cymbal(NoteColor::Blue);
    CHECK(MultSqueeze(kryb, 7).points() == 30);

    Chord ry;
    ry.add_note(NoteColor::Red);
    ry.add_note(NoteColor::Yellow);
    ry.apply_cymbal(NoteColor::Yellow);
    CHECK(MultSqueeze(ry, 8).points() == 15);
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
