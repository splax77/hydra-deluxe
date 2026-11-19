// Unit tests for core/model (chord encoding, squeezes). The chord-code
// round-trip lives in test_chord_code.cpp and the transfer scales in
// test_squeeze_rating.cpp; the cases here pin the string/number forms
// directly.

#include "doctest.h"

#include <cmath>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "core/model.h"
#include "core/scoring.h"
#include "core/timing.h"
#include "core/backend_value.h"
#include "core/rules.h"
#include "core/strutil.h"
#include "corpus_default_cfg.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/graph.h"
#include "search/pather.h"

using namespace hydra;

namespace {

using corpus::default_cfg;

// One corpus chart's record under `cfg`.
struct CorpusRecord {
    std::string chart;
    const HydraRecord* rec;
};

// Every corpus chart with notes, analyzed under `cfg`. corpus::analyzed keeps
// each record for the whole run, so the pointers stay valid.
std::vector<CorpusRecord> corpus_records(const app::AnalysisSettings& cfg) {
    std::vector<CorpusRecord> out;
    for (const std::string& chart : corpus::chart_paths()) {
        if (corpus::song(chart, cfg.reading(), cfg.rules).is_empty()) continue;
        out.push_back({chart, &corpus::analyzed(chart, cfg)});
    }
    if (out.empty()) throw std::runtime_error("no corpus chart has notes under " + corpus::root());
    return out;
}

const std::vector<CorpusRecord>& default_records() {
    static const std::vector<CorpusRecord> out = corpus_records(default_cfg());
    return out;
}

// One path the engine wrote, with the chart and record it came from.
struct CorpusPath {
    std::string chart;
    const HydraRecord* rec;
    const Path* path;
};

// Every path in `records`: each record's paths with their variants, then its
// all-0 paths with theirs.
std::vector<CorpusPath> paths_of(const std::vector<CorpusRecord>& records) {
    std::vector<CorpusPath> out;
    for (const CorpusRecord& r : records) {
        for (const Path* p : r.rec->all_paths()) out.push_back({r.chart, r.rec, p});
        for (const Path* p : r.rec->all_allzero_paths()) out.push_back({r.chart, r.rec, p});
    }
    return out;
}

const std::vector<CorpusPath>& default_paths() {
    static const std::vector<CorpusPath> out = paths_of(default_records());
    return out;
}

// One activation the engine wrote, with the chart it came from.
struct CorpusAct {
    std::string chart;
    const Activation* act;
};

const std::vector<CorpusAct>& default_acts() {
    static const std::vector<CorpusAct> out = [] {
        std::vector<CorpusAct> v;
        for (const CorpusPath& cp : default_paths())
            for (const Activation& a : cp.path->walk_activations()) v.push_back({cp.chart, &a});
        return v;
    }();
    return out;
}

}  // namespace

