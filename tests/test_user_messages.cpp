// Unit tests for app/user_messages: every error's kind becomes a short
// message that says what happened and what to do, and the raw text stays
// available for a details line.

#include "doctest.h"

#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "app/rules_file.h"
#include "app/user_messages.h"
#include "chart_text.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "load_util.h"
#include "parse/midi.h"
#include "parse/song.h"

using hydra::app::plain_error;
using hydra::app::plain_error_detail;

namespace {

const std::string kDatabaseWrite =
    "Hydra couldn't save to its database (hydra.db). Check that the disk isn't full and "
    "that no other copy of Hydra is running, then try again.";
const std::string kChartUnreadable =
    "Hydra couldn't read this chart file. It may be damaged or in a format Hydra doesn't "
    "support; try downloading the song again.";
const std::string kSongFileMissing =
    "Hydra couldn't open the song file. It may have been moved or deleted; run Scan "
    "library to update the library.";
const std::string kNetUnreachable =
    "Hydra couldn't reach dmleaderboards. Check your internet connection and try again.";
const std::string kAudioDecode =
    "Hydra couldn't decode this song's audio files. They may be damaged; try downloading "
    "the song again.";

}  // namespace

// The refusals carry their kind, as check_timing_maps throws them.
constexpr hydra::ErrorKind kRefused = hydra::ErrorKind::ChartTimingRefused;

TEST_CASE("user_messages: refused chart timing names the tick") {
    CHECK(plain_error(hydra::ChartFileError(kRefused, "the tempo at tick 384 is not above 0 BPM")) ==
          "Hydra can't analyze this chart because the tempo at tick 384 is not above 0 "
          "BPM. Fix that line in the chart file or download the song again.");
    CHECK(plain_error(hydra::ChartFileError(
              kRefused, "the time signature at tick 768 makes a measure 0 ticks long")) ==
          "Hydra can't analyze this chart because the time signature at tick 768 makes a "
          "measure 0 ticks long. Fix that line in the chart file or download the song "
          "again.");
    CHECK(plain_error(hydra::ChartFileError(
              kRefused, "the chart's resolution is 0, and it must be above 0")) ==
          "Hydra can't analyze this chart because the chart's resolution is 0, and it must "
          "be above 0. Fix that line in the chart file or download the song again.");
}

// The messages above are typed by hand. This one is thrown by a real refused
// load, so rewording the throw in parse/song.cpp cannot quietly drop the user
// back to the generic "couldn't read this chart file" text.
TEST_CASE("user_messages: a real refused load shows the tick sentence") {
    const std::string chart = testchart::chart_text(
        testchart::section("ExpertDrums", "  0 = N 0 0\n"), 192, "", "  0 = TS 4\n  0 = B 0\n");
    try {
        hydra::load_songbytes_chart(std::vector<uint8_t>(chart.begin(), chart.end()),
                                    hydra::ChartReading{});
        FAIL("a chart with B 0 loaded");
    } catch (const std::exception& e) {
        CHECK(plain_error(e) ==
              "Hydra can't analyze this chart because the tempo at tick 0 is not above 0 "
              "BPM. Fix that line in the chart file or download the song again.");
    }
}

// A missing tick-0 tempo and an unreadable Resolution are refused by real
// loads, and each shows the refusal sentence, never the standard library's
// words ("bpm map has no tick-0 entry", "invalid stoll argument").
TEST_CASE("user_messages: a missing tick-0 tempo or unreadable Resolution shows a sentence") {
    auto shown = [](const std::string& chart) {
        try {
            hydra::load_songbytes_chart(std::vector<uint8_t>(chart.begin(), chart.end()),
                                        hydra::ChartReading{});
        } catch (const std::exception& e) {
            CHECK(plain_error_detail(e).empty());
            return plain_error(e);
        }
        return std::string("(loaded)");
    };
    const std::string drums = testchart::section("ExpertDrums", "  0 = N 0 0\n  384 = N 1 0\n");
    const std::string sync = testchart::section("SyncTrack", "  0 = TS 4\n  0 = B 120000\n");
    CHECK(shown(testchart::chart_text(drums, 192, "", "  0 = TS 4\n  384 = B 60000\n")) ==
          "Hydra can't analyze this chart because it has no tempo at tick 0. Fix that line "
          "in the chart file or download the song again.");
    CHECK(shown(sync + drums) ==
          "Hydra can't analyze this chart because the chart's resolution is missing. Fix that "
          "line in the chart file or download the song again.");
    CHECK(shown(testchart::section("Song", "  Resolution = abc\n") + sync + drums) ==
          "Hydra can't analyze this chart because the chart's resolution is not a whole "
          "number. Fix that line in the chart file or download the song again.");
    CHECK(shown(testchart::section("Song", "  Resolution = 99999999999999999999\n") + sync +
                drums) ==
          "Hydra can't analyze this chart because the chart's resolution is too large. Fix "
          "that line in the chart file or download the song again.");
}

