#include "app/user_messages.h"

#include <new>
#include <optional>

#include "core/error_kind.h"

namespace hydra::app {

namespace {

constexpr const char* kDatabaseWrite =
    "Hydra couldn't save to its database (hydra.db). Check that the disk isn't full and "
    "that no other copy of Hydra is running, then try again.";
constexpr const char* kDatabaseOpen =
    "Hydra couldn't open its database (hydra.db). Check that no other copy of Hydra is "
    "running and that the Hydra folder isn't read-only.";
constexpr const char* kChartUnreadable =
    "Hydra couldn't read this chart file. It may be damaged or in a format Hydra doesn't "
    "support; try downloading the song again.";
constexpr const char* kSongFileMissing =
    "Hydra couldn't open the song file. It may have been moved or deleted; run Scan "
    "library to update the library.";
constexpr const char* kHashFailed =
    "Windows couldn't read a song file to identify it. Restart Hydra and run Scan "
    "library again.";
constexpr const char* kSearchBroken =
    "The analysis failed on this chart because of a bug in Hydra. Please report it with "
    "the song's name.";
constexpr const char* kNetUnreachable =
    "Hydra couldn't reach dmleaderboards. Check your internet connection and try again.";
constexpr const char* kNetTimeout =
    "dmleaderboards didn't answer in time. Its server may be waking up; try again in a "
    "minute.";
constexpr const char* kNetBadReply =
    "dmleaderboards sent a reply Hydra couldn't read. Try again later.";
constexpr const char* kNoScores =
    "This player has no drum scores on dmleaderboards to compare.";
constexpr const char* kNoRecords =
    "There are no analyzed songs to put in a report yet. Analyze some songs first.";
constexpr const char* kReportWrite =
    "Hydra couldn't save the report file. Check that the disk isn't full and the report "
    "folder isn't read-only.";
constexpr const char* kRulesFile =
    "hydra_rules.ini has a line Hydra can't read. Fix or delete that line, then restart "
    "Hydra.";
constexpr const char* kStoredResult =
    "A saved result couldn't be read. Re-analyze this song to replace it.";
constexpr const char* kOutOfMemory =
    "Hydra ran out of memory on this chart. Close other programs and try again.";
constexpr const char* kAudioDecode =
    "Hydra couldn't decode this song's audio files. They may be damaged; try downloading "
    "the song again.";
constexpr const char* kPreviewAssets =
    "Some of Hydra's Preview files are missing. Reinstall Hydra to restore them.";
constexpr const char* kStopped = "Stopped before it finished.";

// The two sentences built around a detail.
std::string http_status_sentence(int code) {
    return "dmleaderboards returned an error (HTTP " + std::to_string(code) + "). Try again later.";
}

std::string timing_refusal_sentence(std::string_view what) {
    return "Hydra can't analyze this chart because " + std::string(what) +
           ". Fix that line in the chart file or download the song again.";
}

// The one owner of "which plain sentence does this failure show": the
// thrower named the kind, and each kind has one sentence. Empty when the
// error can't be answered from its kind alone.
std::optional<std::string> kind_sentence(const KindedError& e) {
    switch (e.kind()) {
        case ErrorKind::Cancelled: return kStopped;
        case ErrorKind::DatabaseOpen: return kDatabaseOpen;
        case ErrorKind::DatabaseWrite: return kDatabaseWrite;
        case ErrorKind::SongFileMissing: return kSongFileMissing;
        case ErrorKind::HashFailed: return kHashFailed;
        case ErrorKind::ChartUnreadable: return kChartUnreadable;
        case ErrorKind::ChartTimingRefused: return timing_refusal_sentence(e.what());
        case ErrorKind::AlreadyPlain: return std::string(e.what());
        case ErrorKind::SearchBroken: return kSearchBroken;
        case ErrorKind::NetUnreachable: return kNetUnreachable;
        case ErrorKind::NetTimeout: return kNetTimeout;
        case ErrorKind::NetHttpStatus:
            if (const std::optional<int> code = e.http_status()) return http_status_sentence(*code);
            return std::nullopt;  // no code to name
        case ErrorKind::NetBadReply: return kNetBadReply;
        case ErrorKind::NoScores: return kNoScores;
        case ErrorKind::NoRecords: return kNoRecords;
        case ErrorKind::ReportWrite: return kReportWrite;
        case ErrorKind::RulesFile: return kRulesFile;
        case ErrorKind::StoredResult: return kStoredResult;
        case ErrorKind::AudioDecode: return kAudioDecode;
        case ErrorKind::PreviewAssets: return kPreviewAssets;
    }
    return std::nullopt;
}

}  // namespace

std::string plain_error(const std::exception& e) {
    if (dynamic_cast<const std::bad_alloc*>(&e)) return kOutOfMemory;
    // A kinded error is answered by kind_sentence alone; its words never
    // pick the sentence.
    if (const auto* kinded = dynamic_cast<const KindedError*>(&e))
        if (std::optional<std::string> sentence = kind_sentence(*kinded)) return *sentence;
    return kSomethingWentWrong;
}

std::string plain_error_detail(const std::exception& e) { return e.what(); }

std::string plain_error_block(const std::exception& e) {
    return plain_error(e) + "\n\n" + plain_error_detail(e);
}

std::string stale_text(bool build, bool rules) {
    std::string cause;
    if (build == rules)  // both, or neither: name both
        cause = "another Hydra version or from different rules in hydra_rules.ini";
    else if (build)
        cause = "another Hydra version";
    else
        cause = "different rules in hydra_rules.ini";
    return "Out of date: this result came from " + cause + ". Re-analyze to refresh it.";
}

}  // namespace hydra::app
