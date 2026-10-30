// The Path Builder's decision list: one row per window of the path on screen,
// with what the window cost (EN-3's ladder, app/path_costs.h), the timing it
// needs and the tight-window note; the end choices of a window; "Finish for
// me"; and the clicks that start and remove a window. Every number is read from its
// owner: costs from cost_list and cost_tier, timings from
// Activation::difficulty and format_ms, the note from
// Activation::within_ms_limit. Plan: docs/superpowers/plans/
// 2026-10-28-path-builder.md, task PB-2a.

#ifndef HYDRA_PATHBUILDER_PB_COACH_H
#define HYDRA_PATHBUILDER_PB_COACH_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "app/job_base.h"
#include "app/path_costs.h"
#include "core/model.h"
#include "search/graph.h"
#include "search/pather.h"

namespace hydra::pathbuilder {

// The tight-window note on a window ("Optimal, flag tight", plan Q1): set
// when a Path limit is on and `act` fails Activation::within_ms_limit. It
// names the timing the window needs (Activation::difficulty through
// format_ms) and the limit (format_ms_whole). Empty when no limit is set or
// the window is inside it. A window that fails a negative limit through a
// squeeze that needs no timing has no figure to name, so its note names the
// limit alone (the user's ruling, "Note, limit only").
std::string tight_window_note(const Activation& act, const std::optional<double>& limit_ms);

// One row of the decision list.
struct CoachRow {
    int64_t act_tick = 0;
    // The window's token in Path::pathstring (Activation::notationstr).
    std::string token;
    // What the window cost and its colour, once the ladder has reached it.
    std::optional<int64_t> cost;
    std::optional<app::CostTier> tier;
    // The timing the window needs, as the Paths tab's button writes it;
    // empty when it needs none.
    std::string timing;
    std::string note;  // tight_window_note
};

// The whole list for one path.
struct CoachList {
    std::vector<CoachRow> rows;
    // The "after your last window" line, once the ladder is complete; none
    // while the path has no window.
    std::optional<int64_t> after_last;
    std::optional<app::CostTier> after_last_tier;
    // Why the ladder stopped (impossible_window_text), empty when it did not.
    std::string failure;
};

// The rows for `shown`, the path on screen, from `run`, the ladder over its
// windows (pinned_windows(shown)) as far as it has got.
CoachList coach_list(const Path& shown, const app::LadderRun& run,
                     const SearchSettings& settings);

// What the builder says when a window cannot happen: a target search's
// failed_reason (TargetResult) in plain words. `at_click` says whether the
// window that broke is the one just clicked.
std::string impossible_window_text(const std::string& failed_reason, bool at_click);

// What the builder says when removing a window would make a later window
// impossible (remove_window): the path stays as it was.
std::string removal_impossible_text();

// One way a window can end.
struct EndChoice {
    PinnedWindow window;   // the window pinned in full with this end
    std::string token;     // its Activation::notationstr
    std::string timing;    // as CoachRow::timing
    int64_t best_finish = 0;  // BF with this end pinned
};

// The distinct ends of the window at `act_tick`, with the windows before it
// (`before`, pinned as they are) kept: search_target with the window pinned
// by its activation alone, so every squeeze variant comes back, then
// best_finish with each end pinned. In search_target's order.
std::vector<EndChoice> end_choices(const ScoreGraph& graph, const SearchSettings& settings,
                                   const std::vector<PinnedWindow>& before, int64_t act_tick);

// What a click on a fill makes of the player's path.
struct ClickResult {
    // The new windows, set when the click worked, and the engine's path for
    // them, empty when no window is left (PlayerPath's rule).
    std::optional<std::vector<PinnedWindow>> windows;
    std::optional<Path> path;
    // impossible_window_text (removal_impossible_text for a removal), or
    // plain_error's sentence and details line for a click the window checks
    // refuse; empty when the click worked.
    std::string message;
    std::string detail;
    // kLaterEndRepickedText when a later window's end had to change; empty
    // otherwise. Set only on a click that worked.
    std::string notice;
};

// What the builder says when a later window's chosen end is no longer
// possible after a click, so the best finish picked a new one.
inline constexpr const char* kLaterEndRepickedText =
    "The end you chose for a later window isn't possible any more, so Hydra picked a new one.";

// Starts a window at `act_tick`: pins only that activation, runs
// best_finish, and keeps the end it chose. The windows before the click keep
// their ends. Those after it keep theirs too while that is still possible;
// when the new window's Star Power rules one out, they keep only their
// activations, take the ends the best finish chose, and the result's notice
// says so. The player's own path is search_target over the new windows. An
// impossible click changes nothing and says why.
ClickResult start_window(const ScoreGraph& graph, const SearchSettings& settings,
                         const std::vector<PinnedWindow>& windows, int64_t act_tick);

// Gives the window on `chosen`'s activation the end `chosen` pins (an
// EndChoice's window), the way start_window treats the windows around it.
ClickResult choose_end(const ScoreGraph& graph, const SearchSettings& settings,
                       const std::vector<PinnedWindow>& windows, const PinnedWindow& chosen);

// Removes the window on `act_tick`, treating the windows around it the way
// start_window does. Removing the last window leaves no window and no path.
// A tick with no window changes nothing: the result is empty.
ClickResult remove_window(const ScoreGraph& graph, const SearchSettings& settings,
                          const std::vector<PinnedWindow>& windows, int64_t act_tick);

// "Finish for me": the path behind BF(n) for a ladder over `window_count`
// windows (LadderRun::best), or nothing until the ladder has BF(n). The
// caller makes it the player's path with player_copy_of.
std::optional<Path> finish_for_me(const app::LadderRun& run, size_t window_count);

// One search on a worker thread, for the end choices and a click: `fn`'s
// answer once finished() and ok().
template <class T>
class WorkJob : public ui::ResultJobBase {
public:
    explicit WorkJob(std::function<T()> fn) {
        try {
            spawn([this, fn = std::move(fn)] {
                run_guarded([&] {
                    result_ = fn();
                    return true;
                });
            });
        } catch (const std::exception& e) {
            fail(e);  // the thread could not start
        }
    }
    ~WorkJob() { shutdown(); }
    T& result() { return result_; }

private:
    T result_{};
};

// The ladder for one path's windows on a worker thread. A newer request
// supersedes it: the progress callback throws once the job is cancelled, as
// Hydra's AnalysisCancelled unwinds a cancelled analysis.
class LadderJob : public ui::ResultJobBase {
public:
    LadderJob(std::shared_ptr<const ScoreGraph> graph, SearchSettings settings,
              std::vector<PinnedWindow> windows);
    ~LadderJob();
    // The run as far as it has got.
    app::LadderRun snapshot() const;
    // How many times the run has grown, so a reader rebuilds only then.
    size_t landed() const { return landed_.load(); }

private:
    std::shared_ptr<const ScoreGraph> graph_;
    SearchSettings settings_;
    std::vector<PinnedWindow> windows_;
    mutable std::mutex mu_;
    app::LadderRun run_;
    std::atomic<size_t> landed_{0};
};

// The decision list's worker: one ladder per path shown, the newest request
// only.
class Coach {
public:
    // Starts the ladder for `windows` unless the one running or done is for
    // the same `key`. A run still going is cancelled.
    void request(std::shared_ptr<const ScoreGraph> graph, const SearchSettings& settings,
                 std::vector<PinnedWindow> windows, const std::string& key);
    // Drops superseded runs that have stopped. Call once per frame.
    void poll();
    bool busy() const;
    // The newest run as far as it has got; empty before any request.
    std::optional<app::LadderRun> run() const;
    // LadderJob::landed of the newest run, plus one once it has finished.
    size_t landed() const;
    // The plain sentence and details line of a ladder that threw.
    std::string error() const;
    std::string error_detail() const;
    const std::string& key() const { return key_; }
    // How many requests a newer one superseded.
    size_t superseded() const { return superseded_; }
    void clear();

private:
    std::string key_;
    std::unique_ptr<LadderJob> job_;
    std::vector<std::unique_ptr<LadderJob>> retired_;
    size_t superseded_ = 0;
};

class PbState;

}  // namespace hydra::pathbuilder

