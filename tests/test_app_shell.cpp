// Unit tests for the window-placement half of ui/app_shell and the DPI and
// font half of ui/imgui_setup: which saved rectangles are safe to reopen at,
// the hydra_ui.ini text, the UI scale, and how the fonts are loaded. A test can't move a real window
// between monitors, so the Win32 side in main.cpp stays thin and everything
// it decides is tested here.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>  // VirtualQuery, to see the fonts are file-mapped

#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"  // SettingsDirtyTimer, to see what marks the ini for saving

#include "headless_imgui_fixture.h"
#include "temp_util.h"
#include "testdata_dirs.h"  // resource_root
#include "ui/app_shell.h"
#include "ui/fonts.h"

using hydra::ui::ScreenRect;
using hydra::ui::WindowPlacement;
using hydra::ui::placement_on_screen;

namespace {

const std::vector<ScreenRect> kOneMonitor = {{0, 0, 1920, 1040}};
const std::vector<ScreenRect> kTwoMonitors = {{0, 0, 1920, 1040}, {-1920, 0, 0, 1080}};

hydra::ui::ImGuiSetupOptions test_options(const std::string& ini_file) {
    hydra::ui::ImGuiSetupOptions opts;
    opts.dpi_scale = 1.0f;
    opts.ini_file = ini_file;
    opts.resource_dir = corpus::resource_root();
    opts.before_ini_load = hydra::ui::install_placement_handler;
    return opts;
}

// ImGui set up on `ini_file`, shut down when the guard goes out of scope, so
// a failed REQUIRE leaves no context alive for the next test. end() shuts it
// down early, for a test that reads the ini DestroyContext writes.
class ImGuiOnIni {
public:
    explicit ImGuiOnIni(const std::string& ini_file) {
        hydra::ui::setup_imgui(test_options(ini_file));
    }
    ~ImGuiOnIni() { end(); }
    void end() {
        if (!live_) return;
        live_ = false;
        hydra::ui::shutdown_imgui();
    }
    ImGuiOnIni(const ImGuiOnIni&) = delete;
    ImGuiOnIni& operator=(const ImGuiOnIni&) = delete;

private:
    bool live_ = true;
};

}  // namespace

TEST_CASE("app_shell: a saved window is reopened only where its title bar can be reached") {
    using hydra::ui::kMinVisiblePx;
    const ScreenRect& area = kOneMonitor.front();
    CHECK(placement_on_screen({100, 100, 1380, 820}, kOneMonitor));
    CHECK_FALSE(placement_on_screen({5000, 100, 6280, 820}, kOneMonitor));  // monitor gone
    // Hanging off the left edge: one pixel too little of the title bar's width
    // showing fails, exactly kMinVisiblePx passes.
    CHECK_FALSE(placement_on_screen({-1250, 100, area.left + kMinVisiblePx - 1, 820}, kOneMonitor));
    CHECK(placement_on_screen({-1180, 100, area.left + kMinVisiblePx, 820}, kOneMonitor));
    // Hanging off the bottom edge: the title bar must show kMinVisiblePx / 2 tall.
    const int half = kMinVisiblePx / 2;
    CHECK_FALSE(placement_on_screen({100, area.bottom - (half - 1), 1380, area.bottom - (half - 1) + 720},
                                    kOneMonitor));
    CHECK(placement_on_screen({100, area.bottom - half, 1380, area.bottom - half + 720}, kOneMonitor));
    CHECK_FALSE(placement_on_screen({100, -700, 1380, 20}, kOneMonitor));   // title bar above
    CHECK_FALSE(placement_on_screen({100, 100, 100, 100}, kOneMonitor));    // no size
    CHECK_FALSE(placement_on_screen({-1800, 40, -400, 940}, kOneMonitor));
    CHECK(placement_on_screen({-1800, 40, -400, 940}, kTwoMonitors));      // left monitor
    CHECK_FALSE(placement_on_screen({100, 100, 1380, 820}, {}));           // no monitors known
}

