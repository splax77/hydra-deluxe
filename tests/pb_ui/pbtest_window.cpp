// The Path Builder's main window, driven headless: a chart opened from the
// command line, a chart with no Star Power, and a chart that fails to open.

// First: its Windows headers come before the test engine's, which undefine
// their Yield() macro (uitest_engine.h says why).
#include "pbtest_harness.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

#include "imgui_te_context.h"

#include "../chart_text.h"
#include "../testdata_dirs.h"

#include "app/display_format.h"  // kFactSeparator
#include "app/preview_view.h"
#include "app/user_messages.h"  // kSettingsRefusedSentence
#include "core/winstr.h"
#include "pathbuilder/pb_menu.h"  // kStartOverLabel
#include "pathbuilder/pb_text.h"
#include "pathbuilder/pb_window.h"  // the hover's words
#include "render/highway_draw.h"      // far_time, time_to_z
#include "render/overlay_layout.h"    // project_to_image
#include "render/preview_renderer.h"  // kPlaybackSpeed
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
// three scores are on screen, the intro, the coach line and the old decision
// list are not, and "Show Hydra's best path" sits in the head row. The fill
// keys and the saved row's words show only while their label is hovered. A
// fill under two saved phrases is drawn as can't. Then a click on the highway
// at the earliest fill the player can take activates there.
void test_open_starts_empty(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = corpus::burnout_notes();
    reset_app(h);
    // Before the costs land, every fill the engine lists has a row whose end
    // and cost read kPendingText (plan Q3); once nothing is running, no
    // figure on screen does. From one run: the first fill's row, which the
    // first frame with the highway loaded shows. The wait is wait_idle's.
    const std::string pending_row = std::string("m20 0:25.123 · 2 phrases · ") +
                                    hydra::pathbuilder::kPendingText + " " +
                                    hydra::pathbuilder::kPendingText;
    IM_CHECK(uitest::wait_until(ctx, [&] { return on_screen(h, pending_row); }, 60.0));
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    IM_CHECK(!on_screen(h, hydra::pathbuilder::kPendingText));
    IM_CHECK_RETV(h.app->state.chart() != nullptr, );
    IM_CHECK(!h.app->state.history().current().path.has_value());
    IM_CHECK(h.app->shown_path() == &*h.app->state.chart()->empty_path);
    // The title line is the song.ini's name and artist, not the file path.
    // From one run.
    IM_CHECK(on_screen(h, "Burnout — Green Day"));
    IM_CHECK(!on_screen(h, h.chart_arg));
    IM_CHECK(!on_screen(h, "Build your own"));
    // The Preview panel's clock at the song's start, from one run.
    IM_CHECK(on_screen(h, "0:00.000"));
    IM_CHECK(!on_screen(h, "Step 1."));
    // Burnout is shorter than the window, so the window is the whole song
    // (plan 3's Q1): the scrollbar's readout, from one run. The scrollbar's
    // name is on screen.
    IM_CHECK(on_screen(h, "0:00 – 2:10 of 2:10"));
    IM_CHECK(on_screen(h, hydra::ui::kTimelineScrollLabel));
    // The list; no cost line under the fills, and no highway of the
    // builder's own.
    IM_CHECK(on_screen(h, hydra::pathbuilder::kChoicesHeading));
    IM_CHECK(!on_screen(h, "and their cost"));
    IM_CHECK(!on_screen(h, "Highway"));
    ctx->SetRef(h.home_ref.c_str());

    // "Show Hydra's best path" sits on the head row, beside the buttons.
    IM_CHECK(on_screen(h, hydra::pathbuilder::kShowBestPathLabel));
    const float best_box_y =
        ctx->ItemInfo(any_ref(hydra::pathbuilder::kShowBestPathLabel).c_str()).RectFull.Min.y;
    const float finish_y =
        ctx->ItemInfo(any_ref(hydra::pathbuilder::kFinishForMeLabel).c_str()).RectFull.Min.y;
    IM_CHECK_LE(std::fabs(best_box_y - finish_y), 1.0f);

    // Each label's words show only while the pointer is on that label. "Your
    // activation" is read with its line's end, because the row's own label
    // starts with the same words.
    const std::string mine_key = std::string(hydra::pathbuilder::kLegendMineText) + IM_NEWLINE;
    const std::vector<const char*> fill_keys = {
        hydra::pathbuilder::kLegendBestText,      hydra::pathbuilder::kLegendUnderOnePercentText,
        hydra::pathbuilder::kLegendCostsMoreText, mine_key.c_str(),
        hydra::pathbuilder::kLegendCantText,      hydra::pathbuilder::kLegendLockedText};
    const std::vector<const char*> saved_words = {hydra::ui::kTimelinePhrasesLabel,
                                                  "2 phrases: needed to activate", "enough to activate"};
    const auto all_on_screen = [&](const std::vector<const char*>& words) {
        return std::all_of(words.begin(), words.end(), [&](const char* w) { return on_screen(h, w); });
    };
    const auto none_on_screen = [&](const std::vector<const char*>& words) {
        return std::none_of(words.begin(), words.end(), [&](const char* w) { return on_screen(h, w); });
    };
    move_off_timeline(ctx);
    IM_CHECK(none_on_screen(fill_keys));
    IM_CHECK(none_on_screen(saved_words));
    IM_CHECK(!on_screen(h, "needed to activate"));

    ctx->MouseMove(any_ref("##label_activations").c_str());
    IM_CHECK(uitest::wait_until(ctx, [&] { return all_on_screen(fill_keys); }, 5.0));
    IM_CHECK(none_on_screen(saved_words));

    ctx->MouseMove(any_ref("##label_saved").c_str());
    IM_CHECK(uitest::wait_until(ctx, [&] { return all_on_screen(saved_words); }, 5.0));
    IM_CHECK(none_on_screen(fill_keys));

    move_off_timeline(ctx);
    IM_CHECK(none_on_screen(saved_words));
    IM_CHECK(!on_screen(h, "needed to activate"));

    // The Preview's "< Act" is on screen and greyed: the empty path has no
    // activation to jump to.
    const ImGuiTestItemInfo prev_act = ctx->ItemInfo("**/< Act##prevact", ImGuiTestOpFlags_NoError);
    IM_CHECK(prev_act.ID != 0);
    IM_CHECK((prev_act.ItemFlags & ImGuiItemFlags_Disabled) != 0);
    for (const char* label : {hydra::pathbuilder::kYourScoreLabel,
                              hydra::pathbuilder::kBestPossibleLabel,
                              hydra::pathbuilder::kBestReachLabel})
        IM_CHECK(on_screen(h, label));
    IM_CHECK(!on_screen(h, "Your decisions"));
    IM_CHECK(on_screen(h, hydra::pathbuilder::kNoStepsText));

    // From one run: Burnout's second fill comes with one phrase saved, so it
    // does not glow.
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

    // A click on the highway at the earliest fill the player can take, from
    // one run the one at tick 36480 (m20), activates there, as a click on it
    // in the timeline does. The highway is moved so the fill starts on the
    // strike line, and the click lands on the floor halfway up the part of
    // the fill the highway shows (project_to_image, the renderer's camera).
    static constexpr int64_t kEarliestTick = 36480;
    hydra::ui::PreviewController& pc = *h.app->preview();
    const auto earliest =
        std::find_if(scene.fills.begin(), scene.fills.end(),
                     [](const hydra::app::PreviewFill& f) { return f.chord_tick == kEarliestTick; });
    IM_CHECK_RETV(earliest != scene.fills.end(), );
    pc.seek_ms(earliest->span.start_ms);
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    const ImRect img = ctx->ItemInfo("**/##highway_image").RectFull;
    IM_CHECK_RETV(img.GetWidth() > 0.0f && img.GetHeight() > 0.0f, );
    const hydra::render::PreviewConfig& cfg = pc.preview_config();
    const double now_s = pc.position_ms() / 1000.0;
    const double shown_end_s = (std::min)(earliest->span.end_ms / 1000.0,
                                          hydra::render::far_time(cfg, now_s, hydra::render::kPlaybackSpeed));
    const double at_s = (now_s + shown_end_s) * 0.5;
    const hydra::render::ImagePoint p = hydra::render::project_to_image(
        cfg, static_cast<int>(img.GetWidth()), static_cast<int>(img.GetHeight()),
        {0.0f, cfg.track.y,
         static_cast<float>(hydra::render::time_to_z(cfg, now_s, at_s, hydra::render::kPlaybackSpeed))});
    ctx->MouseMoveToPos(ImVec2(img.Min.x + p.x, img.Min.y + p.y));
    ctx->MouseClick(ImGuiMouseButton_Left);
    IM_CHECK_RETV(wait_for_edit(ctx), );
    // From one run: the step's compact row, and the clock on the fill's dot,
    // where every click on a fill leaves it.
    IM_CHECK(on_screen(h, "m20 → m24 2 phrases −140"));
    IM_CHECK(on_screen(h, "0:25.123 / 2:10.088"));
}

