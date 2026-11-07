// Hydra (C++ port) — application entry point.
//
// Stands up the Win32 window, DirectX 11 device, and Dear ImGui context
// (single primary window, docking off; multi-viewports on for the report
// windows, see setup_imgui) and runs the frame loop over the library view +
// details modal. The Win32/DX11 pieces it shares with the Path Builder live
// in ui/win32_window.h; this file adds the window identity, the frame
// contents and the skip-while-hidden check.

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <shobjidl.h>
#include <tchar.h>

#include <cstdio>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/user_messages.h"
#include "core/version.h"
#include "core/winstr.h"
#include "ui/resource.h"
#include "ui/app_shell.h"
#include "ui/app_state.h"
#include "ui/icons.h"
#include "ui/win32_window.h"

// `Hydra.exe --uitest <test|all|script-file>`: run a GUI test inside the real
// window at watchable speed (docs/agents/ui-testing.md). `Hydra.exe --live`:
// take commands from the live channel in the real window, as the MCP helper
// tools/hydra_ui_mcp.py launches it. Dev builds only.
#ifdef HYDRA_UITEST_ATTACHED
#include "imgui_te_ui.h"
#include "uitest_harness.h"
#endif

// The main window's device and the changes its window procedure leaves.
static hydra::ui::WindowDevice  g_window;
// How long the loop waits between checks while nothing it draws can be seen.
static constexpr DWORD          kHiddenPollMs = 10;
// The app the frame loop drew last, so WM_ENDSESSION can save its settings
// before Windows ends the process.
static hydra::ui::AppState*     g_FrameApp = nullptr;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Adds one monitor's work area (the screen minus the taskbar) to the list.
static BOOL CALLBACK add_work_area(HMONITOR monitor, HDC, LPRECT, LPARAM data)
{
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if (::GetMonitorInfoW(monitor, &info))
        reinterpret_cast<std::vector<hydra::ui::ScreenRect>*>(data)->push_back(
            { info.rcWork.left, info.rcWork.top, info.rcWork.right, info.rcWork.bottom });
    return TRUE;
}

// Every monitor's work area, in virtual-screen pixels.
static std::vector<hydra::ui::ScreenRect> monitor_work_areas()
{
    std::vector<hydra::ui::ScreenRect> areas;
    ::EnumDisplayMonitors(nullptr, nullptr, add_work_area, reinterpret_cast<LPARAM>(&areas));
    return areas;
}

// The frame Windows puts around a report window's client area on a monitor
// at `dpi`, for report_placement. The style is the one the Win32 backend gives
// a viewport with a title bar and a taskbar button
// (ImGui_ImplWin32_GetWin32StyleFromViewportFlags in imgui_impl_win32.cpp;
// report_window_class asks for both), and the measuring call is the one its
// ImGui_ImplWin32_AdjustWindowRect makes. AdjustWindowRectExForDpi is loaded
// at run time, as the backend loads it, because Windows before 10 (1607) has
// none; there the plain call answers at the system DPI.
static hydra::ui::FrameInsets report_frame_insets(unsigned dpi)
{
    using AdjustForDpiFn = BOOL(WINAPI*)(LPRECT, DWORD, BOOL, DWORD, UINT);
    static const AdjustForDpiFn adjust_for_dpi = [] {
        const HMODULE user32 = ::GetModuleHandleW(L"user32.dll");
        return user32 ? reinterpret_cast<AdjustForDpiFn>(
                            ::GetProcAddress(user32, "AdjustWindowRectExForDpi"))
                      : nullptr;
    }();
    constexpr DWORD style = WS_OVERLAPPEDWINDOW, ex_style = WS_EX_APPWINDOW;
    RECT r{ 0, 0, 0, 0 };
    if (!(adjust_for_dpi && dpi != 0 && adjust_for_dpi(&r, style, FALSE, ex_style, dpi)))
    {
        r = RECT{ 0, 0, 0, 0 };
        ::AdjustWindowRectEx(&r, style, FALSE, ex_style);
    }
    return { -r.left, -r.top, r.right, r.bottom };
}

