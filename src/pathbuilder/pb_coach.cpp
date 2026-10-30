#include "pathbuilder/pb_coach.h"

#include <exception>
#include <set>
#include <utility>

#include "app/user_messages.h"

namespace hydra::pathbuilder {

std::string tight_window_note(const Activation& act, const std::optional<double>& limit_ms) {
    if (!limit_ms || act.within_ms_limit(*limit_ms)) return {};
    const std::optional<double> needs = act.difficulty();
    // The negative-limit edge: tight, with no figure the limit fails on.
    if (!needs || timing_within_limit(*needs, *limit_ms)) return {};
    return "needs " + format_ms(*needs) + ", your limit is " + format_ms_whole(*limit_ms);
}

namespace {

std::string timing_text(const Activation& act) {
    const std::optional<double> needs = act.difficulty();
    return needs ? format_ms(*needs) : std::string();
}

}  // namespace

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
        row.timing = timing_text(a);
        row.note = tight_window_note(a, settings.ms_filter);
        if (costs && i < costs->windows.size()) {
            row.cost = costs->windows[i];
            row.tier = app::cost_tier(*row.cost, bf0);
        }
        out.rows.push_back(std::move(row));
    }
    if (run.complete && costs) {
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

std::vector<EndChoice> end_choices(const ScoreGraph& graph, const SearchSettings& settings,
                                   const std::vector<PinnedWindow>& before, int64_t act_tick) {
    std::vector<PinnedWindow> pins = before;
    pins.push_back(PinnedWindow{act_tick, std::nullopt, false, std::nullopt});
    const size_t k = before.size();
    const TargetResult found = search_target(graph, pins);

    std::vector<EndChoice> out;
    std::set<std::pair<std::optional<int64_t>, std::optional<int64_t>>> seen;
    auto consider = [&](const Path& p) {
        const ActivationWalk acts = p.walk_activations();
        if (acts.size() <= k) return;
        const std::vector<PinnedWindow> full = pinned_windows(p);
        const PinnedWindow& w = full.at(k);
        if (!seen.insert({w.deact_tick, w.squeezed_out}).second) return;
        std::vector<PinnedWindow> with_end = before;
        with_end.push_back(w);
        const TargetResult bf = best_finish(graph, settings, with_end);
        if (bf.paths.empty()) return;
        EndChoice c;
        c.window = w;
        c.token = acts[k].notationstr();
        c.timing = timing_text(acts[k]);
        c.best_finish = bf.paths.front().totalscore();
        out.push_back(std::move(c));
    };
    for (const Path& p : found.paths) {
        consider(p);
        for (const Path& v : p.variants) consider(v);
    }
    return out;
}

ClickResult start_window(const ScoreGraph& graph, const SearchSettings& settings,
                         const std::vector<PinnedWindow>& windows, int64_t act_tick) {
    ClickResult out;
    std::vector<PinnedWindow> pins;
    for (const PinnedWindow& w : windows)
        if (w.act_tick < act_tick) pins.push_back(w);
    pins.push_back(PinnedWindow{act_tick, std::nullopt, false, std::nullopt});
    for (const PinnedWindow& w : windows)
        if (w.act_tick >= act_tick)
            pins.push_back(PinnedWindow{w.act_tick, std::nullopt, false, std::nullopt});
    try {
        const TargetResult bf = best_finish(graph, settings, pins);
        if (bf.paths.empty()) {
            out.message = impossible_window_text(bf.failed_reason, bf.failed_tick == act_tick);
            return out;
        }
        std::vector<PinnedWindow> mine = pinned_windows(bf.paths.front());
        mine.resize(pins.size());
        TargetResult own = search_target(graph, mine);
        if (own.paths.empty()) {
            out.message = impossible_window_text(own.failed_reason, own.failed_tick == act_tick);
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
                        throw_if_cancelled();  // a newer request superseded this one
                    });
                const std::lock_guard<std::mutex> lock(mu_);
                run_ = std::move(done);
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
    key_ = key;
    job_ = std::make_unique<LadderJob>(std::move(graph), settings, std::move(windows));
}

void Coach::poll() {
    for (auto it = retired_.begin(); it != retired_.end();)
        it = (*it)->finished() ? retired_.erase(it) : it + 1;
}

bool Coach::busy() const { return (job_ && !job_->finished()) || !retired_.empty(); }

std::optional<app::LadderRun> Coach::run() const {
    if (!job_) return std::nullopt;
    return job_->snapshot();
}

std::string Coach::error() const {
    return job_ && job_->finished() && !job_->ok() ? job_->message() : std::string();
}

std::string Coach::error_detail() const {
    return job_ && job_->finished() && !job_->ok() ? job_->error() : std::string();
}

void Coach::clear() {
    if (job_) {
        job_->cancel();
        retired_.push_back(std::move(job_));
    }
    key_.clear();
}

}  // namespace hydra::pathbuilder