TEST_CASE("app_shell: the [Hydra][Window] text round-trips") {
    WindowPlacement p;
    p.valid = true;
    p.normal = {-1700, 40, -300, 940};
    p.maximized = true;
    const std::string text = hydra::ui::format_window_placement(p);
    // Size is right minus left and bottom minus top of the rectangle above.
    CHECK(text == "Pos=-1700,40\nSize=" + std::to_string(-300 - -1700) + "," +
                      std::to_string(940 - 40) + "\nMaximized=1\n");

    WindowPlacement back;
    std::istringstream lines(text);
    for (std::string line; std::getline(lines, line);)
        hydra::ui::parse_window_placement_line(line, back);
    CHECK(back == p);

    // Size before Pos reads the same.
    WindowPlacement reordered;
    hydra::ui::parse_window_placement_line("Size=1400,900", reordered);
    hydra::ui::parse_window_placement_line("Pos=-1700,40", reordered);
    CHECK(reordered.normal == p.normal);
    CHECK(reordered.valid);

    // Lines Hydra doesn't know, or a zero size, leave nothing usable.
    WindowPlacement junk;
    hydra::ui::parse_window_placement_line("Pos=abc,1", junk);
    hydra::ui::parse_window_placement_line("Size=0,0", junk);
    hydra::ui::parse_window_placement_line("Colour=blue", junk);
    CHECK_FALSE(junk.valid);
}

// ---- Report windows: report_placement --------------------------------------
//
// The user's monitor, 2,560 x 1,440, as the hydra_ui.ini line in the first
// test below shows. Here it has a stand-in 48-pixel taskbar at the bottom, so
// the work area is 2,560 x 1,392.
const std::vector<ScreenRect> kUserMonitor = {{0, 0, 2560, 1392}};
// The same monitor with a 1,920 x 1,080 one to its right, 40-pixel taskbar.
const std::vector<ScreenRect> kUserTwoMonitors = {{0, 0, 2560, 1392}, {2560, 0, 4480, 1040}};
// The frame Windows puts around a report window at 96 DPI. Measured on
// Windows 11, build 26200, with one call,
// AdjustWindowRectExForDpi(&r, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW, 96)
// on an empty rectangle, which came back {-8, -31, 8, 8}.
const hydra::ui::FrameInsets kFrame96 = {8, 31, 8, 8};
// A normal main window: outer {100, 100, 1380, 820}, so this client area.
const ScreenRect kMainClient = {108, 131, 1372, 812};

using hydra::ui::ReportPlacement;
using hydra::ui::report_placement;

namespace {

// `client` with the OS frame around it: the whole outer window.
ScreenRect outer_of(const ScreenRect& client, const hydra::ui::FrameInsets& f) {
    return {client.left - f.left, client.top - f.top, client.right + f.right, client.bottom + f.bottom};
}

// The client area of an outer window that covers `area` exactly.
ScreenRect inside_frame(const ScreenRect& area, const hydra::ui::FrameInsets& f) {
    return {area.left + f.left, area.top + f.top, area.right - f.right, area.bottom - f.bottom};
}

// `p` saved under `key` the way Hydra saves it: remembered, written to a
// hydra_ui.ini by ImGui's shutdown, then read back from that file by a fresh
// setup_imgui.
WindowPlacement saved_by_hydra(const std::string& key, const WindowPlacement& p) {
    const std::filesystem::path dir = hydra::os_path(testtemp::temp_dir("app_shell_saved_" + key));
    const std::filesystem::path ini = dir / "hydra_ui.ini";
    {
        ImGuiOnIni imgui(ini.string());
        hydra::ui::remember_report_placement(key, p);
    }  // DestroyContext writes the ini
    WindowPlacement back;
    {
        ImGuiOnIni imgui(ini.string());
        back = hydra::ui::report_window_placement(key);
    }
    std::filesystem::remove_all(dir);
    return back;
}

}  // namespace

