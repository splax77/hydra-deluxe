// The Path Builder's click-through tests: a whole path built on Burnout by
// clicking the screen's own widgets (the timeline's fills, the "Where to
// activate next" rows, the step rows and the buttons), a path started past
// the first window of a song longer than it, and the window following the
// playhead while that song plays. Each step is checked only in what the
// screen drew (its text and its items). Every tick, sentence, clock and
// figure is pinned from one run under the GUI tests' settings
// (tests/scratch_settings.h).

// First: its Windows headers come before the test engine's, which undefine
// their Yield() macro (uitest_engine.h says why).
#include "pbtest_harness.h"

#include <cmath>
#include <cstdint>
#include <functional>
#include <string>

#include "imgui_te_context.h"

#include "../testdata_dirs.h"

#include "pathbuilder/pb_coach.h"   // kFinishForMeLabel
#include "pathbuilder/pb_menu.h"    // kStartOverLabel
#include "pathbuilder/pb_text.h"    // kNoStepsText, kBestFillLabel, locked_message
#include "pathbuilder/pb_window.h"  // the score and button labels
#include "ui/preview_panel.h"       // kJumpSeconds, jump_forward_label
#include "ui/preview_timeline.h"    // timeline_fill_id, kTimelineFillsLabel, the scroll buttons
#include "ui/timeline_view.h"       // kTimelineWindowMs, open_view, follow_playing, wheel

