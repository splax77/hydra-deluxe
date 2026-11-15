#include "ui/library_jobs.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/allocator.h"
#include "app/config.h"
#include "app/report.h"
#include "app/saved_figure_fill.h"
#include "app/user_messages.h"
#include "core/error_kind.h"
#include "parse/song.h"  // display_title, display_artist

namespace hydra::ui {

// ---- StoreOpenJob ---------------------------------------------------------

namespace {
StoreOpenGate g_store_open_gate;
}  // namespace

void set_store_open_gate_for_test(StoreOpenGate gate) { g_store_open_gate = std::move(gate); }

std::string StoreOpenProgress::label() const {
    // The library read names its two steps on every file, upgraded or not
    // (the user's answer of 2026-11-12).
    // The conversion's two steps read as such only on an upgrading file; every
    // other state reads as the open.
    switch (step) {
        case Step::ReadingCharts: return "Reading your charts...";
        case Step::ReadingResults: return "Reading your results...";
        case Step::ConvertingRecords:
            if (upgrading) return "Updating the results table...";
            break;
        case Step::ReadingSavedPaths:
            if (upgrading) return kReadingSavedPathsLabel;
            break;
        case Step::Opening: break;
    }
    return "Opening your library...";
}

namespace {

uint64_t row_count(int64_t rows) { return rows > 0 ? static_cast<uint64_t>(rows) : 0; }

}  // namespace

std::string read_library(store::RecordStore& store, const app::Settings& settings,
                         LibraryModel& library, const LibraryReadFn& progress) {
    using Step = StoreOpenProgress::Step;
    // Both steps count out of this one number.
    uint64_t charts_total = 0;
    try {
        store::RowsReadFn on_row;
        if (progress) {
            charts_total = row_count(store.chart_library_count());
            if (!progress(Step::ReadingCharts, 0, charts_total)) throw JobCancelled{};
            on_row = [&](int64_t rows_read) {
                return progress(Step::ReadingCharts, row_count(rows_read), charts_total);
            };
        }
        // The whole scan in one read. Every chart is needed anyway: the chips
        // count them and the search filters them in memory. set_charts sorts
        // the rows itself, so they come unsorted. The order case in
        // tests/test_library_read_order.cpp proves the shown order does not
        // depend on the read order.
        library.set_charts(store.list_chart_library(0, -1, on_row,  // -1 = no limit
                                                    store::LibraryOrder::Unsorted));
    } catch (const Cancelled&) {
        throw;  // a stop, not a failed read
    } catch (const std::exception& e) {
        return app::plain_error(e);
    }
    try {
        RowsAnsweredFn on_slice;
        if (progress) {
            if (!progress(Step::ReadingResults, 0, charts_total)) throw JobCancelled{};
            on_slice = [&](uint64_t rows_answered) {
                return progress(Step::ReadingResults, rows_answered, charts_total);
            };
        }
        // The chart list above stays on the store's own connection, where its
        // threaded read runs (RecordStore::list_chart_library). The results go
        // through the store's reader, so they never wait for a save group.
        read_library_summaries(store.reader(), settings, library, on_slice);
        library.sort_if_out_of_date();
    } catch (const Cancelled&) {
        throw;  // a stop, not a failed read
    } catch (const std::exception& e) {
        return app::plain_error(e);
    }
    return "";
}

void read_library_summaries(store::StoreReads& store, const app::Settings& settings,
                            LibraryModel& library, const RowsAnsweredFn& on_slice,
                            size_t slice_charts) {
    const std::vector<std::string> hashes = library.hashes();
    // The slices are cut in hash order, the order the results tables are kept
    // in, so each slice reads one stretch of them and the slices together read
    // each page about once. Slices of rows in stored order would each reach
    // across the whole of each table, and SQLite's page cache is too small to
    // keep it between slices (hydra_bench --open times both orders).
    std::vector<size_t> by_hash(hashes.size());
    for (size_t i = 0; i < by_hash.size(); ++i) by_hash[i] = i;
    std::stable_sort(by_hash.begin(), by_hash.end(),
                     [&](size_t a, size_t b) { return hashes[a] < hashes[b]; });
    const size_t slice = std::max<size_t>(slice_charts, 1);
    std::vector<store::RecordLookup> answers(hashes.size());  // parallel to the rows
    std::vector<std::string> part;
    for (size_t from = 0; from < by_hash.size(); from += slice) {
        const size_t to = std::min(by_hash.size(), from + slice);
        part.clear();
        for (size_t i = from; i < to; ++i) part.push_back(hashes[by_hash[i]]);
        std::vector<store::RecordLookup> found = store.lookup(part, settings.profile());
        // lookup gives one answer per id asked, in the order asked.
        for (size_t i = from; i < to; ++i) answers[by_hash[i]] = std::move(found[i - from]);
        if (on_slice && !on_slice(static_cast<uint64_t>(to))) throw JobCancelled{};
    }
    library.set_summaries(answers);
}

StoreOpenJob::StoreOpenJob(std::string db_path, core::RulesStamp rules, app::Settings settings)
    : db_path_(std::move(db_path)),
      rules_(std::move(rules)),
      settings_(std::move(settings)),
      gate_(g_store_open_gate) {}

StoreOpenJob::~StoreOpenJob() { shutdown(); }

void StoreOpenJob::start() {
    started_ = std::chrono::steady_clock::now();
    spawn([this] { run(); });
}

void StoreOpenJob::wait() {
    if (thread_.joinable()) thread_.join();
}

double StoreOpenJob::elapsed_s() const {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - started_).count();
}

