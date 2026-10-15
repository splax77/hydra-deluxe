// The one leaderboard fixture the dm_report tests share (audit finding 277):
// a store holding one analyzed corpus chart, a leaderboard score builder, and
// a tile lookup over every score.

#ifndef HYDRA_TESTS_DM_FIXTURE_H
#define HYDRA_TESTS_DM_FIXTURE_H

#include <cstdint>
#include <exception>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"  // Settings::profile, profile_at_clone_hero_cap
#include "app/dm_report.h"
#include "app/report.h"  // Tile
#include "core/model.h"
#include "corpus_util.h"
#include "doctest.h"
#include "net/dmbot_client.h"
#include "report_samples.h"  // every_row
#include "store/record_store.h"

namespace testdm {

inline constexpr const char* kMode = "Expert Pro Drums, 2x Bass";
inline constexpr const char* kHash = "aa11bb22cc33dd44ee55ff6677889900";

// The settings the fixture's records are analyzed under, in chart mode
// `mode`: the defaults with the ms limit off, as the corpus analysis ran.
inline hydra::app::Settings settings(const std::string& mode = kMode) {
    hydra::app::Settings s = hydra::app::Settings{}.with_known_chartmode(mode);
    s.mslimit_enabled = false;
    return s;
}

// The profile the comparison lists under settings(mode), by its name.
inline hydra::store::Profile profile(const std::string& mode = kMode) {
    return settings(mode).profile_at_clone_hero_cap();
}

// The corpus chart every fixture record is made from.
inline const hydra::app::AnalysisResult& sample_chart() {
    static const hydra::app::AnalysisResult result = [] {
        hydra::app::AnalysisSettings analysis;
        analysis.depth_value = 0;
        return corpus::first_analyzed_with_paths(analysis);
    }();
    return result;
}

// Saves `record` for chart `song_id` under `s.profile()`, through the one
// save.
inline void save(hydra::store::RecordStore& store, const std::string& song_id,
                 const hydra::app::Settings& s, const hydra::HydraRecord& record) {
    store.save(hydra::store::prepare_record(song_id, s.profile(), record));
}

// A Ready record with no paths for chart `song_id` at Clone Hero's cap in
// chart mode kMode: an analysis that kept no path.
inline void save_no_paths(hydra::store::RecordStore& store, const std::string& song_id) {
    hydra::HydraRecord empty;
    empty.sp_cap = hydra::kCloneHeroSpCap;
    save(store, song_id, settings(), empty);
}

// An in-memory store holding one analyzed corpus chart under kHash/kMode at
// SP cap 4, listed in the library under `name` (a result's names come from
// the library). Returns the record's best score (the "optimal" side of the
// join).
inline int64_t fill_store(hydra::store::RecordStore& store,
                          const std::string& name = "Stored Title") {
    using namespace hydra;
    const app::AnalysisResult& result = sample_chart();
    store.rebuild_chart_library({store::ChartLibraryEntry{
        kHash, name, "Stored Artist", "Stored Charter", "C:\\charts\\stored\\notes.chart",
        "C:\\charts", "sig-stored"}});
    save(store, kHash, settings(), result.record);
    // A what-if record at 8 bars for the same chart: the comparison must never
    // pick it up (the leaderboard plays at 4 bars).
    app::Settings at8 = settings();
    at8.sp_cap = 8;
    HydraRecord whatif = result.record;
    whatif.sp_cap = 8;
    save(store, kHash, at8, whatif);
    return result.record.best_path().totalscore();
}

// One leaderboard score, posted at `speed` percent (base speed by default).
inline hydra::net::DmScore make_score(const std::string& identifier, int64_t score,
                                      int speed = hydra::net::kBaseSpeedPercent) {
    hydra::net::DmScore s;
    s.identifier = identifier;
    s.song_name = "Board Title";
    s.artist = "Board Artist";
    s.charter = "Board Charter";
    s.score = score;
    s.is_fc = false;
    s.percent = 100;
    s.speed = speed;
    s.posted = "2026-01-01T00:00:00Z";
    s.known = true;
    return s;
}

// The value of the tile named `label` over every score in `rows`.
inline std::string dm_tile(const std::vector<hydra::app::dm_report::DmReportRow>& rows,
                           const std::string& label) {
    for (const hydra::app::report::Tile& t :
         hydra::app::dm_report::dm_tiles(rows, report_samples::every_row(rows.size())))
        if (t.label == label) return t.value;
    FAIL("no tile " << label);
    return {};
}

}  // namespace testdm

#endif  // HYDRA_TESTS_DM_FIXTURE_H