TEST_CASE("app_shell: a saved report bigger than the screen is fitted, frame included") {
    // The pathreport entry ImGui wrote in the user's hydra_ui.ini
    // (build-cpp/Release/hydra_ui.ini, [Window][pathreport]):
    // ViewportPos=0,23 Size=2560,1417, the whole monitor below a 23-pixel band.
    const ScreenRect saved{0, 23, 0 + 2560, 23 + 1417};
    const ScreenRect& area = kUserMonitor.front();
    REQUIRE(outer_of(saved, kFrame96).width() > area.width());
    REQUIRE(outer_of(saved, kFrame96).height() > area.height());

    const ReportPlacement p = report_placement(saved, false, false, kMainClient, kUserMonitor, kFrame96);
    // Too big both ways, so the outer window, frame included, is cut down to
    // the work area and sits on it exactly.
    CHECK(outer_of(p.client, kFrame96) == area);
    CHECK_FALSE(p.maximized);
}

TEST_CASE("app_shell: a report opens maximized when the main window is maximized") {
    const ScreenRect& area = kUserMonitor.front();
    // The main window maximized on that monitor covers its work area.
    const ScreenRect main_maximized_client = area;
    const ReportPlacement p = report_placement(std::nullopt, false, true, main_maximized_client,
                                               kUserMonitor, kFrame96);
    CHECK(p.client == inside_frame(area, kFrame96));
    CHECK(p.maximized);

    // A saved, fitting rectangle does not stop it.
    const ScreenRect fitting{300, 200, 1500, 900};
    REQUIRE(placement_on_screen(fitting, kUserMonitor));
    const ReportPlacement saved = report_placement(fitting, false, true, main_maximized_client,
                                                   kUserMonitor, kFrame96);
    CHECK(saved.client == inside_frame(area, kFrame96));
    CHECK(saved.maximized);
}

TEST_CASE("app_shell: a first open from a normal main window copies it and fits") {
    const ScreenRect& area = kUserMonitor.front();
    // kMainClient's outer window is inside the work area: unchanged.
    const ScreenRect main_outer = outer_of(kMainClient, kFrame96);
    REQUIRE(main_outer.left >= area.left);
    REQUIRE(main_outer.top >= area.top);
    REQUIRE(main_outer.right <= area.right);
    REQUIRE(main_outer.bottom <= area.bottom);
    const ReportPlacement p = report_placement(std::nullopt, false, false, kMainClient,
                                               kUserMonitor, kFrame96);
    CHECK(p.client == kMainClient);
    CHECK_FALSE(p.maximized);

    // A main window hanging off the bottom-right corner. Its outer window
    // fits the work area in size, so it is only moved back by its overhang:
    // the work area's right and bottom less the outer window's.
    const ScreenRect corner_main{1600, 1000, 2600, 1400};
    const ScreenRect corner_outer = outer_of(corner_main, kFrame96);
    REQUIRE(corner_outer.width() <= area.width());
    REQUIRE(corner_outer.height() <= area.height());
    const int dx = area.right - corner_outer.right;
    const int dy = area.bottom - corner_outer.bottom;
    REQUIRE(dx < 0);
    REQUIRE(dy < 0);
    const ReportPlacement corner = report_placement(std::nullopt, false, false, corner_main,
                                                    kUserMonitor, kFrame96);
    CHECK(corner.client == ScreenRect{corner_main.left + dx, corner_main.top + dy,
                                      corner_main.right + dx, corner_main.bottom + dy});
    CHECK_FALSE(corner.maximized);
}

TEST_CASE("app_shell: a saved report on a monitor that is gone opens over the main window") {
    // Saved on the right-hand monitor, through Hydra's own hydra_ui.ini text.
    WindowPlacement on_right;
    on_right.valid = true;
    on_right.normal = {2600, 60, 4400, 1000};
    const WindowPlacement saved = saved_by_hydra("pathreport", on_right);
    REQUIRE(saved.valid);
    REQUIRE(placement_on_screen(saved.normal, kUserTwoMonitors));
    // Then the right-hand monitor is unplugged: only the left-hand one is left.
    const std::vector<ScreenRect> left_only = {kUserTwoMonitors.front()};
    REQUIRE_FALSE(placement_on_screen(saved.normal, left_only));

    const ReportPlacement p = report_placement(saved.normal, saved.maximized, false, kMainClient,
                                               left_only, kFrame96);
    CHECK(p.client == kMainClient);
    CHECK_FALSE(p.maximized);
}

