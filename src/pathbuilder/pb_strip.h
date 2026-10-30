// The Path Builder's strip under the highway: a marker per fill the highway
// shows, and the step line from BF(0) down by each window's cost (spec "The
// screen"; plan task PB-2b). The step line reads EN-3's ladder through
// cost_list and nothing else. The layout half is device-free and
// ImGui-free so the tests can pin it; draw_strip draws it.

#ifndef HYDRA_PATHBUILDER_PB_STRIP_H
#define HYDRA_PATHBUILDER_PB_STRIP_H

#include <cstdint>
#include <optional>
#include <vector>

#include "app/path_costs.h"
#include "app/preview_view.h"

namespace hydra::pathbuilder {

// The step line's levels, left to right: BF(0), then the level after each
// window the ladder has priced (each one cost below the last, from
// cost_list), then, once the run is complete, the player's total (one
// "after your last window" cost below the last window's level). Empty
// before BF(0) lands. No level is above the one before it, because no cost
// is below zero (cost_tier's guard). `window_count` is how many windows the
// path on screen has (the decision list's rows); with none the line is empty
// too, as the decision list then shows no costs (the user's ruling, "Hide
// it").
std::vector<int64_t> step_levels(const app::LadderRun& run, size_t window_count);

// One fill's marker on the strip, in the strip's own x (0 at its left
// edge). Its click area is exactly this rectangle (plan Q4).
struct StripMarker {
    size_t fill = 0;  // index into PreviewScene::fills
    app::PreviewFillState state = app::PreviewFillState::Offered;
    float x0 = 0.0f;
    float x1 = 0.0f;
};

// The markers for `scene`'s fills on a strip `width` wide that spans 0 to
// `end_ms` (the Preview's playback_end_ms). Hidden fills are left out, as
// the highway leaves them out. A marker covers its fill's span and is never
// narrower than `min_w`, centred on the span when widened.
std::vector<StripMarker> strip_markers(const app::PreviewScene& scene, double end_ms,
                                       float width, float min_w);

// The marker under strip x `x`: the last one drawn there, since a later
// marker draws over an earlier one. None when no marker covers x.
std::optional<size_t> marker_at(const std::vector<StripMarker>& markers, float x);

// Draws the strip at the cursor, `height` tall and as wide as the region
// left, and returns the index into scene.fills of a marker clicked this
// frame. `run` (may be null) is the ladder for the path on screen and
// `window_count` that path's windows, as step_levels reads them.
std::optional<size_t> draw_strip(const app::PreviewScene& scene, double end_ms,
                                 const app::LadderRun* run, size_t window_count, float height);

// The strip's name as an item, so the GUI test can find its rectangle.
inline constexpr const char* kStripId = "##strip";

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_STRIP_H
