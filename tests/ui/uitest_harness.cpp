#include "uitest_harness.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <optional>
#include <stdexcept>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_te_context.h"
#include "imgui_te_internal.h"

#include "../scratch_settings.h"
#include "../wait_util.h"  // tests/ is not on the runner's include path

#include "app/analysis.h"
#include "app/config.h"
#include "app/user_messages.h"  // plain_error_block
#include "audio/device.h"
#include "core/winstr.h"
#include "net/dmbot_client.h"
#include "ui/app_state.h"
#include "ui/library_jobs.h"
#include "ui/library_model.h"
#include "ui/preview_controller.h"

namespace fs = std::filesystem;

namespace uitest {

namespace {

// Hydra's scratch files in the engine's temp_dir, and the app's path
// overrides (db, ini, preview assets).
void init_scratch(Harness& h) {
    h.db_path = h.temp_dir + "\\hydra.db";
    h.ini_path = h.temp_dir + "\\hydra_settings.ini";
    h.rules_path = h.temp_dir + "\\hydra_rules.ini";

    // The app's own path lookups now resolve to the scratch files; the real
    // app never sets these.
    hydra::app::PathOverrides po;
    po.db_path = h.db_path;
    po.ini_path = h.ini_path;
    po.rules_path = h.rules_path;
    po.asset_dir = HYDRA_ASSET_DIR;
    hydra::app::set_path_overrides(po);
}

// Whether the library shows every row: the search box is empty.
// EngineHooks::library_unfiltered, and the first half of library_row_title.
bool library_unfiltered(const Harness& h) { return h.app && h.app->search.empty(); }

// Whether a lookup of `label` would scroll the library: the library is
// unfiltered and `label` is a row's Title text (the row's Selectable in
// library_table.cpp). EngineHooks::is_library_row_title.
bool library_row_title(const Harness& h, const std::string& label) {
    if (!library_unfiltered(h)) return false;
    for (const hydra::ui::LibraryRow& row : h.app->library.rows())
        if (hydra::ui::library_cell_text(row, hydra::ui::kColumnTitle) == label) return true;
    return false;
}

}  // namespace

Harness::Harness() {
    hooks.draw_frame = [this] {
        if (app && !app_hidden) hydra::ui::run_frame(*app);
    };
    hooks.jobs_busy = [this] { return uitest::jobs_busy(*this); };
    hooks.state_text = [this] { return uitest::state_text(*this); };
    hooks.reset = [this] { reset_app(*this); };
    hooks.is_library_row_title = [this](const std::string& label) {
        return library_row_title(*this, label);
    };
    hooks.library_unfiltered = [this] { return library_unfiltered(*this); };
    hooks.open_song = [](ImGuiTestContext* ctx, const std::string& title, double settle_seconds) {
        return open_titled_reason(ctx, title, title, settle_seconds);
    };
    hooks.status_line = [this]() -> std::optional<std::string> {
        if (!app) return std::nullopt;
        return app->status_message;
    };
}

bool Harness::init_attached(ID3D11Device* dev, ID3D11DeviceContext* ctx, IDXGISwapChain* sc) {
    if (!Engine::init_attached(dev, ctx, sc)) return false;
    init_scratch(*this);
    return true;
}

bool selects(const std::string& what, const char* test_name) {
    return what == "all" || what == test_name;
}

std::string seed_problem(const std::string& flag, const std::string& path) {
    std::error_code ec;
    if (fs::is_regular_file(hydra::os_path(path), ec)) return "";
    return flag + " \"" + path + "\" is not a " + (flag == "--db" ? "database" : "settings") +
           " file";
}

std::string pipe_problem(bool live, const std::string& pipe_name) {
    if (live == !pipe_name.empty()) return "";
    return "--live needs --pipe <name>, and --pipe needs --live";
}

bool Harness::queue(const std::string& what) {
    if (fs::exists(fs::u8path(what)) && !fs::is_directory(fs::u8path(what))) {
        script_path = what;
        register_script_test(*this);
        ImGuiTestEngine_QueueTest(engine, ImGuiTestEngine_FindTestByName(engine, "hydra", "script"),
                                  ImGuiTestRunFlags_RunFromCommandLine);
        return true;
    }
    ImVector<ImGuiTest*> tests;
    ImGuiTestEngine_GetTestList(engine, &tests);
    int queued = 0;
    for (ImGuiTest* t : tests) {
        if (std::strcmp(t->Name, "script") == 0) continue;
        if (selects(what, t->Name)) {
            ImGuiTestEngine_QueueTest(engine, t, ImGuiTestRunFlags_RunFromCommandLine);
            ++queued;
        }
    }
    return queued > 0;
}

bool Harness::init() {
    hydra::ui::ImGuiSetupOptions opts;
    opts.dpi_scale = 1.0f;
    opts.ini_file = "-";  // no persisted ImGui layout between runs
    opts.resource_dir = HYDRA_RESOURCE_DIR;
    opts.before_ini_load = hydra::ui::install_placement_handler;
    if (!Engine::init(opts)) return false;
    init_scratch(*this);
    return true;
}

// Waits until the GPU has drawn every frame sent so far, so nothing it still
// holds keeps a texture alive past its owner's release.
static void wait_for_gpu(Harness& h) {
    D3D11_QUERY_DESC qd = {D3D11_QUERY_EVENT, 0};
    Microsoft::WRL::ComPtr<ID3D11Query> done;
    if (FAILED(h.device->CreateQuery(&qd, &done))) return;
    h.context->End(done.Get());
    BOOL finished = FALSE;
    try {
        testwait::wait_until(
            [&] { return h.context->GetData(done.Get(), &finished, sizeof(finished), 0) != S_FALSE; },
            "the GPU to finish its frames");
    } catch (const testwait::WaitTimeout& e) {
        // This runs on the test engine's thread, where a throw would end the
        // process with no message, so say why and stop here.
        std::fprintf(stderr, "hydra_uitest: %s\n", e.what());
        std::fflush(stderr);
        std::_Exit(1);
    }
}

void Harness::stop() {
    Engine::stop();
    if (app && app->preview) app->preview->close();
    app.reset();
}

void reset_app(Harness& h, const std::string& rules_text, bool wait_store,
               const std::string& settings_text) {
    // The engine runs this step inside ImGui's end-of-frame hook: the app has
    // drawn this frame and it hasn't rendered yet, so an Image the Preview drew
    // still points at its texture (the warning on PreviewController::close).
    // Let this frame render, then run one with no app drawn, before freeing.
    // The GPU wait makes a missed case fail every run instead of now and then.
    if (h.app && h.app->preview && !h.attached && h.engine && h.engine->TestContext) {
        h.app_hidden = true;
        h.engine->TestContext->Yield();
        h.app_hidden = false;
        wait_for_gpu(h);
    }
    if (h.app && h.app->preview) h.app->preview->close();
    h.app.reset();

    hydra::audio::set_headless(true);
    // The dmleaderboards API, canned: one user whose only score is the first
    // chart of the scanned library (so the join has something to match).
    hydra::net::set_fetcher([&h](const std::string& url, const std::atomic<bool>*) {
        if (url.find("/all-users") != std::string::npos) return canned_dm_users();
        std::string md5 = (h.app && h.app->library_shown_count() > 0)
                              ? h.app->library_row_at(0).entry.md5
                              : "00000000000000000000000000000000";
        return canned_dm_scores({canned_dm_score(md5, 100000)});
    });
    // No test downloads from the network. The update tests install their own
    // downloader over this one (uitest_library.cpp, UpdateSeams).
    hydra::net::set_downloader([](const std::string& url, const std::string&, uint64_t,
                                  std::atomic<uint64_t>*, const std::atomic<bool>*) {
        throw std::runtime_error("hydra_uitest has no canned download for " + url);
    });

    std::error_code ec;
    fs::remove(fs::u8path(h.db_path), ec);
    // --db and --ini: start from a copy of the given file (a database with
    // its WAL and SHM, if any), so the app never opens the original. main()
    // already refused a missing seed. A copy that fails (a locked file, say)
    // stops the run: the tests must not go on against an empty database or
    // the default settings.
    auto copy_seed = [&ec](const std::string& seed, const std::string& dest,
                           std::initializer_list<const char*> suffixes, const char* flag) {
        for (const char* suffix : suffixes) {
            fs::remove(fs::u8path(dest + suffix), ec);
            const fs::path from = fs::u8path(seed + suffix);
            if (!fs::exists(from, ec)) continue;
            if (!fs::copy_file(from, fs::u8path(dest + suffix),
                               fs::copy_options::overwrite_existing, ec)) {
                std::fprintf(stderr, "hydra_uitest: could not copy %s file \"%s\": %s\n", flag,
                             (seed + suffix).c_str(), ec.message().c_str());
                std::exit(1);
            }
        }
    };
    if (!h.seed_db.empty()) copy_seed(h.seed_db, h.db_path, {"", "-wal", "-shm"}, "--db");
    // The settings: the test's own bytes, else --ini's copy, else the GUI
    // tests' settings (tests/scratch_settings.h), written the way the app
    // writes its own ini.
    if (!settings_text.empty()) {
        std::ofstream f(fs::u8path(h.ini_path), std::ios::binary | std::ios::trunc);
        f << settings_text;
        if (!f) {
            std::fprintf(stderr, "hydra_uitest: could not write \"%s\"\n", h.ini_path.c_str());
            std::exit(1);
        }
    } else if (!h.seed_ini.empty()) {
        copy_seed(h.seed_ini, h.ini_path, {""}, "--ini");
    } else if (!scratch_settings().save_file(h.ini_path)) {
        std::fprintf(stderr, "hydra_uitest: could not write \"%s\"\n", h.ini_path.c_str());
        std::exit(1);
    }
    if (rules_text.empty()) {
        fs::remove(fs::u8path(h.rules_path), ec);
    } else {
        std::ofstream f(fs::u8path(h.rules_path), std::ios::trunc);
        f << rules_text;
    }
    h.frame_text.text.clear();
    // The split outlives an AppState (it is hydra_ui.ini's), so a test that
    // dragged it would hand its split to the next test.
    hydra::ui::remember_library_share(hydra::ui::kDefaultLibraryShare);
    hydra::ui::remember_library_hidden(false);

    h.app = std::make_unique<hydra::ui::AppState>();
    h.app->set_render_device(h.device.Get(), h.context.Get());
    if (!wait_store) return;
    // A deliberate block, in the harness only: the tests click at once.
    h.app->wait_store_open();
    if (!h.app->store_open_failed()) return;
    // The text the startup message box would show (D72 item 1).
    std::string why = "unknown error";
    try {
        std::rethrow_exception(h.app->store_open_error());
    } catch (const std::exception& e) {
        why = hydra::app::plain_error_block(e);
    } catch (...) {
    }
    IM_ERRORF("The scratch library did not open:\n%s", why.c_str());
}

bool jobs_busy(Harness& h) { return h.app->any_job_running(); }

namespace {
// Statics, not gate members: a batch job keeps its copy of the analyzer and
// can outlive the gate (until reset_app tears the app down), so the analyzer
// must never point into a gate that is gone.
std::atomic<int> g_gate_allowed{0};
std::atomic<int> g_gate_started{0};
}  // namespace

BatchGate::BatchGate(int workers)
    : seam_(hydra::ui::set_app_batch_analyzer_for_test,
            [](const std::string& path, const hydra::app::AnalysisSettings& settings,
               const std::function<void(float)>& on_progress) {
                const int n = ++g_gate_started;
                testwait::wait_until(
                    [&] {
                        if (n <= g_gate_allowed.load()) return true;
                        on_progress(0.0f);  // throws once Stop is pressed
                        return false;
                    },
                    "the test to let chart " + std::to_string(n) + " through the batch gate");
                return hydra::app::analyze_chart_file(path, settings, on_progress);
            },
            workers) {
    g_gate_allowed = 0;
    g_gate_started = 0;
}

// The gate opens here; seam_ removes the seam after.
BatchGate::~BatchGate() { g_gate_allowed = INT_MAX; }

void BatchGate::allow(int charts) { g_gate_allowed = charts; }

int BatchGate::started() const { return g_gate_started.load(); }

namespace {
// Statics, like the batch gate's: a click's job keeps its copy of the
// analyzer and can outlive the gate.
std::atomic<bool> g_view_gate_open{true};
std::atomic<int> g_view_gate_started{0};
}  // namespace

ViewGate::ViewGate()
    : seam_(hydra::ui::set_view_analyzer_for_test,
            [](const std::string& path, const hydra::app::AnalysisSettings& settings,
               const std::function<void(float)>& on_progress) {
                ++g_view_gate_started;
                testwait::wait_until(
                    [&] {
                        if (g_view_gate_open.load()) return true;
                        on_progress(0.0f);  // throws once Cancel or a setting change stops it
                        return false;
                    },
                    "the test to open the click's gate");
                return hydra::app::analyze_chart_file(path, settings, on_progress);
            }) {
    g_view_gate_open = false;
    g_view_gate_started = 0;
}

// The gate opens here; seam_ removes the seam after.
ViewGate::~ViewGate() { g_view_gate_open = true; }

void ViewGate::open() { g_view_gate_open = true; }

int ViewGate::started() const { return g_view_gate_started.load(); }

namespace {
// Statics, like the other gates': the open's job keeps its copy of the gate
// and can outlive the OpenGate.
std::atomic<bool> g_open_gate_open{true};
std::atomic<int> g_open_gate_started{0};
}  // namespace

OpenGate::OpenGate()
    : seam_(hydra::ui::set_store_open_gate_for_test,
            [](const hydra::ui::StoreOpenProgress& progress,
               const std::function<bool()>& cancelled) {
                // Hold at the conversion, where the upgrade's screen has all
                // its words.
                if (progress.step != hydra::ui::StoreOpenProgress::Step::ConvertingRecords)
                    return;
                if (g_open_gate_open.load()) return;
                ++g_open_gate_started;
                testwait::wait_until([&] { return g_open_gate_open.load() || cancelled(); },
                                     "the test to open the startup open's gate");
            }) {
    g_open_gate_open = false;
    g_open_gate_started = 0;
}

// The gate opens here; seam_ removes the seam after.
OpenGate::~OpenGate() { g_open_gate_open = true; }

void OpenGate::open() { g_open_gate_open = true; }

int OpenGate::started() const { return g_open_gate_started.load(); }

bool on_screen(Harness& h, const std::string& s) {
    return visible_text(h).find(s) != std::string::npos;
}

size_t count_on_screen(Harness& h, const std::string& s) {
    const std::string all = visible_text(h);
    size_t n = 0;
    for (size_t at = all.find(s); at != std::string::npos; at = all.find(s, at + 1)) ++n;
    return n;
}

std::string canned_dm_users() {
    return R"([{"id":"111","username":"alice","elo":1500,
                "stats":{"total_scores":1,"total_score":100000}}])";
}

