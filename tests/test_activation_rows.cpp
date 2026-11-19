// Tests for app/path_view's activation rows on the Song Details screen:
// build_activations, the badges, the squeeze sentences and the backend table.
// The view-model is the interface, so the cases here pin the exact display
// strings. The path list is in test_path_list.cpp.
//
// A1 to A9 in the comments are acceptance cases from the plan (A3 since changed
// by D261: a fill with time to spare gets a grey badge, where the plan said none)
// docs/superpowers/plans/2026-10-22-squeeze-badge-beside-early-fill.md (removed; git show 444631e6:docs/superpowers/plans/2026-10-22-squeeze-badge-beside-early-fill.md).

#include "doctest.h"

#include <cmath>
#include <cstdlib>
#include <iterator>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/display_format.h"
#include "app/path_view.h"
#include "core/note_window.h"
#include "core/squeeze_rating.h"
#include "analyzed_chart.h"
#include "record_fixtures.h"
#include "song_details_fixtures.h"

using namespace hydra;
using namespace hydra::app;
using song_details::burnout;
using song_details::kDot;

namespace {

// D269: inside SP or on the end, the squeezed-out row's word is past the
// first tier exactly when its badge is orange (SPSqueeze::is_difficult). Past
// the end the badge needs no timing, while the word is banded on the row's
// distance from the SP end, so there the word is that offset's tier. The
// scale is reset to x1.00 first, so the raw and the eff. figure are one
// figure; the badge reads raw ms (D269 call 5).
void check_word_agrees_with_badge(Activation act) {
    test::set_transfer(act, TransferScale{});
    const SPSqueeze* sqout = nullptr;
    for (const SPSqueeze& sq : act.sqinouts)
        if (sq.kind == SqueezeKind::SqOut) sqout = &sq;
    REQUIRE(sqout != nullptr);
    const std::string first_tier = timing_tiers().front().name;
    int rows = 0;
    for (const BackendRating& br : rate_activation(act).backends) {
        if (!br.squeezed_out) continue;
        ++rows;
        CHECK_FALSE(br.note.effective_ms.has_value());
        const std::string word = br.row.summarystr(true, br.note.effective_ms);
        CAPTURE(word);
        const double off = br.row.offset();
        if (core::paid_by_sp_walk(off)) {
            const bool past_first_tier = word.rfind(first_tier, 0) != 0;
            CHECK(sqout->is_difficult() == past_first_tier);
        } else if (core::counted_without_squeeze(off, core::default_rules().backend_leeway_ms)) {
            CHECK(word.rfind(first_tier, 0) == 0);
        } else {
            CHECK(word.rfind(tier_for(off, timing_tiers()).first, 0) == 0);
        }
    }
    CHECK(rows == 1);
}

}  // namespace

TEST_CASE("display format: the average multiplier rounds the exact double") {
    // Rounds, not truncates: a slice of 2.3456 would give 2.345.
    CHECK(format_avg_mult(2.3456) == "2.346");

    // The rounding reads the stored binary value, not the decimal literal.
    // `stored - whole` is exact (the two are within a factor of 2), and fma
    // rounds once, so its sign is the exact sign of (stored - halfway).
    const double above_half = 2.0005;
    const double below_half = 1.0005;
    REQUIRE(std::fma(above_half - 2.0, 10000.0, -5.0) > 0.0);
    REQUIRE(std::fma(below_half - 1.0, 10000.0, -5.0) < 0.0);
    CHECK(format_avg_mult(above_half) == "2.001");
    CHECK(format_avg_mult(below_half) == "1.000");
}

TEST_CASE("display format: ms text is one decimal, a space and the unit") {
    // D57 item 1: every one-decimal ms figure reads "163.5 ms".
    CHECK(format_ms(12.3) == "12.3 ms");
    CHECK(format_ms(-20.0) == "-20.0 ms");
    CHECK(format_ms(0.0) == "0.0 ms");
    // A whole figure keeps its decimal.
    CHECK(format_ms(163.0) == "163.0 ms");
    // A figure with more digits rounds to one decimal.
    CHECK(format_ms(12.36) == "12.4 ms");
}

TEST_CASE("build_activations: the early fill reads positive = early on both lines") {
    HydraRecord rec;  // only feeds the footer
    auto view_of = [&rec](const Activation& act) {
        Path p;
        p.activations.push_back(act);
        ActivationsView v = build_activations(p, rec, nullptr);
        REQUIRE(v.acts.size() == 1);
        return v.acts[0];
    };

    // E0: 12.3 ms early, a raw offset of -12.3. The badge and the details line
    // both read 12.3.
    Activation e0;
    test::set_skips(e0, 0);
    test::set_sp_meter(e0, 2);
    e0.e_offset = -12.3;
    ActivationRowView av = view_of(e0);
    // The fixture sets no timecode, so it sits at tick 0 (m1.1.0); the search
    // sets one on every activation it makes (docs/adr/0017).
    CHECK(av.notation == "E0");
    CHECK(av.measure == "m1.1.0");
    REQUIRE(av.badges.size() == 1);
    CHECK(av.badges[0].text == "early fill 12.3 ms");
    CHECK(av.early_fill == "Early fill: 12.3 ms (required)");

    // E-critical but not E0, with SP ready 20 ms before the deadline: the
    // details line and the badge read it negative (D261). The fill spawns
    // however the activation is hit, so there is nothing to time (D51 call
    // 4). Optional, so never warn-coloured.
    Activation e1;
    test::set_skips(e1, 1);
    test::set_sp_meter(e1, 2);
    e1.e_offset = 20.0;
    av = view_of(e1);
    CHECK(av.notation == "E1");
    REQUIRE(av.badges.size() == 1);
    CHECK(av.badges[0].text == "early fill -20.0 ms");
    CHECK_FALSE(av.badges[0].warn);
    CHECK_FALSE(av.difficult);
    CHECK(av.early_fill == "Early fill: -20.0 ms (optional)");
}

TEST_CASE("build_activations: rows and backend rows line up") {
    const AnalysisResult& ar = corpus::analyzed_by_ten_scores();
    const Path& best = ar.record.best_path();
    const SongTiming& timing = ar.song.timing();

    ActivationsView view = build_activations(best, ar.record, &timing);
    CHECK(view.acts.size() == best.all_activations().size());

    std::vector<Activation> acts = best.all_activations();
    for (size_t i = 0; i < view.acts.size(); ++i) {
        const ActivationRowView& av = view.acts[i];
        CHECK(av.notation == acts[i].notationstr());
        CHECK(av.difficult == acts[i].is_difficult());
        CHECK(av.squeeze_sentences.size() == acts[i].sqinouts.size());
        CHECK(av.backends.size() == acts[i].display_backends().size());
    }

    CHECK(!view.summary.empty());
}

namespace {

// Calls visit(chart, song, record) for every corpus chart that analyzes under
// the app's default settings, with the chart captured, and requires at least
// one such chart.
template <typename Visit>
void for_each_corpus_chart(Visit&& visit) {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int charts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (song.is_empty()) continue;
        const HydraRecord* rec = nullptr;
        try {
            rec = &corpus::analyzed(chart, cfg);
        } catch (const std::exception&) {
            continue;
        }
        ++charts;
        visit(chart, song, *rec);
    }
    REQUIRE(charts > 0);
}

// Calls visit(chart, song, record, path) for every kept path of every corpus
// chart that analyzes under the app's default settings.
template <typename Visit>
void for_each_corpus_path(Visit&& visit) {
    for_each_corpus_chart([&](const std::string& chart, const Song& song, const HydraRecord& rec) {
        for (const Path& path : rec.paths) visit(chart, song, rec, path);
    });
}

// Calls visit(act, row) for every activation of every kept corpus path, with
// row the one build_activations makes for act under the song's timing.
template <typename Visit>
void for_each_corpus_row(Visit&& visit) {
    for_each_corpus_path([&](const std::string& chart, const Song& song, const HydraRecord& rec,
                             const Path& path) {
        CAPTURE(chart);
        CAPTURE(path.pathstring());
        const SongTiming& timing = song.timing();
        const std::vector<Activation> acts = path.all_activations();
        const ActivationsView view = build_activations(path, rec, &timing);
        REQUIRE(view.acts.size() == acts.size());
        for (size_t i = 0; i < acts.size(); ++i) {
            CAPTURE(i);
            visit(acts[i], view.acts[i]);
        }
    });
}

// Pitch Dark (testdata/input/test_pitch_dark_ratio) under the app's default
// settings, with its path "3+ 2+ 1". That path's first activation has a
// clamp and then a squeeze-in among its SP end steps (the Pitch Dark case in
// test_sp_meter_curve.cpp requires them).
struct PitchDarkPath {
    const Song* song = nullptr;
    const HydraRecord* rec = nullptr;
    const Path* path = nullptr;
};
PitchDarkPath pitch_dark_path() {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    const std::string chart = corpus::chart_named("test_pitch_dark_ratio");
    REQUIRE_FALSE(chart.empty());
    PitchDarkPath pd;
    pd.song = &corpus::song(chart, cfg.reading(), cfg.rules);
    pd.rec = &corpus::analyzed(chart, cfg);
    pd.path = test::path_named(pd.rec->all_paths(), "3+ 2+ 1");
    REQUIRE(pd.path != nullptr);
    return pd;
}

// Whether a scale line names `side` ("early" or "late").
bool names_side(const std::string& line, const std::string& side) {
    return line.find("(" + side + ")") != std::string::npos;
}

// The one row build_activations makes for `act`, with no timing. The record
// argument is unused by build_activations.
ActivationRowView scale_row_of(const Activation& act) {
    HydraRecord rec;
    Path p;
    p.activations.push_back(act);
    ActivationsView v = build_activations(p, rec, nullptr);
    REQUIRE(v.acts.size() == 1);
    return v.acts[0];
}

// A plain activation with no early fill to time, for the scale-line wording
// cases below.
Activation scale_base() {
    Activation base;
    test::set_skips(base, 0);
    base.e_offset = 300.0;  // not e-critical
    return base;
}

// A backend row `offset_ms` from the SP end (negative = inside SP).
BackendSqueeze backend_at(double offset_ms) {
    BackendSqueeze row;
    row.offset_ms = offset_ms;
    return row;
}

}  // namespace

// Which sides a row or squeeze reads is rate_activation's (SidesRead). On
// every corpus activation the SP end's line names a side exactly when that
// side was read and its stored multiplier is scaled, early before late, and
// the line is orange exactly when rate_activation says a multiplier governs.
TEST_CASE("build_activations: on every corpus activation the scale line names the sides rate_activation read") {
    int scaled_lines = 0;
    int both_sides = 0;
    for_each_corpus_row([&](const Activation& act, const ActivationRowView& av) {
        CAPTURE(av.scale_warning);
        const ActivationRating rate = rate_activation(act);
        // A fresh record always stores its scales (D4).
        REQUIRE(rate.scales.has_value());
        const bool early = rate.post_read.early && is_scaled(rate.scales->post.early);
        const bool late = rate.post_read.late && is_scaled(rate.scales->post.late);
        CHECK(names_side(av.scale_warning, "early") == early);
        CHECK(names_side(av.scale_warning, "late") == late);
        CHECK(av.scale_warning.empty() == !(early || late));
        if (early && late) {
            CHECK(av.scale_warning.find("(early)") < av.scale_warning.find("(late)"));
            ++both_sides;
        }
        if (early || late) ++scaled_lines;

        // A SqIn line needs a SqIn that read a scaled side. rate_activation
        // pairs sqins_read with scales->sqins entry for entry.
        REQUIRE(rate.sqins_read.size() == rate.scales->sqins.size());
        bool sqin_scaled = false;
        auto read = rate.sqins_read.begin();
        for (const TransferScale& s : rate.scales->sqins) {
            sqin_scaled = sqin_scaled || (read->early && is_scaled(s.early)) ||
                          (read->late && is_scaled(s.late));
            ++read;
        }
        if (!sqin_scaled) CHECK(av.sqin_scale_warning.empty());
        CHECK(av.scale_warn == rate.scale_governs);
    });
    // Pitch Dark's first activation (the next case) has a line with both.
    CHECK(scaled_lines > 0);
    CHECK(both_sides > 0);
}

