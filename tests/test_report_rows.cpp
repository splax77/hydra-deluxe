// Structural tests for app/report.{h,cpp}: collect_rows turns each stored
// record of the page's profile into its rows, and generate_report frames
// those rows with the subtitle and footer the report window shows. The tiles
// and the fill comparison page are tested in test_report_page.cpp.

#include "doctest.h"

#include <atomic>
#include <algorithm>
#include <filesystem>
#include <fstream>
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
#include "app/library_query.h"  // fold_for_search
#include "app/path_report_view.h"
#include "app/report.h"
#include "app/user_messages.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "core/squeeze_rating.h"
#include "core/strutil.h"
#include "corpus_util.h"
#include "parse/song.h"  // NoNotesError
#include "dm_fixture.h"  // testdm::save
#include "record_fixtures.h"
#include "report_samples.h"
#include "search/graph.h"
#include "store/record_store.h"
#include "temp_util.h"

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
// chart file: fixture_analyzer hands back test::tied_variant_record() for it.
const char* const kTiedFile = "fixture:tied";

// What a batch in these tests analyzes with: a real chart file as the app
// analyzes it, and kTiedFile as test::tied_variant_record(), filed under the
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

// A second copy of the corpus chart `chart` in another folder, as a scan
// lists it: the chart file, and its song.ini when it has one, copied into the
// scratch folder testtemp::temp_dir(tag), and the copy's own scanned row
// titled `title` (corpus::scanned_entry). The scan's own hash of the copy
// must be the original's, or the two rows are not one chart.
store::ChartLibraryEntry scanned_copy(const std::string& chart, const std::string& tag,
                                      const std::string& title) {
    const size_t slash = chart.rfind('\\');
    REQUIRE(slash != std::string::npos);
    const std::string dir = testtemp::temp_dir(tag);
    const std::string copied = dir + chart.substr(slash);
    testtemp::write_bytes(copied, hydra::read_file_bytes(chart));
    const std::string ini = chart.substr(0, slash) + "\\song.ini";
    if (std::filesystem::exists(hydra::os_path(ini)))
        testtemp::write_bytes(dir + "\\song.ini", hydra::read_file_bytes(ini));
    store::ChartLibraryEntry e = corpus::scanned_entry(copied, title);
    REQUIRE(e.md5 == app::hash_chart_file(chart));
    REQUIRE(e.rootfolder != chart.substr(0, slash));
    return e;
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
        testdm::save(store, "h" + std::to_string(i), settings,
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

// Stores test::tied_variant_record() for chart `hyhash` under
// fixture_settings(cap, legacy_fills), with a library row (kTiedFile), so the
// report has three paths to list: two tied at the top score and a lower one.
// No case calls it now; C130 decides whether it stays.
[[maybe_unused]] void store_tied(store::RecordStore& store, std::vector<store::ChartLibraryEntry>& library,
                const std::string& hyhash, int cap, bool legacy_fills = false) {
    library.push_back(library_entry_for_file(hyhash, "Tied " + hyhash, kTiedFile));
    store.rebuild_chart_library(library);
    HydraRecord record = test::tied_variant_record();
    record.sp_cap = cap;
    record.legacy_fills = legacy_fills;
    testdm::save(store, hyhash, fixture_settings(cap, legacy_fills), record);
}

}  // namespace

// Every corpus chart with paths under the default settings, each listed once
// by its scanned row with its record saved: a row is optimal exactly when its
// score is its record's best. The case "HydraRecord::is_optimal: every corpus
// path tied at the top score" in test_model.cpp shows that under these same
// settings some corpus chart has a second path at the top score. all_paths
// lists paths best first, so that path sits among the kPathsPerRecord a record
// keeps.
TEST_CASE("report rows: a tied top-score variant is optimal too") {
    store::RecordStore store(":memory:");
    const Settings settings;
    const BatchRun run = settings.batch_run();
    std::vector<store::ChartLibraryEntry> library;
    std::vector<const HydraRecord*> records;
    std::map<std::string, int64_t> best_score;
    for (const std::string& chart : corpus::chart_paths()) {
        if (corpus::song(chart, run.settings.reading(), run.settings.rules).is_empty()) continue;
        const HydraRecord& record = corpus::analyzed(chart, run.settings);
        if (record.paths.empty()) continue;
        store::ChartLibraryEntry entry =
            corpus::scanned_entry(chart, "Title " + std::to_string(library.size()));
        // Two corpus files with one md5 are one chart: list it once.
        if (!best_score.emplace(entry.md5, record.best_path().totalscore()).second) continue;
        library.push_back(std::move(entry));
        records.push_back(&record);
    }
    REQUIRE(library.size() > 1);
    store.rebuild_chart_library(library);
    for (size_t i = 0; i < library.size(); ++i)
        testdm::save(store, library[i].md5, settings, *records[i]);

    report::ReportOptions options;
    options.run = run;
    options.max_paths = report::kEveryPathSentinel;
    const std::vector<report::ReportRow> rows = report::collect_rows(store, options).rows;
    std::map<std::string, int> optimal_rows;
    int below = 0;
    for (const report::ReportRow& r : rows) {
        INFO(r.hyhash << " path " << r.path);
        const auto best = best_score.find(r.hyhash);
        REQUIRE(best != best_score.end());
        CHECK(r.optimal == (r.score == best->second));
        if (r.optimal) ++optimal_rows[r.hyhash];
        else ++below;
    }
    // Both sides of the rule turn up: a chart with two optimal rows, and a
    // row below its chart's best.
    CHECK(std::any_of(optimal_rows.begin(), optimal_rows.end(),
                      [](const std::pair<const std::string, int>& c) { return c.second > 1; }));
    CHECK(below > 0);

    const report::GeneratedReport page = report::generate_report(store, options);
    // The subtitle counts each chart once, however many of its paths tie.
    CHECK(page.songs == static_cast<int64_t>(library.size()));
    // The report hands over the same optimal rows.
    REQUIRE(page.paths.size() == rows.size());
    const auto is_optimal = [](const report::ReportRow& r) { return r.optimal; };
    CHECK(std::count_if(page.paths.begin(), page.paths.end(), is_optimal) ==
          std::count_if(rows.begin(), rows.end(), is_optimal));
}

TEST_CASE("report rows list every stored record (4 bars)") { check_cap(4); }

// The row list holds no spare room once collect_rows returns, and the report
// hands that same list over (D282): five charts at one path each, then at
// every path.
TEST_CASE("collect_rows: the row list is sized to its rows") {
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store, 4, 5) == 5);
    for (const int64_t max_paths : {int64_t{1}, report::kEveryPathSentinel}) {
        INFO("max_paths " << max_paths);
        report::ReportOptions options = fixture_options(4);
        options.max_paths = max_paths;
        const report::CollectedRows collected = report::collect_rows(store, options);
        REQUIRE_FALSE(collected.rows.empty());
        CHECK(collected.rows.capacity() == collected.rows.size());
        const report::GeneratedReport page = report::generate_report(store, options);
        CHECK(page.paths.capacity() == page.paths.size());
    }
}

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
            store::prepare_record(e.md5, run.profile,
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
    const std::vector<TimingTier> tiers = timing_tiers();
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
        CHECK(g.ms == shown_timing(p.hardest_ms));
        CHECK(g.efill == p.hardest_efill);
        CHECK(g.mult == p.avgmult);
        CHECK(g.sqin == p.sqin_count);
        CHECK(g.sqout == p.sqout_count);
        CHECK(g.notes == fresh[i].notes);
        CHECK(std::make_pair(g.tier, g.tok) ==
              hydra::tier_for(shown_timing(p.hardest_ms), tiers));
    }
    // Ranks match, except that rows tied at one score may swap them.
    CHECK(ranks_by_chart(page.paths) == fresh_ranks);
}

