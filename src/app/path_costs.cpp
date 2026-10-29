#include "app/path_costs.h"

#include <stdexcept>
#include <utility>

namespace hydra::app {

CostList cost_list(const BestFinishLadder& ladder) {
    if (ladder.bf.empty()) throw std::logic_error("cost_list: the ladder has no BF(0)");
    CostList out;
    for (size_t k = 1; k < ladder.bf.size(); ++k)
        out.windows.push_back(ladder.bf[k - 1] - ladder.bf[k]);
    out.after_last = ladder.bf.back() - ladder.player_total;
    return out;
}

CostTier cost_tier(int64_t cost, int64_t bf0) {
    if (cost < 0)
        throw std::logic_error("cost_tier: cost " + std::to_string(cost) +
                               " is below zero; a pinned search beat the free one");
    if (cost == 0) return CostTier::Green;
    // Whole numbers only (plan Q5): a cost of exactly 1% of BF(0) is red.
    if (100 * cost >= bf0) return CostTier::Red;
    return CostTier::Yellow;
}

LadderRun build_ladder(const ScoreGraph& graph, const SearchSettings& settings,
                       std::vector<PinnedWindow> windows, const LadderProgress& progress) {
    // A prefix is the first k windows in the order the target searches take
    // them, so a list they refuse is refused here, before any search.
    sort_and_check_windows(windows);
    auto report = [&](const LadderRun& run) {
        if (progress) progress(run);
    };

    LadderRun run;
    for (size_t k = 0; k <= windows.size(); ++k) {
        const std::vector<PinnedWindow> prefix(windows.begin(),
                                               windows.begin() + static_cast<std::ptrdiff_t>(k));
        TargetResult bf = best_finish(graph, settings, prefix);
        if (bf.paths.empty()) {
            run.realized_prefix = bf.realized_prefix;
            run.failed_tick = bf.failed_tick;
            run.failed_reason = std::move(bf.failed_reason);
            return run;
        }
        run.ladder.bf.push_back(bf.paths.front().totalscore());
        run.best.push_back(std::move(bf.paths.front()));
        report(run);
    }

    TargetResult own = search_target(graph, windows);
    run.realized_prefix = own.realized_prefix;
    if (own.paths.empty()) {
        run.failed_tick = own.failed_tick;
        run.failed_reason = std::move(own.failed_reason);
        return run;
    }
    run.ladder.player_total = own.paths.front().totalscore();
    run.player_path = std::move(own.paths.front());
    run.complete = true;
    report(run);
    return run;
}

}  // namespace hydra::app