std::string canned_dm_score(const std::string& identifier, int points) {
    return R"({"identifier":")" + identifier +
           R"(","song_name":"x","artist":"y","charter_refs":["z"],"score":)" +
           std::to_string(points) +
           R"(,"is_fc":0,"percent":95,"speed":100,"rank":1,"posted":"2026-01-01"})";
}

std::string canned_dm_scores(const std::vector<std::string>& score_rows) {
    std::string rows;
    for (const std::string& row : score_rows) rows += (rows.empty() ? "" : ",") + row;
    return R"({"scores":[)" + rows + R"(],"unknown_scores":[]})";
}

namespace {

const char* yes_no(bool b) { return b ? "yes" : "no"; }

}  // namespace

std::string state_text(Harness& h) {
    auto& a = *h.app;
    ImGuiTextBuffer out;
    out.appendf("state:\n");
    out.appendf("  store=%s\n", a.store_open_failed() ? "failed"
                                : a.store_ready()      ? "ready"
                                                       : "opening");
    out.appendf("  charts=%zu shown=%zu search=\"%s\"\n", a.library.rows().size(),
                a.library_shown_count(), a.search.c_str());
    for (size_t i = 0; i < a.library_shown_count() && i < 20; ++i) {
        const auto& r = a.library_row_at(i);
        // The status chip's word: "Analyzed", "Stale" or "Not analyzed".
        out.appendf("    row[%zu] \"%s\" - %s (%s) md5=%s status=%s\n", i, r.title.c_str(),
                    r.artist.c_str(), r.charter.c_str(), r.entry.md5.c_str(),
                    hydra::ui::status_label(r.status));
    }
    out.appendf("  selected=%s panel_open=%s viewed_record=%s paths=%zu\n",
                a.selected ? a.selected->title.c_str() : "(none)", yes_no(a.details_open()),
                yes_no(a.viewed.record.has_value()),
                a.viewed.record ? a.viewed.record->paths.size() : 0);
    if (a.viewed.record && !a.viewed.record->paths.empty())
        out.appendf("  best_path=%s\n", a.viewed.record->best_path().pathstring().c_str());
    out.appendf("  chartmode=\"%s\" prodrums=%s bass2x=%s depth=%d auto_open_report=%s\n",
                a.settings.chartmode_key().c_str(), yes_no(a.settings.view_prodrums),
                yes_no(a.settings.view_bass2x), a.settings.depth_value,
                yes_no(a.settings.auto_open_report));
    static const char* const kViewStates[] = {"none",      "analyzing",    "ready",
                                               "cancelled", "failed",       "file-missing",
                                               "rules-broken"};
    out.appendf("  viewed=%s view_job=%s view_pending=%s\n",
                kViewStates[static_cast<int>(a.viewed.state)],
                a.view_job ? (a.view_job->finished() ? "finished" : "running") : "-",
                yes_no(a.view_pending));
    out.appendf("  jobs: scan=%s batch=%s report=%s dm_fetch=%s dm_report=%s\n",
                a.scan_job ? (a.scan_job->snapshot().finished ? "finished" : "running") : "-",
                a.batch_job ? (a.batch_job->snapshot().finished ? "finished" : "running") : "-",
                a.report_job ? (a.report_job->finished() ? "finished" : "running") : "-",
                a.dm_fetch_job ? (a.dm_fetch_job->finished() ? "finished" : "running") : "-",
                a.dm_report_job ? (a.dm_report_job->finished() ? "finished" : "running") : "-");
    if (a.preview)
        out.appendf("  preview: active=%s loading=%s playing=%s error=\"%s\"\n",
                    yes_no(a.preview->active()), yes_no(a.preview->loading()),
                    yes_no(a.preview->playing()), a.preview->error().c_str());
    out.appendf("  status=\"%s\" dm_users=%zu\n", a.status_message.c_str(), a.dm_users.size());
    return out.c_str();
}

