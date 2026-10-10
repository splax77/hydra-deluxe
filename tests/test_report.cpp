// Structural tests for app/report.{h,cpp}: collect_rows must surface one row
// per stored record, and generate_report frames those rows with the subtitle
// and footer the report window shows. The fill comparison page's shell and
// the samples case for the browser check live here too.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <atomic>
#include <algorithm>
#include <cmath>
#include <map>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/display_format.h"
#include "app/dm_report.h"
#include "env_util.h"
#include "app/fill_report.h"
#include "app/html_page.h"
#include "app/library_query.h"
#include "app/path_report_view.h"
#include "app/report.h"
#include "app/report_files.h"
#include "app/user_messages.h"
#include "core/model.h"
#include "core/squeeze_rating.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "record_fixtures.h"
#include "report_samples.h"
#include "search/graph.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "ui/library_jobs.h"
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

// The settings the report fixtures file their results under: depth 10 by
// score, the ms limit off, at SP cap `cap`.
Settings fixture_settings(int cap, bool legacy_fills = false) {
    Settings s;
    s.sp_cap = cap;
    s.depth_mode = 0;
    s.depth_value = 10;
    s.mslimit_enabled = false;
    s.legacy_fills = legacy_fills;
    return s;
}

// A library row's notespath that stands for a fixture record instead of a
// chart file: fixture_analyzer hands back H1's tied-variant record for it.
const char* const kTiedFile = "fixture:tied";

// What the report's pass analyzes with in these tests: a real chart file as
// the app analyzes it, and kTiedFile as H1's tied-variant record, filed under
// the settings it is asked for.
ChartAnalyzer fixture_analyzer() {
    return [](const std::string& path, const AnalysisSettings& settings,
              const std::function<void(float)>& on_progress) {
        if (path != kTiedFile) return analyze_chart_file(path, settings, on_progress);
        HydraRecord record = test::tied_variant_record();
        record.sp_cap = settings.sp_cap;
        record.legacy_fills = settings.legacy_fill_deadline;
        return AnalysisResult{std::move(record), test::beat_song({}, {}, 13440)};
    };
}

// A report over the fixtures' results at SP cap `cap`: their cap, lens and
// settings, analyzed by fixture_analyzer.
report::ReportOptions fixture_options(int cap, bool legacy_fills = false) {
    const BatchRun run = fixture_settings(cap, legacy_fills).batch_run();
    report::ReportOptions options;
    options.cap = run.cap_query();
    options.lens = run.lens;
    options.run = run;
    options.analyze = fixture_analyzer();
    return options;
}

store::ChartLibraryEntry library_entry(const std::string& md5, const std::string& title,
                                       const std::string& notespath) {
    store::ChartLibraryEntry e;
    e.md5 = md5;
    e.title = title;
    e.artist = "Artist";
    e.charter = "Charter";
    e.notespath = notespath;
    e.rootfolder = "C:\\songs";
    return e;
}

// Analyzes the first `want` corpus charts with paths under
// fixture_settings(cap) into `store` as charts h0, h1, ... titled "Title 0",
// "Title 1", ..., and lists each in the library by its file (`library` keeps
// the rows, so a test can add more). Returns the files, h0's first.
std::vector<std::string> fill_store(store::RecordStore& store,
                                    std::vector<store::ChartLibraryEntry>& library, int cap,
                                    int want) {
    const BatchRun run = fixture_settings(cap).batch_run();
    std::vector<std::string> files;
    for (corpus::ChartWithPaths& c :
         corpus::charts_with_paths(run.settings, static_cast<size_t>(want))) {
        const std::string hyhash = "h" + std::to_string(files.size());
        const std::string title = "Title " + std::to_string(files.size());
        store.add_record(fixture_settings(cap).record_key(hyhash), c.result.record);
        library.push_back(library_entry(hyhash, title, c.chart));
        files.push_back(c.chart);
    }
    store.rebuild_chart_library(library);
    return files;
}

int fill_store(store::RecordStore& store, int cap, int want) {
    std::vector<store::ChartLibraryEntry> library;
    return static_cast<int>(fill_store(store, library, cap, want).size());
}

// Analyzes two charts at cap 4 into `store` (fill_store), then lists h0 in two
// library folders and h1 in three, so the library holds 5 copies of 2 charts.
void fill_store_with_copies(store::RecordStore& store) {
    std::vector<store::ChartLibraryEntry> files;
    REQUIRE(fill_store(store, files, 4, 2).size() == 2);
    std::vector<store::ChartLibraryEntry> library;
    for (const auto& [chart, copies] : {std::pair<size_t, int>{0, 2}, {1, 3}}) {
        for (int i = 0; i < copies; ++i) {
            store::ChartLibraryEntry e = files[chart];
            e.rootfolder = "C:\\songs" + std::to_string(i);
            library.push_back(e);
        }
    }
    store.rebuild_chart_library(library);
}

void check_cap(int cap) {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    const int added = static_cast<int>(fill_store(store, library, cap, 5).size());
    REQUIRE(added > 0);

    // One row per shown path, so a record surfaces exactly one rank-1 row.
    report::ReportOptions options = fixture_options(cap);
    options.max_paths = 100;
    std::vector<report::ReportRow> rows =
        report::collect_rows(store, report::ReportSeed{}, options).rows;
    int rank1 = 0;
    for (const report::ReportRow& row : rows)
        if (row.rank == 1) ++rank1;
    CHECK(rank1 == added);
    CHECK(static_cast<int>(rows.size()) >= added);

    // Every stored chart is named on some row.
    for (int i = 0; i < added; ++i) {
        const std::string title = "Title " + std::to_string(i);
        INFO(title);
        CHECK(std::any_of(rows.begin(), rows.end(),
                          [&title](const report::ReportRow& r) { return r.song == title; }));
    }

    MESSAGE(std::to_string(cap) << " bars: " << rows.size() << " rows");
}

