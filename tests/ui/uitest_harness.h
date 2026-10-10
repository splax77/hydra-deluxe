// hydra_uitest — the headless GUI test runner (docs/agents/ui-testing.md).
//
// Runs Hydra's real ImGui UI with no window: a WARP (software) D3D11 device,
// an offscreen render target, and the Dear ImGui Test Engine injecting input.
// Tests find widgets by label ("Scan charts"), click them, and read back the
// app's state and the text on screen. Everything runs on scratch files in a
// temp folder; no browser, sound card, or network is touched.

#ifndef HYDRA_TESTS_UI_UITEST_HARNESS_H
#define HYDRA_TESTS_UI_UITEST_HARNESS_H

#include <chrono>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include "imgui_te_context.h"
#include "imgui_te_engine.h"
#include "ui/app_shell.h"
#include "ui/app_state.h"
#include "ui/column_widths.h"


namespace uitest {

struct Harness {
    // Offscreen D3D11 (WARP) + the render target ImGui draws into.
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> rt;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
    int width = 1280;
    int height = 800;

    // Scratch files. temp_dir holds the settings INI, the DB, and the HTML
    // reports the jobs write next to the DB.
    std::string temp_dir;
    std::string db_path;
    std::string ini_path;
    std::string rules_path;
    std::string shots_dir;  // where `screenshot` files go (default: temp_dir)
    bool keep_temp = false;
    // --db: each test starts from a copy of this database instead of an
    // empty one. The file itself is only read, never opened by the app.
    std::string seed_db;

    std::unique_ptr<hydra::ui::AppState> app;
    // Set by reset_app for the one frame that runs with the old app undrawn,
    // so nothing it drew is still waiting to render when it is freed.
    bool app_hidden = false;
    ImGuiTestEngine* engine = nullptr;
    hydra::ui::FrameText frame_text;

    // Command file for the "script" test (--script).
    std::string script_path;

    // Headless: WARP device, offscreen render target, ImGui context, engine.
    bool init();
    void frame();     // one full ImGui frame + engine PostSwap
    void shutdown();  // engine, ImGui, scratch dir

