#include "ui/preview_timeline.h"

#include <algorithm>
#include <cmath>

#include "app/display_format.h"  // format_duration
#include "core/timing.h"  // kSpActivationBars
#include "imgui_internal.h"  // IM_PI
#include "render/overlay_layout.h"  // ellipsize
#include "ui/fonts.h"     // px, rem
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

// The GUI tests read the screen from ImGui's text log, which hears only
// ImGui's own text calls, so each word the draw list draws is told to it here
// at the place it was drawn.
void log_text(ImVec2 at, const char* text) {
    if (ImGui::GetCurrentContext()->LogEnabled) ImGui::LogRenderedText(&at, text);
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
    log_text(ImVec2(x, y), kTimelineSavedLabel);
    y += name_h + gap;

    // The sample's 2 px border is the whole of its box, so its centre line is
    // half the border above the box's bottom.
    const float dash_y = y + ascent - rem(kActivateSampleRiseRem) - px(kActivateSampleW) * 0.5f;
    draw_dashed(dl, {ImVec2(x, dash_y), ImVec2(x + rem(kCaptionMarkRem), dash_y)}, kActivateColor,
                px(kActivateSampleW));
    dl->AddText(label_font, 0.0f, ImVec2(x + mark_w, y), kDimColor, needed.c_str(), nullptr, wrap_w - mark_w);
    log_text(ImVec2(x + mark_w, y), needed.c_str());
    y += needed_h + gap;

    const float swatch_y1 = y + ascent + rem(kEnoughSwatchDropRem);
    dl->AddRectFilled(ImVec2(x, swatch_y1 - rem(kEnoughSwatchHRem)), ImVec2(x + rem(kCaptionMarkRem), swatch_y1),
                      kEnoughSwatchColor);
    dl->AddText(label_font, 0.0f, ImVec2(x + mark_w, y), kDimColor, kTimelineEnoughLabel, nullptr, wrap_w - mark_w);
    log_text(ImVec2(x + mark_w, y), kTimelineEnoughLabel);
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

// One section the Section row shows: its index in the scene's
// sections, the stretch until the next section (or the song's end), and its
// shade. The hidden section is left out, and the shade alternates over the
// sections shown (TimelineSection says why).
struct ShownSection {
    size_t index = 0;
    double start_ms = 0.0;
    double end_ms = 0.0;
    bool shaded = false;
};

std::vector<ShownSection> shown_sections(const app::PreviewScene& scene, double length_ms) {
    std::vector<ShownSection> out;
    size_t shown = 0;
    for (size_t i = 0; i < scene.sections.size(); ++i) {
        if (scene.sections[i].name == kTimelineHiddenSectionName) continue;
        const double end = i + 1 < scene.sections.size() ? scene.sections[i + 1].ms : length_ms;
        out.push_back({i, scene.sections[i].ms, end, shown++ % 2 == 1});
    }
    return out;
}

bool listed(const std::vector<int64_t>& ticks, int64_t tick) {
    return std::find(ticks.begin(), ticks.end(), tick) != ticks.end();
}

// The look `marks` give the fill `f`, in TimelineFillLook's order: the first
// match wins.
TimelineFillLook fill_look(const app::PreviewFill& f, const TimelineFillMarks& marks) {
    if (f.state == app::PreviewFillState::Taken) return TimelineFillLook::Taken;
    if (listed(marks.locked_ticks, f.chord_tick)) return TimelineFillLook::Locked;
    if (f.state == app::PreviewFillState::Offered && !listed(marks.cant_ticks, f.chord_tick))
        return TimelineFillLook::Offered;
    return TimelineFillLook::Cant;
}

// The caller's tint for the fill on `chord_tick`, the last one given.
std::optional<ImU32> fill_tint(const TimelineFillMarks& marks, int64_t chord_tick) {
    std::optional<ImU32> tint;
    for (const TimelineFillTint& t : marks.tints)
        if (t.chord_tick == chord_tick) tint = t.col;
    return tint;
}

}  // namespace

