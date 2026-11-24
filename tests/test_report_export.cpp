// A report table as a spreadsheet sheet (app/report_export.h), on both real
// column sets: the path report's (app/path_report_view.h) and the
// comparison's (app/dm_report_view.h). Each check states what the sheet
// must hold by naming its owner: the view's visible() rows, each column's
// `cell`, `number` and `title`, and the look the test hands in.
//
// The colours here are made up, one per role, so a test can see which one
// landed where. None of them is a Hydra colour.

#include "doctest.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/dm_report.h"  // DmReportRow, kStatus*
#include "app/dm_report_view.h"
#include "app/path_report_view.h"
#include "app/report.h"  // ReportRow
#include "app/report_export.h"
#include "app/report_files.h"   // replacing_temp_path
#include "app/report_view.h"
#include "app/user_messages.h"  // plain_error
#include "app/xlsx.h"
#include "core/error_kind.h"
#include "report_views.h"  // path_view, dm_view
#include "core/winstr.h"  // os_path, read_file_bytes
#include "miniz.h"
#include "temp_util.h"

using namespace hydra;
using namespace hydra::app;
namespace xlsx = hydra::app::xlsx;
using dm_report::DmReportRow;
using report::ReportRow;
using report_export::CellInk;
using report_export::SheetLook;
using report_view::Column;
using report_view::SortDir;
using report_view::TableView;
using report_views::dm_view;
using report_views::path_view;

