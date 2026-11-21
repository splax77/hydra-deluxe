// Tests for the report's page: the path report's tiles over rows read from
// the store, and the fill comparison page's HTML shell, shared stylesheet and
// script, colour contrast, and the file write that swaps a page in. The rows
// the tiles count are tested in test_report_rows.cpp, and the comparison's
// tiles in test_dm_report.cpp.

#include "doctest.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "app/config.h"
#include "app/fill_report.h"
#include "app/html_page.h"
#include "app/library_query.h"  // search_fold_table
#include "app/report.h"
#include "app/report_files.h"
#include "core/model.h"           // group_thousands, format_ms
#include "core/squeeze_rating.h"  // beyond_tier, timing_tiers
#include "store/chart_index.h"
#include "store/record_store.h"
#include "corpus_util.h"
#include "dm_fixture.h"  // testdm::settings, testdm::save
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

// The title every chart of corpus_page is listed under. The scan's row
// (corpus::scanned_entry) gives every chart the same artist, so with one
// title too, two charts can only be told apart by their hash.
const char* const kSharedTitle = "Same Title";

// A second library folder holding the corpus chart `chart`: the chart file,
// and its song.ini when it has one, copied into the folder `dir`, and the row
// a scan writes for the copy.
store::ChartLibraryEntry listed_again(const std::string& chart, const std::string& dir) {
    const size_t slash = chart.rfind('\\');
    REQUIRE(slash != std::string::npos);
    testtemp::write_bytes(dir + chart.substr(slash), hydra::read_file_bytes(chart));
    const std::string ini = chart.substr(0, slash) + "\\song.ini";
    if (std::filesystem::exists(hydra::os_path(ini)))
        testtemp::write_bytes(dir + "\\song.ini", hydra::read_file_bytes(ini));
    return corpus::scanned_entry(dir + chart.substr(slash), kSharedTitle);
}

// The path report's rows and the library they were read against.
struct CorpusPage {
    std::vector<report::ReportRow> rows;
    store::ChartIndex library;
};

// Three corpus charts with paths, analyzed and saved through the store under
// testdm::settings(), each listed by the row a scan writes for its file, all
// under kSharedTitle. The first chart is listed a second time from a copy of
// its folder, so it counts two library rows. The rows and the library are
// collect_rows' own.
const CorpusPage& corpus_page() {
    static const CorpusPage page = [] {
        const Settings settings = testdm::settings();
        const BatchRun run = settings.batch_run();
        std::vector<corpus::ChartWithPaths> charts = corpus::charts_with_paths(run.settings, 3);
        REQUIRE(charts.size() == 3);
        store::RecordStore store(":memory:");
        std::vector<store::ChartLibraryEntry> library;
        for (const corpus::ChartWithPaths& c : charts)
            library.push_back(corpus::scanned_entry(c.chart, kSharedTitle));
        // The scan reads the copy here; nothing opens it after.
        const std::string copy_dir = testtemp::temp_dir("report_page_copy");
        library.push_back(listed_again(charts[0].chart, copy_dir));
        testtemp::remove_dir(copy_dir);
        // The copy is the same chart, or the library would hold four charts.
        REQUIRE(library.back().md5 == library.front().md5);
        store.rebuild_chart_library(library);
        for (size_t i = 0; i < charts.size(); ++i)
            testdm::save(store, library[i].md5, settings, charts[i].result.record);
        report::ReportOptions options;
        options.run = run;
        report::CollectedRows collected = report::collect_rows(store, options);
        return CorpusPage{std::move(collected.rows), std::move(collected.library)};
    }();
    return page;
}

// The indices of the rows of chart `id`.
std::vector<size_t> rows_of_chart(const CorpusPage& page, const std::string& id) {
    std::vector<size_t> out;
    for (size_t i = 0; i < page.rows.size(); ++i)
        if (page.rows[i].hyhash == id) out.push_back(i);
    return out;
}

// The shown lists a window can reach with its filters: every row, each
// chart's rows alone, every other row, the first row alone, and no row.
std::vector<std::vector<size_t>> shown_lists(const CorpusPage& page) {
    std::vector<std::vector<size_t>> out = {report_samples::every_row(page.rows.size())};
    for (const std::string& id : page.library.song_ids()) out.push_back(rows_of_chart(page, id));
    std::vector<size_t> alternate;
    for (size_t i = 0; i < page.rows.size(); i += 2) alternate.push_back(i);
    out.push_back(alternate);
    out.push_back({0});
    out.push_back({});
    return out;
}

// path_tiles over `shown`, handed the library's own count as generate_report
// hands it over (GeneratedReport::library_charts).
std::vector<report::Tile> page_tiles(const CorpusPage& page, const std::vector<size_t>& shown) {
    return report::path_tiles(page.rows, shown, page.library.count(page.library.song_ids()));
}

