// Tests for app/path_view's path list on the Song Details screen: the path
// buttons and their groups, the list and its all-0 dedupe, path identity, the
// score breakdown and multiplier-squeeze panels, and the PathsTabCache and
// PathsTabUi that build and fold them. The cases read what each panel shows
// from the engine's record on corpus charts. The activation rows are in
// test_activation_rows.cpp.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/display_format.h"
#include "app/path_view.h"
#include "app/preview_view.h"  // path_overlay_key
#include "core/model.h"
#include "core/timing.h"  // kSpActivationBars
#include "search/pather.h"  // best_allzero_paths, search_allzero
#include "analyzed_chart.h"
#include "record_fixtures.h"
#include "song_details_fixtures.h"

using namespace hydra;
using namespace hydra::app;
using song_details::burnout;

namespace {

// Every corpus chart's record under the GUI tests' settings (scratch_settings,
// the settings burnout() is analyzed with), at `sp_cap` bars when given. A
// chart with no notes is passed over, as test_activation_rows' walk does; the
// walk fails when no chart is left.
template <class Visit>
void for_each_corpus_record(Visit&& visit, std::optional<int> sp_cap = std::nullopt) {
    AnalysisSettings cfg = scratch_settings().to_analysis_settings();
    if (sp_cap) cfg.sp_cap = *sp_cap;
    int charts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (song.is_empty()) continue;
        ++charts;
        visit(corpus::analyzed(chart, cfg));
    }
    REQUIRE(charts > 0);
}

// The path buttons the Paths tab builds for `rec` under the same settings.
PathButtonsView buttons_of(const HydraRecord& rec) {
    const Settings s = scratch_settings();
    return build_path_buttons(rec, s.depth_mode, s.depth_value);
}

// One squeeze row against the squeeze it shows, in the words of `pro_drums`:
// its notation first, its gain, its chord's own Chord::rowstr last, and the
// squeeze's own howto.
void check_squeeze_row(const MultSqueezeView& row, const MultSqueeze& msq, bool pro_drums) {
    CAPTURE(row.label);
    const std::string chord = msq.chord().rowstr(pro_drums);
    CHECK(row.label.rfind(msq.notationstr(), 0) == 0);
    CHECK(row.label.find("+" + std::to_string(msq.gain())) != std::string::npos);
    REQUIRE(row.label.size() >= chord.size());
    CHECK(row.label.substr(row.label.size() - chord.size()) == chord);
    CHECK(row.howto == msq.howto(pro_drums));
    CHECK(row.gain == msq.gain());
}

}  // namespace

TEST_CASE("build_score_breakdown: every row reads its own field, values in one column") {
    for_each_corpus_record([](const HydraRecord& rec) {
        for (const Path* p : rec.all_paths()) {
            CAPTURE(p->pathstring());
            // The box's values in its rows' order, each through the formatter
            // the box shows it with.
            const std::vector<std::string> values = {
                format_avg_mult(p->avg_mult()) + "x", group_thousands(p->notes_points),
                group_thousands(p->score_combo),      group_thousands(p->score_sp),
                group_thousands(p->score_solo),       group_thousands(p->score_accents),
                group_thousands(p->score_ghosts),     group_thousands(p->totalscore())};
            const std::vector<std::string> lines = build_score_breakdown(*p);
            REQUIRE(lines.size() == values.size());
            CHECK(lines.front().rfind(std::string(kAverageMultiplierLabel) + ":", 0) == 0);

            // Each line is a label ending in ':', then spaces, then the value.
            // A blank line sets the notes and the total apart.
            std::vector<std::string> labels;
            std::vector<std::string> bodies;
            size_t longest = 0;
            for (size_t i = 0; i < lines.size(); ++i) {
                const bool blank_above = !lines[i].empty() && lines[i].front() == '\n';
                CHECK(blank_above == (i == 1 || i + 1 == lines.size()));
                const std::string body = lines[i].substr(blank_above ? 1 : 0);
                const size_t colon = body.find(':');
                REQUIRE(colon != std::string::npos);
                labels.push_back(body.substr(0, colon + 1));
                bodies.push_back(body);
                longest = (std::max)(longest, labels.back().size());
            }
            // Every value starts two columns after the longest label, and no
            // two rows share a label.
            const size_t column = longest + 2;
            for (size_t i = 0; i < bodies.size(); ++i) {
                CAPTURE(i);
                REQUIRE(bodies[i].size() >= column);
                CHECK(bodies[i].substr(labels[i].size(), column - labels[i].size()) ==
                      std::string(column - labels[i].size(), ' '));
                CHECK(bodies[i].substr(column) == values[i]);
                for (size_t j = 0; j < i; ++j) CHECK(labels[j] != labels[i]);
            }
        }
    });
}