float drawn_font_size() {
    const ImGuiStyle& st = ImGui::GetStyle();
    return st.FontSizeBase * st.FontScaleMain * st.FontScaleDpi;
}

hydra::ui::WidthOf text_measurer(ImFont* font) {
    const float size = drawn_font_size();
    if (!font) font = ImGui::GetIO().FontDefault ? ImGui::GetIO().FontDefault : ImGui::GetFont();
    return hydra::ui::measure_in_font(font, size);
}

float text_width(const char* s, ImFont* font) { return text_measurer(font)(s); }

float rule_width(const char* header, const std::vector<std::string>& cells, ImFont* font) {
    const std::vector<hydra::ui::ColumnSpec> specs = {{header, false, 0.0f, text_measurer(font)}};
    const hydra::ui::MeasuredWidths measured = hydra::ui::measure_widths(
        specs, cells.size(), [&](size_t r, size_t) { return cells[r]; });
    return hydra::ui::place_columns(measured, specs, hydra::ui::TableRoom{}).widths[0];
}

// Scan testdata/input through the UI and land on the populated library.
void scan_library(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    ctx->SetRef("//Hydra");
    ctx->ItemClick("Scan library");
    IM_CHECK(wait_until(ctx, [&] { return h.app->scan_job && h.app->scan_job->snapshot().finished; }, 60));
    ctx->SetRef("//Scanning charts");
    // State, not the modal's wording: T13 rewrites "chart(s) found" with
    // real plurals in this same wave.
    IM_CHECK(h.app->scan_job->snapshot().charts_found > 0);
    ctx->ItemClick("Continue");
    ctx->Yield(2);
    ctx->SetRef("//Hydra");
    IM_CHECK(h.app->scan_job == nullptr);
    IM_CHECK(!h.app->library.rows().empty());
    IM_CHECK(h.app->library_shown_count() > 0);
}

