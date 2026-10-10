// Pins the chart corpus's digests, the same ones hydra_bench's --engine
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

// One engine digest run: the hash and how many charts analysed. The count
// sits beside the hash so a repin can never hide that charts started failing.
struct EngineDigest {
    uint64_t hash = 0;
    size_t analyzed = 0;
};

// What hydra_bench --engine prints as "hash": every chart's whole engine
// result and prepared row (digest::row_hash), a chart that fails left out.
EngineDigest engine_digest(const app::Settings& st) {
    const app::AnalysisSettings settings = st.batch_run().settings;
    EngineDigest out;
    out.hash = digest::kSeed;
    for (const app::ScanItem& it : corpus_charts()) {
        try {
            const app::AnalysisResult res = app::analyze_chart_file(it.notespath, settings);
            const store::PreparedRow row = store::prepare_row(st.record_key(it.md5), res.record);
            out.hash = digest::fold(out.hash, digest::row_hash(row, res.record));
            ++out.analyzed;
        } catch (const std::exception&) {
        }
    }
    return out;
}

// How many corpus charts load at these settings, by the parse digest's own
// load (digest::chart_parse_hash), so a pin whose difficulty some charts lack
// still checks its count without a typed number.
size_t loading_chart_count(const app::Settings& st) {
    const app::AnalysisSettings settings = st.batch_run().settings;
    size_t n = 0;
    for (const app::ScanItem& it : corpus_charts()) {
        std::string fail;
        digest::chart_parse_hash(it.notespath, settings, &fail);
        if (fail.empty()) ++n;
    }
    return n;
}

// Checks one engine digest against its pinned literal, and that every chart
// that loads at these settings also analysed.
void check_engine_digest(const app::Settings& st, uint64_t pinned) {
    const EngineDigest got = engine_digest(st);
    const size_t loading = loading_chart_count(st);
    CHECK_MESSAGE(got.analyzed == loading,
                  got.analyzed << " charts analysed, " << loading << " load");
    CHECK_MESSAGE(got.hash == pinned, "engine digest is now " << hex(got.hash));
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
    check_engine_digest(pinned_settings("Expert", true, true), 0xb95d0ccd1b78b25dULL);
}

// The three below were each pinned from one run on 2026-10-10 at 013f4c66.

TEST_CASE("the corpus's prepared-row digest is pinned at Hard, Pro Drums off, 2x Bass off") {
    check_engine_digest(pinned_settings("Hard", false, false), 0xe040c57fed344ba9ULL);
}

TEST_CASE("the corpus's prepared-row digest is pinned with the CH 1.0 fill rule") {
    app::Settings s = pinned_settings("Expert", true, true);
    s.legacy_fills = true;
    check_engine_digest(s, 0x768e40e5ea908c08ULL);
}

TEST_CASE("the corpus's prepared-row digest is pinned with Note Shuffle on and the ms limit off") {
    app::Settings s = pinned_settings("Expert", true, true);
    s.view_noteshuffle = true;
    s.mslimit_enabled = false;
    check_engine_digest(s, 0x961d9a8ad453baa0ULL);
}

TEST_CASE("the corpus's parse digest is pinned") {
    const uint64_t got = parse_digest(pinned_settings("Expert", true, true));
    CHECK_MESSAGE(got == 0x40c54a484d033934ULL, "parse digest is now " << hex(got));
}

TEST_CASE("the corpus's parse digest is pinned at Hard, Pro Drums off, 2x Bass off") {
    const uint64_t got = parse_digest(pinned_settings("Hard", false, false));
    CHECK_MESSAGE(got == 0x1b8cb6f31677c12dULL, "parse digest is now " << hex(got));
}
