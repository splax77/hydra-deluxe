// End-to-end tests for the two console tools (src/cli/). Each test copies
// the built exes into a fresh temp folder and runs them there, because the
// tools read hydra_settings.ini and hydra_rules.ini from their own folder: a
// folder with neither gives the app's defaults, whatever the developer's build
// folder holds. The exe paths come from CMakeLists.txt, which also builds the
// tools before the tests.

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
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/user_messages.h"  // kDatabaseOpenSentence
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

#if !defined(HYDRA_BATCH_EXE) || !defined(HYDRA_FILLCOMPARE_EXE)
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
        std::ofstream ini(songs / "fixture" / "song.ini", std::ios::binary);
        ini << "[song]\nname = CLI Fixture\nartist = Tester\ncharter = Nobody\n";
    }
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

TEST_CASE("hydra_batch fills a new database under the rule it ran under") {
    CliSandbox box("stamp");
    const std::string normal = box.db("ch11.db");
    RunResult r = run_exe(box.batch, {"--db", normal, box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    CHECK(contains(r.output, "Tester - CLI Fixture"));
    // The header uses the app's names and words (D114 items 9 and 23).
    CHECK(contains(r.output, "Score range: 4 scores"));
    CHECK(contains(r.output, "SP cap     : 4 bars"));
    CHECK(contains(r.output, "Path limit : 10 ms"));
    CHECK(contains(r.output, "Fills      : Clone Hero 1.1"));
    CHECK(contains(r.output, "Found 1 chart."));
    // Every number is the batch's progress, in library rows (D79); the
    // store's raw row count is not printed.
    CHECK(contains(r.output, "[1/1] "));
    // The closing counts are the app's strip words (D114 item 10).
    CHECK(contains(r.output,
                   "\n1 analyzed \xC2\xB7 0 failed \xC2\xB7 0 skipped (already had a result) in "));
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
    CHECK(contains(r.output, "Fills      : Clone Hero 1.0"));
    CHECK(!contains(r.output, "(legacy)"));
    hydra::store::RecordStore store(legacy);
    CHECK(store.record_count() == 1);
}

// D72 item 5: a database that won't open is a run that can't start, so each
// tool prints the sentence and SQLite's text and exits 2.
TEST_CASE("hydra_batch and hydra_fillcompare say why a database won't open") {
    CliSandbox box("junkdb");
    const std::string junk = box.db("junk.db");
    hydra::test::write_junk_db(junk);
    const std::string page = (box.dir / "page.html").u8string();
    const std::pair<fs::path, std::vector<std::string>> runs[] = {
        {box.batch, {"--db", junk, box.folder()}},
        {box.fillcompare, {"--db", junk, "--out", page, "--no-open"}},
    };
    for (const auto& [exe, args] : runs) {
        const RunResult r = run_exe(exe, args);
        INFO(exe.filename().u8string() << " printed: " << r.output);
        CHECK(r.exit_code == 2);
        // Two checks, because the console's text mode turns each "\n" into "\r\n".
        CHECK(contains(r.output, hydra::app::kDatabaseOpenSentence));
        CHECK(contains(r.output, "sqlite exec failed: file is not a database"));
    }
}

TEST_CASE("hydra_batch --legacy-fills writes the tool's own hydra.db under the 1.0 rule") {
    // The sandboxed exe's default database is hydra.db beside it, in the
    // sandbox: the file the app itself would read. Each record carries its
    // fill rule in its profile, so 1.0 results may go there, as the app's
    // "1.0 fills" setting already puts them (D115 task A1-6d).
    CliSandbox box("owndb");
    RunResult r = run_exe(box.batch, {"--legacy-fills", box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    const std::string own_db = (box.dir / "hydra.db").u8string();
    CHECK(contains(r.output, "Database   : " + own_db));
    CHECK(contains(r.output, "Fills      : Clone Hero 1.0"));

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
// names the file once, and the file is left as it was (D115 task A1-6d).
TEST_CASE("hydra_batch refuses a library file too old to convert in one sentence") {
    CliSandbox box("old_library");
    const std::string old_db = box.db("old.db");
    fs::copy_file(fs::u8path(corpus::testdata_root() + "/store/v2.1.0-schema4.db"),
                  fs::u8path(old_db));
    RunResult r = run_exe(box.batch, {"--db", old_db, box.folder()});
    CHECK(r.exit_code == 2);
    // The whole of what the run printed; the console's text mode turns the
    // "\n" into "\r\n".
    CHECK(r.output == "The library file '" + old_db +
                          "' is from a Hydra version older than 2.2.0, and opening it in "
                          "Hydra sets it aside and starts a fresh library.\r\n");
    CHECK_FALSE(fs::exists(fs::u8path(hydra::store::set_aside_path(old_db))));
}

// An option hydra_batch doesn't know (a removed one included, D87) stops the
// run before it opens the database.
TEST_CASE("hydra_batch refuses an unknown option") {
    CliSandbox box("unknown_option");
    const std::string db = box.db("untouched.db");
    RunResult r = run_exe(box.batch, {"--no-such-option", "--db", db});
    INFO(r.output);
    CHECK(r.exit_code == 2);
    CHECK(contains(r.output, "Unknown option: --no-such-option"));
    CHECK_FALSE(fs::exists(fs::u8path(db)));
}

// D114 item 11: a bad hydra_rules.ini prints the tools' sentence and then the
// loader's line, and the run stops before it opens the database.
TEST_CASE("hydra_batch names a bad hydra_rules.ini and stops") {
    CliSandbox box("bad_rules");
    {
        std::ofstream rules(box.dir / "hydra_rules.ini", std::ios::binary);
        rules << "nope = 1\n";
    }
    const std::string db = box.db("untouched.db");
    RunResult r = run_exe(box.batch, {"--db", db, box.folder()});
    INFO(r.output);
    CHECK(r.exit_code == 2);
    // Two checks, because the console's text mode turns each "\n" into "\r\n".
    CHECK(contains(r.output, "hydra_rules.ini has an error:"));
    CHECK(contains(r.output, "hydra_rules.ini: unknown key \"nope\""));
    CHECK_FALSE(fs::exists(fs::u8path(db)));
}

TEST_CASE("hydra_batch writes a 1.0 run and a 1.1 run into one database") {
    // Each record carries its fill rule in its profile, so one file takes
    // both runs and each reads back under its own rule (D115 task A1-6c).
    CliSandbox box("both_rules");
    const std::string db = box.db("both.db");
    RunResult r = run_exe(box.batch, {"--db", db, box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    r = run_exe(box.batch, {"--legacy-fills", "--db", db, box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    // The 1.1 record is no 1.0 result, so the second run analyzes the chart.
    CHECK(contains(r.output, "\n1 analyzed \xC2\xB7 0 failed \xC2\xB7 0 skipped"));

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
    // hydra_fillcompare reads the one file under each rule.
    r = run_exe(box.fillcompare,
                {"--db", db, "--out", (box.dir / "both.html").u8string(), "--no-open"});
    INFO(r.output);
    CHECK(r.exit_code == 0);
    CHECK(contains(r.output, "Compared 1 chart:"));
    CHECK(contains(r.output, "0 only under 1.0, 0 only under 1.1"));
}

TEST_CASE("hydra_batch reuses the GUI's scan cache") {
    CliSandbox box("cache");
    const std::string db = box.db("cached.db");
    {
        auto [items, errors] = hydra::app::discover_charts({box.folder()});
        REQUIRE(items.size() == 1);
        // The row a GUI scan writes, with only its title changed. The file
        // sizes and times still match, so a scan that reads the cache takes
        // this title; one that re-reads song.ini gets "CLI Fixture".
        hydra::store::RecordStore store(db);
        store.rebuild_chart_library({{items[0].md5, "Title From Cache", items[0].artist,
                                      items[0].charter, items[0].notespath,
                                      items[0].rootfolder, items[0].sig}});
    }

    RunResult r = run_exe(box.batch, {"--db", db, box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    CHECK(contains(r.output, "Tester - Title From Cache"));
    CHECK(!contains(r.output, "CLI Fixture"));
}

TEST_CASE("hydra_batch prints an artist made only of tags as (unknown)") {
    // D50 item 5: the progress line cleans the artist by the title's rule.
    // The cached scan row carries the artist, as in the scan-cache case.
    CliSandbox box("tagartist");
    const std::string db = box.db("tags.db");
    {
        auto [items, errors] = hydra::app::discover_charts({box.folder()});
        REQUIRE(items.size() == 1);
        hydra::store::RecordStore store(db);
        store.rebuild_chart_library({{items[0].md5, items[0].title, hydra::test::kTagOnlyTitle,
                                      items[0].charter, items[0].notespath,
                                      items[0].rootfolder, items[0].sig}});
    }

    RunResult r = run_exe(box.batch, {"--db", db, box.folder()});
    INFO(r.output);
    REQUIRE(r.exit_code == 0);
    CHECK(contains(r.output, "(unknown) - CLI Fixture"));
    CHECK(!contains(r.output, "<b>"));
}

TEST_CASE("hydra_batch names the cap with the one count rule") {
    // The sandboxed exe reads its SP cap from hydra_settings.ini beside it.
    CliSandbox box("cap");
    auto header_at_cap = [&](int cap, const char* db_name) {
        hydra::app::Settings settings{};
        settings.sp_cap = cap;
        REQUIRE(settings.save_file((box.dir / "hydra_settings.ini").u8string()));
        RunResult r = run_exe(box.batch, {"--db", box.db(db_name), box.folder()});
        INFO(r.output);
        REQUIRE(r.exit_code == 0);
        return r.output;
    };
    const std::string at_one = header_at_cap(1, "cap1.db");
    CHECK(contains(at_one, "SP cap     : 1 bar"));
    CHECK(!contains(at_one, "SP cap     : 1 bars"));
    CHECK(contains(header_at_cap(1000, "cap1000.db"), "SP cap     : 1,000 bars"));
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

    const fs::path page = box.dir / "compare.html";
    RunResult r = run_exe(box.fillcompare, {"--out", page.u8string(), "--no-open"});
    INFO(r.output);
    CHECK(r.exit_code == 0);
    CHECK(contains(r.output, "Compared 1 chart:"));
    CHECK(contains(r.output, "0 only under 1.0, 0 only under 1.1"));
    CHECK(!contains(r.output, "Warning"));
    CHECK(fs::exists(page));

    // A database with nothing under these settings: the settings sentence,
    // with no fill rule in it, since the page compares both.
    RunResult empty = run_exe(box.fillcompare,
                              {"--db", box.db("empty.db"), "--out",
                               (box.dir / "empty.html").u8string(), "--no-open"});
    INFO(empty.output);
    CHECK(empty.exit_code == 1);
    CHECK(!contains(empty.output, "Warning"));
    CHECK(contains(empty.output,
                   "Nothing is analyzed under these settings (Expert Pro Drums, 2x Bass, "
                   "SP cap 4). Analyze with these settings, or change them."));
    CHECK(!contains(empty.output, "No records to compare"));
}

TEST_CASE("hydra_fillcompare refuses --old and --new and names --db") {
    CliSandbox box("fillcompare_old_flags");
    const std::string db = box.db("untouched.db");
    for (const char* flag : {"--old", "--new"}) {
        RunResult r = run_exe(box.fillcompare, {flag, db, "--no-open"});
        INFO(r.output);
        CHECK(r.exit_code == 2);
        CHECK(contains(r.output, std::string(flag) + " is gone"));
        CHECK(contains(r.output, "--db <path>"));
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
    CHECK(contains(r.output, "Compared 1 chart:"));
    CHECK(contains(r.output, "0 only under 1.0, 0 only under 1.1, 0 in both, not both scored"));
    CHECK(!contains(r.output, "Warning"));

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
    // The parts add up to the total: 1 same + 1 in both, not both scored.
    CHECK(contains(two.output, "Compared 2 charts: 1 same, 0 1.0 higher, 0 1.1 higher, "
                               "0 only under 1.0, 0 only under 1.1, 1 in both, not both scored"));
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
