// HydraPathBuilder.exe — the Path Builder's entry point.
//
// Stands up the Win32 window, the DirectX 11 device and Dear ImGui, then runs
// the frame loop over PbApp (pb_window.h). The window and device pieces are
// the ones Hydra.exe uses too (ui/win32_window.h). The builder sets no
// AppUserModelID, so Windows gives it a taskbar button of its own (the user's
// answer, 2026-10-29, in docs/superpowers/plans/2026-10-28-path-builder.md (removed; git show b29a3c28:docs/superpowers/plans/2026-10-28-path-builder.md)).
//
//   HydraPathBuilder.exe [chart file]   opens the chart, if one is given

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <shellapi.h>

#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/user_messages.h"
#include "core/version.h"  // HYDRA_WIDEN
#include "core/winstr.h"
#include "pathbuilder/pb_open.h"
#include "pathbuilder/pb_resource.h"
#include "pathbuilder/pb_window.h"
#include "ui/imgui_setup.h"
#include "ui/win32_window.h"

// The window's class name and title (the spec's decision 5), which
// CMakeLists.txt builds from HYDRA_APP_NAME.
static constexpr const wchar_t* kPathBuilderTitleW = HYDRA_WIDEN(HYDRA_PATHBUILDER_NAME);
// The ImGui layout file, beside the exe and apart from Hydra's hydra_ui.ini.
static constexpr const char* kPathBuilderUiIni = "hydra_pathbuilder_ui.ini";

// The main window's device and the changes its window procedure leaves.
static hydra::ui::WindowDevice  g_window;
// A file dropped on the window (WM_DROPFILES), opened by the frame loop.
static std::optional<std::string> g_DroppedFile;
// How long the loop waits between checks while the window cannot be seen.
static constexpr DWORD          kHiddenPollMs = 10;

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Why the builder can't start, in a message box: the plain sentence with the
// raw text under it.
static void show_startup_error(HWND owner, const std::exception& e)
{
    const std::wstring text = hydra::utf8_to_wide(hydra::app::plain_error_block(e));
    ::MessageBoxW(owner, text.c_str(), kPathBuilderTitleW, MB_OK | MB_ICONERROR);
}

int main()
{
    const std::vector<std::string> args = hydra::utf8_argv();
    // Before any worker thread: its error sentences leave out Hydra's library
    // advice (app::Program).
    hydra::app::set_program(hydra::app::Program::PathBuilder);

    ImGui_ImplWin32_EnableDpiAwareness();
    const float main_scale = hydra::ui::ui_scale_for_dpi(
        hydra::ui::monitor_dpi(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY)));

    HINSTANCE hInstance = ::GetModuleHandleW(nullptr);
    HICON hIconLarge = (HICON)::LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_PATHBUILDER_ICON),
                                           IMAGE_ICON, 32, 32, 0);
    HICON hIconSmall = (HICON)::LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_PATHBUILDER_ICON),
                                           IMAGE_ICON, 16, 16, 0);

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
                       hInstance, hIconLarge, nullptr, nullptr,
                       nullptr, kPathBuilderTitleW, hIconSmall };
    ::RegisterClassExW(&wc);

    hydra::ui::ImGuiSetupOptions imgui_options;
    imgui_options.dpi_scale = main_scale;
    imgui_options.ini_file = hydra::join_folder(hydra::app::exe_dir(), kPathBuilderUiIni);
    hydra::ui::setup_imgui(imgui_options);

    hydra::ui::WindowStartup startup{ &g_window, &wc, show_startup_error };
    startup.hwnd = ::CreateWindowW(
        wc.lpszClassName, kPathBuilderTitleW, WS_OVERLAPPEDWINDOW, 100, 100,
        (int)(1280 * main_scale), (int)(720 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);
    const HWND hwnd = startup.hwnd;
    if (!startup.create_device())
        return 1;

    if (hIconSmall) ::SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    if (hIconLarge) ::SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge);
    ::DragAcceptFiles(hwnd, TRUE);

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    const float window_scale = hydra::ui::ui_scale_for_dpi(
        hydra::ui::monitor_dpi(::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST)));
    if (window_scale != main_scale)
        hydra::ui::set_ui_scale(window_scale);

    if (!startup.start_backends())
        return 1;

    std::unique_ptr<hydra::pathbuilder::PbApp> app;
    try {
        app = std::make_unique<hydra::pathbuilder::PbApp>(g_window.device, g_window.context);
    } catch (const std::exception& e) {
        return startup.fail(e);
    }
    app->owner = hwnd;
    if (args.size() > 1) app->open(args[1]);

    while (hydra::ui::pump_messages())
    {
        if (g_DroppedFile)
        {
            app->open(*g_DroppedFile);
            g_DroppedFile.reset();
        }

        if (hydra::ui::main_window_hidden(g_window))
        {
            ::Sleep(kHiddenPollMs);
            continue;
        }
        g_window.occluded = false;
        hydra::ui::apply_window_changes(g_window);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        app->draw();
        ImGui::Render();
        hydra::ui::render_frame(g_window);

        // Popups that spill past the main window's edge get OS windows of
        // their own, as in the upstream example.
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        hydra::ui::present_frame(g_window);
    }

    app->close_preview();
    app.reset();
    startup.tear_down();
    return 0;
}

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (hydra::ui::imgui_takes_message(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        hydra::ui::note_resize(g_window, wParam, lParam);
        return 0;
    case WM_DPICHANGED:
        hydra::ui::follow_dpi_change(g_window, hWnd, wParam, lParam);
        return 0;
    case WM_DROPFILES:
        g_DroppedFile = hydra::pathbuilder::take_dropped_file(reinterpret_cast<void*>(wParam));
        return 0;
    }
    return hydra::ui::default_window_message(hWnd, msg, wParam, lParam);
}