TEST_CASE("user_messages: an infinite .mid tempo says so") {
    CHECK(plain_error(hydra::ChartFileError(
              kRefused, "the tempo at tick 96 is infinite (0 microseconds per beat)")) ==
          "Hydra can't analyze this chart because the tempo at tick 96 is infinite (0 "
          "microseconds per beat). Fix that line in the chart file or download the song "
          "again.");
}

TEST_CASE("user_messages: the no-notes message is already plain and passes through") {
    const std::string msg = "No Expert Pro Drums notes in this chart.";
    CHECK(plain_error(hydra::ChartFileError(hydra::ErrorKind::AlreadyPlain, msg)) == msg);
    // The typed error the loaders throw passes through the same way.
    CHECK(plain_error(hydra::NoNotesError(hydra::Difficulty::Expert, true)) == msg);
    CHECK(plain_error(hydra::NoNotesError(hydra::Difficulty::Hard, false)) ==
          "No Hard Drums notes in this chart.");
}

TEST_CASE("user_messages: report, rules, memory") {
    const hydra::app::RulesFileError bad_rules("hydra_rules.ini:3: unknown key \"x\"");
    CHECK(plain_error(bad_rules) ==
          "hydra_rules.ini has an error, so analysis is off until the file is fixed and Hydra "
          "is restarted.");
    // The command-line tools' text (D114 item 11): their sentence, then the
    // loader's line.
    CHECK(hydra::app::rules_file_tool_text(bad_rules) ==
          "hydra_rules.ini has an error:\nhydra_rules.ini:3: unknown key \"x\"");
    CHECK(plain_error(std::bad_alloc()) ==
          "Hydra ran out of memory on this chart. Close other programs and try again.");
}

TEST_CASE("user_messages: stale_text names the real cause") {
    using hydra::app::stale_text;
    CHECK(stale_text(/*build=*/true, /*rules=*/false) ==
          "Out of date: this result came from another Hydra version. Click the song or run a "
          "batch to refresh it.");
    CHECK(stale_text(/*build=*/false, /*rules=*/true) ==
          "Out of date: this result came from different rules in hydra_rules.ini. Click the "
          "song or run a batch to refresh it.");
    // Both causes: the sentence the details panel shows today, unchanged.
    const std::string both =
        "Out of date: this result came from another Hydra version or from different rules "
        "in hydra_rules.ini. Click the song or run a batch to refresh it.";
    CHECK(stale_text(/*build=*/true, /*rules=*/true) == both);
    // Neither (a caller asking for a row that is not stale): no cause to
    // name, so today's sentence, never an empty line.
    CHECK(stale_text(/*build=*/false, /*rules=*/false) == both);
    CHECK(std::string(hydra::app::kNoPathsFound) == "No paths found.");
}

// The file name the notice shows is the one store::set_aside_path gave the
// file, so the notice and the file on disk agree.
TEST_CASE("user_messages: the set-aside notice names the file the store set aside") {
    CHECK(hydra::app::old_library_set_aside_notice("C:\\Hydra\\hydra.db") ==
          "This library file is from a Hydra version older than 2.2.0, so Hydra set it aside "
          "as hydra.db.pre-2.2.0 and started a fresh library.");
    CHECK(hydra::app::old_library_set_aside_notice("C:/some folder/library.db") ==
          "This library file is from a Hydra version older than 2.2.0, so Hydra set it aside "
          "as library.db.pre-2.2.0 and started a fresh library.");
}