namespace hydra::ui {
class PreviewController;
}

namespace hydra::pathbuilder {

// What the "Finish for me" button says.
inline constexpr const char* kFinishForMeLabel = "Finish for me";
// The heading over the decision list.
inline constexpr const char* kDecisionsHeading = "Your decisions";

// The right column: the decision list for the path on screen, its end
// choices, "Finish for me", and the clicks that change the player's path.
// Its searches run on worker threads (Coach, WorkJob); poll takes their
// answers in.
class DecisionList {
public:
    // The path on screen and a key that names it: a left-list row, or the
    // player's own path at a PathHistory::generation. A new key starts its
    // ladder and clears the selected row. `graph` is PbState::search_graph
    // and `settings` the open chart's.
    void show(std::shared_ptr<const ScoreGraph> graph, const SearchSettings& settings,
              const Path* shown, const std::string& key);
    // Takes in a finished click or end choice (an edit of the player's path
    // in `state`'s history) and the end choices of the selected row.
    void poll(PbState& state);
    bool busy() const;
    // Forgets everything, for a newly opened chart.
    void clear();

    // Starts a window at `act_tick` on the player's path (start_window), on
    // a worker. An impossible click leaves the path and sets message().
    // `generation` is PathHistory::generation() at the click: the answer is
    // thrown away if the player's path changed since (undo, redo, Finish for
    // me, another edit), and the player clicks again if they still want it.
    // The same holds for a removal and for an end choice.
    void click_fill(std::shared_ptr<const ScoreGraph> graph, const SearchSettings& settings,
                    const std::vector<PinnedWindow>& windows, int64_t act_tick,
                    uint64_t generation);
    // Removes the window on `act_tick` from the player's path
    // (remove_window), on a worker, as click_fill starts one.
    void click_remove(std::shared_ptr<const ScoreGraph> graph, const SearchSettings& settings,
                      const std::vector<PinnedWindow>& windows, int64_t act_tick,
                      uint64_t generation);
    // The click's or end choice's failure: plain sentence and details line.
    const std::string& message() const { return message_; }
    const std::string& detail() const { return detail_; }
    // The last click's or end choice's ClickResult::notice.
    const std::string& notice() const { return notice_; }
    // True once after poll made an edit of the player's path, so the
    // window shows the player's path again.
    bool take_player_changed();

