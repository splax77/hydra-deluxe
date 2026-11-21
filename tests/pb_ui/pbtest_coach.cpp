// The Path Builder's steps panel, driven headless: a step's ending lines, "Finish the rest for me", the tight-window note, and a click
// on a fill the player cannot activate on; then the Preview's clock under a hover, a drag and a click. The tests read
// each path, figure and sentence from its owner (the engine's record, the decision list, pb_text.h, the Preview's time
// box, the fill's dot, the fill card).

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

#include "app/path_costs.h"    // end_costs
#include "app/preview_view.h"  // build_time_box
#include "app/report.h"        // ChipToken
#include "core/model.h"        // group_thousands
#include "pathbuilder/pb_coach.h"
#include "pathbuilder/pb_files.h"
#include "pathbuilder/pb_text.h"
#include "ui/preview_controller.h"
#include "ui/preview_timeline.h"  // timeline_fill_id, timeline_ms_at
#include "ui/theme.h"             // chip_color
#include "ui/timeline_view.h"     // open_view, window_span_ms

namespace pbtest {

namespace {

using hydra::pathbuilder::EndChoice;
using hydra::pathbuilder::FillRefusal;
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

// Opens step `index` (click_step), waits for its endings to land and gives
// the screen two frames to draw them.
bool open_step(ImGuiTestContext* ctx, size_t index) {
    if (!click_step(ctx, index)) return false;
    if (!wait_idle(ctx)) return false;
    ctx->Yield(2);
    return true;
}

// The player's own total once the decision list's ladder has it
// (LadderRun::player_total, the figure the head shows under "Your score").
std::optional<int64_t> player_total(const PbState& s) {
    const hydra::app::LadderRun* run = s.decisions().run();
    if (!run || !run->complete) return std::nullopt;
    return run->ladder.player_total;
}

// True when the head shows the player's own total, in the screen's grouping
// (group_thousands).
bool shows_player_total(Harness& h) {
    const std::optional<int64_t> total = player_total(h.app->state);
    return total && on_screen(h, hydra::group_thousands(*total));
}

// True when the steps panel shows every step of the player's path as its
// compact row: the line holding the step's span (step_short) also holds the
// phrases it spends, its timings and its cost (cost_or_best of the decision
// list's row for it). False while a step's cost has not landed.
bool shows_step_rows(Harness& h) {
    const PbState& s = h.app->state;
    const hydra::pathbuilder::PlayerPath& mine = s.history().current();
    if (!mine.path) return false;
    const hydra::ActivationWalk acts = mine.path->walk_activations();
    const hydra::pathbuilder::CoachList& list = s.decisions().list();
    if (list.rows.size() < acts.size()) return false;
    const std::string text = visible_text(h);
    for (size_t i = 0; i < acts.size(); ++i) {
        if (!list.rows[i].cost) return false;
        const hydra::pathbuilder::StepShort brief =
            hydra::pathbuilder::step_short(s.chart()->song.timing(), acts[i]);
        const std::string line = uitest::lines_holding(text, brief.span);
        for (const std::string& part :
             {brief.saved, brief.squeezes, hydra::pathbuilder::cost_or_best(*list.rows[i].cost)})
            if (line.find(part) == std::string::npos) return false;
    }
    return true;
}

// ImGui's text log writes a radio button as "(x)" when it is on and "( )"
// when it is off (RadioButton, third_party/imgui/imgui_widgets.cpp). An ending
// line's mark is one.
constexpr const char* kMarkOn = "(x)";
constexpr const char* kMarkOff = "( )";

// The ending of the opened step `step` that the player's window kept
// (is_kept_end), from the endings the decision list worked out for it; none
// when the step is not the one opened or keeps none of them.
std::optional<EndChoice> kept_end(const PbState& s, size_t step) {
    const hydra::pathbuilder::DecisionList& d = s.decisions();
    const std::vector<hydra::PinnedWindow>& windows = s.history().current().windows;
    if (d.selected() != step || step >= windows.size()) return std::nullopt;
    for (const EndChoice& c : d.ends())
        if (hydra::pathbuilder::is_kept_end(windows[step], c)) return c;
    return std::nullopt;
}

// The kept ending of the opened step `step` and the first it did not keep.
struct EndPair {
    EndChoice kept;
    EndChoice other;
};
std::optional<EndPair> end_pair(const PbState& s, size_t step) {
    const std::optional<EndChoice> kept = kept_end(s, step);
    if (!kept) return std::nullopt;
    const std::vector<hydra::PinnedWindow>& windows = s.history().current().windows;
    for (const EndChoice& c : s.decisions().ends())
        if (!hydra::pathbuilder::is_kept_end(windows[step], c)) return EndPair{*kept, c};
    return std::nullopt;
}

// The words of an ending line (end_choice_text), on the open chart.
std::string end_words(const PbState& s, const EndChoice& end) {
    return hydra::pathbuilder::end_choice_text(s.chart()->song.timing(), end.activation);
}

// True when the opened step `step` shows each of its endings as a line: the
// line holding its words also holds its cost (cost_or_best of app::end_costs
// over the endings' best finishes) and its mark, on for the ending the
// player's window kept and off for every other. False with fewer than two
// endings, which draw no lines.
bool shows_end_lines(Harness& h, size_t step) {
    const PbState& s = h.app->state;
    const std::vector<EndChoice>& ends = s.decisions().ends();
    const std::vector<hydra::PinnedWindow>& windows = s.history().current().windows;
    if (s.decisions().selected() != step || ends.size() < 2 || step >= windows.size()) return false;
    std::vector<int64_t> finishes;
    for (const EndChoice& c : ends) finishes.push_back(c.best_finish);
    const std::vector<int64_t> costs = hydra::app::end_costs(finishes);
    const std::string text = visible_text(h);
    for (size_t e = 0; e < ends.size(); ++e) {
        const std::string line = uitest::lines_holding(text, end_words(s, ends[e]));
        const char* mark = hydra::pathbuilder::is_kept_end(windows[step], ends[e]) ? kMarkOn : kMarkOff;
        if (line.find(mark) == std::string::npos) return false;
        if (line.find(hydra::pathbuilder::cost_or_best(costs[e])) == std::string::npos) return false;
    }
    return true;
}

// The test engine's reference for an ending line, by the name draw_steps
// gives it (end_line_id).
std::string end_ref(const EndChoice& c) {
    return any_ref(hydra::pathbuilder::end_line_id(c.window.deact_tick, c.activation));
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

// One click on Burnout's second best-path fill, then "Finish the rest for
// me": the player's path becomes the one behind BF(1), the best finish with
// that one window kept (finish_for_me over the decision list's ladder), and
// the steps panel shows its steps.
void test_finish_for_me(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    PbState& s = h.app->state;
    const hydra::ActivationWalk best = s.chart()->record.best_path().walk_activations();
    IM_CHECK_RETV(best.size() >= 2, );
    const int64_t clicked = best[1].timecode.ticks();
    IM_CHECK_RETV(click_fill(ctx, clicked), );
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK(s.decisions().message().empty());
    int64_t one_window_total = 0;
    {
        const hydra::pathbuilder::PlayerPath& mine = s.history().current();
        IM_CHECK_RETV(mine.windows.size() == 1, );
        IM_CHECK_RETV(mine.path.has_value(), );
        // The click's window alone: one activation, on the clicked fill.
        const hydra::ActivationWalk acts = mine.path->walk_activations();
        IM_CHECK_RETV(acts.size() == 1, );
        IM_CHECK_EQ(acts[0].timecode.ticks(), clicked);
        one_window_total = mine.path->totalscore();
    }
    IM_CHECK(shows_player_total(h));
    IM_CHECK(shows_step_rows(h));

    // What Finish should make: the ladder's path for BF(1).
    const hydra::app::LadderRun* run = s.decisions().run();
    IM_CHECK_RETV(run != nullptr && run->complete, );
    const std::optional<hydra::Path> finish = hydra::pathbuilder::finish_for_me(*run, 1);
    IM_CHECK_RETV(finish.has_value(), );
    const hydra::Path expected = *finish;

    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick(("**/" + std::string(hydra::pathbuilder::kFinishForMeLabel)).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    const hydra::pathbuilder::PlayerPath& done = s.history().current();
    IM_CHECK_RETV(done.path.has_value(), );
    IM_CHECK_STR_EQ(done.path->pathstring().c_str(), expected.pathstring().c_str());
    IM_CHECK_EQ(done.path->totalscore(), expected.totalscore());
    IM_CHECK_EQ(done.windows.size(), expected.walk_activations().size());
    // The one window stays where the click put it, and the best finish with
    // it kept scores at least what that window alone did.
    IM_CHECK_EQ(done.path->walk_activations()[0].timecode.ticks(), clicked);
    IM_CHECK_GE(done.path->totalscore(), one_window_total);
    IM_CHECK(s.history().can_undo());
    IM_CHECK(shows_step_rows(h));
}

// True once the hover card on the fill at `tick` shows `sentence`. A click on
// a refused fill says nothing, so this card is where the player reads why.
bool card_says(ImGuiTestContext* ctx, int64_t tick, const std::string& sentence) {
    Harness& h = harness(ctx);
    ctx->MouseMove(fill_ref(tick).c_str());
    return uitest::wait_until(ctx, [&] { return on_screen(h, sentence); }, 5.0);
}

// The index in scene.fills of the first fill fill_refusal gives `kind`, or
// none.
std::optional<size_t> first_fill(const hydra::app::PreviewScene& scene, FillRefusal kind) {
    for (size_t i = 0; i < scene.fills.size(); ++i)
        if (hydra::pathbuilder::fill_refusal(scene, i) == kind) return i;
    return std::nullopt;
}

// The chord tick of the first fill the player can activate on next
// (fill_refusal gives it none), or none.
std::optional<int64_t> first_open_fill_tick(Harness& h) {
    const hydra::app::PreviewScene& scene = h.app->preview()->scene();
    const std::optional<size_t> i = first_fill(scene, FillRefusal::None);
    if (!i) return std::nullopt;
    return scene.fills[*i].chord_tick;
}

// A click on the first fill fill_refusal gives `kind` changes nothing in the
// player's path, and the fill's card gives fill_reason's sentence for it.
// Then the pointer leaves the timeline: left on the fill, it would carry into
// the next test, whose fresh builder would read it as a hover.
void refused_click_changes_nothing(ImGuiTestContext* ctx, FillRefusal kind) {
    Harness& h = harness(ctx);
    PbState& s = h.app->state;
    const hydra::app::PreviewScene& scene = h.app->preview()->scene();
    const std::optional<size_t> fill = first_fill(scene, kind);
    IM_CHECK_RETV(fill.has_value(), );
    const int64_t tick = scene.fills[*fill].chord_tick;
    const std::string reason = hydra::pathbuilder::fill_reason(scene, *fill);
    IM_CHECK_RETV(!reason.empty(), );
    const uint64_t generation = s.history().generation();
    const size_t windows = s.history().current().windows.size();
    const bool could_undo = s.history().can_undo();
    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick(fill_ref(tick).c_str());
    ctx->Yield(2);
    IM_CHECK_EQ(s.history().generation(), generation);
    IM_CHECK_EQ(s.history().current().windows.size(), windows);
    IM_CHECK_EQ(s.history().can_undo(), could_undo);
    IM_CHECK(card_says(ctx, tick, reason));
    move_off_timeline(ctx);
}

// A click on a fill under two saved phrases changes nothing, and the fill's
// card gives fill_reason's sentence.
void test_cant_activate_reason(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    refused_click_changes_nothing(ctx, FillRefusal::UnderBars);
    IM_CHECK(!h.app->state.history().current().path.has_value());
    IM_CHECK(!on_screen(h, "Can't activate at"));
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
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK_RETV(shows_step_rows(h), );
    refused_click_changes_nothing(ctx, FillRefusal::BeforeLast);
    // The one step is still on screen as it was.
    IM_CHECK(shows_step_rows(h));
}

// White Washed under the GUI tests' Path limit: the first window of Hydra's
// best path that the limit flags (tight_window_note) carries the violet note,
// under its ending lines, when its step is opened.
void test_tight_window_note(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::white_washed_notes()), );
    PbState& s = h.app->state;
    const std::optional<double> limit = s.chart()->settings.ms_filter;
    IM_CHECK_RETV(limit.has_value(), );
    IM_CHECK_RETV(paste_best_path(ctx), );
    // The paste makes every window of the best path the player's.
    IM_CHECK_RETV(s.history().current().windows.size() ==
                      s.chart()->record.best_path().walk_activations().size(), );
    IM_CHECK_RETV(s.history().current().path.has_value(), );
    const hydra::ActivationWalk acts = s.history().current().path->walk_activations();
    std::optional<size_t> tight;
    for (size_t i = 0; i < acts.size() && !tight; ++i)
        if (!hydra::pathbuilder::tight_window_note(acts[i], limit).empty()) tight = i;
    IM_CHECK_RETV(tight.has_value(), );
    const std::string note = hydra::pathbuilder::tight_window_note(acts[*tight], limit);

    IM_CHECK_RETV(open_step(ctx, *tight), );
    IM_CHECK(shows_end_lines(h, *tight));
    const std::optional<EndChoice> kept = kept_end(s, *tight);
    IM_CHECK_RETV(kept.has_value(), );
    const std::string kept_words = end_words(s, *kept);
    const std::string text = visible_text(h);
    IM_CHECK(text.find(kept_words) != std::string::npos);
    IM_CHECK(text.find(note) != std::string::npos);
    IM_CHECK(text.find(kept_words) < text.find(note));
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

// Gives the edit an ending click made time to land and the reopened step its
// endings, then checks the step now keeps `chosen` and the screen follows:
// the ending lines and their marks, the step rows, and the player's total in
// the head, which leaves `old_total` behind when the two differ.
void ending_moved_to(ImGuiTestContext* ctx, const EndChoice& chosen, int64_t old_total) {
    Harness& h = harness(ctx);
    PbState& s = h.app->state;
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK_RETV(s.history().current().windows.size() == 2, );
    IM_CHECK(hydra::pathbuilder::is_kept_end(s.history().current().windows[1], chosen));
    IM_CHECK(shows_end_lines(h, 1));
    IM_CHECK(shows_step_rows(h));
    IM_CHECK(shows_player_total(h));
    const std::optional<int64_t> total = player_total(s);
    IM_CHECK_RETV(total.has_value(), );
    if (*total != old_total) IM_CHECK(!on_screen(h, hydra::group_thousands(old_total)));
}

// Smash Mouth's I'm A Believer: its best path's second activation can end in
// more than one way. The opened step shows an ending line for each, the kept
// one marked; a click on another moves the mark, and the row and the score
// follow. Once a third activation is made, step 2 opened again shows the same
// lines, not clickable, the kept one still marked and the others greyed.
void test_choose_an_ending(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::believer_notes()), );
    PbState& s = h.app->state;
    const hydra::ActivationWalk best = s.chart()->record.best_path().walk_activations();
    IM_CHECK_RETV(best.size() >= 3, );

