// The Path Builder's saved path files and its copy and paste
// (src/pathbuilder/pb_files.h; plan
// docs/superpowers/plans/2026-10-28-path-builder.md (removed; git show b29a3c28:docs/superpowers/plans/2026-10-28-path-builder.md), task PB-4). Every case opens the corpus chart
// "Burnout" through the builder's own state, at the GUI tests' settings.

#include "doctest.h"

#include <string>
#include <vector>

#include "audio_chart_fixtures.h"  // write_text_file
#include "replay_json.h"
#include "record_fixtures.h"
#include "pb_open_util.h"
#include "temp_util.h"
#include "testdata_dirs.h"

#include "app/user_messages.h"
#include "pathbuilder/pb_coach.h"
#include "pathbuilder/pb_files.h"
#include "pathbuilder/pb_state.h"

using hydra::Path;
using hydra::pathbuilder::OpenChart;
using hydra::pathbuilder::PathHistory;
using hydra::pathbuilder::PbState;
using hydra::pathbuilder::Refusal;
using hydra::test::act_ticks;

namespace {

// Burnout, opened under the GUI tests' settings, with the open finished.
const OpenChart& open_burnout(PbState& s) {
    open_and_wait(s, corpus::burnout_notes());
    REQUIRE(s.error().empty());
    REQUIRE(s.chart() != nullptr);
    REQUIRE_FALSE(s.chart()->record.paths.empty());
    return *s.chart();
}

// The same path: the same windows and the same text.
void check_same_path(const Path& got, const Path& want) {
    CHECK(act_ticks(got) == act_ticks(want));
    CHECK(got.pathstring() == want.pathstring());
    CHECK(path_identity(got) == path_identity(want));
}

// A one-window path the player might build: one window on the second
// activation of the first record path that has two or more.
hydra::pathbuilder::ClickResult one_window_path(const OpenChart& c) {
    const Path* two = nullptr;
    for (const Path& p : c.record.paths)
        if (p.walk_activations().size() >= 2) {
            two = &p;
            break;
        }
    REQUIRE(two != nullptr);
    const hydra::ActivationWalk acts = two->walk_activations();
    hydra::pathbuilder::ClickResult one =
        hydra::pathbuilder::start_window(*c.graph, c.settings, {}, acts[1].timecode.ticks());
    REQUIRE(one.path.has_value());
    return one;
}

}  // namespace

TEST_CASE("pb files: a saved path file opens back as the same path, as the player's") {
    PbState s;
    const OpenChart& c = open_burnout(s);
    // A path that is not one of Hydra's.
    const hydra::pathbuilder::ClickResult one = one_window_path(c);
    const Path& mine = *one.path;
    for (const Path& p : c.record.paths) REQUIRE(path_identity(p) != path_identity(mine));
    const std::string file = testtemp::temp_dir("pb_files_save") + "\\path.json";
    hydra::pathbuilder::save_path_file(file, mine, c.song);

    PathHistory& h = s.history();
    REQUIRE_FALSE(hydra::pathbuilder::open_path_file(h, c, file).has_value());
    REQUIRE(h.current().path.has_value());
    check_same_path(*h.current().path, mine);
    // Opening is one edit: undo puts the empty path back, redo the opened one.
    REQUIRE(h.undo());
    CHECK_FALSE(h.current().path.has_value());
    REQUIRE(h.redo());
    REQUIRE(h.current().path.has_value());
    check_same_path(*h.current().path, mine);
}

TEST_CASE("pb files: a real hydra_replay dump opens its first path as the player's") {
    // testdata/replay/burnout-dump.json is hydra_replay's own dump of the
    // corpus Burnout at the GUI tests' settings (scratch_settings: the app's
    // defaults, depth 2 scores), made by
    //   hydra_replay dump --chart "<corpus>/common/Summer Blast _25 Setlist/
    //     Tier 4/Green Day - Burnout/notes.mid" --depth-mode scores --depth 2
    //     --out testdata/replay/burnout-dump.json
    // Its "paths" array is rec.all_paths() in order, so its first entry is
    // the record's first path.
    PbState s;
    const OpenChart& c = open_burnout(s);
    const std::vector<const Path*> all = c.record.all_paths();
    REQUIRE_FALSE(all.empty());
    PathHistory& h = s.history();
    const std::string dump = corpus::testdata_root() + "/replay/burnout-dump.json";
    REQUIRE_FALSE(hydra::pathbuilder::open_path_file(h, c, dump).has_value());
    REQUIRE(h.current().path.has_value());
    CHECK(act_ticks(*h.current().path) == act_ticks(*all.front()));
}

// The user's answer Q2 (docs/superpowers/plans/2026-10-30-path-builder-redesign.md (removed; git show 307d0b5c:docs/superpowers/plans/2026-10-30-path-builder-redesign.md)):
// a path file opens its first path as the player's, as one edit, and leaves
// the rest out.
TEST_CASE("pb files: a path file's first path becomes the player's") {
    PbState s;
    const OpenChart& c = open_burnout(s);
    const hydra::pathbuilder::ClickResult one = one_window_path(c);
    const Path& best = c.record.best_path();
    REQUIRE(path_identity(best) != path_identity(*one.path));
    // A file of two paths, in hydra_replay's shape: the one-window path first.
    const std::string file = testtemp::temp_dir("pb_files_first") + "\\two.json";
    nlohmann::json doc;
    doc["paths"] = hydra::paths_json({&*one.path, &best}, c.song.timing());
    audiochart::write_text_file(file, doc.dump());

    PathHistory& h = s.history();
    REQUIRE_FALSE(hydra::pathbuilder::open_path_file(h, c, file).has_value());
    REQUIRE(h.current().path.has_value());
    check_same_path(*h.current().path, *one.path);
    // One edit: one undo puts the empty path back, and there is no other.
    REQUIRE(h.undo());
    CHECK_FALSE(h.current().path.has_value());
    CHECK_FALSE(h.can_undo());
}

