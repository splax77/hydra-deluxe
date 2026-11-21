#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

#include "uitest_harness.h"

#include "../chart_text.h"

#include "app/preview_view.h"
#include "core/model.h"
#include "core/stars.h"   // path_stars
#include "core/strutil.h"  // ends_with
#include "core/winstr.h"  // fopen_utf8
#include "imgui_internal.h"
#include "parse/song.h"  // no_notes_message
#include "render/overlay_layout.h"
#include "ui/app_state.h"
#include "ui/preview_controller.h"
#include "ui/preview_load_job.h"
#include "ui/preview_panel.h"  // kJumpSeconds, kStepMs, jump_back_label, jump_forward_label
#include "ui/text_role.h"  // font_for, measure_in_role

namespace uitest {

namespace {

// How long chart 0 of testdata/input must run for the Preview tests below:
// their jumps and steps start at 10 s and go one jump further, so the song
// has to reach past that. A guard against the fixture, not a rule of the app.
constexpr double kChart0MinMs = 16000.0;

// How far one jump and one step move the playhead: the panel's own sizes
// (preview_panel.h), which its buttons, keys and labels are built from.
constexpr double kJumpMs = hydra::ui::kJumpSeconds * 1000.0;
constexpr double kPanelStepMs = static_cast<double>(hydra::ui::kStepMs);
// The tests below park at 10 s and jump one jump on, which must stay inside
// chart 0.
constexpr double kParkMs = 10000.0;
static_assert(kParkMs + kJumpMs < kChart0MinMs, "chart 0 must reach one jump past the park");

void test_preview(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    // The loading bar's numbers and labels are PreviewLoadJob::Progress's,
    // pinned in tests/test_preview_load_progress.cpp, and the bar during a
    // real load is preview-load-bar's (uitest_details.cpp). This checks the
    // loaded Preview and its Play and Pause.
    ctx->ItemClick("**/Preview");
    IM_CHECK(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active(); }, 10));
    IM_CHECK(wait_until(ctx, [&] { return !h.app->preview->loading(); }, 120));
    IM_CHECK_STR_EQ(h.app->preview->error().c_str(), "");
    IM_CHECK(!h.app->preview->playing());
    ctx->ItemClick("**/Play");
    IM_CHECK(h.app->preview->playing());
    ctx->ItemClick("**/Pause");
    IM_CHECK(!h.app->preview->playing());
}

// A click's analysis that finishes while the Preview tab is visible must
// still save its summary and reap the job: persistence must not depend on the
// Paths tab drawing. If it did, the job would sit "finished" forever and the
// record would never be stored.
void test_analyze_on_preview(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    ViewGate gate;  // the analysis finishes only once the Preview shows
    if (!open_preview(ctx)) return;
    IM_CHECK(h.app->view_running());
    gate.open();
    IM_CHECK(wait_until(ctx, [&] { return h.app->view_settled(); }, 300));
    IM_CHECK(h.app->viewed.ready());
    IM_CHECK(!h.app->viewed.record->paths.empty());
    IM_CHECK(h.app->view_row_status(0) == hydra::store::RecordStatus::Ready);
    // Switching to Paths shows the stored result, no re-analyze.
    ctx->ItemClick("##DetailsTabs/Paths");
    std::string best = h.app->viewed.record->best_path().pathstring();
    IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(best) != std::string::npos; }, 5));
}

// The "Showing" list's ID. Dear ImGui's BeginCombo reports no label to the
// test engine, so a "**/##previewpath" wildcard never finds it. It is hashed
// on the same ID stack as the "< Act" button under it, whose parent ID the
// engine does know, so this works wherever the tab is drawn.
ImGuiID preview_path_combo(ImGuiTestContext* ctx) {
    const ImGuiTestItemInfo act = ctx->ItemInfo("**/< Act##prevact");
    return ImHashStr("##previewpath", 0, act.ParentID);
}

// Pick `path` in the Preview's "Showing" list: the item whose button in
// build_path_buttons' list stands for it, by the id the tab gives it
// (path_item_id). The Preview tab must be showing.
void pick_preview_path(ImGuiTestContext* ctx, const hydra::Path* path) {
    Harness& h = harness(ctx);
    const hydra::app::PathButtonsView list = hydra::app::build_path_buttons(
        *h.app->viewed.record, h.app->settings.depth_mode, h.app->settings.depth_value);
    size_t index = 0;
    while (index < list.buttons.size() && list.buttons[index].path != path) ++index;
    IM_CHECK(index < list.buttons.size());  // the path is in the list
    if (index >= list.buttons.size()) return;
    const std::string item =
        hydra::app::path_item_id(hydra::app::preview_path_label(list.buttons[index]), index);
    ctx->ItemClick(preview_path_combo(ctx));
    ctx->Yield(1);
    ctx->ItemClick(("//$FOCUSED/" + item).c_str());
    ctx->Yield(2);
}

// A fresh app with chart 0 open and analyzed. Returns false (the check
// already failed) when a step did not work.
bool open_chart0_analyzed(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    scan_library(ctx);
    if (ctx->IsError()) return false;
    open_details(ctx, 0);
    if (ctx->IsError()) return false;
    wait_song_analyzed(ctx);
    return !ctx->IsError();
}

// The path overlay follows the Paths tab's selection. Re-opening the Preview
// for a chart that is already open can't be a plain no-op, or the overlay
// would stay on whatever path was selected the first time -- the record's
// optimal path. Picking another path must swap the overlay in place: no
// re-parse, no audio re-decode, and the playhead left where it was.
void test_preview_path_overlay(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_chart0_analyzed(ctx)) return;

    // A second path to switch to. Path rows are labeled by pathstring, so the
    // one picked must differ from the first path's and be unique among every
    // row the panel draws (the all-0 section included).
    std::vector<const hydra::Path*> paths = h.app->viewed.record->all_paths();
    std::vector<const hydra::Path*> rows = paths;
    for (const hydra::Path* p : h.app->viewed.record->all_allzero_paths())
        rows.push_back(p);
    const hydra::Path* other = nullptr;
    for (size_t i = 1; i < paths.size() && other == nullptr; ++i) {
        std::string label = paths[i]->pathstring();
        if (label == paths[0]->pathstring()) continue;
        size_t seen = 0;
        for (const hydra::Path* p : rows)
            if (p->pathstring() == label) ++seen;
        if (seen == 1) other = paths[i];
    }
    IM_CHECK(other != nullptr);  // the fixture must keep 2+ distinguishable paths
    const std::string first_key = hydra::app::path_overlay_key(paths[0]);
    const std::string other_key = hydra::app::path_overlay_key(other);
    const std::string first_label = paths[0]->pathstring();
    const std::string other_label = other->pathstring();
    IM_CHECK(first_key != other_key);
    size_t first_rows = 0;
    for (const hydra::Path* p : rows)
        if (p->pathstring() == first_label) ++first_rows;
    IM_CHECK_EQ(first_rows, (size_t)1);  // the first path's row is addressable too

    // The Preview opens on the default selection: the record's first path.
    ctx->ItemClick("##DetailsTabs/Preview");
    IM_CHECK(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active(); }, 10));
    IM_CHECK(wait_until(ctx, [&] { return !h.app->preview->loading(); }, 120));
    IM_CHECK_STR_EQ(h.app->preview->error().c_str(), "");
    // The overlay key carries the SP cap after the path's own key, so ask
    // shows_path whose path it is, then compare whole keys against this first
    // one. The path's overlay can land a frame or two after the load itself.
    IM_CHECK(wait_until(ctx, [&] { return h.app->preview->shows_path(first_key); }, 10));
    const std::string first_overlay = h.app->preview->overlay_path_key();
    IM_CHECK(h.app->preview->shows_path(first_key));

    // Park the playhead mid-song: a reload would rewind it to zero.
    IM_CHECK(h.app->preview->playback_end_ms() > 0.0);
    h.app->preview->seek_ms(h.app->preview->playback_end_ms() * 0.5);
    ctx->Yield(2);
    double held = h.app->preview->position_ms();
    IM_CHECK(held > 0.0);

    // Pick the other path in the Preview's list, then visit Paths and come
    // back, so the Preview is re-opened on the same chart.
    pick_preview_path(ctx, other);
    ctx->ItemClick("##DetailsTabs/Paths");
    ctx->Yield(2);
    ctx->ItemClick("##DetailsTabs/Preview");
    ctx->Yield(2);
    IM_CHECK(!h.app->preview->loading());  // swapped in place, not reloaded
    IM_CHECK(wait_until(ctx, [&] { return h.app->preview->shows_path(other_key); }, 10));
    IM_CHECK_FLOAT_NEAR_EQ(h.app->preview->position_ms(), held, 1.0);
    IM_CHECK(h.app->preview->overlay_path_key() != first_overlay);

    // The same chart still previews the first path when it is picked again.
    pick_preview_path(ctx, paths.front());
    ctx->ItemClick("##DetailsTabs/Paths");
    ctx->Yield(2);
    ctx->ItemClick("##DetailsTabs/Preview");
    ctx->Yield(2);
    IM_CHECK(!h.app->preview->loading());
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->preview->overlay_path_key() == first_overlay;
    }, 10));
}

