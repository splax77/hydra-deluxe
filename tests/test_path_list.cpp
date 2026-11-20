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
#include "analyzed_chart.h"
#include "record_fixtures.h"
#include "song_details_fixtures.h"

using namespace hydra;
using namespace hydra::app;
using song_details::burnout;
using song_details::kDot;

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
        for (const Path* z : rec.all_allzero_paths()) {
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
    // Burnout shows one under these settings (the Burnout button test below
    // reads its all-0 button).
    CHECK(charts_showing > 0);
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

TEST_CASE("build_multsqueezes: one labeled entry per squeeze") {
    const HydraRecord& rec = corpus::analyzed_by_ten_scores().record;
    std::vector<MultSqueezeView> v = build_multsqueezes(rec);
    CHECK(v.size() == rec.multsqueezes.size());
    for (size_t i = 0; i < v.size(); ++i) {
        CHECK(v[i].label.find(" pts):   ") != std::string::npos);
        CHECK(v[i].howto == rec.multsqueezes[i].howto());
    }
}

TEST_CASE("PathsTabCache: views are built once and rebuilt only when their inputs move") {
    const AnalysisResult& ar = corpus::analyzed_by_ten_scores();
    const HydraRecord& rec = ar.record;
    const SongTiming& timing = ar.song.timing();
    PathsTabCache cache;

    // The buttons: once per record generation, however many frames ask.
    for (int frame = 0; frame < 5; ++frame) cache.buttons(rec, 7, 0, 10);
    CHECK(cache.buttons_builds() == 1);
    const PathButtonsView& list = cache.buttons(rec, 8, 0, 10);  // the record was re-read
    CHECK(cache.buttons_builds() == 2);

    // Every button carries its path's own notation.
    for (const PathButtonView& b : list.buttons) CHECK(b.notation == b.path->pathstring());

    // The details: once per (path, record, backend limit).
    const Path& best = rec.best_path();
    for (int frame = 0; frame < 5; ++frame)
        cache.details(best, rec, 8, &timing, std::nullopt, core::default_rules());
    CHECK(cache.details_builds() == 1);
    cache.details(best, rec, 8, &timing, 30.0, core::default_rules());
    CHECK(cache.details_builds() == 2);
    cache.details(best, rec, 9, &timing, 30.0, core::default_rules());
    CHECK(cache.details_builds() == 3);
    std::vector<const Path*> all = rec.all_paths();
    if (all.size() > 1) {
        cache.details(*all[1], rec, 9, &timing, 30.0, core::default_rules());
        CHECK(cache.details_builds() == 4);
    }

    // What the cache hands back is what a fresh build gives.
    const PathsTabCache::Details& d =
        cache.details(best, rec, 9, &timing, 30.0, core::default_rules());
    CHECK(d.breakdown == build_score_breakdown(best));
    CHECK(d.squeezes.size() == build_multsqueezes(rec).size());
    CHECK(d.activations.acts.size() ==
          build_activations(best, rec, &timing, 30.0).acts.size());
}

TEST_CASE("path buttons: Burnout's list, in the mockup's groups") {
    const HydraRecord& rec = burnout().record;
    PathButtonsView v = build_path_buttons(rec, /*depth_mode=*/0, /*depth_value=*/2);
    CHECK(v.within_label == "Within 2 scores");
    REQUIRE(v.buttons.size() == 4);

    CHECK(v.buttons[0].path == &rec.best_path());
    CHECK(v.buttons[0].group == PathButtonView::Group::Optimal);
    CHECK(v.buttons[0].notation == "3- 1 2");
    // Inside a fold the score sits on the fold, not on the button.
    CHECK(v.buttons[0].fold == "378,315");
    CHECK(v.buttons[0].fold_count == "1 path");
    CHECK(v.buttons[0].title == "3- 1 2");
    CHECK(v.buttons[0].timing == "163.0 ms");
    CHECK(v.buttons[0].timing_warn);
    CHECK(v.buttons[0].detail.empty());

    CHECK(v.buttons[1].group == PathButtonView::Group::Within);
    CHECK(v.buttons[1].fold == "378,175");
    CHECK(v.buttons[1].title == "0 4 1");
    CHECK(v.buttons[1].timing.empty());  // needs no squeeze
    CHECK(v.buttons[1].detail.empty());
    CHECK(v.buttons[2].group == PathButtonView::Group::Within);
    CHECK(v.buttons[2].fold == "378,075");
    CHECK(v.buttons[2].title == "2 1 2");

    // The all-0 path has no fold, so its score stays on the button.
    CHECK(v.buttons[3].group == PathButtonView::Group::AllZero);
    CHECK(v.buttons[3].fold.empty());
    CHECK(v.buttons[3].fold_count.empty());
    CHECK(v.buttons[3].title == "375,955" + kDot + "0 0 0 0");
    CHECK(v.buttons[3].detail == "2,360 below optimal");

    // The INI's int reaches the heading through Settings::search_depth_mode.
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

TEST_CASE("multiplier squeeze: Burnout's one squeeze and the fold's summary") {
    std::vector<MultSqueezeView> v = build_multsqueezes(burnout().record);
    REQUIRE(v.size() == 1);
    CHECK(v[0].label == "2x   (+15 pts):   [Red snare - Yellow cymbal]");
    CHECK(v[0].howto == "Hit [Red snare] first.");
    CHECK(v[0].gain == 15);
    CHECK(multsqueeze_summary(v) == "+15");
    CHECK(multsqueeze_summary({}) == "none");
    std::vector<MultSqueezeView> three(3, v[0]);
    CHECK(multsqueeze_summary(three) == "3" + kDot + "+45");
}

TEST_CASE("multiplier squeeze: the note names follow the Pro Drums setting") {
    // The Paths tab builds its squeeze rows through the cache, which passes
    // the Pro Drums setting the record was analyzed with (D48 Q11, finding 17).
    const AnalysisResult& ar = burnout();
    const HydraRecord& rec = ar.record;
    const SongTiming& timing = ar.song.timing();
    auto squeezes_of = [&](bool pro_drums) {
        PathsTabCache cache;
        return cache.details(rec.best_path(), rec, 1, &timing, std::nullopt,
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
            const std::string chord = msq.chord().rowstr(pro_drums);
            CHECK(rows[i].label.rfind(msq.notationstr(), 0) == 0);
            CHECK(rows[i].label.find("+" + std::to_string(msq.gain())) != std::string::npos);
            REQUIRE(rows[i].label.size() >= chord.size());
            CHECK(rows[i].label.substr(rows[i].label.size() - chord.size()) == chord);
            CHECK(rows[i].howto == msq.howto(pro_drums));
            CHECK(rows[i].gain == msq.gain());
            if (pro_drums && msq.chord().rowstr(true) != msq.chord().rowstr(false)) ++worded_apart;
        }
    }
    // Burnout's squeeze is on the red pad, which Pro Drums calls a snare, so
    // the setting changes its words.
    CHECK(worded_apart > 0);
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
    const Path& best = rec.best_path();
    PathsTabCache cache;

    cache.details(best, rec, 1, &timing,std::nullopt, core::default_rules());
    REQUIRE(cache.ui().act_open.size() == 3);
    CHECK(cache.ui().act_open == std::vector<char>{0, 0, 0});
    cache.ui().set_all(true);

    // A display setting rebuilds the rows but keeps what is unfolded.
    cache.details(best, rec, 1, &timing,30.0, core::default_rules());
    CHECK(cache.ui().all_open());
    cache.details(best, rec, 1, &timing,30.0, core::default_rules(), 126000.0);
    CHECK(cache.ui().all_open());
    CHECK(cache.details_builds() == 3);

    // Another path starts fresh: every row folded again.
    const Path& other = *rec.all_paths()[1];
    cache.details(other, rec, 1, &timing,30.0, core::default_rules(), 126000.0);
    CHECK(cache.ui().act_open.size() == other.walk_activations().size());
    CHECK(cache.ui().act_open == std::vector<char>(other.walk_activations().size(), 0));
    CHECK_FALSE(cache.ui().all_open());

    // The path buttons: once per record generation and score range.
    for (int frame = 0; frame < 5; ++frame) cache.buttons(rec, 1, 0, 2);
    CHECK(cache.buttons_builds() == 1);
    cache.buttons(rec, 1, 0, 3);
    CHECK(cache.buttons_builds() == 2);
    cache.buttons(rec, 2, 0, 3);
    CHECK(cache.buttons_builds() == 3);
}

TEST_CASE("path_item_id: the label, two hashes and the index") {
    CHECK(path_item_id("3- 1 2  (optimal)", 0) == "3- 1 2  (optimal)##0");
    CHECK(path_item_id("0 4 1", 7) == "0 4 1##7");
}
