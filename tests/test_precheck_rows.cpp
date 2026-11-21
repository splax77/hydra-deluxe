// hydra_precheck's row hits (tools/precheck/check_rows.h): the kind letters,
// the added-line filter, the three notes, the added-element answer and the
// files it never reads. A tiny git repo in a temp folder holds made-up rows;
// the scan engine decides every hit.
#include "doctest.h"

#include <algorithm>
#include <set>
#include <cstdint>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "json.hpp"
#include "precheck/check_rows.h"
#include "precheck/checks.h"
#include "precheck/git_text.h"
#include "precheck/items.h"
#include "precheck/range.h"
#include "single_owner/engine.h"
#include "precheck_fixture.h"
#include "temp_util.h"

namespace fs = std::filesystem;
using nlohmann::json;
using namespace precheck;

namespace {

const char* const kQB = "Which planted value is computed?";
const char* const kQE = "Which test walks the planted tree?";
const char* const kQA = "How many ticks lie between two notes?";
const char* const kQC = "Which test builds a planted chart?";

json planted_row(const std::string& question, const std::string& owner, const std::string& pattern) {
    return {{"question", question}, {"owner", owner},          {"owner_file", owner},
            {"owner_symbol", "x"},  {"date", "2026-10-24"},    {"decision", "PCX-2"},
            {"added_by", "PCX-2"},  {"pattern", pattern},      {"must_match", json::array({"m"})},
            {"must_not_match", json::array({"n"})}};
}

struct RowFixture {
    std::string dir = testtemp::temp_dir("precheck_rows");
    // The rules file as the feature commit holds it.
    std::string feature_rules;

    void put(const std::string& rel, const std::string& text) const {
        precheck_fixture::write_fixture_file(dir, rel, text);
    }
    void git(const std::vector<std::string>& args) const { precheck_fixture::fixture_git(dir, args); }
    void commit(const std::string& message) const { precheck_fixture::commit_all(dir, message); }

    // main: three rows, one owner line for a line nobody has written yet.
    // feature: a new owner line, a new row, a baseline entry, and the
    // planted lines.
    RowFixture() {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        git({"init", "-q", "-b", "main"});
        json a = planted_row(kQA, "src/core/ticks.h", "ticks_between\\(");
        a["calls_owner"] = "chart_ticks::";
        a["owner_lines"] = json::array(
            {{{"file", "src/feature.cpp"}, {"line_text", "int q = ticks_between(x, y);"}, {"why", "old"}}});
        json b = planted_row(kQB, "src/core/planted.h", "planted_value\\(");
        b["scope"] = json::array({"tests"});
        json e = planted_row(kQE, "tests/source_tree.h", "walk_planted_tree\\(");
        e["scope"] = json::array({"tests"});
        json rules = {{"rules", json::array({a, b, e})}};
        put("tests/single_owner/rules/planted.json", rules.dump(2) + "\n");
        put("src/feature.cpp", "int base_line = 0;\n");
        put("tests/test_x.cpp", "int z = planted_value(9);\n");
        commit("main1");

        git({"checkout", "-q", "-b", "feature"});
        rules["rules"][0]["owner_lines"].push_back(
            {{"file", "src/feature.cpp"}, {"line_text", "int k = ticks_between(c, d);"}, {"why", "the planted owner"}});
        json c = planted_row(kQC, "tests/fixture_util.h", "make_planted_chart\\(");
        c["scope"] = json::array({"tests"});
        rules["rules"].push_back(c);
        feature_rules = rules.dump(2) + "\n";
        put("tests/single_owner/rules/planted.json", feature_rules);
        json copy = {{"question", kQB},          {"file", "tests/test_x.cpp"},
                     {"line_text", "int w = planted_value(4);"}, {"removed_by", "FIX-1"},
                     {"date", "2026-10-24"},     {"added_by", "PCX-2"},
                     {"remove_by", "2026-10-24"}};
        put("tests/single_owner/baseline.json", json{{"known_copies", json::array({copy})}}.dump(2) + "\n");
        put("src/feature.cpp",
            "int base_line = 0;\n"
            "int g = ticks_between(a, b);\n"
            "int h = chart_ticks::ticks_between(a, b);\n"
            "// int c = ticks_between(a, b);\n"
            "int k = ticks_between(c, d);\n"
            "int q = ticks_between(x, y);\n");
        put("tests/test_x.cpp",
            "int z = planted_value(9);\n"
            "int v = planted_value(3);\n"
            "auto root = walk_planted_tree(1);\n"
            "int w = planted_value(4);\n"
            "int w = planted_value(4);\n"
            "auto c = make_planted_chart(1);\n");
        put("tests/single_owner/planted_case.cpp", "int v = planted_value(3);\n");
        put("tests/test_single_owner.cpp", "int v = planted_value(3);\n");
        commit("f1");
    }
    ~RowFixture() { testtemp::remove_dir(dir); }
};

// Every row item of one range, the rows it added, and what the row hits
// printed themselves.
struct RowRun {
    std::vector<RowItem> items;
    std::string printed;
    std::vector<AddedRow> added;
    // The file:line of each item check N adds, sorted, at index N - 1.
    std::vector<std::vector<std::string>> per_check;
};

RowRun rows_of(const RowFixture& fx, const std::string& spelling, bool whole, const std::string& rules_at) {
    Repo repo(fx.dir);
    const Range range = resolve_range(repo, spelling, whole, rules_at);
    const std::vector<std::string> files = all_files(repo, range.tip);
    const AddedLines added = whole ? AddedLines::whole(files) : AddedLines::from_diff(repo, range.base, range.tip);
    Items items;
    std::ostringstream out;
    Context ctx{repo, range, added, files, items, out};
    RowRun r;
    r.items = row_items(ctx, "ABCE");
    r.added = added_rows(ctx);
    for (size_t n = 1; n <= kChecks.size(); ++n) {
        Items one;
        Context check_ctx{repo, range, added, files, one, out};
        add_row_items_for_check(static_cast<int>(n), check_ctx);
        std::vector<std::string> at;
        for (const Item& i : one.sorted()) at.push_back(i.file + ":" + std::to_string(i.line));
        std::sort(at.begin(), at.end());
        r.per_check.push_back(at);
    }
    // Reading the rows prints nothing; the warnings print after every check.
    CHECK(out.str().empty());
    print_rule_warnings(ctx);
    r.printed = out.str();
    return r;
}

std::vector<std::string> lines_of(const std::vector<RowItem>& items) {
    std::vector<std::string> out;
    for (const RowItem& i : items) out.push_back(i.kind + " " + i.file + ":" + std::to_string(i.line) + " " + i.why);
    std::sort(out.begin(), out.end());
    return out;
}

// The rules file the fixture writes, in the scan's own rules folder.
const std::string kPlantedRules = std::string(single_owner::kRulesDir) + "/planted.json";

std::string why_for(const std::string& owner) {
    return "the scan row in " + kPlantedRules + " gives this question to " + owner;
}

const std::string kWhyA = why_for("src/core/ticks.h");
const std::string kWhyB = why_for("src/core/planted.h");
const std::string kWhyC = why_for("tests/fixture_util.h");
const std::string kWhyE = why_for("tests/source_tree.h");

// The 1-based line of the opening brace of row `nth` (from 0) in a rules
// file written with dump(2): each element of "rules" opens on a line of four
// spaces and a brace, and nothing else in the file sits at that depth.
int row_open_line(const std::string& text, int nth) {
    std::istringstream in(text);
    std::string line;
    int number = 0;
    int seen = 0;
    while (std::getline(in, line)) {
        ++number;
        if (line == "    {" && seen++ == nth) return number;
    }
    return 0;
}

}  // namespace