// Point the ref at the song panel. It is a child window of the main window,
// and child window names are mangled, so go through WindowInfo.
void set_panel_ref(ImGuiTestContext* ctx) {
    ImGuiWindow* panel = ctx->WindowInfo("//Hydra/##songpanel").Window;
    IM_CHECK(panel != nullptr);
    ctx->SetRef(panel);
}

// Click row `index` of the library view and wait for the song panel.
void open_details(ImGuiTestContext* ctx, size_t index) {
    IM_CHECK(open_details_within(ctx, index, kSettleWaitSeconds));
}

namespace {
// Whether the tab bar can no longer move within `seconds`; the one wait
// wait_tabs_placed and open_details_within share.
bool tabs_placed_within(ImGuiTestContext* ctx, double seconds) {
    Harness& h = harness(ctx);
    return wait_until(ctx, [&] { return h.app->view_settled() || !g_view_gate_open.load(); },
                      seconds);
}
}  // namespace

bool open_details_within(ImGuiTestContext* ctx, size_t index, double settle_seconds) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(index < h.app->library_shown_count(), true);
    const std::string title = h.app->library_row_at(index).title;
    ctx->SetRef("//Hydra");
    ctx->ItemClick(("**/" + escape_ref(title)).c_str());
    ctx->Yield(3);
    IM_CHECK_RETV(h.app->details_open(), true);
    IM_CHECK_RETV(h.app->selected && h.app->selected->title == title, true);
    set_panel_ref(ctx);
    if (ctx->IsError()) return true;
    // The ImGui context outlives reset_app, so the tab bar remembers the tab a
    // previous test left selected. Land on Paths deterministically.
    if (!tabs_placed_within(ctx, settle_seconds)) return false;
    ctx->ItemClick("##DetailsTabs/Paths");
    return true;
}

