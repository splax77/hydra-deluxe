// The Path Builder's decision list (src/pathbuilder/pb_coach.h): the rows a
// ladder fills, the tight-window note, the end choices, "Finish the rest for
// me", the clicks that start and remove a window, the candidate costs and
// queue, and the workers a newer request supersedes.
// Charts come from the corpus; every expected figure is read from its owner.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

#include "scratch_settings.h"
#include "testdata_dirs.h"
#include "pb_open_util.h"

#include "app/config.h"
#include "pathbuilder/pb_coach.h"
#include "pathbuilder/pb_state.h"

using hydra::Activation;
using hydra::ActivationWalk;
using hydra::Path;
using hydra::PinnedWindow;
using hydra::app::LadderRun;
using hydra::pathbuilder::ClickResult;
using hydra::pathbuilder::CoachList;
using hydra::pathbuilder::OpenChart;
using hydra::pathbuilder::PbState;

namespace {

LadderRun ladder(const OpenChart& c, const std::vector<PinnedWindow>& windows) {
    return hydra::app::build_ladder(*c.graph, c.settings, windows);
}

// best_finish's best total with `pins` pinned: the search's own answer.
int64_t best_total(const OpenChart& c, const std::vector<PinnedWindow>& pins) {
    const hydra::TargetResult bf = hydra::best_finish(*c.graph, c.settings, pins);
    REQUIRE_FALSE(bf.paths.empty());
    return bf.paths.front().totalscore();
}

// BF(n + f): best_finish with `windows` pinned whole and one more window at
// `act_tick` pinned by its activation alone.
int64_t fresh_best_finish(const OpenChart& c, std::vector<PinnedWindow> windows, int64_t act_tick) {
    windows.push_back(PinnedWindow{act_tick, std::nullopt, false, std::nullopt});
    return best_total(c, windows);
}

// The path activates once per window, each on its window's tick.
void check_path_on_windows(const Path& path, const std::vector<PinnedWindow>& windows) {
    const ActivationWalk acts = path.walk_activations();
    REQUIRE(acts.size() == windows.size());
    for (size_t i = 0; i < acts.size(); ++i) {
        CAPTURE(i);
        CHECK(acts[i].timecode.ticks() == windows[i].act_tick);
    }
}

// Burnout's best path's second and third fills, clicked in turn on an empty
// path, and the ladder over the two windows the clicks made.
struct TwoClicks {
    int64_t first = 0;  // the first click's tick
    std::vector<PinnedWindow> windows;
    Path path;
    LadderRun run;
};

TwoClicks two_clicks(const OpenChart& c) {
    const ActivationWalk best = c.record.best_path().walk_activations();
    REQUIRE(best.size() >= 3);
    TwoClicks out;
    out.first = best[1].timecode.ticks();
    const int64_t second = best[2].timecode.ticks();
    // Each click pins the activation alone and keeps the end the best finish
    // chose.
    const ClickResult one = hydra::pathbuilder::start_window(*c.graph, c.settings, {}, out.first);
    REQUIRE(one.message.empty());
    REQUIRE(one.windows.has_value());
    REQUIRE(one.windows->size() == 1);
    CHECK(one.windows->front().act_tick == out.first);
    CHECK(one.windows->front().deact_tick.has_value());
    const ClickResult two =
        hydra::pathbuilder::start_window(*c.graph, c.settings, *one.windows, second);
    REQUIRE(two.message.empty());
    REQUIRE(two.windows.has_value());
    REQUIRE(two.windows->size() == 2);
    REQUIRE(two.path.has_value());
    out.windows = *two.windows;
    out.path = *two.path;
    out.run = ladder(c, out.windows);
    REQUIRE(out.run.complete);
    return out;
}

// Each row's tight-window note against its window under `limit`: none inside
// the limit; outside it, the note names the window's own timing
// (Activation::difficulty through format_ms) and the limit (format_ms_whole).
void check_tight_notes(const Path& shown, const CoachList& list, double limit) {
    const ActivationWalk acts = shown.walk_activations();
    REQUIRE(list.rows.size() == acts.size());
    for (size_t i = 0; i < acts.size(); ++i) {
        CAPTURE(i);
        const std::string& note = list.rows[i].note;
        if (acts[i].within_ms_limit(limit)) {
            CHECK(note.empty());
            continue;
        }
        const std::optional<double> needs = acts[i].difficulty();
        REQUIRE(needs.has_value());
        CHECK(note.find(hydra::format_ms(*needs)) != std::string::npos);
        CHECK(note.find(hydra::format_ms_whole(limit)) != std::string::npos);
    }
}

}  // namespace

