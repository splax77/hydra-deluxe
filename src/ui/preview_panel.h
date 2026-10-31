// The Preview panel: the transport row, the scrubber, the 3D note highway
// with its overlays, and the key bar under it. Hydra's Preview tab and the
// Path Builder both draw it, so the two programs show one Preview. It reads
// only the controller it is handed; whatever belongs to the program around
// it (which path to show, the settings file) stays with the caller.
#pragma once

#include <functional>
#include <optional>
#include <string>

#include "render/overlay_layout.h"  // ImagePoint

namespace hydra::ui {

class PreviewController;

// How far one jump moves the playhead (the -Ns/+Ns buttons and the Left and
// Right keys), in whole seconds so every label prints an integer; and how far
// one step moves it (the step buttons, comma and period), in whole
// milliseconds for the same reason. Every label, tooltip and key-bar word in
// preview_panel.cpp is built from these two. They live here so a test reads
// the real distance.
inline constexpr int kJumpSeconds = 5;
inline constexpr int kStepMs = 10;

// The jump buttons' labels, built from kJumpSeconds: "-5s" and "+5s".
const std::string& jump_back_label();
const std::string& jump_forward_label();

// A left click on the highway image: where, in the image's own pixels, and
// the image's size, which together are what PreviewController::fill_at_image
// reads.
struct PreviewImageClick {
    render::ImagePoint at;
    int width = 0;
    int height = 0;
};

// What the panel reads from its caller, and what it reports back each frame.
// The caller owns the volume: the panel only shows it and says when the
// slider moved or was let go, and the caller decides what to store.
struct PreviewPanelState {
    // In: the volume the slider shows, in percent.
    int volume_pct = 0;
    // Out, reset at the start of every draw_preview_panel call.
    bool volume_changed = false;    // the slider moved; volume_pct holds where to
    bool volume_committed = false;  // the slider was let go after an edit
    std::optional<PreviewImageClick> image_click;  // a left click on the highway
};

// Draws the panel for a controller that is open, loaded and has no error
// (the caller shows those states itself). `above_transport` runs once, after
// the audio-warning line and before the transport row, for rows the caller
// keeps there; it may be empty.
void draw_preview_panel(PreviewController& controller, PreviewPanelState& state,
                        const std::function<void()>& above_transport);

}  // namespace hydra::ui