namespace pbtest {

namespace {

using namespace hydra::pathbuilder;

// How long a check waits for a sentence whose search is still running.
constexpr double kTextWaitSeconds = 60.0;

// Burnout's other fills the test clicks, by chord tick, from one run (the
// first two are kFirstFillTick and kSecondFillTick in pbtest_harness.h): the
// fill at m44, which comes after the second activation's Star Power has ended,
// with no phrases saved there; and the fill at m32, which sits between the two
// activations.
constexpr int64_t kCantFillTick = 83040;
constexpr int64_t kLockedFillTick = 59520;

// The Preview's "Act >" and "< Act" buttons (ui::draw_preview_panel).
constexpr const char* kNextActRef = "**/Act >##nextact";
constexpr const char* kPrevActRef = "**/< Act##prevact";

// How far one press of the Preview's forward jump button moves the clock
// (ui::kJumpSeconds), and the reference of that button.
constexpr double kJumpForwardMs = hydra::ui::kJumpSeconds * 1000.0;
std::string jump_forward_ref() { return any_ref(hydra::ui::jump_forward_label()); }

// The long song (long_song_notes), from one run: the Preview's end as the
// clock spells it and in ms, the readout's tail, and the chord tick of its
// fill at m69 (3:15.859), past the first window.
constexpr const char* kLongSongClockTail = " / 5:59.957";
constexpr double kLongSongMs = 359957.0;
constexpr const char* kLongSongOf = " of 5:59";
constexpr int64_t kLongSongFillTick = 133920;

bool is_line_end(char c) { return c == '\r' || c == '\n'; }

// The next non-empty line the screen drew after the line that is exactly
// `label`: the figure under a score's label, or the words under the
// timeline's Fills row. Empty when the label is not drawn.
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
// drew under its Fills row's name, where the words under the fills go.
size_t best_words_under_fills(Harness& h) {
    const std::string line = figure_under(h, hydra::ui::kTimelineFillsLabel);
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

void test_build_a_path_by_clicking(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = corpus::burnout_notes();
    reset_app(h);
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->SetRef(h.home_ref.c_str());

    // The empty start: the guide's first step, no steps, and the three scores.
    IM_CHECK_RETV(wait_text(ctx, "Step 1. Click a glowing fill"), );
    IM_CHECK(on_screen(h, "Burnout — Green Day"));
    IM_CHECK(on_screen(h, kNoStepsText));
    const std::string empty_score = figure_under(h, kYourScoreLabel);
    IM_CHECK_STR_EQ(empty_score.c_str(), "313,635");
    IM_CHECK_STR_EQ(figure_under(h, kBestPossibleLabel).c_str(), "378,315");
    IM_CHECK_STR_EQ(figure_under(h, kBestReachLabel).c_str(), "378,315");

    // Before any click, every fill that costs nothing says "best" under it.
    // From one run: one such fill, m32, whose list row says "best" too.
    IM_CHECK_EQ(best_words_under_fills(h), size_t{1});
    IM_CHECK(on_screen(h, "m32 0:40.945 · 3 phrases · ends m38 · squeeze out 163.0 ms best"));

    // The hover on the first fill: its card gives the phrases saved, the best
    // finish with this fill and its cost, then the best choice now and where
    // Hydra would end it (the words fill-card pins). ImGui's text log writes
    // a card row's two cells on one line, each after a "|".
    const std::string best_line = "Best choice now is m32 (378,315).";
    ctx->MouseMove(fill_ref(kFirstFillTick).c_str());
    IM_CHECK_RETV(wait_text(ctx, best_line), );
    IM_CHECK(on_screen(h, "m20 · 0:25.123"));
    IM_CHECK(on_screen(h, "Phrases saved | 2"));
    IM_CHECK(on_screen(h, "Best finish here | 378,175"));
    IM_CHECK(on_screen(h, "Cost | −140"));
    IM_CHECK(on_screen(h, "Hydra would end it at m24."));
    move_off_timeline(ctx);
    IM_CHECK(!on_screen(h, best_line));

    // The click on it: the steps panel gains its compact row, open because
    // the click selected the step it made, and the score line moves. From
    // one run: the step has one ending, so the opened step draws no ending
    // lines, only its Remove button.
    const std::string step_one_row = "m20 → m24 2 phrases −140";
    ctx->ItemClick(fill_ref(kFirstFillTick).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    IM_CHECK_RETV(wait_text(ctx, step_one_row), );
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK(open_step_remove_reads(ctx, kRemoveLastLabel));
    IM_CHECK(!on_screen(h, kEndItAtLabel));
    IM_CHECK(!on_screen(h, kOldSentenceStart));
    IM_CHECK(!on_screen(h, kNoStepsText));
    IM_CHECK(!on_screen(h, "m40 →"));
    const std::string one_step_score = figure_under(h, kYourScoreLabel);
    IM_CHECK(one_step_score != empty_score);
    IM_CHECK_STR_EQ(one_step_score.c_str(), "327,135");
    IM_CHECK_STR_EQ(figure_under(h, kBestReachLabel).c_str(), "378,175");

    // The list names the second fill. Hovering its row moves the Preview's
    // clock to the fill's dot, the moment its row and card name; moving off
    // puts it back and leaves the Preview paused, its button still reading
    // Play. From one run: the click on the first fill left the clock on that
    // fill's dot (m20, 0:25.123).
    const std::string clock_after_one = "0:25.123 / 2:10.088";
    IM_CHECK(on_screen(h, "m40 0:51.421 · 2 phrases · ends m44 −1,580"));
    IM_CHECK(on_screen(h, clock_after_one));
    IM_CHECK_RETV(scroll_side_to(ctx, choice_ref(kSecondFillTick)), );
    ctx->MouseMove(choice_ref(kSecondFillTick).c_str());
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "0:51.421 / 2:10.088"));
    IM_CHECK(!on_screen(h, clock_after_one));
    move_off_timeline(ctx);
    IM_CHECK(on_screen(h, clock_after_one));
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK(play_button_reads(ctx, kPlayWord));

    // The click on that row makes the second activation: its compact row,
    // open, while the first step's compact row stays. From one run: this
    // step has one ending too, so it draws only its Remove button.
    const std::string step_two_row = "m40 → m44 2 phrases −1,580";
    ctx->ItemClick(choice_ref(kSecondFillTick).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    IM_CHECK_RETV(wait_text(ctx, step_two_row), );
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK(open_step_remove_reads(ctx, kRemoveLastLabel));
    IM_CHECK(!on_screen(h, kEndItAtLabel));
    IM_CHECK(!on_screen(h, kOldSentenceStart));
    IM_CHECK(on_screen(h, step_one_row));
    const std::string two_step_score = figure_under(h, kYourScoreLabel);
    IM_CHECK(two_step_score != one_step_score);
    IM_CHECK_STR_EQ(two_step_score.c_str(), "340,675");
    IM_CHECK_STR_EQ(figure_under(h, kBestReachLabel).c_str(), "376,595");

    // The row click left the clock on the second fill's dot, as a click on a
    // fill does (the clock row-click-moves-clock pins): the second
    // activation's own clock (m40, 0:51.421). "< Act" jumps the Preview from
    // there back to the first activation's clock (m20, 0:25.123), and "Act >"
    // forward again to the second's.
    IM_CHECK(on_screen(h, "0:51.421 / 2:10.088"));
    ctx->ItemClick(kPrevActRef);
    ctx->Yield(2);
    IM_CHECK(on_screen(h, clock_after_one));
    ctx->ItemClick(kNextActRef);
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "0:51.421 / 2:10.088"));

    // A click on the first step's row opens it and closes the second: the
    // open step's button is the one an earlier step draws, and both rows
    // stay.
    IM_CHECK_RETV(click_step(ctx, 0), );
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK(open_step_remove_reads(ctx, kRemoveEarlierLabel));
    IM_CHECK(!on_screen(h, kEndItAtLabel));
    IM_CHECK(!on_screen(h, kOldSentenceStart));
    IM_CHECK(on_screen(h, step_one_row));
    IM_CHECK(on_screen(h, step_two_row));