TEST_CASE("app_shell: a report saved maximized reopens maximized") {
    const ScreenRect& area = kUserMonitor.front();
    const ScreenRect saved{300, 200, 1500, 900};
    REQUIRE(placement_on_screen(saved, kUserMonitor));
    const ReportPlacement p = report_placement(saved, true, false, kMainClient, kUserMonitor, kFrame96);
    CHECK(p.client == inside_frame(area, kFrame96));
    CHECK(p.maximized);
}

// The user's decision of 2026-10-10: a report saved maximized reopens
// maximized on the monitor it was saved on, not the main window's.
TEST_CASE("app_shell: a report saved maximized on a second monitor reopens maximized there") {
    // Saved on the right-hand monitor: outer {2592, 29, 4408, 1008}, title band
    // all on that monitor. The main window is on the left-hand one. The client
    // is the right-hand work area less the frame: {2560+8, 0+31, 4480-8, 1040-8}.
    const ReportPlacement p = report_placement(ScreenRect{2600, 60, 4400, 1000}, true, false,
                                               kMainClient, kUserTwoMonitors, kFrame96);
    CHECK(p.client == ScreenRect{2568, 31, 4472, 1032});
    CHECK(p.maximized);

    // The main window maximized on the left-hand monitor does not pull it over.
    const ReportPlacement main_max = report_placement(ScreenRect{2600, 60, 4400, 1000}, true, true,
                                                      ScreenRect{0, 23, 2560, 1392},
                                                      kUserTwoMonitors, kFrame96);
    CHECK(main_max.client == ScreenRect{2568, 31, 4472, 1032});
    CHECK(main_max.maximized);
}

TEST_CASE("app_shell: a report saved maximized on a monitor that is gone opens on the main window's") {
    // The same saved rectangle with only the left-hand monitor left: it fails
    // placement_on_screen, so the client is that work area less the frame.
    const ReportPlacement p = report_placement(ScreenRect{2600, 60, 4400, 1000}, true, false,
                                               kMainClient, kUserMonitor, kFrame96);
    CHECK(p.client == ScreenRect{8, 31, 2552, 1384});
    CHECK(p.maximized);
}

TEST_CASE("app_shell: a first open with the main window maximized uses the main window's monitor") {
    // The main window maximized on the right-hand monitor, no saved placement:
    // the right-hand work area less the frame.
    const ReportPlacement p = report_placement(std::nullopt, false, true,
                                               ScreenRect{2560, 23, 4480, 1040}, kUserTwoMonitors,
                                               kFrame96);
    CHECK(p.client == ScreenRect{2568, 31, 4472, 1032});
    CHECK(p.maximized);
}

TEST_CASE("app_shell: a saved report is fitted into the monitor holding its title bar") {
    // Fits on the right-hand monitor: outer {2592, 29, 4408, 1008}. Unchanged.
    const ReportPlacement fits = report_placement(ScreenRect{2600, 60, 4400, 1000}, false, false,
                                                  kMainClient, kUserTwoMonitors, kFrame96);
    CHECK(fits.client == ScreenRect{2600, 60, 4400, 1000});
    CHECK_FALSE(fits.maximized);

    // That monitor's whole height below a 23-pixel band. Outer
    // {2552, -8, 4488, 1088}; its title band is 1,920 wide on the right-hand
    // monitor and 8 wide on the left one, so the right-hand one holds it.
    // Capped to 1,920 x 1,040 it is {2552, -8, 4472, 1032}; moved 8 right and
    // 8 down it is {2560, 0, 4480, 1040}; the client is {2568, 31, 4472, 1032}.
    const ReportPlacement big = report_placement(ScreenRect{2560, 23, 4480, 1080}, false, false,
                                                 kMainClient, kUserTwoMonitors, kFrame96);
    CHECK(big.client == ScreenRect{2568, 31, 4472, 1032});
    CHECK_FALSE(big.maximized);
}