// Keeps the remembered placement current as the user moves, resizes,
// maximizes and restores the window (observed_placement says what it keeps).
static void note_window_placement(HWND hWnd)
{
    hydra::ui::remember_window_placement(hydra::ui::observed_placement(
        hWnd, hydra::ui::window_placement(), hydra::ui::WindowRect::Outer));
}

// True when an OS window ImGui made beside the main one (a report window, or
// a popup that spilled past the main window's edge) is on screen. Report
// windows have no OS owner, so one can stay on screen while Hydra is
// minimized.
static bool other_platform_window_showing()
{
    const ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    for (const ImGuiViewport* viewport : pio.Viewports)
    {
        if (viewport == main_viewport) continue;
        const HWND w = static_cast<HWND>(viewport->PlatformHandleRaw);
        if (w && ::IsWindowVisible(w) && !::IsIconic(w)) return true;
    }
    return false;
}

// Why Hydra can't start, in a Windows message box over `owner` (null until
// the window exists): the plain sentence with the raw text under it (D72
// item 1).
static void show_startup_error(HWND owner, const std::exception& e)
{
    const std::wstring text = hydra::utf8_to_wide(hydra::app::plain_error_block(e));
    ::MessageBoxW(owner, text.c_str(), hydra::kWindowTitleW, MB_OK | MB_ICONERROR);
}

// A sentence the startup open left to say once (AppState::startup_notice), in
// an information box over the window. Hydra carries on once it closes.
static void show_startup_notice(HWND owner, const std::string& sentence)
{
    const std::wstring text = hydra::utf8_to_wide(sentence);
    ::MessageBoxW(owner, text.c_str(), hydra::kWindowTitleW, MB_OK | MB_ICONINFORMATION);
}

int main()
{
#ifdef HYDRA_UITEST_ATTACHED
    // The GUI test flags, read only by a dev build; a shipped Hydra takes no
    // arguments. --uitest <what> runs a test or script; --live runs the live
    // channel instead (tests/ui/uitest_live.cpp). --db, --ini, --shots and
    // --pipe mean what they mean to hydra_uitest (tests/ui/uitest_main.cpp).
    // --ui-ini names the ImGui layout file. --open-report, on a launch without
    // the harness, opens the path report once the library is up
    // (AppState::open_path_report_once_shown); the harness ignores it.
    // Everything else is ignored.
    const std::vector<std::string> args = hydra::utf8_argv();
    const int argc = static_cast<int>(args.size());
    std::string uitest_what, uitest_log, seed_db, seed_ini, ui_ini, shots_dir, pipe_name;
    bool live = false, keep_temp = false, open_report = false;
    for (int i = 1; i < argc; ++i) {
        const std::string& a = args[i];
        if (a == "--live") { live = true; continue; }
        if (a == "--keep-temp") { keep_temp = true; continue; }
        if (a == "--open-report") { open_report = true; continue; }
        if (i + 1 >= argc) break;
        if (a == "--uitest") uitest_what = args[++i];
        else if (a == "--uitest-log") uitest_log = args[++i];
        else if (a == "--db") seed_db = args[++i];
        else if (a == "--ini") seed_ini = args[++i];
        else if (a == "--ui-ini") ui_ini = args[++i];
        else if (a == "--shots") shots_dir = args[++i];
        else if (a == "--pipe") pipe_name = args[++i];
    }
    const bool run_harness = live || !uitest_what.empty();
#endif

    // Make the process DPI aware and read the primary monitor's scale.
    ImGui_ImplWin32_EnableDpiAwareness();
    const float main_scale = hydra::ui::ui_scale_for_dpi(
        hydra::ui::monitor_dpi(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY)));

    // An explicit taskbar identity, so pins survive a reinstall to a new path.
    ::SetCurrentProcessExplicitAppUserModelID(hydra::kAppUserModelIDW);

    // App icon, embedded in the exe by src/app/hydra.rc. The pinned-taskbar /
    // Explorer icon comes straight from that PE resource; these runtime loads
    // cover the window class and the live window (WM_SETICON below), and
    // unlike the old cwd-relative resource/icon_app.ico file load they work
    // from any working directory.
    HINSTANCE hInstance = ::GetModuleHandleW(nullptr);
    HICON hIconLarge = (HICON)::LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_APPICON),
                                           IMAGE_ICON, 32, 32, 0);
    HICON hIconSmall = (HICON)::LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_APPICON),
                                           IMAGE_ICON, 16, 16, 0);

    // Create the application window.
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
                       hInstance, hIconLarge, nullptr, nullptr,
                       nullptr, L"Hydra", hIconSmall };
    ::RegisterClassExW(&wc);

    // Set up Dear ImGui before the window exists: setup_imgui reads
    // hydra_ui.ini, which remembers where the window was when Hydra last
    // closed. (Context, theme, DPI and fonts live in imgui_setup.cpp, shared
    // with the headless GUI test runner.)
    hydra::ui::ImGuiSetupOptions imgui_options;
    imgui_options.dpi_scale = main_scale;
    imgui_options.before_ini_load = hydra::ui::install_placement_handler;
