#include "pathbuilder/pb_coach.h"

#include <exception>
#include <set>
#include <utility>

#include "imgui.h"

#include "app/report.h"  // ChipToken
#include "app/user_messages.h"
#include "pathbuilder/pb_state.h"
#include "ui/preview_controller.h"
#include "ui/theme.h"    // chip_color
#include "ui/widgets.h"  // WarnColor

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

namespace {

// The player's windows with `at` put in: the windows before it as they are,
// `at` itself, and the ones after it by their activation only (each takes
// the end the best finish chooses). `replace` drops a window on at's tick.
// Then best_finish, keeping the ends it chose, and search_target for the
// player's own path.
ClickResult repin(const ScoreGraph& graph, const SearchSettings& settings,
                  const std::vector<PinnedWindow>& windows, const PinnedWindow& at,
                  bool replace) {
    const int64_t act_tick = at.act_tick;
    ClickResult out;
    std::vector<PinnedWindow> pins;
    for (const PinnedWindow& w : windows)
        if (w.act_tick < act_tick) pins.push_back(w);
    pins.push_back(at);
    for (const PinnedWindow& w : windows)
        if (w.act_tick > act_tick || (w.act_tick == act_tick && !replace))
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

}  // namespace

ClickResult start_window(const ScoreGraph& graph, const SearchSettings& settings,
                         const std::vector<PinnedWindow>& windows, int64_t act_tick) {
    return repin(graph, settings, windows,
                 PinnedWindow{act_tick, std::nullopt, false, std::nullopt}, false);
}

ClickResult choose_end(const ScoreGraph& graph, const SearchSettings& settings,
                       const std::vector<PinnedWindow>& windows, const PinnedWindow& chosen) {
    return repin(graph, settings, windows, chosen, true);
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

size_t Coach::landed() const {
    if (!job_) return 0;
    return job_->landed() + (job_->finished() ? 1 : 0);
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

// ---- the right column -------------------------------------------------------

namespace {

// The report chips the cost tiers wear (spec decision 3, plan Q1).
ImVec4 tier_color(app::CostTier tier) {
    using app::report::ChipToken;
    switch (tier) {
        case app::CostTier::Green: return ui::chip_color(ChipToken::t0);
        case app::CostTier::Yellow: return ui::chip_color(ChipToken::t1);
        case app::CostTier::Red: return ui::chip_color(ChipToken::t3);
    }
    return ui::chip_color(ChipToken::muted);
}

// A cost as the row writes it: "0", or the points lost, "-13,640".
std::string cost_text(int64_t cost) {
    return cost == 0 ? std::string("0") : "-" + group_thousands(cost);
}

template <class Job>
void drop_finished(std::vector<std::unique_ptr<Job>>& jobs) {
    for (auto it = jobs.begin(); it != jobs.end();)
        it = (*it)->finished() ? jobs.erase(it) : it + 1;
}

}  // namespace

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
    shown_ = shown ? std::optional<Path>(*shown) : std::nullopt;
    shown_windows_.clear();
    try {
        if (shown_) shown_windows_ = pinned_windows(*shown_);
    } catch (const std::exception& e) {
        error_ = app::plain_error(e);
        coach_.clear();
        list_ = CoachList{};
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
    const std::optional<app::LadderRun> run = coach_.run();
    const Path none;
    list_ = coach_list(shown_ ? *shown_ : none, run ? *run : app::LadderRun{}, settings_);
    finish_ = run ? finish_for_me(*run, shown_windows_.size()) : std::nullopt;
}

void DecisionList::poll(PbState& state) {
    coach_.poll();
    drop_finished(retired_ends_);
    drop_finished(retired_clicks_);
    refresh_list();
    if (ends_job_ && ends_job_->finished()) {
        if (ends_job_->ok()) ends_ = std::move(ends_job_->result());
        ends_job_.reset();
    }
    if (click_job_ && click_job_->finished()) {
        if (!click_job_->ok()) {
            message_ = click_job_->message();
            detail_ = click_job_->error();
        } else {
            ClickResult& r = click_job_->result();
            message_ = std::move(r.message);
            detail_ = std::move(r.detail);
            if (r.windows && r.path) {
                PlayerPath next;
                next.windows = std::move(*r.windows);
                next.path = std::move(*r.path);
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
    finish_.reset();
    list_landed_ = static_cast<size_t>(-1);
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

void DecisionList::start_edit(std::function<ClickResult()> fn) {
    if (click_job_) retired_clicks_.push_back(std::move(click_job_));
    message_.clear();
    detail_.clear();
    click_job_ = std::make_unique<WorkJob<ClickResult>>(std::move(fn));
}

void DecisionList::click_fill(std::shared_ptr<const ScoreGraph> graph,
                              const SearchSettings& settings,
                              const std::vector<PinnedWindow>& windows, int64_t act_tick) {
    if (!graph) return;
    start_edit([graph, settings, windows, act_tick] {
        return start_window(*graph, settings, windows, act_tick);
    });
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

void DecisionList::draw(PbState& state, ui::PreviewController* preview) {
    ImGui::TextUnformatted(kDecisionsHeading);
    ImGui::Separator();
    if (!error_.empty()) {
        ui::WarnColor warn;
        ImGui::TextWrapped("%s", error_.c_str());
    }
    if (!message_.empty()) {
        {
            ui::WarnColor warn;
            ImGui::TextWrapped("%s", message_.c_str());
        }
        if (!detail_.empty()) ImGui::TextDisabled("%s", detail_.c_str());
    }
    const std::string ladder_error = coach_.error();
    if (!ladder_error.empty()) {
        {
            ui::WarnColor warn;
            ImGui::TextWrapped("%s", ladder_error.c_str());
        }
        const std::string d = coach_.error_detail();
        if (!d.empty()) ImGui::TextDisabled("%s", d.c_str());
    }

    if (list_.rows.empty()) ImGui::TextDisabled("No windows yet.");
    for (size_t i = 0; i < list_.rows.size(); ++i) {
        const CoachRow& r = list_.rows[i];
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::Selectable(r.token.c_str(), selected_ == i)) {
            select(i);
            if (preview) preview->seek_activation(i);
        }
        ImGui::Indent();
        if (r.cost && r.tier)
            ImGui::TextColored(tier_color(*r.tier), "%s", cost_text(*r.cost).c_str());
        else
            ImGui::TextDisabled("...");
        if (!r.timing.empty()) {
            ImGui::SameLine();
            ImGui::TextDisabled("%s", r.timing.c_str());
        }
        if (!r.note.empty())
            ImGui::TextColored(ui::chip_color(app::report::ChipToken::t5), "%s", r.note.c_str());
        ImGui::Unindent();
        ImGui::PopID();
    }
    if (list_.after_last) {
        ImGui::TextUnformatted("After your last window");
        ImGui::SameLine();
        ImGui::TextColored(tier_color(*list_.after_last_tier), "%s",
                           cost_text(*list_.after_last).c_str());
    }
    if (!list_.failure.empty()) {
        ui::WarnColor warn;
        ImGui::TextWrapped("%s", list_.failure.c_str());
    }

    if (selected_) {
        ImGui::Spacing();
        ImGui::TextDisabled("Ends for this window");
        if (ends_job_) ImGui::TextDisabled("...");
        for (size_t e = 0; e < ends_.size(); ++e) {
            const EndChoice& c = ends_[e];
            std::string label = c.token;
            if (!c.timing.empty()) label += "  " + c.timing;
            label += "  best finish " + group_thousands(c.best_finish);
            ImGui::PushID(static_cast<int>(1000 + e));
            if (ImGui::Selectable(label.c_str()) && graph_) {
                std::shared_ptr<const ScoreGraph> graph = graph_;
                const SearchSettings settings = settings_;
                const std::vector<PinnedWindow> windows = shown_windows_;
                const PinnedWindow chosen = c.window;
                start_edit([graph, settings, windows, chosen] {
                    return choose_end(*graph, settings, windows, chosen);
                });
            }
            ImGui::PopID();
        }
    }

    ImGui::Spacing();
    ImGui::BeginDisabled(!finish_.has_value());
    if (ImGui::Button(kFinishForMeLabel) && finish_) {
        try {
            state.history().edit(player_copy_of(*finish_));
            player_changed_ = true;
        } catch (const std::exception& e) {
            message_ = app::plain_error(e);
            detail_ = app::plain_error_detail(e);
        }
    }
    ImGui::EndDisabled();
}

}  // namespace hydra::pathbuilder