// The line of visible text that holds the scrollbar's readout for a song
// `length` long (the readout ends " of <length>"), or "" when none does.
std::string readout_line(Harness& h, const std::string& length) {
    const std::string text = uitest::visible_text(h);
    const size_t at = text.find(" of " + length);
    if (at == std::string::npos) return {};
    const size_t start = text.rfind('\n', at);
    const size_t end = text.find('\n', at);
    const size_t from = start == std::string::npos ? 0 : start + 1;
    return text.substr(from, end == std::string::npos ? std::string::npos : end - from);
}

// Lays the frame out `width` wide for as long as it lives, as a real window
// that size does: the builder reads its width from the main viewport, which
// Engine::frame sizes from the harness's width. The render target keeps its
// size, so this is no test of what the frame draws.
class FrameWidth {
public:
    FrameWidth(Harness& h, int width) : h_(h), restore_(h.width) { h_.width = width; }
    ~FrameWidth() { h_.width = restore_; }
    FrameWidth(const FrameWidth&) = delete;
    FrameWidth& operator=(const FrameWidth&) = delete;

private:
    Harness& h_;
    int restore_;
};

// The head row at the test's normal width, then on a window where the last two
// items wrap, then on one too narrow for any of them beside the scores.
void test_head_row_wraps(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = corpus::burnout_notes();
    reset_app(h);
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    const std::string undo = any_ref(hydra::pathbuilder::kUndoLabel);
    const std::string start_over = any_ref(hydra::pathbuilder::kStartOverLabel);
    const std::string finish = any_ref(hydra::pathbuilder::kFinishForMeLabel);
    const std::string best = any_ref(hydra::pathbuilder::kShowBestPathLabel);
    const std::string below = any_ref("##label_activations");
    const auto rect = [&](const std::string& ref) { return ctx->ItemInfo(ref.c_str()).RectFull; };

    // Everything fits: each place is the one a row that never wraps gives,
    // from one run of that row.
    constexpr float kRowY = 32.0f;
    constexpr float kUndoX = 588.6f;
    constexpr float kStartOverX = 638.6f;
    constexpr float kBelowY = 194.8f;
    const auto check_normal = [&] {
        IM_CHECK_FLOAT_NEAR_EQ(rect(undo).Min.x, kUndoX, 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(start_over).Min.x, kStartOverX, 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(finish).Min.x, 714.6f, 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(best).Min.x, 864.0f, 0.5f);
        for (const std::string* item : {&undo, &start_over, &finish, &best})
            IM_CHECK_FLOAT_NEAR_EQ(rect(*item).Min.y, kRowY, 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(below).Min.y, kBelowY, 0.5f);
    };
    check_normal();
    const float best_gap = rect(best).Min.x - rect(finish).Max.x;

    // Every item ends inside the window, and what lies under the row starts
    // under the lowest of them.
    const auto check_inside = [&](int width) {
        const ImGuiWindow* window = ctx->GetWindowByRef(h.home_ref.c_str());
        IM_CHECK_RETV(window != nullptr, );
        IM_CHECK_EQ(window->Size.x, static_cast<float>(width));
        float lowest = 0.0f;
        for (const std::string* item : {&undo, &start_over, &finish, &best}) {
            IM_CHECK_LE(rect(*item).Max.x, window->Pos.x + window->Size.x);
            lowest = (std::max)(lowest, rect(*item).Max.y);
        }
        IM_CHECK_GE(rect(below).Min.y, lowest);
        IM_CHECK_GT(rect(below).Min.y, kBelowY);
    };
    const auto left_edge = [&] { return ctx->GetWindowByRef(h.home_ref.c_str())->WorkRect.Min.x; };

    // Undo and Start over keep their places. Finish starts the next line at
    // the row's left edge, and the checkbox stays beside it at its own gap.
    {
        constexpr int kWidth = 800;
        FrameWidth narrow(h, kWidth);
        ctx->Yield(3);
        check_inside(kWidth);
        IM_CHECK_FLOAT_NEAR_EQ(rect(undo).Min.x, kUndoX, 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(start_over).Min.x, kStartOverX, 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(start_over).Min.y, kRowY, 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(finish).Min.x, left_edge(), 0.5f);
        IM_CHECK_GT(rect(finish).Min.y, kRowY);
        IM_CHECK_FLOAT_NEAR_EQ(rect(best).Min.y, rect(finish).Min.y, 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(best).Min.x - rect(finish).Max.x, best_gap, 0.5f);
    }

    // The scores alone reach past the edge, as a long title's row does, so
    // Undo wraps too. The three buttons share a line and the checkbox takes
    // the one under it.
    {
        constexpr int kWidth = 400;
        FrameWidth narrow(h, kWidth);
        ctx->Yield(3);
        check_inside(kWidth);
        IM_CHECK_FLOAT_NEAR_EQ(rect(undo).Min.x, left_edge(), 0.5f);
        IM_CHECK_GT(rect(undo).Min.y, kRowY);
        IM_CHECK_FLOAT_NEAR_EQ(rect(finish).Min.y, rect(undo).Min.y, 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(best).Min.x, left_edge(), 0.5f);
        IM_CHECK_GT(rect(best).Min.y, rect(finish).Min.y);
    }

    // Widening the window brings the row back.
    ctx->Yield(3);
    check_normal();
}

