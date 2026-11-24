// A report window's table as a spreadsheet sheet, and the write that saves
// it. The sheet is a copy of what the window shows: its rows are the
// TableView's visible() rows in that order, its cells are each column's
// `cell` text or `number` (app/report_view.h), and its headings are the
// columns' titles. Nothing here filters, sorts or words a cell again.
//
// The sheet's colours come from a SheetLook the caller hands in. This file
// holds no colour and no rule about which cell is dim, red or a chip: the
// report window fills the look from the functions it draws with
// (ui/report_window.h), so the file reads the way the window does.

#ifndef HYDRA_APP_REPORT_EXPORT_H
#define HYDRA_APP_REPORT_EXPORT_H

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "app/report_files.h"  // write_report_file
#include "app/report_view.h"
#include "app/xlsx.h"

namespace hydra::app::report_export {

// What one table cell's words are drawn in, and, for a chip, the colour of
// its outline. The outline becomes the cell's thin border.
struct CellInk {
    xlsx::Rgb font;
    std::optional<xlsx::Rgb> border;  // set for a chip only
};

// The colours a sheet is drawn in, as the caller answers them. The builder
// asks these and nothing else.
template <class Row>
struct SheetLook {
    xlsx::Rgb background;   // every cell's fill where nothing else sets one
    xlsx::Rgb text;         // every cell's font where nothing else sets one
    xlsx::Rgb header_fill;  // the header row's fill
    xlsx::Rgb header_text;  // the header row's words
    // The fill of the table row at `place`, counted from 0 for the first row
    // under the header: its place in visible(), which is its place on screen.
    std::function<xlsx::Rgb(size_t place)> row_fill;
    // A table cell's ink, for `row`'s cell in `column`.
    std::function<CellInk(const report_view::Column<Row>& column, const Row& row)> ink;
};

// How many characters `text` (UTF-8) holds: each byte that is not a
// continuation byte starts one.
inline size_t text_length(std::string_view text) {
    size_t n = 0;
    for (const char ch : text)
        if ((static_cast<unsigned char>(ch) & 0xC0) != 0x80) ++n;
    return n;
}

// A column's width from the length of its longest text, header included:
// one character more, so the longest text has room beside its neighbour,
// held to Excel's widest.
inline double column_width(size_t longest_text) {
    return (std::min)(static_cast<double>(longest_text + 1), xlsx::kExcelMaxColumnWidth);
}

// `view`'s table as a sheet named `sheet_name`: a header row of the columns'
// titles, then one row for each of visible(), in order. A column with a
// `number` gives a number cell in its `number_format`, or an empty cell where
// the row has no value; any other column gives its `cell` text. Every column
// is in the sheet, in the view's order. Every style comes from `look`.
// Throws std::invalid_argument when `look` lacks row_fill or ink.
template <class Row>
xlsx::Sheet build_sheet(const report_view::TableView<Row>& view, const SheetLook<Row>& look,
                        std::string sheet_name) {
    if (!look.row_fill || !look.ink)
        throw std::invalid_argument("a sheet look needs its row fill and its cell ink");
    const std::vector<report_view::Column<Row>>& columns = view.columns();
    const std::vector<Row>& rows = view.rows();
    const std::vector<size_t>& shown = view.visible();

    xlsx::Sheet sheet;
    sheet.name = std::move(sheet_name);
    sheet.default_style = xlsx::CellStyle{look.background, look.text, std::nullopt};

    std::vector<size_t> longest(columns.size(), 0);
    const xlsx::CellStyle header{look.header_fill, look.header_text, std::nullopt};
    std::vector<xlsx::Cell> head;
    head.reserve(columns.size());
    for (size_t c = 0; c < columns.size(); ++c) {
        head.push_back(xlsx::Cell::of_text(columns[c].title, header));
        longest[c] = text_length(columns[c].title);
    }
    sheet.rows.reserve(1 + shown.size());
    sheet.rows.push_back(std::move(head));

    for (size_t place = 0; place < shown.size(); ++place) {
        const Row& row = rows[shown[place]];
        const xlsx::Rgb fill = look.row_fill(place);
        std::vector<xlsx::Cell> cells;
        cells.reserve(columns.size());
        for (size_t c = 0; c < columns.size(); ++c) {
            const report_view::Column<Row>& column = columns[c];
            const CellInk ink = look.ink(column, row);
            const xlsx::CellStyle style{fill, ink.font, ink.border};
            std::string text = column.cell(row);
            longest[c] = std::max(longest[c], text_length(text));
            if (column.number) {
                const std::optional<double> n = column.number(row);
                cells.push_back(n ? xlsx::Cell::of_number(*n, column.number_format, style)
                                  : xlsx::Cell::empty(style));
            } else {
                cells.push_back(xlsx::Cell::of_text(std::move(text), style));
            }
        }
        sheet.rows.push_back(std::move(cells));
    }

    sheet.widths.reserve(columns.size());
    for (const size_t n : longest) sheet.widths.push_back(column_width(n));
    return sheet;
}

// Saves `sheet` as an .xlsx workbook at `outpath`, through write_report_file
// (app/report_files.h), so a reader never sees a half-written file. Throws a
// KindedError of kind ReportWrite when write_xlsx refuses the sheet
// (`max_rows` is its row limit, which a test passes small) or the file
// cannot be written; plain_error (app/user_messages.h) words it for the user.
inline void write_sheet(const std::filesystem::path& outpath, const xlsx::Sheet& sheet,
                        size_t max_rows = xlsx::kExcelMaxRows) {
    write_report_file(outpath, xlsx::write_xlsx(sheet, max_rows));
}

}  // namespace hydra::app::report_export

#endif  // HYDRA_APP_REPORT_EXPORT_H
