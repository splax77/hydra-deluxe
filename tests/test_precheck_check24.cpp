// Checks 2 and 4 of hydra_precheck, the spellings no scan row asks about: a
// fixture repo in a temp folder, one feature commit over main, and the
// checks run on it through the same Context the exe builds. The row items
// these checks also print come from check_rows.cpp and are tested there.
// Each item's wording is the check's own, so these tests check which lines
// print, under which kind, and that each item names the line or row it is
// about.
#include "doctest.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "core/strutil.h"
#include "core/winstr.h"
#include "json.hpp"
#include "precheck/check_4.h"
#include "precheck/check_rows.h"
#include "precheck/checks.h"
#include "precheck/git_text.h"
#include "precheck/items.h"
#include "precheck/range.h"
#include "precheck_fixture.h"
#include "single_owner/engine.h"
#include "source_tree.h"
#include "temp_util.h"

namespace fs = std::filesystem;

using namespace precheck;
using precheck_fixture::commit_texts;

namespace {

// The feature's lines under test, as the checks read them.
const std::string kHitWindowCheck = "CHECK(w == 2.0);";
const std::string kWantRule = "bool want = read == 3 || read == 4;";
const std::string kSplitHead = "CHECK(f(a) ==";
const std::string kSplitTail = "CHECK(g(b) == h(c) + k(d)));";
const std::string kTiedCount = "CHECK(p.tied_pathcount() == paths.size());";
const std::string kOpensSource = "std::ifstream in(root / \"core\" / \"x.cpp\");";
const std::string kPyConstant = "assert x < mod.LIMIT_MS";
const std::string kWalk = "for r, _, fs in os.walk(os.path.join(ROOT, \"src\")):";

const std::string kSpellCpp = "tests/test_spell.cpp";
const std::string kSpellPy = "tests/test_spell.py";
const std::string kWalkPy = "tools/walk.py";
const std::string kTreeTest = "tests/test_tree.cpp";

// main holds one file with one line; the feature adds the lines under test,
// to that file and to new ones.
struct Fixture {
    std::string dir = testtemp::temp_dir("precheck_check24");
    Fixture() {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        run_git(dir, {"init", "-q", "-b", "main"});
        commit_texts(dir, {{kSpellCpp, "#include \"doctest.h\"\n"}}, "main1");
        run_git(dir, {"checkout", "-q", "-b", "feature"});
        commit_texts(dir,
                     {{kSpellCpp, std::string("#include \"doctest.h\"\n") +
                                      "// the hit window is two hits wide\n" +  // 2
                                      kHitWindowCheck + "\n" +                 // 3
                                      "// hit window again\n" +                // 4
                                      "int a;\n" +                             // 5
                                      "int b;\n" +                             // 6
                                      "CHECK(z == 2.0);\n" +  // 7: three lines from the words
                                      kWantRule + "\n" +      // 8
                                      kSplitHead + "\n" +     // 9
                                      "      " + kSplitTail + "\n" +  // 10: alone it matches too
                                      kTiedCount + "\n" +             // 11
                                      kOpensSource + "\n"},           // 12
                      {kSpellPy, kPyConstant + "\n"},
                      {kWalkPy, kWalk + "\n"},
                      {"src/prod.cpp",
                       "bool want = a == 1 || b == 2;\n"
                       "std::ifstream in(root / \"x.cpp\");\n"}},
                     "f1");
    }
    ~Fixture() { testtemp::remove_dir(dir); }
};

// A rules file's repo-relative path, from the name select_rule_files gives
// it ("rules/x.json").
std::string rules_path(const std::string& name) {
    const std::string dir = single_owner::kRulesDir;
    return dir.substr(0, dir.rfind('/') + 1) + name;
}

// This repository's rules files as they are today, each by its
// repo-relative path, so a fixture runs the checks on the real rows.
std::map<std::string, nlohmann::json> real_rules() {
    std::map<std::string, nlohmann::json> out;
    for (const auto& [name, path] : single_owner::rule_files(sourcetree::root() / single_owner::kRulesDir)) {
        std::ifstream in(path, std::ios::binary);
        out[rules_path(name)] = nlohmann::json::parse(in);
    }
    return out;
}

// main holds `main_rules` and a one-line test file; the feature commit
// writes `feature`.
struct RulesFixture {
    std::string dir;
    RulesFixture(const std::string& tag, const std::map<std::string, nlohmann::json>& main_rules,
                 const std::map<std::string, std::string>& feature)
        : dir(testtemp::temp_dir(tag)) {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        run_git(dir, {"init", "-q", "-b", "main"});
        std::map<std::string, std::string> texts = {{kTreeTest, "int keep = 0;\n"}};
        for (const auto& [path, rules] : main_rules) texts[path] = rules.dump(2) + "\n";
        commit_texts(dir, texts, "main1");
        run_git(dir, {"checkout", "-q", "-b", "feature"});
        commit_texts(dir, feature, "f1");
    }
    ~RulesFixture() { testtemp::remove_dir(dir); }
};

// One check's items on the fixture's feature range.
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
};

