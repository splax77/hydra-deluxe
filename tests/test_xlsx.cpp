// The spreadsheet writer (app/xlsx.h). Each test writes a small sheet, opens
// the zip again with miniz's reader and reads the sheet's and the style
// part's XML. No test compares whole files: miniz stamps the current time on
// every zip entry, so two writes of one sheet differ byte for byte.
//
// The colours here are made up, one per role, so a test can see which one
// landed where. None of them is a Hydra colour.

#include "doctest.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/xlsx.h"
#include "core/error_kind.h"
#include "miniz.h"

using namespace hydra;
namespace xlsx = hydra::app::xlsx;

namespace {

// ---- made-up colours and styles ---------------------------------------------

// A made-up colour and the text the workbook must hold for it: the alpha
// byte FF (fully solid), then red, green and blue in hex.
struct Colour {
    xlsx::Rgb rgb;
    const char* written;
};

const Colour kFillA{{1, 2, 3}, "FF010203"};
const Colour kFontA{{4, 5, 6}, "FF040506"};
const Colour kBorderA{{7, 8, 9}, "FF070809"};
const Colour kFillB{{10, 20, 30}, "FF0A141E"};
const Colour kFontB{{40, 50, 60}, "FF28323C"};
const Colour kSheetFill{{100, 110, 120}, "FF646E78"};
const Colour kSheetFont{{200, 210, 220}, "FFC8D2DC"};

const xlsx::CellStyle kPlainA{kFillA.rgb, kFontA.rgb, std::nullopt};
const xlsx::CellStyle kBorderedA{kFillA.rgb, kFontA.rgb, kBorderA.rgb};
const xlsx::CellStyle kPlainB{kFillB.rgb, kFontB.rgb, std::nullopt};
const xlsx::CellStyle kSheetStyle{kSheetFill.rgb, kSheetFont.rgb, std::nullopt};

// A sheet of `rows` under the made-up sheet style.
xlsx::Sheet sheet_of(std::vector<std::vector<xlsx::Cell>> rows, std::vector<double> widths = {}) {
    xlsx::Sheet s;
    s.name = "Test sheet";
    s.widths = std::move(widths);
    s.default_style = kSheetStyle;
    s.rows = std::move(rows);
    return s;
}

// ---- reading the zip back -----------------------------------------------------

bool zip_holds(const std::string& bytes, const char* name) {
    mz_zip_archive zip{};
    if (!mz_zip_reader_init_mem(&zip, bytes.data(), bytes.size(), 0)) return false;
    const bool found = mz_zip_reader_locate_file(&zip, name, nullptr, 0) >= 0;
    mz_zip_reader_end(&zip);
    return found;
}

std::string part(const std::string& bytes, const char* name) {
    mz_zip_archive zip{};
    REQUIRE(mz_zip_reader_init_mem(&zip, bytes.data(), bytes.size(), 0));
    size_t size = 0;
    void* p = mz_zip_reader_extract_file_to_heap(&zip, name, &size, 0);
    std::string out;
    if (p) out.assign(static_cast<const char*>(p), size);
    mz_free(p);
    mz_zip_reader_end(&zip);
    REQUIRE_MESSAGE(p != nullptr, name);
    return out;
}

std::string sheet_part(const xlsx::Sheet& s) { return part(xlsx::write_xlsx(s), "xl/worksheets/sheet1.xml"); }

// ---- reading the XML ----------------------------------------------------------

// Whether `xml` is well formed as far as these parts go: every tag closes in
// order, and outside tags there is no "<" and no "&" that starts no entity.
bool well_formed(const std::string& xml) {
    std::vector<std::string> open;
    size_t i = 0;
    while (i < xml.size()) {
        if (xml[i] == '<') {
            const size_t gt = xml.find('>', i);
            if (gt == std::string::npos) return false;
            const std::string tag = xml.substr(i + 1, gt - i - 1);
            i = gt + 1;
            if (tag.empty()) return false;
            if (tag[0] == '?') continue;
            const bool closing = tag[0] == '/';
            const bool self_closing = tag.back() == '/';
            const std::string name = tag.substr(closing ? 1 : 0, tag.find_first_of(" /", 1) - (closing ? 1 : 0));
            if (closing) {
                if (open.empty() || open.back() != name) return false;
                open.pop_back();
            } else if (!self_closing) {
                open.push_back(name);
            }
            continue;
        }
        if (xml[i] == '&') {
            const size_t semi = xml.find(';', i);
            if (semi == std::string::npos) return false;
            const std::string entity = xml.substr(i, semi - i + 1);
            if (entity != "&amp;" && entity != "&lt;" && entity != "&gt;" && entity != "&quot;" &&
                entity != "&apos;" && entity != "&#13;")
                return false;
        }
        ++i;
    }
    return open.empty();
}

// The element that starts at `start`, through its end tag or its "/>".
std::string element_at(const std::string& xml, size_t start, const std::string& tag) {
    const size_t gt = xml.find('>', start);
    REQUIRE(gt != std::string::npos);
    if (xml[gt - 1] == '/') return xml.substr(start, gt + 1 - start);
    const std::string end_tag = "</" + tag + ">";
    const size_t end = xml.find(end_tag, gt);
    REQUIRE(end != std::string::npos);
    return xml.substr(start, end + end_tag.size() - start);
}

// Every `tag` element directly in the first `list` element of `xml`.
std::vector<std::string> children(const std::string& xml, const std::string& list, const std::string& tag) {
    const size_t from = xml.find("<" + list);
    REQUIRE_MESSAGE(from != std::string::npos, list);
    const size_t to = xml.find("</" + list + ">", from);
    REQUIRE_MESSAGE(to != std::string::npos, list);
    std::vector<std::string> out;
    size_t pos = xml.find('>', from) + 1;
    while (true) {
        pos = xml.find("<" + tag, pos);
        if (pos == std::string::npos || pos >= to) break;
        const char after = xml[pos + 1 + tag.size()];
        if (after != ' ' && after != '>' && after != '/') {
            ++pos;
            continue;
        }
        out.push_back(element_at(xml, pos, tag));
        pos += out.back().size();
    }
    return out;
}

// The cell element at `ref`, such as "B2".
std::string cell_at(const std::string& sheet, const std::string& ref) {
    const size_t start = sheet.find("<c r=\"" + ref + "\"");
    REQUIRE_MESSAGE(start != std::string::npos, ref);
    return element_at(sheet, start, "c");
}

// An attribute's value in the opening tag of `element`, or empty when absent.
std::string attr(const std::string& element, const std::string& name) {
    const std::string head = element.substr(0, element.find('>'));
    const std::string key = " " + name + "=\"";
    const size_t at = head.find(key);
    if (at == std::string::npos) return {};
    const size_t from = at + key.size();
    return head.substr(from, head.find('"', from) - from);
}

bool has_attr(const std::string& element, const std::string& name) {
    return element.substr(0, element.find('>')).find(" " + name + "=\"") != std::string::npos;
}

// XML's five entities and the carriage-return reference, back to their text.
std::string unescape(std::string s) {
    const std::pair<const char*, const char*> entities[] = {
        {"&lt;", "<"}, {"&gt;", ">"}, {"&quot;", "\""}, {"&apos;", "'"}, {"&#13;", "\r"}, {"&amp;", "&"}};
    for (const auto& [from, to] : entities) {
        const std::string f = from;
        for (size_t at = s.find(f); at != std::string::npos; at = s.find(f, at + 1)) s.replace(at, f.size(), to);
    }
    return s;
}

// The text an inline-string cell holds, unescaped.
std::string text_of(const std::string& cell) {
    const std::string open = "<t xml:space=\"preserve\">";
    const size_t from = cell.find(open);
    REQUIRE(from != std::string::npos);
    const size_t to = cell.find("</t>", from);
    REQUIRE(to != std::string::npos);
    return unescape(cell.substr(from + open.size(), to - from - open.size()));
}

// What one style index stands for in the style part.
struct Resolved {
    std::string xf, font, fill, border;
    std::string num_fmt_id;
    std::string format_code;  // empty for a built-in format
};

Resolved style_of(const std::string& styles, const std::string& index) {
    REQUIRE_FALSE(index.empty());
    const std::vector<std::string> xfs = children(styles, "cellXfs", "xf");
    const size_t i = std::stoul(index);
    REQUIRE(i < xfs.size());
    Resolved r;
    r.xf = xfs[i];
    r.font = children(styles, "fonts", "font").at(std::stoul(attr(r.xf, "fontId")));
    r.fill = children(styles, "fills", "fill").at(std::stoul(attr(r.xf, "fillId")));
    r.border = children(styles, "borders", "border").at(std::stoul(attr(r.xf, "borderId")));
    r.num_fmt_id = attr(r.xf, "numFmtId");
    if (styles.find("<numFmts") != std::string::npos)
        for (const std::string& f : children(styles, "numFmts", "numFmt"))
            if (attr(f, "numFmtId") == r.num_fmt_id) r.format_code = unescape(attr(f, "formatCode"));
    return r;
}

// Whether a resolved style holds a solid `fill` and a font in `font`.
void check_colours(const Resolved& r, const Colour& fill, const Colour& font) {
    CHECK(r.fill.find("patternType=\"solid\"") != std::string::npos);
    CHECK(r.fill.find(std::string("<fgColor rgb=\"") + fill.written + "\"/>") != std::string::npos);
    CHECK(r.font.find(std::string("<color rgb=\"") + font.written + "\"/>") != std::string::npos);
}

}  // namespace

