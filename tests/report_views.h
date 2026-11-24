// Test helpers that put report rows into the TableView a report window holds:
// the rows, each row's search text and the window's real column set.
#ifndef HYDRA_TESTS_REPORT_VIEWS_H
#define HYDRA_TESTS_REPORT_VIEWS_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "app/dm_report_view.h"
#include "app/path_report_view.h"
#include "app/report.h"
#include "app/report_view.h"

namespace hydra::report_views {

inline std::vector<std::string> path_texts(const std::vector<app::report::ReportRow>& rows) {
    std::vector<std::string> out;
    for (const app::report::ReportRow& r : rows)
        out.push_back(app::path_report_view::path_search_text(r));
    return out;
}

inline std::vector<std::string> dm_texts(const std::vector<app::dm_report::DmReportRow>& rows) {
    std::vector<std::string> out;
    for (const app::dm_report::DmReportRow& r : rows)
        out.push_back(app::dm_report_view::dm_search_text(r));
    return out;
}

inline app::report_view::TableView<app::report::ReportRow> path_view(
    std::vector<app::report::ReportRow> rows) {
    std::vector<std::string> texts = path_texts(rows);
    return app::report_view::TableView<app::report::ReportRow>(
        std::make_shared<const std::vector<app::report::ReportRow>>(std::move(rows)),
        std::move(texts), app::path_report_view::path_columns());
}

inline app::report_view::TableView<app::dm_report::DmReportRow> dm_view(
    std::vector<app::dm_report::DmReportRow> rows) {
    std::vector<std::string> texts = dm_texts(rows);
    return app::report_view::TableView<app::dm_report::DmReportRow>(
        std::make_shared<const std::vector<app::dm_report::DmReportRow>>(std::move(rows)),
        std::move(texts), app::dm_report_view::dm_columns());
}

}  // namespace hydra::report_views

#endif  // HYDRA_TESTS_REPORT_VIEWS_H