#ifdef HYDRA_UITEST_ATTACHED
    imgui_options.ini_file = ui_ini;  // empty keeps the default
#endif
    hydra::ui::setup_imgui(imgui_options);

    // Reopen where the user left it, unless that spot is on no monitor now
    // (a monitor was unplugged or rearranged). Otherwise the old default.
    const hydra::ui::WindowPlacement saved = hydra::ui::window_placement();
    const bool use_saved =
        saved.valid && hydra::ui::placement_on_screen(saved.normal, monitor_work_areas());
    const hydra::ui::ScreenRect rect =
        use_saved ? saved.normal
                  : hydra::ui::ScreenRect{ 100, 100, 100 + (int)(1280 * main_scale),
                                           100 + (int)(720 * main_scale) };

    // What startup has stood up so far; a failed step shows why, tears down,
    // and Hydra closes.
    hydra::ui::WindowStartup startup{ &g_window, &wc, show_startup_error };
    startup.hwnd = ::CreateWindowW(
        wc.lpszClassName, hydra::kWindowTitleW, WS_OVERLAPPEDWINDOW, rect.left, rect.top,
        rect.width(), rect.height(), nullptr, nullptr, wc.hInstance, nullptr);
    const HWND hwnd = startup.hwnd;
    if (!startup.create_device())
        return 1;

    // Song-info icons (record/star/pencil/hash), matching hydra_app.py's
    // dpg.add_static_texture loads. Best-effort: see icons.h.
    hydra::ui::load_icons(g_window.device);

    // Window icon, matching dpg.create_viewport's small_icon/large_icon
    // (loaded from the embedded resource above).
    if (hIconSmall) ::SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    if (hIconLarge) ::SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge);

    ::ShowWindow(hwnd, use_saved && saved.maximized ? SW_SHOWMAXIMIZED : SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // setup_imgui assumed the primary monitor's scale. A window reopened on
    // another monitor may need a different one.
    const unsigned window_dpi =
        hydra::ui::monitor_dpi(::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST));
    const float window_scale = hydra::ui::ui_scale_for_dpi(window_dpi);
    if (window_scale != main_scale)
        hydra::ui::set_ui_scale(window_scale);
    // Report windows open on the main window's monitor, so its DPI sets
    // their frame; WM_DPICHANGED measures it again.
    hydra::ui::set_report_frame_insets(report_frame_insets(window_dpi));

    if (!startup.start_backends())
        return 1;

    // The app state. Normally built here; under --uitest the harness owns it
    // (each test starts from a fresh scratch library) and the frame loop
    // re-reads the pointer every frame.
    std::unique_ptr<hydra::ui::AppState> own_app;
    hydra::ui::FrameText* frame_text = nullptr;
