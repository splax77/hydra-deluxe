// Where the highway lands on the Preview image, and the scale that keeps the
// Preview's text boxes (stacked by overlay_tops) beside it rather than on
// it. Device-free: the same camera and track height the renderer draws with,
// projected on the CPU.
//
// The highway's size follows the track height, not the image width, so a
// narrower window leaves less room at its sides. The boxes shrink into that
// room, never past kOverlayMinScale (below it the text stops being readable,
// so they overlap instead) and never above 1 (their configured size).
//
// The next-activation box wraps its lines to the room beside the highway at
// its lowest row (wrap_right_box) rather than shrink every box to fit its
// longest line.
//
// Every text box's size and where its lines go come from one pair,
// measure_text_box and text_box_line_origins (D114 item 27).
//
// Also the "…" cut for a label too long for its space (ellipsize), measured
// by the caller's font so it stays device-free.

#ifndef HYDRA_RENDER_OVERLAY_LAYOUT_H
#define HYDRA_RENDER_OVERLAY_LAYOUT_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "render/preview_config.h"

namespace hydra::render {

// A point on the image, in pixels from its top-left corner (x right, y down).
struct ImagePoint {
    float x = 0.0f;
    float y = 0.0f;
};

// Where world point `p` lands in a width x height Preview image, through the
// renderer's own camera, with the track rectangle anchored at the bottom.
ImagePoint project_to_image(const PreviewConfig& cfg, int width, int height, const Vec3& p);

// The highway's outer edges (the railings' outsides) at image row `y`. Each
// railing edge is a straight line on screen, read off its two projected ends;
// the railing's top and bottom give two lines and the wider one is returned.
// Above the far end the far end's edges are used: nothing is drawn there.
struct HighwaySpan {
    float left = 0.0f;
    float right = 0.0f;
};
HighwaySpan highway_span_at(const PreviewConfig& cfg, int width, int height, float y);

// The boxes' heights at one scale, in image pixels: what the stacking order
// (overlay_tops) reads.
struct OverlayHeights {
    float time_h = 0.0f;
    float score_h = 0.0f;
    float drain_h = 0.0f;
    float gap = 0.0f;         // between two boxes in one column
    float top_margin = 0.0f;  // from the image's top edge to the right column
};

// Where each text box's top edge sits, in image pixels from the image's top
// edge, and where the left column ends.
struct OverlayTops {
    float time = 0.0f;
    float score = 0.0f;
    float left_bottom = 0.0f;
    float drain = 0.0f;
    float next = 0.0f;
};

// The one stacking order of the Preview's text boxes. The fit pass (at scale
// 1) and the draw pass (at the fitted scale) both read it, so a box is never
// fitted in one place and drawn in another.
OverlayTops overlay_tops(const OverlayHeights& sizes, bool drain_drawn, bool score_shown);

// One line of a Preview text box. A Text line is `text` at font size `size`
// in `color`; a Stars line is the score box's star row, `filled` of its stars
// in `color` and the rest in `off_color`, each star `size` tall. `width` is
// the line as the caller's font measures it, and `line_h` how far below its
// top the next line starts. Colours are packed the way the UI's draw list
// takes them.
struct BoxLine {
    enum class Kind { Text, Stars };
    Kind kind = Kind::Text;
    std::string text;
    float size = 0.0f;
    uint32_t color = 0;
    uint32_t off_color = 0;  // Stars only
    int filled = 0;          // Stars only
    float width = 0.0f;
    float line_h = 0.0f;
};

// A Preview text box (the time, score, SP drain and next-activation boxes):
// its lines, top to bottom.
struct TextBox {
    std::vector<BoxLine> lines;
};

// A text box's outer size, in image pixels.
struct BoxExtent {
    float w = 0.0f;
    float h = 0.0f;
};

// The one size rule for every text box: its widest line and its lines'
// heights, with `inset` between the text and the panel's edge on all four
// sides.
BoxExtent measure_text_box(const TextBox& box, float inset);

// Where each of the box's lines starts (its top-left), for a box whose
// top-left is `min`: every line left-aligned at the same inset that
// measure_text_box keeps.
std::vector<ImagePoint> text_box_line_origins(const TextBox& box, ImagePoint min, float inset);

// The boxes to fit, at scale 1, in image pixels. A width of 0 means absent.
// The right column's boxes share one right edge, left of the gauge. The tops
// and the left column's height come from overlay_tops.
struct OverlayBoxes {
    float left_w = 0.0f;      // the top-left column
    float left_h = 0.0f;      //   from the image's top edge
    float right_edge = 0.0f;  // x of the right column's right edge
    float right_w = 0.0f;     // the SP drain box
    float right_h = 0.0f;
    float right_top = 0.0f;   //   y of its top edge
    float next_w = 0.0f;      // the next-activation box
    float next_h = 0.0f;
    float next_top = 0.0f;    //   y of its top edge
    float gap = 0.0f;         // clearance kept from the highway's edge
};

inline constexpr float kOverlayMinScale = 0.6f;

// One scale for all the boxes: the largest in [min_scale, 1] at which each box
// clears the highway at its lowest row (where the highway is widest within
// it). Rows are measured at scale 1, so a shrunken box is only more clear.
float overlay_scale(const PreviewConfig& cfg, int width, int height, const OverlayBoxes& boxes,
                    float min_scale = kOverlayMinScale);

// The room at image row `row` for a box whose right edge is at x
// `right_edge`, before it reaches the highway, less `gap`. The highway widens
// downward, so a box's lowest row is the one that limits it.
float right_room(const PreviewConfig& cfg, int width, int height, float right_edge, float row,
                 float gap);

// One part of a box's text: the words, how wide a run of them is, how tall
// each of its lines is, and how many of its last words stay together
// (wrap_words). kKeepWhole keeps the whole text on one line.
struct TextPiece {
    std::string text;
    std::function<float(const std::string&)> width_of;
    float line_h = 0.0f;
    size_t keep_last = 1;
};
inline constexpr size_t kKeepWhole = static_cast<size_t>(-1);

// A box's pieces as wrapped: each piece's lines in order, the widest line,
// and all the lines' heights summed.
struct WrappedText {
    std::vector<std::vector<std::string>> lines;
    float text_w = 0.0f;
    float text_h = 0.0f;
};

// `pieces` stacked in a box whose right edge is at `right_edge` and top at
// row `top`, with the text box's `inset` (measure_text_box's) inside on every
// side, each piece wrapped to the room at the box's lowest row (right_room,
// less the insets). More lines make the box taller and its room smaller, so
// it starts unbroken and wraps again until a pass adds no line; text that
// fits stays one line a piece.
WrappedText wrap_right_box(const PreviewConfig& cfg, int width, int height, float right_edge,
                           float top, float inset, float gap, const std::vector<TextPiece>& pieces);

// `text` broken at its spaces into lines no wider than `max_w`, each as wide
// as fits (the first line takes the most words). A line keeps the text's own
// characters; the spaces at a break go. A word wider than `max_w` has a line
// of its own and runs past it. Text that fits is one line, unchanged. The
// last `keep_last` words stay together, as one word would ("2 of 3" at the
// end of "Next: activation 2 of 3"). The Preview's next-activation box wraps
// its lines this way to stay off the lane.
std::vector<std::string> wrap_words(const std::string& text, float max_w,
                                    const std::function<float(const std::string&)>& width_of,
                                    size_t keep_last = 1);

// `text` as a box `max_w` wide shows it: whole when it fits, otherwise cut
// short and ended in "…" so the result fits. The cut falls between UTF-8
// characters, never inside one, and drops the spaces it would leave before the
// "…". When not even one character fits, the result is "…" alone. `width_of`
// measures a string in the font the box draws with. This is the one rule for
// cutting a label to fit (D48, Q18): the Preview's path picker, and every
// ellipsized label in the UI through ui::text_ellipsized.
std::string ellipsize(const std::string& text, float max_w,
                      const std::function<float(const std::string&)>& width_of);

// The same cut, also setting `kept_w` to the width of the text shown before
// the "…": the whole width when the text fits, 0 when "…" stands alone. The
// library's title cell stops its search highlight there.
std::string ellipsize(const std::string& text, float max_w,
                      const std::function<float(const std::string&)>& width_of,
                      float& kept_w);

}  // namespace hydra::render

#endif  // HYDRA_RENDER_OVERLAY_LAYOUT_H
