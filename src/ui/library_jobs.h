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
#include <unordered_set>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/dynamics_breakdown.h"
#include "app/job_base.h"
#include "app/report.h"
#include "app/time_left.h"
#include "core/model.h"
#include "store/record_store.h"
#include "ui/library_model.h"

namespace hydra::ui {

// ---- StoreOpenJob ---------------------------------------------------------

// The line the startup screen shows while the open fills in old rows' hardest
// figure (StoreOpenProgress::Step::ReadingSavedPaths).
inline constexpr const char* kReadingSavedPathsLabel = "Reading saved paths...";

// Where the startup open of the library file is, for the startup screen. The
// first two steps are the store's own (store::OpenStep; it reports them while
// it opens and, for a convertible file, converts it). ReadingSavedPaths is the
// job's one-time fill of old rows' hardest figure (app::fill_saved_hardest),
// reported only when the fill has charts to read. ReadingCharts and
// ReadingResults are the job's read of the library once the store is open
// (read_library): its chart list, then each row's saved result.
struct StoreOpenProgress {
    enum class Step { Opening, ConvertingRecords, ReadingSavedPaths, ReadingCharts, ReadingResults };
    Step step = Step::Opening;
    // The current step's counts: rows converted out of the rows the
    // conversion converts, as the store counted them (store::OpenProgress),
    // charts read out of the charts the fill reads, or library rows read or
    // answered out of the library's chart count (read_library). 0 of 0 until
    // the first counted step begins.
    uint64_t rows_done = 0;
    uint64_t rows_total = 0;
    // Whether this open has converted the file or filled old rows at any step
    // so far. It stays set through the library read, so the screen keeps its
    // upgrade heading until the library appears.
    bool upgrading = false;
    // Seconds since the job started.
    double elapsed_s = 0.0;

    // The bar is progress_bar_counted of the two counts (ui/widgets.h),
    // drawn once rows_total is known.
    // The line under the heading: what the open is doing now.
    std::string label() const;
};

// Told where the library read is: ReadingCharts or ReadingResults, with the
// rows read or answered out of the library's chart count. A false return
// stops the read (read_library).
using LibraryReadFn = std::function<bool(StoreOpenProgress::Step step, uint64_t rows_done,
                                         uint64_t rows_total)>;

// Reads every chart in the library, then every row's summary under
// `settings` (its chart mode, SP cap and lens), into `library`, and leaves it
// sorted on them: the read at startup and after a scan. Returns the plain
// sentence of the read that failed, or "" when both worked; a failed read
// leaves the model's rows as they were (D73 item 3).
//
// `progress`, when set, is told each step's start at 0, then each row the
// chart list reads, then each slice of results answered, every report out of
// one total, the store's chart_library_count(). When it returns false the read
// throws Cancelled (core/error_kind.h), which this lets through instead of
// returning it as a sentence. Empty for a caller with nothing to show: then
// the read asks for no count and reads the results in one call.
std::string read_library(store::RecordStore& store, const app::Settings& settings,
                         LibraryModel& library, const LibraryReadFn& progress = {});

// How many library rows one slice of the counted results read asks the store
// about: the user's answer of 2026-11-12, "500 charts".
inline constexpr size_t kResultsSliceCharts = 500;

// Told after each slice of results with the rows answered so far. A false
// return stops the read (read_library_summaries).
using RowsAnsweredFn = std::function<bool(uint64_t rows_answered)>;

// Re-reads every row's summary under `settings`: never per row per frame (the
// batch workers share the store's lock). The rows keep their order
// (LibraryModel::sort_if_out_of_date). The store is asked `slice_charts` rows
// at a time (a slice of 0 reads as 1), the slices cut from the rows sorted by
// hash, and `on_slice`, when given, is told after each. Each answer goes back
// to its own row, and the answers go to the model in one call, so the table is
// filtered once. Throws when the read fails, and Cancelled when `on_slice`
// returns false.
void read_library_summaries(store::RecordStore& store, const app::Settings& settings,
                            LibraryModel& library, const RowsAnsweredFn& on_slice = {},
                            size_t slice_charts = kResultsSliceCharts);

// The startup open's test gate (set_store_open_gate_for_test below).
using StoreOpenGate =
    std::function<void(const StoreOpenProgress& progress, const std::function<bool()>& cancelled)>;

// Opens the library file off the UI thread at startup, fills in old rows'
// hardest figure once per profile (app::fill_saved_hardest), then reads the
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

    // A snapshot for the startup screen, read once a frame.
    StoreOpenProgress progress() const;
    // Seconds since start().
    double elapsed_s() const;

