// The dmleaderboards comparison's table: its columns, the text each cell
// shows, the rows its Status control keeps, and each row's search text. The
// shared table itself is app/report_view.h.

#ifndef HYDRA_APP_DM_REPORT_VIEW_H
#define HYDRA_APP_DM_REPORT_VIEW_H

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "app/dm_report.h"
#include "app/report_view.h"

namespace hydra::app::dm_report_view {

using dm_report::DmReportRow;
using report_view::Column;
using report_view::SortSpec;

// What the count line counts the comparison's rows as.
inline constexpr report_view::CountNoun kNoun{"score", "scores"};

// The thirteen columns, in the page's order.
std::vector<Column<DmReportRow>> dm_columns();

// The sort the comparison opens with.
SortSpec dm_first_sort();

// One choice in the Status dropdown. `status` is the DmReportRow::status it
// keeps, or nullopt for every status.
struct StatusChoice {
    std::optional<std::string> status;
    std::string label;
};

// The Status dropdown's eight choices, in the page's order.
std::vector<StatusChoice> status_choices();

// The rows the Status control keeps: those whose status is `status`, or
// every row when nullopt.
std::function<bool(const DmReportRow&)> dm_keep(std::optional<std::string> status);

// The text the search box matches a row against.
std::string dm_search_text(const DmReportRow& row);

}  // namespace hydra::app::dm_report_view

#endif  // HYDRA_APP_DM_REPORT_VIEW_H
