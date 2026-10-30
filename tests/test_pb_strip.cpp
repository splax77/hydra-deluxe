// The Path Builder's strip (src/pathbuilder/pb_strip.h): the step line's
// levels read from the ladder, and the fill markers and their click areas.
// The ladder comes from the corpus, as test_pb_coach.cpp builds it; every
// figure is pinned from one run.

#include "doctest.h"

#include <optional>
#include <vector>

#include "pb_open_util.h"
#include "scratch_settings.h"
#include "testdata_dirs.h"

#include "pathbuilder/pb_coach.h"
#include "pathbuilder/pb_state.h"
#include "pathbuilder/pb_strip.h"

using hydra::ActivationWalk;
using hydra::app::LadderRun;
using hydra::app::PreviewFill;
using hydra::app::PreviewFillState;
using hydra::app::PreviewScene;
using hydra::pathbuilder::ClickResult;
using hydra::pathbuilder::OpenChart;
using hydra::pathbuilder::PbState;
using hydra::pathbuilder::StripMarker;

TEST_CASE("strip: the step line starts at BF(0), steps by each cost, ends at the player's total") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const ActivationWalk best = c.record.best_path().walk_activations();
    REQUIRE(best.size() >= 3);

    // The two-click path test_pb_coach.cpp builds: the best path's second
    // fill, then its third.
    const ClickResult one =
        hydra::pathbuilder::start_window(*c.graph, c.settings, {}, best[1].timecode.ticks());
    REQUIRE(one.windows.has_value());
    const ClickResult two = hydra::pathbuilder::start_window(*c.graph, c.settings, *one.windows,
                                                             best[2].timecode.ticks());
    REQUIRE(two.windows.has_value());
    const LadderRun run = hydra::app::build_ladder(*c.graph, c.settings, *two.windows);
    REQUIRE(run.complete);

    const std::vector<int64_t> levels = hydra::pathbuilder::step_levels(run, two.windows->size());
    // From one run: BF(0), one level per window, then the player's total.
    CHECK(levels == std::vector<int64_t>{378315, 364675, 364675, 364675});
    CHECK(levels.front() == run.ladder.bf.front());
    CHECK(levels.back() == run.ladder.player_total);
    for (size_t k = 1; k < levels.size(); ++k) CHECK(levels[k] <= levels[k - 1]);

    // A run still going has no player's total yet: the line stops at the
    // last window priced.
    LadderRun partial = run;
    partial.complete = false;
    partial.ladder.bf.resize(2);
    CHECK(hydra::pathbuilder::step_levels(partial, two.windows->size()) ==
          std::vector<int64_t>{378315, 364675});
    CHECK(hydra::pathbuilder::step_levels(LadderRun{}, two.windows->size()).empty());
}

TEST_CASE("strip: no step line while the path on screen has no windows") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();

    // The ladder over no windows still lands BF(0) and a total, as the
    // decision list's ladder does for an empty path.
    const LadderRun run = hydra::app::build_ladder(*c.graph, c.settings, {});
    REQUIRE(run.complete);
    REQUIRE(run.ladder.bf.size() == 1);
    CHECK(run.ladder.bf.front() == 378315);
    // The user's ruling, "Hide it": only the markers until a window is added.
    CHECK(hydra::pathbuilder::step_levels(run, 0).empty());
    // The same run read as a path with a window keeps its line.
    CHECK_FALSE(hydra::pathbuilder::step_levels(run, 1).empty());
}

TEST_CASE("strip: markers leave hidden fills out and click exactly where drawn") {
    PreviewScene scene;
    auto fill = [](double start_ms, double end_ms, PreviewFillState state) {
        PreviewFill f;
        f.span.start_ms = start_ms;
        f.span.end_ms = end_ms;
        f.state = state;
        return f;
    };
    scene.fills = {fill(1000, 3000, PreviewFillState::Offered),
                   fill(4000, 6000, PreviewFillState::Hidden),
                   fill(7000, 7010, PreviewFillState::Taken)};
    // A 1000-pixel strip over 10 s: 0.1 px per ms.
    const std::vector<StripMarker> m = hydra::pathbuilder::strip_markers(scene, 10000.0, 1000.0f, 3.0f);
    REQUIRE(m.size() == 2);
    CHECK(m[0].fill == 0);
    CHECK(m[0].state == PreviewFillState::Offered);
    CHECK(m[0].x0 == doctest::Approx(100.0f));
    CHECK(m[0].x1 == doctest::Approx(300.0f));
    // The short fill is widened to the minimum, about its span's middle.
    CHECK(m[1].fill == 2);
    CHECK(m[1].x0 == doctest::Approx(699.0f));
    CHECK(m[1].x1 == doctest::Approx(702.0f));

    CHECK(hydra::pathbuilder::marker_at(m, 100.0f) == std::optional<size_t>(0));
    CHECK(hydra::pathbuilder::marker_at(m, 300.0f) == std::optional<size_t>(0));
    CHECK_FALSE(hydra::pathbuilder::marker_at(m, 99.0f).has_value());
    CHECK_FALSE(hydra::pathbuilder::marker_at(m, 500.0f).has_value());  // the hidden fill
    CHECK(hydra::pathbuilder::marker_at(m, 700.0f) == std::optional<size_t>(2));
}
