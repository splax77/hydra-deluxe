// The command interpreter: what one command line does to the UI. The script
// test (uitest_script.cpp) feeds it a file's lines one at a time; the live
// channel feeds it lines as they arrive. Verb table: docs/agents/ui-testing.md.

#ifndef HYDRA_TESTS_UI_UITEST_COMMANDS_H
#define HYDRA_TESTS_UI_UITEST_COMMANDS_H

#include <string>

#include "imgui_te_context.h"

namespace uitest {

// What a run of commands keeps between lines.
struct CommandSession {
    // The cap on wait-idle and wait-text, in seconds. The timeout verb
    // changes it.
    double timeout = 30.0;
    // The number of the line being run, which the interpreter's own error
    // messages name. The caller sets it.
    int line = 0;
    // Set by the quit verb: the caller stops after this line.
    bool quit = false;
};

// One command's result. ok is false when the line failed: the test is then
// marked failed, and its log holds the reason. text is what the command
// produced for the reader (the screen text, a dump, the state), exactly as a
// script prints it: empty, or whole lines each ending in a newline. When an
// engine action failed (a click that found nothing), text is the
// "!! failed at line" line naming the command; the interpreter's own errors
// name their line in the log instead.
struct CommandResult {
    bool ok = true;
    std::string text;
};

// A fresh app with the ref on //Hydra: how a session starts, and what the
// reset verb does.
void start_session(ImGuiTestContext* ctx);

// Run one command line ("verb rest"), already trimmed, not blank and not a
// comment.
CommandResult run_command(ImGuiTestContext* ctx, CommandSession& session, const std::string& line);

}  // namespace uitest

#endif  // HYDRA_TESTS_UI_UITEST_COMMANDS_H