// The Preview's finer time controls and running score, driven through the
// controller inside the real app. preview-buttons-keys drives the same
// through the panel's buttons and keys.
void test_preview_controls(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    ViewGate gate;  // holds chart 0's analysis while the Preview loads
    if (!open_preview(ctx)) return;  // chart 0, its analysis held
    auto& pc = *h.app->preview;

    // No analyzed path: no score box.
    IM_CHECK(!pc.score_box().shown);

    // Let the analysis land, then visit Paths and come back so the overlay
    // (and with it the score) is rebuilt from the new record's path.
    gate.open();
    wait_song_analyzed(ctx);  // lands on Paths
    if (ctx->IsError()) return;
    ctx->Yield(2);
    ctx->ItemClick("##DetailsTabs/Preview");
    // Wait for the reload to finish and the score box to appear, rather than
    // assume a fixed number of frames is enough.
    IM_CHECK(wait_until(ctx, [&] { return !pc.loading() && pc.score_box().shown; }, 60));

    // At the song's end the box reads the selected path's total.
    IM_CHECK(pc.playback_end_ms() > kChart0MinMs);
    pc.seek_ms(pc.playback_end_ms());
    hydra::app::PreviewScoreBox end = pc.score_box();
    IM_CHECK(end.shown);
    IM_CHECK(end.available);
    const hydra::Path* shown = h.app->viewed.record->all_paths().front();
    IM_CHECK_STR_EQ(end.score.c_str(), hydra::group_thousands(shown->totalscore()).c_str());
    // And the stars the details header shows for that path (path_stars), with
    // the next-star line gone only at the most there are.
    IM_CHECK_EQ(end.stars, hydra::path_stars(*shown));
    IM_CHECK_EQ(end.next_star.empty(), end.stars == hydra::kMaxStars);

    // Jumps of the panel's size (kJumpMs), clamped to 0 and to where playback
    // stops (playback_end_ms, the song's length). The audio's own length
    // plays no part in it; test_preview_controller.cpp pins songs that end
    // before and after their audio.
    pc.seek_ms(kParkMs);
    pc.jump_ms(kJumpMs);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs + kJumpMs, 0.5);
    pc.jump_ms(-kJumpMs);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs, 0.5);
    pc.jump_ms(-(pc.playback_end_ms() + kJumpMs));
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), 0.0, 0.5);
    pc.jump_ms(pc.playback_end_ms() + kJumpMs);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), pc.playback_end_ms(), 0.5);

    // A jump while playing keeps playing.
    pc.seek_ms(kParkMs);
    pc.play();
    IM_CHECK(pc.playing());
    pc.jump_ms(kJumpMs);
    IM_CHECK(pc.playing());

    // A step of the panel's size (kPanelStepMs) pauses, and one step each
    // way returns to the same moment.
    pc.step_ms(0.0);  // pause and snap onto the displayed moment
    IM_CHECK(!pc.playing());
    pc.seek_ms(kParkMs);
    pc.step_ms(kPanelStepMs);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs + kPanelStepMs, 0.001);
    pc.step_ms(-kPanelStepMs);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs, 0.001);

    // A step stops at 0 and at where playback stops: from half a step
    // inside either end, a whole step lands on the end.
    pc.seek_ms(kPanelStepMs / 2.0);
    pc.step_ms(-kPanelStepMs);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), 0.0, 0.001);
    pc.seek_ms(pc.playback_end_ms() - kPanelStepMs / 2.0);
    pc.step_ms(kPanelStepMs);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), pc.playback_end_ms(), 0.001);
}

// The overlay's boxes as drawn, found by the names preview_panel gives them: no
// box overlaps another or the gauge, and the next-activation box stands under
// the drain box (D111 item 3) with their right edges lined up and the left
// column's gap between them. A box not on screen is left out. Returns how
// many boxes it found.
int check_overlay_boxes(ImGuiTestContext* ctx) {
    const char* const ids[] = {"**/##time_box",     "**/##score_box", "**/##drain_box",
                               "**/##next_act_box", "**/##sp_gauge",  "**/##solo_box"};
    enum { kTime, kScore, kDrain, kNext, kGauge, kSolo, kCount };
    ImRect r[kCount];
    bool on[kCount] = {};
    int found = 0;
    for (int i = 0; i < kCount; ++i) {
        const ImGuiTestItemInfo info = ctx->ItemInfo(ids[i], ImGuiTestOpFlags_NoError);
        on[i] = info.ID != 0;
        if (on[i]) r[i] = info.RectFull, ++found;
    }
    for (int i = 0; i < kCount; ++i)
        for (int j = i + 1; j < kCount; ++j)
            if (on[i] && on[j] && r[i].Overlaps(r[j])) {
                ctx->LogError("%s overlaps %s", ids[i], ids[j]);
                IM_CHECK_NO_RET(!r[i].Overlaps(r[j]));
            }
    if (on[kNext] && on[kDrain]) {
        // Two sums that come to the same pixel can differ by float rounding
        // alone. The allowance is the one this file's other position checks
        // (test_layout_drift) already use for "did not move".
        constexpr float kSamePlace = 0.01f;
        IM_CHECK_NO_RET(r[kNext].Min.y >= r[kDrain].Max.y);
        IM_CHECK_NO_RET(std::fabs(r[kNext].Max.x - r[kDrain].Max.x) <= kSamePlace);
        if (on[kTime] && on[kScore]) {
            const float right_gap = r[kNext].Min.y - r[kDrain].Max.y;
            const float left_gap = r[kScore].Min.y - r[kTime].Max.y;
            IM_CHECK_NO_RET(std::fabs(right_gap - left_gap) <= kSamePlace);
        }
    }
    // The solo box on the right, never further right than the drain box's
    // right edge, below the boxes above it. Where its left edge sits beside
    // the highway is check_solo_hugs_highway's.
    if (on[kSolo] && on[kDrain]) {
        constexpr float kSamePlace = 0.01f;
        IM_CHECK_NO_RET(r[kSolo].Max.x <= r[kDrain].Max.x + kSamePlace);
        IM_CHECK_NO_RET(r[kSolo].Min.y >= r[kDrain].Max.y);
        if (on[kNext]) IM_CHECK_NO_RET(r[kSolo].Min.y >= r[kNext].Max.y);
    }
    return found;
}

// The solo box hugs the highway: its left edge is the overlay's gap right of
// the highway's right railing at the box's lowest row (render::highway_span_at),
// where the highway is widest beside it. Called mid-solo, when the box is as
// tall as the scale reserves. The gap is the one between the time box and the
// score box. Needs the score box on screen.
void check_solo_hugs_highway(ImGuiTestContext* ctx, const hydra::ui::PreviewController& pc) {
    const ImRect img = ctx->ItemInfo("**/##highway_image").RectFull;
    const ImRect solo = ctx->ItemInfo("**/##solo_box").RectFull;
    const ImRect time = ctx->ItemInfo("**/##time_box").RectFull;
    const ImRect score = ctx->ItemInfo("**/##score_box").RectFull;
    const float gap = score.Min.y - time.Max.y;
    const float railing =
        img.Min.x + hydra::render::highway_span_at(pc.preview_config(),
                                                   static_cast<int>(img.GetWidth()),
                                                   static_cast<int>(img.GetHeight()),
                                                   solo.Max.y - img.Min.y)
                        .right;
    std::fprintf(stderr, "solo box left %.2f, railing %.2f, gap %.2f\n", solo.Min.x, railing,
                 gap);
    // The allowance check_overlay_boxes uses for two sums of the same pixel.
    constexpr float kSamePlace = 0.01f;
    IM_CHECK_NO_RET(gap > 0.0f);
    IM_CHECK_NO_RET(std::fabs(solo.Min.x - (railing + gap)) <= kSamePlace);
}