    IM_CHECK_RETV(click_fill(ctx, best[0].timecode.ticks()), );
    IM_CHECK_RETV(click_fill(ctx, best[1].timecode.ticks()), );
    IM_CHECK_RETV(open_step(ctx, 1), );
    IM_CHECK(shows_step_rows(h));
    IM_CHECK(on_screen(h, hydra::pathbuilder::kEndItAtLabel));
    IM_CHECK(shows_end_lines(h, 1));
    IM_CHECK(shows_player_total(h));
    // Copies: the endings are worked out again at each edit.
    const std::optional<EndPair> ends = end_pair(s, 1);
    IM_CHECK_RETV(ends.has_value(), );
    const std::string kept_ref = end_ref(ends->kept);
    const std::string other_ref = end_ref(ends->other);
    const std::optional<int64_t> kept_total = player_total(s);
    IM_CHECK_RETV(kept_total.has_value(), );
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK((ctx->ItemInfo(other_ref.c_str()).ItemFlags & ImGuiItemFlags_Disabled) == 0);

    // The other ending: the mark moves, the row and the score follow.
    ctx->ItemClick(other_ref.c_str());
    ending_moved_to(ctx, ends->other, *kept_total);
    const std::optional<int64_t> other_total = player_total(s);
    IM_CHECK_RETV(other_total.has_value(), );

