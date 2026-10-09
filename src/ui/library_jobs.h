// The library tab's background jobs: opening the library file at startup
// (StoreOpenJob), scanning chart folders (ScanJob), batch-analyzing many
// charts (BatchJob), analyzing the clicked chart (ViewJob), and building the
// HTML path report afterwards (ReportJob). AppState owns one of each at most;
// the library files (library_toolbar.cpp, library_dialogs.cpp, library_view.cpp)
// and details_panel.cpp poll them per frame.
//
// Every job is polled once per frame from the render thread via snapshot()/
// finished(); the worker thread never touches ImGui state directly.

#ifndef HYDRA_UI_LIBRARY_JOBS_H
#define HYDRA_UI_LIBRARY_JOBS_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/dynamics_breakdown.h"
#include "app/report.h"
#include "core/model.h"
#include "store/record_store.h"
#include "ui/job_base.h"
#include "ui/library_model.h"

namespace hydra::ui {

class ByteRateClock;  // ui/preview_load_job.h

// ---- StoreOpenJob ---------------------------------------------------------

// Where the startup open of the library file is, for the startup screen. The
// first four steps are the store's own (it reports them while it opens and,
// for an old file, upgrades); LoadingLibrary is the job's read of the chart
// list and its summaries once the store is open.
struct StoreOpenProgress {
    enum class Step { Opening, UpdatingResultsKey, Copying, Finishing, LoadingLibrary };
    Step step = Step::Opening;
    // Rows copied into the upgraded file, out of the rows to copy, both as
    // the store counted them. 0 of 0 until the copy begins, and on a file
    // that needs no upgrade.
    uint64_t rows_done = 0;
    uint64_t rows_total = 0;
    // Whether this open has upgraded the file at any step so far. It stays
    // set through Finishing and LoadingLibrary, so the screen keeps its
    // upgrade heading and full bar until the library appears.
    bool upgrading = false;
    // Seconds since the job started.
    double elapsed_s = 0.0;
    // Seconds left at the measured row rate (ByteRateClock); < 0 while not
    // known.
    double time_left_s = -1.0;

    // The bar is progress_bar_counted of the two counts (ui/widgets.h),
    // drawn once rows_total is known.
    // The line under the heading: what the open is doing now.
    std::string label() const;
    // The Preview loader's time-left words and gate, applied to the copy:
    // "" except while Copying.
    std::string time_left_text() const;
};

// Reads every chart in the library, then every row's summary under
// `settings` (its chart mode, SP cap and lens), into `library`: the read at
// startup and after a scan. Returns the plain sentence of the read that
// failed, or "" when both worked; a failed read leaves the model's rows as
// they were (D73 item 3).
std::string read_library(store::RecordStore& store, const app::Settings& settings,
                         LibraryModel& library);
// Re-reads every row's summary under `settings`: one store call for the
// whole library, never per row per frame (the batch workers share the
// store's lock). Throws when the read fails.
void read_library_summaries(store::RecordStore& store, const app::Settings& settings,
                            LibraryModel& library);

// The startup open's test gate (set_store_open_gate_for_test below).
using StoreOpenGate =
    std::function<void(const StoreOpenProgress& progress, const std::function<bool()>& cancelled)>;

// Opens the library file off the UI thread at startup, then reads the
// library the way AppState::reload_library does, into a model of its own.
// AppState::tick collects it: the store and the model move into AppState, and
// a failed read of the library becomes the status line's problem, as it does
// on the UI thread. A store that will not open fails the job; main.cpp then
// shows the startup message box (D72 item 1) and Hydra closes. Closing the
// window while it runs cancels the open (the store stops at its next
// progress report) and joins the thread.
class StoreOpenJob : public ResultJobBase {
public:
    // `settings` is copied: its chart mode, SP cap and lens pick the
    // summaries the library shows, as reload_library's do.
    StoreOpenJob(std::string db_path, core::RulesStamp rules, app::Settings settings);
    ~StoreOpenJob();

    void start();
    // Blocks until the thread has exited. A deliberate wait, for the test
    // harness only (AppState::wait_store_open); the app never calls it.
    void wait();

    // A snapshot for the startup screen. It also feeds the time-left clock,
    // so the render thread calls it once a frame.
    StoreOpenProgress progress() const;
    // Seconds since start(), without touching the clock.
    double elapsed_s() const;

