// The path report: row collection, tiles and the subtitle and footer framing.
// The GUI's ReportJob builds it and the path report window draws it.

#ifndef HYDRA_APP_REPORT_H
#define HYDRA_APP_REPORT_H

#include <atomic>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "core/error_kind.h"
#include "store/chart_index.h"
#include "store/record_store.h"
#include "core/squeeze_rating.h"

namespace hydra::app {
struct Settings;  // app/config.h
}

namespace hydra::app::report {

// One table row: one stored path of one chart's record under the page's
// profile.
struct ReportRow {
    std::string song;
    std::string artist;
    std::string charter;
    // The path's place in its record, best score first (store::PathRow::rank).
    int rank = 1;
    // Is the path optimal? The stored flag (store::PathRow::optimal), so every
    // path tied at the top score is, as on the Paths tab (D48 Q1). The page's
    // "Optimal paths only" box reads this, not the rank.
    bool optimal = false;
    std::string path;
    int64_t score = 0;
    int acts = 0;
    int skip = 0;
    std::optional<double> ms;
    std::string tier;
    std::string tok;
    // The path's hardest early fill, as the record stored it
    // (store::PathRow::hardest_efill). Unset when the path has none.
    std::optional<double> efill;
    double mult = 0.0;
    int sqin = 0;
    int sqout = 0;
    int notes = 0;
    // The chart this row belongs to (its song id). Every chart count on the
    // page asks store::ChartIndex::count over these ids.
    std::string hyhash;
};

// One tile above a report's table: what it counts and the text it shows.
// path_tiles and dm_report::dm_tiles build them.
struct Tile {
    std::string label;
    std::string value;
};

// What a tile or cell shows where a value is missing: the pages' DASH.
inline constexpr const char* kDash = "—";

// The colour a report chip is drawn in. Each member is named after the pages'
// CSS variable for it: the timing tiers' --t0 to --t5 and --tn, and the dim
// --muted. A path row's tier carries its token's name as ReportRow::tok, and
// dm_report::status_token gives a comparison status's. The theme turns a
// token into a colour.
enum class ChipToken { t0, t1, t2, t3, t4, t5, tn, muted };

// The token a path row's Timing chip is drawn in, read from the token name
// tier_for gave the row (ReportRow::tok). Throws for a name tier_for never
// gives.
inline ChipToken tier_token(const std::string& tok) {
    static constexpr std::pair<const char*, ChipToken> kTokens[] = {
        {"t0", ChipToken::t0}, {"t1", ChipToken::t1}, {"t2", ChipToken::t2},
        {"t3", ChipToken::t3}, {"t4", ChipToken::t4}, {"t5", ChipToken::t5},
        {"tn", ChipToken::tn},
    };
    for (const auto& [name, token] : kTokens)
        if (tok == name) return token;
    throw std::invalid_argument("no chip token named \"" + tok + "\"");
}

// The path report's five tiles over the rows a window shows: `shown` holds
// their indices into `rows`. `hit_window_ms` is the window the rows' tiers
// were labeled with (GeneratedReport::hit_window_ms), which names the Beyond
// edge in the "Past N ms" tile. `library` counts the Charts shown tile
// (GeneratedReport::library), which reads "<shown> of <library>": the charts
// with rows shown, of every chart the library lists, both counted in library
// rows. Takes over the page script's `stats`.
std::vector<Tile> path_tiles(const std::vector<ReportRow>& rows,
                             const std::vector<size_t>& shown, double hit_window_ms,
                             const store::ChartIndex& library);

// The timing_tiers table's two open bands, the one answer to which entries
// they are: Beyond, past the last cutoff, and None, a path with no squeeze.
// tier_for labels rows with them, path_tiles counts the Beyond rows and the
// path report view names both.
const TimingTier& beyond_tier(const std::vector<TimingTier>& tiers);
const TimingTier& none_tier(const std::vector<TimingTier>& tiers);

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

// The Beyond edge (beyond_edge_ms) as every report prints it, in whole ms:
// the page's payload, its footer, the Past tile and the Timing chip.
std::string beyond_edge_text(double hit_window_ms);

// One library chart whose stored result under the page's profile is a
// failure (RecordStatus::CantAnalyze), named from the library.
struct CantAnalyzeChart {
    std::string song_id;
    // The one cleaned title and artist every screen shows (display_title,
    // display_artist), from the chart's naming copy.
    std::string title;
    std::string artist;
    // The stored failure (RecordLookup::failure_kind, failure_text).
    ErrorKind kind = ErrorKind::NoNotes;
    std::string text;
};

// Every chart the library lists, sorted by its result under one profile:
// one RecordStore::lookup over ChartIndex::song_ids.
struct LibraryLookup {
    // The Ready records, by song id.
    std::unordered_map<std::string, store::RecordRow> ready;
    // The CantAnalyze charts, in the library's scan order.
    std::vector<CantAnalyzeChart> cant_analyze;
    // The charts with no result (NotAnalyzed or Stale), in scan order.
    std::vector<std::string> no_result;
};

// Which charts a page covers: every chart `library` lists, sorted three ways
// by its result under `profile`. The store also holds results the library
// doesn't list, and no page shows one (D87 item 4, D92). The path report and
// both comparison pages read their records through here, so the three pages
// agree on which charts are covered.
LibraryLookup library_lookup(store::RecordStore& store, const store::Profile& profile,
                             const store::ChartIndex& library);

// What "--all-paths" asks for, a count no chart reaches. It is the one answer
// to "does this page list every path" (finding 86): generate_report's subtitle
// reads it, so any smaller --paths is written as a count.
inline constexpr int64_t kEveryPathSentinel = 1000000000;

struct ReportOptions {
    // Defaults to every path a record keeps.
    int64_t max_paths = store::kPathsPerRecord;
    double hit_window_ms = kDefaultHitWindowMs;
    std::string db_path;  // names the footer's source database
    // Set this and generate_report hands back an empty result, with no rows.
    // Closing the app while a report builds goes through here; the CLI never
    // sets it.
    const std::atomic<bool>* cancel = nullptr;
    // The profile the page lists: the BatchRun's profile, as the settings that
    // filed the results gave it. Required: the report throws without one.
    std::optional<BatchRun> run;
};

// What collect_rows found: the page's rows, the library they were read
// against, and the library's charts that gave no rows because they have no
// Ready record (library_lookup's other two groups).
struct CollectedRows {
    std::vector<ReportRow> rows;
    store::ChartIndex library;
    std::vector<CantAnalyzeChart> cant_analyze;
    std::vector<std::string> no_result;
};

// The page's rows, read from the store: every Ready record of options.run's
// profile whose chart the library lists (library_lookup), in the library's
// scan order, each giving up to options.max_paths rows in its stored rank
// order. Nothing is analyzed and no chart file is opened. hit_window_ms feeds
// the tier labels only.
CollectedRows collect_rows(store::RecordStore& store, const ReportOptions& options);

// ---- generate_report -------------------------------------------------------
// The whole report in one call: counts + rows + the standard framing
// (subtitle and footer). The GUI's ReportJob is an adapter over this seam.

// Why a report came out with no page (finding 105, D48 Q28).
enum class EmptyReason {
    None,                  // there is a page
    NothingStored,         // the database holds no results at all
    NoLibrary,             // it holds results, but no chart library to report on
    NothingUnderSettings,  // it holds results, but none Ready at this cap and fill rule
    NothingToShow,         // the library's charts all can't be analyzed or have no result
    Cancelled,             // the caller stopped the walk
};

// What an empty report says when no library chart has a row but some have a
// stored failure (EmptyReason::NothingToShow). The coverage lines above it
// say why (GeneratedReport::cant_analyze_line, no_result_line).
inline constexpr const char* kNothingToShow = "Nothing to show";

// What an empty report says when the database has results but no chart
// library (D89 item 2): the report covers library charts only. The DM
// comparison and the fill comparison stop with it too (D92).
inline constexpr const char* kNoChartLibrary =
    "This database has no chart library. Run hydra_batch without folder arguments, or scan "
    "in Hydra, to build one.";

// Whether `store` holds results but no chart library to report them on: the
// one answer every page asks before it says kNoChartLibrary (D89 item 2,
// D92). A database with no results at all keeps each page's own words.
bool lacks_chart_library(store::RecordStore& store);

// The start of the sentence an empty report gives when the database holds
// results under other settings (nothing_under_settings builds the whole
// one). The app shows it through GeneratedReport::why_empty, as
// ReportJob::run does for every empty page (D97).
inline constexpr const char* kNothingUnderSettings = "Nothing is analyzed under these settings";

// The whole sentence an empty page gives, built once for every page that
// lists results (finding 105, D50 item 3): "Nothing is analyzed under these
// settings (<chartmode>, SP cap <cap>, <fills>). Analyze with these settings,
// or change them." `chartmode` is the stored chart mode as every page
// shows it (Profile::chartmode, spelled by app::chartmode_key). The path report
// passes its fill rule as `fills`; the fill comparison passes no fills (it
// compares both rules).
std::string nothing_under_settings(const std::string& chartmode, int cap,
                                   const std::string& fills = std::string());

// One group of the coverage line's Show list: the can't-analyze charts that
// share one reason.
struct CantAnalyzeGroup {
    // The reason as the user reads it: app::plain_error's sentence for the
    // stored kind and text (the group heading).
    std::string sentence;
    // The group's charts in library rows (store::ChartIndex::count).
    int64_t count = 0;
    // One per chart, in the library's scan order; the list shows each as
    // Title · Artist.
    std::vector<CantAnalyzeChart> charts;
};

struct GeneratedReport {
    // The charts with rows on the page, every library copy counted
    // (store::ChartIndex::count, D76, D77).
    int64_t songs = 0;
    int64_t rows = 0;
    // Every chart the library lists, in library rows (ChartIndex::count):
    // the "of" half of the Charts shown tile. With no filters,
    // songs + cant_analyze_charts + no_result_charts == library_charts.
    int64_t library_charts = 0;
    // The library's charts whose stored result is a failure, in library rows,
    // and the Show list's groups, in the order each reason first appears in
    // the library.
    int64_t cant_analyze_charts = 0;
    std::vector<CantAnalyzeGroup> cant_analyze;
    // The library's charts with no result under the page's profile (Not
    // analyzed or Stale), in library rows.
    int64_t no_result_charts = 0;
    // The coverage lines above the table, as the window shows them. Each is
    // empty when its count is 0. cant_analyze_line names the reason when every
    // can't-analyze chart shares one that has a short form ("59 charts have
    // no Expert Pro Drums notes"), else "59 charts can't be analyzed";
    // no_result_line reads "3 charts have no result under these settings".
    std::string cant_analyze_line;
    std::string no_result_line;
    EmptyReason empty_reason = EmptyReason::None;
    // For NothingUnderSettings, the sentence that names the settings
    // (nothing_under_settings builds it). For NoLibrary, kNoChartLibrary.
    // For NothingToShow, kNothingToShow. Empty otherwise: an empty database
    // and a cancel keep each caller's own words.
    std::string why_empty;
    // The rows themselves, one per path, in collect_rows' order. Empty when
    // there is no page.
    std::vector<ReportRow> paths;
    // The library the rows were read against. The window recounts its
    // Charts shown tile through it (path_tiles).
    store::ChartIndex library;
    // The window the rows' tiers were labeled with (ReportOptions::hit_window_ms).
    double hit_window_ms = kDefaultHitWindowMs;
    // The line under the page's heading and the note at its foot, as the page
    // shows them. Empty when there is no page.
    std::string subtitle;
    std::string footer;