    // Valid once finished() && ok(); each moves its part out (call once).
    std::unique_ptr<store::RecordStore> take_store() { return std::move(store_); }
    LibraryModel take_library() { return std::move(library_); }
    // The plain sentence of a library read, or a fill of old rows' hardest
    // figure, that failed after the store opened, or "" when both worked.
    // The read's problem is the one kept when both failed.
    const std::string& library_problem() const { return library_problem_; }
    // The sentence to show once when the open set the old library file aside
    // (store::RecordStore::set_aside_path), or "" when it set nothing aside.
    // Valid once finished() && ok().
    const std::string& startup_notice() const { return startup_notice_; }
    // Whether the opened library needs a scan, as
    // store::RecordStore::chart_library_out_of_date answered right after the
    // library read. Valid once finished() && ok().
    bool rescan_needed() const { return rescan_needed_; }
    // The exception that failed the job, for the startup message box. Null
    // unless finished() && !ok().
    std::exception_ptr failure() const { return failure_; }

private:
    void run();
    // Publishes a step and counts, then waits at the test gate when one is
    // set. False once the job is cancelled: the store stops there.
    bool report(StoreOpenProgress::Step step, uint64_t rows_done, uint64_t rows_total);
    // The published step and counts.
    StoreOpenProgress snapshot() const;

    std::string db_path_;
    core::RulesStamp rules_;
    app::Settings settings_;
    std::unique_ptr<store::RecordStore> store_;
    LibraryModel library_;
    std::string library_problem_;
    std::string startup_notice_;
    bool rescan_needed_ = false;
    std::exception_ptr failure_;
    StoreOpenGate gate_;  // the test seam's, copied at construction

    // Written by the job's thread, read by the render thread.
    std::atomic<int> step_{static_cast<int>(StoreOpenProgress::Step::Opening)};
    std::atomic<uint64_t> rows_done_{0};
    std::atomic<uint64_t> rows_total_{0};
    std::atomic<bool> upgrading_{false};
    std::chrono::steady_clock::time_point started_{};  // set in start(), then read-only
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
    void run();   // scan() with its exceptions caught
    void scan();  // the scan itself

    std::vector<std::string> rootfolders_;
    store::RecordStore& store_;
    std::function<void()> hold_;  // set_scan_hold_for_test's hook, taken at construction

    mutable std::mutex mu_;
    ScanProgress progress_;
};

// Test seam: when set, every scan job built from now on calls this on its own
// thread before it reads anything, so a test can make a scan throw. An empty
// hook clears it. Only tests set it.
void set_scan_hold_for_test(std::function<void()> hold);

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

// The batch's time-left window is this many rounds of its workers: the last
// kTimeLeftWorkerRounds * W finished charts, W being the run's worker count
// (D283). app::TimeLeftClock does the rest.
inline constexpr int kTimeLeftWorkerRounds = 2;

class BatchJob : public JobBase {
public:
    // Runs exactly this plan (app::plan_batch), the one the confirm showed
    // (D79). The rows on screen (LibraryModel::order) decide which rows the
    // plan covers, so the Analyze button hands over the rows the table shows
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
    // nor counted as failed. Works while paused, which JobBase::cancel does
    // not: it wakes no paused worker.
    void stop();

    // The batch's settings: what its results are filed under, for the report
    // that follows it.
    const app::BatchRun& batch_run() const { return run_; }

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
        // Seconds left (app::TimeLeftClock over the last
        // kTimeLeftWorkerRounds rounds of workers). Empty until that window
        // is full, while paused, and once finished.
        std::optional<double> eta_s;
        // The chart a worker started most recently. Empty once finished.
        std::string current_title;
        std::string current_artist;
        // One line per failed library row, `failed` long (run_batch's
        // on_error).
        std::vector<std::string> failures;         // "Title: plain message"
        // "Title: details line", same order; empty when the message already
        // quotes it (app::plain_error_detail).
        std::vector<std::string> failure_details;
        // A run that failed as a whole: the plain sentence and its details
        // line (app::plain_error_detail).
        // It is not a library row, so no count includes it.
        std::string run_error;
        std::string run_error_detail;
        bool finished = false;
    };
    Snapshot snapshot() const;

    // The store's answers for the charts this run saved, waiting for the
    // window to put them on their rows. Each answer is
    // RecordStore::lookup's, asked on the batch's thread under the run's
    // profile right after the chart's save group committed; the job decides
    // no status itself. A chart in two folders has one answer.
    struct SavedRows {
        std::vector<std::pair<std::string, store::RecordLookup>> answers;  // md5, answer
        // app::plain_error of the last lookup that failed since the last
        // take; empty when none did. Its charts are asked about again with
        // the next saved chart, and once more when the run ends.
        std::string problem;
    };
    // Hands over what the tray holds and empties it. Never waits on the
    // store: a lookup runs without the job's own lock.
    SavedRows take_saved_rows();
    // How many charts' answers the job has made so far, taken or not. A
    // snapshot's `completed` can count a chart before its answer is made
    // (run_batch's on_progress fires first), so tests wait on this instead.
    // Reads only the job's own count.
    int answered_count() const;

    // Test seam: called on the batch thread just before each lookup, so a
    // test can hold the batch there. Call before start().
    void set_lookup_hold_for_test(std::function<void()> hold) { lookup_hold_ = std::move(hold); }

