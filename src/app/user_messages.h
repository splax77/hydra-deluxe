// Plain-English versions of the errors Hydra's jobs can hit. The raw text of
// an exception ("add_song failed: disk I/O error") is for a small details
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

// Runs a command-line tool's main body and returns its exit code. An error
// that escapes the body prints its plain_error_block to stderr and the tool
// exits 1 (D73 item 4), so no tool ends with no message.
int run_tool(const std::function<int()>& body);

// Why a stored result is out of date, naming the real cause the store found
// (store::RecordLookup's stale_build and stale_rules): another Hydra version,
// or different rules in hydra_rules.ini. When both are true, or neither (a
// caller asking about a row that is not stale), it is the sentence that names
// both, so there is never an empty line.
std::string stale_text(bool build, bool rules);

// What a result with no paths says in place of them.
inline constexpr const char* kNoPathsFound = "No paths found.";

}  // namespace hydra::app

#endif  // HYDRA_APP_USER_MESSAGES_H