TEST_CASE("path buttons: each path's own hardest timing, warn past the difficult floor") {
    for_each_corpus_record([](const HydraRecord& rec) {
        const PathButtonsView v = buttons_of(rec);
        for (const PathButtonView& b : v.buttons) {
            CAPTURE(b.notation);
            // The hardest timing among the path's activations; none for a path
            // with nothing to time.
            std::optional<double> hardest;
            for (const Activation& act : b.path->walk_activations())
                if (const std::optional<double> d = act.difficulty())
                    hardest = hardest ? (std::max)(*hardest, *d) : *d;
            if (!hardest) {
                CHECK(b.timing.empty());
                CHECK_FALSE(b.timing_warn);
                continue;
            }
            CHECK(b.timing == format_ms(*hardest));
            CHECK(b.timing_warn == past_difficult_floor(*hardest));
        }
    });
}

TEST_CASE("build_path_buttons: a cap too small to activate says why there are no activations") {
    // One bar short of what an activation needs: no path can activate, and
    // every button carries the one sentence that says so, naming the cap.
    const int low_cap = kSpActivationBars - 1;
    std::string said;
    for_each_corpus_record(
        [&](const HydraRecord& rec) {
            const PathButtonsView v = buttons_of(rec);
            REQUIRE_FALSE(v.buttons.empty());
            for (const PathButtonView& b : v.buttons) {
                CHECK(b.path->walk_activations().empty());
                CHECK(b.detail.find(std::to_string(low_cap) + "-bar cap") != std::string::npos);
                if (said.empty()) said = b.detail;
                CHECK(b.detail == said);
            }
        },
        low_cap);
    REQUIRE_FALSE(said.empty());

    // At the bars an activation needs, no button says it.
    for_each_corpus_record(
        [&](const HydraRecord& rec) {
            const PathButtonsView v = buttons_of(rec);
            for (const PathButtonView& b : v.buttons) CHECK(b.detail != said);
        },
        kSpActivationBars);
}

TEST_CASE("build_path_list: one group per run of equal scores, each path in exactly one") {
    for_each_corpus_record([](const HydraRecord& rec) {
        const PathListView list = build_path_list(rec);
        REQUIRE_FALSE(list.groups.empty());
        // Reading the groups in order gives back the traversal, path for path.
        std::vector<const Path*> regrouped;
        for (const PathGroupView& g : list.groups) {
            REQUIRE_FALSE(g.paths.empty());
            for (const Path* p : g.paths) {
                CHECK(g.score_label == group_thousands(p->totalscore()));
                regrouped.push_back(p);
            }
        }
        CHECK(regrouped == rec.all_paths());
        for (size_t i = 1; i < list.groups.size(); ++i)
            CHECK(list.groups[i].score_label != list.groups[i - 1].score_label);
    });
}

// The Preview's overlay key and the list's dedupe must agree on what "the
// same path" is. So the expected all-0 section is worked out here through
// path_overlay_key, and build_path_list's own must match it. A duplicate
// hides only itself (D51 call 4b).
TEST_CASE("build_path_list: an all-0 path shows exactly when no listed path has its overlay key") {
    int charts_showing = 0;
    for_each_corpus_record([&](const HydraRecord& rec) {
        const std::vector<const Path*> flat = rec.all_paths();
        std::vector<const Path*> expected;
        for (const Path* z : best_allzero_paths(rec)) {
            const std::string key = path_overlay_key(z);
            const bool listed = std::any_of(flat.begin(), flat.end(), [&](const Path* p) {
                return path_overlay_key(p) == key;
            });
            if (!listed) expected.push_back(z);
        }
        const PathListView list = build_path_list(rec);
        CHECK(list.allzero == expected);
        CHECK(list.show_allzero == !expected.empty());
        if (expected.empty()) return;
        ++charts_showing;
        CHECK(list.allzero_label == group_thousands(expected.front()->totalscore()));

        // Each shown all-0 path gets a button. One below the optimal score
        // names its shortfall; one that ties it has no detail line.
        const std::optional<int64_t> optimal = rec.optimal_score();
        REQUIRE(optimal.has_value());
        std::vector<const Path*> allzero_buttons;
        const PathButtonsView v = buttons_of(rec);
        for (const PathButtonView& b : v.buttons) {
            if (b.group != PathButtonView::Group::AllZero) continue;
            allzero_buttons.push_back(b.path);
            const int64_t shortfall = *optimal - b.path->totalscore();
            CHECK(shortfall >= 0);
            if (shortfall == 0)
                CHECK(b.detail.empty());
            else
                CHECK(b.detail.find(group_thousands(shortfall)) != std::string::npos);
        }
        CHECK(allzero_buttons == expected);
    });
    // Burnout shows one under these settings.
    CHECK(charts_showing > 0);
}