// Pitch Dark's first activation: its backend rows sit on both sides of the
// SP end, and its squeeze-in reads the late side of its own end's scale.
TEST_CASE("build_activations: Pitch Dark's first scale line names both sides and the SqIn's end") {
    const PitchDarkPath pd = pitch_dark_path();
    const SongTiming& timing = pd.song->timing();
    const std::vector<Activation> acts = pd.path->all_activations();
    REQUIRE_FALSE(acts.empty());
    const ActivationRating rate = rate_activation(acts[0]);
    REQUIRE(rate.scales.has_value());
    REQUIRE(rate.post_read.early);
    REQUIRE(rate.post_read.late);
    REQUIRE(is_scaled(rate.scales->post.early));
    REQUIRE(is_scaled(rate.scales->post.late));
    REQUIRE(rate.scales->sqins.size() == 1);
    REQUIRE(rate.sqins_read.size() == 1);
    REQUIRE(rate.sqins_read[0].late);
    REQUIRE(is_scaled(rate.scales->sqins[0].late));

    const ActivationsView view = build_activations(*pd.path, *pd.rec, &timing);
    REQUIRE(view.acts.size() == acts.size());
    const ActivationRowView& av = view.acts[0];
    CAPTURE(av.scale_warning);
    CAPTURE(av.sqin_scale_warning);
    CHECK(av.scale_warning.rfind("Frontend timing scales ", 0) == 0);
    CHECK(av.scale_warning.find("(early) / ") < av.scale_warning.find("(late)"));
    CHECK(av.scale_warning.find(" at the SP end.") != std::string::npos);
    // One SqIn, so its clause says "the SqIn's", not a number or "each" (D16).
    CHECK(av.sqin_scale_warning.find("(late) at the SqIn's SP end.") != std::string::npos);
    CHECK(av.scale_warn == rate.scale_governs);
}

// The wording cases below build one activation each, because the clause forms
// (x1.00 left out, "each", numbered SqIns) are the display's own words.

TEST_CASE("build_activations: the scale line is empty at x1.00 even past the budget") {
    Activation overbudget = scale_base();
    overbudget.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 250.0});
    // Stored x1.00, not left unknown (D4).
    test::set_transfer(overbudget, TransferScale{1.0, 1.0});
    CHECK(scale_row_of(overbudget).scale_warning.empty());
}

TEST_CASE("build_activations: a row on the SP end reads only the early side") {
    // The only row sits exactly on the SP end. It is inside SP, so the early
    // side governs it: its 0 ms margin is still 0 ms, and the line is orange
    // because a shown multiplier governs a row. Nothing reads the late side,
    // so the line leaves it out.
    Activation on_end = scale_base();
    test::set_transfer(on_end, TransferScale{8.59, 8.76});
    on_end.backends.push_back(backend_at(0.0));
    const ActivationRowView av = scale_row_of(on_end);
    CHECK(av.scale_warning == "Frontend timing scales x8.59 (early) at the SP end.");
    CHECK(av.scale_warn);
    REQUIRE(av.backends.size() == 1);
    CHECK(av.backends[0].rating.find(" (eff. 0.0 ms)") != std::string::npos);
}

TEST_CASE("build_activations: the scale line leaves out a side at x1.00") {
    // A row inside SP and a row past it read both sides; a side at exactly
    // x1.00 is still left out.
    const BackendSqueeze row = backend_at(50.0);
    const BackendSqueeze inside_row = backend_at(-50.0);
    Activation late_only = scale_base();
    test::set_transfer(late_only, TransferScale{1.0, 6.33});
    late_only.backends.push_back(inside_row);
    late_only.backends.push_back(row);
    CHECK(scale_row_of(late_only).scale_warning ==
          "Frontend timing scales x6.33 (late) at the SP end.");
}

TEST_CASE("build_activations: a backend row the late scale moves turns the line orange") {
    Activation backend = scale_base();
    test::set_transfer(backend, TransferScale{1.0, 0.5});
    backend.backends.push_back(backend_at(50.0));
    const ActivationRowView av = scale_row_of(backend);
    CHECK(av.scale_warning == "Frontend timing scales x0.50 (late) at the SP end.");
    CHECK(av.scale_warn);
}

TEST_CASE("build_activations: a SqIn's scale line reads its own stored end") {
    // The SqIn's own end is the end before its phrase extended SP. With the
    // SP end at x1.00, only the SqIn's line shows.
    Activation sqin = scale_base();
    sqin.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 50.0});
    test::set_transfer(sqin, TransferScale{1.0, 0.5}, TransferScale{});
    const ActivationRowView av = scale_row_of(sqin);
    CHECK(av.sqin_scale_warning == "Frontend timing scales x0.50 (late) at the SqIn's SP end.");
    CHECK(av.scale_warning.empty());
    CHECK(av.scale_warn);
}

TEST_CASE("build_activations: the SqIn's end and the SP end each get a line") {
    // Different scales at the two ends. One row on each side of the SP end
    // reads both of its sides.
    Activation both = scale_base();
    both.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 50.0});
    test::set_transfer(both, TransferScale{1.0, 0.5}, TransferScale{1.25, 0.8});
    both.backends.push_back(backend_at(-50.0));
    both.backends.push_back(backend_at(50.0));
    const ActivationRowView av = scale_row_of(both);
    CHECK(av.sqin_scale_warning == "Frontend timing scales x0.50 (late) at the SqIn's SP end.");
    CHECK(av.scale_warning == "Frontend timing scales x1.25 (early) / x0.80 (late) at the SP end.");
}

TEST_CASE("build_activations: SqIns that share a scale read as one clause with \"each\"") {
    // "each", so it doesn't read as a single SqIn (D16).
    Activation shared = scale_base();
    shared.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 50.0});
    shared.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 30.0});
    test::set_transfer(shared, TransferScale{1.0, 0.5}, TransferScale{});
    CHECK(scale_row_of(shared).sqin_scale_warning ==
          "Frontend timing scales x0.50 (late) at each SqIn's SP end.");
}

TEST_CASE("build_activations: the shared SqIn clause keeps \"each\" beside a scaled SP end") {
    // The SP end is read by a row inside SP.
    Activation shared_both = scale_base();
    shared_both.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 50.0});
    shared_both.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 30.0});
    test::set_transfer(shared_both, TransferScale{1.0, 0.5}, TransferScale{1.25, 1.0});
    shared_both.backends.push_back(backend_at(-50.0));
    const ActivationRowView av = scale_row_of(shared_both);
    CHECK(av.sqin_scale_warning == "Frontend timing scales x0.50 (late) at each SqIn's SP end.");
    CHECK(av.scale_warning == "Frontend timing scales x1.25 (early) at the SP end.");
}

TEST_CASE("build_activations: SqIns with different scales get one numbered clause each") {
    // Q5: numbered by the SqIn's place among the SqIns.
    Activation two = scale_base();
    two.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -50.0});
    two.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -30.0});
    test::set_sqin_transfers(two, {TransferScale{0.97, 1.0}, TransferScale{0.95, 1.0}},
                             TransferScale{});
    CHECK(scale_row_of(two).sqin_scale_warning ==
          "Frontend timing scales x0.97 (early) at SqIn 1's SP end; "
          "x0.95 (early) at SqIn 2's SP end.");
}

TEST_CASE("build_activations: a SqIn that prints like the SP end gets no clause, the other keeps its number") {
    // A row inside SP reads the SP end's early side.
    Activation one_differs = scale_base();
    one_differs.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -50.0});
    one_differs.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -30.0});
    test::set_sqin_transfers(one_differs,
                             {TransferScale{1.25, 1.0}, TransferScale{0.95, 1.0}},
                             TransferScale{1.25, 1.0});
    one_differs.backends.push_back(backend_at(-50.0));
    const ActivationRowView av = scale_row_of(one_differs);
    CHECK(av.sqin_scale_warning == "Frontend timing scales x0.95 (early) at SqIn 2's SP end.");
    CHECK(av.scale_warning == "Frontend timing scales x1.25 (early) at the SP end.");
}

TEST_CASE("build_activations: a lone SqIn that prints like the SP end gets no SqIn line") {
    Activation alike = scale_base();
    alike.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -50.0});
    test::set_transfer(alike, TransferScale{1.25, 1.0}, TransferScale{1.25, 1.0});
    alike.backends.push_back(backend_at(-50.0));
    const ActivationRowView av = scale_row_of(alike);
    CHECK(av.sqin_scale_warning.empty());
    CHECK(av.scale_warning == "Frontend timing scales x1.25 (early) at the SP end.");
}

TEST_CASE("build_activations: rows inside SP read only the early side") {
    // Both sides scaled, but both rows sit inside SP, so the late side is
    // left out.
    Activation inside = scale_base();
    test::set_transfer(inside, TransferScale{0.83, 1.20});
    inside.backends.push_back(backend_at(-400.0));
    inside.backends.push_back(backend_at(-300.0));
    const ActivationRowView av = scale_row_of(inside);
    CHECK(av.scale_warning == "Frontend timing scales x0.83 (early) at the SP end.");
    CHECK(av.scale_warn);
}

TEST_CASE("build_activations: rows on both sides of the SP end print both sides, early first") {
    Activation both_sides = scale_base();
    test::set_transfer(both_sides, TransferScale{0.83, 1.20});
    both_sides.backends.push_back(backend_at(50.0));
    both_sides.backends.push_back(backend_at(-50.0));
    const ActivationRowView av = scale_row_of(both_sides);
    CHECK(av.scale_warning ==
          "Frontend timing scales x0.83 (early) / x1.20 (late) at the SP end.");
    CHECK(av.scale_warn);
}

TEST_CASE("build_activations: scales nothing reads give no line and nothing orange") {
    // There are no backend rows, and the only SqIn sits past its end, where
    // its late side is x1.00.
    Activation unread = scale_base();
    unread.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 50.0});
    test::set_transfer(unread, TransferScale{0.7, 1.0}, TransferScale{0.83, 1.20});
    const ActivationRowView av = scale_row_of(unread);
    CHECK(av.scale_warning.empty());
    CHECK_FALSE(av.scale_warn);
}

// D17 rules what the hint covers: SP is counted in measures from the
// activation to the SP end, and early and late hits split at a tempo or time
// signature change. A reword that keeps those passes.
TEST_CASE("kTransferScaleHint explains the measure scaling") {
    const std::string hint = kTransferScaleHint;
    for (const char* term : {"measures", "activation", "SP end", "tempo", "time signature", "Early and late"}) {
        CAPTURE(term);
        CHECK(hint.find(term) != std::string::npos);
    }
}

