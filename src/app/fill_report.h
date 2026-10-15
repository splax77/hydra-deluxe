// Comparison report: the same charts scored under Clone Hero 1.0's fill-spawn
// rule against Clone Hero 1.1's, on the shared report shell like
// app/dm_report.h.
//
// A drum fill only spawns if the player's SP meter filled up by some deadline.
// Each rule's deadline is fill_rule_description's to say (search/graph.h).
// Under 1.1 short fills got stricter and long fills got looser, so most charts
// lose or tie under 1.1 and a few gain.
//
// Each result carries its rule in its profile (store::Lens::legacy_fills,
// docs/adr/0010), so one store holds a chart's 1.0 record beside its 1.1
// record. This reads both from that one store, joins them by chart hash and
// emits a self-contained sortable HTML page.

#ifndef HYDRA_APP_FILL_REPORT_H
#define HYDRA_APP_FILL_REPORT_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "store/chart_index.h"
#include "store/record_store.h"

namespace hydra::app {
struct Settings;  // app/config.h
}

namespace hydra::app::fill_report {

// The six statuses a row can carry. collect_fill_rows writes them,
// tally_fill_rows counts by them, and the page's dropdown, chips and tiles
// read them through the STATUS object build_fill_html writes into the script.
inline constexpr const char* kStatusSame = "same";
inline constexpr const char* kStatusOldHigher = "1.0 higher";
inline constexpr const char* kStatusNewHigher = "1.1 higher";
inline constexpr const char* kStatusOnlyOld = "only 1.0";
inline constexpr const char* kStatusOnlyNew = "only 1.1";
// A record on each side but not a score on each: one side scored, or
// neither kept a path.
inline constexpr const char* kStatusInBoth = "in both";

// What the subtitle calls the kStatusInBoth charts; the tile reads it with a
// capital (D114 item 2).
inline constexpr const char* kInBothPhrase = "in both, not both scored";

// One table row: one chart as both fill rules scored it. Every score field is
// optional because a chart may hold a record under only one of the two.
struct FillCompareRow {
    std::string song, artist, charter, hyhash;
    std::optional<int64_t> old_score, new_score;  // CH 1.0, CH 1.1
    std::optional<int64_t> delta;                 // new - old, only when both
    std::string old_path, new_path;
    std::optional<int> old_acts, new_acts;
    std::optional<int> notes;
    // One of the kStatus* constants above.
    std::string status;
    // How many library rows the chart counts as, from the database's library
    // (D79 item 3): the page's "k", which its tiles add up.
    // collect_fill_rows sets it from store::ChartIndex::copies.
    int copies = 0;
};

// Joins `store`'s Ready records under two profiles, the same but for the fill
// rule (Settings::profile_with_fills: the 1.0 results and the 1.1 ones; the
// settings' own fill rule is ignored). Both reads keep the charts the store's
// library lists (report::listed_records, D92). The join is by song id, over
// the union of both key sets: a chart with a record under one rule only still
// gets a row, labelled by the rule that holds its record. Song, artist and
// charter come from the library's naming copy; the song reads display_title,
// and the artist and charter have their Clone Hero rich-text tags stripped.
// Each row's copies come from the same library.
std::vector<FillCompareRow> collect_fill_rows(store::RecordStore& store,
                                              const Settings& settings);

// Every field counts library rows: each status's charts counted through
// `chart_index`'s ChartIndex::count (D79 item 3). `chart_index` is the
// database's library, whose copies the rows carry.
struct FillCompareStats {
    int total = 0;
    int same = 0;
    int ch10_higher = 0;
    int ch11_higher = 0;
    int only_old = 0;
    int only_new = 0;
    int in_both = 0;
};
FillCompareStats tally_fill_rows(const std::vector<FillCompareRow>& rows,
                                 const store::ChartIndex& chart_index);

// The self-contained comparison page, with its own columns. Its template comes
// from html::page_template and html::render_page fills it (html_page.h). The
// count line's total is tally_fill_rows' over `library`.
std::string build_fill_html(const std::vector<FillCompareRow>& rows,
                            const store::ChartIndex& library,
                            const std::string& subtitle,
                            const std::string& footer);

struct GeneratedFillReport {
    std::string html;  // empty when the database had no record to list
    FillCompareStats stats;
    // Why there is no page, in words a person can act on; empty when there
    // is a page. hydra_fillcompare prints it as it is.
    std::string reason;
};

// The whole comparison in one call: join + tally + the standard page framing.
// A database with results and no chart library
// (report::lacks_chart_library) stops it with report::kNoChartLibrary.
GeneratedFillReport generate_fill_report(store::RecordStore& store,
                                         const Settings& settings);

}  // namespace hydra::app::fill_report

#endif  // HYDRA_APP_FILL_REPORT_H