// The chart's best all-0 path has one reader, best_allzero_paths, and it shares
// its test (best_path_is_allzero) with analyze_chart's choice to skip the all-0
// pass. So on every corpus chart: when the reader names the best path, the pass
// was skipped; otherwise the reader gives the pass's paths, and a pass that left
// nothing is one that, run on the same graph, finds nothing.
TEST_CASE("best_allzero_paths: the best path when it is all-0, else the all-0 pass's paths") {
    const AnalysisSettings cfg = scratch_settings().to_analysis_settings();
    int from_pass = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        const std::vector<const Path*> got = best_allzero_paths(rec);
        for (const Path* p : got) CHECK(p->is_allzero());
        if (best_path_is_allzero(rec)) {
            CHECK(rec.allzero_paths.empty());
            CHECK(got == std::vector<const Path*>{&rec.best_path()});
            continue;
        }
        CHECK(got == rec.all_allzero_paths());
        if (!got.empty()) {
            ++from_pass;
            continue;
        }
        // Empty here must mean the chart has no all-0 path, not that the pass
        // was skipped on a test the reader does not share.
        CHECK(search_allzero(search_graph(song, cfg)).empty());
    }
    // Beg's best path is not all-0, and the pass finds one (the case below).
    CHECK(from_pass > 0);
}

TEST_CASE("best_allzero_paths: Beg's is 0 0 0 at 319,890; a record whose best is all-0 gets it") {
    // The figures the Paths tab showed in the V1 capture for Beg
    // (docs/audit/2026-11-14-test-audit/exits/v1-before, item 13; removed,
    // read it with `git show b2287e56:docs/audit/2026-11-14-test-audit/exits/v1-before`).
    const std::string chart = corpus::chart_named("Evans Blue - Beg");
    REQUIRE_FALSE(chart.empty());
    const HydraRecord& beg = corpus::analyzed(chart, scratch_settings().to_analysis_settings());
    REQUIRE_FALSE(beg.paths.empty());
    CHECK_FALSE(best_path_is_allzero(beg));
    const std::vector<const Path*> got = best_allzero_paths(beg);
    REQUIRE_FALSE(got.empty());
    CHECK(got.front()->pathstring() == "0 0 0");
    CHECK(got.front()->totalscore() == 319890);

    // The same record with that all-0 path as its best path, as analyze_chart
    // leaves a chart whose best path is all-0: the reader returns the best path
    // itself.
    HydraRecord best_is_allzero = beg;
    best_is_allzero.paths = {beg.allzero_paths.front()};
    best_is_allzero.allzero_paths.clear();
    REQUIRE(best_path_is_allzero(best_is_allzero));
    CHECK(best_allzero_paths(best_is_allzero) ==
          std::vector<const Path*>{&best_is_allzero.best_path()});
}

