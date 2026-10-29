// See overlay_layout.h.

#include "render/overlay_layout.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <vector>

#include <DirectXMath.h>

#include "render/highway_draw.h"

using namespace DirectX;

namespace hydra::render {

namespace {

// The renderer's camera over a width x height image's track rectangle: the
// one projection project_to_image runs forward and image_to_floor_time runs
// backward.
struct ImageProjection {
    TrackRect rect;
    HighwayCamera cam;
    XMMATRIX view_proj;
};

ImageProjection image_projection(const PreviewConfig& cfg, int width, int height) {
    ImageProjection p;
    p.rect = track_rect(cfg, width, height);
    p.cam = make_camera(cfg, p.rect.aspect);
    p.view_proj = XMLoadFloat4x4(&p.cam.view) * XMLoadFloat4x4(&p.cam.proj);
    return p;
}

// Normalized device coordinates (x right, y up, each -1 to 1 across the track
// rectangle) to image pixels, and back. Each is the other's inverse.
ImagePoint ndc_to_image(const TrackRect& r, float ndc_x, float ndc_y) {
    ImagePoint out;
    out.x = (ndc_x + 1.0f) * 0.5f * static_cast<float>(r.width);
    out.y = static_cast<float>(r.top) +
            (1.0f - ndc_y) * 0.5f * static_cast<float>(r.track_height);
    return out;
}

void image_to_ndc(const TrackRect& r, ImagePoint p, float& ndc_x, float& ndc_y) {
    ndc_x = p.x / (0.5f * static_cast<float>(r.width)) - 1.0f;
    ndc_y = 1.0f - (p.y - static_cast<float>(r.top)) / (0.5f * static_cast<float>(r.track_height));
}

}  // namespace

ImagePoint project_to_image(const PreviewConfig& cfg, int width, int height, const Vec3& p) {
    const ImageProjection proj = image_projection(cfg, width, height);
    const XMVECTOR ndc = XMVector3TransformCoord(XMVectorSet(p.x, p.y, p.z, 1.0f), proj.view_proj);
    return ndc_to_image(proj.rect, XMVectorGetX(ndc), XMVectorGetY(ndc));
}

std::optional<double> image_to_floor_time(const PreviewConfig& cfg, int width, int height,
                                          ImagePoint point, double now_s, double speed) {
    const PreviewConfig::Track& T = cfg.track;
    const ImageProjection proj = image_projection(cfg, width, height);
    float ndc_x = 0.0f, ndc_y = 0.0f;
    image_to_ndc(proj.rect, point, ndc_x, ndc_y);
    // The pixel's ray runs from the camera through the pixel's point on the
    // near plane (depth 0).
    const XMVECTOR on_near = XMVector3TransformCoord(
        XMVectorSet(ndc_x, ndc_y, 0.0f, 1.0f), XMMatrixInverse(nullptr, proj.view_proj));
    const Vec3 eye = proj.cam.view_pos;
    const double dy = static_cast<double>(XMVectorGetY(on_near)) - eye.y;
    const double dz = static_cast<double>(XMVectorGetZ(on_near)) - eye.z;
    if (dy == 0.0) return std::nullopt;
    const double s = (static_cast<double>(T.y) - eye.y) / dy;
    if (!(s > 0.0)) return std::nullopt;  // the floor lies behind the camera
    const double z = eye.z + s * dz;
    // The highway runs from the strike line (z_now) to the far end (z_future).
    const double z_lo = std::min(T.z_now, T.z_future);
    const double z_hi = std::max(T.z_now, T.z_future);
    if (!(z >= z_lo && z <= z_hi)) return std::nullopt;
    return z_to_time(cfg, now_s, z, speed);
}

HighwaySpan highway_span_at(const PreviewConfig& cfg, int width, int height, float y) {
    const PreviewConfig::Track& T = cfg.track;
    // x at row y on the screen line through a railing edge's two ends.
    auto x_at = [&](float wx, float wy) {
        const ImagePoint a = project_to_image(cfg, width, height, {wx, wy, T.z_past});
        const ImagePoint b = project_to_image(cfg, width, height, {wx, wy, T.z_future});
        if (std::fabs(a.y - b.y) < 1e-3f) return a.x;
        const float row = std::max(y, b.y);  // above the far end: the far end's edge
        return a.x + (row - a.y) * (b.x - a.x) / (b.y - a.y);
    };
    // The railings' outer edges.
    const float xl = railing_outer_x(cfg, false);
    const float xr = railing_outer_x(cfg, true);
    HighwaySpan s;
    s.left = std::min(x_at(xl, T.railing_y_top), x_at(xl, T.railing_y_bottom));
    s.right = std::max(x_at(xr, T.railing_y_top), x_at(xr, T.railing_y_bottom));
    return s;
}

OverlayTops overlay_tops(const OverlayHeights& s, bool drain_drawn, bool score_shown) {
    // Left column: the time box in the corner, the score box under it.
    // Right column: the drain box at the margin, the next-activation box
    // under it, or in its place when it is hidden; the solo box from the
    // middle row down, under the space the next-activation box reserves.
    OverlayTops t;
    t.time = 0.0f;
    t.score = s.time_h + s.gap;
    t.left_bottom = score_shown ? s.time_h + (s.gap + s.score_h) : s.time_h;
    t.drain = s.top_margin;
    t.next = s.top_margin + (drain_drawn ? s.drain_h + s.gap : 0.0f);
    t.solo = std::max(s.mid_row, s.next_h > 0.0f ? t.next + s.next_h + s.gap : t.next);
    return t;
}

BoxExtent measure_text_box(const TextBox& box, float inset) {
    BoxExtent e;
    for (const BoxLine& l : box.lines) {
        e.w = std::max(e.w, l.width);
        e.h += l.line_h;
    }
    e.w += inset * 2.0f;
    e.h += inset * 2.0f;
    return e;
}

std::vector<ImagePoint> text_box_line_origins(const TextBox& box, ImagePoint min, float inset) {
    std::vector<ImagePoint> out;
    out.reserve(box.lines.size());
    float y = min.y + inset;
    for (const BoxLine& l : box.lines) {
        out.push_back({min.x + inset, y});
        y += l.line_h;
    }
    return out;
}

float overlay_scale(const PreviewConfig& cfg, int width, int height, const OverlayBoxes& boxes,
                    float min_scale) {
    float scale = 1.0f;
    if (boxes.left_w > 0.0f) {
        const float room = highway_span_at(cfg, width, height, boxes.left_h).left - boxes.gap;
        scale = std::min(scale, room / boxes.left_w);
    }
    // The right column's boxes, each at its own lowest row.
    auto fit_right = [&](float w, float top, float h) {
        if (w <= 0.0f) return;
        scale = std::min(scale,
                         right_room(cfg, width, height, boxes.right_edge, top + h, boxes.gap) / w);
    };
    fit_right(boxes.right_w, boxes.right_top, boxes.right_h);
    fit_right(boxes.next_w, boxes.next_top, boxes.next_h);
    fit_right(boxes.solo_w, boxes.solo_top, boxes.solo_h);
    return std::clamp(scale, min_scale, 1.0f);
}

float right_room(const PreviewConfig& cfg, int width, int height, float right_edge, float row,
                 float gap) {
    return right_edge - highway_span_at(cfg, width, height, row).right - gap;
}

namespace {
// Where the last `n` words of `text` begin: no line break falls after it.
// The text's end when n is 0.
size_t tail_start(const std::string& text, size_t n) {
    size_t pos = text.size();
    while (pos > 0 && text[pos - 1] == ' ') --pos;
    if (n == 0) return pos;
    for (size_t words = 0; words < n && pos > 0; ++words) {
        while (pos > 0 && text[pos - 1] != ' ') --pos;  // to this word's start
        if (words + 1 < n)
            while (pos > 0 && text[pos - 1] == ' ') --pos;  // to the previous word's end
    }
    return pos;
}
}  // namespace

std::vector<std::string> wrap_words(const std::string& text, float max_w,
                                    const std::function<float(const std::string&)>& width_of,
                                    size_t keep_last) {
    std::vector<std::string> lines;
    const size_t tail = tail_start(text, keep_last);
    const size_t text_end = tail_start(text, 0);  // the last word's end
    size_t start = 0;
    while (start < text.size() && text[start] == ' ') ++start;
    while (start < text.size()) {
        // The ends of the words from `start` on: each space that follows a word, then the text's end.
        size_t best = std::string::npos;
        size_t end = start;
        while (end < text.size()) {
            size_t next = text.find(' ', end);
            if (next == std::string::npos) next = text.size();
            // A word ends here, and a line may: outside the kept tail, or at the text's end.
            if (next > start && text[next - 1] != ' ' && (next < tail || next == text_end)) {
                if (best != std::string::npos && width_of(text.substr(start, next - start)) > max_w)
                    break;
                best = next;  // the first word always goes, even too wide
            }
            end = next + 1;
        }
        lines.push_back(text.substr(start, best - start));
        start = best;
        while (start < text.size() && text[start] == ' ') ++start;
    }
    if (lines.empty()) lines.push_back(text);
    return lines;
}

WrappedText wrap_right_box(const PreviewConfig& cfg, int width, int height, float right_edge,
                           float top, float inset, float gap, const std::vector<TextPiece>& pieces) {
    auto wrap_all = [&](float max_w) {
        WrappedText out;
        for (const TextPiece& p : pieces) {
            out.lines.push_back(wrap_words(p.text, max_w, p.width_of, p.keep_last));
            for (const std::string& l : out.lines.back()) {
                out.text_w = std::max(out.text_w, p.width_of(l));
                out.text_h += p.line_h;
            }
        }
        return out;
    };
    WrappedText wrapped = wrap_all(std::numeric_limits<float>::max());
    // A pass that changes the box adds a line, and a piece never has more
    // lines than characters, so the passes are capped by the characters.
    size_t passes = 1;
    for (const TextPiece& p : pieces) passes += p.text.size();
    for (size_t i = 0; i < passes; ++i) {
        const float bottom = top + inset * 2.0f + wrapped.text_h;
        const float room = right_room(cfg, width, height, right_edge, bottom, gap) - inset * 2.0f;
        WrappedText next = wrap_all(room);
        const bool grew = next.text_h > wrapped.text_h;
        wrapped = std::move(next);
        if (!grew) break;
    }
    return wrapped;
}

std::string ellipsize(const std::string& text, float max_w,
                      const std::function<float(const std::string&)>& width_of,
                      float& kept_w) {
    const float whole_w = width_of(text);
    if (whole_w <= max_w) {
        kept_w = whole_w;
        return text;
    }
    static const std::string kEllipsis = "\xE2\x80\xA6";
    // The places the text may be cut: every character's start, past the
    // first character. A UTF-8 continuation byte (10xxxxxx) starts none.
    std::vector<size_t> cuts;
    for (size_t i = 1; i < text.size(); ++i)
        if ((static_cast<unsigned char>(text[i]) & 0xC0) != 0x80) cuts.push_back(i);
    // The text kept before the "…": the spaces a cut would leave go.
    auto kept = [&](size_t cut) {
        size_t end = cut;
        while (end > 0 && text[end - 1] == ' ') --end;
        return text.substr(0, end);
    };
    // The longest cut that fits: a longer prefix is never narrower.
    size_t lo = 0, hi = cuts.size();  // cuts[0..lo) fit; cuts[hi..) do not
    while (lo < hi) {
        const size_t mid = lo + (hi - lo) / 2;
        if (width_of(kept(cuts[mid]) + kEllipsis) <= max_w)
            lo = mid + 1;
        else
            hi = mid;
    }
    if (lo == 0) {
        kept_w = 0.0f;
        return kEllipsis;
    }
    const std::string shown = kept(cuts[lo - 1]);
    kept_w = width_of(shown);
    return shown + kEllipsis;
}

std::string ellipsize(const std::string& text, float max_w,
                      const std::function<float(const std::string&)>& width_of) {
    float kept_w = 0.0f;
    return ellipsize(text, max_w, width_of, kept_w);
}

}  // namespace hydra::render