// The Stale chip stands for many rows, so it names both causes.
TEST_CASE("user_messages: the Stale chip's tooltip names both causes") {
    CHECK(hydra::app::stale_chip_text() ==
          "Out of date: these results came from another Hydra version or from different rules "
          "in hydra_rules.ini. Click a song or run a batch to refresh them.");
}

// D91: the not-analyzed row's tooltip names the click and the batch, and no
// button (there is none).
TEST_CASE("user_messages: the not-analyzed tooltip names the click and the batch") {
    CHECK(std::string(hydra::app::kNotAnalyzedText) ==
          "Not analyzed yet. Click the song or run a batch to analyze it.");
}

// The thrower names the kind, so the words of the error don't matter: every
// kind reads its own sentence from "x".
TEST_CASE("user_messages: a kinded error reads its kind's sentence, whatever its words") {
    using hydra::ErrorKind;
    using hydra::KindedError;
    const std::pair<ErrorKind, std::string> cases[] = {
        {ErrorKind::Cancelled, "Stopped before it finished."},
        {ErrorKind::DatabaseOpen,
         "Hydra couldn't open its database (hydra.db). Check that no other copy of Hydra is "
         "running and that the Hydra folder isn't read-only."},
        {ErrorKind::DatabaseUpgrade,
         "Hydra couldn't update its library file (hydra.db) for this version. Your charts and "
         "results were not changed. Check that no other copy of Hydra or hydra_batch is "
         "running and that the disk isn't full, then start Hydra again."},
        {ErrorKind::DatabaseWrite, kDatabaseWrite},
        {ErrorKind::DatabaseRead,
         "Hydra couldn't read its database (hydra.db). Check that no other copy of Hydra is "
         "running, then try again."},
        {ErrorKind::SongFileMissing, kSongFileMissing},
        {ErrorKind::HashFailed,
         "Windows couldn't read a song file to identify it. Restart Hydra and run Scan "
         "library again."},
        {ErrorKind::ChartUnreadable, kChartUnreadable},
        {ErrorKind::SearchBroken,
         "The analysis failed on this chart because of a bug in Hydra. Please report it "
         "with the song's name."},
        {ErrorKind::NetUnreachable, kNetUnreachable},
        {ErrorKind::NetTimeout,
         "dmleaderboards didn't answer in time. Its server may be waking up; try again in "
         "a minute."},
        {ErrorKind::NetBadReply, "dmleaderboards sent a reply Hydra couldn't read. Try again later."},
        {ErrorKind::NoScores, "This player has no drum scores on dmleaderboards to compare."},
        {ErrorKind::NoRecords,
         "Nothing is analyzed yet. Click a song or run a batch to analyze it."},
        {ErrorKind::ReportWrite,
         "Hydra couldn't save the report file. Check that the disk isn't full and the "
         "report folder isn't read-only."},
        {ErrorKind::RulesFile,
         "hydra_rules.ini has an error, so analysis is off until the file is fixed and "
         "Hydra is restarted."},
        {ErrorKind::AudioDecode, kAudioDecode},
        {ErrorKind::PreviewAssets,
         "Some of Hydra's Preview files are missing. Reinstall Hydra to restore them."},
        {ErrorKind::UpdateUnverified,
         "Hydra couldn't verify the update it downloaded, so it was not installed. Try again "
         "later, or download the release from GitHub yourself."},
    };
    for (const auto& [kind, sentence] : cases) {
        CAPTURE(static_cast<int>(kind));
        CHECK(plain_error(KindedError(kind, "x")) == sentence);
    }
    CHECK(plain_error(KindedError(ErrorKind::NetHttpStatus, "x", 503)) ==
          "dmleaderboards returned an error (HTTP 503). Try again later.");
    CHECK(plain_error(KindedError(ErrorKind::ChartTimingRefused, "x")) ==
          "Hydra can't analyze this chart because x. Fix that line in the chart file or "
          "download the song again.");
    // The two old-library kinds carry the file's path and Windows' reason.
    // The reason's own full stop goes, since the sentence carries on after it.
    CHECK(plain_error(KindedError(ErrorKind::OldLibraryRefused, "C:\\lib\\hydra.db")) ==
          "The library file 'C:\\lib\\hydra.db' is from a Hydra version older than 2.2.0, and "
          "opening it in Hydra sets it aside and starts a fresh library.");
    CHECK(plain_error(KindedError(ErrorKind::OldLibrarySetAsideFailed, "Access is denied.\r\n")) ==
          "This library file is from a Hydra version older than 2.2.0, and Hydra could not set "
          "it aside: Access is denied. Close any program using hydra.db and start Hydra again.");
    CHECK(plain_error(KindedError(ErrorKind::AlreadyPlain, "x")) == "x");
    // The typed errors carry their kinds too.
    CHECK(plain_error(hydra::ChartFileError("x")) == kChartUnreadable);
    CHECK(plain_error(hydra::MidiError("x")) == kChartUnreadable);
}

