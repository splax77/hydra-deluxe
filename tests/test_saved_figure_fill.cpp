// The one-time fill of the saved hardest figure (app::fill_saved_hardest):
// rows an older build saved with an empty figure get the engine's figure back
// by replaying their saved path on the chart. Each case saves a real corpus
// analysis as an older build left it, with the figure of the rows the fill
// reads emptied, then runs the fill. The rows emptied are the store's own
// answer (rows_missing_hardest), and every figure a case expects is the one a
// fresh save of the same analysis holds (the store's row builder).

#include "doctest.h"

#include <cstddef>
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
#include "core/model.h"  // shown_timing, timing_needs_hitting, ChartFileError
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

// The corpus chart the one-chart cases save.
const char* const kSmashMouth = "Smash Mouth - I_m A Believer";
// A second corpus chart, for the cases that need two in one library.
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

// The index of 1-based rank `rank` in a record's paths.
size_t rank_index(int rank) { return static_cast<size_t>(rank - 1); }

// Empties the saved figure of every rank in `ranks`, as an older build left
// them.
void empty_ranks(PreparedRecord& prepared, const std::vector<int>& ranks) {
    for (int rank : ranks) prepared.record.paths.at(rank_index(rank)).hardest_ms.reset();
}

// Empties the saved figure of every rank.
void empty_every_figure(PreparedRecord& prepared) {
    for (auto& path : prepared.record.paths) path.hardest_ms.reset();
}

// The record a fresh save of `chart`'s analysis under the running settings
// writes.
PreparedRecord fresh_record(const CorpusChart& chart) {
    return hydra::store::prepare_record(
        chart.entry.md5, fill_settings().profile(),
        corpus::analyzed(chart.file, fill_settings().to_analysis_settings()));
}

// The figure of every rank, in rank order, that a fresh save of `chart`'s
// analysis holds: the store's own row (engine_hardest_ms, set where the store
// builds the row), and so the figure the fill must write back.
std::vector<std::optional<double>> fresh_figures(const CorpusChart& chart) {
    RecordStore fresh(":memory:");
    save_old(fresh, chart, {});
    return figures_of(fresh, chart.entry.md5);
}

// The ranks of `chart`, in rank order, that the fill reads once every figure
// is emptied: the store's own answer (RecordStore::rows_missing_hardest).
std::vector<int> fill_ranks(const CorpusChart& chart) {
    RecordStore probe(":memory:");
    save_old(probe, chart, {}, empty_every_figure);
    std::vector<int> out;
    for (const hydra::store::MissingHardestRow& row :
         probe.rows_missing_hardest(fill_settings().profile()))
        out.push_back(row.rank);
    return out;
}

// The analysis of corpus chart `file` under the running settings, or null
// when the engine refuses the chart (a ChartFileError: the file cannot be
// read, or it has no notes at the difficulty). Any other exception fails the
// case that asked.
const hydra::HydraRecord* analyzed_or_refused(const std::string& file) {
    try {
        return &corpus::analyzed(file, fill_settings().to_analysis_settings());
    } catch (const hydra::ChartFileError&) {
        return nullptr;
    }
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

// Every row the fill reads gets back the figure a fresh save of its path
// holds, and every other row keeps the figure it was saved with.
TEST_CASE("a saved path with a squeeze-in gets back the figure a fresh save holds") {
    RecordStore store(":memory:");
    const CorpusChart chart = named_chart(kSmashMouth);
    const ChartLibraryEntry& entry = chart.entry;
    const std::vector<int> emptied = fill_ranks(chart);
    REQUIRE_FALSE(emptied.empty());
    const std::vector<std::optional<double>> fresh = fresh_figures(chart);
    save_old(store, chart, emptied);
    const std::vector<std::optional<double>> before = figures_of(store, entry.md5);
    for (int rank : emptied) CHECK_FALSE(before.at(rank_index(rank)).has_value());

    Counter first;
    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), first.fn()));
    // One chart: reported before it and once it is done.
    CHECK(first.calls == 2);
    CHECK(first.charts_total == 1);
    const std::vector<std::optional<double>> after = figures_of(store, entry.md5);
    for (int rank : emptied) {
        CAPTURE(rank);
        CHECK(after.at(rank_index(rank)).has_value());
    }
    CHECK(after == fresh);
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
    // The first two rows the fill reads: the first one's score is changed, so
    // it no longer matches its replay; the second still does.
    const std::vector<int> read = fill_ranks(chart);
    REQUIRE(read.size() >= 2);
    const int changed = read[0];
    const int matching = read[1];
    const std::vector<std::optional<double>> fresh = fresh_figures(chart);
    save_old(store, chart, {changed, matching}, [changed](PreparedRecord& p) {
        p.record.paths.at(rank_index(changed)).score += 1;
    });

    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), {}));
    const std::vector<std::optional<double>> after = figures_of(store, entry.md5);
    CHECK_FALSE(after.at(rank_index(changed)).has_value());
    REQUIRE(after.at(rank_index(matching)).has_value());
    CHECK(after.at(rank_index(matching)) == fresh.at(rank_index(matching)));
    CHECK(store.hardest_figure_filled(fill_settings().profile()));
}

