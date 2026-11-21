#include "ui/preview_panel.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "app/config.h"        // Settings::clamp, the volume's range
#include "app/preview_view.h"  // scrub_thumb_ms
#include "core/stars.h"        // kMaxStars
#include "imgui.h"
#include "imgui_internal.h"  // SetKeyOwner, owner-aware IsKeyPressed
#include "render/overlay_layout.h"
#include "ui/preview_controller.h"
#include "ui/preview_panel_layout.h"
#include "ui/preview_timeline.h"  // draw_scrub_marks, draw_star
#include "ui/text_role.h"
#include "ui/theme.h"
#include "ui/widgets.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace hydra::ui {

// kJumpSeconds and kStepMs (preview_panel.h) are the one home of the jump and
// step sizes; every label, tooltip and key-bar word here is built from them.
const std::string& jump_back_label() {
    static const std::string s = "-" + std::to_string(kJumpSeconds) + "s";
    return s;
}
const std::string& jump_forward_label() {
    static const std::string s = "+" + std::to_string(kJumpSeconds) + "s";
    return s;
}

namespace {

constexpr double kJumpMs = kJumpSeconds * 1000.0;

// The key bar's words and the tooltips' tails, built from kJumpSeconds and
// kStepMs.
const std::string& jump_words() {
    static const std::string s = std::to_string(kJumpSeconds) + " seconds";
    return s;
}
const std::string& step_words() {
    static const std::string s = std::to_string(kStepMs) + " ms";
    return s;
}

// The bar under the highway naming the Preview's keys: each key drawn as a
// keycap, then what it does, in the transport buttons' left-to-right order.
// A pair is back/forward. A group never splits across lines.
struct KeyHint {
    const char* keys[2];
    const char* action;
};
const KeyHint kKeyHints[] = {
    {{"[", "]"}, "Activation"},
    {{"\xE2\x86\x90", "\xE2\x86\x92"}, jump_words().c_str()},  // left and right arrows
    {{",", "."}, step_words().c_str()},
    {{"Space", nullptr}, "Play/pause"},
};

// Lay out the key bar `width` wide from the cursor and return its height.
// With `draw` false it only measures, so the highway can leave room for it
// first. Key and action text are ImGui Text (hydra_uitest reads it); the caps
// are the draw list.
float key_hints(float width, bool draw) {
    const float text_h = ImGui::GetTextLineHeight();
    const float pad_x = px(5.0f);
    const float cap_h = text_h + px(4.0f);
    const float wall = px(2.0f);            // the darker edge under a cap
    const float cap_gap = px(3.0f);         // between the two caps of a pair
    const float action_gap = px(6.0f);      // cap to its action
    const float group_gap = px(22.0f);      // between groups
    const float row_h = cap_h + wall + ImGui::GetStyle().ItemSpacing.y;
    auto cap_w = [&](const char* key) {
        return std::max(cap_h, ImGui::CalcTextSize(key).x + pad_x * 2.0f);
    };

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float x = 0.0f, y = 0.0f;
    for (const KeyHint& kh : kKeyHints) {
        float group_w = ImGui::CalcTextSize(kh.action).x + action_gap;
        for (const char* key : kh.keys)
            if (key) group_w += cap_w(key) + (key == kh.keys[0] ? 0.0f : cap_gap);
        if (x > 0.0f && !fits_in_row(x, group_w, width)) {
            x = 0.0f;
            y += row_h;
        }
        if (draw) {
            float kx = origin.x + x;
            const float top = origin.y + y;
            for (const char* key : kh.keys) {
                if (!key) continue;
                if (key != kh.keys[0]) kx += cap_gap;
                const float w = cap_w(key);
                const float r = px(3.0f);
                dl->AddRectFilled(ImVec2(kx, top + wall), ImVec2(kx + w, top + cap_h + wall),
                                  IM_COL32(20, 20, 22, 255), r);
                dl->AddRectFilled(ImVec2(kx, top), ImVec2(kx + w, top + cap_h),
                                  IM_COL32(62, 62, 66, 255), r);
                dl->AddRect(ImVec2(kx, top), ImVec2(kx + w, top + cap_h),
                            IM_COL32(110, 110, 116, 255), r);
                // The letter centred on its key cap by hand: a kept exception
                // to apply_theme's left-aligned text (ui/theme.cpp), the
                // user's answer 9.
                const float half_text = ImGui::CalcTextSize(key).x * 0.5f;
                ImGui::SetCursorScreenPos(
                    ImVec2(kx + w * 0.5f - half_text, top + (cap_h - text_h) * 0.5f));
                ImGui::TextColored(kDefaultTextColor, "%s", key);
                kx += w;
            }
            ImGui::SetCursorScreenPos(ImVec2(kx + action_gap, top + (cap_h - text_h) * 0.5f));
            ImGui::TextColored(kSubtleTextColor, "%s", kh.action);
        }
        x += group_w + group_gap;
    }
    const float height = y + row_h;
    if (draw) {
        // Leave the cursor under the bar on a real item (see end_overlay).
        ImGui::SetCursorScreenPos(origin);
        ImGui::Dummy(ImVec2(width, height - ImGui::GetStyle().ItemSpacing.y));
    }
    return height;
}

// Names a drawn overlay box as an item, so hydra_uitest can find where it
// landed by `label` ("**/##next_act_box"). It takes no room, no input and no
// keyboard focus. The label goes to the test engine the way a widget's does;
// without the engine that line does nothing.
void name_box(const char* label, ImVec2 min, ImVec2 max) {
    ImGuiContext& g = *ImGui::GetCurrentContext();
    (void)g;  // read only by the test-engine hook
    const ImGuiID id = ImGui::GetID(label);
    ImGui::ItemAdd(ImRect(min, max), id, nullptr, ImGuiItemFlags_NoNav);
    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
}

// One line of overlay text at `size`: the time box's, the drain box's, the
// next-activation box's and the gauge's "SP" label all step by it.
float line_height(float size) { return size * kOverlayLineHeight; }

// The score box's star row: kMaxStars stars, each as tall as a line of text
// at `size`, with a fifth of that between them. The overlay font has no star
// glyph, so draw_star (ui/preview_timeline) draws each one as a shape.
float star_gap(float size) { return size * 0.2f; }
float star_row_width(float size) {
    return size * static_cast<float>(kMaxStars) + star_gap(size) * (kMaxStars - 1);
}

// The one way a text box is drawn (D114 item 27): its panel at `min`, as big
// as render::measure_text_box says and rounded at the corners `flags` names,
// named `label` for hydra_uitest, then each line where
// render::text_box_line_origins puts it.
void draw_text_box(ImDrawList* dl, ImFont* font, const render::TextBox& box, ImVec2 min,
                   float inset, float corner, ImDrawFlags flags, const char* label) {
    const render::BoxExtent e = render::measure_text_box(box, inset);
    const ImVec2 max(min.x + e.w, min.y + e.h);
    dl->AddRectFilled(min, max, IM_COL32(0, 0, 0, 128), corner, flags);
    name_box(label, min, max);
    const std::vector<render::ImagePoint> at =
        render::text_box_line_origins(box, {min.x, min.y}, inset);
    for (size_t i = 0; i < box.lines.size(); ++i) {
        const render::BoxLine& l = box.lines[i];
        if (l.kind == render::BoxLine::Kind::Stars) {
            // Centred in its line, as the text lines' glyphs sit.
            const float y = at[i].y + (l.line_h - l.size) * 0.5f;
            for (int s = 0; s < kMaxStars; ++s) {
                const bool on = s < l.filled;
                draw_star(dl, ImVec2(at[i].x + (l.size + star_gap(l.size)) * s, y), l.size, on,
                          on ? l.color : l.off_color);
            }
        } else {
            dl->AddText(font, l.size, ImVec2(at[i].x, at[i].y), l.color, l.text.c_str());
        }
    }
}

// One text line of a box, measured by `text_width(size, text)` in the
// overlay font.
template <class Measure>
render::BoxLine text_line(const Measure& text_width, std::string text, float size, ImU32 color,
                          float line_h) {
    render::BoxLine l;
    l.width = text_width(size, text.c_str());
    l.text = std::move(text);
    l.size = size;
    l.color = color;
    l.line_h = line_h;
    return l;
}

// The time, score and drain boxes at one scale (finding R7.18), each
// measured by render::measure_text_box. The fit pass asks at scale 1 to
// choose the scale, the draw pass at the chosen one, so each box is described
// here only. The heights and the gap between two panels are
// render::OverlayHeights, which overlay_tops stacks; the caller sets its
// top_margin.
struct OverlayTextBoxes : render::OverlayHeights {
    float size = 0.0f;    // the time box's text size
    float inset = 0.0f;   // between every box's text and its panel's edge
    float line_h = 0.0f;  // line_height(size)
    float score_size = 0.0f;    // the running score's text size
    float score_line_h = 0.0f;  // the running score's line
    float big_size = 0.0f;      // the large type: the running score, the solo box's first line
    float big_line_h = 0.0f;    // a line of it
    render::TextBox time, score, drain;
    float time_w = 0.0f;
    float score_w = 0.0f;
    float drain_w = 0.0f;
};

// `lines` are the time box's lines, `score` the score box, `drain` the drain
// box; `text_width(size, text)` measures in the overlay font.
template <class Measure>
OverlayTextBoxes overlay_box_sizes(float scale, const render::PreviewConfig& cfg,
                                    const char* const* lines, int line_count,
                                    const hydra::app::PreviewScoreBox& score,
                                    const hydra::app::PreviewDrainBox& drain,
                                    const Measure& text_width) {
    OverlayTextBoxes s;
    s.size = px(cfg.text.time_box_size) * scale;
    s.inset = px(cfg.text.time_box_margin) * scale;
    s.gap = px(kOverlayPanelGap) * scale;
    s.line_h = line_height(s.size);
    const ImU32 white = IM_COL32(255, 255, 255, 255);
    const ImU32 grey = IM_COL32(200, 200, 200, 255);

    for (int i = 0; i < line_count; ++i)
        s.time.lines.push_back(text_line(text_width, lines[i], s.size, white, s.line_h));

    // The running score in large type, then a row of star shapes (earned
    // ones filled in the Star Power gold, the rest grey outlines), then
    // "x<mult> · combo <n>" and "star <n> in <points>" in light grey, each one
    // line at the time box's size. "Score unavailable" is at the time box's
    // size too and has no stars.
    // The large type, which the solo box's first line uses too.
    auto big_line = [](float sz) { return sz * kBigTypeLineHeight; };
    s.big_size = s.size * kBigTypeScale;
    s.big_line_h = big_line(s.big_size);
    s.score_size = score.available ? s.big_size : s.size;
    s.score_line_h = big_line(s.score_size);
    s.score.lines.push_back(text_line(text_width, score.score, s.score_size, white, s.score_line_h));
    if (score.available) {
        render::BoxLine stars;
        stars.kind = render::BoxLine::Kind::Stars;
        stars.size = s.size;
        stars.color = ImGui::GetColorU32(kStarPowerColor);
        stars.off_color = IM_COL32(138, 138, 138, 255);
        stars.filled = score.stars;
        stars.width = star_row_width(s.size);
        stars.line_h = s.line_h;
        s.score.lines.push_back(stars);
    }
    for (const std::string* l : {&score.detail, &score.next_star})
        if (!l->empty()) s.score.lines.push_back(text_line(text_width, *l, s.size, grey, s.line_h));

    // The drain box's header and detail lines in teal while SP runs on the
    // path, grey when idle; the rate line is white. Teal means SP running
    // everywhere (D48, Q26), so the text reads the same teal the floor starts
    // from (the preview config's sp_active_color). The floor draws it
    // darkened (sp_active_darken); the text uses it as is.
    const render::Color& sp_teal = cfg.hydra.sp_active_color;
    const ImU32 accent = drain.active
                             ? ImGui::GetColorU32(ImVec4(sp_teal.r, sp_teal.g, sp_teal.b, sp_teal.a))
                             : grey;
    for (const std::string* l : {&drain.header, &drain.rate, &drain.detail})
        s.drain.lines.push_back(
            text_line(text_width, *l, s.size, l == &drain.rate ? white : accent, s.line_h));

    const render::BoxExtent time = render::measure_text_box(s.time, s.inset);
    const render::BoxExtent score_e = render::measure_text_box(s.score, s.inset);
    const render::BoxExtent drain_e = render::measure_text_box(s.drain, s.inset);
    s.time_w = time.w;
    s.time_h = time.h;
    s.score_w = score_e.w;
    s.score_h = score_e.h;
    s.drain_w = drain_e.w;
    s.drain_h = drain_e.h;
    return s;
}

}  // namespace