// Whether the cap clamped a window and the frontend decides a squeeze on it
// is rate_activation's (cap_clamped); the clamped note is the activation's
// clamp_tick(); its measure is format_measure. On every corpus activation the
// warning shows exactly when cap_clamped is set, naming that note's measure
// when there is a SongTiming and no measure when there is none. A clamped
// window with nothing the frontend decides gets no warning.
TEST_CASE("build_activations: on every corpus activation the overfill warning names the clamped note") {
    int warned = 0;
    for_each_corpus_path([&](const std::string& chart, const Song& song, const HydraRecord& rec,
                             const Path& path) {
        CAPTURE(chart);
        CAPTURE(path.pathstring());
        const SongTiming& timing = song.timing();
        const std::vector<Activation> acts = path.all_activations();
        const ActivationsView with_timing = build_activations(path, rec, &timing);
        const ActivationsView without_timing = build_activations(path, rec, nullptr);
        REQUIRE(with_timing.acts.size() == acts.size());
        REQUIRE(without_timing.acts.size() == acts.size());
        for (size_t i = 0; i < acts.size(); ++i) {
            CAPTURE(i);
            const ActivationRating rate = rate_activation(acts[i]);
            const std::optional<int64_t> clamp = acts[i].clamp_tick();
            if (rate.cap_clamped) {
                REQUIRE(clamp.has_value());
                CHECK(with_timing.acts[i].overfill_warning ==
                      "SP overfilled at " + format_measure(timing, *clamp));
                CHECK(without_timing.acts[i].overfill_warning == "SP overfilled");
                ++warned;
            } else {
                CHECK(with_timing.acts[i].overfill_warning.empty());
                CHECK(without_timing.acts[i].overfill_warning.empty());
            }
        }
    });
    // Pitch Dark's first activation (the next case) is one.
    CHECK(warned > 0);
}

// Pitch Dark's first activation: the cap clamps its window on a note after
// the activation, and its squeeze-in is one the frontend decides. The warning
// names the clamped note's measure, not the activation's.
TEST_CASE("build_activations: Pitch Dark's overfill warning names the clamped note, not the activation") {
    const PitchDarkPath pd = pitch_dark_path();
    const SongTiming& timing = pd.song->timing();
    const std::vector<Activation> acts = pd.path->all_activations();
    REQUIRE_FALSE(acts.empty());
    const Activation& act = acts[0];
    REQUIRE(rate_activation(act).cap_clamped);
    const std::optional<int64_t> clamp = act.clamp_tick();
    REQUIRE(clamp.has_value());
    const std::string clamped_measure = format_measure(timing, *clamp);
    REQUIRE(clamped_measure != format_measure(act.timecode));

    const ActivationsView view = build_activations(*pd.path, *pd.rec, &timing);
    REQUIRE(view.acts.size() == acts.size());
    CHECK(view.acts[0].overfill_warning == "SP overfilled at " + clamped_measure);
}

TEST_CASE("build_activations: the backend limit hides far rows but never "
          "squeezed-out ones") {
    HydraRecord rec;  // only feeds the footer; irrelevant here

    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;  // not e-critical
    const std::pair<double, int64_t> rows_at[] = {{-30.0, 200}, {-100.0, 100},
                                                   {60.0, 300}};
    for (const auto& [ms, tick] : rows_at) {
        BackendSqueeze row;
        row.timecode = Timecode::raw(tick);
        row.offset_ms = ms;
        act.backends.push_back(row);
    }
    // sqout_tick names the +60 row (tick 300), so that row is the
    // squeezed-out one. It has to be the last row in chart order: nothing can
    // be a backend past the note squeezed out of SP, and display_backends
    // drops any row that claims to be.
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, 60.0});
    act.sqout_tick = 300;

    Path p;
    p.activations.push_back(act);

    auto view_at = [&](std::optional<double> limit) {
        ActivationsView v = limit ? build_activations(p, rec, nullptr, limit)
                                  : build_activations(p, rec, nullptr);
        REQUIRE(v.acts.size() == 1);
        return v.acts[0];
    };

    // No limit: every stored row shows, and nothing is said about hiding.
    ActivationRowView all = view_at(std::nullopt);
    CHECK(all.backends.size() == 3);
    CHECK(all.backends_label == "3 notes near the SP end");
    CHECK(all.backends_hidden_line.empty());

    // At 50 ms the -100 row goes; the +60 row stays because it is squeezed out.
    // The heading still counts all three and says what is hidden.
    ActivationRowView at50 = view_at(50.0);
    std::vector<BackendRowView> limited = at50.backends;
    REQUIRE(limited.size() == 2);
    CHECK(limited[0].timing == "-30.0");
    CHECK_FALSE(limited[0].warn);
    CHECK(limited[1].timing == "60.0");
    // +60 ms is past the leeway: the engine never counted this chord, so the
    // row is tagged uncounted and not highlighted (D274, decision 19).
    CHECK(limited[1].rating.find("(uncounted) <-- squeezed out") != std::string::npos);
    CHECK_FALSE(limited[1].warn);
    CHECK(at50.backends_label == "3 notes near the SP end \xC2\xB7 1 hidden: 1 before, 0 after");
    CHECK(at50.backends_hidden_line ==
          "1 hidden: 1 before the SP end and 0 after, beyond 50 ms.");

    // At 20 ms the -30 row goes too.
    ActivationRowView at20 = view_at(20.0);
    CHECK(at20.backends.size() == 1);
    CHECK(at20.backends_label == "3 notes near the SP end \xC2\xB7 2 hidden: 2 before, 0 after");
    CHECK(at20.backends_hidden_line ==
          "2 hidden: 2 before the SP end and 0 after, beyond 20 ms.");
}

TEST_CASE("build_activations: the backend limit says how many rows it hides on each side") {
    HydraRecord rec;  // only feeds the footer; irrelevant here

    // An activation with backend rows at these offsets and no squeeze-out.
    auto view_of = [&rec](std::vector<double> offsets, std::optional<double> limit) {
        Activation act;
        test::set_skips(act, 0);
        act.e_offset = 300.0;  // not e-critical
        int64_t tick = 100;
        for (double ms : offsets) {
            BackendSqueeze row;
            row.timecode = Timecode::raw(tick);
            row.offset_ms = ms;
            act.backends.push_back(row);
            tick += 100;
        }
        Path p;
        p.activations.push_back(act);
        ActivationsView v = build_activations(p, rec, nullptr, limit);
        REQUIRE(v.acts.size() == 1);
        return v.acts[0];
    };

    // Rows on both sides, some hidden: the heading and the line under the
    // table name each side.
    ActivationRowView some = view_of({-100.0, -30.0, 20.0, 80.0}, 50.0);
    CHECK(some.backends.size() == 2);
    CHECK(some.backends_label == "4 notes near the SP end \xC2\xB7 2 hidden: 1 before, 1 after");
    CHECK(some.backends_hidden_line ==
          "2 hidden: 1 before the SP end and 1 after, beyond 50 ms.");

    // Every row hidden: the line, shown in place of "None.", starts "All".
    ActivationRowView every = view_of({-120.0, -90.0, 84.0}, 50.0);
    CHECK(every.backends.empty());
    CHECK(every.backends_label == "3 notes near the SP end \xC2\xB7 3 hidden: 2 before, 1 after");
    CHECK(every.backends_hidden_line ==
          "All 3 hidden: 2 before the SP end and 1 after, beyond 50 ms.");

    // A single row, hidden: no "All", and the heading is singular.
    ActivationRowView one = view_of({-100.0}, 50.0);
    CHECK(one.backends.empty());
    CHECK(one.backends_label == "1 note near the SP end \xC2\xB7 1 hidden: 1 before, 0 after");
    CHECK(one.backends_hidden_line ==
          "1 hidden: 1 before the SP end and 0 after, beyond 50 ms.");

    // The limit set but every row inside it: nothing hidden, heading unchanged.
    ActivationRowView inside = view_of({-30.0, 20.0}, 50.0);
    CHECK(inside.backends.size() == 2);
    CHECK(inside.backends_label == "2 notes near the SP end");
    CHECK(inside.backends_hidden_line.empty());

    // No rows stored at all: the fold keeps "None.", so no hidden line.
    ActivationRowView none = view_of({}, 50.0);
    CHECK(none.backends.empty());
    CHECK(none.backends_label == "0 notes near the SP end");
    CHECK(none.backends_hidden_line.empty());
}

TEST_CASE("build_activations: a squeezed-out row past the leeway is worth 0") {
    HydraRecord rec;  // only feeds the footer

    // Round and Round (Ratt), second activation: an R+Y chord squeezed out
    // 479.999 ms past the SP end. The engine never counts it, so the table
    // must not claim it banks 260 and loses 200, and must not highlight it.
    Activation far;
    test::set_skips(far, 0);
    far.e_offset = 300.0;  // not e-critical
    BackendSqueeze far_row;
    far_row.timecode = Timecode::raw(3256);
    far_row.sp_points = 460;
    far_row.sqout_sp_points = 260;
    far_row.offset_ms = 479.999;
    far.backends.push_back(far_row);
    far.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, 479.999});
    far.sqout_tick = 3256;

    Path p;
    p.activations.push_back(far);
    ActivationsView v = build_activations(p, rec, nullptr);
    REQUIRE(v.acts.size() == 1);
    REQUIRE(v.acts[0].backends.size() == 1);
    CHECK(v.acts[0].backends[0].points == "0");
    CHECK(v.acts[0].backends[0].rating ==
          "Beyond (uncounted) <-- squeezed out");
    CHECK_FALSE(v.acts[0].backends[0].warn);
    check_word_agrees_with_badge(far);

    // The same chord squeezed out 5 ms inside SP really costs 200, and
    // that row keeps its warning colour.
    Activation near = far;
    near.backends[0].offset_ms = -5.0;
    near.sqinouts[0].offset_ms = -5.0;
    Path q;
    q.activations.push_back(near);
    v = build_activations(q, rec, nullptr);
    REQUIRE(v.acts[0].backends.size() == 1);
    CHECK(v.acts[0].backends[0].points == "260");
    CHECK(v.acts[0].backends[0].rating ==
          "Hard <-- squeezed out (-200)");
    CHECK(v.acts[0].backends[0].warn);
    check_word_agrees_with_badge(near);
}

// User decision 19: every row the engine does not count reads 0 and is not
// highlighted, not only squeezed-out ones. Every counted plain row reads
// Normal (D276 item 2).
TEST_CASE("build_activations: plain rows past the leeway show 0") {
    HydraRecord rec;  // only feeds the footer

    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;  // not e-critical
    const double leeway = core::default_rules().backend_leeway_ms;
    const int full_points = 460;
    const int sqout_points = 260;
    // Inside SP, inside the leeway, exactly on it, just past it, far past it,
    // and a phrase chord past it that this path did not squeeze out (a plain
    // row, finding 29).
    const std::pair<int64_t, double> rows_at[] = {{100, -40.0}, {200, leeway - 0.5},
                                                  {250, leeway},  {300, 20.0},
                                                  {400, 120.0},   {500, 150.0}};
    for (const auto& [tick, ms] : rows_at) {
        BackendSqueeze row;
        row.timecode = Timecode::raw(tick);
        row.sp_points = full_points;
        row.sqout_sp_points = sqout_points;
        row.offset_ms = ms;
        act.backends.push_back(row);
    }

    Path p;
    p.activations.push_back(act);
    ActivationsView v = build_activations(p, rec, nullptr);
    REQUIRE(v.acts.size() == 1);
    const std::vector<BackendRowView>& b = v.acts[0].backends;
    REQUIRE(b.size() == std::size(rows_at));
    const std::vector<TimingTier> tiers = timing_tiers();
    int counted = 0;
    int uncounted = 0;
    for (size_t i = 0; i < b.size(); ++i) {
        const double ms = rows_at[i].second;
        CAPTURE(ms);
        // The points are what the engine pays this row with no squeeze out.
        const int pays = core::backend_row_value(ms, full_points, sqout_points,
                                                 core::SqOutPosition::NoSqOut, leeway);
        CHECK(b[i].points == std::to_string(pays));
        if (core::counted_without_squeeze(ms, leeway)) {
            // A counted plain row needs no timing, so it reads the first tier.
            CHECK(b[i].rating == tiers.front().name);
            ++counted;
        } else {
            CHECK(pays == 0);
            CHECK(b[i].rating == tier_for(ms, tiers).first + " (uncounted)");
            ++uncounted;
        }
        CHECK_FALSE(b[i].warn);
    }
    // The leeway's edge is strict: the row exactly on it is uncounted.
    CHECK(counted == 2);
    CHECK(uncounted == 4);
}