// A point `share` of the way across the item `ref`, at its middle height.
ImVec2 point_across(ImGuiTestContext* ctx, const std::string& ref, float share) {
    const ImRect r = ctx->ItemInfo(ref.c_str()).RectFull;
    return ImVec2(r.Min.x + r.GetWidth() * share, r.GetCenter().y);
}

// Presses the left button at `from`, moves to `to` and lets go.
void drag(ImGuiTestContext* ctx, ImVec2 from, ImVec2 to) {
    ctx->MouseMoveToPos(from);
    ctx->MouseDown(ImGuiMouseButton_Left);
    ctx->MouseMoveToPos(to);
    ctx->MouseUp(ImGuiMouseButton_Left);
    ctx->Yield(2);
}

// The timeline scrolled by hand on a song longer than its window, nothing
// playing: the scrollbar's two buttons, a wheel notch over the rows, a drag
// of the scrollbar's thumb, and the click of a list row whose fill is off
// screen, whose hover scrolls nothing. Each step reads the scrollbar's
// readout, from one run.
void test_scroll_the_timeline(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = long_song_notes();
    reset_app(h);
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    // The Preview's end is app::song_length_ms: the chart's stated 5:58.957
    // plus the end pad, so the readouts end "of 5:59" (from one run).
    const std::string length = "5:59";
    // Gives the last action two frames, then reads the readout; the whole
    // line it sits in is logged when it is not the one expected.
    auto reads = [&](const char* expected) {
        ctx->Yield(2);
        const bool shown = on_screen(h, expected);
        if (!shown) ctx->LogError("readout line: [%s]", readout_line(h, length).c_str());
        return shown;
    };
    IM_CHECK(reads("0:00 – 3:00 of 5:59"));
    IM_CHECK(on_screen(h, hydra::ui::kTimelineScrollLabel));

    const std::string forward = any_ref(hydra::ui::kScrollForwardLabel);
    const std::string back = any_ref(hydra::ui::kScrollBackLabel);
    const std::string bar = any_ref(hydra::ui::kTimelineScrollbarId);
    ctx->SetRef(h.home_ref.c_str());

    // The buttons: one window forward stops at the song's end, then one back.
    ctx->ItemClick(forward.c_str());
    IM_CHECK(reads("2:59 – 5:59 of 5:59"));
    ctx->ItemClick(back.c_str());
    IM_CHECK(reads("0:00 – 3:00 of 5:59"));

    // A wheel notch down over the rows, away from the Fills row so no fill
    // is hovered: a third of the window later.
    const ImRect rows = ctx->ItemInfo(any_ref("##timeline").c_str()).RectFull;
    ctx->MouseMoveToPos(ImVec2(rows.GetCenter().x, rows.Min.y + rows.GetHeight() * 0.4f));
    ctx->MouseWheelY(-1.0f);
    IM_CHECK(reads("1:00 – 4:00 of 5:59"));

    // A drag of the scrollbar's thumb, a tenth of the bar to the right, moves
    // the window later by about a tenth of the song.
    ctx->ItemClick(back.c_str());
    IM_CHECK(reads("0:00 – 3:00 of 5:59"));
    drag(ctx, point_across(ctx, bar, 0.25f), point_across(ctx, bar, 0.35f));
    IM_CHECK(reads("0:36 – 3:36 of 5:59"));

    // Hovering the list row of a fill past the window (its chord at 3:58,
    // from one run) moves the window nowhere. A click on the row centres the
    // window on that fill, whose own item is then on the timeline; before the
    // click it has none.
    static constexpr int64_t kLateFillTick = 162240;
    ctx->ItemClick(back.c_str());
    IM_CHECK(reads("0:00 – 3:00 of 5:59"));
    IM_CHECK(ctx->ItemInfo(fill_ref(kLateFillTick).c_str(), ImGuiTestOpFlags_NoError).ID == 0);
    ctx->MouseMove(choice_ref(kLateFillTick).c_str());
    IM_CHECK(reads("0:00 – 3:00 of 5:59"));
    IM_CHECK(ctx->ItemInfo(fill_ref(kLateFillTick).c_str(), ImGuiTestOpFlags_NoError).ID == 0);
    ctx->ItemClick(choice_ref(kLateFillTick).c_str());
    IM_CHECK(wait_for_edit(ctx));
    IM_CHECK(reads("2:28 – 5:28 of 5:59"));
    IM_CHECK(ctx->ItemInfo(fill_ref(kLateFillTick).c_str(), ImGuiTestOpFlags_NoError).ID != 0);
    // The window stays there when the pointer leaves the row.
    move_off_timeline(ctx);
    IM_CHECK(reads("2:28 – 5:28 of 5:59"));
}