namespace {

// ---- a made-up look ------------------------------------------------------------

const xlsx::Rgb kBackground{11, 12, 13};
const xlsx::Rgb kText{21, 22, 23};
const xlsx::Rgb kHeaderFill{31, 32, 33};
const xlsx::Rgb kHeaderText{41, 42, 43};

// Every colour a made-up look's functions handed back, so a test can see
// that the sheet holds no colour the look never gave.
using Handed = std::shared_ptr<std::vector<xlsx::Rgb>>;

// A look whose every answer is its own colour. The fill names the place in
// its blue; the font names the column by its title's length in its green and
// the row by `key` in its blue; a chip column's cells get a border that
// names the row the same way.
template <class Row>
SheetLook<Row> made_up_look(std::function<uint8_t(const Row&)> key, Handed handed) {
    SheetLook<Row> look;
    look.background = kBackground;
    look.text = kText;
    look.header_fill = kHeaderFill;
    look.header_text = kHeaderText;
    look.row_fill = [handed](size_t place) {
        const xlsx::Rgb fill{101, 102, static_cast<uint8_t>(place)};
        handed->push_back(fill);
        return fill;
    };
    look.ink = [key, handed](const Column<Row>& column, const Row& row) {
        CellInk ink;
        ink.font = xlsx::Rgb{151, static_cast<uint8_t>(column.title.size()), key(row)};
        handed->push_back(ink.font);
        if (column.look.chip) {
            ink.border = xlsx::Rgb{201, 202, key(row)};
            handed->push_back(*ink.border);
        }
        return ink;
    };
    return look;
}

// A look that inks every cell of a row alike, so neighbours share a style.
template <class Row>
SheetLook<Row> flat_look() {
    SheetLook<Row> look;
    look.background = kBackground;
    look.text = kText;
    look.header_fill = kHeaderFill;
    look.header_text = kHeaderText;
    look.row_fill = [](size_t place) { return xlsx::Rgb{101, 102, static_cast<uint8_t>(place)}; };
    look.ink = [](const Column<Row>&, const Row&) { return CellInk{xlsx::Rgb{151, 152, 153}, std::nullopt}; };
    return look;
}

// ---- rows and views --------------------------------------------------------------

ReportRow path_row(std::string song, int64_t score, bool optimal, int acts) {
    ReportRow r;
    r.song = std::move(song);
    r.artist = "Artist";
    r.charter = "Charter";
    r.path = "1-2(+)";
    r.optimal = optimal;
    r.score = score;
    r.acts = acts;
    r.ms = 12.5;
    r.mult = 3.5;
    r.notes = 1000;
    return r;
}

// Four paths. The search "one" drops Delta Two; Optimal paths only drops
// Gamma One, which is not optimal.
std::vector<ReportRow> four_paths() {
    std::vector<ReportRow> rows{
        path_row("Alpha One", 300, true, 1),
        path_row("Beta One", 500, true, 2),
        path_row("Gamma One", 900, false, 3),
        path_row("Delta Two", 700, true, 4),
    };
    rows[1].ms.reset();  // Beta One needs no timing: its Hardest ms has no number
    return rows;
}

TableView<ReportRow> filtered_path_view() {
    TableView<ReportRow> view = path_view(four_paths());
    view.set_search("one");
    view.set_keep(path_report_view::path_keep(std::nullopt, true));
    view.set_sort({{"score", SortDir::Descending}});
    return view;
}

uint8_t acts_key(const ReportRow& r) { return static_cast<uint8_t>(r.acts); }

DmReportRow dm_row(std::string song, int64_t actual, const char* status, int percent) {
    DmReportRow r;
    r.song = std::move(song);
    r.artist = "Artist";
    r.charter = "Charter";
    r.actual = actual;
    r.optimal = 200000;
    r.delta = *r.optimal - actual;
    r.timing_required = 12.5;
    r.percent = percent;
    r.rank = 3;
    r.posted = "2026-11-23T10:00:00Z";
    r.status = status;
    return r;
}

// Four comparison rows. The search "song" drops Other Tune; the Status
// dropdown's under optimal drops the at optimal row.
TableView<DmReportRow> filtered_dm_view() {
    std::vector<DmReportRow> rows{
        dm_row("Song North", 150000, dm_report::kStatusUnderOptimal, 91),
        dm_row("Song South", 180000, dm_report::kStatusUnderOptimal, 92),
        dm_row("Song East", 200000, dm_report::kStatusAtOptimal, 93),
        dm_row("Other Tune", 170000, dm_report::kStatusUnderOptimal, 94),
    };
    rows[0].timing_required.reset();  // Song North shows a dash in Timing required: no number
    TableView<DmReportRow> view = dm_view(std::move(rows));
    view.set_search("song");
    view.set_keep(dm_report_view::dm_keep(std::string(dm_report::kStatusUnderOptimal)));
    view.set_sort({{"actual", SortDir::Descending}});
    return view;
}

uint8_t percent_key(const DmReportRow& r) { return static_cast<uint8_t>(r.percent); }

// ---- checks ------------------------------------------------------------------------

// The sheet is the view's header and visible() rows, cell for cell: a
// column's number where it has one, an empty cell where the row gives none,
// and its text otherwise. Each style is the look's for that place, row and
// column.
template <class Row>
void check_sheet_matches(const xlsx::Sheet& sheet, const TableView<Row>& view, const SheetLook<Row>& look) {
    const std::vector<Column<Row>>& columns = view.columns();
    const std::vector<size_t>& shown = view.visible();
    REQUIRE(sheet.rows.size() == 1 + shown.size());

    const xlsx::CellStyle header{look.header_fill, look.header_text, std::nullopt};
    REQUIRE(sheet.rows[0].size() == columns.size());
    for (size_t c = 0; c < columns.size(); ++c) {
        CAPTURE(columns[c].id);
        CHECK(sheet.rows[0][c].holds == xlsx::Cell::Holds::Text);
        CHECK(sheet.rows[0][c].text == columns[c].title);
        CHECK(sheet.rows[0][c].style == header);
    }

    for (size_t k = 0; k < shown.size(); ++k) {
        CAPTURE(k);
        const Row& row = view.rows()[shown[k]];
        const std::vector<xlsx::Cell>& cells = sheet.rows[1 + k];
        REQUIRE(cells.size() == columns.size());
        for (size_t c = 0; c < columns.size(); ++c) {
            const Column<Row>& column = columns[c];
            CAPTURE(column.id);
            const xlsx::Cell& cell = cells[c];
            if (column.number) {
                const std::optional<double> n = column.number(row);
                if (n) {
                    CHECK(cell.holds == xlsx::Cell::Holds::Number);
                    CHECK(cell.number == *n);
                    CHECK(cell.format == column.number_format);
                } else {
                    CHECK(cell.holds == xlsx::Cell::Holds::Empty);
                }
            } else {
                CHECK(cell.holds == xlsx::Cell::Holds::Text);
                CHECK(cell.text == column.cell(row));
            }
            const CellInk ink = look.ink(column, row);
            CHECK(cell.style.fill == look.row_fill(k));
            CHECK(cell.style.font == ink.font);
            CHECK(cell.style.border == ink.border);
        }
    }
}

// The text of column `id`'s cells under the header, top to bottom.
template <class Row>
std::vector<std::string> column_texts(const xlsx::Sheet& sheet, const TableView<Row>& view, const std::string& id) {
    size_t at = 0;
    while (view.columns()[at].id != id) ++at;
    std::vector<std::string> out;
    for (size_t r = 1; r < sheet.rows.size(); ++r) out.push_back(sheet.rows[r][at].text);
    return out;
}

template <class Row>
size_t column_index(const TableView<Row>& view, const std::string& id) {
    for (size_t c = 0; c < view.columns().size(); ++c)
        if (view.columns()[c].id == id) return c;
    FAIL("no column " << id);
    return 0;
}

// The kind of KindedError `f` throws, or nothing when it throws none.
template <class F>
std::optional<ErrorKind> kind_thrown(F f) {
    try {
        f();
    } catch (const KindedError& e) {
        return e.kind();
    }
    return std::nullopt;
}

}  // namespace