    // And back.
    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick(kept_ref.c_str());
    ending_moved_to(ctx, ends->kept, *other_total);

    // A third activation, then step 2 again: its lines are fixed now.
    IM_CHECK_RETV(click_fill(ctx, best[2].timecode.ticks()), );
    IM_CHECK_RETV(s.history().current().windows.size() == 3, );
    IM_CHECK_RETV(open_step(ctx, 1), );
    IM_CHECK(shows_end_lines(h, 1));
    IM_CHECK(hydra::pathbuilder::is_kept_end(s.history().current().windows[1], ends->kept));
    ctx->SetRef(h.home_ref.c_str());
    const ImGuiTestItemInfo kept = ctx->ItemInfo(kept_ref.c_str());
    const ImGuiTestItemInfo other = ctx->ItemInfo(other_ref.c_str());
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
    ctx->ItemClick(other_ref.c_str());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK_EQ(s.history().generation(), generation);
    IM_CHECK_EQ(s.history().current().windows.size(), size_t{3});
    IM_CHECK(shows_end_lines(h, 1));

    // The opened step's old paragraphs and the panel's old lines are gone.
    for (const char* gone : {"Activate at", "This choice costs", "You still have Star Power",
                             "no timing needed", "Cost: how many points", "Before it, the best",
                             "Nothing left on the table"})
        IM_CHECK(!on_screen(h, gone));
}

// The Preview's clock as the screen drew it last frame, "0:25.123 / 2:10.088",
// or empty when it is not drawn.
std::string burnout_clock(Harness& h) {
    const std::string tail_text = clock_tail(h);
    if (tail_text.empty()) return std::string();
    const std::string text = visible_text(h);
    const size_t tail = text.find(tail_text);
    if (tail == std::string::npos) return std::string();
    size_t from = tail;
    while (from > 0 && text[from - 1] != ' ' && text[from - 1] != '\n' && text[from - 1] != '\r') --from;
    return text.substr(from, tail + tail_text.size() - from);
}

// The clock the Preview shows stopped on the fill at chord tick `tick`: the
// time box at the fill's dot (ui::timeline_fill_ms, the moment
// PbApp::click_fill seeks to). Empty when the scene has no fill on that chord.
std::string clock_on_fill(Harness& h, int64_t tick) {
    const hydra::app::PreviewScene& scene = h.app->preview()->scene();
    const std::optional<size_t> i = hydra::app::fill_index_on_chord(scene, tick);
    if (!i) return std::string();
    return clock_text_at(h, hydra::ui::timeline_fill_ms(scene.fills[*i]));
}

// The moment these tests park the paused Preview on, a test input.
constexpr double kParkedMs = 10000.0;

// Opens Burnout with the paused clock on kParkedMs and the test's ref on the
// builder's window. False when it did not settle there: the screen's clock
// is not the time box's for kParkedMs.
bool open_burnout_parked(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_chart(ctx, corpus::burnout_notes())) return false;
    ctx->SetRef(h.home_ref.c_str());
    // The pointer is wherever the last test left it, maybe on a fill.
    move_off_timeline(ctx);
    h.app->preview()->seek_ms(kParkedMs);
    ctx->Yield(2);
    return burnout_clock(h) == clock_text_at(h, kParkedMs);
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

// The clock a drag shows with the pointer where it is now: the time box at
// the moment timeline_ms_at reads under the pointer's x on the rows' item
// ("##timeline", as wide as the layout), over the window the song opens with
// (open_view). The caller checks that window holds the whole song, so it is
// the one on screen.
std::string clock_under_pointer(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    const ImRect rows = ctx->ItemInfo(any_ref("##timeline").c_str()).RectFull;
    hydra::ui::TimelineLayout layout;
    layout.width = rows.GetWidth();
    layout.view = hydra::ui::open_view(h.app->preview()->playback_end_ms());
    return clock_text_at(h, hydra::ui::timeline_ms_at(layout, ImGui::GetIO().MousePos.x - rows.Min.x));
}

// A hover moves nothing. Paused, a fill's card shows and a list row lights its
// fill while the clock and the Play button stay as they were, and they stay so
// when the pointer leaves. Playing, a hover does not pause.
void test_hover_moves_nothing(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_burnout_parked(ctx), );
    const std::string parked = burnout_clock(h);
    const std::optional<int64_t> fill = first_open_fill_tick(h);
    IM_CHECK_RETV(fill.has_value(), );
    const hydra::app::PreviewScene& scene = h.app->preview()->scene();
    const std::optional<size_t> index = hydra::app::fill_index_on_chord(scene, *fill);
    IM_CHECK_RETV(index.has_value(), );
    const std::string heading = hydra::pathbuilder::fill_hover_card(scene, *index, std::nullopt).heading;