StoreOpenProgress StoreOpenJob::snapshot() const {
    StoreOpenProgress p;
    p.step = static_cast<StoreOpenProgress::Step>(step_.load());
    p.rows_done = rows_done_.load();
    p.rows_total = rows_total_.load();
    p.upgrading = upgrading_.load();
    p.elapsed_s = elapsed_s();
    return p;
}

StoreOpenProgress StoreOpenJob::progress() const { return snapshot(); }

bool StoreOpenJob::report(StoreOpenProgress::Step step, uint64_t rows_done, uint64_t rows_total) {
    using Step = StoreOpenProgress::Step;
    // The counts first, then the step: a frame that sees the new step sees
    // its counts too.
    rows_total_.store(rows_total);
    rows_done_.store(rows_done);
    if (step == Step::ConvertingRecords || step == Step::ReadingSavedPaths)
        upgrading_.store(true);
    step_.store(static_cast<int>(step));
    if (gate_) gate_(snapshot(), [this] { return is_cancelled(); });
    return !is_cancelled();
}

namespace {

// The store's step as the startup screen's (StoreOpenProgress::Step).
StoreOpenProgress::Step screen_step(store::OpenStep step) {
    using Step = StoreOpenProgress::Step;
    switch (step) {
        case store::OpenStep::Opening: return Step::Opening;
        case store::OpenStep::ConvertingRecords: return Step::ConvertingRecords;
    }
    return Step::Opening;
}

}  // namespace

void StoreOpenJob::run() {
    using Step = StoreOpenProgress::Step;
    run_guarded([&] {
        try {
            // The store reports its own steps, Opening first; a false return
            // (the app is closing) stops it with the old file as it was.
            const store::OpenProgressFn on_step = [this](const store::OpenProgress& p) {
                return report(screen_step(p.step), row_count(p.rows_done),
                              row_count(p.rows_total));
            };
            store_ = std::make_unique<store::RecordStore>(db_path_, store::OldFilePolicy::SetAside,
                                                          rules_, on_step);
            if (store_->set_aside_path())
                startup_notice_ = app::old_library_set_aside_notice(db_path_);
            // Old rows' hardest figure, before the library reads it. A fill
            // that fails is the status line's problem, as a failed library
            // read is: the app still opens, and the next open tries again.
            bool filled = true;
            std::string fill_problem;
            try {
                filled = app::fill_saved_hardest(
                    *store_, settings_, [this](uint64_t charts_done, uint64_t charts_total) {
                        return report(Step::ReadingSavedPaths, charts_done, charts_total);
                    });
            } catch (const std::exception& e) {
                fill_problem = app::plain_error(e);
            }
            if (!filled) throw JobCancelled{};
            // The library read counts its own two steps; a stop inside it
            // comes out as Cancelled, not as the read's problem.
            library_problem_ = read_library(
                *store_, settings_, library_,
                [this](Step step, uint64_t rows_done, uint64_t rows_total) {
                    return report(step, rows_done, rows_total);
                });
            // A read that fails here is the status line's problem, as a failed
            // library read is; the app still opens.
            if (library_problem_.empty()) {
                try {
                    rescan_needed_ = store_->chart_library_out_of_date();
                } catch (const std::exception& e) {
                    library_problem_ = app::plain_error(e);
                }
            }
            if (library_problem_.empty()) library_problem_ = fill_problem;
            return true;
        } catch (...) {
            // Kept for main's startup box; a cancel is the app closing, so
            // nothing will show it. run_guarded records the failure.
            if (!is_cancelled()) failure_ = std::current_exception();
            store_.reset();
            throw;
        }
    });
}

