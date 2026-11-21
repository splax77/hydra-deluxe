// The Path Builder's cost list (app/path_costs.h; plan
// docs/superpowers/plans/2026-10-28-path-builder.md (removed; git show b29a3c28:docs/superpowers/plans/2026-10-28-path-builder.md), task EN-3). The corpus
// cases run at the GUI's default settings. Each player path is built from the
// chart's own analysis, and each check is a property of the ladder's totals.

#include "doctest.h"

#include <cstdint>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/display_format.h"
#include "app/path_costs.h"
#include "core/model.h"
#include "corpus_default_cfg.h"
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

using corpus::default_cfg;
using corpus::song_named;

namespace {

const HydraRecord& record_named(const std::string& name) {
    return corpus::analyzed(corpus::chart_named(name), default_cfg());
}

// The path search_target prices for activations at exactly `ticks`, best
// variant, as full pins.
std::vector<PinnedWindow> full_pins_at(const Song& song, const std::vector<int64_t>& ticks) {
    const std::vector<Path> paths = search_target(song, default_cfg(), activation_pins(ticks)).paths;
    REQUIRE_FALSE(paths.empty());
    return pinned_windows(paths.front());
}

const char* const kAllies = "Mutemath - Allies";

// One player path on Mutemath "Allies", built from the chart's own best and
// all-0 paths.
struct Case {
    std::string what;
    std::vector<PinnedWindow> windows;
};

std::vector<Case> allies_cases() {
    const Song& song = song_named(kAllies);
    const HydraRecord& rec = record_named(kAllies);
    REQUIRE_FALSE(rec.allzero_paths.empty());
    const std::vector<int64_t> best = test::act_ticks(rec.best_path());
    const std::vector<int64_t> allzero = test::act_ticks(rec.allzero_paths.front());
    REQUIRE(best.size() >= 3);
    REQUIRE(allzero.size() >= 3);
    std::vector<int64_t> moved = best;
    moved[2] = allzero[2];
    std::vector<Case> out;
    out.push_back({"the best path with its third window moved to the all-0 path's third",
                   full_pins_at(song, moved)});
    out.push_back({"the all-0 path", pinned_windows(rec.allzero_paths.front())});
    out.push_back({"the best path's first two windows, nothing after",
                   full_pins_at(song, {best[0], best[1]})});
    return out;
}

LadderRun ladder_for(const Song& song, const std::vector<PinnedWindow>& windows) {
    const ScoreGraph graph = search_graph(song, default_cfg());
    return build_ladder(graph, default_cfg(), windows);
}

}  // namespace

TEST_CASE("cost list: the costs sum exactly to the gap") {
    const Song& song = song_named(kAllies);
    const int64_t best_total = record_named(kAllies).best_path().totalscore();
    for (const Case& c : allies_cases()) {
        CAPTURE(c.what);
        const LadderRun run = ladder_for(song, c.windows);
        REQUIRE(run.complete);
        const std::vector<int64_t>& bf = run.ladder.bf;
        REQUIRE(bf.size() == c.windows.size() + 1);
        // BF(0) pins nothing, so it is the analysis's own best path.
        CHECK(bf.front() == best_total);

        // Window k's cost is BF(k-1) - BF(k); the line after the last window
        // is BF(n) less the player's own total.
        const CostList costs = cost_list(run.ladder);
        REQUIRE(costs.windows.size() == c.windows.size());
        for (size_t k = 1; k < bf.size(); ++k) CHECK(costs.windows[k - 1] == bf[k - 1] - bf[k]);
        CHECK(costs.after_last == bf.back() - run.ladder.player_total);
        const int64_t sum =
            std::accumulate(costs.windows.begin(), costs.windows.end(), int64_t{0}) +
            costs.after_last;
        CHECK(sum == bf.front() - run.ladder.player_total);
    }
}

