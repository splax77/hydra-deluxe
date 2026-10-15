// The command-file interpreter: `hydra_uitest --script file.txt` runs one
// "script" test that reads the file line by line. Lets Claude (or anyone)
// poke the real UI without a rebuild. What each line does is run_command's
// (uitest_commands.cpp); this file only reads, echoes and prints.

#include <cstdio>
#include <fstream>
#include <string>

#include "core/strutil.h"
#include "uitest_harness.h"  // before the engine header: see uitest_commands.cpp
#include "uitest_commands.h"

namespace uitest {

namespace {

using hydra::trim;

// Echo each line, run it, print what it produced, and stop at the first
// failure or at quit.
void run_script(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    start_session(ctx);

    std::ifstream f(h.script_path);
    if (!f) {
        IM_ERRORF("cannot open script %s", h.script_path.c_str());
        return;
    }

    CommandSession session;
    std::string line;
    while (std::getline(f, line)) {
        ++session.line;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        std::printf("> %s\n", line.c_str());
        std::fflush(stdout);

        const CommandResult r = run_command(ctx, session, line);
        std::fputs(r.text.c_str(), stdout);
        if (!r.ok || session.quit) return;
        std::fflush(stdout);
    }
}

}  // namespace

void register_script_test(Harness& h) {
    ImGuiTest* t = IM_REGISTER_TEST(h.engine, "hydra", "script");
    t->UserData = &h;
    t->TestFunc = run_script;
}

}  // namespace uitest
