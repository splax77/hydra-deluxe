// PreviewRenderer — executes the highway draw list on Direct3D 11 into an
// offscreen texture the GUI shows with ImGui::Image (docs/adr/0005).
//
// This is the device half of the Onyx port (docs/adr/0008): the pure half
// (track_state + highway_draw) decides what to draw and where; this class
// loads the verbatim Onyx assets from `asset_dir` (models, textures, the
// config JSON, the HLSL shader files), owns the anti-aliased scene target,
// draws each command with the ported Phong shader, resolves, and composites
// the highway onto the background colour with Onyx's horizon fade.
//
// The track rectangle is anchored at the bottom of the target and is
// min(height, width * height_width_ratio) tall, as Onyx lays out one highway.

#ifndef HYDRA_RENDER_PREVIEW_RENDERER_H
#define HYDRA_RENDER_PREVIEW_RENDERER_H

#include <d3d11.h>

#include <memory>
#include <string>

#include <cstddef>
#include <optional>

#include "app/preview_view.h"
#include "render/overlay_layout.h"  // ImagePoint
#include "render/preview_config.h"
#include "render/track_state.h"

namespace hydra::render {

// The playback speed the highway is drawn at. Onyx's draw code takes one;
// Hydra always plays at 1x. Everything that reads the drawn highway back
// (PreviewRenderer::fill_at_image, a test projecting a point onto it) uses
// this one.
inline constexpr double kPlaybackSpeed = 1.0;

class PreviewRenderer {
public:
    // Loads every asset from `asset_dir` (assets/preview) and builds the
    // pipeline on the shared device. Throws std::runtime_error naming the
    // missing or broken asset / D3D object.
    PreviewRenderer(ID3D11Device* device, ID3D11DeviceContext* context,
                    const std::string& asset_dir);
    ~PreviewRenderer();

    PreviewRenderer(const PreviewRenderer&) = delete;
    PreviewRenderer& operator=(const PreviewRenderer&) = delete;

    // (Re)create the size-dependent targets.
    void resize(int width, int height);
    // Free the size-dependent targets, keeping the assets and pipeline (D95
    // call 4: the Preview frees them when it closes). Until the next resize,
    // texture_srv() is null, width() and height() are 0 and render() draws
    // nothing.
    void release_targets();

    // Replace the chart being drawn (builds the track state once, here, on
    // the calling thread).
    void set_scene(const hydra::app::PreviewScene& scene,
                   const TrackStateOptions& opts = TrackStateOptions{});
    // Replace the chart being drawn with a timeline already built from
    // `scene` (build_track_state on a worker thread), so this call only moves
    // it in. The caller guarantees `state` was built from `scene`; the scene
    // itself is not read again.
    void set_scene(const hydra::app::PreviewScene& scene, TrackState state);

    // Draw the frame at `now_ms` into the offscreen target.
    void render(double now_ms);

    // The fill lit under `point` on a width x height image of the highway
    // drawn at `now_ms`, as its index into the drawn scene's fills: the
    // floor time there (image_to_floor_time) read by TrackState::fill_at.
    // None off the floor or where no fill is lit.
    std::optional<size_t> fill_at_image(ImagePoint point, int width, int height,
                                        double now_ms) const;

    // The final colour texture's shader-resource view (null before resize).
    ID3D11ShaderResourceView* texture_srv() const;

    int width() const;
    int height() const;
    const PreviewConfig& config() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace hydra::render

#endif  // HYDRA_RENDER_PREVIEW_RENDERER_H
