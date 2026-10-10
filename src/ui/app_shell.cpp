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
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>

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
// Each report window's placement by report_placement_key: what hydra_ui.ini
// said, then what place_report_window noted since. A map, so the pointer
// placement_read_open hands ImGui stays put as entries are added.
std::map<std::string, WindowPlacement, std::less<>> g_report_placements;
// The OS frame around a report window, from main.cpp.
FrameInsets g_report_frame;
// The reports that opened maximized and wait for ImGui to make their OS
// window, by report_placement_key.
std::set<std::string, std::less<>> g_maximize_pending;
// A report's [Hydra] section is "[Hydra][Window:<key>]".
constexpr std::string_view kReportSectionPrefix = "Window:";

// Asks ImGui to save hydra_ui.ini soon. WndProc can run before the context
// exists (CreateWindow sends its first WM_SIZE early); a remembered value is
// kept either way.
void mark_ini_dirty() {
    if (ImGui::GetCurrentContext()) ImGui::MarkIniSettingsDirty();
}

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

// The [Hydra][Window], [Hydra][Window:<key>] and [Hydra][Layout] sections'
// handler. ImGui calls ReadOpen for each "[Hydra][<name>]" header and
// ReadLine for each line under it, and WriteAll whenever it saves the file.
void* placement_read_open(ImGuiContext*, ImGuiSettingsHandler*, const char* name) {
    if (std::strcmp(name, "Window") == 0) return &g_placement;
    if (std::strcmp(name, "Layout") == 0) return &g_layout;
    const std::string_view section(name);
    if (section.substr(0, kReportSectionPrefix.size()) == kReportSectionPrefix &&
        section.size() > kReportSectionPrefix.size())
        return &g_report_placements[std::string(section.substr(kReportSectionPrefix.size()))];
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
    for (const auto& [key, placement] : g_report_placements) {
        if (!placement.valid) continue;
        out->appendf("[%s][%.*s%s]\n", handler->TypeName,
                     static_cast<int>(kReportSectionPrefix.size()), kReportSectionPrefix.data(),
                     key.c_str());
        out->append(format_window_placement(placement).c_str());
        out->append("\n");
    }
    out->appendf("[%s][Layout]\n", handler->TypeName);
    out->append(format_layout(g_layout).c_str());
    out->append("\n");
}

}  // namespace

namespace {

// The band along the top of `r` where its title bar is.
ScreenRect title_band(const ScreenRect& r) {
    return {r.left, r.top, r.right, r.top + kMinVisiblePx};
}

// How far two rectangles overlap across and down; zero or less on an axis
// where they miss.
struct Overlap {
    int w, h;
};
Overlap overlap_of(const ScreenRect& a, const ScreenRect& b) {
    return {std::min(a.right, b.right) - std::max(a.left, b.left),
            std::min(a.bottom, b.bottom) - std::max(a.top, b.top)};
}

}  // namespace

