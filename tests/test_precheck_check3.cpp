// hydra_precheck's check 3, numbers with no decision (kind D): the number
// reading and paragraph split in tools/precheck/decisions.h, and the check
// itself on a tiny git repo in a temp folder whose docs decide some numbers
// and not others.
#include "doctest.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "precheck/checks.h"
#include "precheck/decisions.h"
#include "precheck/git_text.h"
#include "precheck/items.h"
#include "precheck/range.h"
#include "temp_util.h"

namespace fs = std::filesystem;
using namespace precheck;

namespace {

// Writes each file under `dir` and commits them all on the current branch.
void commit_texts(const std::string& dir, const std::map<std::string, std::string>& texts, const std::string& message) {
    for (const auto& [rel, text] : texts) {
        const std::string path = dir + "\\" + rel;
        fs::create_directories(hydra::os_path(fs::path(hydra::os_path(path)).parent_path()));
        testtemp::write_bytes(path, std::vector<uint8_t>(text.begin(), text.end()));
    }
    const std::vector<std::string> who = {"-c", "user.name=precheck", "-c", "user.email=precheck@example.invalid",
                                          "-c", "core.autocrlf=false"};
    std::vector<std::string> add = who;
    add.insert(add.end(), {"add", "-A"});
    run_git(dir, add);
    std::vector<std::string> commit = who;
    commit.insert(commit.end(), {"commit", "-q", "-m", message});
    run_git(dir, commit);
}

// Check 3's item lines and printed text for `spelling` in `dir`.
struct Check3Run {
    std::vector<std::string> lines;
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
    for (const Item& item : items.sorted()) run.lines.push_back(item_line(item));
    run.printed = out.str();
    return run;
}

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
    CHECK(record_name(true, "0014") == "ADR 14");
    CHECK(record_name(false, "049") == "D49");
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
    CHECK(run.lines == std::vector<std::string>{
                           "D src/offsets.cpp:1  37 in: const int kOffsetLimit = 37;  -- no ADR, CONTEXT.md, plan "
                           "decision or D-record names 37 beside any of: offset, limit",
                           "D src/offsets.cpp:6  77 in: const int kDepth = 77;  -- cites D49, which does not "
                           "contain 77",
                           "D src/offsets.cpp:11  78 in: const int kDepth2 = 78;  -- no ADR, CONTEXT.md, plan "
                           "decision or D-record names 78 beside any of: depth2",
                           "D tests/test_offsets.cpp:2  4322 in: CHECK(gap_ms < 4322);  -- no ADR, CONTEXT.md, "
                           "plan decision or D-record names 4322 beside any of: gap",
                       });
}

TEST_CASE("precheck check 3: a whole-tree run has no new numbers") {
    Fixture fx;
    const Check3Run run = run_check3(fx.dir, "feature", true);
    CHECK(run.printed == "check 3 skipped: -WholeTree has no \"new\" numbers\n");
    CHECK(run.lines.empty());
}
