// Sortable HTML path report: row collection and page building. Split from
// the CLI's main() so the GUI's ReportJob and the tests
// (tests/test_report.cpp) can build the page without a process spawn.

#ifndef HYDRA_APP_REPORT_H
#define HYDRA_APP_REPORT_H

#include <atomic>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "store/record_store.h"
#include "core/squeeze_rating.h"

namespace hydra::app::report {

// One table row. Field order is the JSON key order the page's script reads.
// `hyhash` goes out as "c", a small per-chart number in order of first
// appearance, and `copies` as "k"; the Charts tile adds up "k" once per "c".
struct ReportRow {
    std::string song;
    std::string artist;
    std::string charter;
    std::string mode;
    // The path's place in its record's list, best score first. The subtitle
    // counts one record per rank-1 row.
    int rank = 1;
    // Is the path optimal? Read from HydraRecord::is_optimal, so every path
    // tied at the top score is, as on the Paths tab (D48 Q1). The page's "Best
    // path only" box and its bold rows read this, not the rank.
    bool optimal = false;
    std::string path;
    int64_t score = 0;
    int acts = 0;
    int skip = 0;
    std::optional<double> ms;
    std::string tier;
    std::string tok;
    // Hardest early fill among the path's E0 activations, in the same
    // difficulty convention as `ms` (e_difficulty = -e_offset: positive means
    // you must hit that much early, negative is slack). Skipped E activations
    // don't count — their fill is irrelevant to the path. Unset when the path
    // has no E0 fill (distinct from a real 0.0ms fill).
    std::optional<double> efill;
    double mult = 0.0;  // already round(x, 3)'d, like the Python row
    int sqin = 0;
    int sqout = 0;
    int notes = 0;
    // The chart this row belongs to. page_charts (report.cpp) is the one place
    // that groups rows by it: the page's "c" number and the subtitle's count.
    std::string hyhash;
    // How many library rows that chart counts as, every copy counted (D76):
    // what the subtitle and the Charts tile add up. collect_rows sets it from
    // store::RecordStore::copies_of, which also answers for a chart the
    // library doesn't list (D77).
    int copies = 0;
};

// (label, token) for a hardest-squeeze value (raw ms), e.g. (Extreme, t2).
// nullopt -> (None, tn). Bands derive from the two-hit budget
// (nominal_budget_ms): up to and including kDifficultMs is Normal (an absolute
// floor), then quarters of the budget, and Beyond past the whole budget. A
// timing exactly on an edge belongs to the band below it (D48 Q3), so 2.0 ms is
// Normal and the budget itself is Insane+. At the historical W = 70 this is the
// original 2/35/70/105/140 ladder.
std::pair<std::string, std::string> tier_for(const std::optional<double>& ms,
                                             double hit_window_ms = kDefaultHitWindowMs);

// The same label, read from an already-built timing_tiers table, so a caller
// labeling many rows builds the table once.
std::pair<std::string, std::string> tier_for(const std::optional<double>& ms,
                                             const std::vector<TimingTier>& tiers);

// Every listed record for one chart mode, cap and lens, keyed by its chart
// hash in lower case. Both comparison pages join on this.
std::unordered_map<std::string, store::RecordListing> records_by_hash(
    store::RecordStore& store, const std::string& chartmode, const store::CapQuery& cap,
    const store::Lens& lens);

// Every chart the store's library lists, with its copies
// (RecordStore::library_copies), keyed like records_by_hash so a page looks
// up both with one key. Every page reads its library copies and its "is this
// chart in the library" answer from here; a chart's count goes through
// RecordStore::copies_of.
std::unordered_map<std::string, int> library_copies_by_hash(store::RecordStore& store);

// The path report's default: the top 5 paths per chart.
inline constexpr int64_t kDefaultReportPaths = 5;
// What "--all-paths" asks for, a count no chart reaches. It is the one answer
// to "does this page list every path" (finding 86): generate_report's subtitle
// reads it, so any smaller --paths is written as a count.
inline constexpr int64_t kEveryPathSentinel = 1000000000;

// One chart's rows as its record gives them, best score first, up to
// max_paths. Every field the record answers is set; the chart's names, mode,
// hash, copies and tier label are left for collect_rows, which knows them.
// The batch's seed and the report's own pass both build rows here.
std::vector<ReportRow> chart_rows(const HydraRecord& record, int64_t max_paths);

// The rows a batch just worked out, so the report that follows it does not
// analyze those charts again (D87 item 5). run_batch fills it through
// BatchCallbacks::report_seed, once per chart it saved.
struct ReportSeed {
    // What the batch filed its results under. A report under other settings
    // cannot use these rows, so collect_rows throws rather than list them.
    std::string chartmode;
    store::CapQuery cap;
    store::Lens lens;
    // How many rows each chart keeps. A report asking for more throws.
    int64_t max_paths = kDefaultReportPaths;
    // chart_rows for each saved chart, by normalize_chart_hash of its md5.
    std::unordered_map<std::string, std::vector<ReportRow>> rows;

