// The Path Builder's cost list: what each of the player's windows costs
// against Hydra's best path. Every cost is a difference of two best-finish
// totals (best_finish and search_target, search/pather.h); nothing here
// prices a path itself. Plan: docs/superpowers/plans/2026-10-28-path-builder.md,
// task EN-3.

#ifndef HYDRA_APP_PATH_COSTS_H
#define HYDRA_APP_PATH_COSTS_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "core/model.h"
#include "search/graph.h"
#include "search/pather.h"

namespace hydra::app {

// The totals every cost is read from. bf[k] is BF(k): best_finish's best
// total with the player's first k windows pinned, so bf[0] is Hydra's best
// path. player_total is search_target's total for all the player's windows
// and no other activation.
struct BestFinishLadder {
    std::vector<int64_t> bf;
    int64_t player_total = 0;
};

// What each window cost. windows[k - 1] is window k's cost, bf[k - 1] - bf[k]:
// what pinning it lost against leaving it to the search. after_last is the
// "after your last window" line, bf[n] - player_total: what the player loses
// by activating nowhere after the last window. The costs add up to
// bf[0] - player_total.
struct CostList {
    std::vector<int64_t> windows;
    int64_t after_last = 0;
};

// The one place a cost is worked out. Throws std::logic_error on a ladder
// with no BF(0), which is a caller bug.
CostList cost_list(const BestFinishLadder& ladder);

// A cost's colour (spec decision 3, plan Q5).
enum class CostTier { Green, Yellow, Red };

// Which colour `cost` is, against BF(0). The one owner of the tier rule.
// A cost is never negative: BF(k) never beats BF(k - 1), because a pinned
// search only chooses among the paths the free one could. So a negative cost
// is a bug upstream and throws std::logic_error rather than getting a colour.
CostTier cost_tier(int64_t cost, int64_t bf0);

// One ladder run as it stands. The ladder's bf holds the values landed so
// far, best[k] is the path behind bf[k] (best_finish's paths.front()), and
// player_path is the path behind player_total (search_target's
// paths.front()). `complete` is set once player_total has landed; until then
// player_total means nothing. A failed run carries the failure the run that
// stopped it reported (TargetResult's realized_prefix, failed_tick and
// failed_reason); on success realized_prefix is the window count.
struct LadderRun {
    BestFinishLadder ladder;
    std::vector<Path> best;
    std::optional<Path> player_path;
    bool complete = false;
    size_t realized_prefix = 0;
    std::optional<int64_t> failed_tick;
    std::string failed_reason;
};

// Called with the run so far each time a value lands.
using LadderProgress = std::function<void(const LadderRun&)>;

// Builds the ladder for the player's windows: best_finish on `graph` for
// each prefix length 0 to n, in order, then search_target for the player's
// own total. `progress` hears each value as it lands, so a UI fills as the
// answers arrive. The run stops at the first prefix no path realizes and
// carries that failure, with the values before it. The windows are taken in
// act_tick order, as the target searches take them; their checks (and
// exceptions) are sort_and_check_windows's (search/pather.h), made before the
// first search. `graph` is search_graph(song, settings),
// the one graph a chart's searches share; every search here runs on it.
// Device-free: the builder's worker thread calls it.
LadderRun build_ladder(const ScoreGraph& graph, const SearchSettings& settings,
                       std::vector<PinnedWindow> windows, const LadderProgress& progress = {});

}  // namespace hydra::app

#endif  // HYDRA_APP_PATH_COSTS_H
