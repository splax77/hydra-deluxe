// Hydra (C++ port) — application entry point.
//
// Stands up the Win32 window, DirectX 11 device, and Dear ImGui context
// (single primary window, docking off; multi-viewports on for the report
// windows, see setup_imgui) and runs the frame loop over the library view +
// details modal. The Win32/DX11 plumbing is the upstream docking-branch
// example_win32_directx11 boilerplate, unchanged except for the window
// identity, the frame contents and the skip-while-hidden check.

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <shellscalingapi.h>  // MONITOR_DPI_TYPE only; GetDpiForMonitor is loaded at run time
#include <shobjidl.h>
#include <tchar.h>

#include <cstdio>
#include <exception>
#include <memory>
#include <stdexcept>
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

// `Hydra.exe --uitest <test|all|script-file>`: run a GUI test inside the real
// window at watchable speed (docs/agents/ui-testing.md).
#ifdef HYDRA_UITEST_ATTACHED
#include "imgui_te_ui.h"
#include "uitest_harness.h"
#endif

// Direct3D state.
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;
// Set by WM_DPICHANGED, applied by the frame loop before the next frame.
static float                    g_PendingUiScale = 0.0f;
// How long the loop waits between checks while nothing it draws can be seen.
static constexpr DWORD          kHiddenPollMs = 10;

HRESULT CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
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

// A monitor's DPI, read the way the ImGui Win32 backend reads it. Windows 8.1
// and later answer through GetDpiForMonitor in shcore.dll, loaded at run time
// because Hydra does not link Shcore.lib. Older Windows has no per-monitor
// DPI, so the screen's LOGPIXELSX answers instead. 0 means no reading, which
// ui_scale_for_dpi turns into the unscaled 1.0.
static unsigned monitor_dpi(HMONITOR monitor)
{
    using GetDpiForMonitorFn = HRESULT(WINAPI*)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);
    static const GetDpiForMonitorFn get_dpi = [] {
        const HMODULE shcore = ::LoadLibraryW(L"shcore.dll");
        return shcore ? reinterpret_cast<GetDpiForMonitorFn>(
                            ::GetProcAddress(shcore, "GetDpiForMonitor"))
                      : nullptr;
    }();
    if (get_dpi)
    {
        UINT x = 0, y = 0;
        return SUCCEEDED(get_dpi(monitor, MDT_EFFECTIVE_DPI, &x, &y)) ? x : 0;
    }
    const HDC dc = ::GetDC(nullptr);
    const int dpi = dc ? ::GetDeviceCaps(dc, LOGPIXELSX) : 0;
    if (dc) ::ReleaseDC(nullptr, dc);
    return dpi > 0 ? static_cast<unsigned>(dpi) : 0;
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
// windows are owned by the main window, so Windows hides them while Hydra is
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

// The raw text for a Win32, Direct3D or ImGui backend call that failed at
// startup. It carries no error kind, so the box reads the fallback sentence.
static std::runtime_error startup_call_failed(const char* call, const char* code_name,
                                              unsigned long code)
{
    char text[128];
    std::snprintf(text, sizeof(text), "%s failed (%s 0x%08lX)", call, code_name, code);
    return std::runtime_error(text);
}

