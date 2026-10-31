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

// Every size and colour below is a mockup's, the spec for this screen. Sizes
// in rem come from the second mockup,
// docs/superpowers/specs/2026-10-31-path-builder-mockup-2/mockup_template.html,
// and are drawn through rem, so they follow the user's font size (plan
// 2026-10-31-path-builder-redesign-2, Q1, and the rem-sizes answer under it).
// Sizes the mockups give in CSS pixels (borders, outlines, strokes) are drawn
// through px; where the second mockup gives none, the first mockup's,
// docs/superpowers/specs/2026-10-30-path-builder-mockup/mockup_template.html,
// stands. Text draws at the user's font size (Q7), so a mockup font size is
// not copied. Each comment names the rule or line it comes from.

constexpr ImU32 rgba(int r, int g, int b, float a) {
    return IM_COL32(r, g, b, static_cast<int>(a * 255.0f + 0.5f));
}

// `col` with the alpha `alpha_of` has.
constexpr ImU32 with_alpha_of(ImU32 col, ImU32 alpha_of) {
    return (col & ~IM_COL32_A_MASK) | (alpha_of & IM_COL32_A_MASK);
}

// A size in rem: one rem is the current font size, as the second mockup's
// `html{font-size:15px}` (line 17) makes it there.
float rem(float r) { return r * ImGui::GetFontSize(); }

// Row heights, from renderRows' `style="height:...rem"` on each `.row`
// (second mockup, lines 451 to 476).
constexpr float kAxisRowRem = 1.2f;         // line 451
constexpr float kSectionsRowRem = 1.5f;     // line 454
constexpr float kPhrasesRowRem = 1.4f;      // line 461
constexpr float kSavedRowRem = 3.9f;        // line 464
constexpr float kGhostRowRem = 2.0f;        // line 469
constexpr float kActivationsRowRem = 2.2f;  // line 473
constexpr float kFillsRowRem = 2.9f;        // line 476

// The label column: `.rlab{width:12.5rem; padding-right:.5rem}` (second
// mockup, line 61), in its `color:var(--muted)`. --muted and --dim are the
// second mockup's (line 11).
constexpr float kLabelColumnRem = 12.5f;
constexpr float kLabelPadRightRem = 0.5f;
constexpr ImU32 kMutedColor = rgba(0xa7, 0xad, 0xb4, 1.0f);  // --muted
constexpr ImU32 kDimColor = rgba(0x70, 0x75, 0x7e, 1.0f);    // --dim

// A row's top edge: `.row{border-top:1px solid #2d2f34}` (second mockup,
// line 59), none on the first (line 60).
constexpr ImU32 kRowEdgeColor = rgba(0x2d, 0x2f, 0x34, 1.0f);

// The axis labels: `.axis span{top:.1rem; color:var(--dim)}` (second mockup,
// line 85).
constexpr float kAxisLabelTopRem = 0.1f;

// Sections: `.sec{padding:.2rem .25rem}` (second mockup, line 65), .2 rem
// above the name and .25 rem left and right of it, and its
// `border-left:1px solid #3a3d44`. The shaded sections are
// `.sec.odd{background:#27292e}` (line 66), which renderRows gives the
// sections at odd places in the list (`i % 2 ? " odd" : ""`, line 458).
constexpr ImU32 kSectionShade = rgba(0x27, 0x29, 0x2e, 1.0f);
constexpr ImU32 kSectionEdge = rgba(0x3a, 0x3d, 0x44, 1.0f);
constexpr float kSectionPadTopRem = 0.2f;
constexpr float kSectionPadSideRem = 0.25f;

// The phrase stars: the second mockup's `.star{color:var(--star);
// font-size:.85rem}` (line 67, --star on line 13), the font size being the
// star's height. The saved line is drawn in the same --star.
constexpr ImU32 kStarColor = rgba(0xf2, 0xc9, 0x4c, 1.0f);  // --star
constexpr float kStarRem = 0.85f;

// The saved row (the second mockup's drawMeter): the curve 5 px in from the
// top and bottom (`y = n => H - 5 - n / CAP * (H - 10)`, line 528),
// `stroke-width="2"` (line 550); the activate line `stroke="#7c9"
// stroke-dasharray="4 4" opacity=".75"` (line 549). The caption's dash sample
// is the same #7c9 at full opacity (line 63), so both read kActivateColor.
constexpr float kSavedInset = 5.0f;
constexpr float kSavedLineW = 2.0f;
constexpr ImU32 kActivateColor = rgba(0x77, 0xcc, 0x99, 1.0f);
constexpr ImU32 kActivateLineColor = with_alpha_of(kActivateColor, rgba(0, 0, 0, 0.75f));
constexpr float kActivateDash = 4.0f;

