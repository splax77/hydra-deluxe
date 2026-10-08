#include "ui/app_shell.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>

#include "imgui.h"
#include "imgui_internal.h"  // g.LogBuffer for FrameText; ImGuiSettingsHandler

#include "app/config.h"
#include "audio/mapped_file.h"
#include "core/winstr.h"
#include "ui/app_state.h"
#include "ui/details_view.h"
#include "ui/fonts.h"
#include "ui/library_view.h"
#include "ui/theme.h"

namespace hydra::ui {

namespace {

// What hydra_ui.ini said, then what main.cpp reported since.
WindowPlacement g_placement;
// What hydra_ui.ini said, then what the user dragged the split to and the
// library button set since.
Layout g_layout;
// The theme at scale 1, captured by setup_imgui after apply_theme().
ImGuiStyle g_base_style;

// The size every UI font loads at, in pixels at scale 1. The CJK fallback
// merged into the main font must load at the same size: ImGui scales merged
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

// Reads "<a>,<b>" into two ints; false unless the text is exactly that.
bool read_int_pair(std::string_view text, int& a, int& b) {
    const size_t comma = text.find(',');
    if (comma == std::string_view::npos) return false;
    const char* first = text.data();
    const char* mid = first + comma;
    const char* last = first + text.size();
    const auto r1 = std::from_chars(first, mid, a);
    if (r1.ec != std::errc() || r1.ptr != mid) return false;
    const auto r2 = std::from_chars(mid + 1, last, b);
    return r2.ec == std::errc() && r2.ptr == last;
}

// The [Hydra][Window] and [Hydra][Layout] sections' handler. ImGui calls
// ReadOpen for each "[Hydra][<name>]" header and ReadLine for each line under
// it, and WriteAll whenever it saves the file.
void* placement_read_open(ImGuiContext*, ImGuiSettingsHandler*, const char* name) {
    if (std::strcmp(name, "Window") == 0) return &g_placement;
    if (std::strcmp(name, "Layout") == 0) return &g_layout;
    return nullptr;
}

void placement_read_line(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line) {
    if (entry == &g_layout)
        parse_layout_line(line, g_layout);
    else
        parse_window_placement_line(line, *static_cast<WindowPlacement*>(entry));
}

void placement_write_all(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out) {
    if (g_placement.valid) {
        out->appendf("[%s][Window]\n", handler->TypeName);
        out->append(format_window_placement(g_placement).c_str());
        out->append("\n");
    }
    out->appendf("[%s][Layout]\n", handler->TypeName);
    out->append(format_layout(g_layout).c_str());
    out->append("\n");
}

}  // namespace

bool placement_on_screen(const ScreenRect& r, const std::vector<ScreenRect>& work_areas) {
    if (r.width() < kMinVisiblePx || r.height() < kMinVisiblePx) return false;
    const ScreenRect title_band{r.left, r.top, r.right, r.top + kMinVisiblePx};
    for (const ScreenRect& area : work_areas) {
        const int w = std::min(title_band.right, area.right) - std::max(title_band.left, area.left);
        const int h = std::min(title_band.bottom, area.bottom) - std::max(title_band.top, area.top);
        if (w >= kMinVisiblePx && h >= kMinVisiblePx / 2) return true;
    }
    return false;
}

std::string format_window_placement(const WindowPlacement& p) {
    return "Pos=" + std::to_string(p.normal.left) + "," + std::to_string(p.normal.top) + "\n" +
           "Size=" + std::to_string(p.normal.width()) + "," + std::to_string(p.normal.height()) +
           "\n" + "Maximized=" + (p.maximized ? "1" : "0") + "\n";
}

void parse_window_placement_line(std::string_view line, WindowPlacement& p) {
    int a = 0, b = 0;
    if (line.substr(0, 4) == "Pos=" && read_int_pair(line.substr(4), a, b)) {
        const int w = p.normal.width(), h = p.normal.height();
        p.normal = {a, b, a + w, b + h};
    } else if (line.substr(0, 5) == "Size=" && read_int_pair(line.substr(5), a, b)) {
        p.normal.right = p.normal.left + a;
        p.normal.bottom = p.normal.top + b;
    } else if (line == "Maximized=1") {
        p.maximized = true;
    } else if (line == "Maximized=0") {
        p.maximized = false;
    }
    p.valid = p.normal.width() > 0 && p.normal.height() > 0;
}

WindowPlacement window_placement() { return g_placement; }

void remember_window_placement(const WindowPlacement& p) {
    if (p == g_placement) return;
    g_placement = p;
    // WndProc can run before the context exists (CreateWindow sends its
    // first WM_SIZE early); the placement is kept either way.
    if (ImGui::GetCurrentContext()) ImGui::MarkIniSettingsDirty();
}

std::string format_layout(const Layout& layout) {
    char text[64];
    std::snprintf(text, sizeof(text), "LibraryShare=%.4f\nLibraryHidden=%d\n",
                  layout.library_share, layout.library_hidden ? 1 : 0);
    return text;
}

void parse_layout_line(std::string_view line, Layout& layout) {
    if (line == "LibraryHidden=1") {
        layout.library_hidden = true;
        return;
    }
    if (line == "LibraryHidden=0") {
        layout.library_hidden = false;
        return;
    }
    constexpr std::string_view kKey = "LibraryShare=";
    if (line.substr(0, kKey.size()) != kKey) return;
    const std::string value(line.substr(kKey.size()));
    char* end = nullptr;
    const float v = std::strtof(value.c_str(), &end);
    if (end == value.c_str() || *end != '\0' || !share_is_valid(v)) return;
    layout.library_share = v;
}

bool share_is_valid(float share) { return share > 0.0f && share < 1.0f; }

float library_share() { return g_layout.library_share; }

void remember_library_share(float share) {
    if (!share_is_valid(share) || share == g_layout.library_share) return;
    g_layout.library_share = share;
    if (ImGui::GetCurrentContext()) ImGui::MarkIniSettingsDirty();
}

bool library_hidden() { return g_layout.library_hidden; }

void remember_library_hidden(bool hidden) {
    if (hidden == g_layout.library_hidden) return;
    g_layout.library_hidden = hidden;
    if (ImGui::GetCurrentContext()) ImGui::MarkIniSettingsDirty();
}

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
    // dpg.set_primary_window: docking stays off, and a popup or tooltip only
    // gets its own borderless OS window when it would otherwise be cut off at
    // the main window's edge. Those get no taskbar button. Without a platform
    // backend (the GUI test runner) ImGui turns viewports back off itself.
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

