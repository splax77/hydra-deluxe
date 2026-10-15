// Tests for app/path_view's path list on the Song Details screen: the path
// buttons and their groups, the list and its all-0 dedupe, path identity, the
// score breakdown and multiplier-squeeze panels, and the PathsTabCache and
// PathsTabUi that build and fold them. The cases pin the exact display
// strings. The activation rows are in test_activation_rows.cpp.

#include "doctest.h"

#include <string>
#include <vector>

#include "app/path_view.h"
#include "app/preview_view.h"  // path_overlay_key
#include "analyzed_chart.h"
#include "record_fixtures.h"
#include "song_details_fixtures.h"

using namespace hydra;
using namespace hydra::app;
using song_details::burnout;
using song_details::kDot;

TEST_CASE("build_score_breakdown: exact lines, rounded like the report") {
    Path p;
    p.score_base = 3;
    p.score_combo = 2;  // 5/3 = 1.6666...

    std::vector<std::string> lines = build_score_breakdown(p);
    REQUIRE(lines.size() == 8);
    // Rounded to three places, the same as the report's mult column. Every
    // number starts in one column, two spaces after the longest label, and is
    // not padded (answer 11 of the column-shares plan).
    CHECK(lines[0] == "Average Multiplier:  1.667x");
    CHECK(lines[1] == "\nNotes:               3");
    CHECK(lines[2] == "Combo Bonus:         2");
    CHECK(lines[3] == "Star Power:          0");
    CHECK(lines[4] == "Solo Bonus:          0");
    CHECK(lines[5] == "Accent Notes:        0");
    CHECK(lines[6] == "Ghost Notes:         0");
    CHECK(lines[7] == "\nTotal Score:         5");
}

TEST_CASE("path buttons: each path's own hardest timing, warn past the difficult floor") {
    auto button_of = [](const Path& p) {
        HydraRecord rec;
        rec.paths.push_back(p);
        PathButtonsView v = build_path_buttons(rec, 0, 2);
        REQUIRE(v.buttons.size() == 1);
        return v.buttons[0];
    };
    PathButtonView none = button_of(Path{});  // no activations, no difficulty
    CHECK(none.timing.empty());
    CHECK(none.detail.empty());

    Path hard;
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;  // not e-critical
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -12.5});
    hard.activations.push_back(act);
    PathButtonView b = button_of(hard);
    CHECK(b.timing == "12.5 ms");
    CHECK(b.timing_warn);
    CHECK(b.detail.empty());

    Path easy = hard;
    easy.activations[0].sqinouts[0].offset_ms = -1.5;
    b = button_of(easy);
    CHECK(b.timing == "1.5 ms");
    CHECK_FALSE(b.timing_warn);

    // A path whose only squeeze is a free squeeze-in has nothing to time,
    // so its button shows no figure (D51 call 4).
    Path free_in = hard;
    free_in.activations[0].sqinouts[0] = SPSqueeze{SqueezeKind::SqIn, -163.0};
    b = button_of(free_in);
    CHECK(b.timing.empty());
    CHECK_FALSE(b.timing_warn);
}

TEST_CASE("build_path_buttons: a 1-bar cap says why there are no activations") {
    HydraRecord rec;
    rec.sp_cap = 1;
    rec.paths.push_back(Path{});  // a 1-bar cap never activates
    PathButtonsView v = build_path_buttons(rec, 0, 2);
    REQUIRE(v.buttons.size() == 1);
    CHECK(v.buttons[0].detail == "A 1-bar cap can never activate Star Power.");

    rec.sp_cap = 4;
    v = build_path_buttons(rec, 0, 2);
    REQUIRE(v.buttons.size() == 1);
    CHECK(v.buttons[0].detail.empty());
}