// ---- rows, cells and headings --------------------------------------------------------

TEST_CASE("report export: the path sheet holds the rows the view shows, in its order") {
    const TableView<ReportRow> view = filtered_path_view();
    const SheetLook<ReportRow> look = made_up_look<ReportRow>(acts_key, std::make_shared<std::vector<xlsx::Rgb>>());
    const xlsx::Sheet sheet = report_export::build_sheet(view, look, "Path report");

    CHECK(sheet.name == "Path report");
    // The search and the keep-rule leave Beta One and Alpha One; Score high
    // to low puts Beta One first.
    REQUIRE(view.visible().size() == 2);
    CHECK(column_texts(sheet, view, "song") == std::vector<std::string>{"Beta One", "Alpha One"});
    check_sheet_matches(sheet, view, look);
}

TEST_CASE("report export: the comparison sheet holds the rows the view shows, in its order") {
    const TableView<DmReportRow> view = filtered_dm_view();
    const SheetLook<DmReportRow> look = made_up_look<DmReportRow>(percent_key, std::make_shared<std::vector<xlsx::Rgb>>());
    const xlsx::Sheet sheet = report_export::build_sheet(view, look, "dmleaderboards");

    REQUIRE(view.visible().size() == 2);
    CHECK(column_texts(sheet, view, "song") == std::vector<std::string>{"Song South", "Song North"});
    check_sheet_matches(sheet, view, look);
}

TEST_CASE("report export: a changed sort changes the sheet's order") {
    TableView<ReportRow> view = filtered_path_view();
    const SheetLook<ReportRow> look = made_up_look<ReportRow>(acts_key, std::make_shared<std::vector<xlsx::Rgb>>());
    view.set_sort({{"score", SortDir::Ascending}});
    const xlsx::Sheet sheet = report_export::build_sheet(view, look, "Path report");
    CHECK(column_texts(sheet, view, "song") == std::vector<std::string>{"Alpha One", "Beta One"});
    check_sheet_matches(sheet, view, look);
}

TEST_CASE("report export: filters that keep nothing give the header row alone") {
    TableView<ReportRow> view = filtered_path_view();
    view.set_search("nothing matches this");
    REQUIRE(view.visible().empty());
    const xlsx::Sheet sheet = report_export::build_sheet(view, flat_look<ReportRow>(), "Path report");
    REQUIRE(sheet.rows.size() == 1);
    CHECK(sheet.rows[0].size() == view.columns().size());
}