TEST_CASE("app_shell: with no frame and no monitor list a report keeps the main window's rectangle") {
    // The GUI test runner's case for the frame: zero insets, so a maximized
    // report's client is the whole work area.
    const ReportPlacement runner = report_placement(std::nullopt, false, true,
                                                    ScreenRect{0, 0, 1280, 800},
                                                    {{0, 0, 1280, 800}}, hydra::ui::FrameInsets{});
    CHECK(runner.client == ScreenRect{0, 0, 1280, 800});
    CHECK(runner.maximized);

    // No monitors known: nothing to fit into.
    const ReportPlacement none = report_placement(std::nullopt, false, false, kMainClient, {},
                                                  kFrame96);
    CHECK(none.client == kMainClient);
    CHECK_FALSE(none.maximized);
}

TEST_CASE("app_shell: a report's placement key is its window's ### ID") {
    CHECK(hydra::ui::report_placement_key("Path report \xE2\x80\x94 Hydra###pathreport") ==
          "pathreport");
    CHECK(hydra::ui::report_placement_key("dmleaderboards: someone \xE2\x80\x94 Hydra###dmreport") ==
          "dmreport");
    CHECK(hydra::ui::report_placement_key("plain") == "plain");
}

TEST_CASE("app_shell: the [Hydra][Window:pathreport] text round-trips") {
    WindowPlacement p;
    p.valid = true;
    p.normal = {8, 31, 2552, 1384};
    p.maximized = true;
    const std::string text = hydra::ui::format_window_placement(p);
    CHECK(text == "Pos=8,31\nSize=2544,1353\nMaximized=1\n");

    WindowPlacement back;
    std::istringstream lines(text);
    for (std::string line; std::getline(lines, line);)
        hydra::ui::parse_window_placement_line(line, back);
    CHECK(back == p);
}

TEST_CASE("app_shell: hydra_ui.ini keeps each report's placement in its own section") {
    const std::filesystem::path dir = hydra::os_path(testtemp::temp_dir("app_shell_reports"));
    const std::filesystem::path ini = dir / "hydra_ui.ini";
    {
        std::ofstream f(ini);
        f << "[Hydra][Window]\nPos=100,100\nSize=1280,720\nMaximized=0\n\n"
             "[Hydra][Window:pathreport]\nPos=8,31\nSize=2544,1353\nMaximized=1\n\n";
    }

    ImGuiOnIni imgui(ini.string());
    const WindowPlacement path = hydra::ui::report_window_placement("pathreport");
    CHECK(path.valid);
    CHECK(path.normal == ScreenRect{8, 31, 2552, 1384});
    CHECK(path.maximized);
    CHECK_FALSE(hydra::ui::report_window_placement("dmreport").valid);
    // The main window's section is still its own.
    CHECK(hydra::ui::window_placement().normal == ScreenRect{100, 100, 1380, 820});

    // Remembering the same placement leaves the ini alone; a new one marks it
    // for saving.
    const ImGuiContext& g = *ImGui::GetCurrentContext();
    REQUIRE(g.SettingsDirtyTimer <= 0.0f);
    hydra::ui::remember_report_placement("pathreport", path);
    CHECK(g.SettingsDirtyTimer <= 0.0f);
    WindowPlacement dm;
    dm.valid = true;
    dm.normal = {2600, 60, 4400, 1000};
    hydra::ui::remember_report_placement("dmreport", dm);
    CHECK(g.SettingsDirtyTimer > 0.0f);
    imgui.end();  // DestroyContext writes the ini

    std::ifstream f(ini);
    std::stringstream text;
    text << f.rdbuf();
    CHECK(text.str().find("[Hydra][Window:pathreport]\nPos=8,31\nSize=2544,1353\nMaximized=1\n") !=
          std::string::npos);
    CHECK(text.str().find("[Hydra][Window:dmreport]\nPos=2600,60\nSize=1800,940\nMaximized=0\n") !=
          std::string::npos);
    f.close();
    std::filesystem::remove_all(dir);
}

TEST_CASE("app_shell: the UI scale for a monitor DPI") {
    CHECK(hydra::ui::ui_scale_for_dpi(96) == doctest::Approx(1.0f));
    CHECK(hydra::ui::ui_scale_for_dpi(120) == doctest::Approx(1.25f));
    CHECK(hydra::ui::ui_scale_for_dpi(144) == doctest::Approx(1.5f));
    CHECK(hydra::ui::ui_scale_for_dpi(192) == doctest::Approx(2.0f));
    CHECK(hydra::ui::ui_scale_for_dpi(0) == doctest::Approx(1.0f));  // no reading: unscaled
}