// The store saves the engine's figure, sign included; the report shows it
// through shown_timing, so a squeeze-in inside Star Power reads as no timing.
// Foo Fighters - Everlong's best path under the default settings, "1 2 3+",
// has its only squeeze-in inside Star Power, so the engine's figure is below
// 0 (test_saved_figure_fill.cpp reads it there). Every row of its report is
// checked: a saved figure below 0 shows no timing and the none tier, and any
// other saved figure shows as shown_timing gives it.
TEST_CASE("path report: a saved figure below 0 shows no timing and the none tier") {
    store::RecordStore store(":memory:");
    const std::string chart = corpus::chart_named("Foo Fighters - Everlong");
    REQUIRE_FALSE(chart.empty());
    const store::ChartLibraryEntry entry = corpus::scanned_entry(chart, "Everlong");
    store.rebuild_chart_library({entry});
    const Settings settings;
    const BatchRun run = settings.batch_run();
    testdm::save(store, entry.md5, settings, analyze_chart_file(chart, run.settings).record);
    const std::vector<store::RecordLookup> saved = store.lookup({entry.md5}, run.profile);
    REQUIRE(saved.size() == 1);

    report::ReportOptions options;
    options.run = run;
    options.max_paths = report::kEveryPathSentinel;
    const std::vector<report::ReportRow> rows = report::collect_rows(store, options).rows;
    REQUIRE(rows.size() == saved[0].record.paths.size());
    const std::vector<TimingTier> tiers = timing_tiers();
    const TimingTier& none = hydra::none_tier(tiers);
    int below_zero = 0;
    for (const store::PathRow& p : saved[0].record.paths) {
        const auto row = std::find_if(rows.begin(), rows.end(), [&p](const report::ReportRow& r) {
            return r.path == p.path && r.rank == p.rank;
        });
        REQUIRE(row != rows.end());
        INFO("path " << p.path);
        CHECK(row->ms == shown_timing(p.hardest_ms));
        if (p.hardest_ms && *p.hardest_ms < 0.0) {
            ++below_zero;
            CHECK_FALSE(row->ms.has_value());
            CHECK(row->tier == none.name);
            CHECK(row->tok == none.tok);
        }
    }
    CHECK(below_zero > 0);
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
    // D76: the first chart sits in two library folders, the corpus's and a
    // real copy's, so it counts twice, like the library counts it. D87 item
    // 4: the second chart has a result but no library row, so the page leaves
    // it out.
    store::RecordStore store(":memory:");
    const Settings settings = fixture_settings(4);
    std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(settings.batch_run().settings, 2);
    REQUIRE(charts.size() == 2);
    const store::ChartLibraryEntry first = corpus::scanned_entry(charts[0].chart, "First");
    const store::ChartLibraryEntry copy = scanned_copy(charts[0].chart, "report_rows_copy", "First");
    const std::string dropped = app::hash_chart_file(charts[1].chart);
    REQUIRE_FALSE(dropped.empty());
    REQUIRE(dropped != first.md5);
    store.rebuild_chart_library({first, copy});
    testdm::save(store, first.md5, settings, charts[0].result.record);
    testdm::save(store, dropped, settings, charts[1].result.record);

    const report::GeneratedReport out = report::generate_report(store, fixture_options(4));
    CHECK(out.songs == 2);
    CHECK(out.subtitle.rfind(counted(2, "chart", "charts") + " — ", 0) == 0);
    // Every row is the first chart's; the dropped one is left out.
    REQUIRE_FALSE(out.paths.empty());
    for (const report::ReportRow& r : out.paths) {
        CHECK(r.hyhash == first.md5);
        CHECK(r.song == display_title(first.title));
    }
    // The report keeps no chart index (D282): each row carries its chart's
    // copies, so one shown row of the first chart still counts both copies,
    // and all its rows together count them once.
    REQUIRE(out.paths.size() > 1);
    for (const report::ReportRow& r : out.paths) CHECK(r.copies == 2);
    CHECK(report::path_tiles(out.paths, {0}, out.library_charts)[0].value ==
          "2 of 2");
    CHECK(report::path_tiles(out.paths, every_row(out.paths.size()), out.library_charts)[0]
              .value == "2 of 2");
}