std::string readout_of(double bars, int cap);  // defined below, by its first users

// The solo box on a chart with solos: the running percent and count, the end
// message on the last note, the bonus after it, then hidden. Its values are
// build_solo_box's, pinned by the unit tests; this checks the panel feeds it
// the playhead, draws it where it belongs and keeps the scale still.
void test_preview_solo_box(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    // Levitating's Expert Drums has a solo (the [ExpertDrums] "solo" event).
    open_titled(ctx, "levitating", "Levitating (feat. DaBaby) (Dua Lipa) (L)");
    if (ctx->IsError()) return;
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    ctx->ItemClick("##DetailsTabs/Preview");
    IM_CHECK(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active(); }, 10));
    auto& pc = *h.app->preview;
    IM_CHECK(wait_until(ctx, [&] { return !pc.loading() && pc.score_box().shown; }, 120));
    // The chart's solo sections as the scene read them: Levitating's
    // [ExpertDrums] has one "solo" event (tick 132480, ended at 140160).
    const std::vector<hydra::app::PreviewSoloNotes>& solos = pc.scene().solo_notes;
    IM_CHECK(!solos.empty());
    if (solos.empty()) return;
    const hydra::app::PreviewSoloNotes& solo = solos.front();
    const double start_ms = solo.steps.front().ms;
    const double last_ms = solo.steps.back().ms;
    IM_CHECK(start_ms > 0.0);
    IM_CHECK(last_ms > start_ms);

    // Before the solo's first chord: no solo yet.
    pc.seek_ms(0.0);
    ctx->Yield(2);
    IM_CHECK(!pc.solo_box().shown);
    IM_CHECK(ctx->ItemInfo("**/##solo_box", ImGuiTestOpFlags_NoError).ID == 0);
    const float scale = pc.overlay_scale();
    pc.seek_ms(start_ms - 1.0);
    IM_CHECK(!pc.solo_box().shown);

    // On the solo's first chord: the percent of its notes struck so far
    // (solo_percent, the game's rounding) and the solo's note total.
    pc.seek_ms(start_ms);
    const hydra::app::PreviewSoloBox first = pc.solo_box();
    IM_CHECK(first.shown);
    const std::string first_percent =
        std::to_string(hydra::app::solo_percent(solo.steps.front().notes_hit, solo.notes)) + "%";
    IM_CHECK_STR_EQ(first.headline.c_str(), first_percent.c_str());
    IM_CHECK(first.detail.find(hydra::group_thousands(solo.notes)) != std::string::npos);
    std::fprintf(stderr, "first solo at %.0f ms: %s, %s\n", start_ms, first.headline.c_str(),
                 first.detail.c_str());

    // Mid-solo: drawn against the highway, clear of the other boxes, at the
    // scale it had before the solo.
    ctx->Yield(2);
    IM_CHECK(ctx->ItemInfo("**/##solo_box", ImGuiTestOpFlags_NoError).ID != 0);
    check_overlay_boxes(ctx);
    check_solo_hugs_highway(ctx, pc);
    IM_CHECK_EQ(pc.overlay_scale(), scale);
    const float playing_left = ctx->ItemInfo("**/##solo_box").RectFull.Min.x;

    // The chart has no Star Power phrase (the scene's phrases are read from
    // its notes' SP flags), yet the gauge and the drain box show: the gauge
    // empty, the drain box in its idle form.
    IM_CHECK(pc.scene().sp_phrases.empty());
    IM_CHECK(pc.has_sp_gauge());
    IM_CHECK(ctx->ItemInfo("**/##sp_gauge", ImGuiTestOpFlags_NoError).ID != 0);
    IM_CHECK(ctx->ItemInfo("**/##drain_box", ImGuiTestOpFlags_NoError).ID != 0);
    IM_CHECK_STR_EQ(pc.sp_meter_readout().c_str(), readout_of(0.0, pc.sp_meter_cap()).c_str());
    const hydra::app::PreviewDrainBox drain = pc.drain_box();
    IM_CHECK(drain.shown);
    IM_CHECK(!drain.active);
    IM_CHECK(screenshot(ctx, "solo-box-playing.png"));

    // Just before the solo's last chord the running count still shows.
    pc.seek_ms(last_ms - 1.0);
    const hydra::app::PreviewSoloBox running = pc.solo_box();
    IM_CHECK(running.shown);
    IM_CHECK(!running.detail.empty());

    // On the last chord: the one-line end message, no count under it. Its
    // words are build_solo_box's literal, with no owner to read them from.
    pc.seek_ms(last_ms);
    const hydra::app::PreviewSoloBox result = pc.solo_box();
    IM_CHECK(result.shown);
    IM_CHECK(result.detail.empty());
    IM_CHECK(!result.headline.empty());
    IM_CHECK(result.headline != running.headline);
    ctx->Yield(2);
    check_overlay_boxes(ctx);
    // The one-line result keeps the left edge the two-line box had.
    IM_CHECK_EQ(ctx->ItemInfo("**/##solo_box").RectFull.Min.x, playing_left);
    IM_CHECK(screenshot(ctx, "solo-box-perfect.png"));

    // kSoloResultMs after the last chord the bonus: the solo's own bonus
    // (the scene's sum of solo_bonus over its chords).
    pc.seek_ms(last_ms + hydra::app::kSoloResultMs);
    const hydra::app::PreviewSoloBox bonus = pc.solo_box();
    IM_CHECK(bonus.shown);
    IM_CHECK(bonus.headline != result.headline);
    IM_CHECK(bonus.headline.find(hydra::group_thousands(solo.bonus)) != std::string::npos);
    IM_CHECK(bonus.detail.empty());
    std::fprintf(stderr, "bonus: %s\n", bonus.headline.c_str());
    ctx->Yield(2);
    IM_CHECK(screenshot(ctx, "solo-box-bonus.png"));
    IM_CHECK_EQ(pc.overlay_scale(), scale);

    // kSoloTailMs after the last chord the box is gone, unless the chart's
    // next solo has begun by then.
    const double tail_ms = last_ms + hydra::app::kSoloTailMs;
    pc.seek_ms(tail_ms);
    const bool next_begun = solos.size() > 1 && solos[1].steps.front().ms <= tail_ms;
    IM_CHECK_EQ(pc.solo_box().shown, next_begun);
}

