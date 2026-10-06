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
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "core/model.h"
#include "parse/song.h"
#include "search/pather.h"
#include "store/record_store.h"

namespace hydra::app {

// One chart file found on disk, with enough metadata to register it in the
// store. `sig` fingerprints the source files
// (sizes + mtimes) so a later rescan can skip re-hashing unchanged charts;
// it never leaves the charts table and is not part of record identity.
struct ScanItem {
    std::string md5;
    std::string title;
    std::string artist;
    std::string charter;
    std::string notespath;
    std::string rootfolder;
    std::string sig;
    // The chart's stated length and delay (D75). A scan always sets it; it is
    // empty only on an item made from a library row an older scan wrote
    // (ChartLibraryEntry::timing), and chart_timing_meta fills that in.
    std::optional<store::ChartTimingMeta> timing;
};

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
// (parent_folder), relative to the root it was found under.
// Re-encountered folders are skipped.
//
// The walk itself is a serial single pass; hashing/metadata reads run on a
// batch_worker_count() thread pool. Results keep the serial walk's order.
// `cache` (from RecordStore::chart_library_cache), if given, lets a chart
// whose fingerprint is unchanged reuse its previous md5/metadata
// without any file I/O. A failing chart file is skipped with an error entry;
// its folder's other charts and subtree still scan (unlike the Python
// original, which dropped the whole folder).
std::pair<std::vector<ScanItem>, std::vector<std::string>> discover_charts(
    const std::vector<std::string>& rootfolders, const ScanCallbacks& callbacks,
    const store::ChartLibraryCache* cache = nullptr);

// Compatibility form: folder progress only, no cache, no cancel.
std::pair<std::vector<ScanItem>, std::vector<std::string>> discover_charts(
    const std::vector<std::string>& rootfolders,
    const std::function<void(int)>& cb_progress = nullptr);

// The chart file's hyhash: the same MD5 the library scan writes to the
// songmeta/charts rows, so a tool can look a chart up in the record store
// by path alone. Returns an empty string if the file cannot be read.
std::string hash_chart_file(const std::string& path);

// Whether a chart's files still give the fingerprint the scan stored in
// `sig` (ScanItem::sig, ChartLibraryEntry::sig); pending_chart_of in
// analysis.cpp decides which files that covers. This is the
// rescan's own shortcut: when it says yes, the stored md5 still holds and the
// file need not be hashed again. False when `sig` is empty, the file is gone,
// or a folder chart has lost its song.ini, so the caller hashes.
bool chart_files_unchanged(const std::string& notespath, const std::string& sig);

// A chart hash in the one spelling used for matching: its ASCII letters
// lowered. The scan already writes lowercase hex (see hash_chart_file), so
// this is for hashes from elsewhere, such as a leaderboard or an older row
// (audit finding 192).
std::string normalize_chart_hash(std::string_view hash);

// The library entry a scan row becomes. The two types hold the same seven
// strings; this copies each by name, so a reordered field cannot slip through
// a positional copy (audit finding 256).
store::ChartLibraryEntry to_library_entry(const ScanItem& item);

// Saves a finished scan as the library, replacing the last one. Scan library
// and hydra_batch with no folder arguments both save through here (D79), so
// both leave the same library behind. Empty on success, else the problem line
// the scan shows.
std::optional<std::string> save_scan_as_library(store::RecordStore& store,
                                                const std::vector<ScanItem>& items);

// The [song] section of a song.ini as lower-cased key -> value, with the
// value's leading blanks trimmed. A key seen twice keeps its last value.
// Section and key names match in any case, `;` and `#` start comments, and a
// UTF-8 BOM is skipped. Throws std::runtime_error when the file cannot be
// read. The library scan (name, artist, charter) and the Preview (delay) both
// read song.ini through here.
std::map<std::string, std::string> read_song_ini_keys(const std::string& path);

// The settings a batch run applies uniformly. Everything the search itself reads
// lives on the SearchSettings base; the two flags here are parse-time only.
struct AnalysisSettings : SearchSettings {
    bool prodrums = true;
    bool bass2x = true;
    // Which charted difficulty to read. Expert by default, so every existing
    // caller keeps the behavior it had.
    Difficulty difficulty = Difficulty::Expert;
};

// Loads and analyzes one chart file (.mid/.chart/.sng/.srb), producing a record and
// the song's timing (for the store's songmeta row).
struct AnalysisResult {
    HydraRecord record;
    Song song;  // carries tick_resolution/tpm_changes/bpm_changes for add_song
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
// The single-chart Analyze job and run_batch both stop searches with it.
struct AnalysisCancelled {};

// Analyzes one chart file. analyze_chart_file is the real one; run_batch
// takes another only from a test.
using ChartAnalyzer = std::function<AnalysisResult(
    const std::string& path, const AnalysisSettings& settings,
    const std::function<void(float)>& on_progress)>;

// The song's length an analysis saves on the side, as the store saves it: the
// owner's answer (app::chart_song_length_ms, D75) for `song`, the chart at
// `notespath` parsed under `settings`, from what the scan read
// (chart_timing_meta over `scanned`). Read, with or without a length, unless
// that throws: then not read, so a failed read costs only the length, which
// stays as it was until opening the song reads it. No audio is opened.
// run_batch and the single-chart Analyze job both save through here.
// SongLengthJob calls the owner itself: its read is the whole job, and
// AppState::update_song_length decides what a failed job leaves.
store::SongLength analysis_song_length(const std::optional<store::ChartTimingMeta>& scanned,
                                       const std::string& notespath, const Song& song,
                                       const AnalysisSettings& settings);

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

// The default batch pool size: one core is left for the UI (or shell) and for
// whatever else the machine is doing; capped so peak memory (a discography
// chart can reach hundreds of MB) stays bounded.
int batch_worker_count();

// Everything one batch run is: the search settings, and the chart mode and
// lens its results are filed under. Settings::batch_run() fills all three
// from one Settings, so they cannot disagree. Building one by hand is for
// tests.
struct BatchRun {
    std::string chartmode;
    store::Lens lens;
    AnalysisSettings settings;

