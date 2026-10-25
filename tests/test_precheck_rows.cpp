// hydra_precheck's row hits (tools/precheck/check_rows.h): the kind letters,
// the added-line filter, the three notes, the added-element answer and the
// files it never reads. A tiny git repo in a temp folder holds made-up rows;
// the scan engine decides every hit.
#include "doctest.h"

#include <algorithm>
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

    void put(const std::string& rel, const std::string& text) const {
        const fs::path p = fs::path(hydra::os_path(dir)) / fs::u8path(rel);
        fs::create_directories(p.parent_path());
        testtemp::write_bytes(p.u8string(), std::vector<uint8_t>(text.begin(), text.end()));
    }
    void git(const std::vector<std::string>& args) const {
        std::vector<std::string> a = {"-c", "user.name=rows", "-c", "user.email=rows@example.invalid",
                                      "-c", "core.autocrlf=false"};
        a.insert(a.end(), args.begin(), args.end());
        run_git(dir, a);
    }
    void commit(const std::string& message) const {
        git({"add", "-A"});
        git({"commit", "-q", "-m", message});
    }

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
        put("tests/single_owner/rules/planted.json", rules.dump(2) + "\n");
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
    r.printed = out.str();
    return r;
}

std::vector<std::string> lines_of(const std::vector<RowItem>& items) {
    std::vector<std::string> out;
    for (const RowItem& i : items) out.push_back(i.kind + " " + i.file + ":" + std::to_string(i.line) + " " + i.why);
    std::sort(out.begin(), out.end());
    return out;
}

const std::string kWhyA = "the scan row in tests/single_owner/rules/planted.json gives this question to src/core/ticks.h";
const std::string kWhyB = "the scan row in tests/single_owner/rules/planted.json gives this question to src/core/planted.h";
const std::string kWhyC = "the scan row in tests/single_owner/rules/planted.json gives this question to tests/fixture_util.h";
const std::string kWhyE = "the scan row in tests/single_owner/rules/planted.json gives this question to tests/source_tree.h";

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
              "B tests/test_x.cpp:4 " + kWhyB + " (known copy: FIX-1; this range added that entry)",
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
    CHECK(r.added[0].file == "tests/single_owner/rules/planted.json");
    CHECK(r.added[0].line > 0);
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
    CHECK(r.added.size() == 4);
    // The scan's own files are never read.
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
    const std::string short_hash = repo.resolve_commit("bare").substr(0, 7);
    const RowRun r = rows_of(fx, "bare", true, "");
    CHECK(r.items.empty());
    CHECK(r.printed == "precheck: tests/single_owner/rules has no rule files at " + short_hash +
                           ", so no scan rows were applied\n");
}

TEST_CASE("precheck rows: the kind letter from the file and the row's owner") {
    CHECK(row_kind("src/a.cpp", "tests/source_tree.h") == "A");
    CHECK(row_kind("tools/x/tests/a.cpp", "src/core/a.h") == "B");
    CHECK(row_kind("tests/a.cpp", "Tests/Source_Tree.h (walk)") == "E");
    CHECK(row_kind("tests/a.cpp", "the helper in tests/util.h") == "C");
    CHECK(row_kind("tests/a.cpp", "src/mytests/a.h") == "B");
}
