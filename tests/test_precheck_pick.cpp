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

TEST_CASE("precheck pick (b): a deleted class's multi-line member body gives the class, not the member") {
    const std::string base =
        "#pragma once\n"
        "class Square {\n"
        "public:\n"
        "    int side() const {\n"
        "        const int twice = s_ * 2;\n"
        "        return twice / 2;\n"
        "    }\n"
        "    int s_ = 0;\n"
        "};\n"
        "int keep_me();\n";
    PickRepo fx("precheck_pick_b2", {{"src/shapes.h", base},
                                     {"tests/test_square.cpp", "int sq() { Square q; return 0; }\n"},
                                     {"tests/test_side.cpp", "int sd() { return side() + s_; }\n"}});
    const TestPick p = fx.pick({{"src/shapes.h", "#pragma once\nint keep_me();\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(name_in(p, "Square") != nullptr);
    CHECK(name_in(p, "side") == nullptr);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_square.cpp"});
}

TEST_CASE("precheck pick (a): a function changed only by the comment above it does not walk its callers") {
    const std::string base =
        "void deep() { int x = 1; }\n"
        "// old words\n"
        "void shallow() { deep(); }\n"
        "void top() { shallow(); }\n";
    PickRepo fx("precheck_pick_a3", {{"src/chain.cpp", base},
                                     {"tests/test_shallow.cpp", "void t1() { shallow(); }\n"},
                                     {"tests/test_top.cpp", "void t2() { top(); }\n"}});
    std::string tip = base;
    tip.replace(tip.find("old words"), 9, "new words");
    const TestPick p = fx.pick({{"src/chain.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    REQUIRE(p.names.size() == 1);
    CHECK(p.names[0].name == "shallow");
    CHECK(p.names[0].kind == NameKind::Comment);
    CHECK(p.callers.empty());
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_shallow.cpp"});
}

TEST_CASE("precheck pick (a): a call to an unrelated function of the same name is not a caller") {
    const std::string one_cpp = "#include \"one.h\"\nvoid run() {\n    int x = 1;\n}\n";
    PickRepo fx("precheck_pick_a_scope",
                {{"src/a/one.h", "#pragma once\nenum class Mode { A, B };\nvoid run();\n"},
                 {"src/a/one.cpp", one_cpp},
                 {"src/a/user.cpp",
                  "#include \"one.h\"\nvoid use(Mode m) {\n    if (m == Mode::A) {\n        run();\n    }\n}\n"},
                 {"src/b/two.cpp",
                  "enum class Kind { A, B };\nstatic void run() { int y = 2; }\nvoid go(Kind k) {\n"
                  "    if (k == Kind::B) {\n        run();\n    }\n}\n"},
                 {"tests/test_mode_a.cpp",
                  "#include \"a/one.h\"\nint ma() { return static_cast<int>(Mode::A); }\n"},
                 {"tests/test_kind_b.cpp", "int kb() { return static_cast<int>(Kind::B); }\n"}});
    std::string tip = one_cpp;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"src/a/one.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    REQUIRE(name_in(p, "run") != nullptr);
    for (const CallerKey& k : p.callers) CHECK(k.caller != "go");
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_mode_a.cpp"});
    CHECK(reasons_of(p, "tests/test_mode_a.cpp") == std::vector<std::string>{"Mode::A"});
}

TEST_CASE("precheck pick (a): a caller that includes the declaring header through another header counts") {
    const std::string one_cpp = "#include \"one.h\"\nvoid run() {\n    int x = 1;\n}\n";
    PickRepo fx("precheck_pick_a_chain",
                {{"src/a/one.h", "#pragma once\nenum class Mode { A, B };\nvoid run();\n"},
                 {"src/a/one.cpp", one_cpp},
                 {"src/a/mid.h", "#pragma once\n#include \"one.h\"\n"},
                 {"src/c/far.cpp",
                  "#include \"a/mid.h\"\nvoid far(Mode m) {\n    if (m == Mode::B) {\n        run();\n    }\n}\n"},
                 {"tests/test_mode_b.cpp",
                  "#include \"a/one.h\"\nint mb() { return static_cast<int>(Mode::B); }\n"}});
    std::string tip = one_cpp;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"src/a/one.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_mode_b.cpp"});
    CHECK(reasons_of(p, "tests/test_mode_b.cpp") == std::vector<std::string>{"Mode::B"});
}

TEST_CASE("precheck pick (a): a .cpp that reaches its declaring header only through another header still has its callers") {
    const std::string one_cpp = "#include \"mid.h\"\nvoid run() {\n    int x = 1;\n}\n";
    PickRepo fx("precheck_pick_a_indirect",
                {{"src/a/one.h", "#pragma once\nenum class Mode { A, B };\nvoid run();\n"},
                 {"src/a/mid.h", "#pragma once\n#include \"one.h\"\n"},
                 {"src/a/one.cpp", one_cpp},
                 {"src/a/user.cpp",
                  "#include \"one.h\"\nvoid use(Mode m) {\n    if (m == Mode::B) {\n        run();\n    }\n}\n"},
                 {"tests/test_mode_b.cpp",
                  "#include \"a/one.h\"\nint mb() { return static_cast<int>(Mode::B); }\n"}});
    std::string tip = one_cpp;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"src/a/one.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_mode_b.cpp"});
}

TEST_CASE("precheck pick (a): a function only in a .cpp has callers only in that file") {
    const std::string here =
        "enum class Mode { A, B };\nstatic void helper() {\n    int x = 1;\n}\n"
        "void inside(Mode m) {\n    if (m == Mode::A) {\n        helper();\n    }\n}\n";
    PickRepo fx("precheck_pick_a_static",
                {{"src/s/here.cpp", here},
                 {"src/s/near.h", "#pragma once\nenum class Kind { A, B };\n"},
                 {"src/s/there.cpp",
                  "#include \"near.h\"\nvoid helper();\nvoid outside(Kind k) {\n    if (k == Kind::B) {\n"
                  "        helper();\n    }\n}\n"},
                 {"tests/test_mode_a.cpp", "int ma() { return static_cast<int>(Mode::A); }\n"},
                 {"tests/test_kind_b.cpp", "int kb() { return static_cast<int>(Kind::B); }\n"}});
    std::string tip = here;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"src/s/here.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_mode_a.cpp"});
}

