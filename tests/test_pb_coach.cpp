// The Path Builder's decision list (src/pathbuilder/pb_coach.h): the rows a
// ladder fills, the tight-window note, the end choices, "Finish for me", a
// click that starts a window, and the worker a newer request supersedes.
// Charts come from the corpus; every figure is pinned from one run.

#include "doctest.h"

#include <string>
#include <vector>

#include "scratch_settings.h"
#include "testdata_dirs.h"
#include "wait_util.h"

#include "pathbuilder/pb_coach.h"
#include "pathbuilder/pb_state.h"

using hydra::Activation;
using hydra::ActivationWalk;
using hydra::Path;
using hydra::PinnedWindow;
using hydra::app::CostTier;
using hydra::app::LadderRun;
using hydra::pathbuilder::ClickResult;
using hydra::pathbuilder::CoachList;
using hydra::pathbuilder::OpenChart;
using hydra::pathbuilder::PbState;

namespace {

std::string white_washed_notes() {
    return corpus::root() + "/common/IB24/T5/August Burns Red - White Washed/notes.mid";
}

// Opens `path` under `settings` and waits for the open to end.
void open_and_wait(PbState& s, const std::string& path, const hydra::app::Settings& settings) {
    s.open_with(path, settings);
    testwait::wait_until(
        [&] {
            s.poll();
            return !s.busy();
        },
        "the Path Builder's open to finish");
}

LadderRun ladder(const OpenChart& c, const std::vector<PinnedWindow>& windows) {
    return hydra::app::build_ladder(*c.graph, c.settings, windows);
}

}  // namespace

TEST_CASE("coach rows: a two-click path fills the ladder, the rows, the ends and Finish for me") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const ActivationWalk best = c.record.best_path().walk_activations();
    REQUIRE(best.size() >= 3);

    // Two clicks: the best path's second fill, then its third. Each pins the
    // activation alone and keeps the end the best finish chose.
    const int64_t first = best[1].timecode.ticks();
    const int64_t second = best[2].timecode.ticks();
    const ClickResult one = hydra::pathbuilder::start_window(*c.graph, c.settings, {}, first);
    REQUIRE(one.message.empty());
    REQUIRE(one.windows.has_value());
    REQUIRE(one.windows->size() == 1);
    CHECK(one.windows->front().act_tick == first);
    CHECK(one.windows->front().deact_tick.has_value());
    const ClickResult two =
        hydra::pathbuilder::start_window(*c.graph, c.settings, *one.windows, second);
    REQUIRE(two.message.empty());
    REQUIRE(two.windows.has_value());
    REQUIRE(two.windows->size() == 2);
    REQUIRE(two.path.has_value());

    const LadderRun run = ladder(c, *two.windows);
    REQUIRE(run.complete);
    // From one run: skipping the best path's first window costs 13,640.
    CHECK(run.ladder.bf == std::vector<int64_t>{378315, 364675, 364675});
    CHECK(run.ladder.player_total == 364675);
    CHECK(two.path->pathstring() == "9 1");
    CHECK(run.ladder.player_total == two.path->totalscore());

    const CoachList list = hydra::pathbuilder::coach_list(*two.path, run, c.settings);
    REQUIRE(list.rows.size() == 2);
    CHECK(list.rows[0].token == "9");
    CHECK(list.rows[0].cost == 13640);
    CHECK(list.rows[0].tier == CostTier::Red);
    CHECK(list.rows[1].token == "1");
    CHECK(list.rows[1].cost == 0);
    CHECK(list.rows[1].tier == CostTier::Green);
    CHECK(list.after_last == 0);
    // Rows inside the limit carry no note, and these need no timing.
    for (const auto& r : list.rows) {
        CHECK(r.note.empty());
        CHECK(r.timing.empty());
    }
    CHECK(list.failure.empty());

    // The first window's end choices: one end, from one run.
    const auto ends = hydra::pathbuilder::end_choices(*c.graph, c.settings, {}, first);
    REQUIRE(ends.size() == 1);
    CHECK(ends[0].token == "9");
    CHECK(ends[0].window.deact_tick == 128640);
    CHECK(ends[0].best_finish == 364675);

    // Finish for me is BF(2)'s path.
    const std::optional<Path> finish = hydra::pathbuilder::finish_for_me(run, 2);
    REQUIRE(finish.has_value());
    CHECK(finish->pathstring() == run.best[2].pathstring());
    CHECK(finish->totalscore() == run.ladder.bf[2]);
    CHECK(finish->pathstring() == "9 1");
}