TEST_CASE("precheck rows: the kinds and the notes on a range") {
    RowFixture fx;
    const RowRun r = rows_of(fx, "main...feature", false, "");
    CHECK(r.printed.empty());
    CHECK(lines_of(r.items) == std::vector<std::string>{
              "A src/feature.cpp:2 " + kWhyA,
              "A src/feature.cpp:5 " + kWhyA +
                  " (this range lists it as an owner line: \"the planted owner\"; check that reason)",
              "B tests/test_x.cpp:2 " + kWhyB,
              "B tests/test_x.cpp:4 " + kWhyB + " (" + known_copy_note("FIX-1") + "; this range added that entry)",
              "B tests/test_x.cpp:5 " + kWhyB + " (a second copy of a listed line; each entry covers one line)",
              "C tests/test_x.cpp:6 " + kWhyC,
              "E tests/test_x.cpp:3 " + kWhyE,
          });
    // The what names the question and the trimmed line.
    for (const RowItem& i : r.items)
        if (i.file == "tests/test_x.cpp" && i.line == 2)
            CHECK(i.what == "answers \"Which planted value is computed?\": int v = planted_value(3);");
    // The one new row; the three rows from main are not new.
    REQUIRE(r.added.size() == 1);
    CHECK(r.added[0].rule.question == kQC);
    CHECK(r.added[0].file == kPlantedRules);
    // The new row is the fourth in the file the fixture wrote.
    const int row_line = row_open_line(fx.feature_rules, 3);
    REQUIRE(row_line > 0);
    CHECK(r.added[0].line == row_line);
}

TEST_CASE("precheck rows: the rows read at another commit add nothing") {
    RowFixture fx;
    const RowRun r = rows_of(fx, "main...feature", false, "main");
    // main's rows have no new owner line, no baseline and no chart row, so
    // the owner line reads as a plain hit and the copies as plain hits.
    CHECK(lines_of(r.items) == std::vector<std::string>{
              "A src/feature.cpp:2 " + kWhyA,
              "A src/feature.cpp:5 " + kWhyA,
              "B tests/test_x.cpp:2 " + kWhyB,
              "B tests/test_x.cpp:4 " + kWhyB,
              "B tests/test_x.cpp:5 " + kWhyB,
              "E tests/test_x.cpp:3 " + kWhyE,
          });
    CHECK(r.added.empty());
}