// The Charts shown tile and generate_report's subtitle both count through
// store::count_charts, so with every row shown the two totals agree, and the
// tile's "of" reads the library count generate_report hands over.
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
    CHECK(out.library_charts == 5);
    CHECK(report::path_tiles(out.paths, every_row(out.paths.size()), out.library_charts)[0]
              .value == "5 of 5");
}

// What a window draws from: the rows and the subtitle and footer the page
// shows. With every row shown, the Charts shown tile reads the subtitle's
// chart count.
TEST_CASE("generate_report hands over its rows, subtitle and footer") {
    store::RecordStore store(":memory:");
    fill_store_with_copies(store);

    const report::ReportOptions options = fixture_options(4);
    const report::GeneratedReport out = report::generate_report(store, options);
    REQUIRE(out.rows > 0);
    CHECK(static_cast<int64_t>(out.paths.size()) == out.rows);
    const std::vector<report::ReportRow> collected = report::collect_rows(store, options).rows;
    REQUIRE(collected.size() == out.paths.size());
    for (size_t i = 0; i < collected.size(); ++i) {
        CHECK(out.paths[i].hyhash == collected[i].hyhash);
        CHECK(out.paths[i].path == collected[i].path);
    }

    CHECK(out.subtitle.rfind("5 charts — ", 0) == 0);
    CHECK(out.footer.rfind("Generated from ", 0) == 0);
    CHECK(out.footer.find("'Beyond' means past the 170 ms window.") != std::string::npos);

    CHECK(report::path_tiles(out.paths, every_row(out.paths.size()), out.library_charts)[0]
              .value == "5 of 5");
}

// ---- coverage: every library chart is shown, can't be analyzed, or has no
// result (D249 item 5) ----------

TEST_CASE("path report coverage: shown + can't analyze + no result is the library count (corpus)") {
    // Every corpus chart in the library, one of them twice. Each is analyzed
    // the way a batch analyzes it; a remembered failure is stored as the
    // chart's result, and every seventh chart is left unanalyzed. Hard, where
    // many corpus charts have no charting, so some charts can't be analyzed.
    store::RecordStore store(":memory:");
    Settings settings = fixture_settings(4);
    settings.view_difficulty = "Hard";
    const BatchRun run = settings.batch_run();
    std::vector<store::ChartLibraryEntry> library;
    for (const std::string& path : corpus::chart_paths())
        library.push_back(corpus::scanned_entry(path, "Title " + std::to_string(library.size())));
    REQUIRE(library.size() > 1);
    store::ChartLibraryEntry copy = library[0];
    copy.rootfolder = "C:\\other";
    library.push_back(copy);
    store.rebuild_chart_library(library);

    for (size_t i = 0; i + 1 < library.size(); ++i) {
        if (i % 7 == 6) continue;
        const std::string& id = library[i].md5;
        try {
            testdm::save(store, id, settings,
                         analyze_chart_file(library[i].notespath, run.settings).record);
        } catch (const KindedError& e) {
            if (remembered_failure(e.kind()))
                store.save(store::failure_record(id, run.profile, e.kind(), e.what()));
        } catch (const std::exception&) {
            // Not remembered: the chart stays without a result.
        }
    }

    report::ReportOptions options;
    options.run = run;
    const report::GeneratedReport page = report::generate_report(store, options);
    MESSAGE("library " << page.library_charts << ", shown " << page.songs << ", can't analyze "
                       << page.cant_analyze_charts << ", no result " << page.no_result_charts);
    CHECK(page.library_charts == static_cast<int64_t>(library.size()));
    // Each part is present, so the sum below cannot pass on an empty corpus.
    CHECK(page.songs > 0);
    CHECK(page.cant_analyze_charts > 0);
    CHECK(page.no_result_charts > 0);
    CHECK(page.songs + page.cant_analyze_charts + page.no_result_charts == page.library_charts);
    // Every stored failure is a missing Hard part, so the line names the part
    // NoNotesError's own text names, after the count.
    const std::optional<std::string> hard_part =
        no_notes_part(NoNotesError(Difficulty::Hard, true).what());
    REQUIRE(hard_part.has_value());
    CHECK(page.cant_analyze_line ==
          counted(page.cant_analyze_charts, "chart has", "charts have") + " no " + *hard_part);
    // The no-result line opens with its own count.
    CHECK(page.no_result_line.rfind(
              counted(page.no_result_charts, "chart has", "charts have"), 0) == 0);
    // The Charts shown tile reads the same two counts, with no filters.
    CHECK(report::path_tiles(page.paths, every_row(page.paths.size()), page.library_charts)[0]
              .value == std::to_string(page.songs) + " of " + std::to_string(page.library_charts));
}

