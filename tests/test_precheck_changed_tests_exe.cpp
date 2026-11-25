// hydra_precheck --changed-tests: the C++ test files a range changes.
// The exe runs as a child process on fixture repos the test commits, and each
// expected path is one the test itself wrote. Which files count as C++ tests is
// test_file and is_cpp (precheck/range.h); the cases call those to show each
// fixture file is the kind the case says it is.
#include "doctest.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "precheck/git_text.h"
#include "precheck/range.h"
#include "precheck_fixture.h"
#include "temp_util.h"

#ifndef HYDRA_PRECHECK_EXE
#error "HYDRA_PRECHECK_EXE must be defined (see CMakeLists.txt)"
#endif

namespace fs = std::filesystem;
using namespace precheck;
using namespace precheck_fixture;

namespace {

const std::string kSrc = "src/run.cpp";
const std::string kCppTest = "tests/test_alpha.cpp";
const std::string kOtherCppTest = "tests/test_Zeta.cpp";
const std::string kPyTest = "tools/test_tool.py";
const std::string kDoc = "docs/plan.md";

// One of each kind of file, all in the base commit.
const std::map<std::string, std::string> kBase = {{kSrc, "int run() { return 1; }\n"},
                                                  {kCppTest, "int a() { return 1; }\n"},
                                                  {kOtherCppTest, "int z() { return 1; }\n"},
                                                  {kPyTest, "def test_one():\n    assert True\n"},
                                                  {kDoc, "old\n"}};

ExeRun run_changed_tests(const std::string& repo, const std::string& range) {
    return run_exe_mode(repo, "--changed-tests", range);
}

}  // namespace

TEST_CASE("precheck --changed-tests: a changed C++ test file is printed, alone") {
    REQUIRE(test_file(kCppTest));
    REQUIRE(is_cpp(kCppTest));
    REQUIRE_FALSE(test_file(kSrc));
    FixtureRepo fx("precheck_changed_tests_cpp", kBase);
    // The tip also changes a source file and a doc, which are not tests.
    fx.commit_tip({{kCppTest, "int a() { return 2; }\n"}, {kSrc, "int run() { return 2; }\n"}, {kDoc, "new\n"}});
    const ExeRun r = run_changed_tests(fx.dir, "HEAD");
    INFO(r.err);
    CHECK(r.code == 0);
    CHECK(r.lines == std::vector<std::string>{kCppTest});
}

TEST_CASE("precheck --changed-tests: two changed C++ test files come in byte order") {
    FixtureRepo fx("precheck_changed_tests_two", kBase);
    fx.commit_tip({{kCppTest, "int a() { return 2; }\n"}, {kOtherCppTest, "int z() { return 2; }\n"}});
    const ExeRun r = run_changed_tests(fx.dir, "HEAD");
    INFO(r.err);
    CHECK(r.code == 0);
    REQUIRE(r.lines.size() == 2);
    // Byte order puts 'Z' (0x5A) before 'a' (0x61), so a case-blind sort
    // would put test_alpha first and fail here.
    CHECK(std::is_sorted(r.lines.begin(), r.lines.end()));
    CHECK(std::count(r.lines.begin(), r.lines.end(), kCppTest) == 1);
    CHECK(std::count(r.lines.begin(), r.lines.end(), kOtherCppTest) == 1);
}

TEST_CASE("precheck --changed-tests: a range changing only source code prints nothing and exits 0") {
    REQUIRE_FALSE(test_file(kSrc));
    FixtureRepo fx("precheck_changed_tests_src", kBase);
    fx.commit_tip({{kSrc, "int run() { return 2; }\n"}});
    const ExeRun r = run_changed_tests(fx.dir, "HEAD");
    INFO(r.err);
    CHECK(r.code == 0);
    CHECK(r.lines.empty());
}

TEST_CASE("precheck --changed-tests: a changed Python test is a test but not C++, so nothing is printed") {
    REQUIRE(test_file(kPyTest));
    REQUIRE_FALSE(is_cpp(kPyTest));
    FixtureRepo fx("precheck_changed_tests_py", kBase);
    fx.commit_tip({{kPyTest, "def test_one():\n    assert 1 == 1\n"}});
    const ExeRun r = run_changed_tests(fx.dir, "HEAD");
    INFO(r.err);
    CHECK(r.code == 0);
    CHECK(r.lines.empty());
}

TEST_CASE("precheck --changed-tests: a deleted C++ test file is not printed") {
    REQUIRE(test_file(kCppTest));
    REQUIRE(is_cpp(kCppTest));
    FixtureRepo fx("precheck_changed_tests_deleted", kBase);
    fx.commit_delete(kCppTest);
    const ExeRun r = run_changed_tests(fx.dir, "HEAD");
    INFO(r.err);
    CHECK(r.code == 0);
    CHECK(r.lines.empty());
}

TEST_CASE("precheck --changed-tests: a bad range exits 1 with git's error on stderr") {
    FixtureRepo fx("precheck_changed_tests_bad", kBase);
    const ExeRun r = run_changed_tests(fx.dir, "no-such-ref...HEAD");
    CHECK(r.code == 1);
    CHECK(r.lines.empty());
    CHECK(r.err.find("no-such-ref") != std::string::npos);
}