// The items at file:line.
std::vector<Item> items_at(const std::vector<Item>& items, const std::string& file, int line) {
    std::vector<Item> at;
    for (const Item& i : items)
        if (i.file == file && i.line == line) at.push_back(i);
    return at;
}

// The label of the one item of `kind` at file:line: its text before the
// ": <code>" that names the line it prints. Empty when there is not exactly
// one such item, when it gives no reason, or when it does not end with the
// line's code.
std::string one_item_label(const std::vector<Item>& items, const std::string& kind, const std::string& file,
                           int line, const std::string& code) {
    const std::vector<Item> at = items_at(items, file, line);
    if (at.size() != 1 || at[0].kind != kind || at[0].why.empty()) return "";
    const std::string tail = ": " + code;
    const std::string& text = at[0].what;
    if (text.size() <= tail.size() || !hydra::ends_with(text, tail)) return "";
    return text.substr(0, text.size() - tail.size());
}

}  // namespace

TEST_CASE("precheck check 2: each recompute spelling prints once, naming its line") {
    Fixture fx;
    Run run(fx.dir);
    Context ctx = run.context();
    run_check_2(ctx);
    const std::vector<Item> got = run.items.sorted();
    INFO(got.size());
    // Each line matches a different spelling, so each label differs.
    const std::vector<std::string> labels = {
        one_item_label(got, "B", kSpellCpp, 3, kHitWindowCheck), one_item_label(got, "B", kSpellCpp, 8, kWantRule),
        // The two-line statement prints at its first line, read whole.
        one_item_label(got, "B", kSpellCpp, 9, kSplitHead + " " + kSplitTail),
        one_item_label(got, "B", kSpellCpp, 11, kTiedCount), one_item_label(got, "B", kSpellPy, 1, kPyConstant)};
    for (const std::string& label : labels) CHECK(!label.empty());
    CHECK(std::set<std::string>(labels.begin(), labels.end()).size() == labels.size());
    // The 2.0 three lines from "hit window" is out of reach, line 10 was
    // shown with line 9, and src/ is production code.
    CHECK(items_at(got, kSpellCpp, 7).empty());
    CHECK(items_at(got, kSpellCpp, 10).empty());
    CHECK(got.size() == 5);
}

TEST_CASE("precheck check 4: source-tree reads outside src/, and a row's line prints once") {
    Fixture fx;
    Run run(fx.dir);
    Context ctx = run.context();
    run_check_4_spellings(ctx, Check4Rows{});
    std::vector<Item> got = run.items.sorted();
    const std::string opens = one_item_label(got, "E", kSpellCpp, 12, kOpensSource);
    const std::string reads = one_item_label(got, "E", kWalkPy, 1, kWalk);
    CHECK(!opens.empty());
    CHECK(!reads.empty());
    CHECK(opens != reads);  // the C++ line opens a file, the Python line walks the tree
    // Each sends the reader to the one scan's rules folder.
    for (const Item& i : got) {
        const std::string& why = i.why;
        CHECK(why.find(single_owner::kRulesDir) != std::string::npos);
    }
    CHECK(got.size() == 2);  // src/prod.cpp opens a source file too, but src/ is not read

    // A line a row already printed is not printed again.
    Run again(fx.dir);
    Context ctx2 = again.context();
    Check4Rows rows;
    rows.e_hit_lines.insert({kSpellCpp, 12});
    run_check_4_spellings(ctx2, rows);
    got = again.items.sorted();
    CHECK(items_at(got, kSpellCpp, 12).empty());
    CHECK(one_item_label(got, "E", kWalkPy, 1, kWalk) == reads);
    CHECK(got.size() == 1);
}

