// Dear ImGui's start-up and shutdown for any of Hydra's GUI programs: the
// context, the theme, the UI scale, the fonts, and the frame-text capture the
// GUI test runner reads. It knows nothing of Hydra's own window; app_shell.h
// adds that (the window placements and the per-frame draw) and installs its
// settings handler through ImGuiSetupOptions::before_ini_load.

#ifndef HYDRA_UI_IMGUI_SETUP_H
#define HYDRA_UI_IMGUI_SETUP_H

#include <string>

#include "imgui.h"

namespace hydra::ui {

struct ImGuiSetupOptions {
    // Monitor DPI scale; sizes, fonts, and px() all follow it.
    float dpi_scale = 1.0f;
    // Where ImGui persists table column widths. Empty = the exe-relative
    // default (hydra_ui.ini); "-" = don't persist at all (tests).
    std::string ini_file;
    // Directory holding the fonts. Empty = app::resource_dir().
    std::string resource_dir;
    // Called once, right before setup_imgui reads the ini file, so a settings
    // handler added here sees its sections on the first read. Null: nothing
    // to add. Hydra passes install_placement_handler (ui/app_shell.h).
    void (*before_ini_load)() = nullptr;
};

// CreateContext + flags + theme + DPI scaling + fonts. The platform/renderer
// backends (ImGui_ImplWin32_Init / ImGui_ImplDX11_Init) are the caller's:
// the app has a window, the runner has none.
void setup_imgui(const ImGuiSetupOptions& options);
void shutdown_imgui();  // DestroyContext, then unmaps the font files

// The colour each frame is cleared to before ImGui draws, as red, green, blue
// and alpha. Hydra.exe clears its window with it and the GUI test runner its
// offscreen target.
inline constexpr float kClearColor[4] = {0.10f, 0.11f, 0.13f, 1.00f};

// ---- UI scale ----------------------------------------------------------

// The UI scale for a monitor's DPI: 96 DPI is 1.0. A DPI of 0 reads as 1.0.
float ui_scale_for_dpi(unsigned dpi);

// The theme at `scale`: base with ScaleAllSizes(scale) and FontScaleDpi =
// scale. Always from the unscaled base, because ImGui rounds every size on
// each ScaleAllSizes call, so scaling an already-scaled style drifts.
ImGuiStyle scaled_style(const ImGuiStyle& base, float scale);

// Rescales the running UI to `scale`: ImGui's paddings and sizes, the font
// size, and px(). setup_imgui calls it once; main.cpp calls it again when the
// window lands on a monitor with another scale. Call between frames only.
void set_ui_scale(float scale);

// ---- Frame text ----------------------------------------------------------

// Everything ImGui drew this frame, as text, in draw order. Filled between
// begin_frame_text_capture and end_frame_text_capture when `enabled`; the
// runner's wait-text/expect-text search it.
struct FrameText {
    bool enabled = false;
    std::string text;
};

// Call right after ImGui::NewFrame() and right before ImGui::Render(), around
// everything the frame draws. Both do nothing when `capture` is null or not
// enabled.
void begin_frame_text_capture(FrameText* capture);
void end_frame_text_capture(FrameText* capture);

}  // namespace hydra::ui

#endif  // HYDRA_UI_IMGUI_SETUP_H
