// The Path Builder's sentences (pathbuilder/pb_text.h; plan
// docs/superpowers/plans/2026-10-30-path-builder-redesign.md, task CO-1).
// The corpus cases run at the GUI's default settings, whose Path limit is
// 10 ms; every sentence is pinned from one run.

#include "doctest.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/preview_view.h"
#include "core/model.h"
#include "corpus_util.h"
#include "pathbuilder/pb_text.h"
#include "search/pather.h"

using namespace hydra;
using namespace hydra::pathbuilder;

namespace {

const app::AnalysisSettings& default_cfg() {
    static const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    return cfg;
}

const Song& song_named(const std::string& name) {
    const std::string chart = corpus::chart_named(name);
    REQUIRE_FALSE(chart.empty());
    return corpus::song(chart, default_cfg().reading(), default_cfg().rules);
}

// The one path search_target makes of activation-only pins at `ticks`.
Path player_path(const Song& song, const std::vector<int64_t>& ticks) {
    std::vector<PinnedWindow> wins;
    for (const int64_t t : ticks) wins.push_back(PinnedWindow{t, std::nullopt, false, std::nullopt});
    const TargetResult tr = search_target(song, default_cfg(), wins);
    REQUIRE(tr.paths.size() >= 1);
    return tr.paths[0];
}

app::PreviewScene scene_for(const Song& song, const Path& path) {
    return app::build_preview_scene(song, &path, default_cfg().sp_cap, default_cfg().rules);
}

size_t fill_index(const app::PreviewScene& scene, int64_t chord_tick) {
    for (size_t i = 0; i < scene.fills.size(); ++i)
        if (scene.fills[i].chord_tick == chord_tick) return i;
    FAIL("no fill at tick " << chord_tick);
    return 0;
}

}  // namespace

TEST_CASE("fill reason: inside a window, before the last activation, under two bars, refused by "
          "the game, and none for an offered fill") {
    // Trivium "Feast of Fire". With no activation, its third fill comes with
    // enough bars saved and the engine still does not offer it.
    const Song& song = song_named("Trivium - Feast of Fire");
    const app::PreviewScene empty = scene_for(song, player_path(song, {}));
    REQUIRE(empty.fills.size() >= 4);
    CHECK(fill_reason(empty, 0) == "You have 1 phrase saved here. You need 2 to activate.");
    CHECK(fill_reason(empty, 2) == "The game doesn't show this fill here.");
    CHECK(fill_reason(empty, 3).empty());
    const HydraRecord& rec =
        corpus::analyzed(corpus::chart_named("Trivium - Feast of Fire"), default_cfg());
    // On Hydra's best path, the fill after the first activation's fill is
    // inside that window, and a fill the path passed between two windows is
    // before the last activation although the engine offered it.
    const app::PreviewScene best = scene_for(song, rec.best_path());
    REQUIRE(best.fills.size() > 5);
    CHECK(best.fills[4].state == app::PreviewFillState::Taken);
    CHECK(fill_reason(best, 4).empty());
    CHECK(best.fills[5].chord_tick == 72480);
    CHECK(fill_reason(best, 5) == "Star Power is already on here. Activation 1 runs until m40.");
    CHECK(best.fills[3].state == app::PreviewFillState::Offered);
    CHECK(fill_reason(best, 3) == kBeforeLastActivationText);
}

TEST_CASE("step sentence: a squeeze-out over the limit carries the ms figure and the violet note") {
    // August Burns Red "White Washed": its best path's fourth window needs a
    // squeeze-out tighter than the 10 ms limit (test_path_costs.cpp).
    const std::string name = "August Burns Red - White Washed";
    const Song& song = song_named(name);
    const HydraRecord& rec = corpus::analyzed(corpus::chart_named(name), default_cfg());
    const std::vector<Activation> acts = rec.best_path().all_activations();
    REQUIRE(acts.size() == 5);
    const app::PreviewScene scene = scene_for(song, rec.best_path());
    const StepText s =
        step_sentence(*scene.timing, acts[3], 1234, 707414, 706180, default_cfg().ms_filter);
    // The cost and the two best finishes are the caller's figures; these are
    // made up to show both wordings.
    CHECK(s.sentence ==
          "Activate at m132 (2:51.485) with 2 phrases saved. Star Power runs until m136, ending "
          "with a squeeze-out that needs 130.4 ms. This choice costs 1,234 points.");
    CHECK(s.why == "Before it, the best you could still finish with was 707,414. After it, 706,180.");
    CHECK(s.note == "needs 130.4 ms, your limit is 10 ms");
    const StepText free =
        step_sentence(*scene.timing, acts[0], 0, 706180, 706180, default_cfg().ms_filter);
    CHECK(free.sentence ==
          "Activate at m23 (0:43.279) with 2 phrases saved. Star Power runs until m29. This "
          "choice costs nothing.");
    CHECK(free.why == "The best you can still finish with stays at 706,180.");
    CHECK(free.note.empty());
}

