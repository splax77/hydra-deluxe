// The command-file interpreter: `hydra_uitest --script file.txt` runs one
// "script" test that reads the file line by line. Lets Claude (or anyone)
// poke the real UI without a rebuild. What each line does is run_command's
// (uitest_commands.cpp); this file only reads, echoes and prints.

#include <cstdio>
#include <fstream>
#include <string>

#include "uitest_harness.h"  // before the engine header: see uitest_commands.cpp
#include "uitest_commands.h"

namespace uitest {

namespace {

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
    std::string raw;
    std::string line;
    while (std::getline(f, raw)) {
        ++session.line;
        if (!command_line(raw, line)) continue;
        std::printf("> %s\n", line.c_str());
        std::fflush(stdout);

        const CommandResult r = run_command(ctx, session, line);
        std::fputs(r.text.c_str(), stdout);
        if (!r.ok || session.quit) return;
        std::fflush(stdout);
    }
}

}  // namespace

bool command_line(const std::string& raw, std::string& command) {
    // Leading blanks go. Trailing ones stay, because type keeps its text
    // exactly; a carriage return ending the line is the line's end, not text.
    size_t end = raw.size();
    if (end > 0 && raw[end - 1] == '\r') --end;
    const size_t start = raw.find_first_not_of(" \t");
    command = start < end ? raw.substr(start, end - start) : std::string();
    return !command.empty() && command[0] != '#';
}

void register_script_test(Harness& h) { register_test(h, "script", run_script); }

}  // namespace uitest