// ---- the workbook's parts ------------------------------------------------------

TEST_CASE("xlsx: the zip holds every part a workbook needs") {
    const std::string bytes = xlsx::write_xlsx(sheet_of({{xlsx::Cell::of_text("Song", kPlainA)}}));
    for (const char* name : {"[Content_Types].xml", "_rels/.rels", "xl/workbook.xml", "xl/_rels/workbook.xml.rels",
                             "xl/worksheets/sheet1.xml", "xl/styles.xml"}) {
        CAPTURE(name);
        REQUIRE(zip_holds(bytes, name));
        CHECK(well_formed(part(bytes, name)));
    }
    const std::string types = part(bytes, "[Content_Types].xml");
    CHECK(types.find("PartName=\"/xl/worksheets/sheet1.xml\"") != std::string::npos);
    CHECK(types.find("PartName=\"/xl/styles.xml\"") != std::string::npos);
    CHECK(part(bytes, "xl/workbook.xml").find("<sheet name=\"Test sheet\"") != std::string::npos);
}

// ---- text cells ----------------------------------------------------------------

TEST_CASE("xlsx: a text cell that starts like a formula stays text") {
    for (const std::string text : {"=SUM(A1)", "+1", "-1", "@A1"}) {
        CAPTURE(text);
        const std::string sheet = sheet_part(sheet_of({{xlsx::Cell::of_text(text, kPlainA)}}));
        const std::string cell = cell_at(sheet, "A1");
        CHECK(attr(cell, "t") == "inlineStr");
        CHECK(text_of(cell) == text);
        CHECK(sheet.find("<f") == std::string::npos);  // no formula element anywhere
        CHECK(cell.find("<v>") == std::string::npos);
    }
}

