// The path report's table: its columns, the text each cell shows, the rows
// its Timing and Best path only controls keep, and each row's search text.
// The shared table itself is app/report_view.h.

#ifndef HYDRA_APP_PATH_REPORT_VIEW_H
#define HYDRA_APP_PATH_REPORT_VIEW_H

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "app/report.h"
#include "app/report_view.h"

namespace hydra::app::path_report_view {

using report::ReportRow;
using report_view::Column;
using report_view::SortSpec;

// What the count line counts the path report's rows as.
inline constexpr const char* kNoun = "paths";

// The fifteen columns, in the page's order. `hit_window_ms` is the window
// the rows' tiers were labeled under; the Timing chip names its Beyond edge.
std::vector<Column<ReportRow>> path_columns(double hit_window_ms);

// The sort the report opens with.
SortSpec path_first_sort();

// A timing tier as the Timing chip and dropdown name it, from its name in
// timing_tiers(hit_window_ms).
std::string tier_label(const std::string& tier_name, double hit_window_ms);

// One choice in the Timing dropdown. `tier` is the timing_tiers name it
// keeps, or nullopt for every tier.
struct TimingChoice {
    std::optional<std::string> tier;
    std::string label;
};

// The Timing dropdown's choices, in order: the one that keeps every tier,
// then one for each of timing_tiers(hit_window_ms).
std::vector<TimingChoice> timing_choices(double hit_window_ms);

// The rows the path report's controls keep: those labeled `tier` (every
// tier when nullopt), and only optimal paths when `best_only` is set.
std::function<bool(const ReportRow&)> path_keep(std::optional<std::string> tier, bool best_only);

// The text the search box matches a row against.
std::string path_search_text(const ReportRow& row);

}  // namespace hydra::app::path_report_view

#endif  // HYDRA_APP_PATH_REPORT_VIEW_H
