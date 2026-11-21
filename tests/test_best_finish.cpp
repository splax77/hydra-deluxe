// best_finish: the best path that starts with the player's pinned windows and
// runs free after them (the Path Builder's BF(k), plan
// docs/superpowers/plans/2026-10-28-path-builder.md (removed; git show b29a3c28:docs/superpowers/plans/2026-10-28-path-builder.md), task EN-2). Every case
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

// Runs `visit(song, record)` on every corpus chart that has notes, at `cfg`.
// A chart with no notes at cfg's difficulty has no path to finish: it is
// named, and the analysis must refuse it with NoNotesError, so a chart lost
// any other way fails. Every chart is either visited or refused that way.
template <typename Visit>
void on_every_corpus_chart(const app::AnalysisSettings& cfg, Visit&& visit) {
    const std::vector<std::string>& all = corpus::chart_paths();
    size_t visited = 0;
    size_t refused = 0;
    for (const std::string& chart : all) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) {
            const std::string msg = "no notes at this difficulty, so no best finish: " + chart;
            MESSAGE(msg);
            CHECK_THROWS_AS((void)corpus::analyzed(chart, cfg), NoNotesError);
            ++refused;
            continue;
        }
        ++visited;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        REQUIRE_FALSE(rec.paths.empty());
        visit(song, rec);
    }
    CHECK(visited + refused == all.size());
    // With no chart visited, neither identity below was checked at all.
    REQUIRE_MESSAGE(visited > 0, "no corpus chart has notes at the default difficulty");
}

}  // namespace

// Identity 1: with nothing pinned, the best finish is the main search's best
// path, string and total, on every corpus chart.
TEST_CASE("best finish: with nothing pinned it is Hydra's best path on every corpus chart") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    on_every_corpus_chart(cfg, [&](const Song& song, const HydraRecord& rec) {
        const TargetResult got = best_finish(song, cfg, {});
        REQUIRE_FALSE(got.paths.empty());
        CHECK(got.paths.front().pathstring() == rec.best_path().pathstring());
        CHECK(got.paths.front().totalscore() == rec.best_path().totalscore());
        CHECK(got.realized_prefix == 0);
    });
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
    on_every_corpus_chart(cfg, [&](const Song& song, const HydraRecord& rec) {
        CAPTURE(rec.best_path().pathstring());
        const std::vector<PinnedWindow> pins = pinned_windows(rec.best_path());

        const TargetResult target = search_target(song, cfg, pins);
        REQUIRE_FALSE(target.paths.empty());
        const TargetResult got = best_finish(song, cfg, pins);
        REQUIRE_FALSE(got.paths.empty());
        CHECK(got.realized_prefix == pins.size());
        const Path* own = nullptr;
        for (const Path* p : test::roots_and_tied(got.paths))
            if (p->walk_activations().size() == pins.size()) {
                own = p;
                break;
            }
        REQUIRE(own != nullptr);
        CHECK(own->totalscore() == target.paths.front().totalscore());
        CHECK(replay_path(song, windows_for_path(*own), cfg.rules).final.total() ==
              own->totalscore());
    });
}

// Mutemath "Allies": its best path's first window fits the default Path
// limit (the REQUIRE below checks it), so pinning it in full leaves the best
// total where it was: BF(1) == BF(0).
TEST_CASE("best finish: pinning the best path's first window keeps the best total") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::string chart = corpus::chart_named("Mutemath - Allies");
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
}

// A pin one tick after the best path's first activation is no fill node, so
// no path activates there. best_finish reports what search_target reports:
// that window, as the first one no path realizes, broken at its activation.
TEST_CASE("best finish: an impossible window reports search_target's reason") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::string chart = corpus::chart_named("Mutemath - Allies");
    REQUIRE_FALSE(chart.empty());
    const Song& song = corpus::song(chart, cfg.reading());
    const HydraRecord& rec = corpus::analyzed(chart, cfg);
    REQUIRE_FALSE(rec.paths.empty());
    const std::vector<int64_t> ticks = test::act_ticks(rec.best_path());
    REQUIRE_FALSE(ticks.empty());
    const int64_t nowhere = ticks.front() + 1;

    const std::vector<PinnedWindow> pins = activation_pins({nowhere});
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