// The saved row's caption, in the label column under the row's name (second
// mockup, renderRows, lines 464 and 465). Each caption line is
// `.rlab small{color:var(--dim); margin-top:.1rem}` (line 62). The activate
// line's sample is `.rlab .dash{width:1.4rem; border-top:2px dashed #7c9;
// vertical-align:.2rem; margin-right:.3rem}` (line 63): its bottom .2 rem
// above the text's baseline. The shading's swatch is `width:1.4rem;
// height:.6rem; background:rgba(54,208,214,.28); margin-right:.3rem;
// vertical-align:-.05rem` (line 465): its bottom .05 rem below the baseline.
constexpr float kCaptionGapRem = 0.1f;
constexpr float kCaptionMarkRem = 1.4f;     // the sample's and the swatch's width
constexpr float kCaptionMarkGapRem = 0.3f;  // between a mark and its words
constexpr float kActivateSampleW = 2.0f;
constexpr float kActivateSampleRiseRem = 0.2f;
constexpr ImU32 kEnoughSwatchColor = rgba(54, 208, 214, 0.28f);
// The shading itself: drawMeter's `fill="rgba(54,208,214,.16)"` (line 546).
constexpr ImU32 kEnoughShadeColor = rgba(54, 208, 214, 0.16f);
constexpr float kEnoughSwatchHRem = 0.6f;
constexpr float kEnoughSwatchDropRem = 0.05f;

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

// The saved row's label: its name, then the activate line's sample and its
// words, then the shading's swatch and its words, as one block centred
// between `y0` and `y1` the way the other rows' names are. Each piece of text
// wraps at `wrap_w` less any mark before it.
void draw_saved_caption(ImDrawList* dl, float x, float y0, float y1, float wrap_w) {
    ScopedFont font(TextRole::Label);
    ImFont* const label_font = font_for(TextRole::Label).font;
    const float ascent = ImGui::GetFontBaked()->Ascent;
    const float mark_w = rem(kCaptionMarkRem) + rem(kCaptionMarkGapRem);
    const std::string needed = std::to_string(kSpActivationBars) + kTimelineActivateLabelTail;
    const float name_h = ImGui::CalcTextSize(kTimelineSavedLabel, nullptr, false, wrap_w).y;
    const float needed_h = ImGui::CalcTextSize(needed.c_str(), nullptr, false, wrap_w - mark_w).y;
    const float enough_h = ImGui::CalcTextSize(kTimelineEnoughLabel, nullptr, false, wrap_w - mark_w).y;
    const float gap = rem(kCaptionGapRem);
    float y = (y0 + y1) * 0.5f - (name_h + gap + needed_h + gap + enough_h) * 0.5f;

    dl->AddText(label_font, 0.0f, ImVec2(x, y), kMutedColor, kTimelineSavedLabel, nullptr, wrap_w);
    y += name_h + gap;

    // The sample's 2 px border is the whole of its box, so its centre line is
    // half the border above the box's bottom.
    const float dash_y = y + ascent - rem(kActivateSampleRiseRem) - px(kActivateSampleW) * 0.5f;
    draw_dashed(dl, {ImVec2(x, dash_y), ImVec2(x + rem(kCaptionMarkRem), dash_y)}, kActivateColor,
                px(kActivateSampleW));
    dl->AddText(label_font, 0.0f, ImVec2(x + mark_w, y), kDimColor, needed.c_str(), nullptr, wrap_w - mark_w);
    y += needed_h + gap;

    const float swatch_y1 = y + ascent + rem(kEnoughSwatchDropRem);
    dl->AddRectFilled(ImVec2(x, swatch_y1 - rem(kEnoughSwatchHRem)), ImVec2(x + rem(kCaptionMarkRem), swatch_y1),
                      kEnoughSwatchColor);
    dl->AddText(label_font, 0.0f, ImVec2(x + mark_w, y), kDimColor, kTimelineEnoughLabel, nullptr, wrap_w - mark_w);
}