    // The main window's placement lives in the same file, as a
    // [Hydra][Window] section. Read the file now instead of on the first
    // frame (ImGui skips its own first-frame read once this has run), so
    // main.cpp can put the window back before it is shown.
    g_placement = WindowPlacement{};
    g_layout = Layout{};
    ImGuiSettingsHandler placement_handler;
    placement_handler.TypeName = "Hydra";
    placement_handler.TypeHash = ImHashStr("Hydra");
    placement_handler.ReadOpenFn = placement_read_open;
    placement_handler.ReadLineFn = placement_read_line;
    placement_handler.WriteAllFn = placement_write_all;
    ImGui::AddSettingsHandler(&placement_handler);  // ImGui keeps a copy
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

    // CJK fallback: Clone Hero libraries are full of Japanese (and other
    // non-Latin) titles, which rendered as ?/boxes with the Latin-only fonts.
    // Merge the first available system font into the main font; ImGui's
    // dynamic font loader rasterizes glyphs on demand, so this costs nothing
    // until a non-Latin title is actually drawn.
    if (main_font) {
        const char* cjk_candidates[] = {
            "C:\\Windows\\Fonts\\YuGothM.ttc",   // Yu Gothic Medium (Win 8.1+)
            "C:\\Windows\\Fonts\\meiryo.ttc",    // Meiryo
            "C:\\Windows\\Fonts\\msgothic.ttc",  // MS Gothic (bitmap-ish, last resort)
        };
        for (const char* path : cjk_candidates) {
            if (!hydra::file_exists_utf8(path)) continue;
            ImFontConfig merge;
            merge.MergeMode = true;
            if (add_font_file(path, kFontSize, &merge)) break;
        }
    }
}

void shutdown_imgui() {
    ImGui::DestroyContext();
    g_font_files.clear();  // the atlas that read them is gone
}