TEST_CASE("coach rows: a two-click path's rows read their costs and tiers from the ladder") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const TwoClicks p = two_clicks(c);
    check_path_on_windows(p.path, p.windows);
    CHECK(p.run.ladder.player_total == p.path.totalscore());

    const hydra::app::CostList costs = hydra::app::cost_list(p.run.ladder);
    const int64_t bf0 = p.run.ladder.bf.front();
    const ActivationWalk acts = p.path.walk_activations();
    const CoachList list = hydra::pathbuilder::coach_list(p.path, p.run, c.settings);
    REQUIRE(list.rows.size() == acts.size());
    REQUIRE(costs.windows.size() == list.rows.size());
    for (size_t i = 0; i < list.rows.size(); ++i) {
        CAPTURE(i);
        const hydra::pathbuilder::CoachRow& row = list.rows[i];
        CHECK(row.act_tick == acts[i].timecode.ticks());
        CHECK(row.token == acts[i].notationstr());
        REQUIRE(row.cost.has_value());
        CHECK(*row.cost == costs.windows[i]);
        REQUIRE(row.tier.has_value());
        CHECK(*row.tier == hydra::app::cost_tier(costs.windows[i], bf0));
        // A row inside the limit, or under no limit, carries no note.
        if (!c.settings.ms_filter || acts[i].within_ms_limit(*c.settings.ms_filter))
            CHECK(row.note.empty());
    }
    REQUIRE(list.after_last.has_value());
    CHECK(*list.after_last == costs.after_last);
    REQUIRE(list.after_last_tier.has_value());
    CHECK(*list.after_last_tier == hydra::app::cost_tier(costs.after_last, bf0));
    CHECK(list.failure.empty());
}

TEST_CASE("coach end: each of a clicked window's ends carries its own best finish, and the click kept one") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const TwoClicks p = two_clicks(c);
    const auto ends = hydra::pathbuilder::end_choices(*c.graph, c.settings, {}, p.first);
    REQUIRE_FALSE(ends.empty());
    size_t kept = 0;
    for (size_t e = 0; e < ends.size(); ++e) {
        CAPTURE(e);
        CHECK(ends[e].window.act_tick == p.first);
        CHECK(ends[e].best_finish == best_total(c, std::vector<PinnedWindow>{ends[e].window}));
        kept += hydra::pathbuilder::is_kept_end(p.windows.front(), ends[e]) ? 1 : 0;
    }
    CHECK(kept == 1);
}

TEST_CASE("coach finish: on a two-click path, Finish the rest for me is BF(2)'s path") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const TwoClicks p = two_clicks(c);
    const size_t n = p.windows.size();
    const std::optional<Path> finish = hydra::pathbuilder::finish_for_me(p.run, n);
    REQUIRE(finish.has_value());
    REQUIRE(p.run.best.size() > n);
    CHECK(finish->pathstring() == p.run.best[n].pathstring());
    CHECK(finish->totalscore() == p.run.ladder.bf[n]);
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

