// The Path Builder's steps panel and guide line, driven headless: "Finish the
// rest for me", the tight-window note, and a click on a fill the player
// cannot activate on. Every figure is pinned from one run under the GUI
// tests' settings (tests/scratch_settings.h).

// First: its Windows headers come before the test engine's, which undefine
// their Yield() macro (uitest_engine.h says why).
#include "pbtest_harness.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_te_context.h"

#include "../testdata_dirs.h"

#include "app/report.h"  // ChipToken
#include "pathbuilder/pb_coach.h"
#include "pathbuilder/pb_files.h"
#include "pathbuilder/pb_text.h"
#include "ui/preview_timeline.h"  // timeline_fill_id
#include "ui/theme.h"             // chip_color

namespace pbtest {

namespace {

using hydra::pathbuilder::PbState;

// Opens `chart` in a fresh builder and waits for the open to end.
bool open_chart(ImGuiTestContext* ctx, const std::string& chart) {
    Harness& h = harness(ctx);
    h.chart_arg = chart;
    reset_app(h);
    if (!wait_idle(ctx)) return false;
    ctx->Yield(2);
    return h.app->state.chart() != nullptr;
}

// Pastes Hydra's best path as the player's (Edit > Paste) and waits for its
// ladder and overlay.
bool paste_best_path(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    const hydra::pathbuilder::OpenChart* chart = h.app->state.chart();
    ImGui::SetClipboardText(
        hydra::pathbuilder::copy_text(chart->record, chart->record.best_path()).c_str());
    ctx->SetRef(h.home_ref.c_str());
    ctx->MenuClick("Edit/Paste");
    return wait_for_edit(ctx);
}

// The vertical span of the glyphs the steps panel drew last frame in
// `color`, or nothing when it drew none.
std::optional<std::pair<float, float>> glyph_span(ImU32 color) {
    const ImGuiWindow* column = uitest::window_named("##steps");
    if (!column || !column->DrawList) return std::nullopt;
    const ImVec2 white = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
    std::optional<std::pair<float, float>> span;
    for (const ImDrawVert& v : column->DrawList->VtxBuffer) {
        if (v.col != color || (v.uv.x == white.x && v.uv.y == white.y)) continue;
        if (!span) span = std::make_pair(v.pos.y, v.pos.y);
        span->first = (std::min)(span->first, v.pos.y);
        span->second = (std::max)(span->second, v.pos.y);
    }
    return span;
}

// One click on Burnout, then "Finish the rest for me": the player's path
// becomes the one behind BF(1), the best finish with that one window kept,
// and the steps panel says so.
void test_finish_for_me(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    PbState& s = h.app->state;
    const hydra::ActivationWalk best = s.chart()->record.best_path().walk_activations();
    IM_CHECK_RETV(best.size() >= 2, );
    IM_CHECK_RETV(click_fill(ctx, best[1].timecode.ticks()), );
    IM_CHECK(s.decisions().message().empty());
    {
        const hydra::pathbuilder::PlayerPath& mine = s.history().current();
        IM_CHECK_RETV(mine.windows.size() == 1, );
        IM_CHECK_RETV(mine.path.has_value(), );
        // From one run: the click's window alone, and what it costs.
        IM_CHECK(mine.path->pathstring() == "9");
        IM_CHECK_EQ(mine.path->totalscore(), int64_t{349875});
    }
    // The steps panel's one step and the line after it, from one run.
    IM_CHECK(on_screen(h, "1. Activate at "));
    IM_CHECK(on_screen(h, "This choice costs 13,640 points."));
    IM_CHECK(on_screen(h, hydra::pathbuilder::leftover_text(14800)));
    IM_CHECK(!on_screen(h, "2. Activate at "));

    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick(("**/" + std::string(hydra::pathbuilder::kFinishForMeLabel)).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    const hydra::pathbuilder::PlayerPath& done = s.history().current();
    IM_CHECK_RETV(done.path.has_value(), );
    // From one run: BF(1)'s path and total.
    IM_CHECK(done.path->pathstring() == "9 1");
    IM_CHECK_EQ(done.path->totalscore(), int64_t{364675});
    IM_CHECK_EQ(done.windows.size(), size_t{2});
    IM_CHECK(s.history().can_undo());
    IM_CHECK(on_screen(h, hydra::pathbuilder::finished_for_player_message(1).text));
    IM_CHECK(on_screen(h, "2. Activate at "));
}

// A click on a fill under two saved phrases: the guide line says why, in
// fill_reason's sentence, and the player's path stays as it was.
void test_cant_activate_reason(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    PbState& s = h.app->state;
    const uint64_t generation = s.history().generation();
    // From one run: Burnout's fill at this tick comes before two phrases are
    // saved (pbtest_window.cpp's open-starts-empty pins the same fill).
    constexpr int64_t kUnderTwoTick = 21120;
    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick(("**/" + hydra::ui::timeline_fill_id(kUnderTwoTick)).c_str());
    ctx->Yield(2);
    const std::string guide = h.app->guide().text;
    IM_CHECK(h.app->guide().refuse);
    IM_CHECK_STR_EQ(guide.c_str(), "Can't activate at m12: You have 1 phrase saved here. You need 2 to activate.");
    IM_CHECK(on_screen(h, guide));
    IM_CHECK_EQ(s.history().generation(), generation);
    IM_CHECK(!s.history().current().path.has_value());
    IM_CHECK(!s.history().can_undo());
}

// White Washed under the GUI tests' Path limit (10 ms, the app's default):
// Hydra's best path's fourth window needs more, so its step carries the
// violet note.
void test_tight_window_note(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::white_washed_notes()), );
    PbState& s = h.app->state;
    IM_CHECK(s.chart()->settings.ms_filter == std::optional<double>(10.0));
    IM_CHECK_RETV(paste_best_path(ctx), );
    IM_CHECK_RETV(s.history().current().windows.size() == 5, );
    // From one run.
    IM_CHECK(on_screen(h, "5. Activate at "));
    IM_CHECK(on_screen(h, "needs 130.4 ms, your limit is 10 ms"));

    // The note is the only text the panel draws in the violet chip's colour,
    // and it is one line.
    const ImU32 violet =
        ImGui::GetColorU32(hydra::ui::chip_color(hydra::app::report::ChipToken::t5));
    const std::optional<std::pair<float, float>> span = glyph_span(violet);
    IM_CHECK_RETV(span.has_value(), );
    IM_CHECK_LE(span->second - span->first, ImGui::GetTextLineHeightWithSpacing());
}

}  // namespace

void register_coach_tests(Harness& h) {
    uitest::register_test(h, "finish-for-me", test_finish_for_me);
    uitest::register_test(h, "cant-activate-reason", test_cant_activate_reason);
    uitest::register_test(h, "tight-window-note", test_tight_window_note);
}

}  // namespace pbtest
