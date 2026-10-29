// best_finish: the best path that starts with the player's pinned windows and
// runs free after them (the Path Builder's BF(k), plan
// docs/superpowers/plans/2026-10-28-path-builder.md, task EN-2). Every case
// runs on the corpus (testdata/input) at the GUI's default settings.

#include "doctest.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "app/config.h"
#include "core/model.h"
#include "core/replay.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "search/pather.h"

using namespace hydra;

namespace {

// Every root and tied variant in `paths`, roots first.
std::vector<const Path*> roots_and_tied(const std::vector<Path>& paths) {
    std::vector<const Path*> out;
    for (const Path& p : paths) out.push_back(&p);
    for (const Path* v : test::all_tied(paths)) out.push_back(v);
    return out;
}

// The corpus chart whose notespath holds `name`.
std::string corpus_chart(const std::string& name) {
    for (const std::string& p : corpus::chart_paths())
        if (p.find(name) != std::string::npos) return p;
    return "";
}

}  // namespace

// Identity 1: with nothing pinned, the best finish is the main search's best
// path, string and total, on every corpus chart.
TEST_CASE("best finish: with nothing pinned it is Hydra's best path on every corpus chart") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int charts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        ++charts;
        CAPTURE(chart);
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        REQUIRE_FALSE(rec.paths.empty());
        const TargetResult got = best_finish(song, cfg, {});
        REQUIRE_FALSE(got.paths.empty());
        CHECK(got.paths.front().pathstring() == rec.best_path().pathstring());
        CHECK(got.paths.front().totalscore() == rec.best_path().totalscore());
        CHECK(got.realized_prefix == 0);
    }
    CHECK(charts > 0);
}

// Identity 2: every window of a corpus chart's best path pinned in full.
// A best finish may still activate again after the last window, which is not
// this identity, so the case compares the returned path (root or tied
// variant) that activates at the pinned windows and nowhere else. Its total
// is search_target's for the same windows, and replaying its own windows
// gives the same total.
TEST_CASE("best finish: every window of a corpus best path pinned in full, with no free fill "
          "left, equals search_target and the replay") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int charts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        ++charts;
        CAPTURE(chart);
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        REQUIRE_FALSE(rec.paths.empty());
        CAPTURE(rec.best_path().pathstring());
        const std::vector<PinnedWindow> pins = pinned_windows(rec.best_path());

        const TargetResult target = search_target(song, cfg, pins);
        REQUIRE_FALSE(target.paths.empty());
        const TargetResult got = best_finish(song, cfg, pins);
        REQUIRE_FALSE(got.paths.empty());
        CHECK(got.realized_prefix == pins.size());
        const Path* own = nullptr;
        for (const Path* p : roots_and_tied(got.paths))
            if (p->walk_activations().size() == pins.size()) {
                own = p;
                break;
            }
        REQUIRE(own != nullptr);
        CHECK(own->totalscore() == target.paths.front().totalscore());
        CHECK(replay_path(song, windows_for_path(*own), cfg.rules).final.total() ==
              own->totalscore());
    }
    CHECK(charts > 0);
}

// Mutemath "Allies": its best path's first window fits the default 10 ms
// Path limit, so pinning it in full leaves the best total where it was:
// BF(1) == BF(0). The total is pinned from one run.
TEST_CASE("best finish: pinning the best path's first window keeps the best total") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::string chart = corpus_chart("Mutemath - Allies");
    REQUIRE_FALSE(chart.empty());
    const Song& song = corpus::song(chart, cfg.reading());
    const HydraRecord& rec = corpus::analyzed(chart, cfg);
    REQUIRE_FALSE(rec.paths.empty());
    const Path& best = rec.best_path();
    REQUIRE(cfg.ms_filter.has_value());
    REQUIRE_FALSE(best.all_activations().empty());
    REQUIRE(best.all_activations().front().within_ms_limit(*cfg.ms_filter));

    const ScoreGraph graph = search_graph(song, cfg);
    const TargetResult bf0 = best_finish(graph, cfg, {});
    const TargetResult bf1 = best_finish(graph, cfg, {pinned_windows(best).front()});
    REQUIRE_FALSE(bf0.paths.empty());
    REQUIRE_FALSE(bf1.paths.empty());
    CHECK(bf1.paths.front().totalscore() == bf0.paths.front().totalscore());
    CHECK(bf0.paths.front().totalscore() == 451830);
}

// A pin one tick after the best path's first activation is no fill node, so
// no path activates there. best_finish reports what search_target reports:
// that window, as the first one no path realizes, broken at its activation.
TEST_CASE("best finish: an impossible window reports search_target's reason") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::string chart = corpus_chart("Mutemath - Allies");
    REQUIRE_FALSE(chart.empty());
    const Song& song = corpus::song(chart, cfg.reading());
    const HydraRecord& rec = corpus::analyzed(chart, cfg);
    REQUIRE_FALSE(rec.paths.empty());
    const std::vector<int64_t> ticks = test::act_ticks(rec.best_path());
    REQUIRE_FALSE(ticks.empty());
    const int64_t nowhere = ticks.front() + 1;

    const std::vector<PinnedWindow> pins = {PinnedWindow{nowhere}};
    const TargetResult target = search_target(song, cfg, pins);
    const TargetResult got = best_finish(song, cfg, pins);
    CHECK(got.paths.empty());
    CHECK(got.failed_reason == "activation");
    CHECK(got.failed_tick == std::optional<int64_t>(nowhere));
    CHECK(got.realized_prefix == 0);
    CHECK(got.failed_reason == target.failed_reason);
    CHECK(got.failed_tick == target.failed_tick);
    CHECK(got.realized_prefix == target.realized_prefix);
}
