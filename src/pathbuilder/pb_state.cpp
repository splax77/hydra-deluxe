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
    TargetResult none = search_target(*c.graph, {});
    if (!none.paths.empty()) c.empty_path = std::move(none.paths.front());

    c.entry.notespath = notespath;
    c.entry.md5 = app::hash_chart_file(notespath);
    // One read of the chart's metadata for its names and its timing, as the
    // file states them; chart_title_line decides what the screen shows.
    app::ChartMeta meta = app::read_chart_meta(notespath);
    c.entry.title = std::move(meta.title);
    c.entry.artist = std::move(meta.artist);
    c.entry.timing = meta.timing;
    return chart;
}

std::string chart_title_line(const OpenChart& chart) {
    const std::string title = display_title(chart.entry.title);
    if (title == kUnknownTitle) return chart.chart_path;
    return title + " — " + display_artist(chart.entry.artist);
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
    ++generation_;
}

void PathHistory::edit(PlayerPath next) {
    states_.resize(current_ + 1);
    states_.push_back(std::move(next));
    current_ = states_.size() - 1;
    ++generation_;
}

bool PathHistory::undo() {
    if (!can_undo()) return false;
    --current_;
    ++generation_;
    return true;
}

bool PathHistory::redo() {
    if (!can_redo()) return false;
    ++current_;
    ++generation_;
    return true;
}

void PbState::open(const std::string& notespath) {
    open_with(notespath, app::Settings::load());
}

void PbState::open_with(const std::string& notespath, const app::Settings& settings) {
    requested_ = notespath;
    preview_volume_ = settings.preview_volume;
    if (job_) {
        job_->cancel();
        retired_.push_back(std::move(job_));
    }
    job_ = std::make_unique<OpenJob>(notespath, settings);
}

void PbState::poll() {
    ui::drop_finished_jobs(retired_);
    decisions_.poll(*this);
    if (!job_ || !job_->finished()) return;
    if (job_->ok()) {
        // The decision list's searches run on the old chart's graph, which
        // they share; clear them before that chart goes.
        decisions_.clear();
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