namespace {

// The settings the coverage cases below report at: fixture_settings(4) read
// at Hard, where some corpus charts have no part, so their analysis fails the
// way a batch remembers (the corpus case above finds them the same way).
Settings hard_settings() {
    Settings s = fixture_settings(4);
    s.view_difficulty = "Hard";
    return s;
}

// A corpus chart whose Hard analysis threw a remembered failure.
struct HardFailure {
    std::string chart;
    ErrorKind kind = ErrorKind::NoNotes;
    std::string text;
};

// The first `want` corpus charts, in corpus order, whose Hard analysis
// throws a remembered failure; fewer when the corpus holds fewer.
std::vector<HardFailure> hard_failures(size_t want) {
    std::vector<HardFailure> found;
    const BatchRun run = hard_settings().batch_run();
    for (const std::string& path : corpus::chart_paths()) {
        if (found.size() >= want) break;
        try {
            analyze_chart_file(path, run.settings);
        } catch (const KindedError& e) {
            if (remembered_failure(e.kind())) found.push_back({path, e.kind(), e.what()});
        } catch (const std::exception&) {
            // Not remembered: a batch stores nothing for it.
        }
    }
    return found;
}

// Lists `failure`'s chart in the library under its scanned row titled
// `title`, and stores its failure under hard_settings(), as a batch does.
// Returns the library row.
store::ChartLibraryEntry store_hard_failure(store::RecordStore& store,
                                            std::vector<store::ChartLibraryEntry>& library,
                                            const HardFailure& failure,
                                            const std::string& title) {
    library.push_back(corpus::scanned_entry(failure.chart, title));
    store.rebuild_chart_library(library);
    store.save(store::failure_record(library.back().md5, hard_settings().profile(), failure.kind,
                                     failure.text));
    return library.back();
}

// Lists the first corpus chart that analyzes at Hard under its scanned row
// and saves its record under hard_settings(). Returns the library row.
store::ChartLibraryEntry store_hard_ready(store::RecordStore& store,
                                          std::vector<store::ChartLibraryEntry>& library) {
    const Settings settings = hard_settings();
    std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(settings.batch_run().settings, 1);
    REQUIRE(charts.size() == 1);
    library.push_back(corpus::scanned_entry(charts[0].chart, "Ready"));
    store.rebuild_chart_library(library);
    testdm::save(store, library.back().md5, settings, charts[0].result.record);
    return library.back();
}

// A report under hard_settings().
report::ReportOptions hard_options() {
    report::ReportOptions options;
    options.run = hard_settings().batch_run();
    return options;
}

// The coverage line's words for a NoNotes failure's text: the part its
// sentence names (no_notes_part, the sentence's one reader).
std::string no_part_words(const std::string& text) {
    const std::optional<std::string> part = no_notes_part(text);
    REQUIRE(part.has_value());
    return "no " + *part;
}

}  // namespace

TEST_CASE("path report coverage: a chart with a stored failure gets no row and one list entry") {
    const std::vector<HardFailure> failures = hard_failures(1);
    REQUIRE(failures.size() == 1);
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    const store::ChartLibraryEntry ok = store_hard_ready(store, library);
    const store::ChartLibraryEntry bad =
        store_hard_failure(store, library, failures[0], "<b>Bad</b> Song");

    const report::GeneratedReport page = report::generate_report(store, hard_options());
    REQUIRE(page.empty_reason == report::EmptyReason::None);
    REQUIRE_FALSE(page.paths.empty());
    for (const report::ReportRow& r : page.paths) CHECK(r.hyhash == ok.md5);
    CHECK(page.songs == 1);
    CHECK(page.library_charts == 2);
    CHECK(page.cant_analyze_charts == 1);
    CHECK(page.no_result_charts == 0);
    REQUIRE(page.cant_analyze.size() == 1);
    const report::CantAnalyzeGroup& group = page.cant_analyze[0];
    CHECK(group.sentence == plain_error(KindedError(failures[0].kind, failures[0].text)));
    CHECK(group.count == 1);
    REQUIRE(group.charts.size() == 1);
    CHECK(group.charts[0].song_id == bad.md5);
    // The library's names, cleaned the way every screen cleans them.
    CHECK(group.charts[0].title == display_title(bad.title));
    CHECK(group.charts[0].title != bad.title);
    CHECK(group.charts[0].artist == display_artist(bad.artist));
    CHECK(group.charts[0].kind == failures[0].kind);
    CHECK(group.charts[0].text == failures[0].text);
    CHECK(page.cant_analyze_line ==
          counted(1, "chart has", "charts have") + " " + no_part_words(failures[0].text));
    CHECK(page.no_result_line.empty());
    CHECK(report::path_tiles(page.paths, every_row(page.paths.size()), page.library_charts)[0]
              .value == "1 of 2");
}

