// Structural tests for app/report.{h,cpp}: collect_rows turns each stored
// record of the page's profile into its rows, and generate_report frames
// those rows with the subtitle and footer the report window shows. The tiles
// and the fill comparison page are tested in test_report_page.cpp.

#include "doctest.h"

#include <atomic>
#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/display_format.h"
#include "app/path_report_view.h"
#include "app/report.h"
#include "app/user_messages.h"
#include "core/model.h"
#include "core/squeeze_rating.h"
#include "core/strutil.h"
#include "corpus_util.h"
#include "record_fixtures.h"
#include "report_samples.h"
#include "search/graph.h"
#include "store/record_store.h"

using namespace hydra;
using namespace hydra::app;

namespace {

using report_samples::every_row;

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

// What a batch in these tests analyzes with: a real chart file as the app
// analyzes it, and kTiedFile as H1's tied-variant record, filed under the
// settings it is asked for.
AnalysisResult fixture_analyzer(const std::string& path, const AnalysisSettings& settings,
                                const std::function<void(float)>& on_progress) {
    if (path != kTiedFile) return analyze_chart_file(path, settings, on_progress);
    HydraRecord record = test::tied_variant_record();
    record.sp_cap = settings.sp_cap;
    record.legacy_fills = settings.legacy_fills;
    return AnalysisResult{std::move(record), test::beat_song({}, {}, 13440)};
}

// A report over the records filed under fixture_settings(cap, legacy_fills).
report::ReportOptions fixture_options(int cap, bool legacy_fills = false) {
    report::ReportOptions options;
    options.run = fixture_settings(cap, legacy_fills).batch_run();
    return options;
}

store::ChartLibraryEntry library_entry_for_file(const std::string& md5, const std::string& title,
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

// Files `record` for chart `song_id` through the one save, under `settings`'
// profile.
void save(store::RecordStore& store, const Settings& settings, const std::string& song_id,
          const HydraRecord& record) {
    store.save(store::prepare_record(song_id, settings.profile(), record));
}

// Analyzes the first `want` corpus charts with paths under
// fixture_settings(cap) into `store` as charts h0, h1, ... titled "Title 0",
// "Title 1", ..., and lists each in the library by its file (`library` keeps
// the rows, so a test can add more). Returns the files, h0's first.
std::vector<std::string> fill_store(store::RecordStore& store,
                                    std::vector<store::ChartLibraryEntry>& library, int cap,
                                    int want) {
    const Settings settings = fixture_settings(cap);
    std::vector<std::string> files;
    for (corpus::ChartWithPaths& c :
         corpus::charts_with_paths(settings.batch_run().settings, static_cast<size_t>(want))) {
        const std::string hyhash = "h" + std::to_string(files.size());
        const std::string title = "Title " + std::to_string(files.size());
        library.push_back(library_entry_for_file(hyhash, title, c.chart));
        files.push_back(c.chart);
    }
    store.rebuild_chart_library(library);
    for (size_t i = 0; i < files.size(); ++i) {
        const BatchRun run = settings.batch_run();
        save(store, settings, "h" + std::to_string(i),
             analyze_chart_file(files[i], run.settings).record);
    }
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
    std::vector<report::ReportRow> rows = report::collect_rows(store, options).rows;
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

// The fields that tell two report rows apart, so two reports compare row by
// row with tied-score rows free to swap places (the D87 rule).
using RowKey = std::tuple<std::string, std::string, int64_t>;
RowKey row_key(const report::ReportRow& r) { return {r.hyhash, r.path, r.score}; }

// The rows sorted by row_key.
std::vector<report::ReportRow> by_key(std::vector<report::ReportRow> rows) {
    std::stable_sort(rows.begin(), rows.end(),
                     [](const report::ReportRow& a, const report::ReportRow& b) {
                         return row_key(a) < row_key(b);
                     });
    return rows;
}

// Each chart's (score, rank) pairs, sorted: two rows tied at one score may
// swap ranks, and nothing else may move.
std::map<std::string, std::vector<std::pair<int64_t, int>>> ranks_by_chart(
    const std::vector<report::ReportRow>& rows) {
    std::map<std::string, std::vector<std::pair<int64_t, int>>> out;
    for (const report::ReportRow& r : rows) out[r.hyhash].emplace_back(r.score, r.rank);
    for (auto& [id, pairs] : out) std::sort(pairs.begin(), pairs.end());
    return out;
}

// Stores H1's tied-variant record for chart `hyhash` under
// fixture_settings(cap, legacy_fills), with a library row (kTiedFile), so the
// report has three paths to list: two tied at the top score and a lower one.
void store_tied(store::RecordStore& store, std::vector<store::ChartLibraryEntry>& library,
                const std::string& hyhash, int cap, bool legacy_fills = false) {
    library.push_back(library_entry_for_file(hyhash, "Tied " + hyhash, kTiedFile));
    store.rebuild_chart_library(library);
    HydraRecord record = test::tied_variant_record();
    record.sp_cap = cap;
    record.legacy_fills = legacy_fills;
    save(store, fixture_settings(cap, legacy_fills), hyhash, record);
}

}  // namespace

TEST_CASE("report rows: a tied top-score variant is optimal too") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    store_tied(store, library, "tied", 4);

    report::ReportOptions options = fixture_options(4);
    options.max_paths = 100;
    const std::vector<report::ReportRow> rows = report::collect_rows(store, options).rows;
    REQUIRE(rows.size() == 3);
    // Best score first: the root and its tied variant, then the lower root.
    CHECK(rows[0].score == rows[1].score);
    CHECK(rows[2].score < rows[1].score);
    CHECK(rows[0].optimal);
    CHECK(rows[1].optimal);
    CHECK_FALSE(rows[2].optimal);

    const report::GeneratedReport page = report::generate_report(store, fixture_options(4));
    // The subtitle still counts one chart, however many paths tie.
    CHECK(page.songs == 1);
    // The report hands both tied paths over as optimal, and the lower one not.
    REQUIRE(page.paths.size() == 3);
    CHECK(std::count_if(page.paths.begin(), page.paths.end(),
                        [](const report::ReportRow& r) { return r.optimal; }) == 2);
    CHECK_FALSE(page.paths[2].optimal);
}

TEST_CASE("report rows list every stored record (4 bars)") { check_cap(4); }

// The spec's row-by-row test, in its batch-then-read form: a batch saves the
// charts, the report reads them back, and every row matches the row a fresh
// analysis of the same chart under the same settings gives.
TEST_CASE("path report: a batch's records read back as a fresh analysis's rows") {
    store::RecordStore store(":memory:");
    const Settings settings = fixture_settings(4);
    const BatchRun run = settings.batch_run();

    std::vector<store::ChartLibraryEntry> library;
    for (corpus::ChartWithPaths& c : corpus::charts_with_paths(run.settings, 5)) {
        const std::string id = "h" + std::to_string(library.size());
        library.push_back(library_entry_for_file(id, "Title " + id, c.chart));
    }
    REQUIRE(library.size() == 5);
    library.push_back(library_entry_for_file("tied", "Tied", kTiedFile));
    store.rebuild_chart_library(library);

    BatchCallbacks callbacks;
    int errors = 0;
    callbacks.on_error = [&](const std::string&, const std::string&, const std::string&) {
        ++errors;
    };
    callbacks.analyze = fixture_analyzer;
    run_batch(plan_batch(library, {}), run, store, batch_worker_count(), callbacks);
    REQUIRE(errors == 0);

    report::ReportOptions options = fixture_options(4);
    const report::GeneratedReport page = report::generate_report(store, options);
    REQUIRE(page.empty_reason == report::EmptyReason::None);
    CHECK(page.songs == 6);

    // What a fresh analysis of each chart keeps, through the one save's own
    // turn of an analysis into a record: each kept path with its chart.
    struct Fresh {
        const store::ChartLibraryEntry* entry;
        int notes;
        store::PathRow path;
    };
    std::vector<Fresh> fresh;
    std::map<std::string, std::vector<std::pair<int64_t, int>>> fresh_ranks;
    for (const store::ChartLibraryEntry& e : library) {
        const store::RecordRow record =
            store::prepare_record(e.md5, run.profile(),
                                  fixture_analyzer(e.notespath, run.settings, {}).record)
                .record;
        for (const store::PathRow& p : record.paths) {
            if (p.rank > options.max_paths) break;
            fresh.push_back({&e, record.notecount, p});
            fresh_ranks[e.md5].emplace_back(p.score, p.rank);
        }
    }
    for (auto& [id, pairs] : fresh_ranks) std::sort(pairs.begin(), pairs.end());
    std::stable_sort(fresh.begin(), fresh.end(), [](const Fresh& a, const Fresh& b) {
        return RowKey{a.entry->md5, a.path.path, a.path.score} <
               RowKey{b.entry->md5, b.path.path, b.path.score};
    });

    // Field by field, each report row against the fresh path with its key.
    const std::vector<TimingTier> tiers = timing_tiers(options.hit_window_ms);
    const std::vector<report::ReportRow> got = by_key(page.paths);
    REQUIRE(got.size() == fresh.size());
    for (size_t i = 0; i < got.size(); ++i) {
        const report::ReportRow& g = got[i];
        const store::PathRow& p = fresh[i].path;
        const store::ChartLibraryEntry& e = *fresh[i].entry;
        INFO(e.md5 << " path " << p.path);
        CHECK(g.hyhash == e.md5);
        CHECK(g.song == display_title(e.title));
        CHECK(g.artist == display_artist(e.artist));
        CHECK(g.charter == display_charter(e.charter));
        CHECK(g.optimal == p.optimal);
        CHECK(g.path == p.path);
        CHECK(g.score == p.score);
        CHECK(g.acts == p.actcount);
        CHECK(g.skip == p.maxskip);
        CHECK(g.ms == p.hardest_ms);
        CHECK(g.efill == p.hardest_efill);
        CHECK(g.mult == p.avgmult);
        CHECK(g.sqin == p.sqin_count);
        CHECK(g.sqout == p.sqout_count);
        CHECK(g.notes == fresh[i].notes);
        CHECK(std::make_pair(g.tier, g.tok) == report::tier_for(p.hardest_ms, tiers));
    }
    // Ranks match, except that rows tied at one score may swap them.
    CHECK(ranks_by_chart(page.paths) == fresh_ranks);
}

// prepare_record numbers a record's paths in all_paths() order, so that order
// has to be best first already. This guards it at the report's own settings
// (fill_store's), where variants appear.
TEST_CASE("all_paths lists paths best first on every corpus chart") {
    AnalysisSettings settings;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 10;
    settings.sp_cap = 4;
    int charts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, settings.reading(), settings.rules);
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
    // row, so the page leaves it out.
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    REQUIRE(fill_store(store, library, 4, 2).size() == 2);
    store::ChartLibraryEntry first = library[0];
    store::ChartLibraryEntry copy = first;
    copy.rootfolder = "C:\\other";
    store.rebuild_chart_library({first, copy});

