// The Path Builder's click-through tests: a whole path built on Burnout by
// clicking the screen's own widgets (the timeline's fills, the "Where to
// activate next" rows, the step rows and the buttons), a path started past
// the first window of a song longer than it, and the window following the
// playhead while that song plays. Each step is checked in what the screen
// drew against the engine's own answer for the open chart, read in-process
// from the builder the test drives: the fills to click are the ones the
// scene offers (fill_refusal), the scores are the engine paths' totals, the
// card's figures are the decision list's priced candidates, the sentences
// are fill_reason's, the clocks are the time box's (app::build_time_box) and
// the timeline's windows are ui/timeline_view.h's. No figure is typed.

// First: its Windows headers come before the test engine's, which undefine
// their Yield() macro (uitest_engine.h says why).
#include "pbtest_harness.h"

#include <cmath>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "imgui_te_context.h"

#include "../testdata_dirs.h"

#include "app/path_costs.h"         // CandidateCost, costs_nothing
#include "app/preview_view.h"       // PreviewScene, fill_index_on_chord, build_time_box
#include "core/model.h"             // group_thousands, Path
#include "pathbuilder/pb_coach.h"   // kFinishForMeLabel
#include "pathbuilder/pb_menu.h"    // kStartOverLabel
#include "pathbuilder/pb_state.h"   // OpenChart, chart_title_line
#include "pathbuilder/pb_text.h"    // kNoStepsText, kBestFillLabel, fill_reason, the card
#include "pathbuilder/pb_window.h"  // the score and button labels
#include "ui/preview_controller.h"  // scene, playback_end_ms
#include "ui/preview_panel.h"       // kJumpSeconds, jump_forward_label
#include "ui/preview_timeline.h"    // timeline_fill_ms, kTimelineActivationsLabel, the scroll buttons
#include "ui/timeline_view.h"       // open_view, page, show, clamp, follow_playing, wheel, readout

