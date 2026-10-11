// Tests for the report's page: the tiles above the tables, and the fill
// comparison page's HTML shell, shared stylesheet and script, colour contrast
// and print rules, and the file write that swaps a page in. The rows the
// tiles count are tested in test_report_rows.cpp.

#include "doctest.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "app/dm_report.h"
#include "app/fill_report.h"
#include "app/html_page.h"
#include "app/report.h"
#include "app/report_files.h"
#include "report_samples.h"
#include "temp_util.h"
#include "wcag_util.h"

using namespace hydra;
using namespace hydra::app;

namespace {

size_t occurrences(const std::string& text, const std::string& what) {
    size_t n = 0;
    for (size_t at = text.find(what); at != std::string::npos; at = text.find(what, at + 1)) ++n;
    return n;
}

using report_samples::every_row;

using TilePairs = std::vector<std::pair<std::string, std::string>>;

// The tiles as (label, value) pairs, so one CHECK compares them all.
TilePairs tile_pairs(const std::vector<report::Tile>& tiles) {
    TilePairs out;
    for (const report::Tile& t : tiles) out.emplace_back(t.label, t.value);
    return out;
}

}  // namespace

TEST_CASE("write_report_file swaps the page in and leaves no .tmp behind") {
    const std::string path = testtemp::temp_path("report_file", ".html");

    hydra::app::write_report_file(path, "<html>old</html>\n");
    hydra::app::write_report_file(path, "<html>\nnew\n</html>\n");

    // The page's bytes land as given: no "\r\n" is added (D114 item 28 gives
    // CRLF to the settings file only).
    std::ifstream in(path, std::ios::binary);
    std::ostringstream contents;
    contents << in.rdbuf();
    in.close();
    CHECK(contents.str() == "<html>\nnew\n</html>\n");

    CHECK_FALSE(std::filesystem::exists(path + ".tmp"));

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

TEST_CASE("the fill page carries the shared stylesheet and script") {
    const std::string fill = fill_report::build_fill_html({}, "sub", "foot");
    CHECK(fill.find(html::report_css()) != std::string::npos);
    CHECK(fill.find(html::kReportJs) != std::string::npos);
    // Rules no page used, and the theme switch nothing ever sets, are gone.
    CHECK(fill.find(".delta {") == std::string::npos);
    CHECK(fill.find(".rank {") == std::string::npos);
    CHECK(fill.find("data-theme") == std::string::npos);
    // The page searches words only (D56 item 1, D57 item 3): accents fold,
    // and every typed word must appear in the text the page shows; quotes
    // and field prefixes are ordinary words. It never joins its fields and
    // looks for the query as one lowercased run, and it carries the fold
    // table Hydra built from the library's fold.
    CHECK(fill.find("toLowerCase().includes(") == std::string::npos);
    CHECK(fill.find("const FOLD = {\"A\":\"a\",") != std::string::npos);
    // The shared script looks the query's characters up in that table and
    // matches against each row's search text, which Hydra folded in C++ (the
    // "a row's search text is folded" case pins that fold). These checks
    // stand for the facts, not the exact expressions, so a rewrite of the same
    // logic passes. The script holds no fold of its own: no lowercasing and
    // no accented letter, raw or escaped. Every typed word must match (D56
    // item 1), so the script keeps an every-word test; a change to any-word
    // matching (`.some(`) fails here.
    const std::string script = html::kReportJs;
    CHECK(script.find("FOLD[") != std::string::npos);
    CHECK(script.find(".search") != std::string::npos);
    CHECK(script.find(".every(") != std::string::npos);
    CHECK(script.find("toLowerCase") == std::string::npos);
    CHECK(std::none_of(script.begin(), script.end(),
                       [](char c) { return static_cast<unsigned char>(c) >= 0x80; }));
    for (const char* range : {"\\u00", "\\u01", "\\uff", "\\uFF"})
        CHECK(script.find(range) == std::string::npos);
    CHECK(fill.find("<div class=\"wrap fill\">") != std::string::npos);

    // Every number on the page groups through the shared fmt, which uses one
    // fixed rule (1,234), whatever language the browser is set to (D48 Q12).
    // The single-owner scan keeps it that way.
    CHECK(std::string(html::kReportJsHead).find(".toLocaleString('en-US')") !=
          std::string::npos);
    CHECK(fill.find(".toLocaleString()") == std::string::npos);
}

// The six path rows and four scores the samples page shows
// (tests/report_samples.h). The path_tiles and dm_tiles cases are pinned on
// them.
using report_samples::kSampleHitWindowMs;
using report_samples::sample_dm_rows;
using report_samples::sample_path_rows;

TEST_CASE("path_tiles: the six sample rows") {
    const std::vector<report::ReportRow> rows = sample_path_rows();
    CHECK(tile_pairs(report::path_tiles(rows, every_row(rows.size()), kSampleHitWindowMs)) ==
          TilePairs{{"Charts shown", "3"},
                    {"Paths shown", "6"},
                    {"Hardest ms", "171.0 ms"},
                    {"Past 170 ms", "1"},
                    {"Highest skip", "1"}});
}

TEST_CASE("path_tiles: the tiles count only the rows shown") {
    const std::vector<report::ReportRow> rows = sample_path_rows();
    // Song A's three rows and Song C's one: two charts, no Beyond row, and the
    // hardest timing is Song C's 90 ms.
    CHECK(tile_pairs(report::path_tiles(rows, {0, 1, 2, 5}, kSampleHitWindowMs)) ==
          TilePairs{{"Charts shown", "2"},
                    {"Paths shown", "4"},
                    {"Hardest ms", "90.0 ms"},
                    {"Past 170 ms", "0"},
                    {"Highest skip", "1"}});
    // Song A's mode with no squeeze alone: no timing to show.
    CHECK(tile_pairs(report::path_tiles(rows, {2}, kSampleHitWindowMs)) ==
          TilePairs{{"Charts shown", "1"},
                    {"Paths shown", "1"},
                    {"Hardest ms", "—"},
                    {"Past 170 ms", "0"},
                    {"Highest skip", "0"}});
    // Nothing passes the filters.
    CHECK(tile_pairs(report::path_tiles(rows, {}, kSampleHitWindowMs)) ==
          TilePairs{{"Charts shown", "0"},
                    {"Paths shown", "0"},
                    {"Hardest ms", "—"},
                    {"Past 170 ms", "0"},
                    {"Highest skip", "0"}});
}

TEST_CASE("path_tiles: Charts shown adds each chart's copies once") {
    std::vector<report::ReportRow> rows = sample_path_rows();
    // Song A is in the library 1,200 times and Song B twice; Song A's three
    // rows still count its copies once.
    for (report::ReportRow& r : rows) r.copies = r.hyhash == "Song A" ? 1200 : 2;
    CHECK(report::path_tiles(rows, every_row(rows.size()), kSampleHitWindowMs)[0].value ==
          "1,204");
    CHECK(report::path_tiles(rows, {0, 1, 2}, kSampleHitWindowMs)[0].value == "1,200");
    CHECK(report::path_tiles(rows, {3}, kSampleHitWindowMs)[0].value == "2");
}

TEST_CASE("dm_tiles: the four sample scores") {
    const std::vector<dm_report::DmReportRow> rows = sample_dm_rows();
    CHECK(tile_pairs(dm_report::dm_tiles(rows, every_row(rows.size()))) ==
          TilePairs{{"Scores shown", "4"},
                    {"Under optimal", "1"},
                    {"At optimal", "0"},
                    {"Above optimal", "1"},
                    {"Not analyzed", "1"},
                    {"Not in library", "1"},
                    {"Other speed", "0"},
                    {"Avg % of optimal", "98.80%"},
                    {"Points left on table", "3,456"}});
}

// ---- the page shell ----------------------------------------------------------

namespace {

// WCAG 2.2 relative luminance of "#rrggbb" (testwcag owns the formula).
double luminance(const std::string& hex) {
    auto channel = [&hex](size_t at) {
        return std::stoi(hex.substr(at, 2), nullptr, 16) / 255.0;
    };
    return testwcag::relative_luminance(channel(1), channel(3), channel(5));
}

double contrast(const std::string& a, const std::string& b) {
    return testwcag::contrast_ratio(luminance(a), luminance(b));
}

// The "--name: #rrggbb" tokens of the first ":root {" block at or after `from`.
std::map<std::string, std::string> css_tokens(const std::string& css, size_t from) {
    std::map<std::string, std::string> out;
    const size_t open = css.find(":root {", from);
    REQUIRE(open != std::string::npos);
    const std::string block = css.substr(open, css.find('}', open) - open);
    size_t at = 0;
    while ((at = block.find("--", at)) != std::string::npos) {
        const size_t colon = block.find(':', at);
        const size_t semi = block.find(';', colon);
        const size_t hash = block.find('#', colon);
        if (hash != std::string::npos && hash < semi && semi - hash == 7)
            out[block.substr(at + 2, colon - at - 2)] = block.substr(hash, 7);
        if (semi == std::string::npos) break;
        at = semi;
    }
    return out;
}

// The inside of the first "@media print" block, up to its matching brace, or
// "" when the stylesheet has none.
std::string print_block(const std::string& css) {
    const size_t media = css.find("@media print");
    if (media == std::string::npos) return {};
    const size_t open = css.find('{', media);
    if (open == std::string::npos) return {};
    int depth = 1;
    for (size_t i = open + 1; i < css.size(); ++i) {
        if (css[i] == '{') ++depth;
        if (css[i] == '}' && --depth == 0) return css.substr(open + 1, i - open - 1);
    }
    return {};
}

// The declarations of the rule in `css` whose whole selector is `selector`,
// with every space taken out, or "" when there is no such rule.
std::string rule_body(const std::string& css, const std::string& selector) {
    auto blank = [](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; };
    for (size_t at = css.find(selector); at != std::string::npos;
         at = css.find(selector, at + 1)) {
        // Only a selector that starts its rule: the text before it, past any
        // spaces, is the start of `css` or the end of the rule before.
        size_t before = at;
        while (before > 0 && blank(css[before - 1])) --before;
        if (before > 0 && css[before - 1] != '}' && css[before - 1] != '{') continue;
        size_t open = at + selector.size();
        while (open < css.size() && blank(css[open])) ++open;
        if (open >= css.size() || css[open] != '{') continue;
        const size_t close = css.find('}', open);
        std::string body;
        for (size_t i = open + 1; i < close && i < css.size(); ++i)
            if (!blank(css[i])) body.push_back(css[i]);
        return body;
    }
    return {};
}

}  // namespace

TEST_CASE("the fill page is a standards-mode document") {
    const std::string fill = fill_report::build_fill_html({}, "sub", "foot");
    CHECK(fill.rfind("<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n", 0) == 0);
    CHECK(fill.find("</style>\n</head>\n<body>\n") != std::string::npos);
    REQUIRE(fill.size() > 16);
    CHECK(fill.substr(fill.size() - 16) == "</body>\n</html>\n");
    CHECK(occurrences(fill, "<title>") == 1);
}

TEST_CASE("report colours meet WCAG contrast in both themes") {
    const std::string css = html::report_css();
    const std::map<std::string, std::string> light = css_tokens(css, 0);
    const std::map<std::string, std::string> dark =
        css_tokens(css, css.find("prefers-color-scheme: dark"));
    for (const auto* theme : {&light, &dark}) {
        // Body text, dim text, header text and every chip colour, on the page,
        // on a cell, and on a hovered row or the header band.
        for (const char* text : {"ink", "muted", "t0", "t3", "tn"}) {
            for (const char* ground : {"paper", "surface", "raised"}) {
                INFO(text << " on " << ground);
                CHECK(contrast(theme->at(text), theme->at(ground)) >= 4.5);
            }
        }
        // The accent is the large bold heading word and the focus ring: 3:1.
        CHECK(contrast(theme->at("sp"), theme->at("surface")) >= 3.0);
        CHECK(contrast(theme->at("sp"), theme->at("raised")) >= 3.0);
    }
    // The dark scheme's chip colours are the windows' (html_page.h's
    // kChipRgb*), written in when the stylesheet is built.
    CHECK(dark.at("t0") == "#4fbf94");
    CHECK(dark.at("t3") == "#f2686b");
    CHECK(dark.at("tn") == "#949aa6");
    CHECK(css.find("__CHIP_") == std::string::npos);
}

TEST_CASE("report table header sticks and the page prints") {
    const std::string css = html::report_css();
    const size_t at = css.find(".tablewrap {");
    REQUIRE(at != std::string::npos);
    const std::string rule = css.substr(at, css.find('}', at) - at);
    // Scrolling both ways makes the wrapper the header's sticky container.
    CHECK(rule.find("overflow: auto;") != std::string::npos);
    CHECK(rule.find("max-height:") != std::string::npos);
    CHECK(rule.find("overflow-x") == std::string::npos);

    // On paper the controls go and the header repeats on every page.
    const std::string print = print_block(css);
    REQUIRE_FALSE(print.empty());
    CHECK(rule_body(print, ".controls").find("display:none") != std::string::npos);
    CHECK(rule_body(print, "thead").find("display:table-header-group") != std::string::npos);
}

TEST_CASE("report pages number their rows in a # column") {
    // The shared script adds the column, so the fill page gets it: a header
    // that reads "#" and cells that carry the idx class the stylesheet sizes.
    const std::string js = html::kReportJs;
    CHECK(js.find("'#'") != std::string::npos);
    CHECK(js.find("'idx") != std::string::npos);
    // The sort control and the arrows walk the sortable headers, which the #
    // one is not.
    CHECK(js.find("th.sortable") != std::string::npos);
    CHECK(std::string(html::report_css()).find("td.idx") != std::string::npos);
}

TEST_CASE("path report counts charts by hash in the Charts shown tile") {
    report::ReportRow a;
    a.song = "Same";
    a.artist = "Name";
    a.path = "1";
    a.tier = "None";
    a.tok = "tn";
    a.hyhash = "h1";
    a.copies = 1;
    report::ReportRow a2 = a;
    a2.rank = 2;
    report::ReportRow b = a;  // another chart with the same title and artist
    b.hyhash = "h2";

    // Two charts, however many rows each has, and even when the names match.
    CHECK(report::path_tiles({a, a2, b}, every_row(3), 85.0)[0].value == "2");
    CHECK(report::path_tiles({a, a2, b}, {0, 1}, 85.0)[0].value == "1");
}
