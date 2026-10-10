// The window-independent half of the GUI shell: ImGui context setup, the UI
// scale, the remembered window placement, and the per-frame draw, shared by
// Hydra.exe (src/ui/main.cpp) and the headless GUI test runner (tests/ui,
// docs/agents/ui-testing.md). main.cpp keeps only the Win32 window,
// swapchain, and message pump; the runner swaps those for an offscreen render
// target and the Test Engine's injected input.

#ifndef HYDRA_UI_APP_SHELL_H
#define HYDRA_UI_APP_SHELL_H

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "imgui.h"

namespace hydra::ui {

class AppState;

struct ImGuiSetupOptions {
    // Monitor DPI scale; sizes, fonts, and px() all follow it.
    float dpi_scale = 1.0f;
    // Where ImGui persists table column widths. Empty = the exe-relative
    // default (hydra_ui.ini); "-" = don't persist at all (tests).
    std::string ini_file;
    // Directory holding the fonts. Empty = app::resource_dir().
    std::string resource_dir;
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

// ---- The main window's placement, kept in hydra_ui.ini ----------------

// A rectangle in virtual-screen pixels, the units of GetWindowRect and of a
// monitor's work area. right/bottom are one past the last pixel.
struct ScreenRect {
    int left = 0, top = 0, right = 0, bottom = 0;
    int width() const { return right - left; }
    int height() const { return bottom - top; }
    // Spelled out: the project builds as C++17, which has no defaulted ==.
    bool operator==(const ScreenRect& o) const {
        return left == o.left && top == o.top && right == o.right && bottom == o.bottom;
    }
    bool operator!=(const ScreenRect& o) const { return !(*this == o); }
};

// Where the main window, or a report window, was when it last closed.
struct WindowPlacement {
    bool valid = false;  // false: nothing saved yet, so use the default
    // The un-maximized rectangle: the whole outer window for the main window
    // (what CreateWindow takes), the client area for a report window (what
    // ImGui takes).
    ScreenRect normal;
    bool maximized = false;
    bool operator==(const WindowPlacement& o) const {
        return valid == o.valid && normal == o.normal && maximized == o.maximized;
    }
    bool operator!=(const WindowPlacement& o) const { return !(*this == o); }
};

// How much of a saved window's title-bar band must sit on one monitor for
// the user to grab it: this many pixels wide, half as many tall.
inline constexpr int kMinVisiblePx = 64;

// True when the top kMinVisiblePx rows of `r` (where the title bar is)
// overlap one of `work_areas` by at least kMinVisiblePx wide and
// kMinVisiblePx / 2 tall. False for a rectangle smaller than that.
bool placement_on_screen(const ScreenRect& r, const std::vector<ScreenRect>& work_areas);

// The body of the [Hydra][Window] section: "Pos=x,y", "Size=w,h" and
// "Maximized=0|1", one per line. Parsing takes one line at a time, in any
// order, and ignores lines it doesn't know.
std::string format_window_placement(const WindowPlacement& p);
void parse_window_placement_line(std::string_view line, WindowPlacement& p);

// The placement setup_imgui read from hydra_ui.ini, updated by every
// remember_window_placement since. valid is false when there was none.
WindowPlacement window_placement();
// Records where the window is now. It reaches hydra_ui.ini with ImGui's own
// settings: a few seconds later, and again when ImGui shuts down.
void remember_window_placement(const WindowPlacement& p);

// ---- The library/song-panel split, kept in hydra_ui.ini ---------------

// The library's share of the main window's width while the song panel is
// open, until the user drags the edge between them.
inline constexpr float kDefaultLibraryShare = 0.4f;

// The [Hydra][Layout] section: the split, and whether the song panel's "Hide
// library" button has the library hidden.
struct Layout {
    float library_share = kDefaultLibraryShare;
    bool library_hidden = false;
};

// The section's body: "LibraryShare=<0..1>" and "LibraryHidden=<0|1>".
// Parsing takes one line and ignores lines it doesn't know, shares outside
// (0, 1) and hidden values other than 0 and 1.
std::string format_layout(const Layout& layout);
void parse_layout_line(std::string_view line, Layout& layout);

// True when `share` is strictly between 0 and 1, so both the library and the
// song panel keep some width. False for 0, 1 and NaN.
bool share_is_valid(float share);

// The share setup_imgui read from hydra_ui.ini (kDefaultLibraryShare when
// there was none), updated by every remember_library_share since.
float library_share();
// Records a split the user dragged to. It reaches hydra_ui.ini with ImGui's
// own settings, like the window placement.
void remember_library_share(float share);

// Whether the library is hidden while a song is open, as hydra_ui.ini said
// and the "Hide library" / "Show library" button set since. It only hides the
// library beside an open song; with no song open the library always shows.
bool library_hidden();
void remember_library_hidden(bool hidden);

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

// ---- Report windows: their own OS windows (D103) -----------------------

// The window class every report window carries. It always gets its own OS
// window (never merged into the main window), with a real title bar and its
// own taskbar button, and Hydra's main window as its OS owner, so it
// minimizes and restores with Hydra and sits in front of it. Every other
// ImGui window keeps setup_imgui's settings. Pass it to SetNextWindowClass.
ImGuiWindowClass report_window_class();

// The OS frame around a report window's client area, in pixels on each side:
// what Windows adds to the rectangle ImGui asks for. main.cpp measures it for
// the style the Win32 backend gives a report window; the GUI test runner has
// no OS windows and leaves it at zero.
struct FrameInsets {
    int left = 0, top = 0, right = 0, bottom = 0;
};

// Where a report window opens: its client rectangle (ImGui's own units: the
// area inside the OS frame) and whether it opens maximized.
struct ReportPlacement {
    ScreenRect client;
    bool maximized = false;
    bool operator==(const ReportPlacement& o) const {
        return client == o.client && maximized == o.maximized;
    }
};

// Where a report window opens, worked out with no window. `saved_client` and
// `saved_maximized` are what the report was last time (nothing for a first
// open); `main_maximized` and `main_client` describe the main window now.
//
// A report opens maximized when the main window is maximized or the report
// was saved maximized; its client rectangle is then the main window's work
// area less the frame (Windows sizes a maximized window itself, so this is
// only the size until it does). Otherwise it reopens at the saved rectangle
// when placement_on_screen passes, else over the main window, and its whole
// outer window, frame included, is fitted into the work area holding its
// title bar: shrunk to that area's size, then moved inside it.
ReportPlacement report_placement(const std::optional<ScreenRect>& saved_client,
                                 bool saved_maximized,
                                 bool main_maximized,
                                 const ScreenRect& main_client,
                                 const std::vector<ScreenRect>& work_areas,
                                 const FrameInsets& frame);

// The frame report_placement uses, from main.cpp: at startup and whenever
// the main window lands on a monitor with another DPI.
void set_report_frame_insets(const FrameInsets& frame);

// The name a report's placement is kept under in hydra_ui.ini, as a
// [Hydra][Window:<key>] section: the window name's "###" ID part
// ("pathreport" for "Path report — Hydra###pathreport"), or the whole name
// when it has none.
std::string report_placement_key(std::string_view window_name);

// A report's saved placement, from its [Hydra][Window:<key>] section, then
// whatever place_report_window noted since. For a report, `normal` is the
// client rectangle, as report_placement takes it. valid is false when
// nothing was saved.
WindowPlacement report_window_placement(std::string_view key);
// Records a report's placement; hydra_ui.ini is marked for saving only when
// it changed.
void remember_report_placement(std::string_view key, const WindowPlacement& p);

// Call before Begin(name) on every frame the report window shows.
//
// When the window opens, it is placed by report_placement, from the saved
// placement (the [Hydra][Window:<key>] section, else ImGui's own [Window]
// entry, from a hydra_ui.ini written before that section existed) and the
// main window's. A report that opens maximized is maximized on a later
// frame, once ImGui has made its OS window (D103 item 14).
//
// While the report's OS window exists, every frame notes whether it is
// maximized and, when it is neither maximized nor minimized, its client
// rectangle, through remember_report_placement.
void place_report_window(const char* name);

// Everything ImGui drew this frame, as text, in draw order. Filled by
// run_frame when `enabled`; the runner's wait-text/expect-text search it.
struct FrameText {
    bool enabled = false;
    std::string text;
};

// One frame of Hydra's UI between ImGui::NewFrame() and ImGui::Render():
// the library window and the details modal.
void run_frame(AppState& app, FrameText* capture = nullptr);

}  // namespace hydra::ui

#endif  // HYDRA_UI_APP_SHELL_H