TEST_CASE("coach click: a refusal is current only while the player's path stays as it was") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    hydra::pathbuilder::DecisionList& d = s.decisions();
    CHECK_FALSE(d.message_current(s.history().generation()));  // nothing refused yet

    // Tick 0 holds no fill, so the click is refused and the path stays.
    const uint64_t at_click = s.history().generation();
    d.click_fill(s.search_graph(), c.settings, s.history().current().windows, 0, at_click);
    testwait::poll_until_idle(d, "the refused click's search to land", s);
    REQUIRE_FALSE(d.message().empty());
    CHECK(s.history().generation() == at_click);
    CHECK(d.message_current(at_click));

    // Any later edit of the player's path takes the refusal out of date.
    s.history().edit(hydra::pathbuilder::PlayerPath{});
    CHECK_FALSE(d.message_current(s.history().generation()));
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
    const double limit = *c.settings.ms_filter;
    const Path& best = c.record.best_path();
    const ActivationWalk acts = best.walk_activations();
    // The first of the best path's windows the limit rules out.
    size_t first_over = acts.size();
    for (size_t i = 0; i < acts.size(); ++i) {
        if (!acts[i].within_ms_limit(limit)) {
            first_over = i;
            break;
        }
    }
    REQUIRE(first_over < acts.size());

    // Hydra's best path on screen.
    const LadderRun run = ladder(c, hydra::pinned_windows(best));
    REQUIRE(run.complete);
    check_tight_notes(best, hydra::pathbuilder::coach_list(best, run, c.settings), limit);

    // Under no limit no row has a note.
    hydra::SearchSettings open_limit = c.settings;
    open_limit.ms_filter.reset();
    for (const hydra::pathbuilder::CoachRow& r :
         hydra::pathbuilder::coach_list(best, run, open_limit).rows)
        CHECK(r.note.empty());

    // The player's own path, the best path's windows up to the first one over
    // the limit pinned in full: its last row carries the note too.
    std::vector<PinnedWindow> mine_pins = hydra::pinned_windows(best);
    mine_pins.resize(first_over + 1);
    const hydra::TargetResult mine = hydra::search_target(*c.graph, mine_pins);
    REQUIRE_FALSE(mine.paths.empty());
    const LadderRun own = ladder(c, mine_pins);
    const CoachList own_list = hydra::pathbuilder::coach_list(mine.paths.front(), own, c.settings);
    check_tight_notes(mine.paths.front(), own_list, limit);
    REQUIRE_FALSE(own_list.rows.empty());
    CHECK_FALSE(own_list.rows.back().note.empty());
}

TEST_CASE("coach rows: a squeeze with time to spare under a lower limit names the limit alone") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    // An all-0 window whose every timing has time to spare: each figure is
    // below 0 (timing_needs_hitting), so Activation::difficulty is empty and
    // the window has no figure a failed limit could name. `room` is its
    // highest figure.
    std::optional<Activation> spare;
    double room = 0.0;
    for (const Path& p : c.record.allzero_paths) {
        const ActivationWalk acts = p.walk_activations();
        for (size_t i = 0; i < acts.size() && !spare; ++i) {
            if (acts[i].difficulty().has_value()) continue;
            const std::vector<hydra::ActivationTiming> timings = acts[i].badge_timings();
            if (timings.empty()) continue;
            bool all_spare = true;
            double highest = timings.front().ms;
            for (const hydra::ActivationTiming& t : timings) {
                all_spare = all_spare && !hydra::timing_needs_hitting(t.ms);
                highest = std::max(highest, t.ms);
            }
            if (!all_spare) continue;
            spare = acts[i];
            room = highest;
        }
        if (spare) break;
    }
    REQUIRE(spare.has_value());

    // Two whole-ms limits, as the Path limit setting holds them: one below
    // the room and one at or above it. Both are limits a player can set
    // (Settings::clamp leaves them as they are).
    using hydra::app::Settings;
    const int below = static_cast<int>(std::floor(room)) - 1;
    const int inside = static_cast<int>(std::ceil(room));
    CHECK(Settings::clamp(&Settings::mslimit_value, below) == below);
    CHECK(Settings::clamp(&Settings::mslimit_value, inside) == inside);

    // Below the room the window is over the limit, and the note names the
    // limit and none of the window's figures.
    CHECK_FALSE(spare->within_ms_limit(below));
    const std::string note = hydra::pathbuilder::tight_window_note(*spare, static_cast<double>(below));
    CHECK(note.find(hydra::format_ms_whole(below)) != std::string::npos);
    for (const hydra::ActivationTiming& t : spare->badge_timings())
        CHECK(note.find(hydra::format_ms(t.ms)) == std::string::npos);
    // Inside a limit it meets, no note.
    CHECK(spare->within_ms_limit(inside));
    CHECK(hydra::pathbuilder::tight_window_note(*spare, static_cast<double>(inside)).empty());
}

