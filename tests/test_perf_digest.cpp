// Pins the 97-chart corpus's digests, the same ones hydra_bench's --engine
// and --parse modes print, so a change meant to leave Hydra's output alone
// (the speedups plan, D86) fails here if it moves a single field of the
// engine's answer.
//
// Each literal comes from one run of hydra_bench on testdata/input with the
// engine and parsers unchanged. The digests are tests/song_digest.h's, the
// header the tool uses too. A change that is meant to move results bumps its
// stamp in src/store/stored_versions.h and repins these on purpose; the
// failure message prints the digest the test got.

#include "doctest.h"

#include <cstdint>
#include <cstdio>
#include <exception>
#include <optional>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "parse/song.h"
#include "song_digest.h"
#include "store/record_store.h"

#ifndef HYDRA_INPUT_DIR
#error "HYDRA_INPUT_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

namespace {

// The settings each digest is pinned under, built as the app builds them
// (Settings::batch_run, through to_analysis_settings), with every field the
// plan names set here so a change of app default cannot move the pin. The
// rest (rules, fills) is the app's default, as in an exe folder with no ini.
app::Settings pinned_settings(const char* difficulty, bool pro, bool bass2x) {
    app::Settings s = digest::digest_settings(difficulty, pro, bass2x);
    s.sp_cap = kCloneHeroSpCap;
    s.depth_mode = 0;  // scores
    s.depth_value = 4;
    s.mslimit_enabled = true;
    s.mslimit_value = 10;
    return s;
}

// The corpus as hydra_bench takes a folder: scanned, each chart once as its
// first copy, in scan order.
std::vector<app::ScanItem> corpus_charts() {
    auto [items, errors] = app::discover_charts({HYDRA_INPUT_DIR});
    return app::plan_batch(items, {}).todo;
}

std::string hex(uint64_t h) {
    char buf[17];
    std::snprintf(buf, sizeof buf, "%016llx", static_cast<unsigned long long>(h));
    return buf;
}

// What hydra_bench --engine prints as "hash": every chart's whole engine
// result and prepared row (digest::row_hash), a chart that fails left out.
uint64_t engine_digest(const app::Settings& st) {
    const app::AnalysisSettings settings = st.batch_run().settings;
    uint64_t all = digest::kSeed;
    for (const app::ScanItem& it : corpus_charts()) {
        try {
            const app::AnalysisResult res = app::analyze_chart_file(it.notespath, settings);
            const store::PreparedRow row = store::prepare_row(st.record_key(it.md5), res.record);
            all = digest::fold(all, digest::row_hash(row, res.record));
        } catch (const std::exception&) {
        }
    }
    return all;
}

// What hydra_bench --parse prints as "hash": every chart's parsed song and
// dynamics count, or its failure.
uint64_t parse_digest(const app::Settings& st) {
    const app::AnalysisSettings settings = st.batch_run().settings;
    uint64_t all = digest::kSeed;
    for (const app::ScanItem& it : corpus_charts())
        all = digest::fold(all, digest::chart_parse_hash(it.notespath, settings));
    return all;
}

}  // namespace

TEST_CASE("the corpus's prepared-row digest is pinned") {
    const uint64_t got = engine_digest(pinned_settings("Expert", true, true));
    CHECK_MESSAGE(got == 0x1a0929fcb662ebaaULL, "engine digest is now " << hex(got));
}

TEST_CASE("the corpus's parse digest is pinned") {
    const uint64_t got = parse_digest(pinned_settings("Expert", true, true));
    CHECK_MESSAGE(got == 0x40c54a484d033934ULL, "parse digest is now " << hex(got));
}

TEST_CASE("the corpus's parse digest is pinned at Hard, Pro Drums off, 2x Bass off") {
    const uint64_t got = parse_digest(pinned_settings("Hard", false, false));
    CHECK_MESSAGE(got == 0x1b8cb6f31677c12dULL, "parse digest is now " << hex(got));
}
