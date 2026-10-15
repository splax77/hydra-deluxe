// The analyzed record the store tests save, look up and relabel. The three
// store test files (test_store_results.cpp, test_store_library.cpp and
// test_store_open.cpp) all build their rows from it, so it lives here once.
#ifndef HYDRA_TESTS_STORE_FIXTURES_H
#define HYDRA_TESTS_STORE_FIXTURES_H

#include "app/analysis.h"
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

}  // namespace hydra::test

#endif
