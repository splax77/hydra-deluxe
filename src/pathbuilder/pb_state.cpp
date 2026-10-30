#include "pathbuilder/pb_state.h"

#include <stdexcept>
#include <utility>

#include "app/preview_source.h"
#include "core/winstr.h"  // read_file_bytes
#include "parse/song.h"

namespace hydra::pathbuilder {

std::unique_ptr<OpenChart> load_open_chart(const std::string& notespath,
                                           const app::Settings& settings,
                                           const std::atomic<bool>* cancel) {
    const app::AnalysisSettings run = settings.batch_run().settings;
    const app::SharedBytes container = app::read_preview_container(
        [](const std::string& p) { return read_file_bytes(p); }, notespath);
    auto chart = std::make_unique<OpenChart>(
        app::resolve_preview_song(notespath, container, run.reading(), run.rules,
                                  fill_rule_for(run.legacy_fills))
            .song);
    OpenChart& c = *chart;
    c.chart_path = notespath;
    c.settings = run;
    require_notes(c.song, c.settings.difficulty, c.settings.prodrums);
    c.graph = std::make_shared<const ScoreGraph>(search_graph(c.song, c.settings));
    c.record = analyze_chart(c.song, c.settings, app::stop_on_cancel(cancel));

    c.entry.notespath = notespath;
    c.entry.md5 = app::hash_chart_file(notespath);
    c.entry.timing = app::chart_timing_meta(std::nullopt, notespath);
    return chart;
}

OpenJob::OpenJob(std::string notespath, app::Settings settings)
    : notespath_(std::move(notespath)), settings_(std::move(settings)) {
    try {
        spawn([this] {
            run_guarded([this] {
                try {
                    chart_ = load_open_chart(notespath_, settings_, &cancel_);
                } catch (const app::AnalysisCancelled&) {
                    throw_if_cancelled();  // a newer open replaced this one
                    return false;
                }
                return true;
            });
        });
    } catch (const std::exception& e) {
        fail(e);  // the thread could not start
    }
}

OpenJob::~OpenJob() { shutdown(); }

std::unique_ptr<OpenChart> OpenJob::take() {
    if (!finished() || !ok()) return nullptr;
    return std::move(chart_);
}

PlayerPath player_copy_of(const Path& path) {
    PlayerPath p;
    p.windows = pinned_windows(path);
    p.path = path;
    return p;
}

void PathHistory::reset(PlayerPath start) {
    states_.assign(1, std::move(start));
    current_ = 0;
}

void PathHistory::edit(PlayerPath next) {
    states_.resize(current_ + 1);
    states_.push_back(std::move(next));
    current_ = states_.size() - 1;
}

bool PathHistory::undo() {
    if (!can_undo()) return false;
    --current_;
    return true;
}

bool PathHistory::redo() {
    if (!can_redo()) return false;
    ++current_;
    return true;
}

void PbState::open(const std::string& notespath) {
    open_with(notespath, app::Settings::load());
}

void PbState::open_with(const std::string& notespath, const app::Settings& settings) {
    requested_ = notespath;
    if (job_) {
        job_->cancel();
        retired_.push_back(std::move(job_));
    }
    job_ = std::make_unique<OpenJob>(notespath, settings);
}

void PbState::poll() {
    for (auto it = retired_.begin(); it != retired_.end();)
        it = (*it)->finished() ? retired_.erase(it) : it + 1;
    if (!job_ || !job_->finished()) return;
    if (job_->ok()) {
        chart_ = job_->take();
        error_.clear();
        error_detail_.clear();
        history_.reset(PlayerPath{});
    } else {
        // The chart that was open stays open; the error says why the new one
        // did not.
        error_ = job_->message();
        error_detail_ = job_->error();
    }
    job_.reset();
}

}  // namespace hydra::pathbuilder