// The SP drain box beside the gauge. Its values are build_drain_box's, pinned
// by the unit tests; this checks the panel feeds it the playhead and the
// viewed path, and saves a frame of the active box to look at.
void test_preview_drain_box(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_preview(ctx)) return;  // chart 0; the click analyzes it
    auto& pc = *h.app->preview;

    // Wait for the analysis, then visit Paths and come back so the overlay is
    // rebuilt from the new record's path (as preview-controls does).
    wait_song_analyzed(ctx);  // lands on Paths
    if (ctx->IsError()) return;
    ctx->Yield(2);
    ctx->ItemClick("##DetailsTabs/Preview");
    IM_CHECK(wait_until(ctx, [&] { return !pc.loading() && pc.score_box().shown; }, 60));
    IM_CHECK(pc.has_sp_gauge());

    // The drawn path is the default selection, the record's first path.
    const hydra::Path* drawn = h.app->viewed.record->all_paths().front();
    IM_CHECK(wait_until(ctx, [&] { return pc.shows_path(hydra::app::path_overlay_key(drawn)); }, 10));
    const hydra::ActivationWalk acts = drawn->walk_activations();
    const std::vector<hydra::app::PreviewActivation>& shown = pc.scene().activations;
    IM_CHECK_EQ(shown.size(), acts.size());
    if (shown.size() != acts.size()) return;

    // Before anything is banked or spent: the idle box.
    pc.seek_ms(0.0);
    const hydra::app::PreviewDrainBox idle = pc.drain_box();
    IM_CHECK(idle.shown);
    IM_CHECK(!idle.active);
    IM_CHECK(!idle.header.empty());
    IM_CHECK(!idle.rate.empty());
    // All drawn, the next-activation box under the drain box when the path
    // has an activation ahead.
    IM_CHECK_EQ(pc.next_act_box().shown, !acts.empty());
    ctx->Yield(2);
    IM_CHECK_EQ(check_overlay_boxes(ctx), acts.empty() ? 4 : 5);

    // Each activation's Star Power, at the moments the record gives: the box
    // is active from the activation's chord to the record's SP end, and idle
    // from that end on unless the next activation has begun by then. The
    // header and the second line change with it; their words are
    // build_drain_box's literals, with no owner to read them from.
    bool saved = false;
    for (size_t i = 0; i < acts.size(); ++i) {
        const hydra::Activation& rec = acts[i];
        const hydra::app::PreviewActivation& act = shown[i];
        IM_CHECK_EQ(act.tick, rec.timecode.ticks());
        IM_CHECK_FLOAT_NEAR_EQ(act.ms, rec.timecode.ms(), 0.5);
        const std::optional<int64_t> end = rec.window_end_tick();
        IM_CHECK_EQ(act.has_sp_end, end.has_value());
        if (!end || !act.has_sp_end) continue;  // no window: nothing runs (checked above)
        IM_CHECK_EQ(act.sp_end_tick, *end);
        IM_CHECK(act.sp_end_ms > act.ms);
        for (double at : {act.ms, (act.ms + act.sp_end_ms) / 2.0}) {
            pc.seek_ms(at);
            const hydra::app::PreviewDrainBox on = pc.drain_box();
            IM_CHECK(on.shown);
            IM_CHECK(on.active);
            IM_CHECK(on.header != idle.header);
            IM_CHECK(!on.detail.empty());
        }
        if (!saved) {
            // A frame of the active box, for a person to look at.
            ctx->Yield(2);
            check_overlay_boxes(ctx);
            IM_CHECK(screenshot(ctx, "drain-box-active.png"));
            saved = true;
        }
        // At the SP end. The boxes read the playhead held inside the song, so
        // an end past where playback stops is never reached: SP still runs
        // there.
        pc.seek_ms(act.sp_end_ms);
        const bool next_begun = i + 1 < shown.size() && shown[i + 1].ms <= act.sp_end_ms;
        const bool past_song = act.sp_end_ms > pc.playback_end_ms();
        IM_CHECK_EQ(pc.drain_box().active, next_begun || past_song);
        if (!next_begun && !past_song) IM_CHECK(pc.drain_box().header == idle.header);
    }
}

// The readout under the SP gauge for a level of `bars`, in sp_meter_readout's
// own format: a flat curve at that level, read anywhere.
std::string readout_of(double bars, int cap) {
    hydra::app::SpMeterCurve flat;
    flat.cap = cap;
    flat.segments.push_back({0.0, 0.0, bars, bars});
    return hydra::app::sp_meter_readout(flat, 0.0);
}

// The gauge readout at a squeeze-out reads the record's level
// (Activation::meter_bars_at). On an early squeeze-out Star Power ends on the
// squeezed chord with a little still in the meter, so one tick before that
// chord the readout is the record's level there. On a late one SP ends where
// the meter runs out, so the readout there is empty.
//
// The charts are picked by reading each corpus chart's record in turn, never
// by name: a chart is opened by searching its title, and the walk stops once
// both kinds are checked. The early case must be one whose readout is above
// "0.0", so it fails if the gauge drained to the squeezed chord instead.
void test_preview_squeeze_out_gauge(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    std::vector<std::string> titles;
    for (size_t i = 0; i < h.app->library_shown_count(); ++i) {
        const std::string& t = h.app->library_row_at(i).title;
        if (std::find(titles.begin(), titles.end(), t) == titles.end()) titles.push_back(t);
    }

    bool early_checked = false, late_checked = false;
    for (const std::string& title : titles) {
        if (early_checked && late_checked) break;
        const std::string why = open_titled_reason(ctx, title, title);
        if (ctx->IsError()) return;
        if (!why.empty()) {
            std::fprintf(stderr, "skipped \"%s\": %s\n", title.c_str(), why.c_str());
            continue;
        }
        set_panel_ref(ctx);
        if (ctx->IsError()) return;
        IM_CHECK(wait_until(ctx, [&] { return h.app->view_settled(); }, kSettleWaitSeconds));
        if (!h.app->viewed.ready() || h.app->viewed.record->paths.empty()) continue;
        const hydra::Path& best = h.app->viewed.record->best_path();
        const hydra::ActivationWalk acts = best.walk_activations();

        // The optimal path's squeeze-outs of each kind still unchecked. Early
        // or late is the record's own answer: the window ends on the squeezed
        // chord exactly when it is early (window_end_tick).
        std::vector<const hydra::Activation*> earlies;
        const hydra::Activation* late = nullptr;
        for (const hydra::Activation& a : acts) {
            if (!a.sqout_tick || !a.window_end_tick()) continue;
            if (*a.window_end_tick() == *a.sqout_tick) {
                if (!early_checked) earlies.push_back(&a);
            } else if (!late_checked && !late) {
                late = &a;
            }
        }
        if (earlies.empty() && !late) continue;

        ctx->ItemClick("##DetailsTabs/Preview");
        IM_CHECK(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active(); }, 10));
        auto& pc = *h.app->preview;
        IM_CHECK(wait_until(ctx, [&] { return !pc.loading(); }, 120));
        IM_CHECK_STR_EQ(pc.error().c_str(), "");
        IM_CHECK(h.app->details_ui.selected_path == &best);
        const std::string key = hydra::app::path_overlay_key(&best);
        IM_CHECK(wait_until(ctx, [&] { return pc.shows_path(key); }, 30));
        IM_CHECK(pc.scene().timing.has_value());
        if (ctx->IsError()) return;
        const hydra::SongTiming& timing = *pc.scene().timing;
        const int cap = pc.sp_meter_cap();
        pc.pause();

        for (const hydra::Activation* early : earlies) {
            const int64_t tick = *early->sqout_tick - 1;
            const double level = *early->meter_bars_at(tick, timing);
            const std::string want = readout_of(level, cap);
            // A level that prints as empty cannot tell the record's level from
            // a gauge drained to the chord; look on for another.
            if (want == readout_of(0.0, cap)) continue;
            pc.seek_ms(timing.ms_index().at(tick));
            std::fprintf(stderr, "early squeeze-out on \"%s\", chord %lld: readout %s, record %s\n",
                         title.c_str(), static_cast<long long>(*early->sqout_tick),
                         pc.sp_meter_readout().c_str(), want.c_str());
            IM_CHECK_STR_EQ(pc.sp_meter_readout().c_str(), want.c_str());
            early_checked = true;
            break;
        }
        if (late) {
            const int64_t tick = *late->window_end_tick();
            const double level = *late->meter_bars_at(tick, timing, /*just_before=*/true);
            IM_CHECK_EQ(level, 0.0);
            pc.seek_ms(timing.ms_index().at(tick));
            std::fprintf(stderr, "late squeeze-out on \"%s\", meter end %lld, chord %lld: readout %s\n",
                         title.c_str(), static_cast<long long>(tick),
                         static_cast<long long>(*late->sqout_tick), pc.sp_meter_readout().c_str());
            IM_CHECK_EQ(pc.sp_meter_bars(), 0.0);
            IM_CHECK_STR_EQ(pc.sp_meter_readout().c_str(), readout_of(level, cap).c_str());
            late_checked = true;
        }
        if (ctx->IsError()) return;
    }
    IM_CHECK(early_checked);
    IM_CHECK(late_checked);
}

// Sets the harness display width for one test and puts it back however the
// test ends, so a failed check cannot leave later tests on a narrow display.
struct DisplayWidth {
    Harness& h;
    int saved;
    DisplayWidth(Harness& harness_, int width) : h(harness_), saved(harness_.width) {
        h.width = width;
    }
    ~DisplayWidth() { h.width = saved; }
};

