// Plain-English versions of the errors Hydra's jobs can hit. The raw text of
// an exception (RecordStore::save's "saving a record failed: disk I/O error")
// is for a small details line. What the user reads first is a short message
// that says what happened and what to do, picked by the error's kind (kind_sentence in
// user_messages.cpp). A sentence that more than one screen shows also lives
// here, so it is written once. A sentence only one screen shows may stay with
// that screen.

#ifndef HYDRA_APP_USER_MESSAGES_H
#define HYDRA_APP_USER_MESSAGES_H

#include <exception>
#include <functional>
#include <string>
#include <string_view>

namespace hydra {
enum class ErrorKind;  // core/error_kind.h
}

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
// The library toolbar, the path report window and the song panel show it.
inline constexpr const char* kAnalysisOffSentence =
    "hydra_rules.ini has an error, so analysis is off until the file is fixed and "
    "Hydra is restarted.";

// What a refused update reads as (ErrorKind::UpdateUnverified, decision 9 of
// docs/superpowers/plans/2026-10-22-auto-updater.md). The version bar shows
// it for every refusal; the details line under it names the check.
inline constexpr const char* kUpdateUnverifiedSentence =
    "Hydra couldn't verify the update it downloaded, so it was not installed. Try again "
    "later, or download the release from GitHub yourself.";

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

// The raw text, for a small details line under the plain message. Empty when
// the sentence already quotes the raw text (KindSentence::quotes_raw in
// user_messages.cpp), so no place shows it twice. Every screen and tool that
// shows a details line takes it from here, never from the exception's what()
// (the single-owner row error-detail-owner), and draws nothing when it is
// empty.
std::string plain_error_detail(const std::exception& e);

// The one line a place with room for a single line per failure shows (the
// scan's problem list, hydra_batch's per-chart lines): the details line, or
// the sentence when the sentence quotes the raw text and the details line is
// empty. The string form takes a sentence and details line already worked out
// by plain_error and plain_error_detail (BatchCallbacks::on_error).
std::string plain_error_line(const std::exception& e);
std::string plain_error_line(const std::string& sentence, const std::string& detail);

// plain_error, a blank line, then plain_error_detail: one block of text, for
// a place with no separate details line (Hydra's startup message box and the
// command-line tools' stderr, D72). With no detail it is the sentence alone.
std::string plain_error_block(const std::exception& e);

// plain_error_block for a failure the store kept as a chart's result
// (store::RecordLookup's failure_kind and failure_text): the error is rebuilt
// from its kind and raw text, so the words come from kind_sentence like any
// live error's, never from stored wording. The library's Can't analyze cell
// shows it when hovered.
std::string stored_failure_text(ErrorKind kind, const std::string& text);

// How a command-line tool ends on an error: its plain_error_block on stderr,
// then the exit code the caller passes (D72 item 5, D73 item 4).
int tool_error(const std::exception& e, int exit_code);

// Runs a command-line tool's main body and returns its exit code. An error
// that escapes the body ends the tool through tool_error with code 1 (D73
// item 4), so no tool ends with no message.
int run_tool(const std::function<int()>& body);

// Why a stored result is out of date, naming the real cause the store found
// (store::RecordLookup's stale_build and stale_rules): another Hydra version,
// or different rules in hydra_rules.ini. When both are true, or neither (a
// caller asking about a row that is not stale), it is the sentence that names
// both, so there is never an empty line.
std::string stale_text(bool build, bool rules);

// What the app says after its open set a library file too old to convert
// aside (store::OldFilePolicy::SetAside). `db_path` is the library file's own
// path; the name shown is the one store::set_aside_path gave the file.
std::string old_library_set_aside_notice(const std::string& db_path);

// What the library's Stale chip says when hovered. The chip stands for many
// rows, so it names both causes stale_text can name.
std::string stale_chip_text();

// What a library row with no result says when hovered (D91). The click and the
// batch are the two ways to analyze it.
inline constexpr const char* kNotAnalyzedText =
    "Not analyzed yet. Click the song or run a batch to analyze it.";

// What a result with no paths says in place of them.
inline constexpr const char* kNoPathsFound = "No paths found.";

// The batch's path report outcome, read by the batch's finished strip, the
// status line and the path report window (D114 item 14).
inline constexpr const char* kPathReportReady = "The path report is ready.";
inline constexpr const char* kPathReportFailed = "The path report could not be built.";

// The comparison window's failure sentence, in the path report's shape (D114
// item 14).
inline constexpr const char* kComparisonFailed = "The comparison could not be built.";

// Under anything that waits on dmleaderboards: the user list in the picker
// and the comparison window while it builds.
inline constexpr const char* kLeaderboardWaking =
    "The leaderboard server can take a moment to wake up.";

}  // namespace hydra::app

#endif  // HYDRA_APP_USER_MESSAGES_H
