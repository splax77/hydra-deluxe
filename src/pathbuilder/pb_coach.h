// The Path Builder's decision list: one row per window of the path on screen,
// with what the window cost (EN-3's ladder, app/path_costs.h) and the
// tight-window note; the end choices of a window and which one it kept;
// "Finish the rest for me"; and the clicks that start and remove a window.
// Every number is read from its owner: costs from cost_list and cost_tier,
// the note from Activation::within_ms_limit. Plan: docs/superpowers/plans/
// 2026-10-28-path-builder.md, task PB-2a. The candidate queue, which prices
// each fill the player could activate on next (app::candidate_costs), is task
// CO-2 of docs/superpowers/plans/2026-10-30-path-builder-redesign.md; that
// plan's Q1 (the player builds left to right) is why a click changes no
// window after it.

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

// One way a window can end.
struct EndChoice {
    PinnedWindow window;      // the window pinned in full with this end
    int64_t best_finish = 0;  // BF with this end pinned
    // The window's activation in the path search_target found with this end,
    // so the screen reads the end's measure and squeezes from it.
    Activation activation;
};

// Whether `end` is the end the player's window `window` kept: the same
// pair end_choices tells ends apart by.
bool is_kept_end(const PinnedWindow& window, const EndChoice& end);

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
    // impossible_window_text, or plain_error's sentence and details line for
    // a click the window checks refuse; empty when the click worked.
    std::string message;
    std::string detail;
};

// Starts a window at `act_tick`: pins only that activation, runs
// best_finish, and keeps the end it chose. The windows before the click keep
// their ends. The player builds left to right, so the screen sends no click
// before a window; a window after the click keeps only its activation and
// takes the end the best finish chose. The player's own path is
// search_target over the new windows. An impossible click changes nothing
// and says why.
ClickResult start_window(const ScoreGraph& graph, const SearchSettings& settings,
                         const std::vector<PinnedWindow>& windows, int64_t act_tick);

// Gives the window on `chosen`'s activation the end `chosen` pins (an
// EndChoice's window), the way start_window treats the windows around it.
ClickResult choose_end(const ScoreGraph& graph, const SearchSettings& settings,
                       const std::vector<PinnedWindow>& windows, const PinnedWindow& chosen);

// Removes the window on `act_tick` and every window after it; the windows
// before it keep their ends. Removing the first window leaves no window and
// no path. A tick with no window changes nothing: the result is empty.
ClickResult remove_window(const ScoreGraph& graph, const SearchSettings& settings,
                          const std::vector<PinnedWindow>& windows, int64_t act_tick);

// "Finish the rest for me": the path behind BF(n) for a ladder over `window_count`
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

// Called with each candidate's finish as it lands, and the candidate's
// activation in that finish's path (none when the path has no activation
// after the player's windows).
using CandidateProgress =
    std::function<void(const app::CandidateFinish&, const std::optional<Activation>&)>;

// The candidate queue's searches: for each tick of `ticks`, in chart order,
// best_finish with `windows` pinned as they are and a window on the tick
// pinned by its activation alone; its best total is the candidate's
// BF(n + f). One search per tick, one at a time; `progress` hears each as it
// lands. A tick no path activates on throws std::logic_error, because the
// screen hands only the fills the engine lists as open after the last window
// (Path::trailing_skipped_fill_ticks). The window checks are best_finish's.
std::vector<app::CandidateFinish> candidate_finishes(const ScoreGraph& graph,
                                                     const SearchSettings& settings,
                                                     const std::vector<PinnedWindow>& windows,
                                                     std::vector<int64_t> ticks,
                                                     const CandidateProgress& progress = {});

