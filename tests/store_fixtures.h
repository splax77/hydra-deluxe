// The analyzed record the store tests save, look up and relabel. The three
// store test files (test_store_results.cpp, test_store_library.cpp and
// test_store_open.cpp) all build their rows from it, so it lives here once.
#ifndef HYDRA_TESTS_STORE_FIXTURES_H
#define HYDRA_TESTS_STORE_FIXTURES_H

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "doctest.h"

#include "app/analysis.h"
#include "app/config.h"
#include "core/model.h"
#include "corpus_util.h"
#include "store/record_store.h"

namespace hydra::test {

// One analyzed corpus chart: the first chart with paths (corpus_util.h), its
// record at 4 bars.
inline const app::AnalysisResult& fixture() {
    static const app::AnalysisResult f = [] {
        app::AnalysisSettings settings;
        settings.sp_cap = 4;
        settings.depth_mode = DepthMode::Scores;
        settings.depth_value = 0;
        settings.ms_filter = std::nullopt;
        return ::corpus::first_analyzed_with_paths(settings);
    }();
    return f;
}

// The same record relabeled as if it had run at another cap. The paths are
// the 4-bar paths, which is fine: these tests check which row a lookup picks,
// not what is in it.
inline HydraRecord at_cap(int cap) {
    HydraRecord r = fixture().record;
    r.sp_cap = cap;
    return r;
}

// The fixture's own profile under chart mode `mode`: 4 bars, no ms limit, the
// 1.1 fill rule. The profile a record from at_cap(4) is saved under.
inline store::Profile at4(const std::string& mode = "mode") {
    return store::Profile{mode, store::CapQuery::at(4), store::Lens{}};
}

// at_cap with every path taken out: "analyzed, and nothing survived".
inline HydraRecord no_paths_at_cap(int cap) {
    HydraRecord r = at_cap(cap);
    r.paths.clear();
    r.allzero_paths.clear();
    return r;
}

// The figures chart `id`'s Ready record under `profile` holds, in rank order.
inline std::vector<std::optional<double>> saved_figures(store::RecordStore& store,
                                                        const std::string& id,
                                                        const store::Profile& profile) {
    const store::RecordLookup found = store.lookup_one(id, profile);
    REQUIRE(found.status == store::RecordStatus::Ready);
    std::vector<std::optional<double>> out;
    for (const store::PathRow& p : found.record.paths) out.push_back(p.hardest_ms);
    return out;
}

// Lists `entry` as the library and saves the analysis of the corpus chart
// file `chart` under `settings` for it, with the hardest figure of every rank
// in `emptied` left empty, as an older build saved it. `edit` changes the
// prepared record further before it is saved.
inline void save_as_old_build(store::RecordStore& store, const std::string& chart,
                              const store::ChartLibraryEntry& entry,
                              const app::Settings& settings, const std::vector<int>& emptied,
                              const std::function<void(store::PreparedRecord&)>& edit = {}) {
    store.rebuild_chart_library({entry});
    store::PreparedRecord prepared = store::prepare_record(
        entry.md5, settings.profile(), ::corpus::analyzed(chart, settings.to_analysis_settings()));
    for (int rank : emptied)
        prepared.record.paths.at(static_cast<size_t>(rank - 1)).hardest_ms.reset();
    if (edit) edit(prepared);
    store.save(prepared);
}

}  // namespace hydra::test

#endif