// The fields that tell two report rows apart, so two reports compare row by row.
using RowKey = std::tuple<std::string, std::string, std::string, int64_t, int>;
std::vector<RowKey> row_keys(const std::vector<report::ReportRow>& rows) {
    std::vector<RowKey> out;
    for (const report::ReportRow& r : rows)
        out.emplace_back(r.hyhash, r.mode, r.path, r.score, r.copies);
    return out;
}

// Stores H1's tied-variant record for chart `hyhash` under
// fixture_settings(cap, legacy_fills), with its song and a library row
// (kTiedFile), so the report has three paths to list: two tied at the top
// score and a lower one.
void store_tied(store::RecordStore& store, std::vector<store::ChartLibraryEntry>& library,
                const std::string& hyhash, int cap, bool legacy_fills = false,
                const std::optional<std::string>& chartmode = std::nullopt) {
    HydraRecord record = test::tied_variant_record();
    record.sp_cap = cap;
    record.legacy_fills = legacy_fills;
    store::RecordKey key = fixture_settings(cap, legacy_fills).record_key(hyhash);
    if (chartmode) key.chartmode = *chartmode;
    store.add_record(key, record);
    library.push_back(library_entry(hyhash, "Tied " + hyhash, kTiedFile));
    store.rebuild_chart_library(library);
}

// The same analyzer, counting the files it is asked for.
struct CountingAnalyzer {
    std::mutex mu;
    std::vector<std::string> paths;
    ChartAnalyzer inner = fixture_analyzer();

    ChartAnalyzer analyzer() {
        return [this](const std::string& path, const AnalysisSettings& settings,
                      const std::function<void(float)>& on_progress) {
            {
                std::lock_guard<std::mutex> lock(mu);
                paths.push_back(path);
            }
            return inner(path, settings, on_progress);
        };
    }
};

// The scan rows a batch over the first `count` charts of `library` reads,
// each made by the GUI's own conversion, ui::scan_item_of.
std::vector<ScanItem> scan_items(const std::vector<store::ChartLibraryEntry>& library,
                                 size_t count) {
    std::vector<ScanItem> items;
    for (size_t i = 0; i < count && i < library.size(); ++i)
        items.push_back(ui::scan_item_of(library[i]));
    return items;
}

}  // namespace

TEST_CASE("report rows: a tied top-score variant is optimal too") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    store_tied(store, library, "tied", 4);

    report::ReportOptions options = fixture_options(4);
    options.max_paths = 100;
    const std::vector<report::ReportRow> rows =
        report::collect_rows(store, report::ReportSeed{}, options).rows;
    REQUIRE(rows.size() == 3);
    // Best score first: the root and its tied variant, then the lower root.
    CHECK(rows[0].score == rows[1].score);
    CHECK(rows[2].score < rows[1].score);
    CHECK(rows[0].optimal);
    CHECK(rows[1].optimal);
    CHECK_FALSE(rows[2].optimal);

    const report::GeneratedReport page = report::generate_report(store, fixture_options(4));
    // The subtitle still counts one record, however many paths tie.
    CHECK(page.records == 1);
    // The report hands both tied paths over as optimal, and the lower one not.
    REQUIRE(page.paths.size() == 3);
    CHECK(std::count_if(page.paths.begin(), page.paths.end(),
                        [](const report::ReportRow& r) { return r.optimal; }) == 2);
    CHECK_FALSE(page.paths[2].optimal);
}

TEST_CASE("report rows list every stored record (4 bars)") { check_cap(4); }

// collect_rows lists a record's paths in all_paths() order and numbers them
// from 1, so that order has to be best first already. This guards it at the
// report's own settings (fill_store's), where variants appear.
TEST_CASE("all_paths lists paths best first on every corpus chart") {
    AnalysisSettings settings;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 10;
    settings.sp_cap = 4;
    int charts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, settings.prodrums, settings.bass2x,
                                        settings.difficulty, settings.rules);
        if (song.is_empty()) continue;
        ++charts;
        const std::vector<const Path*> all = corpus::analyzed(chart, settings).all_paths();
        for (size_t i = 1; i < all.size(); ++i) {
            INFO(chart << " path " << i);
            CHECK(all[i]->totalscore() <= all[i - 1]->totalscore());
        }
    }
    CHECK(charts > 0);
}

TEST_CASE("report counts every library copy of a chart, and leaves out a chart the library dropped") {
    // D76: chart h0 sits in two library folders, so it counts twice, like
    // the library counts it. D87 item 4: chart h1 has a result but no library
    // row, so the page has no file to analyze and leaves it out.
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    REQUIRE(fill_store(store, library, 4, 2).size() == 2);
    store::ChartLibraryEntry first = library[0];
    store::ChartLibraryEntry copy = first;
    copy.rootfolder = "C:\\other";
    store.rebuild_chart_library({first, copy});
    REQUIRE(store.library_copies() == std::unordered_map<std::string, int>{{"h0", 2}});

    const report::GeneratedReport out = report::generate_report(store, fixture_options(4));
    CHECK(out.records == 2);
    CHECK(out.songs == 2);
    CHECK(out.subtitle.find("2 records across 2 charts") != std::string::npos);
    // Every row is h0's and carries its two copies; h1 is left out.
    REQUIRE_FALSE(out.paths.empty());
    for (const report::ReportRow& r : out.paths) {
        CHECK(r.hyhash == "h0");
        CHECK(r.copies == 2);
        CHECK(r.song != "Title 1");
    }
}