// A press and drag on a song longer than its window: the clock stays inside
// the window however far past the track the pointer goes, and neither the
// drag, nor a wheel notch during it, nor the release moves the window. Each
// clock and readout is from one run.
void test_drag_stays_in_the_window(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = long_song_notes();
    reset_app(h);
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    move_off_timeline(ctx);
    const std::string window = "0:00 – 3:00 of 5:59";
    IM_CHECK_RETV(on_screen(h, window), );

    const ImRect rows = ctx->ItemInfo(any_ref("##timeline").c_str()).RectFull;
    const float saved_y = rows.Min.y + rows.GetHeight() * 0.4f;
    ctx->MouseMoveToPos(ImVec2(rows.GetCenter().x, saved_y));
    ctx->MouseDown(ImGuiMouseButton_Left);
    ctx->MouseMoveToPos(ImVec2(rows.Max.x + 40.0f, saved_y));
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "3:00.000 / 5:59.957"));
    IM_CHECK(on_screen(h, window));
    ctx->MouseWheelY(-1.0f);
    ctx->Yield(2);
    IM_CHECK(on_screen(h, window));
    ctx->MouseMoveToPos(ImVec2(rows.Min.x - 40.0f, rows.Max.y + 40.0f));
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "0:00.000 / 5:59.957"));
    IM_CHECK(on_screen(h, window));
    ctx->MouseMoveToPos(ImVec2(rows.GetCenter().x, saved_y));
    ctx->Yield(2);
    ctx->MouseUp(ImGuiMouseButton_Left);
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "1:29.913 / 5:59.957"));
    IM_CHECK(on_screen(h, window));
    move_off_timeline(ctx);
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