TEST_CASE("path report coverage: the lines' words") {
    // Three real Hard failures: the corpus case above shows every remembered
    // Hard failure is a missing part, so all three share one reason.
    const std::vector<HardFailure> failures = hard_failures(3);
    REQUIRE(failures.size() == 3);
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    store_hard_ready(store, library);

    // Every failure shares one reason with a short form: the line names it.
    store_hard_failure(store, library, failures[0], "N1");
    store_hard_failure(store, library, failures[1], "N2");
    report::GeneratedReport page = report::generate_report(store, hard_options());
    CHECK(page.cant_analyze_line ==
          counted(2, "chart has", "charts have") + " " + no_part_words(failures[0].text));

    // A second reason: a chart file the reader cannot read. Its text is the
    // real reader's, from a file holding no chart.
    const std::string junk = testtemp::temp_path("report_rows_junk", ".chart");
    {
        std::ofstream f(junk, std::ios::binary);
        f << "this is not a chart\n";
    }
    std::optional<KindedError> unreadable;
    try {
        analyze_chart_file(junk, hard_settings().batch_run().settings);
    } catch (const KindedError& e) {
        unreadable.emplace(e.kind(), e.what());
    }
    REQUIRE(unreadable.has_value());
    REQUIRE(remembered_failure(unreadable->kind()));
    REQUIRE(unreadable->kind() != failures[0].kind);
    // The junk file's library row: a scan lists no unreadable file by its
    // md5, so the row is built here.
    library.push_back(library_entry_for_file("junk", "U1", junk));
    store.rebuild_chart_library(library);
    store.save(store::failure_record("junk", hard_settings().profile(), unreadable->kind(),
                                     unreadable->what()));
    const store::ChartLibraryEntry third = store_hard_failure(store, library, failures[2], "N3");

    // Two reasons: the generic words, and the Show list groups by reason.
    page = report::generate_report(store, hard_options());
    CHECK(page.cant_analyze_line == counted(4, "chart", "charts") + " can't be analyzed");
    REQUIRE(page.cant_analyze.size() == 2);
    const std::string no_notes_sentence =
        plain_error(KindedError(failures[0].kind, failures[0].text));
    const std::string junk_sentence = plain_error(*unreadable);
    const auto group_of = [&page](const std::string& sentence) {
        const auto it = std::find_if(
            page.cant_analyze.begin(), page.cant_analyze.end(),
            [&sentence](const report::CantAnalyzeGroup& g) { return g.sentence == sentence; });
        REQUIRE(it != page.cant_analyze.end());
        return *it;
    };
    const report::CantAnalyzeGroup missing = group_of(no_notes_sentence);
    CHECK(missing.count == 3);
    REQUIRE(missing.charts.size() == 3);
    CHECK(std::any_of(missing.charts.begin(), missing.charts.end(),
                      [&third](const report::CantAnalyzeChart& c) {
                          return c.song_id == third.md5;
                      }));
    const report::CantAnalyzeGroup junk_group = group_of(junk_sentence);
    CHECK(junk_group.count == 1);
    REQUIRE(junk_group.charts.size() == 1);
    CHECK(junk_group.charts[0].song_id == "junk");

    // One reason with no short form: the generic words, singular for 1.
    store::RecordStore lone(":memory:");
    std::vector<store::ChartLibraryEntry> lone_library;
    store_hard_ready(lone, lone_library);
    lone_library.push_back(library_entry_for_file("junk", "U1", junk));
    lone.rebuild_chart_library(lone_library);
    lone.save(store::failure_record("junk", hard_settings().profile(), unreadable->kind(),
                                    unreadable->what()));
    CHECK(report::generate_report(lone, hard_options()).cant_analyze_line ==
          counted(1, "chart", "charts") + " can't be analyzed");

    // The no-result line: a corpus chart listed with no record, then two.
    // Each line opens with its own count, and the four counts add up.
    const std::vector<std::string>& paths = corpus::chart_paths();
    std::vector<std::string> unlisted;
    for (const std::string& p : paths) {
        const std::string md5 = app::hash_chart_file(p);
        if (md5.empty()) continue;
        if (std::none_of(library.begin(), library.end(),
                         [&md5](const store::ChartLibraryEntry& e) { return e.md5 == md5; }))
            unlisted.push_back(p);
        if (unlisted.size() == 2) break;
    }
    REQUIRE(unlisted.size() == 2);
    library.push_back(corpus::scanned_entry(unlisted[0], "None 1"));
    store.rebuild_chart_library(library);
    page = report::generate_report(store, hard_options());
    CHECK(page.no_result_charts == 1);
    CHECK(page.no_result_line.rfind(counted(1, "chart has", "charts have"), 0) == 0);
    library.push_back(corpus::scanned_entry(unlisted[1], "None 2"));
    store.rebuild_chart_library(library);
    page = report::generate_report(store, hard_options());
    CHECK(page.no_result_charts == 2);
    CHECK(page.no_result_line.rfind(counted(2, "chart has", "charts have"), 0) == 0);
    CHECK(page.songs + page.cant_analyze_charts + page.no_result_charts == page.library_charts);
}

TEST_CASE("path report coverage: no Ready chart but a stored failure reads Nothing to show") {
    const std::vector<HardFailure> failures = hard_failures(1);
    REQUIRE(failures.size() == 1);
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> library;
    store_hard_failure(store, library, failures[0], "Bad");
    // A second corpus chart, listed with no result.
    std::string unlisted;
    for (const std::string& p : corpus::chart_paths()) {
        const std::string md5 = app::hash_chart_file(p);
        if (!md5.empty() && md5 != library[0].md5) {
            unlisted = p;
            break;
        }
    }
    REQUIRE_FALSE(unlisted.empty());
    library.push_back(corpus::scanned_entry(unlisted, "None"));
    store.rebuild_chart_library(library);

    const report::GeneratedReport page = report::generate_report(store, hard_options());
    CHECK(page.paths.empty());
    CHECK(page.empty_reason == report::EmptyReason::NothingToShow);
    CHECK(page.why_empty == report::kNothingToShow);
    // The lines still say why.
    CHECK(page.cant_analyze_line ==
          counted(1, "chart has", "charts have") + " " + no_part_words(failures[0].text));
    CHECK(page.no_result_line.rfind(counted(1, "chart has", "charts have"), 0) == 0);
    CHECK(page.library_charts == 2);
}

