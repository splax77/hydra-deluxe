// The self-test of hydra_precheck as a whole: the exe runs as a child process
// on a fixture repo in a temp folder, where every planted item is known, and
// on two first-review ranges of this repository's own history.
//
// The fixture's main branch holds one test helper, one ADR that decides a
// 45-second deadline, two src/ files, and a copy of this repository's scan
// rule files and baseline, so the rows the exe reads are today's. The feature
// branch plants nine items, one or more per check, beside lines each check
// must pass. Each check, turned off, must lose every item it plants; the list
// of checks is the exe's own (--list-checks), so a new check with no planted
// item fails by name.
#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "core/unknown_option.h"
#include "core/winstr.h"
#include "precheck/git_text.h"
#include "precheck/items.h"
#include "precheck/range.h"
#include "precheck_fixture.h"
#include "single_owner/engine.h"
#include "source_tree.h"
#include "temp_util.h"

#ifndef HYDRA_PRECHECK_EXE
#error "HYDRA_PRECHECK_EXE must be defined (see CMakeLists.txt)"
#endif

namespace fs = std::filesystem;
using namespace precheck_fixture;

namespace {

// One exe run: its exit code, its stdout split into lines, and its stderr.
struct SelfTestRun {
    int code = -1;
    std::vector<std::string> lines;
    std::string err;
};

SelfTestRun precheck_on(const std::string& repo, std::vector<std::string> args) {
    args.insert(args.begin(), {HYDRA_PRECHECK_EXE, "--repo", repo});
    precheck::ChildProcess child(args);
    std::istringstream text(child.read_all());
    SelfTestRun run;
    for (std::string line; std::getline(text, line);) run.lines.push_back(line);
    run.code = child.wait(run.err);
    return run;
}

// The item lines of a run: each starts with one of the exe's kind letters.
std::vector<std::string> item_lines(const SelfTestRun& run) {
    const std::string& kinds = precheck::kind_order();
    std::vector<std::string> items;
    for (const std::string& l : run.lines)
        if (l.size() > 1 && l[1] == ' ' && kinds.find(l[0]) != std::string::npos) items.push_back(l);
    return items;
}

// A range run that must succeed; returns its item lines.
std::vector<std::string> items_of(const std::string& repo, const std::vector<std::string>& args,
                                  SelfTestRun* whole = nullptr) {
    const SelfTestRun run = precheck_on(repo, args);
    INFO("stderr: " << run.err);
    REQUIRE(run.code == 0);
    if (whole) *whole = run;
    return item_lines(run);
}

// One expectation: a line starting "<kind> <file>:" whose text matches `like`.
struct Expect {
    std::string kind, file, like;
    int check = 0;  // the check that plants it, on the fixture
};

bool expect_holds(const std::vector<std::string>& lines, const Expect& e) {
    const std::string start = e.kind + " " + e.file + ":";
    const std::regex rx(e.like);
    for (const std::string& l : lines)
        if (l.rfind(start, 0) == 0 && std::regex_search(l, rx)) return true;
    return false;
}

void check_items(const std::string& label, const std::vector<std::string>& lines, const std::vector<Expect>& expect,
                 const std::vector<std::string>& absent) {
    for (const Expect& e : expect) {
        INFO(label << ": no '" << e.kind << " " << e.file << "' line matching /" << e.like << "/");
        CHECK(expect_holds(lines, e));
    }
    for (const std::string& a : absent) {
        const std::regex rx(a);
        for (const std::string& l : lines) {
            INFO(label << ": this line matches /" << a << "/ but should not: " << l);
            CHECK_FALSE(std::regex_search(l, rx));
        }
    }
    // Every item is one plain line: kind, file:line, what, and why.
    const std::regex shape("^[" + precheck::kind_order() + "] \\S+:\\d+  .+  -- .+$");
    for (const std::string& l : lines) {
        INFO(label << ": not one plain item line: " << l);
        CHECK(std::regex_match(l, shape));
    }
}

void check_fixture_clean(const std::string& dir) { CHECK(precheck::run_git(dir, {"status", "--porcelain"}).empty()); }

// The lines that start with `prefix`, joined by " | " so a failure prints them.
std::string starting(const std::vector<std::string>& lines, const std::string& prefix) {
    std::string got;
    for (const std::string& l : lines)
        if (l.rfind(prefix, 0) == 0) got += (got.empty() ? "" : " | ") + l;
    return got;
}

// The source-dir macro's name, in two pieces so this file's own lines do not
// answer the real scan row that asks for it.
const std::string kSourceMacro = std::string("HYDRA_") + "SOURCE_DIR";

const char* const kHelpersA = R"x(#include <vector>

namespace {

// Adds the positive values and subtracts the negative ones.
int sum_values(const std::vector<int>& values) {
    int total = 0;
    for (int v : values) {
        if (v > 0) total += v;
        else total -= v;
    }
    return total;
}

}  // namespace)x";

const char* const kAdr = R"x(# 1. Test limits

The batch test's deadline is 45 seconds.)x";

const char* const kOldTypes = R"x(#pragma once
#include <string>
#include <utility>

namespace fx {

struct SongEntry {
    std::string title, artist;
    std::string album;
    std::pair<std::string, std::string> credit;
    bool starred = false;
};

struct Letters {
    int alpha;
    int beta;
    int gamma;
    int delta;
};

enum class Verdict { Pass, Fail };

}  // namespace fx)x";

const char* const kOldHelpers = R"x(#include "core/old_types.h"

namespace fx {
namespace {

std::string trim_label(const std::string& s) { return s; }

}  // namespace

// Old wording: "a comment quoting text".
const char* missing_text() { return "chart file is missing"; }
const char* short_text() { return "too short"; }

}  // namespace fx)x";

// Check 1: the same helper with its names changed.
const char* const kHelpersB = R"x(#include <vector>

namespace {

int add_up(const std::vector<int>& xs) {
    int acc = 0;
    for (int x : xs) {
        if (x > 0) acc += x;
        else acc -= x;
    }
    return acc;
}

}  // namespace)x";

// Check 2: an offset recomputed from two times, which the scan row "How far
// is a note from an SP end, in a test?" flags. The gap between two chart
// ticks and the "!= SqIn" skip are must-not-match lines of their rows, so the
// exe must pass them. Check 3: 37 has no decision; 45 is decided by ADR 0001
// next to "deadline".
std::string offsets_text() {
    return std::string(R"x(#include "doctest.h"

TEST_CASE("offsets") {
    const auto deadline = std::chrono::seconds(45);
    CHECK(rows.size() > 37);
    const double offset = note.timecode.ms() - )x") +
           "end.ms();\n" + R"x(    CHECK(offset < 0.0);
    double gap = st.timecode(69120).ms() - st.timecode(68880).ms();
    for (const auto& s : steps) {
        if (s.kind != SpEndKind::SqIn) continue;
    }
})x";
}