// The Preview's text boxes shrink to sit beside the highway in a narrow
// panel and keep their size in a wide one. The song panel shares the window
// with the library, so at the harness's 1280 px the default split already
// shrinks them a little; with the library dragged to its narrowest they are
// full size, and a 640 px display shrinks them more.
void test_preview_overlay_fit(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_preview(ctx)) return;
    auto& pc = *h.app->preview;

    ctx->Yield(3);
    const float split_scale = pc.overlay_scale();
    IM_CHECK(split_scale <= 1.0f);
    IM_CHECK(split_scale >= hydra::render::kOverlayMinScale);

    // The library at its narrowest (what dragging its edge left does): the
    // split sets its width from the share the next time the panel opens.
    hydra::ui::remember_library_share(0.01f);
    h.app->library_ui.panel_was_open = false;
    ctx->Yield(5);
    IM_CHECK_EQ(pc.overlay_scale(), 1.0f);  // a wide panel: full size
    check_overlay_boxes(ctx);
    IM_CHECK(screenshot(ctx, "overlay-fit-wide.png"));

    DisplayWidth narrow(h, 640);
    ctx->Yield(5);
    IM_CHECK(pc.overlay_scale() < 1.0f);
    IM_CHECK(pc.overlay_scale() >= hydra::render::kOverlayMinScale);
    check_overlay_boxes(ctx);
    IM_CHECK(screenshot(ctx, "overlay-fit-narrow.png"));
}

// The Preview's transport buttons and keys: the jump buttons and Left / Right
// jump kJumpMs, the step buttons and comma / period step kPanelStepMs, Space
// plays and pauses. The step buttons' labels have no public owner
// (preview_panel.cpp builds them in a function-local static), so they are
// clicked by the words they show.
void test_preview_buttons_keys(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_preview(ctx)) return;
    auto& pc = *h.app->preview;
    IM_CHECK(!pc.playing());
    IM_CHECK(pc.playback_end_ms() > kChart0MinMs);

    // The jump buttons by the labels the panel gives them (jump_forward_label,
    // jump_back_label), each moving one jump (kJumpMs).
    pc.seek_ms(kParkMs);
    ctx->ItemClick(("**/" + escape_ref(hydra::ui::jump_forward_label())).c_str());
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs + kJumpMs, 0.5);
    ctx->ItemClick(("**/" + escape_ref(hydra::ui::jump_back_label())).c_str());
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs, 0.5);

    // The arrows jump exactly one jump. Had keyboard navigation also taken
    // the arrow and nudged the scrubber, the playhead would be off by the
    // nudge.
    ctx->KeyPress(ImGuiKey_RightArrow);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs + kJumpMs, 0.5);
    ctx->KeyPress(ImGuiKey_LeftArrow);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs, 0.5);

    // A step pauses.
    ctx->ItemClick("**/Play");
    IM_CHECK(pc.playing());
    ctx->ItemClick("**/10 ms >");
    IM_CHECK(!pc.playing());

    // The buttons and the keys each step exactly one step (kPanelStepMs).
    pc.seek_ms(kParkMs);
    ctx->ItemClick("**/10 ms >");
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs + kPanelStepMs, 0.001);
    ctx->ItemClick(("**/" + escape_ref("< 10 ms")).c_str());
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs, 0.001);
    ctx->KeyPress(ImGuiKey_Period);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs + kPanelStepMs, 0.001);
    ctx->KeyPress(ImGuiKey_Comma);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), kParkMs, 0.001);

    // Space plays and pauses. Tab puts the keyboard focus box on the button
    // after "< 10 ms"; had navigation also taken Space it would have
    // pressed that button too, which undoes the toggle or steps the playhead.
    ctx->ItemClick(("**/" + escape_ref("< 10 ms")).c_str());
    ctx->KeyPress(ImGuiKey_Tab);
    IM_CHECK(!pc.playing());
    const double t1 = pc.position_ms();
    ctx->KeyPress(ImGuiKey_Space);
    IM_CHECK(pc.playing());
    IM_CHECK(pc.position_ms() >= t1);
    ctx->KeyPress(ImGuiKey_Space);
    IM_CHECK(!pc.playing());
}

// Click-and-hold on the time bar while playing pauses playback for the hold,
// as Onyx does. Playing on and re-seeking the audio to the held time every
// frame would come out as a buzz. The transport must be paused while the
// mouse is down, sit still at the held time, and resume on release.
void test_scrub_hold(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_preview(ctx)) return;
    ctx->ItemClick("**/Play");
    IM_CHECK(h.app->preview->playing());
    ctx->MouseMove("**/##scrub");
    ctx->MouseDown(0);
    ctx->Yield(5);
    IM_CHECK(!h.app->preview->playing());
    double held = h.app->preview->position_ms();
    ctx->Yield(30);
    IM_CHECK_FLOAT_NEAR_EQ(h.app->preview->position_ms(), held, 0.5);
    ctx->MouseUp(0);
    ctx->Yield(2);
    IM_CHECK(h.app->preview->playing());
    // The same hold while paused stays paused afterwards.
    ctx->ItemClick("**/Pause");
    ctx->MouseMove("**/##scrub");
    ctx->MouseDown(0);
    ctx->Yield(5);
    ctx->MouseUp(0);
    ctx->Yield(2);
    IM_CHECK(!h.app->preview->playing());
}

// A widget must not move under the mouse because a number next to it changed
// width (the font is proportional). The number here is the live time readout,
// at the end of the scrubber's line.
void test_layout_drift(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_preview(ctx)) return;
    ctx->ItemClick("**/Play");
    ImVec2 vol0 = ctx->ItemInfo("**/##volume").RectFull.Min;
    ImVec2 scrub0 = ctx->ItemInfo("**/##scrub").RectFull.Min;
    float scrubw0 = ctx->ItemInfo("**/##scrub").RectFull.GetWidth();
    double t0 = h.app->preview->position_ms();
    // Let the readout pass through several different digit strings.
    IM_CHECK(wait_until(ctx, [&] { return h.app->preview->position_ms() > t0 + 1500.0; }, 10));
    for (int i = 0; i < 20; ++i) {
        ctx->Yield(3);
        IM_CHECK_FLOAT_NEAR_EQ(ctx->ItemInfo("**/##volume").RectFull.Min.x, vol0.x, 0.01f);
        IM_CHECK_FLOAT_NEAR_EQ(ctx->ItemInfo("**/##scrub").RectFull.Min.x, scrub0.x, 0.01f);
        IM_CHECK_FLOAT_NEAR_EQ(ctx->ItemInfo("**/##scrub").RectFull.GetWidth(), scrubw0, 0.01f);
    }
    ctx->ItemClick("**/Pause");
    // Play/Pause swap must not shift the scrubber either.
    IM_CHECK_FLOAT_NEAR_EQ(ctx->ItemInfo("**/##scrub").RectFull.Min.x, scrub0.x, 0.01f);
}

// Burnout analyzed, its Preview open with the optimal path's overlay loaded.
// Returns false (the check already failed) when a step did not work.
bool open_burnout_preview(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return false;
    open_titled(ctx, "burnout", "Burnout");
    if (ctx->IsError()) return false;
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return false;
    const hydra::Path& best = h.app->viewed.record->best_path();
    // The tests on Burnout step through its activations, so its best path
    // must have one: a path that never activates fails here, not quietly.
    IM_CHECK_RETV(!best.walk_activations().empty(), false);
    ctx->ItemClick("##DetailsTabs/Preview");
    IM_CHECK_RETV(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active(); }, 10),
                  false);
    const std::string key = hydra::app::path_overlay_key(&best);
    IM_CHECK_RETV(wait_until(ctx, [&] {
        return !h.app->preview->loading() && h.app->preview->shows_path(key);
    }, 120), false);
    IM_CHECK_RETV(h.app->preview->error().empty(), false);
    IM_CHECK_RETV(h.app->preview->scene().activations.size() == best.walk_activations().size(),
                  false);
    return true;
}

// The record's activation `i` on the path the Preview draws, and the
// scene's copy of it: the playhead lands on the scene's onset, which must be
// the record's chord.
const hydra::app::PreviewActivation& checked_activation(ImGuiTestContext* ctx,
                                                        const hydra::Path& path, size_t i) {
    const hydra::app::PreviewActivation& act = harness(ctx).app->preview->scene().activations[i];
    const hydra::Activation& rec = path.walk_activations()[i];
    IM_CHECK_NO_RET(act.tick == rec.timecode.ticks());
    IM_CHECK_NO_RET(std::fabs(act.ms - rec.timecode.ms()) <= 0.5);
    IM_CHECK_NO_RET(act.notation == rec.notationstr_verbose());
    return act;
}