// The drawn path, bands and taken fills: --sp (kTimelinePathColor). The
// bands: `.band{top:.35rem; bottom:.35rem; border-radius:.25rem}` (second
// mockup, line 68), `.band.mine{color:#0d1418}`,
// `.band.mine.sel{outline:2px solid #fff; outline-offset:1px}`. A fill a
// caller highlights takes the same outline (Q6).
constexpr float kBandInsetRem = 0.35f;
constexpr float kBandRoundingRem = 0.25f;
constexpr ImU32 kBandTextColor = rgba(0x0d, 0x14, 0x18, 1.0f);
constexpr ImU32 kSelectedColor = rgba(0xff, 0xff, 0xff, 1.0f);
constexpr float kSelectedW = 2.0f;
constexpr float kSelectedOffset = 1.0f;

// The fills. From the second mockup: `.fill{top:1.05rem; width:1.15rem}`
// (line 72), the centre 1.05 rem down the row; the ring
// `box-shadow:0 0 0 .18rem` (line 73), .18 rem wide; the hover's
// `scale(1.25)` (line 76); can't `.fill.no{width:.8rem}` (line 79); locked
// `.fill.lock{background:#4b4f57; width:.5rem}` (line 80); the word under a
// fill `.flab{top:1.85rem}` (line 84), its top 1.85 rem down the row. From the
// first mockup: offered `.fill.use{background:var(--glow);
// box-shadow:... rgba(54,208,214,.25)}`, whose ring a tinted fill draws in
// its tint at the same alpha; can't's `border:2px solid var(--dim)`.
constexpr float kFillRem = 1.15f;
constexpr float kFillCenterRem = 1.05f;
constexpr ImU32 kOfferedColor = rgba(0x36, 0xd0, 0xd6, 1.0f);  // --glow
constexpr ImU32 kOfferedRingColor = rgba(54, 208, 214, 0.25f);
constexpr float kOfferedRingRem = 0.18f;
constexpr float kOfferedHoverScale = 1.25f;
constexpr float kCantRem = 0.8f;
constexpr float kCantBorder = 2.0f;
constexpr float kLockedRem = 0.5f;
constexpr ImU32 kLockedColor = rgba(0x4b, 0x4f, 0x57, 1.0f);
constexpr float kBestLabelTopRem = 1.85f;

// The legend's swatches: `.legend i{width:.75rem; height:.75rem;
// border-radius:50%; margin-right:.3rem}` (second mockup, line 49). Their
// colours are the fills' above; `.lg-use`'s glow is drawn as the offered
// fill's ring, `.lg-no`'s `border:2px solid var(--dim)` is kCantBorder, and
// `.lg-lock` (line 56) is the locked fill's dot.
constexpr float kSwatchRem = 0.75f;
constexpr float kSwatchGapRem = 0.3f;

// The playhead: `.playhead{width:2px; background:#fff; opacity:.6}` (second
// mockup, line 86).
constexpr float kPlayheadW = 2.0f;
constexpr ImU32 kPlayheadColor = rgba(0xff, 0xff, 0xff, 0.6f);

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

void legend_swatch(LegendSwatch kind, ImU32 col) {
    const float line = ImGui::GetTextLineHeight();
    const ImVec2 mn = ImGui::GetCursorScreenPos();
    const float size = rem(kSwatchRem);
    ImGui::Dummy(ImVec2(size + rem(kSwatchGapRem), line));
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float r = size * 0.5f;
    const ImVec2 c(mn.x + r, mn.y + line * 0.5f);
    switch (kind) {
        case LegendSwatch::Offered:
            dl->AddCircleFilled(c, r + rem(kOfferedRingRem), kOfferedRingColor);
            dl->AddCircleFilled(c, r, kOfferedColor);
            break;
        case LegendSwatch::Tinted:
            dl->AddCircleFilled(c, r + rem(kOfferedRingRem), with_alpha_of(col, kOfferedRingColor));
            dl->AddCircleFilled(c, r, col);
            break;
        case LegendSwatch::Activation:
            dl->AddCircleFilled(c, r, kTimelinePathColor);
            break;
        case LegendSwatch::Cant:
            // As on the Fills row, the border sits inside the box.
            dl->AddCircle(c, (size - px(kCantBorder)) * 0.5f, kDimColor, 0, px(kCantBorder));
            break;
        case LegendSwatch::Locked:
            // `.lg-lock` is the locked fill's own size, not the swatch's.
            dl->AddCircleFilled(c, rem(kLockedRem) * 0.5f, kLockedColor);
            break;
    }
}