int main()
{
    // --uitest <what> [--uitest-log <file>]; everything else is ignored.
    const std::vector<std::string> args = hydra::utf8_argv();
    const int argc = static_cast<int>(args.size());
    std::string uitest_what, uitest_log;
    for (int i = 1; i + 1 < argc; ++i) {
        if (args[i] == "--uitest") uitest_what = args[++i];
        else if (args[i] == "--uitest-log") uitest_log = args[++i];
    }
#ifndef HYDRA_UITEST_ATTACHED
    (void)uitest_what; (void)uitest_log;
#endif

    // Make the process DPI aware and read the primary monitor's scale.
    ImGui_ImplWin32_EnableDpiAwareness();
    const float main_scale = hydra::ui::ui_scale_for_dpi(
        monitor_dpi(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY)));

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
    // closed. (Context, theme, DPI and fonts live in app_shell.cpp, shared
    // with the headless GUI test runner.)
    hydra::ui::ImGuiSetupOptions imgui_options;
    imgui_options.dpi_scale = main_scale;
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

    HWND hwnd = ::CreateWindowW(
        wc.lpszClassName, hydra::kWindowTitleW, WS_OVERLAPPEDWINDOW, rect.left, rect.top,
        rect.width(), rect.height(), nullptr, nullptr, wc.hInstance, nullptr);

    // Undoes what startup has stood up so far. The normal exit and a failed
    // startup both end here, so the two can't drift apart.
    bool win32_backend_up = false, dx11_backend_up = false;
    auto tear_down = [&]
    {
        if (dx11_backend_up) ImGui_ImplDX11_Shutdown();
        if (win32_backend_up) ImGui_ImplWin32_Shutdown();
        hydra::ui::shutdown_imgui();
        CleanupDeviceD3D();
        if (hwnd) ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    };
    // A startup step that fails shows why, tears down, and Hydra closes.
    auto fail_startup = [&](const std::exception& e)
    {
        show_startup_error(hwnd, e);
        tear_down();
        return 1;
    };

    if (!hwnd)
        return fail_startup(startup_call_failed("CreateWindowW", "error", ::GetLastError()));
    if (const HRESULT hr = CreateDeviceD3D(hwnd); hr != S_OK)
        return fail_startup(startup_call_failed("D3D11CreateDeviceAndSwapChain", "HRESULT",
                                                static_cast<unsigned long>(hr)));

    // Song-info icons (record/star/pencil/hash), matching hydra_app.py's
    // dpg.add_static_texture loads. Best-effort: see icons.h.
    hydra::ui::load_icons(g_pd3dDevice);

    // Window icon, matching dpg.create_viewport's small_icon/large_icon
    // (loaded from the embedded resource above).
    if (hIconSmall) ::SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    if (hIconLarge) ::SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge);

    ::ShowWindow(hwnd, use_saved && saved.maximized ? SW_SHOWMAXIMIZED : SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // setup_imgui assumed the primary monitor's scale. A window reopened on
    // another monitor may need a different one.
    const unsigned window_dpi = monitor_dpi(::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST));
    const float window_scale = hydra::ui::ui_scale_for_dpi(window_dpi);
    if (window_scale != main_scale)
        hydra::ui::set_ui_scale(window_scale);
    // Report windows open on the main window's monitor, so its DPI sets
    // their frame; WM_DPICHANGED measures it again.
    hydra::ui::set_report_frame_insets(report_frame_insets(window_dpi));

    win32_backend_up = ImGui_ImplWin32_Init(hwnd);
    if (!win32_backend_up)
        return fail_startup(startup_call_failed("ImGui_ImplWin32_Init", "error", ::GetLastError()));
    dx11_backend_up = ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    if (!dx11_backend_up)
        return fail_startup(startup_call_failed("ImGui_ImplDX11_Init", "error", ::GetLastError()));

    const ImVec4 clear_color = ImVec4(hydra::ui::kClearColor[0], hydra::ui::kClearColor[1],
                                      hydra::ui::kClearColor[2], hydra::ui::kClearColor[3]);

    // The app state. Normally built here; under --uitest the harness owns it
    // (each test starts from a fresh scratch library) and the frame loop
    // re-reads the pointer every frame.
    std::unique_ptr<hydra::ui::AppState> own_app;
    hydra::ui::FrameText* frame_text = nullptr;
