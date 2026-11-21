// hydra_precheck --tests: the merge-test picker's printed lines
// (docs/development.md, "The merge test picker").
// The exe runs as a child process on fixture repos, and its lines must be
// pick_lines' lines for the picker's own result on the same repo: pick_lines
// owns the wording. Each test then checks the pick names the right thing. The
// line formatter's own test runs it on real picks from fixture repos.
#include "doctest.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "core/strutil.h"
#include "core/winstr.h"
#include "precheck/changed_names.h"
#include "precheck/git_text.h"
#include "precheck/range.h"
#include "precheck/test_pick.h"
#include "precheck_fixture.h"
#include "temp_util.h"

#ifndef HYDRA_PRECHECK_EXE
#error "HYDRA_PRECHECK_EXE must be defined (see CMakeLists.txt)"
#endif

namespace fs = std::filesystem;
using namespace precheck;
using namespace precheck_fixture;

namespace {

const std::string kRunBase =
    "enum class Mode { A, B };\n"
    "void a() { int x = 1; }\n"
    "void b() { int y = 2; }\n"
    "void run(Mode m) { if (m == Mode::A) a(); else b(); }\n";

// A fixture repo whose main branch holds `base`; commit_tip() commits the
// tip on top, so the range "HEAD" is that one commit.
struct ExeRepo {
    std::string dir;
    ExeRepo(const std::string& name, const std::map<std::string, std::string>& base)
        : dir(testtemp::temp_dir(name)) {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        fixture_git(dir, {"init", "-q", "-b", "main"});
        commit_texts(dir, base, "base");
    }
    ~ExeRepo() { testtemp::remove_dir(dir); }
    void commit_tip(const std::map<std::string, std::string>& tip) const { commit_texts(dir, tip, "tip"); }

    // The picker's own result for HEAD, as the exe's --tests mode computes it.
    TestPick pick(const PickLimits& limits = {}) const {
        Repo repo(dir);
        return pick_tests(repo, resolve_range(repo, "HEAD", false, ""), limits);
    }
    // The shared-code rule's answer for `file` at HEAD.
    std::optional<WholeSuite> shared(const std::string& file, int header_includer_limit) const {
        Repo repo(dir);
        return shared_code(repo, "HEAD", file, header_includer_limit);
    }
};

struct ExeRun {
    std::vector<std::string> lines;
    std::string err;
    int code = -1;
};

ExeRun run_tests_mode(const std::string& repo, const std::string& range) {
    ChildProcess p({HYDRA_PRECHECK_EXE, "--repo", repo, "--tests", range});
    ExeRun r;
    std::istringstream text(p.read_all());
    for (std::string l; std::getline(text, l);) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        r.lines.push_back(l);
    }
    r.code = p.wait(r.err);
    return r;
}

// Runs the exe on HEAD, checks it exits 0 and prints exactly pick_lines' lines
// for the picker's result, and returns that result.
TestPick exe_prints_pick(const ExeRepo& fx) {
    const ExeRun r = run_tests_mode(fx.dir, "HEAD");
    INFO(r.err);
    CHECK(r.code == 0);
    const TestPick pick = fx.pick();
    CHECK(r.lines == pick_lines(pick));
    return pick;
}

std::string tip_changing_a() {
    std::string tip = kRunBase;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    return tip;
}

// The comma-separated patterns of a `tests: "-sf=..."` line.
std::vector<std::string> patterns_of(const std::string& line) {
    const std::string head = "tests: \"-sf=";
    REQUIRE(line.rfind(head, 0) == 0);
    REQUIRE(line.size() > head.size());
    REQUIRE(line.back() == '"');
    std::vector<std::string> out;
    std::istringstream in(line.substr(head.size(), line.size() - head.size() - 1));
    for (std::string p; std::getline(in, p, ',');) out.push_back(p);
    return out;
}

// Whether one printed line holds every one of `parts`.
bool some_line_holds(const std::vector<std::string>& lines, const std::vector<std::string>& parts) {
    return std::any_of(lines.begin(), lines.end(), [&](const std::string& l) {
        return std::all_of(parts.begin(), parts.end(), [&](const std::string& p) { return l.find(p) != std::string::npos; });
    });
}