    const report::GeneratedReport out = report::generate_report(store, fixture_options(4));
    CHECK(out.songs == 2);
    CHECK(out.subtitle.rfind("2 charts — ", 0) == 0);
    // Every row is h0's; h1 is left out.
    REQUIRE_FALSE(out.paths.empty());
    for (const report::ReportRow& r : out.paths) {
        CHECK(r.hyhash == "h0");
        CHECK(r.song != "Title 1");
    }
}

// Two library copies of one chart count 2 on the header and on the Charts
// shown tile: both ask store::ChartIndex::count over the page's rows.
TEST_CASE("path report: two library copies count 2 in songs and in the Charts shown tile") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    REQUIRE(fill_store(store, library, 4, 1).size() == 1);
    store::ChartLibraryEntry copy = library[0];
    copy.rootfolder = "C:\\other";
    library.push_back(copy);
    store.rebuild_chart_library(library);

    const report::GeneratedReport out = report::generate_report(store, fixture_options(4));
    REQUIRE_FALSE(out.paths.empty());
    CHECK(out.songs == 2);
    CHECK(report::path_tiles(out.paths, every_row(out.paths.size()), out.hit_window_ms,
                             out.library)[0]
              .value == "2");
    // One shown row of the chart still counts both copies.
    CHECK(report::path_tiles(out.paths, {0}, out.hit_window_ms, out.library)[0].value == "2");
}

