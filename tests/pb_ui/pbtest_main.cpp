// hydra_pathbuilder_uitest: run the Path Builder's GUI tests headlessly.
// The flags mean what they mean to hydra_uitest (tests/ui/uitest_main.cpp),
// without --all, --live, --db, --ini and --jobs:
//
//   hydra_pathbuilder_uitest --test <name>     run one; repeatable
//   hydra_pathbuilder_uitest --script <file>   run a command file
//   hydra_pathbuilder_uitest --list            list the tests
//   options: --keep-temp  --shots <dir>
//
// More than one --test runs them one after another in this process; each
// starts from a fresh builder (reset_app).
//
// Prints [PASS]/[FAIL] per test and exits 0 only if everything passed.

#include <cstdio>
#include <string>
#include <vector>

#include "app/user_messages.h"
#include "core/winstr.h"
#include "pbtest_harness.h"

namespace {

int usage() {
    std::fprintf(stderr, "usage: hydra_pathbuilder_uitest (--test <name> | --script <file> | --list)"
                         " [--keep-temp] [--shots <dir>]\n");
    return 2;
}

}  // namespace

int main() {
    hydra::app::set_program(hydra::app::Program::PathBuilder);
    bool list = false;
    std::vector<std::string> wanted;
    pbtest::Harness h;

    uitest::ArgReader args(hydra::utf8_argv());
    while (args.next()) {
        const std::string& a = args.flag();
        std::string v;
        if (a == "--list") list = true;
        else if (a == "--keep-temp") h.keep_temp = true;
        else if (a == "--test" || a == "--script") { if (!args.value(v)) return usage(); wanted.push_back(v); }
        else if (a == "--shots") { if (!args.value(h.shots_dir)) return usage(); }
        else return usage();
    }
    if (!list && wanted.empty()) return usage();

    if (!h.init()) return 1;
    pbtest::register_tests(h);
    pbtest::register_coach_tests(h);

    if (list) {
        ImVector<ImGuiTest*> tests;
        ImGuiTestEngine_GetTestList(h.engine, &tests);
        for (ImGuiTest* t : tests) std::printf("%s\n", t->Name);
        h.shutdown();
        return 0;
    }

    ImGuiTestEngine_Start(h.engine, ImGui::GetCurrentContext());
    for (const std::string& w : wanted) {
        if (!h.queue(w)) {
            std::fprintf(stderr, "hydra_pathbuilder_uitest: no test or script file \"%s\" (try --list)\n",
                         w.c_str());
            h.shutdown();
            return 2;
        }
    }
    while (!ImGuiTestEngine_IsTestQueueEmpty(h.engine)) {
        h.frame();
        h.print_results(stdout);
    }
    h.frame();

    const int failed = h.print_results(stdout);
    if (h.keep_temp) std::printf("scratch files kept in %s\n", h.temp_dir.c_str());
    std::fflush(stdout);
    h.shutdown();
    return failed == 0 ? 0 : 1;
}