double timeline_fill_ms(const app::PreviewFill& fill) { return fill.span.end_ms; }

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

float legend_swatch_width() { return rem(kSwatchRem) + rem(kSwatchGapRem); }

void legend_swatch(LegendSwatch kind, ImU32 col) {
    const float line = ImGui::GetTextLineHeight();
    const ImVec2 mn = ImGui::GetCursorScreenPos();
    const float size = rem(kSwatchRem);
    ImGui::Dummy(ImVec2(legend_swatch_width(), line));
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
                                              TimelineView view, float width,
                                              const TimelineMetrics& metrics,
                                              const TimelineFillMarks& marks) {
    if (!app::has_song_length(length_ms) || !(view.span_ms > 0.0)) return std::nullopt;
    TimelineLayout out;
    out.width = width;
    out.length_ms = length_ms;
    out.view = view;
    // Every x is the view's fraction, which is not held to 0..1, so a mark
    // outside the window lands outside the track.
    auto x_of = [&](double ms) { return static_cast<float>(fraction(view, ms)) * width; };

    // The ghost row shows only while the scene has ghost windows to put in it.
    float y = 0.0f;
    for (size_t r = 0; r < kTimelineRowCount; ++r) {
        const bool no_ghost = static_cast<TimelineRow>(r) == TimelineRow::Ghost && scene.ghost_windows.empty();
        const float h = no_ghost ? 0.0f : metrics.row_h[r];
        out.rows[r] = {0.0f, y, width, y + h};
        y += h;
    }

    // The window's span can come from a song length read from the chart's
    // metadata, so the label count follows the width: when one step is
    // narrower than the gap the labels need, take every n-th step, counted
    // from the song's start so a label keeps its moment as the window moves.
    // The song's start gets no label, as the third mockup's renderRows skips
    // it (`if (s && ...)`, line 548). Only the steps inside the window get
    // one: the mockup's `vis` also takes steps just past either edge, but
    // inside_start would pull such a label in over a moment it does not name.
    if (width > 0.0f) {
        const double step_px = kTimelineAxisStepMs / view.span_ms * static_cast<double>(width);
        const double every = static_cast<double>(metrics.axis_label_gap) > step_px
                                 ? std::ceil(static_cast<double>(metrics.axis_label_gap) / step_px)
                                 : 1.0;
        // The first labelled step at or after the window's start.
        const double first =
            std::max(every, std::ceil(view.start_ms / kTimelineAxisStepMs / every) * every);
        for (double k = first; k * kTimelineAxisStepMs < length_ms; k += every) {
            const double ms = k * kTimelineAxisStepMs;
            if (!in_view(view, ms)) break;  // past the window's end
            TimelineTick t;
            t.ms = ms;
            t.x = x_of(ms);
            t.text = app::format_duration(ms / 1000.0);
            t.text_x = inside_start(t.x, measure(metrics.axis_width, t.text), width);
            out.axis_ticks.push_back(std::move(t));
        }
    }

    const TimelineRect& sec = out.row(TimelineRow::Sections);
    for (const ShownSection& shown : shown_sections(scene, length_ms)) {
        // A section wholly before or after the window is left out (line 556).
        if (fraction(view, shown.end_ms) < 0.0 || fraction(view, shown.start_ms) > 1.0) continue;
        TimelineSection s;
        s.name = scene.sections[shown.index].name;
        s.shaded = shown.shaded;
        s.rect = {x_of(shown.start_ms), sec.y0, x_of(shown.end_ms), sec.y1};
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
    float from_x = x_of(0.0);
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
    const float best_w = measure(metrics.cost_width, marks.best_label);
    for (size_t i = 0; i < scene.fills.size(); ++i) {
        const app::PreviewFill& f = scene.fills[i];
        // A fill outside the window gets no item and no word (line 578).
        if (!in_view(view, timeline_fill_ms(f))) continue;
        TimelineFillItem item;
        item.fill = i;
        item.chord_tick = f.chord_tick;
        item.id = timeline_fill_id(f.chord_tick);
        item.state = f.state;
        item.look = fill_look(f, marks);
        item.tint = fill_tint(marks, f.chord_tick);
        item.x = x_of(timeline_fill_ms(f));
        item.y = fr.y0 + metrics.fill_center_y;
        // The hit box's sides are set below, once every fill in view is placed.
        item.hit = {item.x, fr.y0, item.x, fr.y1};

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

    // Each fill's hit box reaches halfway to its neighbours in x, the first
    // from the row's left edge and the last to its right edge, so the boxes
    // tile the row and a point picks the nearest fill (the third mockup's
    // `nearest`, lines 599 to 608). Fills are taken in x order, whatever
    // order the scene lists them in.
    std::vector<TimelineFillItem*> by_x;
    by_x.reserve(out.fills.size());
    for (TimelineFillItem& item : out.fills) by_x.push_back(&item);
    std::stable_sort(by_x.begin(), by_x.end(),
                     [](const TimelineFillItem* a, const TimelineFillItem* b) { return a->x < b->x; });
    for (size_t k = 0; k < by_x.size(); ++k) {
        TimelineFillItem& item = *by_x[k];
        item.hit.x0 = k == 0 ? 0.0f : (by_x[k - 1]->x + item.x) * 0.5f;
        item.hit.x1 = k + 1 == by_x.size() ? width : (item.x + by_x[k + 1]->x) * 0.5f;
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
    if (layout.row(TimelineRow::Fills).contains(x, y)) return hit;
    // The share of the width is the share of the window.
    hit.kind = TimelineHit::Kind::Time;
    hit.ms = ms_at(layout.view, static_cast<double>(x) / static_cast<double>(layout.width));
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

namespace {

// The scrollbar, from the third mockup,
// docs/superpowers/specs/2026-10-31-path-builder-mockup-3/mockup_template.html.
// --line, --text and --muted are its line 11.
constexpr ImU32 kLineColor = rgba(0x3a, 0x3d, 0x44, 1.0f);  // --line
constexpr ImU32 kTextColor = rgba(0xe8, 0xe8, 0xe8, 1.0f);  // --text
constexpr float kBorderW = 1.0f;  // `border:1px solid var(--line)` on the buttons and bar

// The scrollbar's row: `.sbrow{gap:.3rem; margin-top:.3rem}` (line 114), its
// label in --dim (line 115). The buttons: `.sbbtn{background:#30333a;
// color:var(--text); border-radius:.2rem; width:1.8rem; height:1.3rem;
// padding:0}` (line 116), `.sbbtn:hover{background:#3a3e46}` (line 117). The
// bar: `.sb{height:1.1rem; background:#1e2024; border-radius:.55rem}`
// (line 118). The thumb: `.thumb{top:1px; bottom:1px; background:#5a5f69;
// border-radius:.5rem; min-width:1.2rem}` (line 119),
// `.thumb:hover{background:#7a808b}` (line 120). The readout:
// `.sbtime{color:var(--muted); min-width:9.5rem; text-align:right}`
// (line 121).
constexpr float kScrollGapRem = 0.3f;
constexpr ImU32 kScrollButtonColor = rgba(0x30, 0x33, 0x3a, 1.0f);
constexpr ImU32 kScrollButtonHoverColor = rgba(0x3a, 0x3e, 0x46, 1.0f);
constexpr float kScrollButtonRoundingRem = 0.2f;
constexpr float kScrollButtonWRem = 1.8f;
constexpr float kScrollButtonHRem = 1.3f;
constexpr float kScrollBarRem = 1.1f;
constexpr ImU32 kScrollBarColor = rgba(0x1e, 0x20, 0x24, 1.0f);
constexpr float kScrollBarRoundingRem = 0.55f;
constexpr ImU32 kThumbColor = rgba(0x5a, 0x5f, 0x69, 1.0f);
constexpr ImU32 kThumbHoverColor = rgba(0x7a, 0x80, 0x8b, 1.0f);
constexpr float kThumbInset = 1.0f;
constexpr float kThumbRoundingRem = 0.5f;
constexpr float kThumbMinRem = 1.2f;
constexpr float kReadoutMinRem = 9.5f;

// A press on the scrollbar's bar `x` pixels into its `width`, which shows the
// whole song: a press inside [grab_x0, grab_x1] (the thumb) starts a drag
// that keeps the moment under the pointer at the same place in the window; a
// press anywhere else gives `on_press`'s window. While the drag lasts, each
// move of the pointer gives the window it asks for. The point is not held
// inside the song (the scrollbar's uAt, line 702). The drag is kept in
// ImGui's state storage under the item's id, so the timeline keeps no state
// of its own.
template <typename OnPress>
std::optional<TimelineView> press_or_drag(TimelineView view, double length_ms, float x, float width, float grab_x0,
                                          float grab_x1, OnPress on_press) {
    const ImGuiID id = ImGui::GetItemID();
    ImGuiStorage* store = ImGui::GetStateStorage();
    const ImGuiID drag_key = ImHashStr("drag", 0, id);
    const ImGuiID grab_key = ImHashStr("grab", 0, id);
    const double share = width > 0.0f ? static_cast<double>(x) / static_cast<double>(width) : 0.0;
    const double at = ms_at(whole_song_view(length_ms), share);
    if (ImGui::IsItemActivated()) {
        const bool on_grab = x >= grab_x0 && x <= grab_x1;
        store->SetBool(drag_key, on_grab);
        if (!on_grab) return on_press(at, share);
        store->SetFloat(grab_key, static_cast<float>(at - view.start_ms));
        return std::nullopt;
    }
    if (ImGui::IsItemActive() && store->GetBool(drag_key) && ImGui::GetIO().MouseDelta.x != 0.0f) {
        view.start_ms = at - static_cast<double>(store->GetFloat(grab_key));
        return clamp(view, length_ms);
    }
    return std::nullopt;
}

// The bar's border: kBorderW wide in kLineColor, inside the box from `b0` to
// `b1` as a CSS border is, so its centre line is half a border in.
void draw_line_border(ImDrawList* dl, ImVec2 b0, ImVec2 b1, float rounding) {
    const float border = px(kBorderW);
    dl->AddRect(ImVec2(b0.x + border * 0.5f, b0.y + border * 0.5f), ImVec2(b1.x - border * 0.5f, b1.y - border * 0.5f),
                kLineColor, rounding, ImDrawFlags_None, border);
}

// A label column's words, `text` in `col` in the Label role, wrapping at
// `wrap_w`, as one block centred between `y0` and `y1`. The wrapping is the
// font-taking AddText's; size 0 is the size ScopedFont pushed. Returns the
// block's height; with `y1` below `y0` it only measures. Every row's name and
// the scrollbar's are drawn through here.
float label_block(ImDrawList* dl, float x, float y0, float y1, float wrap_w, const char* text, ImU32 col) {
    ScopedFont font(TextRole::Label);
    ImFont* const label_font = font_for(TextRole::Label).font;
    const float h = ImGui::CalcTextSize(text, nullptr, false, wrap_w).y;
    if (y1 < y0) return h;
    const float y = (y0 + y1) * 0.5f - h * 0.5f;
    dl->AddText(label_font, 0.0f, ImVec2(x, y), col, text, nullptr, wrap_w);
    log_text(ImVec2(x, y), text);
    return h;
}

// One of the scrollbar's two buttons at `at`, `size` big, with its hover
// hint, in the mockup's colours. True when pressed.
bool scroll_button(ImVec2 at, ImVec2 size, const char* label, const char* hint) {
    ImGui::SetCursorScreenPos(at);
    ImGui::PushStyleColor(ImGuiCol_Button, kScrollButtonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kScrollButtonHoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, kScrollButtonHoverColor);
    ImGui::PushStyleColor(ImGuiCol_Border, kLineColor);
    ImGui::PushStyleColor(ImGuiCol_Text, kTextColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, px(kBorderW));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rem(kScrollButtonRoundingRem));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    bool pressed = false;
    {
        ScopedFont font(TextRole::Label);
        pressed = ImGui::Button(label, size);
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(5);
    ImGui::SetItemTooltip("%s", hint);
    return pressed;
}

// The scrollbar's row at `top`, `avail_w` wide: its label, the back button,
// the bar, the forward button and the readout, each centred in the row. A
// press, a drag or a button goes into `ev`. Returns the row's height.
float draw_scrollbar_row(ImVec2 top, float avail_w, float label_w, float label_wrap, TimelineView view,
                         double length_ms, TimelineEvents& ev) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float border = px(kBorderW);
    const float gap = rem(kScrollGapRem);
    const ImVec2 button(rem(kScrollButtonWRem), rem(kScrollButtonHRem));
    const std::string text = readout(view, length_ms);
    const ImVec2 text_sz = text_size(TextRole::Figure, text.c_str());
    const float readout_w = std::max(rem(kReadoutMinRem), text_sz.x);
    const float bar_w = avail_w - label_w - 4.0f * gap - 2.0f * button.x - readout_w;
    const float bar_h = rem(kScrollBarRem);
    const float label_h = label_block(dl, 0.0f, 0.0f, -1.0f, label_wrap, kTimelineScrollLabel, kDimColor);
    const float row_h = std::max({button.y, bar_h, label_h, text_sz.y});
    const float mid = top.y + row_h * 0.5f;
    label_block(dl, top.x, top.y, top.y + row_h, label_wrap, kTimelineScrollLabel, kDimColor);

    float x = top.x + label_w + gap;
    if (scroll_button(ImVec2(x, mid - button.y * 0.5f), button, kScrollBackLabel, kScrollBackHint))
        ev.view = page(view, -1, length_ms);
    x += button.x + gap;

    if (bar_w > 2.0f * border) {
        const ImVec2 b0(x, mid - bar_h * 0.5f), b1(x + bar_w, mid + bar_h * 0.5f);
        const ImVec2 in0(b0.x + border, b0.y + border);
        const float in_w = bar_w - 2.0f * border;
        const ScrollbarLayout sb =
            scrollbar_layout(view, length_ms, in_w, bar_h - 2.0f * border, rem(kThumbMinRem), px(kThumbInset));
        ImGui::SetCursorScreenPos(b0);
        ImGui::InvisibleButton(kTimelineScrollbarId, ImVec2(bar_w, bar_h));
        const float mx = ImGui::GetIO().MousePos.x - in0.x;
        const bool on_thumb = ImGui::IsItemHovered() && mx >= sb.thumb.x0 && mx <= sb.thumb.x1;
        const bool dragging = ImGui::IsItemActive() && ImGui::GetStateStorage()->GetBool(ImHashStr("drag", 0, ImGui::GetItemID()));
        // A press beside the thumb moves one window toward it (line 711).
        if (std::optional<TimelineView> v = press_or_drag(view, length_ms, mx, in_w, sb.thumb.x0, sb.thumb.x1,
                                                          [&](double, double share) {
                                                              return std::optional(scrollbar_click(view, share, length_ms));
                                                          }))
            ev.view = v;
        dl->AddRectFilled(b0, b1, kScrollBarColor, rem(kScrollBarRoundingRem));
        draw_line_border(dl, b0, b1, rem(kScrollBarRoundingRem));
        dl->AddRectFilled(ImVec2(in0.x + sb.thumb.x0, in0.y + sb.thumb.y0), ImVec2(in0.x + sb.thumb.x1, in0.y + sb.thumb.y1),
                          on_thumb || dragging ? kThumbHoverColor : kThumbColor, rem(kThumbRoundingRem));
    }
    x += std::max(0.0f, bar_w) + gap;

    if (scroll_button(ImVec2(x, mid - button.y * 0.5f), button, kScrollForwardLabel, kScrollForwardHint))
        ev.view = page(view, 1, length_ms);
    x += button.x + gap;

    // The readout, right-aligned in its column, as ImGui text so the GUI
    // tests read it.
    ImGui::SetCursorScreenPos(ImVec2(x + readout_w - text_sz.x, mid - text_sz.y * 0.5f));
    {
        ScopedFont font(TextRole::Figure);
        ImGui::PushStyleColor(ImGuiCol_Text, kMutedColor);
        ImGui::TextUnformatted(text.c_str());
        ImGui::PopStyleColor();
    }
    return row_h;
}

}  // namespace

ScrollbarLayout scrollbar_layout(TimelineView view, double length_ms, float width, float height, float thumb_min_w,
                                 float thumb_inset) {
    ScrollbarLayout out;
    const ThumbShares t = thumb(view, length_ms);
    const float x0 = static_cast<float>(t.left) * width;
    const float w = std::max(static_cast<float>(t.width) * width, thumb_min_w);
    out.thumb = {x0, thumb_inset, x0 + w, height - thumb_inset};
    out.readout = readout(view, length_ms);
    return out;
}

TimelineEvents PreviewTimeline::draw(const TimelineInput& in) const {
    TimelineEvents ev;
    if (in.scene == nullptr) return ev;
    const ImVec2 top = ImGui::GetCursorScreenPos();
    const float avail_w = ImGui::GetContentRegionAvail().x;
    const float label_w = rem(kLabelColumnRem);
    const float track_w = avail_w - label_w;
    if (!(track_w > 0.0f)) return ev;
    const TimelineView view = in.view.value_or(open_view(in.length_ms));
    const std::optional<TimelineLayout> laid =
        timeline_layout(*in.scene, in.length_ms, view, track_w, timeline_metrics(), in);
    if (!laid) return ev;
    const TimelineLayout& l = *laid;
    const float label_wrap = label_w - rem(kLabelPadRightRem);

    // The rows start at the cursor; the scrollbar is the only whole-song
    // control.
    const ImVec2 origin = top;
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
    for (size_t r = 0; r < kTimelineRowCount; ++r) {
        const TimelineRect& row = l.rows[r];
        if (!(row.y1 > row.y0)) continue;
        if (r > 0)
            dl->AddRectFilled(ImVec2(origin.x, t0.y + row.y0), at(l.width, row.y0 + px(1.0f)), kRowEdgeColor);
        if (static_cast<TimelineRow>(r) == TimelineRow::Saved)
            draw_saved_caption(dl, origin.x, t0.y + row.y0, t0.y + row.y1, label_wrap);
        if (names[r] == nullptr) continue;
        label_block(dl, origin.x, t0.y + row.y0, t0.y + row.y1, label_wrap, names[r], kMutedColor);
    }

    // Everything on the track is clipped to the rows' rectangle, so a mark
    // half outside the window shows its visible half (the third mockup's
    // `.track{overflow:hidden}`, line 98).
    dl->PushClipRect(t0, at(l.width, l.height()), true);

    // The axis's time labels, where the layout put them.
    {
        ScopedFont font(TextRole::TimelineAxis);
        const float y = l.row(TimelineRow::Axis).y0 + rem(kAxisLabelTopRem);
        for (const TimelineTick& t : l.axis_ticks) dl->AddText(at(t.text_x, y), kDimColor, t.text.c_str());
    }

    // Sections: the ones the layout marks shaded in kSectionShade, each with
    // its left edge and its name, cut to fit by the layout.
    {
        ScopedFont font(TextRole::TimelineSection);
        for (size_t i = 0; i < l.sections.size(); ++i) {
            const TimelineRect& r = l.sections[i].rect;
            const ImVec2 mn = at(r.x0, r.y0), mx = at(r.x1, r.y1);
            if (l.sections[i].shaded) dl->AddRectFilled(mn, mx, kSectionShade);
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

    // The fills, each its own item over its hit box, before the rows' item so
    // a fill takes the hover over the row under it. ImGui refuses an item
    // with no width, which only a fill whose neighbours on both sides share
    // its x gets; the pointer reaches that moment through them.
    for (const TimelineFillItem& f : l.fills) {
        bool hovered = false;
        if (f.hit.x1 > f.hit.x0) {
            ImGui::SetCursorScreenPos(at(f.hit.x0, f.hit.y0));
            if (ImGui::InvisibleButton(f.id.c_str(), ImVec2(f.hit.x1 - f.hit.x0, f.hit.y1 - f.hit.y0)))
                ev.clicked_fill = f.fill;
            hovered = ImGui::IsItemHovered();
        }
        if (hovered) ev.hovered_fill = f.fill;
        const ImVec2 c = at(f.x, f.y);
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
        for (const TimelineBestLabel& b : l.best_labels) {
            dl->AddText(at(b.rect.x0, b.rect.y0), b.col, in.best_label.c_str());
            log_text(at(b.rect.x0, b.rect.y0), in.best_label.c_str());
        }
    }

    // The rows' own item: a click off every fill lands on a band, a moment,
    // or, in the Fills row, nothing.
    ImGui::SetCursorScreenPos(t0);
    if (ImGui::InvisibleButton("##timeline", ImVec2(l.width, l.height()))) {
        const ImVec2 m = ImGui::GetIO().MousePos;
        const TimelineHit hit = timeline_hit(l, m.x - t0.x, m.y - t0.y);
        if (hit.kind == TimelineHit::Kind::Band) ev.clicked_band = hit.index;
        if (hit.kind == TimelineHit::Kind::Time) ev.clicked_ms = hit.ms;
    }
    const ImGuiID rows_id = ImGui::GetItemID();

    // The playhead, over every row, while it is inside the window (the third
    // mockup's placePlayhead, line 662).
    if (in_view(l.view, in.position_ms)) {
        const float x = static_cast<float>(fraction(l.view, in.position_ms)) * l.width;
        dl->AddRectFilled(at(x, 0.0f), at(x + px(kPlayheadW), l.height()), kPlayheadColor);
    }
    dl->PopClipRect();

    // The wheel over the rows, their labels included, scrolls the window
    // (line 729), with the fills' items on top of the rows' own. ImGui's
    // wheel is positive upward and the mockup's deltaY positive downward, so
    // a notch up moves the window back. Taking the wheel's key keeps the
    // window's own scroll from moving too.
    if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(origin, at(l.width, l.height()))) {
        ImGui::SetKeyOwner(ImGuiKey_MouseWheelY, rows_id);
        const float notches = -ImGui::GetIO().MouseWheel;
        if (notches != 0.0f) ev.view = wheel(view, notches, in.length_ms);
    }

    // The scrollbar, .3 rem under the rows, then an item over its label
    // column that ends the timeline's block, so the caller's next item
    // starts under it.
    const float sb_y = t0.y + l.height() + rem(kScrollGapRem);
    const float sb_h = draw_scrollbar_row(ImVec2(top.x, sb_y), avail_w, label_w, label_wrap, view, in.length_ms, ev);
    ImGui::SetCursorScreenPos(ImVec2(top.x, sb_y));
    ImGui::Dummy(ImVec2(label_w, sb_h));
    return ev;
}

}  // namespace hydra::ui
