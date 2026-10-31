// The Path Builder's decision list (src/pathbuilder/pb_coach.h): the rows a
// ladder fills, the tight-window note, the end choices, "Finish the rest for
// me", the clicks that start and remove a window, the candidate costs and
// queue, and the workers a newer request supersedes.
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

TEST_CASE("coach rows: a two-click path fills the ladder, the rows, the ends and Finish the rest for me") {
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

    // Finish the rest for me is BF(2)'s path.
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

// A click on an unused fill that sits before a window of the path used to
// fail with the generic sentence, since the new window was pinned with no end
// before windows pinned with theirs (sort_and_check_windows).
TEST_CASE("coach click: a fill that rules out a later window is refused in the later-window sentence") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> best = hydra::pinned_windows(c.record.best_path());
    REQUIRE(best.size() == 3);
    constexpr int64_t kFillTick = 36480;
    REQUIRE(kFillTick < best[0].act_tick);

    // From one run: this fill can end one way, and with it the first window
    // (tick 59520) cannot happen, even with its activation alone. The click
    // is refused in the later-window sentence, not the generic one.
    const ClickResult r = hydra::pathbuilder::start_window(*c.graph, c.settings, best, kFillTick);
    CHECK_FALSE(r.windows.has_value());
    CHECK_FALSE(r.path.has_value());
    CHECK(r.message == hydra::pathbuilder::impossible_window_text("activation", false));
    CHECK(r.detail.empty());
}

TEST_CASE("coach end: choosing the last window's end pins it") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    // The best path's first four windows; the fourth is the last.
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
    CHECK(chosen.message.empty());
    REQUIRE(chosen.windows->size() == 4);
    CHECK((*chosen.windows)[3].deact_tick == 257280);
    CHECK(chosen.path->pathstring() == "0 1 3 0+");
}

// The player builds left to right (plan 2026-10-30, Q1), so Remove takes the
// window and every later one.
TEST_CASE("coach remove: removing a window drops it and every later one") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> best = hydra::pinned_windows(c.record.best_path());
    REQUIRE(best.size() == 3);

    // The second window: the first keeps its end. From one run.
    const ClickResult r =
        hydra::pathbuilder::remove_window(*c.graph, c.settings, best, best[1].act_tick);
    REQUIRE(r.windows.has_value());
    REQUIRE(r.path.has_value());
    CHECK(r.message.empty());
    REQUIRE(r.windows->size() == 1);
    CHECK((*r.windows)[0].act_tick == best[0].act_tick);
    CHECK((*r.windows)[0].deact_tick == best[0].deact_tick);
    CHECK(r.path->pathstring() == "3-");
    CHECK(r.path->totalscore() == 334495);

    // The first window: nothing is left.
    const ClickResult none =
        hydra::pathbuilder::remove_window(*c.graph, c.settings, best, best[0].act_tick);
    REQUIRE(none.windows.has_value());
    CHECK(none.windows->empty());
    CHECK_FALSE(none.path.has_value());
    CHECK(none.message.empty());
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
}

TEST_CASE("coach finish: Finish the rest for me makes BF(n)'s path the player's path as one edit") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const ActivationWalk best = c.record.best_path().walk_activations();
    REQUIRE(best.size() >= 2);
    const ClickResult one = hydra::pathbuilder::start_window(*c.graph, c.settings, {},
                                                             best[1].timecode.ticks());
    REQUIRE(one.windows.has_value());
    REQUIRE(one.path.has_value());
    hydra::pathbuilder::DecisionList& d = s.decisions();
    d.show(s.search_graph(), c.settings, &*one.path, "one window");
    testwait::poll_until_idle(d, "the ladder to land", s);
    REQUIRE(d.can_finish());

    // Finishing leaves the player's history one edit longer, holding the
    // path behind BF(1) (the ladder's own answer), and tells the screen to
    // show the player's path.
    const LadderRun run = ladder(c, *one.windows);
    const uint64_t before = s.history().generation();
    d.finish(s);
    CHECK(s.history().generation() == before + 1);
    REQUIRE(s.history().current().path.has_value());
    CHECK(s.history().current().path->pathstring() == run.best[1].pathstring());
    CHECK(s.history().current().path->pathstring() == "9 1");
    CHECK(d.take_player_changed());
    CHECK(d.message().empty());
}

