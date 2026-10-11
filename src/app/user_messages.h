// Plain-English versions of the errors Hydra's jobs can hit. The raw text of
// an exception ("add_row failed: disk I/O error") is for a small details
// line. What the user reads first is a short message that says what
// happened and what to do, picked by the error's kind. Every job and view
// that shows an error takes the wording from here, so it lives in one place.

#ifndef HYDRA_APP_USER_MESSAGES_H
#define HYDRA_APP_USER_MESSAGES_H

#include <exception>
#include <functional>
#include <string>
#include <string_view>

namespace hydra::app {

// What every error Hydra doesn't recognize reads as.
inline constexpr const char* kSomethingWentWrong =
    "Something went wrong. Try again, and if it keeps happening, report it with the "
    "details below.";

// What a database that won't open reads as (ErrorKind::DatabaseOpen). Tests
// that see it reach the user compare against this name, not a copy of the words.
inline constexpr const char* kDatabaseOpenSentence =
    "Hydra couldn't open its database (hydra.db). Check that no other copy of Hydra is "
    "running and that the Hydra folder isn't read-only.";

// What a failed database read reads as (ErrorKind::DatabaseRead, D73 item 1).
inline constexpr const char* kDatabaseReadSentence =
    "Hydra couldn't read its database (hydra.db). Check that no other copy of Hydra is "
    "running, then try again.";

// What a bad hydra_rules.ini reads as in the app (ErrorKind::RulesFile, D114
// items 11 and 13): analysis stays off while it is bad (AppState::rules_error).
// The library toolbar and the path report window show it.
inline constexpr const char* kAnalysisOffSentence =
    "hydra_rules.ini has an error, so analysis is off until the file is fixed and "
    "Hydra is restarted.";

// What a command-line tool prints for a bad hydra_rules.ini (D114 item 11):
// the tools' sentence, then the loader's line from `e`. The tools stop instead
// of running on (app::load_tool_rules), so the app's sentence does not fit.
std::string rules_file_tool_text(const std::exception& e);

// What happened and what to do, in at most two short sentences. The sentence
// comes from the error's kind (core/error_kind.h), through the one switch,
// kind_sentence in user_messages.cpp; the error's words never pick it. Running
// out of memory is known by its type, and an error with no kind reads
// kSomethingWentWrong.
std::string plain_error(const std::exception& e);

// The raw text, for a small details line under the plain message.
std::string plain_error_detail(const std::exception& e);

// plain_error, a blank line, then plain_error_detail: one block of text, for
// a place with no separate details line (Hydra's startup message box and the
// command-line tools' stderr, D72).
std::string plain_error_block(const std::exception& e);

// How a command-line tool ends on an error: its plain_error_block on stderr,
// then the exit code the caller passes (D72 item 5, D73 item 4).
int tool_error(const std::exception& e, int exit_code);

// Runs a command-line tool's main body and returns its exit code. An error
// that escapes the body ends the tool through tool_error with code 1 (D73
// item 4), so no tool ends with no message.
int run_tool(const std::function<int()>& body);

// Why a stored result is out of date, naming the real cause the store found
// (store::SummaryLookup's stale_build and stale_rules): another Hydra version,
// or different rules in hydra_rules.ini. When both are true, or neither (a
// caller asking about a row that is not stale), it is the sentence that names
// both, so there is never an empty line.
std::string stale_text(bool build, bool rules);

// What the library's Stale chip says when hovered. The chip stands for many
// rows, so it names both causes stale_text can name.
std::string stale_chip_text();

// What a library row with no result says when hovered (D91). The click and the
// batch are the two ways to analyze it.
inline constexpr const char* kNotAnalyzedText =
    "Not analyzed yet. Click the song or run a batch to analyze it.";

// What a result with no paths says in place of them.
inline constexpr const char* kNoPathsFound = "No paths found.";

}  // namespace hydra::app

#endif  // HYDRA_APP_USER_MESSAGES_H