void wait_tabs_placed(ImGuiTestContext* ctx) {
    IM_CHECK(tabs_placed_within(ctx, kSettleWaitSeconds));
}

// Narrow the library with `search` typed into the search box, then open the
// row titled `title`.
std::string open_titled_reason(ImGuiTestContext* ctx, const std::string& search,
                               const std::string& title, double settle_seconds) {
    Harness& h = harness(ctx);
    ctx->SetRef("//Hydra");
    ctx->ItemInputValue("**/##search", search.c_str());
    if (ctx->IsError()) return "";  // the engine's own failure is in the log
    size_t idx = 0;
    auto find_row = [&] {
        for (size_t i = 0; i < h.app->library_shown_count(); ++i)
            if (h.app->library_row_at(i).title == title) { idx = i; return true; }
        return false;
    };
    if (!wait_until(ctx, find_row, 5)) {
        if (ctx->IsError()) return "";
        const size_t shown = h.app->library_shown_count();
        std::string why = "no row titled \"" + title + "\" after searching \"" + search +
                          "\": the search shows " + std::to_string(shown) +
                          (shown == 1 ? " row" : " rows");
        const size_t kNamed = 5;
        for (size_t i = 0; i < shown && i < kNamed; ++i)
            why += (i == 0 ? ", titled \"" : "\", \"") + h.app->library_row_at(i).title;
        if (shown > 0) why += shown > kNamed ? "\" and more" : "\"";
        return why;
    }
    if (!open_details_within(ctx, idx, settle_seconds) && !ctx->IsError()) {
        char secs[32];
        std::snprintf(secs, sizeof(secs), "%g", settle_seconds);
        return "opened \"" + title + "\", but its analysis was still running after " + secs +
               " s; use wait-idle";
    }
    return "";
}