// The song ids of the rows at `shown`.
std::vector<std::string> shown_ids(const CorpusPage& page, const std::vector<size_t>& shown) {
    std::vector<std::string> ids;
    for (size_t i : shown) ids.push_back(page.rows[i].hyhash);
    return ids;
}

// The Charts shown tile's text for `shown`: the charts with rows shown, every
// library copy counted (ChartIndex::count, D76, D77), of every chart the
// library lists, counted the same way.
std::string charts_shown_text(const CorpusPage& page, const std::vector<size_t>& shown) {
    return group_thousands(page.library.count(shown_ids(page, shown))) + " of " +
           group_thousands(page.library.count(page.library.song_ids()));
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

    CHECK_FALSE(std::filesystem::exists(hydra::app::replacing_temp_path(path)));

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

TEST_CASE("the fill page carries the shared stylesheet and script") {
    const std::string fill = fill_report::build_fill_html({}, store::ChartIndex{}, "sub", "foot");
    CHECK(fill.find(html::report_css()) != std::string::npos);
    CHECK(fill.find(html::report_js()) != std::string::npos);
    // Rules no page used, and the theme switch nothing ever sets, are gone.
    CHECK(fill.find(".delta {") == std::string::npos);
    CHECK(fill.find(".rank {") == std::string::npos);
    CHECK(fill.find("data-theme") == std::string::npos);
    // The page folds a typed query with the Library's own fold (D56 item 1):
    // it carries every entry of search_fold_table, each written as the page
    // writes its data (json_escape_into).
    const std::vector<FoldEntry> table = search_fold_table();
    REQUIRE_FALSE(table.empty());
    REQUIRE(fill.find("const FOLD = {") != std::string::npos);
    for (const FoldEntry& e : table) {
        std::string pair;
        html::json_escape_into(pair, e.from);
        pair.push_back(':');
        html::json_escape_into(pair, e.to);
        INFO(pair);
        CHECK(fill.find(pair) != std::string::npos);
    }
}

// ---- the path report's tiles -------------------------------------------------

TEST_CASE("path_tiles: each tile counts the rows shown, read from the store") {
    const CorpusPage& page = corpus_page();
    REQUIRE_FALSE(page.rows.empty());
    const std::string beyond = beyond_tier(timing_tiers()).name;
    for (const std::vector<size_t>& shown : shown_lists(page)) {
        CAPTURE(shown.size());
        const std::vector<report::Tile> tiles = page_tiles(page, shown);
        // path_tiles' order: Charts shown, Paths shown, Hardest ms, Past
        // <edge> ms, Highest skip.
        REQUIRE(tiles.size() == 5);
        CHECK(tiles[0].value == charts_shown_text(page, shown));
        CHECK(tiles[1].value == group_thousands(static_cast<int64_t>(shown.size())));

        // The largest shown timing, as the Hardest ms column writes it
        // (format_ms), or the dash when no shown row has one.
        std::optional<double> hardest;
        int64_t past_edge = 0;
        int highest_skip = 0;
        for (size_t i : shown) {
            const report::ReportRow& r = page.rows[i];
            if (r.ms) hardest = hardest ? (std::max)(*hardest, *r.ms) : *r.ms;
            // The rows tier_for labeled Beyond.
            if (r.tier == beyond) ++past_edge;
            highest_skip = (std::max)(highest_skip, r.skip);
        }
        CHECK(tiles[2].value == (hardest ? format_ms(*hardest) : std::string(report::kDash)));
        CHECK(tiles[3].label == "Past " + report::beyond_edge_text() + " ms");
        CHECK(tiles[3].value == group_thousands(past_edge));
        CHECK(tiles[4].value == std::to_string(highest_skip));
    }
}

TEST_CASE("path_tiles: Charts shown adds each chart's copies once") {
    const CorpusPage& page = corpus_page();
    // The first chart sits in two library folders, so its rows count two
    // library rows, however many rows it has (D76 item 2: every copy counts).
    const std::string& twice = page.library.song_ids().front();
    REQUIRE(page.library.copies(twice) == 2);
    const std::vector<size_t> its_rows = rows_of_chart(page, twice);
    REQUIRE_FALSE(its_rows.empty());
    CHECK(page_tiles(page, its_rows)[0].value == charts_shown_text(page, its_rows));
    CHECK(page.library.count(shown_ids(page, its_rows)) == 2);
    // The "of" half counts the same copies: one more library row than charts.
    CHECK(page.library.count(page.library.song_ids()) ==
          static_cast<int64_t>(page.library.song_ids().size()) + 1);
}

TEST_CASE("path report counts charts by hash in the Charts shown tile") {
    const CorpusPage& page = corpus_page();
    // Every chart shows one title and artist, and still each counts on its own.
    std::set<std::string> ids;
    std::set<std::pair<std::string, std::string>> names;
    for (const report::ReportRow& r : page.rows) {
        ids.insert(r.hyhash);
        names.insert({r.song, r.artist});
    }
    REQUIRE(ids.size() == page.library.song_ids().size());
    REQUIRE(names.size() == 1);
    // Every row shown: every library row counts, though one name covers all.
    const std::vector<size_t> all = report_samples::every_row(page.rows.size());
    CHECK(page_tiles(page, all)[0].value ==
          group_thousands(page.library.count(page.library.song_ids())) + " of " +
              group_thousands(page.library.count(page.library.song_ids())));
    for (const std::string& id : page.library.song_ids()) {
        CAPTURE(id);
        const std::vector<size_t> shown = rows_of_chart(page, id);
        CHECK(page.library.count(shown_ids(page, shown)) == page.library.copies(id));
        CHECK(page_tiles(page, shown)[0].value == charts_shown_text(page, shown));
    }
}

// ---- the page shell ----------------------------------------------------------

// M07: the page's empty state is the windows' sentence, written in by C++.
TEST_CASE("page_template writes the windows' empty-state sentence into the script") {
    const std::string page = html::page_template("t", "", "");
    CHECK(page.find(html::kNothingMatches) != std::string::npos);
    CHECK(page.find("__NOTHING_MATCHES__") == std::string::npos);
}

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

// Every token the stylesheet colours text with: the name in each
// "color: var(--name)" declaration, spaces after the colon or not. A longer
// property that ends in "color" (border-color, scrollbar-color) is not text.
std::set<std::string> text_tokens(const std::string& css) {
    const std::string property = "color:";
    const std::string var = "var(--";
    std::set<std::string> out;
    for (size_t at = css.find(property); at != std::string::npos;
         at = css.find(property, at + 1)) {
        if (at > 0) {
            const unsigned char before = static_cast<unsigned char>(css[at - 1]);
            if (before == '-' || std::isalnum(before) != 0) continue;
        }
        size_t value = at + property.size();
        while (value < css.size() && css[value] == ' ') ++value;
        if (css.compare(value, var.size(), var) != 0) continue;
        const size_t name = value + var.size();
        const size_t close = css.find(')', name);
        if (close == std::string::npos) break;
        out.insert(css.substr(name, close - name));
    }
    return out;
}

// WCAG 2.2's contrast minimums (success criterion 1.4.3): normal text, and
// large text, which 1.4.11 also asks of a focus ring.
constexpr double kWcagTextContrast = 4.5;
constexpr double kWcagLargeTextContrast = 3.0;

}  // namespace