TEST_CASE("path_identity: engine paths share one exactly when score, starts and SP ends match") {
    for_each_corpus_record([](const HydraRecord& rec) {
        std::vector<const Path*> paths = rec.all_paths();
        const size_t listed = paths.size();
        const std::vector<const Path*> zero = rec.all_allzero_paths();
        paths.insert(paths.end(), zero.begin(), zero.end());

        // What makes two paths the same path: the score, and each activation's
        // start tick and SP end tick, in walk order.
        using Shape = std::vector<std::pair<int64_t, std::optional<int64_t>>>;
        std::vector<Shape> shapes;
        std::vector<std::string> ids;
        for (const Path* p : paths) {
            Shape s;
            for (const Activation& act : p->walk_activations())
                s.emplace_back(act.timecode.ticks(), act.deact_tick());
            shapes.push_back(std::move(s));
            ids.push_back(path_identity(*p));
            CHECK_FALSE(ids.back().empty());
        }
        for (size_t i = 0; i < paths.size(); ++i) {
            for (size_t j = 0; j < i; ++j) {
                CAPTURE(paths[i]->pathstring());
                CAPTURE(paths[j]->pathstring());
                const bool same = paths[i]->totalscore() == paths[j]->totalscore() &&
                                  shapes[i] == shapes[j];
                CHECK((ids[i] == ids[j]) == same);
                // The engine never lists one path twice.
                if (i < listed) CHECK(ids[i] != ids[j]);
            }
        }
    });
}

// Two answers to "are these the same path" exist: path_identity, which the
// Paths tab's all-0 dedupe and the Preview's overlay key read, and the
// notation text, which full_pin_mismatch compares. This case only finds out
// whether they ever disagree on a path the search returns. It picks no side:
// a failure names the chart (the captured corpus path) and both paths, for
// the user to decide which answer is right.
TEST_CASE("path_identity and pathstring: corpus paths share one exactly when they share the other") {
    for_each_corpus_record([](const HydraRecord& rec) {
        std::vector<const Path*> paths = rec.all_paths();
        const std::vector<const Path*> zero = rec.all_allzero_paths();
        paths.insert(paths.end(), zero.begin(), zero.end());

        std::vector<std::string> ids;
        std::vector<std::string> texts;
        for (const Path* p : paths) {
            ids.push_back(path_identity(*p));
            texts.push_back(p->pathstring());
        }
        for (size_t i = 0; i < paths.size(); ++i) {
            for (size_t j = 0; j < i; ++j) {
                const bool same_identity = ids[i] == ids[j];
                const bool same_text = texts[i] == texts[j];
                CHECK_MESSAGE(same_identity == same_text,
                              ("the two same-path tests disagree: path '" + texts[j] +
                               "' (identity " + ids[j] + ") and path '" + texts[i] +
                               "' (identity " + ids[i] + "): " +
                               (same_identity ? "same identity, different text"
                                              : "same text, different identity")));
            }
        }
    });
}

TEST_CASE("build_multsqueezes: one entry per squeeze, each read from that squeeze") {
    // build_multsqueezes words its rows with Pro Drums on.
    size_t squeezes = 0;
    for_each_corpus_record([&](const HydraRecord& rec) {
        const std::vector<MultSqueezeView> v = build_multsqueezes(rec);
        REQUIRE(v.size() == rec.multsqueezes.size());
        squeezes += v.size();
        for (size_t i = 0; i < v.size(); ++i) check_squeeze_row(v[i], rec.multsqueezes[i], true);
    });
    // Burnout has a multiplier squeeze (the Pro Drums test below needs it), so
    // the walk reads at least one.
    REQUIRE(squeezes > 0);
}

TEST_CASE("PathsTabCache: views are built once and rebuilt only when their inputs move") {
    // Burnout lists more than one path, so the cache can be moved to a second.
    const AnalysisResult& ar = burnout();
    const HydraRecord& rec = ar.record;
    const SongTiming& timing = ar.song.timing();
    const ChordTimes chords = chord_times(ar.song);
    PathsTabCache cache;

    // The buttons: once per record generation and score range, however many
    // frames ask.
    for (int frame = 0; frame < 5; ++frame) cache.buttons(rec, 7, 0, 10);
    CHECK(cache.buttons_builds() == 1);
    const PathButtonsView& list = cache.buttons(rec, 8, 0, 10);  // the record was re-read
    CHECK(cache.buttons_builds() == 2);
    // Every button carries its path's own notation.
    for (const PathButtonView& b : list.buttons) CHECK(b.notation == b.path->pathstring());
    cache.buttons(rec, 8, 0, 11);  // the score range moved
    CHECK(cache.buttons_builds() == 3);

    // The details: once per (path, record, backend limit).
    const Path& best = rec.best_path();
    for (int frame = 0; frame < 5; ++frame)
        cache.details(best, rec, 8, &timing, chords, std::nullopt, core::default_rules());
    CHECK(cache.details_builds() == 1);
    cache.details(best, rec, 8, &timing, chords, 30.0, core::default_rules());
    CHECK(cache.details_builds() == 2);
    cache.details(best, rec, 9, &timing, chords, 30.0, core::default_rules());
    CHECK(cache.details_builds() == 3);
    const std::vector<const Path*> all = rec.all_paths();
    REQUIRE(all.size() > 1);
    const Path& other = *all[1];
    REQUIRE(&other != &best);
    cache.details(other, rec, 9, &timing, chords, 30.0, core::default_rules());
    CHECK(cache.details_builds() == 4);

    // What the cache hands back after each move is the selected path's own,
    // never the one it held before: its activations, its score and the
    // record's squeezes.
    for (const Path* p : {&other, &best}) {
        CAPTURE(p->pathstring());
        const PathsTabCache::Details& d =
            cache.details(*p, rec, 9, &timing, chords, 30.0, core::default_rules());
        CHECK(d.activations.acts.size() == p->walk_activations().size());
        REQUIRE_FALSE(d.breakdown.empty());
        CHECK(d.breakdown.back().find(group_thousands(p->totalscore())) != std::string::npos);
        REQUIRE(d.squeezes.size() == rec.multsqueezes.size());
        for (size_t i = 0; i < d.squeezes.size(); ++i)
            check_squeeze_row(d.squeezes[i], rec.multsqueezes[i], true);
    }
}

