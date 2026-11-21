// The Path Builder's Copy and Paste, driven headless through the Edit menu.
// The test engine keeps ImGui's clipboard to itself while a test runs, so the
// real Windows clipboard is never touched.

// First: its Windows headers come before the test engine's, which undefine
// their Yield() macro (uitest_engine.h says why).
#include "pbtest_harness.h"

#include <cstdint>
#include <string>

#include "imgui.h"
#include "imgui_te_context.h"

#include "../testdata_dirs.h"

#include "app/user_messages.h"  // plain_error
#include "core/error_kind.h"    // KindedError
#include "pathbuilder/pb_files.h"

namespace pbtest {

namespace {

// Pastes Hydra's best path as the player's, copies it back, then pastes text that
// names no path on the chart: the refusal shows its sentence and the reason,
// and the player's path stays as it was.
void test_paste_refused_keeps_path(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = corpus::burnout_notes();
    reset_app(h);
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    const hydra::pathbuilder::OpenChart* chart = h.app->state.chart();
    IM_CHECK(chart != nullptr);
    if (!chart) return;
    const hydra::Path& best = chart->record.best_path();

    const std::string best_text = hydra::pathbuilder::copy_text(chart->record, best);
    ImGui::SetClipboardText(best_text.c_str());
    ctx->SetRef(h.home_ref.c_str());
    ctx->MenuClick("Edit/Paste");
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    const hydra::pathbuilder::PathHistory& history = h.app->state.history();
    IM_CHECK(history.current().path.has_value());
    IM_CHECK(h.app->refusal().empty());
    if (!history.current().path) return;
    IM_CHECK_STR_EQ(history.current().path->pathstring().c_str(), best.pathstring().c_str());
    const uint64_t generation = history.generation();

    // Copy writes the pasted path back as the same text.
    ImGui::SetClipboardText("");
    ctx->MenuClick("Edit/Copy");
    ctx->Yield(2);
    IM_CHECK_STR_EQ(ImGui::GetClipboardText(), best_text.c_str());

    ImGui::SetClipboardText("99");
    ctx->MenuClick("Edit/Paste");
    ctx->Yield(2);
    // The refusal is the sentence for a path text that does not fit, and its
    // detail names the pasted window.
    const hydra::KindedError refused(hydra::ErrorKind::PathTextRefused, "");
    IM_CHECK_STR_EQ(h.app->refusal().c_str(), hydra::app::plain_error(refused).c_str());
    IM_CHECK(h.app->refusal_detail().find("\"99\"") != std::string::npos);
    IM_CHECK(on_screen(h, h.app->refusal()));
    IM_CHECK(on_screen(h, h.app->refusal_detail()));
    IM_CHECK(history.generation() == generation);
    IM_CHECK(history.current().path.has_value());
    if (!history.current().path) return;
    IM_CHECK_STR_EQ(history.current().path->pathstring().c_str(), best.pathstring().c_str());
}

}  // namespace

void register_file_tests(Harness& h) {
    uitest::register_test(h, "paste-refused-keeps-path", test_paste_refused_keeps_path);
}

}  // namespace pbtest
