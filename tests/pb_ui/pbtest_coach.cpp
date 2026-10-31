// The Path Builder's steps panel and guide line, driven headless: a step's
// ending lines, "Finish the rest for me", the tight-window note, and a click
// on a fill the player cannot activate on. Every figure is pinned from one run under the GUI
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
    // The steps panel's one step as its compact row. From one run.
    IM_CHECK(on_screen(h, "m58 → m68 4 phrases −13,640"));

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
    // The second step is on screen as its compact row. From one run.
    IM_CHECK(on_screen(h, "m88 → m92 2 phrases best"));
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
    // As the locked-fill test: the pointer must not stay on the fill.
    move_off_timeline(ctx);
}

// One activation on Burnout, then a click on a fill before it: the guide
// line says the builder goes left to right, in locked_message's words, and
// the path stays as it was.
void test_locked_fill_says_left_to_right(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    PbState& s = h.app->state;
    const hydra::ActivationWalk best = s.chart()->record.best_path().walk_activations();
    IM_CHECK_RETV(best.size() >= 2, );
    IM_CHECK_RETV(click_fill(ctx, best[1].timecode.ticks()), );
    // From one run: Burnout's fill at m12, before that activation, and the
    // one step's compact row.
    constexpr int64_t kEarlierTick = 21120;
    const char* const step_row = "m58 → m68 4 phrases −13,640";
    IM_CHECK_RETV(on_screen(h, step_row), );
    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick(("**/" + hydra::ui::timeline_fill_id(kEarlierTick)).c_str());
    ctx->Yield(2);
    IM_CHECK(h.app->guide().refuse);
    IM_CHECK(on_screen(h, hydra::pathbuilder::locked_message("m12").text));
    // The path is as it was: the one step, and none at m12.
    IM_CHECK(on_screen(h, step_row));
    IM_CHECK(!on_screen(h, "m12 →"));
    // The pointer would stay on that fill into the next test, whose fresh
    // builder would read it as a hover.
    move_off_timeline(ctx);
}

// White Washed under the GUI tests' Path limit (10 ms, the app's default):
// Hydra's best path's fourth window needs more, so its step carries the
// violet note, under its ending lines.
void test_tight_window_note(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::white_washed_notes()), );
    PbState& s = h.app->state;
    IM_CHECK(s.chart()->settings.ms_filter == std::optional<double>(10.0));
    IM_CHECK_RETV(paste_best_path(ctx), );
    IM_CHECK_RETV(s.history().current().windows.size() == 5, );
    // The fourth step opens to its ending lines and the note when its row is
    // clicked. From one run.
    IM_CHECK_RETV(click_step(ctx, 3), );
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    const std::string note = "needs 130.4 ms, your limit is 10 ms";
    const std::string kept_line = "(x) ends m136 · squeeze out 130.4 ms";
    const std::string text = visible_text(h);
    IM_CHECK(text.find(kept_line) != std::string::npos);
    IM_CHECK(text.find(note) != std::string::npos);
    IM_CHECK(text.find(kept_line) < text.find(note));
    IM_CHECK(!on_screen(h, "Activate at"));

    // The note is the only text the panel draws in the violet chip's colour,
    // and it is one line.
    const ImU32 violet =
        ImGui::GetColorU32(hydra::ui::chip_color(hydra::app::report::ChipToken::t5));
    const std::optional<std::pair<float, float>> span = glyph_span(violet);
    IM_CHECK_RETV(span.has_value(), );
    IM_CHECK_LE(span->second - span->first, ImGui::GetTextLineHeightWithSpacing());
}

// How many glyph vertices the steps panel drew last frame inside `rect`:
// in `color` only, or in any colour when `color` is 0.
size_t glyphs_in(const ImRect& rect, ImU32 color) {
    const ImGuiWindow* column = uitest::window_named("##steps");
    if (!column || !column->DrawList) return 0;
    const ImVec2 white = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
    size_t n = 0;
    for (const ImDrawVert& v : column->DrawList->VtxBuffer) {
        if (v.uv.x == white.x && v.uv.y == white.y) continue;
        if (color != 0 && v.col != color) continue;
        if (rect.Contains(v.pos)) ++n;
    }
    return n;
}