// ---- ScanJob --------------------------------------------------------------

namespace {
std::function<void()> g_scan_hold;
}  // namespace

void set_scan_hold_for_test(std::function<void()> hold) { g_scan_hold = std::move(hold); }

ScanJob::ScanJob(std::vector<std::string> rootfolders, store::RecordStore& store)
    : rootfolders_(std::move(rootfolders)), store_(store), hold_(g_scan_hold) {}

void ScanJob::start() { spawn([this] { run(); }); }

ScanProgress ScanJob::snapshot() const {
    std::lock_guard<std::mutex> lock(mu_);
    return progress_;
}

void ScanJob::run() {
    // An exception ends the scan as a problem in its list, as a failed save
    // does, and the library stays as it was.
    try {
        scan();
    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(mu_);
        progress_.errors.push_back(app::plain_error_line(e));
        progress_.phase = ScanProgress::Phase::Done;
        progress_.finished = true;
    }
}

void ScanJob::scan() {
    if (hold_) hold_();
    // The previous scan's rows: any chart whose files are unchanged
    // (sig_unchanged) reuses its md5/metadata without being read again.
    const store::ChartLibraryCache cache = app::load_scan_cache(store_);

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

    auto [items, errors] = app::discover_charts(rootfolders_, callbacks, cache);

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

namespace {

// The batch's time-left clock for a run on `workers` workers (D283).
app::TimeLeftClock batch_time_left(int workers) {
    return app::TimeLeftClock::over_last_units(kTimeLeftWorkerRounds * workers);
}

}  // namespace

BatchJob::BatchJob(app::BatchPlan plan, app::BatchRun run, store::RecordStore& store)
    : plan_(std::move(plan)),
      run_(std::move(run)),
      store_(store),
      workers_(app::batch_worker_count()),
      time_left_(batch_time_left(workers_)) {}

void BatchJob::set_analyzer_for_test(app::ChartAnalyzer analyze, int workers) {
    analyze_ = std::move(analyze);
    workers_ = workers;  // stored as given: run_work_pool checks it
    std::lock_guard<std::mutex> lock(mu_);
    time_left_ = batch_time_left(workers_);
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
    // However the run ended (done, stopped or failed), the memory its charts
    // freed then goes back to Windows (D95 call 1), on this job's thread.
    spawn([this] {
        run();
        app::return_freed_memory();
    });
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
    if (!s.finished && !s.paused) s.eta_s = time_left_.read(s.elapsed_s, s.total);
    return s;
}

BatchJob::SavedRows BatchJob::take_saved_rows() {
    std::lock_guard<std::mutex> lock(mu_);
    SavedRows taken = std::move(tray_);
    tray_ = SavedRows{};
    return taken;
}

int BatchJob::answered_count() const {
    std::lock_guard<std::mutex> lock(mu_);
    return answered_;
}

void BatchJob::note_saved(const std::string& md5) noexcept {
    try {
        // run_batch runs each chart once, so an answered chart's answer still
        // stands when its second folder's row is reported.
        if (answered_md5s_.count(md5) == 0 &&
            std::find(waiting_.begin(), waiting_.end(), md5) == waiting_.end())
            waiting_.push_back(md5);
    } catch (...) {
        // Out of memory noting it: the chart's row catches up at the next
        // whole read, and the run goes on.
    }
    answer_waiting();
}

void BatchJob::answer_waiting() noexcept {
    try {
        if (waiting_.empty()) return;
        if (lookup_hold_) lookup_hold_();
        // mu_ stays free here, so a frame's snapshot never waits on the store.
        std::vector<store::RecordLookup> found = store_.lookup(waiting_, run_.profile);
        {
            std::lock_guard<std::mutex> lock(mu_);
            for (size_t i = 0; i < waiting_.size() && i < found.size(); ++i)
                tray_.answers.emplace_back(waiting_[i], std::move(found[i]));
            answered_ += static_cast<int>(waiting_.size());
        }
        answered_md5s_.insert(waiting_.begin(), waiting_.end());
        waiting_.clear();
    } catch (const std::exception& e) {
        // The charts stay waiting; the window shows the sentence as it shows
        // any failed store read.
        try {
            std::string sentence = app::plain_error(e);
            std::lock_guard<std::mutex> lock(mu_);
            tray_.problem = std::move(sentence);
        } catch (...) {
        }
    } catch (...) {
    }
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
        // A save group's charts share the time their report lands, on the
        // batch clock, which leaves paused time out.
        time_left_.record(clock_.elapsed_s(steady_seconds()), p.completed);
    };
    callbacks.on_error = [this](const std::string& title, const std::string& sentence,
                                const std::string& error) {
        std::lock_guard<std::mutex> lock(mu_);
        snap_.failures.push_back(title + ": " + sentence);
        // Empty when the sentence already quotes it: the list draws nothing.
        snap_.failure_details.push_back(error.empty() ? std::string() : title + ": " + error);
    };
    // A saved result and a saved failure both reach their row through the
    // store's own answer; the record handed over is not read.
    callbacks.on_result = [this](const app::ScanItem& item, const store::PreparedRecord&) {
        note_saved(item.md5);
    };
    callbacks.on_failure_saved = [this](const app::ScanItem& item, const store::PreparedRecord&) {
        note_saved(item.md5);
    };
    callbacks.cancel = &cancel_;
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
    // Charts whose lookup failed are asked about once more before the run
    // ends, so a finished snapshot means the tray holds the last answers.
    try {
        app::run_batch(plan_, run_, store_, workers_, callbacks);
    } catch (const std::exception& e) {
        answer_waiting();
        finish_failed(e, app::plain_error_detail(e));
        return;
    }

    answer_waiting();
    std::lock_guard<std::mutex> lock(mu_);
    finish_locked();
}