TEST_CASE("format_measure: one form for both tabs") {
    // One 4/4 grid with no tempo or signature change, so every measure is
    // the same length and each expected string is written from the grid.
    const int64_t beat = 192;
    const int64_t measure = 4 * beat;
    std::map<int64_t, int64_t> tpm{{0, measure}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    SongTiming timing(static_cast<int>(beat), tpm, bpm, {{0, {4, 4}}});
    // "m<measure>.<beat>.<tick past the beat>", measure and beat 1-based.
    auto on_grid = [&](int64_t m, int64_t b, int64_t t) {
        const int64_t tick = m * measure + b * beat + t;
        const std::string want = "m" + std::to_string(m + 1) + "." + std::to_string(b + 1) + "." +
                                 std::to_string(t);
        CAPTURE(tick);
        CHECK(format_measure(timing, tick) == want);
        // The Timecode overload, which the Preview reads, prints the same.
        CHECK(format_measure(timing.timecode(tick)) == want);
    };
    on_grid(0, 0, 0);
    on_grid(1, 1, 0);
    on_grid(1, 1, 40);
    on_grid(2, 3, beat - 1);

    // On every real chart, each of the first measure lines its own grid walk
    // reports prints as the next measure number, beat 1, tick 0.
    int lines_checked = 0;
    for_each_corpus_chart([&](const std::string&, const Song& song, const HydraRecord&) {
        const SongTiming& real = song.timing();
        const MeasureIndex& grid = real.measure_index();
        int64_t number = 0;
        grid.walk(grid.measure_line_tick(30), [&](int64_t tick, MeasureIndex::Line line) {
            if (line != MeasureIndex::Line::Measure) return;
            CAPTURE(tick);
            CHECK(format_measure(real, tick) == "m" + std::to_string(number + 1) + ".1.0");
            ++number;
            ++lines_checked;
        });
    });
    CHECK(lines_checked > 0);
}

TEST_CASE("activation rows: each of Burnout's rows reads its activation") {
    const AnalysisResult& ar = burnout();
    const HydraRecord& rec = ar.record;
    const Path& best = rec.best_path();
    const SongTiming& timing = ar.song.timing();
    const std::vector<Activation> acts = best.all_activations();

    ActivationsView view = build_activations(best, rec, &timing);
    REQUIRE(view.acts.size() == acts.size());
    for (size_t i = 0; i < acts.size(); ++i) {
        const Activation& act = acts[i];
        const ActivationRowView& av = view.acts[i];
        CAPTURE(i);
        CHECK(av.number == static_cast<int>(i) + 1);
        CHECK(av.measure == format_measure(act.timecode));
        CHECK(av.notation == act.notationstr());
        CHECK(av.sp_bars == act.sp_meter());
        CHECK(av.bars == bars_text(act.sp_meter()));
        CHECK(av.chord == act.chord.rowstr(true));
        CHECK(av.squeeze_sentences.size() == act.sqinouts.size());
        CHECK(av.backends.size() == act.display_backends().size());
        // One badge per timing the activation lists, in its order, orange
        // exactly where the activation calls that timing difficult.
        const std::vector<ActivationTiming> timings = act.badge_timings();
        REQUIRE(av.badges.size() == timings.size());
        for (size_t j = 0; j < timings.size(); ++j) {
            CHECK(av.badges[j].text.find(format_ms(timings[j].ms)) != std::string::npos);
            CHECK(av.badges[j].warn == timings[j].difficult);
        }
        // No song length given: no timeline.
        CHECK_FALSE(av.song_fraction.has_value());
    }
    CHECK(view.timeline_end.empty());

    // Burnout's best path squeezes a note out (its first activation's badge
    // reads "squeeze out"), so this loop has a row to check.
    int squeezed_out = 0;
    for (const Activation& act : best.activations)
        if (act.sqout_row()) {
            check_word_agrees_with_badge(act);
            ++squeezed_out;
        }
    CHECK(squeezed_out >= 1);
}

TEST_CASE("activation timeline: onset over the song's length, and the end measure") {
    // 120 BPM, 4/4, 192 ticks a beat: a measure is 2000 ms.
    std::map<int64_t, int64_t> tpm{{0, 768}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    SongTiming timing(192, tpm, bpm, {{0, {4, 4}}});
    Activation act;
    act.timecode = timing.timecode(768);  // measure 2, 2000 ms
    test::set_sp_meter(act, 2);
    test::set_skips(act, 0);
    act.e_offset = 300.0;  // not e-critical
    Path p;
    p.activations.push_back(act);
    HydraRecord rec;

    ActivationsView view = build_activations(p, rec, &timing, std::nullopt,
                                             core::default_rules(), 10000.0);
    REQUIRE(view.acts.size() == 1);
    REQUIRE(view.acts[0].song_fraction.has_value());
    CHECK(*view.acts[0].song_fraction == doctest::Approx(0.2));
    CHECK(view.timeline_start == "m1");
    CHECK(view.timeline_end == "m6");  // 10 s is tick 3840, the start of measure 6
    CHECK(view.summary == "1" + kDot + "no SP left over");

    // No timing: no fraction, whatever the length.
    ActivationsView blind = build_activations(p, rec, nullptr, std::nullopt,
                                              core::default_rules(), 10000.0);
    CHECK_FALSE(blind.acts[0].song_fraction.has_value());
    CHECK(blind.timeline_start.empty());
    CHECK(blind.timeline_end.empty());
}

namespace {

// A timing's figure as every display writes a needed timing (format_ms,
// D265: one decimal everywhere).
std::string figure_of(const ActivationTiming& t) { return format_ms(t.ms); }

// The word in front of a badge's figure: the badge text up to the space
// before `figure`. Requires the text to end in that space and figure.
std::string badge_word_of(const ActivationBadge& b, const std::string& figure) {
    const std::string tail = " " + figure;
    REQUIRE(b.text.size() > tail.size());
    REQUIRE(b.text.compare(b.text.size() - tail.size(), tail.size(), tail) == 0);
    return b.text.substr(0, b.text.size() - tail.size());
}

// Calls visit(act) for every activation of every kept path of every corpus
// chart that analyzes under the app's default settings.
template <typename Visit>
void for_each_corpus_activation(Visit&& visit) {
    for_each_corpus_path([&](const std::string& chart, const Song& /*song*/, const HydraRecord& /*rec*/,
                             const Path& path) {
        CAPTURE(chart);
        CAPTURE(path.pathstring());
        for (const Activation& act : path.all_activations()) {
            CAPTURE(act.notationstr_verbose());
            visit(act);
        }
    });
}

// The badge words of every timing part the corpus shows, each read off a
// real badge, after checking every corpus badge against its timing: the
// badge ends in the timing's figure and is orange exactly when the timing
// is difficult.
std::map<TimingPart, std::string> corpus_badge_words() {
    std::map<TimingPart, std::string> words;
    for_each_corpus_activation([&](const Activation& act) {
        const std::vector<ActivationTiming> timings = act.badge_timings();
        const std::vector<ActivationBadge> badges = activation_badges(act);
        REQUIRE(badges.size() == timings.size());
        for (size_t i = 0; i < timings.size(); ++i) {
            CAPTURE(badges[i].text);
            const std::string word = badge_word_of(badges[i], figure_of(timings[i]));
            const auto placed = words.emplace(timings[i].part, word);
            // One word per part, wherever it shows.
            CHECK(placed.first->second == word);
            CHECK(badges[i].warn == timings[i].difficult);
        }
    });
    return words;
}

// Not e-critical, nothing to time.
Activation no_timing() {
    Activation a;
    test::set_skips(a, 0);
    a.e_offset = 300.0;
    return a;
}

// The user's row on "The Age of Betrayal" (not in the corpus): E3+, an
// optional early fill with no time to spare and a squeeze-in that already
// lands inside SP.
Activation user_row() {
    Activation a;
    test::set_skips(a, 3);
    a.e_offset = 0.0;
    a.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -12.0});
    return a;
}

// The one button build_path_buttons makes for a record holding only `act`.
PathButtonView only_button(const Activation& act) {
    Path p;
    p.activations.push_back(act);
    HydraRecord rec;
    rec.paths.push_back(p);
    const Settings s = Settings();
    const PathButtonsView v = build_path_buttons(rec, s.depth_mode, s.depth_value);
    REQUIRE(v.buttons.size() == 1);
    return v.buttons[0];
}

}  // namespace

// On every activation of every kept corpus path the badges are
// Activation::badge_timings() in its order: one per squeeze, plus the early
// fill first when the activation is E-critical; each the timing's figure
// after its part's word; orange exactly when the timing is difficult. Each
// part keeps one word and the three words differ. Beg (IB24 T1) has an E0
// fill and a squeeze out and Pitch Dark (test_pitch_dark_ratio) a squeeze in
// (the audit's engine records), so all three parts show.
TEST_CASE("activation badges: one per timing, the fill first, then each squeeze") {
    int orange = 0;
    int grey = 0;
    for_each_corpus_activation([&](const Activation& act) {
        const std::vector<ActivationTiming> timings = act.badge_timings();
        const std::vector<ActivationBadge> badges = activation_badges(act);
        const size_t fill = act.is_e_critical() ? 1 : 0;
        REQUIRE(timings.size() - fill == act.sqinouts.size());
        REQUIRE(badges.size() == timings.size());
        if (fill == 1) CHECK(timings[0].part == TimingPart::EarlyFill);
        for (size_t k = 0; k < act.sqinouts.size(); ++k) {
            const SPSqueeze& sq = act.sqinouts[k];
            const ActivationTiming& t = timings[k + fill];
            CHECK(t.part == (sq.kind == SqueezeKind::SqIn ? TimingPart::SqueezeIn : TimingPart::SqueezeOut));
            CHECK(t.ms == sq.difficulty());
            CHECK(t.required == !sq.needs_no_timing());
        }
        for (const ActivationBadge& b : badges) ++(b.warn ? orange : grey);
    });
    const std::map<TimingPart, std::string> words = corpus_badge_words();
    REQUIRE(words.size() == 3);
    CHECK(words.at(TimingPart::EarlyFill) != words.at(TimingPart::SqueezeIn));
    CHECK(words.at(TimingPart::EarlyFill) != words.at(TimingPart::SqueezeOut));
    CHECK(words.at(TimingPart::SqueezeIn) != words.at(TimingPart::SqueezeOut));
    CHECK(orange > 0);
    CHECK(grey > 0);
}

// A free squeeze-in (already inside SP) has nothing to time, but the row
// still shows it with its figure (D80), grey, and the activation still needs
// no timing. A named input: the corpus is not shown to hold one.
TEST_CASE("activation badges: a squeeze that needs no timing keeps its badge, grey") {
    Activation free_in = no_timing();
    free_in.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -315.8});
    REQUIRE(free_in.sqinouts[0].needs_no_timing());
    const std::vector<ActivationTiming> timings = free_in.badge_timings();
    const std::vector<ActivationBadge> badges = activation_badges(free_in);
    REQUIRE(timings.size() == 1);
    REQUIRE(badges.size() == 1);
    CHECK_FALSE(timings[0].required);
    CHECK_FALSE(badges[0].warn);
    badge_word_of(badges[0], figure_of(timings[0]));
    CHECK_FALSE(free_in.needs_timing());
    // Beside a timed squeeze, both show in list order.
    free_in.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 1.0});
    const std::vector<ActivationTiming> two = free_in.badge_timings();
    const std::vector<ActivationBadge> two_badges = activation_badges(free_in);
    REQUIRE(two.size() == 2);
    REQUIRE(two_badges.size() == 2);
    for (size_t i = 0; i < 2; ++i) {
        CHECK(two[i].ms == free_in.sqinouts[i].difficulty());
        badge_word_of(two_badges[i], figure_of(two[i]));
    }
}