    // Attached (Hydra.exe --uitest): the app already owns the window, device,
    // and ImGui context; the harness adds scratch paths, the engine (at a
    // watchable speed), and captures from the swapchain. main.cpp drives the
    // frames itself and calls engine_post_swap() after Present.
    bool init_attached(ID3D11Device* dev, ID3D11DeviceContext* ctx, IDXGISwapChain* swapchain);
    bool attached = false;
    IDXGISwapChain* swapchain = nullptr;  // attached only
    // Queue a test by name or, when `what` names a file, as a script.
    bool queue(const std::string& what);
    // Print [PASS]/[FAIL] (+ the log of each failure) for every test that has
    // finished since the last call; returns the failure count of all tests
    // printed so far. The runner calls it after every frame, so a crash later
    // in the run still leaves the earlier results on screen.
    int print_results(FILE* out);
    std::vector<const ImGuiTest*> printed;  // tests print_results has shown
    int printed_failures = 0;
    // The test the engine is running now, or nullptr between tests.
    const ImGuiTest* running_test() const;
    // Stop the engine and drop the app. Must run before ImGui::DestroyContext;
    // shutdown() calls it too (idempotent).
    void stop();
    bool stopped = false;
};

// Whether a test name given on the command line picks the test called
// `test_name`: "all" picks every test, any other name only the test with
// exactly that name.
bool selects(const std::string& what, const char* test_name);

// Fresh app state for a test: rewrite the scratch INI from
// scratch_settings() (tests/scratch_settings.h), delete the DB, rebuild AppState on those
// paths, and (re)install the headless seams. Call at the start of TestFunc —
// the GUI thread is parked between frames while TestFunc runs, so swapping
// the AppState here is safe.
// rules_text: the scratch hydra_rules.ini's contents. Empty (the default)
// means no rules file, so every other test runs today's rules.
// wait_store: the app opens its store on a worker thread. True (the default)
// blocks in AppState::wait_store_open until the store is ready, so the test
// can click at once, and fails the test if the open failed. Only a test that
// looks at the startup screen passes false.
void reset_app(Harness& h, const std::string& rules_text = "", bool wait_store = true);

// Register every C++ test (the uitest_<area>.cpp files, in the order
// uitest_tests.cpp fixes) and, when h.script_path is set, the "script" test
// (uitest_script.cpp). Each test's UserData is &h.
void register_tests(Harness& h);
void register_script_test(Harness& h);

// ---- helpers shared by the tests and the script interpreter --------------

inline Harness& harness(ImGuiTestContext* ctx) {
    return *static_cast<Harness*>(ctx->Test->UserData);
}

// Yield frames until pred() holds or `seconds` of wall-clock pass. Jobs run
// on real threads, so the wait is wall-clock, not frame-count. Returns false
// at once when the test has already failed (ctx->IsError()).
bool wait_until(ImGuiTestContext* ctx, const std::function<bool()>& pred, double seconds);

// True while any background job (scan, batch, analyze, report, DM fetch/report)
// or the Preview's load is still running.
bool jobs_busy(Harness& h);

// Holds a batch's charts at a gate until the test lets them through. The test
// library analyzes in a blink, so an ungated run can start and finish between
// two frames, and a test that looks at the running batch races it. While a
// gate lives, every batch the app starts runs on `workers` workers (default
// one). Each chart waits at the gate, ticking progress so Stop still reaches
// it, until allow() has let that many charts through, counted in the order
// they reached it. The "Now:" title is set before a chart reaches the gate,
// so with one worker started() == n means chart n is on screen and held.
// With several, started() - allowed is how many are held at once.
// Make the gate before starting the batch. Its destructor opens the gate and
// removes the seam, so a check that fails early leaves nothing stuck.
// Only one gate can exist at a time (it resets shared counters): make it after reset_app.
class BatchGate {
public:
    explicit BatchGate(int workers = 1);
    ~BatchGate();
    BatchGate(const BatchGate&) = delete;
    BatchGate& operator=(const BatchGate&) = delete;
    // Let the first `charts` charts of the run through the gate.
    void allow(int charts);
    // Charts that have reached the gate so far (1 = the first is held there).
    int started() const;
};

// Holds a click's analysis open until the test lets it through. A test chart
// analyzes in a few milliseconds, so a test that looks at the running click
// (its progress box, Cancel, a setting changed under it) would race it. While
// a gate lives, every click's job waits at the gate, ticking progress so
// Cancel and a setting change still stop it, until open(). started() counts
// the jobs that reached it. Make it after reset_app; its destructor opens it
// and removes the seam. Only one gate can exist at a time.
class ViewGate {
public:
    ViewGate();
    ~ViewGate();
    ViewGate(const ViewGate&) = delete;
    ViewGate& operator=(const ViewGate&) = delete;
    void open();
    int started() const;
};

// Holds the app's store open on its worker thread until the test lets it
// through. A scratch library opens in a blink, so a test that looks at the
// startup screen would race it. While a gate lives, the open's progress waits
// at the gate once it reaches the upgrade's copy, until open(). started()
// counts the opens that reached it. Make it BEFORE reset_app (the open starts
// in AppState's constructor) and pass wait_store=false; its destructor opens
// it and removes the seam. Only one gate can exist at a time.
class OpenGate {
public:
    OpenGate();
    ~OpenGate();
    OpenGate(const OpenGate&) = delete;
    OpenGate& operator=(const OpenGate&) = delete;
    void open();
    int started() const;
};

// All text ImGui drew last frame plus the status line, for substring checks.
std::string visible_text(Harness& h);

// True when `s` is somewhere in visible_text(h).
bool on_screen(Harness& h, const std::string& s);

// The dmleaderboards API's canned answers, which reset_app serves and the
// tests that serve their own variant build from: the one user (alice), one
// score row, and the scores reply that wraps a list of rows.
std::string canned_dm_users();
std::string canned_dm_score(const std::string& identifier, int points);
std::string canned_dm_scores(const std::vector<std::string>& score_rows);

// A window drawn last frame whose name holds `part`, or nullptr when none
// was. A part finds child windows too, whose names ImGui mangles.
ImGuiWindow* window_named(const char* part);

// Print the widget tree (label, id, state flags, rect) of one window, or of
// every window when `window_name` is empty.
void dump_widgets(ImGuiTestContext* ctx, const std::string& window_name);

// Print the key AppState fields.
void dump_state(Harness& h);

// Save the current frame to `file` (PNG). Returns false if the capture failed.
bool screenshot(ImGuiTestContext* ctx, const std::string& file);

// Escape '/' and '#' in a label so it can be used as one path segment of an
// ImGuiTestRef ("**/" + escape(title)).
std::string escape_ref(const std::string& label);

// How wide ImGui draws `s` in `font` (the current font when null), at the
// size it draws text now: the style's base size times its main and DPI
// scales. The one place a GUI test measures text.
float text_width(const char* s, ImFont* font = nullptr);

// text_width's measuring, as the width rule's WidthOf (ui/column_widths.h),
// for a test that checks a column against the rule.
hydra::ui::WidthOf text_measurer(ImFont* font = nullptr);

// ---- the checked-in C++ tests ---------------------------------------------

// One checked-in test: the name --test and --list use, and its body.
struct TestEntry {
    const char* name;
    void (*fn)(ImGuiTestContext*);
};

// Each area file's tests, in the order that file lists them.
const std::vector<TestEntry>& library_tests();       // uitest_library.cpp
const std::vector<TestEntry>& details_tests();       // uitest_details.cpp
const std::vector<TestEntry>& preview_tests();       // uitest_preview.cpp
const std::vector<TestEntry>& batch_report_tests();  // uitest_batch_reports.cpp

// Steps most tests start with (defined in uitest_harness.cpp).
// Scan testdata/input through the UI and land on the populated library.
void scan_library(ImGuiTestContext* ctx);
// Point the ref at the song panel child (//Hydra/##songpanel).
void set_panel_ref(ImGuiTestContext* ctx);
// Wait until the song panel's tab bar can't move under a click. The headline
// above it (render_headline in details_panel.cpp) grows when the click's
// record lands and pushes the tabs down, so a tab click whose mouse is still
// on its way hovers empty space. Holds once the click has settled, or while a
// closed ViewGate keeps it from landing.
void wait_tabs_placed(ImGuiTestContext* ctx);
// Click row `index` of the library view, wait for the song panel, point the
// ref at it, and land on its Paths tab. Waits for wait_tabs_placed first, so
// without a ViewGate the click's analysis has settled when it returns.
void open_details(ImGuiTestContext* ctx, size_t index);
// Type `search` into the library's search box, then open the row titled `title`.
void open_titled(ImGuiTestContext* ctx, const std::string& search, const std::string& title);
// Wait for the open song's analysis (the click, or a setting changed with the
// song open, starts it: D87 item 1, D90 item 1) and check it is Ready. Then
// lands the panel on its Paths tab.
void wait_song_analyzed(ImGuiTestContext* ctx);
// Fresh app, scan, open chart 0's Preview and wait for the load. False on error.
bool open_preview(ImGuiTestContext* ctx);

// ---- the Analysis settings panel (settings_bar.cpp) ------------------------
// The panel is a popup: a top-level window of its own, outside //Hydra, that
// takes focus when it opens. Its controls sit in tables, so refs to them
// start with **/.
// Open the panel from its button and leave the ref on it.
ImGuiWindow* open_settings_panel(ImGuiTestContext* ctx);
// The ID of the open panel's control `label` that sits in the same group as
// `sibling`, a checkbox ("##prodrums"). A **/ ref can't find a combo (the test engine files
// combos with an empty label), so a combo is found through a checkbox beside
// it: both share their group table's ID scope. (A number box won't do as the
// sibling: its parts sit in an ID scope of their own.)
ImGuiID settings_control(ImGuiTestContext* ctx, const char* sibling, const char* label);
// Pick `item` from the open panel's combo `combo` ("##difficulty"), found
// beside `sibling` (settings_control).
void settings_combo_pick(ImGuiTestContext* ctx, const char* sibling, const char* combo,
                         const char* item);
// Close the panel with Esc and point refs back at //Hydra. An open popup
// keeps every other window from being hovered, so a test closes it before
// clicking anywhere else.
void close_settings_panel(ImGuiTestContext* ctx);

}  // namespace uitest

#endif  // HYDRA_TESTS_UI_UITEST_HARNESS_H