// Checks pick_lines(pick) names everything the pick holds: each changed name
// with its kind, each common key with its file count, and, for a whole-suite
// pick, a last line naming the file, the rule and the count it compared.
void check_lines_name_pick(const TestPick& pick) {
    const std::vector<std::string> lines = pick_lines(pick);
    REQUIRE_FALSE(lines.empty());
    for (const PickName& n : pick.names) {
        const std::string kind = n.base_only ? "deleted" : kind_word(n.kind);
        const std::string msg = "changed name " + n.name + " (" + kind + ")";
        CHECK_MESSAGE(some_line_holds(lines, {n.name, kind}), msg);
    }
    for (const CommonName& c : pick.common) {
        const std::string msg = "common key " + c.name;
        CHECK_MESSAGE(some_line_holds(lines, {c.name, std::to_string(c.files)}), msg);
    }
    if (pick.whole_suite) {
        const WholeSuite& w = *pick.whole_suite;
        CHECK(lines.front().find("-sf=") == std::string::npos);
        CHECK(lines.back().find(w.rule) != std::string::npos);
        // pick_lines prints the count only for a rule that compares one.
        if (w.count != 0) {
            CHECK(lines.back().find(std::to_string(w.count)) != std::string::npos);
        }
        if (!w.file.empty()) {
            CHECK(lines.back().find(w.file) != std::string::npos);
        }
    }
}

// The two-test fixture: both test files name the selector Mode::A, and the
// tip changes a(), which run() calls under Mode::A.
const std::map<std::string, std::string> kTwoTests = {
    {"src/run.cpp", kRunBase},
    {"tests/test_alpha.cpp", "int p() { return static_cast<int>(Mode::A); }\n"},
    {"tests/test_Zeta.cpp", "int q() { return static_cast<int>(Mode::A); }\n"}};

}  // namespace

TEST_CASE("precheck --tests: a test naming only the selector value is the tests line") {
    ExeRepo fx("precheck_pick_exe_a",
               {{"src/run.cpp", kRunBase},
                {"tests/test_mode.cpp", "int pick_mode() { return static_cast<int>(Mode::A); }\n"},
                {"tests/test_other.cpp", "int other() { return 7; }\n"}});
    fx.commit_tip({{"src/run.cpp", tip_changing_a()}});
    const TestPick pick = exe_prints_pick(fx);
    REQUIRE_FALSE(pick.whole_suite);
    REQUIRE(pick.tests.size() == 1);
    CHECK(pick.tests[0].file == "tests/test_mode.cpp");
    CHECK(pick.tests[0].reasons == std::vector<std::string>{"Mode::A"});
    // The selector came from the call of a() under Mode::A in run().
    CHECK(std::any_of(pick.callers.begin(), pick.callers.end(), [](const CallerKey& k) {
        return k.selector && k.key == "Mode::A" && k.callee == "a" && k.caller == "run";
    }));
    CHECK(patterns_of(pick_lines(pick).front()) == std::vector<std::string>{"*test_mode.cpp"});
}

TEST_CASE("precheck --tests: a changed build file prints the whole suite and why") {
    const std::string build_file = "CMakeLists.txt";
    ExeRepo fx("precheck_pick_exe_whole",
               {{build_file, "project(x)\n"}, {"tests/test_mode.cpp", "int m() { return 1; }\n"}});
    fx.commit_tip({{build_file, "project(y)\n"}});
    const TestPick pick = exe_prints_pick(fx);
    REQUIRE(pick.whole_suite);
    CHECK(pick.tests.empty());
    CHECK(pick.whole_suite->file == build_file);
    const std::optional<WholeSuite> rule = fx.shared(build_file, kHeaderIncluderLimit);
    REQUIRE(rule);
    CHECK(pick.whole_suite->rule == rule->rule);
    check_lines_name_pick(pick);
}

TEST_CASE("precheck --tests: a range changing no code prints no tests, not an empty -sf= filter") {
    // An empty -sf= filter runs every test case.
    ExeRepo fx("precheck_pick_exe_none", {{"docs/plan.md", "old\n"}, {"tests/test_mode.cpp", "int m() { return 1; }\n"}});
    fx.commit_tip({{"docs/plan.md", "new\n"}});
    const TestPick pick = exe_prints_pick(fx);
    CHECK(pick.tests.empty());
    CHECK_FALSE(pick.whole_suite);
    CHECK(pick_lines(pick).front().find("-sf=") == std::string::npos);
}