TEST_CASE("a network failure's sentence names the server that failed") {
    using hydra::ErrorKind;
    using hydra::KindedError;
    using hydra::NetService;
    const auto with = [](ErrorKind kind, std::optional<NetService> service,
                         std::optional<int> status = std::nullopt) {
        return plain_error(KindedError(kind, "x", status, service));
    };
    // GitHub: the user's ruling during T3a of the auto-updater plan.
    CHECK(with(ErrorKind::NetUnreachable, NetService::GitHub) ==
          "Hydra couldn't reach GitHub. Check your internet connection and try again.");
    CHECK(with(ErrorKind::NetTimeout, NetService::GitHub) ==
          "GitHub didn't answer in time. Try again later.");
    CHECK(with(ErrorKind::NetBadReply, NetService::GitHub) ==
          "GitHub sent a reply Hydra couldn't read. Try again later.");
    CHECK(with(ErrorKind::NetHttpStatus, NetService::GitHub, 403) ==
          "GitHub returned an error (HTTP 403). Try again later.");
    // The leaderboard named, or no server named, keeps the dmleaderboards words.
    for (const std::optional<NetService> service :
         {std::optional<NetService>(NetService::Leaderboard), std::optional<NetService>()}) {
        CHECK(with(ErrorKind::NetUnreachable, service) == kNetUnreachable);
        CHECK(with(ErrorKind::NetTimeout, service) ==
              "dmleaderboards didn't answer in time. Its server may be waking up; try again in "
              "a minute.");
        CHECK(with(ErrorKind::NetBadReply, service) ==
              "dmleaderboards sent a reply Hydra couldn't read. Try again later.");
        CHECK(with(ErrorKind::NetHttpStatus, service, 503) ==
              "dmleaderboards returned an error (HTTP 503). Try again later.");
    }
    // The detail line stays the raw text for every server.
    CHECK(plain_error_detail(
              KindedError(ErrorKind::NetUnreachable, "raw", std::nullopt, NetService::GitHub)) ==
          "raw");
}

TEST_CASE("the transport's host names come from the service") {
    CHECK(hydra::net_service_host_name(hydra::NetService::GitHub) == "GitHub");
    CHECK(hydra::net_service_host_name(hydra::NetService::Leaderboard) == "leaderboard");
}

// D73 item 4: a tool's body that throws ends with exit code 1 (and its
// plain_error_block on stderr); one that returns keeps its own code.
TEST_CASE("user_messages: run_tool turns an escaping error into exit code 1") {
    using hydra::app::run_tool;
    CHECK(run_tool([] { return 0; }) == 0);
    CHECK(run_tool([] { return 2; }) == 2);
    CHECK(run_tool([]() -> int {
              throw hydra::KindedError(hydra::ErrorKind::DatabaseRead, "x");
          }) == 1);
}