    // Valid once finished() && ok(); each moves its part out (call once).
    std::unique_ptr<store::RecordStore> take_store() { return std::move(store_); }
    LibraryModel take_library() { return std::move(library_); }
    // The plain sentence of a library read that failed after the store
    // opened, or "" when the read worked.
    const std::string& library_problem() const { return library_problem_; }
    // The exception that failed the job, for the startup message box. Null
    // unless finished() && !ok().
    std::exception_ptr failure() const { return failure_; }

private:
    void run();
    // Publishes a step and counts, then waits at the test gate when one is
    // set. False once the job is cancelled: the store stops there.
    bool report(StoreOpenProgress::Step step, uint64_t rows_done, uint64_t rows_total);
    // The published step and counts, with no time-left estimate.
    StoreOpenProgress snapshot() const;

    std::string db_path_;
    core::RulesStamp rules_;
    app::Settings settings_;
    std::unique_ptr<store::RecordStore> store_;
    LibraryModel library_;
    std::string library_problem_;
    std::exception_ptr failure_;
    StoreOpenGate gate_;  // the test seam's, copied at construction

    // Written by the job's thread, read by the render thread.
    std::atomic<int> step_{static_cast<int>(StoreOpenProgress::Step::Opening)};
    std::atomic<uint64_t> rows_done_{0};
    std::atomic<uint64_t> rows_total_{0};
    std::atomic<bool> upgrading_{false};
    std::chrono::steady_clock::time_point started_{};  // set in start(), then read-only

    // The time-left clock, fed from progress() on the render thread.
    mutable std::mutex clock_mutex_;
    std::unique_ptr<ByteRateClock> clock_;
};

// GUI-test seam: when set, every startup open built from now on calls `gate`
// from its thread at every progress report, after publishing it, and carries
// on once the gate returns. `cancelled` reads true once the app is closing,
// so a gate that waits must give up then. An empty function clears it. Only
// the uitest harness sets it.
void set_store_open_gate_for_test(StoreOpenGate gate);

// ---- ScanJob --------------------------------------------------------------

struct ScanProgress {
    // Enumerating: walking folders (count grows, no denominator yet).
    // Reading: hashing charts / reading metadata, charts_done/charts_total.
    // Writing: replacing the library table. Done: finished (or cancelled).
    enum class Phase { Enumerating, Reading, Writing, Done };
    Phase phase = Phase::Enumerating;
    int folders_seen = 0;
    int charts_total = 0;
    int charts_done = 0;
    int charts_cached = 0;  // satisfied by the rescan cache, no file reads
    int charts_found = 0;   // final scan item count
    std::vector<std::string> errors;
    bool cancelled = false;
    bool finished = false;
};

// A cancelled scan writes nothing: the previous library stays as-is.
class ScanJob : public JobBase {
public:
    ScanJob(std::vector<std::string> rootfolders, store::RecordStore& store);
    ~ScanJob() { shutdown(); }

    void start();
    ScanProgress snapshot() const;

private:
    void run();

    std::vector<std::string> rootfolders_;
    store::RecordStore& store_;

    mutable std::mutex mu_;
    ScanProgress progress_;
};

// ---- BatchJob ---------------------------------------------------------

// How long a batch has been working, with paused stretches left out. Times
// are seconds on any steady clock: BatchJob feeds it steady_clock readings,
// tests feed it plain numbers.
class BatchClock {
public:
    void start(double now);
    void pause(double now);   // does nothing unless running
    void resume(double now);  // does nothing unless paused
    void finish(double now);  // freezes the total; a paused clock stops at its pause
    double elapsed_s(double now) const;
    bool paused() const { return paused_at_.has_value(); }

private:
    std::optional<double> started_, paused_at_, finished_at_;
    double paused_total_ = 0.0;
};

// How many charts must finish before a time-left estimate shows.
inline constexpr int kEtaMinFinished = 3;

// Time left: the average wall time per finished chart so far, times the
// charts still to go. Empty until kEtaMinFinished charts have finished.
std::optional<double> batch_eta_s(double elapsed_s, int completed, int total);

// A library entry as the batch's scan row: the fields of the same names, with
// `sig` left empty (nothing the batch runs reads it). The other direction is
// app::to_library_entry; this one can move beside it once analysis.h is free.
app::ScanItem scan_item_of(const store::ChartLibraryEntry& e);

class BatchJob : public JobBase {
public:
    // Runs exactly this plan (app::plan_batch), the one the confirm showed
    // (D79). The library screen's own search (T12) decides which rows the
    // plan covers, so "Analyze search (N)..." hands over the N rows it shows
    // instead of a search string SQL would read differently.
    BatchJob(app::BatchPlan plan, app::BatchRun run, store::RecordStore& store);
    ~BatchJob() {
        stop();
        shutdown();
    }

    // Test seam: what analyzes one chart, and how many workers run. Call
    // before start().
    void set_analyzer_for_test(app::ChartAnalyzer analyze, int workers);

    void start();