namespace {

// Report windows still being given the first-open placement: from the frame
// they open until ImGui has finished fitting their size to their content.
std::unordered_set<std::string> g_first_open_placing;

ScreenRect screen_rect(const ImVec2& pos, const ImVec2& size) {
    const int left = static_cast<int>(pos.x), top = static_cast<int>(pos.y);
    return {left, top, left + static_cast<int>(size.x), top + static_cast<int>(size.y)};
}

// Every monitor's work area, as the platform backend reported them. With no
// backend (the GUI test runner), ImGui's stand-in monitor: the main viewport.
// main.cpp's monitor_work_areas asks Windows directly, because it runs before
// the backend exists; this list is in ImGui's own coordinates and also
// answers in the test runner, where the windows are not on any real monitor.
std::vector<ScreenRect> imgui_work_areas() {
    std::vector<ScreenRect> areas;
    for (const ImGuiPlatformMonitor& m : ImGui::GetPlatformIO().Monitors)
        areas.push_back(screen_rect(m.WorkPos, m.WorkSize));
    if (areas.empty()) {
        const ImGuiPlatformMonitor* m = ImGui::GetViewportPlatformMonitor(ImGui::GetMainViewport());
        areas.push_back(screen_rect(m->WorkPos, m->WorkSize));
    }
    return areas;
}

// Where the window was last: the live window when it has shown this session,
// else its hydra_ui.ini entry (ImGui saves a window that had its own OS
// window relative to that window, and any other relative to the main
// window). Nothing when there is neither.
std::optional<ScreenRect> saved_rect(const char* name) {
    if (const ImGuiWindow* w = ImGui::FindWindowByName(name))
        return screen_rect(w->Pos, w->SizeFull);
    const ImGuiWindowSettings* s = ImGui::FindWindowSettingsByID(ImHashStr(name));
    if (!s) return std::nullopt;
    const ImVec2 origin = s->ViewportId ? ImVec2(s->ViewportPos.x, s->ViewportPos.y)
                                        : ImGui::GetMainViewport()->Pos;
    return screen_rect(ImVec2(origin.x + s->Pos.x, origin.y + s->Pos.y),
                       ImVec2(s->Size.x, s->Size.y));
}

// Whether the spike window shows; its own close button clears it.
bool g_report_spike = false;

// The spike window: empty, in the report windows' class and placement.
void render_report_spike() {
    if (!g_report_spike) return;
    constexpr const char* kName = "Report spike";
    place_report_window(kName);
    const ImGuiWindowClass window_class = report_window_class();
    ImGui::SetNextWindowClass(&window_class);
    // No collapse arrow: a report window minimizes with its OS title bar.
    ImGui::Begin(kName, &g_report_spike, ImGuiWindowFlags_NoCollapse);
    ImGui::End();
}

}  // namespace

ImGuiWindowClass report_window_class() {
    ImGuiWindowClass c;
    c.ParentViewportId = ImGui::GetMainViewport()->ID;
    c.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
    c.ViewportFlagsOverrideClear = ImGuiViewportFlags_NoDecoration | ImGuiViewportFlags_NoTaskBarIcon;
    return c;
}

void place_report_window(const char* name) {
    const ImGuiWindow* w = ImGui::FindWindowByName(name);
    if (!w || !w->WasActive) {  // opening this frame
        const std::optional<ScreenRect> saved = saved_rect(name);
        if (saved && placement_on_screen(*saved, imgui_work_areas()))
            g_first_open_placing.erase(name);
        else
            g_first_open_placing.insert(name);
    } else if (w->AutoFitFramesX <= 0 && w->AutoFitFramesY <= 0) {
        g_first_open_placing.erase(name);  // its size has settled
    }
    if (g_first_open_placing.count(name) == 0) return;

    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    const ImGuiPlatformMonitor* monitor = ImGui::GetViewportPlatformMonitor(main_viewport);
    const ImVec2 work_min = monitor->WorkPos;
    const ImVec2 work_max(work_min.x + monitor->WorkSize.x, work_min.y + monitor->WorkSize.y);
    const ImVec2 centre = main_viewport->GetCenter();
    ImGui::SetNextWindowPos(ImVec2(std::clamp(centre.x, work_min.x, work_max.x),
                                   std::clamp(centre.y, work_min.y, work_max.y)),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), monitor->WorkSize);
}

void show_report_spike(bool shown) { g_report_spike = shown; }
bool report_spike_shown() { return g_report_spike; }

void run_frame(AppState& app, FrameText* capture) {
    const bool capturing = capture && capture->enabled;
    if (capturing) {
        // ImGui's text log spans every window until EndFrame; started here,
        // on the implicit Debug window, it sees all of Hydra's windows. It
        // also un-clips table rows so off-screen rows are captured.
        ImGui::LogToBuffer();
    }

    // State first, once per frame: the panel's closing edge, storing and
    // reaping the analyze job, a finished Dynamics count.
    app.tick(ImGui::GetTime());
    // Batch ends, report starts and reaps, parked leaderboard jobs: state
    // work, done here once a frame rather than inside a view.
    app.update_background_jobs();
    // The main window is the only thing that reaches the store, so until the
    // startup open has brought it, the startup screen draws instead.
    if (app.store_ready())
        render_main_window(app);
    else
        render_startup_screen(app);
    render_report_spike();
    // The number boxes apply edits live but leave the INI until the edit
    // ends (AppState::edit_settings). An edit has ended once no widget is
    // active: the +/- button is released, or the text box lost focus.
    if (!ImGui::IsAnyItemActive()) app.flush_settings();

    if (capturing) {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        capture->text.assign(g.LogBuffer.begin(), g.LogBuffer.end());
        ImGui::LogFinish();  // clears the buffer
    }
}

}  // namespace hydra::ui