// The candidate queue on a worker thread, superseded the way LadderJob is.
class CandidateJob : public ui::ResultJobBase {
public:
    CandidateJob(std::shared_ptr<const ScoreGraph> graph, SearchSettings settings,
                 std::vector<PinnedWindow> windows, std::vector<int64_t> ticks);
    ~CandidateJob();
    // The finishes landed so far, in chart order.
    std::vector<app::CandidateFinish> snapshot() const;
    // Each landed finish's candidate activation (CandidateProgress), in the
    // same order.
    std::vector<std::optional<Activation>> activations() const;
    // How many finishes have landed.
    size_t landed() const { return landed_.load(); }

private:
    std::shared_ptr<const ScoreGraph> graph_;
    SearchSettings settings_;
    std::vector<PinnedWindow> windows_;
    std::vector<int64_t> ticks_;
    mutable std::mutex mu_;
    std::vector<app::CandidateFinish> done_;
    std::vector<std::optional<Activation>> acts_;
    std::atomic<size_t> landed_{0};
};

// The decision list's worker: one ladder per path shown, the newest request
// only, then the candidate queue for that path once its ladder is complete.
class Coach {
public:
    // Starts the ladder for `windows` unless the one running or done is for
    // the same `key`. A run still going is cancelled, and so is the candidate
    // queue, whose ticks were for the old windows.
    void request(std::shared_ptr<const ScoreGraph> graph, const SearchSettings& settings,
                 std::vector<PinnedWindow> windows, const std::string& key);
    // Asks for the candidate queue over `ticks` on the current request's
    // windows; the caller hands the ticks of that same path. It starts once
    // the ladder is complete (none starts after a ladder that failed). The
    // same ticks again start nothing; other ticks cancel the queue running
    // and start afresh. Returns whether this was a new request.
    bool request_candidates(std::vector<int64_t> ticks);
    // Drops superseded runs that have stopped and starts a waiting candidate
    // queue. Call once per frame.
    void poll();
    bool busy() const;
    // The newest run as far as it has got; empty before any request.
    std::optional<app::LadderRun> run() const;
    // LadderJob::landed of the newest run, plus one once it has finished.
    size_t landed() const;
    // The newest candidate queue's finishes as far as it has got.
    std::vector<app::CandidateFinish> candidates() const;
    // CandidateJob::activations of the newest queue.
    std::vector<std::optional<Activation>> candidate_activations() const;
    // CandidateJob::landed of the newest queue, plus one once it has finished.
    size_t candidates_landed() const;
    // The plain sentence and details line of a ladder that threw, or else of
    // a candidate queue that threw.
    std::string error() const;
    std::string error_detail() const;
    const std::string& key() const { return key_; }
    // How many ladder requests a newer one superseded.
    size_t superseded() const { return superseded_; }
    // How many candidate queues a newer request cancelled while they ran.
    size_t candidates_superseded() const { return candidates_superseded_; }
    void clear();

private:
    std::string key_;
    std::shared_ptr<const ScoreGraph> graph_;
    SearchSettings settings_;
    std::vector<PinnedWindow> windows_;
    std::unique_ptr<LadderJob> job_;
    std::vector<std::unique_ptr<LadderJob>> retired_;
    size_t superseded_ = 0;

    // The ticks asked for, and whether their queue has started (or was
    // skipped after a ladder that failed).
    std::optional<std::vector<int64_t>> candidate_ticks_;
    bool candidates_started_ = false;
    std::unique_ptr<CandidateJob> candidate_job_;
    std::vector<std::unique_ptr<CandidateJob>> retired_candidates_;
    size_t candidates_superseded_ = 0;

    void retire_candidates();
    void start_candidates();
};

class PbState;

// What the "Finish the rest for me" button says.
inline constexpr const char* kFinishForMeLabel = "Finish the rest for me";

// What the screen reads for the path on screen: its ladder and rows, its end
// choices, "Finish the rest for me", the candidate costs, and the clicks
// that change the player's path. pb_window.cpp draws all of it.
// Its searches run on worker threads (Coach, WorkJob); poll takes their
// answers in.
class DecisionList {
public:
    // The path on screen and a key that names it: the player's own path at
    // a PathHistory::generation. A new key starts its ladder and clears the
    // selected row. `graph` is PbState::search_graph and `settings` the open
    // chart's.
    void show(std::shared_ptr<const ScoreGraph> graph, const SearchSettings& settings,
              const Path* shown, const std::string& key);
    // Takes in a finished click or end choice (an edit of the player's path
    // in `state`'s history) and the end choices of the selected row.
    void poll(PbState& state);
    bool busy() const;
    // Forgets everything, for a newly opened chart.
    void clear();