// A click on an unused fill before a window of the path, refused because a
// later window can no longer happen, gets the later-window sentence and not
// the generic one, even though the new window is pinned with no end before
// windows pinned with theirs (sort_and_check_windows).
TEST_CASE("coach click: a fill that rules out a later window is refused in the later-window sentence") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const Path& best_path = c.record.best_path();
    const std::vector<PinnedWindow> best = hydra::pinned_windows(best_path);
    const ActivationWalk acts = best_path.walk_activations();
    REQUIRE_FALSE(acts.empty());
    REQUIRE_FALSE(best.empty());

    // The fills the best path passed over before its first window
    // (Activation::skipped_fill_ticks): unused fills before every window.
    const std::vector<int64_t> unused = acts[0].skipped_fill_ticks;
    size_t refused = 0;
    for (const int64_t tick : unused) {
        CAPTURE(tick);
        REQUIRE(tick < best[0].act_tick);
        // On a path with no window the fill can be activated, so a refusal
        // once the path's windows are kept is a later window's doing.
        const ClickResult alone = hydra::pathbuilder::start_window(*c.graph, c.settings, {}, tick);
        if (!alone.windows.has_value()) continue;
        const ClickResult r = hydra::pathbuilder::start_window(*c.graph, c.settings, best, tick);
        if (r.windows.has_value()) continue;
        ++refused;
        CHECK_FALSE(r.path.has_value());
        CHECK(r.message == hydra::pathbuilder::impossible_window_text("activation", false));
        CHECK(r.detail.empty());
    }
    // Burnout's best path has such a fill: its first window cannot happen
    // once the earliest unused fill is taken.
    CHECK(refused > 0);
}

namespace {

// White Washed's best path cut to its first four windows, and the ends of
// the fourth, the last.
struct FourthWindowEnds {
    std::vector<PinnedWindow> four;
    std::vector<hydra::pathbuilder::EndChoice> ends;
};

FourthWindowEnds fourth_window_ends(const OpenChart& c) {
    FourthWindowEnds out;
    out.four = hydra::pinned_windows(c.record.best_path());
    REQUIRE(out.four.size() >= 4);
    out.four.resize(4);
    const std::vector<PinnedWindow> before(out.four.begin(), out.four.begin() + 3);
    out.ends = hydra::pathbuilder::end_choices(*c.graph, c.settings, before, out.four[3].act_tick);
    return out;
}

}  // namespace

TEST_CASE("coach end: choosing the last window's end pins it") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const auto [four, ends] = fourth_window_ends(c);
    REQUIRE_FALSE(ends.empty());
    size_t others = 0;  // ends the best path did not keep
    for (size_t e = 0; e < ends.size(); ++e) {
        CAPTURE(e);
        const hydra::pathbuilder::EndChoice& end = ends[e];
        others += hydra::pathbuilder::is_kept_end(four[3], end) ? 0 : 1;
        const ClickResult chosen =
            hydra::pathbuilder::choose_end(*c.graph, c.settings, four, end.window);
        REQUIRE(chosen.windows.has_value());
        REQUIRE(chosen.path.has_value());
        CHECK(chosen.message.empty());
        REQUIRE(chosen.windows->size() == four.size());
        const PinnedWindow& pinned = (*chosen.windows)[3];
        CHECK(pinned.deact_tick == end.window.deact_tick);
        CHECK(hydra::pathbuilder::is_kept_end(pinned, end));
        check_path_on_windows(*chosen.path, *chosen.windows);

        // Each end carries the activation the path takes with it, so the
        // screen reads the end's words from the end.
        const ActivationWalk acts = chosen.path->walk_activations();
        REQUIRE(acts.size() == four.size());
        CHECK(acts[3].notationstr() == end.activation.notationstr());
        const std::vector<hydra::ActivationTiming> want = end.activation.badge_timings();
        const std::vector<hydra::ActivationTiming> got = acts[3].badge_timings();
        REQUIRE(got.size() == want.size());
        for (size_t t = 0; t < got.size(); ++t) {
            CAPTURE(t);
            CHECK(got[t].part == want[t].part);
            CHECK(got[t].required == want[t].required);
            CHECK(hydra::format_ms(got[t].ms) == hydra::format_ms(want[t].ms));
        }
    }
    // At least one choice changes the window's end.
    CHECK(others > 0);
}