TEST_CASE("build_path_list: score groups and the all-0 dedupe rule") {
    const HydraRecord& rec = corpus::analyzed_by_ten_scores().record;
    PathListView list = build_path_list(rec);

    // One group per distinct score along the traversal; every path lands in
    // exactly one group, in order.
    REQUIRE(!list.groups.empty());
    std::vector<const Path*> flat = rec.all_paths();
    size_t total = 0;
    for (const PathGroupView& g : list.groups) total += g.paths.size();
    CHECK(total == flat.size());
    CHECK(list.groups.front().score_label ==
          group_thousands(flat.front()->totalscore()));

    // An all-0 path that duplicates a listed path (the same path_identity)
    // stays hidden.
    HydraRecord dup = rec;
    dup.allzero_paths.clear();
    dup.allzero_paths.push_back(rec.paths.front());
    CHECK_FALSE(build_path_list(dup).show_allzero);

    // A different score shows the section. Its label is the score alone; the
    // button's detail line is the one place that says what it costs.
    HydraRecord worse = dup;
    worse.allzero_paths.front().score_base -= 100;
    PathListView wl = build_path_list(worse);
    CHECK(wl.show_allzero);
    CHECK(wl.allzero_label == group_thousands(worse.allzero_paths.front().totalscore()));
    const PathButtonsView wb = build_path_buttons(worse, 0, 2);
    REQUIRE_FALSE(wb.buttons.empty());
    CHECK(wb.buttons.back().group == PathButtonView::Group::AllZero);
    CHECK(wb.buttons.back().detail == "100 below optimal");

    // An all-0 path that ties the optimal score but is a different path (its
    // last deact one tick later) shows, with no detail line at all.
    HydraRecord tied = dup;
    Path& tied_path = tied.allzero_paths.front();
    REQUIRE_FALSE(tied_path.activations.empty());
    REQUIRE_FALSE(tied_path.activations.back().sp_end_steps.empty());
    tied_path.activations.back().sp_end_steps.back().end_tick += 1;
    REQUIRE(build_path_list(tied).show_allzero);
    const PathButtonsView tb = build_path_buttons(tied, 0, 2);
    REQUIRE_FALSE(tb.buttons.empty());
    CHECK(tb.buttons.back().group == PathButtonView::Group::AllZero);
    CHECK(tb.buttons.back().path->totalscore() == *tied.optimal_score());
    CHECK(tb.buttons.back().detail.empty());
}

// D51 Q4b.
TEST_CASE("build_path_list: one duplicate hides only itself") {
    const HydraRecord& rec = corpus::analyzed_by_ten_scores().record;
    REQUIRE_FALSE(rec.paths.empty());
    const Path& first = rec.paths.front();
    REQUIRE_FALSE(first.activations.empty());
    REQUIRE_FALSE(first.activations.back().sp_end_steps.empty());

    // The all-0 list holds the first listed path, plus one variant of it
    // whose last deact tick is one later: a path the list does not have.
    HydraRecord one = rec;
    one.allzero_paths.clear();
    one.allzero_paths.push_back(first);
    Path& dup = one.allzero_paths.front();
    dup.variants.clear();
    Path variant = first;
    variant.variants.clear();
    variant.activations.back().sp_end_steps.back().end_tick += 1;
    dup.variants.push_back(variant);
    dup.recount_tied_paths();

    const PathListView list = build_path_list(one);
    CHECK(list.show_allzero);
    REQUIRE(list.allzero.size() == 1);
    CHECK(path_identity(*list.allzero.front()) == path_identity(variant));
}

TEST_CASE("path_identity: same path, rescored path, trimmed path") {
    const std::vector<const Path*> paths = corpus::analyzed_by_ten_scores().record.all_paths();
    REQUIRE_FALSE(paths.empty());
    const Path& first = *paths.front();
    REQUIRE_FALSE(first.activations.empty());

    // A copy is the same path.
    Path copy = first;
    CHECK_FALSE(path_identity(first).empty());
    CHECK(path_identity(first) == path_identity(copy));

    // Same activations, one more point: a different path.
    Path rescored = first;
    rescored.score_base += 1;
    CHECK(path_identity(first) != path_identity(rescored));

    // One activation fewer: a different path.
    Path trimmed = first;
    trimmed.activations.pop_back();
    CHECK(path_identity(first) != path_identity(trimmed));

    // Only the last activation's deact tick moved: a different path.
    Path moved = first;
    REQUIRE_FALSE(moved.activations.back().sp_end_steps.empty());
    moved.activations.back().sp_end_steps.back().end_tick += 1;
    CHECK(path_identity(first) != path_identity(moved));
}