TEST_CASE("report lists only the wanted cap and names it") {
    store::RecordStore store(":memory:");
    std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(fixture_settings(4).batch_run().settings, 1);
    REQUIRE(charts.size() == 1);
    const store::ChartLibraryEntry entry = corpus::scanned_entry(charts[0].chart, "Title 0");
    store.rebuild_chart_library({entry});
    testdm::save(store, entry.md5, fixture_settings(4), charts[0].result.record);
    // The same chart analyzed again under fixture_settings(cap, legacy_fills).
    const auto save_at = [&store, &entry](int cap, bool legacy_fills) {
        const Settings settings = fixture_settings(cap, legacy_fills);
        const AnalysisResult result =
            analyze_chart_file(entry.notespath, settings.batch_run().settings);
        REQUIRE_FALSE(result.record.paths.empty());
        testdm::save(store, entry.md5, settings, result.record);
    };
    save_at(8, false);
    REQUIRE(store.ready_records(fixture_settings(8).profile()).size() == 1);

    report::ReportOptions options = fixture_options(4);
    report::GeneratedReport four = report::generate_report(store, options);
    CHECK(hydra::ends_with(four.subtitle, "SP cap " + bars_text(4)));
    // The subtitle counts what the report lists: the one chart at 4 bars, not
    // the 8-bar record the database also holds for the same chart.
    CHECK(four.songs == 1);
    CHECK(four.subtitle.rfind(counted(1, "chart", "charts") + " — ", 0) == 0);
    int rank1 = 0;
    options.max_paths = 100;
    for (const report::ReportRow& row : report::collect_rows(store, options).rows)
        if (row.rank == 1) ++rank1;
    CHECK(rank1 == 1);

    // The cap reads through the house count rule (bars_text, D48 Q12), at
    // the one-bar and the thousand-bar ends.
    for (const int cap : {1, 1000}) {
        INFO("cap " << cap);
        save_at(cap, false);
        CHECK(hydra::ends_with(report::generate_report(store, fixture_options(cap)).subtitle,
                               "SP cap " + bars_text(cap)));
    }

    // A 1.0 report names its rule by the fill rule's one long name.
    save_at(4, true);
    CHECK(hydra::ends_with(report::generate_report(store, fixture_options(4, /*legacy_fills=*/true))
                               .subtitle,
                           "SP cap " + bars_text(4) + " — " +
                               fill_rule_name(FillDeadlineRule::Ch10, FillRuleNameStyle::Long) +
                               " fills"));
}

// A database filled under the 1.0 rule, as hydra_batch --legacy-fills leaves
// one, reports its chart under the 1.0 rule when the report is asked for 1.0
// fills, and lists nothing under the 1.1 rule.
TEST_CASE("generate_report reports a 1.0-fills database under the 1.0 rule") {
    store::RecordStore store(":memory:");
    const Settings legacy = fixture_settings(4, /*legacy_fills=*/true);
    std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(legacy.batch_run().settings, 1);
    REQUIRE(charts.size() == 1);
    const store::ChartLibraryEntry entry = corpus::scanned_entry(charts[0].chart, "Title 0");
    store.rebuild_chart_library({entry});
    testdm::save(store, entry.md5, legacy, charts[0].result.record);

    const report::GeneratedReport ch10 =
        report::generate_report(store, fixture_options(4, /*legacy_fills=*/true));
    CHECK(ch10.songs == 1);
    REQUIRE_FALSE(ch10.paths.empty());
    for (const report::ReportRow& r : ch10.paths) CHECK(r.hyhash == entry.md5);
    // The subtitle names the 1.0 rule by the fill rule's one long name.
    CHECK(hydra::ends_with(ch10.subtitle,
                           std::string(fill_rule_name(FillDeadlineRule::Ch10,
                                                      FillRuleNameStyle::Long)) +
                               " fills"));

    // Under the 1.1 rule the page is empty, and its sentence is the one
    // nothing_under_settings builds from the page's own profile and rule.
    const report::GeneratedReport ch11 = report::generate_report(store, fixture_options(4));
    CHECK(ch11.paths.empty());
    CHECK(ch11.empty_reason == report::EmptyReason::NothingUnderSettings);
    CHECK(ch11.why_empty ==
          report::nothing_under_settings(
              fixture_settings(4).profile().chartmode, 4,
              std::string(fill_rule_name(FillDeadlineRule::Ch11, FillRuleNameStyle::Long)) +
                  " fills"));
}

TEST_CASE("collect_rows: the library names go through display_title, display_artist and "
          "display_charter") {
    // test_song_input.cpp pins what those three do. This checks only that the
    // page calls them: one tagged name each, on a corpus chart's scanned row
    // over its real record. The page names a chart by its library row
    // (ChartIndex::naming).
    store::RecordStore store(":memory:");
    std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(fixture_settings(4).batch_run().settings, 1);
    REQUIRE(charts.size() == 1);
    store::ChartLibraryEntry entry = corpus::scanned_entry(charts[0].chart, "<b>Bold Title</b>");
    entry.artist = test::kTagOnlyTitle;
    entry.charter = " <b>Bob</b> ";
    store.rebuild_chart_library({entry});
    testdm::save(store, entry.md5, fixture_settings(4), charts[0].result.record);

    const std::vector<report::ReportRow> rows =
        report::collect_rows(store, fixture_options(4)).rows;
    REQUIRE(!rows.empty());
    for (const report::ReportRow& row : rows) {
        CHECK(row.song == display_title(entry.title));
        CHECK(row.artist == display_artist(entry.artist));
        CHECK(row.charter == display_charter(entry.charter));
        // Each name is changed by its cleaner, so a page that skipped one
        // would show the raw name and fail here.
        CHECK(row.song != entry.title);
        CHECK(row.artist != entry.artist);
        CHECK(row.charter != entry.charter);
    }
}

