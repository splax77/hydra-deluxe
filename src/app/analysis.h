// Analysis orchestration: search/pather.h paths one chart, and this
// file does the rest: finding charts on disk, hashing/reading their
// metadata, and running many of them across a thread pool into a RecordStore.
//
// A std::thread pool runs charts in parallel and shares memory, so no worker
// processes or row copying across a pipe are needed.

#ifndef HYDRA_APP_ANALYSIS_H
#define HYDRA_APP_ANALYSIS_H

#include <atomic>
#include <cstdint>
#include <exception>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/model.h"
#include "parse/chart_reading.h"
#include "parse/song.h"
#include "search/pather.h"
#include "store/record_store.h"

namespace hydra::app {

// One chart file found on disk: the library entry the scan saves for it, one
// shape for both (concept audit M21), so nothing copies between them.
using ScanItem = store::ChartLibraryEntry;

// The chart's stated length and delay, read from its files through the same
// three readers the scan uses: song.ini beside a folder chart (none when
// there is no song.ini), a .sng's metadata, a .srb's metadata block. Only the
// metadata is read, never the audio. Throws when a file cannot be read.
store::ChartTimingMeta read_chart_timing_meta(const std::string& notespath);

// What the scan read for the chart at `notespath`, or, for a library row an
// older scan wrote (`scanned` empty), read_chart_timing_meta's answer. The
// open-song backfill and an analysis both ask here (SL1 open question 3).
store::ChartTimingMeta chart_timing_meta(const std::optional<store::ChartTimingMeta>& scanned,
                                         const std::string& notespath);

// Progress/cancel hooks for the extended scan. Callbacks fire on the calling
// thread only (never a worker), like run_batch's.
struct ScanCallbacks {
    // Running count of folders visited during enumeration (monotonic; the
    // same values the simple overload's cb_progress sees).
    std::function<void(int)> on_folders;
    // Chart-reading progress: fired once with done=0 when the total becomes
    // known (enumeration finished), then per chart processed. `cached` counts
    // charts satisfied from the rescan cache without touching the file.
    std::function<void(int done, int total, int cached)> on_charts;
    const std::atomic<bool>* cancel = nullptr;
};

// Recursively searches rootfolders for chart-bearing folders: a notes file
// (pick_notes_file says which) alongside a song.ini (find_song_ini), plus
// every .sng and .srb file. A chart's rootfolder is its folder's parent
// (parent_folder), relative to the root the walk reached it from.
// Re-encountered folders are skipped.
//
// The walk lists folders on a batch_worker_count() thread pool, then hands
// back the serial walk's order and decides each folder's root (walk_folders
// in analysis.cpp). Hashing/metadata reads then run on a thread pool of the
// same size. Results keep the walk's order.
// `cache` (load_scan_cache) lets a chart whose fingerprint is unchanged reuse
// its previous md5/metadata without any file I/O; an empty cache is no help,
// so every chart is read. A failing chart file is skipped with an error entry;
// its folder's other charts and subtree still scan (unlike the Python
// original, which dropped the whole folder).
std::pair<std::vector<ScanItem>, std::vector<std::string>> discover_charts(
    const std::vector<std::string>& rootfolders, const ScanCallbacks& callbacks,
    const store::ChartLibraryCache& cache);

// The same scan with no rescan cache.
std::pair<std::vector<ScanItem>, std::vector<std::string>> discover_charts(
    const std::vector<std::string>& rootfolders, const ScanCallbacks& callbacks = {});

// The chart file's hyhash: the id Clone Hero gives the song (song_id_read in
// analysis.cpp is the rule). It is the same id the
// library scan writes to the charts rows, so a tool can look a chart up in the
// record store by path alone. Returns an empty string if the file cannot be
// read or holds no id.
std::string hash_chart_file(const std::string& path);

// The fingerprint a chart's files give now, as the scan would store it in
// `sig` (ChartLibraryEntry::sig); pending_chart_of in
// analysis.cpp decides which files that covers. Empty when the file is gone or
// a folder chart has no song.ini. chart_changed_since compares against it; a
// click that finds an edited chart stores it with the new hash
// (RecordStore::reidentify_chart).
std::string chart_files_sig(const std::string& notespath);

// What a chart's files are now, when they are not what a library row stored.
struct ChartNow {
    std::string md5;  // hash_chart_file now; empty when the file cannot be read
    std::string sig;  // chart_files_sig, read once before the hash, as the scan does