// ---- ViewJob ----------------------------------------------------------

namespace {
app::ChartAnalyzer g_view_analyzer;
std::function<void()> g_view_before_save_check;

// Whether a chart's stored answer already says what `fresh` would save, so a
// click saves nothing (D87 item 2): the same Ready record, or the same stored
// failure, kind and text.
bool stored_says_same(const store::RecordLookup& found, const store::RecordRow& fresh) {
    if (fresh.failure_kind)
        return found.status == store::RecordStatus::CantAnalyze &&
               found.failure_kind == fresh.failure_kind && found.failure_text == fresh.failure_text;
    return found.status == store::RecordStatus::Ready && found.record == fresh;
}

// The record a click's failed analysis is stored as: app::prepare_failed_chart's
// answer for what the analysis threw. Nothing when it threw nothing.
std::optional<store::PreparedRecord> click_failure_record(const std::string& song_id,
                                                          const store::Profile& profile,
                                                          const std::exception_ptr& thrown) {
    if (!thrown) return std::nullopt;
    try {
        std::rethrow_exception(thrown);
    } catch (const std::exception& e) {
        return app::prepare_failed_chart(song_id, profile, e);
    } catch (...) {
    }
    return std::nullopt;
}
}  // namespace

void set_view_analyzer_for_test(app::ChartAnalyzer analyze) {
    g_view_analyzer = std::move(analyze);
}

void set_view_before_save_check_for_test(std::function<void()> hook) {
    g_view_before_save_check = std::move(hook);
}

ViewJob::ViewJob(store::ChartLibraryEntry song, store::Profile profile,
                 app::AnalysisSettings settings, bool analysis_off, int generation,
                 store::RecordStore& store)
    : song_(std::move(song)),
      profile_(std::move(profile)),
      settings_(std::move(settings)),
      analysis_off_(analysis_off),
      generation_(generation),
      store_(store),
      analyze_(g_view_analyzer ? g_view_analyzer : app::ChartAnalyzer(app::analyze_chart_file)),
      before_save_check_(g_view_before_save_check) {}

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
            // asking for a hash on every click (D96). A fingerprint that
            // could never show the file unchanged (store::sig_can_show_unchanged)
            // would not stop that, so it is only saved with a new hash.
            if (const std::optional<app::ChartNow> now =
                    app::chart_changed_since(song_.notespath, song_.sig)) {
                if (now->md5.empty())
                    throw std::runtime_error("could not read " + song_.notespath);
                if (now->differs_from(song_.md5) || store::sig_can_show_unchanged(now->sig)) {
                    out_.new_md5 = now->md5;
                    out_.new_sig = now->sig;
                    out_.files_changed = true;
                }
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
                                                            out_.analysis->song);
                } catch (const app::AnalysisCancelled&) {
                    throw;
                } catch (const std::exception& e) {
                    out_.analysis.reset();
                    out_.analysis_failure = std::current_exception();
                    out_.analysis_error = app::plain_error_detail(e);
                    out_.analysis_message = app::plain_error(e);
                }
            }
            throw_if_cancelled();
            // When the count may reuse the analysis's song: see
            // app::dynamics_for_settings.
            try {
                out_.dynamics = app::dynamics_for_settings(
                    path, settings_.reading(), out_.analysis ? &out_.analysis->song : nullptr);
            } catch (const std::exception& e) {
                out_.dynamics_error = app::plain_error_detail(e);
                out_.dynamics_message = app::plain_error(e);
            }
            write_results();
            return true;
        } catch (const app::AnalysisCancelled&) {
            return false;  // no error text: a cancel is the user's own click
        } catch (const JobCancelled&) {
            return false;
        }
    });
}