void open_titled(ImGuiTestContext* ctx, const std::string& search, const std::string& title) {
    const std::string why = open_titled_reason(ctx, search, title);
    if (!why.empty()) IM_ERRORF("%s", why.c_str());
}

// Wait for the open song's analysis and a Ready record.
void wait_song_analyzed(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    set_panel_ref(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(wait_until(ctx, [&] { return h.app->view_settled(); }, kSettleWaitSeconds));
    IM_CHECK(h.app->viewed.ready());
    // Only now: the headline has landed, so the tab bar stays put.
    ctx->ItemClick("##DetailsTabs/Paths");
}

// Shared: open chart 0's Preview and wait for the load. Returns false on error.
bool open_preview(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return false;
    open_details(ctx, 0);
    if (ctx->IsError()) return false;
    ctx->ItemClick("**/Preview");
    IM_CHECK_RETV(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active(); }, 10), false);
    IM_CHECK_RETV(wait_until(ctx, [&] { return !h.app->preview->loading(); }, 120), false);
    IM_CHECK_RETV(h.app->preview->error().empty(), false);
    return true;
}

ImGuiWindow* open_settings_panel(ImGuiTestContext* ctx) {
    ctx->SetRef("//Hydra");
    ctx->ItemClick("###analysissettings");
    ctx->Yield();
    ImGuiWindow* panel = ctx->GetWindowByRef("//$FOCUSED");
    IM_CHECK_RETV(panel != nullptr && (panel->Flags & ImGuiWindowFlags_Popup) != 0, nullptr);
    ctx->SetRef(panel);
    return panel;
}

ImGuiID settings_control(ImGuiTestContext* ctx, const char* sibling, const char* label) {
    const ImGuiID parent = ctx->ItemInfo((std::string("**/") + sibling).c_str()).ParentID;
    return ctx->GetID(label, parent);
}

void settings_combo_pick(ImGuiTestContext* ctx, const char* sibling, const char* combo,
                         const char* item) {
    ctx->ItemClick(settings_control(ctx, sibling, combo));
    ImGuiWindow* list = ctx->GetWindowByRef("//$FOCUSED");
    IM_CHECK(list != nullptr);
    if (list == nullptr) return;
    ctx->ItemClick((std::string("//") + list->Name + "/**/" + item).c_str());
}

void close_settings_panel(ImGuiTestContext* ctx) {
    ctx->KeyPress(ImGuiKey_Escape);
    IM_CHECK(wait_until(ctx, [] {
        return !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    }, 5));
    ctx->SetRef("//Hydra");
}

}  // namespace uitest
