// Unit tests for core/model (chord encoding, squeezes). The chord-code
// round-trip lives in test_chord_code.cpp and the transfer scales in
// test_squeeze_rating.cpp; the cases here pin the string/number forms
// directly.

#include "doctest.h"

#include <algorithm>
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
#include "app/user_messages.h"
#include "core/model.h"
#include "core/scoring.h"
#include "core/squeeze_rating.h"
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

// How many backend rows a walk saw, and how many of them the search priced
// apart (sqout_sp_points not equal to sp_points).
struct BackendRowCounts {
    int rows = 0;
    int rows_priced_apart = 0;
};

// Runs `per_row(sp, sq)` once for every backend row of every corpus
// activation, with that row's sp_points and sqout_sp_points. Each call runs
// under CAPTURE of the chart, the row's tick and both prices, so a failing
// check inside it names the row.
template <class PerRow>
BackendRowCounts for_each_backend_row(PerRow&& per_row) {
    BackendRowCounts counts;
    for (const CorpusAct& ca : default_acts()) {
        for (const BackendSqueeze& b : ca.act->backends) {
            ++counts.rows;
            const int sp = b.sp_points;
            const int sq = b.sqout_sp_points;
            if (sp != sq) ++counts.rows_priced_apart;
            CAPTURE(ca.chart);
            CAPTURE(b.timecode.ticks());
            CAPTURE(sp);
            CAPTURE(sq);
            per_row(sp, sq);
        }
    }
    return counts;
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

// Every note of every corpus chart is priced from one 1x value,
// ChordNote::basescore: the Notes, accent and ghost shares add up to it, and
// with the combo share they make it at the note's own multiplier, which is
// to_multiplier of the combo once that note is hit.
TEST_CASE("note value: one owner for base, cymbal and dynamic points") {
    const app::AnalysisSettings& cfg = default_cfg();
    int priced = 0;
    for (const CorpusRecord& r : default_records()) {
        CAPTURE(r.chart);
        const Song& song = corpus::song(r.chart, cfg.reading(), cfg.rules);
        const ChordScoreTable table =
            chord_score_table(song, cfg.rules.sqout_rule, ChordScoreDetail::WithNotes);
        REQUIRE(table.rows.size() == song.sequence.size());
        for (size_t i = 0; i < table.rows.size(); ++i) {
            const ChordScoreRow& row = table.rows[i];
            const std::vector<ChordNote> notes = song.sequence[i].chord.notes(true);
            CAPTURE(song.sequence[i].timecode.ticks());
            REQUIRE(static_cast<size_t>(row.note_end - row.note_begin) == notes.size());
            for (size_t k = 0; k < notes.size(); ++k) {
                CAPTURE(k);
                const CategoryScores& n = table.notes[row.note_begin + k];
                const int at_1x = notes[k].basescore();
                CHECK(n.notes + n.accent + n.ghost == at_1x);
                CHECK(n.notes + n.accent + n.ghost + n.combo == at_1x * n.multiplier);
                CHECK(n.multiplier == to_multiplier(n.combo_after));
                ++priced;
            }
        }
    }
    CHECK(priced > 0);
}

// On every multiplier squeeze the engine finds in the corpus (a chord whose
// notes straddle a multiplier step, so its notes pay at different
// multipliers), each note the squeeze-out rule pays outside Star Power
// (sqout_pays_outside) is cut by its basescore at its own multiplier: the
// one the combo reaches as that note is hit, one note at a time. Every other
// note is not cut. Checked under both rules.
TEST_CASE("category_scores: the squeeze-out cut is basescore at each note's multiplier") {
    int squeezes = 0;
    for (const CorpusRecord& r : default_records()) {
        for (const MultSqueeze& ms : r.rec->multsqueezes) {
            CAPTURE(r.chart);
            CAPTURE(ms.combo());
            ++squeezes;
            const Chord& c = ms.chord();
            const std::vector<ChordNote> notes = c.notes(true);
            CHECK(crosses_multiplier_step(ms.combo(), static_cast<int>(notes.size())));
            for (const core::SqOutRule rule :
                 {core::SqOutRule::FirstNote, core::SqOutRule::WholeChord}) {
                CAPTURE(core::sqout_rule_name(rule));
                std::vector<CategoryScores> per_note;
                const CategoryScores cs = category_scores(c, ms.combo(), &per_note, rule);
                REQUIRE(per_note.size() == notes.size());
                int total = 0;
                for (size_t k = 0; k < notes.size(); ++k) {
                    CAPTURE(k);
                    const int combo_once_hit = ms.combo() + static_cast<int>(k) + 1;
                    const int cut = sqout_pays_outside(k, rule)
                                        ? notes[k].basescore() * to_multiplier(combo_once_hit)
                                        : 0;
                    CHECK(per_note[k].sqout_reduction == cut);
                    total += cut;
                }
                CHECK(cs.sqout_reduction == total);
            }
        }
    }
    // "Path pathstring and pathstring_verbose" requires a corpus record with
    // multiplier squeezes, so this loop has some to check.
    CHECK(squeezes > 0);
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

    // The activation's own E answers on real activations are "early-fill
    // window: each corpus activation keeps its search's window"; the
    // SPSqueeze's are "squeeze timing and difficulty follow the offset on
    // every corpus squeeze".
    const double w = kDefaultEarlyFillWindowMs;
    CHECK(is_e0(w - 0.1, 0, w));
    CHECK_FALSE(is_e0(w, 0, w));
    CHECK_FALSE(is_e0(10.0, 1, w));
    CHECK(early_fill_difficulty(-4.0) == 4.0);
    CHECK_FALSE(std::signbit(early_fill_difficulty(0.0)));
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
    // The limit sits on that fill's own figure (70 ms to spare is -70 ms), so
    // the limit itself lets it in and a hair below shuts it out, but only at
    // 85, where it is an E. At 60 it is no E, and no limit applies to it.
    const double fill_ms = early_fill_difficulty(70.0);
    CHECK(fill_within_limit(70.0, 0, fill_ms, kHitWindowMs));
    CHECK_FALSE(fill_within_limit(70.0, 0, fill_ms - 0.1, kHitWindowMs));
    CHECK(fill_within_limit(70.0, 0, fill_ms - 0.1, kDefaultEarlyFillWindowMs));
}

// The 0 ms window, the setting's lowest: a fill spawns only when SP was
// ready by its deadline, and no activation that spawns is an early fill.
// The engine's own activations are checked on the first few corpus charts
// analysed at 0 ms (one more analysis per chart is the slow part).
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
    app::AnalysisSettings none = default_cfg();
    none.early_fill_window_ms = 0.0;
    std::vector<CorpusRecord> none_records;
    for (const CorpusRecord& r : default_records()) {
        if (none_records.size() == 5) break;
        none_records.push_back({r.chart, &corpus::analyzed(r.chart, none)});
    }
    int acts = 0;
    for (const CorpusPath& cp : paths_of(none_records)) {
        for (const Activation& a : cp.path->walk_activations()) {
            CAPTURE(cp.chart);
            CAPTURE(a.timecode.ticks());
            ++acts;
            CHECK(a.e_window_ms == 0.0);
            CHECK_FALSE(a.is_e_critical());
            CHECK_FALSE(a.e_difficulty().has_value());
        }
    }
    // "early-fill window: each corpus activation keeps its search's window"
    // finds activations on these charts at the other windows; a fill whose
    // SP was ready by its deadline still spawns at 0 ms.
    CHECK(acts > 0);
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

// A chord of n notes hit at combo c pays its first note at the multiplier of
// the combo that note makes (c + 1) and its last at the one the last note
// makes (c + n); a chord with no notes keeps the combo's. to_multiplier owns
// where the steps sit (Clone Hero's 10, 20 and 30, model.h; pinned in
// test_timing.cpp). Every combo up to past the last step, every chord size
// Clone Hero allows (D51 call 3).
TEST_CASE("multiplier_step: the multipliers at a chord's two ends") {
    for (int combo = 0; combo < 40; ++combo) {
        for (int notes = 0; notes <= 5; ++notes) {
            CAPTURE(combo);
            CAPTURE(notes);
            const int first = to_multiplier(notes == 0 ? combo : combo + 1);
            const int last = to_multiplier(combo + notes);
            const MultiplierStep step = multiplier_step(combo, notes);
            CHECK(step.first == first);
            CHECK(step.last == last);
        }
        // category_scores, the payout's own copy of the two ends, reads a
        // chord with no notes the same way: the combo's multiplier at both.
        const CategoryScores empty = category_scores(Chord{}, combo);
        CHECK(empty.multiplier == to_multiplier(combo));
        CHECK(empty.multiplier_after == to_multiplier(combo));
        CHECK(empty.combo_after == combo);
    }
    // category_scores reads the same two ends on every multiplier squeeze the
    // engine wrote for the corpus: the chords that straddle a step, where a
    // wrong end would change what the squeeze is worth.
    int squeezes = 0;
    for (const CorpusRecord& r : default_records()) {
        for (const MultSqueeze& ms : r.rec->multsqueezes) {
            CAPTURE(r.chart);
            CAPTURE(ms.combo());
            ++squeezes;
            const int notes = ms.chord().count();
            const CategoryScores cs = category_scores(ms.chord(), ms.combo());
            CHECK(cs.multiplier == to_multiplier(ms.combo() + 1));
            CHECK(cs.multiplier_after == to_multiplier(ms.combo() + notes));
        }
    }
    // "Path pathstring and pathstring_verbose" requires a corpus record with
    // multiplier squeezes, so this loop has some to check.
    CHECK(squeezes > 0);
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

// D51 Q4; D302. Checked through what the screens read: an activation's
// difficulty() (the badges and the path button, through Path::difficulty)
// and a saved figure as the library shows it (shown_timing of
// engine_hardest_ms).
TEST_CASE("Activation::difficulty: squeezes inside and slack fills need none") {
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
    CHECK_FALSE(free_in.hardest().has_value());
    CHECK_FALSE(free_in.difficulty().has_value());

    // A squeeze-in dead on the SP end still needs 0.0 ms, like the
    // squeeze-out on the same note (D302).
    Activation on_end_in = plain();
    on_end_in.sqinouts = {SPSqueeze{SqueezeKind::SqIn, 0.0}};
    CHECK(on_end_in.difficulty() == std::optional<double>(0.0));
    CHECK(on_end_in.hardest() ==
          std::optional<HardestTiming>(HardestTiming{TimingPart::SqueezeIn, 0.0}));

    // A squeeze-out dead on the SP end still has to be hit late.
    Activation on_end_out = plain();
    on_end_out.sqinouts = {SPSqueeze{SqueezeKind::SqOut, 0.0}};
    CHECK(on_end_out.difficulty() == std::optional<double>(0.0));

    // A squeeze-out 12.5 ms past the end has to be hit 12.5 ms late.
    Activation late_out = plain();
    late_out.sqinouts = {SPSqueeze{SqueezeKind::SqOut, -12.5}};
    CHECK(late_out.difficulty() == std::optional<double>(12.5));

    // A required (E0) fill with 10 ms to spare needs none; with none to
    // spare it does, like the squeeze-out on the SP end.
    Activation slack_fill = plain();
    slack_fill.e_offset = 10.0;
    REQUIRE(slack_fill.is_E0());
    CHECK_FALSE(slack_fill.hardest().has_value());
    CHECK_FALSE(slack_fill.difficulty().has_value());
    Activation tight_fill = plain();
    tight_fill.e_offset = 0.0;
    REQUIRE(tight_fill.is_E0());
    CHECK(tight_fill.difficulty() == std::optional<double>(0.0));

    // A path needs timing once any of its activations does: its difficulty
    // and the library's shown figure both stay empty until then.
    Path p;
    p.activations = {free_in, slack_fill};
    CHECK_FALSE(p.difficulty().has_value());
    CHECK_FALSE(shown_timing(engine_hardest_ms(p)).has_value());
    p.activations.push_back(on_end_in);
    CHECK(p.difficulty() == std::optional<double>(0.0));
    CHECK(shown_timing(engine_hardest_ms(p)) == std::optional<double>(0.0));
    Path q;
    q.activations = {free_in, slack_fill, late_out};
    CHECK(q.difficulty() == std::optional<double>(12.5));
    CHECK(shown_timing(engine_hardest_ms(q)) == std::optional<double>(12.5));
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

// A path token is the E mark when the activation is E-critical under its own
// window, then the skip count, then one symbol per squeeze. Checked on every
// corpus activation at the default window, and on the first few corpus
// charts at the widest one, so the mark follows each search's window. The
// mark is one text, the same on every E-critical activation, and absent on
// the rest.
TEST_CASE("Activation notationstr: E prefix, skips, symbols") {
    app::AnalysisSettings wide = default_cfg();
    wide.early_fill_window_ms = kHitWindowMs;
    std::vector<CorpusRecord> wide_records;
    for (const CorpusRecord& r : default_records()) {
        if (wide_records.size() == 5) break;
        wide_records.push_back({r.chart, &corpus::analyzed(r.chart, wide)});
    }
    const std::vector<CorpusRecord>* const runs[] = {&default_records(), &wide_records};
    std::optional<std::string> mark;
    int critical = 0, plain = 0;
    for (const std::vector<CorpusRecord>* records : runs) {
        for (const CorpusPath& cp : paths_of(*records)) {
            for (const Activation& a : cp.path->walk_activations()) {
                CAPTURE(cp.chart);
                CAPTURE(a.e_window_ms);
                CAPTURE(a.timecode.ticks());
                std::string tail = std::to_string(a.skips());
                for (const SPSqueeze& sq : a.sqinouts) tail += sq.symbol();
                const std::string n = a.notationstr();
                CAPTURE(n);
                REQUIRE(ends_with(n, tail));
                const std::string prefix = n.substr(0, n.size() - tail.size());
                if (a.is_e_critical()) {
                    ++critical;
                    CHECK_FALSE(prefix.empty());
                    if (!mark) mark = prefix;
                    CHECK(prefix == *mark);
                } else {
                    ++plain;
                    CHECK(prefix.empty());
                }
            }
        }
    }
    // "early-fill window: each corpus activation keeps its search's window"
    // requires both answers on these same records.
    CHECK(critical > 0);
    CHECK(plain > 0);
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
// before the SP end were already paid in full by the SP walk. The prices are
// each corpus backend row's own (sp_points and sqout_sp_points, as the search
// stored them), and every offset is set from the leeway rule.
TEST_CASE("backend_row_value: every engine case") {
    const double lw = core::default_rules().backend_leeway_ms;
    using P = core::SqOutPosition;
    using core::backend_row_value;
    REQUIRE(lw > 0.0);
    const double inside_sp = -lw;  // before the SP end
    const double in_leeway = lw / 2;  // past the end, inside the leeway
    const double last_counted = std::nextafter(lw, 0.0);
    const double past_leeway = 2 * lw;

    const auto [rows, rows_priced_apart] = for_each_backend_row([&](int sp, int sq) {
        // No squeeze-out: full value inside SP or inside the leeway, else 0.
        CHECK(backend_row_value(inside_sp, sp, sq, P::NoSqOut, lw) == sp);
        CHECK(backend_row_value(0.0, sp, sq, P::NoSqOut, lw) == sp);
        CHECK(backend_row_value(last_counted, sp, sq, P::NoSqOut, lw) == sp);
        CHECK(backend_row_value(lw, sp, sq, P::NoSqOut, lw) == 0);

        // Before the squeezed-out chord: same as no squeeze-out.
        CHECK(backend_row_value(inside_sp, sp, sq, P::Before, lw) == sp);
        CHECK(backend_row_value(in_leeway, sp, sq, P::Before, lw) == sp);
        CHECK(backend_row_value(past_leeway, sp, sq, P::Before, lw) == 0);

        // The squeezed-out chord itself keeps only its reduced value, and
        // only where it would have been counted at all.
        CHECK(backend_row_value(inside_sp, sp, sq, P::Exact, lw) == sq);
        CHECK(backend_row_value(in_leeway, sp, sq, P::Exact, lw) == sq);
        CHECK(backend_row_value(lw, sp, sq, P::Exact, lw) == 0);
        CHECK(backend_row_value(past_leeway, sp, sq, P::Exact, lw) == 0);

        // After the squeezed-out chord nothing is under Star Power.
        CHECK(backend_row_value(inside_sp, sp, sq, P::After, lw) == 0);
        CHECK(backend_row_value(in_leeway, sp, sq, P::After, lw) == 0);

        // The leeway is the user's rule (hydra_rules.ini), not a constant:
        // a row exactly on this leeway counts under a wider one.
        CHECK(backend_row_value(lw, sp, sq, P::NoSqOut, std::nextafter(lw, past_leeway)) == sp);
    });
    // The anchor test above finds SqOut squeezes on the corpus, and set_sqout
    // throws unless such an activation holds a backend row, so rows exist.
    REQUIRE(rows > 0);
    // The Exact cases only tell sqout_sp_points from sp_points on a row where
    // the two differ, so the corpus must hold one: a chord squeezed out loses
    // its first note's doubling.
    REQUIRE(rows_priced_apart > 0);

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
// The leeway and both edges are read from the rules, not typed.
TEST_CASE("backend leeway: just inside the leeway is counted, exactly on it is not") {
    const double lw = core::default_rules().backend_leeway_ms;
    REQUIRE(lw > 0.0);
    const double inside = std::nextafter(lw, 0.0);
    CHECK(core::counted_without_squeeze(inside, lw));
    CHECK_FALSE(core::counted_without_squeeze(lw, lw));

    // The details table labels a plain (not squeezed-out) row by the same
    // rule: the timing tier's word (tier_for on timing_tiers) for what the
    // row needs, and a tag after it only on the row the engine does not count.
    const std::vector<TimingTier> tiers = timing_tiers();
    BackendSqueeze row;
    row.offset_ms = inside;
    const std::string counted_word =
        tier_for(backend_row_timing(inside, false, lw), tiers).first;
    CHECK(row.summarystr(false, std::nullopt, lw) == counted_word);
    row.offset_ms = lw;
    const std::string uncounted_word = tier_for(backend_row_timing(lw, false, lw), tiers).first;
    const std::string uncounted = row.summarystr(false, std::nullopt, lw);
    CHECK(starts_with(uncounted, uncounted_word));
    CHECK(uncounted.size() > uncounted_word.size());
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

TEST_CASE("early_fill_needs_timing: zero slack needs timing, time to spare does not") {
    CHECK(early_fill_needs_timing(0.0));
    CHECK(early_fill_needs_timing(-0.001));
    CHECK_FALSE(early_fill_needs_timing(0.001));
}

// The figures are the two sides of timing_needs_hitting's edge at 0, and
// every figure the engine saves for a corpus path (engine_hardest_ms). A
// figure that needs hitting shows as itself, unrounded; any other shows none.
TEST_CASE("shown_timing: a saved figure shows when it needs hitting") {
    CHECK_FALSE(shown_timing(std::nullopt).has_value());
    std::vector<double> figures = {std::nextafter(0.0, -1.0), 0.0};
    for (const CorpusPath& cp : default_paths())
        if (const std::optional<double> saved = engine_hardest_ms(*cp.path)) figures.push_back(*saved);
    for (double ms : figures) {
        CAPTURE(ms);
        const std::optional<double> shown = shown_timing(ms);
        CHECK(shown.has_value() == timing_needs_hitting(ms));
        if (shown) CHECK(*shown == ms);
    }
    CHECK_FALSE(shown_timing(std::nextafter(0.0, -1.0)).has_value());
    CHECK(shown_timing(0.0).has_value());
}

// On every corpus path the engine wrote: the saved figure is the largest of
// every squeeze's difficulty() and every E0 fill's e_difficulty() over the
// activations the path walks, sign included, and none when there is neither.
// Through shown_timing it shows what Path::difficulty() shows on the live
// path, so a fresh row reads as it did.
TEST_CASE("engine_hardest_ms: every corpus path's largest figure, sign included") {
    int with_figure = 0;
    for (const CorpusPath& cp : default_paths()) {
        const Path& p = *cp.path;
        CAPTURE(cp.chart);
        CAPTURE(p.pathstring());
        std::vector<double> figures;
        for (const Activation& act : p.walk_activations()) {
            for (const SPSqueeze& sq : act.sqinouts) figures.push_back(sq.difficulty());
            if (const std::optional<double> e = act.e_difficulty()) figures.push_back(*e);
        }
        const std::optional<double> saved = engine_hardest_ms(p);
        CHECK(saved.has_value() == !figures.empty());
        if (saved) {
            ++with_figure;
            bool listed = false;
            for (double ms : figures) {
                CHECK(ms <= *saved);
                listed = listed || ms == *saved;
            }
            CHECK(listed);
        }
        CHECK(shown_timing(saved) == p.difficulty());
    }
    // The anchor test above finds SqOut squeezes on the corpus, so some path
    // walks a squeeze and has a figure.
    CHECK(with_figure > 0);
}

// The dropdown indexes kAllDifficulties by the enum's value, so order
// matters. Each name reads back as its own difficulty, and no two share one.
TEST_CASE("difficulty names: one list, one spelling") {
    std::set<std::string> names;
    for (size_t i = 0; i < std::size(kAllDifficulties); ++i) {
        const Difficulty d = kAllDifficulties[i];
        const std::string name = difficulty_name(d);
        CAPTURE(name);
        CHECK(static_cast<size_t>(d) == i);
        CHECK_FALSE(name.empty());
        CHECK(difficulty_from_name(name) == std::optional<Difficulty>(d));
        CHECK(names.insert(name).second);
    }
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

// For every difficulty and both Pro Drums settings: the sentence names the
// difficulty (difficulty_name), and no two of the eight sentences are the
// same, so the drum mode changes it too.
TEST_CASE("no_notes_message names the difficulty and the drum mode") {
    std::set<std::string> sentences;
    for (Difficulty d : kAllDifficulties) {
        for (bool pro : {true, false}) {
            const std::string msg = no_notes_message(d, pro);
            CAPTURE(msg);
            CHECK(msg.find(difficulty_name(d)) != std::string::npos);
            CHECK(sentences.insert(msg).second);
        }
    }
}

// The missing part read back from every one of the eight sentences: it sits
// inside the sentence, names the difficulty, and no two are the same. A
// sentence with the part cut out, another message (app::kSomethingWentWrong)
// and an empty text read as none.
TEST_CASE("no_notes_part reads the missing part back out of no_notes_message's sentence") {
    std::set<std::string> parts;
    for (Difficulty d : kAllDifficulties) {
        for (bool pro : {true, false}) {
            const std::string msg = no_notes_message(d, pro);
            CAPTURE(msg);
            const std::optional<std::string> part = no_notes_part(msg);
            REQUIRE(part.has_value());
            const size_t at = msg.find(*part);
            REQUIRE(at != std::string::npos);
            CHECK(part->find(difficulty_name(d)) != std::string::npos);
            CHECK(parts.insert(*part).second);
            // The same sentence with nothing between its two ends.
            const std::string bare = msg.substr(0, at) + msg.substr(at + part->size());
            CHECK_FALSE(no_notes_part(bare).has_value());
        }
    }
    CHECK_FALSE(no_notes_part(app::kSomethingWentWrong).has_value());
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
// get them right). A chord straddles a step when the payout (category_scores,
// which owns each note's multiplier) pays its first note below its last;
// multiplier_step's own test pins those ends to to_multiplier's steps
// (Clone Hero's 10, 20 and 30, model.h). The graph asks applies(), and the
// constructor refuses what applies() refuses; both are checked against that
// rule, for every combo up to past the last step.
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
    int straddles = 0;
    for (int n = 2; n <= 5; ++n)
        for (int combo = 0; combo < 40; ++combo) {
            CAPTURE(n);
            CAPTURE(combo);
            const Chord mixed = chord_of(n, true);
            const CategoryScores paid = category_scores(mixed, combo);
            const bool straddle = paid.multiplier < paid.multiplier_after;
            if (straddle) ++straddles;
            CHECK(MultSqueeze::applies(mixed, combo) == straddle);
            CHECK(accepted(mixed, combo) == straddle);
            const Chord same = chord_of(n, false);
            CHECK_FALSE(MultSqueeze::applies(same, combo));
            CHECK_FALSE(accepted(same, combo));
        }
    // A straddling chord must turn up, or only the refusals were checked.
    CHECK(straddles > 0);
}

// The best order puts the cymbals past the step and the pads before it; the
// worst order swaps them. Each cymbal that the best order moves past the step
// is worth its cymbal bonus more per multiplier gained, so the gain is that
// count times kCymbalBonusPoints times the step's rise. Each note's
// multiplier is read from the payout (category_scores): the notes paid past
// the step are those paid above the first note's multiplier.
TEST_CASE("MultSqueeze::gain is the best order minus the worst order") {
    auto paid_past_step = [](const Chord& chord, int combo) {
        std::vector<CategoryScores> per_note;
        category_scores(chord, combo, &per_note);
        int past = 0;
        for (const CategoryScores& note : per_note)
            if (note.multiplier > per_note.front().multiplier) ++past;
        return past;
    };
    auto rise = [](const Chord& chord, int combo) {
        const CategoryScores cs = category_scores(chord, combo);
        return cs.multiplier_after - cs.multiplier;
    };

    // Kick and Red pads with yellow and blue cymbals, hit at combo 7: the
    // last two notes are paid past the step, and both cymbals fit there.
    Chord kryb;
    kryb.add_note(NoteColor::Kick);
    kryb.add_note(NoteColor::Red);
    kryb.add_note(NoteColor::Yellow);
    kryb.add_note(NoteColor::Blue);
    kryb.apply_cymbal(NoteColor::Yellow);
    kryb.apply_cymbal(NoteColor::Blue);
    const int kryb_cymbals = 2;
    REQUIRE(paid_past_step(kryb, 7) == kryb_cymbals);
    CHECK(MultSqueeze(kryb, 7).gain() == kryb_cymbals * kCymbalBonusPoints * rise(kryb, 7));

    // A red pad and a yellow cymbal at combo 8: only the last note is paid
    // past the step, and the one cymbal goes there.
    Chord ry;
    ry.add_note(NoteColor::Red);
    ry.add_note(NoteColor::Yellow);
    ry.apply_cymbal(NoteColor::Yellow);
    const int ry_cymbals = 1;
    REQUIRE(paid_past_step(ry, 8) == ry_cymbals);
    CHECK(MultSqueeze(ry, 8).gain() == ry_cymbals * kCymbalBonusPoints * rise(ry, 8));
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
    // Each note is named as its own one-note chord (Chord::rowstr), the way
    // the advice names it, so no advice text is typed here.
    auto named = [](const Chord& chord, NoteColor c) {
        Chord one;
        one.insert_note(*chord.at(c));
        return one.rowstr();
    };
    const MultSqueeze ms(kryb, 7);
    const std::string advice = ms.howto();
    CAPTURE(advice);
    CHECK(advice.find(named(kryb, NoteColor::Yellow)) != std::string::npos);
    CHECK(advice.find(named(kryb, NoteColor::Blue)) != std::string::npos);
    CHECK(advice.find(named(kryb, NoteColor::Kick)) == std::string::npos);
    CHECK(advice.find(named(kryb, NoteColor::Red)) == std::string::npos);
    CHECK(ms.multiplier() == 2);

    // Accent the Red (100): now the two cymbals tie across the step, so
    // either may cross. The kick must stay before the step and the accented
    // Red must cross it, so the advice names the kick first and the Red
    // after it, and neither cymbal.
    kryb.apply_accent(NoteColor::Red);
    const std::string accented = MultSqueeze(kryb, 7).howto();
    CAPTURE(accented);
    const size_t kick_at = accented.find(named(kryb, NoteColor::Kick));
    const size_t red_at = accented.find(named(kryb, NoteColor::Red));
    REQUIRE(kick_at != std::string::npos);
    REQUIRE(red_at != std::string::npos);
    CHECK(kick_at < red_at);
    CHECK(accented.find(named(kryb, NoteColor::Yellow)) == std::string::npos);
    CHECK(accented.find(named(kryb, NoteColor::Blue)) == std::string::npos);
}

// One note of `chord` as the advice names it: its own one-note chord
// (Chord::rowstr) in the Pro Drums setting's words.
static std::string named_note(const Chord& chord, NoteColor c, bool pro) {
    Chord one;
    one.insert_note(*chord.at(c));
    return one.rowstr(pro);
}

// Whether `advice` names exactly the one note `lone` of `chord`'s lanes in
// `lanes`, and none of the others.
static bool names_only(const std::string& advice, const Chord& chord,
                       const std::vector<NoteColor>& lanes, NoteColor lone) {
    for (const NoteColor c : lanes) {
        const bool named = advice.find(named_note(chord, c, true)) != std::string::npos;
        if (named != (c == lone)) return false;
    }
    return true;
}

// A 3-note chord splits two and one across the step. When one end holds a
// single note, the advice names that note whichever side of the step the
// chord falls on; only three different values need direction-specific advice.
// Which end the note must take is MultSqueeze::direction: a lone cheap note
// is hit first and a lone dear one last, so the two read opposite ways.
TEST_CASE("MultSqueeze::howto names the lone note of a 3-note chord") {
    // Kick (50) + two cymbals (65): the kick is the lone cheap note.
    Chord kyb;
    kyb.add_note(NoteColor::Kick);
    kyb.add_note(NoteColor::Yellow);
    kyb.add_note(NoteColor::Blue);
    kyb.apply_cymbal(NoteColor::Yellow);
    kyb.apply_cymbal(NoteColor::Blue);
    const std::vector<NoteColor> kyb_lanes = {NoteColor::Kick, NoteColor::Yellow,
                                              NoteColor::Blue};
    const std::string cheap_end = MultSqueeze(kyb, 17).direction();
    for (int combo : {17, 18, 27, 28}) {
        CAPTURE(combo);
        const MultSqueeze ms(kyb, combo);
        CAPTURE(ms.howto());
        CHECK(names_only(ms.howto(), kyb, kyb_lanes, NoteColor::Kick));
        CHECK(ms.direction() == cheap_end);
    }

    // Kick + Red (50) + one cymbal (65): the cymbal is the lone dear note.
    Chord kry;
    kry.add_note(NoteColor::Kick);
    kry.add_note(NoteColor::Red);
    kry.add_note(NoteColor::Yellow);
    kry.apply_cymbal(NoteColor::Yellow);
    const std::vector<NoteColor> kry_lanes = {NoteColor::Kick, NoteColor::Red,
                                              NoteColor::Yellow};
    const std::string dear_end = MultSqueeze(kry, 17).direction();
    CHECK(dear_end != cheap_end);
    for (int combo : {17, 18, 27, 28}) {
        CAPTURE(combo);
        const MultSqueeze ms(kry, combo);
        CAPTURE(ms.howto());
        CHECK(names_only(ms.howto(), kry, kry_lanes, NoteColor::Yellow));
        CHECK(ms.direction() == dear_end);
    }

    // Kick (50) + cymbal (65) + accented Red (100): three values, so the
    // direction decides. Combo 17 carries one note over, the dear Red; 18
    // leaves one behind, the cheap kick.
    Chord kya;
    kya.add_note(NoteColor::Kick);
    kya.add_note(NoteColor::Red);
    kya.add_note(NoteColor::Yellow);
    kya.apply_cymbal(NoteColor::Yellow);
    kya.apply_accent(NoteColor::Red);
    const std::vector<NoteColor> kya_lanes = kry_lanes;
    const MultSqueeze over(kya, 17);
    const MultSqueeze behind(kya, 18);
    CAPTURE(over.howto());
    CAPTURE(behind.howto());
    CHECK(names_only(over.howto(), kya, kya_lanes, NoteColor::Red));
    CHECK(over.direction() == dear_end);
    CHECK(names_only(behind.howto(), kya, kya_lanes, NoteColor::Kick));
    CHECK(behind.direction() == cheap_end);
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
    // Each setting's advice names the note in its own words and not in the
    // other setting's, and the two words differ for a pad.
    auto names_in_own_words = [](const MultSqueeze& ms, const Chord& chord, NoteColor c) {
        const std::string pro_name = named_note(chord, c, true);
        const std::string plain_name = named_note(chord, c, false);
        CAPTURE(ms.howto(true));
        CAPTURE(ms.howto(false));
        CHECK(pro_name != plain_name);
        CHECK(ms.howto(true).find(pro_name) != std::string::npos);
        CHECK(ms.howto(false).find(plain_name) != std::string::npos);
        CHECK(ms.howto(false).find(pro_name) == std::string::npos);
        CHECK(ms.howto(true).find(plain_name) == std::string::npos);
    };
    names_in_own_words(MultSqueeze(kyb, 17), kyb, NoteColor::Yellow);

    // Kick (50) + accented red (100) + yellow cymbal (65): combo 17 carries
    // the dear red over.
    Chord kya;
    kya.add_note(NoteColor::Kick);
    kya.add_note(NoteColor::Red);
    kya.add_note(NoteColor::Yellow);
    kya.apply_cymbal(NoteColor::Yellow);
    kya.apply_accent(NoteColor::Red);
    names_in_own_words(MultSqueeze(kya, 17), kya, NoteColor::Red);
    // A cymbal says cymbal either way.
    Chord kry;
    kry.add_note(NoteColor::Kick);
    kry.add_note(NoteColor::Red);
    kry.add_note(NoteColor::Yellow);
    kry.apply_cymbal(NoteColor::Yellow);
    const MultSqueeze cymbal(kry, 17);
    const std::string cymbal_name = named_note(kry, NoteColor::Yellow, true);
    CHECK(named_note(kry, NoteColor::Yellow, false) == cymbal_name);
    CHECK(cymbal.howto(false) == cymbal.howto(true));
    CHECK(cymbal.howto(false).find(cymbal_name) != std::string::npos);
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

// On every path the engine writes for the corpus. A window's Star Power ends
// on its squeezed-out chord when core::after_sp_end says the chord is not
// after the stored end (deact_tick), and on the stored end otherwise; the
// stored end itself is never moved. The bars an activation spends arrived
// where the search stored them (bank_rise_ticks, or trailing_bank_ticks
// after the last activation), except that after a squeezed-out activation
// the first one arrives on its chord.
TEST_CASE("Bank arrivals and window_end_tick: a squeezed-out chord ends the window it is not after") {
    int squeezed_out = 0, plain = 0, moved = 0;
    for (const CorpusPath& cp : default_paths()) {
        const Path& p = *cp.path;
        CAPTURE(cp.chart);
        CAPTURE(p.pathstring());
        const ActivationWalk acts = p.walk_activations();
        for (size_t i = 0; i <= acts.size(); ++i) {
            CAPTURE(i);
            if (i < acts.size()) {
                const Activation& a = acts[i];
                const std::optional<int64_t> end = a.deact_tick();
                REQUIRE(end.has_value());
                if (a.sqout_tick && !core::after_sp_end(*a.sqout_tick, *end)) {
                    ++squeezed_out;
                    CHECK(a.window_end_tick() == a.sqout_tick);
                } else {
                    ++plain;
                    CHECK(a.window_end_tick() == end);
                }
            }
            const std::vector<int64_t>& stored =
                i == acts.size() ? p.trailing_bank_ticks : acts[i].bank_rise_ticks;
            const std::vector<int64_t> arrivals = p.bank_arrivals(i);
            REQUIRE(arrivals.size() == stored.size());
            if (stored.empty()) continue;
            for (size_t j = 1; j < stored.size(); ++j) CHECK(arrivals[j] == stored[j]);
            if (i > 0 && acts[i - 1].sqout_tick) {
                ++moved;
                CHECK(arrivals.front() == *acts[i - 1].sqout_tick);
            } else {
                CHECK(arrivals.front() == stored.front());
            }
        }
    }
    // The corpus holds SqOuts ("squeeze timing and difficulty follow the
    // offset on every corpus squeeze"), each stamped by set_sqout with its
    // chord. The search picks a squeezed-out chord only where it is not
    // after the stored end (core/sqout_chord.h), and the chord ends an SP
    // phrase whose bar the next list holds, so all three cases turn up.
    CHECK(squeezed_out > 0);
    CHECK(plain > 0);
    CHECK(moved > 0);
}

// The meter's level on the record (Activation::meter_bars_at), on every path
// the engine writes for the corpus. It is never more than the bars the
// activation spent at its note (the stored end drops a fraction of a tick,
// D15, so it may sit just below). A step never shortens Star Power, so the
// level just after a step's tick is never below the level just before it.
// The meter is empty on the last stored step's end. On an early squeeze-out
// whose chord is before that end, SP ends on the chord with the meter above
// empty (SP-25 drops the rest). A hand-built activation with no step has no
// level.
TEST_CASE("Activation::meter_bars_at: the level the record gives inside each window") {
    const app::AnalysisSettings& cfg = default_cfg();
    CHECK_FALSE(Activation{}.meter_bars_at(0, corpus::song(default_paths().front().chart,
                                                           cfg.reading(), cfg.rules)
                                                  .timing())
                    .has_value());
    int windows = 0, steps = 0, early_out = 0;
    for (const CorpusPath& cp : default_paths()) {
        CAPTURE(cp.chart);
        CAPTURE(cp.path->pathstring());
        const SongTiming& timing = corpus::song(cp.chart, cfg.reading(), cfg.rules).timing();
        for (const Activation& a : cp.path->walk_activations()) {
            const int64_t act = a.timecode.ticks();
            CAPTURE(act);
            const std::optional<int64_t> deact = a.deact_tick();
            const std::optional<int64_t> end = a.window_end_tick();
            REQUIRE(deact.has_value());
            REQUIRE(end.has_value());
            ++windows;
            CHECK(*a.meter_bars_at(act, timing) <= a.sp_meter() + 1e-9);
            for (size_t k = 1; k < a.sp_end_steps.size(); ++k) {
                const int64_t t = a.sp_end_steps[k].tick;
                CAPTURE(t);
                ++steps;
                CHECK(*a.meter_bars_at(t, timing) >= *a.meter_bars_at(t, timing, true));
            }
            CHECK(*a.meter_bars_at(*deact, timing, true) == 0.0);
            if (*end < *deact) {
                ++early_out;
                CHECK(*a.meter_bars_at(*end, timing, true) > 0.0);
            }
        }
    }
    CHECK(windows > 0);
    CHECK(steps > 0);
    CHECK(early_out > 0);
}

// What squeezing a row out costs, on every corpus backend row at its own
// prices (sp_points and sqout_sp_points, as the search stored them), with
// offsets set from the leeway rule: where the row would be counted it keeps
// its reduced value and loses the rest, and where it would not be counted it
// loses all of it.
TEST_CASE("backend_value: sqout_cost is what the squeezed-out chord loses") {
    const double lw = core::default_rules().backend_leeway_ms;
    REQUIRE(lw > 0.0);
    const double inside_sp = -lw;
    const double in_leeway = lw / 2;
    const double past_leeway = 2 * lw;
    const auto [rows, rows_priced_apart] = for_each_backend_row([&](int sp, int sq) {
        CHECK(core::sqout_cost(inside_sp, sp, sq, lw) == sp - sq);
        CHECK(core::sqout_cost(0.0, sp, sq, lw) == sp - sq);
        CHECK(core::sqout_cost(in_leeway, sp, sq, lw) == sp - sq);
        CHECK(core::sqout_cost(lw, sp, sq, lw) == sp);
        CHECK(core::sqout_cost(past_leeway, sp, sq, lw) == sp);
        // The leeway is the user's rule, not a constant: a row past this
        // leeway is counted under a wider one.
        CHECK(core::sqout_cost(lw, sp, sq, past_leeway) == sp - sq);
    });
    // "backend_row_value: every engine case" requires both on the corpus.
    REQUIRE(rows > 0);
    REQUIRE(rows_priced_apart > 0);
}

// What the code does with a row's offset: set_sqout builds each SqOut from
// its row's offset, which every corpus SqOut shows. A row with no offset (a
// hand-built or corrupt record; the engine always stores one) fails loudly,
// naming its tick, and leaves the activation with no squeeze-out.
TEST_CASE("BackendSqueeze::offset: the stored offset, or a loud failure") {
    const Activation* first = nullptr;
    int sqouts = 0;
    for (const CorpusAct& ca : default_acts()) {
        const Activation& a = *ca.act;
        if (!a.sqout_tick) continue;
        CAPTURE(ca.chart);
        CAPTURE(*a.sqout_tick);
        ++sqouts;
        if (!first) first = &a;
        const BackendSqueeze* row = a.sqout_row();
        REQUIRE(row != nullptr);
        int found = 0;
        for (const SPSqueeze& sq : a.sqinouts) {
            if (sq.kind != SqueezeKind::SqOut) continue;
            ++found;
            CHECK(sq.offset_ms == row->offset());
        }
        CHECK(found == 1);
    }
    // "squeeze timing and difficulty follow the offset on every corpus
    // squeeze" requires a SqOut, and set_sqout stamps each one's chord.
    REQUIRE(sqouts > 0);
    REQUIRE(first != nullptr);

    // The same activation as the engine wrote it, before its squeeze-out,
    // with the offset gone from the squeezed-out row.
    const int64_t tick = *first->sqout_tick;
    Activation unsqueezed = *first;
    unsqueezed.sqout_tick.reset();
    unsqueezed.sqinouts.erase(
        std::remove_if(unsqueezed.sqinouts.begin(), unsqueezed.sqinouts.end(),
                       [](const SPSqueeze& sq) { return sq.kind == SqueezeKind::SqOut; }),
        unsqueezed.sqinouts.end());
    int emptied = 0;
    for (BackendSqueeze& b : unsqueezed.backends) {
        if (b.timecode.ticks() != tick) continue;
        b.offset_ms.reset();
        ++emptied;
    }
    REQUIRE(emptied == 1);
    bool threw = false;
    try {
        unsqueezed.set_sqout(tick);
    } catch (const std::logic_error& e) {
        threw = true;
        CHECK(std::string(e.what()).find(std::to_string(tick)) != std::string::npos);
    }
    CHECK(threw);
    CHECK_FALSE(unsqueezed.sqout_tick.has_value());
}

// Every lane's one flag: the kick's is 2x and a cymbal pad's (allows_cymbals)
// is a cymbal, so a lane has a flag exactly when it is the kick or a cymbal
// pad. set_lane_flag sets that flag and nothing else, lane_flag reads it
// back, and a lane with no flag refuses it loudly.
TEST_CASE("lane flags: the kick carries 2x, the cymbal pads carry a cymbal, red carries nothing") {
    const NoteColor lanes[] = {NoteColor::Kick, NoteColor::Red, NoteColor::Yellow,
                               NoteColor::Blue, NoteColor::Green};
    int flagged = 0, unflagged = 0;
    for (const NoteColor c : lanes) {
        CAPTURE(color_str(c));
        const bool kick = c == NoteColor::Kick;
        CHECK(lane_allows_flag(c) == (kick || allows_cymbals(c)));
        ChordNote note{c};
        CHECK_FALSE(lane_flag(note));
        if (!lane_allows_flag(c)) {
            ++unflagged;
            CHECK_THROWS_AS(set_lane_flag(note), std::logic_error);
            continue;
        }
        ++flagged;
        set_lane_flag(note);
        CHECK(lane_flag(note));
        CHECK(note.is2x == kick);
        CHECK(note.is_cymbal() == !kick);
    }
    CHECK(flagged > 0);
    CHECK(unflagged > 0);
}

// The display word for a dynamic is dynamic_str's word with its first letter
// capitalised, and a plain note has no word on screen. The two dynamics'
// words differ, and the stored word for a plain note differs from both.
TEST_CASE("dynamic_label: the display word for each dynamic") {
    for (const NoteDynamicType dyn : {NoteDynamicType::Ghost, NoteDynamicType::Accent}) {
        const std::string label = dynamic_label(dyn);
        const std::string word = dynamic_str(dyn);
        CAPTURE(word);
        CAPTURE(label);
        REQUIRE_FALSE(label.empty());
        CHECK(to_lower_ascii(label) == word);
        CHECK(label.substr(1) == word.substr(1));
        CHECK(label[0] != lower_ascii(label[0]));
        CHECK(word != dynamic_str(NoteDynamicType::Normal));
    }
    CHECK(dynamic_label(NoteDynamicType::Normal).empty());
    CHECK_FALSE(dynamic_str(NoteDynamicType::Normal).empty());
    CHECK(dynamic_str(NoteDynamicType::Ghost) != dynamic_str(NoteDynamicType::Accent));
}

// squeeze_kind_from_name reads back exactly the words type_name writes: each
// kind's own word gives that kind, and the word in another case, cut short,
// run on, or empty gives nothing.
TEST_CASE("squeeze_kind_from_name: type_name's two words and nothing else") {
    for (SqueezeKind kind : {SqueezeKind::SqIn, SqueezeKind::SqOut}) {
        const std::string name = SPSqueeze{kind, 0.0}.type_name();
        CAPTURE(name);
        REQUIRE(name.size() > 1);
        CHECK(squeeze_kind_from_name(name) == std::optional<SqueezeKind>(kind));
        const std::string lower = to_lower_ascii(name);
        REQUIRE(lower != name);
        CHECK_FALSE(squeeze_kind_from_name(lower).has_value());
        CHECK_FALSE(squeeze_kind_from_name(name.substr(0, name.size() - 1)).has_value());
        CHECK_FALSE(squeeze_kind_from_name(name + name).has_value());
    }
    CHECK(std::string(SPSqueeze{SqueezeKind::SqIn, 0.0}.type_name()) !=
          std::string(SPSqueeze{SqueezeKind::SqOut, 0.0}.type_name()));
    CHECK_FALSE(squeeze_kind_from_name("").has_value());
}

// The same six values "replay score fields: each name reads its own member"
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
