// Checks for three test helpers that other test files lean on.
//
// tests/corpus_util.h caches corpus parses and analyses for the whole run.
// Its checks pin what the corpus loops rely on: one answer per chart and
// settings, the same answer a direct call gives, and a failure that repeats
// instead of turning into a silent empty result.
//
// tests/temp_util.h builds every test's per-process scratch path. Its check
// pins that the names it hands back sit where the helper says they do.
//
// tests/scratch_settings.h holds the settings the GUI tests and the Burnout
// reference cases run under. Its check pins them against the app's defaults.

#include "doctest.h"

#include <filesystem>
#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "core/model.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "scratch_settings.h"
#include "search/pather.h"
#include "temp_util.h"

using namespace hydra;
using hydra::app::AnalysisSettings;
using hydra::app::Settings;

TEST_CASE("corpus cache: one parse and one analysis per chart and settings") {
    const std::string path = corpus::first_chart_with_notes();

    const Song& a = corpus::song(path, true, true);
    CHECK(&corpus::song(path, true, true) == &a);
    CHECK(&corpus::song(path, true, true, Difficulty::Hard) != &a);

    SearchSettings cfg;
    cfg.sp_cap = 4;
    cfg.depth_value = 0;
    const HydraRecord& r = corpus::analyzed(path, cfg);
    CHECK(&corpus::analyzed(path, cfg) == &r);

    // The cached record is the one a direct call produces.
    const HydraRecord direct = analyze_chart(load_songpath(path, true, true), cfg);
    REQUIRE(r.paths.size() == direct.paths.size());
    REQUIRE(!r.paths.empty());
    CHECK(r.best_path().pathstring() == direct.best_path().pathstring());
    CHECK(r.best_path().totalscore() == direct.best_path().totalscore());

    // Plain SearchSettings and the matching AnalysisSettings share one entry.
    app::AnalysisSettings same;
    static_cast<SearchSettings&>(same) = cfg;
    CHECK(&corpus::analyzed(path, same) == &r);

    // Different settings are a different entry.
    cfg.depth_value = 1;
    CHECK(&corpus::analyzed(path, cfg) != &r);
}

TEST_CASE("corpus cache: a failure is thrown again on every call") {
    // A chart with no Hard charting parses to an empty song, and analyzing it
    // throws NoNotesError, as analyze_chart_file does. The second call must
    // throw the same error again.
    const std::string no_hard = corpus::first_chart_without_notes(Difficulty::Hard);
    CAPTURE(no_hard);

    app::AnalysisSettings hard;
    hard.difficulty = Difficulty::Hard;
    CHECK_THROWS_WITH_AS(corpus::analyzed(no_hard, hard),
                         "No Hard Pro Drums notes in this chart.", NoNotesError);
    CHECK_THROWS_WITH_AS(corpus::analyzed(no_hard, hard),
                         "No Hard Pro Drums notes in this chart.", NoNotesError);
}

// tests/temp_util.h owns every test's per-process scratch name (audit finding
// 287). The temp folder is read from Windows once here, as the input; the rest
// is the helper's own output.
TEST_CASE("temp_util: the scratch path is the temp folder, the tag and this process") {
    wchar_t tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);
    const std::string temp = hydra::wide_to_utf8(tmp);
    const std::string pid = std::to_string(GetCurrentProcessId());

    const std::string file = testtemp::temp_path("probe", ".db");
    CHECK(hydra::starts_with(file, temp));
    CHECK(file.find("hydra_test_probe_") != std::string::npos);
    CHECK(hydra::ends_with_ci(file, "_" + pid + ".db"));

    const std::string dir = testtemp::temp_dir("probe");
    CHECK(hydra::is_directory_utf8(dir));
    CHECK(hydra::starts_with(dir, temp));
    CHECK(hydra::ends_with_ci(dir, "_" + pid));

    // A non-ASCII tag ("Zoë" as UTF-8) names a folder that exists.
    const std::string zoe = testtemp::temp_dir("Zo\xC3\xAB");
    CHECK(hydra::is_directory_utf8(zoe));
    CHECK(zoe.find("Zo\xC3\xAB") != std::string::npos);

    std::error_code ec;
    std::filesystem::remove_all(hydra::os_path(dir), ec);
    std::filesystem::remove_all(hydra::os_path(zoe), ec);
}

TEST_CASE("scratch_settings: the GUI tests' settings are the app's defaults at depth 2") {
    const Settings s = scratch_settings();
    CHECK(s.depth_value == 2);
    CHECK(s.auto_open_report == false);
    REQUIRE(s.chartfolders.size() == 1);
    CHECK(s.chartfolders[0] == corpus::root());
    CHECK(s.sp_cap == Settings{}.sp_cap);
    CHECK(s.mslimit_value == Settings{}.mslimit_value);
    const AnalysisSettings a = scratch_settings().to_analysis_settings();
    CHECK(a.depth_value == 2);
    CHECK(a.depth_mode == DepthMode::Scores);
}