    // A fill the player cannot activate on: the guide line says why, and the
    // path stays as it was.
    ctx->ItemClick(fill_ref(kCantFillTick).c_str());
    ctx->Yield(2);
    move_off_timeline(ctx);
    IM_CHECK(on_screen(h, "Can't activate at m44: You have 0 phrases saved here. You need 2 to activate."));
    IM_CHECK(on_screen(h, step_two_row));
    IM_CHECK_STR_EQ(figure_under(h, kYourScoreLabel).c_str(), two_step_score.c_str());

    // A fill before the last activation is locked: the guide line says the
    // builder goes left to right, in locked_message's words, and the path
    // stays as it was.
    ctx->ItemClick(fill_ref(kLockedFillTick).c_str());
    ctx->Yield(2);
    move_off_timeline(ctx);
    IM_CHECK(on_screen(h, locked_message("m32").text));
    IM_CHECK(on_screen(h, step_one_row));
    IM_CHECK(on_screen(h, step_two_row));
    IM_CHECK(!on_screen(h, "m32 →"));
    IM_CHECK_STR_EQ(figure_under(h, kYourScoreLabel).c_str(), two_step_score.c_str());

    // Undo takes the second step back.
    ctx->ItemClick(any_ref(kUndoLabel).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK(!on_screen(h, step_two_row));
    IM_CHECK(on_screen(h, step_one_row));
    IM_CHECK_STR_EQ(figure_under(h, kYourScoreLabel).c_str(), one_step_score.c_str());

    // "Finish the rest for me": the player's score becomes the best they could
    // still reach.
    const std::string reach_before_finish = figure_under(h, kBestReachLabel);
    ctx->ItemClick(any_ref(kFinishForMeLabel).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK_RETV(wait_text(ctx, "Hydra kept your 1 activation and added the best rest of the path. Undo "
                                 "takes it back."), );
    IM_CHECK(on_screen(h, step_one_row));
    IM_CHECK(on_screen(h, "m58 → m68 4 phrases best"));
    IM_CHECK(on_screen(h, "m88 → m92 2 phrases best"));
    const std::string finished_score = figure_under(h, kYourScoreLabel);
    IM_CHECK_STR_EQ(finished_score.c_str(), reach_before_finish.c_str());
    IM_CHECK_STR_EQ(finished_score.c_str(), figure_under(h, kBestReachLabel).c_str());
    IM_CHECK_STR_EQ(finished_score.c_str(), "378,175");

    // Start over: back to no steps.
    ctx->ItemClick(any_ref(kStartOverLabel).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    IM_CHECK_RETV(wait_text(ctx, kNoStepsText), );
    IM_CHECK(on_screen(h, "Started over."));
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

// Gives the last action two frames, then reads `expected` on screen; the
// lines that hold a readout are logged when it is not there.
bool readout_reads(ImGuiTestContext* ctx, const std::string& expected) {
    Harness& h = harness(ctx);
    ctx->Yield(2);
    if (on_screen(h, expected)) return true;
    ctx->LogError("readout lines: %s", uitest::lines_holding(visible_text(h), kLongSongOf).c_str());
    return false;
}

// The long song past three minutes: one window forward, the card of a fill
// there, a click on it, and "< Act" from a window scrolled back to the start,
// which brings the window to the activation. Every figure is from one run.
void test_build_a_path_on_a_long_song(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_long_song(ctx), );
    const std::string forward = any_ref(hydra::ui::kScrollForwardLabel);
    const std::string back = any_ref(hydra::ui::kScrollBackLabel);
    IM_CHECK_RETV(readout_reads(ctx, "0:00 – 3:00 of 5:59"), );
    IM_CHECK_STR_EQ(figure_under(h, kYourScoreLabel).c_str(), "729,960");
    // The fill is past the window, so it has no item yet.
    IM_CHECK(ctx->ItemInfo(fill_ref(kLongSongFillTick).c_str(), ImGuiTestOpFlags_NoError).ID == 0);

    // One window forward: the window stops at the song's end.
    ctx->ItemClick(forward.c_str());
    IM_CHECK_RETV(readout_reads(ctx, "2:59 – 5:59 of 5:59"), );

    // The hover on the fill: its card, headed by its measure and clock.
    const std::string best_line = "Best choice now is m15 (1,150,660).";
    ctx->MouseMove(fill_ref(kLongSongFillTick).c_str());
    IM_CHECK_RETV(wait_text(ctx, best_line), );
    IM_CHECK(on_screen(h, "m69 · 3:15.859"));
    IM_CHECK(on_screen(h, "Phrases saved | 4"));
    IM_CHECK(on_screen(h, "Best finish here | 944,680"));
    IM_CHECK(on_screen(h, "Cost | −205,980"));
    IM_CHECK(on_screen(h, "Hydra would end it at m90."));

    // The click on it: the step's compact row and the scores.
    ctx->ItemClick(fill_ref(kLongSongFillTick).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    IM_CHECK_RETV(wait_text(ctx, "m69 → m90 4 phrases −205,980"), );
    IM_CHECK_STR_EQ(figure_under(h, kYourScoreLabel).c_str(), "879,860");
    IM_CHECK_STR_EQ(figure_under(h, kBestReachLabel).c_str(), "944,680");

    // The click left the clock on the fill's dot, the card heading's moment;
    // the forward jump carries it past the activation, still inside the
    // window, so the window stays.
    IM_CHECK(on_screen(h, "3:15.859" + std::string(kLongSongClockTail)));
    ctx->ItemClick(jump_forward_ref().c_str());
    IM_CHECK(readout_reads(ctx, "2:59 – 5:59 of 5:59"));
    IM_CHECK(on_screen(h, "3:20.859" + std::string(kLongSongClockTail)));
    // Back to the song's start: the clock stays where it was, outside the
    // window.
    ctx->ItemClick(back.c_str());
    IM_CHECK(readout_reads(ctx, "0:00 – 3:00 of 5:59"));
    IM_CHECK(on_screen(h, "3:20.859" + std::string(kLongSongClockTail)));
    // "< Act" jumps the clock back to the activation, outside the window,
    // and the window comes to it.
    ctx->ItemClick(kPrevActRef);
    IM_CHECK(readout_reads(ctx, "2:30 – 5:30 of 5:59"));
    IM_CHECK(on_screen(h, "3:15.859" + std::string(kLongSongClockTail)));
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
    const size_t of = text.find(kLongSongOf);
    const size_t at_dash = of == std::string::npos ? std::string::npos : text.rfind(dash, of);
    if (at_dash != std::string::npos) {
        const std::string start = word_before(text, at_dash);
        const std::string end = text.substr(at_dash + dash.size(), of - at_dash - dash.size());
        shown.readout = start + dash + end + kLongSongOf;
        shown.start_s = static_cast<int>(clock_text_ms(start) / 1000.0);
        shown.end_s = static_cast<int>(clock_text_ms(end) / 1000.0);
    }
    const size_t tail = text.find(kLongSongClockTail);
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
// only the last window, held at the song's end, is pinned.
void test_follow_the_playhead(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(open_long_song(ctx), );
    const std::string opening = "0:00 – 3:00 of 5:59";
    IM_CHECK_RETV(readout_reads(ctx, opening), );

    ctx->ItemClick(any_ref(kPlayWord).c_str());
    ctx->Yield(2);
    IM_CHECK_RETV(play_button_reads(ctx, kPauseWord), );

    // The first turn comes before the playhead crosses one whole window.
    const bool turned = press_jump_until(
        ctx, presses_across(hydra::ui::kTimelineWindowMs),
        [&] { return read_long_song(h).readout != opening; }, "the window never turned");
    IM_CHECK_RETV(turned, );
    const LongSongShown first = read_long_song(h);
    IM_CHECK_RETV(first.start_s >= 0 && first.clock_ms >= 0.0, );
    // The turn put the playhead a quarter in, from a playhead past three
    // quarters of the window and no later than the clock now reads. The clock
    // is real time, so those two bound the start; neither is a clock text.
    const hydra::ui::TimelineView open = hydra::ui::open_view(kLongSongMs);
    const double earliest_ms =
        (hydra::ui::kFollowTurnShare - hydra::ui::kPlayheadLeadShare) * open.span_ms;
    const double latest_ms =
        hydra::ui::follow_playing(open, first.clock_ms, kLongSongMs).start_ms;
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
        static_cast<int>(hydra::ui::wheel(open, 1.0, kLongSongMs).start_ms / 1000.0);
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
    // window comes to the playhead, held at the song's end.
    ctx->ItemClick(any_ref(kPauseWord).c_str());
    ctx->Yield(2);
    IM_CHECK(play_button_reads(ctx, kPlayWord));
    IM_CHECK_STR_EQ(read_long_song(h).readout.c_str(), wheeled.readout.c_str());
    ctx->ItemClick(any_ref(kPlayWord).c_str());
    IM_CHECK(readout_reads(ctx, "2:59 – 5:59 of 5:59"));
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