TEST_CASE("library copies are keyed like records_by_hash, and an unlisted chart counts once") {
    // The scan writes lower case, but an older row may not: a page joins on
    // normalize_chart_hash, so the copies do too.
    store::RecordStore store(":memory:");
    store::ChartLibraryEntry upper;
    upper.md5 = "ABCDEF00112233445566778899AABBCC";
    upper.title = "Upper";
    upper.notespath = "C:\\songs\\a\\notes.chart";
    store::ChartLibraryEntry lower = upper;
    lower.md5 = "abcdef00112233445566778899aabbcc";
    lower.notespath = "C:\\other\\a\\notes.chart";
    store.rebuild_chart_library({upper, lower});

    const std::unordered_map<std::string, int> library = report::library_copies_by_hash(store);
    CHECK(library ==
          std::unordered_map<std::string, int>{{"abcdef00112233445566778899aabbcc", 2}});
    CHECK(store::RecordStore::copies_of(library, "abcdef00112233445566778899aabbcc") == 2);
    CHECK(store::RecordStore::copies_of(library, "00ff00ff00ff00ff00ff00ff00ff00ff") == 1);
}

// The Charts shown tile adds up each chart's copies once over the rows shown
// (path_tiles), and generate_report's subtitle adds them up once over every
// row. This pins that both count the same thing: every row carries its
// chart's copies, and with every row shown the two totals agree.
TEST_CASE("path report: the Charts shown tile adds up the copies generate_report adds up") {
    store::RecordStore store(":memory:");
    fill_store_with_copies(store);

    const report::GeneratedReport out = report::generate_report(store, fixture_options(4));
    CHECK(out.songs == 5);
    CHECK(out.subtitle.find(" across 5 charts") != std::string::npos);

    // Every row of a chart carries that chart's copies.
    size_t rows_h0 = 0, rows_h1 = 0;
    for (const report::ReportRow& r : out.paths) {
        INFO(r.hyhash);
        CHECK(r.copies == (r.hyhash == "h0" ? 2 : 3));
        ++(r.hyhash == "h0" ? rows_h0 : rows_h1);
    }
    REQUIRE(rows_h0 > 0);
    REQUIRE(rows_h1 > 0);
    CHECK(static_cast<int64_t>(rows_h0 + rows_h1) == out.rows);
    CHECK(report::path_tiles(out.paths, every_row(out.paths.size()), out.hit_window_ms)[0]
              .value == std::to_string(out.songs));
}

// What a window draws from: the rows, the subtitle and footer the page shows,
// and the hit window the rows were labeled with. With every row shown, the
// Charts shown tile reads the subtitle's chart count.
TEST_CASE("generate_report hands over its rows, subtitle and footer") {
    store::RecordStore store(":memory:");
    fill_store_with_copies(store);

    report::ReportOptions options = fixture_options(4);
    options.hit_window_ms = 85.0;
    const report::GeneratedReport out = report::generate_report(store, options);
    REQUIRE(out.rows > 0);
    CHECK(static_cast<int64_t>(out.paths.size()) == out.rows);
    const std::vector<report::ReportRow> collected =
        report::collect_rows(store, report::ReportSeed{}, options).rows;
    REQUIRE(collected.size() == out.paths.size());
    for (size_t i = 0; i < collected.size(); ++i) {
        CHECK(out.paths[i].hyhash == collected[i].hyhash);
        CHECK(out.paths[i].path == collected[i].path);
    }
    CHECK(out.hit_window_ms == 85.0);

    CHECK(out.subtitle.find(" across 5 charts — ") != std::string::npos);
    CHECK(out.footer.rfind("Generated from ", 0) == 0);
    CHECK(out.footer.find("'Beyond' means past the 170 ms window.") != std::string::npos);

    CHECK(report::path_tiles(out.paths, every_row(out.paths.size()), out.hit_window_ms)[0]
              .value == "5");
}

TEST_CASE("report lists only the wanted cap and names it") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    const std::vector<std::string> files = fill_store(store, library, 4, 1);
    REQUIRE(files.size() == 1);
    // The same chart again at 8 bars.
    const BatchRun eight = fixture_settings(8).batch_run();
    store.add_record(fixture_settings(8).record_key("h0"), analyze_chart_file(files[0], eight.settings).record);
    REQUIRE(store.counts().second == 2);

    report::ReportOptions options = fixture_options(4);
    report::GeneratedReport four = report::generate_report(store, options);
    CHECK(hydra::ends_with(four.subtitle, "SP cap 4 bars"));
    // The subtitle counts what the report lists: the one record at 4 bars, not
    // the 8-bar record the database also holds for the same chart.
    CHECK(four.records == 1);
    CHECK(four.songs == 1);
    CHECK(four.subtitle.rfind("1 record across 1 chart — ", 0) == 0);
    int rank1 = 0;
    options.max_paths = 100;
    for (const report::ReportRow& row :
         report::collect_rows(store, report::ReportSeed{}, options).rows)
        if (row.rank == 1) ++rank1;
    CHECK(rank1 == 1);

    // The cap reads through the house count rule: one bar, and commas from
    // 1,000 (D48 Q12).
    store_tied(store, library, "one", 1);
    store_tied(store, library, "thousand", 1000);
    CHECK(hydra::ends_with(report::generate_report(store, fixture_options(1)).subtitle, "SP cap 1 bar"));
    CHECK(hydra::ends_with(report::generate_report(store, fixture_options(1000)).subtitle,
                           "SP cap 1,000 bars"));

    // A 1.0 report names its rule by the fill rule's one long name.
    store_tied(store, library, "legacy", 4, /*legacy_fills=*/true);
    CHECK(hydra::ends_with(report::generate_report(store, fixture_options(4, /*legacy_fills=*/true))
                               .subtitle,
                           std::string("SP cap 4 bars — ") +
                               fill_rule_name(FillDeadlineRule::Ch10, FillRuleNameStyle::Long) +
                               " fills"));
}

