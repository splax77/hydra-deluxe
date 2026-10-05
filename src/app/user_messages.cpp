#include "app/user_messages.h"

#include <initializer_list>
#include <new>

#include "app/report.h"  // kNothingUnderSettings
#include "app/rules_file.h"
#include "core/model.h"
#include "core/strutil.h"
#include "parse/midi.h"
#include "parse/song.h"
#include "store/serialize.h"

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

bool starts_with_any(std::string_view s, std::initializer_list<std::string_view> prefixes) {
    for (std::string_view p : prefixes)
        if (starts_with(s, p)) return true;
    return false;
}

// The sentence of parse/song.h NoNotesError, built by no_notes_message: "No
// <difficulty> [Pro ]Drums notes in this chart." It is already written for
// the user. It is matched by its words, not by the error's type, because a
// batch failure reaches plain_error_text as text only (ui/library_jobs.cpp).
bool is_no_notes_message(std::string_view s) {
    return starts_with(s, "No ") && ends_with(s, " notes in this chart.");
}

}  // namespace

std::string plain_error_text(std::string_view what) {
    // A cancel: net/dmbot_client.cpp and ui/job_base.h's JobCancelled.
    if (what == "cancelled") return kStopped;

    // net/dmbot_client.cpp: fail() appends " (error <GetLastError>)" to these.
    // 12002 is ERROR_WINHTTP_TIMEOUT.
    if (what.find("(error 12002)") != std::string_view::npos) return kNetTimeout;
    if (starts_with_any(what, {"malformed leaderboard URL", "could not start the network session",
                               "could not connect to the leaderboard", "could not build the request",
                               "could not send the request", "no response from the leaderboard",
                               "could not read the response"}))
        return kNetUnreachable;
    if (starts_with(what, "leaderboard returned HTTP ")) {
        std::string_view code = what.substr(std::string_view("leaderboard returned HTTP ").size());
        return "dmleaderboards returned an error (HTTP " + std::string(code) + "). Try again later.";
    }
    if (what == "the leaderboard sent a response Hydra couldn't read" ||
        what == "unexpected user-list format")
        return kNetBadReply;
    // ui/dm_jobs.cpp and ui/library_jobs.cpp.
    if (what == "this user has no scores to compare") return kNoScores;
    if (what == "no records stored yet") return kNoRecords;
    // app/report.cpp generate_report: results exist, but none under the
    // report's cap and fill rule. The sentence names them and is already
    // written for the user.
    if (starts_with(what, report::kNothingUnderSettings)) return std::string(what);
    // app/report_files.cpp write_report_file.
    if (starts_with(what, "cannot write ")) return kReportWrite;

    // app/analysis.cpp stream_md5, audio/mapped_file.cpp, and core/winstr.cpp
    // (read_file_bytes, which parse/midi.cpp reads through).
    if (starts_with_any(what, {"cannot open file: ", "cannot read file size: "}))
        return kSongFileMissing;
    if (what == "MD5 hashing failed" || starts_with(what, "BCryptOpenAlgorithmProvider(MD5)"))
        return kHashFailed;

    // store/record_store.cpp.
    if (starts_with(what, "failed to open database ")) return kDatabaseOpen;
    if (starts_with_any(what, {"add_song failed", "add_row ", "put_dynamics failed",
                               "meta_set failed", "reindex failed",
                               "rebuild_chart_library failed", "sqlite exec failed",
                               "prepare failed"}))
        return kDatabaseWrite;

    // search/engine.cpp.
    if (what == "search reached a broken state") return kSearchBroken;

    // The chart readers: core/model.cpp, parse/song.cpp, parse/srb.cpp,
    // parse/midi.cpp. The no-notes message is already plain.
    if (is_no_notes_message(what)) return std::string(what);
    // parse/song.cpp check_timing_maps and apply_timesig: timing that can't
    // measure time. The raw text names the line, so the user sees it.
    if (is_timing_refusal(what))
        return "Hydra can't analyze this chart because " + std::string(what) +
               ". Fix that line in the chart file or download the song again.";
    if (what == "Duplicate note." || what == "expected a [section] header" ||
        what == "No chart files found in SNG file." || what == "Truncated SNG file." ||
        what == "Truncated SRB file." || what == "SMPTE time division is not supported" ||
        starts_with_any(what, {"unexpected chart type: ", "SRB stream", "SRB inflate",
                               "not a MIDI file", "Message length "}))
        return kChartUnreadable;

    // store/serialize.cpp and store/path_codec.cpp.
    if (what == "truncated blob" ||
        starts_with_any(what, {"path node ", "unsupported path node format",
                               "unsupported path structure format"}))
        return kStoredResult;

    // audio/decode.cpp, audio/stream_mix.cpp, render/preview_renderer.cpp,
    // render/preview_config.cpp. audio/mixer.cpp's mix_stems is a test
    // reference only, so its text has no entry.
    if (starts_with_any(what, {"decode_audio:", "StreamMix: "})) return kAudioDecode;
    if (starts_with_any(what, {"PreviewRenderer: missing", "3d-config.json:"}))
        return kPreviewAssets;

    return kSomethingWentWrong;
}

std::string plain_error(const std::exception& e) {
    if (dynamic_cast<const std::bad_alloc*>(&e)) return kOutOfMemory;
    if (dynamic_cast<const RulesFileError*>(&e)) return kRulesFile;
    std::string text = plain_error_text(e.what());
    if (text != kSomethingWentWrong) return text;
    // Types whose every message means the same thing to the user.
    if (dynamic_cast<const ChartFileError*>(&e) || dynamic_cast<const MidiError*>(&e))
        return kChartUnreadable;
    if (dynamic_cast<const store::SerializeError*>(&e)) return kStoredResult;
    return text;
}

std::string plain_error_detail(const std::exception& e) { return e.what(); }

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