void draw_preview_panel(PreviewController& controller, PreviewPanelState& state,
                        const std::function<void()>& above_transport) {
    PreviewController* const pc = &controller;
    state.volume_changed = false;
    state.volume_committed = false;
    state.image_click.reset();

    // The audio output device failed (PreviewController::audio_warning): the
    // chart previews muted. One line says so (D109's words) and the highway
    // below draws as usual; the device's own message is a hover away.
    if (pc->has_audio_warning()) {
        ImGui::TextColored(kWarningColor,
                           "Hydra couldn't use your audio device; the preview is muted.");
        hint(pc->audio_warning().c_str());
    }
    // The caller's rows over the transport (Hydra's chart-changed line and
    // path picker).
    if (above_transport) above_transport();

    // Row 1, the caller's: which path the overlay draws. Row 2: the
    // activation jumps, the transport buttons and the volume. Row 3: the
    // scrubber, a gold mark per activation, and the clock. The clock sits in
    // a fixed slot (see widgets.h), so nothing walks under a held mouse as
    // its digits change.
    // The jump and step buttons' words, built once from kJumpSeconds and
    // kStepMs.
    const std::string& back_jump = jump_back_label();
    const std::string& fwd_jump = jump_forward_label();
    static const std::string back_step = "< " + step_words();
    static const std::string fwd_step = step_words() + " >";
    static const std::string back_jump_tip = "Back " + jump_words() + " (Left arrow)";
    static const std::string fwd_jump_tip = "Forward " + jump_words() + " (Right arrow)";
    static const std::string back_step_tip = "Back " + step_words() + " (Comma)";
    static const std::string fwd_step_tip = "Forward " + step_words() + " (Period)";
    // Read once per frame (the controller caches it per scene); the
    // scrubber's gold ticks below draw from the same list.
    const std::vector<double>& scrub_marks = pc->scrub_marks();
    // Greyed exactly when a jump would do nothing, a load included.
    const bool no_acts = !pc->has_activations();
    // Each tooltip follows its button's disabled block, so it is not drawn in
    // the greyed-out colours, and overflow_tooltip shows it even while the
    // button is greyed out.
    begin_disabled_button(no_acts);
    if (ImGui::Button("< Act##prevact")) pc->jump_activation(-1);
    end_disabled_button(no_acts);
    overflow_tooltip("Previous activation ([)");
    ImGui::SameLine();
    begin_disabled_button(no_acts);
    if (ImGui::Button("Act >##nextact")) pc->jump_activation(+1);
    end_disabled_button(no_acts);
    overflow_tooltip("Next activation (])");
    ImGui::SameLine(0.0f, px(16.0f));
    if (ImGui::Button(back_jump.c_str())) pc->jump_ms(-kJumpMs);
    hint(back_jump_tip.c_str());
    ImGui::SameLine();
    if (ImGui::Button(back_step.c_str())) pc->step_ms(-kStepMs);
    hint(back_step_tip.c_str());
    ImGui::SameLine();
    const float play_w = std::max(button_slot_width("Play"), button_slot_width("Pause"));
    if (button_in_slot(pc->playing() ? "Pause" : "Play", play_w)) pc->toggle();
    hint("Play or pause (Space)");
    ImGui::SameLine();
    if (ImGui::Button(fwd_step.c_str())) pc->step_ms(kStepMs);
    hint(fwd_step_tip.c_str());
    ImGui::SameLine();
    if (ImGui::Button(fwd_jump.c_str())) pc->jump_ms(kJumpMs);
    hint(fwd_jump_tip.c_str());
    ImGui::SameLine(0.0f, px(16.0f));

    // Volume: the caller's setting (PreviewPanelState), which the caller
    // applies and stores. The range is the setting's (finding 72): the end
    // stops are the smallest and largest int clamped by the owner, and
    // AlwaysClamp keeps Ctrl+click typing inside them too.
    using hydra::app::Settings;
    const int vol_min = Settings::clamp(&Settings::preview_volume, INT_MIN);
    const int vol_max = Settings::clamp(&Settings::preview_volume, INT_MAX);
    ImGui::TextUnformatted("Vol");
    ImGui::SameLine();
    int volume = state.volume_pct;
    ImGui::SetNextItemWidth(px(110.0f));
    if (ImGui::SliderInt("##volume", &volume, vol_min, vol_max, "%d%%",
                         ImGuiSliderFlags_AlwaysClamp)) {
        state.volume_pct = volume;
        state.volume_changed = true;
    }
    if (ImGui::IsItemDeactivatedAfterEdit()) state.volume_committed = true;

    // The clock drives the scrubber, so a chart with no audio still scrubs.
    // The scrubber ends where playback does (PreviewController::playback_end_ms);
    // should the clock run past it between frames, the thumb waits at the
    // right end.
    const hydra::app::PreviewTimeBox box = pc->time_box();
    const double scrub_len_ms = pc->playback_end_ms();
    float pos_s =
        static_cast<float>(hydra::app::scrub_thumb_ms(pc->position_ms(), scrub_len_ms) / 1000.0);
    // A slider cannot have an empty range, so with no usable edge it spans 1 s.
    const float len_s = hydra::app::has_song_length(scrub_len_ms)
                            ? static_cast<float>(scrub_len_ms / 1000.0)
                            : 1.0f;
    // The clock's slot fits its widest form: every digit drawn as the widest one.
    std::string readout_sample = box.timestamp;
    const char widest = widest_digits(1)[0];
    for (char& c : readout_sample)
        if (c >= '0' && c <= '9') c = widest;
    float readout_w = 0.0f;
    {
        ScopedFont figure(TextRole::Figure);
        readout_w = text_slot_width(readout_sample.c_str());
    }
    ImGui::SetNextItemWidth(std::max(px(120.0f), ImGui::GetContentRegionAvail().x - readout_w -
                                                     ImGui::GetStyle().ItemSpacing.x));
    if (ImGui::SliderFloat("##scrub", &pos_s, 0.0f, len_s, ""))
        pc->seek_ms(static_cast<double>(pos_s) * 1000.0);
    // Holding the scrubber pauses playback (Onyx's rule); release resumes.
    pc->set_scrubbing(ImGui::IsItemActive());
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("Gold marks are this path's activations.");
    draw_scrub_marks(scrub_marks);
    ImGui::SameLine();
    {
        ScopedFont figure(TextRole::Figure);
        text_in_slot(box.timestamp.c_str(), readout_w);
    }
    ImGui::NewLine();  // text_in_slot leaves the cursor on its line; the highway goes below

    // Keys: Space plays or pauses, Left/Right jump kJumpSeconds, comma/period
    // step kStepMs, [ and ] jump between activations; a held arrow or
    // comma/period repeats. Not while a text field
    // has the keyboard. Keyboard navigation (on in imgui_setup.cpp) reads the
    // arrows and Space only when nobody owns them, so the Preview claims them
    // every frame it shows; a claim made this frame still holds during next
    // frame's navigation update, so even the first press lands here rather
    // than moving focus, nudging the scrubber, or pressing whichever button
    // was clicked last.
    if (!ImGui::GetIO().WantTextInput) {
        const ImGuiID keys_owner = ImGui::GetID("##preview_keys");
        ImGui::SetKeyOwner(ImGuiKey_LeftArrow, keys_owner);
        ImGui::SetKeyOwner(ImGuiKey_RightArrow, keys_owner);
        ImGui::SetKeyOwner(ImGuiKey_Space, keys_owner);
        if (ImGui::IsKeyPressed(ImGuiKey_Space, ImGuiInputFlags_None, keys_owner)) pc->toggle();
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, ImGuiInputFlags_Repeat, keys_owner))
            pc->jump_ms(-kJumpMs);
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, ImGuiInputFlags_Repeat, keys_owner))
            pc->jump_ms(kJumpMs);
        if (ImGui::IsKeyPressed(ImGuiKey_Comma, true)) pc->step_ms(-kStepMs);
        if (ImGui::IsKeyPressed(ImGuiKey_Period, true)) pc->step_ms(kStepMs);
        // [ and ] jump between the drawn path's activations; no repeat.
        if (ImGui::IsKeyPressed(ImGuiKey_LeftBracket, false)) pc->jump_activation(-1);
        if (ImGui::IsKeyPressed(ImGuiKey_RightBracket, false)) pc->jump_activation(+1);
    }

    // Highway viewport: the remaining region, less the key bar under it.
    ImVec2 avail = ImGui::GetContentRegionAvail();
    int w = static_cast<int>(avail.x);
    int h = static_cast<int>(avail.y - key_hints(avail.x, /*draw=*/false));
    ID3D11ShaderResourceView* srv = pc->render(w, h);
    if (srv != nullptr && w > 0 && h > 0) {
        ImGui::Image((ImTextureID)(intptr_t)srv,
                     ImVec2(static_cast<float>(w), static_cast<float>(h)));

        // The text overlays: the time box and score box top-left, the SP drain
        // box top-right beside the gauge, the next-activation box under
        // the drain box (D111 item 3), and the solo box lower down, against
        // the highway, during a solo. They share one scale, fitted by
        // render::overlay_scale so they sit beside the highway: their
        // configured size whenever there is room, smaller in a narrow window,
        // never below kOverlayMinScale (under that they overlap rather than
        // become unreadable). Everything is measured at scale 1 first, then
        // drawn at the fitted scale. The gauge keeps its size.
        ImFont* const font = font_for(TextRole::Overlay).font;
        const render::PreviewConfig& pcfg = pc->preview_config();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 origin = ImGui::GetItemRectMin();
        const ImVec2 img_max = ImGui::GetItemRectMax();
        // Named so a GUI test can place the highway (render::highway_span_at)
        // in screen pixels, and so a left click on it is read off the one item.
        name_box("##highway_image", origin, img_max);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            const ImVec2 m = ImGui::GetIO().MousePos;
            state.image_click = PreviewImageClick{{m.x - origin.x, m.y - origin.y}, w, h};
        }
        auto text_width = [](float sz, const char* s) {
            return measure_in_role(TextRole::Overlay, sz)(s);
        };

        // The time box's lines, the way Onyx draws its own (top-left,
        // monospace, on a translucent dark panel): the playhead's measure and
        // the one where playback ends, the tempo and signature in force, and the practice
        // section (absent on charts that have none). The clock is beside the
        // scrubber.
        const std::string where = box.position + "  of " + box.length;
        const char* lines[3];
        int line_count = 0;
        lines[line_count++] = where.c_str();
        lines[line_count++] = box.tempo.c_str();
        if (!box.section_line.empty()) lines[line_count++] = box.section_line.c_str();
        hydra::app::PreviewNextActBox next = pc->next_act_box();
        hydra::app::PreviewScoreBox score = pc->score_box();
        const bool has_gauge = pc->has_sp_gauge();
        hydra::app::PreviewDrainBox drain = pc->drain_box();
        // The drain box is shown only over a scene with a gauge: it asks the
        // same has_sp_gauge itself (app::build_drain_box).
        const bool drain_drawn = drain.shown;

        // The gauge's geometry, which is not scaled.
        const float bar_w = px(kGaugeBarWidth);
        const float gauge_inset = px(kGaugeInset);  // from the image's right edge
        const float v_margin = px(kGaugeVMargin);
        const float d_gap = px(kDrainGaugeGap);  // between the drain box and the gauge
        const float gauge_left = img_max.x - gauge_inset - bar_w;

        // Scale-1 sizes, and the extents the fit needs (image pixels).
        // Every box's top, from render::overlay_tops, at the sizes' scale.
        // The right column starts at the gauge's margin, which is not scaled.
        // `next_h` is the height the next-activation box reserves at the
        // sizes' scale; only the solo box's top reads it.
        auto tops_at = [&](const OverlayTextBoxes& s, float next_h) {
            render::OverlayHeights heights = s;
            heights.top_margin = v_margin;
            heights.next_h = next_h;
            heights.mid_row = static_cast<float>(h) * 0.5f;
            return render::overlay_tops(heights, drain_drawn, score.shown);
        };
        const OverlayTextBoxes sz1 =
            overlay_box_sizes(1.0f, pcfg, lines, line_count, score, drain, text_width);
        const render::OverlayTops tops1 = tops_at(sz1, 0.0f);
        render::OverlayBoxes fit;
        fit.left_w = score.shown ? std::max(sz1.time_w, sz1.score_w) : sz1.time_w;
        fit.left_h = tops1.left_bottom;
        // The right column's right edge, in image pixels: the drain box's, and
        // the next-activation box's under it.
        const float right_edge = gauge_left - d_gap - origin.x;
        fit.right_edge = right_edge;
        if (drain_drawn) {
            fit.right_w = sz1.drain_w;
            fit.right_h = sz1.drain_h;
            fit.right_top = tops1.drain;
        }
        auto next_top_at = [&](const OverlayTextBoxes& s) { return tops_at(s, 0.0f).next; };
        // Its lines: the header, the notation in larger type, then where it
        // is and its chord. The header's count ("2 of 3") stays on one line,
        // and the notation never breaks.
        auto width_at = [&](float sz) {
            return [&text_width, sz](const std::string& s) { return text_width(sz, s.c_str()); };
        };
        auto next_sizes = [](const OverlayTextBoxes& s) {
            return std::array<float, 3>{s.size, s.size * kNextActNotationScale, s.size};
        };
        auto next_pieces = [&](const hydra::app::PreviewNextActBox& b, const OverlayTextBoxes& s) {
            const std::array<float, 3> sizes = next_sizes(s);
            return std::vector<render::TextPiece>{
                {b.header, width_at(sizes[0]), line_height(sizes[0]), kHeaderCountWords},
                {b.notation, width_at(sizes[1]), b.notation.empty() ? 0.0f : line_height(sizes[1]),
                 render::kKeepWhole},
                {b.detail, width_at(sizes[2]), line_height(sizes[2])},
            };
        };
        // Wrapped to the room beside the highway at the box's lowest row.
        auto wrap_next = [&](const std::vector<render::TextPiece>& pieces, const OverlayTextBoxes& s) {
            return render::wrap_right_box(pcfg, w, h, right_edge, next_top_at(s), s.inset, sz1.gap,
                                          pieces);
        };
        // The box itself: each wrapped line in its piece's size and colour,
        // its number in the best-path gold the scrubber's marks use.
        auto next_box = [&](const hydra::app::PreviewNextActBox& b, const OverlayTextBoxes& s) {
            const std::vector<render::TextPiece> pieces = next_pieces(b, s);
            const render::WrappedText t = wrap_next(pieces, s);
            const std::array<float, 3> sizes = next_sizes(s);
            const ImU32 white = IM_COL32(255, 255, 255, 255);
            const ImU32 colors[3] = {ImGui::GetColorU32(kBestPathColor), white, white};
            render::TextBox box;
            for (size_t i = 0; i < pieces.size(); ++i)
                for (const std::string& l : t.lines[i])
                    box.lines.push_back(
                        text_line(text_width, l, sizes[i], colors[i], pieces[i].line_h));
            return box;
        };
        // The scale reserves the largest box this path shows, so it stays
        // the same all through playback, whichever activation is next, and
        // after the last one.
        for (const hydra::app::PreviewNextActBox& b : pc->next_act_boxes()) {
            const render::BoxExtent e = render::measure_text_box(next_box(b, sz1), sz1.inset);
            fit.next_w = std::max(fit.next_w, e.w);
            fit.next_h = std::max(fit.next_h, e.h);
        }
        fit.next_top = tops1.next;
        // The solo box: its percent or result in the running score's large
        // type, then the notes hit so far at the time box's size, both white.
        // Like the next-activation box, the scale reserves the largest box
        // any of the chart's solos shows, so it holds still through a solo.
        const hydra::app::PreviewSoloBox solo = pc->solo_box();
        auto solo_box = [&](const hydra::app::PreviewSoloBox& b, const OverlayTextBoxes& s) {
            const ImU32 white = IM_COL32(255, 255, 255, 255);
            render::TextBox box;
            box.lines.push_back(text_line(text_width, b.headline, s.big_size, white, s.big_line_h));
            if (!b.detail.empty())
                box.lines.push_back(text_line(text_width, b.detail, s.size, white, s.line_h));
            return box;
        };
        for (const hydra::app::PreviewSoloBox& b : pc->solo_boxes()) {
            const render::BoxExtent e = render::measure_text_box(solo_box(b, sz1), sz1.inset);
            fit.solo_w = std::max(fit.solo_w, e.w);
            fit.solo_h = std::max(fit.solo_h, e.h);
        }
        fit.solo_top = tops_at(sz1, fit.next_h).solo;
        fit.gap = sz1.gap;
        const float scale = render::overlay_scale(pcfg, w, h, fit);
        pc->set_overlay_scale(scale);

        const OverlayTextBoxes sz =
            overlay_box_sizes(scale, pcfg, lines, line_count, score, drain, text_width);
        // The next-activation box's reserve shrinks with the text: a box
        // wrapped at a smaller scale has the same room or more, so it is never
        // taller than this.
        const render::OverlayTops tops = tops_at(sz, fit.next_h * scale);
        const float corner = px(6.0f) * scale;

        // The time box, in the image's top-left corner: its bottom-right
        // corner rounded.
        draw_text_box(dl, font, sz.time, ImVec2(origin.x, origin.y + tops.time), sz.inset, corner,
                      ImDrawFlags_RoundCornersBottomRight, "##time_box");

        // The score box, under the time box. Absent until the chart is
        // analyzed; "Score unavailable" when the path can't be replayed to
        // its stored score. Right corners rounded, since it sits against the
        // left edge.
        if (score.shown)
            draw_text_box(dl, font, sz.score, ImVec2(origin.x, origin.y + tops.score), sz.inset,
                          corner, ImDrawFlags_RoundCornersRight, "##score_box");

        // The Star Power meter: a gauge down the image's right edge, filling
        // bottom-up as phrases are collected and draining while SP is active.
        // Hydra's own overlay, like the time box above -- not part of the Onyx
        // render. The value is the view-model's curve read at the playhead, so
        // it is anchored to the same engine truth the path overlay is.
        if (has_gauge) {
            // "SP" above the gauge in the Star Power gold, the banked bars
            // under it. Both start at the gauge's left edge (text at the
            // left, as apply_theme in ui/theme.cpp has it; the user's answer
            // 9); a readout too wide for that moves left to end inside the image.
            const float label_size = px(14.0f);
            const float label_h = line_height(label_size);
            dl->AddText(font, label_size, ImVec2(gauge_left, origin.y + v_margin),
                        ImGui::GetColorU32(kStarPowerColor), "SP");
            const std::string readout = pc->sp_meter_readout();
            const float rw = text_width(label_size, readout.c_str());
            dl->AddText(font, label_size,
                        ImVec2(std::min(gauge_left, img_max.x - px(2.0f) - rw),
                               img_max.y - v_margin - label_size),
                        IM_COL32(255, 255, 255, 255), readout.c_str());
            ImVec2 gauge_min(gauge_left, origin.y + v_margin + label_h);
            ImVec2 gauge_max(img_max.x - gauge_inset, img_max.y - v_margin - label_h);
            if (gauge_max.y > gauge_min.y) {
                dl->AddRectFilled(gauge_min, gauge_max, IM_COL32(0, 0, 0, 128), px(4.0f));
                name_box("##sp_gauge", gauge_min, gauge_max);

                // The meter floored its cap at the setting's minimum
                // (preview_view.cpp, finding 139).
                const int cap = pc->sp_meter_cap();
                float fill = static_cast<float>(pc->sp_meter_bars()) / static_cast<float>(cap);
                fill = fill < 0.0f ? 0.0f : (fill > 1.0f ? 1.0f : fill);

                const float fill_pad = px(2.0f);
                ImVec2 in_min(gauge_min.x + fill_pad, gauge_min.y + fill_pad);
                ImVec2 in_max(gauge_max.x - fill_pad, gauge_max.y - fill_pad);
                const float in_h = in_max.y - in_min.y;
                // The Star Power gold, a touch see-through.
                ImVec4 fill_color = kStarPowerColor;
                fill_color.w = kStarPowerFillAlpha;
                if (in_h > 0.0f && fill > 0.0f)
                    dl->AddRectFilled(ImVec2(in_min.x, in_max.y - in_h * fill), in_max,
                                      ImGui::GetColorU32(fill_color));
                // One line per whole-bar boundary, over the fill, so a glance
                // reads how many bars are banked and not just how full it is.
                for (int b = 1; b < cap; ++b) {
                    const float y = in_max.y - in_h * (static_cast<float>(b) /
                                                       static_cast<float>(cap));
                    dl->AddLine(ImVec2(in_min.x, y), ImVec2(in_max.x, y),
                                IM_COL32(0, 0, 0, 160), px(1.0f));
                }
            }
        }

        // The SP drain box, top-right just left of the gauge and level with
        // its top, where the highway is narrowest: how long a bar of SP lasts
        // at the playhead, then "empties in" (SP running on the path) or
        // "full meter" (if activated here). Every number is
        // build_drain_box's; its colours are overlay_box_sizes'.
        if (drain_drawn)
            draw_text_box(dl, font, sz.drain,
                          ImVec2(gauge_left - d_gap - sz.drain_w, origin.y + tops.drain), sz.inset,
                          corner, ImDrawFlags_None, "##drain_box");

        // The next activation, under the drain box with their right edges
        // lined up: its number, then its notation in larger type, then where
        // it is and its chord. Hidden past the last one. A line too wide for
        // the room beside the highway wraps at its spaces, so the box grows
        // down rather than over the lane; the notation is one line, never
        // split.
        if (next.shown) {
            const render::TextBox n_box = next_box(next, sz);
            const float next_w = render::measure_text_box(n_box, sz.inset).w;
            draw_text_box(dl, font, n_box,
                          ImVec2(origin.x + right_edge - next_w, origin.y + tops.next), sz.inset,
                          corner, ImDrawFlags_None, "##next_act_box");
        }

        // The solo box, right of the highway a little below the middle, as
        // the game places its own. Its left edge hugs the highway: the room
        // render::right_room leaves beside the railing at the lowest row the
        // scale reserves for the chart's solo boxes, where the highway is
        // widest, starts there. So the edge holds still when the box drops
        // from two lines to the one-line result. A box wider than that room
        // (the scale already at its floor) keeps its right edge on the
        // column's, as the boxes above do. Every line is build_solo_box's.
        if (solo.shown) {
            const render::TextBox s_box = solo_box(solo, sz);
            const render::BoxExtent s_ext = render::measure_text_box(s_box, sz.inset);
            const float solo_h = std::max(fit.solo_h * scale, s_ext.h);
            const float room =
                render::right_room(pcfg, w, h, right_edge, tops.solo + solo_h, sz.gap);
            const float solo_left = right_edge - std::max(room, s_ext.w);
            draw_text_box(dl, font, s_box, ImVec2(origin.x + solo_left, origin.y + tops.solo),
                          sz.inset, corner, ImDrawFlags_None, "##solo_box");
        }
    }

    // The keys, under the highway.
    key_hints(ImGui::GetContentRegionAvail().x, /*draw=*/true);
}

}  // namespace hydra::ui
