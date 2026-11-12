// The one-time fill of the saved hardest figure (app::fill_saved_hardest):
// rows an older build saved with an empty figure get the engine's figure back
// by replaying their saved path on the chart. Each case saves a real corpus
// analysis as an older build left it, with the figure of the rows named
// emptied, then runs the fill. Every figure and path text below is pinned
// from one run.

#include "doctest.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/saved_figure_fill.h"
#include "core/model.h"  // shown_timing
#include "corpus_util.h"
#include "db_file_util.h"     // scalar_on_file
#include "display_fixtures.h"  // save_old_build_record
#include "search/graph.h"      // ScoreGraph
#include "search/pathstring_read.h"
#include "store/record_store.h"
#include "store_fixtures.h"  // save_as_old_build, saved_figures
#include "temp_util.h"       // ScopedFile

using hydra::app::Settings;
using hydra::store::ChartLibraryEntry;
using hydra::store::PreparedRecord;
using hydra::store::RecordStore;

namespace {

// The corpus chart whose best path, "1 0+ 0 0", carries a squeeze-in at
// exactly 0.0 ms.
const char* const kSmashMouth = "Smash Mouth - I_m A Believer";
// The corpus chart whose best path's only squeeze-in is inside Star Power.
const char* const kEverlong = "Foo Fighters - Everlong";

// The running settings every case fills under: the defaults.
const Settings& fill_settings() {
    static const Settings s;
    return s;
}

// A corpus chart's file, and the library row a scan writes for it.
struct CorpusChart {
    std::string file;
    ChartLibraryEntry entry;
};

// The corpus chart whose path holds `name`.
CorpusChart named_chart(const std::string& name) {
    const std::string chart = corpus::chart_named(name);
    REQUIRE_FALSE(chart.empty());
    return {chart, corpus::scanned_entry(chart, name)};
}

// Saves `chart` as an older build left it (hydra::test::save_as_old_build)
// under the running settings.
void save_old(RecordStore& store, const CorpusChart& chart, const std::vector<int>& emptied,
              const std::function<void(PreparedRecord&)>& edit = {}) {
    hydra::test::save_as_old_build(store, chart.file, chart.entry, fill_settings(), emptied,
                                   edit);
}

// The saved figure of every rank of chart `md5`'s record, in rank order.
std::vector<std::optional<double>> figures_of(RecordStore& store, const std::string& md5) {
    return hydra::test::saved_figures(store, md5, fill_settings().profile());
}

// A progress callback that counts its calls and the charts it was told of,
// and answers stop at call `stop_at` (0 = never).
struct Counter {
    int calls = 0;
    uint64_t charts_total = 0;
    int stop_at = 0;
    hydra::app::SavedFigureProgress fn() {
        return [this](uint64_t, uint64_t total) {
            ++calls;
            charts_total = total;
            return calls != stop_at;
        };
    }
};

}  // namespace

TEST_CASE("a saved path with a squeeze-in on the SP end gets its figure back") {
    RecordStore store(":memory:");
    const CorpusChart chart = named_chart(kSmashMouth);
    const ChartLibraryEntry& entry = chart.entry;
    save_old(store, chart, {1, 2, 3});
    const std::vector<std::optional<double>> before = figures_of(store, entry.md5);
    REQUIRE(before.size() == 5);
    CHECK_FALSE(before[0].has_value());
    CHECK_FALSE(before[1].has_value());
    CHECK_FALSE(before[2].has_value());

    Counter first;
    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), first.fn()));
    // One chart: reported before it and once it is done.
    CHECK(first.calls == 2);
    CHECK(first.charts_total == 1);
    const hydra::store::RecordLookup found = store.lookup_one(entry.md5, fill_settings().profile());
    CHECK(found.record.paths[0].path == "1 0+ 0 0");
    CHECK(figures_of(store, entry.md5) ==
          std::vector<std::optional<double>>{0.0, 0.0, 0.0, std::nullopt, 0.0});
    CHECK(store.hardest_figure_filled(fill_settings().profile()));

    // The marker is set, so a second run reads no chart.
    Counter second;
    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), second.fn()));
    CHECK(second.calls == 0);
}

TEST_CASE("a saved path whose chart is gone keeps its empty figure") {
    RecordStore store(":memory:");
    CorpusChart chart = named_chart(kSmashMouth);
    ChartLibraryEntry& entry = chart.entry;
    entry.notespath = entry.rootfolder + "\\no such folder\\notes.chart";
    save_old(store, chart, {1});

    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), {}));
    CHECK_FALSE(figures_of(store, entry.md5)[0].has_value());
    CHECK(store.hardest_figure_filled(fill_settings().profile()));
}