TEST_CASE("a stop leaves the marker unset") {
    RecordStore store(":memory:");
    const CorpusChart chart = named_chart(kSmashMouth);
    const ChartLibraryEntry& entry = chart.entry;
    const std::vector<int> read = fill_ranks(chart);
    REQUIRE_FALSE(read.empty());
    const int rank = read.front();
    save_old(store, chart, {rank});

    Counter stopped;
    stopped.stop_at = 1;
    CHECK_FALSE(hydra::app::fill_saved_hardest(store, fill_settings(), stopped.fn()));
    CHECK(stopped.calls == 1);
    CHECK_FALSE(store.hardest_figure_filled(fill_settings().profile()));
    CHECK_FALSE(figures_of(store, entry.md5).at(rank_index(rank)).has_value());

    // The next run goes on and finishes, with the figure a fresh save holds.
    Counter next;
    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), next.fn()));
    CHECK(next.charts_total == 1);
    REQUIRE(figures_of(store, entry.md5).at(rank_index(rank)).has_value());
    CHECK(figures_of(store, entry.md5) == fresh_figures(chart));
    CHECK(store.hardest_figure_filled(fill_settings().profile()));
}

TEST_CASE("a chart whose replay throws keeps its rows empty, and the fill goes on") {
    // Two charts in one library, so save_as_old_build (which lists one chart
    // as the whole library) is not used: every row the fill reads on either
    // chart is saved with an empty figure.
    RecordStore store(":memory:");
    const hydra::store::Profile profile = fill_settings().profile();
    const CorpusChart throws = named_chart(kEverlong);
    const CorpusChart kept = named_chart(kSmashMouth);
    const std::vector<int> throws_ranks = fill_ranks(throws);
    const std::vector<int> kept_ranks = fill_ranks(kept);
    REQUIRE_FALSE(throws_ranks.empty());
    REQUIRE_FALSE(kept_ranks.empty());
    store.rebuild_chart_library({throws.entry, kept.entry});
    const auto save = [&](const CorpusChart& chart, const std::vector<int>& emptied) {
        PreparedRecord prepared = fresh_record(chart);
        empty_ranks(prepared, emptied);
        store.save(prepared);
    };
    save(throws, throws_ranks);
    save(kept, kept_ranks);

    // The replay throws for the stored text of the first row the fill reads
    // on Everlong, and only for it. Everlong's other rows would replay, but
    // the whole chart is skipped. No Smash Mouth row has that text, so the
    // throw stays on Everlong.
    const std::string throwing_text = store.lookup_one(throws.entry.md5, profile)
                                          .record.paths.at(rank_index(throws_ranks.front()))
                                          .path;
    for (const hydra::store::PathRow& p : store.lookup_one(kept.entry.md5, profile).record.paths)
        REQUIRE(p.path != throwing_text);
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

    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), {}, replay));
    CHECK(thrown == 1);
    const std::vector<std::optional<double>> skipped = figures_of(store, throws.entry.md5);
    for (int rank : throws_ranks) {
        CAPTURE(rank);
        CHECK_FALSE(skipped.at(rank_index(rank)).has_value());
    }
    CHECK(figures_of(store, kept.entry.md5) == fresh_figures(kept));
    CHECK(store.hardest_figure_filled(profile));
}

