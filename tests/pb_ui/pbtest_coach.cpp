// The Path Builder's steps panel, driven headless: a step's ending lines, "Finish the rest for me", the tight-window note, and a click
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
#include "ui/preview_controller.h"
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
    // The second step is on screen as its compact row. From one run.
    IM_CHECK(on_screen(h, "m88 → m92 2 phrases best"));
}

// True once the hover card on the fill at `tick` shows `sentence`. A click on
// a refused fill says nothing, so this card is where the player reads why.
bool card_says(ImGuiTestContext* ctx, int64_t tick, const std::string& sentence) {
    Harness& h = harness(ctx);
    ctx->MouseMove(fill_ref(tick).c_str());
    return uitest::wait_until(ctx, [&] { return on_screen(h, sentence); }, 5.0);
}

// A click on a fill under two saved phrases changes nothing, and the fill's
// card gives fill_reason's sentence.
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
    IM_CHECK_EQ(s.history().generation(), generation);
    IM_CHECK(!s.history().current().path.has_value());
    IM_CHECK(!s.history().can_undo());
    IM_CHECK(card_says(ctx, kUnderTwoTick, "You have 1 phrase saved here. You need 2 to activate."));
    IM_CHECK(!on_screen(h, "Can't activate at"));
    // As the locked-fill test: the pointer must not stay on the fill.
    move_off_timeline(ctx);
}

// One activation on Burnout, then a click on a fill before it: the path stays
// as it was, and the fill's card says the builder goes left to right.
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
    // The path is as it was: the one step, and none at m12.
    IM_CHECK(on_screen(h, step_row));
    IM_CHECK(!on_screen(h, "m12 →"));
    IM_CHECK(card_says(ctx, kEarlierTick, hydra::pathbuilder::kBeforeLastActivationText));
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
// The first fill's dot as the clock reads it, where a click on it parks the
// Preview: the moment its card heading names, "m20 · 0:25.123" (from one run).
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

// The clock these tests park the paused Preview on, and how it reads.
constexpr double kParkedMs = 10000.0;
constexpr const char* kParkedClock = "0:10.000 / 2:10.088";

// Opens Burnout with the paused clock on kParkedClock and the test's ref on
// the builder's window. False when it did not settle.
bool open_burnout_parked(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_chart(ctx, corpus::burnout_notes())) return false;
    ctx->SetRef(h.home_ref.c_str());
    // The pointer is wherever the last test left it, maybe on a fill.
    move_off_timeline(ctx);
    h.app->preview()->seek_ms(kParkedMs);
    ctx->Yield(2);
    return burnout_clock(h) == kParkedClock;
}

// The panel's button reads "Pause" while the Preview plays, and "Play" while
// it does not (preview_panel.cpp's transport row).
bool reads_paused(Harness& h) { return on_screen(h, "Play") && !on_screen(h, "Pause"); }
bool reads_playing(Harness& h) { return on_screen(h, "Pause"); }

// True when the clock reads differently a quarter second after the first
// reading, which only a running Preview does.
bool clock_runs(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    const std::string first = burnout_clock(h);
    ctx->SleepNoSkip(0.25f, 1.0f / 60.0f);
    return first != burnout_clock(h);
}

// A point on the Star Power saved row, `share` of the way across the track.
ImVec2 saved_row_point(ImGuiTestContext* ctx, float share) {
    const ImRect rows = ctx->ItemInfo(any_ref("##timeline").c_str()).RectFull;
    return ImVec2(rows.Min.x + rows.GetWidth() * share, rows.Min.y + rows.GetHeight() * 0.4f);
}

// A hover moves nothing. Paused, a fill's card shows and a list row lights its
// fill while the clock and the Play button stay as they were, and they stay so
// when the pointer leaves. Playing, a hover does not pause.
void test_hover_moves_nothing(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_burnout_parked(ctx), );

    ctx->MouseMove(fill_ref(kFirstFillTick).c_str());
    IM_CHECK(uitest::wait_until(ctx, [&] { return on_screen(h, "m20 · 0:25.123"); }, 5.0));
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), kParkedClock);
    IM_CHECK(reads_paused(h));
    move_off_timeline(ctx);
    IM_CHECK(!on_screen(h, "m20 · 0:25.123"));
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), kParkedClock);
    IM_CHECK(reads_paused(h));

    IM_CHECK_RETV(scroll_side_to(ctx, choice_ref(kFirstFillTick)), );
    ctx->MouseMove(choice_ref(kFirstFillTick).c_str());
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), kParkedClock);
    IM_CHECK(reads_paused(h));
    move_off_timeline(ctx);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), kParkedClock);
    IM_CHECK(reads_paused(h));

    ctx->ItemClick(any_ref("Play").c_str());
    ctx->Yield(2);
    IM_CHECK_RETV(reads_playing(h), );
    ctx->MouseMove(fill_ref(kFirstFillTick).c_str());
    ctx->Yield(2);
    IM_CHECK(reads_playing(h));
    IM_CHECK(clock_runs(ctx));
    IM_CHECK_RETV(scroll_side_to(ctx, choice_ref(kFirstFillTick)), );
    ctx->MouseMove(choice_ref(kFirstFillTick).c_str());
    ctx->Yield(2);
    IM_CHECK(reads_playing(h));
    IM_CHECK(clock_runs(ctx));
    move_off_timeline(ctx);
    IM_CHECK(reads_playing(h));
    ctx->ItemClick(any_ref("Pause").c_str());
    ctx->Yield(2);
}