    ctx->MouseMove(fill_ref(*fill).c_str());
    IM_CHECK(uitest::wait_until(ctx, [&] { return on_screen(h, heading); }, 5.0));
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), parked.c_str());
    IM_CHECK(reads_paused(h));
    move_off_timeline(ctx);
    IM_CHECK(!on_screen(h, heading));
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), parked.c_str());
    IM_CHECK(reads_paused(h));

    IM_CHECK_RETV(scroll_side_to(ctx, choice_ref(*fill)), );
    ctx->MouseMove(choice_ref(*fill).c_str());
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), parked.c_str());
    IM_CHECK(reads_paused(h));
    move_off_timeline(ctx);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), parked.c_str());
    IM_CHECK(reads_paused(h));

    ctx->ItemClick(any_ref("Play").c_str());
    ctx->Yield(2);
    IM_CHECK_RETV(reads_playing(h), );
    ctx->MouseMove(fill_ref(*fill).c_str());
    ctx->Yield(2);
    IM_CHECK(reads_playing(h));
    IM_CHECK(clock_runs(ctx));
    IM_CHECK_RETV(scroll_side_to(ctx, choice_ref(*fill)), );
    ctx->MouseMove(choice_ref(*fill).c_str());
    ctx->Yield(2);
    IM_CHECK(reads_playing(h));
    IM_CHECK(clock_runs(ctx));
    move_off_timeline(ctx);
    IM_CHECK(reads_playing(h));
    ctx->ItemClick(any_ref("Pause").c_str());
    ctx->Yield(2);
}