    // The SP cap this run's results are filed under and looked up by:
    // CapQuery::at is the owner.
    store::CapQuery cap_query() const { return store::CapQuery::at(settings.sp_cap); }
};

// The charts a batch under `run` skips because they already have a result
// (store::RecordStore::analyzed_hashes decides what counts), as md5s. Empty
// when `redo`. One query for the whole library.
std::unordered_set<std::string> charts_with_result(store::RecordStore& store,
                                                   const BatchRun& run, bool redo);

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

    // The scan rows todo stands for.
    int todo_rows() const;
};

// The one place a scan list becomes a batch's to-do list and skipped count.
// `already` is charts_with_result's answer. A batch makes its plan once,
// before it starts, and runs that plan (D79): the GUI's confirm shows the
// plan the run then takes.
BatchPlan plan_batch(const std::vector<ScanItem>& items,
                     const std::unordered_set<std::string>& already);

// Progress, result and cancel hooks for run_batch. The three callbacks fire
// on the calling thread only (never a worker), so they may touch UI state.
struct BatchCallbacks {
    // Fires once per library row, after that row is counted and before the
    // row's own on_error or on_result, so a caller numbering its lines reads
    // the number here.
    std::function<void(const BatchProgress&)> on_progress;
    // A chart that failed, in its analysis or its save: the sentence
    // app::plain_error gives for the exception, and its raw text.
    std::function<void(const std::string& title, const std::string& sentence,
                       const std::string& error)>
        on_error;
    // Fires after the row is written to the store, with the row it wrote (the
    // batch CLI prints score and best path from it).
    std::function<void(const ScanItem&, const store::PreparedRow&)> on_result;
    // Setting *cancel stops the run. No new chart starts, and a running search
    // stops at its next progress tick. A stopped chart is neither a result nor
    // a failure, and nothing more is written once the cancel is seen.
    const std::atomic<bool>* cancel = nullptr;
    // What analyzes one chart. Empty means analyze_chart_file. Its song's
    // length is saved too (analysis_song_length).
    ChartAnalyzer analyze;
};

// Runs the analysis + store::prepare_row for every chart in `plan` on a
// batch_worker_count()-sized pool (app/work_pool.h), writing results into
// `store` from the calling thread only. Which charts run and which are
// skipped is the plan's answer (plan_batch); the run reads nothing from the
// store before its first chart. Every BatchProgress count comes from here,
// all in one callback.
void run_batch(const BatchPlan& plan, const BatchRun& run, store::RecordStore& store,
               int worker_count, const BatchCallbacks& callbacks = {});

}  // namespace hydra::app

#endif  // HYDRA_APP_ANALYSIS_H
