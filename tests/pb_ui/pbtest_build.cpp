// The Path Builder's build-a-whole-path test: one GUI test that builds a path
// on Burnout by clicking the screen's own widgets (the timeline's fills, the
// "Where to activate next" rows, the step rows and the buttons), and checks
// each step only in the text the screen drew. Task PB-6 of
// docs/superpowers/plans/2026-10-30-path-builder-redesign.md, updated by TS-4
// of docs/superpowers/plans/2026-10-31-path-builder-redesign-2.md. Every
// tick, sentence, clock and figure is pinned from one run under the GUI tests'
// settings (tests/scratch_settings.h).

// First: its Windows headers come before the test engine's, which undefine
// their Yield() macro (uitest_engine.h says why).
#include "pbtest_harness.h"

#include <cstdint>
#include <string>

#include "imgui_te_context.h"

#include "../testdata_dirs.h"

#include "pathbuilder/pb_coach.h"   // kFinishForMeLabel
#include "pathbuilder/pb_menu.h"    // kStartOverLabel
#include "pathbuilder/pb_text.h"    // kNoStepsText, kBestFillLabel, locked_message
#include "pathbuilder/pb_window.h"  // the score and button labels
#include "ui/preview_timeline.h"    // timeline_fill_id, kTimelineFillsLabel

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

// The Preview's "Act >" button (ui::draw_preview_panel).
constexpr const char* kNextActRef = "**/Act >##nextact";

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
    IM_CHECK(on_screen(h, "m32 0:40.945 · 3 phrases · ends m38 · squeeze-out 163.0 ms best"));

    // The hover on the first fill: its tooltip gives the best finish with
    // this fill and the best finish now, and the cost between them.
    ctx->MouseMove(fill_ref(kFirstFillTick).c_str());
    IM_CHECK_RETV(wait_text(ctx, "If you activate here"), );
    IM_CHECK(on_screen(h, "Fill at m20 · 0:25.123"));
    IM_CHECK(on_screen(h, "If you activate here, the best you can still finish with is 378,175."));
    IM_CHECK(on_screen(h, "Your best choice right now is m32, which keeps 378,315."));
    IM_CHECK(on_screen(h, "So activating here costs 140 points."));
    move_off_timeline(ctx);
    IM_CHECK(!on_screen(h, "If you activate here"));

    // The click on it: the steps panel gains its compact row, open to its
    // sentence because the click selected the step it made, and the score
    // line moves.
    const std::string step_one_row = "m20 → m24 2 phrases −140";
    const std::string step_one =
        "Activate at m20 (0:25.123) with 2 phrases saved. Star Power runs until m24. This choice "
        "costs 140 points.";
    ctx->ItemClick(fill_ref(kFirstFillTick).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    IM_CHECK_RETV(wait_text(ctx, step_one_row), );
    IM_CHECK(on_screen(h, step_one));
    IM_CHECK(!on_screen(h, kNoStepsText));
    IM_CHECK(!on_screen(h, "m40 →"));
    const std::string one_step_score = figure_under(h, kYourScoreLabel);
    IM_CHECK(one_step_score != empty_score);
    IM_CHECK_STR_EQ(one_step_score.c_str(), "327,135");
    IM_CHECK_STR_EQ(figure_under(h, kBestReachLabel).c_str(), "378,175");

    // The list names the second fill. Hovering its row moves the Preview's
    // clock to the fill's span start; moving off puts it back. From one run:
    // the click on the first fill left the clock at that fill's span start.
    const std::string clock_after_one = "0:23.840 / 2:10.088";
    IM_CHECK(on_screen(h, "m40 0:51.421 · 2 phrases · ends m44 −1,580"));
    IM_CHECK(on_screen(h, clock_after_one));
    ctx->MouseMove(choice_ref(kSecondFillTick).c_str());
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "0:50.095 / 2:10.088"));
    IM_CHECK(!on_screen(h, clock_after_one));
    move_off_timeline(ctx);
    IM_CHECK(on_screen(h, clock_after_one));

    // The click on that row makes the second activation: its compact row,
    // open to its sentence, while the first step shrinks to its compact row.
    const std::string step_two_row = "m40 → m44 2 phrases −1,580";
    const std::string step_two =
        "Activate at m40 (0:51.421) with 2 phrases saved. Star Power runs until m44. This choice "
        "costs 1,580 points.";
    ctx->ItemClick(choice_ref(kSecondFillTick).c_str());
    IM_CHECK_RETV(wait_for_edit(ctx), );
    move_off_timeline(ctx);
    IM_CHECK_RETV(wait_text(ctx, step_two_row), );
    IM_CHECK(on_screen(h, step_two));
    IM_CHECK(on_screen(h, step_one_row));
    IM_CHECK(!on_screen(h, step_one));
    const std::string two_step_score = figure_under(h, kYourScoreLabel);
    IM_CHECK(two_step_score != one_step_score);
    IM_CHECK_STR_EQ(two_step_score.c_str(), "340,675");
    IM_CHECK_STR_EQ(figure_under(h, kBestReachLabel).c_str(), "376,595");

    // "Act >" jumps the Preview from where it stands, before the first
    // activation, to the first activation's clock (m20, 0:25.123).
    IM_CHECK(on_screen(h, clock_after_one));
    ctx->ItemClick(kNextActRef);
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "0:25.123 / 2:10.088"));

    // A click on the first step's row opens it to its sentence and closes the
    // second.
    IM_CHECK_RETV(click_step(ctx, 0), );
    ctx->SetRef(h.home_ref.c_str());
    IM_CHECK(on_screen(h, step_one));
    IM_CHECK(!on_screen(h, step_two));
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

}  // namespace

void register_build_tests(Harness& h) {
    uitest::register_test(h, "build-a-path-by-clicking", test_build_a_path_by_clicking);
}

}  // namespace pbtest