// The Charts shown tile and generate_report's subtitle both count through
// ChartIndex::count, so with every row shown the two totals agree.
TEST_CASE("path report: the Charts shown tile counts what generate_report counts") {
    store::RecordStore store(":memory:");
    fill_store_with_copies(store);

    const report::GeneratedReport out = report::generate_report(store, fixture_options(4));
    CHECK(out.songs == 5);
    CHECK(out.subtitle.rfind("5 charts — ", 0) == 0);

    size_t rows_h0 = 0, rows_h1 = 0;
    for (const report::ReportRow& r : out.paths) ++(r.hyhash == "h0" ? rows_h0 : rows_h1);
    REQUIRE(rows_h0 > 0);
    REQUIRE(rows_h1 > 0);
    CHECK(static_cast<int64_t>(rows_h0 + rows_h1) == out.rows);
    CHECK(report::path_tiles(out.paths, every_row(out.paths.size()), out.hit_window_ms,
                             out.library)[0]
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
    const std::vector<report::ReportRow> collected = report::collect_rows(store, options).rows;
    REQUIRE(collected.size() == out.paths.size());
    for (size_t i = 0; i < collected.size(); ++i) {
        CHECK(out.paths[i].hyhash == collected[i].hyhash);
        CHECK(out.paths[i].path == collected[i].path);
    }
    CHECK(out.hit_window_ms == 85.0);

    CHECK(out.subtitle.rfind("5 charts — ", 0) == 0);
    CHECK(out.footer.rfind("Generated from ", 0) == 0);
    CHECK(out.footer.find("'Beyond' means past the 170 ms window.") != std::string::npos);

    CHECK(report::path_tiles(out.paths, every_row(out.paths.size()), out.hit_window_ms,
                             out.library)[0]
              .value == "5");
}

TEST_CASE("report lists only the wanted cap and names it") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    const std::vector<std::string> files = fill_store(store, library, 4, 1);
    REQUIRE(files.size() == 1);
    // The same chart again at 8 bars.
    const BatchRun eight = fixture_settings(8).batch_run();
    save(store, fixture_settings(8), "h0", analyze_chart_file(files[0], eight.settings).record);
    REQUIRE(store.ready_records(fixture_settings(8).profile()).size() == 1);

    report::ReportOptions options = fixture_options(4);
    report::GeneratedReport four = report::generate_report(store, options);
    CHECK(hydra::ends_with(four.subtitle, "SP cap 4 bars"));
    // The subtitle counts what the report lists: the one chart at 4 bars, not
    // the 8-bar record the database also holds for the same chart.
    CHECK(four.songs == 1);
    CHECK(four.subtitle.rfind("1 chart — ", 0) == 0);
    int rank1 = 0;
    options.max_paths = 100;
    for (const report::ReportRow& row : report::collect_rows(store, options).rows)
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
    store.rebuild_chart_library({library_entry_for_file("h0", "Title 0", charts[0].chart)});
    save(store, legacy, "h0", charts[0].result.record);

    const report::GeneratedReport ch10 =
        report::generate_report(store, fixture_options(4, /*legacy_fills=*/true));
    CHECK(ch10.songs == 1);
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
    // test_song_input.cpp pins what those three do. This checks only that the
    // page calls them: one tagged name each. The page names a chart by its
    // library row (ChartIndex::naming), so a rescan's names replace
    // store_tied's own.
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    store_tied(store, library, "u0", 4);
    library[0].title = "<b>Bold Title</b>";
    library[0].artist = test::kTagOnlyTitle;
    library[0].charter = " <b>Bob</b> ";
    store.rebuild_chart_library(library);

    const std::vector<report::ReportRow> rows =
        report::collect_rows(store, fixture_options(4)).rows;
    REQUIRE(!rows.empty());
    for (const report::ReportRow& row : rows) {
        CHECK(row.song == "Bold Title");
        CHECK(row.artist == kUnknownTitle);
        CHECK(row.charter == "Bob");
    }
}

