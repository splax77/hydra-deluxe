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
    // changes it; the live channel reports it in every reply.
    double timeout = 30.0;
    // The number of the line being run, which a script's error messages
    // name. The script test sets it.
    int line = 0;
    // True for the live channel: errors name no line, a failure comes back
    // in the result instead of failing the test where it can, and no wait
    // may outlast the timeout.
    bool live = false;
    // Set by the quit verb: the caller stops after this line.
    bool quit = false;
};

// One command's result. ok is false when the line failed. text is what the
// command produced for the reader (the screen text, a dump, the state),
// exactly as a script prints it: empty, or whole lines each ending in a
// newline.
//
// On a failure in a script, text is the "!! failed at line" line when an
// engine action failed (a click that found nothing), and empty when the
// interpreter refused the line; the test is marked failed and its log names
// the line. In live mode, text is the one-line reason with no line number
// and no newline; the test is marked failed only by a failed engine action,
// whose log lines the live channel adds to its reply.
struct CommandResult {
    bool ok = true;
    std::string text;
};

// A fresh app with the ref on //Hydra: how a session starts, and what the
// reset verb does.
void start_session(ImGuiTestContext* ctx);

// Run one command line ("verb rest"), as command_line hands it over: not
// blank and not a comment.
CommandResult run_command(ImGuiTestContext* ctx, CommandSession& session, const std::string& line);

}  // namespace uitest

#endif  // HYDRA_TESTS_UI_UITEST_COMMANDS_H