TEST_CASE("path buttons: the listed paths in their score folds, then the all-0 path") {
    // Each button's timing is checked by "each path's own hardest timing", and
    // the all-0 button's shortfall by "an all-0 path shows exactly when".
    for_each_corpus_record([](const HydraRecord& rec) {
        const PathButtonsView v = buttons_of(rec);
        const std::vector<const Path*> listed = rec.all_paths();
        const PathListView list = build_path_list(rec);
        REQUIRE_FALSE(listed.empty());
        REQUIRE(v.buttons.size() == listed.size() + list.allzero.size());
        const std::optional<int64_t> optimal = rec.optimal_score();
        REQUIRE(optimal.has_value());
        CHECK(v.buttons.front().path == &rec.best_path());

        for (size_t i = 0; i < v.buttons.size(); ++i) {
            const PathButtonView& b = v.buttons[i];
            CAPTURE(b.notation);
            CHECK(b.notation == b.path->pathstring());
            if (i >= listed.size()) {
                // The all-0 path has no fold, so its score stays on the button.
                CHECK(b.path == list.allzero[i - listed.size()]);
                CHECK(b.group == PathButtonView::Group::AllZero);
                CHECK(b.fold.empty());
                CHECK(b.fold_count.empty());
                CHECK(b.title ==
                      group_thousands(b.path->totalscore()) + kFactSeparator + b.notation);
                continue;
            }
            // A listed path sits in its traversal place, in Optimal exactly
            // when it reaches the record's optimal score. Inside a fold the
            // score sits on the fold, which counts every path with that score.
            CHECK(b.path == listed[i]);
            CHECK(b.group == (b.path->totalscore() == *optimal ? PathButtonView::Group::Optimal
                                                               : PathButtonView::Group::Within));
            CHECK(b.fold == group_thousands(b.path->totalscore()));
            const int64_t sharing = std::count_if(listed.begin(), listed.end(), [&](const Path* p) {
                return p->totalscore() == b.path->totalscore();
            });
            CHECK(b.fold_count.rfind(group_thousands(sharing) + " ", 0) == 0);
            CHECK(b.title == b.notation);
        }
    });

    // The INI's int reaches the heading through Settings::search_depth_mode.
    const HydraRecord& rec = burnout().record;
    CHECK(build_path_buttons(rec, /*depth_mode=*/0, 2).within_label == "Within 2 scores");
    CHECK(build_path_buttons(rec, /*depth_mode=*/0, 1).within_label == "Within 1 score");
    CHECK(build_path_buttons(rec, /*depth_mode=*/1, 5000).within_label ==
          "Within 5,000 points");
    CHECK(build_path_buttons(rec, /*depth_mode=*/1, 1).within_label == "Within 1 point");

    // A top score tied between a root and its variant: both tied paths sit in
    // the Optimal group, and the lower root does not.
    const HydraRecord tied = test::tied_variant_record();
    PathButtonsView tv = build_path_buttons(tied, /*depth_mode=*/0, /*depth_value=*/2);
    REQUIRE(tv.buttons.size() == 3);
    CHECK(tv.buttons[0].path == &tied.paths.at(0));
    CHECK(tv.buttons[0].group == PathButtonView::Group::Optimal);
    CHECK(tv.buttons[1].path == &tied.paths.at(0).variants.at(0));
    CHECK(tv.buttons[1].group == PathButtonView::Group::Optimal);
    CHECK(tv.buttons[2].path == &tied.paths.at(1));
    CHECK(tv.buttons[2].group == PathButtonView::Group::Within);
    // The two tied paths share one fold that counts both; the lower root
    // starts its own.
    CHECK(tv.buttons[0].fold == tv.buttons[1].fold);
    CHECK(tv.buttons[0].fold_count == "2 paths");
    CHECK(tv.buttons[1].fold_count == "2 paths");
    CHECK(tv.buttons[2].fold != tv.buttons[0].fold);
    CHECK(tv.buttons[2].fold_count == "1 path");
}