TEST_CASE("a row that disagrees with its replay keeps its empty figure") {
    RecordStore store(":memory:");
    const CorpusChart chart = named_chart(kSmashMouth);
    const ChartLibraryEntry& entry = chart.entry;
    save_old(store, chart, {1, 2}, [](PreparedRecord& p) { p.record.paths[0].score += 1; });

    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), {}));
    const std::vector<std::optional<double>> after = figures_of(store, entry.md5);
    // Rank 1's score no longer matches its replay; rank 2 still does.
    CHECK_FALSE(after[0].has_value());
    CHECK(after[1] == std::optional<double>(0.0));
    CHECK(store.hardest_figure_filled(fill_settings().profile()));
}

TEST_CASE("a stop leaves the marker unset") {
    RecordStore store(":memory:");
    const CorpusChart chart = named_chart(kSmashMouth);
    const ChartLibraryEntry& entry = chart.entry;
    save_old(store, chart, {1});

    Counter stopped;
    stopped.stop_at = 1;
    CHECK_FALSE(hydra::app::fill_saved_hardest(store, fill_settings(), stopped.fn()));
    CHECK(stopped.calls == 1);
    CHECK_FALSE(store.hardest_figure_filled(fill_settings().profile()));
    CHECK_FALSE(figures_of(store, entry.md5)[0].has_value());

    // The next run goes on and finishes.
    Counter next;
    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), next.fn()));
    CHECK(next.charts_total == 1);
    CHECK(figures_of(store, entry.md5)[0] == std::optional<double>(0.0));
    CHECK(store.hardest_figure_filled(fill_settings().profile()));
}

TEST_CASE("a chart whose replay throws keeps its rows empty, and the fill goes on") {
    // Two charts in one library, so save_as_old_build (which lists one chart
    // as the whole library) is not used: Everlong's ranks 1 and 2 and Smash
    // Mouth's rank 1 are saved with an empty figure.
    RecordStore store(":memory:");
    const CorpusChart throws = named_chart(kEverlong);
    const CorpusChart kept = named_chart(kSmashMouth);
    store.rebuild_chart_library({throws.entry, kept.entry});
    const auto save = [&](const CorpusChart& chart, const std::vector<int>& emptied) {
        PreparedRecord prepared = hydra::store::prepare_record(
            chart.entry.md5, fill_settings().profile(),
            corpus::analyzed(chart.file, fill_settings().to_analysis_settings()));
        for (int rank : emptied)
            prepared.record.paths.at(static_cast<size_t>(rank - 1)).hardest_ms.reset();
        store.save(prepared);
    };
    save(throws, {1, 2});
    save(kept, {1});

    // The replay throws for Everlong's rank 1 text only. Its rank 2 would
    // replay, but the whole chart is skipped.
    const std::string throwing_text = "1 2 3+";
    int thrown = 0;
    const hydra::app::PathReplay replay = [&](const hydra::Song& song,
                                              const hydra::ScoreGraph& graph,
                                              const std::string& text) {
        if (text == throwing_text) {
            ++thrown;
            throw std::logic_error("a replay that breaks");
        }
        return hydra::paths_from_pathstring_on_graph(song, graph, text);
    };
    CHECK(store.lookup_one(throws.entry.md5, fill_settings().profile()).record.paths[0].path ==
          throwing_text);

    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), {}, replay));
    CHECK(thrown == 1);
    const std::vector<std::optional<double>> skipped = figures_of(store, throws.entry.md5);
    CHECK_FALSE(skipped[0].has_value());
    CHECK_FALSE(skipped[1].has_value());
    CHECK(figures_of(store, kept.entry.md5)[0] == std::optional<double>(0.0));
    CHECK(store.hardest_figure_filled(fill_settings().profile()));
}