    // Pause stops handing out new charts. Charts already being analyzed
    // finish and are stored. Resume carries on. Both may be called before
    // start(); neither does anything once the run has finished.
    void pause();
    void resume();
    // Ends the run. Results already stored stay stored. A chart in the middle
    // of its analysis stops at its next progress tick and is neither stored
    // nor counted as failed. Works while paused.
    void stop();
    // The old name for stop(), kept until T13 moves the batch modal's
    // callers to stop().
    void cancel() { stop(); }

    // The batch's settings: what its results are filed under, for the report
    // that follows it.
    const app::BatchRun& batch_run() const { return run_; }
    // The report rows of every chart the run saved (D87 item 5), for the
    // report that follows it. Read once the snapshot says finished; it moves
    // the rows out and leaves the job an empty seed.
    app::report::ReportSeed take_report_seed() { return std::exchange(seed_, {}); }

    struct Snapshot {
        bool preparing = true;  // BatchJob::run has not reached its first chart yet
        // The batch's counts, copied from app::BatchProgress (run_batch is
        // their owner; see its fields for what each one counts).
        int total = 0;
        int completed = 0;
        int analyzed = 0;
        int skipped = 0;
        int failed = 0;
        bool paused = false;
        // Wall time since start(), paused time left out; frozen once finished.
        double elapsed_s = 0.0;
        // Seconds left. Empty until kEtaMinFinished charts finish, while
        // paused, and once finished.
        std::optional<double> eta_s;
        // The chart a worker started most recently. Empty once finished.
        std::string current_title;
        std::string current_artist;
        // One line per failed library row, `failed` long (run_batch's
        // on_error).
        std::vector<std::string> failures;         // "Title: plain message"
        std::vector<std::string> failure_details;  // "Title: raw error", same order
        // A run that failed as a whole: the plain sentence and the raw text.
        // It is not a library row, so no count includes it.
        std::string run_error;
        std::string run_error_detail;
        bool finished = false;
    };
    Snapshot snapshot() const;

private:
    void run();
    // Ends a run that failed as a whole, around its charts: the plain
    // sentence for `e` with `detail` under it in run_error, the clock
    // stopped. No count changes.
    void finish_failed(const std::exception& e, std::string detail);
    // Marks the snapshot finished: every way run() ends goes through here.
    // The caller holds mu_.
    void finish_locked();
    // On a worker, before each chart: waits while paused. Throws
    // app::AnalysisCancelled once stopped, so the chart counts as stopped.
    void wait_while_paused();
    // On a worker: records the chart it is starting as the current one.
    void note_started(const std::string& notespath);

    app::BatchPlan plan_;
    // notespath -> index into plan_.todo. Built before run_batch starts and
    // only read after that, so workers read it without a lock.
    std::unordered_map<std::string, size_t> by_path_;
    app::BatchRun run_;
    store::RecordStore& store_;
    int workers_;
    app::ChartAnalyzer analyze_;  // empty = app::analyze_chart_file
    // Filled by run_batch on the job's thread (BatchCallbacks::report_seed).
    app::report::ReportSeed seed_;

    std::mutex pause_mu_;
    std::condition_variable pause_cv_;
    bool paused_ = false;  // guarded by pause_mu_

    mutable std::mutex mu_;
    Snapshot snap_;     // guarded by mu_
    BatchClock clock_;  // guarded by mu_
};

// GUI-test seam: when set, every batch the app starts runs this analyzer on
// this many workers (see BatchJob::set_analyzer_for_test), so a GUI test can
// hold a run open instead of racing a small library to its end. An empty
// analyzer clears it. Only the uitest harness sets it.
void set_app_batch_analyzer_for_test(app::ChartAnalyzer analyze, int workers);
// Hands the seam to a batch about to start; does nothing when it is unset.
void apply_app_batch_analyzer_for_test(BatchJob& job);

// ---- ViewJob ----------------------------------------------------------

// What one click's job found on its thread (D87 items 1 and 3).
struct ViewOutcome {
    // Set when the chart's files changed since the scan: the hash and
    // fingerprint they give now. The hash is the row's own when the files
    // were only saved again with the same content (D96); ViewJob::run says
    // when such a fingerprint is worth saving.
    bool files_changed = false;
    std::string new_md5;
    std::string new_sig;
    // The engine's analysis. Unset when analysis is off (broken rules) or
    // it failed; then analysis_message / analysis_error say why.
    std::optional<app::AnalysisResult> analysis;
    store::SongLength length;
    std::string analysis_message;  // app::plain_error
    std::string analysis_error;    // the raw text
    // The Dynamics count, or why it failed.
    std::optional<app::DynamicsBreakdown> dynamics;
    std::string dynamics_message;
    std::string dynamics_error;
};

// The song panel's job: re-identifies an edited chart, runs the engine's
// normal analysis and counts the Dynamics, all for one click. cancel()
// interrupts the search at its next progress tick; the stretch before the
// first tick (parse + graph build) can't be interrupted. The UI thread drops
// a result whose generation is not the current click's.
class ViewJob : public ResultJobBase {
public:
    // The song, its RecordKey and the settings are snapshotted at start, so
    // the result is saved against the song and settings it ran for.
    // `analysis_off` (broken rules) skips the engine and keeps the count.
    ViewJob(store::ChartLibraryEntry song, store::RecordKey key, app::AnalysisSettings settings,
            bool analysis_off, int generation);
    ~ViewJob() { shutdown(); }