// A press and drag on the Star Power saved row: the clock follows the pointer
// while held (the moment under it, clock_under_pointer), onto the Fills row
// too, where no card shows and no fill takes the release, and the release
// leaves the clock where it is. Paused stays paused. A press while playing
// holds the song still, and the release plays on.
void test_drag_moves_the_clock(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_burnout_parked(ctx), );
    // The timeline's window holds the whole song, so it never moves and
    // clock_under_pointer's window is the one on screen.
    const double length_ms = h.app->preview()->playback_end_ms();
    IM_CHECK_RETV(hydra::ui::window_span_ms(length_ms) == length_ms, );
    hydra::pathbuilder::PbState& s = h.app->state;
    const uint64_t generation = s.history().generation();
    const std::optional<int64_t> fill_tick = first_open_fill_tick(h);
    IM_CHECK_RETV(fill_tick.has_value(), );

    ctx->MouseMoveToPos(saved_row_point(ctx, 0.25f));
    ctx->MouseDown(ImGuiMouseButton_Left);
    ctx->Yield(2);
    const std::string at_quarter = burnout_clock(h);
    IM_CHECK_STR_EQ(at_quarter.c_str(), clock_under_pointer(ctx).c_str());
    IM_CHECK(at_quarter != clock_text_at(h, kParkedMs));
    ctx->MouseMoveToPos(saved_row_point(ctx, 0.5f));
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), clock_under_pointer(ctx).c_str());
    IM_CHECK(burnout_clock(h) != at_quarter);
    IM_CHECK(reads_paused(h));

    // Down onto the Fills row, on a fill's hit box.
    const ImRect fill = ctx->ItemInfo(fill_ref(*fill_tick).c_str()).RectFull;
    ctx->MouseMoveToPos(fill.GetCenter());
    ctx->Yield(2);
    const std::string on_fills_row = burnout_clock(h);
    IM_CHECK_STR_EQ(on_fills_row.c_str(), clock_under_pointer(ctx).c_str());
    IM_CHECK(!on_screen(h, hydra::pathbuilder::kPhrasesSavedLabel));
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
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), clock_under_pointer(ctx).c_str());
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
    const std::string parked = burnout_clock(h);
    const std::optional<int64_t> fill = first_open_fill_tick(h);
    IM_CHECK_RETV(fill.has_value(), );
    const uint64_t generation = h.app->state.history().generation();
    ctx->MouseMove(fill_ref(*fill).c_str());
    ctx->MouseDown(ImGuiMouseButton_Left);
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), parked.c_str());
    ctx->MouseMoveToPos(saved_row_point(ctx, 0.75f));
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), parked.c_str());
    ctx->MouseUp(ImGuiMouseButton_Left);
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), parked.c_str());
    IM_CHECK_EQ(h.app->state.history().generation(), generation);
    move_off_timeline(ctx);
}

