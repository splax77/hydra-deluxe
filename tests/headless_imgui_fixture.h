// A headless ImGui context with the app's fonts, for unit tests that measure
// text or drive real tables with no window or renderer. The column-width,
// text-role and report-view tests all build one, so it lives here once.

#ifndef HYDRA_TESTS_HEADLESS_IMGUI_FIXTURE_H
#define HYDRA_TESTS_HEADLESS_IMGUI_FIXTURE_H

#include "imgui.h"

#include "testdata_dirs.h"  // resource_root
#include "ui/app_shell.h"   // setup_imgui, shutdown_imgui
#include "ui/fonts.h"       // g_mono_font

namespace hydra::test {

// Sets ImGui up with the app's fonts at scale 1 and an 800 x 600 display.
// When it goes out of scope it shuts ImGui down and puts g_mono_font back as
// it found it, so a test that clears the mono font leaves no trace.
struct HeadlessImGui {
    ImFont* saved_mono = hydra::ui::g_mono_font;
    HeadlessImGui() {
        hydra::ui::ImGuiSetupOptions opts;
        opts.dpi_scale = 1.0f;
        opts.ini_file = "-";
        opts.resource_dir = ::corpus::resource_root();
        hydra::ui::setup_imgui(opts);
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(800.0f, 600.0f);
        io.DeltaTime = 1.0f / 60.0f;
        // No renderer is attached, so say the fonts need no texture upload.
        io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    }
    ~HeadlessImGui() {
        hydra::ui::shutdown_imgui();
        hydra::ui::g_mono_font = saved_mono;
    }
    HeadlessImGui(const HeadlessImGui&) = delete;
    HeadlessImGui& operator=(const HeadlessImGui&) = delete;
};

}  // namespace hydra::test

#endif  // HYDRA_TESTS_HEADLESS_IMGUI_FIXTURE_H