// The "Showing" list: it names the drawn path, lists the all-0 path under its
// own name, and a pick changes the one selection the Paths tab reads too. A
// pending "Show in Preview" lands the playhead on its activation. (Clicking
// the link itself is checked in paths-rows.)
void test_preview_path_picker(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_burnout_preview(ctx)) return;
    auto& pc = *h.app->preview;
    const hydra::HydraRecord& record = *h.app->viewed.record;
    const hydra::Path& best = record.best_path();
    // The list's items as the picker builds them (build_path_buttons,
    // preview_path_label), so no label is typed here.
    const hydra::app::PathButtonsView list = hydra::app::build_path_buttons(
        record, h.app->settings.depth_mode, h.app->settings.depth_value);
    size_t best_index = list.buttons.size();
    size_t allzero_index = list.buttons.size();
    for (size_t i = 0; i < list.buttons.size(); ++i) {
        if (list.buttons[i].path == &best && best_index == list.buttons.size()) best_index = i;
        if (list.buttons[i].group == hydra::app::PathButtonView::Group::AllZero &&
            allzero_index == list.buttons.size())
            allzero_index = i;
    }
    IM_CHECK(best_index < list.buttons.size());
    IM_CHECK(allzero_index < list.buttons.size());
    if (best_index >= list.buttons.size() || allzero_index >= list.buttons.size()) return;
    const std::string best_label = hydra::app::preview_path_label(list.buttons[best_index]);
    const std::string allzero_label = hydra::app::preview_path_label(list.buttons[allzero_index]);

    ctx->Yield(2);
    IM_CHECK(visible_text(h).find("Showing") != std::string::npos);
    IM_CHECK(on_screen(h, best_label));

    // The list sits on its own line above the buttons, wide enough for its
    // longest path, so no path is cut off under the arrow: each label, its
    // frame padding each side and the arrow square fit in the box.
    {
        const ImRect combo = ctx->ItemInfo(preview_path_combo(ctx)).RectFull;
        const ImRect act = ctx->ItemInfo("**/< Act##prevact").RectFull;
        IM_CHECK_GE(act.Min.y, combo.Max.y);
        const ImGuiStyle& s = ImGui::GetStyle();
        const hydra::ui::WidthOf notation_w = hydra::ui::measure_in_role(hydra::ui::TextRole::Notation);
        for (const hydra::app::PathButtonView& b : list.buttons) {
            const float need =
                notation_w(hydra::app::preview_path_label(b)) + s.FramePadding.x * 2.0f + combo.GetHeight();
            IM_CHECK_LE(need, combo.GetWidth() + 1.0f);
        }
    }

    // A path too long for its line (40 activations run to about 100
    // characters) ends in "…" inside the box, in the picker's own font, at
    // the narrowest panel. The picker draws its label through this same cut.
    {
        hydra::ui::remember_library_share(0.99f);
        h.app->library_ui.panel_was_open = false;
        ctx->Yield(3);
        const ImGuiTestItemInfo combo = ctx->ItemInfo(preview_path_combo(ctx));
        ImGuiWindow* win = combo.Window;
        IM_CHECK(win != nullptr);
        if (win == nullptr) return;
        const ImGuiStyle& s = ImGui::GetStyle();
        const float line = win->WorkRect.Max.x - combo.RectFull.Min.x;  // the most the box gets
        const float chrome = s.FramePadding.x * 2.0f + combo.RectFull.GetHeight();
        const hydra::ui::WidthOf notation_w = hydra::ui::measure_in_role(hydra::ui::TextRole::Notation);
        auto width_of = [&notation_w](const std::string& t) { return notation_w(t); };
        std::string long_label;
        for (int i = 0; i < 40; ++i) long_label += (i % 3 == 0 ? "2- " : "1 ");
        long_label += " (optimal)";
        IM_CHECK(width_of(long_label) > line - chrome);  // too long for the line
        const std::string shown = hydra::render::ellipsize(long_label, line - chrome, width_of);
        IM_CHECK(shown != long_label);
        IM_CHECK(width_of(shown) <= line - chrome);
        IM_CHECK(shown.size() > 3 && shown.substr(shown.size() - 3) == "\xE2\x80\xA6");
        ImFont* const notation_font = hydra::ui::font_for(hydra::ui::TextRole::Notation).font;
        IM_CHECK(notation_font->IsGlyphInFont(0x2026));  // drawn, not a "?"
        hydra::ui::remember_library_share(0.5f);
        h.app->library_ui.panel_was_open = false;
        ctx->Yield(3);
    }

    ctx->ItemClick(preview_path_combo(ctx));
    ctx->Yield(1);
    IM_CHECK(on_screen(h, allzero_label));
    // Close the list on the path already shown. (PopupCloseAll would also
    // close the Song Details modal the tab sits in.)
    ctx->ItemClick(("//$FOCUSED/" + hydra::app::path_item_id(best_label, best_index)).c_str());
    ctx->Yield(1);
    IM_CHECK(h.app->details_ui.selected_path == &best);

    // A pick of the record's second path selects that path, here and on the
    // Paths tab.
    IM_CHECK(record.all_paths().size() >= 2);
    if (record.all_paths().size() < 2) return;
    const hydra::Path* second = record.all_paths()[1];
    pick_preview_path(ctx, second);
    IM_CHECK(h.app->details_ui.selected_path == second);
    const std::string key = hydra::app::path_overlay_key(second);
    IM_CHECK(wait_until(ctx, [&] { return pc.shows_path(key); }, 30));
    ctx->ItemClick("##DetailsTabs/Paths");
    ctx->Yield(2);
    IM_CHECK(h.app->details_ui.selected_path == second);
    ctx->ItemClick("##DetailsTabs/Preview");
    ctx->Yield(2);

    // Back to the optimal path, then a pending jump to its first activation:
    // the playhead lands on the record's first activation, and the box names
    // it.
    pick_preview_path(ctx, record.all_paths().front());
    IM_CHECK(h.app->details_ui.selected_path == &best);
    IM_CHECK(wait_until(ctx, [&] { return pc.shows_path(hydra::app::path_overlay_key(&best)); }, 30));
    pc.seek_ms(0.0);
    h.app->details_ui.paths_tab.ui().preview_jump = 0;
    IM_CHECK(wait_until(ctx, [&] {
        return !h.app->details_ui.paths_tab.ui().preview_jump.has_value();
    }, 30));
    const hydra::app::PreviewActivation& first = checked_activation(ctx, best, 0);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), first.ms, 0.5);
    IM_CHECK(pc.next_act_box().shown);
    IM_CHECK_STR_EQ(pc.next_act_box().notation.c_str(), first.notation.c_str());
}

