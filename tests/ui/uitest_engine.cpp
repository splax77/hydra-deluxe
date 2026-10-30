#include "uitest_engine.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <thread>

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"
#include "imgui_te_context.h"
#include "imgui_te_internal.h"

#include "../temp_util.h"
#include "../warp_util.h"  // tests/ is not on the runner's include path

#include "app/allocator.h"

namespace fs = std::filesystem;

namespace uitest {

namespace {

// The engine's screen-capture callback: copy the requested rect of the
// offscreen render target (R8G8B8A8) to CPU memory.
bool capture_pixels(ImGuiID /*viewport_id*/, int x, int y, int w, int h, unsigned int* pixels,
                    void* user) {
    auto& hz = *static_cast<Engine*>(user);
    // Headless: the offscreen target. Attached: the swapchain's back buffer.
    Microsoft::WRL::ComPtr<ID3D11Texture2D> source = hz.rt;
    if (hz.attached) {
        if (!hz.swapchain || FAILED(hz.swapchain->GetBuffer(0, IID_PPV_ARGS(&source))))
            return false;
        D3D11_TEXTURE2D_DESC sd;
        source->GetDesc(&sd);
        hz.width = (int)sd.Width;
        hz.height = (int)sd.Height;
    }
    D3D11_TEXTURE2D_DESC d;
    source->GetDesc(&d);
    d.Usage = D3D11_USAGE_STAGING;
    d.BindFlags = 0;
    d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    d.MiscFlags = 0;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;
    if (FAILED(hz.device->CreateTexture2D(&d, nullptr, &staging))) return false;
    hz.context->CopyResource(staging.Get(), source.Get());
    D3D11_MAPPED_SUBRESOURCE m;
    if (FAILED(hz.context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &m))) return false;
    const auto* src = static_cast<const unsigned char*>(m.pData);
    for (int row = 0; row < h; ++row) {
        int sy = y + row;
        if (sy < 0 || sy >= hz.height) continue;
        for (int col = 0; col < w; ++col) {
            int sx = x + col;
            if (sx < 0 || sx >= hz.width) continue;
            std::memcpy(&pixels[row * w + col], src + sy * m.RowPitch + sx * 4, 4);
        }
    }
    hz.context->Unmap(staging.Get(), 0);
    return true;
}

// The scratch folder.
void init_temp(Engine& e) {
    e.temp_dir = testtemp::temp_dir("uitest");
    if (e.shots_dir.empty()) e.shots_dir = e.temp_dir;
}

void init_engine(Engine& e, ImGuiTestRunSpeed speed) {
    e.engine = ImGuiTestEngine_CreateContext();
    ImGuiTestEngineIO& eio = ImGuiTestEngine_GetIO(e.engine);
    eio.ConfigRunSpeed = speed;
    eio.ConfigNoThrottle = speed == ImGuiTestRunSpeed_Fast;
    eio.ConfigFixedDeltaTime = speed == ImGuiTestRunSpeed_Fast ? 1.0f / 60.0f : 0.0f;
    eio.ConfigSavedSettings = false;
    eio.ConfigMouseDrawCursor = speed != ImGuiTestRunSpeed_Fast;
    eio.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
    eio.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
    eio.ConfigLogToTTY = false;  // the runner prints each failed test's log itself
    eio.ConfigWatchdogWarning = 120.0f;
    eio.ConfigWatchdogKillTest = 600.0f;  // a real analysis can take a while
    eio.ScreenCaptureFunc = capture_pixels;
    eio.ScreenCaptureUserData = &e;
    e.frame_text.enabled = true;
}

}  // namespace

bool Engine::init_attached(ID3D11Device* dev, ID3D11DeviceContext* ctx, IDXGISwapChain* sc) {
    attached = true;
    device = dev;
    context = ctx;
    swapchain = sc;
    init_temp(*this);
    init_engine(*this, ImGuiTestRunSpeed_Normal);
    return true;
}

void register_test(Engine& e, const char* name, void (*fn)(ImGuiTestContext*)) {
    ImGuiTest* t = IM_REGISTER_TEST(e.engine, "hydra", name);
    t->UserData = &e;
    t->TestFunc = fn;
}

int Engine::print_results(FILE* out) {
    ImVector<ImGuiTest*> tests;
    ImGuiTestEngine_GetTestList(engine, &tests);
    for (ImGuiTest* t : tests) {
        // Not run, still waiting, or still running: nothing to print yet.
        const ImGuiTestStatus status = t->Output.Status;
        if (status != ImGuiTestStatus_Success && status != ImGuiTestStatus_Error) continue;
        if (std::find(printed.begin(), printed.end(), t) != printed.end()) continue;
        printed.push_back(t);
        const bool ok = status == ImGuiTestStatus_Success;
        std::fprintf(out, "[%s] %s/%s\n", ok ? "PASS" : "FAIL", t->Category, t->Name);
        if (!ok) {
            ++printed_failures;
            std::fprintf(out, "---- log ----\n%s---- end ----\n", t->Output.Log.Buffer.c_str());
        }
    }
    std::fflush(out);
    return printed_failures;
}