TEST_CASE("the fill on several workers writes the same figures as on one") {
    // The first corpus charts, each its own chart, that have a row for the
    // fill to read once every rank's figure is emptied (the store's own
    // answer, rows_missing_hardest), saved that way in one library.
    const size_t want = 6;
    const hydra::app::AnalysisSettings analysis = fill_settings().to_analysis_settings();
    std::vector<ChartLibraryEntry> entries;
    std::vector<PreparedRecord> saved;
    std::set<std::string> ids;
    for (const std::string& file : corpus::chart_paths()) {
        if (saved.size() == want) break;
        try {
            const hydra::HydraRecord& record = corpus::analyzed(file, analysis);
            if (record.paths.empty()) continue;
            ChartLibraryEntry entry = corpus::scanned_entry(file, file);
            if (!ids.insert(entry.md5).second) continue;
            PreparedRecord prepared =
                hydra::store::prepare_record(entry.md5, fill_settings().profile(), record);
            for (auto& path : prepared.record.paths) path.hardest_ms.reset();
            RecordStore probe(":memory:");
            probe.rebuild_chart_library({entry});
            probe.save(prepared);
            if (probe.rows_missing_hardest(fill_settings().profile()).empty()) continue;
            entries.push_back(std::move(entry));
            saved.push_back(std::move(prepared));
        } catch (const std::exception&) {
            continue;
        }
    }
    REQUIRE(saved.size() == want);

    // Every chart's figures, in `entries` order, after a fill on `workers`
    // threads of a fresh store holding the charts above. `stopped`, when set,
    // runs first on the same store and answers stop part-way.
    const auto filled_on = [&](int workers, Counter& counter, Counter* stopped = nullptr) {
        RecordStore store(":memory:");
        store.rebuild_chart_library(entries);
        for (const PreparedRecord& p : saved) store.save(p);
        if (stopped) {
            CHECK_FALSE(hydra::app::fill_saved_hardest(store, fill_settings(), stopped->fn(),
                                                       hydra::paths_from_pathstring_on_graph,
                                                       workers));
            CHECK_FALSE(store.hardest_figure_filled(fill_settings().profile()));
        }
        CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), counter.fn(),
                                             hydra::paths_from_pathstring_on_graph, workers));
        CHECK(store.hardest_figure_filled(fill_settings().profile()));
        std::vector<std::vector<std::optional<double>>> out;
        for (const ChartLibraryEntry& e : entries) out.push_back(figures_of(store, e.md5));
        return out;
    };
    Counter one_counter;
    const auto one = filled_on(1, one_counter);
    Counter four_counter;
    const auto four = filled_on(4, four_counter);
    // A stop after the first chart is written, and the next run finishes.
    Counter stopped;
    stopped.stop_at = 2;
    Counter resumed;
    CHECK(filled_on(4, resumed, &stopped) == one);
    CHECK(stopped.calls == 2);

    // Some rank got its figure back, so the comparison is not of empty rows.
    size_t figures = 0;
    for (const auto& chart : one)
        for (const std::optional<double>& f : chart) figures += f.has_value() ? 1 : 0;
    CHECK(figures > 0);
    CHECK(four == one);
    // Before the first chart, after each, and once more after the last.
    CHECK(one_counter.calls == static_cast<int>(want) + 1);
    CHECK(four_counter.calls == static_cast<int>(want) + 1);
    CHECK(four_counter.charts_total == want);
}

TEST_CASE("an inside-Star-Power squeeze-in fills a negative figure that shows as none") {
    RecordStore store(":memory:");
    const CorpusChart chart = named_chart(kEverlong);
    const ChartLibraryEntry& entry = chart.entry;
    save_old(store, chart, {1});
    const hydra::store::RecordLookup saved = store.lookup_one(entry.md5, fill_settings().profile());
    REQUIRE(saved.record.paths.size() >= 1);
    CHECK(saved.record.paths[0].path == "1 2 3+");
    CHECK(saved.record.paths[0].sqin_count == 1);
    CHECK(saved.record.paths[0].sqout_count == 0);

    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), {}));
    const std::optional<double> figure = figures_of(store, entry.md5)[0];
    REQUIRE(figure.has_value());
    CHECK(*figure == -189.83750000002328);
    CHECK_FALSE(hydra::shown_timing(figure).has_value());
}

