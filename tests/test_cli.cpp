// End-to-end tests for the two console tools (src/cli/) and hydra_replay's
// flags. Each test copies the built exes into a fresh temp folder and runs
// them there, because the tools read hydra_settings.ini and hydra_rules.ini
// from their own folder: a folder with neither gives the app's defaults,
// whatever the developer's build folder holds. The exe paths come from
// CMakeLists.txt, which also builds the tools before the tests.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "json.hpp"

#include "app/analysis.h"
#include "app/config.h"
#include "app/fill_report.h"  // generate_fill_report, FillCompareStats
#include "app/report.h"  // nothing_under_settings
#include "app/rules_file.h"  // load_rules_file, RulesFileError, kRulesFileName
#include "app/user_messages.h"  // kDatabaseOpenSentence, ini_file_tool_text
#include "cli/batch_scope.h"
#include "core/unknown_option.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "db_file_util.h"  // write_junk_db
#include "display_fixtures.h"  // kTagOnlyTitle, name_chart, save_batch_record
#include "parse/chart_files.h"
#include "parse/song.h"
#include "search/graph.h"
#include "store/record_store.h"
#include "store/upgrade_files.h"  // set_aside_path
#include "temp_util.h"
#include "wait_util.h"

#if !defined(HYDRA_BATCH_EXE) || !defined(HYDRA_FILLCOMPARE_EXE) || !defined(HYDRA_REPLAY_EXE)
#error "the CLI exe paths must be defined (see CMakeLists.txt)"
#endif

namespace fs = std::filesystem;