// What hydra_report's "--legacy-fills database" case pinned, now that the
// tool is gone: a database filled under the 1.0 rule, as hydra_batch
// --legacy-fills leaves one, reports its chart under the 1.0 rule when the
// report is asked for 1.0 fills, and lists nothing under the 1.1 rule.
TEST_CASE("generate_report reports a 1.0-fills database under the 1.0 rule") {
    store::RecordStore store(":memory:");
    const Settings legacy = fixture_settings(4, /*legacy_fills=*/true);
    std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(legacy.batch_run().settings, 1);
    REQUIRE(charts.size() == 1);
    store.add_record(legacy.record_key("h0"), charts[0].result.record);
    store.rebuild_chart_library({library_entry("h0", "Title 0", charts[0].chart)});
    store.set_engine_mode(engine_mode_stamp(FillDeadlineRule::Ch10));

    const report::GeneratedReport ch10 =
        report::generate_report(store, fixture_options(4, /*legacy_fills=*/true));
    CHECK(ch10.records == 1);
    REQUIRE_FALSE(ch10.paths.empty());
    for (const report::ReportRow& r : ch10.paths) CHECK(r.song == "Title 0");
    CHECK(hydra::ends_with(ch10.subtitle, "SP cap 4 bars — Clone Hero 1.0 fills"));

    const report::GeneratedReport ch11 = report::generate_report(store, fixture_options(4));
    CHECK(ch11.paths.empty());
    CHECK(ch11.empty_reason == report::EmptyReason::NothingUnderSettings);
    CHECK(ch11.why_empty == report::nothing_under_settings(4, "Clone Hero 1.1 fills"));
}

TEST_CASE("collect_rows: the library names go through display_title, display_artist and "
          "display_charter") {
    // test_song.cpp pins what those three do. This checks only that the page
    // calls them: one tagged name each. The page names a chart by its library
    // row (the naming copy, kNamingCopiesSql's pick), so a rescan's names
    // replace store_tied's own.
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    store_tied(store, library, "u0", 4);
    library[0].title = "<b>Bold Title</b>";
    library[0].artist = test::kTagOnlyTitle;
    library[0].charter = " <b>Bob</b> ";
    store.rebuild_chart_library(library);

    const std::vector<report::ReportRow> rows =
        report::collect_rows(store, report::ReportSeed{}, fixture_options(4)).rows;
    REQUIRE(!rows.empty());
    for (const report::ReportRow& row : rows) {
        CHECK(row.song == "Bold Title");
        CHECK(row.artist == kUnknownTitle);
        CHECK(row.charter == "Bob");
    }
}

TEST_CASE("fill_rule_for: the legacy_fills flag and the file stamp each name one fill rule") {
    // legacy_fills on is the Clone Hero 1.0 rule; off is the normal 1.1 rule.
    CHECK(fill_rule_for(true) == FillDeadlineRule::Ch10);
    CHECK(fill_rule_for(false) == FillDeadlineRule::Ch11);
    // A stamp reads back as the rule that wrote it; any other text is no rule.
    CHECK(fill_rule_from_stamp("ch10") == FillDeadlineRule::Ch10);
    CHECK(fill_rule_from_stamp("ch11") == FillDeadlineRule::Ch11);
    CHECK_FALSE(fill_rule_from_stamp("ch12").has_value());
    CHECK_FALSE(fill_rule_from_stamp("").has_value());
}

TEST_CASE("tier_for: raw-ms bands derived from the two-hit budget") {
    using report::tier_for;

    // Default window 85 -> budget 170: bands 2 / 42.5 / 85 / 127.5 / 170.
    // Each edge belongs to the band below it (D48 Q3): a timing on the
    // difficult floor is not past it, so 2.0 ms reads Normal, and 170.0 ms is
    // still inside the two-hit budget, so it reads Insane+.
    CHECK(tier_for(std::nullopt).first == "None");
    CHECK(tier_for(1.9).first == "Normal");
    CHECK_FALSE(past_difficult_floor(2.0));
    CHECK(tier_for(2.0).first == "Normal");
    CHECK(past_difficult_floor(2.1));
    CHECK(tier_for(2.1).first == "Hard");
    CHECK(tier_for(42.5).first == "Hard");
    CHECK(tier_for(42.6).first == "Extreme");
    CHECK(tier_for(85.0).first == "Extreme");
    CHECK(tier_for(85.1).first == "Insane");
    CHECK(tier_for(127.5).first == "Insane");
    CHECK(tier_for(127.6).first == "Insane+");
    CHECK(tier_for(170.0).first == "Insane+");
    CHECK(tier_for(170.0).second == "t4");
    CHECK(tier_for(170.1).first == "Beyond");
    CHECK(tier_for(170.1).second == "t5");

    // At the historical 70 ms window the original 2/35/70/105/140 ladder
    // reproduces, with the same edge rule.
    CHECK(tier_for(2.0, 70.0).first == "Normal");
    CHECK(tier_for(35.0, 70.0).first == "Hard");
    CHECK(tier_for(35.1, 70.0).first == "Extreme");
    CHECK(tier_for(70.1, 70.0).first == "Insane");
    CHECK(tier_for(105.1, 70.0).first == "Insane+");
    CHECK(tier_for(140.0, 70.0).first == "Insane+");
    CHECK(tier_for(140.1, 70.0).first == "Beyond");
}