// A click on a fill leaves the clock on that fill's dot (clock_on_fill) with
// the pointer still on it, and it stays when the pointer leaves: on the first
// fill the player can activate on, on the taken fill the click made, and on
// the next fill the player can activate on, clicked while the Preview plays,
// which stops there.
void test_fill_click_moves_clock(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_burnout_parked(ctx), );
    const std::optional<int64_t> first = first_open_fill_tick(h);
    IM_CHECK_RETV(first.has_value(), );
    const std::string first_dot = clock_on_fill(h, *first);
    IM_CHECK_RETV(!first_dot.empty(), );
    // A click that left the clock parked would not pass.
    IM_CHECK_RETV(first_dot != clock_text_at(h, kParkedMs), );
    ctx->ItemClick(fill_ref(*first).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), first_dot.c_str());
    move_off_timeline(ctx);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), first_dot.c_str());

    // The same fill, taken now.
    h.app->preview()->seek_ms(kParkedMs);
    ctx->Yield(2);
    ctx->ItemClick(fill_ref(*first).c_str());
    ctx->Yield(2);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), first_dot.c_str());
    move_off_timeline(ctx);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), first_dot.c_str());

    const std::optional<int64_t> second = first_open_fill_tick(h);
    IM_CHECK_RETV(second.has_value() && *second != *first, );
    const std::string second_dot = clock_on_fill(h, *second);
    IM_CHECK_RETV(!second_dot.empty() && second_dot != first_dot, );
    ctx->ItemClick(any_ref("Play").c_str());
    ctx->Yield(2);
    IM_CHECK_RETV(reads_playing(h), );
    ctx->ItemClick(fill_ref(*second).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), second_dot.c_str());
    IM_CHECK(reads_paused(h));
}