// Check 4: a test that reads the source tree itself.
std::string stray_scan_text() {
    return R"x(#include <fstream>

TEST_CASE("model.h mentions the window") {
    std::ifstream in(std::string()x" +
           kSourceMacro + R"x() + "/src/core/model.h");
    CHECK(in.good());
})x";
}

// Check 4 again: a new scan row with no must-match examples. It sits one
// folder down, as a row in its own file may (ADR 0029). The scan's loader
// refuses a row without its owner and record fields, so the row has them.
const char* const kPlantedRules = R"x({
  "rules": [
    {
      "question": "Which planted row has no examples?",
      "owner": "nobody",
      "owner_file": "nobody.h",
      "owner_symbol": "nobody",
      "date": "2026-10-24",
      "decision": "PCX-7",
      "added_by": "PCX-7",
      "pattern": "\\bplanted_no_examples\\b",
      "must_not_match": [
        "int planted_must_not = 0;"
      ]
    }
  ]
})x";

// Check 5: one second home per probe. A record sharing four field names with
// SongEntry (two of them on each side only through a comma list), an enum
// name already defined, a free helper name already defined, and a long string
// already written. Passed: a record sharing only three names with Letters, a
// short string, a string the other file has only in a comment, and an
// #include path both files write.
const char* const kNewTypes = R"x(#pragma once
#include <string>
#include <utility>