TEST_CASE("coach end: the kept end is the window's own end") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const auto [four, ends] = fourth_window_ends(c);
    REQUIRE(ends.size() == 2);
    // The best path kept the squeeze-out end (its fourth token is "0-").
    CHECK_FALSE(hydra::pathbuilder::is_kept_end(four[3], ends[0]));
    CHECK(hydra::pathbuilder::is_kept_end(four[3], ends[1]));
    CHECK(ends[1].window.deact_tick == four[3].deact_tick);

    // Once the player chooses the squeeze-in end, that end is the kept one.
    const ClickResult chosen = hydra::pathbuilder::choose_end(*c.graph, c.settings, four, ends[0].window);
    REQUIRE(chosen.windows.has_value());
    REQUIRE(chosen.windows->size() == 4);
    CHECK(hydra::pathbuilder::is_kept_end((*chosen.windows)[3], ends[0]));
    CHECK_FALSE(hydra::pathbuilder::is_kept_end((*chosen.windows)[3], ends[1]));
}

// An earlier step's endings are computed when the step is opened
// (DecisionList::select), never stored. The windows after it are not in
// `before`, so its list is the one it had when it was the last window.
TEST_CASE("coach end: an earlier window's endings are the same list") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> all = hydra::pinned_windows(c.record.best_path());
    // The fourth window must have one after it to be an earlier window.
    REQUIRE(all.size() > 4);

    // A window's ends as the screen opens it (DecisionList::select) on the
    // path of the first `count` windows.
    hydra::pathbuilder::DecisionList& d = s.decisions();
    auto ends_when_opened = [&](size_t row, size_t count) {
        const std::vector<PinnedWindow> windows(all.begin(),
                                                all.begin() + static_cast<std::ptrdiff_t>(count));
        const hydra::TargetResult found = hydra::search_target(*c.graph, windows);
        REQUIRE_FALSE(found.paths.empty());
        d.show(s.search_graph(), c.settings, &found.paths.front(), std::to_string(count));
        d.select(row);
        testwait::poll_until_idle(d, "the step's endings to land", s);
        return d.ends();
    };
    // The same ends, in the same order, at the same best finishes.
    auto same_list = [](const std::vector<hydra::pathbuilder::EndChoice>& a,
                        const std::vector<hydra::pathbuilder::EndChoice>& b) {
        REQUIRE(a.size() == b.size());
        for (size_t e = 0; e < a.size(); ++e) {
            CAPTURE(e);
            CHECK(a[e].activation.notationstr() == b[e].activation.notationstr());
            CHECK(a[e].window.deact_tick == b[e].window.deact_tick);
            CHECK(a[e].window.squeezed_out == b[e].window.squeezed_out);
            CHECK(a[e].best_finish == b[e].best_finish);
        }
    };
    auto kept_count = [&](size_t row, const std::vector<hydra::pathbuilder::EndChoice>& ends) {
        size_t kept = 0;
        for (const auto& e : ends) kept += hydra::pathbuilder::is_kept_end(all[row], e) ? 1 : 0;
        return kept;
    };

    // The third window, opened as the last of three and as the third of the
    // whole path.
    const auto third_last = ends_when_opened(2, 3);
    const auto third_earlier = ends_when_opened(2, all.size());
    REQUIRE_FALSE(third_last.empty());
    same_list(third_earlier, third_last);
    CHECK(kept_count(2, third_earlier) == 1);

    // The fourth window, opened as the last of four and as the fourth of the
    // whole path.
    const auto fourth_last = ends_when_opened(3, 4);
    const auto fourth_earlier = ends_when_opened(3, all.size());
    REQUIRE_FALSE(fourth_last.empty());
    same_list(fourth_earlier, fourth_last);
    CHECK(kept_count(3, fourth_earlier) == 1);
}