// A tom is worth the base points and a cymbal adds its bonus, both read from
// their owners. A ghost or an accent is worth twice the same note played
// plain.
TEST_CASE("basescore: base points, the cymbal bonus, and a dynamic doubles the note") {
    const ChordNote tom{NoteColor::Red};
    const ChordNote cymbal{NoteColor::Yellow, NoteDynamicType::Normal, NoteCymbalType::Cymbal,
                           false};
    CHECK(tom.basescore() == kNoteBasePoints);
    CHECK(cymbal.basescore() == kNoteBasePoints + kCymbalBonusPoints);
    for (const NoteDynamicType dyn : {NoteDynamicType::Accent, NoteDynamicType::Ghost}) {
        CAPTURE(static_cast<int>(dyn));
        const ChordNote dyn_tom{NoteColor::Red, dyn};
        const ChordNote dyn_cymbal{NoteColor::Yellow, dyn, NoteCymbalType::Cymbal, false};
        CHECK(dyn_tom.basescore() == tom.basescore() * 2);
        CHECK(dyn_cymbal.basescore() == cymbal.basescore() * 2);
    }
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

// The copied path writes each timing of an activation the way every screen
// writes a timing (format_ms, D265): the activation's short notation, then
// its timings in badge order between brackets. Checked on every activation
// the engine writes for the corpus.
TEST_CASE("notationstr_verbose: each corpus activation's timings, as format_ms writes them") {
    int timed = 0;
    for (const CorpusAct& ca : default_acts()) {
        const Activation& a = *ca.act;
        CAPTURE(ca.chart);
        CAPTURE(a.timecode.ticks());
        const std::vector<ActivationTiming> timings = a.badge_timings();
        if (timings.empty()) {
            CHECK(a.notationstr_verbose() == a.notationstr());
            continue;
        }
        ++timed;
        std::string inner;
        for (size_t i = 0; i < timings.size(); ++i) {
            if (i) inner += "/";
            inner += format_ms(timings[i].ms);
        }
        CHECK(a.notationstr_verbose() == a.notationstr() + " (" + inner + ")");
    }
    // The corpus must give the copy text a timing to write, or this checks
    // only the untimed form.
    CHECK(timed > 0);
}

// The symbols the notation writes and the names the stored text reads back.
TEST_CASE("squeeze symbols and names: one per kind, and each name reads back as its kind") {
    const SPSqueeze sqin{SqueezeKind::SqIn, 0.0};
    const SPSqueeze sqout{SqueezeKind::SqOut, 0.0};
    CHECK(std::string(sqin.symbol()) != std::string(sqout.symbol()));
    CHECK(std::string(sqin.type_name()) != std::string(sqout.type_name()));
    CHECK(squeeze_kind_from_name(sqin.type_name()) == std::optional<SqueezeKind>(SqueezeKind::SqIn));
    CHECK(squeeze_kind_from_name(sqout.type_name()) ==
          std::optional<SqueezeKind>(SqueezeKind::SqOut));

    // On every corpus activation, the notation ends with one symbol per
    // squeeze in list order, after the skip count.
    int squeezed = 0;
    for (const CorpusAct& ca : default_acts()) {
        const Activation& a = *ca.act;
        if (a.sqinouts.empty()) continue;
        CAPTURE(ca.chart);
        CAPTURE(a.timecode.ticks());
        ++squeezed;
        std::string syms;
        for (const SPSqueeze& sq : a.sqinouts) syms += sq.symbol();
        const std::string n = a.notationstr();
        REQUIRE(n.size() > syms.size());
        CHECK(ends_with(n, syms));
        CHECK(ends_with(n.substr(0, n.size() - syms.size()), std::to_string(a.skips())));
    }
    CHECK(squeezed > 0);
}

// What the code does with a squeeze's offset, on every squeeze the engine
// writes for the corpus: its difficulty is squeeze_difficulty's for its
// kind, its timing is the offset turned round, and it is difficult exactly
// when that difficulty is past the floor.
TEST_CASE("squeeze timing and difficulty follow the offset on every corpus squeeze") {
    int sqins = 0, sqouts = 0;
    for (const CorpusAct& ca : default_acts()) {
        for (const SPSqueeze& sq : ca.act->sqinouts) {
            CAPTURE(ca.chart);
            CAPTURE(sq.offset_ms);
            const bool is_sqin = sq.kind == SqueezeKind::SqIn;
            (is_sqin ? sqins : sqouts)++;
            CHECK(sq.difficulty() == squeeze_difficulty(is_sqin, sq.offset_ms));
            CHECK(sq.timing() == -sq.offset_ms + 0.0);
            CHECK(sq.is_difficult() == past_difficult_floor(sq.difficulty()));
            CHECK(sq.needs_no_timing() == !timing_needs_hitting(sq.difficulty()));
        }
    }
    CHECK(sqins > 0);
    CHECK(sqouts > 0);
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
}

// Path::is_allzero on every path the engine writes for the corpus: a path is
// all-0 exactly when it has activations and each one fits the rule at its
// own window. Every path the all-0 pass writes is all-0.
TEST_CASE("Path::is_allzero: every corpus path, and every path of the all-0 pass") {
    int allzero = 0, other = 0;
    for (const CorpusPath& cp : default_paths()) {
        const Path& p = *cp.path;
        CAPTURE(cp.chart);
        CAPTURE(p.pathstring());
        bool each = p.has_activations();
        for (const Activation& a : p.walk_activations())
            each = each && allzero_activation(a.e_offset, a.skips(), a.e_window_ms);
        CHECK(p.is_allzero() == each);
        (p.is_allzero() ? allzero : other)++;
    }
    for (const CorpusRecord& r : default_records()) {
        CAPTURE(r.chart);
        for (const Path* p : r.rec->all_allzero_paths()) {
            CAPTURE(p->pathstring());
            CHECK(p->is_allzero());
        }
    }
    // Both answers must turn up, or the comparison above checks only one.
    CHECK(allzero > 0);
    CHECK(other > 0);
}

// Every activation the engine writes carries the window its search ran
// under, at the default and at the widest the setting allows, and its E
// test reads that window. The widest window runs on the first few corpus
// charts only: one more analysis per chart is the slow part.
TEST_CASE("early-fill window: each corpus activation keeps its search's window") {
    app::AnalysisSettings wide = default_cfg();
    wide.early_fill_window_ms = kHitWindowMs;
    std::vector<CorpusRecord> wide_records;
    for (const CorpusRecord& r : default_records()) {
        if (wide_records.size() == 5) break;
        wide_records.push_back({r.chart, &corpus::analyzed(r.chart, wide)});
    }
    const std::pair<double, const std::vector<CorpusRecord>*> runs[] = {
        {default_cfg().early_fill_window_ms, &default_records()},
        {kHitWindowMs, &wide_records},
    };
    for (const auto& run : runs) {
        const double window = run.first;
        const std::vector<CorpusRecord>& records = *run.second;
        CAPTURE(window);
        int critical = 0, not_critical = 0;
        for (const CorpusPath& cp : paths_of(records)) {
            for (const Activation& a : cp.path->walk_activations()) {
                CAPTURE(cp.chart);
                CAPTURE(a.timecode.ticks());
                CHECK(a.e_window_ms == window);
                CHECK(a.is_e_critical() == is_e0(a.e_offset, 0, window));
                CHECK(a.is_E0() == is_e0(a.e_offset, a.skips(), window));
                (a.is_e_critical() ? critical : not_critical)++;
            }
        }
        // Both answers must turn up, or the E test is checked on one side only.
        CHECK(critical > 0);
        CHECK(not_critical > 0);
    }
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
    }
    // Activation::is_e_critical against is_e0 on real activations is
    // "early-fill window: each corpus activation keeps its search's window".
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

// On every corpus path: a path is difficult when one of its activations is,
// and an activation is difficult when a timing it must hit is warn-coloured
// (ActivationTiming::required and ::difficult, badge_timings).
TEST_CASE("Path::is_difficult: a corpus path is difficult when an activation's required timing is") {
    Path empty;
    CHECK_FALSE(empty.is_difficult());

    int difficult = 0, plain = 0;
    for (const CorpusPath& cp : default_paths()) {
        const Path& p = *cp.path;
        CAPTURE(cp.chart);
        CAPTURE(p.pathstring_verbose({}));
        bool any_act = false;
        for (const Activation& a : p.walk_activations()) {
            bool any_timing = false;
            for (const ActivationTiming& t : a.badge_timings())
                any_timing = any_timing || (t.required && t.difficult);
            CHECK(a.is_difficult() == any_timing);
            any_act = any_act || a.is_difficult();
        }
        CHECK(p.is_difficult() == any_act);
        (p.is_difficult() ? difficult : plain)++;
    }
    // Both answers must turn up, or the comparison checks only one.
    CHECK(difficult > 0);
    CHECK(plain > 0);
}

// D48 Q3: a timing exactly on the floor is inside it; the next value up is
// past it. The floor is read from its owner.
TEST_CASE("past_difficult_floor: kDifficultMs itself is not past it") {
    const double above = std::nextafter(kDifficultMs, kDifficultMs + 1.0);
    CHECK_FALSE(past_difficult_floor(kDifficultMs));
    CHECK(past_difficult_floor(above));
    for (const SqueezeKind kind : {SqueezeKind::SqIn, SqueezeKind::SqOut}) {
        const bool is_sqin = kind == SqueezeKind::SqIn;
        CAPTURE(is_sqin);
        // The offset whose difficulty is the floor, and the one just past it.
        const SPSqueeze on{kind, is_sqin ? kDifficultMs : -kDifficultMs};
        const SPSqueeze past{kind, is_sqin ? above : -above};
        REQUIRE(on.difficulty() == kDifficultMs);
        CHECK_FALSE(on.is_difficult());
        CHECK(past.is_difficult());
    }
}

// The highest total over every path a record holds, variants included.
static std::optional<int64_t> top_total(const HydraRecord& rec) {
    std::optional<int64_t> top;
    for (const Path* p : rec.all_paths())
        if (!top || p->totalscore() > *top) top = p->totalscore();
    return top;
}

// On every corpus record: a path is optimal exactly when it ties the highest
// total of any path the record holds. A tied variant counts as optimal too.
TEST_CASE("HydraRecord::is_optimal: every corpus path tied at the top score") {
    int tied_beside_best = 0, below = 0;
    for (const CorpusRecord& r : default_records()) {
        CAPTURE(r.chart);
        const std::optional<int64_t> top = top_total(*r.rec);
        REQUIRE(top.has_value());
        const Path& best = r.rec->paths.at(0);
        for (const Path* p : r.rec->all_paths()) {
            CAPTURE(p->pathstring());
            const bool at_top = p->totalscore() == *top;
            CHECK(r.rec->is_optimal(*p) == at_top);
            if (at_top && p != &best) ++tied_beside_best;
            if (!at_top) ++below;
        }
    }
    // A path tied with the best one and a path below it must both turn up,
    // or the check reads one side of the rule only.
    CHECK(tied_beside_best > 0);
    CHECK(below > 0);
}

// On every corpus record the optimal score is the highest total of any path
// it holds. A record with no paths, the state before any analysis, has none,
// and no path is optimal in it.
TEST_CASE("HydraRecord::optimal_score: the highest corpus path total, empty with no paths") {
    for (const CorpusRecord& r : default_records()) {
        CAPTURE(r.chart);
        CHECK(r.rec->optimal_score() == top_total(*r.rec));
    }
    const HydraRecord empty{};
    CHECK_FALSE(empty.optimal_score().has_value());
    CHECK_FALSE(empty.is_optimal(default_records().front().rec->paths.at(0)));
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

// On every corpus activation, hardest() names one of the activation's own
// timings (badge_timings) and leaves out the two kinds that need no hitting:
// a squeeze that needs no timing and a fill with time to spare. A required
// timing larger than the one named never exists. A required fill is named
// only when it beats every squeeze, so a tie names the squeeze (D48 Q2). An
// optional fill is named only when nothing required is (D48 Q10), and is
// never a difficulty.
TEST_CASE("Activation::hardest: on every corpus activation, the largest timing to hit") {
    using P = TimingPart;
    int named_squeeze = 0, named_none = 0;
    for (const CorpusAct& ca : default_acts()) {
        const Activation& a = *ca.act;
        CAPTURE(ca.chart);
        CAPTURE(a.timecode.ticks());
        CAPTURE(a.notationstr_verbose());
        const std::vector<ActivationTiming> timings = a.badge_timings();
        // The timings that take part: those that need hitting.
        std::vector<ActivationTiming> to_hit;
        for (const ActivationTiming& t : timings)
            if (t.part == P::EarlyFill ? early_fill_needs_timing(a.e_offset) : t.required)
                to_hit.push_back(t);

        const std::optional<HardestTiming> h = a.hardest();
        CHECK(h.has_value() == !to_hit.empty());
        if (!h) {
            ++named_none;
            CHECK_FALSE(a.difficulty().has_value());
            continue;
        }
        if (h->part != P::EarlyFill) ++named_squeeze;
        bool listed = false;
        for (const ActivationTiming& t : to_hit) listed = listed || (t.part == h->part && t.ms == h->ms);
        CHECK(listed);
        for (const ActivationTiming& t : to_hit) {
            if (!t.required) continue;
            CHECK(t.ms <= h->ms);
            // A required fill named over a squeeze must beat it outright.
            if (h->part == P::EarlyFill && t.part != P::EarlyFill) CHECK(t.ms < h->ms);
        }
        const bool optional_fill = h->part == P::EarlyFill && !a.is_E0();
        if (optional_fill) {
            for (const ActivationTiming& t : to_hit) CHECK_FALSE(t.required);
            CHECK_FALSE(a.difficulty().has_value());
        } else {
            CHECK(a.difficulty() == std::optional<double>(h->ms));
        }
    }
    // A squeeze named hardest and an activation with nothing to time must
    // both turn up, or the checks above read one side only.
    CHECK(named_squeeze > 0);
    CHECK(named_none > 0);
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

// On every corpus activation, badge_timings lists the early fill first when
// the activation is E-critical, then each squeeze in sqinouts order. The fill
// is required when it is an E0 and warn-coloured only when required and past
// the floor (D48 Q10, D261). A squeeze is required when it needs timing and
// warn-coloured when it is difficult (D80). A timing that needs no hitting
// has a figure below 0 (D302).
TEST_CASE("Activation::badge_timings: every corpus activation's timings in notation order") {
    using P = TimingPart;
    int with_fill = 0, with_squeeze = 0;
    for (const CorpusAct& ca : default_acts()) {
        const Activation& a = *ca.act;
        CAPTURE(ca.chart);
        CAPTURE(a.timecode.ticks());
        const std::vector<ActivationTiming> t = a.badge_timings();
        const size_t fill = a.is_e_critical() ? 1 : 0;
        REQUIRE(t.size() == fill + a.sqinouts.size());
        if (fill) {
            ++with_fill;
            CHECK(t[0].part == P::EarlyFill);
            CHECK(t[0].ms == early_fill_difficulty(a.e_offset));
            CHECK(t[0].required == a.is_E0());
            CHECK(t[0].difficult == (t[0].required && past_difficult_floor(t[0].ms)));
            if (!early_fill_needs_timing(a.e_offset)) CHECK(t[0].ms < 0.0);
        }
        for (size_t k = 0; k < a.sqinouts.size(); ++k) {
            const SPSqueeze& sq = a.sqinouts[k];
            const ActivationTiming& row = t[fill + k];
            CAPTURE(k);
            ++with_squeeze;
            CHECK(row.part == (sq.kind == SqueezeKind::SqIn ? P::SqueezeIn : P::SqueezeOut));
            CHECK(row.ms == sq.difficulty());
            CHECK(row.required == !sq.needs_no_timing());
            CHECK(row.difficult == sq.is_difficult());
            if (!row.required) CHECK(row.ms < 0.0);
        }
    }
    // A listed fill and a listed squeeze must both turn up.
    CHECK(with_fill > 0);
    CHECK(with_squeeze > 0);
}

// D85: a timing exactly at the limit is inside it, a 0 ms timing at a 0 ms
// limit included. On every corpus activation, the limit that just holds is
// the largest figure the limit judges: every squeeze, free ones too, and the
// early fill only when it is an E0 (an optional fill is never a limit's
// business). At that figure the activation is inside; a hair below it, it is
// outside. A path is inside when every activation is; one with none always is.
TEST_CASE("Activation::within_ms_limit: inside at its largest timing, outside just below") {
    using P = TimingPart;
    int judged = 0, unjudged = 0;
    for (const CorpusPath& cp : default_paths()) {
        const Path& p = *cp.path;
        CAPTURE(cp.chart);
        CAPTURE(p.pathstring_verbose({}));
        std::optional<double> path_top;
        for (const Activation& a : p.walk_activations()) {
            CAPTURE(a.timecode.ticks());
            std::optional<double> top;
            for (const ActivationTiming& t : a.badge_timings()) {
                if (t.part == P::EarlyFill && !t.required) continue;
                if (!top || t.ms > *top) top = t.ms;
            }
            if (!top) {
                ++unjudged;
                // Nothing the limit judges: inside any limit at all.
                CHECK(a.within_ms_limit(-std::numeric_limits<double>::max()));
                continue;
            }
            ++judged;
            CHECK(a.within_ms_limit(*top));
            CHECK_FALSE(a.within_ms_limit(std::nextafter(*top, -std::numeric_limits<double>::max())));
            if (!path_top || *top > *path_top) path_top = top;
        }
        if (path_top) {
            CHECK(p.within_ms_limit(*path_top));
            CHECK_FALSE(p.within_ms_limit(std::nextafter(*path_top, -std::numeric_limits<double>::max())));
        } else {
            CHECK(p.within_ms_limit(-std::numeric_limits<double>::max()));
        }
    }
    CHECK(judged > 0);
    CHECK(unjudged > 0);

    const Path none;
    CHECK(none.within_ms_limit(0.0));
}

// On every corpus activation, read against the SP-end steps the engine wrote.
// Each end is measured from the latest clamp at or before the step that set
// it, else from the activation (D1). The k-th SqIn is the k-th SqIn step; its
// end is the end in force just before that step, and its chord is that
// step's note. A SqOut is measured from the final end, with that end's
// anchor.
TEST_CASE("Activation: each end's anchor and each squeeze's end, from the corpus steps") {
    int sqins = 0, sqouts = 0, clamped = 0;
    for (const CorpusAct& ca : default_acts()) {
        const Activation& a = *ca.act;
        CAPTURE(ca.chart);
        CAPTURE(a.timecode.ticks());
        const std::vector<SpEndStep>& steps = a.sp_end_steps;
        REQUIRE_FALSE(steps.empty());

        // Each step's anchor, walked forward: the latest clamp so far.
        std::vector<int64_t> anchor(steps.size());
        int64_t latest = a.timecode.ticks();
        for (size_t s = 0; s < steps.size(); ++s) {
            if (is_clamp_kind(steps[s].kind)) {
                latest = steps[s].tick;
                ++clamped;
            }
            anchor[s] = latest;
            CAPTURE(s);
            CHECK(a.end_anchor_tick(s) == anchor[s]);
        }
        CHECK(a.deact_anchor_tick() == std::optional<int64_t>(anchor.back()));

        // The SqIn steps, in order, after the activation's own.
        std::vector<size_t> sqin_steps;
        for (size_t s = 1; s < steps.size(); ++s)
            if (is_sqin_kind(steps[s].kind)) sqin_steps.push_back(s);
        size_t rank = 0;
        for (size_t k = 0; k < a.sqinouts.size(); ++k) {
            CAPTURE(k);
            if (a.sqinouts[k].kind == SqueezeKind::SqOut) {
                ++sqouts;
                CHECK(a.squeeze_end_tick(k) == a.deact_tick());
                CHECK(a.squeeze_anchor_tick(k) == a.deact_anchor_tick());
                CHECK(a.squeeze_chord_tick(k) == a.sqout_tick);
                continue;
            }
            ++sqins;
            REQUIRE(rank < sqin_steps.size());
            const size_t s = sqin_steps[rank++];
            CHECK(a.squeeze_end_tick(k) == std::optional<int64_t>(steps[s - 1].end_tick));
            CHECK(a.squeeze_anchor_tick(k) == std::optional<int64_t>(anchor[s - 1]));
            CHECK(a.squeeze_chord_tick(k) == std::optional<int64_t>(steps[s].tick));
        }
    }
    // A SqIn, a SqOut and a clamp must each turn up, or part of the rule
    // above goes unread.
    CHECK(sqins > 0);
    CHECK(sqouts > 0);
    CHECK(clamped > 0);
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

// The text of a list of pieces, each pair split by `sep`.
static std::string pieces_joined(const std::vector<std::string>& parts, std::string_view sep) {
    std::string out;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) out += sep;
        out += parts[i];
    }
    return out;
}

// The pieces of `text` between each `sep`.
static std::vector<std::string> split_on(const std::string& text, std::string_view sep) {
    std::vector<std::string> out;
    size_t from = 0;
    for (size_t at; (at = text.find(sep, from)) != std::string::npos; from = at + sep.size())
        out.push_back(text.substr(from, at - from));
    out.push_back(text.substr(from));
    return out;
}

// On every corpus path: the short text is each activation's notation in walk
// order, a space apart. The copied text is three sections split by the
// writer's separator: the record's multiplier squeezes (or the words for
// none), the activations' long notations, and the score grouped by
// group_thousands. A path with no activations writes the same words for
// none in both texts, and its score section differs from a real path's only
// in the number.
TEST_CASE("Path pathstring and pathstring_verbose: every corpus path's sections") {
    const Path empty;
    const std::vector<std::string> empty_sections =
        split_on(empty.pathstring_verbose({}), kPathSectionSeparator);
    REQUIRE(empty_sections.size() == 3);
    CHECK(empty_sections[1] == empty.pathstring());
    const std::string zero = group_thousands(0);
    REQUIRE(empty_sections[2].size() > zero.size());
    CHECK(empty_sections[2].substr(empty_sections[2].size() - zero.size()) == zero);
    const std::string score_words =
        empty_sections[2].substr(0, empty_sections[2].size() - zero.size());

    int with_mult = 0, without_mult = 0;
    for (const CorpusPath& cp : default_paths()) {
        const Path& p = *cp.path;
        CAPTURE(cp.chart);
        std::vector<std::string> shorts, longs;
        for (const Activation& a : p.walk_activations()) {
            shorts.push_back(a.notationstr());
            longs.push_back(a.notationstr_verbose());
        }
        if (shorts.empty()) {
            CHECK(p.pathstring() == empty.pathstring());
        } else {
            CHECK(p.pathstring() == pieces_joined(shorts, " "));
        }

        const std::vector<MultSqueeze>& mults = cp.rec->multsqueezes;
        const std::vector<std::string> sections =
            split_on(p.pathstring_verbose(mults), kPathSectionSeparator);
        CAPTURE(p.pathstring_verbose(mults));
        REQUIRE(sections.size() == 3);
        if (mults.empty()) {
            ++without_mult;
            CHECK(sections[0] == empty_sections[0]);
        } else {
            ++with_mult;
            std::vector<std::string> ms;
            for (const MultSqueeze& m : mults) ms.push_back(m.notationstr());
            CHECK(sections[0] == pieces_joined(ms, ", "));
        }
        CHECK(sections[1] == (longs.empty() ? empty_sections[1] : pieces_joined(longs, " ")));
        CHECK(sections[2] == score_words + group_thousands(p.totalscore()));
    }
    // A record with multiplier squeezes and one without must both turn up.
    CHECK(with_mult > 0);
    CHECK(without_mult > 0);
}

// walk_activations reads a path's activations in place: its own, then the
// tail it shares with its parent, in that order, and copies nothing. Checked
// on every corpus path; a variant with both its own activations and a shared
// tail must turn up.
TEST_CASE("Path::walk_activations: own activations then the variant tail, in place") {
    int both_sides = 0;
    for (const CorpusPath& cp : default_paths()) {
        const Path& p = *cp.path;
        CAPTURE(cp.chart);
        CAPTURE(p.pathstring());
        const size_t own = p.activations.size();
        const size_t tail = p.variant_tail.size();
        if (own > 0 && tail > 0) ++both_sides;

        const ActivationWalk walk = p.walk_activations();
        REQUIRE(walk.size() == own + tail);
        CHECK(walk.empty() == (own + tail == 0));
        CHECK(p.has_activations() == !walk.empty());
        // In place: each element is the very object in the path, not a copy.
        std::vector<const Activation*> expected;
        for (const Activation& a : p.activations) expected.push_back(&a);
        for (const Activation& a : p.variant_tail) expected.push_back(&a);
        for (size_t i = 0; i < expected.size(); ++i) CHECK(&walk[i] == expected[i]);
        if (!walk.empty()) {
            CHECK(&walk.front() == expected.front());
            CHECK(&walk.back() == expected.back());
        }
        // A range-for visits the same objects in the same order.
        std::vector<const Activation*> seen;
        for (const Activation& a : walk) seen.push_back(&a);
        CHECK(seen == expected);

        // The same sequence the copying all_activations() hands out.
        const std::vector<Activation> copied = p.all_activations();
        REQUIRE(copied.size() == walk.size());
        for (size_t i = 0; i < copied.size(); ++i) {
            CHECK(copied[i].timecode.ticks() == walk[i].timecode.ticks());
            CHECK(copied[i].skipped_fill_ticks == walk[i].skipped_fill_ticks);
        }
    }
    CHECK(both_sides > 0);

    // A path with nothing on either side, as a chart with no activation gives.
    const Path none;
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
// The engine's copy-out (set_sqout) already drops every row past the
// squeezed-out chord, so a list holding one is a state no chart reaches. What
// a real activation shows is checked instead: each of its rows inside the
// squeeze window, its squeezed-out row wherever it falls, nothing past that
// chord, all in the stored order.
TEST_CASE("display_backends: every corpus activation shows its window rows and its squeezed-out row") {
    int with_sqout = 0;
    for (const CorpusAct& ca : default_acts()) {
        const Activation& a = *ca.act;
        CAPTURE(ca.chart);
        CAPTURE(a.timecode.ticks());
        std::vector<BackendSqueeze> expected;
        for (const BackendSqueeze& b : a.backends) {
            const bool past = core::sqout_position(b.timecode.ticks(), a.sqout_tick) ==
                              core::SqOutPosition::After;
            if (!past && (within_squeeze_window(b.offset()) || a.is_sqout_backend(b)))
                expected.push_back(b);
        }
        const std::vector<BackendSqueeze> shown = a.display_backends();
        CHECK(shown == expected);
        if (const BackendSqueeze* row = a.sqout_row()) {
            ++with_sqout;
            bool listed = false;
            for (const BackendSqueeze& b : shown) listed = listed || b == *row;
            CHECK(listed);
        }
    }
    // The anchor test above finds SqOut squeezes on the corpus, and set_sqout
    // throws unless the activation has a row on the squeezed-out chord, so a
    // squeezed-out row must turn up here.
    CHECK(with_sqout > 0);
}

TEST_CASE("Chord rowstr / notationstr / disco flip") {
    Chord c;
    c.add_note(NoteColor::Red);
    c.add_note(NoteColor::Green);
    // notationstr holds one place per lane, kick first: each note's letter
    // (color_notationstr) in its lane's place, a space where the lane is empty.
    const std::string notation = c.notationstr();
    const NoteColor lanes[] = {NoteColor::Kick, NoteColor::Red, NoteColor::Yellow,
                               NoteColor::Blue, NoteColor::Green};
    REQUIRE(notation.size() == std::size(lanes) + 2);
    for (size_t i = 0; i < std::size(lanes); ++i) {
        CAPTURE(color_str(lanes[i]));
        const std::string place = notation.substr(i + 1, 1);
        CHECK(place == (c.at(lanes[i]).has_value() ? color_notationstr(lanes[i]) : " "));
    }
    // rowstr names each note in the Pro Drums setting's words (ChordNote::str),
    // red before green, either way the setting is.
    for (bool pro : {true, false}) {
        CAPTURE(pro);
        const std::string row = c.rowstr(pro);
        const size_t red_at = row.find(ChordNote{NoteColor::Red}.str(pro));
        const size_t green_at = row.find(ChordNote{NoteColor::Green}.str(pro));
        REQUIRE(red_at != std::string::npos);
        REQUIRE(green_at != std::string::npos);
        CHECK(red_at < green_at);
    }
    CHECK(c.rowstr(true) != c.rowstr(false));

    // Disco flip swaps red<->yellow (red becomes a yellow cymbal).
    Chord d;
    d.add_note(NoteColor::Red);
    d.apply_disco_flip();
    REQUIRE(d.at(NoteColor::Yellow).has_value());
    CHECK(d.at(NoteColor::Yellow)->cymbaltype == NoteCymbalType::Cymbal);
    CHECK_FALSE(d.at(NoteColor::Red).has_value());
}

TEST_CASE("a ghost or accent kick scores double, like a pad") {
    // A plain kick is the base points (kNoteBasePoints); a ghost or accent
    // kick is worth twice a plain one, the same doubling a red pad gets.
    const int kick = ChordNote{NoteColor::Kick}.basescore();
    CHECK(kick == kNoteBasePoints);
    for (NoteDynamicType dyn : {NoteDynamicType::Ghost, NoteDynamicType::Accent}) {
        CAPTURE(dynamic_label(dyn));
        CHECK(ChordNote{NoteColor::Kick, dyn}.basescore() == 2 * kick);
        CHECK(ChordNote{NoteColor::Kick, dyn}.basescore() ==
              ChordNote{NoteColor::Red, dyn}.basescore());
        // A 2x kick is still a kick: the foot, not the value, is what changes.
        CHECK(ChordNote{NoteColor::Kick, dyn, NoteCymbalType::Normal, true}.basescore() ==
              ChordNote{NoteColor::Kick, dyn}.basescore());
    }
}

TEST_CASE("ChordNote::str shows the kick's dynamic and its 2x flag") {
    // Each note reads as its name (note_label), and a ghost or accent adds its
    // word (dynamic_label) after the name; a plain note adds nothing. The 2x
    // flag is part of the kick's name, so a 2x kick and a plain one read
    // differently at every dynamic.
    const NoteDynamicType dyns[] = {NoteDynamicType::Normal, NoteDynamicType::Ghost,
                                    NoteDynamicType::Accent};
    std::vector<ChordNote> notes;
    for (NoteDynamicType dyn : dyns) {
        notes.push_back(ChordNote{NoteColor::Kick, dyn, NoteCymbalType::Normal, false});
        notes.push_back(ChordNote{NoteColor::Kick, dyn, NoteCymbalType::Normal, true});
        notes.push_back(ChordNote{NoteColor::Red, dyn});
        notes.push_back(ChordNote{NoteColor::Yellow, dyn});
        notes.push_back(ChordNote{NoteColor::Yellow, dyn, NoteCymbalType::Cymbal, false});
    }
    for (const ChordNote& note : notes) {
        for (bool pro : {true, false}) {
            CAPTURE(note.str(pro));
            const std::string name = note_label(note, pro);
            const std::string word = dynamic_label(note.dynamictype);
            const std::string shown = note.str(pro);
            if (note.dynamictype == NoteDynamicType::Normal) {
                CHECK(word.empty());
                CHECK(shown == name);
                continue;
            }
            REQUIRE_FALSE(word.empty());
            CHECK(starts_with(shown, name));
            CHECK(shown.size() > name.size() + word.size());
            CHECK(shown.find(word, name.size()) != std::string::npos);
        }
    }
    for (NoteDynamicType dyn : dyns) {
        CAPTURE(dynamic_label(dyn));
        CHECK(ChordNote{NoteColor::Kick, dyn, NoteCymbalType::Normal, true}.str() !=
              ChordNote{NoteColor::Kick, dyn, NoteCymbalType::Normal, false}.str());
    }
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

    const ChordNote kick{NoteColor::Kick};

    // With Pro Drums off a tom or the snare is its colour alone (color_str).
    CHECK(note_label(yellow_tom, false) == color_str(NoteColor::Yellow));
    CHECK(note_label(green_tom, false) == color_str(NoteColor::Green));
    CHECK(note_label(red, false) == color_str(NoteColor::Red));
    // With Pro Drums on each takes a type word after its colour.
    for (const ChordNote& pad : {yellow_tom, green_tom, red}) {
        CAPTURE(color_str(pad.colortype));
        const std::string colour = color_str(pad.colortype);
        CHECK(starts_with(note_label(pad, true), colour));
        CHECK(note_label(pad, true).size() > colour.size());
    }
    // A tom and the snare take different type words.
    const std::string tom_word = note_label(green_tom, true).substr(color_str(NoteColor::Green).size());
    const std::string snare_word = note_label(red, true).substr(color_str(NoteColor::Red).size());
    CHECK(tom_word != snare_word);
    CHECK(note_label(yellow_tom, true).substr(color_str(NoteColor::Yellow).size()) == tom_word);

    // A cymbal is named the same either way, and not as the tom of its colour.
    CHECK(note_label(yellow_cym, true) == note_label(yellow_cym, false));
    CHECK(note_label(yellow_cym, false) != note_label(yellow_tom, false));
    CHECK(note_label(yellow_cym, true) != note_label(yellow_tom, true));
    CHECK(starts_with(note_label(yellow_cym, true), color_str(NoteColor::Yellow)));

    // The kick has no tom or snare: its name ignores the setting, and the 2x
    // flag gives it a name of its own.
    CHECK(note_label(kick, true) == color_str(NoteColor::Kick));
    CHECK(note_label(kick, false) == note_label(kick, true));
    CHECK(note_label(kick2x, false) == note_label(kick2x, true));
    CHECK(note_label(kick2x, true) != note_label(kick, true));

    // A ghost or accent is not part of the name.
    for (NoteDynamicType dyn : {NoteDynamicType::Ghost, NoteDynamicType::Accent})
        for (bool pro : {true, false})
            CHECK(note_label(ChordNote{NoteColor::Green, dyn}, pro) == note_label(green_tom, pro));
}

TEST_CASE("Chord::code spells a ghost/accent kick in the kick lane") {
    // A kick and a red, with the kick plain, ghosted and accented. The kick's
    // place in the code is its lane's, the first (NoteColor::Kick is lane 1).
    auto kick_and_red = [](NoteDynamicType dyn) {
        Chord c;
        c.add_note(NoteColor::Kick).dynamictype = dyn;
        c.add_note(NoteColor::Red);
        return c;
    };
    const Chord normal = kick_and_red(NoteDynamicType::Normal);
    const Chord ghost = kick_and_red(NoteDynamicType::Ghost);
    const Chord accent = kick_and_red(NoteDynamicType::Accent);
    const size_t kick_place = static_cast<size_t>(NoteColor::Kick) - 1;

    // The three codes differ in the kick's place and nowhere else.
    const std::string n = normal.code(), g = ghost.code(), a = accent.code();
    REQUIRE(n.size() == g.size());
    REQUIRE(n.size() == a.size());
    REQUIRE(kick_place < n.size());
    for (size_t i = 0; i < n.size(); ++i) {
        CAPTURE(i);
        if (i == kick_place) {
            CHECK(n[i] != g[i]);
            CHECK(n[i] != a[i]);
            CHECK(g[i] != a[i]);
        } else {
            CHECK(n[i] == g[i]);
            CHECK(n[i] == a[i]);
        }
    }
    // Each of those marks is a note's, not the empty lane's: a chord of the
    // red alone has another mark in the kick's place.
    Chord red_only;
    red_only.add_note(NoteColor::Red);
    CHECK(red_only.code()[kick_place] != n[kick_place]);
    CHECK(red_only.code()[kick_place] != g[kick_place]);
    CHECK(red_only.code()[kick_place] != a[kick_place]);

    // The row names the ghost kick by its own words (ChordNote::str).
    const ChordNote ghost_kick{NoteColor::Kick, NoteDynamicType::Ghost};
    CHECK(ghost.rowstr().find(ghost_kick.str()) != std::string::npos);
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