#ifdef HYDRA_UITEST_ATTACHED
    std::unique_ptr<uitest::Harness> uitest;
    bool uitest_results_written = false;
    bool uitest_windows_open = true;
    if (!uitest_what.empty()) {
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
        uitest->init_attached(g_pd3dDevice, g_pd3dDeviceContext, g_pSwapChain);
        uitest::register_tests(*uitest);
        ImGuiTestEngine_Start(uitest->engine, ImGui::GetCurrentContext());
        if (!uitest->queue(uitest_what)) {
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
            return fail_startup(e);
        }
        // Hand the GUI's shared D3D11 device to AppState so the Preview tab
        // can build its renderer on it (same pattern as load_icons above).
        own_app->set_render_device(g_pd3dDevice, g_pd3dDeviceContext);
    }

    bool done = false;
    while (!done)
    {
        // Drain the Win32 message queue.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // A database that won't open or upgrade closes Hydra with the
        // startup message box (D72 item 1), outside any ImGui frame.
        if (own_app && own_app->store_open_failed())
        {
            const std::exception_ptr failure = own_app->store_open_error();
            own_app.reset();
            try {
                std::rethrow_exception(failure);
            } catch (const std::exception& e) {
                return fail_startup(e);
            }
        }

        // Skip rendering while nothing Hydra draws can be seen: the main
        // window is minimized or occluded, and no other OS window of ours
        // shows. A report window still on screen keeps the frames coming.
        const bool main_hidden = g_SwapChainOccluded &&
            g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED;
        if (main_hidden && !other_platform_window_showing())
        {
            ::Sleep(kHiddenPollMs);
            continue;
        }
        g_SwapChainOccluded = false;

        // Apply any queued resize.
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight,
                                        DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // The window moved to a monitor with another scale (WM_DPICHANGED).
        // Restyle here, between frames: ImGui's style must not change inside
        // one.
        if (g_PendingUiScale > 0.0f)
        {
            hydra::ui::set_ui_scale(g_PendingUiScale);
            g_PendingUiScale = 0.0f;
        }

        // Begin the frame.
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        hydra::ui::AppState* app = own_app.get();
#ifdef HYDRA_UITEST_ATTACHED
        if (uitest) {
            app = uitest->app.get();
            ImGuiTestEngine_ShowTestEngineWindows(uitest->engine, &uitest_windows_open);
        }
#endif
        if (app) hydra::ui::run_frame(*app, frame_text);

        // Render.
        ImGui::Render();
        const float clear_with_alpha[4] = {
            clear_color.x * clear_color.w, clear_color.y * clear_color.w,
            clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
                                                   clear_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Create, move and draw the OS windows ImGui keeps beside the main
        // one (the report windows), as the upstream example does.
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        HRESULT hr = g_pSwapChain->Present(1, 0);   // vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
        // An occluded swapchain returns at once instead of waiting for
        // vsync, and the other windows present without it, so pace the
        // frames that only a report window still shows.
        if (main_hidden)
            ::Sleep(kHiddenPollMs);

#ifdef HYDRA_UITEST_ATTACHED
        if (uitest) {
            ImGuiTestEngine_PostSwap(uitest->engine);
            // Once the queue drains, write the results and keep the window
            // open so the end state can be inspected.
            if (!uitest_results_written && ImGuiTestEngine_IsTestQueueEmpty(uitest->engine)) {
                uitest_results_written = true;
                uitest->print_results(stdout);
                std::printf("scratch files in %s\n", uitest->temp_dir.c_str());
                std::fflush(stdout);
            }
        }
#endif
    }

#ifdef HYDRA_UITEST_ATTACHED
    if (uitest) uitest->stop();  // before ImGui's context goes away
#endif
    own_app.reset();

    tear_down();
#ifdef HYDRA_UITEST_ATTACHED
    // The engine outlives the ImGui context. Attached, its shutdown touches
    // neither the device nor the window, so it can follow the teardown.
    if (uitest) {
        uitest->keep_temp = true;  // leave the scratch files for inspection
        uitest->shutdown();
    }
#endif
    return 0;
}

// S_OK, or the HRESULT the device creation failed with, for the startup box.
HRESULT CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
        &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)  // Fall back to the WARP software driver.
        res = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
            featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
            &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return res;

    // Disable DXGI's Alt+Enter, which does not play well with viewports.
    IDXGIFactory* pSwapChainFactory = nullptr;
    if (SUCCEEDED(g_pSwapChain->GetParent(IID_PPV_ARGS(&pSwapChainFactory))))
    {
        pSwapChainFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
        pSwapChainFactory->Release();
    }

    CreateRenderTarget();
    return S_OK;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward-declared in imgui_impl_win32.cpp.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        note_window_placement(hWnd);
        return 0;
    case WM_MOVE:
        note_window_placement(hWnd);
        return 0;
    case WM_DPICHANGED:
    {
        // Windows moved us to a monitor with another scale. Take the
        // rectangle it suggests (the same physical size there), and rescale
        // the UI before the next frame. HIWORD and LOWORD of wParam carry the
        // same DPI.
        const RECT* suggested = reinterpret_cast<const RECT*>(lParam);
        ::SetWindowPos(hWnd, nullptr, suggested->left, suggested->top,
                       suggested->right - suggested->left, suggested->bottom - suggested->top,
                       SWP_NOZORDER | SWP_NOACTIVATE);
        g_PendingUiScale = hydra::ui::ui_scale_for_dpi(HIWORD(wParam));
        hydra::ui::set_report_frame_insets(report_frame_insets(HIWORD(wParam)));
        return 0;
    }
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)  // Disable the ALT app menu.
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
