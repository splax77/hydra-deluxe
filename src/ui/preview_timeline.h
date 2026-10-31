// The Preview's whole-song timeline: one row per kind of thing in a
// PreviewScene, laid out over one width, each x from song_fraction (the
// scrub bar's own rule). The layout and the hit test are pure, so they run
// with no ImGui context; the drawing is thin over them. The scrub bar's gold
// marks and the score box's star glyph are drawn here too, so one place owns
// that geometry.
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
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

// The colour of the drawn path: its activation bands and its taken fills.
// Exported so a caller's mark for an activation is the bands' colour.
inline constexpr ImU32 kTimelinePathColor = IM_COL32(0x5e, 0xc8, 0xff, 0xff);  // the mockup's --sp

// The marks the Path Builder's legend explains, one swatch each: a fill the
// player can activate on, the player's own activation, a fill they can't, a
// fill before their last activation, and an offered fill in a caller's
// colour (Tinted).
enum class LegendSwatch { Offered, Activation, Cant, Locked, Tinted };

// One legend swatch of `kind`, drawn in the colours the Fills row draws that
// mark in, as an item one text line tall at the cursor. `col` is the colour
// of a Tinted swatch; the other kinds ignore it. The legend's words go after
// it with ImGui::SameLine(0, 0).
void legend_swatch(LegendSwatch kind, ImU32 col = 0);

// A filled circle `diameter` wide in `col`, as an item at the cursor, with
// `centred_text` (when not null) centred in it in the colour the activation
// bands write their numbers in. Callers outside ui/ draw their dots through
// this, so the draw list stays here.
void dot_item(ImU32 col, float diameter, const char* centred_text = nullptr);

// The axis puts a time label every 30 s, as the mockup does
// (docs/superpowers/specs/2026-10-30-path-builder-mockup/mockup_template.html,
// renderRows: `s += 30`).
inline constexpr double kTimelineAxisStepMs = 30000.0;

// The row names in the label column, as the mockup writes them (renderRows'
// `rlab` cells).
inline constexpr const char* kTimelineSectionsLabel = "Section";
inline constexpr const char* kTimelinePhrasesLabel = "Star Power phrases";
inline constexpr const char* kTimelineSavedLabel = "Star Power saved";
inline constexpr const char* kTimelineGhostLabel = "Hydra's best";
inline constexpr const char* kTimelineActivationsLabel = "Your activations";
inline constexpr const char* kTimelineFillsLabel = "Fills";
// The saved row's caption under its name, as the second mockup writes it
// (docs/superpowers/specs/2026-10-31-path-builder-mockup-2/mockup_template.html,
// renderRows): the activate line's words after its number, kSpActivationBars
// ("2 phrases: needed to activate", line 464), and the shading's words
// (line 465).
inline constexpr const char* kTimelineActivateLabelTail = " phrases: needed to activate";
inline constexpr const char* kTimelineEnoughLabel = "enough to activate";

// The section name the Section row leaves out: the practice section many
// charts end with. The match is exact, case included, on the name the scene
// holds (PreviewSection::name), as the mockup's renderRows tests it
// (`if (s.name === "End") return;`). The section before it still ends where
// it starts. The user chose to hide it on the timeline only; the Preview tab
// keeps showing it.
inline constexpr const char* kTimelineHiddenSectionName = "End";

// The timeline's rows, top to bottom.
enum class TimelineRow { Axis, Sections, Phrases, Saved, Ghost, Activations, Fills };
inline constexpr size_t kTimelineRowCount = 7;

// How wide a piece of text draws, in pixels. Empty measures every text as 0
// wide.
using TimelineTextWidth = std::function<float(const std::string&)>;