TEST_CASE("coach click: an impossible click leaves the path unchanged and says why") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    // Tick 0 holds no fill, so nothing can activate there.
    const ClickResult r = hydra::pathbuilder::start_window(*c.graph, c.settings, {}, 0);
    CHECK_FALSE(r.windows.has_value());
    CHECK_FALSE(r.path.has_value());
    CHECK(r.message == hydra::pathbuilder::impossible_window_text("activation", true));
    CHECK(r.detail.empty());
}

TEST_CASE("coach rows: a window over the Path limit carries the tight-window note") {
    hydra::app::Settings limited = scratch_settings();
    limited.mslimit_enabled = true;
    limited.mslimit_value = 10;
    PbState s;
    open_and_wait(s, white_washed_notes(), limited);
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    REQUIRE(c.settings.ms_filter == std::optional<double>(10.0));
    const Path& best = c.record.best_path();
    const ActivationWalk acts = best.walk_activations();
    REQUIRE(acts.size() == 5);
    CHECK_FALSE(acts[3].within_ms_limit(10.0));

    // Hydra's best path on screen.
    const LadderRun run = ladder(c, hydra::pinned_windows(best));
    REQUIRE(run.complete);
    const CoachList list = hydra::pathbuilder::coach_list(best, run, c.settings);
    REQUIRE(list.rows.size() == 5);
    REQUIRE(list.rows[3].tier.has_value());
    CHECK(*list.rows[3].tier == CostTier::Green);
    CHECK(list.rows[0].note.empty());

    // Under no limit the same row has no note.
    hydra::SearchSettings open_limit = c.settings;
    open_limit.ms_filter.reset();
    CHECK(hydra::pathbuilder::coach_list(best, run, open_limit).rows[3].note.empty());

    // From one run: the tight window is the fourth, a squeeze-out.
    CHECK(list.rows[3].token == "0-");
    CHECK(list.rows[3].timing == "130.4 ms");
    CHECK(list.rows[3].note == "needs 130.4 ms, your limit is 10 ms");

    // The player's own path, the best path's first four windows pinned in
    // full: its fourth row carries the same note.
    std::vector<PinnedWindow> four = hydra::pinned_windows(best);
    four.resize(4);
    const hydra::TargetResult mine = hydra::search_target(*c.graph, four);
    REQUIRE_FALSE(mine.paths.empty());
    const LadderRun own = ladder(c, four);
    const CoachList own_list = hydra::pathbuilder::coach_list(mine.paths.front(), own, c.settings);
    REQUIRE(own_list.rows.size() == 4);
    CHECK(own_list.rows[3].note == "needs 130.4 ms, your limit is 10 ms");
    for (size_t i = 0; i < 3; ++i) CHECK(own_list.rows[i].note.empty());
}

TEST_CASE("coach worker: a newer request supersedes the older") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> best = hydra::pinned_windows(c.record.best_path());

    hydra::pathbuilder::Coach coach;
    coach.request(c.graph, c.settings, best, "first");
    coach.request(c.graph, c.settings, {}, "second");
    // The first was still running when the second came, so it was cancelled.
    CHECK(coach.superseded() == 1);
    CHECK(coach.key() == "second");
    testwait::wait_until(
        [&] {
            coach.poll();
            return !coach.busy();
        },
        "the coach's ladders to stop");
    const std::optional<LadderRun> run = coach.run();
    REQUIRE(run.has_value());
    CHECK(run->complete);
    CHECK(run->ladder.bf.size() == 1);  // the second request's: no window
    CHECK(coach.error().empty());
    // The same key again starts nothing.
    coach.request(c.graph, c.settings, best, "second");
    CHECK(coach.superseded() == 1);
    CHECK_FALSE(coach.busy());
}
