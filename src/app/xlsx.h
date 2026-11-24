// The spreadsheet writer: one sheet of cells in, the bytes of an .xlsx
// workbook out. It writes no file itself; the caller saves the bytes.
//
// The writer holds no colour of its own. Every colour in the workbook is one
// the caller put in a CellStyle. A text cell is always stored as text, so a
// cell that starts with "=" is never read as a formula.

#ifndef HYDRA_APP_XLSX_H
#define HYDRA_APP_XLSX_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace hydra::app::xlsx {

// The size of an Excel sheet, from Excel's published specifications and
// limits. A workbook past either one is one Excel calls damaged, so
// write_xlsx refuses it (docs/adr/0033). The last column is also where the
// sheet's default style ends (write_xlsx's cols element).
inline constexpr size_t kExcelMaxRows = 1048576;
inline constexpr size_t kExcelMaxColumns = 16384;  // column XFD
// The widest a column can be, in Excel's character units (docs/adr/0033).
inline constexpr double kExcelMaxColumnWidth = 255;

// A colour as plain red, green and blue, 0 to 255 each.
struct Rgb {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    bool operator==(const Rgb& o) const { return r == o.r && g == o.g && b == o.b; }
    bool operator!=(const Rgb& o) const { return !(*this == o); }
};

// How one cell looks: its solid fill, its font colour, and, when set, a thin
// border of that colour on all four sides.
struct CellStyle {
    Rgb fill;
    Rgb font;
    std::optional<Rgb> border;
    bool operator==(const CellStyle& o) const {
        return fill == o.fill && font == o.font && border == o.border;
    }
    bool operator!=(const CellStyle& o) const { return !(*this == o); }
};

// One cell: empty, text, or a number shown in an Excel number format. An
// empty cell is still written with its style, so it keeps its row's fill.
struct Cell {
    enum class Holds { Empty, Text, Number };
    Holds holds = Holds::Empty;
    std::string text;    // Text: UTF-8, stored as given less what XML cannot hold
    double number = 0;   // Number: the value Excel sorts and sums
    std::string format;  // Number: an Excel number format code; empty is General
    CellStyle style;

    static Cell empty(const CellStyle& style);
    static Cell of_text(std::string text, const CellStyle& style);
    static Cell of_number(double number, std::string format, const CellStyle& style);
};

// One sheet. Row 0 is the header row, which the workbook freezes.
struct Sheet {
    std::string name;            // the sheet's tab name, in a form Excel takes
    std::vector<double> widths;  // column widths from the first column, in Excel's character units
    CellStyle default_style;     // the style of every cell the rows never write
    std::vector<std::vector<Cell>> rows;
};

// The .xlsx bytes of a workbook holding `sheet` alone. Throws a KindedError
// of kind ReportWrite when the sheet has more rows than `max_rows`, more
// columns than kExcelMaxColumns, or a number that is not finite. `max_rows`
// is Excel's limit; a test passes a small one.
std::string write_xlsx(const Sheet& sheet, size_t max_rows = kExcelMaxRows);

}  // namespace hydra::app::xlsx

#endif  // HYDRA_APP_XLSX_H
