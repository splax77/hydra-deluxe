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
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/saved_figure_fill.h"
#include "core/model.h"  // shown_timing
#include "corpus_util.h"
#include "store/record_store.h"
#include "store_fixtures.h"  // save_as_old_build, saved_figures

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
const Settings& default_settings() {
    static const Settings s;
    return s;
}

// A corpus chart's file, and the library row a scan writes for it.
struct CorpusChart {
    std::string file;
    ChartLibraryEntry entry;
};

// The corpus chart whose path holds `name`.
CorpusChart corpus_chart(const std::string& name) {
    const std::string chart = corpus::chart_named(name);
    REQUIRE_FALSE(chart.empty());
    return {chart, corpus::scanned_entry(chart, name)};
}

// Saves `chart` as an older build left it (hydra::test::save_as_old_build)
// under the running settings.
void save_old(RecordStore& store, const CorpusChart& chart, const std::vector<int>& emptied,
              const std::function<void(PreparedRecord&)>& edit = {}) {
    hydra::test::save_as_old_build(store, chart.file, chart.entry, default_settings(), emptied,
                                   edit);
}

// The saved figure of every rank of chart `md5`'s record, in rank order.
std::vector<std::optional<double>> figures_of(RecordStore& store, const std::string& md5) {
    return hydra::test::saved_figures(store, md5, default_settings().profile());
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
    const CorpusChart chart = corpus_chart(kSmashMouth);
    const ChartLibraryEntry& entry = chart.entry;
    save_old(store, chart, {1, 2, 3});
    const std::vector<std::optional<double>> before = figures_of(store, entry.md5);
    REQUIRE(before.size() == 5);
    CHECK_FALSE(before[0].has_value());
    CHECK_FALSE(before[1].has_value());
    CHECK_FALSE(before[2].has_value());

    Counter first;
    CHECK(hydra::app::fill_saved_hardest(store, default_settings(), first.fn()));
    // One chart: reported before it and once it is done.
    CHECK(first.calls == 2);
    CHECK(first.charts_total == 1);
    const hydra::store::RecordLookup found = store.lookup_one(entry.md5, default_settings().profile());
    CHECK(found.record.paths[0].path == "1 0+ 0 0");
    CHECK(figures_of(store, entry.md5) ==
          std::vector<std::optional<double>>{0.0, 0.0, 0.0, std::nullopt, 0.0});
    CHECK(store.hardest_figure_filled(default_settings().profile()));

    // The marker is set, so a second run reads no chart.
    Counter second;
    CHECK(hydra::app::fill_saved_hardest(store, default_settings(), second.fn()));
    CHECK(second.calls == 0);
}

TEST_CASE("a saved path whose chart is gone keeps its empty figure") {
    RecordStore store(":memory:");
    CorpusChart chart = corpus_chart(kSmashMouth);
    ChartLibraryEntry& entry = chart.entry;
    entry.notespath = entry.rootfolder + "\\no such folder\\notes.chart";
    save_old(store, chart, {1});

    CHECK(hydra::app::fill_saved_hardest(store, default_settings(), {}));
    CHECK_FALSE(figures_of(store, entry.md5)[0].has_value());
    CHECK(store.hardest_figure_filled(default_settings().profile()));
}

TEST_CASE("a row that disagrees with its replay keeps its empty figure") {
    RecordStore store(":memory:");
    const CorpusChart chart = corpus_chart(kSmashMouth);
    const ChartLibraryEntry& entry = chart.entry;
    save_old(store, chart, {1, 2}, [](PreparedRecord& p) { p.record.paths[0].score += 1; });

    CHECK(hydra::app::fill_saved_hardest(store, default_settings(), {}));
    const std::vector<std::optional<double>> after = figures_of(store, entry.md5);
    // Rank 1's score no longer matches its replay; rank 2 still does.
    CHECK_FALSE(after[0].has_value());
    CHECK(after[1] == std::optional<double>(0.0));
    CHECK(store.hardest_figure_filled(default_settings().profile()));
}

TEST_CASE("a stop leaves the marker unset") {
    RecordStore store(":memory:");
    const CorpusChart chart = corpus_chart(kSmashMouth);
    const ChartLibraryEntry& entry = chart.entry;
    save_old(store, chart, {1});

    Counter stopped;
    stopped.stop_at = 1;
    CHECK_FALSE(hydra::app::fill_saved_hardest(store, default_settings(), stopped.fn()));
    CHECK(stopped.calls == 1);
    CHECK_FALSE(store.hardest_figure_filled(default_settings().profile()));
    CHECK_FALSE(figures_of(store, entry.md5)[0].has_value());

    // The next run goes on and finishes.
    Counter next;
    CHECK(hydra::app::fill_saved_hardest(store, default_settings(), next.fn()));
    CHECK(next.charts_total == 1);
    CHECK(figures_of(store, entry.md5)[0] == std::optional<double>(0.0));
    CHECK(store.hardest_figure_filled(default_settings().profile()));
}

TEST_CASE("an inside-Star-Power squeeze-in fills a negative figure that shows as none") {
    RecordStore store(":memory:");
    const CorpusChart chart = corpus_chart(kEverlong);
    const ChartLibraryEntry& entry = chart.entry;
    save_old(store, chart, {1});
    const hydra::store::RecordLookup saved = store.lookup_one(entry.md5, default_settings().profile());
    REQUIRE(saved.record.paths.size() >= 1);
    CHECK(saved.record.paths[0].path == "1 2 3+");
    CHECK(saved.record.paths[0].sqin_count == 1);
    CHECK(saved.record.paths[0].sqout_count == 0);

    CHECK(hydra::app::fill_saved_hardest(store, default_settings(), {}));
    const std::optional<double> figure = figures_of(store, entry.md5)[0];
    REQUIRE(figure.has_value());
    CHECK(*figure == -189.83750000002328);
    CHECK_FALSE(hydra::shown_timing(figure).has_value());
}