// The numbers the fill card gives the fill at chord tick `tick` once its
// price has landed: the decision list's candidate for it, its ladder's BF(n)
// and the name of the fill that costs nothing (best_fill_name, the helper
// pb_window.cpp's draw_preview fills FillHoverNumbers with). None while any
// of them is missing.
std::optional<hydra::pathbuilder::FillHoverNumbers> card_numbers(const PbState& s,
                                                                 const hydra::app::PreviewScene& scene,
                                                                 int64_t tick) {
    const hydra::pathbuilder::DecisionList& d = s.decisions();
    const hydra::app::LadderRun* run = d.run();
    if (!run || !run->complete || run->ladder.bf.empty()) return std::nullopt;
    const std::vector<hydra::app::CandidateCost>& cands = d.candidates();
    const auto mine = std::find_if(cands.begin(), cands.end(),
                                   [&](const hydra::app::CandidateCost& c) { return c.act_tick == tick; });
    if (mine == cands.end()) return std::nullopt;
    const std::optional<std::string> best_fill = hydra::pathbuilder::best_fill_name(scene, cands);
    if (!best_fill) return std::nullopt;
    hydra::pathbuilder::FillHoverNumbers n;
    n.best_finish = mine->best_finish;
    n.best_now = run->ladder.bf.back();
    n.cost = mine->cost;
    n.tier = mine->tier;
    n.best_fill = *best_fill;
    n.hydra_end = d.candidate_end(static_cast<size_t>(mine - cands.begin()));
    return n;
}

