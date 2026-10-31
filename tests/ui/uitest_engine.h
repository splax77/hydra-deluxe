// The app-free half of the headless GUI runner (docs/agents/ui-testing.md).
//
// Runs a program's real ImGui UI with no window: a WARP (software) D3D11
// device, an offscreen render target, and the Dear ImGui Test Engine
// injecting input. It reads back the text on screen and the widget tree, and
// runs the command verbs (uitest_commands.h). Nothing here knows Hydra's
// window: the program reaches it through the hooks in EngineHooks. Hydra's
// own runner (uitest_harness.h) supplies them from its AppState.

#ifndef HYDRA_TESTS_UI_UITEST_ENGINE_H
#define HYDRA_TESTS_UI_UITEST_ENGINE_H

#include <cstdio>
#include <functional>
#include <optional>
#include <string>
#include <vector>

// The Windows headers first: they define a Yield() macro that the test
// engine's headers, included after them, undefine.
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include "imgui_te_context.h"
#include "imgui_te_engine.h"
#include "ui/imgui_setup.h"

namespace uitest {

// What the engine asks the program it drives. Each hook is one place the
// command verbs or frame() reach the program; an empty hook answers as a
// program with nothing of that kind would (see each).
struct EngineHooks {
    // Draw the program's UI for one frame. frame() calls it between
    // ImGui::NewFrame and ImGui::Render, inside the FrameText capture.
    // Empty: nothing is drawn.
    std::function<void()> draw_frame;
    // True while any background work the program runs is still going; the
    // wait-idle verb waits for false. Empty: never busy.
    std::function<bool()> jobs_busy;
    // The state verb's text: whole lines, each ending in a newline.
    // Empty: the verb prints nothing.
    std::function<std::string()> state_text;
    // A fresh program, as a session starts and the reset verb asks.
    // Empty: nothing is reset.
    std::function<void()> reset;
    // True when finding a widget labelled `label` by a wildcard lookup would
    // scroll a long list to reach it (in Hydra: the search box is empty and
    // the label is a library row's title). The item verbs refuse such a
    // lookup, and an absolute ref to such a label too. Empty: never.
    std::function<bool(const std::string& label)> is_library_row_title;
    // True when the library list shows every row, so a key that pages it or
    // jumps to an end can scroll it whole (in Hydra: the search box is
    // empty). The key verb refuses such a key while that list is focused.
    // Empty: never.
    std::function<bool()> library_unfiltered;
    // The open verb: open the song titled `title`, waiting at most
    // `settle_seconds` for it to settle. Returns the plain reason it could
    // not, or "" when it opened (or an engine action failed instead, its
    // reason in the test's log). Empty: the verb refuses, because the
    // program has no songs to open.
    std::function<std::string(ImGuiTestContext* ctx, const std::string& title,
                              double settle_seconds)>
        open_song;
    // A status line visible_text adds after the drawn text, or nullopt for
    // none. Empty: none.
    std::function<std::optional<std::string>()> status_line;
};

struct Engine {
    Engine() = default;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    virtual ~Engine() = default;

    // Offscreen D3D11 (WARP) + the render target ImGui draws into.
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> rt;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
    int width = 1280;
    int height = 800;

    // The scratch folder, made by init and deleted by shutdown unless
    // keep_temp is set.
    std::string temp_dir;
    std::string shots_dir;  // where `screenshot` files go (default: temp_dir)
    bool keep_temp = false;

    ImGuiTestEngine* engine = nullptr;
    // Every string ImGui drew last frame (ui/imgui_setup.h).
    hydra::ui::FrameText frame_text;
    EngineHooks hooks;
    // The window a session starts on and the open verb returns to.
    std::string home_ref = "//Hydra";

    // Command file for the "script" test (--script).
    std::string script_path;

    // Headless: WARP device, offscreen render target, scratch folder, ImGui
    // (set up with `options`), the DX11 backend and the engine.
    bool init(const hydra::ui::ImGuiSetupOptions& options);
    void frame();     // one full ImGui frame + engine PostSwap
    void shutdown();  // stop(), then the engine, ImGui, scratch dir

