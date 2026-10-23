#include "app/user_messages.h"

#include <cstdio>
#include <filesystem>
#include <new>
#include <optional>

#include "core/error_kind.h"
#include "store/library_file.h"
#include "store/upgrade_files.h"

namespace hydra::app {

namespace {

constexpr const char* kDatabaseWrite =
    "Hydra couldn't save to its database (hydra.db). Check that the disk isn't full and "
    "that no other copy of Hydra is running, then try again.";
constexpr const char* kDatabaseUpgrade =
    "Hydra couldn't update its library file (hydra.db) for this version. Your charts and "
    "results were not changed. Check that no other copy of Hydra or hydra_batch is running "
    "and that the disk isn't full, then start Hydra again.";
constexpr const char* kChartUnreadable =
    "Hydra couldn't read this chart file. It may be damaged or in a format Hydra doesn't "
    "support; try downloading the song again.";
constexpr const char* kNoteShuffleFreezes =
    "Clone Hero freezes loading this chart with Note Shuffle on.";
constexpr const char* kSongFileMissing =
    "Hydra couldn't open the song file. It may have been moved or deleted; run Scan "
    "library to update the library.";
constexpr const char* kHashFailed =
    "Windows couldn't read a song file to identify it. Restart Hydra and run Scan "
    "library again.";
constexpr const char* kSearchBroken =
    "The analysis failed on this chart because of a bug in Hydra. Please report it with "
    "the song's name.";
// The network failure sentences for one server (KindedError::service). The
// GitHub row is the user's ruling during T3a of
// docs/superpowers/plans/2026-10-22-auto-updater.md.
struct NetSentences {
    const char* server;  // the server's name in the HTTP status sentence
    const char* unreachable;
    const char* timeout;
    const char* bad_reply;
};
constexpr NetSentences kLeaderboardNet{
    "dmleaderboards",
    "Hydra couldn't reach dmleaderboards. Check your internet connection and try again.",
    "dmleaderboards didn't answer in time. Its server may be waking up; try again in a "
    "minute.",
    "dmleaderboards sent a reply Hydra couldn't read. Try again later.",
};
constexpr NetSentences kGitHubNet{
    "GitHub",
    "Hydra couldn't reach GitHub. Check your internet connection and try again.",
    "GitHub didn't answer in time. Try again later.",
    "GitHub sent a reply Hydra couldn't read. Try again later.",
};

// An error with no service is the leaderboard's: its client's own reply
// checks (net/dmbot_client.cpp) throw without one.
const NetSentences& net_sentences(const KindedError& e) {
    switch (e.service().value_or(NetService::Leaderboard)) {
        case NetService::GitHub: return kGitHubNet;
        case NetService::Leaderboard: break;
    }
    return kLeaderboardNet;
}

constexpr const char* kNoScores =
    "This player has no drum scores on dmleaderboards to compare.";
constexpr const char* kNoRecords =
    "Nothing is analyzed yet. Click a song or run a batch to analyze it.";
constexpr const char* kReportWrite =
    "Hydra couldn't save the report file. Check that the disk isn't full and the report "
    "folder isn't read-only.";
constexpr const char* kOutOfMemory =
    "Hydra ran out of memory on this chart. Close other programs and try again.";
constexpr const char* kAudioDecode =
    "Hydra couldn't decode this song's audio files. They may be damaged; try downloading "
    "the song again.";
constexpr const char* kPreviewAssets =
    "Some of Hydra's Preview files are missing. Reinstall Hydra to restore them.";
constexpr const char* kStopped = "Stopped before it finished.";

// The two sentences built around a detail.
std::string http_status_sentence(const char* server, int code) {
    return std::string(server) + " returned an error (HTTP " + std::to_string(code) +
           "). Try again later.";
}

std::string timing_refusal_sentence(std::string_view what) {
    return "Hydra can't analyze this chart because " + std::string(what) +
           ". Fix that line in the chart file or download the song again.";
}

// The words every old-file sentence opens with, naming the release the store
// draws the line at.
std::string older_than_cutoff() {
    return std::string("from a Hydra version older than ") + store::kOldestConvertibleVersion;
}

// `reason` is Windows' own text (store::rename_file), which ends with its own
// full stop; the sentence carries on after it, so that one goes.
std::string set_aside_failed_sentence(std::string reason) {
    while (!reason.empty() && (reason.back() == '.' || reason.back() == ' ' ||
                               reason.back() == '\r' || reason.back() == '\n'))
        reason.pop_back();
    return "This library file is " + older_than_cutoff() +
           ", and Hydra could not set it aside: " + reason +
           ". Close any program using hydra.db and start Hydra again.";
}

std::string old_library_refused_sentence(std::string_view path) {
    return "The library file '" + std::string(path) + "' is " + older_than_cutoff() +
           ", and opening it in Hydra sets it aside and starts a fresh library.";
}

// A kind's sentence, and whether the sentence quotes the error's own text.
// A sentence that quotes it already shows it, so plain_error_detail gives no
// details line under it (the user's rule, 2026-10-15: every failure line is
// derived once).
struct KindSentence {
    std::string sentence;
    bool quotes_raw = false;
};

KindSentence quoting(std::string sentence) { return KindSentence{std::move(sentence), true}; }

// The one owner of "which plain sentence does this failure show": the
// thrower named the kind, and each kind has one sentence. Empty when the
// error can't be answered from its kind alone.
std::optional<KindSentence> kind_sentence(const KindedError& e) {
    switch (e.kind()) {
        case ErrorKind::Cancelled: return KindSentence{kStopped};
        case ErrorKind::DatabaseOpen: return KindSentence{kDatabaseOpenSentence};
        case ErrorKind::DatabaseUpgrade: return KindSentence{kDatabaseUpgrade};
        case ErrorKind::OldLibraryRefused: return quoting(old_library_refused_sentence(e.what()));
        case ErrorKind::OldLibrarySetAsideFailed:
            return quoting(set_aside_failed_sentence(e.what()));
        case ErrorKind::DatabaseWrite: return KindSentence{kDatabaseWrite};
        case ErrorKind::DatabaseRead: return KindSentence{kDatabaseReadSentence};
        case ErrorKind::SongFileMissing: return KindSentence{kSongFileMissing};
        case ErrorKind::HashFailed: return KindSentence{kHashFailed};
        case ErrorKind::ChartUnreadable: return KindSentence{kChartUnreadable};
        case ErrorKind::ChartTimingRefused: return quoting(timing_refusal_sentence(e.what()));
        case ErrorKind::NoteShuffleFreezes: return KindSentence{kNoteShuffleFreezes};
        case ErrorKind::NoNotes: return quoting(e.what());
        case ErrorKind::AlreadyPlain: return quoting(e.what());
        case ErrorKind::SearchBroken: return KindSentence{kSearchBroken};
        case ErrorKind::NetUnreachable: return KindSentence{net_sentences(e).unreachable};
        case ErrorKind::NetTimeout: return KindSentence{net_sentences(e).timeout};
        case ErrorKind::NetHttpStatus:
            if (const std::optional<int> code = e.http_status())
                return KindSentence{http_status_sentence(net_sentences(e).server, *code)};
            return std::nullopt;  // no code to name
        case ErrorKind::NetBadReply: return KindSentence{net_sentences(e).bad_reply};
        case ErrorKind::NoScores: return KindSentence{kNoScores};
        case ErrorKind::NoRecords: return KindSentence{kNoRecords};
        case ErrorKind::ReportWrite: return KindSentence{kReportWrite};
        case ErrorKind::RulesFile: return KindSentence{kAnalysisOffSentence};
        case ErrorKind::AudioDecode: return KindSentence{kAudioDecode};
        case ErrorKind::PreviewAssets: return KindSentence{kPreviewAssets};
        case ErrorKind::UpdateUnverified: return KindSentence{kUpdateUnverifiedSentence};
    }
    return std::nullopt;
}

// kind_sentence's answer for any error: empty for one with no kind, or a kind
// that can't be answered alone.
std::optional<KindSentence> kind_sentence_of(const std::exception& e) {
    if (const auto* kinded = dynamic_cast<const KindedError*>(&e)) return kind_sentence(*kinded);
    return std::nullopt;
}

}  // namespace

std::string plain_error(const std::exception& e) {
    if (dynamic_cast<const std::bad_alloc*>(&e)) return kOutOfMemory;
    // A kinded error is answered by kind_sentence alone; its words never
    // pick the sentence.
    if (std::optional<KindSentence> kind = kind_sentence_of(e)) return std::move(kind->sentence);
    return kSomethingWentWrong;
}

std::string plain_error_detail(const std::exception& e) {
    const std::optional<KindSentence> kind = kind_sentence_of(e);
    return kind && kind->quotes_raw ? std::string() : std::string(e.what());
}

std::string plain_error_line(const std::string& sentence, const std::string& detail) {
    return detail.empty() ? sentence : detail;
}

std::string plain_error_line(const std::exception& e) {
    return plain_error_line(plain_error(e), plain_error_detail(e));
}

std::string rules_file_tool_text(const std::exception& e) {
    return std::string("hydra_rules.ini has an error:\n") + e.what();
}

std::string plain_error_block(const std::exception& e) {
    if (plain_error_detail(e).empty()) return plain_error(e);
    return plain_error(e) + "\n\n" + plain_error_detail(e);
}

std::string stored_failure_text(ErrorKind kind, const std::string& text) {
    return plain_error_block(KindedError(kind, text));
}

int tool_error(const std::exception& e, int exit_code) {
    std::fprintf(stderr, "%s\n", plain_error_block(e).c_str());
    return exit_code;
}

int run_tool(const std::function<int()>& body) {
    try {
        return body();
    } catch (const std::exception& e) {
        return tool_error(e, 1);
    }
}

namespace {

// The two causes a stored result can be out of date for. stale_text and
// stale_chip_text both read them, so each is written once.
constexpr const char* kStaleBuildCause = "another Hydra version";
constexpr const char* kStaleRulesCause = "different rules in hydra_rules.ini";

std::string both_stale_causes() {
    return std::string(kStaleBuildCause) + " or from " + kStaleRulesCause;
}

}  // namespace

std::string stale_text(bool build, bool rules) {
    std::string cause;
    if (build == rules)  // both, or neither: name both
        cause = both_stale_causes();
    else if (build)
        cause = kStaleBuildCause;
    else
        cause = kStaleRulesCause;
    return "Out of date: this result came from " + cause +
           ". Click the song or run a batch to refresh it.";
}

std::string old_library_set_aside_notice(const std::string& db_path) {
    const std::string name =
        std::filesystem::u8path(store::set_aside_path(db_path)).filename().u8string();
    return "This library file is " + older_than_cutoff() + ", so Hydra set it aside as " + name +
           " and started a fresh library.";
}

std::string stale_chip_text() {
    return "Out of date: these results came from " + both_stale_causes() +
           ". Click a song or run a batch to refresh them.";
}

}  // namespace hydra::app
