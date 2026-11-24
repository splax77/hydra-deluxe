// The number each report column's cell stands for in a spreadsheet
// (Column::number in app/report_view.h), for the path report's columns
// (app/path_report_view.h) and the comparison's (app/dm_report_view.h). Each
// expected number is the row field the test sets; no test writes a cell's
// text or a number format again.

#include "doctest.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "app/display_format.h"  // py_round3
#include "app/dm_report.h"       // DmReportRow, kPercentDecimals, kStatus*
#include "app/dm_report_view.h"
#include "app/path_report_view.h"
#include "app/report.h"          // ReportRow, kDash
#include "app/report_view.h"

using namespace hydra;
using namespace hydra::app;
using dm_report::DmReportRow;
using report::ReportRow;
using report_view::Column;

namespace {

template <class Row>
const Column<Row>& find_column(const std::vector<Column<Row>>& cols, const std::string& id) {
    for (const Column<Row>& c : cols)
        if (c.id == id) return c;
    FAIL("no column " << id);
    return cols.front();
}

// The cell's text with its thousands commas taken out, read as a whole number.
double whole_number_in(const std::string& text) {
    std::string digits = text;
    digits.erase(std::remove(digits.begin(), digits.end(), ','), digits.end());
    return static_cast<double>(std::stoll(digits));
}

// A column's number for `row`, which must be set.
template <class Row>
double number_of(const Column<Row>& c, const Row& row) {
    REQUIRE(c.number);
    const std::optional<double> n = c.number(row);
    REQUIRE(n.has_value());
    return *n;
}

ReportRow full_path_row() {
    ReportRow r;
    r.song = "Song";
    r.artist = "Artist";
    r.charter = "Charter";
    r.path = "1-2(+)";
    r.tier = "Easy";
    r.score = 1234567;
    r.acts = 3;
    r.skip = 2;
    r.ms = 12.25;
    r.efill = -3.5;
    r.mult = 3.8765;
    r.sqin = 1;
    r.sqout = 4;
    r.notes = 2345;
    return r;
}

DmReportRow under_optimal_row() {
    DmReportRow r;
    r.song = "Song";
    r.artist = "Artist";
    r.charter = "Charter";
    r.actual = 198010;
    r.optimal = 200000;
    r.delta = 1990;
    r.pct_h = 9901;
    r.is_fc = true;
    r.percent = 97;
    r.speed = 100;
    r.rank = 5;
    r.posted = "2026-11-23T10:00:00Z";
    r.status = dm_report::kStatusUnderOptimal;
    return r;
}

}  // namespace

TEST_CASE("report numbers: each path column's number is the row field its text is written from") {
    const std::vector<Column<ReportRow>> cols = path_report_view::path_columns();
    const ReportRow row = full_path_row();

    CHECK(number_of(find_column(cols, "score"), row) == static_cast<double>(row.score));
    CHECK(number_of(find_column(cols, "acts"), row) == static_cast<double>(row.acts));
    CHECK(number_of(find_column(cols, "skip"), row) == static_cast<double>(row.skip));
    CHECK(number_of(find_column(cols, "sqin"), row) == static_cast<double>(row.sqin));
    CHECK(number_of(find_column(cols, "sqout"), row) == static_cast<double>(row.sqout));
    CHECK(number_of(find_column(cols, "notes"), row) == static_cast<double>(row.notes));
    // ms 12.25 is an exact binary half of the one decimal the window shows
    // ("12.2 ms"; printf rounds it to even, Excel would show 12.3), so the
    // number is the figure the window shows. Avg mult goes through the
    // window's own rounding, py_round3.
    CHECK(number_of(find_column(cols, "ms"), row) == 12.2);
    CHECK(number_of(find_column(cols, "efill"), row) == *row.efill);
    CHECK(number_of(find_column(cols, "mult"), row) == py_round3(row.mult));

    // The whole-number columns' text is that same number, commas aside.
    for (const char* id : {"score", "acts", "skip", "sqin", "sqout", "notes"}) {
        CAPTURE(id);
        const Column<ReportRow>& c = find_column(cols, id);
        CHECK(whole_number_in(c.cell(row)) == number_of(c, row));
    }
}

TEST_CASE("report numbers: an unset path timing gives no number") {
    const std::vector<Column<ReportRow>> cols = path_report_view::path_columns();
    ReportRow row = full_path_row();
    row.ms.reset();
    row.efill.reset();
    for (const char* id : {"ms", "efill"}) {
        CAPTURE(id);
        const Column<ReportRow>& c = find_column(cols, id);
        REQUIRE(c.number);
        CHECK_FALSE(c.number(row).has_value());
    }
}

