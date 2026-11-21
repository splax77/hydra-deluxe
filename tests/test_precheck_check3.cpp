// hydra_precheck's check 3, numbers with no decision (kind D): the number
// reading and paragraph split in tools/precheck/decisions.h, and the check
// itself on a tiny git repo in a temp folder whose docs decide some numbers
// and not others.
#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "core/winstr.h"
#include "precheck/checks.h"
#include "precheck/decisions.h"
#include "precheck/git_text.h"
#include "precheck/items.h"
#include "precheck/range.h"
#include "precheck_fixture.h"
#include "source_tree.h"
#include "temp_util.h"

namespace fs = std::filesystem;
using namespace precheck;

using precheck_fixture::commit_texts;

namespace {

// Check 3's items and printed text for `spelling` in `dir`.
struct Check3Run {
    std::vector<Item> items;
    std::string printed;
};
Check3Run run_check3(const std::string& dir, const std::string& spelling, bool whole_tree) {
    Repo repo(dir);
    const Range range = resolve_range(repo, spelling, whole_tree, "");
    const std::vector<std::string> files = all_files(repo, range.tip);
    const AddedLines added =
        whole_tree ? AddedLines::whole(files) : AddedLines::from_diff(repo, range.base, range.tip);
    Items items;
    std::ostringstream out;
    Context ctx{repo, range, added, files, items, out};
    run_check_3(ctx);
    Check3Run run;
    run.items = items.sorted();
    run.printed = out.str();
    return run;
}

bool holds(const std::string& text, const std::string& part) { return text.find(part) != std::string::npos; }

// A D item's text names the number it flags, the line it read it from and
// `about`: what the number is, or the record a citation names.
void check_names(const Item& item, const std::string& number, const std::string& code, const std::string& about) {
    INFO(item.file + ":" + std::to_string(item.line));
    CHECK(item.kind == "D");
    CHECK(item.what.rfind(number, 0) == 0);
    CHECK(holds(item.what, code));
    CHECK(holds(item.why, number));
    CHECK(holds(item.why, about));
}

bool is_digit(char c) { return c >= '0' && c <= '9'; }

// The fixture: main decides 45 for an offset limit, 65,535 for a mask, 2^46
// for a shift and D49's depth of 30; feature adds src/offsets.cpp and
// tests/test_offsets.cpp; then main records a window of 64 after the fork.
struct Fixture {
    std::string dir = testtemp::temp_dir("precheck_check3");
    Fixture() {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        run_git(dir, {"init", "-q", "-b", "main"});
        commit_texts(dir,
                     {{"docs/adr/0014-offsets.md",
                       "# Offsets\n\nThe offset limit is 45 ms.\n\nThe mask is 65,535 wide.\n\n"
                       "- A shift of 2^46 bits.\n"},
                      {"docs/audit/2026-10-03-fix-decisions.md",
                       "# Decisions\n\n**D49** The search depth is 30.\n"},
                      {"src/keep.cpp", "int keep = 0;\n"}},
                     "main1");
        run_git(dir, {"checkout", "-q", "-b", "feature"});
        commit_texts(dir,
                     {{"src/offsets.cpp",
                       "const int kOffsetLimit = 37;\n"         // 1: no decision
                       "int offset_limit = 45;\n"               // 2: decided by ADR 14
                       "const unsigned kMaskWidth = 0xFFFF;\n"  // 3: 65,535
                       "const int kShiftBits = 46;\n"           // 4: 2^46
                       "// D49\n"                               // 5
                       "const int kDepth = 77;\n"               // 6: cites D49, not there
                       "// D49 again\n"                         // 7
                       "\n"                                     // 8
                       "\n"                                     // 9
                       "\n"                                     // 10
                       "const int kDepth2 = 78;\n"              // 11: the citation is four lines up
                       "int window = 64;\n"                     // 12: decided on main after the fork
                       "int scratch = 2;\n"                     // 13: 0, 1 and 2 are skipped
                       "int idx = buf[311];\n"                  // 14: an index
                       "int keys[256];\n"                       // 15: a container's size
                       "bool fits = i < 256;\n"                 // 16: that size again
                       "long to_ms(long s) { return s * 1000; }\n"  // 17: a unit factor
                       "#if LIMIT > 999\n"                      // 18: a preprocessor line
                       "#endif\n"                               // 19
                       "static_assert(LIMIT < 998);\n"          // 20: static_assert
                       "int w = fit(buf, 316);\n"},             // 21: passed to a call
                      {"tests/test_offsets.cpp",
                       "CHECK(score == 4321);\n"                // 1: a pinned result
                       "CHECK(gap_ms < 4322);\n"},              // 2: a test's comparison
                     },
                     "f1");
        run_git(dir, {"checkout", "-q", "main"});
        commit_texts(dir, {{"docs/adr/0015-window.md", "The window is 64 wide.\n"}}, "main2");
    }
    ~Fixture() { testtemp::remove_dir(dir); }
};

}  // namespace

