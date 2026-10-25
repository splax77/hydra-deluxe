// The merge-test picker (tools/precheck/test_pick.h) on fixture repos: one
// level of callers with the selector values, deleted names, helper headers,
// changed test files, the three limits and the shared-code paths. Plan:
// docs/superpowers/plans/2026-10-25-merge-test-picker.md, Task 3 (a) to (i).
#include "doctest.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "precheck/git_text.h"
#include "precheck/range.h"
#include "precheck/test_pick.h"
#include "precheck_fixture.h"
#include "temp_util.h"

namespace fs = std::filesystem;
using namespace precheck;
using namespace precheck_fixture;

namespace {

// A fixture repo whose main branch holds `base`; pick() commits the tip on
// top and picks for that one commit.
struct PickRepo {
    std::string dir;
    PickRepo(const std::string& name, const std::map<std::string, std::string>& base)
        : dir(testtemp::temp_dir(name)) {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        fixture_git(dir, {"init", "-q", "-b", "main"});
        commit_texts(dir, base, "base");
    }
    ~PickRepo() { testtemp::remove_dir(dir); }

    TestPick pick(const std::map<std::string, std::string>& tip, const PickLimits& limits = {}) const {
        commit_texts(dir, tip, "tip");
        Repo repo(dir);
        return pick_tests(repo, resolve_range(repo, "HEAD", false, ""), limits);
    }
};

std::vector<std::string> picked_files(const TestPick& p) {
    std::vector<std::string> out;
    for (const PickedTest& t : p.tests) out.push_back(t.file);
    return out;
}

std::vector<std::string> reasons_of(const TestPick& p, const std::string& file) {
    for (const PickedTest& t : p.tests)
        if (t.file == file) return t.reasons;
    return {};
}

const PickName* name_in(const TestPick& p, const std::string& name) {
    for (const PickName& n : p.names)
        if (n.name == name) return &n;
    return nullptr;
}

const std::string kRunBase =
    "enum class Mode { A, B };\n"
    "void a() { int x = 1; }\n"
    "void b() { int y = 2; }\n"
    "void run(Mode m) { if (m == Mode::A) a(); else b(); }\n"
    "void outer() { run(Mode::B); }\n";

}  // namespace

TEST_CASE("precheck pick: the three limits are the user's numbers") {
    CHECK(kCommonNameLimit == 30);
    CHECK(kPickSizeLimit == 40);
    CHECK(kHeaderIncluderLimit == 20);
    const PickLimits d;
    CHECK(d.common_name == kCommonNameLimit);
    CHECK(d.pick_size == kPickSizeLimit);
    CHECK(d.header_includers == kHeaderIncluderLimit);
}

TEST_CASE("precheck pick (a): a test naming only the selector value of a changed function is picked") {
    PickRepo fx("precheck_pick_a",
                {{"src/run.cpp", kRunBase},
                 {"tests/test_mode.cpp", "int pick_mode() { return static_cast<int>(Mode::A); }\n"},
                 {"tests/test_run.cpp", "void go_run() { run(Mode::B); }\n"},
                 {"tests/test_outer.cpp", "void go_outer() { outer(); }\n"},
                 {"tests/test_other.cpp", "int other() { return 7; }\n"}});
    std::string tip = kRunBase;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"src/run.cpp", tip}});

    REQUIRE_FALSE(p.whole_suite);
    REQUIRE(p.names.size() == 1);
    CHECK(p.names[0].name == "a");
    CHECK(p.names[0].file == "src/run.cpp");
    CHECK_FALSE(p.names[0].base_only);

    REQUIRE(p.callers.size() == 2);
    CHECK(p.callers[0].key == "run");
    CHECK(p.callers[0].callee == "a");
    CHECK(p.callers[0].caller == "run");
    CHECK_FALSE(p.callers[0].selector);
    CHECK(p.callers[1].key == "Mode::A");
    CHECK(p.callers[1].callee == "a");
    CHECK(p.callers[1].caller == "run");
    CHECK(p.callers[1].selector);

    // test_outer names only outer, a caller two levels up: not followed.
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_mode.cpp", "tests/test_run.cpp"});
    CHECK(reasons_of(p, "tests/test_mode.cpp") == std::vector<std::string>{"Mode::A"});
    CHECK(reasons_of(p, "tests/test_run.cpp") == std::vector<std::string>{"run"});
}

