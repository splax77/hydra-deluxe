// The Win32 window and Direct3D 11 pieces every one of Hydra's GUI programs
// stands its main window on: the monitor's DPI, the device and swap chain,
// the start-up steps and their undo, the message pump, the per-frame resize,
// render and present, and the window messages they all answer alike. Each
// program keeps its own WndProc and frame loop and calls these from them.
// The pieces follow the upstream docking-branch example_win32_directx11.

#ifndef HYDRA_UI_WIN32_WINDOW_H
#define HYDRA_UI_WIN32_WINDOW_H

#include <d3d11.h>

#include <exception>
#include <stdexcept>

namespace hydra::ui {

// A monitor's DPI, read the way the ImGui Win32 backend reads it. Windows 8.1
// and later answer through GetDpiForMonitor in shcore.dll, loaded at run time
// because Hydra does not link Shcore.lib. Older Windows has no per-monitor
// DPI, so the screen's LOGPIXELSX answers instead. 0 means no reading, which
// ui_scale_for_dpi turns into the unscaled 1.0.
unsigned monitor_dpi(HMONITOR monitor);

// The Direct3D 11 device behind a program's main window, its swap chain and
// render target, and the changes the window procedure leaves for the frame
// loop to apply between frames.
struct WindowDevice {
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swap_chain = nullptr;
    ID3D11RenderTargetView* render_target = nullptr;
    // The last present found the window hidden (minimized or covered).
    bool occluded = false;
    // The client size WM_SIZE left to apply; 0 when none waits.
    UINT resize_width = 0, resize_height = 0;
    // The UI scale WM_DPICHANGED left to apply; 0 when none waits.
    float pending_ui_scale = 0.0f;
};

// Makes the device, the swap chain on `hwnd` and its render target. S_OK, or
// the HRESULT the device creation failed with, for the start-up box.
HRESULT create_window_device(WindowDevice& w, HWND hwnd);
// Releases whatever create_window_device made. Safe on a partial device.
void release_window_device(WindowDevice& w);

// The raw text for a Win32, Direct3D or ImGui backend call that failed at
// startup. It carries no error kind, so the box reads the fallback sentence.
std::runtime_error startup_call_failed(const char* call, const char* code_name,
                                       unsigned long code);

// What a program's start-up has stood up so far. The normal exit and a
// failed start-up both undo it through tear_down, so the two can't drift
// apart. The program fills `hwnd` once CreateWindowW returns.
struct WindowStartup {
    WindowDevice* device = nullptr;
    const WNDCLASSEXW* window_class = nullptr;
    // Shows why the program can't start, over `owner` (null until the window
    // exists). Each program titles its own box.
    void (*show_error)(HWND owner, const std::exception& e) = nullptr;
    HWND hwnd = nullptr;
    bool win32_backend_up = false, dx11_backend_up = false;

    // Undoes what is up: the ImGui backends, ImGui, the device, the window
    // and its class.
    void tear_down();
    // A start-up step that failed: shows why, tears down, and returns 1, the
    // program's exit code.
    int fail(const std::exception& e);
    // Checks the window was made, then makes the device. On a failure it
    // calls fail and returns false.
    bool create_device();
    // Starts ImGui's Win32 and DirectX 11 backends on the window and device.
    // On a failure it calls fail and returns false.
    bool start_backends();
};

// Dispatches every waiting window message. False once WM_QUIT came, when the
// frame loop ends.
bool pump_messages();

// True when the main window can't be seen: the last present found it hidden
// and a test present still does.
bool main_window_hidden(WindowDevice& w);

// Applies a resize and a UI scale the window procedure left, between frames:
// ImGui's style must not change inside one.
void apply_window_changes(WindowDevice& w);

// Clears the main window to kClearColor (ui/imgui_setup.h) and draws ImGui's
// frame into it. Call after ImGui::Render().
void render_frame(WindowDevice& w);

// Presents the frame at vsync and notes whether the window was hidden.
void present_frame(WindowDevice& w);

// ---- The window procedure's shared steps ----------------------------------

// True when ImGui's Win32 backend took the message; the window procedure
// then returns true at once.
bool imgui_takes_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// WM_SIZE: keeps the new client size for apply_window_changes. False for a
// minimize, which keeps nothing.
bool note_resize(WindowDevice& w, WPARAM wparam, LPARAM lparam);

// WM_DPICHANGED: Windows moved the window to a monitor with another scale.
// Takes the rectangle it suggests (the same physical size there) and keeps
// the new scale for apply_window_changes. Returns the new DPI.
unsigned follow_dpi_change(WindowDevice& w, HWND hwnd, WPARAM wparam, LPARAM lparam);

// Every message the program's own window procedure leaves: turns off the ALT
// app menu, quits on WM_DESTROY, and hands the rest to DefWindowProcW.
LRESULT default_window_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

}  // namespace hydra::ui

#endif  // HYDRA_UI_WIN32_WINDOW_H