TEST_CASE("xlsx: markup and control characters cannot break the sheet") {
    // \x01 is a control character XML cannot hold, \xFF starts no UTF-8
    // sequence, and \xEF\xBF\xBF is U+FFFF, which XML cannot hold either. The
    // carriage return, tab, newline and the accented letter are characters
    // XML holds, so they stay.
    const std::string given = "a&b<c>d\"e'f\x01g\rh\ti\nj\xFFk\xEF\xBF\xBFl \xC3\xA9";
    const std::string kept = "a&b<c>d\"e'fg\rh\ti\njkl \xC3\xA9";
    const std::string sheet = sheet_part(sheet_of({{xlsx::Cell::of_text(given, kPlainA)}}));
    CHECK(well_formed(sheet));
    CHECK(text_of(cell_at(sheet, "A1")) == kept);
    CHECK(sheet.find('\x01') == std::string::npos);
}

TEST_CASE("xlsx: the sheet's name is escaped in the workbook") {
    xlsx::Sheet s = sheet_of({});
    s.name = "A & B";
    const std::string workbook = part(xlsx::write_xlsx(s), "xl/workbook.xml");
    CHECK(well_formed(workbook));
    CHECK(workbook.find("<sheet name=\"A &amp; B\"") != std::string::npos);
}

// ---- number and empty cells ----------------------------------------------------

TEST_CASE("xlsx: a number cell is a number in its format") {
    const xlsx::Sheet s = sheet_of({{xlsx::Cell::of_number(3208, "#,##0", kPlainA),
                                     xlsx::Cell::of_number(0.9901, "0.00%", kPlainA)}});
    const std::string bytes = xlsx::write_xlsx(s);
    const std::string sheet = part(bytes, "xl/worksheets/sheet1.xml");
    const std::string styles = part(bytes, "xl/styles.xml");

    const std::string a1 = cell_at(sheet, "A1");
    CHECK_FALSE(has_attr(a1, "t"));  // no type: a number
    CHECK(a1.find("<v>3208</v>") != std::string::npos);
    CHECK(style_of(styles, attr(a1, "s")).format_code == "#,##0");

    const std::string b1 = cell_at(sheet, "B1");
    const size_t v = b1.find("<v>");
    REQUIRE(v != std::string::npos);
    CHECK(std::stod(b1.substr(v + 3)) == 0.9901);
    CHECK(style_of(styles, attr(b1, "s")).format_code == "0.00%");
}