TEST_CASE("the report names a chart the scan found twice by its first copy") {
    store::RecordStore store(":memory:");
    const Settings settings = fixture_settings(4);
    std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(settings.batch_run().settings, 1);
    REQUIRE(charts.size() == 1);
    // The scan listed the corpus file first, as "Zed", then a real copy of it
    // in another folder as "Alpha". Alpha sorts first by name, but the first
    // row names the chart.
    const store::ChartLibraryEntry zed = corpus::scanned_entry(charts[0].chart, "Zed");
    const store::ChartLibraryEntry alpha =
        scanned_copy(charts[0].chart, "report_rows_first_copy", "Alpha");
    store.rebuild_chart_library({zed, alpha});
    testdm::save(store, zed.md5, settings, charts[0].result.record);

    const report::GeneratedReport page = report::generate_report(store, fixture_options(4));
    REQUIRE_FALSE(page.paths.empty());
    for (const report::ReportRow& r : page.paths) CHECK(r.song == display_title(zed.title));
    CHECK(page.songs == 2);  // both copies count (D76)
}

TEST_CASE("fill_rule_for: the legacy_fills flag names one fill rule") {
    // legacy_fills on is the Clone Hero 1.0 rule; off is the normal 1.1 rule.
    CHECK(fill_rule_for(true) == FillDeadlineRule::Ch10);
    CHECK(fill_rule_for(false) == FillDeadlineRule::Ch11);
}

TEST_CASE("tier_for: raw-ms bands derived from the two-hit budget") {
    using report::tier_for;

    // The names and edges come from their owner, timing_tiers(). The ladder
    // opens at the difficult floor past_difficult_floor tests and closes at
    // the two-hit budget (nominal_budget_ms); each edge belongs to the band
    // below it (D48 Q3).
    const std::vector<TimingTier> tiers = timing_tiers();
    REQUIRE(tiers.size() >= 3);
    const TimingTier& none = hydra::none_tier(tiers);
    const TimingTier& beyond = hydra::beyond_tier(tiers);
    CHECK(tier_for(std::nullopt) == std::make_pair(std::string(none.name), std::string(none.tok)));

    // The first band ends on the difficult floor: on it is not past it.
    REQUIRE(tiers.front().cutoff.has_value());
    const double difficult_floor = *tiers.front().cutoff;
    CHECK_FALSE(past_difficult_floor(difficult_floor));
    CHECK(past_difficult_floor(difficult_floor + 0.01));
    CHECK(tier_for(difficult_floor).first == tiers.front().name);
    CHECK(tier_for(difficult_floor + 0.01).first != tiers.front().name);

    // The last banded edge is the two-hit budget: on it is still inside,
    // just past it reads Beyond.
    std::optional<double> last_edge;
    std::string last_name;
    for (const TimingTier& t : tiers) {
        if (!t.cutoff) continue;
        if (last_edge) CHECK(*t.cutoff > *last_edge);
        last_edge = t.cutoff;
        last_name = t.name;
    }
    REQUIRE(last_edge.has_value());
    CHECK(*last_edge == nominal_budget_ms());
    CHECK(tier_for(*last_edge).first == last_name);
    CHECK(tier_for(*last_edge + 0.01) ==
          std::make_pair(std::string(beyond.name), std::string(beyond.tok)));
}

TEST_CASE("tier_for walks the timing_tiers table edge by edge") {
    // tier_for and the page's embedded tier table read the one ladder in
    // core/squeeze_rating.h, so every banded entry's cutoff is exactly where
    // the label flips to the next entry's: the cutoff itself still reads the
    // entry's own name (D48 Q3), and just past it reads the next one.
    std::vector<TimingTier> tiers = timing_tiers();
    REQUIRE(tiers.size() >= 2);
    for (size_t i = 0; i + 1 < tiers.size(); ++i) {
        if (!tiers[i].cutoff) continue;
        const double cutoff = *tiers[i].cutoff;
        CHECK(report::tier_for(cutoff).first == tiers[i].name);
        CHECK(report::tier_for(cutoff + 0.01).first == tiers[i + 1].name);
    }
}

// A corpus chart's real rows, listed under names with a tag and an accent:
// each row's search text is already folded and holds no tag, and a search for
// any shown name or the path finds the row.
TEST_CASE("path report: a row's search text is folded and tag-free") {
    store::RecordStore store(":memory:");
    std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(fixture_settings(4).batch_run().settings, 1);
    REQUIRE(charts.size() == 1);
    store::ChartLibraryEntry entry = corpus::scanned_entry(charts[0].chart, "<i>Halo</i>");
    entry.artist = "Beyonc\xc3\xa9";  // Beyoncé
    entry.charter = "<b>Bob</b>";
    store.rebuild_chart_library({entry});
    testdm::save(store, entry.md5, fixture_settings(4), charts[0].result.record);

    const std::vector<report::ReportRow> rows =
        report::collect_rows(store, fixture_options(4)).rows;
    REQUIRE_FALSE(rows.empty());
    for (const report::ReportRow& row : rows) {
        INFO("path " << row.path);
        const std::string text = path_report_view::path_search_text(row);
        // Folded: folding it again changes nothing, and it differs from the
        // shown artist, which the fold changes.
        CHECK(fold_for_search(text) == text);
        CHECK(text.find(row.artist) == std::string::npos);
        // Tag-free: no tag's opening bracket is left.
        CHECK(text.find('<') == std::string::npos);
        for (const std::string& shown : {row.song, row.artist, row.charter, row.path}) {
            INFO("search for " << shown);
            CHECK(report_view::search_matches(report_view::search_words(shown), text));
        }
    }
}

