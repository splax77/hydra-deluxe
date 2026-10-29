// hydra_precheck --tests: the merge-test picker's printed lines (plan
// docs/superpowers/plans/2026-10-25-merge-test-picker.md, Step 7 and Task 4).
// The line formatter runs directly on hand-made picks; the exe runs as a
// child process on fixture repos.
#include "doctest.h"

#include <filesystem>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "precheck/git_text.h"
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

const std::vector<std::string> kAlways = {
    "always: \"-tc=docs match code*\"",
    "always: \"-sf=*single_owner*\"",
    "always: \"-sf=*test_precheck*\"",
};

// The printed lines: the tests line, the three always lines, then `rest`.
std::vector<std::string> expected_lines(const std::string& tests_line, const std::vector<std::string>& rest) {
    std::vector<std::string> out{tests_line};
    out.insert(out.end(), kAlways.begin(), kAlways.end());
    out.insert(out.end(), rest.begin(), rest.end());
    return out;
}

// Lines 1 to 3 of a printed run, the always lines.
std::vector<std::string> always_of(const std::vector<std::string>& lines) {
    REQUIRE(lines.size() >= 4);
    return {lines.begin() + 1, lines.begin() + 4};
}

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

}  // namespace

TEST_CASE("precheck --tests: a test naming only the selector value is the tests line") {
    ExeRepo fx("precheck_pick_exe_a",
               {{"src/run.cpp", kRunBase},
                {"tests/test_mode.cpp", "int pick_mode() { return static_cast<int>(Mode::A); }\n"},
                {"tests/test_other.cpp", "int other() { return 7; }\n"}});
    fx.commit_tip({{"src/run.cpp", tip_changing_a()}});
    const ExeRun r = run_tests_mode(fx.dir, "HEAD");
    INFO(r.err);
    CHECK(r.code == 0);
    CHECK(r.lines == expected_lines("tests: \"-sf=*test_mode.cpp\"",
                                    {
                                        // One-line function: the changed line is its head line.
                                        "names: a (declared)",
                                        "callers: run <- a; Mode::A selects a in run",
                                        "common: (none)",
                                        "why: tests/test_mode.cpp  Mode::A",
                                    }));
}

TEST_CASE("precheck --tests: a changed build file prints the whole suite and why") {
    ExeRepo fx("precheck_pick_exe_whole",
               {{"CMakeLists.txt", "project(x)\n"}, {"tests/test_mode.cpp", "int m() { return 1; }\n"}});
    fx.commit_tip({{"CMakeLists.txt", "project(y)\n"}});
    const ExeRun r = run_tests_mode(fx.dir, "HEAD");
    INFO(r.err);
    CHECK(r.code == 0);
    CHECK(r.lines == expected_lines("tests: whole suite", {
                                                              "names: (none)",
                                                              "callers: (none)",
                                                              "common: (none)",
                                                              "why: CMakeLists.txt  build file",
                                                          }));
}

TEST_CASE("precheck --tests: a range changing no code prints no tests, not an empty -sf= filter") {
    // An empty -sf= filter runs every test case.
    ExeRepo fx("precheck_pick_exe_none", {{"docs/plan.md", "old\n"}, {"tests/test_mode.cpp", "int m() { return 1; }\n"}});
    fx.commit_tip({{"docs/plan.md", "new\n"}});
    const ExeRun r = run_tests_mode(fx.dir, "HEAD");
    INFO(r.err);
    CHECK(r.code == 0);
    REQUIRE(r.lines.size() >= 4);
    CHECK(r.lines[0] == "tests: (none)");
    CHECK(always_of(r.lines) == kAlways);
    CHECK(r.lines.back() == "why: no changed code names a test");
}

TEST_CASE("precheck --tests: a bad range exits 1 with git's error on stderr") {
    ExeRepo fx("precheck_pick_exe_bad", {{"src/run.cpp", kRunBase}});
    const ExeRun r = run_tests_mode(fx.dir, "no-such-ref...HEAD");
    CHECK(r.code == 1);
    CHECK(r.lines.empty());
    CHECK(r.err.find("no-such-ref") != std::string::npos);
}

TEST_CASE("precheck --tests: two picked files come in byte order, none ending in a bare *") {
    ExeRepo fx("precheck_pick_exe_two",
               {{"src/run.cpp", kRunBase},
                {"tests/test_alpha.cpp", "int p() { return static_cast<int>(Mode::A); }\n"},
                {"tests/test_Zeta.cpp", "int q() { return static_cast<int>(Mode::A); }\n"}});
    fx.commit_tip({{"src/run.cpp", tip_changing_a()}});
    const ExeRun r = run_tests_mode(fx.dir, "HEAD");
    INFO(r.err);
    CHECK(r.code == 0);
    REQUIRE(r.lines.size() >= 4);
    const std::vector<std::string> pats = patterns_of(r.lines[0]);
    // 'Z' (0x5A) sorts before 'a' (0x61) in byte order.
    CHECK(pats == std::vector<std::string>{"*test_Zeta.cpp", "*test_alpha.cpp"});
    for (const std::string& p : pats) CHECK(p.back() != '*');
    CHECK(always_of(r.lines) == kAlways);
}

TEST_CASE("precheck pick lines: patterns sort by name, kinds and counts print, whole-suite counts show") {
    TestPick p;
    p.tests = {{"tests/test_b.cpp", {"changed in range", "Song"}}, {"tests/test_a.cpp", {"Gone"}}};
    p.names = {{"Gone", NameKind::Declared, "src/x.h", true}, {"place", NameKind::Comment, "src/x.cpp", false}};
    p.common = {{"ticks", 46}, {"Song", 77}};
    CHECK(pick_lines(p) == expected_lines("tests: \"-sf=*test_a.cpp,*test_b.cpp\"",
                                          {
                                              "names: Gone (deleted) place (comment)",
                                              "callers: (none)",
                                              "common: ticks (46 files) Song (77 files)",
                                              "why: tests/test_a.cpp  Gone",
                                              "why: tests/test_b.cpp  changed in range; Song",
                                          }));

    TestPick whole;
    whole.whole_suite = WholeSuite{"src/core/model.h", "header with more direct includers than 20", 94};
    const std::vector<std::string> lines = pick_lines(whole);
    REQUIRE(lines.size() == 8);
    CHECK(lines[0] == "tests: whole suite");
    CHECK(lines[7] == "why: src/core/model.h  header with more direct includers than 20 (94)");

    TestPick size;
    size.whole_suite = WholeSuite{"", "more picked test files than 40", 45};
    CHECK(pick_lines(size).back() == "why: more picked test files than 40 (45)");
}
