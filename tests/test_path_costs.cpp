// The Path Builder's cost list (app/path_costs.h; plan
// docs/superpowers/plans/2026-10-28-path-builder.md, task EN-3). The corpus
// cases run at the GUI's default settings, whose Path limit is 10 ms; every
// total is pinned from one run.

#include "doctest.h"

#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/path_costs.h"
#include "core/model.h"
#include "corpus_util.h"
#include "record_fixtures.h"
#include "search/graph.h"
#include "search/pather.h"

// A failed CHECK on a list of totals prints the totals.
namespace doctest {
template <>
struct StringMaker<std::vector<int64_t>> {
    static String convert(const std::vector<int64_t>& v) {
        std::string s = "{";
        for (size_t i = 0; i < v.size(); ++i) s += (i ? ", " : "") + std::to_string(v[i]);
        return String((s + "}").c_str());
    }
};
}  // namespace doctest

using namespace hydra;
using hydra::app::BestFinishLadder;
using hydra::app::CostList;
using hydra::app::CostTier;
using hydra::app::LadderRun;
using hydra::app::build_ladder;
using hydra::app::cost_list;
using hydra::app::cost_tier;

namespace {

const app::AnalysisSettings& default_cfg() {
    static const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    return cfg;
}

const Song& song_named(const std::string& name) {
    const std::string chart = corpus::chart_named(name);
    REQUIRE_FALSE(chart.empty());
    return corpus::song(chart, default_cfg().reading());
}

const HydraRecord& record_named(const std::string& name) {
    return corpus::analyzed(corpus::chart_named(name), default_cfg());
}

// The path search_target prices for activations at exactly `ticks`, best
// variant, as full pins.
std::vector<PinnedWindow> full_pins_at(const Song& song, const std::vector<int64_t>& ticks) {
    const std::vector<Path> paths = search_target(song, default_cfg(), ticks);
    REQUIRE_FALSE(paths.empty());
    return pinned_windows(paths.front());
}

// One hand-built player path on Mutemath "Allies" and what its ladder must
// read.
struct Case {
    std::string what;
    std::vector<PinnedWindow> windows;
    std::vector<int64_t> bf;
    int64_t player_total;
    std::vector<int64_t> costs;
    int64_t after_last;
};

// Allies' best path activates at 9984 22272 37632 49920 (0- 1- 1 0, 451830).
// Each case's totals and costs are pinned from one run.
std::vector<Case> allies_cases() {
    const Song& song = song_named("Mutemath - Allies");
    const HydraRecord& rec = record_named("Mutemath - Allies");
    REQUIRE_FALSE(rec.allzero_paths.empty());
    REQUIRE(test::act_ticks(rec.allzero_paths.front()) ==
            std::vector<int64_t>{9984, 19200, 34560, 49920});
    return {
        {"the best path with its third window moved to 34560",
         full_pins_at(song, {9984, 22272, 34560, 49920}),
         {451830, 451830, 451830, 447770, 447770},
         447770,
         {0, 0, 4060, 0},
         0},
        {"the all-0 path (activations at 9984 19200 34560 49920)",
         pinned_windows(rec.allzero_paths.front()),
         {451830, 451830, 438490, 434430, 434430},
         434430,
         {0, 13340, 4060, 0},
         0},
        {"a two-window path (activations at 9984 22272, nothing after)",
         full_pins_at(song, {9984, 22272}),
         {451830, 451830, 450430},
         407610,
         {0, 1400},
         42820},
    };
}

LadderRun ladder_for(const Song& song, const std::vector<PinnedWindow>& windows) {
    const ScoreGraph graph = search_graph(song, default_cfg());
    return build_ladder(graph, default_cfg(), windows);
}

}  // namespace

TEST_CASE("cost list: the costs sum exactly to the gap") {
    const Song& song = song_named("Mutemath - Allies");
    for (const Case& c : allies_cases()) {
        CAPTURE(c.what);
        const LadderRun run = ladder_for(song, c.windows);
        REQUIRE(run.complete);
        REQUIRE(run.ladder.bf.size() == c.windows.size() + 1);
        CHECK(run.ladder.bf == c.bf);
        CHECK(run.ladder.player_total == c.player_total);

        const CostList costs = cost_list(run.ladder);
        CHECK(costs.windows == c.costs);
        CHECK(costs.after_last == c.after_last);
        const int64_t sum =
            std::accumulate(costs.windows.begin(), costs.windows.end(), int64_t{0}) +
            costs.after_last;
        CHECK(sum == run.ladder.bf.front() - run.ladder.player_total);
    }
}

