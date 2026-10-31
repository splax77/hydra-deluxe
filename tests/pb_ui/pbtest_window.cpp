// The Path Builder's main window, driven headless: a chart opened from the
// command line, a chart with no Star Power, and a chart that fails to open.

// First: its Windows headers come before the test engine's, which undefine
// their Yield() macro (uitest_engine.h says why).
#include "pbtest_harness.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>

#include "imgui_te_context.h"

#include "../chart_text.h"
#include "../testdata_dirs.h"

#include "app/display_format.h"  // kFactSeparator
#include "app/preview_view.h"
#include "core/winstr.h"
#include "pathbuilder/pb_text.h"
#include "ui/preview_controller.h"

namespace pbtest {

namespace {

// Writes `text` to `path`, replacing the file. False when it could not.
bool write_file(const std::string& path, const std::string& text) {
    std::FILE* f = hydra::fopen_utf8(path, L"wb");
    if (!f) return false;
    std::fputs(text.c_str(), f);
    std::fclose(f);
    return true;
}

// Opens the chart from the command line: the screen starts from nothing. The
// intro, step 1 of the guide and the three scores are on screen, the old
// decision list is not, and a fill under two saved phrases is drawn as can't
// while the guide names a later fill as the earliest.
void test_open_starts_empty(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = corpus::burnout_notes();
    reset_app(h);
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    IM_CHECK_RETV(h.app->state.chart() != nullptr, );
    IM_CHECK(!h.app->state.history().current().path.has_value());
    IM_CHECK(h.app->shown_path() == &*h.app->state.chart()->empty_path);
    // The title line is the song.ini's name and artist, not the file path.
    // From one run.
    IM_CHECK(on_screen(h, "Burnout — Green Day"));
    IM_CHECK(!on_screen(h, h.chart_arg));
    // The intro is drawn with the star inside it, so its two sides are on
    // screen and the joined sentence is not.
    const std::string intro = hydra::pathbuilder::intro_text();
    const size_t star = hydra::pathbuilder::intro_star_at();
    IM_CHECK(on_screen(h, intro.substr(0, star) + hydra::pathbuilder::kStarOpen));
    IM_CHECK(on_screen(h, hydra::pathbuilder::kStarClose + intro.substr(star)));
    IM_CHECK(!on_screen(h, intro));
    // The highway's "where" line at the song's start: the measure, then the clock.
    IM_CHECK(on_screen(h, std::string("m1") + hydra::app::kFactSeparator + "0:00.000"));
    IM_CHECK(on_screen(h, "Step 1."));
    for (const char* label : {hydra::pathbuilder::kYourScoreLabel,
                              hydra::pathbuilder::kBestPossibleLabel,
                              hydra::pathbuilder::kBestReachLabel})
        IM_CHECK(on_screen(h, label));
    IM_CHECK(!on_screen(h, "Your decisions"));
    IM_CHECK(on_screen(h, hydra::pathbuilder::kNoStepsText));

    // From one run: Burnout's second fill comes with one phrase saved, so it
    // does not glow, and the guide names the fill at tick 36480 (m20) as the
    // earliest.
    static constexpr int64_t kUnderTwoTick = 21120;
    const hydra::app::PreviewScene& scene = h.app->preview()->scene();
    const auto under_two =
        std::find_if(scene.fills.begin(), scene.fills.end(),
                     [](const hydra::app::PreviewFill& f) { return f.chord_tick == kUnderTwoTick; });
    IM_CHECK_RETV(under_two != scene.fills.end(), );
    IM_CHECK(under_two->state != hydra::app::PreviewFillState::Offered);
    IM_CHECK_STR_EQ(
        hydra::pathbuilder::fill_reason(scene, static_cast<size_t>(under_two - scene.fills.begin()))
            .c_str(),
        "You have 1 phrase saved here. You need 2 to activate.");
    IM_CHECK_STR_EQ(h.app->guide().text.c_str(),
                    "Step 1. Click a glowing fill to make your first activation. The earliest you "
                    "can activate is m20 (0:25.123), once you've saved 2 phrases. Hover a fill to "
                    "see it on the highway.");
    IM_CHECK(on_screen(h, h.app->guide().text));
}

// A chart with notes and no Star Power phrase opens and says there is
// nothing to build.
void test_no_star_power(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    using namespace testchart;
    const std::string notes = h.temp_dir + "\\notes.chart";
    IM_CHECK(write_file(notes, chart_text(section("ExpertDrums", line(0, "N 1 0") +
                                                                     line(192, "N 2 0") +
                                                                     line(384, "N 1 0")))));
    h.chart_arg = notes;
    reset_app(h);
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    IM_CHECK(h.app->state.chart() != nullptr);
    IM_CHECK(on_screen(h, hydra::pathbuilder::kNothingToBuildText));
    // No song.ini, so no name: the title line is the file path.
    IM_CHECK(on_screen(h, notes));
}

// A chart that cannot be read shows the plain sentence for it and its details
// line, and opens nothing. The sentence is the builder's (D267): Hydra's
// without the Scan library advice, since the builder has no library.
void test_file_open_error(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = h.temp_dir + "\\missing\\notes.mid";
    reset_app(h);
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    IM_CHECK(h.app->state.chart() == nullptr);
    IM_CHECK(h.app->state.error() ==
             "Hydra couldn't open the song file. It may have been moved or deleted.");
    IM_CHECK(h.app->state.error_detail() == "cannot open file: " + h.chart_arg);
    IM_CHECK(on_screen(h, h.app->state.error()));
    IM_CHECK(on_screen(h, h.app->state.error_detail()));
}

}  // namespace

void register_tests(Harness& h) {
    uitest::register_test(h, "open-starts-empty", test_open_starts_empty);
    uitest::register_test(h, "no-star-power", test_no_star_power);
    uitest::register_test(h, "file-open-error", test_file_open_error);
}

}  // namespace pbtest
