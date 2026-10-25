// Checks 2 and 4 of hydra_precheck, the spellings no scan row asks about: a
// fixture repo in a temp folder, one feature commit over main, and the
// checks run on it through the same Context the exe builds. The row items
// these checks also print come from check_rows.cpp and are tested there.
#include "doctest.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "core/winstr.h"
#include "json.hpp"
#include "precheck/check_4.h"
#include "precheck/checks.h"
#include "precheck/git_text.h"
#include "precheck/items.h"
#include "precheck/range.h"
#include "precheck_fixture.h"
#include "single_owner/engine.h"
#include "temp_util.h"

namespace fs = std::filesystem;

using namespace precheck;
using precheck_fixture::commit_texts;

namespace {

// main holds one line in each file the feature edits; the feature adds the
// lines under test.
struct Fixture {
    std::string dir = testtemp::temp_dir("precheck_check24");
    Fixture() {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        run_git(dir, {"init", "-q", "-b", "main"});
        commit_texts(dir, {{"tests/test_spell.cpp", "#include \"doctest.h\"\n"}}, "main1");
        run_git(dir, {"checkout", "-q", "-b", "feature"});
        commit_texts(dir,
                     {{"tests/test_spell.cpp",
                       "#include \"doctest.h\"\n"
                       "// the hit window is two hits wide\n"   // 2
                       "CHECK(w == 2.0);\n"                     // 3
                       "// hit window again\n"                  // 4
                       "int a;\n"                               // 5
                       "int b;\n"                               // 6
                       "CHECK(z == 2.0);\n"                     // 7: three lines from the words
                       "bool want = read == 3 || read == 4;\n"  // 8
                       "CHECK(f(a) ==\n"                        // 9
                       "      CHECK(g(b) == h(c) + k(d)));\n"   // 10: alone it matches too
                       "CHECK(p.tied_pathcount() == paths.size());\n"  // 11
                       "std::ifstream in(root / \"core\" / \"x.cpp\");\n"},  // 12
                      {"tests/test_spell.py", "assert x < mod.LIMIT_MS\n"},
                      {"tools/walk.py", "for r, _, fs in os.walk(os.path.join(ROOT, \"src\")):\n"},
                      {"src/prod.cpp",
                       "bool want = a == 1 || b == 2;\n"
                       "std::ifstream in(root / \"x.cpp\");\n"}},
                     "f1");
    }
    ~Fixture() { testtemp::remove_dir(dir); }
};

// One check's items on the fixture's feature range, as printed lines.
struct Run {
    Repo repo;
    Range range;
    AddedLines added;
    std::vector<std::string> files;
    Items items;
    std::ostringstream out;
    explicit Run(const std::string& dir)
        : repo(dir),
          range(resolve_range(repo, "main...feature", false, "")),
          added(AddedLines::from_diff(repo, range.base, range.tip)),
          files(all_files(repo, range.tip)) {}
    Context context() { return Context{repo, range, added, files, items, out}; }
    std::vector<std::string> lines() const {
        std::vector<std::string> v;
        for (const Item& i : items.sorted()) v.push_back(item_line(i));
        return v;
    }
};

bool has(const std::vector<std::string>& lines, const std::string& line) {
    for (const std::string& l : lines)
        if (l == line) return true;
    return false;
}

}  // namespace

TEST_CASE("precheck check 2: each recompute spelling prints once, with its wording") {
    Fixture fx;
    Run run(fx.dir);
    Context ctx = run.context();
    run_check_2(ctx);
    const std::vector<std::string> got = run.lines();
    INFO(got.size());
    CHECK(has(got, "B tests/test_spell.cpp:3  a typed 2.0 beside a hit-window check: CHECK(w == 2.0);  -- the "
                   "hit-window rule has one owner; call it or pin the literal"));
    CHECK(has(got, "B tests/test_spell.cpp:8  the expected answer built from the predicate's own comparisons: bool "
                   "want = read == 3 || read == 4;  -- pin a literal list of inputs and answers instead of "
                   "restating the rule"));
    // The two-line statement prints at its first line, read whole, and its
    // second line does not print again.
    CHECK(has(got, "B tests/test_spell.cpp:9  an expected value computed from other calls: CHECK(f(a) == CHECK(g(b) "
                   "== h(c) + k(d)));  -- a test pins literals from one run; it never computes the expected value"));
    CHECK(has(got, "B tests/test_spell.cpp:11  the tied-path count worked out a second way: CHECK(p.tied_pathcount() "
                   "== paths.size());  -- Path::recount_tied_paths owns the count; pin it"));
    CHECK(has(got, "B tests/test_spell.py:1  a fixture decides with the module's own constant: assert x < "
                   "mod.LIMIT_MS  -- the fixture makes the comparison production makes; pin the inputs as literals"));
    // The 2.0 three lines from "hit window" is out of reach, line 10 was
    // shown with line 9, and src/ is production code.
    CHECK(got.size() == 5);
}

