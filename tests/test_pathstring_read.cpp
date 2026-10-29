// paths_from_pathstring: Hydra's path text read back into a path (the Path
// Builder's paste, plan docs/superpowers/plans/2026-10-28-path-builder.md,
// tasks PB-3 and PB-3b). Every case runs on the corpus (testdata/input) at the GUI's
// default settings.

#include "doctest.h"

#include <cstdint>
#include <functional>
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

// Every path the record keeps on every corpus chart (the main search's and
// the best all-0 pass's, tied variants included), written with `write` and
// read back, is the same path and writes the same text. A chart whose text
// reads two ways fails here, naming the chart and the text.
void check_every_kept_path_round_trips(
    const std::function<std::string(const Path&, const HydraRecord&)>& write) {
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
            const std::string text = write(*p, rec);
            CAPTURE(chart);
            CAPTURE(text);
            const Path got = paths_from_pathstring(song, cfg, text);
            CHECK(act_ticks(got) == act_ticks(*p));
            CHECK(got.pathstring() == p->pathstring());
            CHECK(write(got, rec) == text);
            // The same ends and score too: two tied variants that print the
            // same text would read back as one of them, and this names it.
            CHECK(path_identity(got) == path_identity(*p));
        }
    }
    CHECK(paths > 0);
}

}  // namespace

// The writer and the reader agree on Path::pathstring's text.
TEST_CASE("pathstring read: every corpus best path round-trips") {
    check_every_kept_path_round_trips(
        [](const Path& p, const HydraRecord&) { return p.pathstring(); });
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
    const std::string none = Path().pathstring();
    const Path got = paths_from_pathstring(song, cfg, none);
    CHECK_FALSE(got.has_activations());
    CHECK(got.pathstring() == none);
}

// The writer and the reader agree on the long text Hydra's Copy path writes:
// Path::pathstring_verbose with the record's multiplier squeezes.
TEST_CASE("pathstring read: every corpus path round-trips from Hydra's long Copy path text") {
    check_every_kept_path_round_trips([](const Path& p, const HydraRecord& rec) {
        return p.pathstring_verbose(rec.multsqueezes);
    });
}

// The long text the writer gives a path with no windows, with a real chart's
// multiplier squeezes in its first section, reads back as a path with no
// windows.
TEST_CASE("pathstring read: long text with no activations gives a path with no activations") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::string chart = corpus::first_chart_with_notes();
    const Song& song = corpus::song(chart, cfg.reading());
    const std::string none = Path().pathstring_verbose(corpus::analyzed(chart, cfg).multsqueezes);
    const Path got = paths_from_pathstring(song, cfg, none);
    CHECK_FALSE(got.has_activations());
    CHECK(got.pathstring() == Path().pathstring());
}

// In the long text, a window no fill can place is refused with its position
// too. The first corpus chart whose best path has exactly two windows gets
// its second window's text swapped for one no chart offers.
TEST_CASE("pathstring read: a long-text window no fill can place is refused with its position") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        const Path& best = rec.best_path();
        if (best.walk_activations().size() != 2) continue;
        CAPTURE(chart);
        std::string text = best.pathstring_verbose(rec.multsqueezes);
        const std::string second = best.walk_activations()[1].notationstr_verbose();
        const size_t at = text.rfind(second + " | ");
        REQUIRE(at != std::string::npos);
        text.replace(at, second.size(), "99");
        CAPTURE(text);
        CHECK_THROWS_WITH_AS(paths_from_pathstring(song, cfg, text),
                             "window 2 (at \"99\") matches no fill on this chart",
                             std::invalid_argument);
        return;
    }
    FAIL("no corpus chart's best path has exactly two windows");
}

// Text holding the long form's section separator but not the writer's number
// of sections is refused.
TEST_CASE("pathstring read: long text with the wrong number of sections is refused") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const Song& song = corpus::song(corpus::first_chart_with_notes(), cfg.reading());
    CHECK_THROWS_WITH_AS(paths_from_pathstring(song, cfg, "2x | 1"),
                         "the path text has 2 sections where Hydra's long Copy path text has 3",
                         std::invalid_argument);
}
