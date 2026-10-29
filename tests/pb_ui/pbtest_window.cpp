// The Path Builder's main window, driven headless: a chart opened from the
// command line, a chart with no Star Power, and a chart that fails to open.

// First: its Windows headers come before the test engine's, which undefine
// their Yield() macro (uitest_engine.h says why).
#include "pbtest_harness.h"

#include <cstdio>
#include <string>

#include "imgui_te_context.h"

#include "../chart_text.h"
#include "../testdata_dirs.h"

#include "core/winstr.h"
#include "pathbuilder/pb_paths.h"

namespace pbtest {

namespace {

// Writes `text` to `path`, replacing the file. False when it could not.
bool write_file(const std::string& path, const std::string& text) {
    std::FILE* f = hydra::fopen_utf8(path, L"wb");
    if (!f) return false;
    std::fputs(text.c_str(), f);
    std::fclose(f);
    return true;
}

// The chart test_pb_state.cpp opens too.
std::string burnout() {
    return corpus::root() + "/common/Summer Blast _25 Setlist/Tier 4/Green Day - Burnout/notes.mid";
}

bool on_screen(Harness& h, const std::string& part) {
    return uitest::visible_text(h).find(part) != std::string::npos;
}

// Opens the chart from the command line and reads the left list: every row's
// label, total and gap is on screen.
void test_open_and_list(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = burnout();
    reset_app(h);
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    IM_CHECK(h.app->state.chart() != nullptr);
    const auto& rows = h.app->rows();
    IM_CHECK(!rows.empty());
    if (rows.empty()) return;
    for (const hydra::pathbuilder::PathRow& r : rows) {
        IM_CHECK(on_screen(h, r.label));
        IM_CHECK(on_screen(h, hydra::pathbuilder::total_text(r) + "  (" +
                                  hydra::pathbuilder::gap_text(r) + ")"));
    }
    // Burnout's best total under the GUI tests' settings, from one run.
    IM_CHECK(on_screen(h, "378,315  (0)"));
    // Picking a row and pressing "Edit a copy" makes it the player's path.
    ctx->SetRef(h.home_ref.c_str());
    ctx->ItemClick("**/Best path");
    ctx->ItemClick("**/Edit a copy");
    ctx->Yield(2);
    IM_CHECK(h.app->state.history().current().path.has_value());
    IM_CHECK(h.app->state.history().can_undo());
}

// A chart with notes and no Star Power phrase opens and says there is
// nothing to build.
void test_no_star_power(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    using namespace testchart;
    const std::string notes = h.temp_dir + "\\notes.chart";
    IM_CHECK(write_file(notes, chart_text(section("ExpertDrums", line(0, "N 1 0") +
                                                                     line(192, "N 2 0") +
                                                                     line(384, "N 1 0")))));
    h.chart_arg = notes;
    reset_app(h);
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    IM_CHECK(h.app->state.chart() != nullptr);
    IM_CHECK(on_screen(h, hydra::pathbuilder::kNothingToBuildText));
}

// A chart that cannot be read shows the plain sentence for it and its details
// line, and opens nothing. The sentence is the builder's (D267): Hydra's
// without the Scan library advice, since the builder has no library.
void test_file_open_error(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    h.chart_arg = h.temp_dir + "\\missing\\notes.mid";
    reset_app(h);
    IM_CHECK(wait_idle(ctx));
    ctx->Yield(2);
    IM_CHECK(h.app->state.chart() == nullptr);
    IM_CHECK(h.app->state.error() ==
             "Hydra couldn't open the song file. It may have been moved or deleted.");
    IM_CHECK(h.app->state.error_detail() == "cannot open file: " + h.chart_arg);
    IM_CHECK(on_screen(h, h.app->state.error()));
    IM_CHECK(on_screen(h, h.app->state.error_detail()));
}

}  // namespace

void register_tests(Harness& h) {
    uitest::register_test(h, "open-and-list", test_open_and_list);
    uitest::register_test(h, "no-star-power", test_no_star_power);
    uitest::register_test(h, "file-open-error", test_file_open_error);
}

}  // namespace pbtest