TEST_CASE("app_shell: scaling always starts from the unscaled style") {
    const ImGuiStyle base;
    const ImGuiStyle once = hydra::ui::scaled_style(base, 1.5f);
    const ImGuiStyle again = hydra::ui::scaled_style(base, 1.5f);
    CHECK(once.FramePadding.x == again.FramePadding.x);
    CHECK(once.ItemSpacing.y == again.ItemSpacing.y);
    CHECK(once.FontScaleDpi == doctest::Approx(1.5f));
    const ImGuiStyle doubled = hydra::ui::scaled_style(base, 2.0f);
    CHECK(doubled.FramePadding.x == 8.0f);  // pinned from one run
    const ImGuiStyle unscaled = hydra::ui::scaled_style(base, 1.0f);
    CHECK(unscaled.FramePadding.x == base.FramePadding.x);
    CHECK(unscaled.ScrollbarSize == base.ScrollbarSize);
}

TEST_CASE("app_shell: hydra_ui.ini remembers the window placement") {
    const std::filesystem::path dir = hydra::os_path(testtemp::temp_dir("app_shell"));
    const std::filesystem::path ini = dir / "hydra_ui.ini";
    {
        std::ofstream f(ini);
        f << "[Hydra][Window]\nPos=-1700,40\nSize=1400,900\nMaximized=1\n\n";
    }

    hydra::ui::setup_imgui(test_options(ini.string()));
    const WindowPlacement read = hydra::ui::window_placement();
    CHECK(read.valid);
    CHECK(read.normal == ScreenRect{-1700, 40, -300, 940});
    CHECK(read.maximized);

    WindowPlacement moved;
    moved.valid = true;
    moved.normal = {200, 150, 1480, 870};
    moved.maximized = false;
    hydra::ui::remember_window_placement(moved);
    hydra::ui::shutdown_imgui();  // DestroyContext writes the ini

    std::ifstream f(ini);
    std::stringstream text;
    text << f.rdbuf();
    CHECK(text.str().find("[Hydra][Window]\nPos=200,150\nSize=1280,720\nMaximized=0\n") !=
          std::string::npos);
    f.close();
    std::filesystem::remove_all(dir);
}

TEST_CASE("app_shell: the [Hydra][Layout] text round-trips and refuses junk") {
    CHECK(hydra::ui::format_layout({0.35f, false}) == "LibraryShare=0.3500\nLibraryHidden=0\n");
    CHECK(hydra::ui::format_layout({0.35f, true}) == "LibraryShare=0.3500\nLibraryHidden=1\n");
    hydra::ui::Layout layout;
    hydra::ui::parse_layout_line("LibraryShare=0.3500", layout);
    CHECK(layout.library_share == doctest::Approx(0.35f));
    CHECK_FALSE(layout.library_hidden);
    hydra::ui::parse_layout_line("LibraryHidden=1", layout);
    CHECK(layout.library_hidden);
    // Anything else leaves both alone.
    for (const char* line : {"LibraryShare=abc", "LibraryShare=0", "LibraryShare=1",
                             "LibraryShare=1.5", "LibraryShare=-0.2", "LibraryShare=0.3x",
                             "LibraryShare=", "Colour=blue", "LibraryHidden=2",
                             "LibraryHidden=", "LibraryHidden=yes"})
        hydra::ui::parse_layout_line(line, layout);
    CHECK(layout.library_share == doctest::Approx(0.35f));
    CHECK(layout.library_hidden);
    hydra::ui::parse_layout_line("LibraryHidden=0", layout);
    CHECK_FALSE(layout.library_hidden);
}

TEST_CASE("app_shell: share_is_valid takes only the open interval") {
    CHECK_FALSE(hydra::ui::share_is_valid(0.0f));
    CHECK_FALSE(hydra::ui::share_is_valid(1.0f));
    CHECK(hydra::ui::share_is_valid(0.5f));
    CHECK_FALSE(hydra::ui::share_is_valid(std::numeric_limits<float>::quiet_NaN()));
}

