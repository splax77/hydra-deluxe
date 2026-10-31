// hydra_uitest — Hydra's half of the headless GUI test runner
// (docs/agents/ui-testing.md).
//
// The engine (uitest_engine.h) runs a UI with no window and reads it back;
// this file hands it Hydra: the AppState it draws, the hooks the command
// verbs reach Hydra through, the seams, the gates and the helpers every
// test calls. Tests find widgets by label ("Scan charts"), click them, and
// read back the app's state and the text on screen. Everything runs on
// scratch files in a temp folder; no browser, sound card, or network is
// touched.

#ifndef HYDRA_TESTS_UI_UITEST_HARNESS_H
#define HYDRA_TESTS_UI_UITEST_HARNESS_H

#include <chrono>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// The engine first: its Windows headers come before the test engine's.
#include "uitest_engine.h"

#include "../scoped_hook.h"
#include "ui/app_shell.h"
#include "ui/app_state.h"
#include "ui/column_widths.h"


namespace uitest {

struct Harness : Engine {
    // Installs the engine's hooks (EngineHooks) on this harness's AppState.
    Harness();

    // Hydra's scratch files, in the engine's temp_dir beside the HTML
    // reports the jobs write next to the DB.
    std::string db_path;
    std::string ini_path;
    std::string rules_path;
    // --db: each test starts from a copy of this database instead of an
    // empty one. The file itself is only read, never opened by the app.
    std::string seed_db;
    // --ini: each test starts from a copy of this settings file instead of
    // scratch_settings(). Copied the same way as seed_db.
    std::string seed_ini;

    std::unique_ptr<hydra::ui::AppState> app;
    // Set by reset_app for the one frame that runs with the old app undrawn,
    // so nothing it drew is still waiting to render when it is freed.
    bool app_hidden = false;

    // The live test's pipe, exactly as --pipe gave it. The live channel opens
    // this name and never makes one up.
    std::string pipe_name;
    // The file the process's output goes to: Hydra.exe's attached-mode log.
    // Empty means stdout, as in hydra_uitest. The live hello line reports it.
    std::string log_path;
    // Where a crash's minidump is written, set by whoever installs the crash
    // handler that writes it (hydra_uitest's main). Empty when nothing
    // writes one. The live hello line reports it.
    std::string crash_dump;

    // Headless: Engine::init with Hydra's ImGui options, then Hydra's
    // scratch paths.
    bool init();

    // Attached (Hydra.exe --uitest): Engine::init_attached, then Hydra's
    // scratch paths. main.cpp drives the frames itself and calls
    // ImGuiTestEngine_PostSwap after Present.
    bool init_attached(ID3D11Device* dev, ID3D11DeviceContext* ctx, IDXGISwapChain* swapchain);
    // Queue a test by name or, when `what` names a file, as a script.
    bool queue(const std::string& what);
    // Stop the engine and drop the app (Engine::stop).
    void stop() override;
};

// Whether a test name given on the command line picks the test called
// `test_name`: "all" picks every test, any other name only the test with
// exactly that name.
bool selects(const std::string& what, const char* test_name);

// Why a seed the command line gave (`flag` is "--db" or "--ini") cannot be
// used: it is not an existing file. Empty when it can. A mistyped seed would
// otherwise start every test on an empty library or the default settings.
// The one place that decides; hydra_uitest's main and Hydra.exe --uitest/--live
// both ask it.
std::string seed_problem(const std::string& flag, const std::string& path);

// Why the --live and --pipe flags the command line gave cannot be used
// together: --live needs a pipe name and a pipe name needs --live. Empty when
// they can. The client picks the name; Hydra never makes one up. The one place
// that decides; hydra_uitest's main and Hydra.exe both ask it.
std::string pipe_problem(bool live, const std::string& pipe_name);

// Fresh app state for a test: rewrite the scratch INI from
// scratch_settings() (tests/scratch_settings.h) or copy --ini's seed over it,
// delete the DB or copy --db's seed over it, rebuild AppState on those
// paths, and (re)install the headless seams. Call at the start of TestFunc —
// the GUI thread is parked between frames while TestFunc runs, so swapping
// the AppState here is safe.
// rules_text: the scratch hydra_rules.ini's contents. Empty (the default)
// means no rules file, so every other test runs today's rules.
// wait_store: the app opens its store on a worker thread. True (the default)
// blocks in AppState::wait_store_open until the store is ready, so the test
// can click at once, and fails the test if the open failed. Only a test that
// looks at the startup screen passes false.
// settings_text: the scratch hydra_settings.ini's exact bytes. Empty (the
// default) means the INI described above.
void reset_app(Harness& h, const std::string& rules_text = "", bool wait_store = true,
               const std::string& settings_text = "");

// Register every C++ test (the uitest_<area>.cpp files, in the order
// uitest_tests.cpp fixes) through register_test (uitest_engine.h). Each
// test's UserData is h's Engine.
void register_tests(Harness& h);
// Register the "live" test (uitest_live.cpp), which --live queues: it takes
// command lines from a named pipe. What ends it is run_live's loop
// (uitest_live.cpp).
void register_live_test(Harness& h);

// ---- helpers shared by the tests ------------------------------------------

// The Harness a test runs under (Hydra's tests are all registered on one).
inline Harness& harness(ImGuiTestContext* ctx) {
    return static_cast<Harness&>(engine_of(ctx));
}

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

private:
    ScopedHook seam_;
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

private:
    ScopedHook seam_;
};