    // Starts a window at `act_tick` on `windows` (start_window), on a
    // worker; the answer becomes the player's path as one edit. `windows`
    // are the player's own. An impossible click leaves the path and sets
    // message().
    // `generation` is PathHistory::generation() at the click: the answer is
    // thrown away if the player's path changed since (undo, redo, Finish the
    // rest for me, another edit), and the player clicks again if they still
    // want it.
    // The same holds for a removal and for an end choice.
    void click_fill(std::shared_ptr<const ScoreGraph> graph, const SearchSettings& settings,
                    const std::vector<PinnedWindow>& windows, int64_t act_tick,
                    uint64_t generation);
    // Removes the window on `act_tick` from `windows` (remove_window), on a
    // worker, as click_fill starts one.
    void click_remove(std::shared_ptr<const ScoreGraph> graph, const SearchSettings& settings,
                      const std::vector<PinnedWindow>& windows, int64_t act_tick,
                      uint64_t generation);
    // Gives the shown path's window on `chosen`'s activation the end `chosen`
    // pins (an EndChoice's window, from ends()), on a worker, as click_fill
    // starts one.
    void choose_end(const PinnedWindow& chosen, uint64_t generation);
    // Selects the shown path's window `row` and starts its end choices
    // (end_choices) on a worker; ends() holds them once they land.
    void select(size_t row);
    // Whether "Finish the rest for me" has a path yet (finish_for_me).
    bool can_finish() const { return finish_.has_value(); }
    // "Finish the rest for me": makes finish_for_me's path the player's path
    // (player_copy_of) as one edit. Does nothing until can_finish(); a path
    // player_copy_of refuses sets message().
    void finish(PbState& state);

    // Prices the fills the player could activate on next (Coach's candidate
    // queue) for the path on screen: `ticks` are the shown path's
    // PreviewScene::trailing_skipped_fill_ticks. Call it after show, each
    // frame; the same ticks again start nothing.
    void price_fills(std::vector<int64_t> ticks);
    // The candidates priced so far, in chart order, each with its cost
    // against BF(n) (app::candidate_costs); empty until the ladder is
    // complete.
    const std::vector<app::CandidateCost>& candidates() const { return candidates_; }
    // candidates()[i]'s activation in its best finish's path (the hover's
    // "Hydra would end it at" line), or null when that path has none.
    const Activation* candidate_end(size_t i) const;
    // The click's or end choice's failure: plain sentence and details line.
    const std::string& message() const { return message_; }
    const std::string& detail() const { return detail_; }
    // Why the path on screen has no ladder: pinned_windows refused it, or
    // the ladder or the candidate queue threw (Coach::error). Plain sentence
    // and details line; empty when nothing failed.
    std::string error() const;
    std::string error_detail() const;
    // True once after poll made an edit of the player's path, so the
    // window shows the player's path again.
    bool take_player_changed();

    // The rows for the path on screen as the ladder stands.
    const CoachList& list() const { return list_; }
    std::optional<size_t> selected() const { return selected_; }
    const std::vector<EndChoice>& ends() const { return ends_; }
    // The ladder for the path on screen as far as it has got, as the rows
    // read it; null before its first request. The three scores read it.
    const app::LadderRun* run() const { return run_ ? &*run_ : nullptr; }

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
    // "Finish the rest for me"'s path (finish_for_me), once the ladder has it.
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
    bool player_changed_ = false;

    std::vector<app::CandidateCost> candidates_;
    std::vector<std::optional<Activation>> candidate_ends_;  // parallel to candidates_
    size_t candidates_landed_ = static_cast<size_t>(-1);

    void start_edit(std::function<ClickResult()> fn, uint64_t generation);
    void refresh_list();
    void refresh_candidates();
};

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_COACH_H