// The two writes run here, on the job's thread, so a batch's save group
// makes this thread wait and not the window. A cancel seen just before
// either write ends the job with nothing more written; a closed or replaced
// click is cancelled too (AppState::close_details, AppState::start_view).
void ViewJob::write_results() {
    // Changed chart files: the library row takes the hash and fingerprint
    // they give now, before anything is saved under them. An edited chart
    // gets a new hash (D87 item 3); one only saved again keeps its hash and
    // takes the new fingerprint (D96). A write that fails fails the job and
    // saves nothing: a summary under a hash no library row has would be an
    // orphan.
    if (out_.files_changed) {
        throw_if_cancelled();
        store_.reidentify_chart(song_.notespath, out_.new_md5, out_.new_sig);
        out_.wrote_hash = true;
    }
    if (analysis_off_) return;
    const std::string& song_id = out_.stored_id(song_.md5);
    // The record is built as the batch builds it, and a failure the store
    // remembers is the chart's result, as a batch stores it.
    std::optional<store::PreparedRecord> prepared;
    try {
        if (out_.analysis)
            prepared = store::prepare_record(song_id, profile_, out_.analysis->record);
        else
            prepared = click_failure_record(song_id, profile_, out_.analysis_failure);
    } catch (const std::exception& e) {
        out_.save_error = app::plain_error(e);
        return;
    }
    if (!prepared) return;
    if (before_save_check_) before_save_check_();
    throw_if_cancelled();
    try {
        if (stored_says_same(store_.lookup_one(song_id, prepared->profile), prepared->record))
            return;  // nothing new to save
        store_.save(*prepared);
        out_.wrote_save = true;
    } catch (const std::exception& e) {
        out_.save_error = app::plain_error(e);
    }
}

// ---- ReportJob --------------------------------------------------------

namespace {
std::function<void()> g_report_hold;
}  // namespace

void set_report_hold_for_test(std::function<void()> hold) { g_report_hold = std::move(hold); }

ReportJob::ReportJob(store::RecordStore& store, app::BatchRun run)
    : store_(store), run_(std::move(run)), hold_(g_report_hold) {}

void ReportJob::start() {
    spawn([this] { run(); });
}

void ReportJob::run() {
    run_guarded([this] {
        // One seam for the whole report: rows, counts, and framing come from
        // generate_report.
        app::report::ReportOptions options;
        options.max_paths = store::kPathsPerRecord;
        options.db_path = app::db_path();
        // Closing Hydra or the window's Cancel sets this.
        options.cancel = &cancel_;
        options.run = run_;
        if (hold_) hold_();
        app::report::GeneratedReport report = app::report::generate_report(store_, options);
        // Checked before the "no records" throw: a cancelled run has no rows
        // because it stopped, not because the store is empty, and the last
        // report in memory stays as it was.
        if (is_cancelled()) return false;
        // generate_report says why the page is empty (GeneratedReport::
        // why_empty). When it gives a sentence, the strip shows that sentence
        // as it is, the one generate_report wrote. When it gives none, the
        // database holds no results, and the app's own sentence says so (D97).
        // A page whose charts all can't be analyzed or have no result is a
        // page, not a failure: the window shows its coverage lines with
        // why_empty as its empty text (D249 item 5).
        if (report.rows == 0 && report.empty_reason != app::report::EmptyReason::NothingToShow) {
            if (!report.why_empty.empty())
                throw KindedError(ErrorKind::AlreadyPlain, report.why_empty);
            throw KindedError(ErrorKind::NoRecords, "no records stored yet");
        }

        result_ = std::make_shared<const app::report::GeneratedReport>(std::move(report));
        return true;
    });
}

}  // namespace hydra::ui