TEST_CASE("precheck pick (a): a tools/ function's callers in tools/ are followed") {
    const std::string lib_cpp = "#include \"lib.h\"\nvoid step() {\n    int x = 1;\n}\n";
    PickRepo fx("precheck_pick_a_tools",
                {{"tools/x/lib.h", "#pragma once\nenum class Opt { Fast, Slow };\nvoid step();\n"},
                 {"tools/x/lib.cpp", lib_cpp},
                 {"tools/x/main.cpp",
                  "#include \"lib.h\"\nvoid drive(Opt o) {\n    if (o == Opt::Fast) {\n        step();\n    }\n}\n"},
                 {"tests/test_fast.cpp", "#include \"x/lib.h\"\nint f() { return static_cast<int>(Opt::Fast); }\n"}});
    std::string tip = lib_cpp;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"tools/x/lib.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_fast.cpp"});
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

// Step 5 under the user's ruling (plan, "User decisions", last paragraph): a
// test file counts for a name only if it reaches the header declaring it.
namespace {

const std::string kOneCpp = "#include \"one.h\"\nvoid run() {\n    int x = 1;\n}\n";

TestPick pick_two_runs(const std::string& name) {
    PickRepo fx(name, {{"src/a/one.h", "#pragma once\nvoid run();\n"},
                       {"src/a/one.cpp", kOneCpp},
                       {"src/b/two.h", "#pragma once\nvoid run();\n"},
                       {"tests/fix.h", "#pragma once\n#include \"a/one.h\"\n"},
                       {"tests/test_t_a.cpp", "#include \"a/one.h\"\nvoid ta() { run(); }\n"},
                       {"tests/test_t_b.cpp", "#include \"b/two.h\"\nvoid tb() { run(); }\n"},
                       {"tests/test_t_c.cpp", "#include \"fix.h\"\nvoid tc() { run(); }\n"}});
    std::string tip = kOneCpp;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    return fx.pick({{"src/a/one.cpp", tip}});
}

}  // namespace

TEST_CASE("precheck pick (j): a test naming an unrelated function of the same name is not picked") {
    const TestPick p = pick_two_runs("precheck_pick_j1");
    REQUIRE_FALSE(p.whole_suite);
    const std::vector<std::string> files = picked_files(p);
    CHECK(std::find(files.begin(), files.end(), "tests/test_t_a.cpp") != files.end());
    CHECK(std::find(files.begin(), files.end(), "tests/test_t_b.cpp") == files.end());
    CHECK(reasons_of(p, "tests/test_t_a.cpp") == std::vector<std::string>{"run"});
}

TEST_CASE("precheck pick (j): a test reaching the declaring header through a test helper header is picked") {
    const TestPick p = pick_two_runs("precheck_pick_j2");
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_t_a.cpp", "tests/test_t_c.cpp"});
    CHECK(reasons_of(p, "tests/test_t_c.cpp") == std::vector<std::string>{"run"});
}