TEST_CASE("tier_for walks the timing_tiers table edge by edge") {
    // tier_for and the page's embedded tier table read the one ladder in
    // core/squeeze_rating.h, so every banded entry's cutoff is exactly where
    // the label flips to the next entry's: the cutoff itself still reads the
    // entry's own name (D48 Q3), and just past it reads the next one.
    std::vector<TimingTier> tiers = timing_tiers(85.0);
    REQUIRE(tiers.size() >= 2);
    for (size_t i = 0; i + 1 < tiers.size(); ++i) {
        if (!tiers[i].cutoff) continue;
        const double cutoff = *tiers[i].cutoff;
        CHECK(report::tier_for(cutoff, 85.0).first == tiers[i].name);
        CHECK(report::tier_for(cutoff + 0.01, 85.0).first == tiers[i + 1].name);
    }
}

TEST_CASE("path report: a row's search text is folded and tag-free") {
    report::ReportRow row;
    row.song = "Halo";
    row.artist = "Beyonc\xc3\xa9";  // Beyoncé
    row.charter = "<b>Bob</b>";
    row.path = "1";
    row.tier = "None";
    row.tok = "tn";
    CHECK(path_report_view::path_search_text(row) == "halo beyonce bob 1");
}

TEST_CASE("generate_report: one seam frames the report for every entry point") {
    store::RecordStore store(":memory:");
    const int added = fill_store(store, 4, 2);
    REQUIRE(added > 0);

    report::ReportOptions options = fixture_options(4);
    options.max_paths = report::kDefaultReportPaths;
    options.db_path = "C:/somewhere/hydra.db";
    report::GeneratedReport result = report::generate_report(store, options);
    CHECK(result.songs == added);
    CHECK(result.records == added);
    CHECK(result.rows >= added);

    // The framing strings are part of the interface: the GUI's ReportJob
    // ships exactly this subtitle and footer. The cut is per chart and mode,
    // and the report lists every mode at the current cap.
    CHECK(result.empty_reason == report::EmptyReason::None);
    const std::string subtitle = counted(result.records, "record", "records") + " across " +
                                 counted(result.songs, "chart", "charts") +
                                 " — every mode at the current cap, top 5 paths per chart "
                                 "and mode — SP cap 4 bars";
    CHECK(result.subtitle == subtitle);
    // D50 item 1. The default 85 ms hit window puts Beyond past 170 ms, the
    // same number the Beyond chip and the "Past 170 ms" tile print.
    CHECK(result.footer ==
          "Generated from hydra.db. Timing tiers measure how big each "
          "squeeze is, in steps of your hit window. The Paths tab's row "
          "labels measure how far a hit lands from the Star Power end, so the "
          "two can differ. 'Beyond' means past the 170 ms window.");

    // --all-paths wording.
    options.max_paths = report::kEveryPathSentinel;
    CHECK(report::generate_report(store, options)
              .subtitle.find(counted(result.songs, "chart", "charts") + " — every path") !=
          std::string::npos);

    // An empty store yields no rows, and says nothing is stored.
    store::RecordStore empty(":memory:");
    report::GeneratedReport none = report::generate_report(empty, options);
    CHECK(none.rows == 0);
    CHECK(none.paths.empty());
    CHECK(none.empty_reason == report::EmptyReason::NothingStored);

    // Records stored at cap 4, asked at cap 8: the report is empty because of
    // the settings, and the reason names them (finding 105, D48 Q28).
    options = fixture_options(8);
    report::GeneratedReport off = report::generate_report(store, options);
    CHECK(off.rows == 0);
    CHECK(off.paths.empty());
    CHECK(off.empty_reason == report::EmptyReason::NothingUnderSettings);
    const std::string sentence =
        "Nothing is analyzed under these settings (SP cap 8, Clone Hero 1.1 fills). "
        "Analyze with these settings, or change them.";
    CHECK(off.why_empty == sentence);
    // The app's ReportJob throws that sentence as AlreadyPlain, and the strip
    // shows it as it is.
    CHECK(plain_error(hydra::KindedError(hydra::ErrorKind::AlreadyPlain, off.why_empty)) ==
          sentence);
}

TEST_CASE("generate_report: \"every path\" is the sentinel's, a huge --paths stays a count (86)") {
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store, 4, 1) > 0);

    // --all-paths asks for the sentinel, and only that reads "every path".
    report::ReportOptions options = fixture_options(4);
    options.max_paths = report::kEveryPathSentinel;
    CHECK(report::generate_report(store, options).subtitle.find(" — every path — ") !=
          std::string::npos);

    // A large --paths below it lists at most that many, so it says so.
    options.max_paths = 200000000;
    const std::string subtitle = report::generate_report(store, options).subtitle;
    CHECK(subtitle.find("top 200,000,000 paths per chart and mode") != std::string::npos);
    CHECK(subtitle.find(" — every path") == std::string::npos);

    // 140: the hit window keeps a decimal, starting from the default.
    CHECK(std::is_same<decltype(report::ReportOptions{}.hit_window_ms), double>::value);
    CHECK(report::ReportOptions{}.hit_window_ms == kDefaultHitWindowMs);
}

TEST_CASE("nothing_under_settings frames the cap, the middle words and the ending") {
    // One frame for every empty page (finding 105, D50 item 3): the path
    // report passes its fill rule, the fill comparison its chart mode and
    // " in either database".
    CHECK(report::nothing_under_settings(8, "Clone Hero 1.1 fills") ==
          "Nothing is analyzed under these settings (SP cap 8, Clone Hero 1.1 fills). "
          "Analyze with these settings, or change them.");
    CHECK(report::nothing_under_settings(4, "Expert Pro Drums, 2x Bass",
                                         " in either database") ==
          "Nothing is analyzed under these settings (SP cap 4, Expert Pro Drums, 2x Bass) "
          "in either database. Analyze with these settings, or change them.");
    CHECK(report::nothing_under_settings(1200, "Clone Hero 1.0 fills") ==
          "Nothing is analyzed under these settings (SP cap 1,200, Clone Hero 1.0 fills). "
          "Analyze with these settings, or change them.");
}

