// The Preview's whole-song timeline: one row per kind of thing in a
// PreviewScene, laid out over one width, each x from song_fraction (the
// scrub bar's own rule). The layout and the hit test are pure, so they run
// with no ImGui context; the drawing is thin over them. The scrub bar's gold
// marks and the score box's star glyph are drawn here too, so one place owns
// that geometry.
#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "app/preview_view.h"
#include "imgui.h"

namespace hydra::ui {

// Gold ticks over the scrubber just drawn (the last ImGui item), one per
// entry of `marks` (build_scrub_marks' fractions).
void draw_scrub_marks(const std::vector<double>& marks);

// One five-point star `size` tall with its top-left at `min`: filled in `col`
// when `filled`, else an outline in `col`.
void draw_star(ImDrawList* dl, ImVec2 min, float size, bool filled, ImU32 col);

// The timeline's rows, top to bottom.
enum class TimelineRow { Axis, Sections, Phrases, Saved, Ghost, Activations, Fills };
inline constexpr size_t kTimelineRowCount = 7;

// Each row's height and the side of a fill's square hit box, in pixels. The
// caller supplies them: the timeline owns no size of its own.
struct TimelineMetrics {
    float row_h[kTimelineRowCount] = {};
    float fill_size = 0.0f;
};

// A rectangle in the timeline's own pixels: x from its left edge, y from its
// top.
struct TimelineRect {
    float x0 = 0.0f, y0 = 0.0f, x1 = 0.0f, y1 = 0.0f;
    bool contains(float x, float y) const { return x >= x0 && x < x1 && y >= y0 && y < y1; }
};

struct TimelinePoint {
    float x = 0.0f, y = 0.0f;
};

// One fill: its index in PreviewScene::fills, its chord tick, the ImGui item
// name a GUI test and the player click (timeline_fill_id), the state it is
// drawn by, and its hit box in the Fills row, centred on the fill's end.
struct TimelineFillItem {
    size_t fill = 0;
    int64_t chord_tick = 0;
    std::string id;
    app::PreviewFillState state = app::PreviewFillState::Hidden;
    float x = 0.0f;
    TimelineRect hit;
};

// One section: its name and the stretch until the next section (or the
// song's end).
struct TimelineSection {
    std::string name;
    TimelineRect rect;
};

// One activation's window as a band in the Activations row: its index in
// PreviewScene::activations and the rectangle over its sp_window().
struct TimelineBand {
    size_t activation = 0;
    TimelineRect rect;
};

struct TimelineLayout {
    float width = 0.0f;
    double length_ms = 0.0;
    TimelineRect rows[kTimelineRowCount];
    std::vector<TimelineSection> sections;
    std::vector<float> phrase_x;  // each SP phrase's start
    // The Star Power saved line: each meter segment's start and end, in
    // order, so a jump between two segments is a vertical step.
    std::vector<TimelinePoint> saved_line;
    float activate_y = 0.0f;  // the saved row's height at kSpActivationBars
    std::vector<float> ghost_x;  // each ghost fill's end (PreviewScene::ghost_fills)
    std::vector<TimelineBand> bands;
    std::vector<TimelineFillItem> fills;
    float height() const { return rows[kTimelineRowCount - 1].y1; }
    const TimelineRect& row(TimelineRow r) const { return rows[static_cast<size_t>(r)]; }
};

// The item name of the fill on `chord_tick`: "##fill_<chord_tick>".
std::string timeline_fill_id(int64_t chord_tick);

// Lay the scene out `width` pixels wide over `length_ms` (the Preview's
// playback end). No value when song_fraction has no song length to use.
std::optional<TimelineLayout> timeline_layout(const app::PreviewScene& scene, double length_ms,
                                              float width, const TimelineMetrics& metrics);

// What a point lands on: a fill's hit box, else an activation band, else,
// anywhere inside the timeline, the moment under it. Outside, nothing.
struct TimelineHit {
    enum class Kind { None, Fill, Band, Time };
    Kind kind = Kind::None;
    size_t index = 0;  // the fill's or the activation's index in the scene
    double ms = 0.0;   // for Time
};
TimelineHit timeline_hit(const TimelineLayout& layout, float x, float y);

}  // namespace hydra::ui