TEST_CASE("generate_report: one seam frames the report for every entry point") {
    store::RecordStore store(":memory:");
    const int added = fill_store(store, 4, 2);
    REQUIRE(added > 0);

    report::ReportOptions options = fixture_options(4);
    options.max_paths = store::kPathsPerRecord;
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
    // D50 item 1. The 85 ms hit window puts Beyond past 170 ms, the same
    // number the Beyond chip and the "Past 170 ms" tile print. D278 item 4:
    // the footer names the window's number.
    CHECK(result.footer ==
          "Generated from hydra.db. Timing tiers measure how big each "
          "squeeze is, in steps of the 85 ms hit window. The Paths tab's row "
          "labels measure how far a hit lands from the Star Power end, so the "
          "two can differ. 'Beyond' means past the 170 ms window.");

    // The every-path wording.
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
        "Nothing is analyzed under these settings (Expert Pro Drums, 2x Bass, SP cap 8, "
        "Clone Hero 1.1 fills). Analyze with these settings, or change them.";
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

    // Only the sentinel reads "every path".
    report::ReportOptions options = fixture_options(4);
    options.max_paths = report::kEveryPathSentinel;
    const std::string every = report::generate_report(store, options).subtitle;
    CHECK(every.find(" — every path — ") != std::string::npos);
    // The sentinel is never printed as a count.
    CHECK(every.find(counted(report::kEveryPathSentinel, "path", "paths")) == std::string::npos);

    // A large max_paths below it lists at most that many, so it names that
    // count through the house count rule (hydra::counted).
    options.max_paths = report::kEveryPathSentinel - 1;
    const std::string subtitle = report::generate_report(store, options).subtitle;
    CHECK(subtitle.find(counted(options.max_paths, "path", "paths")) != std::string::npos);
    CHECK(subtitle.find(" — every path") == std::string::npos);
}

TEST_CASE("nothing_under_settings frames the chart mode, the cap and the fills") {
    // One frame for every empty page (finding 105, D50 item 3): the chart mode
    // comes first. The path report passes its fill rule, the fill comparison
    // no fills.
    CHECK(report::nothing_under_settings("Expert Pro Drums, 2x Bass", 8, "Clone Hero 1.1 fills") ==
          "Nothing is analyzed under these settings (Expert Pro Drums, 2x Bass, SP cap 8, "
          "Clone Hero 1.1 fills). Analyze with these settings, or change them.");
    CHECK(report::nothing_under_settings("Expert Pro Drums, 2x Bass", 4) ==
          "Nothing is analyzed under these settings (Expert Pro Drums, 2x Bass, SP cap 4). "
          "Analyze with these settings, or change them.");
    CHECK(report::nothing_under_settings("Hard Drums, 1x Bass", 1200, "Clone Hero 1.0 fills") ==
          "Nothing is analyzed under these settings (Hard Drums, 1x Bass, SP cap 1,200, "
          "Clone Hero 1.0 fills). Analyze with these settings, or change them.");
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
    testdm::save(store, "t", fixture_settings(4), record);

    const report::GeneratedReport page = report::generate_report(store, fixture_options(4));
    CHECK(page.rows == 0);
    CHECK(page.paths.empty());
    CHECK(page.empty_reason == report::EmptyReason::NoLibrary);
    CHECK(page.why_empty == std::string(report::kNoChartLibrary));
    // The ruled words, as a literal, so a change to the constant fails here
    // (D305 item 5).
    CHECK(page.why_empty ==
          "This database has no chart library. Run hydra_batch --whole-library, or scan in Hydra, "
          "to build one.");
}

TEST_CASE("tier_for over a built table matches the one-argument form") {
    const std::vector<TimingTier> tiers = timing_tiers();
    const std::vector<std::optional<double>> samples = {
        std::nullopt, 0.0, 1.9, 2.0, 42.5, 127.4, 169.9, 170.0, 500.0};
    for (const std::optional<double>& ms : samples)
        CHECK(hydra::tier_for(ms, tiers) == report::tier_for(ms));
}

// The page lists the current profile only, so a change of the chart mode
// alone puts an open page out of date (question 2 of
// docs/superpowers/plans/2026-10-12-one-record-per-chart/part-A2-report-callers.md (removed; `git show 444631e6:docs/superpowers/plans/2026-10-12-one-record-per-chart/part-A2-report-callers.md`)).
TEST_CASE("PageSettings::of differs when only the chart mode changes") {
    const app::Settings before;
    app::Settings mode_only = before;
    mode_only.view_difficulty = "Hard";
    mode_only.view_prodrums = false;
    REQUIRE(mode_only.chartmode_key() != before.chartmode_key());
    REQUIRE(mode_only.sp_cap == before.sp_cap);
    REQUIRE(mode_only.lens() == before.lens());
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
