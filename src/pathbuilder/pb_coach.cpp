#include "pathbuilder/pb_coach.h"

#include <algorithm>
#include <exception>
#include <set>
#include <stdexcept>
#include <utility>

#include "app/user_messages.h"
#include "pathbuilder/pb_state.h"

namespace hydra::pathbuilder {

std::string tight_window_note(const Activation& act, const std::optional<double>& limit_ms) {
    if (!limit_ms || act.within_ms_limit(*limit_ms)) return {};
    const std::optional<double> needs = act.difficulty();
    // The negative-limit edge: over the limit with no figure the limit fails
    // on, so the note names the limit alone.
    if (!needs || timing_within_limit(*needs, *limit_ms))
        return "over your limit of " + format_ms_whole(*limit_ms);
    return "needs " + format_ms(*needs) + ", your limit is " + format_ms_whole(*limit_ms);
}

CoachList coach_list(const Path& shown, const app::LadderRun& run,
                     const SearchSettings& settings) {
    CoachList out;
    std::optional<app::CostList> costs;
    if (!run.ladder.bf.empty()) costs = app::cost_list(run.ladder);
    const int64_t bf0 = run.ladder.bf.empty() ? 0 : run.ladder.bf.front();

    const ActivationWalk acts = shown.walk_activations();
    for (size_t i = 0; i < acts.size(); ++i) {
        const Activation& a = acts[i];
        CoachRow row;
        row.act_tick = a.timecode.ticks();
        row.token = a.notationstr();
        row.note = tight_window_note(a, settings.ms_filter);
        if (costs && i < costs->windows.size()) {
            row.cost = costs->windows[i];
            row.tier = app::cost_tier(*row.cost, bf0);
        }
        out.rows.push_back(std::move(row));
    }
    // A path with no window shows only "No windows yet." (the user's ruling,
    // "Hide the line").
    if (run.complete && costs && !out.rows.empty()) {
        out.after_last = costs->after_last;
        out.after_last_tier = app::cost_tier(costs->after_last, bf0);
    }
    if (!run.failed_reason.empty()) out.failure = impossible_window_text(run.failed_reason, false);
    return out;
}

std::string impossible_window_text(const std::string& failed_reason, bool at_click) {
    if (!at_click)
        return "This window makes one of your later windows impossible, so the path stays as "
               "it was.";
    if (failed_reason == "activation")
        return "Star Power can't be activated at this fill on your path: the meter isn't "
               "ready, or the fill doesn't appear.";
    if (failed_reason == "window_end")
        return "Star Power can't end where this window was pinned to end.";
    if (failed_reason == "sqout")
        return "This window can't end with the squeeze-out it was pinned to.";
    return app::kSomethingWentWrong;
}

namespace {

// What tells two ends of one window apart: end_choices lists each once, and
// is_kept_end matches the player's window by it.
using EndKey = std::pair<std::optional<int64_t>, std::optional<int64_t>>;

EndKey end_key(const PinnedWindow& w) { return {w.deact_tick, w.squeezed_out}; }

}  // namespace

std::vector<EndChoice> end_choices(const ScoreGraph& graph, const SearchSettings& settings,
                                   const std::vector<PinnedWindow>& before, int64_t act_tick) {
    std::vector<PinnedWindow> pins = before;
    pins.push_back(PinnedWindow{act_tick, std::nullopt, false, std::nullopt});
    const size_t k = before.size();
    const TargetResult found = search_target(graph, pins);

    std::vector<EndChoice> out;
    std::set<EndKey> seen;
    auto consider = [&](const Path& p) {
        const ActivationWalk acts = p.walk_activations();
        if (acts.size() <= k) return;
        const std::vector<PinnedWindow> full = pinned_windows(p);
        const PinnedWindow& w = full.at(k);
        if (!seen.insert(end_key(w)).second) return;
        std::vector<PinnedWindow> with_end = before;
        with_end.push_back(w);
        const TargetResult bf = best_finish(graph, settings, with_end);
        if (bf.paths.empty()) return;
        EndChoice c;
        c.window = w;
        c.best_finish = bf.paths.front().totalscore();
        c.activation = acts[k];
        out.push_back(std::move(c));
    };
    for (const Path& p : found.paths) {
        consider(p);
        for (const Path& v : p.variants) consider(v);
    }
    return out;
}

bool is_kept_end(const PinnedWindow& window, const EndChoice& end) {
    return end_key(window) == end_key(end.window);
}

namespace {

// The windows of `windows` that activate before `tick`.
std::vector<PinnedWindow> windows_before(const std::vector<PinnedWindow>& windows, int64_t tick) {
    std::vector<PinnedWindow> before;
    for (const PinnedWindow& w : windows)
        if (w.act_tick < tick) before.push_back(w);
    return before;
}

// The player's windows changed at `tick`: the windows before it as they are,
// then `at`, then the ones after it by their activation alone. `replace`
// drops a window already on `tick`.
std::vector<PinnedWindow> pins_around(const std::vector<PinnedWindow>& windows, int64_t tick,
                                      const PinnedWindow& at, bool replace) {
    std::vector<PinnedWindow> pins = windows_before(windows, tick);
    pins.push_back(at);
    for (const PinnedWindow& w : windows)
        if (w.act_tick > tick || (w.act_tick == tick && !replace))
            pins.push_back(PinnedWindow{w.act_tick, std::nullopt, false, std::nullopt});
    return pins;
}

// best_finish over `pins`, keeping the ends it chose, then search_target over
// those windows for the player's own path. `clicked` is the window the click
// put in; a failure says, in impossible_window_text, whether it broke there.
// A removal (`clicked` empty) keeps only windows the player's path already
// realized, so a failure there is a bug and throws. No pin left: no window
// and no path.
ClickResult repin(const ScoreGraph& graph, const SearchSettings& settings,
                  const std::vector<PinnedWindow>& pins, std::optional<int64_t> clicked) {
    ClickResult out;
    try {
        if (pins.empty()) {
            out.windows.emplace();
            return out;
        }
        auto refusal = [&](const TargetResult& failed) {
            if (!clicked)
                throw std::logic_error("remove_window: the windows before the removed one no "
                                       "longer realize (" + failed.failed_reason + ")");
            return impossible_window_text(failed.failed_reason, failed.failed_tick == clicked);
        };
        TargetResult bf = best_finish(graph, settings, pins);
        if (bf.paths.empty()) {
            out.message = refusal(bf);
            return out;
        }
        std::vector<PinnedWindow> mine = pinned_windows(bf.paths.front());
        mine.resize(pins.size());
        TargetResult own = search_target(graph, mine);
        if (own.paths.empty()) {
            out.message = refusal(own);
            return out;
        }
        out.windows = std::move(mine);
        out.path = std::move(own.paths.front());
    } catch (const std::exception& e) {
        out.message = app::plain_error(e);
        out.detail = app::plain_error_detail(e);
    }
    return out;
}

}  // namespace

ClickResult start_window(const ScoreGraph& graph, const SearchSettings& settings,
                         const std::vector<PinnedWindow>& windows, int64_t act_tick) {
    return repin(graph, settings,
                 pins_around(windows, act_tick,
                             PinnedWindow{act_tick, std::nullopt, false, std::nullopt}, false),
                 act_tick);
}

ClickResult choose_end(const ScoreGraph& graph, const SearchSettings& settings,
                       const std::vector<PinnedWindow>& windows, const PinnedWindow& chosen) {
    return repin(graph, settings, pins_around(windows, chosen.act_tick, chosen, true),
                 chosen.act_tick);
}

ClickResult remove_window(const ScoreGraph& graph, const SearchSettings& settings,
                          const std::vector<PinnedWindow>& windows, int64_t act_tick) {
    bool found = false;
    for (const PinnedWindow& w : windows) found = found || w.act_tick == act_tick;
    if (!found) return {};
    return repin(graph, settings, windows_before(windows, act_tick), std::nullopt);
}

std::optional<Path> finish_for_me(const app::LadderRun& run, size_t window_count) {
    if (run.best.size() <= window_count) return std::nullopt;
    return run.best[window_count];
}

LadderJob::LadderJob(std::shared_ptr<const ScoreGraph> graph, SearchSettings settings,
                     std::vector<PinnedWindow> windows)
    : graph_(std::move(graph)), settings_(std::move(settings)), windows_(std::move(windows)) {
    try {
        spawn([this] {
            run_guarded([this] {
                throw_if_cancelled();
                app::LadderRun done = app::build_ladder(
                    *graph_, settings_, windows_, [this](const app::LadderRun& so_far) {
                        {
                            const std::lock_guard<std::mutex> lock(mu_);
                            run_ = so_far;
                        }
                        ++landed_;
                        throw_if_cancelled();  // a newer request superseded this one
                    });
                {
                    const std::lock_guard<std::mutex> lock(mu_);
                    run_ = std::move(done);
                }
                ++landed_;
                return true;
            });
        });
    } catch (const std::exception& e) {
        fail(e);  // the thread could not start
    }
}

LadderJob::~LadderJob() { shutdown(); }

app::LadderRun LadderJob::snapshot() const {
    const std::lock_guard<std::mutex> lock(mu_);
    return run_;
}

std::vector<app::CandidateFinish> candidate_finishes(const ScoreGraph& graph,
                                                     const SearchSettings& settings,
                                                     const std::vector<PinnedWindow>& windows,
                                                     std::vector<int64_t> ticks,
                                                     const CandidateProgress& progress) {
    std::sort(ticks.begin(), ticks.end());
    std::vector<app::CandidateFinish> out;
    out.reserve(ticks.size());
    for (const int64_t tick : ticks) {
        std::vector<PinnedWindow> pins = windows;
        pins.push_back(PinnedWindow{tick, std::nullopt, false, std::nullopt});
        TargetResult bf = best_finish(graph, settings, std::move(pins));
        if (bf.paths.empty())
            throw std::logic_error("candidate_finishes: no path activates at tick " +
                                   std::to_string(tick) + " (" + bf.failed_reason + ")");
        const Path& finish = bf.paths.front();
        out.push_back(app::CandidateFinish{tick, finish.totalscore()});
        // The candidate is the activation after the player's windows.
        const ActivationWalk acts = finish.walk_activations();
        const std::optional<Activation> act =
            windows.size() < acts.size() ? std::optional<Activation>(acts[windows.size()]) : std::nullopt;
        if (progress) progress(out.back(), act);
    }
    return out;
}

CandidateJob::CandidateJob(std::shared_ptr<const ScoreGraph> graph, SearchSettings settings,
                           std::vector<PinnedWindow> windows, std::vector<int64_t> ticks)
    : graph_(std::move(graph)),
      settings_(std::move(settings)),
      windows_(std::move(windows)),
      ticks_(std::move(ticks)) {
    try {
        spawn([this] {
            run_guarded([this] {
                throw_if_cancelled();
                candidate_finishes(*graph_, settings_, windows_, ticks_,
                                   [this](const app::CandidateFinish& landed,
                                          const std::optional<Activation>& act) {
                                       {
                                           const std::lock_guard<std::mutex> lock(mu_);
                                           done_.push_back(landed);
                                           acts_.push_back(act);
                                       }
                                       ++landed_;
                                       throw_if_cancelled();  // a newer request superseded this one
                                   });
                return true;
            });
        });
    } catch (const std::exception& e) {
        fail(e);  // the thread could not start
    }
}

CandidateJob::~CandidateJob() { shutdown(); }

std::vector<app::CandidateFinish> CandidateJob::snapshot() const {
    const std::lock_guard<std::mutex> lock(mu_);
    return done_;
}

std::vector<std::optional<Activation>> CandidateJob::activations() const {
    const std::lock_guard<std::mutex> lock(mu_);
    return acts_;
}

void Coach::request(std::shared_ptr<const ScoreGraph> graph, const SearchSettings& settings,
                    std::vector<PinnedWindow> windows, const std::string& key) {
    if (job_ && key == key_) return;
    if (job_) {
        if (!job_->finished()) {
            job_->cancel();
            ++superseded_;
        }
        retired_.push_back(std::move(job_));
    }
    retire_candidates();
    candidate_ticks_.reset();
    key_ = key;
    graph_ = graph;
    settings_ = settings;
    windows_ = windows;
    job_ = std::make_unique<LadderJob>(std::move(graph), settings, std::move(windows));
}

bool Coach::request_candidates(std::vector<int64_t> ticks) {
    if (candidate_ticks_ && *candidate_ticks_ == ticks) return false;
    retire_candidates();
    candidate_ticks_ = std::move(ticks);
    start_candidates();
    return true;
}

void Coach::retire_candidates() {
    candidates_started_ = false;
    if (!candidate_job_) return;
    if (!candidate_job_->finished()) {
        candidate_job_->cancel();
        ++candidates_superseded_;
    }
    retired_candidates_.push_back(std::move(candidate_job_));
}

void Coach::start_candidates() {
    if (!candidate_ticks_ || candidates_started_ || !job_ || !job_->finished()) return;
    candidates_started_ = true;
    // Only a complete ladder has BF(n) to price against.
    if (!job_->ok() || !job_->snapshot().complete) return;
    candidate_job_ = std::make_unique<CandidateJob>(graph_, settings_, windows_, *candidate_ticks_);
}

void Coach::poll() {
    ui::drop_finished_jobs(retired_);
    ui::drop_finished_jobs(retired_candidates_);
    start_candidates();
}

bool Coach::busy() const {
    // A queue still waiting on its ladder counts, so a reader that polls
    // until idle sees it start.
    const bool waiting = candidate_ticks_ && !candidates_started_ && job_;
    return (job_ && !job_->finished()) || !retired_.empty() || waiting ||
           (candidate_job_ && !candidate_job_->finished()) || !retired_candidates_.empty();
}

std::vector<app::CandidateFinish> Coach::candidates() const {
    if (!candidate_job_) return {};
    return candidate_job_->snapshot();
}

std::vector<std::optional<Activation>> Coach::candidate_activations() const {
    if (!candidate_job_) return {};
    return candidate_job_->activations();
}

size_t Coach::candidates_landed() const {
    if (!candidate_job_) return 0;
    return candidate_job_->landed() + (candidate_job_->finished() ? 1 : 0);
}

std::optional<app::LadderRun> Coach::run() const {
    if (!job_) return std::nullopt;
    return job_->snapshot();
}

size_t Coach::landed() const {
    if (!job_) return 0;
    return job_->landed() + (job_->finished() ? 1 : 0);
}

std::string Coach::error() const {
    if (job_ && job_->finished() && !job_->ok()) return job_->message();
    if (candidate_job_ && candidate_job_->finished() && !candidate_job_->ok())
        return candidate_job_->message();
    return {};
}

std::string Coach::error_detail() const {
    if (job_ && job_->finished() && !job_->ok()) return job_->error();
    if (candidate_job_ && candidate_job_->finished() && !candidate_job_->ok())
        return candidate_job_->error();
    return {};
}

void Coach::clear() {
    if (job_) {
        job_->cancel();
        retired_.push_back(std::move(job_));
    }
    if (candidate_job_) {
        candidate_job_->cancel();
        retired_candidates_.push_back(std::move(candidate_job_));
    }
    candidate_ticks_.reset();
    candidates_started_ = false;
    key_.clear();
    graph_.reset();
    windows_.clear();
}

// ---- the decision list ------------------------------------------------------

void DecisionList::show(std::shared_ptr<const ScoreGraph> graph, const SearchSettings& settings,
                        const Path* shown, const std::string& key) {
    if (key == key_ && graph_ == graph) return;
    key_ = key;
    graph_ = std::move(graph);
    settings_ = settings;
    selected_.reset();
    ends_.clear();
    if (ends_job_) retired_ends_.push_back(std::move(ends_job_));
    error_.clear();
    candidates_.clear();
    candidate_ends_.clear();
    candidates_landed_ = static_cast<size_t>(-1);
    shown_ = shown ? std::optional<Path>(*shown) : std::nullopt;
    shown_windows_.clear();
    try {
        if (shown_) shown_windows_ = pinned_windows(*shown_);
    } catch (const std::exception& e) {
        error_ = app::plain_error(e);
        coach_.clear();
        list_ = CoachList{};
        run_.reset();
        finish_.reset();
        return;
    }
    coach_.request(graph_, settings_, shown_windows_, key_);
    list_landed_ = static_cast<size_t>(-1);
    refresh_list();
}

void DecisionList::refresh_list() {
    const size_t landed = coach_.landed();
    if (landed == list_landed_) return;
    list_landed_ = landed;
    run_ = coach_.run();
    const Path none;
    list_ = coach_list(shown_ ? *shown_ : none, run_ ? *run_ : app::LadderRun{}, settings_);
    finish_ = run_ ? finish_for_me(*run_, shown_windows_.size()) : std::nullopt;
}

void DecisionList::price_fills(std::vector<int64_t> ticks) {
    // A shown path pinned_windows refused has no ladder to price against.
    if (!error_.empty() || !graph_) return;
    if (!coach_.request_candidates(std::move(ticks))) return;
    candidates_.clear();
    candidate_ends_.clear();
    candidates_landed_ = static_cast<size_t>(-1);
}

void DecisionList::refresh_candidates() {
    const size_t landed = coach_.candidates_landed();
    if (landed == candidates_landed_) return;
    candidates_landed_ = landed;
    candidates_.clear();
    candidate_ends_.clear();
    // The queue starts only after a complete ladder, so run_ (refreshed
    // first) holds BF(0) and BF(n) by the time a candidate lands.
    const std::vector<app::CandidateFinish> landed_finishes = coach_.candidates();
    if (landed_finishes.empty() || !run_ || !run_->complete) return;
    candidates_ = app::candidate_costs(run_->ladder.bf.front(), run_->ladder.bf.back(),
                                       landed_finishes);
    // Read after the finishes, so it holds at least as many.
    candidate_ends_ = coach_.candidate_activations();
    candidate_ends_.resize(candidates_.size());
}

const Activation* DecisionList::candidate_end(size_t i) const {
    if (i >= candidate_ends_.size() || !candidate_ends_[i]) return nullptr;
    return &*candidate_ends_[i];
}

std::string DecisionList::error() const {
    if (!error_.empty()) return error_;
    return coach_.error();
}

std::string DecisionList::error_detail() const {
    if (!error_.empty()) return {};
    return coach_.error_detail();
}

void DecisionList::poll(PbState& state) {
    coach_.poll();
    ui::drop_finished_jobs(retired_ends_);
    ui::drop_finished_jobs(retired_clicks_);
    refresh_list();
    refresh_candidates();
    if (ends_job_ && ends_job_->finished()) {
        if (ends_job_->ok()) ends_ = std::move(ends_job_->result());
        ends_job_.reset();
    }
    if (click_job_ && click_job_->finished()) {
        if (state.history().generation() != click_generation_) {
            // The player's path changed while the search ran (undo, redo,
            // Finish the rest for me, another edit), so its answer is for a path that
            // is gone. The player clicks again if they still want it.
        } else if (!click_job_->ok()) {
            message_ = click_job_->message();
            detail_ = click_job_->error();
            message_generation_ = click_generation_;
        } else {
            ClickResult& r = click_job_->result();
            message_ = std::move(r.message);
            detail_ = std::move(r.detail);
            message_generation_ = click_generation_;
            if (r.windows) {
                PlayerPath next;
                next.windows = std::move(*r.windows);
                next.path = std::move(r.path);
                state.history().edit(std::move(next));
                player_changed_ = true;
            }
        }
        click_job_.reset();
    }
}

bool DecisionList::busy() const {
    return coach_.busy() || ends_job_ || click_job_ || !retired_ends_.empty() ||
           !retired_clicks_.empty();
}

void DecisionList::clear() {
    coach_.clear();
    key_.clear();
    graph_.reset();
    shown_.reset();
    shown_windows_.clear();
    list_ = CoachList{};
    run_.reset();
    finish_.reset();
    list_landed_ = static_cast<size_t>(-1);
    candidates_.clear();
    candidate_ends_.clear();
    candidates_landed_ = static_cast<size_t>(-1);
    error_.clear();
    selected_.reset();
    ends_.clear();
    if (ends_job_) retired_ends_.push_back(std::move(ends_job_));
    if (click_job_) retired_clicks_.push_back(std::move(click_job_));
    message_.clear();
    detail_.clear();
}

bool DecisionList::take_player_changed() {
    const bool changed = player_changed_;
    player_changed_ = false;
    return changed;
}

void DecisionList::start_edit(std::function<ClickResult()> fn, uint64_t generation) {
    if (click_job_) retired_clicks_.push_back(std::move(click_job_));
    message_.clear();
    detail_.clear();
    click_generation_ = generation;
    click_job_ = std::make_unique<WorkJob<ClickResult>>(std::move(fn));
}

void DecisionList::click_fill(std::shared_ptr<const ScoreGraph> graph,
                              const SearchSettings& settings,
                              const std::vector<PinnedWindow>& windows, int64_t act_tick,
                              uint64_t generation) {
    if (!graph) return;
    start_edit(
        [graph, settings, windows, act_tick] {
            return start_window(*graph, settings, windows, act_tick);
        },
        generation);
}

void DecisionList::click_remove(std::shared_ptr<const ScoreGraph> graph,
                                const SearchSettings& settings,
                                const std::vector<PinnedWindow>& windows, int64_t act_tick,
                                uint64_t generation) {
    if (!graph) return;
    start_edit(
        [graph, settings, windows, act_tick] {
            return remove_window(*graph, settings, windows, act_tick);
        },
        generation);
}

void DecisionList::choose_end(const PinnedWindow& chosen, uint64_t generation) {
    if (!graph_) return;
    std::shared_ptr<const ScoreGraph> graph = graph_;
    const SearchSettings settings = settings_;
    const std::vector<PinnedWindow> windows = shown_windows_;
    start_edit(
        [graph, settings, windows, chosen] {
            return pathbuilder::choose_end(*graph, settings, windows, chosen);
        },
        generation);
}

void DecisionList::finish(PbState& state) {
    if (!finish_) return;
    try {
        state.history().edit(player_copy_of(*finish_));
        player_changed_ = true;
    } catch (const std::exception& e) {
        message_ = app::plain_error(e);
        detail_ = app::plain_error_detail(e);
        message_generation_ = state.history().generation();
    }
}

void DecisionList::select(size_t row) {
    selected_ = row;
    ends_.clear();
    if (ends_job_) retired_ends_.push_back(std::move(ends_job_));
    if (row >= shown_windows_.size() || !graph_) return;
    const std::vector<PinnedWindow> before(shown_windows_.begin(),
                                           shown_windows_.begin() + static_cast<std::ptrdiff_t>(row));
    const int64_t act_tick = shown_windows_[row].act_tick;
    std::shared_ptr<const ScoreGraph> graph = graph_;
    const SearchSettings settings = settings_;
    ends_job_ = std::make_unique<WorkJob<std::vector<EndChoice>>>([graph, settings, before, act_tick] {
        return end_choices(*graph, settings, before, act_tick);
    });
}

}  // namespace hydra::pathbuilder
