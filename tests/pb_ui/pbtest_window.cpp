// The Path Builder's main window, driven headless: a chart opened from the
// command line, a chart with no Star Power, and a chart that fails to open.

// First: its Windows headers come before the test engine's, which undefine
// their Yield() macro (uitest_engine.h says why).
#include "pbtest_harness.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "imgui_te_context.h"

#include "../chart_text.h"
#include "../testdata_dirs.h"

#include "app/config.h"          // path_overrides
#include "app/display_format.h"  // kFactSeparator
#include "app/ini_errors.h"      // ini_bad_value_text, kWantWholeNumber
#include "app/path_costs.h"      // CandidateCost
#include "app/preview_view.h"
#include "app/user_messages.h"  // kSettingsRefusedSentence, kRulesRefusedSentence, plain_error
#include "core/error_kind.h"    // KindedError, cannot_open_file_text
#include "core/model.h"         // group_thousands
#include "core/rules.h"         // default_rules
#include "core/winstr.h"
#include "pathbuilder/pb_menu.h"  // kStartOverLabel
#include "pathbuilder/pb_text.h"
#include "pathbuilder/pb_window.h"  // the hover's words
#include "render/highway_draw.h"      // far_time, time_to_z
#include "render/overlay_layout.h"    // project_to_image
#include "render/preview_renderer.h"  // kPlaybackSpeed
#include "ui/fonts.h"  // px
#include "ui/preview_controller.h"
#include "ui/preview_timeline.h"  // timeline_fill_ms, timeline_ms_at
#include "ui/timeline_view.h"     // open_view, readout and the window's moves

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

