// The Path Builder's decision list (src/pathbuilder/pb_coach.h): the rows a
// ladder fills, the tight-window note, the end choices, "Finish for me", the
// clicks that start and remove a window, and the worker a newer request
// supersedes.
// Charts come from the corpus; every figure is pinned from one run.

#include "doctest.h"

#include <string>
#include <vector>

#include "scratch_settings.h"
#include "testdata_dirs.h"
#include "pb_open_util.h"

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
    open_and_wait(s, corpus::white_washed_notes(), limited);
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

TEST_CASE("coach rows: a squeeze with time to spare under a lower limit names the limit alone") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    // From one run: White Washed's all-0 path ends on a squeeze-in with
    // 130.4 ms to spare, so it needs no timing.
    REQUIRE_FALSE(c.record.allzero_paths.empty());
    const Path& allzero = c.record.allzero_paths.front();
    CHECK(allzero.pathstring() == "0 0 0 0 0 0+");
    const ActivationWalk acts = allzero.walk_activations();
    REQUIRE(acts.size() == 6);
    const Activation& spare = acts[5];
    CHECK_FALSE(spare.difficulty().has_value());
    // A limit below its figure: the window is over it, with no timing to name.
    CHECK_FALSE(spare.within_ms_limit(-200.0));
    CHECK(hydra::pathbuilder::tight_window_note(spare, -200.0) == "over your limit of -200 ms");
    // Inside a limit it meets, no note.
    CHECK(hydra::pathbuilder::tight_window_note(spare, -10.0).empty());
}

TEST_CASE("coach click: a later window keeps the end the player chose while it can") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    // The best path's first four windows, the fourth given its squeeze-in end.
    std::vector<PinnedWindow> four = hydra::pinned_windows(c.record.best_path());
    four.resize(4);
    const std::vector<PinnedWindow> before(four.begin(), four.begin() + 3);
    const auto ends = hydra::pathbuilder::end_choices(*c.graph, c.settings, before, four[3].act_tick);
    // From one run: the squeeze-in end, then the squeeze-out end.
    REQUIRE(ends.size() == 2);
    CHECK(ends[0].token == "0+");
    CHECK(ends[0].window.deact_tick == 257280);
    CHECK(ends[1].token == "0-");
    CHECK(ends[1].window.deact_tick == 253440);
    const ClickResult chosen = hydra::pathbuilder::choose_end(*c.graph, c.settings, four, ends[0].window);
    REQUIRE(chosen.windows.has_value());
    REQUIRE(chosen.path.has_value());
    CHECK(chosen.path->pathstring() == "0 1 3 0+");

    // An edit before it (the first window given its own end again) keeps the
    // fourth window's squeeze-in, though the best finish would pick the
    // squeeze-out for it.
    const ClickResult again =
        hydra::pathbuilder::choose_end(*c.graph, c.settings, *chosen.windows, chosen.windows->front());
    REQUIRE(again.windows.has_value());
    REQUIRE(again.path.has_value());
    CHECK(again.message.empty());
    CHECK(again.notice.empty());
    CHECK((*again.windows)[3].deact_tick == 257280);
    CHECK(again.path->pathstring() == "0 1 3 0+");
    std::vector<PinnedWindow> act_only = *chosen.windows;
    act_only[3] = PinnedWindow{act_only[3].act_tick, std::nullopt, false, std::nullopt};
    const hydra::TargetResult picked = hydra::best_finish(*c.graph, c.settings, act_only);
    REQUIRE_FALSE(picked.paths.empty());
    CHECK(hydra::pinned_windows(picked.paths.front())[3].deact_tick == 253440);
}

TEST_CASE("coach click: a later end that is no longer possible is picked again, and said so") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> best = hydra::pinned_windows(c.record.best_path());
    REQUIRE(best.size() == 3);
    const auto ends = hydra::pathbuilder::end_choices(*c.graph, c.settings, {}, best[0].act_tick);
    // From one run: the first window can end on a squeeze-in instead.
    REQUIRE_FALSE(ends.empty());
    CHECK(ends[0].token == "3+");
    CHECK(ends[0].window.deact_tick == 74880);
    const ClickResult r = hydra::pathbuilder::choose_end(*c.graph, c.settings, best, ends[0].window);
    REQUIRE(r.windows.has_value());
    REQUIRE(r.path.has_value());
    CHECK(r.message.empty());
    CHECK(r.notice == hydra::pathbuilder::kLaterEndRepickedText);
    CHECK(r.path->pathstring() == "3+ 0 2");
    CHECK((*r.windows)[1].deact_tick != best[1].deact_tick);
}