namespace fx {

struct LibraryRow {
    std::string title;
    std::string artist, album;
    std::pair<std::string, std::string> credit;
    int plays = 0;
};

struct Greek {
    int alpha;
    int beta;
    int gamma;
    int omega;
};

enum class Verdict { Good, Bad };

}  // namespace fx)x";

const char* const kNewHelpers = R"x(#include "core/old_types.h"

namespace fx {
namespace {

std::string trim_label(const std::string& s) { return s + " "; }

}  // namespace

const char* load_error() { return "chart file is missing"; }
const char* brief_text() { return "too short"; }
const char* quoted_text() { return "a comment quoting text"; }

}  // namespace fx)x";

std::string read_lf(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream text;
    text << in.rdbuf();
    std::string s = text.str();
    std::string out;
    for (size_t i = 0; i < s.size(); ++i)
        if (!(s[i] == '\r' && i + 1 < s.size() && s[i + 1] == '\n')) out += s[i];
    return out;
}

std::string planted_rules_path() { return std::string(single_owner::kRulesDir) + "/planted/fixture.json"; }

struct SelfTestFixture {
    std::string dir = testtemp::temp_dir("precheck_selftest");
    SelfTestFixture() {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        fixture_git(dir, {"init", "-q", "-b", "main"});
        write_fixture_file(dir, "tests/helpers_a.cpp", kHelpersA);
        write_fixture_file(dir, "docs/adr/0001-limits.md", kAdr);
        write_fixture_file(dir, "src/core/old_types.h", kOldTypes);
        write_fixture_file(dir, "src/core/old_helpers.cpp", kOldHelpers);
        // The scan rows and known copies come from this repository's own
        // files, so the fixture tests them as they are today.
        const fs::path root = sourcetree::root();
        for (const auto& [name, path] : single_owner::rule_files(root / single_owner::kRulesDir))
            write_fixture_file(dir, std::string(single_owner::kRulesDir) + "/" + name, read_lf(path));
        write_fixture_file(dir, single_owner::kBaselineFile, read_lf(root / single_owner::kBaselineFile));
        commit_all(dir, "main");
        fixture_git(dir, {"checkout", "-q", "-b", "feature"});
        commit_texts(dir,
                     {{"tests/helpers_b.cpp", kHelpersB},
                      {"tests/test_offsets.cpp", offsets_text()},
                      {"tests/test_stray_scan.cpp", stray_scan_text()},
                      {planted_rules_path(), kPlantedRules},
                      {"src/app/new_types.h", kNewTypes},
                      {"src/app/new_helpers.cpp", kNewHelpers}},
                     "feature");
    }
    ~SelfTestFixture() { testtemp::remove_dir(dir); }
};

// Is this commit in this repository? A shallow or partial clone may lack the
// two old ranges.
bool have_commit(const std::string& repo, const std::string& name) {
    try {
        precheck::run_git(repo, {"cat-file", "-e", name + "^{commit}"});
        return true;
    } catch (const precheck::GitError&) {
        return false;
    }
}

const std::vector<std::string> kRealAbsent = {"^\\w tests/test_single_owner\\.cpp:"};

}  // namespace