TEST_CASE("guide text: step 1 names the earliest offered fill; a finished path says so") {
    // Allister "Overrated": with no activation the first fill comes with one
    // bar saved, so the earliest the player can activate is the second.
    const Song& song = song_named("Allister - Overrated");
    const app::PreviewScene empty = scene_for(song, player_path(song, {}));
    GuideFacts start;
    for (size_t i = 0; i < empty.fills.size(); ++i) {
        if (empty.fills[i].state == app::PreviewFillState::Offered && fill_reason(empty, i).empty()) {
            start.earliest_fill = fill_name(empty, empty.fills[i]);
            start.earliest_ms = empty.timing->timecode(empty.fills[i].chord_tick).ms();
            break;
        }
    }
    start.fills_after_last = empty.trailing_skipped_fill_ticks.size();
    const GuideMessage step1 = guide_text(start);
    CHECK(step1.text ==
          "Step 1. Click a glowing fill to make your first activation. The earliest you can "
          "activate is m30 (0:45.789), once you've saved 2 phrases. Hover a fill to see it on "
          "the highway.");
    CHECK_FALSE(step1.refuse);

    // Hydra's best path ends with no fill offered after its last window.
    const HydraRecord& rec =
        corpus::analyzed(corpus::chart_named("Allister - Overrated"), default_cfg());
    const Path& best = rec.best_path();
    const app::PreviewScene done = scene_for(song, best);
    REQUIRE(done.trailing_skipped_fill_ticks.empty());
    GuideFacts finished;
    finished.last_end = measure_label(
        done.timing->timecode(*best.all_activations().back().deact_tick()));
    finished.player_total = best.totalscore();
    const GuideMessage end = guide_text(finished);
    CHECK(end.text ==
          "Your Star Power runs until m90. No fill after that comes with 2 saved phrases, so "
          "your path is finished. Your score is 228,710.");

    // A one-shot message wins over both.
    finished.message = cant_activate_message("m12", kBeforeLastActivationText);
    CHECK(guide_text(finished).text ==
          "Can't activate at m12: This is before your last activation. You build left to "
          "right; Undo goes back a step.");
    CHECK(guide_text(finished).refuse);
}

TEST_CASE("fill hover text: the two numbers and the difference agree") {
    // Allister "Overrated" with no activation: BF(0) is the best finish over
    // no windows, and a fill's own number is the best finish with it pinned.
    const Song& song = song_named("Allister - Overrated");
    const app::PreviewScene empty = scene_for(song, player_path(song, {}));
    const int64_t bf0 = best_finish(song, default_cfg(), {}).paths.at(0).totalscore();
    const HydraRecord& rec =
        corpus::analyzed(corpus::chart_named("Allister - Overrated"), default_cfg());
    const int64_t best_tick = rec.best_path().all_activations().front().timecode.ticks();
    const size_t pick = fill_index(empty, 55680);
    const TargetResult with =
        best_finish(song, default_cfg(), {PinnedWindow{55680, std::nullopt, false, std::nullopt}});
    const Path& with_path = with.paths.at(0);
    const std::vector<Activation> with_acts = with_path.all_activations();
    FillHoverNumbers n;
    n.best_finish = with_path.totalscore();
    n.best_now = bf0;
    n.cost = bf0 - n.best_finish;  // candidate_costs's subtraction, for the fixture only
    n.best_fill = fill_name(empty, empty.fills[fill_index(empty, best_tick)]);
    n.hydra_end = &with_acts.front();
    // The three figures in the text, pinned from one run: 228,710 less
    // 226,210 is the 2,500 it says.
    CHECK(bf0 == 228710);
    CHECK(n.best_finish == 226210);
    CHECK(fill_hover_text(empty, pick, n) ==
          "Fill at m30 · 0:45.789\n"
          "You have 2 phrases saved.\n"
          "If you activate here, the best you can still finish with is 226,210.\n"
          "Your best choice right now is m38, which keeps 228,710.\n"
          "So activating here costs 2,500 points.\n"
          "Hydra would end it at m34.");
    // Before the candidate's search lands, and on a fill under two bars.
    CHECK(fill_hover_text(empty, pick, std::nullopt) ==
          "Fill at m30 · 0:45.789\nYou have 2 phrases saved.\n...");
    CHECK(fill_hover_text(empty, 0, std::nullopt) ==
          "Fill at m20 · 0:30.000\nYou have 1 phrase saved here. You need 2 to activate.");
    // The best fill itself costs nothing and says so.
    FillHoverNumbers b = n;
    b.best_finish = bf0;
    b.cost = 0;
    b.hydra_end = nullptr;
    CHECK(fill_hover_text(empty, fill_index(empty, best_tick), b) ==
          "Fill at m38 · 0:58.421\n"
          "You have 3 phrases saved.\n"
          "If you activate here, the best you can still finish with is 228,710.\n"
          "That is the best choice right now.");
}