// A squeeze-in exactly on the SP end is not free: it needs 0.0 ms (D302), so
// it is required and the path button shows its figure. A named input: the
// rule's edge exactly.
TEST_CASE("path button: a squeeze exactly on the SP end shows its 0.0 ms") {
    Activation on_end_in = no_timing();
    on_end_in.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 0.0});
    const std::vector<ActivationTiming> timings = on_end_in.badge_timings();
    REQUIRE(timings.size() == 1);
    CHECK(timings[0].required);
    CHECK_FALSE(timings[0].difficult);
    CHECK(on_end_in.needs_timing());
    CHECK(only_button(on_end_in).timing == format_ms(0.0));
}

// A required fill with time to spare is nothing to time (D261, D51 call 4).
TEST_CASE("Activation: a required fill with time to spare needs no timing") {
    Activation slack = no_timing();
    slack.e_offset = 10.0;  // SP ready 10 ms before the fill's deadline
    REQUIRE(slack.is_E0());
    CHECK_FALSE(slack.difficulty().has_value());
    CHECK(slack.within_ms_limit(0.0));
}

// The timeline mark's colour (difficult) and the pills' colours agree on
// every activation of every kept corpus path.
TEST_CASE("activation badges: the row is difficult exactly when a badge is orange") {
    int difficult_rows = 0;
    int plain_rows = 0;
    for_each_corpus_path([&](const std::string& chart, const Song& song, const HydraRecord& rec,
                             const Path& path) {
        CAPTURE(chart);
        CAPTURE(path.pathstring());
        const SongTiming& timing = song.timing();
        const ActivationsView v = build_activations(path, rec, &timing);
        for (const ActivationRowView& av : v.acts) {
            CAPTURE(av.number);
            bool any_warn = false;
            for (const ActivationBadge& b : av.badges) any_warn = any_warn || b.warn;
            CHECK(av.difficult == any_warn);
            ++(av.difficult ? difficult_rows : plain_rows);
        }
    });
    CHECK(difficult_rows > 0);
    CHECK(plain_rows > 0);
}

// On every kept corpus path the copied path (notationstr_verbose) carries
// each badge's figure in badge order, and the path button shows a timing
// exactly when the path needs one, that timing being one of its badges'
// figures.
TEST_CASE("activation badges: the badges, the copied path and the path button show one figure") {
    int buttons_with_timing = 0;
    for_each_corpus_chart([&](const std::string& chart, const Song& /*song*/, const HydraRecord& rec) {
        CAPTURE(chart);
        const Settings s = Settings();
        const PathButtonsView v = build_path_buttons(rec, s.depth_mode, s.depth_value);
        for (const PathButtonView& b : v.buttons) {
            REQUIRE(b.path != nullptr);
            CAPTURE(b.notation);
            std::vector<std::string> figures;
            for (const Activation& act : b.path->all_activations()) {
                CAPTURE(act.notationstr());
                const std::string verbose = act.notationstr_verbose();
                std::string joined;
                for (const ActivationTiming& t : act.badge_timings()) {
                    joined += (joined.empty() ? "" : "/") + figure_of(t);
                    figures.push_back(figure_of(t));
                }
                const std::string expected =
                    joined.empty() ? act.notationstr() : act.notationstr() + " (" + joined + ")";
                CHECK(verbose == expected);
            }
            CHECK(b.timing.empty() == !b.path->needs_timing());
            if (!b.timing.empty()) {
                ++buttons_with_timing;
                bool among = false;
                for (const std::string& f : figures) among = among || f == b.timing;
                CHECK(among);
            }
        }
    });
    CHECK(buttons_with_timing > 0);
}

// The user's row: an optional fill with no time to spare and a free squeeze
// both show a badge, and the path button shows no timing: neither is
// anything to time.
TEST_CASE("activation badges: an optional fill and a free squeeze show, the button shows nothing") {
    const Activation act = user_row();
    REQUIRE_FALSE(act.is_E0());
    REQUIRE(act.sqinouts[0].needs_no_timing());
    CHECK(activation_badges(act).size() == 2);
    CHECK_FALSE(act.needs_timing());
    CHECK(only_button(act).timing.empty());
}

TEST_CASE("measure labels: one numbering rule for the rows, the timeline and the first measure") {
    // The timeline case's timing: 192 ticks a beat, 4/4, 120 BPM.
    std::map<int64_t, int64_t> tpm{{0, 768}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    SongTiming timing(192, tpm, bpm, {{0, {4, 4}}});
    // The timeline's label is the rows' measure text up to its beat, so the
    // two never number a measure differently.
    for (const int64_t tick : {int64_t{0}, int64_t{768}, int64_t{1000}}) {
        CAPTURE(tick);
        const Timecode tc = timing.timecode(tick);
        const std::string label = measure_label(tc);
        CHECK(format_measure(tc).rfind(label + ".", 0) == 0);
    }
    // Ticks a whole measure apart get labels one apart, from 1.
    CHECK(measure_label(timing.timecode(0)) == "m" + std::to_string(1));
    CHECK(measure_label(timing.timecode(768)) == "m" + std::to_string(2));
    // The first measure with no timing is the chart's first tick.
    CHECK(first_measure_label() == format_measure(timing, 0));
}

// The widest badge is the widest part word followed by the squeeze window's
// bound as a figure (longest_activation_badge says why the window bounds
// every figure), and no badge on any kept corpus path is wider.
TEST_CASE("longest_activation_badge: the widest wording with the window's figure") {
    const std::string longest = longest_activation_badge();
    std::string widest_word;
    for (const auto& part_word : corpus_badge_words())
        if (part_word.second.size() > widest_word.size()) widest_word = part_word.second;
    CHECK(longest == widest_word + " " + format_ms(-kSqueezeWindowMs));
    int badges = 0;
    for_each_corpus_activation([&](const Activation& act) {
        for (const ActivationBadge& b : activation_badges(act)) {
            CAPTURE(b.text);
            CHECK(b.text.size() <= longest.size());
            ++badges;
        }
    });
    CHECK(badges > 0);
}

namespace {

// Checks one squeeze's sentence against the engine's answers for it: the
// edge it prints is the squeeze's timing() as a figure; it is a "no more
// than" limit exactly when the squeeze needs no timing (D302); the squeezed
// note goes the way squeeze_presses presses it; a SqOut names its squeezed
// chord when the rating found the squeezed-out row; a SqIn carries its eff.
// figure exactly when the rating gave one; orange exactly when the squeeze is
// difficult.
void check_sentence(const TextLine& s, const SPSqueeze& sq, const BackendRating* squeezed_out,
                    std::optional<double> effective_ms) {
    CAPTURE(s.text);
    const std::string edge = format_ms(std::fabs(sq.timing()));
    const std::string side = squeeze_presses(sq).squeezed == Press::Late ? " late" : " early";
    CHECK(s.text.find(edge + side) != std::string::npos);
    CHECK((s.text.find("no more than " + edge) != std::string::npos) == sq.needs_no_timing());
    if (sq.kind == SqueezeKind::SqOut) {
        CHECK(s.text.find("after Star Power ends.") != std::string::npos);
        if (squeezed_out)
            CHECK(s.text.find(squeezed_out->row.chord.notationstr()) != std::string::npos);
        CHECK(s.text.find("eff.") == std::string::npos);
    } else {
        CHECK(s.text.find("before Star Power ends.") != std::string::npos);
        if (effective_ms) {
            CHECK(s.text.find("(eff. " + format_ms(*effective_ms) + ")") != std::string::npos);
        } else {
            CHECK(s.text.find("eff.") == std::string::npos);
        }
    }
    CHECK(s.warn == sq.is_difficult());
}

}  // namespace

// Every squeeze on every kept corpus path gets one sentence, read from the
// engine's squeeze and rating (check_sentence).
TEST_CASE("squeeze sentences: SqIn and SqOut are one sentence each") {
    int sqins = 0;
    int sqouts_naming_a_chord = 0;
    for_each_corpus_row([&](const Activation& act, const ActivationRowView& av) {
        const ActivationRating rate = rate_activation(act);
        const BackendRating* squeezed_out = nullptr;
        for (const BackendRating& br : rate.backends)
            if (br.squeezed_out) squeezed_out = &br;
        REQUIRE(av.squeeze_sentences.size() == act.sqinouts.size());
        for (size_t k = 0; k < act.sqinouts.size(); ++k) {
            const std::optional<double> eff =
                k < rate.note_effective_ms.size() ? rate.note_effective_ms[k] : std::nullopt;
            check_sentence(av.squeeze_sentences[k], act.sqinouts[k], squeezed_out, eff);
            if (act.sqinouts[k].kind == SqueezeKind::SqIn) ++sqins;
            else if (squeezed_out) ++sqouts_naming_a_chord;
        }
    });
    // Pitch Dark has a squeeze in and Beg a squeeze out of a named chord.
    CHECK(sqins > 0);
    CHECK(sqouts_naming_a_chord > 0);
}

// A squeezed-out row far past the SP end: its sentence still names its chord
// and says nothing of its cost (the backend table shows that). Typed by
// hand: no corpus record is shown to hold a squeezed-out row this far out.
// The chord, points and 480 ms are the Round and Round row (not in the
// corpus).
TEST_CASE("squeeze sentences: a squeezed-out row far past the end names its chord") {
    Activation far = no_timing();
    BackendSqueeze row;
    row.timecode = Timecode::raw(3256);
    row.chord.add_note(NoteColor::Red);
    row.chord.add_note(NoteColor::Yellow);
    row.sp_points = 460;
    row.sqout_sp_points = 260;
    row.offset_ms = 479.999;
    far.backends.push_back(row);
    far.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, row.offset()});
    far.sqout_tick = 3256;
    Path p;
    p.activations.push_back(far);
    HydraRecord rec;
    const ActivationsView v = build_activations(p, rec, nullptr);
    REQUIRE(v.acts.size() == 1);
    REQUIRE(v.acts[0].squeeze_sentences.size() == 1);
    const ActivationRating rate = rate_activation(far);
    const BackendRating* squeezed_out = nullptr;
    for (const BackendRating& br : rate.backends)
        if (br.squeezed_out) squeezed_out = &br;
    REQUIRE(squeezed_out != nullptr);
    check_sentence(v.acts[0].squeeze_sentences[0], far.sqinouts[0], squeezed_out, std::nullopt);
    const std::string& text = v.acts[0].squeeze_sentences[0].text;
    CHECK(text.find("point") == std::string::npos);
}

// D302: a squeeze exactly on the SP end needs 0.0 ms, a squeeze-in the same
// as a squeeze-out. Believer (IB24 T1) has both: a squeeze in and a squeeze
// out at offset 0 (the audit's engine records, paths 0 and 4).
TEST_CASE("squeeze sentences at the SP end: the squeeze-in earns, the squeeze-out earns") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    const std::string chart = corpus::chart_named("I_m A Believer");
    REQUIRE_FALSE(chart.empty());
    const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
    const HydraRecord& rec = corpus::analyzed(chart, cfg);
    const SongTiming& timing = song.timing();
    int on_end_in = 0;
    int on_end_out = 0;
    for (const Path& path : rec.paths) {
        CAPTURE(path.pathstring());
        const std::vector<Activation> acts = path.all_activations();
        const ActivationsView view = build_activations(path, rec, &timing);
        REQUIRE(view.acts.size() == acts.size());
        for (size_t i = 0; i < acts.size(); ++i) {
            REQUIRE(view.acts[i].squeeze_sentences.size() == acts[i].sqinouts.size());
            for (size_t k = 0; k < acts[i].sqinouts.size(); ++k) {
                const SPSqueeze& sq = acts[i].sqinouts[k];
                if (sq.offset_ms != 0.0) continue;
                const std::string& text = view.acts[i].squeeze_sentences[k].text;
                CAPTURE(text);
                // On the end it is still to earn: a timing, not a limit.
                CHECK_FALSE(sq.needs_no_timing());
                CHECK(text.find("more than " + format_ms(0.0)) != std::string::npos);
                CHECK(text.find("no more than") == std::string::npos);
                ++(sq.kind == SqueezeKind::SqIn ? on_end_in : on_end_out);
            }
        }
    }
    CHECK(on_end_in > 0);
    CHECK(on_end_out > 0);
}

