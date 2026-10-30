#include "pbtest_harness.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#include "imgui.h"
#include "imgui_te_context.h"
#include "imgui_te_internal.h"

#include "../scratch_settings.h"

#include "app/config.h"
#include "audio/device.h"
#include "ui/preview_controller.h"

namespace fs = std::filesystem;

namespace pbtest {

Harness::Harness() {
    home_ref = std::string("//") + hydra::pathbuilder::kMainWindowName;
    hooks.draw_frame = [this] {
        if (app && !app_hidden) app->draw();
    };
    hooks.jobs_busy = [this] { return app && app->busy(); };
    hooks.state_text = [this] {
        std::string t;
        if (!app) return t;
        const hydra::pathbuilder::PbState& s = app->state;
        t += "chart: " + (s.chart() ? s.chart()->chart_path : std::string("(none)")) + "\n";
        t += "rows: " + std::to_string(app->rows().size()) + "\n";
        if (!s.error().empty()) t += "error: " + s.error() + "\n";
        return t;
    };
    hooks.reset = [this] { reset_app(*this); };
    // The builder has no library, so no label is a library row's title.
    hooks.is_library_row_title = [](const std::string&) { return false; };
}

bool Harness::init() {
    hydra::ui::ImGuiSetupOptions opts;
    opts.dpi_scale = 1.0f;
    opts.ini_file = "-";  // no persisted ImGui layout between runs
    opts.resource_dir = HYDRA_RESOURCE_DIR;
    if (!Engine::init(opts)) return false;
    ini_path = temp_dir + "\\hydra_settings.ini";
    hydra::app::PathOverrides po;
    po.ini_path = ini_path;
    po.db_path = temp_dir + "\\hydra.db";
    po.rules_path = temp_dir + "\\hydra_rules.ini";
    po.asset_dir = HYDRA_ASSET_DIR;
    hydra::app::set_path_overrides(po);
    return true;
}

void Harness::stop() {
    Engine::stop();
    if (app) app->close_preview();
    app.reset();
}

bool Harness::queue(const std::string& what) {
    if (fs::exists(fs::u8path(what)) && !fs::is_directory(fs::u8path(what))) {
        script_path = what;
        uitest::register_script_test(*this);
        ImGuiTestEngine_QueueTest(engine, ImGuiTestEngine_FindTestByName(engine, "hydra", "script"),
                                  ImGuiTestRunFlags_RunFromCommandLine);
        return true;
    }
    ImGuiTest* t = ImGuiTestEngine_FindTestByName(engine, "hydra", what.c_str());
    if (!t || std::strcmp(t->Name, "script") == 0) return false;
    ImGuiTestEngine_QueueTest(engine, t, ImGuiTestRunFlags_RunFromCommandLine);
    return true;
}

void reset_app(Harness& h) {
    // As Hydra's runner does (uitest_harness.cpp, reset_app): the frame this
    // runs in still points at the highway's texture, so let it render with
    // nothing drawn before the texture goes.
    if (h.app && h.engine && h.engine->TestContext) {
        h.app_hidden = true;
        h.engine->TestContext->Yield();
        h.app_hidden = false;
        h.context->Flush();
    }
    if (h.app) h.app->close_preview();
    h.app.reset();

    hydra::audio::set_headless(true);
    if (!scratch_settings().save_file(h.ini_path)) {
        std::fprintf(stderr, "hydra_pathbuilder_uitest: could not write \"%s\"\n",
                     h.ini_path.c_str());
        std::exit(1);
    }
    h.app = std::make_unique<hydra::pathbuilder::PbApp>(h.device.Get(), h.context.Get());
    if (!h.chart_arg.empty()) h.app->open(h.chart_arg);
}

bool wait_idle(ImGuiTestContext* ctx, double seconds) {
    Harness& h = harness(ctx);
    // A frame first, so a load the last frame asked for has started.
    ctx->Yield(2);
    return uitest::wait_until(ctx, [&] { return !h.app || !h.app->busy(); }, seconds);
}

bool click_fill(ImGuiTestContext* ctx, int64_t act_tick) {
    hydra::pathbuilder::PbState& s = harness(ctx).app->state;
    s.decisions().click_fill(s.search_graph(), s.chart()->settings, s.history().current().windows,
                             act_tick, s.history().generation());
    return wait_for_edit(ctx);
}

bool wait_for_edit(ImGuiTestContext* ctx) {
    if (!wait_idle(ctx)) return false;
    ctx->Yield(2);
    // The edit's frame asked for the new path's ladder and overlay.
    if (!wait_idle(ctx)) return false;
    ctx->Yield(2);
    return true;
}

}  // namespace pbtest