// Only the kind picks a sentence. An untyped error that happens to carry a
// thrower's words reads the fallback like any other.
TEST_CASE("user_messages: an untyped error reads the fallback, whatever its words") {
    for (const char* raw : {"cannot write C:\\x\\hydra_paths.html", "cancelled"}) {
        CAPTURE(raw);
        CHECK(plain_error(std::runtime_error(raw)) ==
              "Something went wrong. Try again, and if it keeps happening, report it with "
              "the details below.");
    }
}

TEST_CASE("user_messages: anything else falls back, and the detail keeps the raw text") {
    const std::runtime_error odd("prepare_record: key asks for sp_cap 5");
    CHECK(plain_error(odd) == hydra::app::kSomethingWentWrong);
    CHECK(std::string(hydra::app::kSomethingWentWrong) ==
          "Something went wrong. Try again, and if it keeps happening, report it with the "
          "details below.");
    CHECK(plain_error_detail(odd) == "prepare_record: key asks for sp_cap 5");
}

// The startup message box and the command-line tools show both at once
// (D72 items 1 and 5).
TEST_CASE("user_messages: plain_error_block puts the sentence above the raw text") {
    using hydra::app::plain_error_block;
    CHECK(plain_error_block(hydra::KindedError(hydra::ErrorKind::DatabaseOpen,
                                               "sqlite exec failed: file is not a database")) ==
          "Hydra couldn't open its database (hydra.db). Check that no other copy of Hydra is "
          "running and that the Hydra folder isn't read-only.\n"
          "\n"
          "sqlite exec failed: file is not a database");
    CHECK(plain_error_block(std::runtime_error("CreateWindowW failed (GetLastError 0x00000578)")) ==
          "Something went wrong. Try again, and if it keeps happening, report it with the "
          "details below.\n"
          "\n"
          "CreateWindowW failed (GetLastError 0x00000578)");
    // The refused old library's sentence already names the file, so the path
    // shows once (D115 task A1-6d).
    CHECK(plain_error_block(hydra::KindedError(hydra::ErrorKind::OldLibraryRefused,
                                               "C:\\lib\\hydra.db")) ==
          "The library file 'C:\\lib\\hydra.db' is from a Hydra version older than 2.2.0, and "
          "opening it in Hydra sets it aside and starts a fresh library.");
    // The details line a window shows separately is empty for it too.
    CHECK(plain_error_detail(hydra::KindedError(hydra::ErrorKind::OldLibraryRefused,
                                                "C:\\lib\\hydra.db"))
              .empty());
    // The set-aside failure's sentence quotes Windows' whole reason, so the
    // reason shows once too.
    CHECK(plain_error_block(hydra::KindedError(hydra::ErrorKind::OldLibrarySetAsideFailed,
                                               "Access is denied.")) ==
          "This library file is from a Hydra version older than 2.2.0, and Hydra could not set "
          "it aside: Access is denied. Close any program using hydra.db and start Hydra again.");
}

// The user's rule (2026-10-15): every failure line is derived once. A sentence
// built from the error's own text already shows that text, so no details line
// goes under it, whichever kind built it.
TEST_CASE("user_messages: no details line repeats a sentence that quotes the raw text") {
    using hydra::app::plain_error_block;
    const hydra::NoNotesError no_notes(hydra::Difficulty::Expert, true);
    CHECK(plain_error_detail(no_notes).empty());
    CHECK(plain_error_block(no_notes) == plain_error(no_notes));
    CHECK(plain_error_detail(hydra::ChartFileError(hydra::ErrorKind::AlreadyPlain, "x")).empty());
    CHECK(plain_error_detail(
              hydra::ChartFileError(kRefused, "the tempo at tick 384 is not above 0 BPM"))
              .empty());
    CHECK(plain_error_detail(hydra::KindedError(hydra::ErrorKind::OldLibrarySetAsideFailed,
                                                "Access is denied.\r\n"))
              .empty());
    // A sentence that does not quote the raw text keeps it as the details line.
    CHECK(plain_error_detail(hydra::KindedError(hydra::ErrorKind::ChartUnreadable, "bad header")) ==
          "bad header");
}