TEST_CASE("the report names a chart the scan found twice by its first copy") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    REQUIRE(fill_store(store, library, 4, 1).size() == 1);
    // The scan listed the real file first, as "Zed", then a copy of the same
    // md5 as "Alpha". Alpha sorts first by name, but the first row names it.
    library[0].title = "Zed";
    store::ChartLibraryEntry other = library[0];
    other.title = "Alpha";
    other.rootfolder = "C:\\other";
    store.rebuild_chart_library({library[0], other});

    const report::GeneratedReport page = report::generate_report(store, fixture_options(4));
    REQUIRE_FALSE(page.paths.empty());
    for (const report::ReportRow& r : page.paths) CHECK(r.song == "Zed");
    CHECK(page.songs == 2);  // both copies count (D76)
}

TEST_CASE("fill_rule_for: the legacy_fills flag names one fill rule") {
    // legacy_fills on is the Clone Hero 1.0 rule; off is the normal 1.1 rule.
    CHECK(fill_rule_for(true) == FillDeadlineRule::Ch10);
    CHECK(fill_rule_for(false) == FillDeadlineRule::Ch11);
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
    CHECK(result.rows >= added);

    // The framing strings are part of the interface: the GUI's ReportJob
    // ships exactly this subtitle and footer. The page lists one profile, and
    // the cut is per chart.
    CHECK(result.empty_reason == report::EmptyReason::None);
    CHECK(result.subtitle == counted(result.songs, "chart", "charts") +
                                 " — top 5 paths per chart — SP cap 4 bars");
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

// Finding 86.
TEST_CASE("generate_report: \"every path\" is the sentinel's, a huge --paths stays a count") {
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
    CHECK(subtitle.find("top 200,000,000 paths per chart") != std::string::npos);
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
    // Closing Hydra while the report builds: no page is framed.
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

TEST_CASE("a report with no analysis settings is refused") {
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    REQUIRE(fill_store(store, library, 4, 1).size() == 1);
    // Its run is what picks the records it lists.
    report::ReportOptions options = fixture_options(4);
    options.run.reset();
    CHECK_THROWS_AS(report::generate_report(store, options), std::invalid_argument);
}

// D89.
TEST_CASE("a report on results with no chart library says the library is missing") {
    // What hydra_batch with folder arguments leaves: results, and no scan
    // ever saved as the library. (A scan that lists nothing deletes the
    // results too, D87 item 4.)
    store::RecordStore store(":memory:");
    HydraRecord record = test::tied_variant_record();
    record.sp_cap = 4;
    save(store, fixture_settings(4), "t", record);

    const report::GeneratedReport page = report::generate_report(store, fixture_options(4));
    CHECK(page.rows == 0);
    CHECK(page.paths.empty());
    CHECK(page.empty_reason == report::EmptyReason::NoLibrary);
    CHECK(page.why_empty == std::string(report::kNoChartLibrary));
}

TEST_CASE("tier_for over a built table matches the window form") {
    const std::vector<TimingTier> tiers = timing_tiers(85.0);
    const std::vector<std::optional<double>> samples = {
        std::nullopt, 0.0, 1.9, 2.0, 42.5, 127.4, 169.9, 170.0, 500.0};
    for (const std::optional<double>& ms : samples)
        CHECK(report::tier_for(ms, tiers) == report::tier_for(ms, 85.0));
}

// The page lists the current profile only, so a change of the chart mode
// alone puts an open page out of date (plan question 2).
TEST_CASE("PageSettings::of differs when only the chart mode changes") {
    const app::Settings before;
    app::Settings mode_only = before;
    mode_only.view_difficulty = "Hard";
    mode_only.view_prodrums = false;
    REQUIRE(mode_only.chartmode_key() != before.chartmode_key());
    REQUIRE(mode_only.sp_cap == before.sp_cap);
    REQUIRE(mode_only.lens() == before.lens());
    REQUIRE(mode_only.hit_window_ms == before.hit_window_ms);
    CHECK(report::PageSettings::of(mode_only) != report::PageSettings::of(before));
    CHECK(report::settings_change_touches(before, mode_only));
}

// D103 item 22: a settings change marks the path report out of date only when
// it moves a setting the report reads.
TEST_CASE("settings_change_touches: the path report reads the SP cap and the lens") {
    const app::Settings before;
    CHECK_FALSE(app::report::settings_change_touches(before, before));

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

// The path report labels its timing tiers, its "Past N ms" tile and its
// footer from the hit window, so a new window makes an open page out of date.
TEST_CASE("settings_change_touches: the path report reads the hit window") {
    const app::Settings before;
    app::Settings window = before;
    window.hit_window_ms = before.hit_window_ms + 10.0;
    CHECK(app::report::settings_change_touches(before, window));
}