// Holds the app's store open on its worker thread until the test lets it
// through. A scratch library opens in a blink, so a test that looks at the
// startup screen would race it. While a gate lives, the open's progress waits
// at the gate once it reaches the conversion to records, until open(). started()
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

private:
    ScopedHook seam_;
};

// True when `s` is somewhere in visible_text(h) (uitest_engine.h).
bool on_screen(Harness& h, const std::string& s);

// How many times `s` appears in visible_text(h).
size_t count_on_screen(Harness& h, const std::string& s);

// The dmleaderboards API's canned answers, which reset_app serves and the
// tests that serve their own variant build from: the one user (alice), one
// score row, and the scores reply that wraps a list of rows.
std::string canned_dm_users();
std::string canned_dm_score(const std::string& identifier, int points);
std::string canned_dm_scores(const std::vector<std::string>& score_rows);

// The key AppState fields, one per line, each ending in a newline. The
// state verb's text (EngineHooks::state_text).
std::string state_text(Harness& h);

// How wide ImGui draws `s` in `font` (the current font when null), at the
// size it draws text now: the style's base size times its main and DPI
// scales. The one place a GUI test measures text.
float text_width(const char* s, ImFont* font = nullptr);

// The pixel size ImGui draws text at now. text_width and text_measurer use it.
float drawn_font_size();

// text_width's measuring, as the width rule's WidthOf (ui/column_widths.h),
// for a test that checks a column against the rule.
hydra::ui::WidthOf text_measurer(ImFont* font = nullptr);

// The width the production rule gives a one-column table that never cuts, for
// `cells` under `header`, measured in `font` as text_measurer does. What a
// test expects of a Dynamics, Stars or backend table column.
float rule_width(const char* header, const std::vector<std::string>& cells,
                 ImFont* font = nullptr);

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

// Opens what the Paths tab starts with folded (defined in uitest_paths.cpp),
// for tests outside that file that check the whole tab.
void open_paths_folds(ImGuiTestContext* ctx);

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
// The longest the open helpers wait for the tabs to settle, in seconds.
constexpr double kSettleWaitSeconds = 300;
// Click row `index` of the library view, wait for the song panel, point the
// ref at it, and land on its Paths tab. Waits for wait_tabs_placed first, so
// without a ViewGate the click's analysis has settled when it returns.
void open_details(ImGuiTestContext* ctx, size_t index);
// open_details with its own wait cap. False when the tabs had not settled
// within `settle_seconds` (the song is open, the Paths tab is not clicked, and
// no failure is recorded); true otherwise, also after an engine action failed.
bool open_details_within(ImGuiTestContext* ctx, size_t index, double settle_seconds);
// Type `search` into the library's search box, then open the row titled `title`.
void open_titled(ImGuiTestContext* ctx, const std::string& search, const std::string& title);
// open_titled's one sequence, which fails no test itself when no shown row has
// that exact title or the tabs do not settle within `settle_seconds`: it
// returns the plain reason (the search, how many rows it shows, a few of their
// titles; or that the analysis is still running). Empty when the row opened,
// or when an engine action failed instead (ctx->IsError(), its reason in the log).
std::string open_titled_reason(ImGuiTestContext* ctx, const std::string& search,
                               const std::string& title,
                               double settle_seconds = kSettleWaitSeconds);
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