TEST_CASE("multiplier squeeze: the fold's summary counts the squeezes and adds their gains") {
    CHECK(multsqueeze_summary({}) == "none");
    // Each record's squeezes, and three copies of its first: one squeeze shows
    // its gain alone, several show how many beside the sum of their gains.
    size_t summed = 0;
    for_each_corpus_record([&](const HydraRecord& rec) {
        const std::vector<MultSqueezeView> v = build_multsqueezes(rec);
        if (v.empty()) return;
        ++summed;
        int64_t total = 0;
        for (const MultSqueeze& msq : rec.multsqueezes) total += msq.gain();
        const std::string sum = "+" + group_thousands(total);
        CHECK(multsqueeze_summary(v) ==
              (v.size() == 1 ? sum : std::to_string(v.size()) + kFactSeparator + sum));
        const std::vector<MultSqueezeView> three(3, v.front());
        CHECK(multsqueeze_summary(three) ==
              std::to_string(three.size()) + kFactSeparator + "+" +
                  group_thousands(static_cast<int64_t>(three.size()) *
                                  rec.multsqueezes.front().gain()));
    });
    // Burnout has a multiplier squeeze (the Pro Drums test below needs it).
    REQUIRE(summed > 0);
}

TEST_CASE("multiplier squeeze: the note names follow the Pro Drums setting") {
    // The Paths tab builds its squeeze rows through the cache, which passes
    // the Pro Drums setting the record was analyzed with (D48 Q11, finding 17).
    const AnalysisResult& ar = burnout();
    const HydraRecord& rec = ar.record;
    const SongTiming& timing = ar.song.timing();
    const ChordTimes chords = chord_times(ar.song);
    auto squeezes_of = [&](bool pro_drums) {
        PathsTabCache cache;
        return cache.details(rec.best_path(), rec, 1, &timing, chords, std::nullopt,
                             core::default_rules(), std::nullopt, pro_drums).squeezes;
    };
    // Each row names its chord and its how-to in the words of the setting it
    // was given: the squeeze's own Chord::rowstr and howto under that setting.
    int worded_apart = 0;
    for (const bool pro_drums : {true, false}) {
        CAPTURE(pro_drums);
        const std::vector<MultSqueezeView> rows = squeezes_of(pro_drums);
        REQUIRE(rows.size() == rec.multsqueezes.size());
        for (size_t i = 0; i < rows.size(); ++i) {
            const MultSqueeze& msq = rec.multsqueezes[i];
            check_squeeze_row(rows[i], msq, pro_drums);
            if (pro_drums && msq.chord().rowstr(true) != msq.chord().rowstr(false)) ++worded_apart;
        }
    }
    // Burnout's squeeze is on the red pad, which Pro Drums calls a snare, so
    // the setting changes its words.
    CHECK(worded_apart > 0);
}

// The Effective figures switch starts off, and nothing that opens or closes
// rows, or starts a new path, moves it: it keeps the user's choice from path
// to path while the app runs.
TEST_CASE("PathsTabUi: the Effective figures switch starts off and outlives a new path") {
    PathsTabUi ui;
    CHECK_FALSE(ui.effective_figures);
    ui.reset(3);
    CHECK_FALSE(ui.effective_figures);
    ui.effective_figures = true;
    ui.reset(2);
    CHECK(ui.effective_figures);
    ui.set_all(true);
    CHECK(ui.effective_figures);
    ui.set_all(false);
    CHECK(ui.effective_figures);
    ui.click_row(1);
    CHECK(ui.effective_figures);
    ui.click_row(1);
    CHECK(ui.effective_figures);
}

