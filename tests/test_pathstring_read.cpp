// paths_from_pathstring: Hydra's path text read back into a path (the Path
// Builder's paste, plan docs/superpowers/plans/2026-10-28-path-builder.md (removed; git show b29a3c28:docs/superpowers/plans/2026-10-28-path-builder.md),
// tasks PB-3 and PB-3b). Every case runs on the corpus (testdata/input) at the GUI's
// default settings.

#include "doctest.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "app/config.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/pather.h"
#include "search/pathstring_read.h"

using namespace hydra;

namespace {

using test::act_ticks;

// The song of corpus chart `chart` under `cfg`. Every chart the corpus scan
// lists has Expert drum notes, so a chart that loads empty fails here by name
// instead of being passed over.
const Song& song_with_notes(const std::string& chart, const app::AnalysisSettings& cfg) {
    const Song& song = corpus::song(chart, cfg.reading());
    REQUIRE_MESSAGE(!song.is_empty(), chart << " loads with no notes");
    return song;
}

// The refusal `read` throws: a KindedError of kind PathTextRefused whose text
// names each of `names` (such as a window's position and its token). The wording
// around them is the reader's own (pathstring_read.cpp's refused calls),
// which its header does not give.
void check_refused_naming(const std::function<Path()>& read, const std::vector<std::string>& names) {
    try {
        read();
        FAIL("the text was not refused");
    } catch (const KindedError& e) {
        const std::string what = e.what();
        CAPTURE(what);
        CHECK(e.kind() == ErrorKind::PathTextRefused);
        for (const std::string& name : names) {
            CAPTURE(name);
            CHECK(what.find(name) != std::string::npos);
        }
    }
}

// Every path the record keeps on every corpus chart (the main search's and
// the best all-0 pass's, tied variants included), written with `write` and
// read back, is the same path and writes the same text. A chart whose text
// reads two ways fails here, naming the chart and the text.
void check_every_kept_path_round_trips(
    const std::function<std::string(const Path&, const HydraRecord&)>& write) {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int paths = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = song_with_notes(chart, cfg);
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

// The writer and the reader agree on Path::pathstring's text, for every path
// the record keeps (the main search's and the all-0 pass's), not only the best.
TEST_CASE("pathstring read: every kept path on every corpus chart round-trips") {
    check_every_kept_path_round_trips(
        [](const Path& p, const HydraRecord&) { return p.pathstring(); });
}

// A token no fill can place is refused, and the refusal says which window of
// the text it is (counted from 1) and the token. Every corpus chart whose best
// path has two or more windows keeps its first window and gets a second that
// no chart offers ("99").
TEST_CASE("pathstring read: a token no fill can place is refused with its position") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int checked = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = song_with_notes(chart, cfg);
        const Path& best = corpus::analyzed(chart, cfg).best_path();
        if (best.walk_activations().size() < 2) continue;
        ++checked;
        CAPTURE(chart);
        const std::string text = best.walk_activations()[0].notationstr() + " 99";
        CAPTURE(text);
        check_refused_naming([&] { return paths_from_pathstring(song, cfg, text); },
                             {"window 2 (", "\"99\""});
    }
    const std::string msg = "no corpus chart's best path has two windows, out of " +
                            std::to_string(corpus::chart_paths().size()) + " charts";
    CHECK_MESSAGE(checked > 0, msg);
}

// Every refusal names its kind, which gives the paste its own sentence
// (app::plain_error).
TEST_CASE("pathstring read: every refusal is ErrorKind::PathTextRefused") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const Song& song = corpus::song(corpus::first_chart_with_notes(), cfg.reading());
    for (const std::string& text : {std::string("   "), std::string("99")}) {
        CAPTURE(text);
        try {
            paths_from_pathstring(song, cfg, text);
            FAIL("the text was not refused");
        } catch (const KindedError& e) {
            CHECK(e.kind() == ErrorKind::PathTextRefused);
        }
    }
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
// too. Every corpus chart whose best path has exactly two windows gets its
// second window's text swapped for one no chart offers ("99"); with two
// windows the swapped one is the last, so the refusal's rest of the windows
// section is the token alone.
TEST_CASE("pathstring read: a long-text window no fill can place is refused with its position") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int checked = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = song_with_notes(chart, cfg);
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        const Path& best = rec.best_path();
        if (best.walk_activations().size() != 2) continue;
        ++checked;
        CAPTURE(chart);
        std::string text = best.pathstring_verbose(rec.multsqueezes);
        const std::string second = best.walk_activations()[1].notationstr_verbose();
        const size_t at = text.rfind(second + std::string(kPathSectionSeparator));
        REQUIRE(at != std::string::npos);
        text.replace(at, second.size(), "99");
        CAPTURE(text);
        check_refused_naming([&] { return paths_from_pathstring(song, cfg, text); },
                             {"window 2 (", "\"99\""});
    }
    const std::string msg = "no corpus chart's best path has exactly two windows, out of " +
                            std::to_string(corpus::chart_paths().size()) + " charts";
    CHECK_MESSAGE(checked > 0, msg);
}

// One graph per chart (search_graph), reused for every read on that chart,
// leaves nothing behind from one read to the next: every kept path on every
// corpus chart, in both texts, read on that one graph, is the kept path
// itself, with its engine_hardest_ms figure (the figure a saved row keeps).
TEST_CASE("pathstring read: one graph reused for every read gives back each kept path") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int paths = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = song_with_notes(chart, cfg);
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        const ScoreGraph graph = search_graph(song, cfg);
        std::vector<const Path*> kept = rec.all_paths();
        for (const Path* p : rec.all_allzero_paths()) kept.push_back(p);
        for (const Path* p : kept) {
            for (const std::string& text : {p->pathstring(), p->pathstring_verbose(rec.multsqueezes)}) {
                ++paths;
                CAPTURE(chart);
                CAPTURE(text);
                const Path by_graph = paths_from_pathstring_on_graph(song, graph, text);
                CHECK(path_identity(by_graph) == path_identity(*p));
                CHECK(engine_hardest_ms(by_graph) == engine_hardest_ms(*p));
            }
        }
    }
    CHECK(paths > 0);
}

// Text holding the long form's section separator but not the writer's number
// of sections is refused, naming both counts. The writer's count is read from
// its own long text for a path with no windows, as the reader reads it.
TEST_CASE("pathstring read: long text with the wrong number of sections is refused") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const Song& song = song_with_notes(corpus::first_chart_with_notes(), cfg);
    const std::string none_long = Path().pathstring_verbose({});
    size_t writer_sections = 1;
    for (size_t at = none_long.find(kPathSectionSeparator); at != std::string::npos;
         at = none_long.find(kPathSectionSeparator, at + kPathSectionSeparator.size()))
        ++writer_sections;
    const std::string two = "2x" + std::string(kPathSectionSeparator) + "1";
    REQUIRE(writer_sections != 2);
    check_refused_naming([&] { return paths_from_pathstring(song, cfg, two); },
                         {"has 2 sections", "has " + std::to_string(writer_sections)});
}