TEST_CASE("report export: the headings are every column's title, in order, with no row-number column") {
    const TableView<ReportRow> path = filtered_path_view();
    const xlsx::Sheet path_sheet = report_export::build_sheet(path, flat_look<ReportRow>(), "Path report");
    REQUIRE(path_sheet.rows[0].size() == path.columns().size());
    for (size_t c = 0; c < path.columns().size(); ++c) CHECK(path_sheet.rows[0][c].text == path.columns()[c].title);
    CHECK(path_sheet.rows[0][0].text == "Song");

    const TableView<DmReportRow> dm = filtered_dm_view();
    const xlsx::Sheet dm_sheet = report_export::build_sheet(dm, flat_look<DmReportRow>(), "dmleaderboards");
    REQUIRE(dm_sheet.rows[0].size() == dm.columns().size());
    for (size_t c = 0; c < dm.columns().size(); ++c) CHECK(dm_sheet.rows[0][c].text == dm.columns()[c].title);
    CHECK(dm_sheet.rows[0][0].text == "Song");
}

TEST_CASE("report export: a number column's missing value is an empty cell") {
    const TableView<ReportRow> path = filtered_path_view();
    const xlsx::Sheet path_sheet = report_export::build_sheet(path, flat_look<ReportRow>(), "Path report");
    // Beta One, first under the header, has no Hardest ms.
    CHECK(path_sheet.rows[1][column_index(path, "ms")].holds == xlsx::Cell::Holds::Empty);
    CHECK(path_sheet.rows[2][column_index(path, "ms")].holds == xlsx::Cell::Holds::Number);

    const TableView<DmReportRow> dm = filtered_dm_view();
    const xlsx::Sheet dm_sheet = report_export::build_sheet(dm, flat_look<DmReportRow>(), "dmleaderboards");
    // Song North, second under the header, has no Timing required.
    CHECK(dm_sheet.rows[2][column_index(dm, "timing")].holds == xlsx::Cell::Holds::Empty);
    CHECK(dm_sheet.rows[1][column_index(dm, "timing")].holds == xlsx::Cell::Holds::Number);
}

// ---- widths ------------------------------------------------------------------------

TEST_CASE("report export: a column is one character wider than its longest text, header included") {
    const TableView<ReportRow> view = filtered_path_view();
    const xlsx::Sheet sheet = report_export::build_sheet(view, flat_look<ReportRow>(), "Path report");
    REQUIRE(sheet.widths.size() == view.columns().size());
    // "Alpha One" is 9 characters, longer than the heading "Song".
    CHECK(sheet.widths[column_index(view, "song")] == 10);
    // "Artist" is both the heading and every cell: 6 characters.
    CHECK(sheet.widths[column_index(view, "artist")] == 7);
    // The heading "Note count" (10) is longer than the cells' "1,000".
    CHECK(sheet.widths[column_index(view, "notes")] == 11);
}

TEST_CASE("report export: a width counts characters, not bytes") {
    // "Café Ünï" is 8 characters and 11 bytes of UTF-8.
    TableView<ReportRow> view = path_view({path_row("Caf\xC3\xA9 \xC3\x9Cn\xC3\xAF", 100, true, 1)});
    const xlsx::Sheet sheet = report_export::build_sheet(view, flat_look<ReportRow>(), "Path report");
    CHECK(sheet.widths[column_index(view, "song")] == 9);
}

TEST_CASE("report export: a width is held to Excel's widest column") {
    TableView<ReportRow> view = path_view({path_row(std::string(300, 'x'), 100, true, 1)});
    const xlsx::Sheet sheet = report_export::build_sheet(view, flat_look<ReportRow>(), "Path report");
    CHECK(sheet.widths[column_index(view, "song")] == xlsx::kExcelMaxColumnWidth);
}

// ---- colours ------------------------------------------------------------------------

TEST_CASE("report export: the sheet's default style is the look's background and text") {
    const TableView<ReportRow> view = filtered_path_view();
    const xlsx::Sheet sheet = report_export::build_sheet(
        view, made_up_look<ReportRow>(acts_key, std::make_shared<std::vector<xlsx::Rgb>>()), "Path report");
    CHECK(sheet.default_style.fill == kBackground);
    CHECK(sheet.default_style.font == kText);
    CHECK_FALSE(sheet.default_style.border.has_value());
}

