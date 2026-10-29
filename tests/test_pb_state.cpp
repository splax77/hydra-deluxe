// The Path Builder's open chart and the player's path history
// (src/pathbuilder/pb_state.h), and the left list built from the record
// (pb_paths.h). Charts come from the corpus, or are written to a scratch
// folder when the corpus has no chart of the kind a test needs.

#include "doctest.h"

#include <string>

#include "audio_chart_fixtures.h"  // write_text_file
#include "chart_text.h"
#include "program_scope.h"
#include "scratch_settings.h"
#include "temp_util.h"
#include "testdata_dirs.h"
#include "wait_util.h"

#include "pathbuilder/pb_paths.h"
#include "pathbuilder/pb_state.h"

using hydra::pathbuilder::OpenChart;
using hydra::pathbuilder::PathHistory;
using hydra::pathbuilder::PbState;
using hydra::pathbuilder::PlayerPath;

namespace {

// Opens `path` under the GUI tests' settings and waits for the open to end.
void open_and_wait(PbState& s, const std::string& path) {
    s.open_with(path, scratch_settings());
    testwait::wait_until(
        [&] {
            s.poll();
            return !s.busy();
        },
        "the Path Builder's open to finish");
}

}  // namespace

TEST_CASE("pb state: a corpus chart opens with its record, and the list holds every path") {
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

    const auto rows = hydra::pathbuilder::path_rows(c->record, {});
    CHECK(rows.size() == c->record.paths.size() + c->record.allzero_paths.size());
    REQUIRE_FALSE(rows.empty());
    CHECK(rows.front().label == "Best path");
    CHECK(rows.front().gap == 0);
    CHECK(rows.front().total == c->record.best_path().totalscore());
    CHECK(hydra::pathbuilder::gap_text(rows.front()) == "0");
    // The player's path starts empty, with nothing to undo.
    CHECK(s.history().current().windows.empty());
    CHECK_FALSE(s.history().can_undo());
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