    // The page's coverage (plan 2026-10-15, question 5). T5 added these
    // names so the window compiles before T4's report change lands; T4 owns
    // them and fills them, and the merge keeps T4's version.
    struct CantAnalyzeChart {
        std::string song_id;
        std::string song;
        std::string artist;
        ErrorKind kind = ErrorKind::ChartUnreadable;
        std::string text;
    };
    struct CantAnalyzeGroup {
        std::string heading;
        std::vector<CantAnalyzeChart> rows;
    };
    int64_t library_charts = 0;
    std::vector<CantAnalyzeChart> cant_analyze;
    int64_t no_result_charts = 0;
    // The two lines above the table, empty when there is nothing to say.
    std::string coverage_line;
    std::string no_result_line;
    // The Show list's charts, grouped by reason.
    std::vector<CantAnalyzeGroup> cant_analyze_groups;
};

// The library's charts through collect_rows.
GeneratedReport generate_report(store::RecordStore& store, const ReportOptions& options);

// The user settings generate_report reads, stated once. A new read of a
// setting by the page joins this list, and settings_change_touches compares
// through it. The profile holds the chart mode, so a change of difficulty,
// Pro Drums, 2x Bass or Note Shuffle puts an open page out of date. The rules
// are not here: hydra_rules.ini is read once at start-up (AppState), so no
// open page outlives a change to them.
struct PageSettings {
    store::Profile profile;
    double hit_window_ms = kDefaultHitWindowMs;

    static PageSettings of(const Settings& settings);
    bool operator==(const PageSettings& other) const {
        return profile == other.profile && hit_window_ms == other.hit_window_ms;
    }
    bool operator!=(const PageSettings& other) const { return !(*this == other); }
};

// Whether moving from `before` to `after` changes a setting the path report
// reads (PageSettings), so a report built under `before` is out of date
// (D103 item 22).
bool settings_change_touches(const Settings& before, const Settings& after);

}  // namespace hydra::app::report

#endif  // HYDRA_APP_REPORT_H