const ImGuiTest* Engine::running_test() const {
    if (!engine || !engine->TestContext) return nullptr;
    return engine->TestContext->Test;
}

bool Engine::init(const hydra::ui::ImGuiSetupOptions& options) {
#if defined(NDEBUG) && defined(HYDRA_MIMALLOC)
    // This runner links what the program it tests links, so a link order
    // that leaves malloc on the Windows heap here does the same to the app
    // (D88). The Debug runtime is never redirected, and a build without
    // mimalloc (HYDRA_MIMALLOC in CMakeLists.txt) has no redirect to check.
    if (!hydra::app::malloc_redirected()) {
        std::fprintf(stderr, "hydra_uitest: malloc is not going to mimalloc; mimalloc.dll "
                             "must come first in the exe's imports (CMakeLists.txt)\n");
        return false;
    }
#endif
    if (!warp::make_device(device, context)) {
        std::fprintf(stderr, "hydra_uitest: could not create a WARP D3D11 device\n");
        return false;
    }
    D3D11_TEXTURE2D_DESC td{};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET;
    if (FAILED(device->CreateTexture2D(&td, nullptr, &rt)) ||
        FAILED(device->CreateRenderTargetView(rt.Get(), nullptr, &rtv))) {
        std::fprintf(stderr, "hydra_uitest: could not create the render target\n");
        return false;
    }

    init_temp(*this);

    hydra::ui::setup_imgui(options);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    ImGui_ImplDX11_Init(device.Get(), context.Get());

    init_engine(*this, ImGuiTestRunSpeed_Fast);
    return true;
}

