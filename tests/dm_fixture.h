// The one leaderboard fixture the dm_report tests share (audit finding 277):
// a store holding one analyzed corpus chart, and a leaderboard score builder.

#ifndef HYDRA_TESTS_DM_FIXTURE_H
#define HYDRA_TESTS_DM_FIXTURE_H

#include <cstdint>
#include <exception>
#include <string>

#include "app/analysis.h"
#include "core/model.h"
#include "corpus_util.h"
#include "doctest.h"
#include "net/dmbot_client.h"
#include "store/record_store.h"

namespace testdm {

inline constexpr const char* kMode = "Expert Pro Drums, 2x Bass";
inline constexpr const char* kHash = "aa11bb22cc33dd44ee55ff6677889900";

// An in-memory store holding one analyzed corpus chart under kHash/kMode at
// SP cap 4. Returns the record's best score (the "optimal" side of the join).
inline int64_t fill_store(hydra::store::RecordStore& store,
                          const std::string& name = "Stored Title") {
    using namespace hydra;
    app::AnalysisSettings settings;
    settings.depth_value = 0;
    for (const std::string& path : corpus::chart_paths()) {
        try {
            app::AnalysisResult result = app::analyze_chart_file(path, settings);
            if (result.song.is_empty() || result.record.paths.empty()) continue;
            store.add_song(kHash, name, "Stored Artist", "Stored Charter", result.song);
            store.add_record(
                store::RecordKey{kHash, kMode, store::CapQuery::at(kCloneHeroSpCap)},
                result.record);
            // A what-if record at 8 bars for the same chart: the comparison
            // must never pick it up (the leaderboard plays at 4 bars).
            HydraRecord whatif = result.record;
            whatif.sp_cap = 8;
            store.add_record(store::RecordKey{kHash, kMode, store::CapQuery::at(8)}, whatif);
            return result.record.best_path().totalscore();
        } catch (const std::exception&) {
            continue;
        }
    }
    REQUIRE_MESSAGE(false, "no corpus chart analyzed");
    return 0;
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

}  // namespace testdm

#endif  // HYDRA_TESTS_DM_FIXTURE_H