// Remove takes the window and every later one (remove_window).
TEST_CASE("coach remove: removing a window drops it and every later one") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> best = hydra::pinned_windows(c.record.best_path());
    REQUIRE(best.size() >= 2);

    // The second window: the first keeps its end, and the path activates on
    // it alone.
    const ClickResult r =
        hydra::pathbuilder::remove_window(*c.graph, c.settings, best, best[1].act_tick);
    REQUIRE(r.windows.has_value());
    REQUIRE(r.path.has_value());
    CHECK(r.message.empty());
    REQUIRE(r.windows->size() == 1);
    CHECK((*r.windows)[0].act_tick == best[0].act_tick);
    CHECK((*r.windows)[0].deact_tick == best[0].deact_tick);
    check_path_on_windows(*r.path, *r.windows);

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
    REQUIRE(windows.size() >= 2);
    // Last window first, then each one left; the windows before a removed
    // one keep their ends.
    while (!windows.empty()) {
        const ClickResult r = hydra::pathbuilder::remove_window(*c.graph, c.settings, windows,
                                                                windows.back().act_tick);
        REQUIRE(r.windows.has_value());
        CHECK(r.message.empty());
        REQUIRE(r.windows->size() + 1 == windows.size());
        for (size_t i = 0; i < r.windows->size(); ++i) {
            CAPTURE(i);
            CHECK((*r.windows)[i].act_tick == windows[i].act_tick);
            CHECK((*r.windows)[i].deact_tick == windows[i].deact_tick);
        }
        windows = *r.windows;
        // A window left has a path; none left has none (PlayerPath's rule).
        CHECK(r.path.has_value() == !windows.empty());
        if (r.path) check_path_on_windows(*r.path, windows);
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

    // Whether the first ladder is still running when the second request
    // comes is a race, so only what holds either way is checked: the second
    // request is the one the coach keeps, and its answer is the one shown.
    hydra::pathbuilder::Coach coach;
    coach.request(c.graph, c.settings, best, "first");
    coach.request(c.graph, c.settings, {}, "second");
    CHECK(coach.key() == "second");
    testwait::poll_until_idle(coach, "the coach's ladders to stop");
    const std::optional<LadderRun> run = coach.run();
    REQUIRE(run.has_value());
    CHECK(run->complete);
    CHECK(run->ladder.bf.size() == 1);  // the second request's: BF(0) alone, no window
    CHECK(coach.error().empty());
    // The same key again starts nothing.
    const size_t superseded = coach.superseded();
    coach.request(c.graph, c.settings, best, "second");
    CHECK(coach.superseded() == superseded);
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
    const size_t n = windows.size();
    const LadderRun run = ladder(c, windows);
    REQUIRE(run.complete);
    REQUIRE(run.ladder.bf.size() > n);
    // BF(n)'s next activation after the player's windows.
    const ActivationWalk bf_acts = run.best.at(n).walk_activations();
    REQUIRE(bf_acts.size() > n);
    const int64_t next = bf_acts[n].timecode.ticks();
    const std::vector<int64_t> ticks = candidate_ticks(run);
    REQUIRE_FALSE(ticks.empty());

    // The screen's way in: show the player's path, hand it the ticks.
    const hydra::TargetResult mine = hydra::search_target(*c.graph, windows);
    REQUIRE_FALSE(mine.paths.empty());
    hydra::pathbuilder::DecisionList& d = s.decisions();
    d.show(s.search_graph(), c.settings, &mine.paths.front(), "two windows");
    d.price_fills(ticks);
    testwait::poll_until_idle(d, "the candidate queue to finish", s);
    const std::vector<hydra::app::CandidateCost>& got = d.candidates();
    REQUIRE(got.size() == ticks.size());

    // Each fill's best finish is the search's own BF(n + f); its cost and
    // tier are candidate_costs's against BF(n) and BF(0).
    std::vector<hydra::app::CandidateFinish> finishes;
    for (const hydra::app::CandidateCost& g : got)
        finishes.push_back(hydra::app::CandidateFinish{g.act_tick, g.best_finish});
    const std::vector<hydra::app::CandidateCost> want =
        hydra::app::candidate_costs(run.ladder.bf.front(), run.ladder.bf[n], finishes);
    REQUIRE(want.size() == got.size());
    for (size_t i = 0; i < got.size(); ++i) {
        CAPTURE(i);
        CHECK(got[i].act_tick == ticks[i]);
        CHECK(got[i].best_finish == fresh_best_finish(c, windows, ticks[i]));
        CHECK(got[i].cost == want[i].cost);
        CHECK(got[i].tier == want[i].tier);
        // The one fill that loses nothing is BF(n)'s own next activation.
        CHECK(hydra::app::costs_nothing(got[i].cost) == (got[i].act_tick == next));
    }
}

