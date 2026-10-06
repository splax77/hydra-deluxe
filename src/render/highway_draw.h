// Highway draw list — the pure port of Onyx's `drawDrumPlay`: given the track
// state, the config and the current time, produce the ordered list of draw
// commands for one frame. Every Onyx number (camera, time->depth, lane
// extents, gem boxes, fade timings) is applied here, device-free and
// unit-tested; the renderer only executes commands.
//
// Coordinates are Onyx's: right-handed, +X right, +Y up, the floor at
// track.y, the strike line at z = track.z_now, the future toward -Z.
//
// Hydra vocabulary in identifiers; Onyx names in comments where they differ
// (Onyx "targets" = the strike line, "overdrive" = SP phrase, "BRE" lanes =
// the fill look).

#ifndef HYDRA_RENDER_HIGHWAY_DRAW_H
#define HYDRA_RENDER_HIGHWAY_DRAW_H

#include <vector>

#include <DirectXMath.h>

#include "render/preview_config.h"
#include "render/track_state.h"

namespace hydra::render {

// TriangleLeft / TriangleRight: make_triangle with its apex toward -X / +X.
enum class MeshId { Tom, Cymbal, Kick, Flat, Box, TriangleLeft, TriangleRight, Count };

enum class TextureId {
    None,
    BoxRed, BoxYellow, BoxBlue, BoxGreen, BoxEnergy,
    CymbalYellow, CymbalBlue, CymbalGreen, CymbalEnergy,
    LongKick, LongEnergy,
    OverlayGhost, OverlayAccent,
    Line1, Line2, Line3,
    TargetRed, TargetYellow, TargetBlue, TargetGreen,
    TargetRedLight, TargetYellowLight, TargetBlueLight, TargetGreenLight,
    LaneRed, LaneYellow, LaneBlue, LaneGreen,
    Count
};

// The asset file name under assets/preview/textures for a texture.
const char* texture_file(TextureId id);

enum class MaterialKind { Color, Texture, TextureOverlay };

struct Material {
    MaterialKind kind = MaterialKind::Color;
    Color color;                       // Color
    TextureId texture = TextureId::None;  // Texture / TextureOverlay base
    TextureId overlay = TextureId::None;  // TextureOverlay second layer
};

// Which light a draw uses: the highway's fixed light, the per-gem light
// offset from the gem's top centre (Onyx LightOffset), or none, so the
// material's colour shows exactly as given (Hydra's SP end marks, D81).
enum class LightKind { Global, GemOffset, Unlit };

enum class DepthMode { Less, Always };

// Onyx's ObjectStretch: the unit mesh scaled and moved to fill the box lo..hi
// (a Flat keeps Y scale 1 since its box is flat).
struct DrawCommand {
    MeshId mesh = MeshId::Flat;
    float lo[3] = {0, 0, 0};
    float hi[3] = {0, 0, 0};
    Material material;
    float alpha = 1.0f;
    LightKind light = LightKind::Global;
    DepthMode depth = DepthMode::Less;
};

struct HighwayCamera {
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 proj;
    Vec3 view_pos;
};

// Onyx setUpTrackView: view = translate(-camera) then rotate down by
// `rotate` degrees about X; projection = perspective(fov, aspect, near, far),
// right-handed. `aspect` is the track rectangle's width / height.
HighwayCamera make_camera(const PreviewConfig& cfg, float aspect);

// Where the track rectangle sits in a width x height preview, and at what
// size. Onyx lays out one highway min(height, width * height_width_ratio)
// tall, anchored at the bottom. The renderer sizes its scene target and its
// camera with this and the overlay layout projects through it, so the two
// cannot disagree. A size below 1 counts as 1.
struct TrackRect {
    int width = 1, height = 1;  // the image, each at least 1
    int track_height = 1;       // the track rectangle's height, at least 1
    int top = 0;                // its top row: it hugs the image's bottom edge
    float aspect = 1.0f;        // the camera's aspect: width / track_height
};
TrackRect track_rect(const PreviewConfig& cfg, int width, int height);

// track_rect's track height alone.
int track_height(const PreviewConfig& cfg, int width, int height);

// Onyx's far end of the highway: the time at z_future, now + secs_future *
// speed.
double far_time(const PreviewConfig& cfg, double now_s, double speed);

// Onyx timeToZ: z_now at `now`, z_future at far_time, linear.
double time_to_z(const PreviewConfig& cfg, double now_s, double t_s, double speed);
// Its inverse: the time at a depth (used for the window's near edge).
double z_to_time(const PreviewConfig& cfg, double now_s, double z, double speed);

// The X extent of a pad's lane: the note area split into four, left to right.
void pad_x(const PreviewConfig& cfg, Pad pad, float& x1, float& x2);

// The X extent of the left (right = false) or right railing. The railings,
// their SP end markers and the overlay's highway_span_at all place
// themselves by it.
void railing_x(const PreviewConfig& cfg, bool right, float& x1, float& x2);

// The X of a railing's outer edge, the one away from the lanes. The SP end
// markers and the overlay's highway_span_at ask it.
float railing_outer_x(const PreviewConfig& cfg, bool right);

// The model matrix for a DrawCommand's box (row-major, DirectXMath row vectors).
DirectX::XMMATRIX stretch_matrix(const DrawCommand& cmd);

// The per-draw light: the highway light, the gem light offset from the
// box's top centre, or for Unlit a full ambient with no diffuse or specular,
// which the object shader turns into the material colour unchanged.
LightConfig light_for(const PreviewConfig& cfg, const DrawCommand& cmd);

// One frame's draw list in Onyx's order: floor spans, railings, beat lines,
// lane strips, strike-line targets and their glows, Hydra's SP end marks
// (D81), then gems (far first).
std::vector<DrawCommand> build_highway_draws(const TrackState& state,
                                             const PreviewConfig& cfg,
                                             double now_s, double speed);

}  // namespace hydra::render

#endif  // HYDRA_RENDER_HIGHWAY_DRAW_H
