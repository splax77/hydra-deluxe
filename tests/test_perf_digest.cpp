// Checks the per-chart digests hydra_bench's --engine and --parse modes fold
// (tests/song_digest.h) on every corpus chart. Comparing two builds' digests
// only proves a change left Hydra's output alone when the same build gives
// the same digest every time, so each test reads every chart twice and
// checks the two answers match, chart by chart. Each also checks which
// charts it reads: every chart that loads analyses, and a chart that does
// not load has no notes at those settings (NoNotesError), never another
// failure.

#include "doctest.h"

#include <cstdint>
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

// The settings each check runs under, built as the app builds them
// (Settings::batch_run, through to_analysis_settings), with the depth, SP cap
// and ms limit set here so a change of app default cannot change which
// search the checks read. The rest (rules, fills) is the app's default, as in
// an exe folder with no ini.
app::Settings digest_run_settings(const char* difficulty, bool pro, bool bass2x) {
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

// One chart's engine answer, as hydra_bench --engine hashes it
// (digest::row_hash), or nullopt when the chart fails to analyse.
std::optional<uint64_t> engine_row(const app::ScanItem& it, const app::Settings& st,
                                   const app::AnalysisSettings& settings) {
    try {
        const app::AnalysisResult res = app::analyze_chart_file(it.notespath, settings);
        const store::PreparedRecord prepared =
            store::prepare_record(it.md5, st.profile(), res.record);
        return digest::row_hash(prepared, res.record);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

// The sentence a chart with no notes at these settings fails with, in the
// form the parse digest records a failure (digest::failure_text).
std::string no_notes_text(const app::AnalysisSettings& settings) {
    const NoNotesError none(settings.reading().difficulty, settings.reading().prodrums);
    return digest::failure_text(none);
}

// Checks that every chart that does not load at these settings fails only
// because it has no notes there, and that at least one chart loads. Returns
// how many load, by the parse digest's own load (digest::chart_parse_hash).
size_t check_loading(const std::vector<app::ScanItem>& charts,
                   const app::AnalysisSettings& settings) {
    const std::string none = no_notes_text(settings);
    size_t loading = 0;
    for (const app::ScanItem& it : charts) {
        std::string fail;
        digest::chart_parse_hash(it.notespath, settings, &fail);
        if (fail.empty()) {
            ++loading;
            continue;
        }
        CHECK_MESSAGE(fail == none, it.notespath << " fails with " << fail);
    }
    CHECK(loading > 0);
    return loading;
}

// Analyses every corpus chart twice and checks each chart's two answers are
// the same, that every chart that loads analysed, and that every chart that
// does not load has no notes at these settings.
void check_engine_digest(const app::Settings& st) {
    const app::AnalysisSettings settings = st.batch_run().settings;
    const std::vector<app::ScanItem> charts = corpus_charts();
    size_t analysed = 0;
    for (const app::ScanItem& it : charts) {
        const std::optional<uint64_t> first = engine_row(it, st, settings);
        const std::optional<uint64_t> second = engine_row(it, st, settings);
        CHECK_MESSAGE((first == second), it.notespath << " gives two different answers");
        if (first) ++analysed;
    }
    const size_t loading = check_loading(charts, settings);
    CHECK_MESSAGE(analysed == loading, analysed << " charts analysed, " << loading << " load");
}

// Parses every corpus chart twice and checks each chart's two parse digests
// (digest::chart_parse_hash) are the same, and that every chart that does
// not load has no notes at these settings.
void check_parse_digest(const app::Settings& st) {
    const app::AnalysisSettings settings = st.batch_run().settings;
    const std::vector<app::ScanItem> charts = corpus_charts();
    for (const app::ScanItem& it : charts) {
        const uint64_t first = digest::chart_parse_hash(it.notespath, settings);
        const uint64_t second = digest::chart_parse_hash(it.notespath, settings);
        CHECK_MESSAGE((first == second), it.notespath << " parses two different ways");
    }
    check_loading(charts, settings);
}

}  // namespace

TEST_CASE("every corpus chart's prepared row is the same on a second analysis") {
    check_engine_digest(digest_run_settings("Expert", true, true));
}

TEST_CASE("every corpus chart's prepared row is the same on a second analysis at Hard, Pro Drums off, 2x Bass off") {
    check_engine_digest(digest_run_settings("Hard", false, false));
}

TEST_CASE("every corpus chart's prepared row is the same on a second analysis with the CH 1.0 fill rule") {
    app::Settings s = digest_run_settings("Expert", true, true);
    s.legacy_fills = true;
    check_engine_digest(s);
}

TEST_CASE("every corpus chart's prepared row is the same on a second analysis with Note Shuffle on and the ms limit off") {
    app::Settings s = digest_run_settings("Expert", true, true);
    s.view_noteshuffle = true;
    s.mslimit_enabled = false;
    check_engine_digest(s);
}

TEST_CASE("every corpus chart parses the same way twice") {
    check_parse_digest(digest_run_settings("Expert", true, true));
}

TEST_CASE("every corpus chart parses the same way twice at Hard, Pro Drums off, 2x Bass off") {
    check_parse_digest(digest_run_settings("Hard", false, false));
}

TEST_CASE("every corpus chart parses the same way twice at Medium, Pro Drums off, 2x Bass off") {
    check_parse_digest(digest_run_settings("Medium", false, false));
}

TEST_CASE("every corpus chart parses the same way twice at Easy, Pro Drums off, 2x Bass off") {
    check_parse_digest(digest_run_settings("Easy", false, false));
}
