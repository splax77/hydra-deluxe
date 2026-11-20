// Pins the chart corpus's digests, the same ones hydra_bench's --engine
// and --parse modes print, so a change meant to leave Hydra's output alone
// (D86) fails here if it moves a single field of the engine's answer. "The
// plan" below is docs/superpowers/plans/2026-10-06-perf-speedups.md (removed; git show 444631e6:docs/superpowers/plans/2026-10-06-perf-speedups.md).
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
#include "testdata_dirs.h"  // root

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
    auto [items, errors] = app::discover_charts({corpus::root()});
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
// result and prepared record (digest::row_hash), a chart that fails left out.
EngineDigest engine_digest(const app::Settings& st) {
    const app::AnalysisSettings settings = st.batch_run().settings;
    EngineDigest out;
    out.hash = digest::kSeed;
    for (const app::ScanItem& it : corpus_charts()) {
        try {
            const app::AnalysisResult res = app::analyze_chart_file(it.notespath, settings);
            const store::PreparedRecord prepared =
                store::prepare_record(it.md5, st.profile(), res.record);
            out.hash = digest::fold(out.hash, digest::row_hash(prepared, res.record));
            ++out.analyzed;
        } catch (const std::exception&) {
        }
    }
    return out;
}

// How many of the digest's charts load at these settings
// (digest::loading_chart_count), so a pin whose difficulty some charts lack
// still checks its count without a typed number.
size_t loading_chart_count(const app::Settings& st) {
    std::vector<std::string> notespaths;
    for (const app::ScanItem& it : corpus_charts()) notespaths.push_back(it.notespath);
    return digest::loading_chart_count(notespaths, st.batch_run().settings);
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

// Checks one parse digest against its pinned literal, and how many charts
// load at these settings against the count pinned beside it, so a repin can
// never hide that charts stopped loading.
void check_parse_digest(const app::Settings& st, uint64_t pinned, size_t pinned_loading) {
    const uint64_t got = parse_digest(st);
    const size_t loading = loading_chart_count(st);
    CHECK_MESSAGE(loading == pinned_loading, loading << " charts load");
    CHECK_MESSAGE(got == pinned, "parse digest is now " << hex(got));
}

}  // namespace

// Each of the six digests below is pinned from one run.

TEST_CASE("the corpus's prepared-row digest is pinned") {
    check_engine_digest(pinned_settings("Expert", true, true), 0xf5ba2b3eeca8426dULL);
}

TEST_CASE("the corpus's prepared-row digest is pinned at Hard, Pro Drums off, 2x Bass off") {
    check_engine_digest(pinned_settings("Hard", false, false), 0x7cae719f5799dce9ULL);
}

TEST_CASE("the corpus's prepared-row digest is pinned with the CH 1.0 fill rule") {
    app::Settings s = pinned_settings("Expert", true, true);
    s.legacy_fills = true;
    check_engine_digest(s, 0xabd1a401a72080a9ULL);
}

TEST_CASE("the corpus's prepared-row digest is pinned with Note Shuffle on and the ms limit off") {
    app::Settings s = pinned_settings("Expert", true, true);
    s.view_noteshuffle = true;
    s.mslimit_enabled = false;
    check_engine_digest(s, 0x2ee8e78d4c2ddac4ULL);
}

TEST_CASE("the corpus's parse digest is pinned") {
    const uint64_t got = parse_digest(pinned_settings("Expert", true, true));
    CHECK_MESSAGE(got == 0x2d2b6ba616fa0df2ULL, "parse digest is now " << hex(got));
}

TEST_CASE("the corpus's parse digest is pinned at Hard, Pro Drums off, 2x Bass off") {
    const uint64_t got = parse_digest(pinned_settings("Hard", false, false));
    CHECK_MESSAGE(got == 0xdbbb09ba79b73c78ULL, "parse digest is now " << hex(got));
}

// Pinned from one run, with the parsers unchanged.
TEST_CASE("the corpus's parse digest is pinned at Medium, Pro Drums off, 2x Bass off") {
    check_parse_digest(pinned_settings("Medium", false, false), 0x1f78ad0ac3f9a90aULL, 68);
}

TEST_CASE("the corpus's parse digest is pinned at Easy, Pro Drums off, 2x Bass off") {
    check_parse_digest(pinned_settings("Easy", false, false), 0xa5187516819c3195ULL, 68);
}