TEST_CASE("pb files: a path file that cannot be read changes nothing and says why") {
    PbState s;
    const OpenChart& c = open_burnout(s);
    const std::string file = testtemp::temp_dir("pb_files_missing") + "\\missing.json";
    PathHistory& h = s.history();
    const std::optional<Refusal> refused = hydra::pathbuilder::open_path_file(h, c, file);
    REQUIRE(refused.has_value());
    CHECK(refused->message == "Hydra couldn't read this path file, so your path stays as it was.");
    CHECK(refused->detail == "cannot read the path file: " + file);
    CHECK_FALSE(h.can_undo());
    CHECK_FALSE(h.current().path.has_value());
}

TEST_CASE("pb files: a path file that does not fit the chart changes nothing and says why") {
    PbState s;
    const OpenChart& c = open_burnout(s);
    const std::string dir = testtemp::temp_dir("pb_files_refused");
    PathHistory& h = s.history();

    // Not JSON at all.
    const std::string not_json = dir + "\\not_json.json";
    audiochart::write_text_file(not_json, "not a path");
    std::optional<Refusal> refused = hydra::pathbuilder::open_path_file(h, c, not_json);
    REQUIRE(refused.has_value());
    CHECK(refused->message ==
          "This file doesn't hold a path for this chart, so your path stays as it was.");
    CHECK(refused->detail == "the path file " + not_json + " is not valid JSON");

    // hydra_replay's shape, with a window on a tick Burnout has no fill on.
    const std::string off_chart = dir + "\\off_chart.json";
    audiochart::write_text_file(
        off_chart, R"({"paths": [{"activations": [{"act_tick": 1, "deact_tick": 2}]}]})");
    refused = hydra::pathbuilder::open_path_file(h, c, off_chart);
    REQUIRE(refused.has_value());
    CHECK(refused->message ==
          "This file doesn't hold a path for this chart, so your path stays as it was.");
    CHECK(refused->detail.rfind("path 1 of " + off_chart, 0) == 0);

    CHECK_FALSE(h.can_undo());
    CHECK_FALSE(h.current().path.has_value());
}

TEST_CASE("pb files: a path file that cannot be written says why") {
    PbState s;
    const OpenChart& c = open_burnout(s);
    // A folder that does not exist under the scratch folder.
    const std::string file = testtemp::temp_dir("pb_files_unwritable") + "\\missing\\path.json";
    try {
        hydra::pathbuilder::save_path_file(file, c.record.best_path(), c.song);
        FAIL("the save did not refuse");
    } catch (const std::exception& e) {
        CHECK(hydra::app::plain_error(e) ==
              "Hydra couldn't save the path file. Check that the disk isn't full and the folder "
              "isn't read-only.");
        CHECK(hydra::app::plain_error_detail(e) == "cannot write the path file: " + file);
    }
}

// Copy writes the long text Hydra's Copy path writes, so a Copy that wrote
// the short form fails here.
TEST_CASE("pb files: Copy writes Hydra's long Copy path text") {
    PbState s;
    const OpenChart& c = open_burnout(s);
    const Path& best = c.record.best_path();
    const std::string text = hydra::pathbuilder::copy_text(c.record, best);
    CHECK(text == best.pathstring_verbose(c.record.multsqueezes));
    CHECK(text != best.pathstring());
}

TEST_CASE("pb files: a copied path pastes back as the same path, long or short") {
    PbState s;
    const OpenChart& c = open_burnout(s);
    PathHistory& h = s.history();
    for (const Path& p : c.record.paths) {
        CAPTURE(p.pathstring());
        // The long text Copy writes.
        REQUIRE_FALSE(
            hydra::pathbuilder::paste_path(h, c, hydra::pathbuilder::copy_text(c.record, p))
                .has_value());
        REQUIRE(h.current().path.has_value());
        check_same_path(*h.current().path, p);
        // The short text Path::pathstring writes gives the same path.
        REQUIRE(h.undo());
        REQUIRE_FALSE(hydra::pathbuilder::paste_path(h, c, p.pathstring()).has_value());
        REQUIRE(h.current().path.has_value());
        check_same_path(*h.current().path, p);
    }
}

// The spec's Errors section: a pasted path that does not fit the chart is
// refused with the reason, and the player's path stays as it was.
TEST_CASE("pb files: a paste that does not fit the chart is refused and leaves the path as it was") {
    PbState s;
    const OpenChart& c = open_burnout(s);
    PathHistory& h = s.history();
    const Path& best = c.record.best_path();
    REQUIRE_FALSE(hydra::pathbuilder::paste_path(h, c, best.pathstring()).has_value());
    const uint64_t generation = h.generation();

    const std::optional<Refusal> refused = hydra::pathbuilder::paste_path(h, c, "99");
    REQUIRE(refused.has_value());
    // The user's own words for a paste that does not fit (PB-4, 2026-10-29).
    CHECK(refused->message ==
          "This text doesn't match a path on this chart, so your path stays as it was.");
    CHECK(refused->detail == "window 1 (\"99\") matches no fill on this chart");
    CHECK(h.generation() == generation);
    CHECK_FALSE(h.can_redo());
    REQUIRE(h.current().path.has_value());
    check_same_path(*h.current().path, best);
    // The paste before it is still the one edit to undo.
    REQUIRE(h.undo());
    CHECK_FALSE(h.can_undo());
}
