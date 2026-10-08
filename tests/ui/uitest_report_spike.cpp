// The viewport spike's GUI test (report windows plan, task 1): the empty
// "Report spike" window opens by its name and closes from its own close
// button. Under hydra_uitest there is no platform backend, so ImGui turns
// viewports off and the window draws inside the app. Task 5 replaces the
// spike window, and this test, with the real report windows.

#include "uitest_harness.h"

#include "imgui_internal.h"
#include "ui/app_shell.h"

namespace uitest {

namespace {

// True when the window called `name` was drawn last frame.
bool window_shown(const char* name) {
    const ImGuiWindow* w = ImGui::FindWindowByName(name);
    return w != nullptr && w->WasActive;
}

void test_report_spike(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    ctx->Yield(2);
    IM_CHECK(!window_shown("Report spike"));

    hydra::ui::show_report_spike(true);
    ctx->Yield(3);
    IM_CHECK(window_shown("Report spike"));

    ctx->WindowClose("//Report spike");
    ctx->Yield(3);
    IM_CHECK(!window_shown("Report spike"));
    IM_CHECK(!hydra::ui::report_spike_shown());
}

}  // namespace

void register_report_spike_tests(Harness& h) {
    ImGuiTest* t = IM_REGISTER_TEST(h.engine, "hydra", "report-spike");
    t->UserData = &h;
    t->TestFunc = test_report_spike;
}

}  // namespace uitest