TEST_CASE("generate_report hands back nothing when its cancel flag is set") {
    // Closing Hydra while the report builds. The walk stops between records,
    // and no page is framed from the part of the library it managed to read.
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store, 4, 2) > 0);

    std::atomic<bool> cancel{true};
    report::ReportOptions options = fixture_options(4);
    options.max_paths = 5;
    options.db_path = "C:/somewhere/hydra.db";
    options.cancel = &cancel;

    report::GeneratedReport result = report::generate_report(store, options);
    CHECK(result.rows == 0);
    CHECK(result.paths.empty());
    CHECK(result.empty_reason == report::EmptyReason::Cancelled);
}

TEST_CASE("the report reuses the charts a batch just analyzed (D87 item 5)") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    const std::vector<std::string> files = fill_store(store, library, 4, 3);
    REQUIRE(files.size() == 3);
    const BatchRun run = fixture_settings(4).batch_run();

    // A batch over the first two charts hands their rows to the seed.
    report::ReportSeed seed = report::ReportSeed::for_run(run);
    CountingAnalyzer batch;
    BatchCallbacks callbacks;
    callbacks.analyze = batch.analyzer();
    callbacks.report_seed = &seed;
    const std::vector<ScanItem> items = scan_items(library, 2);
    run_batch(plan_batch(items, {}), run, store, 2, callbacks);
    REQUIRE(batch.paths.size() == 2);
    CHECK(seed.rows.size() == 2);

    // The report analyzes only the chart the batch did not.
    CountingAnalyzer pass;
    report::ReportOptions options = fixture_options(4);
    options.analyze = pass.analyzer();
    const report::GeneratedReport page = report::generate_report(store, options, seed);
    CHECK(pass.paths == std::vector<std::string>{files[2]});
    CHECK(page.records == 3);
    // The report is the one a fresh analysis of all three gives.
    const report::GeneratedReport fresh = report::generate_report(store, fixture_options(4));
    CHECK(row_keys(page.paths) == row_keys(fresh.paths));
    CHECK(page.subtitle == fresh.subtitle);
    CHECK(page.footer == fresh.footer);

    // Rows filed under other settings are refused, by the batch and the report.
    report::ReportSeed other = report::ReportSeed::for_run(fixture_settings(5).batch_run());
    callbacks.report_seed = &other;
    CHECK_THROWS_AS(run_batch(plan_batch(items, {}), run, store, 2, callbacks),
                    std::invalid_argument);
    other.rows = seed.rows;
    CHECK_THROWS_AS(report::generate_report(store, options, other), std::invalid_argument);
    // So is a report whose settings are not its cap and lens, or missing.
    options.run = fixture_settings(5).batch_run();
    CHECK_THROWS_AS(report::generate_report(store, options), std::invalid_argument);
    options.run.reset();
    CHECK_THROWS_AS(report::generate_report(store, options), std::invalid_argument);
}

TEST_CASE("cancelling the report's pass stops it and builds no page") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    const int charts = 2 * batch_worker_count() + 1;
    for (int i = 0; i < charts; ++i) store_tied(store, library, "c" + std::to_string(i), 4);

    // The first chart any worker starts sets the flag, so each worker
    // analyzes at most the one chart it had started.
    std::atomic<bool> cancel{false};
    std::atomic<int> calls{0};
    report::ReportOptions options = fixture_options(4);
    options.cancel = &cancel;
    const ChartAnalyzer inner = fixture_analyzer();
    options.analyze = [&](const std::string& path, const AnalysisSettings& settings,
                          const std::function<void(float)>& on_progress) {
        ++calls;
        cancel = true;
        return inner(path, settings, on_progress);
    };
    const report::GeneratedReport result = report::generate_report(store, options);
    CHECK(result.empty_reason == report::EmptyReason::Cancelled);
    CHECK(result.paths.empty());
    CHECK(result.rows == 0);
    CHECK(calls.load() <= batch_worker_count());
    CHECK(calls.load() < charts);
}

TEST_CASE("the report analyzes the copy that names a chart the scan found twice") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    const std::vector<std::string> files = fill_store(store, library, 4, 1);
    REQUIRE(files.size() == 1);
    // The scan listed the real file first, as "Zed", then a copy of the same
    // md5 whose file is gone, as "Alpha". Alpha sorts first by name.
    library[0].title = "Zed";
    store::ChartLibraryEntry moved = library[0];
    moved.title = "Alpha";
    moved.notespath = testtemp::temp_path("report_moved_copy", ".chart");
    store.rebuild_chart_library({library[0], moved});

    CountingAnalyzer pass;
    report::ReportOptions options = fixture_options(4);
    options.analyze = pass.analyzer();
    const report::GeneratedReport page = report::generate_report(store, options);
    CHECK(pass.paths == std::vector<std::string>{files[0]});
    CHECK(page.failures.empty());
    CHECK(page.records == 2);  // both copies count (D76)
}

TEST_CASE("a report on results with no chart library says the library is missing (D89)") {
    // What hydra_batch with folder arguments leaves: results, and no scan
    // ever saved as the library. (A scan that lists nothing deletes the
    // results too, D87 item 4.)
    store::RecordStore store(":memory:");
    HydraRecord record = test::tied_variant_record();
    record.sp_cap = 4;
    store.add_record(fixture_settings(4).record_key("t"), record);

    const report::GeneratedReport page = report::generate_report(store, fixture_options(4));
    CHECK(page.rows == 0);
    CHECK(page.paths.empty());
    CHECK(page.empty_reason == report::EmptyReason::NoLibrary);
    CHECK(page.why_empty == std::string(report::kNoChartLibrary));
}