TEST_CASE("app_shell: hydra_ui.ini remembers the library split, not the child's own width") {
    const std::filesystem::path dir = hydra::os_path(testtemp::temp_dir("app_shell_split"));
    const std::filesystem::path ini = dir / "hydra_ui.ini";
    {
        // A saved library child at the largest split, with no share.
        std::ofstream f(ini);
        f << "[Window][Hydra/##library_06E76512]\nSize=2064,1313\n\n";
    }
    hydra::ui::setup_imgui(test_options(ini.string()));
    CHECK(hydra::ui::library_share() == doctest::Approx(hydra::ui::kDefaultLibraryShare));
    CHECK_FALSE(hydra::ui::library_hidden());
    hydra::ui::remember_library_share(0.25f);
    hydra::ui::remember_library_hidden(true);
    hydra::ui::shutdown_imgui();  // DestroyContext writes the ini

    hydra::ui::setup_imgui(test_options(ini.string()));
    CHECK(hydra::ui::library_share() == doctest::Approx(0.25f));
    CHECK(hydra::ui::library_hidden());
    hydra::ui::shutdown_imgui();

    std::ifstream f(ini);
    std::stringstream text;
    text << f.rdbuf();
    CHECK(text.str().find("[Hydra][Layout]\nLibraryShare=0.2500\nLibraryHidden=1\n") !=
          std::string::npos);
    f.close();
    std::filesystem::remove_all(dir);
}

TEST_CASE("app_shell: set_ui_scale rescales sizes, fonts and px() together") {
    hydra::ui::setup_imgui(test_options("-"));
    const ImGuiStyle base = ImGui::GetStyle();
    const float before = base.FramePadding.x;

    hydra::ui::set_ui_scale(2.0f);
    CHECK(ImGui::GetStyle().FramePadding.x == hydra::ui::scaled_style(base, 2.0f).FramePadding.x);
    CHECK(ImGui::GetStyle().FontScaleDpi == doctest::Approx(2.0f));
    CHECK(hydra::ui::g_ui_scale == doctest::Approx(2.0f));
    CHECK(hydra::ui::px(10.0f) == doctest::Approx(20.0f));

    hydra::ui::set_ui_scale(1.0f);
    CHECK(ImGui::GetStyle().FramePadding.x == before);
    CHECK(ImGui::GetStyle().FontScaleDpi == doctest::Approx(1.0f));
    CHECK(hydra::ui::px(10.0f) == doctest::Approx(10.0f));
    hydra::ui::shutdown_imgui();
}

TEST_CASE("app_shell: the atlas reads every UI font from a file map it does not own") {
    const hydra::test::HeadlessImGui imgui;
    const ImFontAtlas* atlas = ImGui::GetIO().Fonts;
    REQUIRE(atlas->Sources.Size >= 2);
    // Every source, the merged Japanese fallback too when this machine has one.
    for (int i = 0; i < atlas->Sources.Size; ++i) {
        const ImFontConfig& source = atlas->Sources[i];
        CAPTURE(i);
        CHECK_FALSE(source.FontDataOwnedByAtlas);
        MEMORY_BASIC_INFORMATION info{};
        REQUIRE(VirtualQuery(source.FontData, &info, sizeof(info)) == sizeof(info));
        CHECK(info.Type == MEM_MAPPED);
    }
}

TEST_CASE("app_shell: a folder without the font files loads no fonts") {
    const std::string empty = testtemp::temp_dir("app_shell_no_fonts");
    hydra::ui::ImGuiSetupOptions opts = test_options("-");
    opts.resource_dir = empty;
    hydra::ui::setup_imgui(opts);
    // ImGui adds its built-in font on the first frame.
    CHECK(ImGui::GetIO().Fonts->Fonts.Size == 0);
    CHECK(ImGui::GetIO().FontDefault == nullptr);
    CHECK(hydra::ui::g_mono_font == nullptr);
    hydra::ui::shutdown_imgui();
    std::filesystem::remove_all(hydra::os_path(empty));
}