// The fill reads one chart's rows on one graph: a replay that keeps the
// address of the graph it is handed sees the same one for every row of the
// chart. Smash Mouth's ranks 1 to 3 are the rows it reads, as in the first
// case above.
TEST_CASE("the fill hands every row of one chart the same graph") {
    RecordStore store(":memory:");
    const CorpusChart chart = named_chart(kSmashMouth);
    save_old(store, chart, {1, 2, 3});

    // Both the graph's own address and the address of its first node are kept:
    // a graph rebuilt per row could reuse the stack slot of the one before it,
    // but its nodes are heap-allocated, so the node address is a second sign
    // (not a proof: freed heap blocks can be handed out again).
    std::vector<const hydra::ScoreGraph*> seen;
    std::vector<const hydra::ScoreGraphNode*> seen_start;
    const hydra::app::PathReplay replay = [&](const hydra::Song& song,
                                              const hydra::ScoreGraph& graph,
                                              const std::string& text) {
        seen.push_back(&graph);
        seen_start.push_back(graph.start());
        return hydra::paths_from_pathstring_on_graph(song, graph, text);
    };
    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), {}, replay, 1));
    REQUIRE(seen.size() == 3);
    CHECK(seen[1] == seen[0]);
    CHECK(seen[2] == seen[0]);
    CHECK(seen_start[1] == seen_start[0]);
    CHECK(seen_start[2] == seen_start[0]);
    // The figures are the ones the first case pins.
    CHECK(figures_of(store, chart.entry.md5) ==
          std::vector<std::optional<double>>{0.0, 0.0, 0.0, std::nullopt, 0.0});
}

// A result this build saves through the store's own save never leaves the
// fill a row to read: every saved path with a squeeze-in holds its figure
// (store::PathRow::hardest_ms, set where the store builds the row), and the
// store's own question for the fill (rows_missing_hardest) finds none.
TEST_CASE("a result this build saves holds a saved figure on every path with a squeeze-in") {
    RecordStore store(":memory:");
    const CorpusChart smash = named_chart(kSmashMouth);
    const CorpusChart everlong = named_chart(kEverlong);
    store.rebuild_chart_library({smash.entry, everlong.entry});
    const hydra::store::Profile profile = fill_settings().profile();
    int squeezed = 0;
    for (const CorpusChart* chart : {&smash, &everlong}) {
        store.save(hydra::store::prepare_record(
            chart->entry.md5, profile,
            corpus::analyzed(chart->file, fill_settings().to_analysis_settings())));
        const hydra::store::RecordLookup found = store.lookup_one(chart->entry.md5, profile);
        REQUIRE(found.status == hydra::store::RecordStatus::Ready);
        for (const hydra::store::PathRow& p : found.record.paths) {
            if (p.sqin_count == 0) continue;
            ++squeezed;
            CAPTURE(chart->entry.md5);
            CAPTURE(p.rank);
            CHECK(p.hardest_ms.has_value());
        }
    }
    CHECK(squeezed > 0);
    CHECK(store.rows_missing_hardest(profile).empty());
}

// A row saved under a results stamp this build does not accept is never read
// by the fill, even with an empty figure and a squeeze-in: the fill reads no
// chart, calls neither the progress callback nor the replay, and the row is
// as it was. Smash Mouth's ranks 1 to 3 are the rows the first case above
// fills under the current stamp; here they sit under an older one.
TEST_CASE("a row under an older stamp is left alone by the fill") {
    const testtemp::ScopedFile db("older_stamp_fill", ".db");
    const CorpusChart chart = named_chart(kSmashMouth);
    const std::string empty_rows =
        "SELECT COUNT(*) FROM record_paths WHERE hardest_ms IS NULL AND sqin_count > 0";
    int replays = 0;
    Counter counter;
    {
        RecordStore store(db.path);
        store.rebuild_chart_library({chart.entry});
        PreparedRecord prepared = hydra::store::prepare_record(
            chart.entry.md5, fill_settings().profile(),
            corpus::analyzed(chart.file, fill_settings().to_analysis_settings()));
        for (int rank : {1, 2, 3})
            prepared.record.paths.at(static_cast<size_t>(rank - 1)).hardest_ms.reset();
        hydra::test::save_old_build_record(store, prepared);
        REQUIRE(store.lookup_one(chart.entry.md5, fill_settings().profile()).status ==
                hydra::store::RecordStatus::Stale);
        REQUIRE(hydra::test::scalar_on_file(db.path, empty_rows) == 3);

        const hydra::app::PathReplay replay = [&](const hydra::Song& song,
                                                  const hydra::ScoreGraph& graph,
                                                  const std::string& text) {
            ++replays;
            return hydra::paths_from_pathstring_on_graph(song, graph, text);
        };
        CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), counter.fn(), replay));
        CHECK(store.lookup_one(chart.entry.md5, fill_settings().profile()).status ==
              hydra::store::RecordStatus::Stale);
    }
    CHECK(counter.calls == 0);
    CHECK(replays == 0);
    CHECK(hydra::test::scalar_on_file(db.path, empty_rows) == 3);
}