TEST_CASE("report export: a row's fill follows its place and its font follows the row after a sort") {
    TableView<ReportRow> view = filtered_path_view();
    const SheetLook<ReportRow> look = made_up_look<ReportRow>(acts_key, std::make_shared<std::vector<xlsx::Rgb>>());
    const size_t song = column_index(view, "song");

    // Score high to low: Beta One (2 acts) at place 0, Alpha One (1 act) at 1.
    xlsx::Sheet sheet = report_export::build_sheet(view, look, "Path report");
    CHECK(sheet.rows[1][song].style.fill == xlsx::Rgb{101, 102, 0});
    CHECK(sheet.rows[1][song].style.font.b == 2);
    CHECK(sheet.rows[2][song].style.fill == xlsx::Rgb{101, 102, 1});
    CHECK(sheet.rows[2][song].style.font.b == 1);

    // Low to high moves Alpha One to place 0: it takes place 0's fill and
    // keeps its own font.
    view.set_sort({{"score", SortDir::Ascending}});
    sheet = report_export::build_sheet(view, look, "Path report");
    CHECK(sheet.rows[1][song].text == "Alpha One");
    CHECK(sheet.rows[1][song].style.fill == xlsx::Rgb{101, 102, 0});
    CHECK(sheet.rows[1][song].style.font.b == 1);
    CHECK(sheet.rows[2][song].style.fill == xlsx::Rgb{101, 102, 1});
    CHECK(sheet.rows[2][song].style.font.b == 2);
}

TEST_CASE("report export: a cell has a border exactly when the look gives one") {
    const TableView<ReportRow> path = filtered_path_view();
    const xlsx::Sheet path_sheet = report_export::build_sheet(
        path, made_up_look<ReportRow>(acts_key, std::make_shared<std::vector<xlsx::Rgb>>()), "Path report");
    // The made-up look borders the chip columns only: Timing here.
    CHECK(path_sheet.rows[1][column_index(path, "tier")].style.border == xlsx::Rgb{201, 202, 2});
    CHECK_FALSE(path_sheet.rows[1][column_index(path, "song")].style.border.has_value());

    const TableView<DmReportRow> dm = filtered_dm_view();
    const xlsx::Sheet dm_sheet = report_export::build_sheet(
        dm, made_up_look<DmReportRow>(percent_key, std::make_shared<std::vector<xlsx::Rgb>>()), "dmleaderboards");
    // Status is the comparison's chip; Song South's percent is 92.
    CHECK(dm_sheet.rows[1][column_index(dm, "status")].style.border == xlsx::Rgb{201, 202, 92});
    CHECK_FALSE(dm_sheet.rows[1][column_index(dm, "song")].style.border.has_value());
}

TEST_CASE("report export: a number cell with no value has its neighbours' style") {
    const TableView<ReportRow> view = filtered_path_view();
    const xlsx::Sheet sheet = report_export::build_sheet(view, flat_look<ReportRow>(), "Path report");
    // Beta One's Hardest ms is empty; Max skip sits left of it, Timing right.
    const std::vector<xlsx::Cell>& beta = sheet.rows[1];
    const xlsx::Cell& empty = beta[column_index(view, "ms")];
    REQUIRE(empty.holds == xlsx::Cell::Holds::Empty);
    CHECK(empty.style == beta[column_index(view, "skip")].style);
    CHECK(empty.style == beta[column_index(view, "tier")].style);
}

