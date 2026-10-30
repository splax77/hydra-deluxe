// The Path Builder's clicks on the highway and on the strip under it, driven
// headless: a click on an offered fill on the highway starts a window, a
// click on a taken fill's marker on the strip removes it, and a click while
// a left-list path is on screen acts on a copy of it.

// First: its Windows headers come before the test engine's, which undefine
// their Yield() macro (uitest_engine.h says why).
#include "pbtest_harness.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_te_context.h"

#include "../testdata_dirs.h"

#include "app/preview_view.h"
#include "pathbuilder/pb_coach.h"
#include "pathbuilder/pb_strip.h"
#include "search/pather.h"            // pinned_windows
#include "render/highway_draw.h"     // far_time, time_to_z
#include "render/overlay_layout.h"   // project_to_image
#include "render/preview_renderer.h" // kPlaybackSpeed
#include "ui/preview_controller.h"

namespace pbtest {

namespace {

using hydra::app::PreviewFill;
using hydra::app::PreviewFillState;
using hydra::app::PreviewScene;
using hydra::pathbuilder::PbState;

// Opens Burnout in a fresh builder and waits for the chart and the highway.
bool open_burnout(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = corpus::burnout_notes();
    reset_app(h);
    if (!wait_idle(ctx)) return false;
    ctx->Yield(2);
    // The highway's load starts on the frame after the chart lands.
    if (!wait_idle(ctx)) return false;
    ctx->Yield(2);
    return h.app->state.chart() != nullptr && h.app->preview() != nullptr &&
           !h.app->preview()->loading();
}

// The scene's fill on the chord at `tick`, or none.
std::optional<size_t> fill_on(const PreviewScene& scene, int64_t tick) {
    for (size_t i = 0; i < scene.fills.size(); ++i)
        if (scene.fills[i].chord_tick == tick) return i;
    return std::nullopt;
}

// A click on the highway, on the fill Hydra's best path activates second:
// the player's path, empty until now, gains that window and the decision
// list a row.
void test_highway_click_starts_window(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_burnout(ctx), );
    PbState& s = h.app->state;
    hydra::ui::PreviewController& pc = *h.app->preview();
    IM_CHECK_RETV(s.decisions().list().rows.empty(), );
    const hydra::ActivationWalk best = s.chart()->record.best_path().walk_activations();
    IM_CHECK_RETV(best.size() >= 2, );
    const int64_t tick = best[1].timecode.ticks();
    const std::optional<size_t> fill = fill_on(pc.scene(), tick);
    IM_CHECK_RETV(fill.has_value(), );
    const PreviewFill f = pc.scene().fills[*fill];
    IM_CHECK_RETV(f.state == PreviewFillState::Offered, );

    // Put the fill's middle halfway up the highway, then project it.
    const hydra::render::PreviewConfig& cfg = pc.preview_config();
    const double speed = hydra::render::kPlaybackSpeed;
    const double mid_s = (f.span.start_ms + f.span.end_ms) / 2000.0;
    const double now_s = mid_s - hydra::render::far_time(cfg, 0.0, speed) / 2.0;
    pc.pause();
    pc.seek_ms(now_s * 1000.0);
    ctx->Yield(2);
    const ImRect img = ctx->ItemInfo("**/##highway_image").RectFull;
    const int w = static_cast<int>(img.GetWidth());
    const int hgt = static_cast<int>(img.GetHeight());
    IM_CHECK_RETV(w > 0 && hgt > 0, );
    const hydra::render::ImagePoint p = hydra::render::project_to_image(
        cfg, w, hgt,
        {0.0f, cfg.track.y,
         static_cast<float>(hydra::render::time_to_z(cfg, pc.position_ms() / 1000.0, mid_s, speed))});
    IM_CHECK_RETV(pc.fill_at_image(p, w, hgt) == std::optional<size_t>(*fill), );

    ctx->MouseMoveToPos(ImVec2(img.Min.x + p.x, img.Min.y + p.y));
    ctx->MouseClick(0);
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK(s.decisions().message().empty());
    const hydra::pathbuilder::PlayerPath& mine = s.history().current();
    IM_CHECK_RETV(mine.windows.size() == 1, );
    IM_CHECK(mine.windows.front().act_tick == tick);
    IM_CHECK(s.decisions().list().rows.size() == 1);
    IM_CHECK(!on_screen(h, "No windows yet."));
}

// A click on the strip, on the marker of the player's one window: the window
// goes, and the path is empty again.
void test_strip_click_removes_window(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_burnout(ctx), );
    PbState& s = h.app->state;
    hydra::ui::PreviewController& pc = *h.app->preview();
    const hydra::ActivationWalk best = s.chart()->record.best_path().walk_activations();
    IM_CHECK_RETV(best.size() >= 2, );
    const int64_t tick = best[1].timecode.ticks();
    IM_CHECK_RETV(click_fill(ctx, tick), );
    IM_CHECK_RETV(s.history().current().windows.size() == 1, );
    IM_CHECK_RETV(s.decisions().list().rows.size() == 1, );
    const std::optional<size_t> fill = fill_on(pc.scene(), tick);
    IM_CHECK_RETV(fill.has_value(), );
    IM_CHECK_RETV(pc.scene().fills[*fill].state == PreviewFillState::Taken, );