bool placement_on_screen(const ScreenRect& r, const std::vector<ScreenRect>& work_areas) {
    if (r.width() < kMinVisiblePx || r.height() < kMinVisiblePx) return false;
    const ScreenRect band = title_band(r);
    for (const ScreenRect& area : work_areas) {
        const Overlap o = overlap_of(band, area);
        if (o.w >= kMinVisiblePx && o.h >= kMinVisiblePx / 2) return true;
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
    mark_ini_dirty();
}

std::string report_placement_key(std::string_view window_name) {
    const size_t id = window_name.find("###");
    return std::string(id == std::string_view::npos ? window_name : window_name.substr(id + 3));
}

WindowPlacement report_window_placement(std::string_view key) {
    const auto it = g_report_placements.find(key);
    return it == g_report_placements.end() ? WindowPlacement{} : it->second;
}

void remember_report_placement(std::string_view key, const WindowPlacement& p) {
    if (p == report_window_placement(key)) return;
    g_report_placements.insert_or_assign(std::string(key), p);
    mark_ini_dirty();
}

void set_report_frame_insets(const FrameInsets& frame) { g_report_frame = frame; }

namespace {

ScreenRect grown(const ScreenRect& r, const FrameInsets& f) {
    return {r.left - f.left, r.top - f.top, r.right + f.right, r.bottom + f.bottom};
}

ScreenRect shrunk(const ScreenRect& r, const FrameInsets& f) {
    return {r.left + f.left, r.top + f.top, r.right - f.right, r.bottom - f.bottom};
}

// The work area `r` overlaps most, or nullptr when it overlaps none.
const ScreenRect* area_overlapping_most(const ScreenRect& r, const std::vector<ScreenRect>& areas) {
    const ScreenRect* best = nullptr;
    long long best_overlap = 0;
    for (const ScreenRect& area : areas) {
        const Overlap o = overlap_of(r, area);
        if (o.w <= 0 || o.h <= 0) continue;
        const long long size = static_cast<long long>(o.w) * o.h;
        if (size > best_overlap) {
            best_overlap = size;
            best = &area;
        }
    }
    return best;
}

// `outer` no bigger than `area`, then moved the least distance that puts it
// inside.
ScreenRect fitted_into(ScreenRect outer, const ScreenRect& area) {
    outer.right = outer.left + std::min(outer.width(), area.width());
    outer.bottom = outer.top + std::min(outer.height(), area.height());
    int dx = 0, dy = 0;
    if (outer.right > area.right) dx = area.right - outer.right;
    if (outer.left + dx < area.left) dx = area.left - outer.left;
    if (outer.bottom > area.bottom) dy = area.bottom - outer.bottom;
    if (outer.top + dy < area.top) dy = area.top - outer.top;
    return {outer.left + dx, outer.top + dy, outer.right + dx, outer.bottom + dy};
}

}  // namespace

ReportPlacement report_placement(const std::optional<ScreenRect>& saved_client,
                                 bool saved_maximized,
                                 bool main_maximized,
                                 const ScreenRect& main_client,
                                 const std::vector<ScreenRect>& work_areas,
                                 const FrameInsets& frame) {
    const ScreenRect* main_area = area_overlapping_most(main_client, work_areas);
    if (!main_area && !work_areas.empty()) main_area = &work_areas.front();

    if (main_maximized || saved_maximized)
        return {main_area ? shrunk(*main_area, frame) : main_client, true};

    const ScreenRect candidate =
        saved_client && placement_on_screen(*saved_client, work_areas) ? *saved_client
                                                                       : main_client;
    const ScreenRect outer = grown(candidate, frame);
    // Taken on the outer window, where the title bar really is.
    const ScreenRect* area = area_overlapping_most(title_band(outer), work_areas);
    if (!area) area = main_area;
    if (!area) return {candidate, false};
    return {shrunk(fitted_into(outer, *area), frame), false};
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
    mark_ini_dirty();
}

bool library_hidden() { return g_layout.library_hidden; }

void remember_library_hidden(bool hidden) {
    if (hidden == g_layout.library_hidden) return;
    g_layout.library_hidden = hidden;
    mark_ini_dirty();
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

    // The main window's placement lives in the same file, as a
    // [Hydra][Window] section. Read the file now instead of on the first
    // frame (ImGui skips its own first-frame read once this has run), so
    // main.cpp can put the window back before it is shown.
    g_placement = WindowPlacement{};
    g_report_placements.clear();
    g_maximize_pending.clear();
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

// Where the window was last, un-maximized: its [Hydra][Window:<key>] entry
// when there is one (the OS window's own rectangle, noted while it showed),
// else the live window when it has shown this session (the GUI test runner,
// which has no OS windows to note), else ImGui's own [Window] entry from
// hydra_ui.ini (ImGui saves a window that had its own OS window relative to
// that window, and any other relative to the main window). Nothing when there
// is none of these.
std::optional<ScreenRect> saved_rect(const char* name, std::string_view key) {
    if (const WindowPlacement p = report_window_placement(key); p.valid) return p.normal;
    if (const ImGuiWindow* w = ImGui::FindWindowByName(name))
        return screen_rect(w->Pos, w->SizeFull);
    const ImGuiWindowSettings* s = ImGui::FindWindowSettingsByID(ImHashStr(name));
    if (!s) return std::nullopt;
    const ImVec2 origin = s->ViewportId ? ImVec2(s->ViewportPos.x, s->ViewportPos.y)
                                        : ImGui::GetMainViewport()->Pos;
    return screen_rect(ImVec2(origin.x + s->Pos.x, origin.y + s->Pos.y),
                       ImVec2(s->Size.x, s->Size.y));
}

// Every frame an open report's OS window exists: note whether it is
// maximized and, while it is neither maximized nor minimized, its client
// rectangle in screen pixels (the one Windows restores it to), then carry out
// a maximize that waited for the window. The client rectangle is read
// directly, not through GetWindowPlacement, whose rcNormalPosition is the
// outer window in workspace coordinates and would need converting back.
void follow_report_os_window(const std::string& key, const ImGuiWindow& w) {
    if (!w.ViewportOwned || !w.Viewport) return;
    const HWND hwnd = static_cast<HWND>(w.Viewport->PlatformHandleRaw);
    if (!hwnd || ::IsIconic(hwnd)) return;  // no OS window (yet, or in the test runner)

    WindowPlacement p = report_window_placement(key);
    p.maximized = ::IsZoomed(hwnd) != FALSE;
    RECT client;
    POINT origin{0, 0};
    if (!p.maximized && ::GetClientRect(hwnd, &client) && ::ClientToScreen(hwnd, &origin)) {
        p.normal = {origin.x, origin.y, origin.x + client.right, origin.y + client.bottom};
        p.valid = true;
    }
    remember_report_placement(key, p);

    if (g_maximize_pending.erase(key) > 0) ::ShowWindow(hwnd, SW_MAXIMIZE);
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
    const std::string key = report_placement_key(name);
    const ImGuiWindow* w = ImGui::FindWindowByName(name);
    if (w && w->WasActive) {  // already open: it stays where it is
        follow_report_os_window(key, *w);
        return;
    }

    const WindowPlacement saved = report_window_placement(key);
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    const ReportPlacement p =
        report_placement(saved_rect(name, key), saved.valid && saved.maximized,
                         window_placement().maximized,
                         screen_rect(main_viewport->Pos, main_viewport->Size),
                         imgui_work_areas(), g_report_frame);
    if (p.maximized)
        g_maximize_pending.insert(key);
    else
        g_maximize_pending.erase(key);

    // An explicit size also stops ImGui fitting the window to its content,
    // so one frame is enough.
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(p.client.left), static_cast<float>(p.client.top)),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(p.client.width()),
                                    static_cast<float>(p.client.height())),
                             ImGuiCond_Always);
}

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