// A bad settings file refuses the song on the top line (D304 item 8). With no
// song open the window stays up with none, and a good file opens it and takes
// the line away. With a song open, the song and the player's path stay. The
// open goes through PbApp::open, the one door the command line, a dropped file
// and File > Open share.
void test_bad_settings_refused(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg.clear();
    reset_app(h);
    const std::vector<uint8_t> good_bytes = hydra::read_file_bytes(h.ini_path);
    const std::string good_settings(good_bytes.begin(), good_bytes.end());
    // The volume line sits before the bad one, so a read that stopped part way
    // would have it.
    const std::string bad_settings = "preview_volume=7\nsp_cap=banana\n";
    const std::string detail = h.ini_path + ": sp_cap = \"banana\" is not a whole number";
    IM_CHECK_RETV(write_file(h.ini_path, bad_settings), );
    h.app->open(corpus::burnout_notes());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK(h.app->state.chart() == nullptr);
    IM_CHECK(!h.app->state.opening());
    IM_CHECK(!h.app->state.preview_volume().has_value());
    IM_CHECK(on_screen(h, hydra::app::kSettingsRefusedSentence));
    IM_CHECK(on_screen(h, detail));

    IM_CHECK_RETV(write_file(h.ini_path, good_settings), );
    h.app->open(corpus::burnout_notes());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK_RETV(h.app->state.chart() != nullptr, );
    IM_CHECK(!on_screen(h, hydra::app::kSettingsRefusedSentence));
    IM_CHECK(!on_screen(h, detail));
    IM_CHECK(h.app->refusal().empty());

    // A song open with a window on the player's path; then the file goes bad.
    IM_CHECK_RETV(click_fill(ctx, kFirstFillTick), );
    const hydra::pathbuilder::OpenChart* open_song = h.app->state.chart();
    const uint64_t generation = h.app->state.history().generation();
    IM_CHECK_RETV(h.app->state.history().current().windows.size() == 1, );
    IM_CHECK_RETV(write_file(h.ini_path, bad_settings), );
    h.app->open(long_song_notes());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK(h.app->state.chart() == open_song);
    IM_CHECK(h.app->state.history().generation() == generation);
    IM_CHECK(h.app->state.history().current().windows.size() == 1);
    IM_CHECK(h.app->state.requested_path() != long_song_notes());
    IM_CHECK(on_screen(h, "Burnout — Green Day"));
    IM_CHECK(on_screen(h, hydra::app::kSettingsRefusedSentence));
    IM_CHECK(on_screen(h, detail));
}

}  // namespace

void register_tests(Harness& h) {
    uitest::register_test(h, "open-starts-empty", test_open_starts_empty);
    uitest::register_test(h, "head-row-wraps", test_head_row_wraps);
    uitest::register_test(h, "scroll-the-timeline", test_scroll_the_timeline);
    uitest::register_test(h, "drag-stays-in-the-window", test_drag_stays_in_the_window);
    uitest::register_test(h, "no-star-power", test_no_star_power);
    uitest::register_test(h, "file-open-error", test_file_open_error);
    uitest::register_test(h, "bad-settings-refused", test_bad_settings_refused);
}

}  // namespace pbtest