TEST_CASE("report export: every colour in the sheet is one the look gave") {
    const Handed handed = std::make_shared<std::vector<xlsx::Rgb>>();
    const TableView<DmReportRow> view = filtered_dm_view();
    const xlsx::Sheet sheet =
        report_export::build_sheet(view, made_up_look<DmReportRow>(percent_key, handed), "dmleaderboards");

    std::vector<xlsx::Rgb> given = *handed;
    for (const xlsx::Rgb& c : {kBackground, kText, kHeaderFill, kHeaderText}) given.push_back(c);
    const auto was_given = [&](const xlsx::Rgb& c) {
        for (const xlsx::Rgb& g : given)
            if (g == c) return true;
        return false;
    };

    CHECK(was_given(sheet.default_style.fill));
    CHECK(was_given(sheet.default_style.font));
    for (const std::vector<xlsx::Cell>& row : sheet.rows) {
        for (const xlsx::Cell& cell : row) {
            CHECK(was_given(cell.style.fill));
            CHECK(was_given(cell.style.font));
            if (cell.style.border) CHECK(was_given(*cell.style.border));
        }
    }
}

TEST_CASE("report export: a look without its row fill or ink is refused") {
    const TableView<ReportRow> view = filtered_path_view();
    const auto build = [&](const SheetLook<ReportRow>& look) {
        return report_export::build_sheet(view, look, "Path report");
    };
    SheetLook<ReportRow> no_fill = flat_look<ReportRow>();
    no_fill.row_fill = nullptr;
    CHECK_THROWS_AS(build(no_fill), std::invalid_argument);
    SheetLook<ReportRow> no_ink = flat_look<ReportRow>();
    no_ink.ink = nullptr;
    CHECK_THROWS_AS(build(no_ink), std::invalid_argument);
}

// ---- the write -------------------------------------------------------------------------

TEST_CASE("report export: a good write leaves a workbook the zip reader opens") {
    const xlsx::Sheet sheet = report_export::build_sheet(filtered_path_view(), flat_look<ReportRow>(), "Path report");
    const testtemp::ScopedFile file("report_export", ".xlsx");
    const std::filesystem::path out = std::filesystem::u8path(file.path);
    report_export::write_sheet(out, sheet);

    const std::vector<uint8_t> bytes = read_file_bytes(file.path);
    mz_zip_archive zip{};
    REQUIRE(mz_zip_reader_init_mem(&zip, bytes.data(), bytes.size(), 0));
    CHECK(mz_zip_reader_locate_file(&zip, "xl/worksheets/sheet1.xml", nullptr, 0) >= 0);
    mz_zip_reader_end(&zip);
    CHECK_FALSE(std::filesystem::exists(os_path(replacing_temp_path(out))));
}

TEST_CASE("report export: a write into a missing folder throws ReportWrite and leaves no temp file") {
    const std::string dir = testtemp::temp_path("report_export_missing", "");
    testtemp::remove_dir(dir);
    const std::filesystem::path out = std::filesystem::u8path(dir + "\\Path report.xlsx");
    const xlsx::Sheet sheet = report_export::build_sheet(filtered_path_view(), flat_look<ReportRow>(), "Path report");

    std::optional<std::string> sentence;
    const std::optional<ErrorKind> kind = kind_thrown([&] {
        try {
            report_export::write_sheet(out, sheet);
        } catch (const KindedError& e) {
            sentence = plain_error(e);
            throw;
        }
    });
    CHECK(kind == ErrorKind::ReportWrite);
    // The user reads the sentence Hydra gives every ReportWrite failure.
    CHECK(sentence == plain_error(KindedError(ErrorKind::ReportWrite, "")));
    CHECK_FALSE(std::filesystem::exists(os_path(out)));
    CHECK_FALSE(std::filesystem::exists(os_path(replacing_temp_path(out))));
}

TEST_CASE("report export: a sheet over the row limit throws ReportWrite and writes no file") {
    // Two table rows under a header is three rows: one over a limit of two.
    const xlsx::Sheet sheet = report_export::build_sheet(filtered_path_view(), flat_look<ReportRow>(), "Path report");
    REQUIRE(sheet.rows.size() == 3);
    const testtemp::ScopedFile file("report_export_limit", ".xlsx");
    const std::filesystem::path out = std::filesystem::u8path(file.path);
    CHECK(kind_thrown([&] { report_export::write_sheet(out, sheet, 2); }) == ErrorKind::ReportWrite);
    CHECK_FALSE(std::filesystem::exists(os_path(out)));
}
