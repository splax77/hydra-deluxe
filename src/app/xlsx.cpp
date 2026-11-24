#include "app/xlsx.h"

#include <charconv>
#include <cmath>
#include <map>
#include <string_view>
#include <tuple>
#include <utility>

#include "core/error_kind.h"
#include "miniz.h"
#include "parse/text_decode.h"

namespace hydra::app::xlsx {

Cell Cell::empty(const CellStyle& style) {
    Cell c;
    c.style = style;
    return c;
}

Cell Cell::of_text(std::string text, const CellStyle& style) {
    Cell c;
    c.kind = Kind::Text;
    c.text = std::move(text);
    c.style = style;
    return c;
}

Cell Cell::of_number(double number, std::string format, const CellStyle& style) {
    Cell c;
    c.kind = Kind::Number;
    c.number = number;
    c.format = std::move(format);
    c.style = style;
    return c;
}

namespace {

[[noreturn]] void refuse(const std::string& why) {
    throw KindedError(ErrorKind::ReportWrite, "cannot write the spreadsheet: " + why);
}

// ---- text ------------------------------------------------------------------

// Whether an XML 1.0 document may hold this code point: the spec's "Char"
// production (XML 1.0, section 2.2). Anything else is dropped from a cell.
bool xml_char(uint32_t cp) {
    return cp == 0x9 || cp == 0xA || cp == 0xD || (cp >= 0x20 && cp <= 0xD7FF) ||
           (cp >= 0xE000 && cp <= 0xFFFD) || (cp >= 0x10000 && cp <= 0x10FFFF);
}

// How many bytes the UTF-8 sequence led by `lead` takes, or 0 when no
// sequence starts with that byte.
size_t sequence_length(unsigned char lead) {
    if (lead < 0x80) return 1;
    if ((lead & 0xE0) == 0xC0) return 2;
    if ((lead & 0xF0) == 0xE0) return 3;
    if ((lead & 0xF8) == 0xF0) return 4;
    return 0;
}

// The code point of one well-formed UTF-8 sequence.
uint32_t code_point(std::string_view seq) {
    const auto at = [&](size_t i) { return static_cast<uint32_t>(static_cast<unsigned char>(seq[i])); };
    switch (seq.size()) {
        case 1: return at(0);
        case 2: return ((at(0) & 0x1F) << 6) | (at(1) & 0x3F);
        case 3: return ((at(0) & 0x0F) << 12) | ((at(1) & 0x3F) << 6) | (at(2) & 0x3F);
        default:
            return ((at(0) & 0x07) << 18) | ((at(1) & 0x3F) << 12) | ((at(2) & 0x3F) << 6) | (at(3) & 0x3F);
    }
}

// Appends `text` as XML character data or an attribute value. Markup
// characters become entities, a carriage return becomes a character
// reference so a reader keeps it, and a code point XML cannot hold is
// dropped, as is a byte that starts no well-formed UTF-8 sequence
// (is_valid_utf8 owns which sequences are well formed).
void append_escaped(std::string& out, std::string_view text) {
    size_t i = 0;
    while (i < text.size()) {
        const size_t len = sequence_length(static_cast<unsigned char>(text[i]));
        if (len == 0 || i + len > text.size() || !is_valid_utf8(text.substr(i, len))) {
            ++i;
            continue;
        }
        const std::string_view seq = text.substr(i, len);
        i += len;
        const uint32_t cp = code_point(seq);
        if (!xml_char(cp)) continue;
        switch (cp) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            case '\r': out += "&#13;"; break;
            default: out.append(seq);
        }
    }
}

std::string escaped(std::string_view text) {
    std::string out;
    append_escaped(out, text);
    return out;
}

// The shortest text that reads back as exactly `v`. The caller has refused a
// value that is not finite.
std::string number_text(double v) {
    char buf[64];
    const auto res = std::to_chars(buf, buf + sizeof buf, v);
    return std::string(buf, res.ptr);
}

// A column's letters from its index from 0: A, B, ... Z, AA, ... XFD.
std::string column_name(size_t index) {
    std::string s;
    size_t n = index + 1;
    while (n > 0) {
        const size_t rem = (n - 1) % 26;
        s.insert(s.begin(), static_cast<char>('A' + rem));
        n = (n - 1) / 26;
    }
    return s;
}

// A colour in the workbook's ARGB form: the alpha byte, always fully solid,
// then the caller's red, green and blue.
std::string argb(const Rgb& c) {
    static constexpr char kHex[] = "0123456789ABCDEF";
    std::string s = "FF";
    for (const uint8_t v : {c.r, c.g, c.b}) {
        s += kHex[v >> 4];
        s += kHex[v & 0xF];
    }
    return s;
}

uint32_t packed(const Rgb& c) {
    return (static_cast<uint32_t>(c.r) << 16) | (static_cast<uint32_t>(c.g) << 8) | c.b;
}

// The head every part starts with, and the two namespaces the parts name.
constexpr const char* kXmlHead = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
constexpr const char* kMainNs = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
constexpr const char* kRelNs = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

// ---- styles ----------------------------------------------------------------

// The workbook's style part, built as cells ask for styles. Each distinct
// pair of a CellStyle and a number format is one entry, however many cells
// share it. The first entry of each list is one Excel requires and the
// writer never hands out: a plain font, no fill (and the gray125 fill Excel
// reserves second), no border, and a plain cell format.
class Styles {
public:
    // The cell format index of `style` shown in `format`, added on first use.
    size_t index(const CellStyle& style, const std::string& format) {
        // The General format (empty) is Excel's built-in format 0 and needs
        // no entry of its own.
        size_t num_fmt = 0;
        if (!format.empty()) num_fmt = intern(formats_, format_list_, format, format, kFirstCustomFormat);
        const size_t font = intern(fonts_, font_list_, packed(style.font), style.font, 1);
        const size_t fill = intern(fills_, fill_list_, packed(style.fill), style.fill, 2);
        size_t border = 0;
        if (style.border) border = intern(borders_, border_list_, packed(*style.border), *style.border, 1);
        const Xf key{num_fmt, font, fill, border};
        return intern(xf_index_, xf_list_, key, key, 1);
    }

