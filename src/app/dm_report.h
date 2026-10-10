// Comparison report: one dmleaderboards user's actual scores against Hydra's
// computed optimal for the same charts. It joins the fetched scores to stored
// records by Clone Hero's song id (leaderboard `identifier` == Hydra `hyhash`,
// whose rule is song_id_read's, in app/analysis.cpp); the
// comparison window draws the rows.
//
// "Above optimal" is expected, not an error: Hydra's optimal intentionally
// excludes several score backends, and many leaderboard scores were set on an
// older Clone Hero version whose fill-spawning let players reach totals that
// are impossible in the current game. Such rows are labelled, not flagged.

#ifndef HYDRA_APP_DM_REPORT_H
#define HYDRA_APP_DM_REPORT_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "app/report.h"  // Tile, ChipToken
#include "net/dmbot_client.h"
#include "parse/song.h"  // Difficulty
#include "store/record_store.h"

namespace hydra::app {
struct Settings;  // app/config.h
}

namespace hydra::app::dm_report {

// The words collect_dm_rows writes in DmReportRow::status, one per state a
// score can be in. Every reader compares against these names, never the words.
// Hydra has a result, and the score is below, equal to or over its optimal.
inline constexpr const char* kStatusUnderOptimal = "under optimal";
inline constexpr const char* kStatusAtOptimal = "at optimal";
inline constexpr const char* kStatusAboveOptimal = "above optimal";
// Hydra has a result, but its analysis kept no path (D51 call 11).
inline constexpr const char* kStatusNoPaths = "no paths";
// The last scan found the chart, but it has no current result at Clone Hero's
// cap, kCloneHeroSpCap, for this mode.
inline constexpr const char* kStatusNotAnalyzed = "not analyzed";
// The last scan never found the chart.
inline constexpr const char* kStatusNotInLibrary = "not in library";
// Played at a speed other than net::kBaseSpeedPercent: shown, never compared.
inline constexpr const char* kStatusOtherSpeed = "other speed";

// The decimals every comparison percent is written to: DmReportRow::pct_h,
// the % of opt cells on the page and in the window, and the average tile.
inline constexpr int kPercentDecimals = 2;

// One table row: a single leaderboard score plus the Hydra record it joins to.
struct DmReportRow {
    std::string song;
    std::string artist;
    std::string charter;
    std::string identifier;             // Clone Hero's song id (the join key)
    int64_t actual = 0;                 // score the player posted
    std::optional<int64_t> optimal;     // Hydra best-path score; unset with no current result
    std::optional<int64_t> delta;       // optimal - actual (points left)
    // Actual as a percent of optimal, in whole hundredths of a percent
    // (percent_steps, the number the cell's text is written from: 9901 reads
    // 99.01%). Set only at base speed with an optimal score above zero.
    std::optional<int64_t> pct_h;
    bool is_fc = false;
    int percent = 0;
    int speed = net::kBaseSpeedPercent;
    std::optional<int> rank;
    std::string posted;                 // ISO-8601 timestamp
    // One of the kStatus words above.
    std::string status;
    // Whether the score beat Hydra's optimal: collect_dm_rows' above-optimal
    // answer, kept even when the status became kStatusOtherSpeed, so an
    // off-speed row still reads "+N over" on the page (D64).
    bool above_optimal = false;
};

// The one gate on comparing with dmleaderboards (finding 170): the leaderboard
// holds Clone Hero scores, so the comparison needs Clone Hero's rules, and a
// path its scores can be held against (D104 item 6). Returns an empty string
// when these settings allow it, and otherwise the sentence that names the
// first rule they break. The library toolbar's button and collect_dm_rows
// both ask it.
std::string why_not_comparable(Difficulty difficulty, int sp_cap, bool legacy_fills,
                               bool note_shuffle);

// Joins every fetched score against the store's records for `chartmode`,
// preferring the leaderboard's own song/artist metadata and falling back to the
// joined Hydra record's when the leaderboard entry is an "unknown" one.
// Throws an AlreadyPlain KindedError with why_not_comparable's sentence when
// `lens` breaks a Clone Hero rule, and with report::kNoChartLibrary when the
// store has results but no library (report::lacks_chart_library, D92). Only
// results whose chart the library lists join (report::records_by_hash).
std::vector<DmReportRow> collect_dm_rows(store::RecordStore& store,
                                         const std::vector<net::DmScore>& scores,
                                         const std::string& chartmode,
                                         const store::Lens& lens);

// ---- generate_dm_report ----------------------------------------------------
// The whole comparison in one call: join + tally + the standard page framing.
// The GUI's DmReportJob is an adapter over this seam; the tally previously
// lived in the job layer, comparing the status literals the tests pin.

struct DmReportStats {
    int total = 0;
    int under_optimal = 0;
    int at_optimal = 0;
    int above_optimal = 0;
    int not_analyzed = 0;    // in the library, no current result
    int not_in_library = 0;
    int other_speed = 0;     // played off base speed: shown, not compared
    int no_paths = 0;        // analyzed, but the analysis kept no path
};
DmReportStats tally_dm_rows(const std::vector<DmReportRow>& rows);
// The same tally over the rows a window shows: `shown` holds their indices
// into `rows`.
DmReportStats tally_dm_rows(const std::vector<DmReportRow>& rows,
                            const std::vector<size_t>& shown);

// The colour a row's status chip is drawn in. Takes over the page script's
// STATUS_CLASS table.
report::ChipToken status_token(const std::string& status);

// The comparison's nine tiles over the rows a window shows: `shown` holds
// their indices into `rows`. The counts are tally_dm_rows'. Takes over the
// page script's `stats`.
std::vector<report::Tile> dm_tiles(const std::vector<DmReportRow>& rows,
                                   const std::vector<size_t>& shown);

// "1 under optimal, 0 at optimal, 1 above optimal, 0 not analyzed, 1 not in
// your library", plus ", 1 with no paths" and ", 2 at other speeds" when
// there are any. The comparison window shows it when the player had no scores.
std::string counts_phrase(const DmReportStats& stats);

struct GeneratedDmReport {
    DmReportStats stats;
    // collect_dm_rows' rows, one per score, in the leaderboard's order.
    std::vector<DmReportRow> rows;
    // The line under the page's heading and the note at its foot, as the page
    // shows them. Empty when the user had no scores to compare.
    std::string subtitle;
    std::string footer;
    // Whose scores, compared in which chart mode: generate_dm_report's own
    // arguments, set even when there were no scores.
    std::string username;
    std::string chartmode;
};

GeneratedDmReport generate_dm_report(store::RecordStore& store,
                                     const std::vector<net::DmScore>& scores,
                                     const std::string& chartmode,
                                     const store::Lens& lens,
                                     const std::string& username);

// Whether moving from `before` to `after` changes a setting the comparison
// reads, so a comparison built under `before` is out of date (D103 item 22).
bool settings_change_touches(const Settings& before, const Settings& after);

}  // namespace hydra::app::dm_report

#endif  // HYDRA_APP_DM_REPORT_H