TEST_CASE("precheck rows: the whole tree counts every line and every entry as added") {
    RowFixture fx;
    const RowRun r = rows_of(fx, "feature", true, "");
    const std::vector<std::string> got = lines_of(r.items);
    CHECK(std::count(got.begin(), got.end(), "B tests/test_x.cpp:1 " + kWhyB) == 1);
    // main's owner line counts as written too, so its line prints with the note.
    CHECK(std::count(got.begin(), got.end(),
                     "A src/feature.cpp:6 " + kWhyA +
                         " (this range lists it as an owner line: \"old\"; check that reason)") == 1);
    // Every row the fixture wrote is added, each once.
    std::vector<std::string> questions;
    for (const AddedRow& a : r.added) questions.push_back(a.rule.question);
    std::sort(questions.begin(), questions.end());
    std::vector<std::string> written = {kQA, kQB, kQC, kQE};
    std::sort(written.begin(), written.end());
    CHECK(questions == written);
    // The scan's own files are never read. Both hold the line the scan flags
    // at tests/test_x.cpp:2, so the loop below has a hit to miss.
    REQUIRE(std::count(got.begin(), got.end(), "B tests/test_x.cpp:2 " + kWhyB) == 1);
    for (const RowItem& i : r.items) {
        CHECK(i.file != "tests/test_single_owner.cpp");
        CHECK(i.file.rfind("tests/single_owner/", 0) != 0);
    }
}

TEST_CASE("precheck rows: a revision with no rules files says so") {
    RowFixture fx;
    fx.git({"checkout", "-q", "--orphan", "bare"});
    fx.git({"rm", "-rq", "--cached", "."});
    fx.put("src/only.cpp", "int g = ticks_between(a, b);\n");
    fx.git({"add", "src/only.cpp"});
    fx.git({"commit", "-q", "-m", "bare"});
    Repo repo(fx.dir);
    const RowRun r = rows_of(fx, "bare", true, "");
    CHECK(r.items.empty());
    // One line, naming the rules folder and the commit the rows were read
    // at. The commit word is whatever git resolves back to that commit.
    const std::string head = "precheck: " + std::string(single_owner::kRulesDir) + " has no rule files at ";
    const std::string tail = ", so no scan rows were applied\n";
    REQUIRE(r.printed.size() > head.size() + tail.size());
    CHECK(r.printed.rfind(head, 0) == 0);
    CHECK(r.printed.compare(r.printed.size() - tail.size(), tail.size(), tail) == 0);
    const std::string commit_word = r.printed.substr(head.size(), r.printed.size() - head.size() - tail.size());
    INFO(commit_word);
    CHECK(commit_word.find(' ') == std::string::npos);
    CHECK(repo.resolve_commit(commit_word) == repo.resolve_commit("bare"));
}

TEST_CASE("precheck rows: each hit reaches the check its file and the row's owner give") {
    RowFixture fx;
    const RowRun r = rows_of(fx, "main...feature", false, "");
    REQUIRE(r.per_check.size() >= 4);
    // Check 1: a test file's hit on a row another test file owns.
    CHECK(r.per_check[0] == std::vector<std::string>{"tests/test_x.cpp:6"});
    // Check 2: production code's hits, and a test file's hits on rows
    // production code owns.
    CHECK(r.per_check[1] == std::vector<std::string>{"src/feature.cpp:2", "src/feature.cpp:5", "tests/test_x.cpp:2",
                                                     "tests/test_x.cpp:4", "tests/test_x.cpp:5"});
    // Check 4: a test file's hit on a row the source-tree reader owns.
    CHECK(r.per_check[3] == std::vector<std::string>{"tests/test_x.cpp:3"});
    // No other check takes a row hit.
    for (size_t n = 0; n < r.per_check.size(); ++n)
        if (n != 0 && n != 1 && n != 3) CHECK(r.per_check[n].empty());
}

TEST_CASE("precheck rows: the owner's spelling and where tests sit do not change the kind") {
    const std::string in_source = row_kind("src/a.cpp", "src/core/a.h");
    const std::string on_source = row_kind("tests/a.cpp", "src/core/a.h");
    const std::string on_tree = row_kind("tests/a.cpp", "tests/source_tree.h");
    const std::string on_test = row_kind("tests/a.cpp", "tests/fixture_util.h");
    // The four places a hit can sit are four kinds.
    CHECK(std::set<std::string>{in_source, on_source, on_tree, on_test}.size() == 4);
    // Production code is one kind whoever owns the row.
    CHECK(row_kind("src/a.cpp", "tests/source_tree.h") == in_source);
    // A tests folder under tools holds test files too.
    CHECK(row_kind("tools/x/tests/a.cpp", "src/core/a.h") == on_source);
    // The owner matches with case ignored and words after it.
    CHECK(row_kind("tests/a.cpp", "Tests/Source_Tree.h (walk)") == on_tree);
    // A test file named inside a sentence is still a test file.
    CHECK(row_kind("tests/a.cpp", "the helper in tests/util.h") == on_test);
    // "tests/" inside a longer folder name is not a tests folder.
    CHECK(row_kind("tests/a.cpp", "src/mytests/a.h") == on_source);
}