    std::string xml() const {
        std::string x = std::string(kXmlHead) + "<styleSheet xmlns=\"" + kMainNs + "\">";
        if (!format_list_.empty()) {
            x += "<numFmts count=\"" + std::to_string(format_list_.size()) + "\">";
            for (size_t i = 0; i < format_list_.size(); ++i) {
                x += "<numFmt numFmtId=\"" + std::to_string(kFirstCustomFormat + i) + "\" formatCode=\"" +
                     escaped(format_list_[i]) + "\"/>";
            }
            x += "</numFmts>";
        }
        x += "<fonts count=\"" + std::to_string(1 + font_list_.size()) + "\">" + font_xml(nullptr);
        for (const Rgb& c : font_list_) x += font_xml(&c);
        x += "</fonts>";
        x += "<fills count=\"" + std::to_string(2 + fill_list_.size()) +
             "\"><fill><patternFill patternType=\"none\"/></fill>"
             "<fill><patternFill patternType=\"gray125\"/></fill>";
        for (const Rgb& c : fill_list_)
            x += "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"" + argb(c) + "\"/></patternFill></fill>";
        x += "</fills>";
        x += "<borders count=\"" + std::to_string(1 + border_list_.size()) +
             "\"><border><left/><right/><top/><bottom/><diagonal/></border>";
        for (const Rgb& c : border_list_) {
            const std::string side = " style=\"thin\"><color rgb=\"" + argb(c) + "\"/>";
            x += "<border><left" + side + "</left><right" + side + "</right><top" + side + "</top><bottom" + side +
                 "</bottom><diagonal/></border>";
        }
        x += "</borders>";
        x += "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>";
        x += "<cellXfs count=\"" + std::to_string(1 + xf_list_.size()) +
             "\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>";
        for (const auto& [num_fmt, font, fill, border] : xf_list_) {
            x += "<xf numFmtId=\"" + std::to_string(num_fmt) + "\" fontId=\"" + std::to_string(font) +
                 "\" fillId=\"" + std::to_string(fill) + "\" borderId=\"" + std::to_string(border) +
                 "\" xfId=\"0\" applyNumberFormat=\"1\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\"/>";
        }
        x += "</cellXfs>";
        x += "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>";
        x += "</styleSheet>";
        return x;
    }

private:
    // Excel numbers its own built-in formats below this; a workbook's own
    // formats start here (ECMA-376 Part 1, 18.8.30).
    static constexpr size_t kFirstCustomFormat = 164;