TEST_CASE("precheck pick (a): a selector on the guard line above the call counts") {
    const std::string base =
        "enum class Rule { Old, New };\n"
        "void place() { int k = 1; }\n"
        "void check(Rule r) {\n"
        "    if (r == Rule::New) {\n"
        "        int z = 0;\n"
        "        place();\n"
        "    }\n"
        "}\n";
    PickRepo fx("precheck_pick_a_guard",
                {{"src/song.cpp", base}, {"tests/test_rule.cpp", "Rule pick_rule() { return Rule::New; }\n"}});
    std::string tip = base;
    tip.replace(tip.find("int k = 1;"), 10, "int k = 2;");
    const TestPick p = fx.pick({{"src/song.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(reasons_of(p, "tests/test_rule.cpp") == std::vector<std::string>{"Rule::New"});
}

TEST_CASE("precheck pick (b): a deleted class is a name and its members are not keys") {
    const std::string base =
        "#pragma once\n"
        "class Square {\n"
        "public:\n"
        "    int side() const { return s_; }\n"
        "    int s_ = 0;\n"
        "};\n"
        "int keep_me();\n";
    PickRepo fx("precheck_pick_b", {{"src/shapes.h", base},
                                    {"tests/test_square.cpp", "int sq() { Square q; return 0; }\n"},
                                    {"tests/test_side.cpp", "int sd() { return side() + s_; }\n"}});
    const TestPick p = fx.pick({{"src/shapes.h", "#pragma once\nint keep_me();\n"}});
    REQUIRE_FALSE(p.whole_suite);
    const PickName* sq = name_in(p, "Square");
    REQUIRE(sq != nullptr);
    CHECK(sq->base_only);
    CHECK(name_in(p, "side") == nullptr);
    CHECK(name_in(p, "s_") == nullptr);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_square.cpp"});
}

TEST_CASE("precheck pick (c): a key in a helper header picks every test file that includes it") {
    PickRepo fx("precheck_pick_c",
                {{"src/calc.cpp", "int scale(int v) { return v * 2; }\n"},
                 {"tests/calc_helpers.h", "#pragma once\ninline int twice(int v) { return scale(v); }\n"},
                 {"tests/test_c1.cpp", "#include \"calc_helpers.h\"\nint c1() { return twice(1); }\n"},
                 {"tests/test_c2.cpp", "#include \"calc_helpers.h\"\nint c2() { return twice(2); }\n"},
                 {"tests/test_c3.cpp", "int c3() { return 3; }\n"}});
    const TestPick p = fx.pick({{"src/calc.cpp", "int scale(int v) { return v * 3; }\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_c1.cpp", "tests/test_c2.cpp"});
    CHECK(reasons_of(p, "tests/test_c1.cpp") == std::vector<std::string>{"scale"});
}

TEST_CASE("precheck pick (c): a key only a comment in a helper header names picks nothing") {
    PickRepo fx("precheck_pick_c_comment",
                {{"src/calc.cpp", "int scale(int v) { return v * 2; }\n"},
                 {"tests/calc_helpers.h", "#pragma once\n// Built on scale.\ninline int twice(int v) { return v; }\n"},
                 {"tests/test_c1.cpp", "#include \"calc_helpers.h\"\nint c1() { return twice(1); }\n"}});
    const TestPick p = fx.pick({{"src/calc.cpp", "int scale(int v) { return v * 3; }\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(p.tests.empty());
}

TEST_CASE("precheck pick (d): a test file the range changes is picked") {
    PickRepo fx("precheck_pick_d", {{"tests/test_d.cpp", "int d1() { return 1; }\n"},
                                    {"tests/test_e.cpp", "int e1() { return 1; }\n"}});
    const TestPick p = fx.pick({{"tests/test_d.cpp", "int d1() { return 2; }\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_d.cpp"});
    CHECK(reasons_of(p, "tests/test_d.cpp") == std::vector<std::string>{"changed in range"});
}

TEST_CASE("precheck pick (e): a common caller key picks nothing; a common changed name runs everything") {
    const std::string base = "int leaf() { return 1; }\nint hub() { return leaf(); }\n";
    const std::map<std::string, std::string> files = {
        {"src/w.cpp", base},
        {"tests/test_e1.cpp", "int e1() { return hub(); }\n"},
        {"tests/test_e2.cpp", "int e2() { return hub(); }\n"},
        {"tests/test_e3.cpp", "int e3() { return hub(); }\n"},
        {"tests/test_e4.cpp", "int e4() { return leaf(); }\n"}};
    PickLimits tight;
    tight.common_name = 2;

    SUBCASE("the common key is a caller") {
        PickRepo fx("precheck_pick_e1", files);
        const TestPick p = fx.pick({{"src/w.cpp", "int leaf() { return 2; }\nint hub() { return leaf(); }\n"}}, tight);
        REQUIRE_FALSE(p.whole_suite);
        REQUIRE(p.common.size() == 1);
        CHECK(p.common[0].name == "hub");
        CHECK(p.common[0].files == 3);
        CHECK(picked_files(p) == std::vector<std::string>{"tests/test_e4.cpp"});
    }
    SUBCASE("the common key heads a changed definition") {
        PickRepo fx("precheck_pick_e2", files);
        const TestPick p =
            fx.pick({{"src/w.cpp", "int leaf() { return 1; }\nint hub() { return leaf() + 1; }\n"}}, tight);
        REQUIRE(p.whole_suite);
        CHECK(p.whole_suite->file == "src/w.cpp");
        CHECK(p.whole_suite->count == 3);
        CHECK(p.tests.empty());
    }
    SUBCASE("at the limit the key still picks") {
        PickRepo fx("precheck_pick_e3", files);
        tight.common_name = 3;
        const TestPick p =
            fx.pick({{"src/w.cpp", "int leaf() { return 1; }\nint hub() { return leaf() + 1; }\n"}}, tight);
        REQUIRE_FALSE(p.whole_suite);
        CHECK(p.common.empty());
        CHECK(picked_files(p).size() == 3);
    }
}

TEST_CASE("precheck pick (f): more picked files than the size limit runs the whole suite") {
    const std::map<std::string, std::string> files = {{"src/f.cpp", "int many() { return 1; }\n"},
                                                      {"tests/test_f1.cpp", "int f1() { return many(); }\n"},
                                                      {"tests/test_f2.cpp", "int f2() { return many(); }\n"},
                                                      {"tests/test_f3.cpp", "int f3() { return many(); }\n"}};
    PickLimits tight;
    SUBCASE("over") {
        tight.pick_size = 2;
        PickRepo fx("precheck_pick_f1", files);
        const TestPick p = fx.pick({{"src/f.cpp", "int many() { return 2; }\n"}}, tight);
        REQUIRE(p.whole_suite);
        CHECK(p.whole_suite->file.empty());
        CHECK(p.whole_suite->count == 3);
        CHECK(p.tests.empty());
    }
    SUBCASE("at the limit") {
        tight.pick_size = 3;
        PickRepo fx("precheck_pick_f2", files);
        const TestPick p = fx.pick({{"src/f.cpp", "int many() { return 2; }\n"}}, tight);
        REQUIRE_FALSE(p.whole_suite);
        CHECK(picked_files(p).size() == 3);
    }
}

TEST_CASE("precheck pick (g): a header with more includers than the limit runs the whole suite") {
    const std::map<std::string, std::string> files = {
        {"src/wide.h", "#pragma once\nint wide();\n"},
        {"src/u1.cpp", "#include \"wide.h\"\nint u1() { return wide(); }\n"},
        {"src/sub/u2.cpp", "#include \"wide.h\"\nint u2() { return 2; }\n"},
        {"tools/u3.cpp", "#include \"../src/wide.h\"\nint u3() { return 3; }\n"},
        {"tools/u4.cpp", "// #include \"wide.h\" is not a real include\nint u4() { return 4; }\n"}};
    const std::map<std::string, std::string> tip = {{"src/wide.h", "#pragma once\nint wide();\nint wider();\n"}};
    PickLimits tight;
    SUBCASE("over") {
        tight.header_includers = 2;
        PickRepo fx("precheck_pick_g1", files);
        const TestPick p = fx.pick(tip, tight);
        REQUIRE(p.whole_suite);
        CHECK(p.whole_suite->file == "src/wide.h");
        CHECK(p.whole_suite->count == 3);
    }
    SUBCASE("at the limit") {
        tight.header_includers = 3;
        PickRepo fx("precheck_pick_g2", files);
        CHECK_FALSE(fx.pick(tip, tight).whole_suite);
    }
}

namespace {

// shared_code on one path in a repo that holds nothing that includes it.
std::optional<WholeSuite> shared_code_of(const std::string& path) {
    PickRepo fx("precheck_pick_h", {{"src/x.cpp", "int x;\n"}});
    Repo repo(fx.dir);
    return shared_code(repo, "HEAD", path);
}

}  // namespace

TEST_CASE("precheck pick (h): a CMakeLists.txt in any folder runs the whole suite") {
    CHECK(shared_code_of("CMakeLists.txt"));
    CHECK(shared_code_of("third_party/foo/CMakeLists.txt"));
}
TEST_CASE("precheck pick (h): CMakePresets.json runs the whole suite") { CHECK(shared_code_of("CMakePresets.json")); }
TEST_CASE("precheck pick (h): a file under cmake/ runs the whole suite") { CHECK(shared_code_of("cmake/flags.cmake")); }
TEST_CASE("precheck pick (h): build_cpp.ps1 runs the whole suite") { CHECK(shared_code_of("build_cpp.ps1")); }
TEST_CASE("precheck pick (h): a workflow file runs the whole suite") { CHECK(shared_code_of(".github/workflows/ci.yml")); }
TEST_CASE("precheck pick (h): tests/test_main.cpp runs the whole suite") { CHECK(shared_code_of("tests/test_main.cpp")); }
TEST_CASE("precheck pick (h): tests/shards.txt runs the whole suite") { CHECK(shared_code_of("tests/shards.txt")); }
TEST_CASE("precheck pick (h): a file under third_party/doctest/ runs the whole suite") {
    CHECK(shared_code_of("third_party/doctest/doctest.h"));
}
TEST_CASE("precheck pick (h): other files are not shared code") {
    CHECK_FALSE(shared_code_of("README.md"));
    CHECK_FALSE(shared_code_of("src/x.cpp"));
    CHECK_FALSE(shared_code_of("tests/test_cli.cpp"));
    CHECK_FALSE(shared_code_of("third_party/json/json.hpp"));
}
TEST_CASE("precheck pick (h): a changed build file makes the pick the whole suite") {
    PickRepo fx("precheck_pick_h_pick", {{"CMakeLists.txt", "project(x)\n"}, {"tests/test_h.cpp", "int h;\n"}});
    const TestPick p = fx.pick({{"CMakeLists.txt", "project(y)\n"}, {"tests/test_h.cpp", "int h2;\n"}});
    REQUIRE(p.whole_suite);
    CHECK(p.whole_suite->file == "CMakeLists.txt");
    CHECK(p.tests.empty());
}

TEST_CASE("precheck pick (i): files under tests/single_owner/ are never picked") {
    PickRepo fx("precheck_pick_i",
                {{"src/k.cpp", "constexpr int kStamp = 1;\n"},
                 {"tests/single_owner/special.cpp", "int s() { return kStamp; }\n"},
                 {"tests/single_owner/other.cpp", "int o;\n"},
                 {"tests/test_k.cpp", "int k() { return kStamp; }\n"}});
    const TestPick p =
        fx.pick({{"src/k.cpp", "constexpr int kStamp = 2;\n"}, {"tests/single_owner/other.cpp", "int o2;\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_k.cpp"});
}
