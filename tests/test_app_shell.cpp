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

#include <cmath>
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
//
// ImGui's shutdown writes the ini only once settings were loaded, and
// LoadIniSettingsFromDisk marks them loaded only when the file exists. The
// app's first frame marks them loaded even with no file, but no frame runs
// here, so the ini starts as a Hydra that has run before leaves it: the main
// window's [Hydra][Window] section, in format_window_placement's text.
WindowPlacement saved_by_hydra(const std::string& key, const WindowPlacement& p) {
    const std::filesystem::path dir = hydra::os_path(testtemp::temp_dir("app_shell_saved_" + key));
    const std::filesystem::path ini = dir / "hydra_ui.ini";
    {
        WindowPlacement main_window;
        main_window.valid = true;
        main_window.normal = {100, 100, 1380, 820};
        std::ofstream f(ini);
        f << "[Hydra][Window]\n" << hydra::ui::format_window_placement(main_window) << "\n";
    }
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

namespace {

// The right-hand monitor of kUserTwoMonitors, and a report saved on it.
const ScreenRect& kRightMonitor = kUserTwoMonitors.back();
const ScreenRect kSavedOnRight{2600, 60, 4400, 1000};

// True when `inner` lies wholly inside `outer`.
bool contains(const ScreenRect& outer, const ScreenRect& inner) {
    return inner.left >= outer.left && inner.top >= outer.top && inner.right <= outer.right &&
           inner.bottom <= outer.bottom;
}

}  // namespace

TEST_CASE("app_shell: a report saved maximized on a second monitor reopens maximized there") {
    // Its title bar can be reached on the right-hand monitor, not the left one.
    REQUIRE(placement_on_screen(kSavedOnRight, {kRightMonitor}));
    REQUIRE_FALSE(placement_on_screen(kSavedOnRight, {kUserTwoMonitors.front()}));
    const ScreenRect on_right = inside_frame(kRightMonitor, kFrame96);
    // The main window is a normal one on the left-hand monitor.
    const ReportPlacement p = report_placement(kSavedOnRight, true, false, kMainClient,
                                               kUserTwoMonitors, kFrame96);
    CHECK(p.client == on_right);
    CHECK(p.maximized);

    // The main window maximized on the left-hand monitor, covering its work
    // area, does not pull it over.
    const ScreenRect main_maximized_client = kUserTwoMonitors.front();
    const ReportPlacement main_max = report_placement(kSavedOnRight, true, true, main_maximized_client,
                                                      kUserTwoMonitors, kFrame96);
    CHECK(main_max.client == on_right);
    CHECK(main_max.maximized);
}

TEST_CASE("app_shell: a report saved maximized on a monitor that is gone opens on the main window's") {
    // Only the left-hand monitor is left, so the saved rectangle is off screen.
    REQUIRE_FALSE(placement_on_screen(kSavedOnRight, kUserMonitor));
    const ReportPlacement p = report_placement(kSavedOnRight, true, false, kMainClient, kUserMonitor,
                                               kFrame96);
    CHECK(p.client == inside_frame(kUserMonitor.front(), kFrame96));
    CHECK(p.maximized);
}

TEST_CASE("app_shell: a first open with the main window maximized uses the main window's monitor") {
    // The main window maximized on the right-hand monitor covers its work
    // area; there is no saved placement.
    const ScreenRect main_maximized_client = kRightMonitor;
    const ReportPlacement p = report_placement(std::nullopt, false, true, main_maximized_client,
                                               kUserTwoMonitors, kFrame96);
    CHECK(p.client == inside_frame(kRightMonitor, kFrame96));
    CHECK(p.maximized);
}

TEST_CASE("app_shell: a saved report is fitted into the monitor holding its title bar") {
    // A saved report whose outer window fits the right-hand monitor stays.
    REQUIRE(contains(kRightMonitor, outer_of(kSavedOnRight, kFrame96)));
    const ReportPlacement fits = report_placement(kSavedOnRight, false, false, kMainClient,
                                                  kUserTwoMonitors, kFrame96);
    CHECK(fits.client == kSavedOnRight);
    CHECK_FALSE(fits.maximized);

    // A saved report taller than the right-hand monitor. Its outer window
    // also reaches a few pixels onto the left-hand monitor, but nearly all
    // of its title bar is on the right-hand one.
    const ScreenRect tall{kRightMonitor.left, kRightMonitor.top + 23, kRightMonitor.right,
                          kRightMonitor.bottom + 40};
    const ScreenRect tall_outer = outer_of(tall, kFrame96);
    REQUIRE(tall_outer.left < kRightMonitor.left);
    REQUIRE(tall_outer.height() > kRightMonitor.height());
    const ReportPlacement big = report_placement(tall, false, false, kMainClient, kUserTwoMonitors,
                                                 kFrame96);
    // Its outer window, frame included, now lies inside the right-hand
    // monitor, cut to that monitor's height.
    const ScreenRect big_outer = outer_of(big.client, kFrame96);
    CHECK(contains(kRightMonitor, big_outer));
    CHECK(big_outer.height() == kRightMonitor.height());
    CHECK_FALSE(big.maximized);
}

TEST_CASE("app_shell: in the GUI test runner a report opens over the main window, no frame added") {
    // The runner has no platform backend: main.cpp never sets a frame, and
    // ImGui's stand-in monitor is the main viewport. This runs
    // place_report_window, the code a report runs there, on a headless ImGui.
    const hydra::test::HeadlessImGui imgui;
    ImGui::NewFrame();
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const char* name = "Placement test###placement_test";
    hydra::ui::place_report_window(name);
    ImGui::Begin(name);
    ImGui::End();
    ImGui::EndFrame();

    const ScreenRect main_rect{static_cast<int>(viewport->Pos.x), static_cast<int>(viewport->Pos.y),
                               static_cast<int>(viewport->Pos.x + viewport->Size.x),
                               static_cast<int>(viewport->Pos.y + viewport->Size.y)};
    REQUIRE(main_rect.width() > 0);
    REQUIRE(main_rect.height() > 0);
    const ImGuiWindow* report = ImGui::FindWindowByName(name);
    REQUIRE(report != nullptr);
    const ScreenRect report_rect{static_cast<int>(report->Pos.x), static_cast<int>(report->Pos.y),
                                 static_cast<int>(report->Pos.x + report->Size.x),
                                 static_cast<int>(report->Pos.y + report->Size.y)};
    CHECK(report_rect == main_rect);
}

TEST_CASE("app_shell: a report's placement key is its window's ### ID") {
    // Whatever the title in front, the key is the ID after "###"; a title's
    // own "##" does not end it.
    const std::string id = "anyreport";
    CHECK(hydra::ui::report_placement_key("Any report \xE2\x80\x94 Hydra###" + id) == id);
    CHECK(hydra::ui::report_placement_key("Any##report###" + id) == id);
    const std::string no_id = "plain";
    CHECK(hydra::ui::report_placement_key(no_id) == no_id);
}

namespace {

// A [Hydra][Window] section's body in hydra_ui.ini for the rectangle `r`:
// Pos is its top-left corner, Size its width and height.
std::string placement_text(const ScreenRect& r, bool maximized) {
    return "Pos=" + std::to_string(r.left) + "," + std::to_string(r.top) + "\nSize=" +
           std::to_string(r.width()) + "," + std::to_string(r.height()) + "\nMaximized=" +
           (maximized ? "1" : "0") + "\n";
}

}  // namespace

TEST_CASE("app_shell: hydra_ui.ini keeps each report's placement in its own section") {
    const std::filesystem::path dir = hydra::os_path(testtemp::temp_dir("app_shell_reports"));
    const std::filesystem::path ini = dir / "hydra_ui.ini";
    const ScreenRect main_rect{100, 100, 1380, 820};
    // The pathreport client maximized on kUserMonitor, inside the frame.
    const ScreenRect path_rect = inside_frame(kUserMonitor.front(), kFrame96);
    {
        std::ofstream f(ini);
        f << "[Hydra][Window]\n" << placement_text(main_rect, false) << "\n"
          << "[Hydra][Window:pathreport]\n" << placement_text(path_rect, true) << "\n";
    }

    ImGuiOnIni imgui(ini.string());
    const WindowPlacement path = hydra::ui::report_window_placement("pathreport");
    CHECK(path.valid);
    CHECK(path.normal == path_rect);
    CHECK(path.maximized);
    CHECK_FALSE(hydra::ui::report_window_placement("dmreport").valid);
    // The main window's section is still its own.
    CHECK(hydra::ui::window_placement().normal == main_rect);

    // Remembering the same placement leaves the ini alone; a new one marks it
    // for saving.
    const ImGuiContext& g = *ImGui::GetCurrentContext();
    REQUIRE(g.SettingsDirtyTimer <= 0.0f);
    hydra::ui::remember_report_placement("pathreport", path);
    CHECK(g.SettingsDirtyTimer <= 0.0f);
    WindowPlacement dm;
    dm.valid = true;
    dm.normal = kSavedOnRight;
    hydra::ui::remember_report_placement("dmreport", dm);
    CHECK(g.SettingsDirtyTimer > 0.0f);
    imgui.end();  // DestroyContext writes the ini

    std::ifstream f(ini);
    std::stringstream text;
    text << f.rdbuf();
    CHECK(text.str().find("[Hydra][Window:pathreport]\n" + placement_text(path_rect, true)) !=
          std::string::npos);
    CHECK(text.str().find("[Hydra][Window:dmreport]\n" + placement_text(dm.normal, false)) !=
          std::string::npos);
    f.close();
    std::filesystem::remove_all(dir);
}

TEST_CASE("app_shell: the UI scale for a monitor DPI") {
    // The scale is the monitor's DPI over Windows' default DPI: the default
    // is 1.0, and each Windows scale setting is that fraction of it.
    constexpr unsigned kDefault = USER_DEFAULT_SCREEN_DPI;
    CHECK(hydra::ui::ui_scale_for_dpi(kDefault) == doctest::Approx(1.0f));
    CHECK(hydra::ui::ui_scale_for_dpi(kDefault * 5 / 4) == doctest::Approx(1.25f));
    CHECK(hydra::ui::ui_scale_for_dpi(kDefault * 3 / 2) == doctest::Approx(1.5f));
    CHECK(hydra::ui::ui_scale_for_dpi(kDefault * 2) == doctest::Approx(2.0f));
    CHECK(hydra::ui::ui_scale_for_dpi(0) == doctest::Approx(1.0f));  // no reading: unscaled
}

namespace {

// `size` at `scale` the way ImGuiStyle::ScaleAllSizes sizes it: multiplied,
// then cut to a whole pixel.
float scaled_size(float size, float scale) { return std::trunc(size * scale); }

}  // namespace

TEST_CASE("app_shell: scaling always starts from the unscaled style") {
    ImGuiOnIni imgui("-");
    const ImGuiStyle base = ImGui::GetStyle();
    REQUIRE(base.FramePadding.x > 0.0f);
    REQUIRE(base.ItemSpacing.y > 0.0f);

    // Scale 2 set twice is still twice the base, not four times.
    hydra::ui::set_ui_scale(2.0f);
    hydra::ui::set_ui_scale(2.0f);
    CHECK(ImGui::GetStyle().FramePadding.x == scaled_size(base.FramePadding.x, 2.0f));
    CHECK(ImGui::GetStyle().ItemSpacing.y == scaled_size(base.ItemSpacing.y, 2.0f));
    CHECK(ImGui::GetStyle().FontScaleDpi == doctest::Approx(2.0f));

    // Scale 1.5 after it is 1.5 times the base, not 1.5 times the doubled style.
    hydra::ui::set_ui_scale(1.5f);
    CHECK(ImGui::GetStyle().FramePadding.x == scaled_size(base.FramePadding.x, 1.5f));
    CHECK(ImGui::GetStyle().ItemSpacing.y == scaled_size(base.ItemSpacing.y, 1.5f));
    CHECK(ImGui::GetStyle().FontScaleDpi == doctest::Approx(1.5f));
}

TEST_CASE("app_shell: hydra_ui.ini remembers the window placement") {
    const std::filesystem::path dir = hydra::os_path(testtemp::temp_dir("app_shell"));
    const std::filesystem::path ini = dir / "hydra_ui.ini";
    // A window on the left-hand monitor of kTwoMonitors, saved maximized.
    const ScreenRect saved_rect{-1700, 40, -300, 940};
    {
        std::ofstream f(ini);
        f << "[Hydra][Window]\n" << placement_text(saved_rect, true) << "\n";
    }

    ImGuiOnIni imgui(ini.string());
    const WindowPlacement read = hydra::ui::window_placement();
    CHECK(read.valid);
    CHECK(read.normal == saved_rect);
    CHECK(read.maximized);

    WindowPlacement moved;
    moved.valid = true;
    moved.normal = {200, 150, 1480, 870};
    moved.maximized = false;
    hydra::ui::remember_window_placement(moved);
    imgui.end();  // DestroyContext writes the ini

    std::ifstream f(ini);
    std::stringstream text;
    text << f.rdbuf();
    CHECK(text.str().find("[Hydra][Window]\n" + placement_text(moved.normal, false)) !=
          std::string::npos);
    f.close();
    std::filesystem::remove_all(dir);
}

namespace {

// The lines of `text`, without their line ends.
std::vector<std::string> lines_of(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream in(text);
    for (std::string line; std::getline(in, line);) lines.push_back(line);
    return lines;
}

// A "key=value" line's "key=" part.
std::string key_of(const std::string& line) {
    const std::size_t eq = line.find('=');
    REQUIRE(eq != std::string::npos);
    return line.substr(0, eq + 1);
}

}  // namespace

TEST_CASE("app_shell: the [Hydra][Layout] text round-trips and refuses junk") {
    const hydra::ui::Layout shown{0.35f, false};
    const hydra::ui::Layout hidden{0.35f, true};
    const std::vector<std::string> shown_lines = lines_of(hydra::ui::format_layout(shown));
    const std::vector<std::string> hidden_lines = lines_of(hydra::ui::format_layout(hidden));
    REQUIRE(shown_lines.size() == 2);
    REQUIRE(hidden_lines.size() == 2);
    // format_layout writes the share line first, then the hidden line.
    const std::string share_key = key_of(hidden_lines[0]);
    const std::string hidden_key = key_of(hidden_lines[1]);
    CHECK(key_of(shown_lines[0]) == share_key);
    CHECK(key_of(shown_lines[1]) == hidden_key);

    // Each line format_layout writes reads back into a default layout.
    hydra::ui::Layout layout;
    REQUIRE(layout.library_share != doctest::Approx(hidden.library_share));
    REQUIRE(layout.library_hidden != hidden.library_hidden);
    for (const std::string& line : hidden_lines) hydra::ui::parse_layout_line(line, layout);
    CHECK(layout.library_share == doctest::Approx(hidden.library_share));
    CHECK(layout.library_hidden == hidden.library_hidden);

    // Anything else leaves both alone.
    const std::vector<std::string> junk = {
        share_key + "abc", share_key + "0",   share_key + "1",   share_key + "1.5",
        share_key + "-0.2", share_key + "0.3x", share_key,        "Colour=blue",
        hidden_key + "2",  hidden_key,        hidden_key + "yes"};
    for (const std::string& line : junk) {
        CAPTURE(line);
        hydra::ui::parse_layout_line(line, layout);
        CHECK(layout.library_share == doctest::Approx(hidden.library_share));
        CHECK(layout.library_hidden == hidden.library_hidden);
    }

    // The shown layout's hidden line turns hiding back off.
    hydra::ui::parse_layout_line(shown_lines[1], layout);
    CHECK(layout.library_hidden == shown.library_hidden);
}

TEST_CASE("app_shell: share_is_valid takes only the open interval") {
    CHECK_FALSE(hydra::ui::share_is_valid(0.0f));
    CHECK_FALSE(hydra::ui::share_is_valid(1.0f));
    CHECK(hydra::ui::share_is_valid(0.5f));
    CHECK_FALSE(hydra::ui::share_is_valid(std::numeric_limits<float>::quiet_NaN()));
}

namespace {

// The name ImGui gives the library child, and so its [Window] section in
// hydra_ui.ini: library_view.cpp begins it as "##library" inside the main
// window, which it begins as "Hydra". ImGui adds the child's ID hash to it.
std::string library_child_window_name() {
    const hydra::test::HeadlessImGui imgui;
    ImGui::NewFrame();
    ImGui::Begin("Hydra");
    ImGui::BeginChild("##library");
    const std::string name = ImGui::GetCurrentWindow()->Name;
    ImGui::EndChild();
    ImGui::End();
    ImGui::EndFrame();
    return name;
}

}  // namespace

TEST_CASE("app_shell: hydra_ui.ini remembers the library split, not the child's own width") {
    const std::string child = library_child_window_name();
    const std::filesystem::path dir = hydra::os_path(testtemp::temp_dir("app_shell_split"));
    const std::filesystem::path ini = dir / "hydra_ui.ini";
    {
        // A saved library child at the largest split, with no share.
        std::ofstream f(ini);
        f << "[Window][" << child << "]\nSize=2064,1313\n\n";
    }
    {
        ImGuiOnIni imgui(ini.string());
        CHECK(hydra::ui::library_share() == doctest::Approx(hydra::ui::kDefaultLibraryShare));
        CHECK_FALSE(hydra::ui::library_hidden());
        hydra::ui::remember_library_share(0.25f);
        hydra::ui::remember_library_hidden(true);
    }  // DestroyContext writes the ini

    {
        ImGuiOnIni imgui(ini.string());
        CHECK(hydra::ui::library_share() == doctest::Approx(0.25f));
        CHECK(hydra::ui::library_hidden());
    }

    std::ifstream f(ini);
    std::stringstream text;
    text << f.rdbuf();
    CHECK(text.str().find("[Hydra][Layout]\n" + hydra::ui::format_layout({0.25f, true})) !=
          std::string::npos);
    f.close();
    std::filesystem::remove_all(dir);
}

TEST_CASE("app_shell: set_ui_scale rescales sizes, fonts and px() together") {
    hydra::ui::setup_imgui(test_options("-"));
    const ImGuiStyle base = ImGui::GetStyle();
    const float before = base.FramePadding.x;

    hydra::ui::set_ui_scale(2.0f);
    CHECK(ImGui::GetStyle().FramePadding.x == scaled_size(before, 2.0f));
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
    // Both fonts Hydra draws with, the UI font and the mono one, loaded, so
    // the loop below sees them.
    const ImFont* ui_font = ImGui::GetIO().FontDefault;
    const ImFont* mono_font = hydra::ui::g_mono_font;
    REQUIRE(ui_font != nullptr);
    REQUIRE(mono_font != nullptr);
    REQUIRE(ui_font != mono_font);
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