namespace pbtest {

namespace {

using namespace hydra::pathbuilder;

// How long a check waits for a sentence whose search is still running.
constexpr double kTextWaitSeconds = 60.0;

// The Preview's "Act >" and "< Act" buttons (ui::draw_preview_panel).
constexpr const char* kNextActRef = "**/Act >##nextact";
constexpr const char* kPrevActRef = "**/< Act##prevact";

// How far one press of the Preview's forward jump button moves the clock
// (ui::kJumpSeconds), and the reference of that button.
constexpr double kJumpForwardMs = hydra::ui::kJumpSeconds * 1000.0;
std::string jump_forward_ref() { return any_ref(hydra::ui::jump_forward_label()); }

// What ImGui's text log writes between a table row's two cells: a fill
// card's label and its value land on one line, "Phrases saved | 2".
constexpr const char* kCellJoin = " | ";

bool is_line_end(char c) { return c == '\r' || c == '\n'; }

// The next non-empty line the screen drew after the line that is exactly
// `label`: the figure under a score's label, or the words under the
// timeline's "Your activations" row. Empty when the label is not drawn.
std::string figure_under(Harness& h, const std::string& label) {
    const std::string text = visible_text(h);
    size_t at = 0;
    while ((at = text.find(label, at)) != std::string::npos) {
        const size_t end = at + label.size();
        const bool starts_line = at == 0 || is_line_end(text[at - 1]);
        const bool ends_line = end == text.size() || is_line_end(text[end]);
        if (starts_line && ends_line) {
            size_t from = end;
            while (from < text.size() && is_line_end(text[from])) ++from;
            size_t to = from;
            while (to < text.size() && !is_line_end(text[to])) ++to;
            return text.substr(from, to - from);
        }
        at = end;
    }
    return std::string();
}

// How many times the word kBestFillLabel stands in the line the timeline
// drew under its "Your activations" row's name, where the words under the
// fills go.
size_t best_words_under_fills(Harness& h) {
    const std::string line = figure_under(h, hydra::ui::kTimelineActivationsLabel);
    const std::string word = kBestFillLabel;
    size_t count = 0;
    for (size_t at = line.find(word); at != std::string::npos; at = line.find(word, at + word.size()))
        ++count;
    return count;
}

// Waits until `s` is on screen, at most kTextWaitSeconds.
bool wait_text(ImGuiTestContext* ctx, const std::string& s) {
    Harness& h = harness(ctx);
    return uitest::wait_until(ctx, [&] { return on_screen(h, s); }, kTextWaitSeconds);
}

// The Preview's play button's two words (ui::draw_preview_panel): "Play"
// while paused, "Pause" while playing.
constexpr const char* kPlayWord = "Play";
constexpr const char* kPauseWord = "Pause";

// True when the Preview's play button reads `word` and no button reads the
// other word. The key bar's "Play/pause" is text, not a button, so the
// buttons are looked up as items. Reads from the test's current ref.
bool play_button_reads(ImGuiTestContext* ctx, const char* word) {
    const char* other = std::string(word) == kPlayWord ? kPauseWord : kPlayWord;
    return ctx->ItemInfo(any_ref(word).c_str(), ImGuiTestOpFlags_NoError).ID != 0 &&
           ctx->ItemInfo(any_ref(other).c_str(), ImGuiTestOpFlags_NoError).ID == 0;
}

// True when the opened step's button reads `remove` and no button reads the
// other Remove label (pb_window.h's kRemoveLastLabel and kRemoveEarlierLabel),
// so the step open is the last one or an earlier one as `remove` says. Only
// one step is open at a time. Reads from the test's current ref.
bool open_step_remove_reads(ImGuiTestContext* ctx, const char* remove) {
    const char* other = std::string(remove) == kRemoveLastLabel ? kRemoveEarlierLabel : kRemoveLastLabel;
    return ctx->ItemInfo(any_ref(remove).c_str(), ImGuiTestOpFlags_NoError).ID != 0 &&
           ctx->ItemInfo(any_ref(other).c_str(), ImGuiTestOpFlags_NoError).ID == 0;
}

// The first words of a step sentence ("Activate at m20 ..."). The step panel
// draws no such sentence, so these words must not be on screen.
constexpr const char* kOldSentenceStart = "Activate at";

// The scene the builder's Preview draws now: the player's path's scene.
const hydra::app::PreviewScene& scene_of(Harness& h) { return h.app->preview()->scene(); }

// The index in the scene's fills of the first fill fill_refusal gives
// `kind`, from the song's start, or none. When `outside` is set, only a fill
// whose dot is outside that window counts.
std::optional<size_t> first_fill_where(Harness& h, FillRefusal kind,
                                       const std::optional<hydra::ui::TimelineView>& outside = std::nullopt) {
    const hydra::app::PreviewScene& scene = scene_of(h);
    for (size_t i = 0; i < scene.fills.size(); ++i) {
        if (fill_refusal(scene, i) != kind) continue;
        if (outside && hydra::ui::in_view(*outside, hydra::ui::timeline_fill_ms(scene.fills[i]))) continue;
        return i;
    }
    return std::nullopt;
}

// Where the Preview's clock stands on the fill at chord tick `tick`: its dot
// (ui::timeline_fill_ms, the moment PbApp::click_fill seeks to). Below zero
// when the scene has no fill on that chord.
double fill_dot_ms(Harness& h, int64_t tick) {
    const hydra::app::PreviewScene& scene = scene_of(h);
    const std::optional<size_t> i = hydra::app::fill_index_on_chord(scene, tick);
    return i ? hydra::ui::timeline_fill_ms(scene.fills[*i]) : -1.0;
}

// The player's path as the engine made it from their windows, or null while
// they have none.
const hydra::Path* engine_player_path(Harness& h) {
    const std::optional<hydra::Path>& path = h.app->state.history().current().path;
    return path ? &*path : nullptr;
}

// The player's windows, as the history holds them.
const std::vector<hydra::PinnedWindow>& player_windows(Harness& h) {
    return h.app->state.history().current().windows;
}

// What "Your score" should read: the total of the engine's path for the
// player's windows, or of the chart's empty path while there is none.
std::string expected_your_score(Harness& h) {
    const hydra::Path* path = engine_player_path(h);
    if (path) return hydra::group_thousands(path->totalscore());
    const OpenChart* chart = h.app->state.chart();
    if (!chart || !chart->empty_path) return std::string();
    return hydra::group_thousands(chart->empty_path->totalscore());
}

// The decision list's priced candidate for the fill at chord tick `tick`, or
// null while it has not landed.
const hydra::app::CandidateCost* candidate_at(Harness& h, int64_t tick) {
    for (const hydra::app::CandidateCost& c : h.app->state.decisions().candidates())
        if (c.act_tick == tick) return &c;
    return nullptr;
}

// How many priced candidates lose nothing (app::costs_nothing): the fills
// the screen marks kBestFillLabel.
size_t best_candidate_count(Harness& h) {
    size_t count = 0;
    for (const hydra::app::CandidateCost& c : h.app->state.decisions().candidates())
        if (hydra::app::costs_nothing(c.cost)) ++count;
    return count;
}

// The compact row's span for each activation of the player's path
// (step_short), in step order. Empty while there is no path.
std::vector<std::string> step_spans(Harness& h) {
    std::vector<std::string> spans;
    const hydra::Path* path = engine_player_path(h);
    const OpenChart* chart = h.app->state.chart();
    if (!path || !chart) return spans;
    for (const hydra::Activation& act : path->activations)
        spans.push_back(step_short(chart->song.timing(), act).span);
    return spans;
}

// True when every step row the player's path makes is drawn.
bool all_step_rows_drawn(Harness& h) {
    for (const std::string& span : step_spans(h))
        if (!on_screen(h, span)) return false;
    return true;
}

// The card the fill at scene index `fill` shows while hovered: its heading
// (fill_hover_card's), and once its candidate has landed its three rows, each
// as ImGui's text log joins a label and its value: the phrases saved
// (bars_saved_at), the best finish here and the cost (cost_or_best), from the
// decision list's candidate. Waits for all of them; false when one never
// shows. `best_finish` gets the candidate's best finish.
bool card_shows_candidate(ImGuiTestContext* ctx, size_t fill, int64_t& best_finish) {
    Harness& h = harness(ctx);
    const hydra::app::PreviewScene& scene = scene_of(h);
    const int64_t tick = scene.fills[fill].chord_tick;
    const std::string heading = fill_hover_card(scene, fill, std::nullopt).heading;
    const std::string saved =
        std::string(kPhrasesSavedLabel) + kCellJoin + std::to_string(bars_saved_at(scene, scene.fills[fill]));
    ctx->MouseMove(fill_ref(tick).c_str());
    if (!uitest::wait_until(ctx, [&] { return candidate_at(h, tick) != nullptr && on_screen(h, heading); },
                            kTextWaitSeconds))
        return false;
    const hydra::app::CandidateCost priced = *candidate_at(h, tick);
    best_finish = priced.best_finish;
    const std::string finish =
        std::string(kBestFinishHereLabel) + kCellJoin + hydra::group_thousands(priced.best_finish);
    const std::string cost = std::string(kCostLabel) + kCellJoin + cost_or_best(priced.cost);
    return uitest::wait_until(
        ctx, [&] { return on_screen(h, saved) && on_screen(h, finish) && on_screen(h, cost); },
        kTextWaitSeconds);
}

// A click on the first fill fill_refusal gives `kind` changes nothing in the
// player's path, and its card gives fill_reason's sentence for it.
void refused_fill_changes_nothing(ImGuiTestContext* ctx, FillRefusal kind) {
    Harness& h = harness(ctx);
    const std::optional<size_t> fill = first_fill_where(h, kind);
    IM_CHECK_RETV(fill.has_value(), );
    const hydra::app::PreviewScene& scene = scene_of(h);
    const int64_t tick = scene.fills[*fill].chord_tick;
    const std::string reason = fill_reason(scene, *fill);
    IM_CHECK_RETV(!reason.empty(), );
    const uint64_t generation = h.app->state.history().generation();
    const size_t windows = player_windows(h).size();
    const std::string score = figure_under(h, kYourScoreLabel);
    ctx->ItemClick(fill_ref(tick).c_str());
    ctx->Yield(2);
    ctx->MouseMove(fill_ref(tick).c_str());
    IM_CHECK_RETV(wait_text(ctx, reason), );
    move_off_timeline(ctx);
    IM_CHECK_EQ(h.app->state.history().generation(), generation);
    IM_CHECK_EQ(player_windows(h).size(), windows);
    IM_CHECK(all_step_rows_drawn(h));
    IM_CHECK_STR_EQ(figure_under(h, kYourScoreLabel).c_str(), score.c_str());
}

void test_build_a_path_by_clicking(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = corpus::burnout_notes();
    reset_app(h);
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->SetRef(h.home_ref.c_str());
    const OpenChart* chart = h.app->state.chart();
    IM_CHECK_RETV(chart != nullptr && chart->empty_path.has_value(), );

    // The empty start: no steps, the chart's title, the empty path's total
    // as the player's score, and Hydra's best path's total as both the best
    // possible and the best still in reach.
    IM_CHECK_RETV(wait_text(ctx, kNoStepsText), );
    IM_CHECK(on_screen(h, chart_title_line(*chart)));
    const std::string empty_score = figure_under(h, kYourScoreLabel);
    IM_CHECK_STR_EQ(empty_score.c_str(), expected_your_score(h).c_str());
    const std::string best_total = hydra::group_thousands(chart->record.best_path().totalscore());
    IM_CHECK_STR_EQ(figure_under(h, kBestPossibleLabel).c_str(), best_total.c_str());
    IM_CHECK_STR_EQ(figure_under(h, kBestReachLabel).c_str(), best_total.c_str());

    // Before any click, the fills that cost nothing each say "best" under
    // them: as many as the decision list's candidates that lose nothing,
    // and at least one, Hydra's own next activation.
    IM_CHECK(uitest::wait_until(
        ctx, [&] { return best_candidate_count(h) > 0 && best_words_under_fills(h) == best_candidate_count(h); },
        kTextWaitSeconds));

    // The first fill the player can activate on: its card gives its phrases
    // saved, the best finish with it and the cost the decision list priced.
    const std::optional<size_t> first = first_fill_where(h, FillRefusal::None);
    IM_CHECK_RETV(first.has_value(), );
    const int64_t first_tick = scene_of(h).fills[*first].chord_tick;
    const double first_dot_ms = fill_dot_ms(h, first_tick);
    int64_t first_finish = 0;
    IM_CHECK_RETV(card_shows_candidate(ctx, *first, first_finish), );
    const std::string first_heading = fill_hover_card(scene_of(h), *first, std::nullopt).heading;
    move_off_timeline(ctx);
    IM_CHECK(!on_screen(h, first_heading));

    // The click on it: one window, at that fill; its compact row, open
    // because the click selected the step it made; the score is the engine's
    // path for that window, and the best still in reach is the best finish
    // the card offered.
    ctx->ItemClick(fill_ref(first_tick).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK_RETV(player_windows(h).size() == 1, );
    IM_CHECK_EQ(player_windows(h)[0].act_tick, first_tick);
    const std::vector<std::string> one_step_spans = step_spans(h);
    IM_CHECK_RETV(one_step_spans.size() == 1, );
    const std::string step_one_row = one_step_spans[0];
    IM_CHECK_RETV(wait_text(ctx, step_one_row), );
    IM_CHECK(open_step_remove_reads(ctx, kRemoveLastLabel));
    IM_CHECK(!on_screen(h, kOldSentenceStart));
    IM_CHECK(!on_screen(h, kNoStepsText));
    const std::string one_step_score = figure_under(h, kYourScoreLabel);
    IM_CHECK(one_step_score != empty_score);
    IM_CHECK_STR_EQ(one_step_score.c_str(), expected_your_score(h).c_str());
    const std::string first_finish_text = hydra::group_thousands(first_finish);
    IM_CHECK_STR_EQ(figure_under(h, kBestReachLabel).c_str(), first_finish_text.c_str());

    // The next fill the player can activate on, which the list names.
    // Hovering its row leaves the Preview's clock on the first fill's dot,
    // where the click put it, and moving off does too, its button still
    // reading Play.
    const std::optional<size_t> second = first_fill_where(h, FillRefusal::None);
    IM_CHECK_RETV(second.has_value(), );
    const int64_t second_tick = scene_of(h).fills[*second].chord_tick;
    const double second_dot_ms = fill_dot_ms(h, second_tick);
    const std::string clock_after_one = clock_text_at(h, first_dot_ms);
    IM_CHECK(on_screen(h, clock_after_one));
    IM_CHECK_RETV(scroll_side_to(ctx, choice_ref(second_tick)), );
    ctx->MouseMove(choice_ref(second_tick).c_str());
    ctx->Yield(2);
    IM_CHECK(on_screen(h, clock_after_one));
    move_off_timeline(ctx);
    IM_CHECK(on_screen(h, clock_after_one));
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK(play_button_reads(ctx, kPlayWord));

    // The click on that row makes the second activation at that fill: its
    // compact row, open, while the first step's compact row stays.
    ctx->ItemClick(choice_ref(second_tick).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK_RETV(player_windows(h).size() == 2, );
    IM_CHECK_EQ(player_windows(h)[1].act_tick, second_tick);
    const std::vector<std::string> two_step_spans = step_spans(h);
    IM_CHECK_RETV(two_step_spans.size() == 2, );
    const std::string step_two_row = two_step_spans[1];
    IM_CHECK_RETV(wait_text(ctx, step_two_row), );
    IM_CHECK(open_step_remove_reads(ctx, kRemoveLastLabel));
    IM_CHECK(!on_screen(h, kOldSentenceStart));
    IM_CHECK(on_screen(h, step_one_row));
    const std::string two_step_score = figure_under(h, kYourScoreLabel);
    IM_CHECK(two_step_score != one_step_score);
    IM_CHECK_STR_EQ(two_step_score.c_str(), expected_your_score(h).c_str());

    // The row click left the clock on the second fill's dot. "< Act" jumps
    // the Preview from there back to the first activation's, and "Act >"
    // forward again to the second's.
    const std::string clock_after_two = clock_text_at(h, second_dot_ms);
    IM_CHECK(on_screen(h, clock_after_two));
    ctx->ItemClick(kPrevActRef);
    ctx->Yield(2);
    IM_CHECK(on_screen(h, clock_after_one));
    ctx->ItemClick(kNextActRef);
    ctx->Yield(2);
    IM_CHECK(on_screen(h, clock_after_two));

    // A click on the first step's row opens it and closes the second: the
    // open step's button is the one an earlier step draws, and both rows
    // stay.
    IM_CHECK_RETV(click_step(ctx, 0), );
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK(open_step_remove_reads(ctx, kRemoveEarlierLabel));
    IM_CHECK(!on_screen(h, kOldSentenceStart));
    IM_CHECK(on_screen(h, step_one_row));
    IM_CHECK(on_screen(h, step_two_row));

    // A fill with too few phrases saved, and a fill before the last
    // activation: each card gives fill_reason's sentence, and the path stays
    // as it was.
    refused_fill_changes_nothing(ctx, FillRefusal::UnderBars);
    refused_fill_changes_nothing(ctx, FillRefusal::BeforeLast);
    IM_CHECK_STR_EQ(figure_under(h, kYourScoreLabel).c_str(), two_step_score.c_str());

    // Undo takes the second step back.
    ctx->ItemClick(any_ref(kUndoLabel).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK_EQ(player_windows(h).size(), size_t{1});
    IM_CHECK(!on_screen(h, step_two_row));
    IM_CHECK(on_screen(h, step_one_row));
    IM_CHECK_STR_EQ(figure_under(h, kYourScoreLabel).c_str(), one_step_score.c_str());

    // "Finish the rest for me": the first step stays, the engine's path for
    // the finished windows draws every step row, and the player's score
    // becomes the best they could still reach.
    const std::string reach_before_finish = figure_under(h, kBestReachLabel);
    ctx->ItemClick(any_ref(kFinishForMeLabel).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK_RETV(wait_idle(ctx), );
    IM_CHECK(player_windows(h).size() > 1);
    IM_CHECK_EQ(player_windows(h)[0].act_tick, first_tick);
    IM_CHECK(all_step_rows_drawn(h));
    const std::string finished_score = figure_under(h, kYourScoreLabel);
    IM_CHECK_STR_EQ(finished_score.c_str(), expected_your_score(h).c_str());
    IM_CHECK_STR_EQ(finished_score.c_str(), reach_before_finish.c_str());
    IM_CHECK_STR_EQ(finished_score.c_str(), figure_under(h, kBestReachLabel).c_str());

    // Start over: back to no steps.
    ctx->ItemClick(any_ref(kStartOverLabel).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK_RETV(wait_text(ctx, kNoStepsText), );
    IM_CHECK(player_windows(h).empty());
    IM_CHECK(!on_screen(h, step_one_row));
    IM_CHECK_STR_EQ(figure_under(h, kYourScoreLabel).c_str(), empty_score.c_str());
}

// Opens the long song (long_song_notes) with nothing hovered, and leaves the
// test's ref on the builder's window. False when it did not settle.
bool open_long_song(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = long_song_notes();
    reset_app(h);
    if (!wait_idle(ctx)) return false;
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    // The pointer is wherever the last test left it, maybe on a fill.
    move_off_timeline(ctx);
    return !ctx->IsError();
}

// Where the open song's Preview ends, in ms.
double song_length_ms(Harness& h) { return h.app->preview()->playback_end_ms(); }

// The readout's tail on the open song, " of 5:59": the part of
// ui::readout that names the song's length, the same for every window.
std::string readout_tail(Harness& h) {
    const double length_ms = song_length_ms(h);
    const std::string whole = hydra::ui::readout(hydra::ui::open_view(length_ms), length_ms);
    const size_t of = whole.rfind(" of ");
    return of == std::string::npos ? std::string() : whole.substr(of);
}

// The readout ui::readout gives `view` on the open song.
std::string window_readout(Harness& h, const hydra::ui::TimelineView& view) {
    return hydra::ui::readout(view, song_length_ms(h));
}

// Gives the last action two frames, then reads `expected` on screen; the
// lines that hold a readout are logged when it is not there.
bool readout_reads(ImGuiTestContext* ctx, const std::string& expected) {
    Harness& h = harness(ctx);
    ctx->Yield(2);
    if (on_screen(h, expected)) return true;
    ctx->LogError("readout lines: %s", uitest::lines_holding(visible_text(h), readout_tail(h)).c_str());
    return false;
}

// The long song past its first window: one window forward, the card of a fill
// there, a click on it, and "< Act" from a window scrolled back to the start,
// which brings the window to the activation.
void test_build_a_path_on_a_long_song(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_long_song(ctx), );
    const double length_ms = song_length_ms(h);
    const hydra::ui::TimelineView open = hydra::ui::open_view(length_ms);
    // The song is longer than one window, or there is no far fill to reach.
    IM_CHECK_RETV(open.span_ms < length_ms, );
    const std::string forward = any_ref(hydra::ui::kScrollForwardLabel);
    const std::string back = any_ref(hydra::ui::kScrollBackLabel);
    IM_CHECK_RETV(readout_reads(ctx, window_readout(h, open)), );
    IM_CHECK_STR_EQ(figure_under(h, kYourScoreLabel).c_str(), expected_your_score(h).c_str());

    // The first fill the player can activate on whose dot is past the
    // opening window: it has no item yet.
    const std::optional<size_t> far_fill = first_fill_where(h, FillRefusal::None, open);
    IM_CHECK_RETV(far_fill.has_value(), );
    const int64_t far_tick = scene_of(h).fills[*far_fill].chord_tick;
    const double far_dot_ms = fill_dot_ms(h, far_tick);
    IM_CHECK(ctx->ItemInfo(fill_ref(far_tick).c_str(), ImGuiTestOpFlags_NoError).ID == 0);

    // One window forward (ui::page), held inside the song.
    const hydra::ui::TimelineView ahead = hydra::ui::page(open, 1, length_ms);
    IM_CHECK_RETV(hydra::ui::in_view(ahead, far_dot_ms), );
    ctx->ItemClick(forward.c_str());
    IM_CHECK_RETV(readout_reads(ctx, window_readout(h, ahead)), );

    // The hover on the fill: its card, with the decision list's figures.
    int64_t far_finish = 0;
    IM_CHECK_RETV(card_shows_candidate(ctx, *far_fill, far_finish), );

    // The click on it: one window at that fill, its compact row, the
    // engine's score for it, and the card's best finish as the best still in
    // reach.
    ctx->ItemClick(fill_ref(far_tick).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    IM_CHECK_RETV(wait_idle(ctx), );
    IM_CHECK_RETV(player_windows(h).size() == 1, );
    IM_CHECK_EQ(player_windows(h)[0].act_tick, far_tick);
    const std::vector<std::string> spans = step_spans(h);
    IM_CHECK_RETV(spans.size() == 1, );
    IM_CHECK_RETV(wait_text(ctx, spans[0]), );
    IM_CHECK_STR_EQ(figure_under(h, kYourScoreLabel).c_str(), expected_your_score(h).c_str());
    const std::string far_finish_text = hydra::group_thousands(far_finish);
    IM_CHECK_STR_EQ(figure_under(h, kBestReachLabel).c_str(), far_finish_text.c_str());

    // The click left the clock on the fill's dot; the forward jump carries
    // it on by kJumpForwardMs, and the window goes where ui::show puts it.
    IM_CHECK(on_screen(h, clock_text_at(h, far_dot_ms)));
    ctx->ItemClick(jump_forward_ref().c_str());
    const double jumped_ms = far_dot_ms + kJumpForwardMs;
    const hydra::ui::TimelineView after_jump = hydra::ui::show(ahead, jumped_ms, length_ms);
    IM_CHECK(readout_reads(ctx, window_readout(h, after_jump)));
    IM_CHECK(on_screen(h, clock_text_at(h, jumped_ms)));
    // One window back: the clock stays where it was.
    ctx->ItemClick(back.c_str());
    const hydra::ui::TimelineView behind = hydra::ui::page(after_jump, -1, length_ms);
    IM_CHECK(readout_reads(ctx, window_readout(h, behind)));
    IM_CHECK(on_screen(h, clock_text_at(h, jumped_ms)));
    // "< Act" jumps the clock back to the activation, and the window goes
    // where ui::show puts it for that moment.
    const std::vector<hydra::app::PreviewActivation>& acts = scene_of(h).activations;
    IM_CHECK_RETV(acts.size() == 1, );
    const double act_ms = acts[0].ms;
    ctx->ItemClick(kPrevActRef);
    IM_CHECK(readout_reads(ctx, window_readout(h, hydra::ui::show(behind, act_ms, length_ms))));
    IM_CHECK(on_screen(h, clock_text_at(h, act_ms)));
}

// One reading of the screen on the long song: the readout's start and end
// in whole seconds, its text, and the Preview's clock in ms. A part the
// screen did not show stays below zero.
struct LongSongShown {
    std::string readout;
    int start_s = -1;
    int end_s = -1;
    double clock_ms = -1.0;
};

// "m:ss" or "m:ss.mmm" as ms; below zero when it is neither.
double clock_text_ms(const std::string& t) {
    const size_t colon = t.find(':');
    if (colon == 0 || colon == std::string::npos || colon + 3 > t.size()) return -1.0;
    const size_t dot = t.find('.', colon);
    const std::string mins = t.substr(0, colon);
    const std::string secs = t.substr(colon + 1, dot == std::string::npos ? std::string::npos : dot - colon - 1);
    const std::string frac = dot == std::string::npos ? std::string("0") : t.substr(dot + 1);
    for (const std::string* part : {&mins, &secs, &frac})
        if (part->empty() || part->find_first_not_of("0123456789") != std::string::npos) return -1.0;
    return (std::stoi(mins) * 60.0 + std::stoi(secs)) * 1000.0 + std::stoi(frac);
}

// The word before `at` in `text`, from the last space or line end.
std::string word_before(const std::string& text, size_t at) {
    size_t from = at;
    while (from > 0 && text[from - 1] != ' ' && text[from - 1] != '\n' && text[from - 1] != '\r' &&
           text[from - 1] != '{')
        --from;
    return text.substr(from, at - from);
}

LongSongShown read_long_song(Harness& h) {
    LongSongShown shown;
    const std::string text = visible_text(h);
    const std::string dash = " – ";
    const std::string of_tail = readout_tail(h);
    const size_t of = of_tail.empty() ? std::string::npos : text.find(of_tail);
    const size_t at_dash = of == std::string::npos ? std::string::npos : text.rfind(dash, of);
    if (at_dash != std::string::npos) {
        const std::string start = word_before(text, at_dash);
        const std::string end = text.substr(at_dash + dash.size(), of - at_dash - dash.size());
        shown.readout = start + dash + end + of_tail;
        shown.start_s = static_cast<int>(clock_text_ms(start) / 1000.0);
        shown.end_s = static_cast<int>(clock_text_ms(end) / 1000.0);
    }
    const std::string tail_text = clock_tail(h);
    const size_t tail = tail_text.empty() ? std::string::npos : text.find(tail_text);
    if (tail != std::string::npos) shown.clock_ms = clock_text_ms(word_before(text, tail));
    return shown;
}

// How many forward-jump presses carry the clock across `ms`, rounded up. The
// clock also runs while playing, which only shortens the way.
int presses_across(double ms) { return static_cast<int>(std::ceil(ms / kJumpForwardMs)); }

// Presses the forward jump until `done` holds, at most `cap` presses. False,
// with `what` logged, when it never held.
bool press_jump_until(ImGuiTestContext* ctx, int cap, const std::function<bool()>& done,
                      const char* what) {
    for (int i = 0; i < cap && !done(); ++i) {
        ctx->ItemClick(jump_forward_ref().c_str());
        ctx->Yield(2);
        if (ctx->IsError()) return false;
    }
    if (done()) return true;
    ctx->LogError("%s after %d presses", what, cap);
    return false;
}

// The long song playing: the forward jump carries the playhead past three
// quarters of the window and the window turns; a wheel notch moves it and
// stops the following, so the playhead runs out of the window and the window
// stays; a pause and a second Play turn following back on. The clock runs on
// real time while playing, so where the first turn lands is checked against
// the turn's own constants and ui::follow_playing at the clock after it, and
// the last window is the one ui::clamp holds at the song's end.
void test_follow_the_playhead(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_long_song(ctx), );
    const double length_ms = song_length_ms(h);
    const hydra::ui::TimelineView open = hydra::ui::open_view(length_ms);
    const std::string opening = window_readout(h, open);
    IM_CHECK_RETV(readout_reads(ctx, opening), );

    ctx->ItemClick(any_ref(kPlayWord).c_str());
    ctx->Yield(2);
    IM_CHECK_RETV(play_button_reads(ctx, kPauseWord), );

    // The first turn comes before the playhead crosses one whole window.
    const bool turned = press_jump_until(
        ctx, presses_across(open.span_ms),
        [&] { return read_long_song(h).readout != opening; }, "the window never turned");
    IM_CHECK_RETV(turned, );
    const LongSongShown first = read_long_song(h);
    IM_CHECK_RETV(first.start_s >= 0 && first.clock_ms >= 0.0, );
    // The turn put the playhead a quarter in, from a playhead past three
    // quarters of the window and no later than the clock now reads. The clock
    // is real time, so those two bound the start; neither is a clock text.
    const double earliest_ms =
        (hydra::ui::kFollowTurnShare - hydra::ui::kPlayheadLeadShare) * open.span_ms;
    const double latest_ms =
        hydra::ui::follow_playing(open, first.clock_ms, length_ms).start_ms;
    IM_CHECK_GE(first.start_s, static_cast<int>(earliest_ms / 1000.0));
    IM_CHECK_LE(first.start_s, static_cast<int>(latest_ms / 1000.0));

    // A wheel notch down over the rows, away from the Fills row, moves the
    // window by one notch (ui::wheel) and stops the following.
    const ImRect rows = ctx->ItemInfo(any_ref("##timeline").c_str()).RectFull;
    ctx->MouseMoveToPos(ImVec2(rows.GetCenter().x, rows.Min.y + rows.GetHeight() * 0.4f));
    ctx->MouseWheelY(-1.0f);
    ctx->Yield(2);
    const LongSongShown wheeled = read_long_song(h);
    IM_CHECK_RETV(wheeled.start_s >= 0 && wheeled.clock_ms >= 0.0, );
    const int notch_s =
        static_cast<int>(hydra::ui::wheel(open, 1.0, length_ms).start_ms / 1000.0);
    IM_CHECK_EQ(wheeled.start_s, first.start_s + notch_s);
    IM_CHECK_EQ(wheeled.end_s, first.end_s + notch_s);
    IM_CHECK_RETV(play_button_reads(ctx, kPauseWord), );

    // The forward jump until the playhead is past the window's right edge:
    // the window stays where the wheel put it. The readout floors its end to
    // whole seconds, so the window ends before the next whole second.
    const double past_end_ms = (wheeled.end_s + 1) * 1000.0;
    IM_CHECK_RETV(press_jump_until(
                      ctx, presses_across(past_end_ms - wheeled.clock_ms),
                      [&] { return read_long_song(h).clock_ms > past_end_ms; },
                      "the playhead never left the window"), );
    IM_CHECK_RETV(play_button_reads(ctx, kPauseWord), );
    IM_CHECK_STR_EQ(read_long_song(h).readout.c_str(), wheeled.readout.c_str());

    // A pause moves nothing; a second Play turns following back on, and the
    // window comes to the playhead, held at the song's end: the window
    // ui::clamp makes of one that starts where the song ends.
    ctx->ItemClick(any_ref(kPauseWord).c_str());
    ctx->Yield(2);
    IM_CHECK(play_button_reads(ctx, kPlayWord));
    IM_CHECK_STR_EQ(read_long_song(h).readout.c_str(), wheeled.readout.c_str());
    hydra::ui::TimelineView at_end = open;
    at_end.start_ms = length_ms;
    const std::string last_window = window_readout(h, hydra::ui::clamp(at_end, length_ms));
    ctx->ItemClick(any_ref(kPlayWord).c_str());
    IM_CHECK(readout_reads(ctx, last_window));
    ctx->ItemClick(any_ref(kPauseWord).c_str());
    ctx->Yield(2);
}

}  // namespace

void register_build_tests(Harness& h) {
    uitest::register_test(h, "build-a-path-by-clicking", test_build_a_path_by_clicking);
    uitest::register_test(h, "build-a-path-on-a-long-song", test_build_a_path_on_a_long_song);
    uitest::register_test(h, "follow-the-playhead", test_follow_the_playhead);
}

}  // namespace pbtest
