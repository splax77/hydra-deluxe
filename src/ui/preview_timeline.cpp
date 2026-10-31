#include "ui/preview_timeline.h"

#include <algorithm>
#include <cmath>

#include "core/timing.h"  // kSpActivationBars
#include "imgui_internal.h"  // IM_PI
#include "ui/fonts.h"     // px
#include "ui/theme.h"     // kBestPathColor

namespace hydra::ui {

// ImGui keeps 2 px of padding and half a grab at each end of a float slider,
// so the ticks do too: each sits where the grab's centre does for its time.
void draw_scrub_marks(const std::vector<double>& marks) {
    if (marks.empty()) return;
    const ImVec2 mn = ImGui::GetItemRectMin();
    const ImVec2 mx = ImGui::GetItemRectMax();
    const float grab = ImGui::GetStyle().GrabMinSize;
    const float pad = 2.0f;  // ImGui's slider grab_padding, not scaled
    const float x0 = mn.x + pad + grab * 0.5f;
    const float span = (mx.x - mn.x) - 2.0f * pad - grab;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    for (double f : marks) {
        const float x = x0 + static_cast<float>(f) * span;
        dl->AddRectFilled(ImVec2(x - px(1.5f), mn.y + px(2.0f)), ImVec2(x + px(1.5f), mx.y - px(2.0f)),
                          ImGui::GetColorU32(kBestPathColor));
    }
}

// The overlay font has no star glyph, so the star is drawn as a shape. It is
// concave, so the fill goes through AddConcavePolyFilled.
void draw_star(ImDrawList* dl, ImVec2 min, float size, bool filled, ImU32 col) {
    constexpr int kPoints = 5;
    // Inner radius over outer, and the outline's width over the star's
    // height: both as the user's approved mock drew its stars.
    constexpr float kInner = 0.47f;
    constexpr float kStroke = 0.07f;
    const float r = size * 0.5f;
    const ImVec2 c(min.x + r, min.y + r);
    ImVec2 pts[kPoints * 2];
    for (int i = 0; i < kPoints * 2; ++i) {
        // Start at the top point and go clockwise, outer and inner in turn:
        // a quarter turn back from the right, then a half turn over kPoints
        // steps.
        const float a = IM_PI * (-0.5f + static_cast<float>(i) / kPoints);
        const float rad = (i % 2 == 0) ? r : r * kInner;
        pts[i] = ImVec2(c.x + std::cos(a) * rad, c.y + std::sin(a) * rad);
    }
    if (filled)
        dl->AddConcavePolyFilled(pts, kPoints * 2, col);
    else
        dl->AddPolyline(pts, kPoints * 2, col, ImDrawFlags_Closed, std::max(1.0f, size * kStroke));
}

std::string timeline_fill_id(int64_t chord_tick) { return "##fill_" + std::to_string(chord_tick); }

std::optional<TimelineLayout> timeline_layout(const app::PreviewScene& scene, double length_ms,
                                              float width, const TimelineMetrics& metrics) {
    if (!app::has_song_length(length_ms)) return std::nullopt;
    TimelineLayout out;
    out.width = width;
    out.length_ms = length_ms;
    // Every x is song_fraction's, which has a value whenever the length is usable.
    auto x_of = [&](double ms) { return static_cast<float>(*app::song_fraction(ms, length_ms)) * width; };

    float y = 0.0f;
    for (size_t r = 0; r < kTimelineRowCount; ++r) {
        out.rows[r] = {0.0f, y, width, y + metrics.row_h[r]};
        y += metrics.row_h[r];
    }

    const TimelineRect& sec = out.row(TimelineRow::Sections);
    for (size_t i = 0; i < scene.sections.size(); ++i) {
        const double end = i + 1 < scene.sections.size() ? scene.sections[i + 1].ms : length_ms;
        out.sections.push_back({scene.sections[i].name,
                                {x_of(scene.sections[i].ms), sec.y0, x_of(end), sec.y1}});
    }

    for (const app::PreviewSpan& p : scene.sp_phrases) out.phrase_x.push_back(x_of(p.start_ms));

    // The saved row runs from no bars at its bottom to the meter's cap at its top.
    const TimelineRect& saved = out.row(TimelineRow::Saved);
    const float cap = static_cast<float>(std::max(1, scene.sp_meter.cap));
    auto y_of = [&](double bars) {
        return saved.y1 - static_cast<float>(bars) / cap * (saved.y1 - saved.y0);
    };
    for (const app::SpMeterSegment& s : scene.sp_meter.segments) {
        out.saved_line.push_back({x_of(s.start_ms), y_of(s.start_bars)});
        out.saved_line.push_back({x_of(s.end_ms), y_of(s.end_bars)});
    }
    out.activate_y = y_of(kSpActivationBars);

    for (size_t i : scene.ghost_fills)
        if (i < scene.fills.size()) out.ghost_x.push_back(x_of(scene.fills[i].span.end_ms));

    const TimelineRect& acts = out.row(TimelineRow::Activations);
    for (size_t i = 0; i < scene.activations.size(); ++i) {
        const std::optional<std::pair<double, double>> w = scene.activations[i].sp_window();
        if (!w) continue;
        out.bands.push_back({i, {x_of(w->first), acts.y0, x_of(w->second), acts.y1}});
    }

    const TimelineRect& fr = out.row(TimelineRow::Fills);
    const float half = metrics.fill_size * 0.5f;
    for (size_t i = 0; i < scene.fills.size(); ++i) {
        const app::PreviewFill& f = scene.fills[i];
        TimelineFillItem item;
        item.fill = i;
        item.chord_tick = f.chord_tick;
        item.id = timeline_fill_id(f.chord_tick);
        item.state = f.state;
        item.x = x_of(f.span.end_ms);
        item.hit = {item.x - half, fr.y0, item.x + half, fr.y0 + metrics.fill_size};
        out.fills.push_back(std::move(item));
    }
    return out;
}

TimelineHit timeline_hit(const TimelineLayout& layout, float x, float y) {
    TimelineHit hit;
    for (const TimelineFillItem& f : layout.fills)
        if (f.hit.contains(x, y)) {
            hit.kind = TimelineHit::Kind::Fill;
            hit.index = f.fill;
            return hit;
        }
    for (const TimelineBand& b : layout.bands)
        if (b.rect.contains(x, y)) {
            hit.kind = TimelineHit::Kind::Band;
            hit.index = b.activation;
            return hit;
        }
    if (!(TimelineRect{0.0f, 0.0f, layout.width, layout.height()}.contains(x, y)) ||
        !(layout.width > 0.0f))
        return hit;
    // song_fraction run backwards: the share of the width is the share of the song.
    hit.kind = TimelineHit::Kind::Time;
    hit.ms = static_cast<double>(x) / static_cast<double>(layout.width) * layout.length_ms;
    return hit;
}

}  // namespace hydra::ui