TEST_CASE("candidate queue: it prices only the ticks it is given, in order") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> windows = two_windows(c);
    const LadderRun run = ladder(c, windows);
    REQUIRE(run.complete);
    const std::vector<int64_t> ticks = candidate_ticks(run);
    REQUIRE(ticks.size() >= 2);
    // Two of the fills, the first and the last in chart order, handed the
    // other way round.
    const int64_t earlier = ticks.front();
    const int64_t later = ticks.back();
    REQUIRE(earlier < later);
    hydra::pathbuilder::Coach coach;
    coach.request(c.graph, c.settings, windows, "two windows");
    REQUIRE(coach.request_candidates({later, earlier}));
    CHECK_FALSE(coach.request_candidates({later, earlier}));  // the same ticks start nothing
    settle_coach(coach);
    CHECK(coach.error().empty());
    const std::vector<hydra::app::CandidateFinish> got = coach.candidates();
    REQUIRE(got.size() == 2);
    CHECK(got[0].act_tick == earlier);
    CHECK(got[1].act_tick == later);
    for (const hydra::app::CandidateFinish& f : got) {
        CAPTURE(f.act_tick);
        CHECK(f.best_finish == fresh_best_finish(c, windows, f.act_tick));
    }
}

TEST_CASE("candidate queue: a newer request supersedes the older") {
    PbState s;
    open_and_wait(s, corpus::white_washed_notes(), scratch_settings());
    REQUIRE(s.chart() != nullptr);
    const OpenChart& c = *s.chart();
    const std::vector<PinnedWindow> windows = two_windows(c);
    const LadderRun run = ladder(c, windows);
    REQUIRE(run.complete);
    const std::vector<int64_t> all = candidate_ticks(run);
    REQUIRE(all.size() >= 2);
    const std::vector<int64_t> newer = {all.back()};
    hydra::pathbuilder::Coach coach;
    // The ladder first, so a queue asked for starts at once.
    coach.request(c.graph, c.settings, windows, "two windows");
    settle_coach(coach);

    // Whether the older queue is still running when the newer request comes
    // is a race, so only what holds either way is checked, not how many
    // queues were cancelled.

    // Newer ticks: only they are priced.
    coach.request_candidates(all);
    coach.request_candidates(newer);
    settle_coach(coach);
    const std::vector<hydra::app::CandidateFinish> got = coach.candidates();
    REQUIRE(got.size() == newer.size());
    CHECK(got[0].act_tick == newer.front());

    // A newer path: its ladder drops the queue, whose ticks were for the old
    // windows, and nothing is priced until the screen hands new ticks.
    coach.request_candidates(all);
    coach.request(c.graph, c.settings, {}, "no windows");
    settle_coach(coach);
    CHECK(coach.candidates().empty());
    CHECK(coach.error().empty());
}