    // True when the file was read and hashes to something other than
    // `stored_md5`. An unreadable file (empty md5) is never "different" here;
    // each caller decides what that means for it.
    bool differs_from(const std::string& stored_md5) const {
        return !md5.empty() && md5 != stored_md5;
    }
};

// The one answer to "has this chart changed since its library row was made,
// and what is it now?". This is the rescan's own shortcut: empty when the
// files still give `stored_sig`, so the stored md5 holds and nothing is
// hashed. A missing `stored_sig`, a gone file or a folder chart without its
// song.ini all count as changed, so the file is hashed with the scan's own
// rule and the caller asks ChartNow::differs_from the row's md5. An empty md5
// means the file is unreadable: the Preview load treats that as no change
// (the parse error speaks for it), and the click's ViewJob fails with
// "could not read <path>".
std::optional<ChartNow> chart_changed_since(const std::string& notespath,
                                            const std::string& stored_sig);

// A chart hash in the one spelling used for matching: its ASCII letters
// lowered. The scan already writes lowercase hex (see hash_chart_file), so
// this is for hashes from elsewhere, such as a leaderboard or an older row
// (audit finding 192).
std::string normalize_chart_hash(std::string_view hash);

// Saves a finished scan as the library, replacing the last one. Scan library
// and hydra_batch with no folder arguments both save through here (D79), so
// both leave the same library behind. Empty on success, else the problem line
// the scan shows.
std::optional<std::string> save_scan_as_library(store::RecordStore& store,
                                                const std::vector<ScanItem>& items);

// The first chart folder in `chartfolders` that cannot be read as a folder,
// or nothing when every one can. A library scan over a folder it cannot list
// would save a library without that folder's charts and drop their results,
// so the app's scans and hydra_batch's whole-library run ask here first and
// refuse (D266 item 4, D268). It reads the disk on every call, so callers ask
// only when a scan is asked for.
std::optional<std::string> first_missing_chart_folder(
    const std::vector<std::string>& chartfolders);

// The last saved scan, as the rescan cache discover_charts takes. A store
// that cannot be read gives an empty cache: no cache is only a slower scan,
// not a failed one.
store::ChartLibraryCache load_scan_cache(store::RecordStore& store);

// The [song] section of a song.ini as lower-cased key -> value, with the
// value's leading blanks trimmed. A key seen twice keeps its last value.
// Section and key names match in any case, and `;` and `#` start comments.
// The text is decode_chart_text's (parse/text_decode.h), so every key and
// value is valid UTF-8. Throws std::runtime_error when the file cannot be
// read. The library scan (read_metadata_ini) and the Preview (delay) both
// read song.ini through here.
std::map<std::string, std::string> read_song_ini_keys(const std::string& path);

// The settings a batch run applies uniformly. Everything the search itself
// reads lives on the SearchSettings base; the ChartReading base
// (parse/chart_reading.h) is the parse-time choices, so a function that takes
// a ChartReading takes these settings as they are.
struct AnalysisSettings : SearchSettings, ChartReading {
    const ChartReading& reading() const { return *this; }
    ChartReading& reading() { return *this; }