    // The rows for the path on screen as the ladder stands.
    const CoachList& list() const { return list_; }
    std::optional<size_t> selected() const { return selected_; }
    const std::vector<EndChoice>& ends() const { return ends_; }
    // The ladder for the path on screen as far as it has got, as the rows
    // read it; null before its first request. The strip's step line reads it.
    const app::LadderRun* run() const { return run_ ? &*run_ : nullptr; }

    // Draws the column. `preview` (may be null) is sought to a row's
    // activation when the row is clicked.
    void draw(PbState& state, ui::PreviewController* preview);

private:
    Coach coach_;
    std::string key_;
    std::optional<Path> shown_;
    std::vector<PinnedWindow> shown_windows_;
    std::shared_ptr<const ScoreGraph> graph_;
    SearchSettings settings_;
    CoachList list_;
    // The run list_ was built from (refresh_list).
    std::optional<app::LadderRun> run_;
    // "Finish for me"'s path (finish_for_me), once the ladder has it.
    std::optional<Path> finish_;
    size_t list_landed_ = static_cast<size_t>(-1);
    std::string error_;  // pinned_windows refused the shown path

    std::optional<size_t> selected_;
    std::vector<EndChoice> ends_;
    std::unique_ptr<WorkJob<std::vector<EndChoice>>> ends_job_;
    std::unique_ptr<WorkJob<ClickResult>> click_job_;
    // PathHistory::generation() when click_job_ started.
    uint64_t click_generation_ = 0;
    // Replaced jobs still running: dropped once they finish, so the window
    // never waits on one.
    std::vector<std::unique_ptr<WorkJob<std::vector<EndChoice>>>> retired_ends_;
    std::vector<std::unique_ptr<WorkJob<ClickResult>>> retired_clicks_;
    std::string message_;
    std::string detail_;
    std::string notice_;
    bool player_changed_ = false;

    void select(size_t row);
    void start_edit(std::function<ClickResult()> fn, uint64_t generation);
    void refresh_list();
};

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_COACH_H