// Smash Mouth's best path is "1 0+ 0 0": its second activation can end on a
// squeeze-in at m78 or a squeeze-out at m76, both at 0.0 ms. The opened
// step shows both ending lines in the Paths tab's badge words, the kept one
// marked; a click on the other moves the mark and the score. Once a third
// activation is made, step 2 opened again shows the same lines, not
// clickable, the kept one still marked and the other greyed.
void test_choose_an_ending(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::believer_notes()), );
    PbState& s = h.app->state;
    const hydra::ActivationWalk best = s.chart()->record.best_path().walk_activations();
    IM_CHECK_RETV(best.size() >= 3, );
    // From one run: the best path's first three activations, and the two
    // ends of the second by their deact node's tick.
    IM_CHECK_EQ(best[0].timecode.ticks(), int64_t{26880});
    IM_CHECK_EQ(best[1].timecode.ticks(), int64_t{52992});
    IM_CHECK_EQ(best[2].timecode.ticks(), int64_t{74496});
    const std::string sq_in_end = any_ref("##end_59136");
    const std::string sq_out_end = any_ref("##end_57600");

    IM_CHECK_RETV(click_fill(ctx, best[0].timecode.ticks()), );
    IM_CHECK_RETV(click_fill(ctx, best[1].timecode.ticks()), );
    IM_CHECK_RETV(click_step(ctx, 1), );
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    // From one run: the row, the two lines and the score.
    IM_CHECK(on_screen(h, "m70 → m78 3 phrases · squeeze in 0.0 ms best"));
    IM_CHECK(on_screen(h, hydra::pathbuilder::kEndItAtLabel));
    IM_CHECK(on_screen(h, "(x) ends m78 · squeeze in 0.0 ms best"));
    IM_CHECK(on_screen(h, "( ) ends m76 · squeeze out 0.0 ms −2,680"));
    IM_CHECK(on_screen(h, "336,575"));
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK((ctx->ItemInfo(sq_out_end.c_str()).ItemFlags & ImGuiItemFlags_Disabled) == 0);

    // The other ending: the mark moves, the row and the score follow.
    ctx->ItemClick(sq_out_end.c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK_EQ(s.history().current().windows.size(), size_t{2});
    IM_CHECK(on_screen(h, "m70 → m76 3 phrases · squeeze out 0.0 ms −2,680"));
    IM_CHECK(on_screen(h, "( ) ends m78 · squeeze in 0.0 ms best"));
    IM_CHECK(on_screen(h, "(x) ends m76 · squeeze out 0.0 ms −2,680"));
    IM_CHECK(on_screen(h, "330,075"));
    IM_CHECK(!on_screen(h, "336,575"));

    // And back.
    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick(sq_in_end.c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK(on_screen(h, "(x) ends m78 · squeeze in 0.0 ms best"));
    IM_CHECK(on_screen(h, "336,575"));

    // A third activation, then step 2 again: its lines are fixed now.
    IM_CHECK_RETV(click_fill(ctx, best[2].timecode.ticks()), );
    IM_CHECK_RETV(s.history().current().windows.size() == 3, );
    IM_CHECK_RETV(click_step(ctx, 1), );
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "(x) ends m78 · squeeze in 0.0 ms best"));
    IM_CHECK(on_screen(h, "( ) ends m76 · squeeze out 0.0 ms −2,680"));
    ctx->SetRef(h.home_ref.c_str());
    const ImGuiTestItemInfo kept = ctx->ItemInfo(sq_in_end.c_str());
    const ImGuiTestItemInfo other = ctx->ItemInfo(sq_out_end.c_str());
    IM_CHECK_RETV(kept.ID != 0 && other.ID != 0, );
    IM_CHECK((kept.ItemFlags & ImGuiItemFlags_Disabled) != 0);
    IM_CHECK((other.ItemFlags & ImGuiItemFlags_Disabled) != 0);
    // The kept line's words are in the text colour; the other line's are
    // drawn, but never in it.
    const ImU32 text = ImGui::GetColorU32(ImGuiCol_Text);
    IM_CHECK(glyphs_in(kept.RectFull, text) > 0);
    IM_CHECK(glyphs_in(other.RectFull, 0) > 0);
    IM_CHECK_EQ(glyphs_in(other.RectFull, text), size_t{0});
    // A click on the greyed line changes nothing.
    const uint64_t generation = s.history().generation();
    ctx->ItemClick(sq_out_end.c_str());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK_EQ(s.history().generation(), generation);
    IM_CHECK_EQ(s.history().current().windows.size(), size_t{3});
    IM_CHECK(on_screen(h, "(x) ends m78 · squeeze in 0.0 ms best"));

    // The opened step's old paragraphs and the panel's old lines are gone.
    for (const char* gone : {"Activate at", "This choice costs", "You still have Star Power",
                             "no timing needed", "Cost: how many points", "Before it, the best",
                             "Nothing left on the table"})
        IM_CHECK(!on_screen(h, gone));
}

// The Preview clock's tail on Burnout, the song's length (from one run).
constexpr const char* kBurnoutClockTail = " / 2:10.088";
// The first fill's dot as the clock reads it, where a hover or a click on it
// parks the Preview: the moment its card heading names, "m20 · 0:25.123"
// (from one run).
constexpr const char* kFirstFillDot = "0:25.123";
// How long a check waits for a card whose search is still running.
constexpr double kCardWaitSeconds = 60.0;