    // Two settings are never compared whole: ChartReading's == would look at
    // the reading alone.
    bool operator==(const AnalysisSettings&) const = delete;
    bool operator!=(const AnalysisSettings&) const = delete;
};

// Loads and analyzes one chart file (.mid/.chart/.sng/.srb), producing a record and
// the parsed song (its timing, for the Paths tab and the Preview).
struct AnalysisResult {
    HydraRecord record;
    Song song;
};
// on_progress, if set, is called from the calling thread with a monotonic 0..1
// fraction as the search sweeps the chart — for a single-chart progress bar.
// It may throw AnalysisCancelled to stop the search.
AnalysisResult analyze_chart_file(const std::string& filepath,
                                  const AnalysisSettings& settings,
                                  const std::function<void(float)>& on_progress = {});

// Thrown out of a search's progress callback to stop a cancelled analysis.
// It does not derive from std::exception, so no catch (const std::exception&)
// on the way out (the all-0 pass in search/pather.cpp has one) swallows it.
// ViewJob (the click's job), run_batch and the path report's pass all stop
// searches with it.
struct AnalysisCancelled {};

// The progress callback a run hands each search so that setting *cancel stops
// it at its next tick, by throwing AnalysisCancelled. Empty when `cancel` is
// null: with nothing to check, the search gets no callback at all. run_batch
// and the path report's pass both use it.
std::function<void(float)> stop_on_cancel(const std::atomic<bool>* cancel);

// Analyzes one chart file. analyze_chart_file is the real one; run_batch
// takes another only from a test.
using ChartAnalyzer = std::function<AnalysisResult(
    const std::string& path, const AnalysisSettings& settings,
    const std::function<void(float)>& on_progress)>;

// The song's length a click shows (store::SongLength): the owner's answer
// (app::song_length_ms, D255) for `song`, the chart at `notespath`, from what
// the scan read (chart_timing_meta over `scanned`). The length does not
// depend on how `song` was parsed. Read, with or without a length, unless
// that throws: then not read, so a failed read costs only the length, which
// stays as it was until opening the song reads it. No audio is opened.
// ViewJob (the click's job) asks here; run_batch works out no length.
store::SongLength analysis_song_length(const std::optional<store::ChartTimingMeta>& scanned,
                                       const std::string& notespath, const Song& song);

// How far a batch run has got. run_batch is the only writer of every count
// here; a reader copies them rather than counting its own callbacks. Every
// count is in library rows, each copy of a chart counted (D76), so a finished
// run's analyzed, failed and skipped add up to the rows it was given.
struct BatchProgress {
    int completed = 0;  // analyzed + failed
    int total = 0;      // rows to run: plan_batch's to-do rows
    int analyzed = 0;   // rows whose chart was stored
    int skipped = 0;    // plan_batch's skipped rows, known before the first chart
    int failed = 0;
    std::string current_title;
};

// A batch's three counts as the app's strip and hydra_batch's closing line
// both say them: "21 analyzed · 0 failed · 1 skipped (already had a result)"
// (D114 item 10). Each caller adds its own lead-in and ending.
std::string batch_counts_text(int analyzed, int failed, int skipped);

// The confirm's dim line for the rows in scope whose chart is a stored
// failure (BatchPlan::cant_analyze): "59 of these can't be analyzed. Check
// the box to try them again." The user's answer 6, 2026-10-15.
std::string batch_cant_analyze_text(int64_t rows);

// What a batch says while it lists its charts, before the first one runs. The
// strip under the toolbar and the path report window waiting on a batch both
// show it.
inline constexpr const char* kBatchPreparingText = "Preparing the chart list...";

// The default batch pool size: one core is left for the UI (or shell) and for
// whatever else the machine is doing; capped so peak memory (a discography
// chart can reach hundreds of MB) stays bounded.
int batch_worker_count();

// Everything one batch run is: the profile its records are saved and looked
// up under, and the search settings. Settings::batch_run() fills both from
// one Settings (the profile is Settings::profile()), so they cannot disagree.
// Building one by hand is for tests. A hand-built run whose profile asks for
// a cap, ms limit or fill rule its settings did not search under fails every
// chart it analyzes: store::prepare_record is that check.
struct BatchRun {
    store::Profile profile;
    AnalysisSettings settings;
};

// The charts a batch under `run` skips because they already have a result,
// as md5s, each with its status: Ready, or CantAnalyze for a stored failure
// (store::RecordStore::settled_song_ids decides what counts). Empty when
// `redo`, so a redo tries the stored failures again. One query for the whole
// library.
using SettledCharts = std::unordered_map<std::string, store::RecordStatus>;
SettledCharts charts_with_result(store::RecordStore& store, const BatchRun& run, bool redo);

// The record a failed analysis of `song_id` under `profile` is stored as, or
// nothing when the failure is not one the store remembers
// (remembered_failure in core/error_kind.h). The one place a failure's kind
// is read off the exception for storing; store::failure_record builds the
// record. run_batch and the click both store through it.
std::optional<store::PreparedRecord> prepare_failed_chart(std::string song_id,
                                                          const store::Profile& profile,
                                                          const std::exception& e);

// What a batch over a scan list will do. A chart the scan found in several
// folders (one md5) is analyzed once (D51 call 10), but every count is in
// scan rows, each copy counted, the way the library counts (D76).
struct BatchPlan {
    // The charts to run, in input order, each once as its first copy.
    std::vector<ScanItem> todo;
    // How many scan rows todo[i]'s chart has: its first copy and every later
    // one. Analyzing the chart once settles all of them.
    std::vector<int> rows;
    // Scan rows left out because `already` holds their chart, every copy
    // counted.
    int skipped = 0;
    // The part of `skipped` whose chart is a stored failure (CantAnalyze),
    // every copy counted. The confirm's line names it
    // (batch_cant_analyze_text).
    int cant_analyze = 0;