TEST_CASE("xlsx: a number cell keeps both its number format and its colours") {
    const std::string bytes = xlsx::write_xlsx(sheet_of({{xlsx::Cell::of_number(12, "#,##0", kPlainB)}}));
    const Resolved r = style_of(part(bytes, "xl/styles.xml"), attr(cell_at(part(bytes, "xl/worksheets/sheet1.xml"), "A1"), "s"));
    CHECK(r.format_code == "#,##0");
    check_colours(r, kFillB, kFontB);
}

TEST_CASE("xlsx: an empty cell is written with its style and no value") {
    const std::string bytes = xlsx::write_xlsx(
        sheet_of({{xlsx::Cell::of_text("x", kPlainA), xlsx::Cell::empty(kPlainB), xlsx::Cell::of_text("y", kPlainA)}}));
    const std::string cell = cell_at(part(bytes, "xl/worksheets/sheet1.xml"), "B1");
    CHECK(cell.find("<v>") == std::string::npos);
    CHECK(cell.find("<is>") == std::string::npos);
    CHECK_FALSE(has_attr(cell, "t"));
    check_colours(style_of(part(bytes, "xl/styles.xml"), attr(cell, "s")), kFillB, kFontB);
}

TEST_CASE("xlsx: cells are placed by column letter past Z") {
    std::vector<xlsx::Cell> row;
    for (int i = 0; i < 28; ++i) row.push_back(xlsx::Cell::of_text("c" + std::to_string(i), kPlainA));
    const std::string sheet = sheet_part(sheet_of({row}));
    CHECK(text_of(cell_at(sheet, "Z1")) == "c25");
    CHECK(text_of(cell_at(sheet, "AA1")) == "c26");
    CHECK(text_of(cell_at(sheet, "AB1")) == "c27");
}

// ---- the sheet's layout --------------------------------------------------------

TEST_CASE("xlsx: the header row is frozen") {
    const std::string sheet = sheet_part(sheet_of({{xlsx::Cell::of_text("Song", kPlainA)}}));
    const size_t at = sheet.find("<pane ");
    REQUIRE(at != std::string::npos);
    const std::string pane = element_at(sheet, at, "pane");
    CHECK(attr(pane, "ySplit") == "1");
    CHECK(attr(pane, "topLeftCell") == "A2");
    CHECK(attr(pane, "state") == "frozen");
    CHECK_FALSE(has_attr(pane, "xSplit"));  // no column is frozen
}

TEST_CASE("xlsx: each column width is the one given") {
    const std::vector<double> widths = {12.5, 30, 8};
    const std::vector<std::string> cols =
        children(sheet_part(sheet_of({{xlsx::Cell::of_text("a", kPlainA)}}, widths)), "cols", "col");
    REQUIRE(cols.size() >= widths.size());
    for (size_t i = 0; i < widths.size(); ++i) {
        CAPTURE(i);
        CHECK(attr(cols[i], "min") == std::to_string(i + 1));
        CHECK(attr(cols[i], "max") == std::to_string(i + 1));
        CHECK(std::stod(attr(cols[i], "width")) == widths[i]);
        CHECK(attr(cols[i], "customWidth") == "1");
    }
}

// ---- styles --------------------------------------------------------------------

TEST_CASE("xlsx: a cell's fill, font colour and border colour come back in its style") {
    const std::string bytes = xlsx::write_xlsx(sheet_of({{xlsx::Cell::of_text("Timing", kBorderedA)}}));
    const Resolved r =
        style_of(part(bytes, "xl/styles.xml"), attr(cell_at(part(bytes, "xl/worksheets/sheet1.xml"), "A1"), "s"));
    check_colours(r, kFillA, kFontA);
    for (const char* side : {"left", "right", "top", "bottom"}) {
        CAPTURE(side);
        const std::string open = std::string("<") + side + " style=\"thin\"><color rgb=\"" + kBorderA.written + "\"/>";
        CHECK(r.border.find(open) != std::string::npos);
    }
}

TEST_CASE("xlsx: a cell given no border comes back with none") {
    const std::string bytes = xlsx::write_xlsx(sheet_of({{xlsx::Cell::of_text("Song", kPlainA)}}));
    const Resolved r =
        style_of(part(bytes, "xl/styles.xml"), attr(cell_at(part(bytes, "xl/worksheets/sheet1.xml"), "A1"), "s"));
    check_colours(r, kFillA, kFontA);
    CHECK(r.border.find("style=") == std::string::npos);
    CHECK(r.border.find("<color") == std::string::npos);
}