// The card on Burnout's first fill the player can activate on, once its
// price has landed: fill_hover_card's heading, each of its rows with the
// figure on its label's line, and its grey lines, from the engine's numbers
// for that fill (card_numbers). Pinning that fill never scores above the best
// finish now. The old six sentences are gone, and the card goes with the
// pointer.
void test_fill_card(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    // Every price has landed once the builder is idle, so a card with no
    // numbers then can no longer get them.
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    move_off_timeline(ctx);
    const std::optional<int64_t> tick = first_open_fill_tick(h);
    IM_CHECK_RETV(tick.has_value(), );
    const hydra::app::PreviewScene& scene = h.app->preview()->scene();
    const std::optional<size_t> index = hydra::app::fill_index_on_chord(scene, *tick);
    IM_CHECK_RETV(index.has_value(), );
    const std::optional<hydra::pathbuilder::FillHoverNumbers> numbers =
        card_numbers(h.app->state, scene, *tick);
    IM_CHECK_RETV(numbers.has_value(), );
    IM_CHECK_GE(numbers->best_now, numbers->best_finish);
    const hydra::pathbuilder::FillHoverCard card = hydra::pathbuilder::fill_hover_card(scene, *index, numbers);
    IM_CHECK_RETV(!card.rows.empty() && !card.grey.empty(), );

    ctx->MouseMove(fill_ref(*tick).c_str());
    IM_CHECK_RETV(uitest::wait_until(ctx, [&] { return on_screen(h, card.heading); }, 5.0), );
    // ImGui's text log writes a table row's cells on one line, each after a
    // "|".
    for (const hydra::pathbuilder::CardRow& row : card.rows)
        IM_CHECK(on_screen(h, row.label + " | " + row.value));
    for (const std::string& line : card.grey) IM_CHECK(on_screen(h, line));
    IM_CHECK(!on_screen(h, "If you activate here"));
    move_off_timeline(ctx);
    IM_CHECK(!on_screen(h, card.grey.front()));
}

// One activation on Burnout, then a click on the list row of the next fill
// the player can activate on: once the pointer leaves the list, the clock
// stays on that fill's dot (clock_on_fill), as a click on the fill leaves it.
void test_row_click_moves_clock(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_chart(ctx, corpus::burnout_notes()), );
    const std::optional<int64_t> first = first_open_fill_tick(h);
    IM_CHECK_RETV(first.has_value(), );
    IM_CHECK_RETV(click_fill(ctx, *first), );
    const std::optional<int64_t> second = first_open_fill_tick(h);
    IM_CHECK_RETV(second.has_value() && *second != *first, );
    const std::string second_dot = clock_on_fill(h, *second);
    IM_CHECK_RETV(!second_dot.empty(), );
    // A click that left the clock where it was would not pass.
    IM_CHECK_RETV(burnout_clock(h) != second_dot, );
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK_RETV(scroll_side_to(ctx, choice_ref(*second)), );
    ctx->ItemClick(choice_ref(*second).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    IM_CHECK_STR_EQ(burnout_clock(h).c_str(), second_dot.c_str());
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
