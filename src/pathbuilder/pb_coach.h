// The Path Builder's decision list: one row per window of the path on screen,
// with what the window cost (EN-3's ladder, app/path_costs.h), the timing it
// needs and the tight-window note; the end choices of a window; "Finish for
// me"; and a click that starts a new window. Every number is read from its
// owner: costs from cost_list and cost_tier, timings from
// Activation::difficulty and format_ms, the note from
// Activation::within_ms_limit. Plan: docs/superpowers/plans/
// 2026-10-28-path-builder.md, task PB-2a.

#ifndef HYDRA_PATHBUILDER_PB_COACH_H
#define HYDRA_PATHBUILDER_PB_COACH_H

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
// squeeze that needs no timing has no figure to name; its wording is the
// user's call (the plan's "Out of scope"), so it also reads empty until then.
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
    // The "after your last window" line, once the ladder is complete.
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
    // The new path, when the window can happen.
    std::optional<std::vector<PinnedWindow>> windows;
    std::optional<Path> path;
    // impossible_window_text, or plain_error's sentence and details line for
    // a click the window checks refuse; empty when the click worked.
    std::string message;
    std::string detail;
};

// Starts a window at `act_tick`: pins only that activation, runs
// best_finish, and keeps the end it chose. The windows before the click keep
// their ends; those after it keep their activations and take the ends the
// best finish chose, since the new window changes the Star Power they start
// with. The player's own path is search_target over the new windows. An
// impossible click changes nothing and says why.
ClickResult start_window(const ScoreGraph& graph, const SearchSettings& settings,
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

private:
    std::shared_ptr<const ScoreGraph> graph_;
    SearchSettings settings_;
    std::vector<PinnedWindow> windows_;
    mutable std::mutex mu_;
    app::LadderRun run_;
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

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_COACH_H
