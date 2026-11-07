// The main window's Win32 and Direct3D 11 pieces (win32_window.h).

#include "ui/win32_window.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <shellscalingapi.h>  // MONITOR_DPI_TYPE only; GetDpiForMonitor is loaded at run time

#include <cstdio>

#include "ui/imgui_setup.h"

// Forward-declared in imgui_impl_win32.cpp.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace hydra::ui {

unsigned monitor_dpi(HMONITOR monitor)
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

namespace {

void create_render_target(WindowDevice& w)
{
    ID3D11Texture2D* back_buffer = nullptr;
    w.swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    w.device->CreateRenderTargetView(back_buffer, nullptr, &w.render_target);
    back_buffer->Release();
}

void release_render_target(WindowDevice& w)
{
    if (w.render_target) { w.render_target->Release(); w.render_target = nullptr; }
}

}  // namespace

HRESULT create_window_device(WindowDevice& w, HWND hwnd)
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
    sd.OutputWindow = hwnd;
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
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &w.swap_chain,
        &w.device, &featureLevel, &w.context);
    if (res == DXGI_ERROR_UNSUPPORTED)  // Fall back to the WARP software driver.
        res = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
            featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &w.swap_chain,
            &w.device, &featureLevel, &w.context);
    if (res != S_OK)
        return res;

    // Disable DXGI's Alt+Enter, which does not play well with viewports.
    IDXGIFactory* pSwapChainFactory = nullptr;
    if (SUCCEEDED(w.swap_chain->GetParent(IID_PPV_ARGS(&pSwapChainFactory))))
    {
        pSwapChainFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        pSwapChainFactory->Release();
    }

    create_render_target(w);
    return S_OK;
}

void release_window_device(WindowDevice& w)
{
    release_render_target(w);
    if (w.swap_chain) { w.swap_chain->Release(); w.swap_chain = nullptr; }
    if (w.context) { w.context->Release(); w.context = nullptr; }
    if (w.device) { w.device->Release(); w.device = nullptr; }
}

std::runtime_error startup_call_failed(const char* call, const char* code_name,
                                       unsigned long code)
{
    char text[128];
    std::snprintf(text, sizeof(text), "%s failed (%s 0x%08lX)", call, code_name, code);
    return std::runtime_error(text);
}

void WindowStartup::tear_down()
{
    if (dx11_backend_up) ImGui_ImplDX11_Shutdown();
    if (win32_backend_up) ImGui_ImplWin32_Shutdown();
    shutdown_imgui();
    release_window_device(*device);
    if (hwnd) ::DestroyWindow(hwnd);
    ::UnregisterClassW(window_class->lpszClassName, window_class->hInstance);
}

int WindowStartup::fail(const std::exception& e)
{
    show_error(hwnd, e);
    tear_down();
    return 1;
}

bool WindowStartup::create_device()
{
    if (!hwnd)
    {
        fail(startup_call_failed("CreateWindowW", "error", ::GetLastError()));
        return false;
    }
    if (const HRESULT hr = create_window_device(*device, hwnd); hr != S_OK)
    {
        fail(startup_call_failed("D3D11CreateDeviceAndSwapChain", "HRESULT",
                                 static_cast<unsigned long>(hr)));
        return false;
    }
    return true;
}

bool WindowStartup::start_backends()
{
    win32_backend_up = ImGui_ImplWin32_Init(hwnd);
    if (!win32_backend_up)
    {
        fail(startup_call_failed("ImGui_ImplWin32_Init", "error", ::GetLastError()));
        return false;
    }
    dx11_backend_up = ImGui_ImplDX11_Init(device->device, device->context);
    if (!dx11_backend_up)
    {
        fail(startup_call_failed("ImGui_ImplDX11_Init", "error", ::GetLastError()));
        return false;
    }
    return true;
}

bool pump_messages()
{
    bool quit = false;
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
            quit = true;
    }
    return !quit;
}

bool main_window_hidden(WindowDevice& w)
{
    return w.occluded && w.swap_chain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED;
}

void apply_window_changes(WindowDevice& w)
{
    if (w.resize_width != 0 && w.resize_height != 0)
    {
        release_render_target(w);
        w.swap_chain->ResizeBuffers(0, w.resize_width, w.resize_height, DXGI_FORMAT_UNKNOWN, 0);
        w.resize_width = w.resize_height = 0;
        create_render_target(w);
    }
    if (w.pending_ui_scale > 0.0f)
    {
        set_ui_scale(w.pending_ui_scale);
        w.pending_ui_scale = 0.0f;
    }
}

void render_frame(WindowDevice& w)
{
    const float* clear = kClearColor;
    const float clear_with_alpha[4] = { clear[0] * clear[3], clear[1] * clear[3],
                                        clear[2] * clear[3], clear[3] };
    w.context->OMSetRenderTargets(1, &w.render_target, nullptr);
    w.context->ClearRenderTargetView(w.render_target, clear_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void present_frame(WindowDevice& w)
{
    const HRESULT hr = w.swap_chain->Present(1, 0);   // vsync
    w.occluded = (hr == DXGI_STATUS_OCCLUDED);
}

bool imgui_takes_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam) != 0;
}

bool note_resize(WindowDevice& w, WPARAM wparam, LPARAM lparam)
{
    if (wparam == SIZE_MINIMIZED)
        return false;
    w.resize_width = (UINT)LOWORD(lparam);
    w.resize_height = (UINT)HIWORD(lparam);
    return true;
}

unsigned follow_dpi_change(WindowDevice& w, HWND hwnd, WPARAM wparam, LPARAM lparam)
{
    // HIWORD and LOWORD of wparam carry the same DPI.
    const unsigned dpi = HIWORD(wparam);
    const RECT* suggested = reinterpret_cast<const RECT*>(lparam);
    ::SetWindowPos(hwnd, nullptr, suggested->left, suggested->top,
                   suggested->right - suggested->left, suggested->bottom - suggested->top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    w.pending_ui_scale = ui_scale_for_dpi(dpi);
    return dpi;
}

LRESULT default_window_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_SYSCOMMAND:
        if ((wparam & 0xfff0) == SC_KEYMENU)  // Disable the ALT app menu.
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

}  // namespace hydra::ui