    // The scan rows todo stands for.
    int todo_rows() const;
};

// The one place a scan list becomes a batch's to-do list and skipped count.
// `already` is charts_with_result's answer. A batch makes its plan once,
// before it starts, and runs that plan (D79): the GUI's confirm shows the
// plan the run then takes.
BatchPlan plan_batch(const std::vector<ScanItem>& items, const SettledCharts& already);

// Progress, result and cancel hooks for run_batch. The three callbacks fire
// on the calling thread only (never a worker), so they may touch UI state.
struct BatchCallbacks {
    // Fires once per library row, after that row is counted and before the
    // row's own on_error or on_result, so a caller numbering its lines reads
    // the number here.
    std::function<void(const BatchProgress&)> on_progress;
    // A chart that failed, in its analysis or its save: the sentence
    // app::plain_error gives for the exception, and its details line
    // (app::plain_error_detail, empty when the sentence already quotes it).
    std::function<void(const std::string& title, const std::string& sentence,
                       const std::string& error)>
        on_error;
    // Fires after the record is written to the store, with the record it
    // wrote (the batch CLI prints score and best path off its rank-1 path).
    std::function<void(const ScanItem&, const store::PreparedRecord&)> on_result;
    // Setting *cancel stops the run. No new chart starts, and a running search
    // stops at its next progress tick. A stopped chart is neither a result nor
    // a failure, and nothing more is written once the cancel is seen.
    const std::atomic<bool>* cancel = nullptr;
    // What analyzes one chart. Empty means analyze_chart_file.
    ChartAnalyzer analyze;
};

// Runs the analysis + store::prepare_record for every chart in `plan` on a
// batch_worker_count()-sized pool (app/work_pool.h), writing results into
// `store` from the calling thread only. Which charts run and which are
// skipped is the plan's answer (plan_batch); the run reads nothing from the
// store before its first chart. Every BatchProgress count comes from here,
// all in one callback. A chart whose failure the store remembers
// (prepare_failed_chart) has that failure saved like a result, in the same
// save group, and is still counted and reported as failed in this run.
void run_batch(const BatchPlan& plan, const BatchRun& run, store::RecordStore& store,
               int worker_count, const BatchCallbacks& callbacks = {});

}  // namespace hydra::app

#endif  // HYDRA_APP_ANALYSIS_H
