#include "ui/library_jobs.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <stdexcept>

#include "app/config.h"
#include "app/report.h"
#include "app/report_files.h"
#include "app/user_messages.h"
#include "core/error_kind.h"
#include "parse/song.h"  // display_title, display_artist

namespace hydra::ui {

// ---- ScanJob --------------------------------------------------------------

ScanJob::ScanJob(std::vector<std::string> rootfolders, store::RecordStore& store)
    : rootfolders_(std::move(rootfolders)), store_(store) {}

void ScanJob::start() { spawn([this] { run(); }); }

ScanProgress ScanJob::snapshot() const {
    std::lock_guard<std::mutex> lock(mu_);
    return progress_;
}

void ScanJob::run() {
    // The previous scan's rows: any chart whose files are unchanged
    // (sig_unchanged) reuses its md5/metadata without being read again.
    store::ChartLibraryCache cache;
    try {
        cache = store_.chart_library_cache();
    } catch (const std::exception&) {
        // No cache is only a slow scan, not a failed one.
    }

    app::ScanCallbacks callbacks;
    callbacks.on_folders = [this](int n) {
        std::lock_guard<std::mutex> lock(mu_);
        progress_.folders_seen = n;
    };
    callbacks.on_charts = [this](int done, int total, int cached) {
        std::lock_guard<std::mutex> lock(mu_);
        progress_.phase = ScanProgress::Phase::Reading;
        progress_.charts_done = done;
        progress_.charts_total = total;
        progress_.charts_cached = cached;
    };
    callbacks.cancel = &cancel_;

    auto [items, errors] =
        app::discover_charts(rootfolders_, callbacks, cache.empty() ? nullptr : &cache);

    {
        std::lock_guard<std::mutex> lock(mu_);
        progress_.errors = errors;
        progress_.charts_found = static_cast<int>(items.size());
    }

    if (cancel_.load()) {
        // Leave the existing library untouched.
        std::lock_guard<std::mutex> lock(mu_);
        progress_.cancelled = true;
        progress_.phase = ScanProgress::Phase::Done;
        progress_.finished = true;
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mu_);
        progress_.phase = ScanProgress::Phase::Writing;
    }

    const std::optional<std::string> problem = app::save_scan_as_library(store_, items);

    std::lock_guard<std::mutex> lock(mu_);
    if (problem) progress_.errors.push_back(*problem);
    progress_.phase = ScanProgress::Phase::Done;
    progress_.finished = true;
}

// ---- BatchJob ---------------------------------------------------------