TEST_CASE("precheck check 4: a line a real source-tree row answers prints once, as the row's E item") {
    // The feature adds every example line of the real rows check 4 prints
    // (kind E) to a test file, with the real rules files on main.
    const std::map<std::string, nlohmann::json> rules = real_rules();
    std::string text = "int keep = 0;\n";
    for (const auto& [path, file] : rules)
        for (const nlohmann::json& row : file.at("rules"))
            if (row_kind(kTreeTest, row.at("owner").get<std::string>()) == "E")
                for (const nlohmann::json& example : row.value("must_match", nlohmann::json::array()))
                    text += example.get<std::string>() + "\n";
    RulesFixture fx("precheck_check4_row", rules, {{kTreeTest, text}});

    // The lines check 4's own spellings print when no row answers them.
    Run spellings(fx.dir);
    Context sctx = spellings.context();
    run_check_4_spellings(sctx, Check4Rows{});
    std::set<int> spelled;
    for (const Item& i : spellings.items.sorted())
        if (i.file == kTreeTest) spelled.insert(i.line);

    Run run(fx.dir);
    Context ctx = run.context();
    run_check_4(ctx);
    const std::vector<RowItem> row = row_items(ctx, "E");
    std::set<int> answered;
    for (const RowItem& i : row)
        if (i.file == kTreeTest) answered.insert(i.line);
    std::vector<int> both;
    std::set_intersection(spelled.begin(), spelled.end(), answered.begin(), answered.end(),
                          std::back_inserter(both));
    // The real "reads the source tree" row's example opens a source file, so
    // at least one line is both a row's and a spelling's.
    CHECK(!both.empty());
    const std::vector<Item> got = run.items.sorted();
    for (const int ln : both) {
        INFO(ln);
        std::set<std::pair<std::string, std::string>> want;
        for (const RowItem& i : row)
            if (i.file == kTreeTest && i.line == ln) want.insert({i.what, i.why});
        std::set<std::pair<std::string, std::string>> printed;
        for (const Item& i : items_at(got, kTreeTest, ln)) printed.insert({i.what, i.why});
        CHECK(printed == want);
    }
}

TEST_CASE("precheck check 4: a new rule row with no examples or a variable exemption") {
    // main lacks the first three rows of the real tests.json; the range adds
    // them back, one with its example lists emptied, one with variable names
    // exempted at the front of its pattern, and one as it is.
    std::map<std::string, nlohmann::json> main_rules = real_rules();
    const std::string tests_rules = rules_path("rules/tests.json");
    REQUIRE(main_rules.count(tests_rules) == 1);
    nlohmann::json tip_tests = main_rules.at(tests_rules);
    nlohmann::json& rows = tip_tests.at("rules");
    REQUIRE(rows.size() >= 3);
    const std::string exemption = "(?!(fx|run)\\s*\\.";
    nlohmann::json& bare_row = rows.at(std::size_t{0});
    nlohmann::json& exempting_row = rows.at(std::size_t{1});
    bare_row["must_match"] = nlohmann::json::array();
    bare_row["must_not_match"] = nlohmann::json::array();
    exempting_row["pattern"] = exemption + ")" + exempting_row.at("pattern").get<std::string>();
    const std::string bare = bare_row.at("question").get<std::string>();
    const std::string exempting = exempting_row.at("question").get<std::string>();
    const std::string kept = rows.at(std::size_t{2}).at("question").get<std::string>();
    nlohmann::json& main_rows = main_rules.at(tests_rules).at("rules");
    main_rows.erase(main_rows.begin(), main_rows.begin() + 3);
    RulesFixture fx("precheck_check4_added", main_rules, {{tests_rules, tip_tests.dump(2) + "\n"}});

    Run run(fx.dir);
    Context ctx = run.context();
    run_check_4(ctx);
    const std::vector<AddedRow> added = added_rows(ctx);
    REQUIRE(added.size() == 3);
    const std::vector<Item> got = run.items.sorted();
    std::map<std::string, std::size_t> per_row;
    for (const AddedRow& r : added) {
        INFO(r.rule.question);
        CHECK(r.file == tests_rules);
        for (const Item& i : items_at(got, r.file, r.line)) {
            const std::string& text = i.what;
            CHECK(i.kind == "E");
            CHECK(text.find("\"" + r.rule.question + "\"") != std::string::npos);
            if (r.rule.question == exempting) {
                CHECK(text.find(exemption) != std::string::npos);
            }
            ++per_row[r.rule.question];
        }
    }
    CHECK(per_row[bare] == 2);  // no must-match and no must-not-match examples
    CHECK(per_row[exempting] == 1);
    CHECK(per_row[kept] == 0);
    CHECK(got.size() == 3);
}
