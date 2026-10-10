// The checked-in GUI tests. Each one starts from a fresh scratch app
// (reset_app), drives the real UI by widget label, and checks both the app's
// state and what is on screen. See docs/agents/ui-testing.md for the label
// cheat-sheet and how to add one.

#include <vector>

#include "uitest_harness.h"

namespace uitest {

void register_paths_tests(Harness& h);  // uitest_paths.cpp
void register_report_window_tests(Harness& h);  // uitest_report_windows.cpp
const std::vector<TestEntry>& report_window_flow_tests();  // uitest_report_windows.cpp

void register_tests(Harness& h) {
    // Order does not matter: every run of more than one test gives each test
    // its own process (uitest_main.cpp), so no test sees another's state.
    auto register_entries = [&h](const std::vector<TestEntry>& entries) {
        for (const TestEntry& e : entries) {
            ImGuiTest* t = IM_REGISTER_TEST(h.engine, "hydra", e.name);
            t->UserData = &h;
            t->TestFunc = e.fn;
        }
    };
    for (const std::vector<TestEntry>* area :
         {&library_tests(), &details_tests(), &preview_tests(), &batch_report_tests()})
        register_entries(*area);
    register_paths_tests(h);
    register_report_window_tests(h);
    // The report windows end to end: the app draws them from run_frame, so
    // these have no GUI function of their own.
    register_entries(report_window_flow_tests());
}

}  // namespace uitest