TEST_CASE("coach click: a result that arrives after the player's path changed is dropped") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const ActivationWalk best = c.record.best_path().walk_activations();
    REQUIRE(best.size() >= 2);
    const int64_t tick = best[1].timecode.ticks();
    hydra::pathbuilder::DecisionList& d = s.decisions();
    auto settle = [&] { testwait::poll_until_idle(d, "the click's search to land", s); };

    // The forced order: the click starts, the player's path changes (the
    // empty path stands in for an undo, a redo or Finish the rest for me), then the
    // answer is taken in.
    d.click_fill(s.search_graph(), c.settings, s.history().current().windows, tick,
                 s.history().generation());
    s.history().edit(hydra::pathbuilder::PlayerPath{});
    const uint64_t after_change = s.history().generation();
    settle();
    CHECK(s.history().generation() == after_change);  // the answer made no edit
    CHECK(s.history().current().windows.empty());
    CHECK_FALSE(d.take_player_changed());
    CHECK(d.message().empty());

    // The same click made on the path as it now stands is taken in.
    d.click_fill(s.search_graph(), c.settings, s.history().current().windows, tick,
                 s.history().generation());
    settle();
    CHECK(s.history().generation() == after_change + 1);
    REQUIRE(s.history().current().windows.size() == 1);
    CHECK(s.history().current().windows.front().act_tick == tick);
    CHECK(d.take_player_changed());
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
    testwait::poll_until_idle(coach, "the coach's ladders to stop");
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

namespace {

// White Washed's best path cut to its first two windows: the candidate
// tests' player path.
std::vector<PinnedWindow> two_windows(const OpenChart& c) {
    std::vector<PinnedWindow> two = hydra::pinned_windows(c.record.best_path());
    two.resize(2);
    return two;
}

void settle_coach(hydra::pathbuilder::Coach& coach) {
    testwait::poll_until_idle(coach, "the coach's searches to stop");
}

// The ticks the candidate tests hand the queue, chosen from a corpus path:
// every activation of BF(2)'s path after the player's two windows, and the
// fills it passed over before each (Activation::skipped_fill_ticks), in chart
// order.
std::vector<int64_t> candidate_ticks(const LadderRun& run) {
    std::vector<int64_t> ticks;
    const ActivationWalk acts = run.best.at(2).walk_activations();
    for (size_t i = 2; i < acts.size(); ++i) {
        ticks.insert(ticks.end(), acts[i].skipped_fill_ticks.begin(),
                     acts[i].skipped_fill_ticks.end());
        ticks.push_back(acts[i].timecode.ticks());
    }
    return ticks;
}

}  // namespace

