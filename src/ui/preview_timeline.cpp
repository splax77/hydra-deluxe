#include "ui/preview_timeline.h"

#include <algorithm>
#include <cmath>

#include "app/display_format.h"  // format_duration
#include "core/timing.h"  // kSpActivationBars
#include "imgui_internal.h"  // IM_PI
#include "render/overlay_layout.h"  // ellipsize
#include "ui/fonts.h"     // px
#include "ui/text_role.h"  // ScopedFont, text_size
#include "ui/theme.h"     // kBestPathColor

namespace hydra::ui {

namespace {

// Every size and colour below is the mockup's, the spec for this screen:
// docs/superpowers/specs/2026-10-30-path-builder-mockup/mockup_template.html.
// Each comment names the rule or line there it comes from. Sizes are CSS
// pixels, drawn through px.

constexpr ImU32 rgba(int r, int g, int b, float a) {
    return IM_COL32(r, g, b, static_cast<int>(a * 255.0f + 0.5f));
}

// Row heights, from renderRows' `style="height:..."` on each `.row`.
constexpr float kAxisRowH = 18.0f;
constexpr float kSectionsRowH = 24.0f;
constexpr float kPhrasesRowH = 22.0f;
constexpr float kSavedRowH = 46.0f;
constexpr float kGhostRowH = 30.0f;  // the "Hydra's best" row, line 279
constexpr float kActivationsRowH = 34.0f;
constexpr float kFillsRowH = 50.0f;

// The label column: `.rlab{width:110px; padding-right:8px; color:var(--muted)}`.
constexpr float kLabelColumnW = 110.0f;
constexpr float kLabelPadRight = 8.0f;
constexpr ImU32 kMutedColor = rgba(0xa3, 0xa9, 0xb0, 1.0f);  // --muted
constexpr ImU32 kDimColor = rgba(0x6c, 0x71, 0x7a, 1.0f);    // --dim

// A row's top edge: `.row{border-top:1px solid #2c2e33}`, none on the first.
constexpr ImU32 kRowEdgeColor = rgba(0x2c, 0x2e, 0x33, 1.0f);

// The axis labels: `.axis span{top:2px; color:var(--dim)}`.
constexpr float kAxisLabelTop = 2.0f;

// Sections: `.sec{border-left:1px solid #3a3d44; padding:4px}` and
// `.sec:nth-child(odd){background:#25272b}`.
constexpr ImU32 kSectionShade = rgba(0x25, 0x27, 0x2b, 1.0f);
constexpr ImU32 kSectionEdge = rgba(0x3a, 0x3d, 0x44, 1.0f);
constexpr float kSectionPad = 4.0f;

// The phrase stars: `.star{font-size:13px; color:var(--star)}`. The saved
// line is drawn in the same --star.
constexpr float kStarSize = 13.0f;
constexpr ImU32 kStarColor = rgba(0xf2, 0xd0, 0x6b, 1.0f);  // --star

// The saved row (drawMeter): the curve 6 px in from the top and bottom
// (`y = n => H - 6 - n / 4 * (H - 12)`), stroke-width 2; the activate line
// `stroke="#4a6" stroke-dasharray="4 4" opacity=".7"`; its label at
// `x = w - 4`, `y = y(2) - 3`, `fill="#7c9"`.
constexpr float kSavedInset = 6.0f;
constexpr float kSavedLineW = 2.0f;
constexpr ImU32 kActivateLineColor = rgba(0x44, 0xaa, 0x66, 0.7f);
constexpr float kActivateDash = 4.0f;
constexpr ImU32 kActivateLabelColor = rgba(0x77, 0xcc, 0x99, 1.0f);
constexpr float kActivateLabelRight = 4.0f;
constexpr float kActivateLabelRise = 3.0f;

// The ghost bands: `.band.ghost{border:1.5px dashed #bdbdbd}` (line 48). The
// mockup gives `dashed` no length; the user ruled it the activate line's
// dash, kActivateDash (plan, "User decisions (PV-3)"). A CSS border sits
// inside the box.
constexpr ImU32 kGhostColor = rgba(0xbd, 0xbd, 0xbd, 1.0f);
constexpr float kGhostW = 1.5f;

// Draws the line through `pts` as dashes: kActivateDash on, then kActivateDash
// off, counted along the line from its first point. The activate line and the
// ghost bands' outlines both go through here.
void draw_dashed(ImDrawList* dl, const std::vector<ImVec2>& pts, ImU32 col, float thickness) {
    const float dash = px(kActivateDash);
    if (!(dash > 0.0f)) return;
    float phase = 0.0f;  // how far into the current dash-and-gap the line is
    for (size_t i = 0; i + 1 < pts.size(); ++i) {
        const ImVec2 a = pts[i], b = pts[i + 1];
        const float len = std::hypot(b.x - a.x, b.y - a.y);
        if (!(len > 0.0f)) continue;
        const ImVec2 dir((b.x - a.x) / len, (b.y - a.y) / len);
        for (float pos = 0.0f; pos < len;) {
            const bool on = phase < dash;
            const float to_edge = (on ? dash : 2.0f * dash) - phase;
            const bool reaches = to_edge <= len - pos;
            const float step = reaches ? to_edge : len - pos;
            if (on)
                dl->AddLine(ImVec2(a.x + dir.x * pos, a.y + dir.y * pos),
                            ImVec2(a.x + dir.x * (pos + step), a.y + dir.y * (pos + step)), col, thickness);
            pos += step;
            phase = !reaches ? phase + step : (on ? dash : 0.0f);
        }
    }
}

// The drawn path, bands and taken fills: --sp. The bands:
// `.band{top:6px; bottom:6px; border-radius:4px}`, `.band.mine{color:#0d1418}`,
// `.band.mine.sel{outline:2px solid #fff; outline-offset:1px}`.
constexpr ImU32 kPathColor = rgba(0x5e, 0xc8, 0xff, 1.0f);  // --sp
constexpr float kBandInset = 6.0f;
constexpr float kBandRounding = 4.0f;
constexpr ImU32 kBandTextColor = rgba(0x0d, 0x14, 0x18, 1.0f);
constexpr ImU32 kSelectedColor = rgba(0xff, 0xff, 0xff, 1.0f);
constexpr float kSelectedW = 2.0f;
constexpr float kSelectedOffset = 1.0f;

// The fills: `.fill{top:16px; width:20px; height:20px}`; offered
// `.fill.use{background:var(--glow); box-shadow:0 0 0 3px rgba(54,208,214,.25)}`
// growing `scale(1.2)` on hover; can't
// `.fill.no{border:2px solid var(--dim); width:13px; height:13px}`; the label
// under one `.flab{top:30px}`.
constexpr float kFillSize = 20.0f;
constexpr float kFillCenterY = 16.0f;
constexpr ImU32 kOfferedColor = rgba(0x36, 0xd0, 0xd6, 1.0f);  // --glow
constexpr ImU32 kOfferedRingColor = rgba(54, 208, 214, 0.25f);
constexpr float kOfferedRing = 3.0f;
constexpr float kOfferedHoverScale = 1.2f;
constexpr float kCantSize = 13.0f;
constexpr float kCantBorder = 2.0f;
constexpr float kFillLabelTop = 30.0f;

// The legend's swatches: `.legend i{width:12px; height:12px;
// border-radius:50%; margin-right:6px}`. Their colours are the fills' above;
// `.lg-use`'s glow is drawn as the offered fill's ring, and `.lg-no`'s
// `border:2px solid var(--dim)` is kCantBorder.
constexpr float kSwatchSize = 12.0f;
constexpr float kSwatchGap = 6.0f;

// The playhead: `.playhead{width:2px; background:#fff; opacity:.55}`.
constexpr float kPlayheadW = 2.0f;
constexpr ImU32 kPlayheadColor = rgba(0xff, 0xff, 0xff, 0.55f);

// `text`'s width by `width_of`, or 0 with no measure.
float measure(const TimelineTextWidth& width_of, const std::string& text) {
    return width_of ? width_of(text) : 0.0f;
}

// Where a text `w` wide starts when centred on `center` (the mockup's
// `transform:translateX(-50%)`) but kept between 0 and `width`. Text wider
// than the timeline starts at 0.
float inside_start(float center, float w, float width) {
    return std::max(0.0f, std::min(center - w * 0.5f, width - w));
}

}  // namespace

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

void legend_swatch(LegendSwatch kind) {
    const float line = ImGui::GetTextLineHeight();
    const ImVec2 mn = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(px(kSwatchSize) + px(kSwatchGap), line));
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float r = px(kSwatchSize) * 0.5f;
    const ImVec2 c(mn.x + r, mn.y + line * 0.5f);
    switch (kind) {
        case LegendSwatch::Offered:
            dl->AddCircleFilled(c, r + px(kOfferedRing), kOfferedRingColor);
            dl->AddCircleFilled(c, r, kOfferedColor);
            break;
        case LegendSwatch::Activation:
            dl->AddCircleFilled(c, r, kPathColor);
            break;
        case LegendSwatch::Cant:
            // As on the Fills row, the border sits inside the box.
            dl->AddCircle(c, (px(kSwatchSize) - px(kCantBorder)) * 0.5f, kDimColor, 0, px(kCantBorder));
            break;
    }
}

