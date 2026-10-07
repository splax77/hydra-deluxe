// Comparison report: one dmleaderboards user's actual scores against Hydra's
// computed optimal for the same charts. It joins the fetched scores to stored
// records by chart-file MD5 (leaderboard `identifier` == Hydra `hyhash`) and
// emits a self-contained sortable HTML page on the shared report shell.
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

#include "net/dmbot_client.h"
#include "parse/song.h"  // Difficulty
#include "store/record_store.h"

namespace hydra::app::dm_report {

// One table row: a single leaderboard score plus the Hydra record it joins to.
struct DmReportRow {
    std::string song;
    std::string artist;
    std::string charter;
    std::string identifier;             // chart-file MD5 (the join key)
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
    // "under optimal" | "at optimal" | "above optimal" (Hydra has a result:
    // the score is below, equal to or over its optimal) |
    // "no paths" (Hydra has a result, but its analysis kept no path; D51
    // call 11) |
    // "not analyzed" (the last scan found the chart, but it has no current
    // result at Clone Hero's cap, kCloneHeroSpCap, for this mode) |
    // "not in library" (the last scan never found it) |
    // "other speed" (played at a speed other than net::kBaseSpeedPercent;
    // shown, never compared).
    std::string status;
    // Whether the score beat Hydra's optimal: collect_dm_rows' "above optimal"
    // answer, kept even when the status became "other speed", so an off-speed
    // row still reads "+N over" on the page (D64).
    bool above_optimal = false;
};

// The one gate on comparing with dmleaderboards (finding 170): the leaderboard
// holds Clone Hero scores, so the comparison needs Clone Hero's rules. Returns
// an empty string when these settings allow it, and otherwise the sentence
// that names the first rule they break. The library toolbar's button and
// collect_dm_rows both ask it.
std::string why_not_comparable(Difficulty difficulty, int sp_cap, bool legacy_fills);

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

// The self-contained comparison page. Same __SUBTITLE__/__FOOTER__/__DATA__
// placeholder mechanism as report::build_html, with its own columns.
std::string build_dm_html(const std::vector<DmReportRow>& rows, const std::string& subtitle,
                          const std::string& footer);

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

// "1 under optimal, 0 at optimal, 1 above optimal, 0 not analyzed, 1 not in
// your library", plus ", 1 with no paths" and ", 2 at other speeds" when
// there are any. The page subtitle and the finished window both read it, so
// the two can't drift.
std::string counts_phrase(const DmReportStats& stats);

struct GeneratedDmReport {
    std::string html;  // empty when the user had no scores to compare
    DmReportStats stats;
};

GeneratedDmReport generate_dm_report(store::RecordStore& store,
                                     const std::vector<net::DmScore>& scores,
                                     const std::string& chartmode,
                                     const store::Lens& lens,
                                     const std::string& username);

}  // namespace hydra::app::dm_report

#endif  // HYDRA_APP_DM_REPORT_H