TEST_CASE("precheck --tests: a bad range exits 1 with git's error on stderr") {
    ExeRepo fx("precheck_pick_exe_bad", {{"src/run.cpp", kRunBase}});
    const ExeRun r = run_tests_mode(fx.dir, "no-such-ref...HEAD");
    CHECK(r.code == 1);
    CHECK(r.lines.empty());
    CHECK(r.err.find("no-such-ref") != std::string::npos);
}

TEST_CASE("precheck --tests: two picked files come in byte order, none ending in a bare *") {
    ExeRepo fx("precheck_pick_exe_two", kTwoTests);
    fx.commit_tip({{"src/run.cpp", tip_changing_a()}});
    const TestPick pick = exe_prints_pick(fx);
    REQUIRE(pick.tests.size() == 2);
    const std::vector<std::string> pats = patterns_of(pick_lines(pick).front());
    REQUIRE(pats.size() == 2);
    // Byte order puts 'Z' (0x5A) before 'a' (0x61), so a case-blind sort
    // would put test_alpha first and fail here.
    CHECK(std::is_sorted(pats.begin(), pats.end()));
    for (const std::string& p : pats) {
        CHECK(p.front() == '*');
        CHECK(p.back() != '*');
        // Each pattern is one picked file's name.
        const std::string one_name = "/" + p.substr(1);
        const auto names_file = [&](const PickedTest& t) { return hydra::ends_with(t.file, one_name); };
        CHECK(std::count_if(pick.tests.begin(), pick.tests.end(), names_file) == 1);
    }
}

TEST_CASE("precheck pick lines: real picks print their names, kinds, common counts and whole-suite reasons") {
    SUBCASE("a picked test and the changed names with their kinds") {
        ExeRepo fx("precheck_pick_lines_names", kTwoTests);
        fx.commit_tip({{"src/run.cpp", tip_changing_a()}});
        const TestPick pick = fx.pick();
        REQUIRE_FALSE(pick.names.empty());
        check_lines_name_pick(pick);
    }
    SUBCASE("a common key with its file count") {
        // Both test files name Mode::A, one more than this limit.
        PickLimits tight;
        tight.common_name = 1;
        ExeRepo fx("precheck_pick_lines_common", kTwoTests);
        fx.commit_tip({{"src/run.cpp", tip_changing_a()}});
        const TestPick pick = fx.pick(tight);
        REQUIRE(pick.common.size() == 1);
        CHECK(pick.common[0].name == "Mode::A");
        CHECK(pick.common[0].files == 2);
        check_lines_name_pick(pick);
    }
    SUBCASE("more picked test files than the size limit") {
        PickLimits tight;
        tight.pick_size = 1;
        ExeRepo fx("precheck_pick_lines_size", kTwoTests);
        fx.commit_tip({{"src/run.cpp", tip_changing_a()}});
        const TestPick pick = fx.pick(tight);
        REQUIRE(pick.whole_suite);
        CHECK(pick.whole_suite->file.empty());
        const auto test_files = std::count_if(kTwoTests.begin(), kTwoTests.end(), [](const auto& f) {
            return hydra::starts_with(f.first, "tests/");
        });
        CHECK(pick.whole_suite->count == static_cast<int>(test_files));
        check_lines_name_pick(pick);
    }
    SUBCASE("a header with more direct includers than the limit") {
        const std::string header = "src/wide.h";
        const std::map<std::string, std::string> includers = {
            {"src/u1.cpp", "#include \"wide.h\"\nint u1() { return wide(); }\n"},
            {"src/u2.cpp", "#include \"wide.h\"\nint u2() { return 2; }\n"}};
        std::map<std::string, std::string> files = includers;
        files[header] = "#pragma once\nint wide();\n";
        PickLimits tight;
        tight.header_includers = static_cast<int>(includers.size()) - 1;
        ExeRepo fx("precheck_pick_lines_header", files);
        fx.commit_tip({{header, "#pragma once\nint wide();\nint wider();\n"}});
        const TestPick pick = fx.pick(tight);
        REQUIRE(pick.whole_suite);
        CHECK(pick.whole_suite->file == header);
        CHECK(pick.whole_suite->count == static_cast<int>(includers.size()));
        const std::optional<WholeSuite> rule = fx.shared(header, tight.header_includers);
        REQUIRE(rule);
        CHECK(pick.whole_suite->rule == rule->rule);
        check_lines_name_pick(pick);
    }
}
