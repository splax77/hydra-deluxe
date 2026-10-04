// tests/corpus_util.h caches corpus parses and analyses for the whole run.
// These checks pin what the corpus loops rely on: one answer per chart and
// settings, the same answer a direct call gives, and a failure that repeats
// instead of turning into a silent empty result.

#include "doctest.h"

#include <string>

#include "core/model.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "search/pather.h"

using namespace hydra;

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
