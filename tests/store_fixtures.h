// The analyzed record the store tests save, look up and relabel. The store
// test files all build their rows from it, so it lives here once.
#ifndef HYDRA_TESTS_STORE_FIXTURES_H
#define HYDRA_TESTS_STORE_FIXTURES_H

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "doctest.h"

#include "app/analysis.h"
#include "app/config.h"
#include "core/model.h"
#include "core/rules.h"  // Rules, default_rules
#include "corpus_util.h"
#include "store/record_store.h"
#include "temp_util.h"  // testtemp::temp_path

namespace hydra::test {

// The search settings fixture() runs under, at `cap` bars, with ms limit
// `ms`, Clone Hero 1.0's fill rule when `legacy_fills`, and `rules`.
inline app::AnalysisSettings store_fixture_settings(
    int cap = 4, std::optional<double> ms = std::nullopt, bool legacy_fills = false,
    const core::Rules& rules = core::default_rules()) {
    app::AnalysisSettings settings;
    settings.sp_cap = cap;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 0;
    settings.ms_filter = ms;
    settings.legacy_fills = legacy_fills;
    settings.rules = rules;
    return settings;
}

// One analyzed corpus chart: the first chart with paths (corpus_util.h), its
// record at 4 bars.
inline const app::AnalysisResult& fixture() {
    static const app::AnalysisResult f =
        ::corpus::first_analyzed_with_paths(store_fixture_settings());
    return f;
}

// The fixture's chart file: the first corpus chart with paths at 4 bars.
inline const std::string& fixture_chart() {
    static const std::string path =
        ::corpus::first_chart_with_paths(store_fixture_settings()).chart;
    return path;
}

// A copy of testdata/store/v2.12.1_library.db, the library a real v2.12.1
// hydra_batch wrote (testdata/store/README.md). Its records are another
// build's, so they read Stale for "another build". The store opens the copy
// in WAL mode, which would change the checked-in file.
struct OldLibrary {
    std::string path;
    explicit OldLibrary(const std::string& tag) : path(::testtemp::temp_path(tag, ".db")) {
        const std::string source = ::corpus::testdata_root() + "/store/v2.12.1_library.db";
        REQUIRE(std::filesystem::exists(std::filesystem::u8path(source)));
        remove_files();
        std::filesystem::copy_file(std::filesystem::u8path(source), std::filesystem::u8path(path));
    }
    ~OldLibrary() { remove_files(); }
    OldLibrary(const OldLibrary&) = delete;
    OldLibrary& operator=(const OldLibrary&) = delete;
    void remove_files() const {
        std::error_code ec;
        for (const char* suffix : {"", "-wal", "-shm"})
            std::filesystem::remove(std::filesystem::u8path(path + suffix), ec);
    }
    // The run's profile at `cap` bars; the README gives its settings (4 bars,
    // ms limit 10, score depth 4) and chart mode.
    static store::Profile profile(int cap = 4) {
        return store::Profile{"Expert Pro Drums, 2x Bass", store::CapQuery::at(cap),
                              store::Lens::from(10, 0, 4)};
    }
    // The settings a search under profile() runs with.
    static app::AnalysisSettings settings() {
        app::AnalysisSettings s = store_fixture_settings(4, 10.0);
        s.depth_value = 4;
        return s;
    }
    // The song ids of its three records, as the README gives them.
    static std::vector<std::string> song_ids() {
        return {"0b647b1570475f18d463466f2c4d72a7", "0d4fd734fe5f2bf6ff1fdbc4ff55be8f",
                "bef0759746740f9c1045f64e9a5a2938"};
    }
};

// The fixture's chart analyzed under the old library's settings at `cap` bars:
// a real record that fits OldLibrary::profile(cap).
inline const HydraRecord& old_library_analysis(int cap = 4) {
    app::AnalysisSettings s = OldLibrary::settings();
    s.sp_cap = cap;
    return ::corpus::analyzed(fixture_chart(), s);
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