TEST_CASE("PathsTabUi: one row open at a time, expand and collapse all") {
    PathsTabUi ui;
    // Both footer folds start open. No row does, but every backend table does,
    // so a row shows its table as soon as the user opens it.
    CHECK(ui.mult_open);
    CHECK(ui.breakdown_open);
    ui.reset(3);
    CHECK(ui.act_open == std::vector<char>{0, 0, 0});
    CHECK(ui.backends_open == std::vector<char>{1, 1, 1});
    CHECK_FALSE(ui.all_open());
    // A new path keeps the footer folds as the user left them.
    ui.mult_open = false;
    ui.reset(2);
    CHECK_FALSE(ui.mult_open);
    CHECK(ui.breakdown_open);
    ui.reset(3);

    ui.click_row(2);  // opens row 3 alone
    CHECK(ui.act_open == std::vector<char>{0, 0, 1});
    ui.click_row(2);  // closes it again
    CHECK(ui.act_open == std::vector<char>{0, 0, 0});

    ui.set_all(true);
    CHECK(ui.all_open());
    ui.click_row(1);  // an open row closes and leaves the others open
    CHECK(ui.act_open == std::vector<char>{1, 0, 1});
    ui.set_all(false);
    CHECK(ui.act_open == std::vector<char>{0, 0, 0});
    ui.click_row(7);  // past the end: nothing
    CHECK(ui.act_open == std::vector<char>{0, 0, 0});

    PathsTabUi empty;
    empty.reset(0);
    CHECK_FALSE(empty.all_open());
}

TEST_CASE("PathsTabCache: folds reset for a new path, not for a display setting") {
    const AnalysisResult& ar = burnout();
    const HydraRecord& rec = ar.record;
    const SongTiming& timing = ar.song.timing();
    const ChordTimes chords = chord_times(ar.song);
    const Path& best = rec.best_path();
    PathsTabCache cache;

    // One row per activation of the best path, every one folded. The path
    // needs a row for "all open" to mean anything.
    const size_t rows = best.walk_activations().size();
    REQUIRE(rows > 0);
    cache.details(best, rec, 1, &timing, chords, std::nullopt, core::default_rules());
    REQUIRE(cache.ui().act_open.size() == rows);
    CHECK(cache.ui().act_open == std::vector<char>(rows, 0));
    cache.ui().set_all(true);

    // A display setting rebuilds the rows but keeps what is unfolded.
    cache.details(best, rec, 1, &timing, chords, 30.0, core::default_rules());
    CHECK(cache.ui().all_open());
    cache.details(best, rec, 1, &timing, chords, 30.0, core::default_rules(), 126000.0);
    CHECK(cache.ui().all_open());
    CHECK(cache.details_builds() == 3);

    // Another path starts fresh: every row folded again.
    const std::vector<const Path*> all = rec.all_paths();
    REQUIRE(all.size() > 1);
    const Path& other = *all[1];
    cache.details(other, rec, 1, &timing, chords, 30.0, core::default_rules(), 126000.0);
    CHECK(cache.ui().act_open.size() == other.walk_activations().size());
    CHECK(cache.ui().act_open == std::vector<char>(other.walk_activations().size(), 0));
    CHECK_FALSE(cache.ui().all_open());
}

TEST_CASE("path_item_id: each picker item has its own id and shows its label") {
    // The Preview picker's labels for every corpus chart's buttons. ImGui
    // shows the text before "##" and keys the item on the whole id.
    for_each_corpus_record([](const HydraRecord& rec) {
        const PathButtonsView v = buttons_of(rec);
        std::vector<std::string> ids;
        for (size_t i = 0; i < v.buttons.size(); ++i) {
            const std::string label = preview_path_label(v.buttons[i]);
            CAPTURE(label);
            REQUIRE(label.find("##") == std::string::npos);
            const std::string id = path_item_id(label, i);
            CHECK(id.substr(0, id.find("##")) == label);
            // The same label at another place in the list is another item.
            CHECK(path_item_id(label, i + v.buttons.size()) != id);
            for (const std::string& earlier : ids) CHECK(earlier != id);
            ids.push_back(id);
        }
    });
}