    void start();

    const store::ChartLibraryEntry& song() const { return song_; }
    const store::RecordKey& key() const { return key_; }
    const app::AnalysisSettings& settings() const { return settings_; }
    bool analysis_off() const { return analysis_off_; }
    int generation() const { return generation_; }

    // Monotonic 0..1 search progress, or a negative value before the first
    // report (i.e. show an indeterminate spinner until then).
    float progress() const { return progress_.load(std::memory_order_relaxed); }

    // Valid once finished() && ok(); moves the outcome out (call once).
    ViewOutcome take_outcome() { return std::move(out_); }

private:
    void run();

    store::ChartLibraryEntry song_;
    store::RecordKey key_;
    app::AnalysisSettings settings_;
    bool analysis_off_;
    int generation_;
    std::atomic<float> progress_{-1.0f};
    ViewOutcome out_;
    app::ChartAnalyzer analyze_;  // the test seam's, copied at construction
};

// Test seam: when set, every click's job built from now on runs this analyzer
// in place of app::analyze_chart_file, so a test can hold a click's job open
// (Cancel, the progress box's delay, a burst of setting changes) instead of
// racing a fast chart. An empty analyzer clears it. Only tests set it.
void set_view_analyzer_for_test(app::ChartAnalyzer analyze);

// ---- ReportJob --------------------------------------------------------

// Builds the library's path report in memory (app::report::generate_report)
// for the path report window (D103). Kicked off when a library batch analysis
// finishes, and by the window's Refresh (AppState::request_path_report).
// Analyzing the library's charts can take seconds on a big library, so it runs
// off the render thread like every other job. It writes and opens nothing.
class ReportJob : public ResultJobBase {
public:
    // hit_window_ms feeds the report's timing-tier bands (settings.hit_window_ms).
    // cap/lens: which records the report lists (the user's current SP cap, ms
    // limit and score range). `run` is the batch's settings the charts are
    // analyzed under (ReportOptions::run; the report fails without it), and
    // `seed` the rows the batch already worked out.
    ReportJob(store::RecordStore& store, store::CapQuery cap, store::Lens lens,
              double hit_window_ms = kDefaultHitWindowMs,
              std::optional<app::BatchRun> run = std::nullopt,
              app::report::ReportSeed seed = {});
    ~ReportJob() { shutdown(); }

    void start();

    // What the report is built from, as given to the constructor.
    const store::CapQuery& cap() const { return cap_; }
    const store::Lens& lens() const { return lens_; }
    double hit_window_ms() const { return hit_window_ms_; }

    // Valid once finished() && ok(): the report, shared with AppState's slot
    // rather than copied.
    const std::shared_ptr<const app::report::GeneratedReport>& result() const { return result_; }

    // Test seam: how many charts' rows the job still holds from the batch's
    // seed. Read before start() or once finished().
    size_t seed_charts_for_test() const { return seed_.rows.size(); }

    // How many charts the build has analyzed of how many it will
    // (ReportOptions::progress), as of its latest call: 0 of 0 before the
    // first. Safe to read from the UI thread while the job runs.
    std::pair<int, int> progress() const {
        return {progress_done_.load(std::memory_order_relaxed),
                progress_total_.load(std::memory_order_relaxed)};
    }

private:
    void run();

    std::atomic<int> progress_done_{0};
    std::atomic<int> progress_total_{0};
    store::RecordStore& store_;
    store::CapQuery cap_;
    store::Lens lens_;
    double hit_window_ms_;
    std::optional<app::BatchRun> run_;
    app::report::ReportSeed seed_;
    app::ChartAnalyzer analyze_;  // the test seam's, copied at construction
    std::shared_ptr<const app::report::GeneratedReport> result_;
};

// Test seam: when set, every report job built from now on analyzes its
// charts with this in place of generate_report's own, so a test can hold a
// build open (one build at a time, Cancel, an out-of-date event during a
// build). An empty analyzer clears it. Only tests set it.
void set_report_analyzer_for_test(app::ChartAnalyzer analyze);

}  // namespace hydra::ui

#endif  // HYDRA_UI_LIBRARY_JOBS_H