// "Preview failed: …" wraps inside the panel. On one line, a long error would
// run past the panel's edge, cut off with no way to read it.
void test_preview_error_wraps(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_preview(ctx)) return;
    auto& pc = *h.app->preview;
    // The selection now names a chart whose resolution is 0, so the Preview
    // reloads and fails. The panel prints the plain sentence for it, which
    // quotes the chart's own refusal and so runs long. (A missing file's
    // plain sentence holds no path and fits on one line, so it can't test this.)
    IM_CHECK(h.app->selected.has_value());
    if (!h.app->selected) return;
    const std::string chart = h.temp_dir + "\\notes.chart";
    {
        const std::string text = testchart::chart_text(
            testchart::section("ExpertDrums", testchart::line(0, "N 0 0")), /*resolution=*/0);
        std::FILE* f = hydra::fopen_utf8(chart, L"wb");
        IM_CHECK(f != nullptr);
        if (f == nullptr) return;
        std::fputs(text.c_str(), f);
        std::fclose(f);
    }
    h.app->selected->notespath = chart;
    h.app->selected->md5 = "0123456789abcdef0123456789abcdef";
    IM_CHECK(wait_until(ctx, [&] { return pc.has_error(); }, 30));
    ctx->Yield(3);

    // That sentence is wider than any of the song panel's windows, so on one
    // line it would overflow whichever one holds it: nothing overflowing
    // below means it wrapped.
    // The sentence on screen is the controller's error (app::plain_error's
    // sentence for the failure), shown whole: ImGui logs wrapped text as one
    // piece. The words the panel puts before it are preview_tab.cpp's own,
    // with no owner to read them from, so the check is on the sentence.
    const std::string message = pc.error();
    IM_CHECK(!message.empty());
    IM_CHECK(on_screen(h, message));
    // The sentence already quotes the chart's refusal, so no details line
    // repeats it (app::plain_error_detail; the user's rule of 2026-10-15).
    IM_CHECK(pc.error_detail().empty());
    // The song panel's windows, the message's among them.
    int checked = 0;
    int overflowing = 0;
    float widest_room = 0.0f;
    for (ImGuiWindow* w : ImGui::GetCurrentContext()->Windows) {
        if (!w->WasActive || (w->Flags & ImGuiWindowFlags_Tooltip) ||
            (w->Flags & ImGuiWindowFlags_HorizontalScrollbar) ||
            std::strstr(w->Name, "##songpanel") == nullptr)
            continue;
        ++checked;
        widest_room = (std::max)(widest_room, w->ContentRegionRect.GetWidth());
        if (w->ContentSize.x > w->ContentRegionRect.GetWidth() + 0.5f) {
            ++overflowing;
            std::fprintf(stderr, "OVERFLOW %s: content %.0f px, room %.0f px\n", w->Name,
                         w->ContentSize.x, w->ContentRegionRect.GetWidth());
        }
    }
    IM_CHECK_GT(checked, 0);
    IM_CHECK_GT(ImGui::CalcTextSize(message.c_str()).x, widest_room);
    IM_CHECK_EQ(overflowing, 0);
}

// A mode change on the open chart reloads the Preview's notes (D48, Q22) and
// keeps its place. Evans Blue - Beg charts Expert drums and no Hard, so
// picking Hard in the analysis settings must say so in the Preview, in the
// no-notes sentence analysis uses, rather than keep drawing Expert's notes.
// Back on Expert the highway returns at the same spot, paused. Pro Drums
// toggled while playing keeps playing from about where it was.
void test_preview_mode_reload(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_titled(ctx, "Beg", "Beg");
    if (ctx->IsError()) return;
    ctx->ItemClick("##DetailsTabs/Preview");
    IM_CHECK(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active(); }, 10));
    if (!h.app->preview) return;
    auto& pc = *h.app->preview;
    // A notes reload is not a load (the highway stays up), so wait on every
    // Preview thread.
    IM_CHECK(wait_until(ctx, [&] { return !pc.busy(); }, 120));
    IM_CHECK_STR_EQ(pc.error().c_str(), "");

    // A spot 30 s in, paused.
    const double spot = 30000.0;
    IM_CHECK_GT(pc.playback_end_ms(), spot);
    pc.pause();
    pc.seek_ms(spot);

    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    settings_combo_pick(ctx, "##prodrums", "##difficulty", "Hard");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_difficulty == "Hard"; }, 5));
    IM_CHECK(wait_until(ctx, [&] { return !pc.busy() && pc.has_error(); }, 120));
    const std::string no_hard =
        hydra::no_notes_message(hydra::Difficulty::Hard, h.app->settings.view_prodrums);
    IM_CHECK_STR_EQ(pc.error().c_str(), no_hard.c_str());
    ctx->Yield(2);
    IM_CHECK(visible_text(h).find(no_hard) != std::string::npos);
    IM_CHECK(!pc.playing());
    IM_CHECK_LT(std::abs(pc.position_ms() - spot), 1.0);

    settings_combo_pick(ctx, "##prodrums", "##difficulty", "Expert");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_difficulty == "Expert"; }, 5));
    IM_CHECK(wait_until(ctx, [&] { return !pc.busy() && !pc.has_error() && pc.playback_end_ms() > 0.0; },
                        120));
    IM_CHECK_STR_EQ(pc.error().c_str(), "");
    IM_CHECK(!pc.playing());
    IM_CHECK_LT(std::abs(pc.position_ms() - spot), 1.0);

    // Pro Drums off while playing: still playing, from about the same spot.
    // It plays on while the notes reload, so the playhead only moves forward,
    // and by no more than the wait.
    pc.play();
    IM_CHECK(pc.playing());
    const double before = pc.position_ms();
    IM_CHECK(h.app->settings.view_prodrums);
    ctx->ItemClick("**/##prodrums");
    IM_CHECK(!h.app->settings.view_prodrums);
    ctx->Yield(2);  // the Preview tab's next frame sees the new mode
    IM_CHECK(wait_until(ctx, [&] { return !pc.busy(); }, 120));
    IM_CHECK_STR_EQ(pc.error().c_str(), "");
    IM_CHECK(pc.playing());
    IM_CHECK_GE(pc.position_ms(), before);
    IM_CHECK_LT(pc.position_ms(), before + 10000.0);

    // Back to Pro Drums on, as the other tests expect.
    ctx->ItemClick("**/##prodrums");
    IM_CHECK(h.app->settings.view_prodrums);
    close_settings_panel(ctx);
    pc.pause();
    IM_CHECK(wait_until(ctx, [&] { return !pc.busy(); }, 120));
}

// A setting change re-analyzes the open song, and the song panel holds still
// while it does: the headline and the Preview's "Showing" row keep their
// height, drawn empty, so the tabs and the highway below them don't jump up
// and back (AppState::view_holds_space). The old score is never shown under
// the new setting. The scrubber is the row right above the highway, so its
// place is the highway's.
void test_settings_change_holds_layout(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_chart0_analyzed(ctx)) return;
    ctx->ItemClick("##DetailsTabs/Preview");
    IM_CHECK(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active(); }, 10));
    if (!h.app->preview) return;
    auto& pc = *h.app->preview;
    IM_CHECK(wait_until(ctx, [&] { return !pc.busy(); }, 120));
    IM_CHECK_STR_EQ(pc.error().c_str(), "");
    ctx->Yield(2);

    // Found by ID, so they can be read while the settings popup has the ref.
    const ImGuiID tab_id = ctx->ItemInfo("##DetailsTabs/Preview").ID;
    const ImGuiID scrub_id = ctx->ItemInfo("**/##scrub").ID;
    auto tab_y = [&] { return ctx->ItemInfo(tab_id).RectFull.Min.y; };
    auto scrub_y = [&] { return ctx->ItemInfo(scrub_id).RectFull.Min.y; };
    const float tab0 = tab_y();
    const float scrub0 = scrub_y();
    const std::string score0 = hydra::group_thousands(h.app->viewed.record->best_path().totalscore());
    IM_CHECK(on_screen(h, score0));

    // Held while it re-analyzes: the same places, no score.
    auto check_held = [&](const char* step) {
        IM_CHECK(wait_until(ctx, [&] { return h.app->view_running(); }, 5));
        ctx->Yield(3);
        IM_CHECK(h.app->viewed.state == hydra::ui::ViewedSong::State::Analyzing);
        std::fprintf(stderr, "%s: tabs y %.1f (was %.1f), scrubber y %.1f (was %.1f)\n", step,
                     tab_y(), tab0, scrub_y(), scrub0);
        IM_CHECK_EQ(tab_y(), tab0);
        IM_CHECK_EQ(scrub_y(), scrub0);
        IM_CHECK(!on_screen(h, score0));
    };
    {
        ViewGate gate;
        open_settings_panel(ctx);
        if (ctx->IsError()) return;
        const int cap = h.app->settings.sp_cap + 1;
        ctx->ItemInputValue("**/##spcap", cap);
        IM_CHECK(wait_until(ctx, [&] { return h.app->settings.sp_cap == cap; }, 5));
        check_held("SP cap");
        const bool prodrums = h.app->settings.view_prodrums;
        ctx->ItemClick("**/##prodrums");
        IM_CHECK(h.app->settings.view_prodrums != prodrums);
        check_held("Pro Drums");
        gate.open();
    }

    // Landed: the same places, and the new record's score is back.
    IM_CHECK(wait_until(ctx, [&] { return h.app->view_settled(); }, 300));
    IM_CHECK(h.app->viewed.ready());
    if (!h.app->viewed.ready()) return;
    IM_CHECK(wait_until(ctx, [&] { return !pc.busy(); }, 120));
    ctx->Yield(3);
    const std::string score1 = hydra::group_thousands(h.app->viewed.record->best_path().totalscore());
    IM_CHECK(on_screen(h, score1));
    IM_CHECK_EQ(tab_y(), tab0);
    IM_CHECK_EQ(scrub_y(), scrub0);
    close_settings_panel(ctx);
}