// On every kept corpus path each backend row shows the rating's own eff.
// figure (rate_activation's effective_ms) exactly when the rating gave one,
// with a tooltip then and only then. Pitch Dark's first activation has rows
// scaled on both sides (the scale line case above), so some rows carry one.
TEST_CASE("backend table: a counted row inside SP shows its early-scale eff. figure") {
    int rows_with_eff = 0;
    for_each_corpus_row([&](const Activation& act, const ActivationRowView& av) {
        const ActivationRating rate = rate_activation(act);
        // No backend limit is passed, so every stored row shows.
        REQUIRE(av.backends.size() == rate.backends.size());
        for (size_t r = 0; r < rate.backends.size(); ++r) {
            const BackendRowView& row = av.backends[r];
            CAPTURE(row.rating);
            const std::optional<double>& eff = rate.backends[r].note.effective_ms;
            if (eff) {
                CHECK(row.rating.find("(eff. " + format_ms(*eff) + ")") != std::string::npos);
                CHECK(row.tooltip.find(format_ms(*eff)) != std::string::npos);
                ++rows_with_eff;
            } else {
                CHECK(row.rating.find("eff.") == std::string::npos);
                CHECK(row.tooltip.empty());
            }
        }
    });
    CHECK(rows_with_eff > 0);
}

namespace {

// The multiplier a scale line prints for `side` ("early" or "late"), read
// back as a number; unset when the line names no such side.
std::optional<double> printed_scale(const std::string& line, const std::string& side) {
    const size_t at = line.find(" (" + side + ")");
    if (at == std::string::npos) return std::nullopt;
    const size_t x = line.rfind('x', at);
    if (x == std::string::npos) return std::nullopt;
    return std::strtod(line.c_str() + x + 1, nullptr);
}

// Whether `a` and `b` read the same at two decimals, the scale line's
// shortest form.
bool same_at_two_decimals(double a, double b) { return std::round(a * 100.0) == std::round(b * 100.0); }

// Whether `r` would print as x1.00 at two decimals.
bool reads_one_at_two_decimals(double r) { return same_at_two_decimals(r, kIdentityScale); }

// An activation whose SP-end scale is `post`, with one squeezed-out row
// 187.5 ms inside SP (a row the early side governs), as one path.
Path near_one_path(const TransferScale& post) {
    Activation act = scale_base();
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -187.5});
    test::set_transfer(act, post);
    BackendSqueeze row = backend_at(-187.5);
    row.timecode = Timecode::raw(5000);
    act.backends.push_back(row);
    act.sqout_tick = 5000;
    Path p;
    p.activations.push_back(act);
    return p;
}

// The one row build_activations makes for `p`.
ActivationRowView only_row(const Path& p) {
    HydraRecord rec;
    const ActivationsView v = build_activations(p, rec, nullptr);
    REQUIRE(v.acts.size() == 1);
    return v.acts[0];
}

// The scale line names `side` with a multiplier that never reads 1 and is
// `r` at the two decimals it always prints.
void check_names_scale(const std::string& line, const std::string& side, double r) {
    CAPTURE(line);
    CAPTURE(side);
    const std::optional<double> printed = printed_scale(line, side);
    REQUIRE(printed.has_value());
    CHECK(*printed != 1.0);
    CHECK(same_at_two_decimals(*printed, r));
}

}  // namespace

// A scale a thousandth or more from 1 (is_scaled, D256) is named with enough
// decimals never to read x1.00; a scale nearer 1 is no scale.
TEST_CASE("build_activations: a near-1 multiplier prints its decimals and its row's figure") {
    SUBCASE("a scale that reads x1.00 at two decimals prints more, and its row's figure") {
        const double early = 0.9973;
        REQUIRE(is_scaled(early));
        REQUIRE(reads_one_at_two_decimals(early));
        const Path p = near_one_path(TransferScale{early, kIdentityScale});
        const ActivationRowView av = only_row(p);
        check_names_scale(av.scale_warning, "early", early);
        CHECK_FALSE(printed_scale(av.scale_warning, "late").has_value());
        CHECK(av.scale_warn);
        const ActivationRating rate = rate_activation(p.activations[0]);
        REQUIRE(rate.backends.size() == 1);
        REQUIRE(av.backends.size() == 1);
        const NoteRating& note = rate.backends[0].note;
        REQUIRE(note.effective_ms.has_value());
        CHECK(av.backends[0].rating.find("(eff. " + format_ms(*note.effective_ms) + ")") != std::string::npos);
        const std::string& tip = av.backends[0].tooltip;
        CHECK(tip.find(format_ms(nominal_budget_ms())) != std::string::npos);
        CHECK(tip.find(format_ms(note.budget_ms)) != std::string::npos);
        // The squeeze-out's figure lives on its row only.
        REQUIRE(av.squeeze_sentences.size() == 1);
        CHECK(av.squeeze_sentences[0].text.find("eff.") == std::string::npos);
    }
    // Just past the tolerance on each side, the line names both sides when a
    // second row past the SP end reads the late side, early first.
    const double just_above = kIdentityScale + kScaleIdentityTolerance + 0.0001;
    const double just_below = kIdentityScale - kScaleIdentityTolerance - 0.0001;
    SUBCASE("both sides just past the tolerance both print") {
        REQUIRE(is_scaled(just_above));
        REQUIRE(is_scaled(just_below));
        Path p = near_one_path(TransferScale{just_above, just_below});
        p.activations[0].backends.push_back(backend_at(50.0));
        const ActivationRowView av = only_row(p);
        check_names_scale(av.scale_warning, "early", just_above);
        check_names_scale(av.scale_warning, "late", just_below);
        CHECK(av.scale_warning.find("(early)") < av.scale_warning.find("(late)"));
    }
    SUBCASE("with only the row inside SP, the late side is left out") {
        const ActivationRowView av = only_row(near_one_path(TransferScale{just_above, just_below}));
        check_names_scale(av.scale_warning, "early", just_above);
        CHECK_FALSE(printed_scale(av.scale_warning, "late").has_value());
    }
    SUBCASE("a scale exactly the tolerance from 1 is scaled") {
        const double at_tolerance = kIdentityScale + kScaleIdentityTolerance;
        REQUIRE(is_scaled(at_tolerance));
        const ActivationRowView av = only_row(near_one_path(TransferScale{at_tolerance, kIdentityScale}));
        check_names_scale(av.scale_warning, "early", at_tolerance);
    }
    SUBCASE("a scale nearer 1 than the tolerance is no scale") {
        // Tapestry of the Starless Abstract's late scale from whole-microsecond
        // MIDI tempos (D256).
        const double tapestry_late = 1.00000175;
        REQUIRE_FALSE(is_scaled(tapestry_late));
        Path p = near_one_path(TransferScale{kIdentityScale, tapestry_late});
        p.activations[0].backends.push_back(backend_at(50.0));
        const ActivationRowView av = only_row(p);
        CHECK(av.scale_warning.empty());
        CHECK_FALSE(av.scale_warn);
        REQUIRE(av.backends.size() == 2);
        for (const BackendRowView& row : av.backends) CHECK(row.rating.find("eff.") == std::string::npos);
    }
}

// D4: an activation with no stored scale is a bug the row shows rather than
// hides. Fresh records never reach it (the corpus case above requires a stored
// scale on every activation), so this builds the state by hand.
TEST_CASE("build_activations: an unknown scale says so and shows no eff.") {
    HydraRecord rec;
    Activation act = scale_base();
    act.backends.push_back(backend_at(-40.0));
    Path p;
    p.activations.push_back(act);

    ActivationsView v = build_activations(p, rec, nullptr);
    REQUIRE(v.acts.size() == 1);
    CHECK(v.acts[0].scale_warning == "Transfer scale unknown.");
    CHECK(v.acts[0].scale_warn);  // Q6: orange
    REQUIRE(v.acts[0].backends.size() == 1);
    CHECK(v.acts[0].backends[0].rating.find("eff.") == std::string::npos);
}

// Each note's words are ChordNote::str's, in the Pro Drums setting's words
// (D48 Q11, finding 17). On every corpus activation the row names each of its
// chord's notes in order, in the words of the setting it was built with, and
// the setting changes the row exactly when the chord has a pad that is not a
// cymbal (a snare or a tom).
TEST_CASE("activation rows: the chord names its notes in the Pro Drums setting's words") {
    int rows_the_setting_changes = 0;
    int rows_the_setting_keeps = 0;
    for_each_corpus_path([&](const std::string& chart, const Song& /*song*/, const HydraRecord& rec,
                             const Path& path) {
        CAPTURE(chart);
        CAPTURE(path.pathstring());
        const std::vector<Activation> acts = path.all_activations();
        const ActivationsView pro = build_activations(path, rec, nullptr, std::nullopt,
                                                      core::default_rules(), std::nullopt,
                                                      /*pro_drums=*/true);
        const ActivationsView plain = build_activations(path, rec, nullptr, std::nullopt,
                                                        core::default_rules(), std::nullopt,
                                                        /*pro_drums=*/false);
        REQUIRE(pro.acts.size() == acts.size());
        REQUIRE(plain.acts.size() == acts.size());
        for (size_t i = 0; i < acts.size(); ++i) {
            CAPTURE(i);
            const std::vector<ChordNote> notes = acts[i].chord.notes();
            REQUIRE_FALSE(notes.empty());
            for (const bool pro_drums : {true, false}) {
                CAPTURE(pro_drums);
                const std::string& row = (pro_drums ? pro : plain).acts[i].chord;
                CAPTURE(row);
                size_t from = 0;
                for (const ChordNote& n : notes) {
                    const std::string word = n.str(pro_drums);
                    const size_t at = row.find(word, from);
                    CHECK(at != std::string::npos);
                    if (at != std::string::npos) from = at + word.size();
                }
            }
            // A note whose own name changes with the setting (a snare or a
            // tom, by note_label).
            bool has_drum_pad = false;
            for (const ChordNote& n : notes)
                has_drum_pad = has_drum_pad || n.str(true) != n.str(false);
            CHECK((pro.acts[i].chord != plain.acts[i].chord) == has_drum_pad);
            ++(has_drum_pad ? rows_the_setting_changes : rows_the_setting_keeps);
        }
    });
    CHECK(rows_the_setting_changes > 0);
    CHECK(rows_the_setting_keeps > 0);
}

// ---- the Dynamic Hit Window row ---------------------------------------------
// The figures are the ones hydra_replay schedule prints.