// A press and drag on the Star Power saved row: the clock follows the pointer
// while held, onto the Fills row too, where no card shows and no fill takes
// the release, and the release leaves the clock where it is. Paused stays
// paused. A press while playing holds the song still, and the release plays
// on. Each clock is from one run.
void test_drag_moves_the_clock(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_burnout_parked(ctx), );
    hydra::pathbuilder::PbState& s = h.app->state;
    const uint64_t generation = s.history().generation();

    ctx->MouseMoveToPos(saved_row_point(ctx, 0.25f));
    ctx->MouseDown(ImGuiMouseButton_Left);
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), "0:32.428 / 2:10.088");
    ctx->MouseMoveToPos(saved_row_point(ctx, 0.5f));
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), "1:04.981 / 2:10.088");
    IM_CHECK(reads_paused(h));

    // Down onto the Fills row, on a fill's hit box.
    const ImRect fill = ctx->ItemInfo(fill_ref(kSecondFillTick).c_str()).RectFull;
    ctx->MouseMoveToPos(fill.GetCenter());
    ctx->Yield(2);
    const std::string on_fills_row = burnout_clock(h);
    IM_CHECK_STR_EQ(on_fills_row.c_str(), "0:51.459 / 2:10.088");
    IM_CHECK(!on_screen(h, "Phrases saved"));
    ctx->MouseUp(ImGuiMouseButton_Left);
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), on_fills_row.c_str());
    IM_CHECK(reads_paused(h));
    IM_CHECK_EQ(s.history().generation(), generation);

    // A release back on the row leaves the clock under the pointer too.
    ctx->MouseMoveToPos(saved_row_point(ctx, 0.25f));
    ctx->MouseDown(ImGuiMouseButton_Left);
    ctx->MouseMoveToPos(saved_row_point(ctx, 0.5f));
    ctx->MouseUp(ImGuiMouseButton_Left);
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), "1:04.981 / 2:10.088");
    IM_CHECK(reads_paused(h));

    // Playing: held, the song stands still, and the panel's own scrubber call
    // each frame does not start it again.
    ctx->ItemClick(any_ref("Play").c_str());
    ctx->Yield(2);
    IM_CHECK_RETV(reads_playing(h), );
    ctx->MouseMoveToPos(saved_row_point(ctx, 0.25f));
    ctx->MouseDown(ImGuiMouseButton_Left);
    ctx->Yield(2);
    IM_CHECK(reads_paused(h));
    IM_CHECK(!clock_runs(ctx));
    ctx->MouseUp(ImGuiMouseButton_Left);
    ctx->Yield(2);
    IM_CHECK(reads_playing(h));
    IM_CHECK(clock_runs(ctx));
    ctx->ItemClick(any_ref("Pause").c_str());
    ctx->Yield(2);
}

// A press on the "Your activations" row starts no drag: a press on a fill,
// moved away along the Star Power saved row and let go there, leaves the clock
// where it was, and clicks nothing.
void test_activations_row_never_drags(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_burnout_parked(ctx), );
    const uint64_t generation = h.app->state.history().generation();
    ctx->MouseMove(fill_ref(kFirstFillTick).c_str());
    ctx->MouseDown(ImGuiMouseButton_Left);
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), kParkedClock);
    ctx->MouseMoveToPos(saved_row_point(ctx, 0.75f));
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), kParkedClock);
    ctx->MouseUp(ImGuiMouseButton_Left);
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), kParkedClock);
    IM_CHECK_EQ(h.app->state.history().generation(), generation);
    move_off_timeline(ctx);
}

// A click on a fill leaves the clock on that fill's dot with the pointer still
// on it, and it stays when the pointer leaves: on an offered fill, on the
// taken fill it made, and on a fill clicked while the Preview plays, which
// stops there.
void test_fill_click_moves_clock(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_burnout_parked(ctx), );
    const std::string first_dot = std::string(kFirstFillDot) + kBurnoutClockTail;
    ctx->ItemClick(fill_ref(kFirstFillTick).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), first_dot.c_str());
    move_off_timeline(ctx);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), first_dot.c_str());

    h.app->preview()->seek_ms(kParkedMs);
    ctx->Yield(2);
    ctx->ItemClick(fill_ref(kFirstFillTick).c_str());
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), first_dot.c_str());
    move_off_timeline(ctx);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), first_dot.c_str());

    ctx->ItemClick(any_ref("Play").c_str());
    ctx->Yield(2);
    IM_CHECK_RETV(reads_playing(h), );
    ctx->ItemClick(fill_ref(kSecondFillTick).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    // From one run: the second fill's dot, as row-click-moves-clock pins it.
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), "0:51.421 / 2:10.088");
    IM_CHECK(reads_paused(h));
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
    uitest::register_test(h, "hover-moves-nothing", test_hover_moves_nothing);
    uitest::register_test(h, "drag-moves-the-clock", test_drag_moves_the_clock);
    uitest::register_test(h, "activations-row-never-drags", test_activations_row_never_drags);
    uitest::register_test(h, "fill-click-moves-clock", test_fill_click_moves_clock);
    uitest::register_test(h, "fill-card", test_fill_card);
    uitest::register_test(h, "row-click-moves-clock", test_row_click_moves_clock);
}

}  // namespace pbtest