// The harness's hydra_rules.ini (path_overrides), which no test leaves behind:
// the next test runs on the defaults even when this one stops at a failed check.
struct ScratchRules {
    std::string path = hydra::app::path_overrides().rules_path;
    ~ScratchRules() { remove(); }
    bool write(const std::string& text) const { return write_file(path, text); }
    void remove() const {
        std::error_code ignored;
        std::filesystem::remove(std::filesystem::u8path(path), ignored);
    }
};

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

    // Everything fits: the four items share one line, each to the right of
    // the one before. The places measured here are the row that never wraps,
    // which the narrow windows below are read against.
    const std::string* const items[] = {&undo, &start_over, &finish, &best};
    const float row_y = rect(undo).Min.y;
    for (size_t i = 1; i < 4; ++i) {
        IM_CHECK_FLOAT_NEAR_EQ(rect(*items[i]).Min.y, row_y, 0.5f);
        IM_CHECK_GT(rect(*items[i]).Min.x, rect(*items[i - 1]).Max.x);
    }
    float normal_x[4] = {};
    for (size_t i = 0; i < 4; ++i) normal_x[i] = rect(*items[i]).Min.x;
    const float normal_below_y = rect(below).Min.y;
    IM_CHECK_GE(normal_below_y, rect(undo).Max.y);
    const auto check_normal = [&] {
        for (size_t i = 0; i < 4; ++i) {
            IM_CHECK_FLOAT_NEAR_EQ(rect(*items[i]).Min.x, normal_x[i], 0.5f);
            IM_CHECK_FLOAT_NEAR_EQ(rect(*items[i]).Min.y, row_y, 0.5f);
        }
        IM_CHECK_FLOAT_NEAR_EQ(rect(below).Min.y, normal_below_y, 0.5f);
    };
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
        IM_CHECK_GT(rect(below).Min.y, normal_below_y);
    };
    const auto left_edge = [&] { return ctx->GetWindowByRef(h.home_ref.c_str())->WorkRect.Min.x; };

    // Undo and Start over keep their places. Finish starts the next line at
    // the row's left edge, and the checkbox stays beside it at its own gap.
    {
        constexpr int kWidth = 800;
        FrameWidth narrow(h, kWidth);
        ctx->Yield(3);
        check_inside(kWidth);
        IM_CHECK_FLOAT_NEAR_EQ(rect(undo).Min.x, normal_x[0], 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(start_over).Min.x, normal_x[1], 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(start_over).Min.y, row_y, 0.5f);
        IM_CHECK_FLOAT_NEAR_EQ(rect(finish).Min.x, left_edge(), 0.5f);
        IM_CHECK_GT(rect(finish).Min.y, row_y);
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
        IM_CHECK_GT(rect(undo).Min.y, row_y);
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
// readout, worked out by the window's own moves (ui/timeline_view.h) from the
// song's length as the Preview ends it (PreviewController::playback_end_ms).
void test_scroll_the_timeline(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = long_song_notes();
    reset_app(h);
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    hydra::ui::PreviewController* pc = h.app->preview();
    IM_CHECK_RETV(pc != nullptr, );
    const double len = pc->playback_end_ms();
    // The song is longer than the window, or nothing here can scroll.
    IM_CHECK_RETV(len > hydra::ui::kTimelineWindowMs, );
    const hydra::ui::TimelineView opening = hydra::ui::open_view(len);
    const std::string opening_text = hydra::ui::readout(opening, len);
    const std::string length = opening_text.substr(opening_text.rfind(" of ") + 4);
    // Gives the last action two frames, then reads the readout; the whole
    // line it sits in is logged when it is not the one expected.
    auto reads = [&](const std::string& expected) {
        ctx->Yield(2);
        const bool shown = on_screen(h, expected);
        if (!shown)
            ctx->LogError("readout line: [%s], expected [%s]", readout_line(h, length).c_str(),
                          expected.c_str());
        return shown;
    };
    IM_CHECK(reads(opening_text));
    IM_CHECK(on_screen(h, hydra::ui::kTimelineScrollLabel));

    const std::string forward = any_ref(hydra::ui::kScrollForwardLabel);
    const std::string back = any_ref(hydra::ui::kScrollBackLabel);
    const std::string bar = any_ref(hydra::ui::kTimelineScrollbarId);
    ctx->SetRef(h.home_ref.c_str());

    // The buttons: one window forward stops at the song's end, then one back.
    const hydra::ui::TimelineView paged = hydra::ui::page(opening, 1, len);
    IM_CHECK_LE(paged.start_ms + paged.span_ms, len);
    ctx->ItemClick(forward.c_str());
    IM_CHECK(reads(hydra::ui::readout(paged, len)));
    ctx->ItemClick(back.c_str());
    IM_CHECK(reads(hydra::ui::readout(hydra::ui::page(paged, -1, len), len)));

    // A wheel notch down over the rows, away from the Fills row so no fill
    // is hovered: one notch later in the song.
    const ImRect rows = ctx->ItemInfo(any_ref("##timeline").c_str()).RectFull;
    ctx->MouseMoveToPos(ImVec2(rows.GetCenter().x, rows.Min.y + rows.GetHeight() * 0.4f));
    ctx->MouseWheelY(-1.0f);
    IM_CHECK(reads(hydra::ui::readout(hydra::ui::wheel(opening, 1.0, len), len)));

    // A drag of the scrollbar's thumb a tenth of the bar to the right keeps
    // the moment under the pointer under it, so the window starts that share
    // of the song later. The share is of the bar inside its border
    // (preview_timeline.cpp's kBorderW, one px on each side, which no header
    // gives), so it lies between the drag over the whole bar and the drag
    // over the bar less that border.
    ctx->ItemClick(back.c_str());
    IM_CHECK(reads(opening_text));
    const float bar_w = ctx->ItemInfo(bar.c_str()).RectFull.GetWidth();
    IM_CHECK_RETV(bar_w > 2.0f * hydra::ui::px(1.0f), );
    const ImVec2 grab = point_across(ctx, bar, 0.25f);
    const ImVec2 to = point_across(ctx, bar, 0.35f);
    drag(ctx, grab, to);
    const double moved_px = static_cast<double>(to.x - grab.x);
    const auto dragged = [&](double inner_w) {
        return hydra::ui::readout(
            hydra::ui::clamp({moved_px / inner_w * len, opening.span_ms}, len), len);
    };
    const std::string least = dragged(bar_w);
    const std::string most = dragged(bar_w - 2.0f * hydra::ui::px(1.0f));
    ctx->Yield(2);
    const bool drag_shown = on_screen(h, least) || on_screen(h, most);
    if (!drag_shown)
        ctx->LogError("readout line: [%s], expected [%s] or [%s]", readout_line(h, length).c_str(),
                      least.c_str(), most.c_str());
    IM_CHECK(drag_shown);

    // The first fill the engine lists for the empty path whose own time is
    // past the window: hovering its list row moves the window nowhere. A
    // click on the row centres the window on that fill, whose own item is
    // then on the timeline; before the click it has none.
    const hydra::app::PreviewScene& scene = pc->scene();
    std::optional<size_t> late;
    for (const int64_t tick : scene.trailing_skipped_fill_ticks) {
        const std::optional<size_t> fill = hydra::app::fill_index_on_chord(scene, tick);
        if (fill && !hydra::ui::in_view(opening, hydra::ui::timeline_fill_ms(scene.fills[*fill]))) {
            late = fill;
            break;
        }
    }
    IM_CHECK_RETV(late.has_value(), );
    const int64_t late_tick = scene.fills[*late].chord_tick;
    const std::string centred = hydra::ui::readout(
        hydra::ui::centre_on(opening, hydra::ui::timeline_fill_ms(scene.fills[*late]), len), len);
    ctx->ItemClick(back.c_str());
    IM_CHECK(reads(opening_text));
    IM_CHECK(ctx->ItemInfo(fill_ref(late_tick).c_str(), ImGuiTestOpFlags_NoError).ID == 0);
    ctx->MouseMove(choice_ref(late_tick).c_str());
    IM_CHECK(reads(opening_text));
    IM_CHECK(ctx->ItemInfo(fill_ref(late_tick).c_str(), ImGuiTestOpFlags_NoError).ID == 0);
    ctx->ItemClick(choice_ref(late_tick).c_str());
    IM_CHECK(wait_for_edit(ctx));
    IM_CHECK(reads(centred));
    IM_CHECK(ctx->ItemInfo(fill_ref(late_tick).c_str(), ImGuiTestOpFlags_NoError).ID != 0);
    // The window stays there when the pointer leaves the row.
    move_off_timeline(ctx);
    IM_CHECK(reads(centred));
}

// A press and drag on a song longer than its window: the clock stays inside
// the window however far past the track the pointer goes, and neither the
// drag, nor a wheel notch during it, nor the release moves the window. Each
// clock is the Preview's time box (build_time_box) at the moment the timeline
// reads under the pointer (timeline_ms_at, D304 item 7), over the window the
// song opens with (open_view).
void test_drag_stays_in_the_window(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = long_song_notes();
    reset_app(h);
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    ctx->SetRef(h.home_ref.c_str());
    move_off_timeline(ctx);
    hydra::ui::PreviewController* pc = h.app->preview();
    IM_CHECK_RETV(pc != nullptr, );
    const double len = pc->playback_end_ms();
    IM_CHECK_RETV(len > hydra::ui::kTimelineWindowMs, );
    const std::string window = hydra::ui::readout(hydra::ui::open_view(len), len);
    IM_CHECK_RETV(on_screen(h, window), );

    // The rows' item spans the track from its start to its end, so an x in
    // it is the layout's x less the item's left edge.
    const ImRect rows = ctx->ItemInfo(any_ref("##timeline").c_str()).RectFull;
    hydra::ui::TimelineLayout layout;
    layout.width = rows.GetWidth();
    layout.length_ms = len;
    layout.view = hydra::ui::open_view(len);
    const auto clock_at = [&](float x) {
        return hydra::app::build_time_box(pc->scene(), hydra::ui::timeline_ms_at(layout, x - rows.Min.x), len)
            .timestamp;
    };

    const float saved_y = rows.Min.y + rows.GetHeight() * 0.4f;
    ctx->MouseMoveToPos(ImVec2(rows.GetCenter().x, saved_y));
    ctx->MouseDown(ImGuiMouseButton_Left);
    const float past_right = rows.Max.x + 40.0f;
    ctx->MouseMoveToPos(ImVec2(past_right, saved_y));
    ctx->Yield(2);
    // Past the right edge the clock is the window's end.
    IM_CHECK(hydra::ui::timeline_ms_at(layout, past_right - rows.Min.x) ==
             layout.view.start_ms + layout.view.span_ms);
    IM_CHECK(on_screen(h, clock_at(past_right)));
    IM_CHECK(on_screen(h, window));
    ctx->MouseWheelY(-1.0f);
    ctx->Yield(2);
    IM_CHECK(on_screen(h, window));
    const float past_left = rows.Min.x - 40.0f;
    ctx->MouseMoveToPos(ImVec2(past_left, rows.Max.y + 40.0f));
    ctx->Yield(2);
    // Past the left edge it is the window's start.
    IM_CHECK(hydra::ui::timeline_ms_at(layout, past_left - rows.Min.x) == layout.view.start_ms);
    IM_CHECK(on_screen(h, clock_at(past_left)));
    IM_CHECK(on_screen(h, window));
    const float middle = rows.GetCenter().x;
    ctx->MouseMoveToPos(ImVec2(middle, saved_y));
    ctx->Yield(2);
    ctx->MouseUp(ImGuiMouseButton_Left);
    ctx->Yield(2);
    IM_CHECK(on_screen(h, clock_at(middle)));
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
    // The words are the owners': the sentence a missing song file reads as in
    // the builder (plain_error, the runner being the Path Builder), and the
    // details line naming the path that was asked for.
    const hydra::KindedError missing(hydra::ErrorKind::SongFileMissing,
                                     hydra::cannot_open_file_text(h.chart_arg));
    IM_CHECK(h.app->state.error() == hydra::app::plain_error(missing));
    IM_CHECK(h.app->state.error_detail() == hydra::cannot_open_file_text(h.chart_arg));
    IM_CHECK(on_screen(h, h.app->state.error()));
    IM_CHECK(on_screen(h, h.app->state.error_detail()));
}

// With Burnout open, puts a window on the player's path, writes `bad_text` to
// `file` and tries another song: the open is refused with `sentence` and
// `detail`, and Burnout and the path stay.
void check_open_song_stays(ImGuiTestContext* ctx, Harness& h, const std::string& file,
                           const std::string& bad_text, const char* sentence,
                           const std::string& detail) {
    IM_CHECK_RETV(click_fill(ctx, kFirstFillTick), );
    const hydra::pathbuilder::OpenChart* open_song = h.app->state.chart();
    const uint64_t generation = h.app->state.history().generation();
    IM_CHECK_RETV(h.app->state.history().current().windows.size() == 1, );
    IM_CHECK_RETV(write_file(file, bad_text), );
    h.app->open(long_song_notes());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK(h.app->state.chart() == open_song);
    IM_CHECK(h.app->state.history().generation() == generation);
    IM_CHECK(h.app->state.history().current().windows.size() == 1);
    IM_CHECK(h.app->state.requested_path() != long_song_notes());
    IM_CHECK(on_screen(h, "Burnout — Green Day"));
    IM_CHECK(on_screen(h, sentence));
    IM_CHECK(on_screen(h, detail));
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
    // The details line names the file, the key and the value, in the settings
    // reader's words (ini_bad_value_text).
    const std::string detail =
        hydra::app::ini_bad_value_text(h.ini_path, "sp_cap", "banana", hydra::app::kWantWholeNumber);
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

    check_open_song_stays(ctx, h, h.ini_path, bad_settings,
                          hydra::app::kSettingsRefusedSentence, detail);
}

// A bad rules file refuses the song the way a bad settings file does (D304 item
// 13): no song with none open, a good file opens it, and a song open stays with
// the player's path. With both files bad, the settings file's error shows,
// because the settings are read first.
void test_bad_rules_refused(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg.clear();
    reset_app(h);
    const ScratchRules rules;
    // The leeway line sits before the bad one, so a read that stopped part way
    // would have it.
    const std::string bad_rules = "backend_leeway_ms = 50\nmax_tied_paths = banana\n";
    // The details line names the file, the key and the value, in the rules
    // reader's words (ini_bad_value_text).
    const std::string detail = hydra::app::ini_bad_value_text(rules.path, "max_tied_paths", "banana",
                                                              hydra::app::kWantWholeNumber);
    IM_CHECK_RETV(rules.write(bad_rules), );
    h.app->open(corpus::burnout_notes());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK(h.app->state.chart() == nullptr);
    IM_CHECK(!h.app->state.opening());
    IM_CHECK(!h.app->state.preview_volume().has_value());
    IM_CHECK(on_screen(h, hydra::app::kRulesRefusedSentence));
    IM_CHECK(on_screen(h, detail));
    IM_CHECK(!on_screen(h, hydra::app::kAnalysisOffSentence));

    IM_CHECK_RETV(rules.write("max_tied_paths = 4\n"), );
    h.app->open(corpus::burnout_notes());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK_RETV(h.app->state.chart() != nullptr, );
    IM_CHECK(!on_screen(h, hydra::app::kRulesRefusedSentence));
    IM_CHECK(!on_screen(h, detail));
    IM_CHECK(h.app->refusal().empty());

    check_open_song_stays(ctx, h, rules.path, bad_rules, hydra::app::kRulesRefusedSentence,
                          detail);

    // Both files bad. reset_app writes the settings file back for the next test.
    IM_CHECK_RETV(write_file(h.ini_path, "sp_cap=banana\n"), );
    h.app->open(long_song_notes());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK(on_screen(h, hydra::app::kSettingsRefusedSentence));
    IM_CHECK(!on_screen(h, hydra::app::kRulesRefusedSentence));
    IM_CHECK(!on_screen(h, detail));
}

// The open chart's best path total under the rules it was opened with (the
// engine's record), as the head row's "Best possible" figure writes it
// (group_thousands; the ladder's bf[0] is Hydra's best path, path_costs.h).
std::string best_possible_figure(Harness& h) {
    return hydra::group_thousands(h.app->state.chart()->record.best_path().totalscore());
}

// A rule from the file reaches what the builder computes, and a later open
// reads the file again: the same chart on the defaults, under a wider backend
// leeway, then with the file gone. Each figure is the engine's best path total
// for the rules that open read.
void test_rules_reach_the_builder(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg.clear();
    reset_app(h);
    const ScratchRules rules;
    rules.remove();
    h.app->open(corpus::burnout_notes());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK_RETV(h.app->state.chart() != nullptr, );
    const std::string default_best = best_possible_figure(h);
    IM_CHECK(on_screen(h, default_best));

    IM_CHECK_RETV(rules.write("backend_leeway_ms = 500\n"), );
    h.app->open(corpus::burnout_notes());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK_RETV(h.app->state.chart() != nullptr, );
    IM_CHECK(h.app->state.chart()->settings.rules.backend_leeway_ms == 500.0);
    // The wider leeway changes Burnout's best path, so the search ran under
    // the file's rules, and the screen shows that total, not the default's.
    const std::string wide_best = best_possible_figure(h);
    IM_CHECK_RETV(wide_best != default_best, );
    IM_CHECK(on_screen(h, wide_best));
    IM_CHECK(!on_screen(h, default_best));

    rules.remove();
    h.app->open(corpus::burnout_notes());
    IM_CHECK_RETV(wait_idle(ctx), );
    ctx->Yield(2);
    IM_CHECK_RETV(h.app->state.chart() != nullptr, );
    IM_CHECK(h.app->state.chart()->settings.rules.backend_leeway_ms ==
             hydra::core::default_rules().backend_leeway_ms);
    IM_CHECK(best_possible_figure(h) == default_best);
    IM_CHECK(on_screen(h, default_best));
    IM_CHECK(!on_screen(h, wide_best));
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
    uitest::register_test(h, "bad-rules-refused", test_bad_rules_refused);
    uitest::register_test(h, "rules-reach-the-builder", test_rules_reach_the_builder);
}

}  // namespace pbtest
