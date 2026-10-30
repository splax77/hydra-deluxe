// The Path Builder's saved path files and its copy and paste
// (src/pathbuilder/pb_files.h; plan docs/superpowers/plans/
// 2026-10-28-path-builder.md, task PB-4). Every case opens the corpus chart
// "Burnout" through the builder's own state, at the GUI tests' settings.

#include "doctest.h"

#include <string>
#include <vector>

#include "replay_json.h"
#include "record_fixtures.h"
#include "scratch_settings.h"
#include "temp_util.h"
#include "testdata_dirs.h"
#include "wait_util.h"

#include "app/user_messages.h"
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
    s.open_with(corpus::burnout_notes(), scratch_settings());
    testwait::wait_until(
        [&] {
            s.poll();
            return !s.busy();
        },
        "the Path Builder's open to finish");
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

}  // namespace

TEST_CASE("pb files: a saved path file opens back as the same path, as a branch and the player's") {
    PbState s;
    const OpenChart& c = open_burnout(s);
    const Path& best = c.record.best_path();
    const std::string file = testtemp::temp_dir("pb_files_save") + "\\path.json";

    // The file is hydra_replay's shape: a "paths" array as paths_json writes it.
    CHECK(hydra::pathbuilder::path_file_json(best, c.song)["paths"] ==
          hydra::paths_json({&best}, c.song.timing()));
    hydra::pathbuilder::save_path_file(file, best, c.song);

    PathHistory& h = s.history();
    std::vector<Path> branches;
    REQUIRE_FALSE(hydra::pathbuilder::open_path_file(h, branches, c, file).has_value());
    REQUIRE(branches.size() == 1);
    check_same_path(branches.front(), best);
    REQUIRE(h.current().path.has_value());
    check_same_path(*h.current().path, best);
    // Opening is one edit: undo puts the empty path back, redo the opened one.
    REQUIRE(h.undo());
    CHECK_FALSE(h.current().path.has_value());
    REQUIRE(h.redo());
    REQUIRE(h.current().path.has_value());
    check_same_path(*h.current().path, best);
}

TEST_CASE("pb files: a path file that cannot be read changes nothing and says why") {
    PbState s;
    const OpenChart& c = open_burnout(s);
    const std::string file = testtemp::temp_dir("pb_files_missing") + "\\missing.json";
    PathHistory& h = s.history();
    std::vector<Path> branches;
    const std::optional<Refusal> refused = hydra::pathbuilder::open_path_file(h, branches, c, file);
    REQUIRE(refused.has_value());
    CHECK(refused->message == hydra::app::kSomethingWentWrong);
    CHECK(refused->detail == "cannot read the path file: " + file);
    CHECK(branches.empty());
    CHECK_FALSE(h.can_undo());
    CHECK_FALSE(h.current().path.has_value());
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
    CHECK(refused->message == hydra::app::kSomethingWentWrong);
    CHECK(refused->detail == "window 1 (\"99\") matches no fill on this chart");
    CHECK(h.generation() == generation);
    CHECK_FALSE(h.can_redo());
    REQUIRE(h.current().path.has_value());
    check_same_path(*h.current().path, best);
    // The paste before it is still the one edit to undo.
    REQUIRE(h.undo());
    CHECK_FALSE(h.can_undo());
}