TEST_CASE("build_path_list: the dedupe and the overlay key read one identity") {
    const HydraRecord& rec = corpus::analyzed_by_ten_scores().record;
    const std::vector<const Path*> flat = rec.all_paths();
    REQUIRE_FALSE(flat.empty());

    // The Preview's overlay key is the path's identity, for every path.
    for (const Path* p : flat) CHECK(path_overlay_key(p) == path_identity(*p));

    // Each listed path, offered alone (without its variants) as the all-0
    // path, hides; the same path one deact tick later shows.
    for (const Path* p : flat) {
        HydraRecord one = rec;
        one.allzero_paths.clear();
        one.allzero_paths.push_back(*p);
        one.allzero_paths.front().variants.clear();
        CHECK_FALSE(build_path_list(one).show_allzero);
        if (p->activations.empty() || p->activations.back().sp_end_steps.empty()) continue;
        one.allzero_paths.front().activations.back().sp_end_steps.back().end_tick += 1;
        CHECK(build_path_list(one).show_allzero);
    }
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

    // The details: once per (path, record, hit window, backend limit).
    const Path& best = rec.best_path();
    for (int frame = 0; frame < 5; ++frame)
        cache.details(best, rec, 8, &timing, 70.0, std::nullopt, core::default_rules());
    CHECK(cache.details_builds() == 1);
    cache.details(best, rec, 8, &timing, 71.0, std::nullopt, core::default_rules());
    CHECK(cache.details_builds() == 2);
    cache.details(best, rec, 8, &timing, 71.0, 30.0, core::default_rules());
    CHECK(cache.details_builds() == 3);
    cache.details(best, rec, 9, &timing, 71.0, 30.0, core::default_rules());
    CHECK(cache.details_builds() == 4);
    std::vector<const Path*> all = rec.all_paths();
    if (all.size() > 1) {
        cache.details(*all[1], rec, 9, &timing, 71.0, 30.0, core::default_rules());
        CHECK(cache.details_builds() == 5);
    }

    // What the cache hands back is what a fresh build gives.
    const PathsTabCache::Details& d =
        cache.details(best, rec, 9, &timing, 71.0, 30.0, core::default_rules());
    CHECK(d.breakdown == build_score_breakdown(best));
    CHECK(d.squeezes.size() == build_multsqueezes(rec).size());
    CHECK(d.activations.acts.size() ==
          build_activations(best, rec, &timing, 71.0, 30.0).acts.size());
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
    CHECK(v[0].points == 15);
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
        return cache.details(rec.best_path(), rec, 1, &timing, 70.0, std::nullopt,
                             core::default_rules(), std::nullopt, pro_drums).squeezes;
    };
    const std::vector<MultSqueezeView> on = squeezes_of(true);
    REQUIRE(on.size() == 1);
    CHECK(on[0].label == "2x   (+15 pts):   [Red snare - Yellow cymbal]");
    CHECK(on[0].howto == "Hit [Red snare] first.");
    // With Pro Drums off the red pad has no snare; the cymbal still says cymbal.
    const std::vector<MultSqueezeView> off = squeezes_of(false);
    REQUIRE(off.size() == 1);
    CHECK(off[0].label == "2x   (+15 pts):   [Red - Yellow cymbal]");
    CHECK(off[0].howto == "Hit [Red] first.");
    CHECK(off[0].points == 15);
}

TEST_CASE("PathsTabUi: one row open at a time, expand and collapse all") {
    PathsTabUi ui;
    ui.reset(3);
    CHECK(ui.act_open == std::vector<char>{1, 0, 0});
    CHECK(ui.backends_open == std::vector<char>{1, 1, 1});
    CHECK_FALSE(ui.all_open());

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

    cache.details(best, rec, 1, &timing, 70.0, std::nullopt, core::default_rules());
    REQUIRE(cache.ui().act_open.size() == 3);
    CHECK(cache.ui().act_open[0] == 1);
    cache.ui().set_all(true);

    // A display setting rebuilds the rows but keeps what is unfolded.
    cache.details(best, rec, 1, &timing, 70.0, 30.0, core::default_rules());
    CHECK(cache.ui().all_open());
    cache.details(best, rec, 1, &timing, 70.0, 30.0, core::default_rules(), 126000.0);
    CHECK(cache.ui().all_open());
    CHECK(cache.details_builds() == 3);

    // Another path starts fresh: first row open, the rest folded.
    const Path& other = *rec.all_paths()[1];
    cache.details(other, rec, 1, &timing, 70.0, 30.0, core::default_rules(), 126000.0);
    CHECK(cache.ui().act_open.size() == other.walk_activations().size());
    CHECK(cache.ui().act_open[0] == 1);
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