void Engine::frame() {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();
    hydra::ui::begin_frame_text_capture(&frame_text);
    if (hooks.draw_frame) hooks.draw_frame();
    hydra::ui::end_frame_text_capture(&frame_text);
    ImGui::Render();
    ID3D11RenderTargetView* views[] = {rtv.Get()};
    context->OMSetRenderTargets(1, views, nullptr);
    context->ClearRenderTargetView(rtv.Get(), hydra::ui::kClearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    // Nothing presents here, so nothing paces the software GPU: without a
    // flush, a long wait queues frames faster than WARP draws them and their
    // buffers pile up. The user's call is in D98 item 1.
    context->Flush();
    // A removed device makes every later texture and shader fail quietly and
    // ends in a crash several tests on, so stop the run where it happened.
    const HRESULT removed = device->GetDeviceRemovedReason();
    if (removed != S_OK) {
        const ImGuiTest* test = running_test();
        std::fprintf(stderr,
                     "hydra_uitest: the D3D device was removed (0x%08lX) during test %s\n",
                     static_cast<unsigned long>(removed), test ? test->Name : "(none)");
        std::fflush(stderr);
        std::exit(3);
    }
    ImGuiTestEngine_PostSwap(engine);
}

void Engine::stop() {
    if (engine && !stopped) ImGuiTestEngine_Stop(engine);
    stopped = true;
}

void Engine::shutdown() {
    stop();
    if (!attached) {
        ImGui_ImplDX11_Shutdown();
        hydra::ui::shutdown_imgui();
    }
    if (engine) ImGuiTestEngine_DestroyContext(engine);
    engine = nullptr;
    if (!keep_temp && !temp_dir.empty()) {
        std::error_code ec;
        fs::remove_all(fs::u8path(temp_dir), ec);
    }
}

void idle_frame(ImGuiTestContext* ctx) {
    ctx->Yield();
    // Fast mode spins frames flat out; a short nap keeps the worker
    // threads from being starved while we poll.
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

bool engine_stopping(ImGuiTestContext* ctx) {
    // ImGuiTestEngine_Stop raises this before it runs the test to its end,
    // and ctx->IsError() does not see it. The engine's Abort flag is not
    // used: holding Esc in the window for a moment sets it too, and that must
    // not end a live session the user is looking at.
    return ctx->Engine->TestQueueCoroutineShouldExit;
}

void clear_abort(ImGuiTestContext* ctx) {
    if (engine_stopping(ctx)) return;
    ctx->Abort = false;
    ctx->Engine->Abort = false;
}

bool wait_until(ImGuiTestContext* ctx, const std::function<bool()>& pred, double seconds) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::duration<double>(seconds);
    while (!pred()) {
        // The test has already failed (a click found no such item, a check
        // failed), or the engine is stopping: nothing it waits on will
        // happen, so don't sit out the timeout.
        if (ctx->IsError() || engine_stopping(ctx)) return false;
        if (std::chrono::steady_clock::now() > deadline) return false;
        idle_frame(ctx);
    }
    // One more frame so the screen reflects the state the caller waited for.
    ctx->Yield();
    return true;
}

ImGuiWindow* window_named(const char* part) {
    for (ImGuiWindow* w : ImGui::GetCurrentContext()->Windows)
        if (w->WasActive && std::strstr(w->Name, part)) return w;
    return nullptr;
}

bool fully_shown(const ImGuiWindow* list, const ImRect& row) {
    return row.Min.y >= list->InnerRect.Min.y && row.Max.y <= list->InnerRect.Max.y;
}

std::string visible_text(Engine& e) {
    std::string s = e.frame_text.text;
    if (e.hooks.status_line) {
        if (const std::optional<std::string> status = e.hooks.status_line()) {
            s += "\n";
            s += *status;
        }
    }
    return s;
}

namespace {

// The drawn items under `parent` at every depth (-1), because items inside
// tables and pushed IDs sit deeper than one level. The gather also climbs from
// child windows to the parent. The one gather widget_dump and
// drawn_items_labelled share.
void gather_all(ImGuiTestContext* ctx, ImGuiTestRef parent, ImGuiTestItemList& items) {
    ctx->GatherItems(&items, parent, -1);
}

// One window's line and its item lines; with `contains`, only the item lines
// holding it, and nothing at all when none does. True when anything was
// written.
bool dump_window(ImGuiTestContext* ctx, ImGuiWindow* w, const std::string& contains,
                 ImGuiTextBuffer& out) {
    std::string items_text;
    ImGuiTestItemList items;
    // Each child window gets its own dump, so only this window's items stay.
    gather_all(ctx, ImGuiTestRef(w->ID), items);
    for (const ImGuiTestItemInfo& it : items) {
        if (it.Window != w) continue;
        const std::string line = item_line(it, it.Depth + 1);
        if (contains.empty() || line.find(contains) != std::string::npos) items_text += line;
    }
    if (!contains.empty() && items_text.empty()) return false;
    out.appendf("window \"%s\" id=0x%08X pos=(%.0f,%.0f) size=(%.0f,%.0f)%s%s\n", w->Name, w->ID,
                w->Pos.x, w->Pos.y, w->Size.x, w->Size.y,
                (w->Flags & ImGuiWindowFlags_Popup) ? " popup" : "",
                (w->Flags & ImGuiWindowFlags_ChildWindow) ? " child" : "");
    out.append(items_text.c_str());
    return true;
}

}  // namespace

std::vector<ImGuiTestItemInfo> drawn_items_labelled(ImGuiTestContext* ctx,
                                                    const std::string& label) {
    ImGuiTestItemList items;
    gather_all(ctx, ctx->GetRef(), items);
    const ImGuiID wanted = ImHashStr(label.c_str());
    std::vector<ImGuiTestItemInfo> found;
    for (const ImGuiTestItemInfo& it : items)
        if (ImHashStr(it.DebugLabel) == wanted) found.push_back(it);
    return found;
}

std::string open_modal_note(ImGuiTestContext* ctx) {
    ImGuiWindow* modal = ImGui::GetTopMostPopupModal();
    if (modal == nullptr) return "";
    // The name as drawn: what comes before any "##".
    auto shown = [](const char* label) {
        const char* hash = std::strstr(label, "##");
        return hash ? std::string(label, hash) : std::string(label);
    };
    std::string buttons;
    ImGuiTestItemList items;
    // The engine gathers nothing while the test is in error, and a failed
    // command is the main reason to call this, so the status is lifted for
    // the gather and put back after it.
    const ImGuiTestStatus status = ctx->TestOutput->Status;
    if (ctx->IsError()) ctx->TestOutput->Status = ImGuiTestStatus_Running;
    gather_all(ctx, ImGuiTestRef(modal->ID), items);
    ctx->TestOutput->Status = status;
    for (const ImGuiTestItemInfo& it : items) {
        // Fields, checkboxes and tree nodes are not buttons.
        if (it.StatusFlags & (ImGuiItemStatusFlags_Inputable | ImGuiItemStatusFlags_Checkable |
                              ImGuiItemStatusFlags_Openable))
            continue;
        const std::string label = shown(it.DebugLabel);
        if (label.empty()) continue;
        buttons += (buttons.empty() ? "" : ", ") + label;
    }
    std::string note = "A dialog is open: \"" + shown(modal->Name) + "\"";
    if (!buttons.empty()) note += " (buttons: " + buttons + ")";
    return note + ". Close it first.";
}

std::string lines_holding(const std::string& text, const std::string& part) {
    std::string out;
    size_t start = 0;
    while (start < text.size()) {
        size_t nl = text.find('\n', start);
        if (nl == std::string::npos) nl = text.size();
        // ImGui's log, which the screen text comes from, ends lines in "\r\n".
        // Every text reply passes through here, so this is the one place
        // the '\r' goes.
        const size_t end = nl > start && text[nl - 1] == '\r' ? nl - 1 : nl;
        const std::string line = text.substr(start, end - start);
        if (line.find(part) != std::string::npos) out += line + "\n";
        start = nl + 1;
    }
    if (out.empty()) out = "(no line holding \"" + part + "\")\n";
    return out;
}

std::string item_line(const ImGuiTestItemInfo& it, int depth) {
    std::string flags;
    if (it.ItemFlags & ImGuiItemFlags_Disabled) flags += " disabled";
    if (it.StatusFlags & ImGuiItemStatusFlags_Checkable)
        flags += (it.StatusFlags & ImGuiItemStatusFlags_Checked) ? " checked" : " unchecked";
    if (it.StatusFlags & ImGuiItemStatusFlags_Openable)
        flags += (it.StatusFlags & ImGuiItemStatusFlags_Opened) ? " opened" : " closed";
    if (it.StatusFlags & ImGuiItemStatusFlags_Inputable) flags += " inputable";
    ImGuiTextBuffer out;
    out.appendf("%*s\"%s\" id=0x%08X rect=(%.0f,%.0f %.0fx%.0f)%s\n", depth * 2, "",
                it.DebugLabel, it.ID, it.RectFull.Min.x, it.RectFull.Min.y,
                it.RectFull.GetWidth(), it.RectFull.GetHeight(), flags.c_str());
    return out.c_str();
}

std::string widget_dump(ImGuiTestContext* ctx, const std::string& window_name,
                        const std::string& contains) {
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiTextBuffer out;
    bool any_window = false, any_item = false;
    // The top open modal goes first: it is the window that takes input, and
    // ImGui keeps it last in g.Windows, where a reply cut short from the end
    // (the MCP helper's cap) drops it.
    ImGuiWindow* modal = ImGui::GetTopMostPopupModal();
    std::vector<ImGuiWindow*> order;
    if (modal) order.push_back(modal);
    for (ImGuiWindow* w : g.Windows)
        if (w != modal) order.push_back(w);
    for (ImGuiWindow* w : order) {
        if (!w->WasActive || w->Hidden) continue;
        if (std::strcmp(w->Name, "Debug##Default") == 0) continue;
        if (!window_name.empty() && std::strstr(w->Name, window_name.c_str()) == nullptr) continue;
        any_window = true;
        if (dump_window(ctx, w, contains, out)) any_item = true;
    }
    if (!any_window) out.appendf("(no window matching \"%s\")\n", window_name.c_str());
    else if (!any_item) out.appendf("(no item holding \"%s\")\n", contains.c_str());
    return out.c_str();
}

bool screenshot(ImGuiTestContext* ctx, const std::string& file, std::string* saved_to,
                const ImRect* crop) {
    Engine& e = engine_of(ctx);
    std::string path = file;
    if (fs::u8path(path).is_relative()) path = e.shots_dir + "\\" + path;
    ImGuiCaptureArgs* args = ctx->CaptureArgs;
    ImStrncpy(args->InOutputFile, path.c_str(), IM_ARRAYSIZE(args->InOutputFile));
    args->InCaptureWindows.clear();
    ImRect rect(0, 0, (float)e.width, (float)e.height);
    if (crop) {
        rect.ClipWithFull(*crop);
        if (rect.GetWidth() < 1.0f || rect.GetHeight() < 1.0f) return false;  // off the frame
    }
    args->InCaptureRect = rect;
    args->InPadding = 0.0f;
    bool ok = ctx->CaptureScreenshot(ImGuiCaptureFlags_IncludeOtherWindows |
                                     ImGuiCaptureFlags_HideMouseCursor);
    if (ok && saved_to) *saved_to = path;
    else if (ok) std::printf("screenshot: %s\n", path.c_str());
    return ok;
}

std::string escape_ref(const std::string& label) {
    std::string out;
    for (char c : label) {
        if (c == '/' || c == '#' || c == '\\') out += '\\';
        out += c;
    }
    return out;
}

}  // namespace uitest