// The text boxes keep one size all through a path. If the next-activation box
// counted toward the shared scale with whatever line it showed, every box
// would shrink on Burnout's long second chord and grow back later. Checked at
// the default split and with the song panel at its narrowest.
void test_preview_overlay_steady(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_burnout_preview(ctx)) return;
    auto& pc = *h.app->preview;
    for (float share : {0.5f, 0.99f}) {
        hydra::ui::remember_library_share(share);
        h.app->library_ui.panel_was_open = false;
        ctx->Yield(3);
        pc.seek_ms(0.0);
        ctx->Yield(2);
        const float scale = pc.overlay_scale();
        IM_CHECK(scale >= hydra::render::kOverlayMinScale);
        std::fprintf(stderr, "library share %.2f: overlay scale %.4f\n", share, scale);
        // Every activation of the drawn path, in turn: the jump lands on the
        // record's chord, all five boxes draw, and the scale holds.
        const hydra::Path& best = h.app->viewed.record->best_path();
        const size_t count = pc.scene().activations.size();
        for (size_t i = 0; i < count; ++i) {
            IM_CHECK(pc.jump_activation(+1));
            ctx->Yield(2);
            IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), checked_activation(ctx, best, i).ms, 0.5);
            IM_CHECK_EQ(check_overlay_boxes(ctx), 5);
            if (i == 1) {
                // Burnout's long second chord, its box wrapped beside the lane.
                IM_CHECK(screenshot(ctx, share > 0.9f ? "overlay-steady-narrow.png"
                                                      : "overlay-steady-split.png"));
            }
            IM_CHECK_EQ(pc.overlay_scale(), scale);
        }
        // No activation after the last.
        IM_CHECK(!pc.jump_activation(+1));
        // Past the last activation the box is gone; the scale stays.
        pc.seek_ms(pc.playback_end_ms());
        ctx->Yield(2);
        IM_CHECK(!pc.next_act_box().shown);
        IM_CHECK_EQ(pc.overlay_scale(), scale);
    }
    hydra::ui::remember_library_share(0.5f);
    h.app->library_ui.panel_was_open = false;
    ctx->Yield(3);
}

// Activation jumps by button and key, the scrubber marks, the next-activation
// box, the SP readout, the one measure format, and the key hint.
void test_preview_activation_jumps(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_burnout_preview(ctx)) return;
    auto& pc = *h.app->preview;

    // One mark per activation of the drawn path, in order, each at its
    // record chord's share of the song's one length (song_fraction).
    const hydra::Path& best = h.app->viewed.record->best_path();
    const size_t count = best.walk_activations().size();
    const std::vector<double> marks = pc.scrub_marks();
    IM_CHECK_EQ(marks.size(), count);
    if (marks.size() != count || count == 0) return;
    for (size_t i = 0; i < count; ++i) {
        const hydra::app::PreviewActivation& act = checked_activation(ctx, best, i);
        const std::optional<double> at = hydra::app::song_fraction(act.ms, pc.playback_end_ms());
        IM_CHECK(at.has_value());
        if (at) IM_CHECK_FLOAT_NEAR_EQ(marks[i], *at, 1e-9);
        IM_CHECK(marks[i] > 0.0);
        IM_CHECK(marks[i] < 1.0);
        if (i > 0) IM_CHECK(marks[i - 1] < marks[i]);
    }

    // Before the first activation the box names the first.
    pc.seek_ms(0.0);
    IM_CHECK(pc.next_act_box().shown);
    IM_CHECK_STR_EQ(pc.next_act_box().notation.c_str(), checked_activation(ctx, best, 0).notation.c_str());
    // The box with its notation line, for a person to look at.
    ctx->Yield(2);
    IM_CHECK(screenshot(ctx, "next-act-notation.png"));

    // "Act >" lands on the first activation, "]" on each one after it, and
    // the box names the one the playhead is on.
    ctx->ItemClick("**/Act >##nextact");
    const double act1 = pc.position_ms();
    IM_CHECK_FLOAT_NEAR_EQ(act1, checked_activation(ctx, best, 0).ms, 0.5);
    for (size_t i = 1; i < count; ++i) {
        ctx->KeyPress(ImGuiKey_RightBracket);
        const hydra::app::PreviewActivation& act = checked_activation(ctx, best, i);
        IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), act.ms, 0.5);
        IM_CHECK_STR_EQ(pc.next_act_box().notation.c_str(), act.notation.c_str());
    }
    // "[" goes back one: to the one before the last, or stays on a lone one.
    ctx->KeyPress(ImGuiKey_LeftBracket);
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), checked_activation(ctx, best, count >= 2 ? count - 2 : 0).ms, 0.5);
    // Nothing before the first activation: the playhead stays.
    pc.seek_ms(act1);
    ctx->ItemClick("**/< Act##prevact");
    IM_CHECK_FLOAT_NEAR_EQ(pc.position_ms(), act1, 0.5);

    // A jump keeps playing when playing.
    ctx->ItemClick("**/Play");
    IM_CHECK(pc.playing());
    ctx->KeyPress(ImGuiKey_RightBracket);
    IM_CHECK(pc.playing());
    ctx->ItemClick("**/Pause");
    IM_CHECK(!pc.playing());

    // The readout's cap is the scene's meter cap, in sp_meter_readout's own
    // format (readout_of at any level shows the same "/cap" tail).
    const std::string readout = pc.sp_meter_readout();
    const std::string tail_of_cap = readout_of(0.0, pc.sp_meter_cap());
    const size_t slash = tail_of_cap.find('/');
    IM_CHECK(slash != std::string::npos);
    if (slash == std::string::npos) return;
    const std::string cap_tail = tail_of_cap.substr(slash);
    IM_CHECK(readout.size() > cap_tail.size());
    IM_CHECK(hydra::ends_with(readout, cap_tail));

    // The time box's end is where playback stops, and that is the song's one
    // length (the click's analysis_song_length, D255).
    IM_CHECK(h.app->viewed.song_length_ms.has_value());
    if (h.app->viewed.song_length_ms) IM_CHECK_FLOAT_NEAR_EQ(pc.playback_end_ms(), *h.app->viewed.song_length_ms, 0.001);
    pc.seek_ms(pc.playback_end_ms());
    IM_CHECK_STR_EQ(pc.time_box().position.c_str(), pc.time_box().length.c_str());
    IM_CHECK(pc.time_box().tempo.rfind("BPM ", 0) == 0);

    ctx->Yield(2);
    // The key bar: each group's action is on screen in its short form, never
    // the long one.
    const std::string text = visible_text(h);
    for (const char* action : {"Activation", "5 seconds", "10 ms", "Play/pause", "Space"})
        IM_CHECK(text.find(action) != std::string::npos);
    IM_CHECK(text.find("previous/next activation") == std::string::npos);
}

}  // namespace

const std::vector<TestEntry>& preview_tests() {
    static const std::vector<TestEntry> entries = {
        {"preview", test_preview},
        {"analyze-on-preview", test_analyze_on_preview},
        {"preview-path-overlay", test_preview_path_overlay},
        {"preview-controls", test_preview_controls},
        {"preview-drain-box", test_preview_drain_box},
        {"preview-squeeze-out-gauge", test_preview_squeeze_out_gauge},
        {"preview-solo-box", test_preview_solo_box},
        {"preview-overlay-fit", test_preview_overlay_fit},
        {"preview-buttons-keys", test_preview_buttons_keys},
        {"scrub-hold", test_scrub_hold},
        {"layout-drift", test_layout_drift},
        {"preview-path-picker", test_preview_path_picker},
        {"preview-activation-jumps", test_preview_activation_jumps},
        {"preview-error-wraps", test_preview_error_wraps},
        {"preview-overlay-steady", test_preview_overlay_steady},
        {"preview-mode-reload", test_preview_mode_reload},
        {"settings-change-holds-layout", test_settings_change_holds_layout},
    };
    return entries;
}

}  // namespace uitest