    using Xf = std::tuple<size_t, size_t, size_t, size_t>;  // number format, font, fill, border

    // The index of `key` in `list`, counting from `first`, added on first use.
    template <class K, class V>
    static size_t intern(std::map<K, size_t>& map, std::vector<V>& list, const K& key, const V& value,
                         size_t first) {
        const auto [it, added] = map.emplace(key, first + list.size());
        if (added) list.push_back(value);
        return it->second;
    }

    // A font with Excel's blank-workbook face and size, in `colour` when
    // given and in Excel's own text colour when not.
    static std::string font_xml(const Rgb* colour) {
        std::string f = "<font><sz val=\"11\"/>";
        if (colour) f += "<color rgb=\"" + argb(*colour) + "\"/>";
        return f + "<name val=\"Calibri\"/><family val=\"2\"/></font>";
    }

    std::map<std::string, size_t> formats_;
    std::vector<std::string> format_list_;
    std::map<uint32_t, size_t> fonts_, fills_, borders_;
    std::vector<Rgb> font_list_, fill_list_, border_list_;
    std::map<Xf, size_t> xf_index_;
    std::vector<Xf> xf_list_;
};

// ---- the parts -------------------------------------------------------------

std::string content_types_xml() {
    return std::string(kXmlHead) +
           "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
           "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
           "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
           "<Override PartName=\"/xl/workbook.xml\" "
           "ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
           "<Override PartName=\"/xl/worksheets/sheet1.xml\" "
           "ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
           "<Override PartName=\"/xl/styles.xml\" "
           "ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
           "</Types>";
}

std::string package_rels_xml() {
    return std::string(kXmlHead) +
           "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
           "<Relationship Id=\"rId1\" Type=\"" + kRelNs + "/officeDocument\" Target=\"xl/workbook.xml\"/>"
           "</Relationships>";
}

std::string workbook_xml(const std::string& sheet_name) {
    return std::string(kXmlHead) + "<workbook xmlns=\"" + kMainNs + "\" xmlns:r=\"" + kRelNs +
           "\"><bookViews><workbookView/></bookViews><sheets><sheet name=\"" + escaped(sheet_name) +
           "\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>";
}

std::string workbook_rels_xml() {
    return std::string(kXmlHead) +
           "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
           "<Relationship Id=\"rId1\" Type=\"" + kRelNs + "/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
           "<Relationship Id=\"rId2\" Type=\"" + kRelNs + "/styles\" Target=\"styles.xml\"/>"
           "</Relationships>";
}

// The sheet part. Every style it uses is added to `styles`.
std::string sheet_xml(const Sheet& sheet, Styles& styles) {
    const size_t default_style = styles.index(sheet.default_style, "");
    const std::string default_attr = " style=\"" + std::to_string(default_style) + "\"";

    std::string x = std::string(kXmlHead) + "<worksheet xmlns=\"" + kMainNs + "\" xmlns:r=\"" + kRelNs + "\">";
    // The header row is frozen: the pane splits below row 1.
    x += "<sheetViews><sheetView workbookViewId=\"0\">"
         "<pane ySplit=\"1\" topLeftCell=\"A2\" activePane=\"bottomLeft\" state=\"frozen\"/>"
         "<selection pane=\"bottomLeft\" activeCell=\"A2\" sqref=\"A2\"/>"
         "</sheetView></sheetViews>";

    // Every column from the first to the last carries the default style, so
    // a cell no row writes takes it too. The given widths go on the first
    // columns; one range covers the rest.
    x += "<cols>";
    for (size_t i = 0; i < sheet.widths.size(); ++i) {
        const std::string n = std::to_string(i + 1);
        x += "<col min=\"" + n + "\" max=\"" + n + "\" width=\"" + number_text(sheet.widths[i]) +
             "\" customWidth=\"1\"" + default_attr + "/>";
    }
    if (sheet.widths.size() < kExcelMaxColumns)
        x += "<col min=\"" + std::to_string(sheet.widths.size() + 1) + "\" max=\"" +
             std::to_string(kExcelMaxColumns) + "\"" + default_attr + "/>";
    x += "</cols>";

    x += "<sheetData>";
    for (size_t r = 0; r < sheet.rows.size(); ++r) {
        const std::string row_number = std::to_string(r + 1);
        x += "<row r=\"" + row_number + "\">";
        const std::vector<Cell>& row = sheet.rows[r];
        for (size_t c = 0; c < row.size(); ++c) {
            const Cell& cell = row[c];
            const bool is_number = cell.kind == Cell::Kind::Number;
            const size_t s = styles.index(cell.style, is_number ? cell.format : std::string());
            x += "<c r=\"" + column_name(c) + row_number + "\" s=\"" + std::to_string(s) + "\"";
            switch (cell.kind) {
                case Cell::Kind::Empty: x += "/>"; break;
                case Cell::Kind::Number: x += "><v>" + number_text(cell.number) + "</v></c>"; break;
                case Cell::Kind::Text:
                    // An inline string: the text sits in the cell and is
                    // marked as text, so Excel never evaluates it.
                    x += " t=\"inlineStr\"><is><t xml:space=\"preserve\">";
                    append_escaped(x, cell.text);
                    x += "</t></is></c>";
                    break;
            }
        }
        x += "</row>";
    }
    x += "</sheetData></worksheet>";
    return x;
}

// Refuses a sheet Excel would call damaged, before any byte is written.
void check_fits(const Sheet& sheet, size_t max_rows) {
    if (sheet.rows.size() > max_rows)
        refuse(std::to_string(sheet.rows.size()) + " rows, more than the " + std::to_string(max_rows) +
               " a sheet holds");
    if (sheet.widths.size() > kExcelMaxColumns)
        refuse(std::to_string(sheet.widths.size()) + " column widths, more columns than a sheet holds");
    for (const double w : sheet.widths)
        if (!std::isfinite(w)) refuse("a column width that is not a finite number");
    for (const std::vector<Cell>& row : sheet.rows) {
        if (row.size() > kExcelMaxColumns)
            refuse("a row of " + std::to_string(row.size()) + " cells, more columns than a sheet holds");
        for (const Cell& cell : row)
            if (cell.kind == Cell::Kind::Number && !std::isfinite(cell.number))
                refuse("a number cell that is not a finite number");
    }
}

// Ends the zip writer however write_xlsx leaves.
struct ZipWriter {
    mz_zip_archive zip{};
    bool open = false;
    ~ZipWriter() {
        if (open) mz_zip_writer_end(&zip);
    }
    void add(const char* name, const std::string& data) {
        if (!mz_zip_writer_add_mem(&zip, name, data.data(), data.size(), MZ_DEFAULT_LEVEL))
            refuse(std::string("the zip writer refused part ") + name);
    }
};

}  // namespace

std::string write_xlsx(const Sheet& sheet, size_t max_rows) {
    check_fits(sheet, max_rows);

    Styles styles;
    const std::string sheet_part = sheet_xml(sheet, styles);

    ZipWriter w;
    if (!mz_zip_writer_init_heap(&w.zip, 0, 0)) refuse("the zip writer did not start");
    w.open = true;
    w.add("[Content_Types].xml", content_types_xml());
    w.add("_rels/.rels", package_rels_xml());
    w.add("xl/workbook.xml", workbook_xml(sheet.name));
    w.add("xl/_rels/workbook.xml.rels", workbook_rels_xml());
    w.add("xl/worksheets/sheet1.xml", sheet_part);
    w.add("xl/styles.xml", styles.xml());

    void* buf = nullptr;
    size_t size = 0;
    if (!mz_zip_writer_finalize_heap_archive(&w.zip, &buf, &size)) refuse("the zip writer did not finish");
    std::string bytes(static_cast<const char*>(buf), size);
    mz_free(buf);
    return bytes;
}

}  // namespace hydra::app::xlsx