TEST_CASE("coach remove: the later windows keep the ends the player chose while they can") {
    PbState s;
    open_and_wait(s, white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> best = hydra::pinned_windows(c.record.best_path());
    REQUIRE(best.size() == 5);
    CHECK(c.record.best_path().pathstring() == "0 1 3 0- 0");
    // From one run: removing the second window keeps every later end.
    const ClickResult r =
        hydra::pathbuilder::remove_window(*c.graph, c.settings, best, best[1].act_tick);
    REQUIRE(r.windows.has_value());
    REQUIRE(r.path.has_value());
    CHECK(r.message.empty());
    CHECK(r.notice.empty());
    REQUIRE(r.windows->size() == 4);
    CHECK((*r.windows)[0].act_tick == 42240);
    CHECK((*r.windows)[0].deact_tick == 53760);
    for (size_t i = 1; i < 4; ++i) {
        CHECK((*r.windows)[i].act_tick == best[i + 1].act_tick);
        CHECK((*r.windows)[i].deact_tick == best[i + 1].deact_tick);
    }
    CHECK(r.path->pathstring() == "0 7 0- 0");
    CHECK(r.path->totalscore() == 667020);
}

TEST_CASE("coach remove: a later end that is no longer possible is picked again, and said so") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> best = hydra::pinned_windows(c.record.best_path());
    REQUIRE(best.size() == 3);
    // From one run: without the first window, the second can no longer end
    // where it did.
    const ClickResult r =
        hydra::pathbuilder::remove_window(*c.graph, c.settings, best, best[0].act_tick);
    REQUIRE(r.windows.has_value());
    REQUIRE(r.path.has_value());
    CHECK(r.message.empty());
    CHECK(r.notice == hydra::pathbuilder::kLaterEndRepickedText);
    REQUIRE(r.windows->size() == 2);
    CHECK((*r.windows)[0].act_tick == 109440);
    CHECK((*r.windows)[0].deact_tick == 128640);
    CHECK(best[1].deact_tick == 120960);
    CHECK(r.path->pathstring() == "9 1");
    CHECK(r.path->totalscore() == 364675);
}

TEST_CASE("coach remove: removing every window leaves the empty path") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    std::vector<PinnedWindow> windows = hydra::pinned_windows(c.record.best_path());
    REQUIRE(windows.size() == 3);
    // Last window first: from one run, the first two keep their ends.
    const ClickResult two =
        hydra::pathbuilder::remove_window(*c.graph, c.settings, windows, windows[2].act_tick);
    REQUIRE(two.windows.has_value());
    REQUIRE(two.path.has_value());
    CHECK(two.notice.empty());
    REQUIRE(two.windows->size() == 2);
    CHECK((*two.windows)[0].deact_tick == windows[0].deact_tick);
    CHECK((*two.windows)[1].deact_tick == windows[1].deact_tick);
    CHECK(two.path->pathstring() == "3- 1");
    CHECK(two.path->totalscore() == 358235);
    windows = *two.windows;
    while (!windows.empty()) {
        const ClickResult r = hydra::pathbuilder::remove_window(*c.graph, c.settings, windows,
                                                                windows.back().act_tick);
        REQUIRE(r.windows.has_value());
        CHECK(r.message.empty());
        REQUIRE(r.windows->size() + 1 == windows.size());
        windows = *r.windows;
        // A window left has a path; none left has none (PlayerPath's rule).
        CHECK(r.path.has_value() == !windows.empty());
    }
}

TEST_CASE("coach remove: a tick with no window changes nothing") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> best = hydra::pinned_windows(c.record.best_path());
    const ClickResult r = hydra::pathbuilder::remove_window(*c.graph, c.settings, best, 0);
    CHECK_FALSE(r.windows.has_value());
    CHECK_FALSE(r.path.has_value());
    CHECK(r.message.empty());
    CHECK(r.notice.empty());
}

TEST_CASE("coach remove: a removal that breaks a later window leaves the path and says why") {
    PbState s;
    open_and_wait(s, white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> best = hydra::pinned_windows(c.record.best_path());
    REQUIRE(best.size() == 5);
    // From one run: without the third window, a later one cannot happen.
    const ClickResult r =
        hydra::pathbuilder::remove_window(*c.graph, c.settings, best, best[2].act_tick);
    CHECK_FALSE(r.windows.has_value());
    CHECK_FALSE(r.path.has_value());
    CHECK(r.message == hydra::pathbuilder::impossible_window_text("activation", false));
    CHECK(r.notice.empty());
}

TEST_CASE("coach rows: a path with no window has no after-your-last-window line") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const LadderRun run = ladder(c, {});
    REQUIRE(run.complete);
    const CoachList list = hydra::pathbuilder::coach_list(Path{}, run, c.settings);
    CHECK(list.rows.empty());
    CHECK_FALSE(list.after_last.has_value());
    CHECK_FALSE(list.after_last_tier.has_value());
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