    // Attached: the program already owns the window, device, and ImGui
    // context; the engine adds the scratch folder, the engine itself (at a
    // watchable speed), and captures from the swapchain. The program drives
    // the frames itself and calls ImGuiTestEngine_PostSwap after Present.
    bool init_attached(ID3D11Device* dev, ID3D11DeviceContext* ctx, IDXGISwapChain* swapchain);
    bool attached = false;
    IDXGISwapChain* swapchain = nullptr;  // attached only
    // Print [PASS]/[FAIL] (+ the log of each failure) for every test that has
    // finished since the last call; returns the failure count of all tests
    // printed so far. The runner calls it after every frame, so a crash later
    // in the run still leaves the earlier results on screen.
    int print_results(FILE* out);
    std::vector<const ImGuiTest*> printed;  // tests print_results has shown
    int printed_failures = 0;
    // The test the engine is running now, or nullptr between tests.
    const ImGuiTest* running_test() const;
    // Stop the engine. Must run before ImGui::DestroyContext; shutdown()
    // calls it too (idempotent). A program that owns more than the engine
    // (Hydra's AppState) overrides it to drop that too, after this.
    virtual void stop();
    bool stopped = false;
};

// The engine a test runs under: each test's UserData is its Engine.
inline Engine& engine_of(ImGuiTestContext* ctx) {
    return *static_cast<Engine*>(ctx->Test->UserData);
}

// Register one test under the name --test and --list use, with the engine
// as its UserData. The script test registers through it.
void register_test(Engine& e, const char* name, void (*fn)(ImGuiTestContext*));
// Register the "script" test (uitest_script.cpp), which runs e.script_path.
void register_script_test(Engine& e);
// Whether a line a script file or the live pipe handed over is a command to
// run (defined in uitest_script.cpp, the one place that decides). On true,
// `command` holds the line as run_command takes it.
bool command_line(const std::string& raw, std::string& command);

// True once the test engine is stopping, as when the app's window closed.
// The engine's stop runs the test until it returns, so any loop in a test
// that waits on something outside it must end when this holds.
bool engine_stopping(ImGuiTestContext* ctx);

// Forget an abort the engine raised on its own, such as the person at the
// window holding Esc, so the test goes on. Does nothing while the engine is
// stopping: that abort is meant.
void clear_abort(ImGuiTestContext* ctx);

// Yield frames until pred() holds or `seconds` of wall-clock pass. Jobs run
// on real threads, so the wait is wall-clock, not frame-count. Returns false
// at once when the test has already failed (ctx->IsError()) or the engine is
// stopping (engine_stopping).
bool wait_until(ImGuiTestContext* ctx, const std::function<bool()>& pred, double seconds);

// One step of that wait: a frame, then a short nap. For a loop that polls
// something wait_until's predicate cannot express; such a loop checks
// engine_stopping itself.
void idle_frame(ImGuiTestContext* ctx);

// All text ImGui drew last frame plus the program's status line
// (EngineHooks::status_line), for substring checks.
std::string visible_text(Engine& e);

// A window drawn last frame whose name holds `part`, or nullptr when none
// was. A part finds child windows too, whose names ImGui mangles.
ImGuiWindow* window_named(const char* part);

// Whether `row` sits wholly inside the list's visible area.
bool fully_shown(const ImGuiWindow* list, const ImRect& row);

// The widget tree (label, id, state flags, rect) of every window whose name
// holds `window_name`, or of every window when it is empty. One line per
// window and per item, each ending in a newline. With `contains`, only the
// item lines holding it are kept, each under its window's line; a window
// with no such item is left out.
std::string widget_dump(ImGuiTestContext* ctx, const std::string& window_name,
                        const std::string& contains = "");

// The one line that names the top open modal dialog and its buttons, which
// block every click and key outside it, or "" when no modal is open. No
// newline at the end. Every reply that mentions an open dialog uses it.
std::string open_modal_note(ImGuiTestContext* ctx);

// The lines of `text` that hold `part`, each ending in a newline, or a line
// saying none did. The text verb's filter.
std::string lines_holding(const std::string& text, const std::string& part);

// One item's line in widget_dump's shape, indented two spaces per `depth`.
// The one place an item line is formatted.
std::string item_line(const ImGuiTestItemInfo& it, int depth);

// Save the current frame to `file` (PNG). Returns false if the capture failed.
// The saved file's full path goes to *saved_to when the caller asks for it
// (the screenshot verb puts it in its result); otherwise it is printed.
// With `crop`, only that rectangle is saved, clamped to the frame; without
// it, the whole frame.
bool screenshot(ImGuiTestContext* ctx, const std::string& file, std::string* saved_to = nullptr,
                const ImRect* crop = nullptr);

// Escape '/' and '#' in a label so it can be used as one path segment of an
// ImGuiTestRef ("**/" + escape(title)).
std::string escape_ref(const std::string& label);

// The drawn items under the current ref, child windows included, whose label
// is `label` (compared the way the engine's "**/" lookup compares: by the
// label's hash, so "text###id" matches on what follows "###"). One gather, the
// one widget_dump walks too.
std::vector<ImGuiTestItemInfo> drawn_items_labelled(ImGuiTestContext* ctx,
                                                    const std::string& label);

}  // namespace uitest

#endif  // HYDRA_TESTS_UI_UITEST_ENGINE_H
