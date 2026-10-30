// The Path Builder's decision list, driven headless: the rows a ladder fills,
// "Finish for me", a click that cannot happen, and the tight-window note.
// Every figure is pinned from one run under the GUI tests' settings
// (tests/scratch_settings.h).

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

#include "app/path_costs.h"  // CostTier
#include "app/report.h"      // ChipToken
#include "pathbuilder/pb_coach.h"
#include "ui/theme.h"  // chip_color

namespace pbtest {

namespace {

using hydra::app::CostTier;
using hydra::pathbuilder::CoachRow;
using hydra::pathbuilder::DecisionList;
using hydra::pathbuilder::PbState;

std::string white_washed_notes() {
    return corpus::root() + "/common/IB24/T5/August Burns Red - White Washed/notes.mid";
}

// Opens `chart` in a fresh builder and waits for the open to end.
bool open_chart(ImGuiTestContext* ctx, const std::string& chart) {
    Harness& h = harness(ctx);
    h.chart_arg = chart;
    reset_app(h);
    if (!wait_idle(ctx)) return false;
    ctx->Yield(2);
    return h.app->state.chart() != nullptr;
}

// Picks Hydra's best path in the left list and waits for its ladder.
bool show_best_path(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick("**/Best path");
    if (!wait_idle(ctx)) return false;
    ctx->Yield(2);
    return true;
}

// Starts a window at `act_tick` on the player's path, as a click on a fill
// does, and waits for it and the new path's ladder to land.
bool click_fill(ImGuiTestContext* ctx, int64_t act_tick) {
    Harness& h = harness(ctx);
    PbState& s = h.app->state;
    s.decisions().click_fill(s.search_graph(), s.chart()->settings,
                             s.history().current().windows, act_tick);
    if (!wait_idle(ctx)) return false;
    ctx->Yield(2);
    // The edit's frame asked for the new path's ladder.
    if (!wait_idle(ctx)) return false;
    ctx->Yield(2);
    return true;
}

// The vertical span of the glyphs the decision list's column drew last frame
// in `color`, or nothing when it drew none.
std::optional<std::pair<float, float>> glyph_span(ImU32 color) {
    const ImGuiWindow* column = uitest::window_named("##decisions");
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

// Picking Hydra's best path fills the decision list once its ladder is in:
// one row per window, with its token, cost and timing on screen.
void test_decision_list_fills(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    IM_CHECK(on_screen(h, "No windows yet."));
    IM_CHECK_RETV(show_best_path(ctx), );
    const DecisionList& d = h.app->state.decisions();
    const std::vector<CoachRow>& rows = d.list().rows;
    // From one run: Burnout's best path is "3- 1 2", and none of its windows
    // costs anything.
    IM_CHECK_RETV(rows.size() == 3, );
    const char* tokens[] = {"3-", "1", "2"};
    for (size_t i = 0; i < rows.size(); ++i) {
        IM_CHECK(rows[i].token == tokens[i]);
        IM_CHECK(rows[i].cost == std::optional<int64_t>(0));
        IM_CHECK(rows[i].tier == std::optional<CostTier>(CostTier::Green));
        IM_CHECK(on_screen(h, rows[i].token));
    }
    IM_CHECK(rows[0].timing == "163.0 ms");
    IM_CHECK(rows[1].timing.empty());
    IM_CHECK(rows[2].timing.empty());
    IM_CHECK(d.list().after_last == std::optional<int64_t>(0));
    IM_CHECK(d.list().failure.empty());
    // The first row's cost and timing sit on one line.
    IM_CHECK(on_screen(h, "0 163.0 ms"));
    IM_CHECK(on_screen(h, "After your last window 0"));
    IM_CHECK(!on_screen(h, "No windows yet."));
}

// One click on Burnout, then "Finish for me": the player's path becomes the
// one behind BF(1), the best finish with that one window kept.
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
    const std::vector<CoachRow>& rows = s.decisions().list().rows;
    IM_CHECK_RETV(rows.size() == 1, );
    IM_CHECK(rows[0].token == "9");
    IM_CHECK(rows[0].cost == std::optional<int64_t>(13640));
    IM_CHECK(rows[0].tier == std::optional<CostTier>(CostTier::Red));
    IM_CHECK(s.decisions().list().after_last == std::optional<int64_t>(14800));
    IM_CHECK(on_screen(h, "-13,640"));
    IM_CHECK(on_screen(h, "After your last window -14,800"));

    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick(("**/" + std::string(hydra::pathbuilder::kFinishForMeLabel)).c_str());
    ctx->Yield(2);
    const hydra::pathbuilder::PlayerPath& done = s.history().current();
    IM_CHECK_RETV(done.path.has_value(), );
    // From one run: BF(1)'s path and total.
    IM_CHECK(done.path->pathstring() == "9 1");
    IM_CHECK_EQ(done.path->totalscore(), int64_t{364675});
    IM_CHECK_EQ(done.windows.size(), size_t{2});
    IM_CHECK(s.history().can_undo());
}

// A click on a tick that holds no fill: the column says why, and the
// player's path stays as it was.
void test_impossible_click_message(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    PbState& s = h.app->state;
    const uint64_t generation = s.history().generation();
    IM_CHECK_RETV(click_fill(ctx, 0), );
    const std::string reason = hydra::pathbuilder::impossible_window_text("activation", true);
    IM_CHECK(s.decisions().message() == reason);
    IM_CHECK(s.decisions().detail().empty());
    IM_CHECK(on_screen(h, reason));
    IM_CHECK_EQ(s.history().generation(), generation);
    IM_CHECK(!s.history().current().path.has_value());
    IM_CHECK(s.history().current().windows.empty());
    IM_CHECK(!s.history().can_undo());
}

// White Washed under the GUI tests' Path limit (10 ms, the app's default):
// Hydra's best path's fourth window needs more, so its row carries the
// violet note, and its cost stays green.
void test_tight_window_note(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, white_washed_notes()), );
    PbState& s = h.app->state;
    IM_CHECK(s.chart()->settings.ms_filter == std::optional<double>(10.0));
    IM_CHECK_RETV(show_best_path(ctx), );
    const std::vector<CoachRow>& rows = s.decisions().list().rows;
    // From one run.
    IM_CHECK_RETV(rows.size() == 5, );
    IM_CHECK(rows[3].token == "0-");
    IM_CHECK(rows[3].timing == "130.4 ms");
    IM_CHECK(rows[3].note == "needs 130.4 ms, your limit is 10 ms");
    IM_CHECK(rows[3].tier == std::optional<CostTier>(CostTier::Green));
    for (size_t i : {size_t{0}, size_t{1}, size_t{2}, size_t{4}}) IM_CHECK(rows[i].note.empty());
    IM_CHECK(on_screen(h, "needs 130.4 ms, your limit is 10 ms"));

    // The note is the only text the column draws in the violet chip's colour,
    // and it is one line.
    const ImU32 violet =
        ImGui::GetColorU32(hydra::ui::chip_color(hydra::app::report::ChipToken::t5));
    const std::optional<std::pair<float, float>> span = glyph_span(violet);
    IM_CHECK_RETV(span.has_value(), );
    IM_CHECK_LE(span->second - span->first, ImGui::GetTextLineHeightWithSpacing());
}

}  // namespace

void register_coach_tests(Harness& h) {
    uitest::register_test(h, "decision-list-fills", test_decision_list_fills);
    uitest::register_test(h, "finish-for-me", test_finish_for_me);
    uitest::register_test(h, "impossible-click-message", test_impossible_click_message);
    uitest::register_test(h, "tight-window-note", test_tight_window_note);
}

}  // namespace pbtest