namespace {

struct RunResult {
    int exit_code = -1;
    std::string output;  // stdout and stderr, interleaved
};

// Runs `exe` with `args`, waits for it, and captures everything it prints. A
// run past testwait::kWaitCap is killed, and the test fails naming it.
RunResult run_exe(const fs::path& exe, const std::vector<std::string>& args) {
    std::wstring cmd = L"\"" + exe.wstring() + L"\"";
    for (const std::string& a : args) cmd += L" \"" + hydra::utf8_to_wide(a) + L"\"";

    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE read_end = nullptr, write_end = nullptr;
    REQUIRE(CreatePipe(&read_end, &write_end, &sa, 0));
    SetHandleInformation(read_end, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = write_end;
    si.hStdError = write_end;
    si.hStdInput = nullptr;
    PROCESS_INFORMATION pi{};
    const BOOL started = CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE,
                                       CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(write_end);  // the child holds its own copy
    REQUIRE_MESSAGE(started, "could not start " << exe.u8string());

    // The pipe is drained on its own thread: a read blocks until the child
    // writes or exits, so reading here would leave nothing to time the run.
    RunResult r;
    std::thread reader([&r, read_end] {
        char buf[4096];
        DWORD got = 0;
        while (ReadFile(read_end, buf, sizeof(buf), &got, nullptr) && got > 0)
            r.output.append(buf, got);
    });
    const DWORD cap_ms = static_cast<DWORD>(testwait::kWaitCap.count());
    const bool timed_out = WaitForSingleObject(pi.hProcess, cap_ms) == WAIT_TIMEOUT;
    if (timed_out) {
        TerminateProcess(pi.hProcess, 1);
        // Once the child is gone, so is the pipe's only write end, and the
        // reader's ReadFile returns.
        WaitForSingleObject(pi.hProcess, cap_ms);
    }
    reader.join();
    CloseHandle(read_end);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    std::string command = exe.filename().u8string();
    for (const std::string& a : args) command += " " + a;
    REQUIRE_MESSAGE(!timed_out, command << " ran past " << testwait::cap_text(testwait::kWaitCap)
                                        << " and was stopped; it printed:\n"
                                        << r.output);
    r.exit_code = static_cast<int>(code);
    return r;
}

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

// `text` as a tool's console prints it: the C runtime's text mode turns each
// "\n" into "\r\n".
std::string console_text(const std::string& text) {
    std::string out;
    for (const char c : text) {
        if (c == '\n') out += '\r';
        out += c;
    }
    return out;
}

// What hydra_batch and hydra_fillcompare print when the store at `db` will not
// open: the same open the tools make, under the sandbox's default settings,
// ending through app::tool_error. Empty when the open succeeds.
std::string store_open_refusal(const std::string& db) {
    try {
        hydra::app::open_store(db, hydra::core::RulesStamp::of(hydra::app::Settings{}.rules));
    } catch (const std::exception& e) {
        return console_text(hydra::app::plain_error_block(e) + "\n");
    }
    return {};
}

// hydra_batch's header line for the fill rule a run with or without
// --legacy-fills runs under, in the app's long name for the rule.
std::string fills_header_line(bool legacy_fills) {
    return std::string("Fills      : ") +
           hydra::fill_rule_name(hydra::fill_rule_for(legacy_fills),
                                 hydra::FillRuleNameStyle::Long) +
           "\r\n";
}

// What hydra_batch and hydra_fillcompare print for the bad hydra_rules.ini or
// hydra_settings.ini `file_name` names: app::ini_file_tool_text over the error
// `load` throws, the loader the tools call reading the same file. Empty when
// `load` reads the file without an error.
template <typename Error, typename Load>
std::string ini_file_refusal(std::string_view file_name, Load load) {
    try {
        load();
    } catch (const Error& e) {
        return console_text(hydra::app::ini_file_tool_text(file_name, e) + "\n");
    }
    return {};
}

// The counts hydra_fillcompare printed on its "Compared" line, checked against
// their owner: generate_fill_report over the database at `db`, opened the way
// the tool opens it, under the sandbox's default settings. The line gives the
// total before its colon, then one part per FillCompareStats field in field
// order, each starting with its count. Parts are split at ", ": the thousands
// comma group_thousands writes has no space after it, and the one part with
// no count is the tail of kInBothPhrase, which holds ", " itself.
hydra::app::fill_report::FillCompareStats printed_fill_stats(const std::string& output,
                                                             const std::string& db) {
    const std::string lead = "Compared ";
    const size_t start = output.find(lead);
    REQUIRE_MESSAGE(start != std::string::npos, "no Compared line in:\n" << output);
    const size_t end = output.find('\n', start);
    const std::string line = output.substr(start + lead.size(), end - start - lead.size());
    const size_t colon = line.find(':');
    REQUIRE(colon != std::string::npos);
    std::vector<std::string> parts = {line.substr(0, colon)};
    for (size_t i = colon + 1; i < line.size();) {
        size_t next = line.find(", ", i);
        if (next == std::string::npos) next = line.size();
        parts.push_back(line.substr(i, next - i));
        i = next + 2;
    }
    std::vector<int> counts;
    for (const std::string& part : parts) {
        const size_t first = part.find_first_not_of(' ');
        if (first == std::string::npos) continue;
        std::string digits;
        for (size_t k = first; k < part.size(); ++k) {
            const char c = part[k];
            if (std::isdigit(static_cast<unsigned char>(c))) digits += c;
            else if (c != ',') break;
        }
        if (!digits.empty()) counts.push_back(std::stoi(digits));
    }
    INFO("the Compared line: " << line);
    REQUIRE(counts.size() == 7);
    hydra::app::fill_report::FillCompareStats printed;
    printed.total = counts[0];
    printed.same = counts[1];
    printed.ch10_higher = counts[2];
    printed.ch11_higher = counts[3];
    printed.only_old = counts[4];
    printed.only_new = counts[5];
    printed.in_both = counts[6];

    const hydra::app::Settings settings{};  // the defaults the sandboxed exe reads
    const std::unique_ptr<hydra::store::RecordStore> store =
        hydra::app::open_store(db, hydra::core::RulesStamp::of(settings.rules));
    const hydra::app::fill_report::FillCompareStats owner =
        hydra::app::fill_report::generate_fill_report(*store, settings).stats;
    CHECK(printed.total == owner.total);
    CHECK(printed.same == owner.same);
    CHECK(printed.ch10_higher == owner.ch10_higher);
    CHECK(printed.ch11_higher == owner.ch11_higher);
    CHECK(printed.only_old == owner.only_old);
    CHECK(printed.only_new == owner.only_new);
    CHECK(printed.in_both == owner.in_both);
    // D52: every chart has one status, so the six parts add up to the total.
    CHECK(printed.same + printed.ch10_higher + printed.ch11_higher + printed.only_old +
              printed.only_new + printed.in_both ==
          printed.total);
    return printed;
}

// The smallest corpus .chart that has drum notes and at least two Star Power
// phrases: every tool gets a real path to work with, and each run is quick.
std::string small_chart() {
    static const std::string chosen = [] {
        std::string best;
        uintmax_t best_size = UINTMAX_MAX;
        for (const std::string& p : corpus::chart_paths()) {
            if (hydra::chart_format_of(p) != hydra::ChartFormat::Chart) continue;
            const uintmax_t size = fs::file_size(fs::u8path(p));
            if (size >= best_size) continue;
            const hydra::Song song = hydra::load_songpath(p, hydra::ChartReading{});
            if (song.is_empty() || song.sp_phrase_count() < 2) continue;
            best = p;
            best_size = size;
        }
        return best;
    }();
    return chosen;
}

// A scratch folder holding a copy of each tool and one chart folder with a
// known song name. Removed again when the test ends.
struct CliSandbox {
    fs::path dir;
    fs::path batch, fillcompare;
    fs::path songs;  // holds one chart folder, "fixture"

    explicit CliSandbox(const char* name) {
        dir = hydra::os_path(testtemp::temp_dir(std::string("cli_") + name));
        fs::remove_all(dir);
        fs::create_directories(dir);
        batch = copy_tool(HYDRA_BATCH_EXE);
        fillcompare = copy_tool(HYDRA_FILLCOMPARE_EXE);
        // The tools start only with the DLLs built beside them (see
        // hydra_use_mimalloc in CMakeLists.txt), so those travel with them.
        const fs::path built_dir = fs::u8path(HYDRA_BATCH_EXE).make_preferred().parent_path();
        for (const fs::directory_entry& e : fs::directory_iterator(built_dir))
            if (e.path().extension() == ".dll")
                fs::copy_file(e.path(), dir / e.path().filename());

        const std::string chart = small_chart();
        REQUIRE(!chart.empty());
        songs = dir / "songs";
        fs::create_directories(songs / "fixture");
        fs::copy_file(fs::u8path(chart), songs / "fixture" / "notes.chart");
        write_song_ini("CLI Fixture", "Tester");
    }
    // The chart folder's song.ini, as a chart editor saves one.
    void write_song_ini(const std::string& name, const std::string& artist) const {
        std::ofstream ini(song_ini(), std::ios::binary);
        ini << "[song]\nname = " << name << "\nartist = " << artist << "\ncharter = Nobody\n";
    }
    fs::path song_ini() const { return songs / "fixture" / "song.ini"; }
    ~CliSandbox() {
        std::error_code ec;
        fs::remove_all(dir, ec);
    }
    fs::path copy_tool(const char* built) {
        const fs::path from = fs::u8path(built).make_preferred();
        const fs::path to = dir / from.filename();
        fs::copy_file(from, to);
        return to;
    }
    std::string db(const char* name) const { return (dir / name).u8string(); }
    std::string folder() const { return songs.u8string(); }
};

}  // namespace

TEST_CASE("hydra_batch prints the app's setting words and counts, and fills a new database "
          "under the rule it ran under") {
    CliSandbox box("stamp");
    const std::string normal = box.db("ch11.db");
    RunResult r = run_exe(box.batch, {"--db", normal, box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    CHECK(contains(r.output, "Tester - CLI Fixture"));
    // The header uses the app's names and words (D114 items 9 and 23): each
    // value is the app's own text for the defaults the sandboxed exe reads.
    {
        const hydra::app::SettingsText described =
            hydra::app::describe_settings(hydra::app::Settings{}.batch_run().settings);
        CHECK(contains(r.output, "Score range: " + described.depth + "\r\n"));
        CHECK(contains(r.output, "SP cap     : " + described.cap + "\r\n"));
        CHECK(contains(r.output, "Path limit : " + described.timing + "\r\n"));
        CHECK(contains(r.output, fills_header_line(false)));
        CHECK(contains(r.output, "Early fill : " + described.early_fill + "\r\n"));
    }
    CHECK(contains(r.output, "Found 1 chart."));
    // Every number is the batch's progress, in library rows (D79); the
    // store's raw row count is not printed.
    CHECK(contains(r.output, "[1/1] "));
    // The closing counts are the app's strip words (D114 item 10).
    CHECK(contains(r.output, "\n" + hydra::app::batch_counts_text(1, 0, 0) + " in "));
    CHECK_FALSE(contains(r.output, "Store now holds"));
    {
        hydra::store::RecordStore store(normal);
        CHECK(store.record_count() == 1);
        // A run over folder arguments leaves the library alone (D79).
        CHECK(store.list_chart_library(0, -1).empty());
    }

    const std::string legacy = box.db("ch10.db");
    r = run_exe(box.batch, {"--legacy-fills", "--db", legacy, box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    CHECK(contains(r.output, fills_header_line(true)));
    CHECK(!contains(r.output, "(legacy)"));
    hydra::store::RecordStore store(legacy);
    CHECK(store.record_count() == 1);
}

// D72 item 5: a database that won't open is a run that can't start, so each
// tool prints the sentence and SQLite's text and exits 2. The text is the
// app's own for the store's open of the same file.
TEST_CASE("hydra_batch and hydra_fillcompare say why a database won't open") {
    CliSandbox box("junkdb");
    const std::string junk = box.db("junk.db");
    hydra::test::write_junk_db(junk);
    const std::string refusal = store_open_refusal(junk);
    INFO("the store's own open printed: " << refusal);
    REQUIRE(contains(refusal, hydra::app::kDatabaseOpenSentence));
    const std::string page = (box.dir / "page.html").u8string();
    const std::pair<fs::path, std::vector<std::string>> runs[] = {
        {box.batch, {"--db", junk, box.folder()}},
        {box.fillcompare, {"--db", junk, "--out", page, "--no-open"}},
    };
    for (const auto& [exe, args] : runs) {
        const RunResult r = run_exe(exe, args);
        INFO(exe.filename().u8string() << " printed: " << r.output);
        CHECK(r.exit_code == 2);
        CHECK(contains(r.output, refusal));
    }
}

TEST_CASE("hydra_batch --legacy-fills writes the tool's own hydra.db under the 1.0 rule") {
    // The sandboxed exe's default database is hydra.db beside it, in the
    // sandbox: the file the app itself would read. Each record carries its
    // fill rule in its profile, so 1.0 results may go there, as the app's
    // "1.0 fills" setting already puts them (D115 item 6).
    CliSandbox box("owndb");
    RunResult r = run_exe(box.batch, {"--legacy-fills", box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    const std::string own_db = (box.dir / "hydra.db").u8string();
    CHECK(contains(r.output, "Database   : " + own_db));
    CHECK(contains(r.output, fills_header_line(true)));

    const std::string md5 =
        hydra::app::hash_chart_file((box.songs / "fixture" / "notes.chart").u8string());
    const hydra::app::Settings settings{};  // the defaults the sandboxed exe reads
    hydra::store::RecordStore store(own_db);
    CHECK(store.record_count() == 1);
    CHECK(store.lookup_one(md5, settings.profile_with_fills(hydra::FillDeadlineRule::Ch10))
              .status == hydra::store::RecordStatus::Ready);
    CHECK(store.lookup_one(md5, settings.profile_with_fills(hydra::FillDeadlineRule::Ch11))
              .status != hydra::store::RecordStatus::Ready);
}

// A library file from before 2.2.0 stops the run with the one sentence, which
// names the file once, and the file is left as it was (D115 item 6).
TEST_CASE("hydra_batch refuses a library file too old to convert in one sentence") {
    CliSandbox box("old_library");
    const std::string old_db = box.db("old.db");
    fs::copy_file(fs::u8path(corpus::testdata_root() + "/store/v2.1.0-schema4.db"),
                  fs::u8path(old_db));
    RunResult r = run_exe(box.batch, {"--db", old_db, box.folder()});
    CHECK(r.exit_code == 2);
    CHECK_FALSE(fs::exists(fs::u8path(hydra::store::set_aside_path(old_db))));
    // The whole of what the run printed is the app's text for the store's
    // open of the same file, and that text names the file.
    const std::string refusal = store_open_refusal(old_db);
    CHECK(contains(refusal, old_db));
    CHECK(r.output == refusal);
}

// D271 item 6: every tool prints the one helper's line for an option it does
// not know, in the words the user chose (D276 item 5). Both tools here exit 2,
// and an unknown option (a removed one included, D87) stops the run before it
// opens the database.
TEST_CASE("hydra_batch and hydra_fillcompare print the one unknown-option line") {
    const std::string line = hydra::unknown_option_line("--bogus");
    CHECK(contains(line, "--bogus"));
    CliSandbox box("unknown_option_line");
    const std::string db = box.db("untouched.db");
    for (const fs::path& exe : {box.batch, box.fillcompare}) {
        RunResult r = run_exe(exe, {"--bogus", "--db", db});
        INFO(exe.u8string() << ": " << r.output);
        CHECK(r.exit_code == 2);
        CHECK(contains(r.output, console_text(line + "\n")));
        CHECK_FALSE(fs::exists(fs::u8path(db)));
    }
}

// A whole-library run is asked for by name (hydra::cli::batch_scope). The
// sandbox's settings list no folders, so a run that wrongly went ahead would
// find nothing to read and would not exit 2.
TEST_CASE("hydra_batch refuses a run with no folder and no --whole-library, and a mix of both") {
    CliSandbox box("scope_refused");
    const std::string db = box.db("untouched.db");

    RunResult bare = run_exe(box.batch, {"--db", db});
    INFO(bare.output);
    CHECK(bare.exit_code == 2);
    CHECK(contains(bare.output, hydra::cli::batch_scope_error(hydra::cli::BatchScope::Refused)));
    CHECK_FALSE(fs::exists(fs::u8path(db)));

    RunResult mixed = run_exe(box.batch, {hydra::cli::kWholeLibraryFlag.data(), "--db", db, box.folder()});
    INFO(mixed.output);
    CHECK(mixed.exit_code == 2);
    CHECK(contains(mixed.output, hydra::cli::batch_scope_error(hydra::cli::BatchScope::Ambiguous)));
    CHECK_FALSE(fs::exists(fs::u8path(db)));
}

// D268 ("Refuse, like the app"): a whole-library run saves its scan as the
// library, so a chart folder it cannot read stops it before it scans, saves or
// analyzes anything. The library and the results stay as the first run left
// them, and the run names the folder in the manual scan's sentence.
TEST_CASE("hydra_batch --whole-library refuses a chart folder that cannot be read") {
    CliSandbox box("missing_folder");
    const std::string db = box.db("library.db");
    const std::string settings_file = (box.dir / "hydra_settings.ini").u8string();
    hydra::app::Settings settings{};
    settings.chartfolders = {box.folder()};
    REQUIRE(settings.save_file(settings_file));
    RunResult first = run_exe(box.batch, {hydra::cli::kWholeLibraryFlag.data(), "--db", db});
    INFO(first.output);
    REQUIRE(first.exit_code == 0);
    int64_t records_before = 0;
    size_t library_before = 0;
    {
        hydra::store::RecordStore store(db);
        records_before = store.record_count();
        library_before = store.chart_library_cache().size();
    }
    REQUIRE(records_before == 1);
    REQUIRE(library_before == 1);
    const std::string saved_title = "CLI Fixture";  // CliSandbox's song.ini name
    const auto library_titles = [&db] {
        hydra::store::RecordStore store(db);
        std::vector<std::string> titles;
        for (const hydra::store::ChartLibraryEntry& e : store.list_chart_library(0, -1))
            titles.push_back(e.title);
        return titles;
    };
    REQUIRE(library_titles() == std::vector<std::string>{saved_title});

    // The good folder's song.ini gets a new, longer name, so its size and time
    // no longer match the saved scan: any scan re-reads it and saves the new
    // name as the library row's title. The row keeping the old title is what
    // shows the refused run scanned and saved nothing.
    const std::string renamed_title = "CLI Fixture Renamed";
    box.write_song_ini(renamed_title, "Tester");
    const std::string missing = (box.dir / "gone").u8string();
    REQUIRE_FALSE(fs::exists(fs::u8path(missing)));
    settings.chartfolders.push_back(missing);
    REQUIRE(settings.save_file(settings_file));
    RunResult r =
        run_exe(box.batch, {hydra::cli::kWholeLibraryFlag.data(), "--redo", "--db", db});
    INFO(r.output);
    CHECK(r.exit_code == 2);
    CHECK(contains(r.output, hydra::app::library_scan_blocked_notice(
                                 missing, hydra::app::ScanAsker::User)));
    {
        hydra::store::RecordStore store(db);
        CHECK(store.record_count() == records_before);
        CHECK(store.chart_library_cache().size() == library_before);
    }
    CHECK(library_titles() == std::vector<std::string>{saved_title});

    // A run over folders passed by hand never saves the library, so the
    // settings' missing folder does not stop it.
    RunResult by_hand = run_exe(box.batch, {"--db", db, box.folder()});
    INFO(by_hand.output);
    CHECK(by_hand.exit_code == 0);

    // With the missing folder gone from the settings, a whole-library run
    // scans again and saves the renamed title, so the title check above can
    // tell a run that scanned from one that did not.
    settings.chartfolders = {box.folder()};
    REQUIRE(settings.save_file(settings_file));
    RunResult rescan = run_exe(box.batch, {hydra::cli::kWholeLibraryFlag.data(), "--db", db});
    INFO(rescan.output);
    CHECK(rescan.exit_code == 0);
    CHECK(library_titles() == std::vector<std::string>{renamed_title});
}

// D114 item 11: a bad hydra_rules.ini prints the tools' sentence and then the
// loader's line, and the run stops before it opens the database. The expected
// text is the rules loader's own refusal of the same file.
TEST_CASE("hydra_batch names a bad hydra_rules.ini and stops") {
    CliSandbox box("bad_rules");
    const fs::path rules_file = box.dir / std::string(hydra::app::kRulesFileName);
    {
        std::ofstream rules(rules_file, std::ios::binary);
        rules << "nope = 1\n";
    }
    const std::string refusal = ini_file_refusal<hydra::app::RulesFileError>(
        hydra::app::kRulesFileName, [&] { (void)hydra::app::load_rules_file(rules_file); });
    INFO("the rules loader's refusal: " << refusal);
    REQUIRE(contains(refusal, "nope"));
    const std::string db = box.db("untouched.db");
    RunResult r = run_exe(box.batch, {"--db", db, box.folder()});
    INFO(r.output);
    CHECK(r.exit_code == 2);
    CHECK(contains(r.output, refusal));
    CHECK_FALSE(fs::exists(fs::u8path(db)));
}

namespace {

// D270 call 4: a bad hydra_settings.ini prints the tools' sentence and then
// the loader's line, and the run stops before it opens the database. The file
// is the one the app saves at its defaults, with its sp_cap line's value
// changed to text; the expected text is the settings loader's own refusal of
// that file.
std::string write_bad_settings(const CliSandbox& box) {
    const std::string path = (box.dir / std::string(hydra::app::kSettingsFileName)).u8string();
    REQUIRE(hydra::app::Settings{}.save_file(path));
    std::string text;
    {
        std::ifstream in(fs::u8path(path), std::ios::binary);
        text.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }
    // save_file writes each key as "key=value" on its own CRLF line.
    const std::string cap_line =
        "\r\nsp_cap=" + std::to_string(hydra::app::Settings{}.sp_cap) + "\r\n";
    const size_t at = text.find(cap_line);
    REQUIRE_MESSAGE(at != std::string::npos, "no sp_cap line in:\n" << text);
    text.replace(at, cap_line.size(), "\r\nsp_cap=abc\r\n");
    {
        std::ofstream out(fs::u8path(path), std::ios::binary);
        out << text;
    }
    const std::string refusal = ini_file_refusal<hydra::app::SettingsFileError>(
        hydra::app::kSettingsFileName, [&] { (void)hydra::app::Settings::load_file(path); });
    INFO("the settings loader's refusal: " << refusal);
    REQUIRE(contains(refusal, "abc"));
    return refusal;
}

}  // namespace

TEST_CASE("hydra_batch names a bad hydra_settings.ini and stops") {
    CliSandbox box("bad_settings");
    const std::string refusal = write_bad_settings(box);
    const std::string db = box.db("untouched.db");
    RunResult r = run_exe(box.batch, {"--db", db, box.folder()});
    INFO(r.output);
    CHECK(r.exit_code == 2);
    CHECK(contains(r.output, refusal));
    CHECK_FALSE(fs::exists(fs::u8path(db)));
}

TEST_CASE("hydra_fillcompare names a bad hydra_settings.ini and stops") {
    CliSandbox box("bad_settings_fc");
    const std::string refusal = write_bad_settings(box);
    const std::string db = box.db("untouched.db");
    const std::string page = (box.dir / "page.html").u8string();
    RunResult r = run_exe(box.fillcompare, {"--db", db, "--out", page, "--no-open"});
    INFO(r.output);
    CHECK(r.exit_code == 2);
    CHECK(contains(r.output, refusal));
    CHECK_FALSE(fs::exists(fs::u8path(db)));
    CHECK_FALSE(fs::exists(fs::u8path(page)));
}

TEST_CASE("hydra_batch writes a 1.0 run and a 1.1 run into one database") {
    // Each record carries its fill rule in its profile, so one file takes
    // both runs and each reads back under its own rule (D115 item 6).
    CliSandbox box("both_rules");
    const std::string db = box.db("both.db");
    RunResult r = run_exe(box.batch, {"--db", db, box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    r = run_exe(box.batch, {"--legacy-fills", "--db", db, box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    // The 1.1 record is no 1.0 result, so the second run analyzes the chart:
    // the closing counts, in the app's strip words, say 1 analyzed.
    CHECK(contains(r.output, "\n" + hydra::app::batch_counts_text(1, 0, 0) + " in "));

    const std::string md5 =
        hydra::app::hash_chart_file((box.songs / "fixture" / "notes.chart").u8string());
    {
        const hydra::app::Settings settings{};  // the defaults the sandboxed exe reads
        hydra::store::RecordStore store(db);
        CHECK(store.record_count() == 2);
        for (hydra::FillDeadlineRule rule :
             {hydra::FillDeadlineRule::Ch10, hydra::FillDeadlineRule::Ch11}) {
            INFO(hydra::fill_rule_name(rule, hydra::FillRuleNameStyle::Long));
            CHECK(store.lookup_one(md5, settings.profile_with_fills(rule)).status ==
                  hydra::store::RecordStatus::Ready);
        }
        // A library row, as a scan leaves one: the comparison covers library
        // charts only (D92).
        hydra::test::name_chart(store, md5, "CLI Fixture", "Tester", "Nobody");
    }
    // hydra_fillcompare reads the one file under each rule: the one library
    // chart has a record under both, so none is under one rule only.
    r = run_exe(box.fillcompare,
                {"--db", db, "--out", (box.dir / "both.html").u8string(), "--no-open"});
    INFO(r.output);
    CHECK(r.exit_code == 0);
    const hydra::app::fill_report::FillCompareStats stats = printed_fill_stats(r.output, db);
    CHECK(stats.total == 1);
    CHECK(stats.only_old == 0);
    CHECK(stats.only_new == 0);
}

TEST_CASE("hydra_batch reuses the GUI's scan cache") {
    CliSandbox box("cache");
    const std::string db = box.db("cached.db");
    {
        // The library a GUI scan saves (Scan library: discover_charts, then
        // save_scan_as_library).
        auto [items, errors] = hydra::app::discover_charts({box.folder()});
        REQUIRE(items.size() == 1);
        hydra::store::RecordStore store(db);
        REQUIRE_FALSE(hydra::app::save_scan_as_library(store, items).has_value());
    }
    // song.ini then gets a new name of the same length, and its time is put
    // back, so the files' sizes and times match the scan's. A scan that reads
    // the cache keeps the saved name; one that re-reads song.ini gets the new.
    const fs::file_time_type saved_time = fs::last_write_time(box.song_ini());
    box.write_song_ini("CLI Renamed", "Tester");
    fs::last_write_time(box.song_ini(), saved_time);

    RunResult r = run_exe(box.batch, {"--db", db, box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    CHECK(contains(r.output, "Tester - CLI Fixture"));
    CHECK(!contains(r.output, "CLI Renamed"));
}

TEST_CASE("hydra_batch prints an artist made only of tags as (unknown)") {
    // D50 item 5: the progress line cleans the artist by the title's rule.
    // The artist is the chart folder's own song.ini line, as a chart editor
    // saves rich-text tags, and the run scans it.
    CliSandbox box("tagartist");
    box.write_song_ini("CLI Fixture", hydra::test::kTagOnlyTitle);
    const std::string shown_artist = hydra::display_artist(hydra::test::kTagOnlyTitle);
    REQUIRE(shown_artist == hydra::kUnknownTitle);

    RunResult r = run_exe(box.batch, {"--db", box.db("tags.db"), box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    CHECK(contains(r.output, shown_artist + " - CLI Fixture"));
    CHECK(!contains(r.output, "<b>"));
}

TEST_CASE("hydra_batch names the cap with the one count rule") {
    // The sandboxed exe reads its SP cap from hydra_settings.ini beside it.
    CliSandbox box("cap");
    // The header's cap is describe_settings' words for the cap the run read.
    // hydra::counted, which describe_settings names the cap through, writes
    // the singular at exactly 1 and a thousands comma from 1,000 (core/model.h),
    // so these two caps are its two edge cases. The settings file takes any
    // cap from 1 up: its sp_cap key has no ceiling.
    constexpr int kSingularCap = 1;
    constexpr int kFirstGroupedCap = 1000;
    for (const int cap : {kSingularCap, kFirstGroupedCap}) {
        hydra::app::Settings settings{};
        settings.sp_cap = cap;
        REQUIRE(settings.save_file((box.dir / "hydra_settings.ini").u8string()));
        const std::string db_name = "cap" + std::to_string(cap) + ".db";
        RunResult r = run_exe(box.batch, {"--db", box.db(db_name.c_str()), box.folder()});
        INFO("cap " << cap << " printed: " << r.output);
        REQUIRE(r.exit_code == 0);
        const std::string words =
            hydra::app::describe_settings(settings.batch_run().settings).cap;
        CHECK(contains(r.output, "SP cap     : " + words + "\r\n"));
    }
}

TEST_CASE("hydra_fillcompare reads the tool's own hydra.db without --db") {
    // The sandboxed exe's default database is hydra.db beside it, the file
    // hydra_batch writes without --db (app::db_path).
    CliSandbox box("fillcompare");
    REQUIRE(run_exe(box.batch, {"--legacy-fills", box.folder()}).exit_code == 0);
    REQUIRE(run_exe(box.batch, {box.folder()}).exit_code == 0);
    // A library row for the chart, as a scan leaves one: the comparison covers
    // library charts only (D92), and a batch with folder arguments leaves the
    // library alone.
    {
        const std::string md5 =
            hydra::app::hash_chart_file((box.songs / "fixture" / "notes.chart").u8string());
        hydra::store::RecordStore store((box.dir / "hydra.db").u8string());
        hydra::test::name_chart(store, md5, "CLI Fixture", "Tester", "Nobody");
    }

    // The counts the tool prints are the owner's for the hydra.db beside it:
    // a tool that read some other file would not find the one library chart
    // under both rules.
    const fs::path page = box.dir / "compare.html";
    RunResult r = run_exe(box.fillcompare, {"--out", page.u8string(), "--no-open"});
    INFO(r.output);
    CHECK(r.exit_code == 0);
    const hydra::app::fill_report::FillCompareStats stats =
        printed_fill_stats(r.output, (box.dir / "hydra.db").u8string());
    CHECK(stats.total == 1);
    CHECK(stats.only_old == 0);
    CHECK(stats.only_new == 0);
    CHECK(fs::exists(page));

    // A database with nothing under these settings: the settings sentence,
    // with no fill rule in it, since the page compares both.
    RunResult empty = run_exe(box.fillcompare,
                              {"--db", box.db("empty.db"), "--out",
                               (box.dir / "empty.html").u8string(), "--no-open"});
    INFO(empty.output);
    CHECK(empty.exit_code == 1);
    {
        const hydra::app::Settings defaults{};  // what the sandboxed exe reads
        const hydra::store::Profile profile =
            defaults.profile_with_fills(hydra::FillDeadlineRule::Ch11);
        CHECK(contains(empty.output,
                       hydra::app::report::nothing_under_settings(profile.chartmode,
                                                                  profile.cap.exact)));
    }
}

TEST_CASE("hydra_fillcompare refuses --old and --new and names --db") {
    CliSandbox box("fillcompare_old_flags");
    const std::string db = box.db("untouched.db");
    for (const char* flag : {"--old", "--new"}) {
        RunResult r = run_exe(box.fillcompare, {flag, db, "--no-open"});
        INFO(r.output);
        CHECK(r.exit_code == 2);
        // The refusal names the flag it refused and the one it takes instead.
        CHECK(contains(r.output, flag));
        CHECK(contains(r.output, "--db"));
        CHECK_FALSE(contains(r.output, hydra::unknown_option_line(flag)));
    }
    CHECK_FALSE(fs::exists(fs::u8path(db)));
}

TEST_CASE("hydra_fillcompare compares both rules out of one database") {
    // The app's "1.0 fills" setting keeps both rules' results in one file.
    CliSandbox box("fillcompare_one");
    const std::string db = box.db("both.db");
    {
        const std::string chart = (box.songs / "fixture" / "notes.chart").u8string();
        const std::string md5 = hydra::app::hash_chart_file(chart);
        hydra::store::RecordStore store(db);
        hydra::test::name_chart(store, md5, "CLI Fixture", "Tester", "Nobody");
        for (bool legacy : {false, true}) {
            hydra::app::Settings settings{};  // the defaults the sandboxed exe reads
            settings.legacy_fills = legacy;
            hydra::app::AnalysisResult ar =
                hydra::app::analyze_chart_file(chart, settings.to_analysis_settings());
            store.save(hydra::store::prepare_record(md5, settings.profile(), ar.record));
        }
        REQUIRE(store.record_count() == 2);
    }

    RunResult r = run_exe(box.fillcompare,
                          {"--db", db, "--out", (box.dir / "one.html").u8string(), "--no-open"});
    INFO(r.output);
    CHECK(r.exit_code == 0);
    // The one library chart was analyzed under each rule, and a corpus chart
    // with two Star Power phrases keeps a path under each, so it is scored on
    // both sides.
    const hydra::app::fill_report::FillCompareStats one = printed_fill_stats(r.output, db);
    CHECK(one.total == 1);
    CHECK(one.same + one.ch10_higher + one.ch11_higher == 1);

    // D52, worded by D114 item 26: a second chart with a record under both
    // rules but a score under 1.1 only is counted as "in both, not both scored".
    {
        const std::string chart = (box.songs / "fixture" / "notes.chart").u8string();
        const std::string one_sided = "dd44ee55ff6677889900aa11bb22cc33";
        hydra::store::RecordStore store(db);
        hydra::app::Settings settings{};
        hydra::app::AnalysisResult ar =
            hydra::app::analyze_chart_file(chart, settings.to_analysis_settings());
        hydra::test::name_chart(store, one_sided, "One Sided", "Tester", "Nobody");
        store.save(hydra::store::prepare_record(one_sided, settings.profile(), ar.record));
        settings.legacy_fills = true;
        // A Ready record under 1.0: no paths, so no score.
        hydra::test::save_batch_record(store, one_sided, settings.profile());
    }
    RunResult two = run_exe(box.fillcompare,
                            {"--db", db, "--out", (box.dir / "two.html").u8string(), "--no-open"});
    INFO(two.output);
    CHECK(two.exit_code == 0);
    // printed_fill_stats checks that the parts add up to the total (D52).
    const hydra::app::fill_report::FillCompareStats both = printed_fill_stats(two.output, db);
    CHECK(both.total == 2);
    CHECK(both.in_both == 1);
    CHECK(both.same + both.ch10_higher + both.ch11_higher == 1);
}

// D308 item 1: a typo in --ms, --efill or --depth stops the run with a line
// naming the flag, where it would otherwise price the chart under a number
// nobody typed. Each run names a real chart and a command, so a flag read
// loosely runs to the end and exits 0.
TEST_CASE("hydra_replay refuses --ms, --efill and --depth values that are not whole numbers") {
    CliSandbox box("replay_flags");
    const fs::path replay = box.copy_tool(HYDRA_REPLAY_EXE);
    const std::string chart = (box.songs / "fixture" / "notes.chart").u8string();
    const std::string out = (box.dir / "dump.json").u8string();

    // Each refusal names the flag and the text it refused.
    const struct {
        const char* flag;
        const char* value;
    } refused[] = {
        {"--ms", "abc"},
        {"--efill", "abc"},
        {"--depth", "1x"},
        {"--depth", "99999999999"},
    };
    for (const auto& bad : refused) {
        const RunResult r = run_exe(replay, {"dump", "--chart", chart, bad.flag, bad.value, "--out", out});
        INFO(bad.flag << " " << bad.value << " printed: " << r.output);
        CHECK(r.exit_code == 2);
        CHECK(contains(r.output, bad.flag));
        CHECK(contains(r.output, bad.value));
        CHECK_FALSE(fs::exists(fs::u8path(out)));
        std::error_code ec;
        fs::remove(fs::u8path(out), ec);  // a loose read writes it; the next run starts clean
    }

    // Whole numbers still run, and "off" still turns the limit off. A number
    // past a setting's range is no typo: settings_from pulls it in.
    for (const std::vector<std::string>& flags :
         {std::vector<std::string>{"--ms", "10", "--depth", "1"},
          std::vector<std::string>{"--ms", "off", "--depth", "-1"},
          std::vector<std::string>{"--efill", "85"},
          std::vector<std::string>{"--efill", "900"}}) {
        std::vector<std::string> args = {"dump", "--chart", chart, "--out", out};
        args.insert(args.end(), flags.begin(), flags.end());
        const RunResult r = run_exe(replay, args);
        INFO(flags[1] << " printed: " << r.output);
        CHECK(r.exit_code == 0);
    }
}

// The old skipped-fill decline option is gone: hydra_replay sends any flag it
// does not know to unknown_option_line and exits 2, as it does for a typo.
TEST_CASE("hydra_replay refuses --decline as an unknown option") {
    CliSandbox box("replay_decline");
    const fs::path replay = box.copy_tool(HYDRA_REPLAY_EXE);
    const std::string chart = (box.songs / "fixture" / "notes.chart").u8string();
    const std::string out = (box.dir / "dump.json").u8string();
    const RunResult r = run_exe(replay, {"dump", "--chart", chart, "--decline", "kick_only", "--out", out});
    INFO("printed: " << r.output);
    CHECK(r.exit_code == 2);
    CHECK(contains(r.output, hydra::unknown_option_line("--decline")));
    CHECK_FALSE(fs::exists(fs::u8path(out)));
}

// `hydra_replay score` writes each window's two times beside its ticks, read
// through the chart's own tempo map, so a reader with no chord on the end
// still has the end's time. The chart is the sandbox's corpus chart. The
// window starts halfway through the tempo section before the chart's first
// change of BPM and ends halfway through the section that change starts, so a
// time read at one flat tempo misses the end's time.
TEST_CASE("hydra_replay score writes act_ms and deact_ms for each window") {
    CliSandbox box("replay_score_ms");
    const fs::path replay = box.copy_tool(HYDRA_REPLAY_EXE);
    const std::string chart = (box.songs / "fixture" / "notes.chart").u8string();
    const hydra::Song song = hydra::load_songpath(chart, hydra::ChartReading{});
    const hydra::SongTiming& timing = song.timing();
    const int64_t resolution = timing.tick_resolution();
    const std::vector<int64_t>& starts = timing.ms_index().change_ticks();

    // The time of a tick is the chart's own tempo map's: SongTiming::timecode.
    const auto ms_at = [&](int64_t tick) { return timing.timecode(tick).ms(); };
    size_t change = 1;
    while (change < starts.size() &&
           timing.tempo_at(starts[change]) == timing.tempo_at(starts[change - 1]))
        ++change;
    INFO("the chart's tempo sections: " << starts.size());
    REQUIRE_MESSAGE(change < starts.size(), "the sandbox's chart has no change of BPM: " << chart);
    const int64_t change_tick = starts[change];
    const int64_t act_tick = (starts[change - 1] + change_tick) / 2;
    const int64_t section_end =
        change + 1 < starts.size() ? starts[change + 1] : change_tick + 2 * resolution;
    const int64_t deact_tick = (change_tick + section_end) / 2;
    REQUIRE(deact_tick > change_tick);
    const double bpm_before = timing.tempo_at(act_tick);
    const double bpm_after = timing.tempo_at(deact_tick);
    REQUIRE(bpm_before != bpm_after);
    // The end's time read at the window's opening tempo all the way.
    const double flat_deact_ms = ms_at(act_tick) + static_cast<double>(deact_tick - act_tick) /
                                                       static_cast<double>(resolution) *
                                                       60000.0 / bpm_before;

    const std::string out = (box.dir / "score.json").u8string();
    const std::string acts = std::to_string(act_tick) + ":" + std::to_string(deact_tick);
    const RunResult r = run_exe(replay, {"score", "--chart", chart, "--acts", acts, "--out", out});
    INFO("hydra_replay score --acts " << acts << " printed: " << r.output);
    REQUIRE(r.exit_code == 0);

    std::ifstream in(fs::u8path(out), std::ios::binary);
    REQUIRE(in);
    const nlohmann::json doc = nlohmann::json::parse(in);
    REQUIRE(doc["activations"].size() == 1);
    const nlohmann::json& a = doc["activations"][0];
    CHECK(a["act_tick"].get<int64_t>() == act_tick);
    CHECK(a["deact_tick"].get<int64_t>() == deact_tick);
    CHECK(a["act_ms"].get<double>() == doctest::Approx(ms_at(act_tick)));
    CHECK(a["deact_ms"].get<double>() == doctest::Approx(ms_at(deact_tick)));
    CHECK(a["deact_ms"].get<double>() != doctest::Approx(flat_deact_ms));
}

// The network code lives in hydra_dm, which only the GUI links (CMakeLists.txt),
// so the console tools import no WinHTTP. The import table stores DLL names in
// ASCII in any case, so a case-blind search of the exe's bytes finds one;
// kernel32.dll, which every exe imports, shows the search can see them.
TEST_CASE("hydra_batch and hydra_fillcompare import no winhttp.dll") {
    const auto holds = [](const std::vector<uint8_t>& bytes, const std::string& name) {
        return std::search(bytes.begin(), bytes.end(), name.begin(), name.end(),
                           [](uint8_t b, char c) {
                               return std::tolower(b) == std::tolower(static_cast<unsigned char>(c));
                           }) != bytes.end();
    };
    for (const char* exe : {HYDRA_BATCH_EXE, HYDRA_FILLCOMPARE_EXE}) {
        INFO(exe);
        const std::vector<uint8_t> bytes = hydra::read_file_bytes(exe);
        CHECK(holds(bytes, "kernel32.dll"));
        CHECK_FALSE(holds(bytes, "winhttp.dll"));
    }
}
