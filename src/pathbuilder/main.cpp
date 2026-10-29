// HydraPathBuilder.exe — the Path Builder's entry point.
//
// Stands up the Win32 window, the DirectX 11 device and Dear ImGui, then runs
// the frame loop over PbApp (pb_window.h). The window and device code follow
// Hydra's own src/ui/main.cpp in shape. The builder sets no AppUserModelID,
// so Windows gives it a taskbar button of its own (the user's answer,
// 2026-10-29, in docs/superpowers/plans/2026-10-28-path-builder.md).
//
//   HydraPathBuilder.exe [chart file]   opens the chart, if one is given

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <shellapi.h>
#include <shellscalingapi.h>  // MONITOR_DPI_TYPE only; GetDpiForMonitor is loaded at run time

#include <cstdio>
#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/user_messages.h"
#include "core/winstr.h"
#include "pathbuilder/pb_open.h"
#include "pathbuilder/pb_window.h"
#include "ui/imgui_setup.h"
#include "ui/resource.h"

// The window's class name and title (the spec's decision 5).
static constexpr const wchar_t* kPathBuilderTitleW = L"Hydra Deluxe Path Builder";
// The ImGui layout file, beside the exe and apart from Hydra's hydra_ui.ini.
static constexpr const char* kPathBuilderUiIni = "hydra_pathbuilder_ui.ini";

static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;
// Set by WM_DPICHANGED, applied by the frame loop before the next frame.
static float                    g_PendingUiScale = 0.0f;
// A file dropped on the window (WM_DROPFILES), opened by the frame loop.
static std::optional<std::string> g_DroppedFile;
// How long the loop waits between checks while the window cannot be seen.
static constexpr DWORD          kHiddenPollMs = 10;

static HRESULT CreateDeviceD3D(HWND hWnd);
static void CleanupDeviceD3D();
static void CreateRenderTarget();
static void CleanupRenderTarget();
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// A monitor's DPI, read the way Hydra's main.cpp reads it (monitor_dpi there).
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

// Why the builder can't start, in a message box: the plain sentence with the
// raw text under it.
static void show_startup_error(HWND owner, const std::exception& e)
{
    const std::wstring text = hydra::utf8_to_wide(hydra::app::plain_error_block(e));
    ::MessageBoxW(owner, text.c_str(), kPathBuilderTitleW, MB_OK | MB_ICONERROR);
}

// The raw text for a Win32, Direct3D or ImGui backend call that failed at
// startup.
static std::runtime_error startup_call_failed(const char* call, const char* code_name,
                                              unsigned long code)
{
    char text[128];
    std::snprintf(text, sizeof(text), "%s failed (%s 0x%08lX)", call, code_name, code);
    return std::runtime_error(text);
}

int main()
{
    const std::vector<std::string> args = hydra::utf8_argv();

    ImGui_ImplWin32_EnableDpiAwareness();
    const float main_scale = hydra::ui::ui_scale_for_dpi(
        monitor_dpi(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY)));

    HINSTANCE hInstance = ::GetModuleHandleW(nullptr);
    HICON hIconLarge = (HICON)::LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_APPICON),
                                           IMAGE_ICON, 32, 32, 0);
    HICON hIconSmall = (HICON)::LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_APPICON),
                                           IMAGE_ICON, 16, 16, 0);

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
                       hInstance, hIconLarge, nullptr, nullptr,
                       nullptr, kPathBuilderTitleW, hIconSmall };
    ::RegisterClassExW(&wc);

    hydra::ui::ImGuiSetupOptions imgui_options;
    imgui_options.dpi_scale = main_scale;
    imgui_options.ini_file = hydra::join_folder(hydra::app::exe_dir(), kPathBuilderUiIni);
    hydra::ui::setup_imgui(imgui_options);

    HWND hwnd = ::CreateWindowW(
        wc.lpszClassName, kPathBuilderTitleW, WS_OVERLAPPEDWINDOW, 100, 100,
        (int)(1280 * main_scale), (int)(720 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

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

    if (hIconSmall) ::SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    if (hIconLarge) ::SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge);
    ::DragAcceptFiles(hwnd, TRUE);

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    const float window_scale = hydra::ui::ui_scale_for_dpi(
        monitor_dpi(::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST)));
    if (window_scale != main_scale)
        hydra::ui::set_ui_scale(window_scale);

    win32_backend_up = ImGui_ImplWin32_Init(hwnd);
    if (!win32_backend_up)
        return fail_startup(startup_call_failed("ImGui_ImplWin32_Init", "error", ::GetLastError()));
    dx11_backend_up = ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    if (!dx11_backend_up)
        return fail_startup(startup_call_failed("ImGui_ImplDX11_Init", "error", ::GetLastError()));

    const float* clear = hydra::ui::kClearColor;
    const float clear_with_alpha[4] = { clear[0] * clear[3], clear[1] * clear[3],
                                        clear[2] * clear[3], clear[3] };

    std::unique_ptr<hydra::pathbuilder::PbApp> app;
    try {
        app = std::make_unique<hydra::pathbuilder::PbApp>(g_pd3dDevice, g_pd3dDeviceContext);
    } catch (const std::exception& e) {
        return fail_startup(e);
    }
    app->owner = hwnd;
    if (args.size() > 1) app->open(args[1]);

    bool done = false;
    while (!done)
    {
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

        if (g_DroppedFile)
        {
            app->open(*g_DroppedFile);
            g_DroppedFile.reset();
        }

        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(kHiddenPollMs);
            continue;
        }
        g_SwapChainOccluded = false;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }
        if (g_PendingUiScale > 0.0f)
        {
            hydra::ui::set_ui_scale(g_PendingUiScale);
            g_PendingUiScale = 0.0f;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        app->draw();
        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Popups that spill past the main window's edge get OS windows of
        // their own, as in the upstream example.
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        const HRESULT hr = g_pSwapChain->Present(1, 0);   // vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    app->close_preview();
    app.reset();
    tear_down();
    return 0;
}

static HRESULT CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevelArray, 2,
        D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel,
        &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)  // Fall back to the WARP software driver.
        res = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, featureLevelArray, 2,
            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel,
            &g_pd3dDeviceContext);
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

static void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

static void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

static void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward-declared in imgui_impl_win32.cpp.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
        return 0;
    case WM_DPICHANGED:
    {
        const RECT* suggested = reinterpret_cast<const RECT*>(lParam);
        ::SetWindowPos(hWnd, nullptr, suggested->left, suggested->top,
                       suggested->right - suggested->left, suggested->bottom - suggested->top,
                       SWP_NOZORDER | SWP_NOACTIVATE);
        g_PendingUiScale = hydra::ui::ui_scale_for_dpi(HIWORD(wParam));
        return 0;
    }
    case WM_DROPFILES:
        g_DroppedFile = hydra::pathbuilder::take_dropped_file(reinterpret_cast<void*>(wParam));
        return 0;
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