TEST_CASE("precheck check 4: source-tree reads outside src/, and a row's line prints once") {
    Fixture fx;
    Run run(fx.dir);
    Context ctx = run.context();
    run_check_4_spellings(ctx, Check4Rows{});
    std::vector<std::string> got = run.lines();
    const std::string why =
        "  -- the single-owner scan is the one scan of the source tree; make this a row in tests/single_owner/rules";
    CHECK(has(got, "E tests/test_spell.cpp:12  opens a source file: std::ifstream in(root / \"core\" / \"x.cpp\");" +
                       why));
    CHECK(has(got, "E tools/walk.py:1  reads the source tree: for r, _, fs in os.walk(os.path.join(ROOT, \"src\")):" +
                       why));
    CHECK(got.size() == 2);  // src/prod.cpp opens a source file too, but src/ is not read

    // A line a row already printed is not printed again.
    Run again(fx.dir);
    Context ctx2 = again.context();
    Check4Rows rows;
    rows.e_hit_lines.insert({"tests/test_spell.cpp", 12});
    run_check_4_spellings(ctx2, rows);
    got = again.lines();
    CHECK(got.size() == 1);
    CHECK(!has(got, "E tests/test_spell.cpp:12  opens a source file: std::ifstream in(root / \"core\" / \"x.cpp\");" +
                        why));
}

TEST_CASE("precheck check 4: a source-dir macro line prints once, as the scan row's E item") {
    // The macro's name is built in two pieces so this file's own lines do not
    // answer the real row.
    const std::string macro = std::string("HYDRA_") + "SOURCE_DIR";
    const std::string dir = testtemp::temp_dir("precheck_check4_row");
    testtemp::remove_dir(dir);
    fs::create_directories(hydra::os_path(dir));
    run_git(dir, {"init", "-q", "-b", "main"});
    const nlohmann::json row = {{"question", "Which test reads the source tree?"},
                                {"owner", "sourcetree::root in tests/source_tree.h"},
                                {"owner_file", "tests/source_tree.h"},
                                {"owner_symbol", "sourcetree::root"},
                                {"date", "2026-10-24"},
                                {"decision", "PCX-J2"},
                                {"added_by", "PCX-J2"},
                                {"pattern", "\\b" + macro + "\\b"},
                                {"scope", nlohmann::json::array({"tests"})},
                                {"must_match", nlohmann::json::array({macro})},
                                {"must_not_match", nlohmann::json::array({"HYDRA_SOURCE"})}};
    commit_texts(dir,
                 {{"tests/single_owner/rules/tests.json", nlohmann::json{{"rules", {row}}}.dump(2) + "\n"},
                  {"tests/test_tree.cpp", "int keep = 0;\n"}},
                 "main1");
    run_git(dir, {"checkout", "-q", "-b", "feature"});
    // Both the row and check 4's "opens a source file" spelling match it.
    const std::string line = "std::ifstream in(std::string(" + macro + ") + \"/src/app/x.cpp\");";
    commit_texts(dir, {{"tests/test_tree.cpp", "int keep = 0;\n" + line + "\n"}}, "f1");
    {
        Run run(dir);
        Context ctx = run.context();
        run_check_4(ctx);
        CHECK(run.lines() == std::vector<std::string>{"E tests/test_tree.cpp:2  answers \"Which test reads the "
                                                      "source tree?\": " +
                                                      line +
                                                      "  -- the scan row in tests/single_owner/rules/tests.json "
                                                      "gives this question to sourcetree::root in tests/source_tree.h"});
    }
    testtemp::remove_dir(dir);
}

TEST_CASE("precheck check 4: a new rule row with no examples or a variable exemption") {
    Fixture fx;
    Run run(fx.dir);
    single_owner::OwnerRule bare;
    bare.question = "planted row has no examples";
    bare.pattern = "\\bplanted\\b";
    single_owner::OwnerRule exempting;
    exempting.question = "exempting row";
    exempting.pattern = "\\bscore(?!(base|combo)\\s*\\.)";
    exempting.must_match = {"score"};
    exempting.must_not_match = {"base.score"};
    Check4Rows rows;
    rows.added_rows = {{bare, "tests/single_owner/rules/sub/planted.json", 4},
                       {exempting, "tests/single_owner/rules/sub/planted.json", 20}};
    // The spellings are not under test here: no file is read.
    run.added = AddedLines::whole({});
    Context ctx = run.context();
    run_check_4_spellings(ctx, rows);
    const std::vector<std::string> got = run.lines();
    const std::string examples_why =
        "  -- every scan row ships with lines it must match and must not match, so the self-test proves its pattern";
    CHECK(has(got, "E tests/single_owner/rules/sub/planted.json:4  scan row \"planted row has no examples\" has no "
                   "must-match examples" +
                       examples_why));
    CHECK(has(got, "E tests/single_owner/rules/sub/planted.json:4  scan row \"planted row has no examples\" has no "
                   "must-not-match examples" +
                       examples_why));
    CHECK(has(got, "E tests/single_owner/rules/sub/planted.json:20  scan row \"exempting row\" exempts variable "
                   "names inside its pattern ((?!(base|combo)\\s*\\....)  -- an exemption in the pattern applies in "
                   "every file; list the allowed lines as owner lines with their reason"));
    CHECK(got.size() == 3);
}