TEST_CASE("xlsx: the sheet's default style covers every column to Excel's last") {
    const std::string bytes = xlsx::write_xlsx(sheet_of({{xlsx::Cell::of_text("a", kPlainA)}}, {10, 20}));
    const std::string styles = part(bytes, "xl/styles.xml");
    const std::vector<std::string> cols = children(part(bytes, "xl/worksheets/sheet1.xml"), "cols", "col");
    REQUIRE_FALSE(cols.empty());
    // The ranges run on from column 1 with no gap and end at column 16384
    // (XFD), the last column of a sheet in Excel's specifications.
    unsigned long next = 1;
    for (const std::string& col : cols) {
        CAPTURE(col);
        CHECK(std::stoul(attr(col, "min")) == next);
        next = std::stoul(attr(col, "max")) + 1;
        check_colours(style_of(styles, attr(col, "style")), kSheetFill, kSheetFont);
    }
    CHECK(next - 1 == 16384ul);
}

TEST_CASE("xlsx: every colour in the workbook is one the sheet gave") {
    const std::string bytes = xlsx::write_xlsx(sheet_of({{xlsx::Cell::of_text("a", kBorderedA)},
                                                         {xlsx::Cell::of_number(1, "0", kPlainB)},
                                                         {xlsx::Cell::empty(kPlainA)}}));
    const std::vector<std::string> given = {kFillA.written, kFontA.written,     kBorderA.written,  kFillB.written,
                                            kFontB.written, kSheetFill.written, kSheetFont.written};
    size_t seen = 0;
    for (const char* name : {"xl/styles.xml", "xl/worksheets/sheet1.xml", "xl/workbook.xml"}) {
        const std::string xml = part(bytes, name);
        const std::string key = "rgb=\"";
        for (size_t at = xml.find(key); at != std::string::npos; at = xml.find(key, at + 1)) {
            const size_t from = at + key.size();
            const std::string value = xml.substr(from, xml.find('"', from) - from);
            CAPTURE(value);
            CHECK(std::find(given.begin(), given.end(), value) != given.end());
            ++seen;
        }
        CHECK(xml.find("indexed=") == std::string::npos);  // no colour from Excel's built-in palette
        CHECK(xml.find("theme=") == std::string::npos);    // nor from its theme
    }
    CHECK(seen > 0u);
}

TEST_CASE("xlsx: cells that share a style share one style entry") {
    std::vector<std::vector<xlsx::Cell>> rows;
    for (int r = 0; r < 5; ++r)
        rows.push_back({xlsx::Cell::of_text("a", kPlainA), xlsx::Cell::of_text("b", kPlainB)});
    xlsx::Sheet s = sheet_of(rows);
    s.default_style = kPlainA;  // the sheet's style is one of the two the cells use
    const std::string styles = part(xlsx::write_xlsx(s), "xl/styles.xml");
    // Entry 0 is the plain format Excel requires; then one per style used.
    const std::vector<std::string> xfs = children(styles, "cellXfs", "xf");
    CHECK(xfs.size() == 3u);
    const size_t at = styles.find("<cellXfs ");
    REQUIRE(at != std::string::npos);
    CHECK(attr(styles.substr(at), "count") == "3");
}

// ---- the row limit -------------------------------------------------------------

TEST_CASE("xlsx: a sheet over the row limit is refused, not written") {
    const xlsx::Sheet s = sheet_of({{xlsx::Cell::of_text("h", kPlainA)},
                                    {xlsx::Cell::of_text("1", kPlainA)},
                                    {xlsx::Cell::of_text("2", kPlainA)}});
    CHECK_FALSE(xlsx::write_xlsx(s, 3).empty());  // exactly at the limit: written
    bool refused = false;
    try {
        (void)xlsx::write_xlsx(s, 2);
    } catch (const KindedError& e) {
        refused = e.kind() == ErrorKind::ReportWrite;
    }
    CHECK(refused);
    // The default limit is Excel's sheet of 1,048,576 rows, from Excel's
    // specifications and limits (docs/adr/0033, kExcelMaxRows in xlsx.h).
    CHECK(xlsx::kExcelMaxRows == 1048576u);
}

TEST_CASE("xlsx: a number that is not finite is refused, not written") {
    bool refused = false;
    try {
        (void)xlsx::write_xlsx(sheet_of({{xlsx::Cell::of_number(std::numeric_limits<double>::infinity(), "", kPlainA)}}));
    } catch (const KindedError& e) {
        refused = e.kind() == ErrorKind::ReportWrite;
    }
    CHECK(refused);
}