TEST_CASE("precheck self-test: the fixture's planted items, each check off, the sizes and the whole tree") {
    SelfTestFixture fx;
    const std::vector<Expect> expect = {
        {"C", "tests/helpers_b.cpp", "add_up.*same body.*sum_values", 1},
        {"B", "tests/test_offsets.cpp", "How far is a note from an SP end.*end\\.ms\\(\\)", 2},
        {"D", "tests/test_offsets.cpp", "\\b37 in:", 3},
        {"E", "tests/test_stray_scan.cpp", kSourceMacro, 4},
        {"E", planted_rules_path(), "planted row has no examples.*no must-match examples", 4},
        {"F", "src/app/new_types.h",
         "LibraryRow shares 4 field names with struct SongEntry at src/core/old_types\\.h:\\d+: album, artist, "
         "credit, title",
         5},
        {"F", "src/app/new_types.h", "enum class Verdict is also defined at src/core/old_types\\.h:\\d+", 5},
        {"F", "src/app/new_helpers.cpp", "free function trim_label is also defined at src/core/old_helpers\\.cpp:\\d+",
         5},
        {"F", "src/app/new_helpers.cpp", "\"chart file is missing\" is also written at src/core/old_helpers\\.cpp:\\d+",
         5},
    };
    const std::vector<std::string> absent = {"\\b45 in:",
                                             "^[A-E] tests/helpers_a\\.cpp:",
                                             "^[A-E] tests/test_offsets\\.cpp:\\d+  .*timecode\\(69120\\)",
                                             "^[A-E] tests/test_offsets\\.cpp:\\d+  .*SpEndKind::SqIn",
                                             "^\\w src/core/",
                                             "Greek",
                                             "too short",
                                             "a comment quoting text",
                                             "\"core/old_types\\.h\""};

    SelfTestRun feature;
    const std::vector<std::string> lines = items_of(fx.dir, {"--range", "main...feature"}, &feature);
    check_items("fixture", lines, expect, absent);
    check_fixture_clean(fx.dir);

    // Each check, turned off, must lose every item it plants. The checks are
    // the exe's own list.
    const SelfTestRun list = precheck_on(fx.dir, {"--list-checks"});
    REQUIRE(list.code == 0);
    REQUIRE_FALSE(list.lines.empty());
    for (const std::string& entry : list.lines) {
        const std::string n = entry.substr(0, entry.find(' '));
        std::vector<Expect> planted;
        for (const Expect& e : expect)
            if (std::to_string(e.check) == n) planted.push_back(e);
        INFO("fixture: check " << entry << " plants no item, so nothing proves the test needs it");
        CHECK_FALSE(planted.empty());
        const std::vector<std::string> off = items_of(fx.dir, {"--range", "main...feature", "--disable", n});
        for (const Expect& e : planted) {
            INFO("fixture with check " << n << " off: still reports '" << e.kind << " " << e.file << "' /" << e.like
                                       << "/");
            CHECK_FALSE(expect_holds(off, e));
        }
        check_fixture_clean(fx.dir);
    }

    // Range size. The feature range is small: its line is pinned from one run.
    CHECK(starting(feature.lines, precheck::kSizePrefix) ==
          "range size: 86 changed lines (86 added, 0 deleted), not counting third_party/ or testdata/");
    CHECK(starting(feature.lines, precheck::kLargeRangePrefix).empty());

    // A range over the threshold, but only in the folders left out of the
    // count, gets no note.
    std::string over_limit;
    for (long long i = 1; i <= precheck::kLargeRangeLines + 1; ++i)
        over_limit += (i > 1 ? "\n" : "") + std::string("line ") + std::to_string(i);
    fixture_git(fx.dir, {"checkout", "-q", "-b", "vendored", "feature"});
    for (const std::string& folder : precheck::size_excluded_folders())
        write_fixture_file(fx.dir, folder + "/big.txt", over_limit);
    commit_all(fx.dir, "vendored");
    SelfTestRun vendored;
    (void)items_of(fx.dir, {"--range", "feature...vendored"}, &vendored);
    CHECK(starting(vendored.lines, precheck::kSizePrefix) ==
          "range size: 0 changed lines (0 added, 0 deleted), not counting third_party/ or testdata/");
    CHECK(starting(vendored.lines, precheck::kLargeRangePrefix).empty());
    check_fixture_clean(fx.dir);

    // The same text in a counted folder gets the note.
    fixture_git(fx.dir, {"checkout", "-q", "-b", "big", "feature"});
    write_fixture_file(fx.dir, "docs/big.md", over_limit);
    commit_all(fx.dir, "big");
    SelfTestRun big;
    (void)items_of(fx.dir, {"--range", "feature...big"}, &big);
    CHECK(starting(big.lines, precheck::kSizePrefix) ==
          "range size: 401 changed lines (401 added, 0 deleted), not counting third_party/ or testdata/");
    const std::string note = starting(big.lines, precheck::kLargeRangePrefix);
    CHECK_FALSE(note.empty());
    CHECK(note.find(" | ") == std::string::npos);  // one note line
    check_fixture_clean(fx.dir);

    // The whole tree at the feature tip: the row items print, check 3 says it
    // has nothing new to read, and no size line prints.
    SelfTestRun whole;
    const std::vector<std::string> whole_items = items_of(fx.dir, {"--whole-tree", "feature"}, &whole);
    check_items("whole tree", whole_items, {expect[1], expect[3]}, {});
    CHECK(starting(whole.lines, "check 3 skipped: ") == "check 3 skipped: -WholeTree has no \"new\" numbers");
    CHECK(starting(whole.lines, precheck::kSizePrefix).empty());
    check_fixture_clean(fx.dir);
}