    // An empty seed for `run`'s results.
    static ReportSeed for_run(const BatchRun& run, int64_t max_paths = kDefaultReportPaths);
};

struct ReportOptions {
    int64_t max_paths = kDefaultReportPaths;
    // Which records the page lists: the user's current SP cap and lens.
    store::CapQuery cap = store::CapQuery::at(kCloneHeroSpCap);
    store::Lens lens;
    double hit_window_ms = kDefaultHitWindowMs;
    std::string db_path;  // names the footer's source database
    // Set this and the pass stops between charts and generate_report hands
    // back an empty result -- no rows, no html. Closing the app while a report
    // builds goes through here; the CLI never sets it.
    const std::atomic<bool>* cancel = nullptr;
    // The settings the page's paths are analyzed under: Settings::batch_run()
    // of the settings that filed the results. A chart mode other than the
    // run's own is analyzed under the run's search settings with that mode's
    // parse choices. Required: the report throws without one, and when its cap
    // or lens is not `cap` and `lens`, rather than list another run's paths.
    std::optional<BatchRun> run;
    // What analyzes one chart. Empty means analyze_chart_file; a test passes
    // one to count the calls.
    ChartAnalyzer analyze;
};

// A chart file the pass could not load or analyze. Its chart is left off the
// page (D89 item 1).
struct ReportFailure {
    std::string notespath;
    std::string error;
};

// What collect_rows found.
struct CollectedRows {
    std::vector<ReportRow> rows;
    // One entry per file, however many chart modes failed on it.
    std::vector<ReportFailure> failures;
};

// The page's rows from a fresh analysis (D87 item 5). The charts are the ones
// the library lists that the store holds a Ready row for at options.cap and
// options.lens, in every chart mode; list_records says which, and names
// them. Each is analyzed from the file RecordStore::naming_copy_paths gives.
// A result with no library row is left out (D87 item 4). Each chart
// and mode gives up to options.max_paths rows, best score first. A chart the
// seed holds is not analyzed again. The rest run on batch_worker_count()
// threads; options.cancel stops the pass between charts, and the caller
// throws the half-built result away. hit_window_ms feeds the tier labels only.
CollectedRows collect_rows(store::RecordStore& store, const ReportSeed& seed,
                           const ReportOptions& options);

// The line the page shows under its subtitle, and hydra_report prints, when
// charts were left out (D89 item 1). Empty when none were.
std::string left_out_line(const std::vector<ReportFailure>& failures);

// The self-contained page: the PAGE template with subtitle/footer escaped in
// and a JSON payload {hit_window, tiers, rows} embedded, so the page's tier
// dropdown and stats derive from the same window the rows were labeled with.
// `failures` are the charts the page says it left out, under its subtitle.
std::string build_html(const std::vector<ReportRow>& rows, const std::string& subtitle,
                       const std::string& footer,
                       double hit_window_ms = kDefaultHitWindowMs,
                       const std::vector<ReportFailure>& failures = {});

// ---- generate_report -------------------------------------------------------
// The whole report in one call: counts + rows + the standard page framing
// (subtitle and footer). The GUI's ReportJob and the hydra_report CLI are
// adapters over this seam; both previously composed the same framing strings
// by hand, where they could (and did) drift.

// Why a report came out with no page (finding 105, D48 Q28).
enum class EmptyReason {
    None,                  // there is a page
    NothingStored,         // the database holds no results at all
    NoLibrary,             // it holds results, but no chart library to report on
    NothingUnderSettings,  // it holds results, but none Ready at this cap and fill rule
    Cancelled,             // the caller stopped the walk
};

// What an empty report says when the database has results but no chart
// library (D89 item 2): the report covers library charts only.
inline constexpr const char* kNoChartLibrary =
    "This database has no chart library. Run hydra_batch without folder arguments, or scan "
    "in Hydra, to build one.";

// The start of the sentence an empty report gives when the database holds
// results under other settings. The app's error mapping knows the sentence
// by it and shows it as it is.
inline constexpr const char* kNothingUnderSettings = "Nothing is analyzed under these settings";

// The whole sentence an empty page gives, built once for every page that
// lists results (finding 105, D50 item 3): "Nothing is analyzed under these
// settings (SP cap <cap>, <middle>)<ending>. Analyze with these settings, or
// change them." The path report passes its fill rule as the middle words and
// no ending; the fill comparison passes its chart mode and " in either
// database".
std::string nothing_under_settings(int cap, const std::string& middle,
                                   const std::string& ending = std::string());

struct GeneratedReport {
    std::string html;  // empty when the store held no reportable rows
    // Both count every library copy of a chart (D76, D77).
    int64_t songs = 0;    // charts with rows on the page
    int64_t records = 0;  // records with rows on the page (one rank-1 row each)
    int64_t rows = 0;
    EmptyReason empty_reason = EmptyReason::None;
    // For NothingUnderSettings, the sentence that names the settings: "Nothing
    // is analyzed under these settings (SP cap 8, Clone Hero 1.1 fills).
    // Analyze with these settings, or change them." For NoLibrary,
    // kNoChartLibrary. Empty otherwise: an empty database and a cancel keep
    // each caller's own words.
    std::string why_empty;
    // collect_rows' failures: charts left off the page because their file
    // failed to load or analyze. The page names them (left_out_line).
    std::vector<ReportFailure> failures;
};

// The library's charts through collect_rows, with `seed`'s rows reused.
// hydra_report passes an empty seed.
GeneratedReport generate_report(store::RecordStore& store, const ReportOptions& options,
                                const ReportSeed& seed = ReportSeed{});

}  // namespace hydra::app::report

#endif  // HYDRA_APP_REPORT_H