namespace {

// A corpus chart analyzed the way burnout() analyzes it
// (song_details_fixtures.h), once per test run, with the early-fill window
// at kHitWindowMs: the window these rows' paths and figures were pinned
// under (Get Ur Freak On's E marks need it).
const AnalysisResult& analyzed_like_the_gui(const std::string& name) {
    static std::map<std::string, AnalysisResult> cache;
    auto it = cache.find(name);
    if (it == cache.end()) {
        const std::string path = corpus::chart_named(name);
        if (path.empty()) throw std::runtime_error(name + " is not in testdata/input");
        AnalysisSettings settings = scratch_settings().to_analysis_settings();
        settings.early_fill_window_ms = kHitWindowMs;
        it = cache.emplace(name, analyze_chart_file(path, settings)).first;
    }
    return it->second;
}

const AnalysisResult& tapped_out() { return analyzed_like_the_gui("Mike Orlando - Tapped Out"); }
const AnalysisResult& focus() { return analyzed_like_the_gui("SoundHaven - Focus [Ariana Grande]"); }
const AnalysisResult& epidermis() { return analyzed_like_the_gui("Venetian Snares - Epidermis"); }

// `path`'s rows as the Paths tab builds them, with `chords` (null for none).
ActivationsView window_rows_of(const AnalysisResult& ar, const Path& path, const ChordTimes* chords) {
    const SongTiming& timing = ar.song.timing();
    return build_activations(path, ar.record, &timing, std::nullopt, core::default_rules(),
                             std::nullopt, /*pro_drums=*/true, chords);
}

// The place of `act`'s only squeeze of `kind`.
size_t only_squeeze(const Activation& act, SqueezeKind kind) {
    std::optional<size_t> found;
    for (size_t k = 0; k < act.sqinouts.size(); ++k)
        if (act.sqinouts[k].kind == kind) {
            REQUIRE_FALSE(found.has_value());
            found = k;
        }
    REQUIRE(found.has_value());
    return *found;
}

// Every field of two rows but the Dynamic Hit Window rows.
void check_same_but_windows(const ActivationRowView& a, const ActivationRowView& b) {
    CHECK(a.number == b.number);
    CHECK(a.notation == b.notation);
    CHECK(a.measure == b.measure);
    CHECK(a.sp_bars == b.sp_bars);
    CHECK(a.bars == b.bars);
    REQUIRE(a.badges.size() == b.badges.size());
    for (size_t i = 0; i < a.badges.size(); ++i) {
        CHECK(a.badges[i].text == b.badges[i].text);
        CHECK(a.badges[i].warn == b.badges[i].warn);
    }
    CHECK(a.difficult == b.difficult);
    CHECK(a.song_fraction == b.song_fraction);
    CHECK(a.chord == b.chord);
    CHECK(a.early_fill == b.early_fill);
    REQUIRE(a.squeeze_sentences.size() == b.squeeze_sentences.size());
    for (size_t i = 0; i < a.squeeze_sentences.size(); ++i) {
        CHECK(a.squeeze_sentences[i].text == b.squeeze_sentences[i].text);
        CHECK(a.squeeze_sentences[i].warn == b.squeeze_sentences[i].warn);
    }
    CHECK(a.sqin_scale_warning == b.sqin_scale_warning);
    CHECK(a.scale_warning == b.scale_warning);
    CHECK(a.scale_warn == b.scale_warn);
    CHECK(a.overfill_warning == b.overfill_warning);
    CHECK(a.backends_label == b.backends_label);
    REQUIRE(a.backends.size() == b.backends.size());
    for (size_t i = 0; i < a.backends.size(); ++i) {
        CHECK(a.backends[i].timing == b.backends[i].timing);
        CHECK(a.backends[i].tooltip == b.backends[i].tooltip);
        CHECK(a.backends[i].chord == b.backends[i].chord);
        CHECK(a.backends[i].points == b.backends[i].points);
        CHECK(a.backends[i].rating == b.backends[i].rating);
        CHECK(a.backends[i].warn == b.backends[i].warn);
    }
    CHECK(a.backends_hidden_line == b.backends_hidden_line);
}

// How many squeeze-out rows of `ar`'s best path the owner calls short, and
// how many safe. Each row is checked against the owner's ends
// (squeeze_window_ends): the anchor first, then the squeezed note, each
// figure its end's side and each chord the chord on its end's tick, and the
// tag the safe sum's size.
struct SqOutRowCounts {
    int short_rows = 0;
    int safe_rows = 0;
};
SqOutRowCounts sqout_rows_against_owner(const AnalysisResult& ar) {
    const Path& best = ar.record.best_path();
    const ChordTimes chords = chord_times(ar.song);
    const std::vector<Activation> acts = best.all_activations();
    const ActivationsView view = window_rows_of(ar, best, &chords);
    REQUIRE(view.acts.size() == acts.size());
    SqOutRowCounts counts;
    for (size_t i = 0; i < acts.size(); ++i) {
        const Activation& act = acts[i];
        REQUIRE(view.acts[i].squeeze_windows.size() == act.sqinouts.size());
        for (size_t k = 0; k < act.sqinouts.size(); ++k) {
            if (act.sqinouts[k].kind != SqueezeKind::SqOut) continue;
            if (!view.acts[i].squeeze_windows[k]) continue;
            const WindowRowView& row = *view.acts[i].squeeze_windows[k];
            CAPTURE(i);
            CAPTURE(row.windows);
            const std::optional<WindowEnds> ends = squeeze_window_ends(act, k, chords);
            REQUIRE(ends.has_value());
            REQUIRE(ends->ends.size() == 2);
            REQUIRE(row.figures.size() == 2);
            CHECK(std::optional<int64_t>(ends->ends[0].tick) == act.squeeze_anchor_tick(k));
            CHECK(std::optional<int64_t>(ends->ends[1].tick) == act.squeeze_chord_tick(k));
            for (size_t e = 0; e < 2; ++e) {
                const std::optional<int> c = chords.at(ends->ends[e].tick);
                REQUIRE(c.has_value());
                CHECK(row.figures[e].figure == format_ms(ends->ends[e].side_ms));
                CHECK(row.figures[e].chord == chords.chords[static_cast<size_t>(*c)].notationstr());
            }
            CHECK(row.tag.rfind(format_ms(std::fabs(ends->safe_ms)), 0) == 0);
            CHECK(row.is_short == ends->is_short());
            ++(ends->is_short() ? counts.short_rows : counts.safe_rows);
        }
    }
    return counts;
}

}  // namespace

// The label above the row starts with the words the user chose for it
// (ui-and-display.md, Confirmed: "change Clone Hero to Dynamic Hit Window and
// mock it up."). The Paths tab reads it from this one constant.
TEST_CASE("dynamic hit window: the label reads the user's words") {
    CHECK(std::string(kDynamicHitWindowLabel).rfind("Dynamic Hit Window", 0) == 0);
}

// Tapped Out's best path has a squeeze out whose windows sum below zero.
TEST_CASE("dynamic hit window: Tapped Out's squeeze out is out of reach") {
    const SqOutRowCounts counts = sqout_rows_against_owner(tapped_out());
    CHECK(counts.short_rows >= 1);
}

// Burnout's best path has a squeeze out whose windows sum to zero or more,
// and an activation with no timing badge gets no row.
TEST_CASE("dynamic hit window: Burnout's squeeze out is safe") {
    const AnalysisResult& ar = burnout();
    const SqOutRowCounts counts = sqout_rows_against_owner(ar);
    CHECK(counts.safe_rows >= 1);

    const Path& best = ar.record.best_path();
    const ChordTimes chords = chord_times(ar.song);
    const ActivationsView view = window_rows_of(ar, best, &chords);
    int unbadged = 0;
    for (const ActivationRowView& av : view.acts) {
        if (!av.badges.empty()) continue;
        ++unbadged;
        for (const std::optional<WindowRowView>& w : av.squeeze_windows)
            CHECK_FALSE(w.has_value());
        CHECK_FALSE(av.early_fill_window.has_value());
    }
    CHECK(unbadged >= 1);
}

namespace {

// A row's ends are the owner's, in its order: each figure its end's side
// and each chord the chord on its end's tick; the tag starts with the size
// of the owner's sum, and the row is short exactly when the owner says so.
void check_row_ends(const WindowRowView& row, const WindowEnds& ends, const ChordTimes& chords) {
    CAPTURE(row.windows);
    REQUIRE(row.figures.size() == ends.ends.size());
    for (size_t e = 0; e < ends.ends.size(); ++e) {
        CAPTURE(e);
        CHECK(row.figures[e].figure == format_ms(ends.ends[e].side_ms));
        const std::optional<int> c = chords.at(ends.ends[e].tick);
        REQUIRE(c.has_value());
        CHECK(row.figures[e].chord == chords.chords[static_cast<size_t>(*c)].notationstr());
    }
    CHECK(row.tag.rfind(format_ms(std::fabs(ends.safe_ms)), 0) == 0);
    CHECK(row.is_short == ends.is_short());
}

// The early fill entry of `act`'s badge_timings.
ActivationTiming fill_timing(const Activation& act) {
    std::optional<ActivationTiming> found;
    for (const ActivationTiming& t : act.badge_timings())
        if (t.part == TimingPart::EarlyFill) found = t;
    REQUIRE(found.has_value());
    return *found;
}

}  // namespace

// Every early fill on Tapped Out's best path (an E0 fill and an optional one
// after a skip, the rows the edited-figure case below starts from) gets one
// row with one end and no word: the owner's SP-ready end
// (early_fill_window_end). Its details line names the fill's figure and
// whether it is required.
TEST_CASE("dynamic hit window: an early fill gets one window, required or optional") {
    const AnalysisResult& ar = tapped_out();
    const Path& best = ar.record.best_path();
    const ChordTimes chords = chord_times(ar.song);
    const std::vector<Activation> acts = best.all_activations();
    const ActivationsView view = window_rows_of(ar, best, &chords);
    REQUIRE(view.acts.size() == acts.size());
    int required = 0;
    int optional_fills = 0;
    for (size_t i = 0; i < acts.size(); ++i) {
        const Activation& act = acts[i];
        const ActivationRowView& av = view.acts[i];
        CAPTURE(av.number);
        if (!act.is_e_critical()) {
            CHECK_FALSE(av.early_fill_window.has_value());
            continue;
        }
        CAPTURE(av.early_fill);
        CHECK(av.early_fill.find(figure_of(fill_timing(act))) != std::string::npos);
        CHECK(av.early_fill.find(act.is_E0() ? "(required)" : "(optional)") != std::string::npos);
        REQUIRE(av.early_fill_window.has_value());
        const std::optional<WindowEnds> ends = early_fill_window_end(act, chords);
        REQUIRE(ends.has_value());
        REQUIRE(ends->ends.size() == 1);
        CHECK(std::optional<int64_t>(ends->ends[0].tick) == act.sp_ready_tick());
        check_row_ends(*av.early_fill_window, *ends, chords);
        CHECK(av.early_fill_window->figures[0].word.empty());
        ++(act.is_E0() ? required : optional_fills);
    }
    CHECK(required > 0);
    CHECK(optional_fills > 0);
}

// A squeeze in that needs no timing still gets its row, its ends the
// owner's: the anchor the frontend, the squeezed note the backend. Get Ur
// Freak On's best path holds one (its second activation's squeeze in).
TEST_CASE("dynamic hit window: a squeeze in that needs no timing still gets its row") {
    const AnalysisResult& ar = analyzed_like_the_gui("Eric Moore - Get Ur Freak On");
    const Path& best = ar.record.best_path();
    const ChordTimes chords = chord_times(ar.song);
    const std::vector<Activation> acts = best.all_activations();
    const ActivationsView view = window_rows_of(ar, best, &chords);
    REQUIRE(view.acts.size() == acts.size());
    int free_sqins = 0;
    for (size_t i = 0; i < acts.size(); ++i) {
        const Activation& act = acts[i];
        CAPTURE(i);
        REQUIRE(view.acts[i].squeeze_windows.size() == act.sqinouts.size());
        for (size_t k = 0; k < act.sqinouts.size(); ++k) {
            const SPSqueeze& sq = act.sqinouts[k];
            if (sq.kind != SqueezeKind::SqIn || !sq.needs_no_timing()) continue;
            REQUIRE(view.acts[i].squeeze_windows[k].has_value());
            const WindowRowView& row = *view.acts[i].squeeze_windows[k];
            const std::optional<WindowEnds> ends = squeeze_window_ends(act, k, chords);
            REQUIRE(ends.has_value());
            REQUIRE(ends->ends.size() == 2);
            CHECK(std::optional<int64_t>(ends->ends[0].tick) == act.squeeze_anchor_tick(k));
            CHECK(std::optional<int64_t>(ends->ends[1].tick) == act.squeeze_chord_tick(k));
            check_row_ends(row, *ends, chords);
            CHECK(row.figures[0].word == "frontend");
            CHECK(row.figures[1].word == "backend");
            ++free_sqins;
        }
    }
    CHECK(free_sqins > 0);
}