TEST_CASE("precheck pick (j): a name in a file that defines main is found by its plain name") {
    const std::string main_cpp = "#include \"lib.h\"\nint step() {\n    return 1;\n}\nint main() { return step(); }\n";
    PickRepo fx("precheck_pick_j3", {{"tools/x/lib.h", "#pragma once\nint step();\n"},
                                     {"tools/x/main.cpp", main_cpp},
                                     {"tests/test_step.cpp", "int ts() { return step(); }\n"}});
    std::string tip = main_cpp;
    tip.replace(tip.find("return 1;"), 9, "return 2;");
    const TestPick p = fx.pick({{"tools/x/main.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_step.cpp"});
    CHECK(reasons_of(p, "tests/test_step.cpp") == std::vector<std::string>{"step"});
}

TEST_CASE("precheck pick (j): a deleted class still picks a test that names it, with no include") {
    const std::string base = "#pragma once\nclass Ghost {\npublic:\n    int g = 0;\n};\nint stay();\n";
    PickRepo fx("precheck_pick_j4", {{"src/g/ghost.h", base},
                                     {"tests/test_ghost.cpp", "int tg() { Ghost x; return x.g; }\n"},
                                     {"tests/test_none.cpp", "#include \"g/ghost.h\"\nint tn() { return 0; }\n"}});
    const TestPick p = fx.pick({{"src/g/ghost.h", "#pragma once\nint stay();\n"}});
    REQUIRE_FALSE(p.whole_suite);
    const PickName* g = name_in(p, "Ghost");
    REQUIRE(g != nullptr);
    CHECK(g->base_only);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_ghost.cpp"});
    CHECK(reasons_of(p, "tests/test_ghost.cpp") == std::vector<std::string>{"Ghost"});
}

TEST_CASE("precheck pick (j): a selector value picks only tests that reach its enum's header") {
    const std::string place_cpp =
        "#include \"rule.h\"\nvoid place() {\n    int k = 1;\n}\nvoid check(Rule r) {\n"
        "    if (r == Rule::New) {\n        place();\n    }\n}\n";
    PickRepo fx("precheck_pick_j5",
                {{"src/r/rule.h", "#pragma once\nenum class Rule { Old, New };\nvoid place();\nvoid check(Rule r);\n"},
                 {"src/r/place.cpp", place_cpp},
                 {"tests/test_rule_in.cpp", "#include \"r/rule.h\"\nRule ri() { return Rule::New; }\n"},
                 {"tests/test_rule_out.cpp", "enum class Rule { Old, New };\nRule ro() { return Rule::New; }\n"}});
    std::string tip = place_cpp;
    tip.replace(tip.find("int k = 1;"), 10, "int k = 2;");
    const TestPick p = fx.pick({{"src/r/place.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    bool selector = false;
    for (const CallerKey& k : p.callers) selector = selector || (k.key == "Rule::New" && k.selector);
    CHECK(selector);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_rule_in.cpp"});
    CHECK(reasons_of(p, "tests/test_rule_in.cpp") == std::vector<std::string>{"Rule::New"});
}

TEST_CASE("precheck pick (j): a function with no declaring header is found by its plain name") {
    const std::string lone = "void lone() {\n    int x = 1;\n}\n";
    PickRepo fx("precheck_pick_j6", {{"src/n/lone.cpp", lone},
                                     {"tests/test_lone.cpp", "void tl() { lone(); }\n"}});
    std::string tip = lone;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 2;");
    const TestPick p = fx.pick({{"src/n/lone.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_lone.cpp"});
    CHECK(reasons_of(p, "tests/test_lone.cpp") == std::vector<std::string>{"lone"});
}

TEST_CASE("precheck pick (j): a selector value whose left part is a namespace is found by its plain name") {
    const std::string place_cpp =
        "#include \"rule.h\"\nvoid place() {\n    int k = 1;\n}\nvoid check(int r) {\n"
        "    if (r == util::kFast) {\n        place();\n    }\n}\n";
    PickRepo fx("precheck_pick_j7",
                {{"src/r/rule.h", "#pragma once\nnamespace util { constexpr int kFast = 1; }\nvoid place();\n"},
                 {"src/r/place.cpp", place_cpp},
                 {"tests/test_util.cpp", "int u() { return util::kFast; }\n"}});
    std::string tip = place_cpp;
    tip.replace(tip.find("int k = 1;"), 10, "int k = 2;");
    const TestPick p = fx.pick({{"src/r/place.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    bool selector = false;
    for (const CallerKey& k : p.callers) selector = selector || (k.key == "util::kFast" && k.selector);
    CHECK(selector);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_util.cpp"});
}