// The first-review ranges of the 2026-10-04 gate, with the scan rows at
// HEAD (the rows that name these findings came after the commits), must name
// the findings the reviewers wrote by hand.
TEST_CASE("precheck self-test: the step 1 range names its reviewed findings") {
    const std::string repo = sourcetree::root().u8string();
    if (!have_commit(repo, "c6debcd")) {
        MESSAGE("step 1 range skipped: c6debcd is not in this repository");
        return;
    }
    const std::vector<std::string> lines = items_of(repo, {"--range", "c6debcd^2...c6debcd", "--rules-at", "HEAD"});
    check_items("step 1", lines,
                {{"C", "tests/test_fast_tempo.cpp", "collect_variants.*test_search\\.cpp"},
                 {"C", "tests/test_fast_tempo.cpp", "windows_text.*test_search\\.cpp"},
                 {"B", "tests/test_search.cpp", "plusmeasure\\(act\\.timecode"},
                 {"B", "tests/test_search.cpp", "fabs.*500\\.0"},
                 {"B", "tests/test_search.cpp", "early == 1\\.0"},
                 {"D", "src/search/engine.cpp", "0xFFFF"},
                 {"D", "tests/test_fast_tempo.cpp", "seed <= 48"},
                 {"E", "tests/test_model.cpp", kSourceMacro},
                 {"E", "tests/test_preview_view.cpp", "preview_view\\.cpp"}},
                kRealAbsent);
}

TEST_CASE("precheck self-test: the step 2 range names its reviewed findings") {
    const std::string repo = sourcetree::root().u8string();
    if (!have_commit(repo, "11b9d44")) {
        MESSAGE("step 2 range skipped: 11b9d44 is not in this repository");
        return;
    }
    const std::vector<std::string> lines = items_of(repo, {"--range", "11b9d44^2...11b9d44", "--rules-at", "HEAD"});
    check_items("step 2", lines,
                {{"C", "tests/test_s2_parser_owners.cpp", "deflate_raw.*test_srb\\.cpp"},
                 {"C", "tests/test_s2_offspeed.cpp", "fill_store.*test_dm_report\\.cpp"},
                 {"C", "tests/test_s2_disco.cpp", "chart_with.*test_song\\.cpp"},
                 {"C", "tests/test_s2_dynamics_tag.cpp", "MThd"},
                 {"B", "tests/test_song.cpp", "want = read =="},
                 {"B", "tools/ch_probe/tests/test_s2_window_constants.py", "WINDOW_CAP_MS / 2"},
                 {"D", "tools/ch_probe/tests/test_s2_window_constants.py", "\\b75 in:"},
                 {"E", "tests/test_s2_stamps.cpp", "stored_versions\\.h"}},
                kRealAbsent);
}

TEST_CASE("precheck self-test: a folder that is not a git repository fails by name") {
    const std::string plain = testtemp::temp_dir("precheck_selftest_plain");
    const SelfTestRun run = precheck_on(plain, {});
    CHECK(run.code != 0);
    CHECK(run.err == "not a git repository: " + plain + "\n");
    testtemp::remove_dir(plain);
}

// D271 item 6: the precheck prints the one helper's line for an option it
// does not know, and exits 1 as it does for every error it reports.
TEST_CASE("precheck self-test: an unknown option prints the one unknown-option line") {
    const std::string plain = testtemp::temp_dir("precheck_selftest_unknown_option");
    const SelfTestRun run = precheck_on(plain, {"--bogus"});
    CHECK(run.code == 1);
    CHECK(run.err == hydra::unknown_option_line("--bogus") + "\n");
    testtemp::remove_dir(plain);
}