std::string timeline_fill_id(int64_t chord_tick) { return "##fill_" + std::to_string(chord_tick); }

std::optional<TimelineLayout> timeline_layout(const app::PreviewScene& scene, double length_ms,
                                              float width, const TimelineMetrics& metrics,
                                              const std::vector<TimelineFillLabel>& labels) {
    if (!app::has_song_length(length_ms)) return std::nullopt;
    TimelineLayout out;
    out.width = width;
    out.length_ms = length_ms;
    // Every x is song_fraction's, which has a value whenever the length is usable.
    auto x_of = [&](double ms) { return static_cast<float>(*app::song_fraction(ms, length_ms)) * width; };

    // The ghost row shows only while the scene has ghost windows to put in it.
    float y = 0.0f;
    for (size_t r = 0; r < kTimelineRowCount; ++r) {
        const bool no_ghost = static_cast<TimelineRow>(r) == TimelineRow::Ghost && scene.ghost_windows.empty();
        const float h = no_ghost ? 0.0f : metrics.row_h[r];
        out.rows[r] = {0.0f, y, width, y + h};
        y += h;
    }

    // The song's length is a number read from the chart's metadata, so the
    // label count follows the width: when one step is narrower than the gap
    // the labels need, take every n-th step. n is infinite for a width of 0.
    if (width > 0.0f) {
        const double step_px = kTimelineAxisStepMs / length_ms * static_cast<double>(width);
        const double every = static_cast<double>(metrics.axis_label_gap) > step_px
                                 ? std::ceil(static_cast<double>(metrics.axis_label_gap) / step_px)
                                 : 1.0;
        for (double k = 0.0; k * kTimelineAxisStepMs < length_ms; k += every) {
            const double ms = k * kTimelineAxisStepMs;
            TimelineTick t;
            t.ms = ms;
            t.x = x_of(ms);
            t.text = app::format_duration(ms / 1000.0);
            t.text_x = inside_start(t.x, measure(metrics.axis_width, t.text), width);
            out.axis_ticks.push_back(std::move(t));
        }
    }

    const TimelineRect& sec = out.row(TimelineRow::Sections);
    for (size_t i = 0; i < scene.sections.size(); ++i) {
        if (scene.sections[i].name == kTimelineHiddenSectionName) continue;
        const double end = i + 1 < scene.sections.size() ? scene.sections[i + 1].ms : length_ms;
        TimelineSection s;
        s.name = scene.sections[i].name;
        s.rect = {x_of(scene.sections[i].ms), sec.y0, x_of(end), sec.y1};
        // The name sits section_pad in from both edges.
        s.shown = metrics.section_width
                      ? render::ellipsize(s.name, (s.rect.x1 - s.rect.x0) - 2.0f * metrics.section_pad,
                                          metrics.section_width)
                      : s.name;
        out.sections.push_back(std::move(s));
    }

    for (const app::PreviewSpan& p : scene.sp_phrases) out.phrase_x.push_back(x_of(p.start_ms));

    // The saved row runs from no bars near its bottom to the meter's cap near
    // its top, saved_inset in from each.
    const TimelineRect& saved = out.row(TimelineRow::Saved);
    const float cap = static_cast<float>(scene.sp_meter.cap);
    const float zero_y = saved.y1 - metrics.saved_inset;
    const float span_y = (saved.y1 - saved.y0) - 2.0f * metrics.saved_inset;
    auto y_of = [&](double bars) { return zero_y - static_cast<float>(bars) / cap * span_y; };
    for (const app::SpMeterSegment& s : scene.sp_meter.segments) {
        out.saved_line.push_back({x_of(s.start_ms), y_of(s.start_bars)});
        out.saved_line.push_back({x_of(s.end_ms), y_of(s.end_bars)});
    }
    out.activate_y = y_of(kSpActivationBars);

    // The ghost bands stand in from their row as the activation bands do
    // (`.band` sets both).
    const TimelineRect& ghost = out.row(TimelineRow::Ghost);
    for (const std::pair<double, double>& w : scene.ghost_windows)
        out.ghost_bands.push_back({x_of(w.first), ghost.y0 + metrics.band_inset, x_of(w.second),
                                   ghost.y1 - metrics.band_inset});

    const TimelineRect& acts = out.row(TimelineRow::Activations);
    for (size_t i = 0; i < scene.activations.size(); ++i) {
        const std::optional<std::pair<double, double>> w = scene.activations[i].sp_window();
        if (!w) continue;
        out.bands.push_back({i, {x_of(w->first), acts.y0 + metrics.band_inset, x_of(w->second),
                                 acts.y1 - metrics.band_inset}});
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
        const float cy = fr.y0 + metrics.fill_center_y;
        item.hit = {item.x - half, cy - half, item.x + half, cy + half};
        out.fills.push_back(std::move(item));
    }

    // The cost labels, each under its fill, in order along the timeline.
    for (const TimelineFillItem& f : out.fills)
        for (const TimelineFillLabel& lab : labels) {
            if (lab.chord_tick != f.chord_tick) continue;
            TimelineCostLabel c;
            c.fill = f.fill;
            c.text = lab.text;
            c.col = lab.col;
            const float w = measure(metrics.cost_width, lab.text);
            c.rect.x0 = inside_start(f.x, w, width);
            c.rect.x1 = c.rect.x0 + w;
            out.cost_labels.push_back(std::move(c));
        }
    std::stable_sort(out.cost_labels.begin(), out.cost_labels.end(),
                     [](const TimelineCostLabel& a, const TimelineCostLabel& b) { return a.rect.x0 < b.rect.x0; });
    // Where the last label on each line ends; a box that starts there or
    // before would touch it.
    std::optional<float> line_end[2];
    auto clear_of = [&](int line, float x0) { return !line_end[line] || x0 > *line_end[line]; };
    bool second_line = false;
    for (TimelineCostLabel& c : out.cost_labels) {
        if (clear_of(0, c.rect.x0)) {
            c.line = 0;
        } else if (clear_of(1, c.rect.x0)) {
            c.line = 1;
        } else {
            c.line = *line_end[0] <= *line_end[1] ? 0 : 1;
            ++out.cost_clashes;
        }
        second_line = second_line || c.line == 1;
        line_end[c.line] = c.rect.x1;
        c.rect.y0 = fr.y0 + metrics.cost_top + static_cast<float>(c.line) * metrics.cost_line_h;
        c.rect.y1 = c.rect.y0 + metrics.cost_line_h;
    }
    if (second_line) out.rows[static_cast<size_t>(TimelineRow::Fills)].y1 += metrics.cost_line_h;
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

TimelineMetrics timeline_metrics() {
    TimelineMetrics m;
    m.row_h[static_cast<size_t>(TimelineRow::Axis)] = px(kAxisRowH);
    m.row_h[static_cast<size_t>(TimelineRow::Sections)] = px(kSectionsRowH);
    m.row_h[static_cast<size_t>(TimelineRow::Phrases)] = px(kPhrasesRowH);
    m.row_h[static_cast<size_t>(TimelineRow::Saved)] = px(kSavedRowH);
    m.row_h[static_cast<size_t>(TimelineRow::Ghost)] = px(kGhostRowH);
    m.row_h[static_cast<size_t>(TimelineRow::Activations)] = px(kActivationsRowH);
    m.row_h[static_cast<size_t>(TimelineRow::Fills)] = px(kFillsRowH);
    m.fill_size = px(kFillSize);
    m.fill_center_y = px(kFillCenterY);
    m.band_inset = px(kBandInset);
    m.saved_inset = px(kSavedInset);
    // A label's own width and a space, in the font the labels draw in.
    m.axis_label_gap = text_size(TextRole::TimelineAxis, "00:00 ").x;
    m.cost_top = px(kFillLabelTop);
    m.cost_line_h = text_size(TextRole::TimelineCost, "0").y;
    m.section_pad = px(kSectionPad);
    // Each measure is made this frame, at this frame's scale.
    auto in_role = [](TextRole role) -> TimelineTextWidth {
        return [w = measure_in_role(role)](const std::string& s) { return w(s); };
    };
    m.cost_width = in_role(TextRole::TimelineCost);
    m.section_width = in_role(TextRole::TimelineSection);
    m.axis_width = in_role(TextRole::TimelineAxis);
    return m;
}

TimelineEvents PreviewTimeline::draw(const TimelineInput& in) const {
    TimelineEvents ev;
    if (in.scene == nullptr) return ev;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float label_w = px(kLabelColumnW);
    const float track_w = ImGui::GetContentRegionAvail().x - label_w;
    if (!(track_w > 0.0f)) return ev;
    const std::optional<TimelineLayout> laid =
        timeline_layout(*in.scene, in.length_ms, track_w, timeline_metrics(), in.labels);
    if (!laid) return ev;
    const TimelineLayout& l = *laid;
    const ImVec2 t0(origin.x + label_w, origin.y);
    auto at = [&](float x, float y) { return ImVec2(t0.x + x, t0.y + y); };
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Row edges and the label column.
    const char* const names[kTimelineRowCount] = {nullptr,
                                                  kTimelineSectionsLabel,
                                                  kTimelinePhrasesLabel,
                                                  kTimelineSavedLabel,
                                                  kTimelineGhostLabel,
                                                  kTimelineActivationsLabel,
                                                  kTimelineFillsLabel};
    const float label_wrap = label_w - px(kLabelPadRight);
    for (size_t r = 0; r < kTimelineRowCount; ++r) {
        const TimelineRect& row = l.rows[r];
        if (!(row.y1 > row.y0)) continue;
        if (r > 0)
            dl->AddRectFilled(ImVec2(origin.x, t0.y + row.y0), at(l.width, row.y0 + px(1.0f)), kRowEdgeColor);
        if (names[r] == nullptr) continue;
        // The label wraps, which only the font-taking AddText does; size 0 is
        // the size ScopedFont pushed.
        ScopedFont font(TextRole::Label);
        ImFont* const label_font = font_for(TextRole::Label).font;
        const bool sub = static_cast<TimelineRow>(r) == TimelineRow::Fills;
        const ImVec2 main_sz = ImGui::CalcTextSize(names[r], nullptr, false, label_wrap);
        const ImVec2 sub_sz = sub ? ImGui::CalcTextSize(kTimelineFillsSubLabel, nullptr, false, label_wrap) : ImVec2();
        float y = t0.y + (row.y0 + row.y1) * 0.5f - (main_sz.y + sub_sz.y) * 0.5f;
        dl->AddText(label_font, 0.0f, ImVec2(origin.x, y), kMutedColor, names[r], nullptr, label_wrap);
        if (sub)
            dl->AddText(label_font, 0.0f, ImVec2(origin.x, y + main_sz.y), kDimColor, kTimelineFillsSubLabel,
                        nullptr, label_wrap);
    }

    // The axis's time labels, where the layout put them.
    {
        ScopedFont font(TextRole::TimelineAxis);
        const float y = l.row(TimelineRow::Axis).y0 + px(kAxisLabelTop);
        for (const TimelineTick& t : l.axis_ticks) dl->AddText(at(t.text_x, y), kDimColor, t.text.c_str());
    }

    // Sections: every other one shaded, each with its left edge and its name,
    // cut to fit by the layout.
    {
        ScopedFont font(TextRole::TimelineSection);
        for (size_t i = 0; i < l.sections.size(); ++i) {
            const TimelineRect& r = l.sections[i].rect;
            const ImVec2 mn = at(r.x0, r.y0), mx = at(r.x1, r.y1);
            if (i % 2 == 0) dl->AddRectFilled(mn, mx, kSectionShade);
            dl->AddRectFilled(mn, ImVec2(mn.x + px(1.0f), mx.y), kSectionEdge);
            dl->PushClipRect(mn, mx, true);
            dl->AddText(ImVec2(mn.x + px(kSectionPad), mn.y + px(kSectionPad)), kMutedColor,
                        l.sections[i].shown.c_str());
            dl->PopClipRect();
        }
    }

    // Star Power phrases: a star at each phrase's start.
    {
        const TimelineRect& row = l.row(TimelineRow::Phrases);
        const float size = px(kStarSize);
        const float y = (row.y0 + row.y1) * 0.5f - size * 0.5f;
        for (float x : l.phrase_x) draw_star(dl, at(x - size * 0.5f, y), size, true, kStarColor);
    }

    // Star Power saved: the activate line and its label, then the curve on top.
    {
        const float ay = l.activate_y;
        draw_dashed(dl, {at(0.0f, ay), at(l.width, ay)}, kActivateLineColor, px(1.0f));
        ScopedFont font(TextRole::Label);
        const std::string label = std::to_string(kSpActivationBars) + kTimelineActivateLabelTail;
        const ImVec2 sz = ImGui::CalcTextSize(label.c_str());
        dl->AddText(at(l.width - px(kActivateLabelRight) - sz.x, ay - px(kActivateLabelRise) - sz.y),
                    kActivateLabelColor, label.c_str());
        std::vector<ImVec2> pts;
        pts.reserve(l.saved_line.size());
        for (const TimelinePoint& p : l.saved_line) pts.push_back(at(p.x, p.y));
        if (pts.size() >= 2)
            dl->AddPolyline(pts.data(), static_cast<int>(pts.size()), kStarColor, ImDrawFlags_None, px(kSavedLineW));
    }

    // The ghost bands: a dashed rounded outline, no fill. The stroke's centre
    // line is half its width inside the band's edge, as a CSS border is, and
    // the corners are the drawn bands' rounding less that half width.
    for (const TimelineRect& g : l.ghost_bands) {
        const float half = px(kGhostW) * 0.5f;
        dl->PathRect(at(g.x0 + half, g.y0 + half), at(g.x1 - half, g.y1 - half), px(kBandRounding) - half);
        std::vector<ImVec2> outline(dl->_Path.Data, dl->_Path.Data + dl->_Path.Size);
        dl->PathClear();
        if (outline.empty()) continue;
        outline.push_back(outline.front());
        draw_dashed(dl, outline, kGhostColor, px(kGhostW));
    }

    // The drawn path's activations: numbered bands, the selected one outlined.
    {
        ScopedFont font(TextRole::Label);
        for (const TimelineBand& b : l.bands) {
            const ImVec2 mn = at(b.rect.x0, b.rect.y0), mx = at(b.rect.x1, b.rect.y1);
            dl->AddRectFilled(mn, mx, kPathColor, px(kBandRounding));
            if (in.selected && *in.selected == b.activation) {
                const float out = px(kSelectedOffset) + px(kSelectedW) * 0.5f;
                dl->AddRect(ImVec2(mn.x - out, mn.y - out), ImVec2(mx.x + out, mx.y + out), kSelectedColor,
                            px(kBandRounding), ImDrawFlags_None, px(kSelectedW));
            }
            const std::string n = std::to_string(b.activation + 1);
            const ImVec2 sz = ImGui::CalcTextSize(n.c_str());
            dl->PushClipRect(mn, mx, true);
            dl->AddText(ImVec2((mn.x + mx.x - sz.x) * 0.5f, (mn.y + mx.y - sz.y) * 0.5f), kBandTextColor, n.c_str());
            dl->PopClipRect();
        }
    }

    // The fills, each its own item, before the rows' item so a fill takes the
    // hover over the row under it.
    for (const TimelineFillItem& f : l.fills) {
        const ImVec2 mn = at(f.hit.x0, f.hit.y0);
        ImGui::SetCursorScreenPos(mn);
        if (ImGui::InvisibleButton(f.id.c_str(), ImVec2(f.hit.x1 - f.hit.x0, f.hit.y1 - f.hit.y0)))
            ev.clicked_fill = f.fill;
        const bool hovered = ImGui::IsItemHovered();
        if (hovered) ev.hovered_fill = f.fill;
        const ImVec2 c = at(f.x, (f.hit.y0 + f.hit.y1) * 0.5f);
        const bool cant = std::find(in.cant_ticks.begin(), in.cant_ticks.end(), f.chord_tick) != in.cant_ticks.end();
        if (f.state == app::PreviewFillState::Offered && !cant) {
            const float scale = hovered ? kOfferedHoverScale : 1.0f;
            const float r = px(kFillSize) * 0.5f * scale;
            dl->AddCircleFilled(c, r + px(kOfferedRing) * scale, kOfferedRingColor);
            dl->AddCircleFilled(c, r, kOfferedColor);
        } else if (f.state == app::PreviewFillState::Taken) {
            dl->AddCircleFilled(c, px(kFillSize) * 0.5f, kPathColor);
        } else {
            // The border sits inside the 13 px box, so its centre line is half
            // a border in from the edge.
            dl->AddCircle(c, (px(kCantSize) - px(kCantBorder)) * 0.5f, kDimColor, 0, px(kCantBorder));
        }
    }

    // The cost labels, on the lines and at the places the layout gave them.
    {
        ScopedFont font(TextRole::TimelineCost);
        for (const TimelineCostLabel& c : l.cost_labels)
            dl->AddText(at(c.rect.x0, c.rect.y0), c.col, c.text.c_str());
    }

    // The rows' own item: a click off every fill lands on a band or a moment.
    ImGui::SetCursorScreenPos(t0);
    if (ImGui::InvisibleButton("##timeline", ImVec2(l.width, l.height()))) {
        const ImVec2 m = ImGui::GetIO().MousePos;
        const TimelineHit hit = timeline_hit(l, m.x - t0.x, m.y - t0.y);
        if (hit.kind == TimelineHit::Kind::Band) ev.clicked_band = hit.index;
        if (hit.kind == TimelineHit::Kind::Time) ev.clicked_ms = hit.ms;
    }

    // The playhead, over every row.
    if (const std::optional<double> f = app::song_fraction(in.position_ms, in.length_ms)) {
        const float x = static_cast<float>(*f) * l.width;
        dl->AddRectFilled(at(x, 0.0f), at(x + px(kPlayheadW), l.height()), kPlayheadColor);
    }
    return ev;
}

}  // namespace hydra::ui