#ifdef HYDRA_UITEST_ATTACHED
    std::unique_ptr<uitest::Harness> uitest;
    bool uitest_results_written = false;
    bool uitest_windows_open = true;
    int uitest_failures = 0;
    if (run_harness) {
        // All runner output (results, dump/state text) goes to the log file:
        // a GUI-subsystem exe has no console. Opened shareable so it can be
        // read (tail -f) while the window is still up.
        if (uitest_log.empty()) uitest_log = hydra::join_folder(hydra::app::exe_dir(), "hydra_uitest.log");
        // (_wfreopen, not _wfreopen_s: the _s form opens without sharing; and
        // a GUI exe has no stdout fd to _dup2 onto. Wide, because the path is
        // UTF-8.)
#pragma warning(suppress : 4996)
        if (_wfreopen(hydra::win32_path(uitest_log).c_str(), L"w", stdout))
            setvbuf(stdout, nullptr, _IONBF, 0);
        uitest = std::make_unique<uitest::Harness>();
        // Before init_attached, which points the shots at the scratch folder
        // when none is given.
        uitest->shots_dir = shots_dir;
        uitest->seed_db = seed_db;
        uitest->seed_ini = seed_ini;
        uitest->pipe_name = pipe_name;
        uitest->log_path = uitest_log;
        uitest->init_attached(g_window.device, g_window.context, g_window.swap_chain);
        uitest::register_tests(*uitest);
        if (live) uitest::register_live_test(*uitest);
        ImGuiTestEngine_Start(uitest->engine, ImGui::GetCurrentContext());
        // A mistyped seed is refused here as hydra_uitest's main refuses it
        // (uitest::seed_problem).
        std::string refused;
        if (!seed_db.empty()) refused = uitest::seed_problem("--db", seed_db);
        if (refused.empty() && !seed_ini.empty()) refused = uitest::seed_problem("--ini", seed_ini);
        if (live && !uitest_what.empty()) refused = "--live runs alone, without --uitest";
        if (refused.empty()) refused = uitest::pipe_problem(live, pipe_name);
        if (!refused.empty()) {
            std::printf("%s\n", refused.c_str());
            uitest_failures = 1;
            uitest_results_written = true;
            if (live) ::PostQuitMessage(1);
        } else if (live) {
            ImGuiTestEngine_QueueTest(uitest->engine,
                                      ImGuiTestEngine_FindTestByName(uitest->engine, "hydra", "live"),
                                      ImGuiTestRunFlags_RunFromCommandLine);
        } else if (!uitest->queue(uitest_what)) {
            std::printf("no test or script file \"%s\"\n", uitest_what.c_str());
            uitest_results_written = true;
        }
        frame_text = &uitest->frame_text;
    }
#endif
    if (!frame_text) {
        // Starts opening hydra.db on its own thread; the loop below shows
        // why when it fails. Reading the settings can still throw here.
        try {
            own_app = std::make_unique<hydra::ui::AppState>();
        } catch (const std::exception& e) {
            return startup.fail(e);
        }
        // Hand the GUI's shared D3D11 device to AppState so the Preview tab
        // can build its renderer on it (same pattern as load_icons above).
        own_app->set_render_device(g_window.device, g_window.context);
#ifdef HYDRA_UITEST_ATTACHED
        if (open_report) own_app->open_path_report_once_shown();
#endif
    }

    // Drain the Win32 message queue before each frame.
    while (hydra::ui::pump_messages())
    {
        // A database that won't open or upgrade closes Hydra with the
        // startup message box (D72 item 1), outside any ImGui frame.
        if (own_app && own_app->store_open_failed())
        {
            const std::exception_ptr failure = own_app->store_open_error();
            own_app.reset();
            try {
                std::rethrow_exception(failure);
            } catch (const std::exception& e) {
                return startup.fail(e);
            }
        }
        // A library file set aside at startup is said once, outside any
        // ImGui frame; closing the box starts the scan.
        if (own_app && !own_app->startup_notice().empty())
        {
            show_startup_notice(hwnd, own_app->startup_notice());
            own_app->startup_notice_closed();
        }

        // Skip rendering while nothing Hydra draws can be seen: the main
        // window is minimized or occluded, and no other OS window of ours
        // shows. A report window still on screen keeps the frames coming.
        const bool main_hidden = hydra::ui::main_window_hidden(g_window);
        if (main_hidden && !other_platform_window_showing())
        {
            ::Sleep(kHiddenPollMs);
            continue;
        }
        g_window.occluded = false;

        // Apply a queued resize, and a new scale after the window moved to
        // another monitor (WM_DPICHANGED).
        hydra::ui::apply_window_changes(g_window);

        // Begin the frame.
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        hydra::ui::AppState* app = own_app.get();
#ifdef HYDRA_UITEST_ATTACHED
        if (uitest) {
            app = uitest->app.get();
            // The engine's panel helps a --uitest run being watched; in live
            // mode it only covers the app Claude is driving.
            if (!live) ImGuiTestEngine_ShowTestEngineWindows(uitest->engine, &uitest_windows_open);
        }
#endif
        g_FrameApp = app;
        if (app) hydra::ui::run_frame(*app, frame_text);

        // Render.
        ImGui::Render();
        hydra::ui::render_frame(g_window);

        // Create, move and draw the OS windows ImGui keeps beside the main
        // one (the report windows), as the upstream example does.
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            hydra::ui::maximize_due_report_windows();
        }

        hydra::ui::present_frame(g_window);
        // An occluded swapchain returns at once instead of waiting for
        // vsync, and the other windows present without it, so pace the
        // frames that only a report window still shows.
        if (main_hidden)
            ::Sleep(kHiddenPollMs);