TEST_CASE("report numbers: the path report's text columns have no number") {
    const std::vector<Column<ReportRow>> cols = path_report_view::path_columns();
    for (const char* id : {"song", "artist", "charter", "path", "tier"}) {
        CAPTURE(id);
        const Column<ReportRow>& c = find_column(cols, id);
        CHECK_FALSE(c.number);
        CHECK(c.number_format.empty());
    }
}

TEST_CASE("report numbers: each comparison column's number is the row field its text is written from") {
    const std::vector<Column<DmReportRow>> cols = dm_report_view::dm_columns();
    const DmReportRow row = under_optimal_row();

    CHECK(number_of(find_column(cols, "actual"), row) == static_cast<double>(row.actual));
    CHECK(number_of(find_column(cols, "optimal"), row) == static_cast<double>(*row.optimal));
    CHECK(number_of(find_column(cols, "delta"), row) == static_cast<double>(*row.delta));
    CHECK(number_of(find_column(cols, "rank"), row) == static_cast<double>(*row.rank));
    // A whole percent as Excel holds a percent: a share of one. The row's
    // percent is 97 (the cell's "97%") and its speed 100 ("100%").
    CHECK(number_of(find_column(cols, "percent"), row) == 0.97);
    CHECK(number_of(find_column(cols, "speed"), row) == 1.0);
    // under_optimal_row's pct_h is 9901 hundredths of a percent (198,010 of
    // 200,000 at kPercentDecimals 2, the cell's "99.01%"), which Excel holds
    // as the share 0.9901.
    CHECK(number_of(find_column(cols, "pct_h"), row) == 0.9901);

    for (const char* id : {"actual", "optimal", "delta"}) {
        CAPTURE(id);
        const Column<DmReportRow>& c = find_column(cols, id);
        CHECK(whole_number_in(c.cell(row)) == number_of(c, row));
    }
}

TEST_CASE("report numbers: a score above optimal keeps its negative delta") {
    const std::vector<Column<DmReportRow>> cols = dm_report_view::dm_columns();
    DmReportRow row = under_optimal_row();
    row.actual = 201000;
    row.delta = *row.optimal - row.actual;
    row.status = dm_report::kStatusAboveOptimal;
    row.above_optimal = true;
    CHECK(number_of(find_column(cols, "delta"), row) == static_cast<double>(*row.delta));
    CHECK(number_of(find_column(cols, "delta"), row) < 0.0);
}

TEST_CASE("report numbers: an unset comparison value gives no number") {
    const std::vector<Column<DmReportRow>> cols = dm_report_view::dm_columns();
    DmReportRow row = under_optimal_row();
    row.optimal.reset();
    row.delta.reset();
    row.pct_h.reset();
    row.rank.reset();
    row.status = dm_report::kStatusNotInLibrary;
    for (const char* id : {"optimal", "delta", "pct_h", "rank"}) {
        CAPTURE(id);
        const Column<DmReportRow>& c = find_column(cols, id);
        REQUIRE(c.number);
        CHECK_FALSE(c.number(row).has_value());
    }
}

TEST_CASE("report numbers: the comparison's FC, Posted, Status and name columns have no number") {
    const std::vector<Column<DmReportRow>> cols = dm_report_view::dm_columns();
    for (const char* id : {"song", "artist", "charter", "fc", "posted", "status"}) {
        CAPTURE(id);
        const Column<DmReportRow>& c = find_column(cols, id);
        CHECK_FALSE(c.number);
        CHECK(c.number_format.empty());
    }
}

TEST_CASE("report numbers: a % of opt cell has no number exactly when it shows the dash") {
    const std::vector<Column<DmReportRow>> cols = dm_report_view::dm_columns();
    const Column<DmReportRow>& c = find_column(cols, "pct_h");
    REQUIRE(c.number);

    DmReportRow both = under_optimal_row();
    DmReportRow no_pct = both;
    no_pct.pct_h.reset();
    DmReportRow no_optimal = both;  // pct_h left set, as only has_percent guards it
    no_optimal.optimal.reset();
    DmReportRow neither = both;
    neither.pct_h.reset();
    neither.optimal.reset();

    int dashes = 0;
    for (const DmReportRow& row : {both, no_pct, no_optimal, neither}) {
        const bool dash = c.cell(row) == report::kDash;
        if (dash) ++dashes;
        CHECK(c.number(row).has_value() == !dash);
    }
    // Both kinds of cell were seen, so the check above tested each side.
    CHECK(dashes == 3);
}

TEST_CASE("report numbers: a column has a number format exactly when it has a number") {
    for (const Column<ReportRow>& c : path_report_view::path_columns()) {
        CAPTURE(c.id);
        CHECK(static_cast<bool>(c.number) == !c.number_format.empty());
    }
    for (const Column<DmReportRow>& c : dm_report_view::dm_columns()) {
        CAPTURE(c.id);
        CHECK(static_cast<bool>(c.number) == !c.number_format.empty());
    }
}
