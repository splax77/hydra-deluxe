#include "ui/imgui_setup.h"

#include <cstdint>
#include <exception>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"  // g.LogBuffer for FrameText

#include "app/config.h"
#include "audio/mapped_file.h"
#include "core/winstr.h"
#include "ui/fallback_fonts.h"
#include "ui/fonts.h"
#include "ui/theme.h"

namespace hydra::ui {

namespace {

// The theme at scale 1, captured by setup_imgui after apply_theme().
ImGuiStyle g_base_style;

// The size every UI font loads at, in pixels at scale 1. The backup fonts
// merged behind the main and mono fonts load at it too: ImGui scales merged
// glyphs by the ratio of the two sizes.
constexpr float kFontSize = 18.0f;

// The font files the atlas reads glyphs from, mapped by add_font_file. ImGui
// loads glyphs on demand for as long as its context lives, so shutdown_imgui
// unmaps them only after DestroyContext.
std::vector<std::shared_ptr<const audio::MappedFile>> g_font_files;

// AddFontFromFileTTF, but the atlas reads the file through a read-only memory
// map instead of a heap copy, so Windows keeps only the pages glyphs come
// from. A file that can't be mapped goes to AddFontFromFileTTF itself, so a
// missing or unreadable file fails just as it always has.
ImFont* add_font_file(const std::string& path, float size, const ImFontConfig* config = nullptr) {
    ImFontAtlas* atlas = ImGui::GetIO().Fonts;
    std::shared_ptr<const audio::MappedFile> file;
    try {
        file = audio::MappedFile::open(path);
    } catch (const std::exception&) {
        // Left to AddFontFromFileTTF below.
    }
    if (!file || file->size() == 0 ||
        file->size() > static_cast<size_t>(std::numeric_limits<int>::max()))
        return atlas->AddFontFromFileTTF(path.c_str(), size, config);

    ImFontConfig mapped = config ? *config : ImFontConfig();
    mapped.FontDataOwnedByAtlas = false;
    // ImGui only reads font data; its signature just isn't const.
    ImFont* font = atlas->AddFontFromMemoryTTF(const_cast<uint8_t*>(file->data()),
                                               static_cast<int>(file->size()), size, &mapped);
    if (font) g_font_files.push_back(std::move(file));
    return font;
}

}  // namespace

float ui_scale_for_dpi(unsigned dpi) {
    return dpi == 0 ? 1.0f : static_cast<float>(dpi) / 96.0f;  // 96 = USER_DEFAULT_SCREEN_DPI
}

ImGuiStyle scaled_style(const ImGuiStyle& base, float scale) {
    ImGuiStyle style = base;
    style.ScaleAllSizes(scale);
    style.FontScaleDpi = scale;
    return style;
}

void set_ui_scale(float scale) {
    if (scale <= 0.0f) return;
    ImGuiStyle& style = ImGui::GetStyle();
    // FontSizeBase is filled in by ImGui on the first frame; keep it.
    const float font_size_base = style.FontSizeBase;
    style = scaled_style(g_base_style, scale);
    style.FontSizeBase = font_size_base;
    g_ui_scale = scale;  // for the views' explicit pixel sizes
}

void setup_imgui(const ImGuiSetupOptions& options) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Multi-viewports are on so the report windows can open as real OS
    // windows beside Hydra (D103): a report is something to move to another
    // monitor and keep open while Hydra is in use. Only the report windows
    // opt in, through report_window_class. Everything else stays one primary
    // window filling the OS window, like hydra_app.py's
    // dpg.set_primary_window: docking stays off, and popups and tooltips stay
    // inside the window they open from (D103 item 15; the "Hydra:" block in
    // third_party/imgui/imgui.cpp's WindowSelectViewport). Any other OS window
    // ImGui makes gets no taskbar button. Without a platform backend (the GUI
    // test runner) ImGui turns viewports back off itself.
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigViewportsNoTaskBarIcon = true;

    // Persist ImGui state (library table column widths) next to the exe. The
    // default cwd-relative "imgui.ini" landed wherever the app happened to be
    // launched from.
    static std::string ini_file;
    if (options.ini_file == "-") {
        io.IniFilename = nullptr;
    } else {
        ini_file = options.ini_file.empty()
                       ? join_folder(app::exe_dir(), "hydra_ui.ini")
                       : options.ini_file;
        io.IniFilename = ini_file.c_str();
    }

    // Read the file now instead of on the first frame (ImGui skips its own
    // first-frame read once this has run), so a settings handler added by
    // before_ini_load has its sections read before the window is shown:
    // main.cpp puts Hydra's window back from them.
    if (options.before_ini_load) options.before_ini_load();
    if (io.IniFilename) ImGui::LoadIniSettingsFromDisk(io.IniFilename);

    ImGui::StyleColorsDark();
    apply_theme();

    // Every scale change starts again from this unscaled theme.
    g_base_style = ImGui::GetStyle();
    // Hydra sets the font scale itself, in set_ui_scale, together with the
    // sizes. With ConfigDpiScaleFonts on, ImGui would also overwrite it every
    // frame from the main viewport's DPI, a second writer that Hydra does not
    // keep in step with WM_DPICHANGED.
    io.ConfigDpiScaleFonts = false;
    set_ui_scale(options.dpi_scale);

    // Fonts (resource/ is copied beside the exe by the build; see
    // CMakeLists.txt). exe-relative, not cwd-relative: the app may be
    // launched with any working directory (e.g. a shortcut's Start-in).
    // Falls back to ImGui's built-in font if the files aren't found,
    // rather than asserting.
    const std::string fonts_dir =
        options.resource_dir.empty() ? app::resource_dir() : options.resource_dir;
    ImFont* main_font = add_font_file(join_folder(fonts_dir, "ShipporiAntiqueB1-Regular.ttf"), kFontSize);
    g_mono_font = add_font_file(join_folder(fonts_dir, "CourierPrime-Regular.ttf"), kFontSize);
    if (main_font) io.FontDefault = main_font;

    // Merge the backup fonts (fallback_fonts.h) behind both fonts, so a
    // character neither file holds draws from Windows' fonts instead of as
    // "?". DstFont names the target: without it ImGui merges into the font
    // added last. ImGui rasterizes glyphs on demand, so a backup costs nothing
    // until a character it supplies is drawn.
    static_assert(kFallbackFontFiles.size() + 1 <= 16,
                  "ImFontGlyph::SourceIdx is 4 bits: a font holds at most 16 sources");
    for (ImFont* target : {main_font, g_mono_font}) {
        if (!target) continue;
        for (std::string_view file : kFallbackFontFiles) {
            const std::string path = fallback_font_path(file);
            if (!hydra::file_exists_utf8(path)) continue;
            ImFontConfig merge;
            merge.MergeMode = true;
            merge.DstFont = target;
            add_font_file(path, kFontSize, &merge);
        }
    }
}

void shutdown_imgui() {
    ImGui::DestroyContext();
    g_font_files.clear();  // the atlas that read them is gone
}

void begin_frame_text_capture(FrameText* capture) {
    if (!capture || !capture->enabled) return;
    // ImGui's text log spans every window until EndFrame; started here,
    // on the implicit Debug window, it sees all of Hydra's windows. It
    // also un-clips table rows so off-screen rows are captured.
    ImGui::LogToBuffer();
}

void end_frame_text_capture(FrameText* capture) {
    if (!capture || !capture->enabled) return;
    ImGuiContext& g = *ImGui::GetCurrentContext();
    capture->text.assign(g.LogBuffer.begin(), g.LogBuffer.end());
    ImGui::LogFinish();  // clears the buffer
}

}  // namespace hydra::ui