TEST_CASE("a chart whose file fails to load is left off the report and listed") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    REQUIRE(fill_store(store, library, 4, 2).size() == 2);
    const std::string missing = testtemp::temp_path("report_missing", ".chart");

    // No failures: the report says nothing about charts left out.
    const report::GeneratedReport whole = report::generate_report(store, fixture_options(4));
    CHECK(whole.failures.empty());
    CHECK(report::left_out_line(whole.failures).empty());

    library[1].notespath = missing;
    store.rebuild_chart_library(library);
    const report::GeneratedReport page = report::generate_report(store, fixture_options(4));
    CHECK(page.records == 1);
    for (const report::ReportRow& r : page.paths) CHECK(r.song != "Title 1");
    REQUIRE(page.failures.size() == 1);
    CHECK(page.failures[0].notespath == missing);
    CHECK_FALSE(page.failures[0].error.empty());
    // D89 item 1: the report names the chart it left out; its file is in failures.
    CHECK(report::left_out_line(page.failures) ==
          "Left out: 1 chart whose file couldn't be read.");

    // A chart mode no settings spell is an error, not a skipped chart.
    store_tied(store, library, "odd", 4, false, std::string("Legendary Drums, 3x Bass"));
    CHECK_THROWS_AS(report::generate_report(store, fixture_options(4)), std::runtime_error);
}

