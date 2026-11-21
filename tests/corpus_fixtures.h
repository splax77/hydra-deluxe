// Corpus fixtures that more than one test file builds the same way: the
// settings the engine sweeps search with, so another file's corpus loop can
// reuse the sweeps' cached records, and a second scanned copy of a corpus
// chart in its own folder.

#ifndef HYDRA_TESTS_CORPUS_FIXTURES_H
#define HYDRA_TESTS_CORPUS_FIXTURES_H

#include <filesystem>
#include <optional>
#include <string>

#include "app/analysis.h"  // hash_chart_file
#include "core/model.h"    // kCloneHeroSpCap
#include "core/winstr.h"   // os_path, read_file_bytes
#include "corpus_util.h"   // scanned_entry
#include "doctest.h"
#include "search/engine.h"  // DepthMode
#include "search/pather.h"  // SearchSettings
#include "store/record_store.h"
#include "temp_util.h"

namespace corpus {

// A depth far past the app's default (kDefaultDepthValue), in scores below
// the best, so a sweep's record keeps many alternate paths for the checks
// that walk them.
constexpr int kSweepDepthScores = 200;

// The settings every engine sweep searches with: Clone Hero's SP cap, a depth
// in scores below the best and an optional Path limit. A corpus loop in
// another file that asks for the same settings gets the sweep's record from
// the corpus::analyzed cache.
inline hydra::SearchSettings sweep_settings(int depth_scores, std::optional<double> limit_ms) {
    hydra::SearchSettings cfg;
    cfg.sp_cap = hydra::kCloneHeroSpCap;
    cfg.depth_mode = hydra::DepthMode::Scores;
    cfg.depth_value = depth_scores;
    cfg.ms_filter = limit_ms;
    return cfg;
}

// A second copy of the corpus chart `chart` in another folder, as a scan
// lists it: the chart file, and its song.ini when it has one, copied into the
// scratch folder testtemp::temp_dir(tag), and the copy's own scanned row
// titled `title` (corpus::scanned_entry). The row's rootfolder is that
// scratch folder. The scan's own hash of the copy must be the original's, or
// the two rows are not one chart.
inline hydra::store::ChartLibraryEntry scanned_copy(const std::string& chart,
                                                    const std::string& tag,
                                                    const std::string& title) {
    const size_t slash = chart.rfind('\\');
    REQUIRE(slash != std::string::npos);
    const std::string dir = testtemp::temp_dir(tag);
    const std::string copied = dir + chart.substr(slash);
    testtemp::write_bytes(copied, hydra::read_file_bytes(chart));
    const std::string ini = chart.substr(0, slash) + "\\song.ini";
    if (std::filesystem::exists(hydra::os_path(ini)))
        testtemp::write_bytes(dir + "\\song.ini", hydra::read_file_bytes(ini));
    hydra::store::ChartLibraryEntry e = scanned_entry(copied, title);
    REQUIRE(e.md5 == hydra::app::hash_chart_file(chart));
    REQUIRE(e.rootfolder != chart.substr(0, slash));
    return e;
}

}  // namespace corpus

#endif  // HYDRA_TESTS_CORPUS_FIXTURES_H