TEST_CASE("the fill on several workers writes the same figures as on one") {
    // The first corpus charts, each its own chart, that have a row for the
    // fill to read once every rank's figure is emptied (the store's own
    // answer, rows_missing_hardest), saved that way in one library. A chart
    // is passed over only for one of the reasons named below, each one
    // reported by name; any other exception fails the case.
    const size_t want = 6;
    std::vector<ChartLibraryEntry> entries;
    std::vector<PreparedRecord> saved;
    std::vector<std::string> passed_over;
    std::set<std::string> ids;
    for (const std::string& file : corpus::chart_paths()) {
        if (saved.size() == want) break;
        const hydra::HydraRecord* record = analyzed_or_refused(file);
        if (!record) {
            passed_over.push_back(file + ": the engine refused the chart");
            continue;
        }
        if (record->paths.empty()) {
            passed_over.push_back(file + ": no path");
            continue;
        }
        const CorpusChart chart{file, corpus::scanned_entry(file, file)};
        if (!ids.insert(chart.entry.md5).second) {
            passed_over.push_back(file + ": the same chart as one before it");
            continue;
        }
        if (fill_ranks(chart).empty()) {
            passed_over.push_back(file + ": no row for the fill to read");
            continue;
        }
        PreparedRecord prepared = fresh_record(chart);
        empty_every_figure(prepared);
        entries.push_back(chart.entry);
        saved.push_back(std::move(prepared));
    }
    for (const std::string& why : passed_over) MESSAGE(why);
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

// A row the fill reads always has a squeeze-in, and its figure is the largest
// difficulty over its squeezes and E0 fills (engine_hardest_ms). So a figure
// that needs no hitting (timing_needs_hitting is false) means the squeeze-in
// itself needs none: it sits inside Star Power, before the SP end. The fill
// writes that figure whole, sign included, and shown_timing shows no timing
// for it. The case is the first corpus chart whose fresh save holds such a
// figure on a row the fill reads.
TEST_CASE("an inside-Star-Power squeeze-in fills a negative figure that shows as none") {
    std::optional<CorpusChart> found;
    int found_rank = 0;
    for (const std::string& file : corpus::chart_paths()) {
        const hydra::HydraRecord* record = analyzed_or_refused(file);
        if (!record || record->paths.empty()) continue;
        CorpusChart candidate{file, corpus::scanned_entry(file, file)};
        const std::vector<std::optional<double>> fresh = fresh_figures(candidate);
        for (int rank : fill_ranks(candidate)) {
            const std::optional<double>& f = fresh.at(rank_index(rank));
            if (f && !hydra::timing_needs_hitting(*f)) {
                found_rank = rank;
                break;
            }
        }
        if (found_rank != 0) {
            found = std::move(candidate);
            break;
        }
    }
    REQUIRE_MESSAGE(found.has_value(),
                    "no corpus chart's fresh save holds a figure that needs no hitting on a row "
                    "the fill reads");
    const CorpusChart& chart = *found;
    CAPTURE(chart.file);
    CAPTURE(found_rank);
    const std::vector<std::optional<double>> fresh = fresh_figures(chart);

    RecordStore store(":memory:");
    save_old(store, chart, {found_rank});
    REQUIRE_FALSE(figures_of(store, chart.entry.md5).at(rank_index(found_rank)).has_value());
    CHECK(hydra::app::fill_saved_hardest(store, fill_settings(), {}));
    const std::vector<std::optional<double>> after = figures_of(store, chart.entry.md5);
    CHECK(after == fresh);
    const std::optional<double> figure = after.at(rank_index(found_rank));
    REQUIRE(figure.has_value());
    CHECK_FALSE(hydra::timing_needs_hitting(*figure));
    CHECK_FALSE(hydra::shown_timing(figure).has_value());
}

// The fill reads one chart's rows on one graph: a replay that keeps the
// address of the graph it is handed sees the same one for every row of the
// chart. Every row the fill reads on Smash Mouth is emptied, and it takes two
// rows at least to compare.
TEST_CASE("the fill hands every row of one chart a graph at one address") {
    RecordStore store(":memory:");
    const CorpusChart chart = named_chart(kSmashMouth);
    const std::vector<int> emptied = fill_ranks(chart);
    REQUIRE(emptied.size() >= 2);
    save_old(store, chart, emptied);

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
    // One replay for each row the fill reads.
    REQUIRE(seen.size() == emptied.size());
    for (size_t i = 1; i < seen.size(); ++i) {
        CAPTURE(i);
        CHECK(seen[i] == seen[0]);
        CHECK(seen_start[i] == seen_start[0]);
    }
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
        store.save(fresh_record(*chart));
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
// as it was. The rows emptied are the ones the fill reads on Smash Mouth
// under the current stamp; here they sit under an older one.
TEST_CASE("a row under an older stamp is left alone by the fill") {
    const testtemp::ScopedFile db("older_stamp_fill", ".db");
    const CorpusChart chart = named_chart(kSmashMouth);
    const std::vector<int> emptied = fill_ranks(chart);
    REQUIRE_FALSE(emptied.empty());
    const std::string empty_rows =
        "SELECT COUNT(*) FROM record_paths WHERE hardest_ms IS NULL AND sqin_count > 0";
    int64_t empty_before = 0;
    int replays = 0;
    Counter counter;
    {
        RecordStore store(db.path);
        store.rebuild_chart_library({chart.entry});
        PreparedRecord prepared = fresh_record(chart);
        empty_ranks(prepared, emptied);
        hydra::test::save_old_build_record(store, prepared);
        REQUIRE(store.lookup_one(chart.entry.md5, fill_settings().profile()).status ==
                hydra::store::RecordStatus::Stale);
        empty_before = hydra::test::scalar_on_file(db.path, empty_rows);
        // Every emptied row is one the fill would read under the current stamp.
        REQUIRE(empty_before == static_cast<int64_t>(emptied.size()));

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
    CHECK(hydra::test::scalar_on_file(db.path, empty_rows) == empty_before);
}
