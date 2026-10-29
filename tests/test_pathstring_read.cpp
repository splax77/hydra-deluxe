// paths_from_pathstring: Hydra's path text read back into a path (the Path
// Builder's paste, plan docs/superpowers/plans/2026-10-28-path-builder.md,
// task PB-3). Every case runs on the corpus (testdata/input) at the GUI's
// default settings.

#include "doctest.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/config.h"
#include "core/model.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/pather.h"
#include "search/pathstring_read.h"

using namespace hydra;

namespace {

using test::act_ticks;

}  // namespace

// The writer and the reader agree: every path the record keeps on every
// corpus chart (the main search's and the best all-0 pass's, tied variants
// included), written with Path::pathstring and read back, is the same path.
// A chart whose text reads two ways fails here, naming the chart and the
// text.
TEST_CASE("pathstring read: every corpus best path round-trips") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int paths = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        std::vector<const Path*> kept = rec.all_paths();
        for (const Path* p : rec.all_allzero_paths()) kept.push_back(p);
        for (const Path* p : kept) {
            ++paths;
            CAPTURE(chart);
            CAPTURE(p->pathstring());
            const Path got = paths_from_pathstring(song, cfg, p->pathstring());
            CHECK(act_ticks(got) == act_ticks(*p));
            CHECK(got.pathstring() == p->pathstring());
        }
    }
    CHECK(paths > 0);
}

// A token no fill can place is refused, and the refusal says which window of
// the text it is (counted from 1). The first corpus chart whose best path has
// two windows keeps its first window and gets a second no chart offers.
TEST_CASE("pathstring read: a token no fill can place is refused with its position") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        const Path& best = corpus::analyzed(chart, cfg).best_path();
        if (best.walk_activations().size() < 2) continue;
        CAPTURE(chart);
        const std::string text = best.walk_activations()[0].notationstr() + " 99";
        CHECK_THROWS_WITH_AS(paths_from_pathstring(song, cfg, text),
                             "window 2 (\"99\") matches no fill on this chart",
                             std::invalid_argument);
        return;
    }
    FAIL("no corpus chart's best path has two windows");
}

// The text Path::pathstring writes for a path with no windows reads back as a
// path with no windows.
TEST_CASE("pathstring read: \"(No activations.)\" gives a path with no activations") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const Song& song = corpus::song(corpus::first_chart_with_notes(), cfg.reading());
    const Path got = paths_from_pathstring(song, cfg, "(No activations.)");
    CHECK_FALSE(got.has_activations());
    CHECK(got.pathstring() == "(No activations.)");
}