// Each row's height, the side of a fill's square hit box, how far down the
// Fills row a fill's centre sits, how far an activation band stands in from
// its row's top and bottom, how far the saved line's zero and cap stand in
// from its row's bottom and top, and the least distance between two axis
// labels, in pixels. The layout reads them from here; timeline_metrics()
// gives the screen's. A song so long that every 30 s label would crowd the
// next gets a label every few steps instead, so the number of labels follows
// the width, never the song's length; a gap of 0 turns that off, which only
// a test with a short song should do.
//
// The text the layout places: how far down the Fills row the word under a
// fill starts and how tall its line is, how far a section's name
// stands in from its edges, and a measure for each kind of text in the font
// it draws in (text_role.h's TimelineCost, TimelineSection and TimelineAxis).
struct TimelineMetrics {
    float row_h[kTimelineRowCount] = {};
    float fill_size = 0.0f;
    float fill_center_y = 0.0f;
    float band_inset = 0.0f;
    float saved_inset = 0.0f;
    float axis_label_gap = 0.0f;
    float cost_top = 0.0f;
    float cost_line_h = 0.0f;
    float section_pad = 0.0f;
    TimelineTextWidth cost_width;
    TimelineTextWidth section_width;
    TimelineTextWidth axis_width;
};

// The mockup's sizes for the timeline as PreviewTimeline draws it, each at
// the current font size or the display's scale as preview_timeline.cpp's
// constants say. Call it inside a frame. The ghost row's height
// here is the one it has when a ghost path is set; timeline_layout makes it
// 0 tall for a scene with no ghost windows.
TimelineMetrics timeline_metrics();

// A rectangle in the timeline's own pixels: x from its left edge, y from its
// top.
struct TimelineRect {
    float x0 = 0.0f, y0 = 0.0f, x1 = 0.0f, y1 = 0.0f;
    bool contains(float x, float y) const { return x >= x0 && x < x1 && y >= y0 && y < y1; }
};

struct TimelinePoint {
    float x = 0.0f, y = 0.0f;
};

// How a fill is drawn. timeline_layout picks one per fill from the scene's
// state and the caller's lists (TimelineFillMarks), first match wins: a Taken
// fill is Taken; a fill in locked_ticks is Locked, whatever its state; an
// Offered fill not in cant_ticks is Offered; every other fill is Cant.
enum class TimelineFillLook { Taken, Locked, Offered, Cant };

// One fill: its index in PreviewScene::fills, its chord tick, the ImGui item
// name a GUI test and the player click (timeline_fill_id), the state the
// scene gives it, the look it is drawn in, the caller's tint for it (none
// when the caller gave none), and its hit box in the Fills row, centred on
// the fill's end.
struct TimelineFillItem {
    size_t fill = 0;
    int64_t chord_tick = 0;
    std::string id;
    app::PreviewFillState state = app::PreviewFillState::Hidden;
    TimelineFillLook look = TimelineFillLook::Cant;
    std::optional<ImU32> tint;
    float x = 0.0f;
    TimelineRect hit;
};

// One section: its name, the name as it fits between the section's edges
// (render::ellipsize's cut, as the mockup's `.sec{text-overflow:ellipsis}`),
// and the stretch until the next section (or the song's end). A section named
// kTimelineHiddenSectionName gets none.
struct TimelineSection {
    std::string name;
    std::string shown;
    TimelineRect rect;
};

// One activation's window as a band in the Activations row: its index in
// PreviewScene::activations and the rectangle over its sp_window().
struct TimelineBand {
    size_t activation = 0;
    TimelineRect rect;
};

// One time label on the axis: its moment, its x, its text, and where the text
// starts. The text is centred on its moment, as the mockup's `.axis span`
// is, but never starts left of the timeline or ends right of it, so the first
// label stays out of the label column.
struct TimelineTick {
    double ms = 0.0;
    float x = 0.0f;
    std::string text;
    float text_x = 0.0f;
};

// The colour the caller chose for the fill on `chord_tick`.
struct TimelineFillTint {
    int64_t chord_tick = 0;
    ImU32 col = 0;
};

// What the caller says about its fills, by chord tick. `cant_ticks` are fills
// drawn as can't although the scene says Offered; `locked_ticks` are fills
// drawn as locked whatever the scene says (TimelineFillLook says which list
// wins); `tints` colour offered fills; every fill in `best_ticks` gets
// `best_label` under it.
struct TimelineFillMarks {
    std::vector<int64_t> cant_ticks;
    std::vector<int64_t> locked_ticks;
    std::vector<TimelineFillTint> tints;
    std::vector<int64_t> best_ticks;
    std::string best_label;
};