TEST_CASE("the fill page is a standards-mode document") {
    const std::string fill = fill_report::build_fill_html({}, store::ChartIndex{}, "sub", "foot");
    // The doctype first, or a browser draws the page in quirks mode.
    CHECK(fill.rfind("<!DOCTYPE html>", 0) == 0);
    CHECK(occurrences(fill, "<title>") == 1);
}

TEST_CASE("report colours meet WCAG contrast in every colour scheme") {
    const std::string css = html::report_css();
    const std::set<std::string> texts = text_tokens(css);
    REQUIRE_FALSE(texts.empty());
    // Every ":root {" block is one scheme: the light one, the dark one and
    // the one for paper.
    size_t schemes = 0;
    for (size_t at = css.find(":root {"); at != std::string::npos; at = css.find(":root {", at + 1)) {
        ++schemes;
        const std::map<std::string, std::string> scheme = css_tokens(css, at);
        // Every text colour on the page, on a cell, and on a hovered row or
        // the header band. The accent is the large bold heading word and the
        // focus ring.
        for (const std::string& text : texts) {
            const double minimum = text == "sp" ? kWcagLargeTextContrast : kWcagTextContrast;
            for (const char* ground : {"paper", "surface", "raised"}) {
                INFO(schemes << ": " << text << " on " << std::string(ground));
                REQUIRE(scheme.count(text) == 1);
                REQUIRE(scheme.count(ground) == 1);
                CHECK(contrast(scheme.at(text), scheme.at(ground)) >= minimum);
            }
        }
    }
    CHECK(schemes >= 2);
    // The dark scheme's chip colours are the windows' (html_page.h's
    // kChipRgb*), written in when the stylesheet is built.
    const std::map<std::string, std::string> dark =
        css_tokens(css, css.find("prefers-color-scheme: dark"));
    auto rgb = [&dark](const char* token) {
        return static_cast<uint32_t>(std::stoul(dark.at(token).substr(1), nullptr, 16));
    };
    CHECK(rgb("t0") == html::kChipRgbT0);
    CHECK(rgb("t3") == html::kChipRgbT3);
    CHECK(rgb("tn") == html::kChipRgbTn);
    CHECK(css.find("__CHIP_") == std::string::npos);
}