TEST_CASE("cost list: no cost is below zero") {
    const Song& allies = song_named("Mutemath - Allies");
    for (const Case& c : allies_cases()) {
        CAPTURE(c.what);
        const LadderRun run = ladder_for(allies, c.windows);
        REQUIRE(run.complete);
        for (size_t k = 1; k < run.ladder.bf.size(); ++k)
            CHECK(run.ladder.bf[k] <= run.ladder.bf[k - 1]);
        const CostList costs = cost_list(run.ladder);
        for (const int64_t cost : costs.windows) CHECK(cost >= 0);
        CHECK(costs.after_last >= 0);
    }

    // August Burns Red "White Washed": its optimal path's fourth window needs
    // a squeeze tighter than the 10 ms limit, so the limit does not choose
    // the best path here. Pinning that path window by window costs nothing,
    // tight window included: the ladder is BF(0) all the way down.
    const std::string name = "August Burns Red - White Washed";
    const Song& song = song_named(name);
    const HydraRecord& rec = record_named(name);
    REQUIRE(default_cfg().ms_filter == std::optional<double>(10.0));
    const Path& best = rec.best_path();
    CHECK(best.totalscore() == 706180);
    const std::vector<Activation> acts = best.all_activations();
    REQUIRE(acts.size() == 5);
    CHECK_FALSE(acts[3].within_ms_limit(*default_cfg().ms_filter));

    const LadderRun run = ladder_for(song, pinned_windows(best));
    REQUIRE(run.complete);
    CHECK(run.ladder.bf == std::vector<int64_t>(6, 706180));
    CHECK(run.ladder.player_total == 706180);
    const CostList costs = cost_list(run.ladder);
    for (const int64_t cost : costs.windows) {
        CHECK(cost == 0);
        CHECK(cost_tier(cost, run.ladder.bf.front()) == CostTier::Green);
    }
    CHECK(costs.after_last == 0);
}

TEST_CASE("cost tier: the 1% boundary") {
    CHECK(cost_tier(0, 1000) == CostTier::Green);
    CHECK(cost_tier(10, 1000) == CostTier::Red);
    CHECK(cost_tier(9, 1000) == CostTier::Yellow);
    CHECK(cost_tier(10, 1001) == CostTier::Yellow);
}

TEST_CASE("cost tier: a negative cost is a bug") {
    CHECK_THROWS_AS(cost_tier(-1, 1000), std::logic_error);
}

TEST_CASE("build ladder: a list the target searches refuse is refused before any search") {
    const Song& song = song_named("Mutemath - Allies");
    const ScoreGraph graph = search_graph(song, default_cfg());
    std::vector<size_t> heard;
    const std::vector<PinnedWindow> twice = {PinnedWindow{9984}, PinnedWindow{9984}};
    CHECK_THROWS_AS(build_ladder(graph, default_cfg(), twice,
                                 [&](const LadderRun& r) { heard.push_back(r.ladder.bf.size()); }),
                    std::invalid_argument);
    CHECK(heard.empty());
}

TEST_CASE("search target: a built graph answers as the song does") {
    const Song& song = song_named("Mutemath - Allies");
    const ScoreGraph graph = search_graph(song, default_cfg());
    const std::vector<PinnedWindow> windows = activation_pins({9984, 22272});
    const TargetResult on_graph = search_target(graph, windows);
    const TargetResult on_song = search_target(song, default_cfg(), windows);
    REQUIRE_FALSE(on_graph.paths.empty());
    REQUIRE(on_song.paths.size() == on_graph.paths.size());
    CHECK(on_graph.paths.front().totalscore() == on_song.paths.front().totalscore());
    CHECK(on_graph.paths.front().totalscore() == 407610);
}

// Allies' best path by activation only, its second window moved one tick
// later, where no fill node is. BF(0) and BF(1) land; BF(2) cannot, so the
// run stops there and names that window as search_target would.
TEST_CASE("build ladder: stops at the first impossible prefix and names it") {
    const Song& song = song_named("Mutemath - Allies");
    const std::vector<PinnedWindow> windows = activation_pins({9984, 22273, 37632, 49920});

    std::vector<size_t> heard;
    const ScoreGraph graph = search_graph(song, default_cfg());
    const LadderRun run = build_ladder(graph, default_cfg(), windows,
                                       [&](const LadderRun& r) { heard.push_back(r.ladder.bf.size()); });
    CHECK_FALSE(run.complete);
    CHECK(run.ladder.bf == std::vector<int64_t>{451830, 451830});
    CHECK(run.best.size() == 2);
    CHECK_FALSE(run.player_path.has_value());
    CHECK(run.realized_prefix == 1);
    CHECK(run.failed_tick == std::optional<int64_t>(22273));
    CHECK(run.failed_reason == "activation");
    CHECK(heard == std::vector<size_t>{1, 2});
}