TEST_CASE("write_report_file swaps the page in and leaves no .tmp behind") {
    const std::string path = testtemp::temp_path("report_file", ".html");

    hydra::app::write_report_file(path, "<html>old</html>");
    hydra::app::write_report_file(path, "<html>new</html>");

    std::ifstream in(path, std::ios::binary);
    std::ostringstream contents;
    contents << in.rdbuf();
    CHECK(contents.str() == "<html>new</html>");

    CHECK_FALSE(std::filesystem::exists(path + ".tmp"));

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

TEST_CASE("the fill page carries the shared stylesheet and script") {
    const std::string fill = fill_report::build_fill_html({}, "sub", "foot");
    CHECK(fill.find(html::kReportCss) != std::string::npos);
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
    // The shared script folds the query through that table, splits it into
    // words and keeps a row when every word is in its search text. It holds no
    // fold of its own: no lowercasing and no accented letter, raw or escaped.
    const std::string script = html::kReportJs;
    CHECK(script.find("FOLD[ch]") != std::string::npos);
    CHECK(script.find(".split(/\\s+/)") != std::string::npos);
    CHECK(script.find("words.every(w => r.search.includes(w))") != std::string::npos);
    CHECK(script.find("toLowerCase") == std::string::npos);
    CHECK(std::none_of(script.begin(), script.end(),
                       [](char c) { return static_cast<unsigned char>(c) >= 0x80; }));
    for (const char* range : {"\\u00", "\\u01", "\\uff", "\\uFF"})
        CHECK(script.find(range) == std::string::npos);
    CHECK(fill.find("<div class=\"wrap fill\">") != std::string::npos);

    // Every number on the page groups through the shared fmt, which uses one
    // fixed rule (1,234), whatever language the browser is set to (D48 Q12).
    // The single-owner scan keeps it that way.
    CHECK(std::string(html::kReportJsHead).find("n.toLocaleString('en-US')") !=
          std::string::npos);
    CHECK(fill.find(".toLocaleString()") == std::string::npos);
}

TEST_CASE("tier_for over a built table matches the window form") {
    const std::vector<TimingTier> tiers = timing_tiers(85.0);
    const std::vector<std::optional<double>> samples = {
        std::nullopt, 0.0, 1.9, 2.0, 42.5, 127.4, 169.9, 170.0, 500.0};
    for (const std::optional<double>& ms : samples)
        CHECK(report::tier_for(ms, tiers) == report::tier_for(ms, 85.0));
}

TEST_CASE("records_by_hash keys every listed record by its lower-case hash") {
    store::RecordStore store(":memory:");
    AnalysisSettings settings;
    settings.depth_value = 0;
    const AnalysisResult result = corpus::first_analyzed_with_paths(settings);
    store.rebuild_chart_library({library_entry("ABCDEF0123", "Title", "C:\\songs\\t.chart")});
    store.add_record(store::RecordKey{"ABCDEF0123", "mode", store::CapQuery::at(4)},
                     result.record);

    const std::unordered_map<std::string, store::RecordListing> by_hash =
        report::records_by_hash(store, "mode", store::CapQuery::at(4), store::Lens{});
    REQUIRE(by_hash.size() == 1);
    REQUIRE(by_hash.count("abcdef0123") == 1);
    CHECK(by_hash.at("abcdef0123").hyhash == "ABCDEF0123");
    CHECK(report::records_by_hash(store, "other mode", store::CapQuery::at(4),
                                  store::Lens{})
              .empty());
}

// The six path rows and four scores the samples page shows
// (tests/report_samples.h). The samples case writes them out and the
// path_tiles and dm_tiles cases are pinned on them.
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

// Not an invariant: writes a small fill comparison page into the folder named
// by HYDRA_PAGE_SAMPLES, built from fixed rows, so a page change can be
// checked in a real browser before and after (docs/adr/0016). Run it with
//   hydra_tests.exe --no-skip -tc="report pages: write samples*"
TEST_CASE("report pages: write samples for the browser check" * doctest::skip()) {
    const std::optional<std::string> dir = read_env("HYDRA_PAGE_SAMPLES");
    REQUIRE(dir.has_value());
    const std::filesystem::path out = std::filesystem::u8path(*dir);
    std::filesystem::create_directories(out);

    std::vector<fill_report::FillCompareRow> fill;
    auto add_fill = [&](const char* song, std::optional<int64_t> old_score,
                        std::optional<int64_t> new_score, std::optional<int64_t> delta,
                        const char* status) {
        fill_report::FillCompareRow r;
        r.song = song;
        r.artist = "Artist";
        r.charter = "Charter";
        r.hyhash = song;
        r.old_score = old_score;
        r.new_score = new_score;
        r.delta = delta;
        if (old_score) { r.old_path = "1-E2 0"; r.old_acts = 2; }
        if (new_score) { r.new_path = "1-E2 0-E1"; r.new_acts = 3; }
        r.notes = 900;
        r.status = status;
        fill.push_back(r);
    };
    add_fill("Song A", 100000, 100500, 500, "1.1 higher");
    add_fill("Song B", 100000, 99000, -1000, "1.0 higher");
    add_fill("Song C", 100000, 100000, 0, "same");
    add_fill("Song D", 100000, std::nullopt, std::nullopt, "only 1.0");
    add_fill("Song E", std::nullopt, 100000, std::nullopt, "only 1.1");

    write_report_file(out / "fill.html",
                      fill_report::build_fill_html(fill, "Sample subtitle", "Sample footer"));
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
    const std::string css = html::kReportCss;
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
}

TEST_CASE("report table header sticks and the page prints") {
    const std::string css = html::kReportCss;
    const size_t at = css.find(".tablewrap {");
    REQUIRE(at != std::string::npos);
    const std::string rule = css.substr(at, css.find('}', at) - at);
    // Scrolling both ways makes the wrapper the header's sticky container.
    CHECK(rule.find("overflow: auto;") != std::string::npos);
    CHECK(rule.find("max-height:") != std::string::npos);
    CHECK(rule.find("overflow-x") == std::string::npos);

    CHECK(css.find("@media print {") != std::string::npos);
    CHECK(css.find("thead { display: table-header-group; }") != std::string::npos);
    CHECK(css.find(".controls { display: none; }") != std::string::npos);
}

TEST_CASE("report pages number their rows in a # column") {
    // The shared script adds the column, so the fill page gets it.
    const std::string js = html::kReportJs;
    CHECK(js.find("th.textContent = '#';") != std::string::npos);
    CHECK(js.find("idx.textContent = fmt(++n);") != std::string::npos);
    // The sort control and the arrows walk the real columns, not the # one.
    CHECK(js.find("document.querySelectorAll('#head th.sortable')") != std::string::npos);
    CHECK(std::string(html::kReportCss).find("th.idx, td.idx {") != std::string::npos);
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

// D103 item 22: a settings change marks the path report out of date only when
// it moves a setting the report reads.
TEST_CASE("settings_change_touches: the path report reads the SP cap and the lens, not the mode") {
    const app::Settings before;
    CHECK_FALSE(app::report::settings_change_touches(before, before));

    // The chart mode alone: the report lists every mode.
    app::Settings mode_only = before;
    mode_only.view_difficulty = "Hard";
    mode_only.view_prodrums = false;
    REQUIRE(mode_only.chartmode_key() != before.chartmode_key());
    CHECK_FALSE(app::report::settings_change_touches(before, mode_only));

    app::Settings cap = before;
    cap.sp_cap = 5;
    CHECK(app::report::settings_change_touches(before, cap));

    app::Settings ms_limit = before;
    ms_limit.mslimit_enabled = !before.mslimit_enabled;
    REQUIRE(ms_limit.lens() != before.lens());
    CHECK(app::report::settings_change_touches(before, ms_limit));

    app::Settings fills = before;
    fills.legacy_fills = !before.legacy_fills;
    REQUIRE(fills.lens() != before.lens());
    CHECK(app::report::settings_change_touches(before, fills));

    // A setting no report reads.
    app::Settings auto_open = before;
    auto_open.auto_open_report = !before.auto_open_report;
    CHECK_FALSE(app::report::settings_change_touches(before, auto_open));
}

// The path report window's "Analyzing n of N records" reads these calls.
TEST_CASE("generate_report tells its progress chart by chart, from 0 to the whole") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    const std::vector<std::string> files = fill_store(store, library, 4, 3);
    REQUIRE(files.size() == 3);

    std::vector<std::pair<int, int>> calls;
    report::ReportOptions options = fixture_options(4);
    options.progress = [&](int done, int total) { calls.emplace_back(done, total); };
    const report::GeneratedReport page = report::generate_report(store, options);
    REQUIRE(page.rows > 0);
    REQUIRE_FALSE(calls.empty());
    CHECK(calls.front() == std::make_pair(0, 3));
    CHECK(calls.back() == std::make_pair(3, 3));
    for (size_t i = 1; i < calls.size(); ++i) {
        CHECK(calls[i].second == 3);
        CHECK(calls[i].first >= calls[i - 1].first);  // never goes back
    }

    // A batch's seed that holds every chart leaves nothing to analyze.
    report::ReportSeed seed = report::ReportSeed::for_run(fixture_settings(4).batch_run());
    CountingAnalyzer batch;
    BatchCallbacks callbacks;
    callbacks.analyze = batch.analyzer();
    callbacks.report_seed = &seed;
    run_batch(plan_batch(scan_items(library, library.size()), {}), fixture_settings(4).batch_run(), store, 2, callbacks);
    REQUIRE(seed.rows.size() == 3);
    calls.clear();
    report::generate_report(store, options, seed);
    CHECK(calls == std::vector<std::pair<int, int>>{{0, 0}});
}