// Every timing badge gets its row, so edited figures on either side of the
// needs-hitting line all get one, and the edit moves only what it should.
TEST_CASE("dynamic hit window: a row for every timing badge, edited figures too") {
    SUBCASE("a squeeze in at 0.0 ms and at -0.1 ms each gets the unedited row") {
        // Focus's second activation has a squeeze in with a timing. At 0.0
        // it still has one (its note pressed early); at -0.1 it needs none
        // (pressed late, squeeze_presses). The row names the same ends either
        // way.
        const AnalysisResult& ar = focus();
        const ChordTimes chords = chord_times(ar.song);
        Path p;
        p.activations = ar.record.best_path().all_activations();
        REQUIRE(p.activations.size() >= 2);
        const size_t k = only_squeeze(p.activations[1], SqueezeKind::SqIn);
        const std::optional<WindowRowView> unedited = window_rows_of(ar, p, &chords).acts[1].squeeze_windows[k];
        REQUIRE(unedited.has_value());
        for (const double offset : {0.0, -0.1}) {
            CAPTURE(offset);
            p.activations[1].sqinouts[k].offset_ms = offset;
            REQUIRE(timing_needs_hitting(p.activations[1].sqinouts[k].difficulty()) == (offset >= 0.0));
            const std::optional<WindowRowView> row = window_rows_of(ar, p, &chords).acts[1].squeeze_windows[k];
            REQUIRE(row.has_value());
            CHECK(row->windows == unedited->windows);
        }
    }
    SUBCASE("a required early fill with time to spare adds the spare to its sum") {
        // Tapped Out's third activation is E0 (the case above).
        const AnalysisResult& ar = tapped_out();
        const ChordTimes chords = chord_times(ar.song);
        Path p;
        p.activations = ar.record.best_path().all_activations();
        REQUIRE(p.activations.size() >= 3);
        REQUIRE(p.activations[2].is_E0());
        const std::optional<WindowEnds> unedited = early_fill_window_end(p.activations[2], chords);
        REQUIRE(unedited.has_value());
        REQUIRE(unedited->ends.size() == 1);
        const double spare = 20.0;  // Star Power ready 20 ms before the deadline
        Activation& act = p.activations[2];
        act.e_offset = spare;
        REQUIRE(act.is_E0());
        const ActivationRowView av = window_rows_of(ar, p, &chords).acts[2];
        CHECK(av.early_fill.find(figure_of(fill_timing(act))) != std::string::npos);
        CHECK(av.early_fill.find("(required)") != std::string::npos);
        REQUIRE(av.early_fill_window.has_value());
        REQUIRE(av.early_fill_window->figures.size() == 1);
        // The SP-ready note and its side are unedited; the sum is that side
        // plus the spare.
        CHECK(av.early_fill_window->figures[0].figure == format_ms(unedited->ends[0].side_ms));
        CHECK(av.early_fill_window->tag.rfind(format_ms(unedited->ends[0].side_ms + spare), 0) == 0);
        CHECK_FALSE(av.early_fill_window->is_short);
    }
    SUBCASE("the same early fill made optional gets the unedited row") {
        const AnalysisResult& ar = tapped_out();
        const ChordTimes chords = chord_times(ar.song);
        Path p;
        p.activations = ar.record.best_path().all_activations();
        REQUIRE(p.activations.size() >= 3);
        REQUIRE(p.activations[2].is_E0());
        const std::optional<WindowRowView> unedited = window_rows_of(ar, p, &chords).acts[2].early_fill_window;
        REQUIRE(unedited.has_value());
        test::set_skips(p.activations[2], 1);
        REQUIRE_FALSE(p.activations[2].is_E0());
        // A skip changes neither the SP-ready note nor the figure.
        const ActivationRowView av = window_rows_of(ar, p, &chords).acts[2];
        CHECK(av.early_fill.find("(optional)") != std::string::npos);
        REQUIRE(av.early_fill_window.has_value());
        CHECK(av.early_fill_window->windows == unedited->windows);
        CHECK(av.early_fill_window->tag == unedited->tag);
    }
}

TEST_CASE("dynamic hit window: no chord times, no row anywhere") {
    for (const AnalysisResult* ar : {&tapped_out(), &burnout(), &focus(), &epidermis()}) {
        const ActivationsView view = window_rows_of(*ar, ar->record.best_path(), nullptr);
        REQUIRE_FALSE(view.acts.empty());
        for (const ActivationRowView& av : view.acts) {
            CHECK(av.squeeze_windows.size() == av.squeeze_sentences.size());
            for (const std::optional<WindowRowView>& w : av.squeeze_windows)
                CHECK_FALSE(w.has_value());
            CHECK_FALSE(av.early_fill_window.has_value());
        }
    }
}

TEST_CASE("dynamic hit window: the row sits alongside and changes no other field") {
    int rows_shown = 0;
    for (const AnalysisResult* ar : {&tapped_out(), &burnout(), &focus(), &epidermis()}) {
        const Path& best = ar->record.best_path();
        const ChordTimes chords = chord_times(ar->song);
        const ActivationsView with = window_rows_of(*ar, best, &chords);
        const ActivationsView without = window_rows_of(*ar, best, nullptr);
        CHECK(with.summary == without.summary);
        CHECK(with.timeline_start == without.timeline_start);
        CHECK(with.timeline_end == without.timeline_end);
        REQUIRE(with.acts.size() == without.acts.size());
        for (size_t i = 0; i < with.acts.size(); ++i) {
            check_same_but_windows(with.acts[i], without.acts[i]);
            for (const std::optional<WindowRowView>& w : with.acts[i].squeeze_windows)
                rows_shown += w.has_value() ? 1 : 0;
            rows_shown += with.acts[i].early_fill_window.has_value() ? 1 : 0;
        }
    }
    // The comparison covers rows that show, not only rows that do not: at
    // least the five the cases above pin on these charts.
    CHECK(rows_shown >= 5);
}

namespace {

// What every corpus row is checked for, whatever its kind. `safe_ms` is the
// owner's sum for the row. The tag is its size with no minus sign, then
// " ms short" exactly when the sum is below 0.0 and " ms safe" otherwise
// (the user's ruling of 2026-11-13: "xx ms safe >=0 ms, xx ms short <0ms").
// The ends hold no direction word and no tag word.
void check_any_row(const WindowRowView& row, double safe_ms, int& short_tags, int& safe_tags) {
    CAPTURE(row.windows);
    CAPTURE(row.tag);
    CHECK(row.tag.find('-') == std::string::npos);
    const bool below_zero = safe_ms < 0.0;
    CHECK(row.is_short == below_zero);
    const std::string word = below_zero ? " ms short" : " ms safe";
    REQUIRE(row.tag.size() > word.size());
    CHECK(row.tag.compare(row.tag.size() - word.size(), word.size(), word) == 0);
    ++(below_zero ? short_tags : safe_tags);
    for (const char* banned : {"early", "late", "spare", "short", "safe"})
        CHECK(row.windows.find(banned) == std::string::npos);
    REQUIRE_FALSE(row.figures.empty());
}

}  // namespace

// Every row on every kept path of every corpus chart: rows equal badges, an
// early fill row has one end and no word, the tag's word follows the sum's
// sign, and a squeeze row names its anchor the frontend and its squeezed note
// the backend, the anchor first, whichever way its sentence says to press.
TEST_CASE("dynamic hit window: every corpus row") {
    int squeeze_rows = 0;
    int fill_rows = 0;
    int short_tags = 0;
    int safe_tags = 0;
    // Squeeze rows whose sentence presses the squeezed note early and the
    // anchor late: the rows a label by press direction would switch.
    int note_pressed_early = 0;
    for_each_corpus_chart([&](const std::string& /*chart*/, const Song& song, const HydraRecord& rec) {
        const ChordTimes chords = chord_times(song);
        const SongTiming& timing = song.timing();
        for (const Path& path : rec.paths) {
            const std::vector<Activation> acts = path.all_activations();
            const ActivationsView view = build_activations(path, rec, &timing, std::nullopt,
                                                           core::default_rules(), std::nullopt,
                                                           /*pro_drums=*/true, &chords);
            REQUIRE(view.acts.size() == acts.size());
            for (size_t i = 0; i < acts.size(); ++i) {
                const Activation& act = acts[i];
                const ActivationRowView& av = view.acts[i];
                CAPTURE(path.pathstring());
                CAPTURE(av.number);

                // Every timing badge has its row (the user's ruling of
                // 2026-11-12).
                size_t shown = av.early_fill_window ? 1 : 0;
                for (const std::optional<WindowRowView>& w : av.squeeze_windows)
                    shown += w ? 1 : 0;
                CHECK(shown == av.badges.size());

                if (av.early_fill_window) {
                    const std::optional<WindowEnds> ends = early_fill_window_end(act, chords);
                    REQUIRE(ends.has_value());
                    check_any_row(*av.early_fill_window, ends->safe_ms, short_tags, safe_tags);
                    REQUIRE(av.early_fill_window->figures.size() == 1);
                    CHECK(av.early_fill_window->figures[0].word.empty());
                    CHECK(av.early_fill_window->windows.rfind(av.early_fill_window->figures[0].figure, 0) == 0);
                    ++fill_rows;
                }

                REQUIRE(av.squeeze_windows.size() == act.sqinouts.size());
                REQUIRE(av.squeeze_sentences.size() == act.sqinouts.size());
                for (size_t k = 0; k < act.sqinouts.size(); ++k) {
                    if (!av.squeeze_windows[k]) continue;
                    const WindowRowView& row = *av.squeeze_windows[k];
                    const std::optional<WindowEnds> ends = squeeze_window_ends(act, k, chords);
                    REQUIRE(ends.has_value());
                    check_any_row(row, ends->safe_ms, short_tags, safe_tags);
                    REQUIRE(row.figures.size() == 2);
                    CHECK(row.figures[0].word == "frontend");
                    CHECK(row.figures[1].word == "backend");
                    ++squeeze_rows;

                    // The view keeps the owner's order, and the owner names
                    // each end's tick: the anchor first, the squeezed note
                    // second.
                    REQUIRE(ends->ends.size() == 2);
                    CHECK(std::optional<int64_t>(ends->ends[0].tick) == act.squeeze_anchor_tick(k));
                    CHECK(std::optional<int64_t>(ends->ends[1].tick) == act.squeeze_chord_tick(k));

                    // The sentence's word for the squeezed note, read from
                    // the built view. It never moves the row's words.
                    const std::string& sentence = av.squeeze_sentences[k].text;
                    CAPTURE(sentence);
                    const bool says_late = sentence.find(" ms late") != std::string::npos;
                    const bool says_early = sentence.find(" ms early") != std::string::npos;
                    REQUIRE(says_late != says_early);
                    if (says_early) ++note_pressed_early;
                }
            }
        }
    });
    CHECK(squeeze_rows > 0);
    CHECK(fill_rows > 0);
    // Both tag words occur (Tapped Out's squeeze out is short, Burnout's safe).
    CHECK(short_tags > 0);
    CHECK(safe_tags > 0);
    // At least one squeeze presses its note early (Focus's and Epidermis's
    // squeeze ins, above), so a row that named its ends by press direction
    // fails here.
    CHECK(note_pressed_early > 0);
}