TEST_CASE("cost list: no cost is below zero") {
    const Song& allies = song_named(kAllies);
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

    // August Burns Red "White Washed": its best path has a window that needs
    // a squeeze tighter than the Path limit, so the limit does not choose the
    // best path here. Pinning that path window by window costs nothing,
    // tight window included: the ladder is BF(0) all the way down.
    const std::string name = "August Burns Red - White Washed";
    const Song& song = song_named(name);
    const HydraRecord& rec = record_named(name);
    REQUIRE(default_cfg().ms_filter.has_value());
    const double limit_ms = *default_cfg().ms_filter;
    const Path& best = rec.best_path();
    const std::vector<Activation> acts = best.all_activations();
    REQUIRE_FALSE(acts.empty());
    bool any_tight = false;
    for (const Activation& a : acts) any_tight = any_tight || !a.within_ms_limit(limit_ms);
    CHECK(any_tight);

    const std::vector<PinnedWindow> pins = pinned_windows(best);
    const LadderRun run = ladder_for(song, pins);
    REQUIRE(run.complete);
    CHECK(run.ladder.bf == std::vector<int64_t>(pins.size() + 1, best.totalscore()));
    CHECK(run.ladder.player_total == best.totalscore());
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

TEST_CASE("candidate costs: a negative cost is a bug") {
    // A candidate whose best finish beats BF(n): pinning one more activation
    // can never do that, so the cost does not get a colour.
    const std::vector<hydra::app::CandidateFinish> beats = {{100, 1001}};
    CHECK_THROWS_AS(hydra::app::candidate_costs(1000, 1000, beats), std::logic_error);
    // Level with BF(n) it is the best next fill.
    const std::vector<hydra::app::CandidateCost> level =
        hydra::app::candidate_costs(1000, 1000, {{100, 1000}});
    REQUIRE(level.size() == 1);
    CHECK(level[0].act_tick == 100);
    CHECK(level[0].best_finish == 1000);
    CHECK(level[0].cost == 0);
    CHECK(level[0].tier == CostTier::Green);
}

TEST_CASE("end costs: each end against the best of them, in the order given") {
    // The best end need not come first: end_choices lists them in
    // search_target's order. Each end's finish is written as the highest
    // finish less its shortfall, so its cost is that shortfall.
    const int64_t top = 412850;
    const int64_t short_first = 14640;
    const int64_t short_last = 1234;
    CHECK(hydra::app::end_costs({top - short_first, top, top, top - short_last}) ==
          std::vector<int64_t>{short_first, 0, 0, short_last});
    CHECK(hydra::app::end_costs({}).empty());
}

TEST_CASE("format cost: a cost is written with the minus sign U+2212, in full") {
    CHECK(hydra::app::format_cost(1234) == "\xE2\x88\x92" "1,234");
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

// Allies' best path by activation only, its second window moved one tick
// later, where no fill node is. BF(0) and BF(1) land; BF(2) cannot, so the
// run stops there and names that window as search_target would.
TEST_CASE("build ladder: stops at the first impossible prefix and names it") {
    const Song& song = song_named(kAllies);
    const Path& best = record_named(kAllies).best_path();
    std::vector<int64_t> ticks = test::act_ticks(best);
    REQUIRE(ticks.size() >= 2);
    ticks[1] += 1;
    const int64_t moved_tick = ticks[1];
    const std::vector<PinnedWindow> windows = activation_pins(ticks);

    std::vector<size_t> heard;
    const ScoreGraph graph = search_graph(song, default_cfg());
    const LadderRun run = build_ladder(graph, default_cfg(), windows,
                                       [&](const LadderRun& r) { heard.push_back(r.ladder.bf.size()); });
    CHECK_FALSE(run.complete);
    REQUIRE(run.ladder.bf.size() == 2);
    // BF(0) is the analysis's best path, and pinning that path's own first
    // activation changes nothing.
    CHECK(run.ladder.bf[0] == best.totalscore());
    CHECK(run.ladder.bf[1] == run.ladder.bf[0]);
    CHECK(run.best.size() == 2);
    CHECK_FALSE(run.player_path.has_value());
    CHECK(run.realized_prefix == 1);
    CHECK(run.failed_tick == std::optional<int64_t>(moved_tick));
    CHECK(run.failed_reason == "activation");
    CHECK(heard == std::vector<size_t>{1, 2});
}

TEST_CASE("costs nothing: only a cost of zero is a best choice's") {
    CHECK(hydra::app::costs_nothing(0));
    CHECK_FALSE(hydra::app::costs_nothing(1));
    CHECK_FALSE(hydra::app::costs_nothing(14640));
}