// The word under one fill as the layout places it: its fill's index in the
// scene, its colour (the fill's tint) and its box, one line tall, centred
// under the fill and kept inside the timeline, as the axis labels are.
struct TimelineBestLabel {
    size_t fill = 0;
    ImU32 col = 0;
    TimelineRect rect;
};

struct TimelineLayout {
    float width = 0.0f;
    double length_ms = 0.0;
    TimelineRect rows[kTimelineRowCount];
    // Every kTimelineAxisStepMs after the start (the start itself gets no
    // label), or every few of them when TimelineMetrics::axis_label_gap says
    // they would crowd.
    std::vector<TimelineTick> axis_ticks;
    std::vector<TimelineSection> sections;
    std::vector<float> phrase_x;  // each SP phrase's start
    // The Star Power saved line: each meter segment's start and end, in
    // order, so a jump between two segments is a vertical step.
    std::vector<TimelinePoint> saved_line;
    float activate_y = 0.0f;  // the saved row's height at kSpActivationBars
    // The stretches, each over the saved row's full height, where the player
    // has enough saved to activate: after the end of the last activation's
    // band (from the start with none), wherever saved_line is at or above
    // activate_y. A point exactly on the line counts as enough. Stretches
    // that touch are one.
    std::vector<TimelineRect> saved_enough;
    // The ghost path's windows (PreviewScene::ghost_windows), one band each
    // in the Ghost row. The ghost row is 0 tall when there are none. A point
    // on one lands on the moment under it, as the mockup's ghost bands take
    // no clicks.
    std::vector<TimelineRect> ghost_bands;
    std::vector<TimelineBand> bands;
    std::vector<TimelineFillItem> fills;
    // In the order of the scene's fills.
    std::vector<TimelineBestLabel> best_labels;
    float height() const { return rows[kTimelineRowCount - 1].y1; }
    const TimelineRect& row(TimelineRow r) const { return rows[static_cast<size_t>(r)]; }
};

// The item name of the fill on `chord_tick`: "##fill_<chord_tick>".
std::string timeline_fill_id(int64_t chord_tick);

// Lay the scene out `width` pixels wide over `length_ms` (the Preview's
// playback end), each fill in the look `marks` give it. No value when
// song_fraction has no song length to use.
std::optional<TimelineLayout> timeline_layout(const app::PreviewScene& scene, double length_ms,
                                              float width, const TimelineMetrics& metrics,
                                              const TimelineFillMarks& marks = {});

// What a point lands on: a fill's hit box, else an activation band, else,
// anywhere inside the timeline, the moment under it. Outside, nothing.
struct TimelineHit {
    enum class Kind { None, Fill, Band, Time };
    Kind kind = Kind::None;
    size_t index = 0;  // the fill's or the activation's index in the scene
    double ms = 0.0;   // for Time
};
TimelineHit timeline_hit(const TimelineLayout& layout, float x, float y);

// What the timeline draws this frame: the scene, the fills' marks (the base),
// `selected` to outline that activation's band, and `highlighted` to draw that
// fill (an index into the scene's fills) as hovered, with the selected band's
// outline.
struct TimelineInput : TimelineFillMarks {
    const app::PreviewScene* scene = nullptr;
    double length_ms = 0.0;    // the Preview's playback end
    double position_ms = 0.0;  // the playhead
    std::optional<size_t> selected;
    std::optional<size_t> highlighted;
};

// What happened on the timeline this frame. Indexes are into the scene's
// fills and activations.
struct TimelineEvents {
    std::optional<size_t> hovered_fill;
    std::optional<size_t> clicked_fill;
    std::optional<size_t> clicked_band;
    std::optional<double> clicked_ms;  // a click anywhere else on the rows
};

// The whole-song timeline as ImGui items at the cursor, the window's width
// wide: a label column, then the rows over timeline_layout. Each fill is an
// InvisibleButton named timeline_fill_id(chord_tick). Draws nothing when the
// layout has no value.
class PreviewTimeline {
public:
    TimelineEvents draw(const TimelineInput& in) const;
};

}  // namespace hydra::ui
