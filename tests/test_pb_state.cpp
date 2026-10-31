// The Path Builder's open chart and the player's path history
// (src/pathbuilder/pb_state.h). Charts come from the corpus, or are written
// to a scratch folder when the corpus has no chart of the kind a test needs.

#include "doctest.h"

#include <string>
#include <vector>

#include "audio_chart_fixtures.h"  // write_text_file
#include "chart_text.h"
#include "pb_open_util.h"
#include "program_scope.h"
#include "temp_util.h"
#include "testdata_dirs.h"

#include "pathbuilder/pb_state.h"

using hydra::pathbuilder::OpenChart;
using hydra::pathbuilder::PathHistory;
using hydra::pathbuilder::PbState;
using hydra::pathbuilder::PlayerPath;

TEST_CASE("pb state: a corpus chart opens with its record and its graph") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes());
    REQUIRE(s.error().empty());
    const OpenChart* c = s.chart();
    REQUIRE(c != nullptr);
    CHECK(c->has_star_power());
    REQUIRE_FALSE(c->record.paths.empty());
    CHECK(c->graph != nullptr);
    CHECK_FALSE(c->entry.md5.empty());
    CHECK(c->entry.notespath == corpus::burnout_notes());
    // The song.ini's name and artist, and the title line they make. From one run.
    CHECK(c->entry.title == "Burnout");
    CHECK(c->entry.artist == "Green Day");
    CHECK(hydra::pathbuilder::chart_title_line(*c) == "Burnout — Green Day");
    // The player's path starts empty, with nothing to undo.
    CHECK(s.history().current().windows.empty());
    CHECK_FALSE(s.history().can_undo());
}

// "The empty path" in docs/superpowers/plans/2026-10-30-path-builder-redesign.md:
// the open finds the path with no activation, and the player's own path stays
// empty (Copy and Save read only that).
TEST_CASE("pb state: a newly opened chart has its empty path") {
    PbState s;
    open_and_wait(s, corpus::burnout_notes());
    REQUIRE(s.error().empty());
    const OpenChart* c = s.chart();
    REQUIRE(c != nullptr);
    REQUIRE(c->empty_path.has_value());
    const hydra::Path& empty = *c->empty_path;
    CHECK(empty.walk_activations().empty());
    // From one run.
    CHECK(empty.totalscore() == 313635);
    CHECK(empty.trailing_skipped_fill_ticks ==
          std::vector<int64_t>{36480, 44160, 51840, 59520, 67200, 74880, 83040, 90240, 98400,
                               109440, 117120, 147840, 156000, 167040, 174720});
    CHECK_FALSE(s.history().current().path.has_value());
}

TEST_CASE("pb state: a chart with no Star Power phrase opens and says there is nothing to build") {
    using namespace testchart;
    const std::string dir = testtemp::temp_dir("pb_no_sp");
    const std::string notes = dir + "\\notes.chart";
    audiochart::write_text_file(
        notes, chart_text(section("ExpertDrums", line(0, "N 1 0") + line(192, "N 2 0") +
                                                     line(384, "N 1 0"))));
    PbState s;
    open_and_wait(s, notes);
    REQUIRE(s.error().empty());
    REQUIRE(s.chart() != nullptr);
    CHECK_FALSE(s.chart()->has_star_power());
    // No song.ini, so no name: the title line is the file path.
    CHECK(s.chart()->entry.title.empty());
    CHECK(hydra::pathbuilder::chart_title_line(*s.chart()) == notes);
    CHECK(std::string(hydra::pathbuilder::kNothingToBuildText) ==
          "This chart has no Star Power phrases, so there is no path to build.");
}

// D267: the builder has no library and no Scan button, so its sentence leaves
// the Scan advice out.
TEST_CASE("pb state: a chart that fails to open says why in the plain sentence") {
    ProgramScope builder(hydra::app::Program::PathBuilder);
    PbState s;
    const std::string missing = testtemp::temp_dir("pb_missing") + "\\notes.mid";
    open_and_wait(s, missing);
    CHECK(s.chart() == nullptr);
    CHECK(s.error() == "Hydra couldn't open the song file. It may have been moved or deleted.");
    CHECK(s.error_detail() == "cannot open file: " + missing);
}

TEST_CASE("pb state: undo and redo walk three edits back and forth") {
    PathHistory h;
    auto state_with = [](int64_t tick) {
        PlayerPath p;
        p.windows.push_back({tick, std::nullopt, false, std::nullopt});
        return p;
    };
    h.edit(state_with(100));
    h.edit(state_with(200));
    h.edit(state_with(300));
    CHECK(h.current().windows.at(0).act_tick == 300);
    CHECK_FALSE(h.can_redo());

    REQUIRE(h.undo());
    CHECK(h.current().windows.at(0).act_tick == 200);
    REQUIRE(h.undo());
    REQUIRE(h.undo());
    CHECK(h.current().windows.empty());  // the start: no window
    CHECK_FALSE(h.undo());               // nothing left to undo

    REQUIRE(h.redo());
    CHECK(h.current().windows.at(0).act_tick == 100);
    // An edit after an undo drops what could have been redone.
    h.edit(state_with(150));
    CHECK_FALSE(h.can_redo());
    REQUIRE(h.undo());
    CHECK(h.current().windows.at(0).act_tick == 100);
    REQUIRE(h.redo());
    CHECK(h.current().windows.at(0).act_tick == 150);
}