TEST_CASE("precheck check 3: numbers read by value") {
    CHECK(number_canon(*number_of("1'000'000")) == "1000000");
    CHECK(number_canon(*number_of("1,000,000")) == "1000000");
    CHECK(number_canon(*number_of("0xFFFF")) == "65535");
    CHECK(number_canon(*number_of("500.0")) == "500");
    CHECK(number_canon(*number_of("2.5f")) == "2.5");
    CHECK(number_canon(*number_of("0x1Fu")) == "31");
    CHECK(!number_of("0x").has_value());

    const std::set<std::string> doc = doc_numbers("65,535 and 1 000 000, 2x40, v2 and a.5, 2^46, 1,0000");
    // "2x40" gives 2 and 40; "v2" and "a.5" give nothing; "2^46" gives 2, 46
    // and the power; "1,0000" is 1 and then 0000.
    CHECK(doc == std::set<std::string>{"0", "1", "2", "40", "46", "65535", "1000000", "70368744177664"});
}

TEST_CASE("precheck check 3: a doc splits at blank lines and list items") {
    const std::string text = "Intro\n\nOne\n- two\n  3. three\nfour\n\n\n";
    CHECK(split_paras(text, false) == std::vector<std::string>{"Intro", "One", "- two", "  3. three\nfour"});
    CHECK(split_paras(text, true) == std::vector<std::string>{"Intro", "One\n- two\n  3. three\nfour"});
}

TEST_CASE("precheck check 3: a citation and the record it cites go by one name in the repo's own docs") {
    // The records check 3 reads from this repo's docs, by the name each goes by.
    Repo repo(sourcetree::root().u8string());
    std::set<std::string> records;
    for (const DecisionPara& p : decision_paras(repo, {"HEAD"})) records.insert(p.record);

    // Each numbered ADR: a citation of its number, with or without the
    // leading zeros its file name has, names the record read from the file.
    int adrs = 0;
    for (const std::string& rel : sourcetree::git_listed(sourcetree::root(), "docs/adr")) {
        const std::string file = rel.substr(rel.rfind('/') + 1);
        if (file.size() < 4 || !std::all_of(file.begin(), file.begin() + 4, is_digit)) continue;
        const std::string digits = file.substr(0, 4);
        const size_t first = digits.find_first_not_of('0');
        if (first == std::string::npos) continue;
        ++adrs;
        INFO(rel);
        const std::string cited = record_name(true, digits);
        CHECK(records.count(cited) == 1);
        CHECK(record_name(true, digits.substr(first)) == cited);
        // A D-record of the same number is another record.
        CHECK(record_name(false, digits) != cited);
    }
    REQUIRE(adrs > 0);

    // Each D-record the decision logs open: a citation with a leading zero
    // names it too.
    int d_records = 0;
    for (const std::string& r : records) {
        if (r.size() < 2 || r[0] != 'D' || !std::all_of(r.begin() + 1, r.end(), is_digit)) continue;
        ++d_records;
        INFO(r);
        CHECK(record_name(false, "0" + r.substr(1)) == r);
    }
    REQUIRE(d_records > 0);
}

TEST_CASE("precheck check 3: a plan's decisions are its User decisions block and Test limits sections") {
    const std::string plan =
        "# Plan\n\nWindow 10.\n\n**User decisions (2026-10-24)**\n\nDepth 20.\n\n**Questions**\n\nCap 30.\n\n"
        "## Test limits\n\nSeeds 40.\n\n### Sub\n\nFrames 50.\n\n## Next\n\nProbe 60.\n";
    // A block keeps the blank line before the line that ends it.
    CHECK(plan_decision_paras(plan) == std::vector<std::string>{"**User decisions (2026-10-24)**", "Depth 20.\n",
                                                                "## Test limits", "Seeds 40.", "### Sub",
                                                                "Frames 50.\n"});
}

TEST_CASE("precheck check 3: new numbers with no decision print D, decided ones do not") {
    Fixture fx;
    const Check3Run run = run_check3(fx.dir, "main...feature", false);
    CHECK(run.printed.empty());
    // Only the fixture's undecided lines are flagged; every other line is
    // decided or skipped (the fixture's comments say why).
    std::vector<std::pair<std::string, int>> flagged;
    for (const Item& item : run.items) flagged.emplace_back(item.file, item.line);
    REQUIRE(flagged == std::vector<std::pair<std::string, int>>{
                           {"src/offsets.cpp", 1}, {"src/offsets.cpp", 6}, {"src/offsets.cpp", 11},
                           {"tests/test_offsets.cpp", 2}});
    // Each names what the number is, from the line's own words...
    check_names(run.items[0], "37", "const int kOffsetLimit = 37;", "offset");
    check_names(run.items[2], "78", "const int kDepth2 = 78;", "depth2");
    check_names(run.items[3], "4322", "CHECK(gap_ms < 4322);", "gap");
    // ...or the record its citation names, which lacks it.
    check_names(run.items[1], "77", "const int kDepth = 77;", "D49");
}

TEST_CASE("precheck check 3: a whole-tree run has no new numbers") {
    Fixture fx;
    const Check3Run run = run_check3(fx.dir, "feature", true);
    // One line says check 3 was skipped, and it finds nothing.
    CHECK(run.printed.rfind("check 3 skipped", 0) == 0);
    CHECK(std::count(run.printed.begin(), run.printed.end(), '\n') == 1);
    CHECK(run.items.empty());
}