#ifdef HYDRA_UITEST_ATTACHED
        if (uitest) {
            ImGuiTestEngine_PostSwap(uitest->engine);
            // Once the queue drains, write the results. A --uitest run keeps
            // the window open so the end state can be inspected; the live
            // test returns when its client quits or leaves, and nobody is
            // left to look, so the window closes.
            if (!uitest_results_written && ImGuiTestEngine_IsTestQueueEmpty(uitest->engine)) {
                uitest_results_written = true;
                uitest_failures = uitest->print_results(stdout);
                if (!live) std::printf("scratch files in %s\n", uitest->temp_dir.c_str());
                std::fflush(stdout);
                if (live) ::PostQuitMessage(0);
            }
        }
#endif
    }

#ifdef HYDRA_UITEST_ATTACHED
    if (uitest) uitest->stop();  // before ImGui's context goes away
#endif
    g_FrameApp = nullptr;
    own_app.reset();

    startup.tear_down();
#ifdef HYDRA_UITEST_ATTACHED
    // The engine outlives the ImGui context. Attached, its shutdown touches
    // neither the device nor the window, so it can follow the teardown.
    if (uitest) {
        // A --uitest run leaves its scratch files for inspection. A live run
        // deletes them unless --keep-temp asks otherwise: in window mode they
        // hold a copy of the user's library.
        uitest->keep_temp = !live || keep_temp;
        uitest->shutdown();
        if (live) {
            if (uitest->keep_temp) std::printf("scratch files kept in %s\n", uitest->temp_dir.c_str());
            std::fflush(stdout);
            return uitest_failures == 0 ? 0 : 1;
        }
    }
#endif
    return 0;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (hydra::ui::imgui_takes_message(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (hydra::ui::note_resize(g_window, wParam, lParam))
            note_window_placement(hWnd);
        return 0;
    case WM_MOVE:
        note_window_placement(hWnd);
        return 0;
    case WM_DPICHANGED:
    {
        // The report windows' frame follows the new DPI too.
        const unsigned dpi = hydra::ui::follow_dpi_change(g_window, hWnd, wParam, lParam);
        hydra::ui::set_report_frame_insets(report_frame_insets(dpi));
        return 0;
    }
    // Windows is ending the session, or the installer asks Hydra to close
    // through Restart Manager (ENDSESSION_CLOSEAPP) so it can update it.
    // Hydra always agrees, then saves and closes the way its X does.
    case WM_QUERYENDSESSION:
        return TRUE;
    case WM_ENDSESSION:
        if (wParam)
        {
            // At a log-off Windows may end the process as soon as this
            // returns, so the INI is written first; the normal exit then
            // writes the rest.
            if (g_FrameApp) g_FrameApp->flush_settings();
            ::DestroyWindow(hWnd);
        }
        return 0;
    }
    return hydra::ui::default_window_message(hWnd, msg, wParam, lParam);
}