// The Preview's clock as the screen drew it last frame, "0:25.123 / 2:10.088",
// or empty when it is not drawn.
std::string burnout_clock(Harness& h) {
    const std::string text = visible_text(h);
    const size_t tail = text.find(kBurnoutClockTail);
    if (tail == std::string::npos) return std::string();
    size_t from = tail;
    while (from > 0 && text[from - 1] != ' ' && text[from - 1] != '\n' && text[from - 1] != '\r') --from;
    return text.substr(from, tail + std::string(kBurnoutClockTail).size() - from);
}

// Burnout playing, then a hover on its first fill: the hover pauses and shows
// the fill, and moving off puts the clock back and leaves it paused (the third
// mockup's answers.md, answer 1).
void test_hover_shows_never_plays(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    ctx->SetRef(h.home_ref.c_str());
    // The panel's button reads "Pause" while the Preview plays
    // (preview_panel.cpp's transport row).
    ctx->ItemClick(any_ref("Play").c_str());
    ctx->Yield(2);
    IM_CHECK_RETV(on_screen(h, "Pause"), );

    ctx->MouseMove(fill_ref(kFirstFillTick).c_str());
    ctx->Yield(2);
    IM_CHECK(burnout_clock(h) == std::string(kFirstFillDot) + kBurnoutClockTail);
    IM_CHECK(on_screen(h, "Play"));
    IM_CHECK(!on_screen(h, "Pause"));

    move_off_timeline(ctx);
    IM_CHECK(on_screen(h, "Play"));
    IM_CHECK(!on_screen(h, "Pause"));
    // Nothing resumed: the clock reads the same a quarter second later, and
    // it is back where it was, not on the fill.
    const std::string first = burnout_clock(h);
    ctx->SleepNoSkip(0.25f, 1.0f / 60.0f);
    const std::string second = burnout_clock(h);
    IM_CHECK(!first.empty());
    IM_CHECK_STR_EQ(first.c_str(), second.c_str());
    IM_CHECK(first.find(kFirstFillDot) == std::string::npos);
    IM_CHECK(on_screen(h, "Play"));
    IM_CHECK(!on_screen(h, "Pause"));
}

// The card on Burnout's first fill once its search has landed: the heading,
// the three rows and the two grey lines (the figures pbtest_build.cpp's
// build-a-path-by-clicking pins, from one run).
void test_fill_card(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    ctx->SetRef(h.home_ref.c_str());
    ctx->MouseMove(fill_ref(kFirstFillTick).c_str());
    const std::string best_line = "Best choice now is m32 (378,315).";
    IM_CHECK_RETV(uitest::wait_until(ctx, [&] { return on_screen(h, best_line); }, kCardWaitSeconds), );
    IM_CHECK(on_screen(h, "m20 · 0:25.123"));
    IM_CHECK(on_screen(h, "Phrases saved"));
    IM_CHECK(on_screen(h, "Best finish here"));
    IM_CHECK(on_screen(h, "378,175"));
    IM_CHECK(on_screen(h, "Cost"));
    IM_CHECK(on_screen(h, "−140"));
    IM_CHECK(on_screen(h, "Hydra would end it at m24."));
    // Each figure sits on its label's row: ImGui's text log writes a table
    // row's cells on one line, each after a "|".
    IM_CHECK(on_screen(h, "Phrases saved | 2"));
    IM_CHECK(on_screen(h, "Best finish here | 378,175"));
    IM_CHECK(on_screen(h, "Cost | −140"));
    // The old six sentences are gone.
    IM_CHECK(!on_screen(h, "If you activate here"));
    move_off_timeline(ctx);
    IM_CHECK(!on_screen(h, best_line));
}

// One activation on Burnout, then a click on the second fill's list row: once
// the pointer leaves the list, the clock stays on that fill's dot (bar
// question 7: a row click moves the Preview's clock, like a fill click).
void test_row_click_moves_clock(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    IM_CHECK_RETV(click_fill(ctx, kFirstFillTick), );
    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick(choice_ref(kSecondFillTick).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    // From one run: the second fill's dot, the moment its row names
    // ("m40 0:51.421").
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), "0:51.421 / 2:10.088");
}

}  // namespace

void register_coach_tests(Harness& h) {
    uitest::register_test(h, "choose-an-ending", test_choose_an_ending);
    uitest::register_test(h, "finish-for-me", test_finish_for_me);
    uitest::register_test(h, "cant-activate-reason", test_cant_activate_reason);
    uitest::register_test(h, "tight-window-note", test_tight_window_note);
    uitest::register_test(h, "locked-fill-says-left-to-right", test_locked_fill_says_left_to_right);
    uitest::register_test(h, "hover-shows-never-plays", test_hover_shows_never_plays);
    uitest::register_test(h, "fill-card", test_fill_card);
    uitest::register_test(h, "row-click-moves-clock", test_row_click_moves_clock);
}

}  // namespace pbtest