private:
    void run();
    // On the batch thread, from run_batch's on_result and on_failure_saved:
    // `md5` was saved, so it waits for its answer unless it already has one.
    // Then asks about everything waiting. Lets no exception out, because one
    // out of a batch callback ends the whole run.
    void note_saved(const std::string& md5) noexcept;
    // On the batch thread: asks the store about every waiting chart in one
    // lookup and moves the answers into the tray. A lookup that fails keeps
    // them waiting and leaves its sentence in the tray. Lets no exception out.
    void answer_waiting() noexcept;
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

    std::mutex pause_mu_;
    std::condition_variable pause_cv_;
    bool paused_ = false;  // guarded by pause_mu_

    mutable std::mutex mu_;
    Snapshot snap_;     // guarded by mu_
    BatchClock clock_;  // guarded by mu_
    // Fed each finish on clock_'s time, so paused stretches never count in
    // its window; read by snapshot(). Its window follows workers_ (see
    // kTimeLeftWorkerRounds). Guarded by mu_.
    mutable app::TimeLeftClock time_left_;

    // Saved charts still waiting for their answer, and the charts already
    // answered this run. Only the batch thread (run_batch's callbacks, then
    // run()) touches them, so they need no lock.
    std::vector<std::string> waiting_;
    std::unordered_set<std::string> answered_md5s_;
    std::function<void()> lookup_hold_;  // set_lookup_hold_for_test
    SavedRows tray_;    // guarded by mu_
    int answered_ = 0;  // answers made so far (answered_count); guarded by mu_
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
    std::string analysis_error;    // app::plain_error_detail, may be empty
    // What the failed analysis threw, so the click can store a failure the
    // store remembers (app::prepare_failed_chart) under the chart's id.
    std::exception_ptr analysis_failure;
    // The Dynamics count, or why it failed: a failed count always has its
    // message, and its details line may be empty.
    std::optional<app::DynamicsBreakdown> dynamics;
    std::string dynamics_message;  // app::plain_error
    std::string dynamics_error;    // app::plain_error_detail
};

// The song panel's job: re-identifies an edited chart, runs the engine's
// normal analysis and counts the Dynamics, all for one click. cancel()
// interrupts the search at its next progress tick; the stretch before the
// first tick (parse + graph build) can't be interrupted. The UI thread drops
// a result whose generation is not the current click's.
class ViewJob : public ResultJobBase {
public:
    // The song, its profile and the settings are snapshotted at start, so
    // the result is saved against the song and settings it ran for.
    // `analysis_off` (broken rules) skips the engine and keeps the count.
    ViewJob(store::ChartLibraryEntry song, store::Profile profile, app::AnalysisSettings settings,
            bool analysis_off, int generation);
    ~ViewJob() { shutdown(); }

    void start();

    const store::ChartLibraryEntry& song() const { return song_; }
    // The profile the click's record is saved under (Settings::profile at
    // the click).
    const store::Profile& profile() const { return profile_; }
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
    store::Profile profile_;
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
// finishes (the one opening the report runs included), and by
// AppState::request_path_report when nothing is left to analyze. It
// reads the stored records of one profile, off the render thread like every
// other job. It writes, analyzes and opens nothing.
class ReportJob : public ResultJobBase {
public:
    // `run` names the profile the report lists (ReportOptions::run, the
    // run's profile).
    ReportJob(store::RecordStore& store, app::BatchRun run);
    ~ReportJob() { shutdown(); }

    void start();

    // What the report is built from, as given to the constructor.
    const store::Profile& profile() const { return run_.profile; }

    // Valid once finished() && ok(): the report, shared with AppState's slot
    // rather than copied.
    const std::shared_ptr<const app::report::GeneratedReport>& result() const { return result_; }

private:
    void run();

    store::RecordStore& store_;
    app::BatchRun run_;
    std::function<void()> hold_;  // the test seam's, copied at construction
    std::shared_ptr<const app::report::GeneratedReport> result_;
};

// Test seam: when set, every report job built from now on calls this on its
// own thread before it reads the store, so a test can hold a build open (one
// build at a time, Cancel, an out-of-date event during a build). An empty
// hook clears it. Only tests set it.
void set_report_hold_for_test(std::function<void()> hold);

}  // namespace hydra::ui

#endif  // HYDRA_UI_LIBRARY_JOBS_H