namespace {

double steady_seconds() {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace

void BatchClock::start(double now) {
    started_ = now;
    paused_at_.reset();
    finished_at_.reset();
    paused_total_ = 0.0;
}

void BatchClock::pause(double now) {
    if (started_ && !paused_at_ && !finished_at_) paused_at_ = now;
}

void BatchClock::resume(double now) {
    if (!paused_at_ || finished_at_) return;
    paused_total_ += now - *paused_at_;
    paused_at_.reset();
}

void BatchClock::finish(double now) {
    if (!started_ || finished_at_) return;
    finished_at_ = paused_at_ ? *paused_at_ : now;
    paused_at_.reset();
}

double BatchClock::elapsed_s(double now) const {
    if (!started_) return 0.0;
    const double end = finished_at_ ? *finished_at_ : paused_at_ ? *paused_at_ : now;
    return std::max(0.0, end - *started_ - paused_total_);
}

std::optional<double> batch_eta_s(double elapsed_s, int completed, int total) {
    if (total <= 0 || completed < kEtaMinFinished || completed > total) return std::nullopt;
    return elapsed_s / completed * (total - completed);
}

app::ScanItem scan_item_of(const store::ChartLibraryEntry& e) {
    app::ScanItem item;
    item.md5 = e.md5;
    item.title = e.title;
    item.artist = e.artist;
    item.charter = e.charter;
    item.notespath = e.notespath;
    item.rootfolder = e.rootfolder;
    item.timing = e.timing;
    return item;
}

BatchJob::BatchJob(app::BatchPlan plan, app::BatchRun run, store::RecordStore& store)
    : plan_(std::move(plan)),
      run_(std::move(run)),
      store_(store),
      workers_(app::batch_worker_count()),
      seed_(app::report::ReportSeed::for_run(run_)) {}

void BatchJob::set_analyzer_for_test(app::ChartAnalyzer analyze, int workers) {
    analyze_ = std::move(analyze);
    workers_ = workers;  // stored as given: run_work_pool checks it
}

namespace {
app::ChartAnalyzer g_app_batch_analyzer;
int g_app_batch_workers = 1;
}  // namespace

void set_app_batch_analyzer_for_test(app::ChartAnalyzer analyze, int workers) {
    g_app_batch_analyzer = std::move(analyze);
    g_app_batch_workers = workers;
}

void apply_app_batch_analyzer_for_test(BatchJob& job) {
    if (g_app_batch_analyzer) job.set_analyzer_for_test(g_app_batch_analyzer, g_app_batch_workers);
}

void BatchJob::start() {
    {
        std::lock_guard<std::mutex> lock(mu_);
        const double now = steady_seconds();
        clock_.start(now);
        if (snap_.paused) clock_.pause(now);  // paused before it started
    }
    spawn([this] { run(); });
}

void BatchJob::pause() {
    {
        std::lock_guard<std::mutex> lock(pause_mu_);
        if (paused_) return;
        paused_ = true;
    }
    std::lock_guard<std::mutex> lock(mu_);
    if (snap_.finished) return;
    snap_.paused = true;
    clock_.pause(steady_seconds());
}

void BatchJob::resume() {
    {
        std::lock_guard<std::mutex> lock(pause_mu_);
        if (!paused_) return;
        paused_ = false;
    }
    pause_cv_.notify_all();
    std::lock_guard<std::mutex> lock(mu_);
    snap_.paused = false;
    clock_.resume(steady_seconds());
}

void BatchJob::stop() {
    cancel_.store(true);
    {
        // Taking the lock puts this stop between a waiting worker's checks,
        // so no worker can sleep through it.
        std::lock_guard<std::mutex> lock(pause_mu_);
    }
    pause_cv_.notify_all();
}

void BatchJob::wait_while_paused() {
    std::unique_lock<std::mutex> lock(pause_mu_);
    pause_cv_.wait(lock, [this] { return !paused_ || cancel_.load(); });
    if (cancel_.load()) throw app::AnalysisCancelled{};
}

void BatchJob::note_started(const std::string& notespath) {
    auto it = by_path_.find(notespath);
    if (it == by_path_.end()) return;
    const app::ScanItem& item = plan_.todo[it->second];
    std::lock_guard<std::mutex> lock(mu_);
    snap_.current_title = display_title(item.title);
    snap_.current_artist = display_artist(item.artist);
}

void BatchJob::finish_failed(const std::exception& e, std::string detail) {
    std::lock_guard<std::mutex> lock(mu_);
    snap_.run_error = app::plain_error(e);
    snap_.run_error_detail = std::move(detail);
    finish_locked();
}

void BatchJob::finish_locked() {
    snap_.preparing = false;
    snap_.current_title.clear();
    snap_.current_artist.clear();
    snap_.paused = false;
    clock_.finish(steady_seconds());
    snap_.finished = true;
}

BatchJob::Snapshot BatchJob::snapshot() const {
    std::lock_guard<std::mutex> lock(mu_);
    Snapshot s = snap_;
    s.elapsed_s = clock_.elapsed_s(steady_seconds());
    if (!s.finished && !s.paused) s.eta_s = batch_eta_s(s.elapsed_s, s.completed, s.total);
    return s;
}

void BatchJob::run() {
    for (size_t i = 0; i < plan_.todo.size(); ++i) by_path_.emplace(plan_.todo[i].notespath, i);

    {
        std::lock_guard<std::mutex> lock(mu_);
        snap_.preparing = false;
    }
    if (cancel_.load()) {
        std::lock_guard<std::mutex> lock(mu_);
        finish_locked();
        return;
    }

    app::BatchCallbacks callbacks;
    // run_batch counts; the snapshot copies all five numbers in one step, so
    // a frame never reads a failure before the chart it belongs to.
    callbacks.on_progress = [this](const app::BatchProgress& p) {
        std::lock_guard<std::mutex> lock(mu_);
        snap_.total = p.total;
        snap_.completed = p.completed;
        snap_.analyzed = p.analyzed;
        snap_.skipped = p.skipped;
        snap_.failed = p.failed;
    };
    callbacks.on_error = [this](const std::string& title, const std::string& sentence,
                                const std::string& error) {
        std::lock_guard<std::mutex> lock(mu_);
        snap_.failures.push_back(title + ": " + sentence);
        snap_.failure_details.push_back(title + ": " + error);
    };
    callbacks.cancel = &cancel_;
    callbacks.report_seed = &seed_;
    // The pause gate and the "now analyzing" line sit in front of the real
    // analyzer, so run_batch and its pool stay as they are.
    const app::ChartAnalyzer inner =
        analyze_ ? analyze_ : app::ChartAnalyzer(app::analyze_chart_file);
    callbacks.analyze = [this, inner](const std::string& path,
                                      const app::AnalysisSettings& settings,
                                      const std::function<void(float)>& on_progress) {
        wait_while_paused();
        note_started(path);
        return inner(path, settings, on_progress);
    };
    // The plan was made before the job (D79), so run_batch reads nothing
    // from the store before its first chart. Whatever still throws out of it
    // ends the run as a whole.
    try {
        app::run_batch(plan_, run_, store_, workers_, callbacks);
    } catch (const std::exception& e) {
        finish_failed(e, app::plain_error_detail(e));
        return;
    }

    std::lock_guard<std::mutex> lock(mu_);
    finish_locked();
}

// ---- ViewJob ----------------------------------------------------------

namespace {
app::ChartAnalyzer g_view_analyzer;
}  // namespace

void set_view_analyzer_for_test(app::ChartAnalyzer analyze) {
    g_view_analyzer = std::move(analyze);
}

ViewJob::ViewJob(store::ChartLibraryEntry song, store::RecordKey key,
                 app::AnalysisSettings settings, bool analysis_off, int generation)
    : song_(std::move(song)),
      key_(std::move(key)),
      settings_(std::move(settings)),
      analysis_off_(analysis_off),
      generation_(generation),
      analyze_(g_view_analyzer ? g_view_analyzer : app::ChartAnalyzer(app::analyze_chart_file)) {}

void ViewJob::start() {
    // The thread constructor itself can throw (std::system_error when the OS
    // refuses the thread); route that through the panel's error path instead
    // of letting it escape and terminate the app.
    try {
        spawn([this] { run(); });
    } catch (const std::exception& e) {
        fail(e);
    }
}

void ViewJob::run() {
    run_guarded([this] {
        try {
            // An edited chart is hashed again before anything reads it, with
            // the rescan's own unchanged test (D87 item 3).
            // chart_changed_since is the one answer, shared with the preview
            // load. A file saved again with the same content keeps its hash
            // but still hands back its new fingerprint, so the row stops
            // asking for a hash on every click (D96).
            if (const std::optional<app::ChartNow> now =
                    app::chart_changed_since(song_.notespath, song_.sig)) {
                if (now->md5.empty())
                    throw std::runtime_error("could not read " + song_.notespath);
                out_.new_md5 = now->md5;
                out_.new_sig = now->sig;
                out_.files_changed = true;
            }
            const std::string& path = song_.notespath;
            throw_if_cancelled();
            if (!analysis_off_) {
                try {
                    out_.analysis = analyze_(path, settings_, [this](float f) {
                        if (cancel_.load(std::memory_order_relaxed))
                            throw app::AnalysisCancelled{};
                        progress_.store(f, std::memory_order_relaxed);
                    });
                    out_.length = app::analysis_song_length(song_.timing, path,
                                                            out_.analysis->song, settings_);
                } catch (const app::AnalysisCancelled&) {
                    throw;
                } catch (const std::exception& e) {
                    out_.analysis.reset();
                    out_.analysis_error = e.what();
                    out_.analysis_message = app::plain_error(e);
                }
            }
            throw_if_cancelled();
            // The Dynamics count reuses the analysis's song only when that
            // parse is the count's own; otherwise it parses as the count does.
            try {
                if (out_.analysis && app::analysis_parse_counts_dynamics(settings_.bass2x))
                    out_.dynamics = app::count_dynamics(out_.analysis->song);
                else
                    out_.dynamics = app::count_dynamics(
                        app::load_dynamics_song(path, settings_.prodrums, settings_.difficulty));
            } catch (const std::exception& e) {
                out_.dynamics_error = e.what();
                out_.dynamics_message = app::plain_error(e);
            }
            return true;
        } catch (const app::AnalysisCancelled&) {
            return false;  // no error text: a cancel is the user's own click
        } catch (const JobCancelled&) {
            return false;
        }
    });
}

// ---- ReportJob --------------------------------------------------------

ReportJob::ReportJob(store::RecordStore& store, store::CapQuery cap, store::Lens lens,
                     bool open_when_done, double hit_window_ms,
                     std::optional<app::BatchRun> run, app::report::ReportSeed seed)
    : store_(store),
      cap_(cap),
      lens_(lens),
      open_when_done_(open_when_done),
      hit_window_ms_(hit_window_ms),
      run_(std::move(run)),
      seed_(std::move(seed)) {}

void ReportJob::start() { spawn([this] { run(); }); }

void ReportJob::run() {
    run_guarded([this] {
        // One seam for the whole page — rows, counts, and framing come from
        // generate_report, the same call the hydra_report CLI makes.
        app::report::ReportOptions options;
        options.max_paths = app::report::kDefaultReportPaths;
        options.cap = cap_;
        options.lens = lens_;
        options.hit_window_ms = hit_window_ms_;
        options.db_path = app::db_path();
        // Closing Hydra sets this. Without it the window waits for the whole
        // library to be analyzed before it can shut down.
        options.cancel = &cancel_;
        options.run = run_;
        app::report::GeneratedReport report =
            app::report::generate_report(store_, options, seed_);
        // Checked before the "no records" throw and before any file is
        // written: a cancelled run has no rows because it stopped, not
        // because the store is empty, and it must leave the last report on
        // disk alone.
        if (is_cancelled()) return false;
        // generate_report says why the page is empty. Results stored under
        // other settings throw the sentence that names them, which the strip
        // shows as it is; an empty database keeps the app's own sentence.
        if (report.rows == 0) {
            if (report.empty_reason == app::report::EmptyReason::NothingUnderSettings)
                throw KindedError(ErrorKind::AlreadyPlain, report.why_empty);
            throw KindedError(ErrorKind::NoRecords, "no records stored yet");
        }

        // A browser that won't open the page is not a failed report: the
        // page is saved, and the finished strip says so (audit B1).
        outcome_ = publish_report(app::report_html_path(), report.html, open_when_done_);
        return true;
    });
}

}  // namespace hydra::ui