void dot_item(ImU32 col, float diameter, const char* centred_text) {
    const ImVec2 mn = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(diameter, diameter));
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float r = diameter * 0.5f;
    const ImVec2 c(mn.x + r, mn.y + r);
    dl->AddCircleFilled(c, r, col);
    if (centred_text == nullptr) return;
    // The activation bands' numbers draw in Label too.
    ScopedFont font(TextRole::Label);
    const ImVec2 sz = ImGui::CalcTextSize(centred_text);
    dl->AddText(ImVec2(c.x - sz.x * 0.5f, c.y - sz.y * 0.5f), kBandTextColor, centred_text);
}

std::string timeline_fill_id(int64_t chord_tick) { return "##fill_" + std::to_string(chord_tick); }

std::optional<TimelineLayout> timeline_layout(const app::PreviewScene& scene, double length_ms,
                                              float width, const TimelineMetrics& metrics,
                                              const TimelineFillMarks& marks) {
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
    // The song's start gets no label, as the second mockup's renderRows
    // skips it (`if (s)`, line 452).
    if (width > 0.0f) {
        const double step_px = kTimelineAxisStepMs / length_ms * static_cast<double>(width);
        const double every = static_cast<double>(metrics.axis_label_gap) > step_px
                                 ? std::ceil(static_cast<double>(metrics.axis_label_gap) / step_px)
                                 : 1.0;
        for (double k = every; k * kTimelineAxisStepMs < length_ms; k += every) {
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

    // Where enough is saved: the saved line read against the activate line
    // (a smaller y is higher), from the end of the last activation's band, so
    // the phrase count is compared only where activate_y was placed. A
    // segment that crosses the line splits where its straight line meets it,
    // as it is drawn.
    float from_x = 0.0f;
    for (const TimelineBand& b : out.bands) from_x = std::max(from_x, b.rect.x1);
    for (size_t i = 0; i + 1 < out.saved_line.size(); i += 2) {
        const TimelinePoint a = out.saved_line[i], b = out.saved_line[i + 1];
        const bool a_in = a.y <= out.activate_y, b_in = b.y <= out.activate_y;
        if (!a_in && !b_in) continue;
        float x0 = a.x, x1 = b.x;
        if (a_in != b_in) {
            const float cross = a.x + (out.activate_y - a.y) / (b.y - a.y) * (b.x - a.x);
            (a_in ? x1 : x0) = cross;
        }
        x0 = std::max(x0, from_x);
        if (!(x1 > x0)) continue;
        if (!out.saved_enough.empty() && out.saved_enough.back().x1 >= x0)
            out.saved_enough.back().x1 = std::max(out.saved_enough.back().x1, x1);
        else
            out.saved_enough.push_back({x0, saved.y0, x1, saved.y1});
    }

    const TimelineRect& fr = out.row(TimelineRow::Fills);
    const float half = metrics.fill_size * 0.5f;
    auto listed = [](const std::vector<int64_t>& ticks, int64_t tick) {
        return std::find(ticks.begin(), ticks.end(), tick) != ticks.end();
    };
    const float best_w = measure(metrics.cost_width, marks.best_label);
    for (size_t i = 0; i < scene.fills.size(); ++i) {
        const app::PreviewFill& f = scene.fills[i];
        TimelineFillItem item;
        item.fill = i;
        item.chord_tick = f.chord_tick;
        item.id = timeline_fill_id(f.chord_tick);
        item.state = f.state;
        // TimelineFillLook's order: the first match wins.
        if (f.state == app::PreviewFillState::Taken)
            item.look = TimelineFillLook::Taken;
        else if (listed(marks.locked_ticks, f.chord_tick))
            item.look = TimelineFillLook::Locked;
        else if (f.state == app::PreviewFillState::Offered && !listed(marks.cant_ticks, f.chord_tick))
            item.look = TimelineFillLook::Offered;
        else
            item.look = TimelineFillLook::Cant;
        for (const TimelineFillTint& t : marks.tints)
            if (t.chord_tick == f.chord_tick) item.tint = t.col;
        item.x = x_of(f.span.end_ms);
        const float cy = fr.y0 + metrics.fill_center_y;
        item.hit = {item.x - half, cy - half, item.x + half, cy + half};

        if (listed(marks.best_ticks, f.chord_tick)) {
            TimelineBestLabel b;
            b.fill = i;
            b.col = item.tint.value_or(kOfferedColor);
            b.rect.x0 = inside_start(item.x, best_w, width);
            b.rect.x1 = b.rect.x0 + best_w;
            b.rect.y0 = fr.y0 + metrics.cost_top;
            b.rect.y1 = b.rect.y0 + metrics.cost_line_h;
            out.best_labels.push_back(b);
        }
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

TimelineMetrics timeline_metrics() {
    TimelineMetrics m;
    m.row_h[static_cast<size_t>(TimelineRow::Axis)] = rem(kAxisRowRem);
    m.row_h[static_cast<size_t>(TimelineRow::Sections)] = rem(kSectionsRowRem);
    m.row_h[static_cast<size_t>(TimelineRow::Phrases)] = rem(kPhrasesRowRem);
    m.row_h[static_cast<size_t>(TimelineRow::Saved)] = rem(kSavedRowRem);
    m.row_h[static_cast<size_t>(TimelineRow::Ghost)] = rem(kGhostRowRem);
    m.row_h[static_cast<size_t>(TimelineRow::Activations)] = rem(kActivationsRowRem);
    m.row_h[static_cast<size_t>(TimelineRow::Fills)] = rem(kFillsRowRem);
    m.fill_size = rem(kFillRem);
    m.fill_center_y = rem(kFillCenterRem);
    m.band_inset = rem(kBandInsetRem);
    m.saved_inset = px(kSavedInset);
    // A label's own width and a space, in the font the labels draw in.
    m.axis_label_gap = text_size(TextRole::TimelineAxis, "00:00 ").x;
    m.cost_top = rem(kBestLabelTopRem);
    m.cost_line_h = text_size(TextRole::TimelineCost, "0").y;
    m.section_pad = rem(kSectionPadSideRem);
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
    const float label_w = rem(kLabelColumnRem);
    const float track_w = ImGui::GetContentRegionAvail().x - label_w;
    if (!(track_w > 0.0f)) return ev;
    const std::optional<TimelineLayout> laid = timeline_layout(*in.scene, in.length_ms, track_w, timeline_metrics(), in);
    if (!laid) return ev;
    const TimelineLayout& l = *laid;
    const ImVec2 t0(origin.x + label_w, origin.y);
    auto at = [&](float x, float y) { return ImVec2(t0.x + x, t0.y + y); };
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Row edges and the label column. The saved row's label is its caption.
    const char* const names[kTimelineRowCount] = {nullptr,
                                                  kTimelineSectionsLabel,
                                                  kTimelinePhrasesLabel,
                                                  nullptr,
                                                  kTimelineGhostLabel,
                                                  kTimelineActivationsLabel,
                                                  kTimelineFillsLabel};
    const float label_wrap = label_w - rem(kLabelPadRightRem);
    for (size_t r = 0; r < kTimelineRowCount; ++r) {
        const TimelineRect& row = l.rows[r];
        if (!(row.y1 > row.y0)) continue;
        if (r > 0)
            dl->AddRectFilled(ImVec2(origin.x, t0.y + row.y0), at(l.width, row.y0 + px(1.0f)), kRowEdgeColor);
        if (static_cast<TimelineRow>(r) == TimelineRow::Saved)
            draw_saved_caption(dl, origin.x, t0.y + row.y0, t0.y + row.y1, label_wrap);
        if (names[r] == nullptr) continue;
        // The label wraps, which only the font-taking AddText does; size 0 is
        // the size ScopedFont pushed.
        ScopedFont font(TextRole::Label);
        ImFont* const label_font = font_for(TextRole::Label).font;
        const ImVec2 sz = ImGui::CalcTextSize(names[r], nullptr, false, label_wrap);
        const float y = t0.y + (row.y0 + row.y1) * 0.5f - sz.y * 0.5f;
        dl->AddText(label_font, 0.0f, ImVec2(origin.x, y), kMutedColor, names[r], nullptr, label_wrap);
    }

    // The axis's time labels, where the layout put them.
    {
        ScopedFont font(TextRole::TimelineAxis);
        const float y = l.row(TimelineRow::Axis).y0 + rem(kAxisLabelTopRem);
        for (const TimelineTick& t : l.axis_ticks) dl->AddText(at(t.text_x, y), kDimColor, t.text.c_str());
    }

    // Sections: the odd ones shaded (kSectionShade), each with its left edge
    // and its name, cut to fit by the layout.
    {
        ScopedFont font(TextRole::TimelineSection);
        for (size_t i = 0; i < l.sections.size(); ++i) {
            const TimelineRect& r = l.sections[i].rect;
            const ImVec2 mn = at(r.x0, r.y0), mx = at(r.x1, r.y1);
            if (i % 2 == 1) dl->AddRectFilled(mn, mx, kSectionShade);
            dl->AddRectFilled(mn, ImVec2(mn.x + px(1.0f), mx.y), kSectionEdge);
            dl->PushClipRect(mn, mx, true);
            dl->AddText(ImVec2(mn.x + rem(kSectionPadSideRem), mn.y + rem(kSectionPadTopRem)), kMutedColor,
                        l.sections[i].shown.c_str());
            dl->PopClipRect();
        }
    }

    // Star Power phrases: a star at each phrase's start.
    {
        const TimelineRect& row = l.row(TimelineRow::Phrases);
        const float size = rem(kStarRem);
        const float y = (row.y0 + row.y1) * 0.5f - size * 0.5f;
        for (float x : l.phrase_x) draw_star(dl, at(x - size * 0.5f, y), size, true, kStarColor);
    }

    // Star Power saved: the shading where enough is saved, the activate line,
    // then the curve on top.
    {
        for (const TimelineRect& e : l.saved_enough) dl->AddRectFilled(at(e.x0, e.y0), at(e.x1, e.y1), kEnoughShadeColor);
        const float ay = l.activate_y;
        draw_dashed(dl, {at(0.0f, ay), at(l.width, ay)}, kActivateLineColor, px(1.0f));
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
        dl->PathRect(at(g.x0 + half, g.y0 + half), at(g.x1 - half, g.y1 - half), rem(kBandRoundingRem) - half);
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
            dl->AddRectFilled(mn, mx, kTimelinePathColor, rem(kBandRoundingRem));
            if (in.selected && *in.selected == b.activation) {
                const float out = px(kSelectedOffset) + px(kSelectedW) * 0.5f;
                dl->AddRect(ImVec2(mn.x - out, mn.y - out), ImVec2(mx.x + out, mx.y + out), kSelectedColor,
                            rem(kBandRoundingRem), ImDrawFlags_None, px(kSelectedW));
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
        // A fill the caller highlights is drawn as hovered, with the selected
        // band's outline around it.
        const bool highlighted = in.highlighted && *in.highlighted == f.fill;
        float outer = 0.0f;  // the drawn mark's radius, for the outline
        switch (f.look) {
            case TimelineFillLook::Offered: {
                const float scale = hovered || highlighted ? kOfferedHoverScale : 1.0f;
                const float r = rem(kFillRem) * 0.5f * scale;
                const ImU32 col = f.tint.value_or(kOfferedColor);
                outer = r + rem(kOfferedRingRem) * scale;
                dl->AddCircleFilled(c, outer, with_alpha_of(col, kOfferedRingColor));
                dl->AddCircleFilled(c, r, col);
                break;
            }
            case TimelineFillLook::Taken:
                outer = rem(kFillRem) * 0.5f;
                dl->AddCircleFilled(c, outer, kTimelinePathColor);
                break;
            case TimelineFillLook::Locked:
                outer = rem(kLockedRem) * 0.5f;
                dl->AddCircleFilled(c, outer, kLockedColor);
                break;
            case TimelineFillLook::Cant:
                // The border sits inside the box, so its centre line is half
                // a border in from the edge.
                outer = rem(kCantRem) * 0.5f;
                dl->AddCircle(c, outer - px(kCantBorder) * 0.5f, kDimColor, 0, px(kCantBorder));
                break;
        }
        if (highlighted)
            dl->AddCircle(c, outer + px(kSelectedOffset) + px(kSelectedW) * 0.5f, kSelectedColor, 0, px(kSelectedW));
    }

    // The word under each best fill, where the layout put it.
    {
        ScopedFont font(TextRole::TimelineCost);
        for (const TimelineBestLabel& b : l.best_labels)
            dl->AddText(at(b.rect.x0, b.rect.y0), b.col, in.best_label.c_str());
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