    // The middle of the taken fill's marker, as the strip lays it out.
    const ImRect strip = ctx->ItemInfo(("**/" + std::string(hydra::pathbuilder::kStripId)).c_str())
                             .RectFull;
    const std::vector<hydra::pathbuilder::StripMarker> markers = hydra::pathbuilder::strip_markers(
        pc.scene(), pc.playback_end_ms(), strip.GetWidth(), 0.0f);
    std::optional<float> x;
    for (const hydra::pathbuilder::StripMarker& m : markers)
        if (m.fill == *fill) x = (m.x0 + m.x1) * 0.5f;
    IM_CHECK_RETV(x.has_value(), );
    IM_CHECK_RETV(hydra::pathbuilder::marker_at(markers, *x) == std::optional<size_t>(*fill), );

    ctx->MouseMoveToPos(ImVec2(strip.Min.x + *x, strip.GetCenter().y));
    ctx->MouseClick(0);
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK(s.decisions().message().empty());
    IM_CHECK(s.history().current().windows.empty());
    IM_CHECK(!s.history().current().path.has_value());
    IM_CHECK(s.history().can_undo());
    IM_CHECK(s.decisions().list().rows.empty());
    IM_CHECK(on_screen(h, "No windows yet."));
}

// The user's ruling, "Copy it, then apply": with Best path picked in the left
// list, a click on the strip marker of its last window makes the player's
// path Best path without that window, and one Undo gives back the empty path.
void test_click_on_shown_path_copies(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_burnout(ctx), );
    PbState& s = h.app->state;
    hydra::ui::PreviewController& pc = *h.app->preview();
    IM_CHECK_RETV(s.history().current().windows.empty(), );
    const std::vector<hydra::PinnedWindow> best =
        hydra::pinned_windows(s.chart()->record.best_path());
    IM_CHECK_RETV(best.size() == 3, );

    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick("**/Best path");
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK_RETV(h.app->list_ui().picked.has_value(), );
    const int64_t tick = best[2].act_tick;
    const std::optional<size_t> fill = fill_on(pc.scene(), tick);
    IM_CHECK_RETV(fill.has_value(), );
    IM_CHECK_RETV(pc.scene().fills[*fill].state == PreviewFillState::Taken, );

    const ImRect strip = ctx->ItemInfo(("**/" + std::string(hydra::pathbuilder::kStripId)).c_str())
                             .RectFull;
    const std::vector<hydra::pathbuilder::StripMarker> markers = hydra::pathbuilder::strip_markers(
        pc.scene(), pc.playback_end_ms(), strip.GetWidth(), 0.0f);
    std::optional<float> x;
    for (const hydra::pathbuilder::StripMarker& m : markers)
        if (m.fill == *fill) x = (m.x0 + m.x1) * 0.5f;
    IM_CHECK_RETV(x.has_value(), );
    IM_CHECK_RETV(hydra::pathbuilder::marker_at(markers, *x) == std::optional<size_t>(*fill), );

    const uint64_t before = s.history().generation();
    ctx->MouseMoveToPos(ImVec2(strip.Min.x + *x, strip.GetCenter().y));
    ctx->MouseClick(0);
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK(s.decisions().message().empty());
    // One edit: the copy and the removal together.
    IM_CHECK(s.history().generation() == before + 1);
    const hydra::pathbuilder::PlayerPath& mine = s.history().current();
    IM_CHECK_RETV(mine.windows.size() == 2, );
    for (size_t i = 0; i < 2; ++i) {
        IM_CHECK(mine.windows[i].act_tick == best[i].act_tick);
        IM_CHECK(mine.windows[i].deact_tick == best[i].deact_tick);
    }
    IM_CHECK_RETV(mine.path.has_value(), );
    // From one run, as test_pb_coach.cpp pins it.
    IM_CHECK_STR_EQ(mine.path->pathstring().c_str(), "3- 1");
    // The player's path is on screen again.
    IM_CHECK(!h.app->list_ui().picked.has_value());
    IM_CHECK(s.decisions().list().rows.size() == 2);

    ctx->MenuClick("Edit/Undo");
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    IM_CHECK(s.history().current().windows.empty());
    IM_CHECK(!s.history().current().path.has_value());
    IM_CHECK(!s.history().can_undo());
    IM_CHECK(on_screen(h, "No windows yet."));
}

}  // namespace

void register_strip_tests(Harness& h) {
    uitest::register_test(h, "highway-click-starts-window", test_highway_click_starts_window);
    uitest::register_test(h, "strip-click-removes-window", test_strip_click_removes_window);
    uitest::register_test(h, "click-on-shown-path-copies", test_click_on_shown_path_copies);
}

}  // namespace pbtest