TEST_CASE("candidate costs: the best next fill costs 0 and every other cost is BF(n) minus its best finish") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> windows = two_windows(c);
    const LadderRun run = ladder(c, windows);
    REQUIRE(run.complete);
    // From one run: BF(0), BF(1) and BF(2) agree, and BF(2)'s path is the
    // best path, whose next activation after the two windows is at 199680.
    CHECK(run.ladder.bf == std::vector<int64_t>{706180, 706180, 706180});
    CHECK(run.best[2].pathstring() == "0 1 3 0- 0");
    const int64_t next = run.best[2].walk_activations()[2].timecode.ticks();
    CHECK(next == 199680);
    const std::vector<int64_t> ticks = candidate_ticks(run);
    CHECK(ticks == std::vector<int64_t>{149760, 165120, 180480, 199680, 245760, 272640});

    // The screen's way in: show the player's path, hand it the ticks.
    const hydra::TargetResult mine = hydra::search_target(*c.graph, windows);
    REQUIRE_FALSE(mine.paths.empty());
    hydra::pathbuilder::DecisionList& d = s.decisions();
    d.show(s.search_graph(), c.settings, &mine.paths.front(), "two windows");
    d.price_fills(ticks);
    testwait::poll_until_idle(d, "the candidate queue to finish", s);
    const std::vector<hydra::app::CandidateCost>& got = d.candidates();
    REQUIRE(got.size() == ticks.size());
    // From one run: each fill's BF(2 + f), and its cost against BF(2),
    // 706180. The zero is BF(2)'s own next activation.
    struct Pinned {
        int64_t tick, best_finish, cost;
        CostTier tier;
    };
    const std::vector<Pinned> want = {
        {149760, 705640, 540, CostTier::Yellow}, {165120, 678120, 28060, CostTier::Red},
        {180480, 691800, 14380, CostTier::Red},  {199680, 706180, 0, CostTier::Green},
        {245760, 664160, 42020, CostTier::Red},  {272640, 653220, 52960, CostTier::Red},
    };
    for (size_t i = 0; i < want.size(); ++i) {
        CAPTURE(i);
        CHECK(got[i].act_tick == want[i].tick);
        CHECK(got[i].best_finish == want[i].best_finish);
        CHECK(got[i].cost == want[i].cost);
        CHECK(got[i].tier == want[i].tier);
        CHECK((got[i].cost == 0) == (got[i].act_tick == next));
    }
}

TEST_CASE("candidate queue: it prices only the ticks it is given, in order") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> windows = two_windows(c);
    hydra::pathbuilder::Coach coach;
    coach.request(c.graph, c.settings, windows, "two windows");
    // Two fills, handed out of chart order.
    REQUIRE(coach.request_candidates({199680, 165120}));
    CHECK_FALSE(coach.request_candidates({199680, 165120}));  // the same ticks start nothing
    settle_coach(coach);
    CHECK(coach.error().empty());
    const std::vector<hydra::app::CandidateFinish> got = coach.candidates();
    REQUIRE(got.size() == 2);
    CHECK(got[0].act_tick == 165120);
    CHECK(got[1].act_tick == 199680);
    for (const hydra::app::CandidateFinish& f : got) {
        std::vector<PinnedWindow> pins = windows;
        pins.push_back(PinnedWindow{f.act_tick, std::nullopt, false, std::nullopt});
        const hydra::TargetResult bf = hydra::best_finish(*c.graph, c.settings, pins);
        REQUIRE_FALSE(bf.paths.empty());
        CHECK(f.best_finish == bf.paths.front().totalscore());
    }
}

TEST_CASE("candidate queue: a newer request supersedes the older") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> windows = two_windows(c);
    const std::vector<int64_t> all = {149760, 165120, 180480, 199680, 245760, 272640};
    hydra::pathbuilder::Coach coach;
    // The ladder first, so a queue asked for starts at once.
    coach.request(c.graph, c.settings, windows, "two windows");
    settle_coach(coach);

    // Newer ticks: the queue just started is still on its first search, so
    // it is cancelled, and only the new ticks are priced.
    coach.request_candidates(all);
    REQUIRE(coach.busy());
    coach.request_candidates({199680});
    CHECK(coach.candidates_superseded() == 1);
    settle_coach(coach);
    const std::vector<hydra::app::CandidateFinish> got = coach.candidates();
    REQUIRE(got.size() == 1);
    CHECK(got[0].act_tick == 199680);

    // A newer path: its ladder cancels the queue, whose ticks were for the
    // old windows, and nothing is priced until the screen hands new ticks.
    coach.request_candidates(all);
    REQUIRE(coach.busy());
    coach.request(c.graph, c.settings, {}, "no windows");
    CHECK(coach.candidates_superseded() == 2);
    settle_coach(coach);
    CHECK(coach.candidates().empty());
    CHECK(coach.error().empty());
}
